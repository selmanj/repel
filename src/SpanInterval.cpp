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

boost::optional<SpanInterval> SpanInterval::satisfiesRelation(Interval::INTERVAL_RELATION relation, const SpanInterval& universe) const {
    if (!universe.isLiquid()) throw std::invalid_argument("SpanInterval::satisfiesRelation() - universe variable is not liquid!  technically this can be supported, but currently is not.");
    unsigned int neg_inf = universe.start().start();
    unsigned int pos_inf = universe.start().finish();

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
            return SpanInterval(k+2, pos_inf, neg_inf, pos_inf).normalize();
        case Interval::GREATERTHAN:
            if (j == neg_inf || j == neg_inf+1) return boost::optional<SpanInterval>();
            return SpanInterval(neg_inf, pos_inf, neg_inf, j-2).normalize();
        case Interval::MEETS:
            if (k == pos_inf)
                return boost::optional<SpanInterval>();
            if (l == pos_inf)
                return SpanInterval(k+1, pos_inf, neg_inf, pos_inf).normalize();
            return SpanInterval(k+1, l+1, neg_inf, pos_inf).normalize();
        case Interval::MEETSI:
            if (j == neg_inf)
                return boost::optional<SpanInterval>();
            if (i == neg_inf)
                return SpanInterval(neg_inf, pos_inf, neg_inf, j-1).normalize();
            return SpanInterval(neg_inf, pos_inf, i-1, j-1).normalize();
        case Interval::OVERLAPS:
            if (k == pos_inf || i == pos_inf) return boost::optional<SpanInterval>();
            if (i == k) {
                // special case here
                if (k >= pos_inf-1 ) return boost::optional<SpanInterval>();
                return SpanInterval(i+1, l, k+2, pos_inf).normalize();
            }
            return SpanInterval(i+1, l, k+1, pos_inf).normalize();
        case Interval::OVERLAPSI:
            if (j == neg_inf || l == neg_inf) return boost::optional<SpanInterval>();
            if (j == l) {
                // special case here
                if (j <= neg_inf+1) return boost::optional<SpanInterval>();
                return SpanInterval(neg_inf, j-2, i, l-1).normalize();
            }
            return SpanInterval(neg_inf, j-1, i, l-1).normalize();
        case Interval::STARTS:
            if (k == pos_inf) return boost::optional<SpanInterval>();
            return SpanInterval(i, j, k+1, pos_inf).normalize();
        case Interval::STARTSI:
            if (l == neg_inf) return boost::optional<SpanInterval>();
            return SpanInterval(i, j, neg_inf, l-1).normalize();
        case Interval::FINISHES:
            if (j == neg_inf) return boost::optional<SpanInterval>();
            return SpanInterval(neg_inf, j-1, k, l).normalize();
        case Interval::FINISHESI:
            if (i == pos_inf) return boost::optional<SpanInterval>();
            return SpanInterval(i+1, pos_inf, k, l).normalize();
        case Interval::DURING:                                              // TODO: BUG!  try during on [1:3]
            if (j == neg_inf || k == pos_inf)
                return boost::optional<SpanInterval>();
            return SpanInterval(neg_inf, j-1, k+1, pos_inf).normalize();
        case Interval::DURINGI:
            if (i == pos_inf || l == neg_inf)
                return boost::optional<SpanInterval>();
            return SpanInterval(i+1, pos_inf, neg_inf, l-1).normalize();
        default:
            std::runtime_error e("SpanInterval::siSatisfying() not implemented for relation!");
            throw e;
    }

    //return result;
}


