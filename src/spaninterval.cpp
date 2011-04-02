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


