/*
 * SpanInterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/access.hpp>
#include <climits>
#include <set>
#include <vector>
#include <list>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include "Interval.h"
#include "Log.h"
// forward declaration for the iterator - see below
class SpanIntervalIterator;

/**
 * Class for compactly representing a set of Intervals.  A spanning interval
 * is defined by four integers (or rather, two sets of integer pairs).  One
 * provides the range of starting points and ending points for intervals
 * contained in the set.
 *
 * For example, Spanning interval [(1,5), (6,10)] is a set of intervals that
 * contains all intervals that have their starting point in the range 1-5
 * (inclusive) and their endpoint in the range 6-10 (inclusive).  A Spanning
 * interval is called liquid if the starting/ending range are the same.
 */
class SpanInterval {
public:
    /**
     * Construct a Spanning Interval.  By default, the spanning interval is
     * [(0,0), (0,0].
     */
    SpanInterval();

    /**
     * Construct a liquid Spanning Interval from the given interval.  The
     * Spanning interval is [(liq), (liq)].
     */
    explicit SpanInterval(const Interval& liq);

    SpanInterval(const Interval& start, const Interval& end);
    SpanInterval(unsigned int liqStart, unsigned int liqEnd);
    SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo);

    typedef SpanIntervalIterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    Interval const& start() const;
    Interval const& finish() const;
    void setStart(const Interval& start);
    void setFinish(const Interval& end);

    friend bool operator==(const SpanInterval& a, const SpanInterval& b);
    friend bool operator!=(const SpanInterval& a, const SpanInterval& b);
    friend bool operator>(const SpanInterval& a, const SpanInterval& b);
    friend bool operator<(const SpanInterval& a, const SpanInterval& b);
    friend bool operator>=(const SpanInterval& a, const SpanInterval& b);
    friend bool operator<=(const SpanInterval& a, const SpanInterval& b);
    friend std::size_t hash_value(const SpanInterval& si);

    bool isEmpty() const;
    unsigned int size() const;
    unsigned int liqSize() const;
    bool isLiquid() const;
    SpanInterval toLiquidInc() const;
    SpanInterval toLiquidExc() const;
    boost::optional<SpanInterval> normalize() const;


    template<class OutputIterator>
    void compliment(const SpanInterval& universe, OutputIterator out) const;
    template<class OutputIterator>
    void liqCompliment(const SpanInterval& universe, OutputIterator out) const;

    boost::optional<SpanInterval> satisfiesRelation(Interval::INTERVAL_RELATION relation, const SpanInterval& universe) const;

    template<class OutputIterator>
    void subtract(const SpanInterval& remove, OutputIterator out) const;
    template<class OutputIterator>
    void liqSubtract(const SpanInterval& remove, OutputIterator out) const;

    std::string toString() const;

    friend boost::optional<SpanInterval> intersection(const SpanInterval& a, const SpanInterval& b);
    friend std::ostream& operator<<(std::ostream& o, const SpanInterval& si);

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    Interval start_, finish_;
};

boost::optional<SpanInterval> intersection(const SpanInterval& a, const SpanInterval& b);

class SpanIntervalIterator : public std::iterator<std::forward_iterator_tag, Interval> {
public:
    SpanIntervalIterator() : sp_(0,0,0,0), curr_(0,0), isDead_(true) {};
    SpanIntervalIterator(const SpanInterval& sp) : sp_(0,0,0,0), curr_(0,0), isDead_(true) {
        if (!sp.isEmpty()) {
            sp_ = sp.normalize().get();
            curr_ = Interval(sp_.start().start(), sp_.finish().start());
            isDead_ = false;
        }
    }
    bool operator==(const SpanIntervalIterator& other) const {
        if (isDead_ && other.isDead_) return true;
        if (isDead_ || other.isDead_) return false;
        return (sp_==other.sp_ && curr_==curr_);
    }
    bool operator!=(const SpanIntervalIterator& other) const {
        return !(this->operator ==(other));
    }
    const Interval& operator*() const {
        return curr_;
    }
    const Interval* operator->() const {
        return &curr_;
    }
    SpanIntervalIterator& operator++() {
        if (isDead_) return *this;
        if (curr_.start() == sp_.start().finish() && curr_.finish() == sp_.finish().finish()) {
            // we're at the end, turn into a null value
            sp_ = SpanInterval(0,0,0,0);
            curr_ = Interval(0,0);
            isDead_ = true;
            return *this;
        }

        if (curr_.finish() != sp_.finish().finish()) {
            curr_.setFinish(curr_.finish()+1);
        } else {
            curr_.setStart(curr_.start()+1);
            curr_.setFinish((sp_.finish().start() >= curr_.start() ? sp_.finish().start() : curr_.start()));
        }
        return *this;
    }

    SpanIntervalIterator operator++(int) {
        SpanIntervalIterator old(*this);
        operator ++();
        return old;
    }
private:
    SpanInterval sp_;
    Interval curr_;
    bool isDead_;
};

/**
 * Simple functor for sorting spanning intervals by their starting range
 * start point
 */
struct SpanIntervalStartComparator : std::binary_function<SpanInterval, SpanInterval, bool> {
public:
    bool operator()(const SpanInterval& l, const SpanInterval& r) const {
        return (l.start().start() < r.start().start());
    }
};

/**
 * Simple functor for sorting spanning intervals by their finishing range
 * start point
 */
struct SpanIntervalFinishComparator : std::binary_function<SpanInterval, SpanInterval, bool> {
public:
    bool operator()(const SpanInterval& l, const SpanInterval& r) const {
        return (l.finish().start() < r.finish().start());
    }
};

// IMPLEMENTATION
inline SpanInterval::SpanInterval()
: start_(0, 0), finish_(0, 0) {}
inline SpanInterval::SpanInterval(const Interval& liq)
: start_(liq), finish_(liq) {}
inline SpanInterval::SpanInterval(unsigned int liqStart, unsigned int liqEnd)
: start_(liqStart, liqEnd), finish_(liqStart, liqEnd) {}
inline SpanInterval::SpanInterval(const Interval& start, const Interval& end)
: start_(start), finish_(end) {}

inline SpanInterval::SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo)
: start_(startFrom, startTo), finish_(endFrom, endTo) {}

inline SpanInterval::const_iterator SpanInterval::begin() const {return SpanIntervalIterator(*this);}
inline SpanInterval::const_iterator SpanInterval::end() const {return SpanIntervalIterator();}

inline Interval const& SpanInterval::start() const {return start_;};
inline Interval const& SpanInterval::finish() const {return finish_;};
inline void SpanInterval::setStart(const Interval& start) {start_ = start;};
inline void SpanInterval::setFinish(const Interval& end) {finish_ = end;};

inline bool operator==(const SpanInterval& a, const SpanInterval& b) {
    return (a.start() == b.start() && a.finish() == b.finish());
}
inline bool operator!=(const SpanInterval& a, const SpanInterval& b) {return !operator==(a,b);}
inline bool operator<(const SpanInterval& a, const SpanInterval& b) {
    if (operator==(a,b)) return false;
    if (a.start() < b.start()) return true;
    if (a.start() > b.start()) return false;
    if (a.finish() < b.finish()) return true;
    if (a.finish() > b.finish()) return false;
    // return false as failure (should never hit this point)
    throw std::runtime_error("error while applying operator< on spanintervals; must be equal, but == returns false");
}

inline bool operator> (const SpanInterval& a, const SpanInterval& b) {return  operator<(b,a);}
inline bool operator>=(const SpanInterval& a, const SpanInterval& b) {return !operator<(a,b);}
inline bool operator<=(const SpanInterval& a, const SpanInterval& b) {return !operator>(a,b);}
inline std::size_t hash_value(const SpanInterval& si) {
    std::size_t seed = 0;
    boost::hash_combine(seed, si.start_);
    boost::hash_combine(seed, si.finish_);
    return seed;
}

inline bool SpanInterval::isEmpty() const {
    unsigned int j = std::min(start_.finish(), finish_.finish());
    unsigned int k = std::max(finish_.start(), start_.start());

    if (start_.start() > j
            || finish_.finish() < k)
        return true;
    return false;
}

inline unsigned int SpanInterval::liqSize() const {
    if (isEmpty()) return 0;
    SpanInterval si = normalize().get();

    if (!si.isLiquid())
        LOG_PRINT(LOG_WARN) << "calling liqSize() on a non-liquid interval; this is probably not something you want to do" << std::endl;

    return si.start().size();
}

inline bool SpanInterval::isLiquid() const {
    return (start().start() == finish().start() && start().finish() == finish().finish());
}

// TODO: is this correct?
inline SpanInterval SpanInterval::toLiquidInc() const {
    unsigned int i = std::min(start().start(), finish().start());
    unsigned int j = std::max(start().finish(), finish().finish());
    return SpanInterval(i, j, i, j);
}

inline SpanInterval SpanInterval::toLiquidExc() const {
    unsigned int i = std::max(start().start(), finish().start());
    unsigned int j = std::min(start().finish(), finish().finish());
    return SpanInterval(i, j, i, j);
}

inline boost::optional<SpanInterval> SpanInterval::normalize() const {
    if (isEmpty()) {
        return boost::optional<SpanInterval>();
    }
    int j = std::min(start_.finish(), finish_.finish());
    int k = std::max(finish_.start(), start_.start());

    return boost::optional<SpanInterval>(SpanInterval(start_.start(), j, k, finish_.finish()));
}


inline unsigned int SpanInterval::size() const {
    if (isEmpty()) return 0;

    SpanInterval si = normalize().get();

    unsigned int i = si.start().start();
    unsigned int j = si.start().finish();
    unsigned int k = si.finish().start();
    unsigned int l = si.finish().finish();

    if (j <= k) {
        return ((l-k)+1) * ((j-i)+1);
    }
    // I'm so sorry about the below formula; TODO: rewrite this nicer
    return ((k-i)+1) * ((l-k)+1)
              + (j-k)*(l+1) - (j*(j+1))/2 + (k*(k+1))/2;

}

template <class OutputIterator>
inline void SpanInterval::compliment(const SpanInterval& universe, OutputIterator out) const {
    universe.subtract(*this, out);
}

template <class OutputIterator>
inline void SpanInterval::liqCompliment(const SpanInterval& universe, OutputIterator out) const {
    universe.liqSubtract(*this, out);
}

template <class OutputIterator>
void SpanInterval::subtract(const SpanInterval &remove, OutputIterator out) const {
    boost::optional<SpanInterval> intersectOpt = intersection(*this, remove);
    if (!intersectOpt) {   // no intersection, don't subtract anything
        *out = *this;
    } else {
        SpanInterval intersect = *intersectOpt;
        boost::optional<Interval> a, b, c, d;

        if (intersect.start().start()!=0)          a = Interval(start().start()               , intersect.start().start()-1);
        if (intersect.start().finish()!=UINT_MAX)  b = Interval(intersect.start().finish()+1 , start().finish());
        if (intersect.finish().start()!=0)         c = Interval(finish().start()              , intersect.finish().start()-1);
        if (intersect.finish().finish()!=UINT_MAX) d = Interval(intersect.finish().finish()+1, finish().finish());

        boost::optional<SpanInterval> s1,s2,s3,s4;
        if (a) s1 = SpanInterval(*a, finish()).normalize();
        if (c) s2 = SpanInterval(intersect.start(), *c).normalize();
        if (d) s3 = SpanInterval(intersect.start(), *d).normalize();
        if (b) s4 = SpanInterval(*b, finish()).normalize();

        if (s1) {*out = *s1; out++;}
        if (s2) {*out = *s2; out++;}
        if (s3) {*out = *s3; out++;}
        if (s4) {*out = *s4; out++;}
    }
}

template <class OutputIterator>
void SpanInterval::liqSubtract(const SpanInterval& remove, OutputIterator out) const {
    if (!isLiquid()) throw std::invalid_argument("SpanInterval::liqSubtract - *this is not liquid");
    if (!remove.isLiquid()) throw std::invalid_argument("SpanInterval::liqSubtract - remove is not liquid");

    boost::optional<SpanInterval> intersectOpt = intersection(*this, remove);
    if (!intersectOpt) {
        *out = *this;   // no intersection, don't subtract anything
    } else {
        SpanInterval intersect = *intersectOpt;
        boost::optional<Interval> a, b;

        if (intersect.start().start()!=0)         a = Interval(start().start(), intersect.start().start()-1);
        if (intersect.start().finish()!=UINT_MAX) b = Interval(intersect.start().finish()+1, start().finish());

        boost::optional<SpanInterval> s1, s2;
        if (a) s1 = SpanInterval(*a, *a).normalize();
        if (b) s2 = SpanInterval(*b, *b).normalize();
        if (s1) {*out = *s1; out++;}
        if (s2) {*out = *s2; out++;}
    }
}

inline std::string SpanInterval::toString() const {
    std::stringstream str;
    str << *this;
    return str.str();
}

inline std::ostream& operator<<(std::ostream& o, const SpanInterval& si) {
    o << "[";
    if (si.isLiquid()) {
        o << si.start().start() << ":" << si.start().finish() << "]";
    } else {
        o << "(" << si.start().start() << ", " << si.start().finish() << "), (" << si.finish().start() << ", " << si.finish().finish() << ")]";
    }
    return o;
}

inline boost::optional<SpanInterval> intersection(const SpanInterval& a, const SpanInterval& b) {
    return SpanInterval(std::max(a.start().start(), b.start().start()),
            std::min(a.start().finish(), b.start().finish()),
            std::max(a.finish().start(), b.finish().start()),
            std::min(a.finish().finish(), b.finish().finish())).normalize();    // TODO: more sensible way to pick max interval
}

template <class Archive>
void SpanInterval::serialize(Archive& ar, const unsigned int version) {
    ar & start_;
    ar & finish_;
}

#endif /* SPANINTERVAL_H */
