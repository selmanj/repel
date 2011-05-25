/*
 * siset.h
 *
 *  Created on: May 24, 2011
 *      Author: joe
 */

#ifndef SISET_H_
#define SISET_H_
#include <set>
#include "spaninterval.h"
class SISet {
public:

	const std::set<SpanInterval>& set() const {return set_;};
	bool isDisjoint() const;

	void add(const SpanInterval &s) { set_.insert(s);};
	void makeDisjoint();


private:
	std::set<SpanInterval> set_;
};

#endif /* SISET_H_ */
