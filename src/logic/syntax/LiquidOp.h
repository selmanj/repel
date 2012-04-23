#ifndef LIQUIDOP_H
#define LIQUIDOP_H

#include <boost/shared_ptr.hpp>
#include "Sentence.h"

class Model;
class Domain;

class LiquidOp : public Sentence {
public:
    static const std::size_t TypeCode=7;

    LiquidOp(boost::shared_ptr<Sentence> sentence);
    LiquidOp(const LiquidOp& liq);
    virtual ~LiquidOp();

    friend void swap(LiquidOp& a, LiquidOp& b);
    LiquidOp& operator=(LiquidOp b);

    boost::shared_ptr<Sentence> sentence();
    boost::shared_ptr<const Sentence> sentence() const;

    void setSentence(boost::shared_ptr<Sentence> s);
    virtual std::size_t getTypeCode() const;
    friend std::size_t hash_value(const LiquidOp& l);
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:
    boost::shared_ptr<Sentence> s_;

    virtual Sentence* doClone() const;
    virtual bool doEquals(const Sentence& s) const;
    virtual void doToString(std::stringstream& str) const;
    virtual int doPrecedence() const;

    virtual void visit(SentenceVisitor& v) const;
    virtual bool doContains(const Sentence& s) const;
    virtual std::size_t doHashValue() const;
};

// IMPLEMENTATION

inline LiquidOp::LiquidOp(boost::shared_ptr<Sentence> sentence) : s_(sentence) {};
inline LiquidOp::LiquidOp(const LiquidOp& liq) : s_(liq.s_) {}; // shallow copy
inline LiquidOp::~LiquidOp() {};

inline void swap(LiquidOp& a, LiquidOp& b) {
    using std::swap;
    swap(a.s_, b.s_);
}

inline LiquidOp& LiquidOp::operator=(LiquidOp b) {
    swap(*this, b);
    return *this;
};
inline boost::shared_ptr<Sentence> LiquidOp::sentence() {return s_;};
inline boost::shared_ptr<const Sentence> LiquidOp::sentence() const {return s_;};

inline void LiquidOp::setSentence(boost::shared_ptr<Sentence> s) {s_ = s;}

inline std::size_t LiquidOp::getTypeCode() const { return LiquidOp::TypeCode;}
inline std::size_t hash_value(const LiquidOp& l) {
    std::size_t seed = LiquidOp::TypeCode;
    boost::hash_combine(seed, *l.s_);
    return seed;
}
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
inline std::size_t LiquidOp::doHashValue() const { return hash_value(*this);}
#endif
