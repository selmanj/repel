/*
 * domain.h
 *
 *  Created on: May 20, 2011
 *      Author: joe
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_
#define DOMAIN_CACHE_SIZE 0

#include <set>
#include <string>
#include <map>
#include <utility>
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <boost/optional.hpp>
#include "el_syntax.h"
#include "predcollector.h"
#include "model.h"
#include "../siset.h"
#include "namegenerator.h"
#include "../lrucache.h"

std::string modelToString(const Model& m);

// TODO: this is quite a bad way to handle closed worlds.  Redo this entire mechanic so model's account for this, not the domain.

class Domain {
public:
    Domain();
    template <class FactsForwardIterator>
    Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
            FormulaList formSet,
            bool assumeClosedWorld=true);
    virtual ~Domain();

    const FormulaList& formulas() const;
    void addObservedPredicate(const Atom& a);
    const std::map<std::string, SISet>& observedPredicates() const; // TODO RENAME
    SISet getModifiableSISet(const std::string& name) const;
    SISet getModifiableSISet(const std::string& name, const SISet& where) const;
    void unsetAtomAt(const std::string& name, const SISet& where);
    NameGenerator& nameGenerator();
    Model defaultModel() const;
    Model randomModel() const;
    Interval maxInterval() const;
    SpanInterval maxSpanInterval() const;
    void setMaxInterval(const Interval& maxInterval);
    void clearCache() const {cache_.clear();}

    bool isLiquid(const std::string& predicate) const;
    bool dontModifyObsPreds() const;
    bool assumeClosedWorld() const;
    void setDontModifyObsPreds(bool b);
    void setAssumeClosedWorld(bool b);

    unsigned long score(const ELSentence& s, const Model& m) const;
    unsigned long score(const Model& m) const;

    //SISet satisfied(const Sentence& s, const Model& m, const SISet* where=NULL) const;
private:
    /*
    SISet satisfiedAtom(const Atom& a, const Model& m) const;
    SISet satisfiedNegation(const Negation& n, const Model& m) const;
    SISet satisfiedDisjunction(const Disjunction& d, const Model& m) const;
    SISet satisfiedDiamond(const DiamondOp& d, const Model& m) const;
    SISet satisfiedConjunction(const Conjunction& c, const Model& m) const;
    SISet satisfiedBoolLit(const BoolLit& b, const Model& m) const;

    SISet liqSatisfied(const Sentence& s, const Model& m) const;
    SISet liqSatisfiedAtom(const Atom& a, const Model& m) const;
    SISet liqSatisfiedNegation(const Negation& a, const Model& m) const;
    SISet liqSatisfiedDisjunction(const Disjunction& d, const Model& m) const;
    SISet liqSatisfiedConjunction(const Conjunction& c, const Model& m) const;
    SISet liqSatisfiedBoolLit(const BoolLit& b, const Model& m) const;
*/
    typedef std::pair<const Model*, const Sentence*> ModelSentencePair;

    bool assumeClosedWorld_;
    bool dontModifyObsPreds_;

    std::map<std::string, SISet> obsPreds_;
    std::set<std::string> unobsPreds_;
    std::set<std::string> constants_;
    Interval maxInterval_;

    FormulaList formulas_;
    Model observations_;

    NameGenerator generator_;

    struct ModelSentencePair_cmp {
        bool operator() (const ModelSentencePair& a, const ModelSentencePair& b) const {
            if (a.first < b.first) return true;
            if (a.first > b.first) return false;
            if (a.second < b.second) return true;
            return false;
        }
    };

    mutable LRUCache<ModelSentencePair,SISet,ModelSentencePair_cmp> cache_;
};

// IMPLEMENTATION
inline Domain::Domain() : dontModifyObsPreds_(true), maxInterval_(0,0), formulas_(), generator_(), cache_(DOMAIN_CACHE_SIZE) {};

template <class FactsForwardIterator>
Domain::Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
        FormulaList formSet,
        bool assumeClosedWorld)
        : assumeClosedWorld_(assumeClosedWorld),
          dontModifyObsPreds_(true),
          obsPreds_(),
          unobsPreds_(),
          constants_(),
          maxInterval_(0,0),
          formulas_(formSet),
          observations_(),
          generator_(),
          cache_(DOMAIN_CACHE_SIZE) {

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
    for (FormulaList::iterator it = formSet.begin(); it != formSet.end(); it++) {
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

        if (obsPreds_.find(a->name()) == obsPreds_.end()) {
            SISet newSet(true, maxInterval_);       // TODO: assumes all unobs are liquid!!!
            newSet.add(si);
            obsPreds_.insert(std::pair<std::string, SISet>(a->name(), newSet));
        } else {
            SISet curSet = obsPreds_.find(a->name())->second;
            curSet.add(si);
            obsPreds_.erase(a->name());
            obsPreds_.insert(std::pair<std::string, SISet>(a->name(), curSet));
        }
    }
    // now collect all unobserved preds

    PredCollector predCollector;
    for (FormulaList::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        it->sentence()->visit(predCollector);
    }

    // remove the predicates we know are observed
    std::set<std::string> obsJustPreds;
    for (std::map<std::string, SISet>::const_iterator it = obsPreds_.begin();
            it != obsPreds_.end();
            it++) {
        obsJustPreds.insert(it->first);
    }
    std::set<std::string> foundPreds;
    for (std::set<Atom, atomcmp>::const_iterator it = predCollector.preds.begin(); it != predCollector.preds.end(); it++) {
        foundPreds.insert(it->toString());
    }
    std::set_difference(foundPreds.begin(), foundPreds.end(),
            obsJustPreds.begin(), obsJustPreds.end(),
            std::inserter(unobsPreds_, unobsPreds_.end()));

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
    for(FormulaList::iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        if (!it->isQuantified()) continue;
        SISet set = it->quantification();
        set.setMaxInterval(maxInterval_);
        it->setQuantification(set);
    }
};

inline Domain::~Domain() {};

inline const FormulaList& Domain::formulas() const {return formulas_;};
inline const std::map<std::string, SISet>& Domain::observedPredicates() const {return obsPreds_;};
inline NameGenerator& Domain::nameGenerator() {return generator_;};
inline Model Domain::defaultModel() const {return observations_;};

inline Interval Domain::maxInterval() const {return maxInterval_;};
inline SpanInterval Domain::maxSpanInterval() const {
    return SpanInterval(maxInterval_.start(), maxInterval_.finish(),
            maxInterval_.start(), maxInterval_.finish());
};

inline bool Domain::dontModifyObsPreds() const {return dontModifyObsPreds_;};
inline bool Domain::assumeClosedWorld() const {return assumeClosedWorld_;};
inline void Domain::setDontModifyObsPreds(bool b) {dontModifyObsPreds_ = b;};
inline void Domain::setAssumeClosedWorld(bool b) {assumeClosedWorld_ = b;};

#endif /* DOMAIN_H_ */
