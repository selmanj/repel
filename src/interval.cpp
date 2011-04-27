/*
 * interval.cpp
 *
 */
#include <stdexcept>
#include "interval.h"

Interval::Interval(unsigned int start, unsigned int end)
: start_(start), end_(end)
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

