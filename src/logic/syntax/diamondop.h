#ifndef DIAMONDOP_H
#define DIAMONDOP_H

#include <boost/shared_ptr.hpp>
#include <set>
#include "sentence.h"
#include "sentencevisitor.h"
#include "../../interval.h"

class DiamondOp : public Sentence {
public:
	static const std::set<Interval::INTERVAL_RELATION>& defaultRelations();

	DiamondOp(boost::shared_ptr<Sentence> sentence, const TQConstraints* tqconstraints=0);
	DiamondOp(boost::shared_ptr<Sentence> sentence,
			Interval::INTERVAL_RELATION relation,
			const TQConstraints* tqconstraints=0);
	template <class InputIterator>
	DiamondOp(boost::shared_ptr<Sentence> sentence,
			InputIterator begin,
			InputIterator end,
			const TQConstraints* tqconstraints=0);
	DiamondOp(const DiamondOp& dia); // shallow copy
	virtual ~DiamondOp();

	boost::shared_ptr<Sentence>& sentence();
	boost::shared_ptr<const Sentence> sentence() const;
	const std::set<Interval::INTERVAL_RELATION>& relations() const;
	const TQConstraints& temporalConstraints() const;

private:
	std::set<Interval::INTERVAL_RELATION> rels_;
	boost::shared_ptr<Sentence> s_;
	TQConstraints tqconstraints_;

	virtual Sentence* doClone() const;
	virtual bool doEquals(const Sentence& s) const;

	virtual void doToString(std::stringstream& str) const;

	virtual int doPrecedence() const;
	virtual void visit(SentenceVisitor& v) const;
};

// implementation below
// constructors
inline DiamondOp::DiamondOp(boost::shared_ptr<Sentence> sentence, const TQConstraints* tqconstraints)
	: rels_(), s_(sentence), tqconstraints_() {
		rels_ = std::set<Interval::INTERVAL_RELATION>(DiamondOp::defaultRelations().begin(), DiamondOp::defaultRelations().end());
		if (tqconstraints) tqconstraints_ = *tqconstraints;
}
inline DiamondOp::DiamondOp(boost::shared_ptr<Sentence> sentence,
		Interval::INTERVAL_RELATION relation,
		const TQConstraints* tqconstraints)
	: rels_(), s_(sentence), tqconstraints_() {
	rels_.insert(relation);
	if (tqconstraints) tqconstraints_ = *tqconstraints;
}
template <class InputIterator>
inline DiamondOp::DiamondOp(boost::shared_ptr<Sentence> sentence,
		InputIterator begin,
		InputIterator end,
		const TQConstraints* tqconstraints)
		: rels_(begin, end), s_(sentence), tqconstraints_() {
	if (tqconstraints) tqconstraints_ = *tqconstraints;
}
// copy constructor
inline DiamondOp::DiamondOp(const DiamondOp& dia)
	: rels_(dia.rels_), s_(dia.s_), tqconstraints_(dia.tqconstraints_) {}; // shallow copy
inline DiamondOp::~DiamondOp() {}

// public methods
inline boost::shared_ptr<Sentence>& DiamondOp::sentence() {return s_;}
inline boost::shared_ptr<const Sentence> DiamondOp::sentence() const {return s_;}
inline const std::set<Interval::INTERVAL_RELATION>& DiamondOp::relations() const {return rels_;}
inline const TQConstraints& DiamondOp::temporalConstraints() const {return tqconstraints_;}


// private methods
inline Sentence* DiamondOp::doClone() const { return new DiamondOp(*this); }
inline bool DiamondOp::doEquals(const Sentence& s) const {
	const DiamondOp *dia = dynamic_cast<const DiamondOp*>(&s);
	if (dia == NULL) {
		return false;
	}
	return *s_ == *(dia->s_);
}
inline int DiamondOp::doPrecedence() const { return 2; };
inline void DiamondOp::visit(SentenceVisitor& v) const {
	s_->visit(v);

	v.accept(*this);
}
#endif
