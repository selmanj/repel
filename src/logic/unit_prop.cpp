/*
 * unit_prop.cpp
 *
 *  Created on: Dec 11, 2011
 *      Author: joe
 */

#include "unit_prop.h"

std::list<QCNFClause> propagate_literal(const QCNFLiteral& lit, const QCNFClause& c) {
	CNFClause clause = c.first;
	return propagate_literal(lit, c, clause.begin(), clause.end());
}

std::list<QCNFClause> propagate_literal(const QCNFLiteral& lit, const QCNFClause& c, const CNFClause::const_iterator& begin, const CNFClause::const_iterator& end) {
	boost::shared_ptr<Sentence> cnfLit = lit.first;
	// first figure out what kind of literal we have here.
	if (boost::dynamic_pointer_cast<Atom>(cnfLit)) {
		boost::shared_ptr<Atom> atom = boost::dynamic_pointer_cast<Atom>(cnfLit);
		CNFClause cClause = c.first;
		// search for an occurrence of this atom in c
		for (CNFClause::const_iterator it = begin; it != end; it++) {
			boost::shared_ptr<Sentence> currentLit = *it;
			if (boost::dynamic_pointer_cast<Atom>(currentLit)) {
				boost::shared_ptr<Atom> currentAtom = boost::dynamic_pointer_cast<Atom>(currentLit);
				// continue if they're not the same
				if (*atom != *currentAtom) continue;
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
	std::list<QCNFClause> results;
	results.push_back(c);
	return results;




}
