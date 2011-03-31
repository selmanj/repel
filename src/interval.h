#ifndef INTERVAL_H
#define INTERVAL_H

class Interval {
public:
	Interval();
	Interval(unsigned int start, unsigned int end);
	unsigned int start() const {return st;};
	unsigned int end() const {return en;};
	void setStart(unsigned int start) {st = start;};
	void setEnd(unsigned int end) {en = end;};
private:
	unsigned int st, en;
};

#endif
