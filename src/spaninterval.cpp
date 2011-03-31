/*
 * spaninterval.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#include <algorithm>
#include <exception>
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
  int j = std::min(st.end(), en.end());
  int k = std::max(en.start(), st.start()); 

  if (st.start() > j
      || st.end() < k)
    return true;
  return false;
}

/*
SpanInterval SpanInterval::normalize() throw (std::exception) {
	//if (st.)

}

*/
