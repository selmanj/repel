/*
 * siset.cpp
 *
 *  Created on: May 24, 2011
 *      Author: joe
 */
#include <set>
#include <list>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <boost/foreach.hpp>
#include <iterator>
#include "siset.h"
#include "spaninterval.h"
#include "log.h"
#include "infix_ostream_iterator.h"

std::set<SpanInterval> SISet::asSet() const {
    return std::set<SpanInterval>(set_.begin(), set_.end());
}


SISet SISet::compliment(const SISet& universe) const {
    SISet copy = universe;
    copy.subtract(*this);
    return copy;
}

unsigned int SISet::size() const {
    // look, this doesn't mean much if the list isn't disjoint
    SISet copy(*this);
    copy.makeDisjoint();

    unsigned int sum = 0;
    BOOST_FOREACH(SpanInterval sp, copy.set_) {
        sum += sp.size();
    }
    return sum;
}

unsigned int SISet::liqSize() const {
    SISet copy(*this);
    copy.makeDisjoint();

    unsigned int sum = 0;
    BOOST_FOREACH(SpanInterval sp, copy.set_) {
        sum += sp.liqSize();
    }
    return sum;
}

// O(n^2) for every call :(  perhaps set a flag instead?
bool SISet::isDisjoint() const {
    for (std::list<SpanInterval>::const_iterator fIt = set_.begin(); fIt != set_.end(); fIt++) {
        for (std::list<SpanInterval>::const_iterator sIt = fIt; sIt != set_.end(); sIt++) {
            // dont compare to yourself
            if (sIt == fIt) {
                continue;
            }
            if (intersection(*fIt, *sIt)) {
                return false;
            }
        }
    }
    return true;
}

void SISet::add(const SpanInterval &s) {
    SpanInterval sp = s;
    if (forceLiquid_ && !sp.isLiquid()) {
        std::runtime_error e("tried to add a non-liquid SI to a liquid SI");
        throw e;
    }
    if (forceLiquid_) {
        // we have to merge this with pre-existing intervals
        for (std::list<SpanInterval>::iterator it = set_.begin(); it != set_.end();) {
            SpanInterval b = *it;
            Interval sInt = sp.start();
            Interval bInt = b.start();
            if (meets(sInt, bInt) || meetsI(sInt, bInt)
                    || overlaps(sInt, bInt) || overlapsI(sInt, bInt)
                    || starts(sInt, bInt) || startsI(sInt, bInt)
                    || finishes(sInt, bInt) || finishesI(sInt, bInt)
                    || during(sInt, bInt) || duringI(sInt, bInt)) {
                // they intersect, pull it out
                std::list<SpanInterval>::iterator toErase(it);
                it++;
                set_.erase(toErase);

                // now reset scopy to be the newly resized span interval
                unsigned int i = (b.start().start() < sp.start().start()
                        ? b.start().start() : sp.start().start());
                unsigned int j = (b.start().finish() > sp.start().finish()
                        ? b.start().finish() : sp.start().finish());

                sp.setStart(Interval(i,j));
                sp.setFinish(Interval(i,j));
            } else {
                it++;
            }
        }
        // now insert our element into the list
        set_.push_back(sp);
    } else {
        set_.push_back(sp);
    }
}

void SISet::add(const SISet &b) {
    for (std::list<SpanInterval>::const_iterator it = b.set_.begin(); it != b.set_.end(); it++) {
        add(*it);
    }
}

void SISet::makeDisjoint() {
    if (forceLiquid_) {
        return;
    } else {
        std::list<SpanInterval> setCopy;

        BOOST_FOREACH(SpanInterval siToAdd, set_) {
            SISet sisetToAdd(false);
            sisetToAdd.add(siToAdd);
            // subtract whats already stored and only add if there is some left over
            BOOST_FOREACH(SpanInterval siAlreadyIn, setCopy) {
                sisetToAdd.subtract(siAlreadyIn);
            }
            setCopy.insert(setCopy.end(), sisetToAdd.set_.begin(), sisetToAdd.set_.end());
        }
        set_ = setCopy;
    }
}

void SISet::setForceLiquid(bool forceLiquid) {
    if (forceLiquid && !forceLiquid_) {
        // first, make it disjoint
        makeDisjoint();

        std::list<SpanInterval> newSet;
        BOOST_FOREACH(SpanInterval sp, set_) {
            sp = sp.toLiquidExc();
            if (!sp.isEmpty()) {
                sp.normalize();
                newSet.push_back(sp);
            }
        }
        set_.swap(newSet);
    }
    forceLiquid_ = forceLiquid;
};

void SISet::subtract(const SpanInterval& si) {
    std::list<SpanInterval> newSet;

    if (set_.size() == 0) return;
    if (si.size() == 0) return;

    BOOST_FOREACH(SpanInterval siInSet, set_) {
        if (intersection(siInSet, si)) {
            if (forceLiquid_) siInSet.liqSubtract(si, back_inserter(newSet));
            else siInSet.subtract(si, back_inserter(newSet));
        } else {
            newSet.push_back(siInSet);
        }
    }
    set_ = newSet;
}

void SISet::subtract(const SISet& sis) {
    // O(n^2).  Probably could speed this up by sorting the elements.
    std::list<SpanInterval> result(this->begin(), this->end());

    for (SISet::const_iterator it = sis.begin(); it != sis.end(); it++) {
        std::list<SpanInterval> nextResult;
        SpanInterval toSub = *it;
        if (forceLiquid_ && !sis.forceLiquid_) {    // convert each spaninterval into a liquid one (inclusive)
            boost::optional<SpanInterval> si = toSub.toLiquidInc().normalize();
            if (!si) continue;
            toSub = *si;
        }
        for (std::list<SpanInterval>::iterator it2 = result.begin(); it2 != result.end(); it2++ ) {
            it2->subtract(toSub, std::inserter(nextResult, nextResult.end()));
        }
        result = nextResult;
    }

    *this = SISet::fromRange(result.begin(), result.end(), forceLiquid_);       // TODO make this return a SISet

    /*

    //LOG_PRINT(LOG_DEBUG) << "called SISet::subtract with *this=" << this->toString() << " and sis=" << sis.toString();
    std::list<SISet> toIntersect;

    if (set_.size() == 0) return;
    if (sis.size() == 0) return;

    BOOST_FOREACH(SpanInterval b, sis.set_) {
        SISet copy(*this);
        //copy.setForceLiquid(false);
    //  LOG_PRINT(LOG_DEBUG) << "copy size:" << copy.set_.size();
        copy.subtract(b);
    //  LOG_PRINT(LOG_DEBUG) << "this \\ b (where b = "<< b.toString() <<") = " << copy.toString();
        toIntersect.push_back(copy);
    }
    if (toIntersect.size() == 0) {
        set_.clear();
        return;
    }
    // now collapse the intersect set
    while (toIntersect.size() > 1) {
        SISet a = toIntersect.front();
        toIntersect.pop_front();
        SISet b = toIntersect.front();
        toIntersect.pop_front();

        SISet intersected = intersection(a, b);
    //  LOG_PRINT(LOG_DEBUG) << "a=" << a.toString() <<", b=" << b.toString() << ", a intersect b = "<< intersected.toString();
        toIntersect.push_front(intersected);
    }
    set_ = toIntersect.front().set_;
    //LOG_PRINT(LOG_DEBUG) << "final value: " << this->toString();
    */
}

const SISet SISet::satisfiesRelation(const Interval::INTERVAL_RELATION& rel, const Interval& universe) const {
    SISet newSet(false);

    for (std::list<SpanInterval>::const_iterator it = set_.begin(); it != set_.end(); it++) {
        boost::optional<SpanInterval> siOpt = it->satisfiesRelation(rel, SpanInterval(universe));
        if (siOpt) newSet.add(siOpt.get());
    }

    return newSet;
}


SISet SISet::randomSISet(bool forceLiquid, const Interval& maxInterval) {
    SISet start(forceLiquid);
    if (!forceLiquid) {
        LOG_PRINT(LOG_ERROR) << "generating random SISets for non-liquid events is not yet implemented!";
        return start;
    }

    int bitsLeft = 15;  // TODO: we assume max is 32767, always true?
    int random = rand();
    unsigned int curStart = 0;
    unsigned int curEnd = 0;
    bool buildingSI = false;
    for (unsigned int i = maxInterval.start(); i <= maxInterval.finish(); i++) {
        if (random % 2) {
            if (buildingSI) {
                curEnd = i;
            } else {
                buildingSI = true;
                curStart = i;
                curEnd = i;
            }
        } else {
            if (buildingSI) {
                buildingSI = false;
                start.add(SpanInterval(curStart, curEnd, curStart, curEnd));
            }
        }
        bitsLeft--;
        if (bitsLeft == 0) {
            random = rand();
            bitsLeft = 15;
        } else {
            random = random >> 1;
        }
    }
    return start;
}

SpanInterval SISet::randomSI() const {
    if (size() == 0) {
        LOG_PRINT(LOG_ERROR) << "called randomSI() on an empty SISet.";
        std::runtime_error e("called randomSI() on an empty SISet.");
        throw e;
    }
    // choose a random number from 0 to size
    int index = rand() % set_.size();
    std::list<SpanInterval>::const_iterator it = set_.begin();
    while (index != 0) {
        it++;
        index--;
    }
    return *it;
}

std::string SISet::toString() const {
    std::stringstream sstream;
    sstream << *this;
    return sstream.str();
}

std::ostream& operator<<(std::ostream& o, const SISet& s) {
    o << "{";
    std::list<SpanInterval> copy(s.set_);
    copy.sort();

    infix_ostream_iterator<SpanInterval> oIt(o, ", ");
    std::copy(copy.begin(), copy.end(), oIt);
    o << "}";
    return o;
}


SISet intersection(const SISet& a, const SISet& b) {
    SISet result;
    if (a.forceLiquid() && b.forceLiquid()) {
        result.setForceLiquid(true);
    } else {
        result.setForceLiquid(false);
    }
    // pairwise intersection - ugh.  O(n^2) again
    BOOST_FOREACH(SpanInterval siA, a.set_) {
        BOOST_FOREACH(SpanInterval siB, b.set_) {
            boost::optional<SpanInterval> intersect = intersection(siA, siB);
            if (intersect) {
                result.add(*intersect);
            }
        }
    }
    return result;
};

SISet intersection(const SISet& a, const SpanInterval& si) {
    // cheat for now
    SISet copy(false);
    copy.add(si);
    return intersection(a, copy);
}

SISet span(const SpanInterval& a, const SpanInterval& b, const Interval& maxInterval) {
    unsigned int j = std::min(a.start().finish(), b.start().finish());
    unsigned int k = std::max(a.finish().start(), b.finish().start());

    SISet set(false);

    set.add(SpanInterval(a.start().start(), j, k, a.finish().finish()));
    set.add(SpanInterval(a.start().start(), j, k, b.finish().finish()));
    set.add(SpanInterval(b.start().start(), j, k, a.finish().finish()));
    set.add(SpanInterval(b.start().start(), j, k, b.finish().finish()));

    return set;
};

SISet composedOf(const SpanInterval& i, const SpanInterval& j, Interval::INTERVAL_RELATION rel, const SpanInterval& universe) {
    if (!universe.isLiquid()) throw std::invalid_argument("composedOf() requires universe to be liquid; could be supported in future");
    SISet empty(false);

    if (rel == Interval::EQUALS) {
        boost::optional<SpanInterval> intersect = intersection(i, j);
        SISet result(false);
        if (intersect) result.add(*intersect);
        return result;
    }

    boost::optional<SpanInterval> iPrimeOpt = j.satisfiesRelation(inverseRelation(rel), universe);
    if (!iPrimeOpt) return empty;
    SpanInterval iPrime = iPrimeOpt.get();
    boost::optional<SpanInterval> jPrimeOpt = i.satisfiesRelation(rel, universe);
    if (!jPrimeOpt) return empty;
    SpanInterval jPrime = jPrimeOpt.get();

    boost::optional<SpanInterval> iIntersectOpt = intersection(iPrime, i);
    boost::optional<SpanInterval> jIntersectOpt = intersection(jPrime, j);

    if (!iIntersectOpt || !jIntersectOpt) return empty;

    SpanInterval iIntersect = iIntersectOpt.get();
    SpanInterval jIntersect = jIntersectOpt.get();
//  std::cout << "iIntersect = " << iIntersect.toString() << std::endl;
//  std::cout << "jIntersect = " << jIntersect.toString() << std::endl;

    // TODO this seems right but we are now iterating over pairs of intervals!
/*
    SISet spanned;
    for (SpanInterval::iterator iIt = iIntersect.begin(); iIt != iIntersect.end(); iIt++) {
        for (SpanInterval::iterator jIt = jIntersect.begin(); jIt != jIntersect.end(); jIt++) {
            if (relationHolds(*iIt, rel, *jIt)) {

                Interval interval = span(*iIt, *jIt);
                spanned.add(SpanInterval(interval.start(), interval.start(),
                        interval.finish(), interval.finish(), i.maxInterval()));
            }
        }
    }
    */


    SISet spanned = span(iIntersect, jIntersect, universe.start());
//  std::cout << "returning spanned = " << spanned.toString() << std::endl;
    return spanned;
}

bool equalByInterval(const SISet& a, const SISet& b) {
    // this is expensive
    std::set<Interval> aIntervals;
    std::set<Interval> bIntervals;

    for (std::list<SpanInterval>::const_iterator it = a.set_.begin(); it != a.set_.end(); it++) {
        SpanInterval si = *it;
        aIntervals.insert(si.begin(), si.end());
    }
    for (std::list<SpanInterval>::const_iterator it = b.set_.begin(); it != b.set_.end(); it++) {
        SpanInterval si = *it;
        aIntervals.insert(si.begin(), si.end());
    }

    if (    std::includes(aIntervals.begin(), aIntervals.end(),
                bIntervals.begin(), bIntervals.end())
         &&
            std::includes(bIntervals.begin(), bIntervals.end(),
                aIntervals.begin(), aIntervals.end())
        ) return true;
    return false;
}

unsigned long hammingDistance(const SISet& a, const SISet& b, const SISet& universe) {
    // (a ^ !b) v (!a ^ b)
    SISet aComp = a.compliment(universe);
    SISet bComp = b.compliment(universe);

    SISet abC = intersection(a, bComp);
    SISet aCb = intersection(aComp, b);
    SISet result(abC);
    result.add(aCb);
    if (a.forceLiquid() && b.forceLiquid()) {
        return result.liqSize();
    }
    return result.size();

}
