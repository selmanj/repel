#ifndef DISJUNCTION_H
#define CONJUNCTION_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"
#include "sentencevisitor.h"

class Disjunction : public Sentence {
public:
	Disjunction(const boost::shared_ptr<Sentence>& left, const boost::shared_ptr<Sentence>& right)
		: left_(left), right_(right) {};
	Disjunction(const Disjunction& a) : left_(a.left_), right_(a.right_) {};	// shallow copy
	virtual ~Disjunction() {};

	Disjunction& operator=(const Disjunction& b) {
		left_ = b.left_;
		right_ = b.right_;

		return *this;
	}

	boost::shared_ptr<Sentence>& left() {return left_;};
	boost::shared_ptr<const Sentence> left() const {return left_;};
	boost::shared_ptr<Sentence>& right() {return right_;};
	boost::shared_ptr<const Sentence> right() const {return right_;};

private:
	boost::shared_ptr<Sentence> left_;
	boost::shared_ptr<Sentence> right_;

	virtual Sentence* doClone() const { return new Disjunction(*this); };

	virtual bool doEquals(const Sentence& s) const {
		const Disjunction *con = dynamic_cast<const Disjunction*>(&s);
		if (con == NULL) {
			return false;
		}
		return (*left_ == *(con->left_) && *right_ == *(con->right_));
	};

	virtual void doToString(std::stringstream& str) const {
		if (left_->precedence() > precedence()) {
			str << "(";
			str << left_->toString();
			str << ")";
		} else {
			str << left_->toString();
		}
		str << " v ";
		if (right_->precedence() > precedence()) {
			str << "(";
			str << right_->toString();
			str << ")";
		} else {
			str << right_->toString();
		}
	};

	virtual int doPrecedence() const { return 4; };
	virtual void visit(SentenceVisitor& v) const {
		left_->visit(v);
		right_->visit(v);

		v.accept(*this);
	}
};

#endif
