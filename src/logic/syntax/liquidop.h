#ifndef LIQUIDOP_H
#define LIQUIDOP_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"

class LiquidOp : public Sentence {
public:
    LiquidOp(boost::shared_ptr<Sentence> sentence);
    LiquidOp(const LiquidOp& neg);
    virtual ~LiquidOp();

    LiquidOp& operator=(const LiquidOp& b);
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

inline LiquidOp::LiquidOp(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
inline LiquidOp::LiquidOp(const LiquidOp& neg) : s_(neg.s_) {}; // shallow copy
inline LiquidOp::~LiquidOp() {};

inline LiquidOp& LiquidOp::operator=(const LiquidOp& b) {
    s_ = b.s_;
    return *this;
};
inline boost::shared_ptr<Sentence>& LiquidOp::sentence() {return s_;};
inline boost::shared_ptr<const Sentence> LiquidOp::sentence() const {return s_;};

// private members

inline Sentence* LiquidOp::doClone() const { return new LiquidOp(*this); }
inline bool LiquidOp::doEquals(const Sentence& s) const {
    const LiquidOp *liq = dynamic_cast<const LiquidOp*>(&s);
    if (liq == NULL) {
        return false;
    }
    return *s_ == *(liq->s_);
}

inline int LiquidOp::doPrecedence() const { return 2; };
inline void LiquidOp::visit(SentenceVisitor& v) const {
    s_->visit(v);

    v.accept(*this);
}

inline bool LiquidOp::doContains(const Sentence& s) const {
    return (*this == s || s_->contains(s));
}

#endif
