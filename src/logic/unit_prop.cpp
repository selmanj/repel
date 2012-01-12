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
#include "logic/predcollector.h"
#include "log.h"

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
			QCNFClauseList newFormulas = propagate_literal(unitClause, *it);
			processedFormulas.insert(processedFormulas.end(), newFormulas.begin(), newFormulas.end());
		}
		splitUnitClauses(processedFormulas, unitClauses);
		formulas = processedFormulas;

		propagatedUnitClauses.push_back(unitClause);	// finished propagating

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

QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c) {
	LOG_PRINT(LOG_DEBUG) << "propagate_literal called with lit=" << lit.first->toString()
			<< " and clause c=" << convertFromQCNFClause(c).toString() << std::endl;
	boost::shared_ptr<Sentence> cnfLit = lit.first;
	// first figure out what kind of literal we have here.
	std::queue<QCNFClause> toProcess;
	QCNFClauseList processed;

	toProcess.push(c);
	while (!toProcess.empty()) {
		QCNFClause qClause = toProcess.front();
		LOG_PRINT(LOG_DEBUG) << "working on " << convertFromQCNFClause(qClause).toString() << std::endl;
		CNFClause cClause = qClause.first;
		toProcess.pop();
		bool addCurrentClause = true;
		if (isSimpleLiteral(cnfLit)) {
			// search for an occurrence of this atom in c
			CNFClause::iterator it = cClause.begin();
			while (it != cClause.end()) {
				boost::shared_ptr<Sentence> currentLit = *it;
				if (isSimpleLiteral(currentLit) && *cnfLit == *currentLit) {	// Propagating P into P
					// if they intersect, rewrite the clause over the time where they don't intersect and continue
					SISet litSet = lit.second;
					SISet currentSet = qClause.second;
					SISet intersect = intersection(litSet, currentSet);

					if (intersect.size() != 0) {
						LOG_PRINT(LOG_DEBUG) << "propagating " << cnfLit->toString() << " into " << currentLit->toString() << std::endl;

						// if there is still a timepoint that the clause applies to, rewrite and continue
						SISet leftover = currentSet;
						leftover.subtract(intersect);
						if (leftover.size() != 0) {
							QCNFClause qRestricted = qClause;
							qRestricted.second = leftover;
							toProcess.push(qRestricted);
							addCurrentClause = false;
							break;
						}
					}
				} else if (isSimpleLiteral(currentLit) && isNegatedLiteral(currentLit, cnfLit)) {
					// propagating !P into P (or vice versa)
					// if they intersect, generate two clauses
					SISet litSet = lit.second;
					SISet currentSet = qClause.second;
					SISet intersect = intersection(litSet, currentSet);

					if (intersect.size() != 0) {
						LOG_PRINT(LOG_DEBUG) << "propagating " << cnfLit->toString() << " into " << currentLit->toString() << std::endl;

						SISet leftover = currentSet;
						leftover.subtract(intersect);
						if (leftover.size() != 0) {
							// add a copy of this sentence only over leftover
							QCNFClause qRestricted = qClause;
							qRestricted.second = leftover;
							toProcess.push(qRestricted);
						}
						// delete the current literal and rewrite the time where it applies
						it = cClause.erase(it);
						qClause.first = cClause;
						qClause.second = intersect;
						continue;
					}
				} else if (boost::dynamic_pointer_cast<LiquidOp>(currentLit) != 0) {	// propagating P into [...]
					boost::shared_ptr<LiquidOp> liqLit = boost::dynamic_pointer_cast<LiquidOp>(currentLit);
					// first convert the time into a liquid interval
					SISet liqLitSet = lit.second;
					liqLitSet.setForceLiquid(true);
					SISet currentSet = qClause.second;
					SISet intersect = intersection(liqLitSet, currentSet);
					if (intersect.size() != 0) {
						// this is kinda cheap but it should work
						// call this function again on the inside and fix the resulting sentence if necessary
						QCNFLiteral newLit;
						newLit.first = lit.first;
						newLit.second = liqLitSet;

						QCNFClause newClause;
						SISet liqClauseSet = currentSet;
						liqClauseSet.setForceLiquid(true);
						newClause.first = convertToCNFClause(liqLit);
						newClause.second = liqClauseSet;
						QCNFClauseList newClauseList = propagate_literal(newLit, newClause);

						// TODO: complete this!
						throw std::runtime_error("unimplemented!");
					}
				} else if (boost::dynamic_pointer_cast<DiamondOp>(currentLit) != 0) {
					boost::shared_ptr<DiamondOp> diaCurrentLit = boost::dynamic_pointer_cast<DiamondOp>(currentLit);
					// check to make sure we can propagate here

					if (!(*diaCurrentLit->sentence() == *cnfLit) && isNegatedLiteral(diaCurrentLit->sentence(), cnfLit)) {
						it++;
						continue;
					}

					// double check that there is only one relation for now
					if (diaCurrentLit->relations().size() != 1) {
						throw std::runtime_error("handling more than one relation on a single diamond literal is currently unimplemented in unit_prop()");
					}

					Interval::INTERVAL_RELATION rel = *(diaCurrentLit->relations().begin());
					SISet satisfiesRel = lit.second.satisfiesRelation(rel);
					std::cout << "satisfiesRel = " << satisfiesRel.toString() << ", qclause.second = " << qClause.second.toString() << std::endl;
					SISet intersect = intersection(satisfiesRel, qClause.second);
					std::cout << "intersect = " << intersect.toString() << std::endl;
					// if they don't intersect, nothing to propagate
					if (intersect.size() == 0) {
						it++;
						continue;
					}

					if (*cnfLit == *diaCurrentLit->sentence()) {
						// clause is satisfied, we can drop it (over the intersection that is)
						SISet leftover = qClause.second;
						leftover.subtract(intersect);
						if (leftover.size() != 0) {
							QCNFClause qRestricted = qClause;
							qRestricted.second = leftover;
							toProcess.push(qRestricted);
							addCurrentClause = false;
							break;
						}
					} else if (isNegatedLiteral(cnfLit, diaCurrentLit->sentence())) {

					}


				}
				it++;
			}
		}
		if (addCurrentClause) processed.push_back(qClause);

	}
	LOG_PRINT(LOG_DEBUG) << "returning..." << std::endl;
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
}

QCNFClauseList convertToQCNFClauseList(const FormulaList& list) {
	QCNFClauseList result;
	for (FormulaList::const_iterator it = list.begin(); it != list.end(); it++) {
		result.push_back(convertToQCNFClause(*it));
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
	if (c.first.empty()) {
		throw std::invalid_argument("in convertFromQCNFClause(): cannot make a clause from an empty QCNFClause");
	}
	if (c.first.size() == 1) {
		ELSentence s(c.first.front());
		s.setQuantification(c.second);
		return s;
	}
	typedef boost::shared_ptr<Sentence> SharedSentence;
	CNFClause copy = c.first;
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
	ELSentence els(curDis);
	els.setQuantification(c.second);
	return els;
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

