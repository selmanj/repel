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
	//SpanInterval(unsigned int smallest=0, unsigned int largest=UINT_MAX);
	SpanInterval(const Interval& start, const Interval& end,
			const Interval& maxInterval=Interval(0, UINT_MAX));
	SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo,
			const Interval& maxInterval=Interval(0, UINT_MAX));

	Interval const& start() const {return start_;};
	Interval const& end() const {return end_;};
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
	bool isLiquid() const;
	SpanInterval normalize() const throw(bad_normalize);
	void normalize(std::set<SpanInterval>& collect) const;
	SpanInterval intersection(const SpanInterval&  other) const;
	void compliment(std::set<SpanInterval>& collect) const;
	void liqCompliment(std::set<SpanInterval>& collect) const;
	void subtract(const SpanInterval& remove, std::set<SpanInterval>& collect) const;

	std::string toString() const;

private:
	Interval start_, end_;
	Interval maxInterval_;

};


#endif /* SPANINTERVAL_H */
