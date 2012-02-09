/*
 * spaninterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H


#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <climits>
#include <set>
#include <vector>
#include <list>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include "interval.h"

class SpanIntervalIterator;

class SpanInterval {
public:
    //SpanInterval(unsigned int smallest=0, unsigned int largest=UINT_MAX);
    explicit SpanInterval(const Interval& liq);
    explicit SpanInterval(const Interval& start, const Interval& end);
    explicit SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo);

    typedef SpanIntervalIterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    Interval const& start() const {return start_;};
    Interval const& finish() const {return finish_;};
    void setStart(const Interval& start) {start_ = start;};
    void setFinish(const Interval& end) {finish_ = end;};

    bool operator==(const SpanInterval& b) const;
    bool operator!=(const SpanInterval& b) const;
    bool operator>(const SpanInterval& b) const;
    bool operator<(const SpanInterval& b) const;
    bool operator>=(const SpanInterval& b) const;
    bool operator<=(const SpanInterval& b) const;

    bool isEmpty() const;
    unsigned int size() const;
    unsigned int liqSize() const;
    bool isLiquid() const;
    SpanInterval toLiquid() const;
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

    friend SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);
    friend std::ostream& operator<<(std::ostream& o, const SpanInterval& si);

private:
    Interval start_, finish_;
};

inline bool SpanInterval::isEmpty() const {
    unsigned int j = std::min(start_.finish(), finish_.finish());
    unsigned int k = std::max(finish_.start(), start_.start());

    if (start_.start() > j
            || finish_.finish() < k)
        return true;
    return false;
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
void SpanInterval::compliment(const SpanInterval& universe, OutputIterator out) const {
    universe.subtract(*this, out);
}

template <class OutputIterator>
void SpanInterval::liqCompliment(const SpanInterval& universe, OutputIterator out) const {
    universe.liqSubtract(*this, out);
}

template <class OutputIterator>
void SpanInterval::subtract(const SpanInterval &remove, OutputIterator out) const {
    boost::optional<SpanInterval> intersect = intersection(*this, remove);
    if (!intersect) {   // no intersection, don't subtract anything
        *out = *this;
    } else {
        Interval a(start().start()               , intersect->start().start()-1);
        Interval b(intersect->start().finish()+1 , start().finish());
        Interval c(finish().start()              , intersect->finish().start()-1);
        Interval d(intersect->finish().finish()+1, finish().finish());

        boost::optional<SpanInterval> s1 = SpanInterval(a, c).normalize();
        boost::optional<SpanInterval> s2 = SpanInterval(a, d).normalize();
        boost::optional<SpanInterval> s3 = SpanInterval(b, c).normalize();
        boost::optional<SpanInterval> s4 = SpanInterval(b, d).normalize();

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

    boost::optional<SpanInterval> intersect = intersection(*this, remove);
    if (!intersect) {
        *out = *this;   // no intersection, don't subtract anything
    } else {
        Interval a(start().start(), intersect->start().start()-1);
        Interval b(intersect->start().finish()+1, start().finish());

        boost::optional<SpanInterval> s1 = SpanInterval(a, a).normalize();
        boost::optional<SpanInterval> s2 = SpanInterval(b, b).normalize();
        if (s1) {*out = *s1; out++;}
        if (s2) {*out = *s2; out++;}
    }
}

SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);

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

#endif /* SPANINTERVAL_H */
