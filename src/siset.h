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
	SISet(bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX))
	: forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}
//	SISet(const SpanInterval& si, bool forceLiquid=false,
//			const Interval& maxInterval=Interval(0, UINT_MAX));
	template <class InputIterator>
	SISet(InputIterator begin, InputIterator end,
			bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX))
	: set_(begin, end), forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}

	const std::set<SpanInterval>& set() const {return set_;};
	bool forceLiquid() const {return forceLiquid_;};
	// TODO make some of these friend functions
	bool isDisjoint() const;
	SISet compliment() const;
	Interval maxInterval() const;
	unsigned int size() const;

	// modifiers
	void add(const SpanInterval &s);
	void add(const SISet& b);

	void makeDisjoint();
	void clear() {set_.clear();};
	void setMaxInterval(const Interval& maxInterval);
	void setForceLiquid(bool forceLiquid);
	std::string toString() const;

	friend SISet intersection(const SISet& a, const SISet& b);
	friend SISet span(const SpanInterval& a, const SpanInterval& b);
private:
	bool forceLiquid_;
	std::set<SpanInterval> set_;
	Interval maxInterval_;
};

SISet intersection(const SISet& a, const SISet& b);
SISet span(const SpanInterval& a, const SpanInterval& b);
SISet composedOf(const SpanInterval& a, const SpanInterval& b, Interval::INTERVAL_RELATION);


#endif /* SISET_H_ */
