/*
 * unit_prop.cpp
 *
 *  Created on: Dec 11, 2011
 *      Author: joe
 */

#include <exception>
#include <stdexcept>
#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include "unit_prop.h"
#include "logic/domain.h"
#include "logic/collectors.h"
#include "log.h"

Domain performUnitPropagation(const Domain& d) {
    LOG(LOG_INFO) << "performing unit propagation...";
    std::vector<ELSentence> upforms(d.formulas_begin(), d.formulas_end());
    std::vector<ELSentence> leftover;

    // add quantification to any formulas that may be missing them, and remove
    // upforms that aren't in CNF form
    for (std::vector<ELSentence>::iterator it = upforms.begin(); it != upforms.end(); ) {
        // if it's not infinitely weighted, its not suitable
        if (!it->hasInfWeight()) {
            leftover.push_back(*it);
            it = upforms.erase(it);
            continue;
        }
        if (!it->isQuantified()) {
            SISet everywhere(d.maxSpanInterval(), false, d.maxInterval());
            it->setQuantification(everywhere);
        }
        // check for cnf
        if (!isPELCNFLiteral(*it->sentence()) && !isDisjunctionOfPELCNFLiterals(*it->sentence())) {
            LOG_PRINT(LOG_WARN) << "Sentence: " << *it << " is not in CNF form!  ignoring";
            leftover.push_back(*it);
            it = upforms.erase(it);
            continue;
        }
        it++;
    }
    QCNFClauseList clauses = convertToQCNFClauseList(upforms);

    // convert all the facts into unit clauses
    Model obs = d.defaultModel();
    std::set<Atom, atomcmp> atoms = obs.atoms();

    for (std::set<Atom, atomcmp>::const_iterator it = atoms.begin(); it != atoms.end(); it++) {
        SISet trueAt = obs.getAtom(*it);
        SISet falseAt = trueAt.compliment();    // TODO: fixi t here

        // TODO: why make a copy?  we should have the original shared_ptr
        boost::shared_ptr<Sentence> atomTrue(new Atom(*it));
        boost::shared_ptr<Sentence> atomFalse(new Negation(atomTrue));
        CNFClause a, b;
        a.push_back(atomTrue);
        b.push_back(atomFalse);
        QCNFClause c(a, trueAt), d(b, falseAt);
        /*
        c.first = a;
        d.first = b;
        c.second = trueAt;
        d.second = falseAt;
        */
        if (c.second.size() != 0) clauses.push_back(c);
        if (d.second.size() != 0) clauses.push_back(d);
    }

    QUnitsFormulasPair reducedList = performUnitPropagation(clauses);
    Domain newD;
    std::stringstream newForms;
    newForms << "Unit Clauses:\n";
    for (QCNFLiteralList::const_iterator it = reducedList.first.begin(); it != reducedList.first.end(); it++) {
        ELSentence newS = convertFromQCNFClause(*it);
        newS.setHasInfWeight(true);
        newForms << "\t" << newS << "\n";
        if (isSimpleLiteral(*newS.sentence()))
            newD.addFact(newS);
        else
            newD.addFormula(newS);
    }
    newForms << "upforms:\n";
    for (QCNFClauseList::const_iterator it = reducedList.second.begin(); it != reducedList.second.end(); it++) {
        ELSentence newS = convertFromQCNFClause(*it);
        // should have inf weight
        newS.setHasInfWeight(true);
        newForms << "\t" << newS << "\n";
        newD.addFormula(newS);
    }
    LOG(LOG_INFO) << "unit prop completed.\n" << newForms.str();

    // copy in all the unweighted upforms from the original d
    for (Domain::formula_const_iterator it = leftover.begin(); it != leftover.end(); it++) {
        newD.addFormula(*it);
    }
    return newD;
}

QUnitsFormulasPair performUnitPropagation(const QCNFClauseList& sentences) {
    QCNFClauseList formulas = sentences;

    // first, do a scan over the sentences, collecting unit clauses and collecting which atoms occur in each sentence
    QCNFLiteralList unitClauses;
    splitUnitClauses(formulas, unitClauses);
    LOG(LOG_DEBUG) << "found " << unitClauses.size() << " unit clauses.";

    // maintain our map of where each unit must be true/false so we can
    // detect inconsistencies
    boost::unordered_map<Proposition, SISet> partialModel;
    // ensure we have no contradictions at this point
    enforceUnitProps(unitClauses, partialModel);

    QCNFLiteralList propagatedUnitClauses;

    while (!unitClauses.empty() && !formulas.empty()) {
        // pop one out, propagate it in all our sentences
        QCNFLiteral unitClause = unitClauses.front();
        unitClauses.pop_front();

        QCNFClauseList processedFormulas;
        for (QCNFClauseList::iterator it = formulas.begin(); it != formulas.end(); it++) {
            QCNFClauseList newFormulas = propagateLiteral(unitClause, *it);
            processedFormulas.insert(processedFormulas.end(), newFormulas.begin(), newFormulas.end());
        }
        QCNFLiteralList newUnitClauses;
        splitUnitClauses(processedFormulas, newUnitClauses);
        enforceUnitProps(newUnitClauses, partialModel); // double check that we got no contradictions
        unitClauses.insert(unitClauses.end(), newUnitClauses.begin(), newUnitClauses.end());
        formulas = processedFormulas;

        propagatedUnitClauses.push_back(unitClause);    // finished propagating

    }
    QUnitsFormulasPair result;
    result.first = propagatedUnitClauses;
    // add any remaining unit clauses
    result.first.insert(result.first.end(), unitClauses.begin(), unitClauses.end());
    result.second = formulas;

    return result;



    //throw std::runtime_error("performUnitPropagation unimplemented");
    //return NULL;
}

/*
QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c) {
    CNFClause clause = c.first;
    return propagate_literal(lit, c, clause.begin(), clause.end());
}
*/

QCNFClauseList propagateLiteral(const QCNFLiteral& lit, const QCNFClause& c) {
    LOG(LOG_DEBUG) << "propagate_literal called with lit=" << lit;
    boost::shared_ptr<Sentence> cnfLit = lit.first;
    // first figure out what kind of literal we have here.
    std::queue<QCNFClause> toProcess;
    QCNFClauseList processed;

    toProcess.push(c);
    while (!toProcess.empty()) {
        QCNFClause qClause = toProcess.front();
        LOG(LOG_DEBUG) << "working on " << convertFromQCNFClause(qClause).toString();
        CNFClause *cClause = &qClause.first;
        toProcess.pop();
        bool addCurrentClause = true;
        if (isSimpleLiteral(*cnfLit)) {
            // if we have a negation, unwrap it so we can find the base proposition easier
            boost::shared_ptr<Sentence> baseProp = cnfLit;
            if (boost::dynamic_pointer_cast<Negation>(baseProp).get()!=0) {
                baseProp = boost::dynamic_pointer_cast<Negation>(baseProp)->sentence(); // unwrap
            }
            // search for an occurrence of this atom in c
            CNFClause::iterator it = cClause->begin();
            while (it != cClause->end()) {
                boost::shared_ptr<Sentence> currentLit = *it;
                if (isSimpleLiteral(*currentLit) && *cnfLit == *currentLit) {    // Propagating P into P
                    addCurrentClause = propagateSimpleLitToSimpleLit(lit, qClause, it, toProcess);
                } else if (isSimpleLiteral(*currentLit) && isNegatedLiteral(currentLit, cnfLit)) {
                    addCurrentClause = propagateNegSimpleLitToSimpleLit(lit, qClause, it, toProcess);
                } else if (boost::dynamic_pointer_cast<LiquidOp>(currentLit) != 0
                        && currentLit->contains(*baseProp)) {    // propagating P into [...]
                    addCurrentClause = propagateSimpleLitToLiquidLit(lit, qClause, it, toProcess);
                } else if (boost::dynamic_pointer_cast<DiamondOp>(currentLit) != 0
                        && currentLit->contains(*baseProp)) {
                    addCurrentClause = propagateSimpleLitToDiamond(lit, qClause, it, toProcess);
                }
                if (!addCurrentClause) break;
                it++;
            }
        }
        if (addCurrentClause) {
            processed.push_back(qClause);
        }
    }
    return processed;
}

namespace {
    void splitUnitClauses(QCNFClauseList& sentences, QCNFLiteralList& unitClauses) {
        for (QCNFClauseList::iterator it = sentences.begin(); it != sentences.end(); ) {
            if (it->first.size() == 1) {
                CNFLiteral lit = it->first.front();
                SISet where = it->second;
                // double check that if we have a liquid op, there's only one literal in it
                boost::shared_ptr<LiquidOp> asLiq = boost::dynamic_pointer_cast<LiquidOp>(lit);
                if (asLiq) {
                   CNFClause innerDisj = convertToCNFClause(asLiq->sentence());
                   if (innerDisj.size() != 1) {
                       it++; // bail out
                       continue;
                   } else {
                       lit = innerDisj.front();
                       // convert to liquid
                       SISet newWhere(true, it->second.maxInterval());
                       for (SISet::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                           newWhere.add(it2->toLiquidInc());
                       }
                       where = newWhere;
                   }
                }

                QCNFLiteral qlit(lit, where);
                //qlit.first = lit;
                //qlit.second = where;

                unitClauses.push_back(qlit);
                it = sentences.erase(it);
            } else {
                it++;
            }
        }
    }
    void enforceUnitProps(const QCNFLiteralList& unitClauses, boost::unordered_map<Proposition, SISet>& partialModel) {
        for (QCNFLiteralList::const_iterator it = unitClauses.begin(); it != unitClauses.end(); it++) {
            try {
                Proposition unitProp = convertToProposition(*it);
                Proposition iUnitProp = unitProp.inverse();
                SISet where = it->second;

                // check to see if its negated form is in partial model
                if (partialModel.count(iUnitProp) == 0) {
                    // add it in, woo!
                    if (partialModel.count(unitProp) == 0) {
                        partialModel.insert(std::make_pair(unitProp, where)); // do this so max interval is preserved
                    } else {
                        partialModel.find(unitProp)->second.add(where);
                    }
                } else {
                    // if there's an intersection, throw an exception
                    SISet iTrueAt = partialModel.find(iUnitProp)->second;
                    SISet intersect = intersection(iTrueAt, where);
                    if (!intersect.empty()) {
                        throw contradiction();
                    } else {
                        // safe to add
                        if (partialModel.count(unitProp) == 0) {
                            partialModel.insert(std::make_pair(unitProp, where)); // do this so max interval is preserved
                        } else {
                            partialModel.find(unitProp)->second.add(where);
                        }
                    }
                }
             } catch (std::invalid_argument& e) {
                continue;   // skip this guy for now
             }
        }
    }

    Proposition convertToProposition(const QCNFLiteral& lit) {
        if (       lit.first->getTypeCode() == Atom::TypeCode) {   // we compare this way rather than a dynamic cast (should be faster)
            // perfect, this is easy
            Atom atom = *(boost::shared_static_cast<Atom>(lit.first));
            return Proposition(atom, true);
        } else if (lit.first->getTypeCode() == Negation::TypeCode) {
             // TODO: safe to assume it must be an atom inside?
            Atom innerAtom = *(boost::shared_static_cast<Atom>(     // double cast!  its so bright and VIVID
                    boost::shared_static_cast<Negation>(lit.first)->sentence()));
            return Proposition(innerAtom, false);
        } else if (lit.first->getTypeCode() == LiquidOp::TypeCode) {
            // just step on through
            QCNFLiteral innerLit = lit;
            innerLit.first = boost::shared_static_cast<LiquidOp>(lit.first)->sentence();
            return convertToProposition(innerLit);
        }
        throw std::invalid_argument("convertToProposition(): got passed a lit that we don't know how to handle!");

    }

    bool isNegatedLiteral(boost::shared_ptr<Sentence> left, boost::shared_ptr<Sentence> right) {
        // one of them must be a negation
        if (left->getTypeCode() == Negation::TypeCode) {
            boost::shared_ptr<Negation> neg = boost::static_pointer_cast<Negation>(left);
            if (*neg->sentence() == *right) return true;
        }
        if (right->getTypeCode() == Negation::TypeCode) {
            boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(right);
            if (*neg->sentence() == *left) {
                return true;
            }
        }
        return false;
    }

    bool propagateSimpleLitToSimpleLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences) {
        // if they intersect, rewrite the clause over the time where they don't intersect and continue
        SISet litSet = unit.second;
        SISet currentSet = clause.second;
        SISet intersect = intersection(litSet, currentSet);

        if (intersect.size() != 0) {
            LOG(LOG_DEBUG) << "propagating " << unit.first->toString() << " into " << (*lit)->toString() << std::endl;

            // if there is still a timepoint that the clause applies to, rewrite and continue
            SISet leftover = currentSet;
            leftover.subtract(intersect);
            if (leftover.size() != 0) {
                QCNFClause qRestricted = clause;
                qRestricted.second = leftover;
                newSentences.push(qRestricted);
            }
            return false;

        }
        return true;
    }

    bool propagateNegSimpleLitToSimpleLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences) {
        // propagating !P into P (or vice versa)
        // if they intersect, generate two clauses
        SISet litSet = unit.second;
        SISet currentSet = clause.second;
        SISet intersect = intersection(litSet, currentSet);

        if (intersect.size() != 0) {
            LOG(LOG_DEBUG) << "propagating " << unit.first->toString() << " into " << (*lit)->toString() << std::endl;

            SISet leftover = currentSet;
            leftover.subtract(intersect);
            if (leftover.size() != 0) {
                // add a copy of this sentence only over leftover
                QCNFClause qRestricted = clause;
                qRestricted.second = leftover;
                newSentences.push(qRestricted);
            }
            // delete the current literal and rewrite the time where it applies
            lit = clause.first.erase(lit);
            clause.second = intersect;
        }
        return true;
    }

    bool propagateSimpleLitToDiamond(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences) {
        boost::shared_ptr<DiamondOp> diamondLit = boost::dynamic_pointer_cast<DiamondOp>(*lit);

        // double check that there is only one relation for now
        if (diamondLit->relations().size() != 1) {
            LOG(LOG_WARN) << "handling more than one relation on a single diamond literal is currently unimplemented in unit_prop()";
            return true;
        }

        Interval::INTERVAL_RELATION rel = *(diamondLit->relations().begin());
        SISet satisfiesRel = unit.second.satisfiesRelation(rel);
        SISet intersect = intersection(satisfiesRel, clause.second);
        // if they don't intersect, nothing to propagate
        if (intersect.size() == 0) {
           return true;
        }

        if (*unit.first == *diamondLit->sentence()) {
            // clause is satisfied, we can drop it (over the intersection that is)
            SISet leftover = clause.second;
            leftover.subtract(intersect);
            if (leftover.size() != 0) {
                QCNFClause qRestricted = clause;
                qRestricted.second = leftover;
                newSentences.push(qRestricted);
                return false;
            }
            return false;   // just drop it
        } else if (isNegatedLiteral(unit.first, diamondLit->sentence())) {
            // fancy stuff happens here
            LOG(LOG_ERROR) << "propagating simple lit into diamond op containing negative currently not implemented! ignoring";
            return true;
        }
        LOG(LOG_ERROR) << "warning, propagateSimpleLitTODiamond() called but lit propgated is neither negative or positive";
        return true;
    }

    bool propagateSimpleLitToLiquidLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences) {
        // first, convert our SISets into liquid ones
        SISet unitAtLiq = unit.second;
        unitAtLiq.setForceLiquid(true);
        SISet clauseAtLiq = clause.second;
        clauseAtLiq.setForceLiquid(true);

        // if they don't intersect, no reason to continue
        SISet intersect = intersection(unitAtLiq, clauseAtLiq);
        if (intersect.empty()) return true;

        // this should be a lot simpler since we are now dealing with disjunctions
        boost::shared_ptr<LiquidOp> liqLiteral = boost::static_pointer_cast<LiquidOp>(*lit);
        CNFClause innerDisj = convertToCNFClause(liqLiteral->sentence());

        for(CNFClause::iterator it = innerDisj.begin(); it != innerDisj.end(); it++) {
            if (**it == *unit.first) {
                // we can rewrite this sentence by removing the intersection
                SISet leftover = clause.second;
                leftover.subtract(intersect);
                //leftover.setForceLiquid(true);
                if (leftover.empty()) {
                    return false;
                }
                // can't we just do this?
                clause.second = leftover;
                // update the new intersection
                clauseAtLiq = leftover;
                clauseAtLiq.setForceLiquid(true);
                intersect = intersection(unitAtLiq, clauseAtLiq);
                /*
                QCNFClause copy = clause;
                boost::shared_ptr<Sentence> newInner = convertFromCNFClause(innerDisj);
                boost::shared_ptr<Sentence> liqCopy(new LiquidOp(newInner));
                throw std::runtime_error("not finished!  resume here");
                //copy.first =
                copy.second = leftover;
                newSentences.push(copy);
                */
            } else if (isNegatedLiteral(*it, unit.first)) {
                // two clauses, one for the intersection and one for the leftover

                // make a copy of the clause, replacing the liquid literal with a new one
                CNFClause newInnerDisj = innerDisj;
                newInnerDisj.remove(*it);
                if (newInnerDisj.size() == 0) {
                    // no more liquid literal here
                    QCNFClause smallerClause = clause;
                    smallerClause.first.remove(*lit);
                    if (smallerClause.first.size() != 0) {
                        newSentences.push(smallerClause);
                    }
                } else {
                    boost::shared_ptr<Sentence> newDisj = convertFromCNFClause(newInnerDisj);
                    boost::shared_ptr<Sentence> newLiq(new LiquidOp(newDisj));

                    QCNFClause smallerClause = clause;
                    std::replace(smallerClause.first.begin(), smallerClause.first.end(), *lit, newLiq);
                    smallerClause.second = intersect;
                    newSentences.push(smallerClause);
                }
                SISet leftover = clause.second;
                leftover.subtract(intersect);
                if (leftover.empty()) {
                    return false;
                } else {
                    // update where the current clause applies
                    clause.second = leftover;
                    // update the new intersection
                    clauseAtLiq = leftover;
                    clauseAtLiq.setForceLiquid(true);
                    intersect = intersection(unitAtLiq, clauseAtLiq);
                }
            }
        }

        return true;
    }
}


QCNFClauseList convertToQCNFClauseList(const std::vector<ELSentence>& list) {
    QCNFClauseList result;
    for (std::vector<ELSentence>::const_iterator it = list.begin(); it != list.end(); it++) {
        if (it->hasInfWeight()) {
            result.push_back(convertToQCNFClause(*it));
        }
    }
    return result;
}

CNFClause convertToCNFClause(boost::shared_ptr<Sentence> s) {
    CNFClause c;
    boost::shared_ptr<Disjunction> d = boost::dynamic_pointer_cast<Disjunction>(s);
    if (d.get() == NULL) {
        // not given a disjunction, this is the only literal
        c.push_back(s);
        return c;
    }
    std::queue<boost::shared_ptr<Disjunction> > disjQueue;
    disjQueue.push(d);

    while (!disjQueue.empty()) {
        boost::shared_ptr<Disjunction> curDisj = disjQueue.front();
        disjQueue.pop();
        boost::shared_ptr<Sentence> curLeft = curDisj->left();
        boost::shared_ptr<Sentence> curRight = curDisj->right();

        boost::shared_ptr<Disjunction> leftDisj = boost::dynamic_pointer_cast<Disjunction>(curLeft);
        boost::shared_ptr<Disjunction> rightDisj = boost::dynamic_pointer_cast<Disjunction>(curRight);

        if (leftDisj.get() == NULL) {
            c.push_back(curLeft);
        } else {
            disjQueue.push(leftDisj);
        }

        if (rightDisj.get() == NULL) {
            c.push_back(curRight);
        } else {
            disjQueue.push(rightDisj);
        }
    }

    return c;
}

ELSentence convertFromQCNFClause(const QCNFLiteral& c) {
    ELSentence s(c.first);
    s.setQuantification(c.second);
    return s;
}

ELSentence convertFromQCNFClause(const QCNFClause& c) {
    boost::shared_ptr<Sentence> s = convertFromCNFClause(c.first);
    ELSentence els(s);
    els.setQuantification(c.second);
    return els;
}

boost::shared_ptr<Sentence> convertFromCNFClause(const CNFClause& c) {
    if (c.empty()) {
        throw std::invalid_argument("in convertFromQCNFClause(): cannot make a clause from an empty QCNFClause");
    }
    if (c.size() == 1) {
        return c.front();
    }
    CNFClause copy = c;
    typedef boost::shared_ptr<Sentence> SharedSentence;

    SharedSentence firstS = copy.front();
    copy.pop_front();
    SharedSentence secondS = copy.front();
    copy.pop_front();
    SharedSentence firstDis(new Disjunction(firstS, secondS));
    SharedSentence curDis = firstDis;
    while(!copy.empty()) {
        SharedSentence nextS = copy.front();
        copy.pop_front();
        SharedSentence nextDis(new Disjunction(curDis, nextS));
        curDis = nextDis;
    }
    return curDis;
}

QCNFClause convertToQCNFClause(const ELSentence& el) {
    boost::shared_ptr<Sentence> copy(el.sentence()->clone());
    CNFClause c = convertToCNFClause(copy);
    if (!el.isQuantified()) {
        throw std::logic_error("logic error: given an ELSentence with no quantification and asked to convert to QCNFClause");
    }
    SISet s = el.quantification();
    QCNFClause qc(c, s);
    //qc.first = c;
    //qc.second = s;

    return qc;
}
