/*
 * spaninterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H

class SpanInterval {
public:
	SpanInterval();
	SpanInterval(const Interval& start, const Interval& end);
	SpanInterval(unsigned int start, unsigned int end);
	SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo);

	Interval const& start() const {return start;};
	Interval const& end() const {return end;};
	void setStart(const Interval& start) {st = start;};
	void setEnd(const Interval& end) {en = end;};

	SpanInterval normalize() const;
private:
	Interval st, en;

};

#endif /* SPANINTERVAL_H */
