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
    Conjunction(boost::shared_ptr<Sentence> left,
            boost::shared_ptr<Sentence> right,
            InputIterator begin,
            InputIterator end,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

    Conjunction(boost::shared_ptr<Sentence> left,
            boost::shared_ptr<Sentence> right,
            Interval::INTERVAL_RELATION rel,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

    Conjunction(boost::shared_ptr<Sentence> left,
            boost::shared_ptr<Sentence> right,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints=0);

    Conjunction(const Conjunction& a);
    virtual ~Conjunction();

    friend void swap(Conjunction& left, Conjunction& right);
    Conjunction& operator=(Conjunction b);

    boost::shared_ptr<Sentence> left();
    boost::shared_ptr<const Sentence> left() const;
    boost::shared_ptr<Sentence> right();
    boost::shared_ptr<const Sentence> right() const;
    std::set<Interval::INTERVAL_RELATION> relations();
    const std::set<Interval::INTERVAL_RELATION>& relations() const;
    std::pair<TQConstraints, TQConstraints> tqconstraints();
    std::pair<const TQConstraints, const TQConstraints> tqconstraints() const;

    void setLeft(boost::shared_ptr<Sentence> s);
    void setRight(boost::shared_ptr<Sentence> s);
    template<typename T>
    void setRelations(T begin, T end);
    void setTQConstraints(const std::pair<TQConstraints, TQConstraints>& tq);

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
    virtual bool doContains(const Sentence& s) const;
};

// IMPLEMENTATION
template<class InputIterator>
Conjunction::Conjunction(boost::shared_ptr<Sentence> left,
        boost::shared_ptr<Sentence> right,
        InputIterator begin,
        InputIterator end,
        const std::pair<TQConstraints, TQConstraints>* tqconstraints)
    : left_(left), right_(right), rels_(begin, end), tqconstraints_() {
    if (tqconstraints)
        tqconstraints_ = *tqconstraints;
}

inline Conjunction::Conjunction(boost::shared_ptr<Sentence> left,
            boost::shared_ptr<Sentence> right,
            Interval::INTERVAL_RELATION rel,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints)
        : left_(left), right_(right), rels_(), tqconstraints_() {
    rels_.insert(rel);
    if (tqconstraints) tqconstraints_ = *tqconstraints;
}
inline Conjunction::Conjunction(boost::shared_ptr<Sentence> left,
        boost::shared_ptr<Sentence> right,
        const std::pair<TQConstraints, TQConstraints>* tqconstraints)
        : left_(left), right_(right), rels_(defaultRelations()), tqconstraints_() {
    if (tqconstraints) tqconstraints_ = *tqconstraints;
}
inline Conjunction::Conjunction(const Conjunction& a)
        : left_(a.left_), right_(a.right_), rels_(a.rels_), tqconstraints_(a.tqconstraints_) {}
inline Conjunction::~Conjunction() {}

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

inline boost::shared_ptr<Sentence> Conjunction::left() {return left_;}
inline boost::shared_ptr<const Sentence> Conjunction::left() const {return left_;}
inline boost::shared_ptr<Sentence> Conjunction::right() {return right_;}
inline boost::shared_ptr<const Sentence> Conjunction::right() const {return right_;}

inline std::set<Interval::INTERVAL_RELATION> Conjunction::relations() {return rels_;}
inline const std::set<Interval::INTERVAL_RELATION>& Conjunction::relations() const {return rels_;}
inline std::pair<TQConstraints, TQConstraints> Conjunction::tqconstraints() {return tqconstraints_;}
inline std::pair<const TQConstraints, const TQConstraints> Conjunction::tqconstraints() const {return tqconstraints_;}

inline void Conjunction::setLeft(boost::shared_ptr<Sentence> s) {left_ = s;}
inline void Conjunction::setRight(boost::shared_ptr<Sentence> s) {right_ = s;}
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
inline bool Conjunction::doContains(const Sentence& s) const {
    if (*this == s) return true;
    return (left_->contains(s) || right_->contains(s));
}


#endif
