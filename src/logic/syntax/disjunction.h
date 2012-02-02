#ifndef DISJUNCTION_H
#define CONJUNCTION_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"
#include "sentencevisitor.h"

class Disjunction : public Sentence {
public:
    Disjunction(const boost::shared_ptr<Sentence>& left, const boost::shared_ptr<Sentence>& right);
    Disjunction(const Disjunction& a);
    virtual ~Disjunction();

    Disjunction& operator=(const Disjunction& b);

    boost::shared_ptr<Sentence>& left();
    boost::shared_ptr<const Sentence> left() const;
    boost::shared_ptr<Sentence>& right();
    boost::shared_ptr<const Sentence> right() const;

private:
    boost::shared_ptr<Sentence> left_;
    boost::shared_ptr<Sentence> right_;

    virtual Sentence* doClone() const;

    virtual bool doEquals(const Sentence& s) const;
    virtual void doToString(std::stringstream& str) const;
    virtual void visit(SentenceVisitor& v) const;
    virtual int doPrecedence() const;
    virtual bool doContains(const Sentence& s) const;

};

// IMPLEMENTATION

inline Disjunction::Disjunction(const boost::shared_ptr<Sentence>& left, const boost::shared_ptr<Sentence>& right)
    : left_(left), right_(right) {};
inline Disjunction::Disjunction(const Disjunction& a) : left_(a.left_), right_(a.right_) {};    // shallow copy
inline Disjunction::~Disjunction() {};

inline Disjunction& Disjunction::operator=(const Disjunction& b) {
    left_ = b.left_;
    right_ = b.right_;

    return *this;
}

inline boost::shared_ptr<Sentence>& Disjunction::left() {return left_;};
inline boost::shared_ptr<const Sentence> Disjunction::left() const {return left_;};
inline boost::shared_ptr<Sentence>& Disjunction::right() {return right_;};
inline boost::shared_ptr<const Sentence> Disjunction::right() const {return right_;};

// private members
inline Sentence* Disjunction::doClone() const { return new Disjunction(*this); };

inline bool Disjunction::doEquals(const Sentence& s) const {
    const Disjunction *con = dynamic_cast<const Disjunction*>(&s);
    if (con == NULL) {
        return false;
    }
    return (*left_ == *(con->left_) && *right_ == *(con->right_));
};

inline int Disjunction::doPrecedence() const { return 4; };
inline void Disjunction::visit(SentenceVisitor& v) const {
    left_->visit(v);
    right_->visit(v);

    v.accept(*this);
}

inline bool Disjunction::doContains(const Sentence& s) const {
    if (*this == s) return true;
    return (left_->contains(s) || right_->contains(s));
}

#endif
