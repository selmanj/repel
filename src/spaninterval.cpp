/*
 * spaninterval.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#include <algorithm>
#include <exception>
#include <string>
#include <sstream>
#include "bad_normalize.h"
#include "interval.h"
#include "spaninterval.h"

SpanInterval::SpanInterval(const Interval& start, const Interval& end)
: start_(start), end_(end)
{
}

SpanInterval::SpanInterval(unsigned int start, unsigned int end) 
: start_(start, end), end_(start, end)
{
}

SpanInterval::SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo) 
: start_(startFrom, startTo), end_(endFrom, endTo)
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
	unsigned int j = std::min(start_.end(), end_.end());
	unsigned int k = std::max(end_.start(), start_.start());

	if (start_.start() > j
			|| end_.end() < k)
		return true;
	return false;
}

bool SpanInterval::isLiquid() const {
	return (start().start() == end().start() && start().end() == end().end());
}


SpanInterval SpanInterval::normalize() const throw (bad_normalize) {
	if (isEmpty()) {
		bad_normalize bad;		// TODO: why two lines?  can't collapse into one?
		throw bad;
	}
	int j = std::min(start_.end(), end_.end());
	int k = std::max(end_.start(), start_.start());

	return SpanInterval(start_.start(), j, k, end_.end());
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
	/*
	SpanInterval a(NEG_INF, POS_INF, NEG_INF, end().start()-1);
	a.normalize(collect);
	SpanInterval b(NEG_INF, POS_INF, end().end()+1, POS_INF);
	b.normalize(collect);
	SpanInterval c(NEG_INF, start().start()-1, NEG_INF, POS_INF);
	c.normalize(collect);
	SpanInterval d(start().end()+1, POS_INF, NEG_INF, POS_INF);
	d.normalize(collect);
	*/
	// I think the following ends up the same as the previous, just disjoint
	SpanInterval a(NEG_INF, start().start()-1, NEG_INF, POS_INF);
	SpanInterval b(start().start(), start().end(), NEG_INF, end().start()-1);
	SpanInterval c(start().start(), start().end(), end().end()+1, POS_INF);
	SpanInterval d(start().end()+1, POS_INF, NEG_INF, POS_INF);
	a.normalize(collect);
	b.normalize(collect);
	c.normalize(collect);
	d.normalize(collect);
}

void SpanInterval::subtract(const SpanInterval &remove, std::set<SpanInterval>& collect) const {
	/*
	SpanInterval a(start().start(), remove.start().start()-1, end().start(), remove.end().start()-1);
	a.normalize(collect);
	SpanInterval b(start().start(), remove.start().start()-1, remove.end().end()+1, end().end());
	b.normalize(collect);
	SpanInterval c(remove.start().end()+1, start().end(), end().start(), remove.end().start()-1);
	c.normalize(collect);
	SpanInterval d(remove.start().end()+1, start().end(), remove.end().end()+1, end().end());
	d.normalize(collect);
	*/
	std::set<SpanInterval> compliment;
	remove.compliment(compliment);
	for (std::set<SpanInterval>::const_iterator it = compliment.begin(); it != compliment.end(); it++) {
		collect.insert(this->intersection(*it));
	}
}

std::string SpanInterval::toString() const {
	std::stringstream str;
	str << "[";
	if (isLiquid()) {
		str << start().start() << ":" << start().end() << "]";
	} else {
		str << "(" << start().start() << ", " << start().end() << "), (" << end().start() << ", " << end().end() << ")]";
	}
	return str.str();
}
