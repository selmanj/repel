/*
 * interval.cpp
 *
 */

#include "interval.h"

Interval::Interval(unsigned int start, unsigned int end)
	: st(start), en(end)
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
