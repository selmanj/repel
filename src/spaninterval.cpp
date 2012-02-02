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
#include <iostream>
#include <list>
#include <iterator>
#include "interval.h"
#include "spaninterval.h"
#include "log.h"

/*
SpanInterval::SpanInterval(unsigned int smallest, unsigned int largest)
    : maxInterval_.start()(smallest), maxInterval_.end()(largest) {
}
*/

SpanInterval::SpanInterval(const Interval& start, const Interval& end, const Interval& maxInterval)
: start_(start), finish_(end), maxInterval_(maxInterval)
{
}

SpanInterval::SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo,
        const Interval& maxInterval)
: start_(startFrom, startTo), finish_(endFrom, endTo), maxInterval_(maxInterval)
{
}

SpanInterval::const_iterator SpanInterval::begin() const {
    return SpanIntervalIterator(*this);
}
SpanInterval::const_iterator SpanInterval::end() const {
    return SpanIntervalIterator();
}

boost::optional<SpanInterval> SpanInterval::setMaxInterval(const Interval& maxInterval) const {
    if (isEmpty()) {
        return boost::optional<SpanInterval>();
    }
    // ensure we are working with normalized version
    SpanInterval copy(*this);
    copy = copy.normalize().get();
    // ensure that this interval can still exist
    if (copy.start().finish() < maxInterval.start() || copy.finish().start() > maxInterval.finish()) {
        return boost::optional<SpanInterval>();
    }
    // i, j, k, l all must be within max interval
    unsigned int i = std::min(std::max(copy.start().start(), maxInterval_.start()), maxInterval_.finish());
    unsigned int j = std::min(std::max(copy.start().finish(), maxInterval_.start()), maxInterval_.finish());
    unsigned int k = std::min(std::max(copy.finish().start(), maxInterval_.start()), maxInterval_.finish());
    unsigned int l = std::min(std::max(copy.finish().finish(), maxInterval_.start()), maxInterval_.finish());

    copy = SpanInterval(i, j, k, l, maxInterval);
    return copy.normalize();
}

bool SpanInterval::operator==(const SpanInterval& b) const {
    return (start() == b.start() && finish() == b.finish());
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
    if (start().finish() < b.start().finish()) return true;
    if (start().finish() > b.start().finish()) return false;
    if (finish().start() < b.finish().start()) return true;
    if (finish().start() > b.finish().start()) return false;
    if (finish().finish() < b.finish().finish()) return true;
    if (finish().finish() > b.finish().finish()) return false;
    // return false as failure
    return false;
}

bool SpanInterval::operator>=(const SpanInterval& b) const {
    return !(*this < b);
}

bool SpanInterval::operator<=(const SpanInterval& b) const {
    return !(*this > b);
}


unsigned int SpanInterval::liqSize() const {
    if (isEmpty()) return 0;
    SpanInterval si = normalize().get();

    if (!si.isLiquid())
        LOG_PRINT(LOG_WARN) << "calling liqSize() on a non-liquid interval; this is probably not something you want to do" << std::endl;

    return si.start().size();
}

bool SpanInterval::isLiquid() const {
    return (start().start() == finish().start() && start().finish() == finish().finish());
}

SpanInterval SpanInterval::toLiquid() const {
    unsigned int i = std::max(start().start(), finish().start());
    unsigned int j = std::min(start().finish(), finish().finish());
    return SpanInterval(i, j, i, j, maxInterval_);
}


boost::optional<SpanInterval> SpanInterval::normalize() const {
    if (isEmpty()) {
        return boost::optional<SpanInterval>();
    }
    int j = std::min(start_.finish(), finish_.finish());
    int k = std::max(finish_.start(), start_.start());

    return boost::optional<SpanInterval>(SpanInterval(start_.start(), j, k, finish_.finish(), maxInterval_));
}


boost::optional<SpanInterval> SpanInterval::satisfiesRelation(Interval::INTERVAL_RELATION relation) const {
    unsigned int neg_inf = maxInterval_.start();
    unsigned int pos_inf = maxInterval_.finish();

    // to make it easy on ourselves, normalize
    if (!normalize()) { return boost::optional<SpanInterval>(); }
    SpanInterval n = normalize().get();

    // easy names - [[i,j], [k,l]]
    const unsigned int i = n.start().start();
    const unsigned int j = n.start().finish();
    const unsigned int k = n.finish().start();
    const unsigned int l = n.finish().finish();

    switch (relation) {
        case Interval::EQUALS:
            return n;
        case Interval::LESSTHAN:
            if (k == pos_inf || k == pos_inf-1) return boost::optional<SpanInterval>();
            return SpanInterval(k+2, pos_inf, neg_inf, pos_inf, maxInterval_).normalize();
        case Interval::GREATERTHAN:
            if (j == neg_inf || j == neg_inf+1) return boost::optional<SpanInterval>();
            return SpanInterval(neg_inf, pos_inf, neg_inf, j-2, maxInterval_).normalize();
        case Interval::MEETS:
            if (k == pos_inf)
                return boost::optional<SpanInterval>();
            if (l == pos_inf)
                return SpanInterval(k+1, pos_inf, neg_inf, pos_inf, maxInterval_).normalize();
            return SpanInterval(k+1, l+1, neg_inf, pos_inf, maxInterval_).normalize();
        case Interval::MEETSI:
            if (j == neg_inf)
                return boost::optional<SpanInterval>();
            if (i == neg_inf)
                return SpanInterval(neg_inf, pos_inf, neg_inf, j-1, maxInterval_).normalize();
            return SpanInterval(neg_inf, pos_inf, i-1, j-1, maxInterval_).normalize();
        case Interval::OVERLAPS:
            if (k == pos_inf || i == pos_inf) return boost::optional<SpanInterval>();
            if (i == k) {
                // special case here
                if (k >= pos_inf-1 ) return boost::optional<SpanInterval>();
                return SpanInterval(i+1, l, k+2, pos_inf, maxInterval_).normalize();
            }
            return SpanInterval(i+1, l, k+1, pos_inf, maxInterval_).normalize();
        case Interval::OVERLAPSI:
            if (j == neg_inf || l == neg_inf) return boost::optional<SpanInterval>();
            if (j == l) {
                // special case here
                if (j <= neg_inf+1) return boost::optional<SpanInterval>();
                return SpanInterval(neg_inf, j-2, i, l-1, maxInterval_).normalize();
            }
            return SpanInterval(neg_inf, j-1, i, l-1, maxInterval_).normalize();
        case Interval::STARTS:
            if (k == pos_inf) return boost::optional<SpanInterval>();
            return SpanInterval(i, j, k+1, pos_inf, maxInterval_).normalize();
        case Interval::STARTSI:
            if (l == neg_inf) return boost::optional<SpanInterval>();
            return SpanInterval(i, j, neg_inf, l-1, maxInterval_).normalize();
        case Interval::FINISHES:
            if (j == neg_inf) return boost::optional<SpanInterval>();
            return SpanInterval(neg_inf, j-1, k, l, maxInterval_).normalize();
        case Interval::FINISHESI:
            if (i == pos_inf) return boost::optional<SpanInterval>();
            return SpanInterval(i+1, pos_inf, k, l, maxInterval_).normalize();
        case Interval::DURING:                                              // TODO: BUG!  try during on [1:3]
            if (j == neg_inf || k == pos_inf)
                return boost::optional<SpanInterval>();
            return SpanInterval(neg_inf, j-1, k+1, pos_inf, maxInterval_).normalize();
        case Interval::DURINGI:
            if (i == pos_inf || l == neg_inf)
                return boost::optional<SpanInterval>();
            return SpanInterval(i+1, pos_inf, neg_inf, l-1, maxInterval_).normalize();
        default:
            std::runtime_error e("SpanInterval::siSatisfying() not implemented for relation!");
            throw e;
    }

    //return result;
}

std::string SpanInterval::toString() const {
    std::stringstream str;
    str << "[";
    if (isLiquid()) {
        str << start().start() << ":" << start().finish() << "]";
    } else {
        str << "(" << start().start() << ", " << start().finish() << "), (" << finish().start() << ", " << finish().finish() << ")]";
    }
    return str.str();
}

SpanInterval intersection(const SpanInterval& a, const SpanInterval& b) {
    return SpanInterval(std::max(a.start().start(), b.start().start()),
            std::min(a.start().finish(), b.start().finish()),
            std::max(a.finish().start(), b.finish().start()),
            std::min(a.finish().finish(), b.finish().finish()), a.maxInterval_);    // TODO: more sensible way to pick max interval
}

