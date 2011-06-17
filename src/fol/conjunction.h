#ifndef CONJUNCTION_H
#define CONJUNCTION_H

#include <set>
#include <boost/shared_ptr.hpp>
#include "sentence.h"
#include "sentencevisitor.h"
#include "../interval.h"

class Conjunction : public Sentence {
public:

	static const std::set<Interval::INTERVAL_RELATION>& defaultRelations() {
		static std::set<Interval::INTERVAL_RELATION>* defaults = new std::set<Interval::INTERVAL_RELATION>();
		if (defaults->empty()) {
			defaults->insert(Interval::EQUALS);
		}
		return *defaults;
	}

	template<class InputIterator>
	Conjunction(const boost::shared_ptr<Sentence>& left,
			const boost::shared_ptr<Sentence>& right,
			InputIterator begin,
			InputIterator end)
		: left_(left), right_(right), rels_(begin, end) {};
	Conjunction(const boost::shared_ptr<Sentence>& left, const boost::shared_ptr<Sentence>& right)
		: left_(left), right_(right), rels_(defaultRelations()) {}
	Conjunction(const Conjunction& a)
		: left_(a.left_), right_(a.right_), rels_(a.rels_) {};
	virtual ~Conjunction() {};

	Conjunction& operator=(const Conjunction& b) {
		left_ = b.left_;
		right_ = b.right_;
		rels_ = b.rels_;

		return *this;
	}

	boost::shared_ptr<Sentence> left() {return left_;};
	boost::shared_ptr<const Sentence> left() const {return left_;};
	boost::shared_ptr<Sentence> right() {return right_;};
	boost::shared_ptr<const Sentence> right() const {return right_;};

	const std::set<Interval::INTERVAL_RELATION>& relations() const {return rels_;};

	virtual void visit(SentenceVisitor& v) const {
		left_->visit(v);
		right_->visit(v);

		v.accept(*this);
	}
private:

	boost::shared_ptr<Sentence>  left_;
	boost::shared_ptr<Sentence> right_;
	std::set<Interval::INTERVAL_RELATION> rels_;

	virtual Sentence* doClone() const { return new Conjunction(*this); };
	virtual bool doEquals(const Sentence& s) const {
		const Conjunction *con = dynamic_cast<const Conjunction*>(&s);
		if (con == NULL) {
			return false;
		}
		return (*left_ == *(con->left_) && *right_ == *(con->right_) && rels_ == con->rels_);
	};

	virtual void doToString(std::string& str) const {
		if (left_->precedence() > precedence()) {
			str += "(";
			str += left_->toString();
			str += ")";
		} else {
			str += left_->toString();
		}
		if (rels_ == defaultRelations()) {
			str += " ^ ";
		} else if (rels_.size() == 1 && rels_.find(Interval::MEETS) != rels_.end()) {
			str += " ; ";
		} else {
			str += " ^{";
			if (!rels_.empty())	{	// better be safe, not sure if this could happen but still...
				std::set<Interval::INTERVAL_RELATION>::const_iterator it = rels_.begin();
				str += Interval::relationToString(*it);
				it++;
				for (; it!= rels_.end(); it++) {
					str += ", ";
					str += Interval::relationToString(*it);
				}
			}

			str += "} ";
		}

		if (right_->precedence() > precedence()) {
			str += "(";
			str += right_->toString();
			str += ")";
		} else {
			str += right_->toString();
		}
	};

	virtual int doPrecedence() const { return 3; };


};

#endif
