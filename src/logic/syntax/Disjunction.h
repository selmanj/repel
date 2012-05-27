#ifndef DISJUNCTION_H
#define DISJUNCTION_H

#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "Sentence.h"
#include "SentenceVisitor.h"

class Domain;
class Model;

class Disjunction : public Sentence {
public:
    static const std::size_t TypeCode = 5;

    Disjunction();
    Disjunction(boost::shared_ptr<Sentence> left, boost::shared_ptr<Sentence> right);
    Disjunction(const Disjunction& a);
    virtual ~Disjunction();

    friend void swap(Disjunction& l, Disjunction& r);
    Disjunction& operator=(Disjunction b);

    boost::shared_ptr<Sentence> left();
    boost::shared_ptr<const Sentence> left() const;
    boost::shared_ptr<Sentence> right();
    boost::shared_ptr<const Sentence> right() const;

    void setLeft(boost::shared_ptr<Sentence> s);
    void setRight(boost::shared_ptr<Sentence> s);

    virtual std::size_t getTypeCode() const;
    friend std::size_t hash_value(const Disjunction& d);
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    boost::shared_ptr<Sentence> left_;
    boost::shared_ptr<Sentence> right_;

    virtual Sentence* doClone() const;

    virtual bool doEquals(const Sentence& s) const;
    virtual void doToString(std::stringstream& str) const;
    virtual void visit(SentenceVisitor& v) const;
    virtual int doPrecedence() const;
    virtual bool doContains(const Sentence& s) const;
    virtual std::size_t doHashValue() const;
};


// IMPLEMENTATION
inline Disjunction::Disjunction()
    : left_(), right_() {};
inline Disjunction::Disjunction(boost::shared_ptr<Sentence> left, boost::shared_ptr<Sentence> right)
    : left_(left), right_(right) {};
inline Disjunction::Disjunction(const Disjunction& a) : left_(a.left_), right_(a.right_) {};    // shallow copy
inline Disjunction::~Disjunction() {};

inline void swap(Disjunction& l, Disjunction& r) {
    using std::swap;
    swap(l.left_, r.left_);
    swap(l.right_, r.right_);
}

inline Disjunction& Disjunction::operator=(Disjunction b) {
    swap(*this, b);
    return *this;
}
inline std::size_t Disjunction::getTypeCode() const { return Disjunction::TypeCode;}

inline std::size_t hash_value(const Disjunction& d) {
    std::size_t seed = Disjunction::TypeCode;
    boost::hash_combine(seed, *d.left_);
    boost::hash_combine(seed, *d.right_);
    return seed;
}

inline std::size_t Disjunction::doHashValue() const { return hash_value(*this);}

inline boost::shared_ptr<Sentence> Disjunction::left() {return left_;}
inline boost::shared_ptr<const Sentence> Disjunction::left() const {return left_;}
inline boost::shared_ptr<Sentence> Disjunction::right() {return right_;}
inline boost::shared_ptr<const Sentence> Disjunction::right() const {return right_;}

inline void Disjunction::setLeft(boost::shared_ptr<Sentence> s) {left_ = s;}
inline void Disjunction::setRight(boost::shared_ptr<Sentence> s) {right_ = s;}
// private members
inline Sentence* Disjunction::doClone() const { return new Disjunction(*this); }

inline bool Disjunction::doEquals(const Sentence& s) const {
    const Disjunction *con = dynamic_cast<const Disjunction*>(&s);
    if (con == NULL) {
        return false;
    }
    return (*left_ == *(con->left_) && *right_ == *(con->right_));
}

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

template <class Archive>
void Disjunction::serialize(Archive& ar, const unsigned int version) {
    // register that there is no need to call the base class serialize
    boost::serialization::void_cast_register<Disjunction,Sentence>(
            static_cast<Disjunction*>(NULL),
            static_cast<Sentence*>(NULL)
    );

    ar & left_;
    ar & right_;
}


#endif
