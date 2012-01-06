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

	DiamondOp(boost::shared_ptr<Sentence> sentence);
	DiamondOp(boost::shared_ptr<Sentence> sentence,
			Interval::INTERVAL_RELATION relation);
	template <class InputIterator>
	DiamondOp(boost::shared_ptr<Sentence> sentence,
			InputIterator begin,
			InputIterator end);
	DiamondOp(const DiamondOp& dia); // shallow copy
	virtual ~DiamondOp();

	boost::shared_ptr<Sentence>& sentence();
	boost::shared_ptr<const Sentence> sentence() const;
	const std::set<Interval::INTERVAL_RELATION>& relations() const;

private:
	std::set<Interval::INTERVAL_RELATION> rels_;
	boost::shared_ptr<Sentence> s_;

	virtual Sentence* doClone() const;
	virtual bool doEquals(const Sentence& s) const;

	virtual void doToString(std::stringstream& str) const;

	virtual int doPrecedence() const;
	virtual void visit(SentenceVisitor& v) const;
};

// implementation below
// constructors
inline DiamondOp::DiamondOp(boost::shared_ptr<Sentence> sentence)
	: rels_(), s_(sentence) {
		rels_ = std::set<Interval::INTERVAL_RELATION>(DiamondOp::defaultRelations().begin(), DiamondOp::defaultRelations().end());
}
inline DiamondOp::DiamondOp(boost::shared_ptr<Sentence> sentence,
		Interval::INTERVAL_RELATION relation)
	: rels_(), s_(sentence) {
	rels_.insert(relation);
}
template <class InputIterator>
inline DiamondOp::DiamondOp(boost::shared_ptr<Sentence> sentence,
		InputIterator begin,
		InputIterator end)
		: rels_(begin, end), s_(sentence) {}
// copy constructor
inline DiamondOp::DiamondOp(const DiamondOp& dia)
	: rels_(dia.rels_), s_(dia.s_) {}; // shallow copy
inline DiamondOp::~DiamondOp() {}

// public methods
inline boost::shared_ptr<Sentence>& DiamondOp::sentence() {return s_;}
inline boost::shared_ptr<const Sentence> DiamondOp::sentence() const {return s_;}
inline const std::set<Interval::INTERVAL_RELATION>& DiamondOp::relations() const {return rels_;}

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
