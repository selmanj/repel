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
#include "unit_prop.h"
#include "logic/predcollector.h"
#include "log.h"
/*
QCNFClauseList performUnitPropagation(const QCNFClauseList& sentences) {
	QCNFClauseList newSentences = sentences;

	// first, do a scan over the sentences, collecting unit clauses and collecting which atoms occur in each sentence
	QCNFLiteralList unitClauses;
	std::map<QCNFClauseList::iterator, std::list<Atom>, iterator_cmp > formToAtomsMap;
	for (QCNFClauseList::iterator it = newSentences.begin(); it != newSentences.end(); it++) {
		if (it->first.size() == 1) {
			CNFLiteral lit = it->first.front();
			QCNFLiteral qlit;
			qlit.first = lit;
			qlit.second = it->second;

			unitClauses.push_back(qlit);
			newSentences.erase(it);
		} else {
			PredCollector collector;
			CNFClause clause = it->first;
			//CNFClause clause = qclause.first;
			for (CNFClause::const_iterator cit = clause.begin(); cit != clause.end(); cit++) {
				(*cit)->visit(collector);
			}
			std::pair<QCNFClauseList::iterator, std::list<Atom> > newPair;
			newPair.first = it;
			newPair.second = std::list<Atom>(collector.preds.begin(), collector.preds.end());

			formToAtomsMap.insert(newPair);
		}
	}

	LOG(LOG_DEBUG) << "found " << unitClauses.size() << " unit clauses.";

	throw std::runtime_error("performUnitPropagation unimplemented");
	//return NULL;
}
*/
/*
QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c) {
	CNFClause clause = c.first;
	return propagate_literal(lit, c, clause.begin(), clause.end());
}
*/

QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c) {
	boost::shared_ptr<Sentence> cnfLit = lit.first;
	// first figure out what kind of literal we have here.
	std::queue<QCNFClause> toProcess;
	QCNFClauseList processed;

	toProcess.push(c);
	while (!toProcess.empty()) {
		QCNFClause qClause = toProcess.front();
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
				}
				it++;
			}
		}
		if (addCurrentClause) processed.push_back(qClause);

	}
	return processed;
}

namespace {
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
			if (neg->sentence() == right) return true;
		}
		if (boost::dynamic_pointer_cast<Negation>(right)) {
			boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(right);
			if (neg->sentence()->toString() == left->toString()) {	// TODO Why do I have to compare by string here???
				return true;
			}
		}
		return false;
	}
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
	SharedSentence* curDis;
	CNFClause copy = c.first;
	SharedSentence firstS = copy.front();
	copy.pop_front();
	SharedSentence secondS = copy.front();
	copy.pop_front();

	SharedSentence firstDis(new Disjunction(firstS, secondS));
	curDis = &firstDis;
	while(!copy.empty()) {
		SharedSentence nextS = copy.front();
		copy.pop_front();
		SharedSentence nextDis(new Disjunction(*curDis, nextS));
		curDis = &nextDis;
	}
	ELSentence els(*curDis);
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

