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

// TODO: this is quite a bad way to handle closed worlds.  Redo this entire mechanic so model's account for this, not the domain.

class Domain {
public:
    typedef boost::unordered_map<Proposition, SISet> PropMap;

    typedef std::vector<ELSentence>::const_iterator     formula_const_iterator;
    typedef PropMap::const_iterator                     fact_const_iterator;
    typedef boost::unordered_set<Atom>::const_iterator  atom_const_iterator;
    //typedef boost::unordered_set<Atom>::const_iterator obsAtoms_const_iterator;
    //typedef boost::unordered_set<Atom>::const_iterator unobsAtoms_const_iterator;
    //typedef MergedIterator<obsAtoms_const_iterator, unobsAtoms_const_iterator, const Atom> atoms_const_iterator;

    Domain();
    /*
    template <class FactsForwardIterator>
    Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
            const std::vector<ELSentence>& formSet,
            bool assumeClosedWorld=true);
    virtual ~Domain();
    //const FormulaList& formulas() const;
     */

    formula_const_iterator formulas_begin() const;
    formula_const_iterator formulas_end() const;
    fact_const_iterator facts_begin() const;
    fact_const_iterator facts_end() const;
    atom_const_iterator atoms_begin() const;
    atom_const_iterator atoms_end() const;

    std::size_t atoms_size() const;

    //obsAtoms_const_iterator obsAtoms_begin() const;
    //obsAtoms_const_iterator obsAtoms_end() const;
    //unobsAtoms_const_iterator unobsAtoms_begin() const;
    //unobsAtoms_const_iterator unobsAtoms_end() const;
    //atoms_const_iterator atoms_begin() const;
    //atoms_const_iterator atoms_end() const;

    void clearFormulas();
    void clearFacts();
    void addFormula(const ELSentence& e);
    template <class InputIterator>
    void addFormulas(InputIterator begin, InputIterator end);
    void addFact(const ELSentence& e);
    void addFact(const Proposition& p, const SISet& where);

    //void addObservedPredicate(const Atom& a);
    //const std::map<std::string, SISet>& observedPredicates() const; // TODO RENAME
    SISet getModifiableSISet(const Atom& a) const;
    SISet getModifiableSISet(const Atom& a, const SISet& where) const;
    //void unsetAtomAt(const std::string& name, const SISet& where);

    NameGenerator& nameGenerator();
    Model defaultModel() const;
    Model randomModel() const;
    Interval maxInterval() const;
    SpanInterval maxSpanInterval() const;
    void setMaxInterval(const Interval& maxInterval);

    bool isLiquid(const std::string& predicate) const;
    //bool dontModifyObsPreds() const;
    //bool assumeClosedWorld() const;
    //void setDontModifyObsPreds(bool b);
    //void setAssumeClosedWorld(bool b);

    score_t score(const ELSentence& s, const Model& m) const;
    score_t score(const Model& m) const;

    bool isFullySatisfied(const Model& m) const;
private:

    void growMaxInterval(const Interval& maxInterval);

    //typedef std::pair<const Model*, const Sentence*> ModelSentencePair;

   // bool assumeClosedWorld_;
   // bool dontModifyObsPreds_;

    //std::map<std::string, SISet> obsPredsFixedAt_;
    //boost::unordered_set<Atom> obsPreds_;
    //boost::unordered_set<Atom> unobsPreds_;

    Interval maxInterval_;
    std::vector<ELSentence> formulas_;
    PropMap partialModel_;
    boost::unordered_set<PredicateType> predTypes_;
    boost::unordered_set<Atom> allAtoms_;
    //Model observations_;

    NameGenerator generator_;

    /*
    struct ModelSentencePair_cmp : public std::binary_function<ModelSentencePair, ModelSentencePair, bool> {
        bool operator() (const ModelSentencePair& a, const ModelSentencePair& b) const {
            if (a.first < b.first) return true;
            if (a.first > b.first) return false;
            if (a.second < b.second) return true;
            return false;
        }
    };
    */

    //mutable LRUCache<ModelSentencePair,SISet,ModelSentencePair_cmp> cache_;
};

// IMPLEMENTATION
inline Domain::Domain()
    : maxInterval_(),
      formulas_(),
      partialModel_(),
      predTypes_(),
      allAtoms_(),
      generator_(){};
/*
template <class FactsForwardIterator>
Domain::Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
        const std::vector<ELSentence>& formSet,
        bool assumeClosedWorld)
        : assumeClosedWorld_(assumeClosedWorld),
          dontModifyObsPreds_(true),
          obsPredsFixedAt_(),
          obsPreds_(),
          unobsPreds_(),
          maxInterval_(0,0),
          formulas_(formSet),
          observations_(),
          generator_() {

    // find the maximum interval of time
    if (factsBegin == factsEnd) {
        // what to do for time??
        std::runtime_error e("no facts given: currently need at least one fact to determine the interval to reason over!");
        throw e;
    }
    unsigned int smallest=std::numeric_limits<unsigned int>::max(), largest=std::numeric_limits<unsigned int>::min();
    for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
        SpanInterval interval = it->where();

        boost::optional<SpanInterval> norm = interval.normalize();
        if (!norm) {
            continue;
        }
        interval = norm.get();
        smallest = (std::min)(interval.start().start(), smallest);
        largest = (std::max)(interval.finish().finish(), largest);
    }
    for (std::vector<ELSentence>::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        if (!it->isQuantified()) continue;
        SISet set = it->quantification();
        for (SISet::const_iterator it2 = set.begin(); it2 != set.end(); it2++) {
            smallest = (std::min)(it2->start().start(), smallest);
            largest = (std::max)(it2->finish().finish(), largest);
        }
    }
    maxInterval_ = Interval(smallest, largest);

    // collect all fact predicates
    for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
        boost::shared_ptr<const Atom> a = it->atom();
        SpanInterval si = it->where();

        if (obsPredsFixedAt_.find(a->name()) == obsPredsFixedAt_.end()) {
            SISet newSet(true, maxInterval_);       // TODO: assumes all unobs are liquid!!!
            newSet.add(si);
            obsPredsFixedAt_.insert(std::pair<std::string, SISet>(a->name(), newSet));

            // add it as an observed predicate
            obsPreds_.insert(*a);
        } else {
            SISet curSet = obsPredsFixedAt_.find(a->name())->second;
            curSet.add(si);
            obsPredsFixedAt_.erase(a->name());
            obsPredsFixedAt_.insert(std::pair<std::string, SISet>(a->name(), curSet));
        }
    }
    // now collect all unobserved preds

    PredCollector predCollector;
    for (std::vector<ELSentence>::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        it->sentence()->visit(predCollector);
    }

    for (std::set<Atom, atomcmp>::const_iterator it = predCollector.preds.begin(); it != predCollector.preds.end(); it++) {
        if (obsPreds_.find(*it) == obsPreds_.end()) {
            unobsPreds_.insert(*it);
        }
    }

    // initialize observations
    for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
        boost::shared_ptr<const Atom> atom = it->atom();
        SpanInterval interval = it->where();

        // TODO: we are hardwired for liquidity, come back and fix this later

        if (it->truthVal()) {
            SISet set(true, maxInterval_);

            set.add(interval);
            observations_.setAtom(*atom, set);
        }
    }

    // enforce maxInterval on our formulas
    for(std::vector<ELSentence>::iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        if (!it->isQuantified()) continue;
        SISet set = it->quantification();
        set.setMaxInterval(maxInterval_);
        it->setQuantification(set);
    }
};
*/

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
    if (partialModel_.count(p) == 0) {
        partialModel_.insert(std::make_pair(p, where));
    } else {
        partialModel_.at(p).add(where);
    }
    predTypes_.insert(p.atom.predicateType());
    allAtoms_.insert(p.atom);
    growMaxInterval(where.maxInterval());
}



inline NameGenerator& Domain::nameGenerator() {return generator_;};
inline Model Domain::defaultModel() const {return Model(partialModel_, maxInterval_);};

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
