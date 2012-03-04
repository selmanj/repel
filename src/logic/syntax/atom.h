#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include "term.h"
#include "sentence.h"
#include "sentencevisitor.h"
#include "constant.h"

class Domain;
class Model;

class Atom : public Sentence {
public:
    typedef boost::ptr_vector<Term>::size_type size_type;

    static const std::size_t TypeCode = 0;

    Atom(std::string name);
    template <class AutoPtrIterator>
    Atom(std::string name, AutoPtrIterator first, AutoPtrIterator last);
    Atom(std::string name, std::auto_ptr<Term> ptr);
    Atom(const Atom& a);    // shallow copy

    bool isGrounded() const;

    int arity() const;
    std::string name() const;
    std::string& name();

    Atom& operator=(const Atom& b);

    Term& at(size_type n);
    const Term& at(size_type n) const;

    void push_back(std::auto_ptr<Term> t);
    virtual void visit(SentenceVisitor& v) const;

    friend std::size_t hash_value(const Atom& a);
    friend bool operator==(const Atom& l, const Atom& r);
    friend bool operator!=(const Atom& l, const Atom& r);
    friend std::ostream& operator<<(std::ostream& out, const Atom& a);

    virtual std::size_t getTypeCode() const;
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:
    std::string pred;
    boost::ptr_vector<Term> terms;
    //std::vector<boost::shared_ptr<Term> > terms;

    virtual Sentence* doClone() const;
    virtual bool doEquals(const Sentence& t) const;
    virtual void doToString(std::stringstream& str) const;
    virtual int doPrecedence() const;
    virtual bool doContains(const Sentence& s) const;
    virtual std::size_t doHashValue() const;
};

struct atomcmp {
    bool operator()(const Atom& a, const Atom& b) const {
        return a.toString() < b.toString();
    }
};

/**
 * Simple container class for propositions (literals) without a temporal quantifier.
 */
struct Proposition {
    Proposition(const Atom& a, bool s)
        : atom(a), sign(s) {}

    Proposition inverse() const { return Proposition(atom, !sign);}
    friend std::size_t hash_value(const Proposition& p) {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.atom);
        boost::hash_combine(seed, p.sign);
        return seed;
    }
    friend bool operator==(const Proposition& l, const Proposition& r) { return l.sign == r.sign && l.atom == r.atom; }
    friend bool operator!=(const Proposition& l, const Proposition& r) { return !operator==(l,r);}
    Atom atom;
    bool sign;  // if positive, true, if negative, false
};
// IMPLEMENTATION

inline Atom::Atom(std::string name)
  : pred(name), terms() {};
template <class AutoPtrIterator>
Atom::Atom(std::string name, AutoPtrIterator first, AutoPtrIterator last)
  : pred(name), terms(first, last) {};
inline Atom::Atom(std::string name, std::auto_ptr<Term> ptr)
  : pred(name), terms() { terms.push_back(ptr); }
inline Atom::Atom(const Atom& a)
  : pred(a.pred), terms(a.terms) {};    // shallow copy

inline int Atom::arity() const {return terms.size();};
inline std::string Atom::name() const {return pred;};
inline std::string& Atom::name() {return pred;};

inline Atom& Atom::operator=(const Atom& b) {
    if (this != &b) {
        pred = b.pred;
        terms = b.terms;
    }
    return *this;
}
// TODO make the at() function throw an exception
inline Term& Atom::at(size_type n) {return terms[n];};
inline const Term& Atom::at(size_type n) const {return terms[n];};

inline void Atom::push_back(std::auto_ptr<Term> t)  {terms.push_back(t);};

inline std::size_t hash_value(const Atom& a) {
    std::size_t seed = Atom::TypeCode;
    boost::hash_combine(seed, a.pred);
    boost::hash_range(seed, a.terms.begin(), a.terms.end());
    return seed;
}

inline bool operator==(const Atom& l, const Atom& r) {return (l.pred == r.pred && l.terms == r.terms);}
inline bool operator!=(const Atom& l, const Atom& r) {return !operator==(l, r);}

inline void Atom::visit(SentenceVisitor& v) const {
    v.accept(*this);
}


inline Sentence* Atom::doClone() const {return new Atom(*this);};
inline std::size_t Atom::doHashValue() const {return hash_value(*this);}
inline int Atom::doPrecedence() const {return 0;};
inline bool Atom::doContains(const Sentence& s) const {return *this == s;};
inline std::size_t Atom::getTypeCode() const {return Atom::TypeCode;}


#endif
