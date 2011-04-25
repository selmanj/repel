/*
 * interval.cpp
 *
 */
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
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
