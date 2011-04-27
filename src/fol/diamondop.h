#ifndef LIQUIDOP_H
#define LIQUIDOP_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"

class LiquidOp : public Sentence {
public:
	LiquidOp(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
	LiquidOp(const LiquidOp& neg) : s_(neg.s_) {}; // shallow copy
	virtual ~LiquidOp() {};

	boost::shared_ptr<Sentence> sentence() {return s_;};

private:

	boost::shared_ptr<Sentence> s_;
	virtual Sentence* doClone() const { return new LiquidOp(*this); };
	virtual bool doEquals(const Sentence& s) const {
		const LiquidOp *liq = dynamic_cast<const LiquidOp*>(&s);
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
