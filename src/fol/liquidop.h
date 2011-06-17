#ifndef LIQUIDOP_H
#define LIQUIDOP_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"

class LiquidOp : public Sentence {
public:
	LiquidOp(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
	LiquidOp(const LiquidOp& neg) : s_(neg.s_) {}; // shallow copy
	virtual ~LiquidOp() {};

	LiquidOp& operator=(const LiquidOp& b) {
		s_ = b.s_;
		return *this;
	};
	boost::shared_ptr<Sentence> sentence() {return s_;};
	boost::shared_ptr<const Sentence> sentence() const {return s_;};
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

	virtual void visit(SentenceVisitor& v) const {
		s_->visit(v);

		v.accept(*this);
	}
};

#endif
