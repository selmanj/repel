#ifndef INTERVAL_H
#define INTERVAL_H

#include <string>

class Interval {
public:
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

	Interval();
	Interval(unsigned int start, unsigned int end);
	unsigned int start() const {return s_;};
	unsigned int end() const {return e_;};
	void setStart(unsigned int start) {s_ = start;};
	void setEnd(unsigned int end) {e_ = end;};

	bool operator==(const Interval& b) const;
	bool operator!=(const Interval& b) const;
	bool operator>(const Interval& b) const;	// NOTE!  these are NOT the same as allen relations greater than and less than!!
	bool operator<(const Interval& b) const;
	bool operator>=(const Interval& b) const;
	bool operator<=(const Interval& b) const;

	bool meets(const Interval& b) const;
	bool meetsI(const Interval& b) const;
	bool overlaps(const Interval& b) const;
	bool overlapsI(const Interval& b) const;
	bool starts(const Interval& b) const;
	bool startsI(const Interval& b) const;
	bool during(const Interval& b) const;
	bool duringI(const Interval& b) const;
	bool finishes(const Interval& b) const;
	bool finishesI(const Interval& b) const;
	bool equals(const Interval& b) const;	// same as ==
	bool after(const Interval& b) const;
	bool before(const Interval& b) const;

	static std::string relationToString(INTERVAL_RELATION rel);
private:
	unsigned int s_, e_;
};

#endif
