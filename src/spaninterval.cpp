/*
 * spaninterval.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#include <algorithm>
#include <exception>
#include "bad_normalize.h"
#include "interval.h"
#include "spaninterval.h"

SpanInterval::SpanInterval(const Interval& start, const Interval& end)
	: st(start), en(end)
{
}

SpanInterval::SpanInterval(unsigned int start, unsigned int end) 
  : st(start, end), en(start, end)
{
}

SpanInterval::SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo) 
  : st(startFrom, startTo), en(endFrom, endTo) 
{
}

bool SpanInterval::operator==(const SpanInterval& b) const {
  return (start() == b.start() && end() == b.end());
}

bool SpanInterval::operator!=(const SpanInterval& b) const {
  return !(*this == b);
}

bool SpanInterval::operator>(const SpanInterval& b) const {
  if (*this == b) return false;
  return !(*this < b);
}

bool SpanInterval::operator<(const SpanInterval& b) const {
  if (*this == b) return false;
  if (start().start() < b.start().start()) return true;
  if (start().start() > b.start().start()) return false;
  if (start().end() < b.start().end()) return true;
  if (start().end() > b.start().end()) return false;
  if (end().start() < b.end().start()) return true;
  if (end().start() > b.end().start()) return false;
  if (end().end() < b.end().end()) return true;
  if (end().end() > b.end().end()) return false;
  // return false as failure
  return false;
}

bool SpanInterval::operator>=(const SpanInterval& b) const {
  return !(*this < b);
}

bool SpanInterval::operator<=(const SpanInterval& b) const {
  return !(*this > b);
}

bool SpanInterval::isEmpty() const {
  unsigned int j = std::min(st.end(), en.end());
  unsigned int k = std::max(en.start(), st.start());

  if (st.start() > j
      || st.end() < k)
    return true;
  return false;
}


SpanInterval SpanInterval::normalize() const throw (bad_normalize) {
	if (isEmpty()) {
		bad_normalize bad;		// TODO: why two lines?  can't collapse into one?
		throw bad;
	}
	int j = std::min(st.end(), en.end());
	int k = std::max(en.start(), st.start());

	return SpanInterval(st.start(), j, k, en.end());
}

void SpanInterval::normalize(std::set<SpanInterval>& collect) const {
  try {
    collect.insert(normalize());
  } catch (bad_normalize& e) {
    // do nothing
  }
}

SpanInterval SpanInterval::intersection(const SpanInterval& other) const {
  return SpanInterval(std::max(start().start(), other.start().start()),
                      std::min(start().end(), other.start().end()),
                      std::max(end().start(), other.end().start()),
                      std::min(end().end(), other.end().end()));
}

void SpanInterval::compliment(std::set<SpanInterval>& collect) const {
  SpanInterval a(NEG_INF, POS_INF, NEG_INF, end().start()-1);
  a.normalize(collect);
  SpanInterval b(NEG_INF, POS_INF, end().end()+1, POS_INF);
  b.normalize(collect);
  SpanInterval c(NEG_INF, start().start()-1, NEG_INF, POS_INF);
  c.normalize(collect);
  SpanInterval d(start().end()+1, POS_INF, NEG_INF, POS_INF);
  d.normalize(collect);
}
