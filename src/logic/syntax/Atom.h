#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/access.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "Term.h"
#include "Sentence.h"
#include "SentenceVisitor.h"
#include "Constant.h"

class Domain;
class Model;
struct PredicateType;

class Atom : public Sentence {
public:
    typedef boost::ptr_vector<Term>::size_type      size_type;
    typedef boost::ptr_vector<Term>::const_iterator term_const_iterator;

    static const std::size_t TypeCode = 0;

    Atom(std::string name="_UNKNOWN");
    template <class AutoPtrIterator>
    Atom(std::string name, AutoPtrIterator first, AutoPtrIterator last);
    Atom(std::string name, std::auto_ptr<Term> ptr);
    Atom(const Atom& a);    // shallow copy

    bool isGrounded() const;

    Atom::term_const_iterator term_begin() const;
    Atom::term_const_iterator term_end() const;

    int arity() const;
    std::string name() const;
    PredicateType predicateType() const;

    Atom& operator=(const Atom& b);

    Term& at(size_type n);
    const Term& at(size_type n) const;

    void push_back(std::auto_ptr<Term> t);
    void push_back(const Term& t);
    virtual void visit(SentenceVisitor& v) const;

    friend std::size_t hash_value(const Atom& a);
    friend bool operator==(const Atom& l, const Atom& r);
    friend bool operator!=(const Atom& l, const Atom& r);
    friend std::ostream& operator<<(std::ostream& out, const Atom& a);

    virtual std::size_t getTypeCode() const;
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const;
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

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

/*
struct atomcmp {
    bool operator()(const Atom& a, const Atom& b) const {
        return a.toString() < b.toString();
    }
};
*/

struct AtomStringCompare {
    bool operator()(const Atom& a, const Atom& b) const;
};

/**
 * Class describing the type of an atom.  Right now its just the predicate name and
 * number of arguments.
 */
struct PredicateType {
    PredicateType() : name(), arity(0) {};
    PredicateType(const std::string& predname, unsigned int numArgs)
        : name(predname), arity(numArgs) {}

    friend std::size_t hash_value(const PredicateType& t) {
        std::size_t seed = 0;
        boost::hash_combine(seed, t.name);
        boost::hash_combine(seed, t.arity);
        return seed;
    }

    friend bool operator==(const PredicateType& l, const PredicateType& r) {
        return (l.name == r.name) && (l.arity == r.arity);
    }
    friend bool operator!=(const PredicateType& l, const PredicateType& r) {
        return !operator==(l, r);
    }

    std::string name;
    unsigned int arity;
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

inline PredicateType Atom::predicateType() const {return PredicateType(name(), arity());}


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
inline void Atom::push_back(const Term& t)  {terms.push_back(t.clone());};


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

template <class Archive>
void Atom::serialize(Archive& ar, const unsigned int version) {
    // explicitly register that we don't need to serialize the base class
    boost::serialization::void_cast_register<Atom, Sentence>(
            static_cast<Atom *>(NULL),
            static_cast<Sentence *>(NULL)
    );
    ar & pred;
    ar & terms;
}

template void Atom::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void Atom::serialize(boost::archive::text_iarchive & ar, const unsigned int version);

#endif
