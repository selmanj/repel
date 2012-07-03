/*
 * Domain.h
 *
 *  Created on: May 20, 2011
 *      Author: joe
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <set>
#include <string>
#include <map>
#include <utility>
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include "../util/boost_serialize_unordered_map.hpp"
#include "../util/boost_serialize_unordered_set.hpp"
#include "ELSyntax.h"
#include "Collectors.h"
#include "Model.h"
#include "../SISet.h"
#include "NameGenerator.h"
#include "../LRUCache.h"
#include "../util/Utils.h"

std::string modelToString(const Model& m);

class Domain {
public:
    typedef boost::unordered_map<Proposition, SISet> PropMap;

    typedef std::vector<ELSentence>::const_iterator     formula_const_iterator;
    typedef PropMap::const_iterator                     fact_const_iterator;
    typedef boost::unordered_set<Atom>::const_iterator  atom_const_iterator;

    Domain();
    Domain(const Domain& d);

    friend void swap(Domain& a, Domain& b);

    Domain& operator=(Domain d);

    formula_const_iterator formulas_begin() const;
    formula_const_iterator formulas_end() const;
    fact_const_iterator facts_begin() const;
    fact_const_iterator facts_end() const;
    atom_const_iterator atoms_begin() const;
    atom_const_iterator atoms_end() const;

    std::size_t atoms_size() const;
    std::size_t formulas_size() const;

    void clearFormulas();
    void clearFacts();
    void addFormula(const ELSentence& e);
    template <class InputIterator>
    void addFormulas(InputIterator begin, InputIterator end);
    void addFact(const ELSentence& e);
    void addFact(const std::pair<const Proposition, const SISet>& pair);
    void addFact(const Proposition& p, const SISet& where);

    void addAtom(const Atom& a);
    template <class InputIterator>
    void addAtoms(InputIterator begin, InputIterator end);

    bool hasFact(const Proposition& p) const;
    SISet lookupFact(const Proposition& p) const;

    SISet getModifiableSISet(const Atom& a) const;
    SISet getModifiableSISet(const Atom& a, const SISet& where) const;

    /**
     * Get a copy of this domain with all infinitely-weighted formulas
     * expressed as high weights (i.e. make hard constraints pseudo-hard).
     *
     * @return a copy of the domain with infinite weights replaced with
     *   finite ones.
     */
    Domain replaceInfForms() const;

    NameGenerator& nameGenerator();
    Model defaultModel() const;
    Model randomModel(boost::mt19937& rng) const;
    Interval maxInterval() const;
    SpanInterval maxSpanInterval() const;
    void setMaxInterval(const Interval& maxInterval);

    bool isLiquid(const std::string& predicate) const;
    bool dontModifyObsPreds() const;
    void setDontModifyObsPreds(bool b);

    double score(const ELSentence& s, const Model& m) const;
    double score(const Model& m) const;

    bool isFullySatisfied(const Model& m) const;

    void printDebugDescription(std::ostream& out) const;

    friend bool operator==(const Domain& l, const Domain& r);
    friend bool operator!=(const Domain& l, const Domain& r);

    static const unsigned int hardFormulaFactor = 10;
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    void growMaxInterval(const Interval& maxInterval);

    bool dontModifyObsPreds_;
    Interval maxInterval_;
    std::vector<ELSentence> formulas_;
    PropMap partialModel_;
    //boost::unordered_set<PredicateType> predTypes_;
    boost::unordered_set<Atom> allAtoms_;

    NameGenerator generator_;

    //mutable LRUCache<ModelSentencePair,SISet,ModelSentencePair_cmp> cache_;
};

void swap(Domain& a, Domain& b);

// IMPLEMENTATION
inline Domain::Domain()
    : dontModifyObsPreds_(true),
      maxInterval_(),
      formulas_(),
      partialModel_(),
    //  predTypes_(),
      allAtoms_(),
      generator_(){};

inline Domain::Domain(const Domain& d)
    : dontModifyObsPreds_(d.dontModifyObsPreds_),
      maxInterval_(d.maxInterval_),
      formulas_(d.formulas_),
      partialModel_(d.partialModel_),
    //  predTypes_(d.predTypes_),
      allAtoms_(d.allAtoms_),
      generator_(d.generator_) {};

inline Domain& Domain::operator=(Domain d) {
    swap(*this, d);
    return *this;
}


inline Domain::formula_const_iterator Domain::formulas_begin() const {return formulas_.begin();}
inline Domain::formula_const_iterator Domain::formulas_end() const {return formulas_.end();}
inline Domain::fact_const_iterator Domain::facts_begin() const {return partialModel_.begin();}
inline Domain::fact_const_iterator Domain::facts_end() const {return partialModel_.end();}
inline Domain::atom_const_iterator Domain::atoms_begin() const { return allAtoms_.begin();}
inline Domain::atom_const_iterator Domain::atoms_end() const { return allAtoms_.end();}

inline std::size_t Domain::atoms_size() const { return allAtoms_.size();}

inline void Domain::clearFormulas() {
    formulas_.clear();
}

inline void Domain::clearFacts() {
    partialModel_.clear();
}

template <class InputIterator>
void Domain::addFormulas(InputIterator begin, InputIterator end) {
    for (InputIterator it = begin; it != end; it++) {
        addFormula(*it);
    }
}

inline void Domain::addFact(const std::pair<const Proposition, const SISet>& pair) {
    addFact(pair.first, pair.second);
}

template <class InputIterator>
void Domain::addAtoms(InputIterator begin, InputIterator end) {
    for (InputIterator it = begin; it != end; it++) {
        addAtom(*it);
    }
}

inline bool Domain::hasFact(const Proposition& p) const {return partialModel_.count(p) != 0; }
inline SISet Domain::lookupFact(const Proposition& p) const { return partialModel_.at(p);}

inline NameGenerator& Domain::nameGenerator() {return generator_;};
inline Model Domain::defaultModel() const {return Model(partialModel_, maxInterval_);};
inline void Domain::setDontModifyObsPreds(bool b) { dontModifyObsPreds_ = b; }
inline bool Domain::dontModifyObsPreds() const { return dontModifyObsPreds_; }
inline Interval Domain::maxInterval() const {return maxInterval_;};
inline SpanInterval Domain::maxSpanInterval() const {
    return SpanInterval(maxInterval_.start(), maxInterval_.finish(),
            maxInterval_.start(), maxInterval_.finish());
};

template <class Archive>
void Domain::serialize(Archive& ar, const unsigned int version) {
    ar & dontModifyObsPreds_;
    ar & maxInterval_;
    ar & formulas_;
    ar & partialModel_;
   // ar & predTypes_;
    ar & allAtoms_;
    ar & generator_;
}

inline bool operator!=(const Domain& l, const Domain& r) {return !operator==(l, r);}

#endif /* DOMAIN_H_ */
