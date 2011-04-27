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
	unsigned int start() const {return start_;};
	unsigned int end() const {return end_;};
	void setStart(unsigned int start) {start_ = start;};
	void setEnd(unsigned int end) {end_ = end;};

	bool operator==(const Interval& b) const;
	bool operator!=(const Interval& b) const;
	bool operator>(const Interval& b) const;
	bool operator<(const Interval& b) const;
	bool operator>=(const Interval& b) const;
	bool operator<=(const Interval& b) const;

	static std::string relationToString(INTERVAL_RELATION rel);
private:
	unsigned int start_, end_;
};

#endif
