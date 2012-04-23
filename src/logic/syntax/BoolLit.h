/*
 * boollit.h
 *
 *  Created on: Jun 8, 2011
 *      Author: joe
 */

#ifndef BOOLLIT_H_
#define BOOLLIT_H_

#include "sentence.h"
#include "boost/functional/hash.hpp"

class BoolLit: public Sentence {
public:
    static const std::size_t TypeCode = 1;

    BoolLit(bool value);
    BoolLit(const BoolLit& other);
    virtual ~BoolLit();

    friend void swap(BoolLit& left, BoolLit& right);
    BoolLit& operator=(BoolLit other);

    bool value() const;
    void setValue(bool val);
    virtual void visit(SentenceVisitor& s) const;

    friend std::size_t hash_value(const BoolLit& b);
    virtual std::size_t getTypeCode() const;
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:
    bool val_;

    virtual void doToString(std::stringstream& str) const;
    virtual Sentence* doClone() const;
    virtual bool doEquals(const Sentence& t) const;
    virtual int doPrecedence() const;
    virtual bool doContains(const Sentence& s) const;
    virtual std::size_t doHashValue() const;
};

// IMPLEMENTATION
inline BoolLit::BoolLit(bool value) : val_(value) {}
inline BoolLit::BoolLit(const BoolLit& other) : val_(other.val_) {}
inline BoolLit::~BoolLit() {}

inline void swap(BoolLit& left, BoolLit& right) {
    using std::swap;
    swap(left.val_, right.val_);
}
inline BoolLit& BoolLit::operator=(BoolLit other) {swap(*this, other); return *this;}

inline bool BoolLit::value() const {return val_;};
inline void BoolLit::setValue(bool val) {val_ = val;};

inline std::size_t hash_value(const BoolLit& b) {
    std::size_t seed = BoolLit::TypeCode;
    boost::hash_combine(seed, b.val_);
    return seed;

}
inline std::size_t BoolLit::doHashValue() const { return hash_value(*this);}
inline void BoolLit::visit(SentenceVisitor& s) const {s.accept(*this);}

// private members

inline void BoolLit::doToString(std::stringstream& str) const {
     (val_) ? str << "true" : str << "false";
}
inline Sentence* BoolLit::doClone() const { return new BoolLit(*this);};
inline int BoolLit::doPrecedence() const { return 0;}
inline bool BoolLit::doContains(const Sentence& s) const {return *this == s;}
inline std::size_t BoolLit::getTypeCode() const { return BoolLit::TypeCode;}

#endif /* BOOLLIT_H_ */
