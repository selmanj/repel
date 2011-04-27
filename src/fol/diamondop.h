#ifndef DIAMONDOP_H
#define DIAMONDOP_H

#include <boost/shared_ptr.hpp>
#include <set>
#include "sentence.h"

class DiamondOp : public Sentence {
public:
	DiamondOp(boost::shared_ptr<Sentence> sentence) : s_(sentence) {
		// default is that it overlaps somehow
		rels_.insert(Interval::STARTS);
		rels_.insert(Interval::STARTSI);
		rels_.insert(Interval::DURING);
		rels_.insert(Interval::DURINGI);
		rels_.insert(Interval::FINISHES);
		rels_.insert(Interval::FINISHESI);
		rels_.insert(Interval::OVERLAPS);
		rels_.insert(Interval::OVERLAPSI);
	};

	template <class InputIterator>
	DiamondOp(boost::shared_ptr<Sentence> sentence, InputIterator begin, InputIterator end) : s_(sentence), rels_(begin, end) {};
	DiamondOp(const DiamondOp& dia) : s_(dia.s_) , rels_(dia.rels_) {}; // shallow copy
	virtual ~DiamondOp() {};

	boost::shared_ptr<Sentence> sentence() {return s_;};

private:
	std::set<Interval::INTERVAL_RELATION> rels_;
	boost::shared_ptr<Sentence> s_;

	virtual Sentence* doClone() const { return new DiamondOp(*this); };
	virtual bool doEquals(const Sentence& s) const {
		const DiamondOp *liq = dynamic_cast<const DiamondOp*>(&s);
		if (liq == NULL) {
			return false;
		}
		return *s_ == *(liq->s_);
	};

	virtual void doToString(std::string& str) const {
		str += "[ ";
		if (s_ != NULL) {
			str += s_->toString();
		}
		str += " ]";
	};

	virtual int doPrecedence() const { return 2; };
};

#endif
