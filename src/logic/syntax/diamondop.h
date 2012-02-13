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

    friend void swap(DiamondOp& left, DiamondOp& right);
    DiamondOp& operator=(DiamondOp other);

    boost::shared_ptr<Sentence> sentence();
    boost::shared_ptr<const Sentence> sentence() const;
    const std::set<Interval::INTERVAL_RELATION>& relations() const;
    const TQConstraints& tqconstraints() const;

    void setSentence(boost::shared_ptr<Sentence> s);
    template<typename T>
    void setRelations(T begin, T end);
    void setTQConstraints(const TQConstraints& tq);
protected:
    virtual SISet doSatisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:
    std::set<Interval::INTERVAL_RELATION> rels_;
    boost::shared_ptr<Sentence> s_;
    TQConstraints tqconstraints_;

    virtual Sentence* doClone() const;
    virtual bool doEquals(const Sentence& s) const;

    virtual void doToString(std::stringstream& str) const;

    virtual int doPrecedence() const;
    virtual void visit(SentenceVisitor& v) const;
    virtual bool doContains(const Sentence& s) const;
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
inline void swap(DiamondOp& left, DiamondOp& right) {
    using std::swap;
    swap(left.s_, right.s_);
    swap(left.rels_, right.rels_);
}

inline DiamondOp& DiamondOp::operator=(DiamondOp other) {
    swap(*this, other);
    return *this;
}

inline boost::shared_ptr<Sentence> DiamondOp::sentence() {return s_;}
inline boost::shared_ptr<const Sentence> DiamondOp::sentence() const {return s_;}
inline const std::set<Interval::INTERVAL_RELATION>& DiamondOp::relations() const {return rels_;}
inline const TQConstraints& DiamondOp::tqconstraints() const {return tqconstraints_;}

inline void DiamondOp::setSentence(boost::shared_ptr<Sentence> s) {s_ = s;}
template<typename T>
inline void DiamondOp::setRelations(T begin, T end) {rels_.clear(); std::copy(begin, end, std::inserter(rels_, rels_.end()));}
inline void DiamondOp::setTQConstraints(const TQConstraints& tq) {tqconstraints_ = tq;}

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
inline bool DiamondOp::doContains(const Sentence& s) const {
    if (*this == s) return true;
    return (s_->contains(s));
}

inline void DiamondOp::visit(SentenceVisitor& v) const {
    s_->visit(v);

    v.accept(*this);
}
#endif
