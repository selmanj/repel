/*
 * spaninterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H

#include "interval.h"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <climits>
#include <set>
#include <vector>

class SpanInterval {
public:
	//SpanInterval(unsigned int smallest=0, unsigned int largest=UINT_MAX);
	SpanInterval(const Interval& start, const Interval& end,
			const Interval& maxInterval=Interval(0, UINT_MAX));
	SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo,
			const Interval& maxInterval=Interval(0, UINT_MAX));

	Interval const& start() const {return start_;};
	Interval const& end() const {return end_;};
	Interval const& maxInterval() const {return maxInterval_;};
	void setStart(const Interval& start) {start_ = start;};
	void setEnd(const Interval& end) {end_ = end;};
	void setMaxInterval(const Interval& maxInterval);

	bool operator==(const SpanInterval& b) const;
	bool operator!=(const SpanInterval& b) const;
	bool operator>(const SpanInterval& b) const;
	bool operator<(const SpanInterval& b) const;
	bool operator>=(const SpanInterval& b) const;
	bool operator<=(const SpanInterval& b) const;

	bool isEmpty() const;
	unsigned int size() const;
	bool isLiquid() const;
	SpanInterval toLiquid() const;
	boost::optional<SpanInterval> normalize() const;
	void compliment(std::set<SpanInterval>& collect) const;
	void liqCompliment(std::set<SpanInterval>& collect) const;
	boost::optional<SpanInterval> satisfiesRelation(Interval::INTERVAL_RELATION relation) const;

	void subtract(const SpanInterval& remove, std::set<SpanInterval>& collect) const;

	std::string toString() const;

	friend SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);

private:
	Interval start_, end_;
	Interval maxInterval_;

};

SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);

#endif /* SPANINTERVAL_H */
