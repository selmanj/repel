#ifndef DIAMONDOP_H
#define DIAMONDOP_H

#include <boost/shared_ptr.hpp>
#include <set>
#include "sentence.h"
#include "sentencevisitor.h"

class DiamondOp : public Sentence {
public:
	DiamondOp(boost::shared_ptr<Sentence> sentence)
		: s_(sentence) {
			rels_ = std::set<Interval::INTERVAL_RELATION>(DiamondOp::defaultRelations().begin(), DiamondOp::defaultRelations().end());
	}
	DiamondOp(boost::shared_ptr<Sentence> sentence,
			Interval::INTERVAL_RELATION relation)
		: s_(sentence), rels_() {
		rels_.insert(relation);
	}
	template <class InputIterator>
	DiamondOp(boost::shared_ptr<Sentence> sentence,
			InputIterator begin,
			InputIterator end)
			: s_(sentence), rels_(begin, end) {};
	DiamondOp(const DiamondOp& dia) : s_(dia.s_) , rels_(dia.rels_) {}; // shallow copy
	virtual ~DiamondOp() {};

	boost::shared_ptr<Sentence>& sentence() {return s_;};
	boost::shared_ptr<const Sentence> sentence() const {return s_;};
	const std::set<Interval::INTERVAL_RELATION>& relations() const {return rels_;};

	static const std::set<Interval::INTERVAL_RELATION>& defaultRelations() {
		static std::set<Interval::INTERVAL_RELATION>* defaults = new std::set<Interval::INTERVAL_RELATION>();
		if (defaults->size() == 0) {
			defaults->insert(Interval::STARTS);
			defaults->insert(Interval::STARTSI);
			defaults->insert(Interval::DURING);
			defaults->insert(Interval::DURINGI);
			defaults->insert(Interval::FINISHES);
			defaults->insert(Interval::FINISHESI);
			defaults->insert(Interval::OVERLAPS);
			defaults->insert(Interval::OVERLAPSI);
		}
		return *defaults;
	}
private:
	std::set<Interval::INTERVAL_RELATION> rels_;
	boost::shared_ptr<Sentence> s_;

	virtual Sentence* doClone() const { return new DiamondOp(*this); };
	virtual bool doEquals(const Sentence& s) const {
		const DiamondOp *dia = dynamic_cast<const DiamondOp*>(&s);
		if (dia == NULL) {
			return false;
		}
		return *s_ == *(dia->s_);
	};

	virtual void doToString(std::stringstream& str) const {
		str << "<>";
		if (rels_ != DiamondOp::defaultRelations()) {
			str << "{";
			if (!rels_.empty())	{	// better be safe, not sure if this could happen but still...
				std::set<Interval::INTERVAL_RELATION>::const_iterator it = rels_.begin();
				str << Interval::relationToString(*it);
				it++;
				for (; it!= rels_.end(); it++) {
					str << ", ";
					str << Interval::relationToString(*it);
				}
			}

			str << "}";
		}
		str << " ";

		if (s_ != NULL) {
			// check if we need parenthesis
			if (s_->precedence() > precedence()) {
				str << "(";
				str << s_->toString();
				str << ")";
			} else {
				str << s_->toString();
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
