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

class SpanInterval {
public:
	SpanInterval();
	SpanInterval(const Interval& start, const Interval& end);
	SpanInterval(unsigned int start, unsigned int end);
	SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo);

	Interval const& start() const {return st;};
	Interval const& end() const {return en;};
	void setStart(const Interval& start) {st = start;};
	void setEnd(const Interval& end) {en = end;};

  bool operator==(const SpanInterval& b) const;
  bool operator!=(const SpanInterval& b) const;
  bool operator>(const SpanInterval& b) const;
  bool operator<(const SpanInterval& b) const;
  bool operator>=(const SpanInterval& b) const;
  bool operator<=(const SpanInterval& b) const;

	bool isEmpty() const;
	SpanInterval normalize() const throw(bad_normalize);
  SpanInterval intersection(const SpanInterval&  other) const;
//  void compliment(std::set<SpanInterval>& collect) const;

  static const unsigned int NEG_INF = 0;
  static const unsigned int POS_INF = UINT_MAX;
private:
	Interval st, en;

};

#endif /* SPANINTERVAL_H */
