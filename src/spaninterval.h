/*
 * spaninterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H

#include "interval.h"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <climits>
#include <set>
#include <vector>
#include <list>
#include <iterator>

class SpanIntervalIterator;

class SpanInterval {
public:
    //SpanInterval(unsigned int smallest=0, unsigned int largest=UINT_MAX);
    SpanInterval(const Interval& start, const Interval& end,
            const Interval& maxInterval=Interval(0, UINT_MAX));
    SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo,
            const Interval& maxInterval=Interval(0, UINT_MAX));

    typedef SpanIntervalIterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    Interval const& start() const {return start_;};
    Interval const& finish() const {return finish_;};
    Interval const& maxInterval() const {return maxInterval_;};
    void setStart(const Interval& start) {start_ = start;};
    void setFinish(const Interval& end) {finish_ = end;};
    boost::optional<SpanInterval> setMaxInterval(const Interval& maxInterval) const;

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

    template <class OutputIterator>
    void compliment(OutputIterator out) const;
    template <class OutputIterator>
    void liqCompliment(OutputIterator out) const;
    boost::optional<SpanInterval> satisfiesRelation(Interval::INTERVAL_RELATION relation) const;

    template <class OutputIterator>
    void subtract(const SpanInterval& remove, OutputIterator out) const;
    template <class OutputIterator>
    void liqSubtract(const SpanInterval& remove, OutputIterator out) const;

    std::string toString() const;

    friend SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);

private:
    Interval start_, finish_;
    Interval maxInterval_;

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
void SpanInterval::compliment(OutputIterator out) const {
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
        SpanInterval a(maxInterval_.start(), start().start()-1, maxInterval_.start(), maxInterval_.finish(), maxInterval_);
        if (a.normalize()) {
            *out = (a.normalize().get());
            out++;
        }
    }
    if (finish().start() != maxInterval_.start()) {
        SpanInterval b(start().start(), start().finish(), maxInterval_.start(), finish().start()-1, maxInterval_);
        if (b.normalize()) {
            *out = b.normalize().get();
            out++;
        }
    }
    if (finish().finish() != maxInterval_.finish()) {
        SpanInterval c(start().start(), start().finish(), finish().finish()+1, maxInterval_.finish(), maxInterval_);
        if (c.normalize()) {
            *out = c.normalize().get();
            out++;
        }
    }
    if (start().finish() != maxInterval_.finish()) {
        SpanInterval d(start().finish()+1, maxInterval_.finish(), maxInterval_.start(), maxInterval_.finish(), maxInterval_);
        if (d.normalize()) {
            *out = d.normalize().get();
            out++;
        }
    }
}

template <class OutputIterator>
void SpanInterval::liqCompliment(OutputIterator out) const {
    // at most two intervals
    if (start().start() != maxInterval_.start()) {
        unsigned int end = start().start()-1;
        SpanInterval a(maxInterval_.start(), end, maxInterval_.start(), end, maxInterval_);
        *out = a;
        out++;
    }
    if (finish().finish() != maxInterval_.finish()) {
        unsigned int start = finish().finish()+1;
        SpanInterval b(start, maxInterval_.finish(), start, maxInterval_.finish(), maxInterval_);
        *out = b;
        out++;
    }
}

template <class OutputIterator>
void SpanInterval::subtract(const SpanInterval &remove, OutputIterator out) const {
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
    std::list<SpanInterval> compliment;
    remove.compliment(back_inserter(compliment));
    for (std::list<SpanInterval>::const_iterator it = compliment.begin(); it != compliment.end(); it++) {
        SpanInterval intersect = intersection(*this, *it);
        if (intersect.size() > 0) {
            *out = intersect;
            out++;
        }
    }
}

template <class OutputIterator>
void SpanInterval::liqSubtract(const SpanInterval& remove, OutputIterator out) const {
    // well this is easy
    std::list<SpanInterval> compliment;
    remove.liqCompliment(back_inserter(compliment));
    for (std::list<SpanInterval>::const_iterator it = compliment.begin(); it != compliment.end(); it++) {
        SpanInterval intersect = intersection(*this, *it);
        if (intersect.size() > 0) {
            *out = intersect;
            out++;
        }
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
