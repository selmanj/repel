/*
 * siset.h
 *
 *  Created on: May 24, 2011
 *      Author: joe
 */

#ifndef SISET_H_
#define SISET_H_
#include <set>
#include <list>
#include <iostream>
#include "spaninterval.h"

class SISet {
public:
    SISet(bool forceLiquid=false,
            const Interval& maxInterval=Interval(0, UINT_MAX))
    : set_(), forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}
    explicit SISet(const SpanInterval& si, bool forceLiquid=false,
          const Interval& maxInterval=Interval(0, UINT_MAX));
    template <class InputIterator>
    SISet(InputIterator begin, InputIterator end,
            bool forceLiquid=false,
            const Interval& maxInterval=Interval(0, UINT_MAX))
    : set_(begin, end), forceLiquid_(forceLiquid), maxInterval_(maxInterval) {}

    typedef std::list<SpanInterval>::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    std::set<SpanInterval> asSet() const;   /** DEPRECATED */
    bool forceLiquid() const {return forceLiquid_;};
    // TODO make some of these friend functions
    bool isDisjoint() const;
    SISet compliment() const;
    Interval maxInterval() const;
    unsigned int size() const;
    unsigned int liqSize() const;
    bool empty() const;
    const std::list<SpanInterval>& intervals() const {return set_;}

    // modifiers
    void add(const SpanInterval &s);
    void add(const SISet& b);

    void makeDisjoint();
    void clear() {set_.clear();};
    void setMaxInterval(const Interval& maxInterval);
    void setForceLiquid(bool forceLiquid);
    void subtract(const SpanInterval& si);
    void subtract(const SISet& sis);
    const SISet satisfiesRelation(const Interval::INTERVAL_RELATION& rel) const;

    static SISet randomSISet(bool forceLiquid, const Interval& maxInterval);
    SpanInterval randomSI() const;

    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& o, const SISet& s);
    friend bool operator ==(const SISet& l, const SISet& r);
    friend bool operator !=(const SISet& l, const SISet& r);

    friend SISet intersection(const SISet& a, const SISet& b);
    friend SISet intersection(const SISet& a, const SpanInterval& si);
    friend SISet span(const SpanInterval& a, const SpanInterval& b, const Interval& maxInterval);
    friend bool equalByInterval(const SISet& a, const SISet& b);
private:
    std::list<SpanInterval> set_;
    bool forceLiquid_;
    Interval maxInterval_;
};
SISet span(const SpanInterval& a, const SpanInterval& b, const Interval& maxInterval);
SISet composedOf(const SpanInterval& a, const SpanInterval& b, Interval::INTERVAL_RELATION, const SpanInterval& universe);
bool equalByInterval(const SISet& a, const SISet& b);

unsigned long hammingDistance(const SISet& a, const SISet& b);

// IMPLEMENTATION
inline SISet::const_iterator SISet::begin() const {return set_.begin();}
inline SISet::const_iterator SISet::end() const {return set_.end();}
inline bool SISet::empty() const { return size() == 0;}

inline bool operator==(const SISet& l, const SISet& r) {return (l.set_ == r.set_);}    //TODO: is this the right thing to do???
inline bool operator!=(const SISet& l, const SISet& r) {return !operator==(l,r);}


#endif /* SISET_H_ */
