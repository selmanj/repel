/*
 * siset.cpp
 *
 *  Created on: May 24, 2011
 *      Author: joe
 */
#include <set>
#include "siset.h"
#include "spaninterval.h"

// O(n^2) :(  perhaps set a flag instead?
bool SISet::isDisjoint() const {
	for (std::set<SpanInterval>::const_iterator fIt = set_.begin(); fIt != set_.end(); fIt++) {
		for (std::set<SpanInterval>::const_iterator sIt = fIt; sIt != set_.end(); sIt++) {
			// dont compare to yourself
			if (sIt == fIt) {
				continue;
			}
			if (!fIt->intersection(*sIt).isEmpty()) {
				return false;
			}
		}
	}
	return true;
}

void SISet::makeDisjoint() {
	// scan over all pairs, looking for intersections
	for (std::set<SpanInterval>::const_iterator fIt = set_.begin(); fIt != set_.end(); fIt++) {
		for (std::set<SpanInterval>::const_iterator sIt = fIt; sIt != set_.end(); sIt++) {
			// don't compare to yourself
			if (sIt == fIt) {
				continue;
			}
			SpanInterval intersect = fIt->intersection(*sIt);
			if (!intersect.isEmpty()) {

			}
		}
	}
}
