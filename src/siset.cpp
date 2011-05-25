/*
 * siset.cpp
 *
 *  Created on: May 24, 2011
 *      Author: joe
 */
#include <set>
#include <list>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "siset.h"
#include "spaninterval.h"

SISet SISet::compliment() const {

	// {A U B U C U.. }^c = A^c I B^c I ...
	// {A U B U ..} intersect {C U D U } .. intersect D
	// expensive operation!  faster way to do this?
	std::list<std::set<SpanInterval> > intersections;
	for (std::set<SpanInterval>::const_iterator it = set_.begin(); it != set_.end(); it++) {
		std::set<SpanInterval> compliment;
		if (forceLiquid_) {
			it->liqCompliment(compliment);
		} else {
			it->compliment(compliment);
		}
		intersections.push_back(compliment);
	}

	// now we have a list of set unions that we need to intersect; perform pairwise intersection
	while (intersections.size() > 1) {
		// merge the first two sets
		std::set<SpanInterval> first = intersections.front();
		intersections.pop_front();
		std::set<SpanInterval> second = intersections.front();
		intersections.pop_front();
		std::set<SpanInterval> intersected;
		for (std::set<SpanInterval>::const_iterator lIt = first.begin(); lIt != first.end(); lIt++) {
			for (std::set<SpanInterval>::const_iterator sIt = second.begin(); sIt != second.end(); sIt++) {
				SpanInterval intersection = lIt->intersection(*sIt);
				if (!intersection.isEmpty()) {
					intersected.insert(intersection);
				}
			}
		}
		if (!intersected.empty()) {
			intersections.push_front(intersected);
		}
	}
	if (intersections.empty()) {
		return SISet();
	}
	return SISet(intersections.front().begin(), intersections.front().end(), forceLiquid_);
}


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

void SISet::add(const SpanInterval &s) {
	if (forceLiquid_ && !s.isLiquid()) {
		std::runtime_error e("tried to add a non-liquid SI to a liquid SI");
		throw e;
	}
	set_.insert(s);
}

void SISet::makeDisjoint() {
	if (forceLiquid_) {
		// all our intervals are liquid so we can make them disjoint by
		// merging them
		for (std::set<SpanInterval>::const_iterator fIt = set_.begin(); fIt != set_.end(); fIt++) {
			std::set<SpanInterval>::const_iterator sIt = fIt;
			sIt++;
			if (sIt != set_.end() && !fIt->intersection(*sIt).isEmpty()) {
				// merge the two
				SpanInterval merged(fIt->start().start(), sIt->end().end());
				set_.erase(sIt);	// invalidates sIt
				set_.erase(fIt);	// invalidates fIt
				fIt = set_.insert(merged).first;
			}
		}
	} else {
		// scan over all pairs, looking for intersections
		for (std::set<SpanInterval>::const_iterator fIt = set_.begin(); fIt != set_.end(); fIt++) {
			for (std::set<SpanInterval>::const_iterator sIt = fIt; sIt != set_.end(); sIt++) {
				// don't compare to yourself
				if (sIt == fIt) {
					continue;
				}
				SpanInterval intersect = fIt->intersection(*sIt);
				if (!intersect.isEmpty()) {

					// remove it from the second set
					std::set<SpanInterval> leftover;
					sIt->subtract(intersect, leftover);

					std::set<SpanInterval>::const_iterator toRemove = sIt;
					sIt--;
					set_.erase(toRemove);
					set_.insert(leftover.begin(), leftover.end());
				}
			}
		}
	}
	// don't trust myself - remove this later as an optimization step
	if (!isDisjoint()) {
		std::runtime_error error("inside SISet::makeDisjoint() - set was attempted to make disjoint but isn't!");
		throw error;
	}
}

std::string SISet::toString() const {
	std::stringstream sstream;
	sstream << "{";
	for (std::set<SpanInterval>::const_iterator it = set_.begin(); it != set_.end(); it++) {
		if (it != set_.begin()) {
			sstream << ", ";
		}
		sstream << it->toString();
	}
	sstream << "}";
	return sstream.str();
}
