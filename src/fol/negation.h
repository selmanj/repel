#ifndef NEGATION_H
#define NEGATION_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"

class Negation : public Sentence {
public:
	Negation(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
	Negation(const Negation& neg) : s_(neg.s_->clone()) {}; // deep copy
	virtual ~Negation() {};

	boost::shared_ptr<Sentence> sentence() {return s_;};

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
};

#endif
