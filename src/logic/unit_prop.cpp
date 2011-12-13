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
#include "unit_prop.h"
#include "logic/predcollector.h"
#include "log.h"

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

QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c) {
	CNFClause clause = c.first;
	return propagate_literal(lit, c, clause.begin(), clause.end());
}

QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c, const CNFClause::const_iterator& begin, const CNFClause::const_iterator& end) {
	boost::shared_ptr<Sentence> cnfLit = lit.first;
	// first figure out what kind of literal we have here.
	if (isSimpleLiteral(cnfLit)) {
		CNFClause cClause = c.first;
		// search for an occurrence of this atom in c
		for (CNFClause::const_iterator it = begin; it != end; it++) {
			boost::shared_ptr<Sentence> currentLit = *it;
			if (isSimpleLiteral(currentLit) && *cnfLit == *currentLit) {
				// if they intersect, rewrite the clause over the time where they don't intersect and continue
				SISet litSet = lit.second;
				SISet currentSet = c.second;
				SISet intersect = intersection(litSet, currentSet);

				if (intersect.size() == 0) continue;
				// if there is still a timepoint that the clause applies to, rewrite and continue
				SISet leftover = currentSet;
				leftover.subtract(intersect);
				if (leftover.size() != 0) {
					QCNFClause cRestricted = c;
					cRestricted.second = leftover;
					return propagate_literal(lit, cRestricted, ++it, end);
				}
			}
		}
	}
	QCNFClauseList results;
	results.push_back(c);
	return results;
}

namespace {
	bool isSimpleLiteral(const boost::shared_ptr<Sentence>& lit) {
		if (boost::dynamic_pointer_cast<Atom>(lit)) return true;
		if (boost::dynamic_pointer_cast<Negation>(lit)) {
			boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(lit);
			if (boost::dynamic_pointer_cast<Atom>(neg->sentence())) {
				return true;
			}
		}
		return false;
	}
}

