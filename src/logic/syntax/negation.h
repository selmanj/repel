#ifndef NEGATION_H
#define NEGATION_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"
#include "sentencevisitor.h"

class Negation : public Sentence {
public:
	Negation(boost::shared_ptr<Sentence> sentence);
	Negation(const Negation& neg); // shallow copy
	virtual ~Negation();

	Negation& operator=(const Negation& n);

	boost::shared_ptr<Sentence>& sentence();
	boost::shared_ptr<const Sentence> sentence() const;
private:
	boost::shared_ptr<Sentence> s_;

	virtual Sentence* doClone() const;
	virtual bool doEquals(const Sentence& s) const;
	virtual void doToString(std::stringstream& str) const;
	virtual int doPrecedence() const;
	virtual void visit(SentenceVisitor& v) const;
	virtual bool doContains(const Sentence& s) const;
};

// IMPLEMENTATION

inline Negation::Negation(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
inline Negation::Negation(const Negation& neg) : s_(neg.s_) {}; // shallow copy
inline Negation::~Negation() {};

inline Negation& Negation::operator=(const Negation& n) {
    s_ = n.s_;
    return *this;
}

inline boost::shared_ptr<Sentence>& Negation::sentence() {return s_;}
inline boost::shared_ptr<const Sentence> Negation::sentence() const {return s_;}

// private members
inline Sentence* Negation::doClone() const { return new Negation(*this); }
inline bool Negation::doEquals(const Sentence& s) const {
    const Negation *neg = dynamic_cast<const Negation*>(&s);
    if (neg == NULL) {
        return false;
    }
    return *s_ == *(neg->s_);
}

inline int Negation::doPrecedence() const { return 2; };
inline void Negation::visit(SentenceVisitor& v) const {
    s_->visit(v);

    v.accept(*this);
}

inline bool Negation::doContains(const Sentence& s) const {
    return (*this == s || s_->contains(s));
}

#endif
