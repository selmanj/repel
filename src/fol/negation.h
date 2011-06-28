#ifndef NEGATION_H
#define NEGATION_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"
#include "sentencevisitor.h"

class Negation : public Sentence {
public:
	Negation(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
	Negation(const Negation& neg) : s_(neg.s_) {}; // shallow copy
	virtual ~Negation() {};

	Negation& operator=(const Negation& n) {
		s_ = n.s_;
		return *this;
	}

	boost::shared_ptr<Sentence>& sentence() {return s_;};
	boost::shared_ptr<const Sentence> sentence() const {return s_;};

private:

	boost::shared_ptr<Sentence> s_;
	virtual Sentence* doClone() const { return new Negation(*this); };
	virtual bool doEquals(const Sentence& s) const {
		const Negation *neg = dynamic_cast<const Negation*>(&s);
		if (neg == NULL) {
			return false;
		}
		return *s_ == *(neg->s_);
	};

	virtual void doToString(std::string& str) const {
		str += "!";
		if (s_ != NULL) {
			if (s_->precedence() > precedence()) {
				str += "(";
				str += s_->toString();
				str += ")";
			} else {
				str += s_->toString();
			}
		}
	};

	virtual int doPrecedence() const { return 2; };

	virtual void visit(SentenceVisitor& v) const {
		s_->visit(v);

		v.accept(*this);
	}
};

#endif
