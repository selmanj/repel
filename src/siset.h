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

class SIFactory;

class SISet {
public:
	SISet(bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX))
	: forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}
	SISet(const SpanInterval& si, bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX));
	template <class InputIterator>
	SISet(InputIterator begin, InputIterator end,
			bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX))
	: set_(begin, end), forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}

	const std::set<SpanInterval>& set() const {return set_;};
	bool forceLiquid() const {return forceLiquid_;};

	bool isDisjoint() const;
	SISet compliment() const;
	Interval maxInterval() const;

	// modifiers
	void add(const SpanInterval &s);
	void makeDisjoint();
	void clear() {set_.clear();};
	void setMaxInterval(const Interval& maxInterval);
	void setForceLiquid(bool forceLiquid) {forceLiquid_ = forceLiquid;};
	std::string toString() const;

private:
	bool forceLiquid_;
	std::set<SpanInterval> set_;
	Interval maxInterval_;
};


#endif /* SISET_H_ */
