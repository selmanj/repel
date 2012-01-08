#ifndef CONJUNCTION_H
#define CONJUNCTION_H

#include <set>
#include <utility>
#include <boost/shared_ptr.hpp>
#include "sentence.h"
#include "sentencevisitor.h"
#include "../../interval.h"

class Conjunction : public Sentence {
public:
	template<class InputIterator>
	Conjunction(const boost::shared_ptr<Sentence>& left,
			const boost::shared_ptr<Sentence>& right,
			InputIterator begin,
			InputIterator end,
			const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

	Conjunction(const boost::shared_ptr<Sentence>& left,
			const boost::shared_ptr<Sentence>& right,
			Interval::INTERVAL_RELATION rel,
			const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

	Conjunction(const boost::shared_ptr<Sentence>& left,
			const boost::shared_ptr<Sentence>& right,
			const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

	Conjunction(const Conjunction& a);
	virtual ~Conjunction();

	Conjunction& operator=(const Conjunction& b);

	boost::shared_ptr<Sentence>& left();
	boost::shared_ptr<const Sentence> left() const;
	boost::shared_ptr<Sentence>& right();
	boost::shared_ptr<const Sentence> right() const;
	const std::set<Interval::INTERVAL_RELATION>& relations() const;
	const std::pair<TQConstraints, TQConstraints> tqconstraints() const;

	virtual void visit(SentenceVisitor& v) const;

	static const std::set<Interval::INTERVAL_RELATION>& defaultRelations();
private:

	boost::shared_ptr<Sentence>  left_;
	boost::shared_ptr<Sentence> right_;
	std::set<Interval::INTERVAL_RELATION> rels_;
	std::pair<TQConstraints, TQConstraints> tqconstraints_;

	virtual Sentence* doClone() const;
	virtual bool doEquals(const Sentence& s) const;
	virtual void doToString(std::stringstream& str) const;
	virtual int doPrecedence() const;
};

// IMPLEMENTATION
template<class InputIterator>
Conjunction::Conjunction(const boost::shared_ptr<Sentence>& left,
		const boost::shared_ptr<Sentence>& right,
		InputIterator begin,
		InputIterator end,
		const std::pair<TQConstraints, TQConstraints>* tqconstraints)
	: left_(left), right_(right), rels_(begin, end), tqconstraints_() {
	if (tqconstraints)
		tqconstraints_ = *tqconstraints;
}

inline Conjunction::Conjunction(const boost::shared_ptr<Sentence>& left,
			const boost::shared_ptr<Sentence>& right,
			Interval::INTERVAL_RELATION rel,
			const std::pair<TQConstraints, TQConstraints>* tqconstraints)
		: left_(left), right_(right), rels_(), tqconstraints_() {
	rels_.insert(rel);
	if (tqconstraints) tqconstraints_ = *tqconstraints;
}
inline Conjunction::Conjunction(const boost::shared_ptr<Sentence>& left,
		const boost::shared_ptr<Sentence>& right,
		const std::pair<TQConstraints, TQConstraints>* tqconstraints)
		: left_(left), right_(right), rels_(defaultRelations()), tqconstraints_() {
	if (tqconstraints) tqconstraints_ = *tqconstraints;
}
inline Conjunction::Conjunction(const Conjunction& a)
		: left_(a.left_), right_(a.right_), rels_(a.rels_), tqconstraints_(a.tqconstraints_) {}
inline Conjunction::~Conjunction() {}

inline Conjunction& Conjunction::operator=(const Conjunction& b) {
	if (this != &b) {
		left_ = b.left_;
		right_ = b.right_;
		rels_ = b.rels_;
		tqconstraints_ = b.tqconstraints_;
	}

	return *this;
}

inline boost::shared_ptr<Sentence>& Conjunction::left() {return left_;}
inline boost::shared_ptr<const Sentence> Conjunction::left() const {return left_;}
inline boost::shared_ptr<Sentence>& Conjunction::right() {return right_;}
inline boost::shared_ptr<const Sentence> Conjunction::right() const {return right_;}

inline const std::set<Interval::INTERVAL_RELATION>& Conjunction::relations() const {return rels_;}
inline const std::pair<TQConstraints, TQConstraints> Conjunction::tqconstraints() const {return tqconstraints_;}


// private members
inline Sentence* Conjunction::doClone() const { return new Conjunction(*this); }

inline bool Conjunction::doEquals(const Sentence& s) const {
	const Conjunction *con = dynamic_cast<const Conjunction*>(&s);
	if (con == NULL) {
		return false;
	}
	return (*left_ == *(con->left_)
			&& *right_ == *(con->right_)
			&& rels_ == con->rels_
			&& tqconstraints_ == con->tqconstraints_);
}

inline int Conjunction::doPrecedence() const { return 3; };

#endif
