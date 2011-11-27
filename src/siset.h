/*
 * siset.h
 *
 *  Created on: May 24, 2011
 *      Author: joe
 */

#ifndef SISET_H_
#define SISET_H_
#include <set>
#include <list>
#include "spaninterval.h"

class SISet {
public:
	SISet(bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX))
	: set_(), forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}
//	SISet(const SpanInterval& si, bool forceLiquid=false,
//			const Interval& maxInterval=Interval(0, UINT_MAX));
	template <class InputIterator>
	SISet(InputIterator begin, InputIterator end,
			bool forceLiquid=false,
			const Interval& maxInterval=Interval(0, UINT_MAX))
	: set_(begin, end), forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}

	std::set<SpanInterval> asSet() const;
	bool forceLiquid() const {return forceLiquid_;};
	// TODO make some of these friend functions
	bool isDisjoint() const;
	SISet compliment() const;
	Interval maxInterval() const;
	unsigned int size() const;
	unsigned int liqSize() const;
	const std::list<SpanInterval>& intervals() const {return set_;}

	// modifiers
	void add(const SpanInterval &s);
	void add(const SISet& b);

	void makeDisjoint();
	void clear() {set_.clear();};
	void setMaxInterval(const Interval& maxInterval);
	void setForceLiquid(bool forceLiquid);
	void subtract(const SpanInterval& si);
	void subtract(const SISet& sis);

	static SISet randomSISet(bool forceLiquid, const Interval& maxInterval);
	SpanInterval randomSI() const;

	std::string toString() const;

	bool operator ==(const SISet& a) const {return set_ == a.set_;}
	bool operator !=(const SISet& a) const {return !(*this == a);}

	friend SISet intersection(const SISet& a, const SISet& b);
	friend SISet span(const SpanInterval& a, const SpanInterval& b);
	friend bool equalByInterval(const SISet& a, const SISet& b);
private:
	std::list<SpanInterval> set_;
	bool forceLiquid_;
	Interval maxInterval_;
};

SISet intersection(const SISet& a, const SpanInterval& si);
SISet intersection(const SISet& a, const SISet& b);
SISet span(const SpanInterval& a, const SpanInterval& b);
SISet composedOf(const SpanInterval& a, const SpanInterval& b, Interval::INTERVAL_RELATION);
bool equalByInterval(const SISet& a, const SISet& b);

unsigned long hammingDistance(const SISet& a, const SISet& b);


#endif /* SISET_H_ */
