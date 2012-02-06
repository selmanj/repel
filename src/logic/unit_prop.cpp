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
#include "logic/predcollector.h"
#include "log.h"

QUnitsFormulasPair performUnitPropagation(const Domain& d) {
    LOG(LOG_INFO) << "performing unit propagation...";
    FormulaList formulas = d.formulas();

    // add quantification to any formulas that may be missing them
    for (FormulaList::iterator it = formulas.begin(); it != formulas.end(); it++) {
        if (!it->isQuantified()) {
            SISet everywhere(false, d.maxInterval());
            everywhere.add(d.maxSpanInterval());
            it->setQuantification(everywhere);
        }
    }
    QCNFClauseList clauses = convertToQCNFClauseList(formulas);

    if (!d.assumeClosedWorld()) {
        LOG(LOG_ERROR) << "doUnitProp(): cannot be called on a domain that is not a closed world - this code needs to be rewritten!";
        throw std::runtime_error("doUnitProp(): cannot be called on a domain that is not a closed world - this code needs to be rewritten!");
    }
    // convert all the facts into unit clauses
    Model obs = d.defaultModel();
    std::set<Atom, atomcmp> atoms = obs.atoms();

    for (std::set<Atom>::const_iterator it = atoms.begin(); it != atoms.end(); it++) {
        SISet trueAt = obs.getAtom(*it);
        SISet falseAt = trueAt.compliment();

        // TODO: why make a copy?  we should have the original shared_ptr
        boost::shared_ptr<Sentence> atomTrue(new Atom(*it));
        boost::shared_ptr<Sentence> atomFalse(new Negation(atomTrue));
        CNFClause a, b;
        a.push_back(atomTrue);
        b.push_back(atomFalse);
        QCNFClause c, d;
        c.first = a;
        d.first = b;
        c.second = trueAt;
        d.second = falseAt;

        if (c.second.size() != 0) clauses.push_back(c);
        if (d.second.size() != 0) clauses.push_back(d);
    }

    for (QCNFClauseList::iterator it = clauses.begin(); it != clauses.end(); it++) {
        std::cout << "clause: " << *it << std::endl;
    }

    QUnitsFormulasPair reducedList = performUnitPropagation(clauses);
    std::cout << "unit prop performed, now we have " << reducedList.first.size() << " unit clauses and " << reducedList.second.size() << " formulas:" << std::endl;

    for (QCNFLiteralList::const_iterator it = reducedList.first.begin(); it != reducedList.first.end(); it++) {
        std::cout << *it << std::endl;
    }

    std::cout << std::endl << "formulas:" << std::endl;
    std::copy(reducedList.second.begin(), reducedList.second.end(), std::ostream_iterator<QCNFClause>(std::cout, "\n"));
    return reducedList;
}

QUnitsFormulasPair performUnitPropagation(const QCNFClauseList& sentences) {
    QCNFClauseList formulas = sentences;

    // first, do a scan over the sentences, collecting unit clauses and collecting which atoms occur in each sentence
    QCNFLiteralList unitClauses;
    splitUnitClauses(formulas, unitClauses);

    LOG(LOG_DEBUG) << "found " << unitClauses.size() << " unit clauses.";

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
        splitUnitClauses(processedFormulas, unitClauses);
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
    LOG_PRINT(LOG_DEBUG) << "propagate_literal called with lit=" << lit;
    boost::shared_ptr<Sentence> cnfLit = lit.first;
    // first figure out what kind of literal we have here.
    std::queue<QCNFClause> toProcess;
    QCNFClauseList processed;

    toProcess.push(c);
    while (!toProcess.empty()) {
        QCNFClause qClause = toProcess.front();
        LOG_PRINT(LOG_DEBUG) << "working on " << convertFromQCNFClause(qClause).toString();
        CNFClause *cClause = &qClause.first;
        toProcess.pop();
        bool addCurrentClause = true;
        if (isSimpleLiteral(cnfLit)) {
            // search for an occurrence of this atom in c
            CNFClause::iterator it = cClause->begin();
            while (it != cClause->end()) {
                boost::shared_ptr<Sentence> currentLit = *it;
                if (isSimpleLiteral(currentLit) && *cnfLit == *currentLit) {    // Propagating P into P
                    addCurrentClause = propagateSimpleLitToSimpleLit(lit, qClause, it, toProcess);
                } else if (isSimpleLiteral(currentLit) && isNegatedLiteral(currentLit, cnfLit)) {
                    addCurrentClause = propagateNegSimpleLitToSimpleLit(lit, qClause, it, toProcess);
                } else if (boost::dynamic_pointer_cast<LiquidOp>(currentLit) != 0
                        && currentLit->contains(*cnfLit)) {    // propagating P into [...]
                    addCurrentClause = propagateSimpleLitToLiquidLit(lit, qClause, it, toProcess);
                } else if (boost::dynamic_pointer_cast<DiamondOp>(currentLit) != 0
                        && currentLit->contains(*cnfLit)) {
                    addCurrentClause = propagateSimpleLitToDiamond(lit, qClause, it, toProcess);
                }
                it++;
            }
        }
        if (addCurrentClause) processed.push_back(qClause);

    }
    return processed;
}

namespace {
    void splitUnitClauses(QCNFClauseList& sentences, QCNFLiteralList& unitClauses) {
        for (QCNFClauseList::iterator it = sentences.begin(); it != sentences.end(); ) {
            if (it->first.size() == 1) {
                CNFLiteral lit = it->first.front();
                QCNFLiteral qlit;
                qlit.first = lit;
                qlit.second = it->second;

                unitClauses.push_back(qlit);
                it = sentences.erase(it);
            } else {
                it++;
            }
        }
    }

    bool isSimpleLiteral(const boost::shared_ptr<Sentence>& lit) {
        if (boost::dynamic_pointer_cast<Atom>(lit) != 0) return true;
        if (boost::dynamic_pointer_cast<Negation>(lit) != 0) {
            boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(lit);
            if (boost::dynamic_pointer_cast<Atom>(neg->sentence()) != 0) {
                return true;
            }
        }
        return false;
    }

    bool isNegatedLiteral(const boost::shared_ptr<Sentence>& left, const boost::shared_ptr<Sentence>& right) {
        // one of them must be a negation
        if (boost::dynamic_pointer_cast<Negation>(left)) {
            boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(left);
            if (*neg->sentence() == *right) return true;
        }
        if (boost::dynamic_pointer_cast<Negation>(right)) {
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
            LOG_PRINT(LOG_DEBUG) << "propagating " << unit.first->toString() << " into " << (*lit)->toString() << std::endl;

            // if there is still a timepoint that the clause applies to, rewrite and continue
            SISet leftover = currentSet;
            leftover.subtract(intersect);
            if (leftover.size() != 0) {
                QCNFClause qRestricted = clause;
                qRestricted.second = leftover;
                newSentences.push(qRestricted);
                return false;
            }
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
            LOG_PRINT(LOG_DEBUG) << "propagating " << unit.first->toString() << " into " << (*lit)->toString() << std::endl;

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
        std::cout << "satisfiesRel = " << satisfiesRel.toString() << ", clause.second = " << clause.second.toString() << std::endl;
        SISet intersect = intersection(satisfiesRel, clause.second);
        std::cout << "intersect = " << intersect.toString() << std::endl;
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
            LOG(LOG_ERROR) << "propagating simple lit into liquid op currently not implemented! ignoring";
            return true;
        }
        LOG_PRINT(LOG_ERROR) << "warning, propagateSimpleLitTODiamond() called but lit propgated is neither negative or positive";
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
        boost::shared_ptr<LiquidOp> liqLiteral = boost::dynamic_pointer_cast<LiquidOp>(*lit);
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
                /*
                // two clauses, one for the intersection and one for the leftover
                SISet leftover = clauseAtLiq;
                leftover.subtract(intersect);
                if (!leftover.empty()) {
                    QCNFClause copy = clause;
                    copy.second = leftover;
                    newSentences.push(copy);
                }
                innerDisj.erase(it);
                */
            }
        }


        return true;
    }

}


QCNFClauseList convertToQCNFClauseList(const FormulaList& list) {
    QCNFClauseList result;
    for (FormulaList::const_iterator it = list.begin(); it != list.end(); it++) {
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
    QCNFClause qc;
    qc.first = c;
    qc.second = s;

    return qc;
}
