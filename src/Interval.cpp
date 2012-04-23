/*
 * interval.cpp
 *
 */
#include <stdexcept>
#include <boost/optional.hpp>
#include "interval.h"

std::string relationToString(Interval::INTERVAL_RELATION rel) {
    switch (rel) {
    case Interval::STARTS:
        return "s";
    case Interval::STARTSI:
        return "si";
    case Interval::MEETS:
        return "m";
    case Interval::MEETSI:
        return "mi";
    case Interval::DURING:
        return "d";
    case Interval::DURINGI:
        return "di";
    case Interval::FINISHES:
        return "f";
    case Interval::FINISHESI:
        return "fi";
    case Interval::OVERLAPS:
        return "o";
    case Interval::OVERLAPSI:
        return "oi";
    case Interval::GREATERTHAN:
        return ">";
    case Interval::LESSTHAN:
        return "<";
    case Interval::EQUALS:
        return "=";
    default:
        std::runtime_error error("given a relation that we have no mapping for");
        throw error;
    }
}

Interval::INTERVAL_RELATION inverseRelation(Interval::INTERVAL_RELATION rel) {
    switch (rel) {
        case Interval::STARTS:
            return Interval::STARTSI;
        case Interval::STARTSI:
            return Interval::STARTS;
        case Interval::MEETS:
            return Interval::MEETSI;
        case Interval::MEETSI:
            return Interval::MEETS;
        case Interval::DURING:
            return Interval::DURINGI;
        case Interval::DURINGI:
            return Interval::DURING;
        case Interval::FINISHES:
            return Interval::FINISHESI;
        case Interval::FINISHESI:
            return Interval::FINISHES;
        case Interval::OVERLAPS:
            return Interval::OVERLAPSI;
        case Interval::OVERLAPSI:
            return Interval::OVERLAPS;
        case Interval::GREATERTHAN:
            return Interval::LESSTHAN;
        case Interval::LESSTHAN:
            return Interval::GREATERTHAN;
        case Interval::EQUALS:
            return Interval::EQUALS;
        default:
            throw std::runtime_error("given an interval relation that we have no inverse for");
    }
}

// TODO: make this return a null interval instead
boost::optional<Interval> intersection(const Interval& a, const Interval& b) {
    if (a.isNull() || b.isNull()) return boost::optional<Interval>();
    if (a.finish() >= b.start() && a.start() <= b.finish())
        return Interval(b.start(), (a.finish() < b.finish() ? a.finish() : b.finish()));
    if (a.start() <= b.finish() && a.start() >= b.start())
        return Interval(a.start(), (a.finish() < b.finish() ? a.finish() : b.finish()));
    return boost::optional<Interval>();
}

bool relationHolds(const Interval& a, Interval::INTERVAL_RELATION rel, const Interval& b) {
    switch(rel) {
        case Interval::STARTS:
            return starts(a, b);
        case Interval::STARTSI:
            return startsI(a, b);
        case Interval::DURING:
            return during(a, b);
        case Interval::DURINGI:
            return duringI(a, b);
        case Interval::FINISHES:
            return finishes(a, b);
        case Interval::FINISHESI:
            return finishesI(a, b);
        case Interval::OVERLAPS:
            return overlaps(a, b);
        case Interval::OVERLAPSI:
            return overlapsI(a, b);
        case Interval::MEETS:
            return meets(a, b);
        case Interval::MEETSI:
            return meetsI(a, b);
        case Interval::LESSTHAN:
            return before(a, b);
        case Interval::GREATERTHAN:
            return after(a, b);
        case Interval::EQUALS:
            return equals(a, b);
        default:
            throw std::runtime_error("given an interval relation that we don't handle");
    }
}

std::vector<Interval> Interval::subtract(const Interval& i) const {
    std::vector<Interval> results;
    if (isNull()) {
        return results;
    } else if (i.isNull()) {
        results.push_back(*this);
        return results;
    }

    if (before(*this, i)
            || after(*this, i)
            || meets(*this, i)
            || meetsI(*this, i)) {
        // no possible way for them to intersect
        results.push_back(*this);
    } else if (starts(*this, i)
            || finishes(*this, i)
            || during(*this, i)
            || equals(i, *this)) {
        return results; // i spans *this
    } else if (startsI(*this, i) || overlapsI(*this, i)) {
        // form an interval from the end of i to the end of this
        results.push_back(Interval(i.finish()+1, finish()));
    } else if (overlaps(*this, i) || finishesI(*this, i)) {
        results.push_back(Interval(start(), i.start()-1));
    } else if (duringI(*this, i)) {
        // get start and end
        results.push_back(Interval(start(), i.start()-1));
        results.push_back(Interval(i.finish()+1, finish()));
    }
    return results;

}


