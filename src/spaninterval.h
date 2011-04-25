/*
 * spaninterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H

#include "interval.h"
#include "bad_normalize.h"

#include <climits>
#include <set>

class SpanInterval {
public:
	SpanInterval();
	SpanInterval(const Interval& start, const Interval& end);
	SpanInterval(unsigned int start, unsigned int end);
	SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo);

	Interval const& start() const {return start_;};
	Interval const& end() const {return end_;};
	void setStart(const Interval& start) {start_ = start;};
	void setEnd(const Interval& end) {end_ = end;};

	bool operator==(const SpanInterval& b) const;
	bool operator!=(const SpanInterval& b) const;
	bool operator>(const SpanInterval& b) const;
	bool operator<(const SpanInterval& b) const;
	bool operator>=(const SpanInterval& b) const;
	bool operator<=(const SpanInterval& b) const;

	bool isEmpty() const;
	SpanInterval normalize() const throw(bad_normalize);
	void normalize(std::set<SpanInterval>& collect) const;
	SpanInterval intersection(const SpanInterval&  other) const;
	void compliment(std::set<SpanInterval>& collect) const;

	static const unsigned int NEG_INF = 0;
	static const unsigned int POS_INF = UINT_MAX;
private:
	Interval start_, end_;

};

#endif /* SPANINTERVAL_H */
