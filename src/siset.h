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
	SISet() {}
	template <class InputIterator>
	SISet(InputIterator begin, InputIterator end) : set_(begin, end) {}
	const std::set<SpanInterval>& set() const {return set_;};
	bool isDisjoint() const;
	SISet compliment() const;

	// modifiers
	void add(const SpanInterval &s) { set_.insert(s);};
	void makeDisjoint();
	void clear() {set_.clear();};

	std::string toString() const;

private:
	std::set<SpanInterval> set_;
};

#endif /* SISET_H_ */
