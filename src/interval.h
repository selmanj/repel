#ifndef INTERVAL_H
#define INTERVAL_H

#include <string>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <stdexcept>

/**
 * Class Interval represents an interval and defines useful relations between
 * intervals.
 */
class Interval {
public:

    /**
     * The INTERVAL_RELATION enumeration represents all the possible relations
     * that can hold between two intervals.  They are usually related in terms
     * of time.  @see http://en.wikipedia.org/wiki/Allen's_interval_algebra
     */
    enum INTERVAL_RELATION {
        MEETS,
        MEETSI,
        OVERLAPS,
        OVERLAPSI,
        STARTS,
        STARTSI,
        DURING,
        DURINGI,
        FINISHES,
        FINISHESI,
        EQUALS,
        GREATERTHAN,
        LESSTHAN
    };

    /**
     * Construct an empty interval that spans no interval of time (null interval).
     */
    Interval();

    /**
     * Construct an interval.
     *
     * @param start the start point
     * @param finish the end point
     */
    Interval(unsigned int start, unsigned int finish);

    /**
     * Get the starting point.
     *
     * @return the start point of the interval
     */
    unsigned int start() const;

    /**
     * Get the finish point.
     *
     * @return the finish point of the interval
     */
    unsigned int finish() const;

    /**
     * Get the size (length) of the interval.
     *
     * @return the size of the interval, or 0 if the starting point is greater
     *   than the finishing point.
     */
    unsigned int size() const;

    /**
     * Set the start point.
     *
     * @param start the start point of the interval
     */
    void setStart(unsigned int start);

    /**
     * Set the finish point.
     *
     * @param finish the finish point of the interval
     */
    void setFinish(unsigned int finish);

    /**
     * Check to see if this interval is null.  A null (or empty) interval
     * spans no amount of time and will throw exceptions when its
     * start/endpoints are accessed.
     *
     * @returns true if the interval is null or empty, false otherwise
     */
    bool isNull() const;

    /**
     * Check to see if this interval spans another interval.  An interval "A"
     * spans interval "B" if A.start <= A.start and A.finish >= B.finish.
     *
     * @param i Interval to check to see if it's spanned by this.
     * @return true if this spans i, false otherwise
     */
    bool spans(const Interval& i) const;

    /**
     * Subtract an interval from another.  "Subtract" is defined as removing
     * any overlap (intersection) that the left interval has.  For instance,
     * interval (1,4).subtract(2,3)
     */
    Interval subtract(const Interval& i) const;

    /* friend functions */
    friend bool meets (const Interval& lhs, const Interval& rhs);
    friend bool meetsI(const Interval& lhs, const Interval& rhs);
    friend bool overlaps (const Interval& lhs, const Interval& rhs);
    friend bool overlapsI(const Interval& lhs, const Interval& rhs);
    friend bool starts (const Interval& lhs, const Interval& rhs);
    friend bool startsI(const Interval& lhs, const Interval& rhs);
    friend bool during (const Interval& lhs, const Interval& rhs);
    friend bool duringI(const Interval& lhs, const Interval& rhs);
    friend bool finishes (const Interval& lhs, const Interval& rhs);
    friend bool finishesI(const Interval& lhs, const Interval& rhs);
    friend bool equals(const Interval& lhs, const Interval& rhs);
    friend bool after (const Interval& lhs, const Interval& rhs);
    friend bool before(const Interval& lhs, const Interval& rhs);

    friend std::size_t hash_value(const Interval& i);
    friend std::ostream& operator<<(std::ostream& o, const Interval& i);
    friend bool operator==(const Interval& l, const Interval& r);
    friend bool operator!=(const Interval& l, const Interval& r);
    friend bool operator< (const Interval& l, const Interval& r);
    friend bool operator> (const Interval& l, const Interval& r);
    friend bool operator<=(const Interval& l, const Interval& r);
    friend bool operator>=(const Interval& l, const Interval& r);
    friend Interval span(const Interval& a, const Interval& b);

private:
    unsigned int s_, e_;
    bool isNull_;
};

/**
 * Check for equality.
 */
bool operator==(const Interval& lhs, const Interval& rhs);
/**
 * Check for inequality.
 */
bool operator!=(const Interval& lhs, const Interval& rhs);
/**
 * Check if an interval is less than another interval.  Note that this is not
 * the same as the "<" Allen relation; it merely provides an ordering (the
 * string ordering) on Intervals.
 */
bool operator< (const Interval& lhs, const Interval& rhs);
/**
 * Check if an interval is less than or equal to another interval.
 * @see operator<(const Interval& lhs, const Interval& rhs)
 */
bool operator<=(const Interval& lhs, const Interval& rhs);
/**
 * Check if an interval is greater than or equal to another interval.
 * @see operator<(const Interval& lhs, const Interval& rhs)
 */
bool operator>=(const Interval& lhs, const Interval& rhs);
/**
 * Check if an interval is greater than another interval.
 * @see operator<(const Interval& lhs, const Interval& rhs)
 */
bool operator> (const Interval& lhs, const Interval& rhs);

/**
 * Check if two intervals obey the "meets" Allen relation.
 */
bool meets (const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "meets inverse" Allen relation.
 */
bool meetsI(const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "overlaps" Allen relation.
 */
bool overlaps (const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "overlaps inverse" Allen relation.
 */
bool overlapsI(const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "starts" Allen relation.
 */
bool starts (const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "starts inverse" Allen relation.
 */
bool startsI(const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "during" Allen relation.
 */
bool during (const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "during inverse" Allen relation.
 */
bool duringI(const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "finishes" Allen relation.
 */
bool finishes (const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "finishes inverse" Allen relation.
 */
bool finishesI(const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "equals" Allen relation.
 */
bool equals(const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "greater than" Allen relation.  This was
 * renamed to "after" in order to prevent confusion with operator>.
 */
bool after (const Interval& lhs, const Interval& rhs);
/**
 * Check if two intervals obey the "less than" Allen relation.  This was
 * renamed to "before" in order to prevent confusion with operator<.
 */
bool before(const Interval& lhs, const Interval& rhs);

/**
 * Compute the hash value for an interval.
 */
std::size_t hash_value(const Interval& i);

/**
 * Output the interval to a stream.  An interval is displayed as "(a, b)",
 * where a is the start point and b is the finish point.
 */
std::ostream& operator<<(std::ostream& os, const Interval& obj);

/**
 * Get the string representation of an interval relation.
 *
 * @param rel relation in question
 * @return a string representation of rel
 */
std::string relationToString(Interval::INTERVAL_RELATION rel);

/**
 * Get the inverse relation of a relation, so that the relation holds if the
 * arguments are reversed.  For instance, if "a" meets "b", then "b"
 * meetsInverse "a".   Note that the inverse of the equals relation is the
 * equals relation itself.
 *
 * @param rel relation to invert
 * @return the inverse relation of rel
 */
Interval::INTERVAL_RELATION inverseRelation(Interval::INTERVAL_RELATION rel);

/**
 * Check to see if a particular relation holds between two intervals.  This
 * is just a mapping from each interval relation to the function version (ie,
 * it maps Interval::MEETS to meets().
 *
 * @param lhs relation on the left
 * @param rel relation to check
 * @param rhs relation on the right
 * @return true if the relation holds, false otherwise.
 */
bool relationHolds(const Interval& lhs, Interval::INTERVAL_RELATION rel, const Interval& rhs);

/**
 * Compute the overlap of two intervals.
 *
 * @param a first interval
 * @param b second interval
 * @return either an interval representing the overlap, or an empty
 * boost::optional<Interval> object representing no overlap
 */
boost::optional<Interval> intersection(const Interval& a, const Interval& b);

/**
 * Compute the span of two intervals.  The span is the shortest interval
 * that spans both a and b.
 *
 * @param a first interval
 * @param b second interval
 * @return An interval spanning both intervals
 */
Interval span(const Interval& a, const Interval& b);

// IMPLEMENTATION FOLLOWS
inline Interval::Interval()
    : s_(0), e_(0), isNull_(true) {}
inline Interval::Interval(unsigned int start, unsigned int end)
    : s_(start), e_(end), isNull_(false) {}

inline unsigned int Interval::start() const {
    if (isNull_) throw std::logic_error("cannot get starting point for a null interval");
    return s_;
};
inline unsigned int Interval::finish() const {
    if (isNull_) throw std::logic_error("cannot get finish point for a null interval");
    return e_;
};
inline unsigned int Interval::size() const {
    if (isNull_) return 0;
    return e_-s_+1;
};

inline bool Interval::isNull() const { return isNull_; }
inline void Interval::setStart(unsigned int start) {
    if (isNull_) {
        s_ = start;
        e_ = start;
        isNull_ = false;
    } else if (start > e_) {
        s_ = 0;
        e_ = 0;
        isNull_ = true;
    } else {
        s_ = start;
    }
};
inline void Interval::setFinish(unsigned int end) {
    if (isNull_) {
        s_ = end;
        e_ = end;
    } else if (end < s_) {
        s_ = 0;
        e_ = 0;
        isNull_ = true;
    } else {
        e_ = end;
    }
};

inline bool Interval::spans(const Interval& i) const {
    if (isNull_ || i.isNull_) return false;
    return (s_ <= i.s_ && e_ >= i.e_);
}

inline bool meets(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.e_+1 == rhs.s_;
}

inline bool meetsI(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return rhs.e_+1 == lhs.s_;
}

inline bool overlaps(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ < rhs.s_ && lhs.e_ >= rhs.s_ && lhs.e_ < rhs.e_;
}

inline bool overlapsI(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ > rhs.s_ && lhs.s_ <= rhs.e_ && lhs.e_ > rhs.e_;
}

inline bool starts(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ == rhs.s_ && lhs.e_ < rhs.e_;
}

inline bool startsI(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ == rhs.s_ && rhs.e_ < lhs.e_;
}

inline bool during(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ > rhs.s_ && lhs.e_ < rhs.e_;
}

inline bool duringI(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return rhs.s_ > lhs.s_ && rhs.e_ < lhs.e_;
}

inline bool finishes(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ > rhs.s_ && lhs.e_ == rhs.e_;
}

inline bool finishesI(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return rhs.s_ > lhs.s_ && rhs.e_ == lhs.e_;
}

inline bool equals(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    // in case == is ever defined more exactly, for now just define the
    // relation here as well
    return lhs.s_ == rhs.s_ && lhs.e_ == rhs.e_;
}

inline bool after(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.s_ > rhs.e_+1;
}

inline bool before(const Interval& lhs, const Interval& rhs) {
    if (lhs.isNull_ || rhs.isNull_) return false;
    return lhs.e_+1 < rhs.s_;
}

inline std::size_t hash_value(const Interval& i) {
    std::size_t seed = 0;
    boost::hash_combine(seed, i.isNull_);
    boost::hash_combine(seed, i.s_);
    boost::hash_combine(seed, i.e_);
    return seed;
}

inline std::ostream& operator<<(std::ostream& os, const Interval& obj) {
    if (obj.isNull_) {
        os << "(null)";
    } else {
        os << "(" << obj.s_ << ", " << obj.e_ << ")";
    }
    return os;
}

inline bool operator==(const Interval& lhs, const Interval& rhs) {return (lhs.isNull_ == rhs.isNull_ && lhs.start() == rhs.start() && lhs.finish() == rhs.finish());}
inline bool operator!=(const Interval& lhs, const Interval& rhs) {return !operator==(lhs, rhs);}
inline bool operator< (const Interval& lhs, const Interval& rhs) {
    if (lhs.s_ < rhs.s_
            || (lhs.s_ == rhs.s_
                    && lhs.e_ < rhs.e_)) return true;
    return false;
}
inline bool operator<=(const Interval& lhs, const Interval& rhs) {return !operator> (lhs, rhs);}
inline bool operator>=(const Interval& lhs, const Interval& rhs) {return !operator< (lhs, rhs);}
inline bool operator> (const Interval& lhs, const Interval& rhs) {return  operator< (rhs, lhs);}

inline Interval span(const Interval& a, const Interval& b) {
    if (a.isNull_ || b.isNull_) {
        throw std::logic_error("cannot calculate span for a null interval");
    }
    return Interval((a.start() < b.start() ? a.start() : b.start()),
            (a.finish() > b.finish() ? a.finish() : b.finish()));
}

#endif
