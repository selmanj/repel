/*
 * spaninterval.cpp
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>
#include <boost/optional.hpp>
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

boost::optional<SpanInterval> SpanInterval::setMaxInterval(const Interval& maxInterval) const {
	if (isEmpty()) {
		return boost::optional<SpanInterval>();
	}
	// ensure we are working with normalized version
	SpanInterval copy(*this);
	copy = copy.normalize().get();
	// ensure that this interval can still exist
	if (copy.start().end() < maxInterval.start() || copy.end().start() > maxInterval.end()) {
		return boost::optional<SpanInterval>();
	}
	// i, j, k, l all must be within max interval
	unsigned int i = std::min(std::max(copy.start().start(), maxInterval_.start()), maxInterval_.end());
	unsigned int j = std::min(std::max(copy.start().end(), maxInterval_.start()), maxInterval_.end());
	unsigned int k = std::min(std::max(copy.end().start(), maxInterval_.start()), maxInterval_.end());
	unsigned int l = std::min(std::max(copy.end().end(), maxInterval_.start()), maxInterval_.end());

	copy = SpanInterval(i, j, k, l, maxInterval);
	return copy.normalize();
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

unsigned int SpanInterval::size() const {
	if (isEmpty()) return 0;

	SpanInterval si = normalize().get();

	unsigned int i = si.start().start();
	unsigned int j = si.start().end();
	unsigned int k = si.end().start();
	unsigned int l = si.end().end();

	if (j <= k) {
		return ((l-k)+1) * ((j-i)+1);
	}
	// I'm so sorry about the below formula; TODO: rewrite this nicer
	return ((k-i)+1) * ((l-k)+1)
			  + (j-k)*(l+1) - (j*(j+1))/2 + (k*(k+1))/2;

}

bool SpanInterval::isLiquid() const {
	return (start().start() == end().start() && start().end() == end().end());
}

SpanInterval SpanInterval::toLiquid() const {
	unsigned int i = std::max(start().start(), end().start());
	unsigned int j = std::min(start().end(), end().end());
	return SpanInterval(i, j, i, j, maxInterval_);
}


boost::optional<SpanInterval> SpanInterval::normalize() const {
	if (isEmpty()) {
		return boost::optional<SpanInterval>();
	}
	int j = std::min(start_.end(), end_.end());
	int k = std::max(end_.start(), start_.start());

	return boost::optional<SpanInterval>(SpanInterval(start_.start(), j, k, end_.end(), maxInterval_));
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
		if (a.normalize()) collect.insert(a.normalize().get());
	}
	if (end().start() != maxInterval_.start()) {
		SpanInterval b(start().start(), start().end(), maxInterval_.start(), end().start()-1, maxInterval_);
		if (b.normalize()) collect.insert(b.normalize().get());
	}
	if (end().end() != maxInterval_.end()) {
		SpanInterval c(start().start(), start().end(), end().end()+1, maxInterval_.end(), maxInterval_);
		if (c.normalize()) collect.insert(c.normalize().get());
	}
	if (start().end() != maxInterval_.end()) {
		SpanInterval d(start().end()+1, maxInterval_.end(), maxInterval_.start(), maxInterval_.end(), maxInterval_);
		if (d.normalize()) collect.insert(d.normalize().get());
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

boost::optional<SpanInterval> SpanInterval::satisfiesRelation(Interval::INTERVAL_RELATION relation) const {
	unsigned int neg_inf = maxInterval_.start();
	unsigned int pos_inf = maxInterval_.end();

	switch (relation) {
		case Interval::EQUALS:
			return normalize();
		case Interval::LESSTHAN:
			if (pos_inf-2 < end().start()) return boost::optional<SpanInterval>();
			return SpanInterval(end().start()+2, pos_inf, neg_inf, pos_inf, maxInterval_).normalize();
		case Interval::GREATERTHAN:
			if (neg_inf+2 > start().end()) return boost::optional<SpanInterval>();
			return SpanInterval(neg_inf, pos_inf, neg_inf, start().end()-2, maxInterval_).normalize();
		case Interval::MEETS:
			if (pos_inf-1 < end().start() || pos_inf-1 < end().end())
				return boost::optional<SpanInterval>();
			return SpanInterval(end().start()+1, end().end()+1, neg_inf, pos_inf, maxInterval_).normalize();
		case Interval::MEETSI:
			if (neg_inf+1 > start().start() || neg_inf+1 > start().end())
				return boost::optional<SpanInterval>();
			return SpanInterval(neg_inf, pos_inf, start().start()-1, start().end()-1, maxInterval_).normalize();
		case Interval::OVERLAPS:
			if (pos_inf-1 < end().start() || pos_inf-1 < start().start()) return boost::optional<SpanInterval>();
			return SpanInterval(start().start()+1, end().end(), end().start()+1, pos_inf, maxInterval_).normalize();
		case Interval::OVERLAPSI:
			if (neg_inf+1 > start().end() || neg_inf+1 > end().end()) return boost::optional<SpanInterval>();
			return SpanInterval(neg_inf, start().end()-1, start().start(), end().end()-1, maxInterval_).normalize();
		case Interval::STARTS:
			if (pos_inf-1 < end().start()) return boost::optional<SpanInterval>();
			return SpanInterval(start().start(), start().end(), end().start()+1, pos_inf, maxInterval_).normalize();
		case Interval::STARTSI:
			if (neg_inf+1 > end().end()) return boost::optional<SpanInterval>();
			return SpanInterval(start().start(), start().end(), neg_inf, end().end()-1, maxInterval_).normalize();
		case Interval::FINISHES:
			if (neg_inf+1 > start().end()) return boost::optional<SpanInterval>();
			return SpanInterval(neg_inf, start().end()-1, end().start(), end().end(), maxInterval_).normalize();
		case Interval::FINISHESI:
			if (pos_inf-1 < start().start()) return boost::optional<SpanInterval>();
			return SpanInterval(start().start()+1, pos_inf, end().start(), end().end(), maxInterval_).normalize();
		case Interval::DURING:
			if (neg_inf+1 > start().end() || pos_inf-1 < end().start())
				return boost::optional<SpanInterval>();
			return SpanInterval(neg_inf, start().end()-1, end().start()+1, pos_inf, maxInterval_).normalize();
		case Interval::DURINGI:
			if (pos_inf-1 < start().start() || neg_inf+1 > end().end())
				return boost::optional<SpanInterval>();
			return SpanInterval(start().start()+1, pos_inf, neg_inf, end().end()-1, maxInterval_).normalize();
		default:
			std::runtime_error e("SpanInterval::siSatisfying() not implemented for relation!");
			throw e;
	}

	//return result;
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
		collect.insert(intersection(*this, *it));
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

SpanInterval intersection(const SpanInterval& a, const SpanInterval& b) {
	return SpanInterval(std::max(a.start().start(), b.start().start()),
			std::min(a.start().end(), b.start().end()),
			std::max(a.end().start(), b.end().start()),
			std::min(a.end().end(), b.end().end()), a.maxInterval_);	// TODO: more sensible way to pick max interval
}
