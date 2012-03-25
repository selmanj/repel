#ifndef CONJUNCTION_H
#define CONJUNCTION_H

#include <set>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include "sentence.h"
#include "sentencevisitor.h"
#include "../../interval.h"

class Domain;
class Model;

class Conjunction : public Sentence {
public:
    static const std::size_t TypeCode = 2;

    // Note, all these constructors take ownership of the passed in ptrs
    template<class InputIterator>
    Conjunction(Sentence* left,
            Sentence* right,
            InputIterator begin,
            InputIterator end,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

    Conjunction(Sentence* left,
            Sentence* right,
            Interval::INTERVAL_RELATION rel,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

    Conjunction(Sentence* left,
            Sentence* right,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

    Conjunction(const Conjunction& a);
    virtual ~Conjunction();

    friend void swap(Conjunction& left, Conjunction& right);
    Conjunction& operator=(Conjunction b);

    virtual std::size_t getTypeCode() const;
    friend std::size_t hash_value(const Conjunction& c);

    const Sentence* left() const;
    const Sentence* right() const;
    const std::set<Interval::INTERVAL_RELATION>& relations() const;
    std::pair<const TQConstraints, const TQConstraints> tqconstraints() const;

    void setLeft(Sentence *left);
    void setRight(Sentence *right);
    template<typename T>
    void setRelations(T begin, T end);
    void setTQConstraints(const std::pair<TQConstraints, TQConstraints>& tq);

    virtual void visit(SentenceVisitor& v) const;

    static const std::set<Interval::INTERVAL_RELATION>& defaultRelations();
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:

    Sentence *left_;
    Sentence *right_;
    std::set<Interval::INTERVAL_RELATION> rels_;
    std::pair<TQConstraints, TQConstraints> tqconstraints_;

    virtual Sentence* doClone() const;
    virtual bool doEquals(const Sentence& s) const;
    virtual void doToString(std::stringstream& str) const;
    virtual int doPrecedence() const;
    virtual bool doContains(const Sentence& s) const;
    virtual std::size_t doHashValue() const;
};

// IMPLEMENTATION
template<class InputIterator>
Conjunction::Conjunction(Sentence *left,
        Sentence *right,
        InputIterator begin,
        InputIterator end,
        const std::pair<TQConstraints, TQConstraints>* tqconstraints)
    : left_(left), right_(right), rels_(begin, end), tqconstraints_() {
    if (tqconstraints)
        tqconstraints_ = *tqconstraints;
}


inline void swap(Conjunction& left, Conjunction& right) {
    using std::swap;
    swap(left.left_, right.left_);
    swap(left.right_, right.right_);
    swap(left.rels_, right.rels_);
    swap(left.tqconstraints_, right.tqconstraints_);
}

inline Conjunction& Conjunction::operator=(Conjunction b) {
    swap(*this, b);
    return *this;
}

inline std::size_t Conjunction::getTypeCode() const {
    return Conjunction::TypeCode;
}

inline std::size_t hash_value(const Conjunction& c) {
    std::size_t seed = Conjunction::TypeCode;

    if (c.left_) boost::hash_combine(seed, *c.left_);
    if (c.right_) boost::hash_combine(seed, *c.right_);
    boost::hash_range(seed, c.rels_.begin(), c.rels_.end());
    // TODO: we can't currently hash SISets, so we just ignore TQConstraints
    // This is ok, but causes conflicts and reduces performance.
    return seed;
}


inline const Sentence* Conjunction::left() const {return left_;}
inline const Sentence* Conjunction::right() const {return right_;}

inline const std::set<Interval::INTERVAL_RELATION>& Conjunction::relations() const {return rels_;}
inline std::pair<const TQConstraints, const TQConstraints> Conjunction::tqconstraints() const {return tqconstraints_;}


template<typename T>
inline void Conjunction::setRelations(T begin, T end) {
    rels_.clear();
    std::copy(begin, end, std::inserter(rels_, rels_.end()));
}
inline void Conjunction::setTQConstraints(const std::pair<TQConstraints, TQConstraints>& tq) {
    tqconstraints_ = tq;
}
// private members
inline Sentence* Conjunction::doClone() const { return new Conjunction(*this); }
inline std::size_t Conjunction::doHashValue() const {return hash_value(*this);}


inline bool Conjunction::doEquals(const Sentence& s) const {
    if (s.getTypeCode() != Conjunction::TypeCode) return false;
    const Conjunction &con = static_cast<const Conjunction&>(s);

    if ((left_ == 0 && con.left_ != 0)
            || (left_  != 0 && con.left_  == 0)) return false;
    if ((right_ == 0 && con.right_ != 0)
            || (right_ != 0 && con.right_ == 0)) return false;
    // now either left,right pairs are both 0 or both valid
    return ((!left_ && !con.left_ ? true : *left_ == *(con.left_))
            && (!right_ && !con.right_ ? true : *right_ == *(con.right_))
            && rels_ == con.rels_
            && tqconstraints_ == con.tqconstraints_);
}

inline int Conjunction::doPrecedence() const { return 3; };
inline bool Conjunction::doContains(const Sentence& s) const {
    if (*this == s) return true;
    return ((left_ ? left_->contains(s) : false) || (right_ ? right_->contains(s) : false));
}


#endif
