/*
 * domain.h
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
#include "el_syntax.h"
#include "collectors.h"
#include "model.h"
#include "../siset.h"
#include "namegenerator.h"
#include "../lrucache.h"
#include "../utils.h"

std::string modelToString(const Model& m);

class Domain {
public:
    typedef boost::unordered_map<Proposition, SISet> PropMap;

    typedef std::vector<ELSentence>::const_iterator     formula_const_iterator;
    typedef PropMap::const_iterator                     fact_const_iterator;
    typedef boost::unordered_set<Atom>::const_iterator  atom_const_iterator;

    Domain();

    formula_const_iterator formulas_begin() const;
    formula_const_iterator formulas_end() const;
    fact_const_iterator facts_begin() const;
    fact_const_iterator facts_end() const;
    atom_const_iterator atoms_begin() const;
    atom_const_iterator atoms_end() const;

    std::size_t atoms_size() const;

    void clearFormulas();
    void clearFacts();
    void addFormula(const ELSentence& e);
    template <class InputIterator>
    void addFormulas(InputIterator begin, InputIterator end);
    void addFact(const ELSentence& e);
    void addFact(const Proposition& p, const SISet& where);

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
    Model randomModel() const;
    Interval maxInterval() const;
    SpanInterval maxSpanInterval() const;
    void setMaxInterval(const Interval& maxInterval);

    bool isLiquid(const std::string& predicate) const;
    bool dontModifyObsPreds() const;
    void setDontModifyObsPreds(bool b);

    score_t score(const ELSentence& s, const Model& m) const;
    score_t score(const Model& m) const;

    bool isFullySatisfied(const Model& m) const;

    static const unsigned int hardFormulaFactor = 10;
private:

    void growMaxInterval(const Interval& maxInterval);

    bool dontModifyObsPreds_;
    Interval maxInterval_;
    std::vector<ELSentence> formulas_;
    PropMap partialModel_;
    boost::unordered_set<PredicateType> predTypes_;
    boost::unordered_set<Atom> allAtoms_;

    NameGenerator generator_;

    //mutable LRUCache<ModelSentencePair,SISet,ModelSentencePair_cmp> cache_;
};

// IMPLEMENTATION
inline Domain::Domain()
    : dontModifyObsPreds_(true),
      maxInterval_(),
      formulas_(),
      partialModel_(),
      predTypes_(),
      allAtoms_(),
      generator_(){};

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

inline void Domain::addFact(const ELSentence& e) {
    if (!e.hasInfWeight()) throw std::invalid_argument("Cannot enforce facts that have finite weight");
    if (e.sentence()->getTypeCode() == Atom::TypeCode) {
        boost::shared_ptr<const Atom> atom = boost::static_pointer_cast<const Atom>(e.sentence());
        addFact(Proposition(*atom, true), e.quantification());
    } else if (e.sentence()->getTypeCode() == Negation::TypeCode
            && (boost::static_pointer_cast<const Negation>(e.sentence())->sentence()->getTypeCode() == Atom::TypeCode)) {
        boost::shared_ptr<const Negation> neg = boost::static_pointer_cast<const Negation>(e.sentence());
        addFact(Proposition(*(boost::static_pointer_cast<const Atom>(neg->sentence())), false), e.quantification());
    } else {
        throw std::invalid_argument("unable to add fact because it's not a simple atom or its negation: "+ e.toString());
    }
}

template <class InputIterator>
inline void Domain::addFormulas(InputIterator begin, InputIterator end) {
    for (InputIterator it = begin; it != end; it++) {
        addFormula(*it);
    }
}

inline void Domain::addFact(const Proposition& p, const SISet& where) {
    // resize where
    SISet newSet(where);
    if (!maxInterval_.isNull()) newSet.setMaxInterval(span(where.maxInterval(), maxInterval_));

    if (partialModel_.count(p) == 0) {
        partialModel_.insert(std::make_pair(p, newSet));
    } else {
        partialModel_.at(p).setMaxInterval(newSet.maxInterval());
        partialModel_.at(p).add(newSet);
    }
    predTypes_.insert(p.atom.predicateType());
    allAtoms_.insert(p.atom);
    growMaxInterval(where.maxInterval());
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

inline void Domain::growMaxInterval(const Interval& maxInterval) {
    if (maxInterval_.isNull()) setMaxInterval(maxInterval);
    if (maxInterval.start() < maxInterval_.start()
            || maxInterval.finish() > maxInterval_.finish()) {
        setMaxInterval(span(maxInterval, maxInterval_));
    }
}


#endif /* DOMAIN_H_ */
