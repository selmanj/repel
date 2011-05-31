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

/*
SpanInterval::SpanInterval(unsigned int smallest, unsigned int largest)
	: maxInterval_.start()(smallest), maxInterval_.end()(largest) {
}
*/

SpanInterval::SpanInterval(const Interval& start, const Interval& end, const Interval& maxInterval)
: start_(start), end_(end), maxInterval_(maxInterval)
{
}

SpanInterval::SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo,
		const Interval& maxInterval)
: start_(startFrom, startTo), end_(endFrom, endTo), maxInterval_(maxInterval)
{
}

void SpanInterval::setMaxInterval(const Interval& maxInterval) {
	maxInterval_ = maxInterval;
	// i, j, k, l all must be within max interval
	unsigned int i = std::min(std::max(start().start(), maxInterval_.start()), maxInterval_.end());
	unsigned int j = std::min(std::max(start().end(), maxInterval_.start()), maxInterval_.end());
	unsigned int k = std::min(std::max(end().start(), maxInterval_.start()), maxInterval_.end());
	unsigned int l = std::min(std::max(end().end(), maxInterval_.start()), maxInterval_.end());

	setStart(Interval(i,j));
	setEnd(Interval(k,l));

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

SpanInterval SpanInterval::toLiquid() const {
	unsigned int i = std::max(start().start(), end().start());
	unsigned int j = std::min(start().end(), end().end());
	return SpanInterval(i, j, i, j, maxInterval_);
}


SpanInterval SpanInterval::normalize() const throw (bad_normalize) {
	if (isEmpty()) {
		bad_normalize bad;		// TODO: why two lines?  can't collapse into one?
		throw bad;
	}
	int j = std::min(start_.end(), end_.end());
	int k = std::max(end_.start(), start_.start());

	return SpanInterval(start_.start(), j, k, end_.end(), maxInterval_);
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
			std::min(end().end(), other.end().end()), maxInterval_);
}

void SpanInterval::compliment(std::set<SpanInterval>& collect) const {
	/*
	SpanInterval a(maxInterval_.start(), maxInterval_.end(), maxInterval_.start(), end().start()-1);
	a.normalize(collect);
	SpanInterval b(maxInterval_.start(), maxInterval_.end(), end().end()+1, maxInterval_.end());
	b.normalize(collect);
	SpanInterval c(maxInterval_.start(), start().start()-1, maxInterval_.start(), maxInterval_.end());
	c.normalize(collect);
	SpanInterval d(start().end()+1, maxInterval_.end(), maxInterval_.start(), maxInterval_.end());
	d.normalize(collect);
	*/
	// I think the following ends up the same as the previous, just disjoint
	if (start().start() != maxInterval_.start()) {
		SpanInterval a(maxInterval_.start(), start().start()-1, maxInterval_.start(), maxInterval_.end(), maxInterval_);
		a.normalize(collect);
	}
	if (end().start() != maxInterval_.start()) {
		SpanInterval b(start().start(), start().end(), maxInterval_.start(), end().start()-1, maxInterval_);
		b.normalize(collect);
	}
	if (end().end() != maxInterval_.end()) {
		SpanInterval c(start().start(), start().end(), end().end()+1, maxInterval_.end(), maxInterval_);
		c.normalize(collect);
	}
	if (start().end() != maxInterval_.end()) {
		SpanInterval d(start().end()+1, maxInterval_.end(), maxInterval_.start(), maxInterval_.end(), maxInterval_);
		d.normalize(collect);
	}
}

void SpanInterval::liqCompliment(std::set<SpanInterval>& collect) const {
	// at most two intervals
	if (start().start() != maxInterval_.start()) {
		unsigned int end = start().start()-1;
		SpanInterval a(maxInterval_.start(), end, maxInterval_.start(), end, maxInterval_);
		collect.insert(a);
	}
	if (end().end() != maxInterval_.end()) {
		unsigned int start = end().end()+1;
		SpanInterval b(start, maxInterval_.end(), start, maxInterval_.end(), maxInterval_);
		collect.insert(b);
	}
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

