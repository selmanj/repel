/*
 * spaninterval.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#include <exception>
#include "spaninterval.h"

SpanInterval::SpanInterval(const Interval& start, const Interval& end)
	: st(start), en(end)
{
}

SpanInterval::SpanInterval(unsigned int start, unsigned int end) {
	st = Interval(start, end);
	en = st;
}

SpanInterval::SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo) {
	st = Interval(startFrom, startTo);
	en = Interval(endFrom, endTo);
}

SpanInterval SpanInterval::normalize() throw (std::exception) {
	//if (st.)

}
