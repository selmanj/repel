/*
 * interval.cpp
 *
 */
#include <stdexcept>
#include "interval.h"

Interval::Interval(unsigned int start, unsigned int end)
: s_(start), e_(end)
{
}

bool Interval::operator==(const Interval& b) const {
	return (start() == b.start() && end() == b.end());
}

bool Interval::operator!=(const Interval& b) const {
	return !(operator==(b));
}

bool Interval::operator>(const Interval& b) const {
	if (operator==(b)) return false;
	return !operator<(b);
}

bool Interval::operator<(const Interval& b) const {
	if (operator==(b)) return false;
	if (start() < b.start()) return true;
	if (start() > b.start()) return false;
	if (end() < b.end()) return true;
	if (end() > b.end()) return false;
	return false; // should never hit this point
}

bool Interval::operator>=(const Interval& b) const {
	return !operator<(b);
}

bool Interval::operator<=(const Interval& b) const {
	return !operator>(b);
}

bool Interval::meets(const Interval& b) const {
	return e_+1 == b.s_;
}

bool Interval::meetsI(const Interval& b) const {
	return b.e_+1 == s_;
}

bool Interval::overlaps(const Interval& b) const {
	return s_ < b.s_ && e_ >= b.s_ && e_ < b.e_;
}

bool Interval::overlapsI(const Interval& b) const {
	return s_ > b.s_ && s_ <= b.e_ && e_ > b.e_;
}

bool Interval::starts(const Interval& b) const {
	return s_ == b.s_ && e_ < b.e_;
}

bool Interval::startsI(const Interval& b) const {
	return s_ == b.s_ && b.e_ < e_;
}

bool Interval::during(const Interval& b) const {
	return s_ > b.s_ && e_ < b.e_;
}

bool Interval::duringI(const Interval& b) const {
	return b.s_ > s_ && b.e_ < e_;
}

bool Interval::finishes(const Interval& b) const {
	return s_ > b.s_ && e_ == b.e_;
}

bool Interval::finishesI(const Interval& b) const {
	return b.s_ > s_ && b.e_ == e_;
}

bool Interval::equals(const Interval& b) const {
	// in case == is ever defined more exactly, for now just define the
	// relation here as well
	return s_ == b.s_ && e_ == b.e_;
}

bool Interval::after(const Interval& b) const {
	return s_ > b.e_+1;
}

bool Interval::before(const Interval& b) const {
	return e_+1 < b.s_;
}

std::string Interval::relationToString(INTERVAL_RELATION rel) {
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
