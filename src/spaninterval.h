/*
 * spaninterval.h
 *
 *  Created on: Mar 30, 2011
 *      Author: Joe
 */

#ifndef SPANINTERVAL_H
#define SPANINTERVAL_H

#include "interval.h"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <climits>
#include <set>
#include <vector>

class SpanIntervalIterator;

class SpanInterval {
public:
	//SpanInterval(unsigned int smallest=0, unsigned int largest=UINT_MAX);
	SpanInterval(const Interval& start, const Interval& end,
			const Interval& maxInterval=Interval(0, UINT_MAX));
	SpanInterval(unsigned int startFrom, unsigned int startTo, unsigned int endFrom, unsigned int endTo,
			const Interval& maxInterval=Interval(0, UINT_MAX));

	typedef SpanIntervalIterator iterator;

	iterator begin() const;
	iterator end() const;

	Interval const& start() const {return start_;};
	Interval const& finish() const {return finish_;};
	Interval const& maxInterval() const {return maxInterval_;};
	void setStart(const Interval& start) {start_ = start;};
	void setFinish(const Interval& end) {finish_ = end;};
	boost::optional<SpanInterval> setMaxInterval(const Interval& maxInterval) const;

	bool operator==(const SpanInterval& b) const;
	bool operator!=(const SpanInterval& b) const;
	bool operator>(const SpanInterval& b) const;
	bool operator<(const SpanInterval& b) const;
	bool operator>=(const SpanInterval& b) const;
	bool operator<=(const SpanInterval& b) const;

	bool isEmpty() const;
	unsigned int size() const;
	unsigned int liqSize() const;
	bool isLiquid() const;
	SpanInterval toLiquid() const;
	boost::optional<SpanInterval> normalize() const;
	void compliment(std::set<SpanInterval>& collect) const;
	void liqCompliment(std::set<SpanInterval>& collect) const;
	boost::optional<SpanInterval> satisfiesRelation(Interval::INTERVAL_RELATION relation) const;

	void subtract(const SpanInterval& remove, std::set<SpanInterval>& collect) const;
	void liqSubtract(const SpanInterval& remove, std::set<SpanInterval>& collect) const;

	std::string toString() const;

	friend SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);

private:
	Interval start_, finish_;
	Interval maxInterval_;

};

inline bool SpanInterval::isEmpty() const {
	unsigned int j = std::min(start_.finish(), finish_.finish());
	unsigned int k = std::max(finish_.start(), start_.start());

	if (start_.start() > j
			|| finish_.finish() < k)
		return true;
	return false;
}

inline unsigned int SpanInterval::size() const {
	if (isEmpty()) return 0;

	SpanInterval si = normalize().get();

	unsigned int i = si.start().start();
	unsigned int j = si.start().finish();
	unsigned int k = si.finish().start();
	unsigned int l = si.finish().finish();

	if (j <= k) {
		return ((l-k)+1) * ((j-i)+1);
	}
	// I'm so sorry about the below formula; TODO: rewrite this nicer
	return ((k-i)+1) * ((l-k)+1)
			  + (j-k)*(l+1) - (j*(j+1))/2 + (k*(k+1))/2;

}


SpanInterval intersection(const SpanInterval& a, const SpanInterval& b);

class SpanIntervalIterator : public std::iterator<std::forward_iterator_tag, Interval> {
public:
	SpanIntervalIterator() : sp_(1,0,1,0), curr_(0,0) {};
	SpanIntervalIterator(const SpanInterval& sp) : sp_(1,0,1,0), curr_(0,0) {
		if (!sp.isEmpty()) {
			sp_ = sp.normalize().get();
			curr_ = Interval(sp_.start().start(), sp_.finish().start());
		}
	}
	bool operator==(const SpanIntervalIterator& other) const {
		return (sp_==other.sp_ && curr_==curr_);
	}
	bool operator!=(const SpanIntervalIterator& other) const {
		return !(this->operator ==(other));
	}
	const Interval& operator*() const {
		return curr_;
	}
	const Interval* operator->() const {
		return &curr_;
	}
	SpanIntervalIterator& operator++() {
		if (sp_.isEmpty()) {return *this;}	// do nothing
		if (curr_.start() == sp_.start().finish() && curr_.finish() == sp_.finish().finish()) {
			// we're at the end, turn into a null value
			sp_ = SpanInterval(1,0,1,0);
			curr_ = Interval(0,0);
			return *this;
		}
		if (curr_.finish() != sp_.finish().finish()) {
			curr_.setFinish(curr_.finish()+1);
		} else {
			curr_.setStart(curr_.start()+1);
			curr_.setFinish((sp_.finish().start() >= curr_.start() ? sp_.finish().start() : curr_.start()));
		}
		return *this;
	}

	SpanIntervalIterator operator++(int) {
		SpanIntervalIterator old(*this);
		operator ++();
		return old;
	}
private:
	SpanInterval sp_;
	Interval curr_;
};

#endif /* SPANINTERVAL_H */
