/*
 * mcsat.cpp
 *
 *  Created on: Feb 8, 2012
 *      Author: selman.joe@gmail.com
 */

#include "mcsat.h"
#include "maxwalksat.h"
#include "../logic/unit_prop.h"
#include "../logic/domain.h"
#include "../logic/syntax/elsentence.h"

void MCSat::run() {
    if (d_ == 0) {
        throw std::logic_error("MCSat::run() - Domain not set");
    }
    if (sampleStrategy_ == 0) {
        throw std::logic_error("MCSat::run() - SampleStrategy not set");
    }
    samples_.clear();
    samples_.reserve(numSamples_);

    // first, run unit propagation on our domain to get a new reduced one.
    Domain reduced = MCSat::applyUP(*d_);
    // default model is our initial sample
    Model prevModel = reduced.defaultModel();
    Domain prevDomain = reduced;
    samples_.push_back(prevModel);

    for (unsigned int iteration = 1; iteration < numSamples_; iteration++) {
        std::vector<ELSentence> newSentences;

        sampleStrategy_->sampleSentences(prevModel, prevDomain, newSentences);
        // make a new domain using new Sentences
        Domain curDomain;
        for (Domain::fact_const_iterator it = prevDomain.facts_begin(); it != prevDomain.facts_end(); it++) {
            curDomain.addFact(*it);
        }
        for (std::vector<ELSentence>::const_iterator it = newSentences.begin(); it != newSentences.end(); it++) {
            curDomain.addFormula(*it);
        }

        boost::unordered_set<Model> curModels = sampleSat(prevModel, curDomain);
        assert(!curModels.empty());
        // choose a random model
        boost::unordered_set<Model>::size_type index = (rand() % curModels.size());
        boost::unordered_set<Model>::const_iterator it = curModels.begin();
        while (index > 0) {
            it++;
            index--;
        }
        // add the model
        samples_.push_back(*it);
        prevModel = *it;
        prevDomain = curDomain;
    }
}

Domain MCSat::applyUP(const Domain& d) {
    Domain reduced;
    try {
        reduced = performUnitPropagation(d);
    } catch (contradiction& c) {
        // rewrite error message
        throw contradiction("Contradiction found in MCSat::run() when running unit prop()");
    }

    // default model is guaranteed to satisfy the facts
    Model m = reduced.defaultModel();
    // check to make sure hard clauses are satisfied
    std::vector<ELSentence> hardClauses;
    std::remove_copy_if(reduced.formulas_begin(), reduced.formulas_end(), std::back_inserter(hardClauses), std::not1(IsHardClausePred()));
    for (std::vector<ELSentence>::const_iterator it = hardClauses.begin(); it != hardClauses.end(); it++) {
        if (!it->fullySatisfied(m, reduced)) {
            throw contradiction("Contradiction found in MCSat::run() when verifying hard clauses are satisfied");
        }
    }
    return reduced;
}

void MCSatSampleSegmentsStrategy::sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled) {
    for (Domain::formula_const_iterator it = d.formulas_begin(); it != d.formulas_end(); it++) {
        ELSentence currentSentence = *it;
        SISet satisfied = currentSentence.dSatisfied(m, d);

        if (satisfied.empty()) continue;

    }
    throw std::runtime_error("MCSatSampleSegmentsStrategy::sampleSentences() not yet implemented");
}

boost::unordered_set<Model> MCSat::sampleSat(const Model& initialModel, const Domain& d) {
    boost::unordered_set<Model> models;
    models.insert(initialModel); // always include the initial model

    // transform domain into a SAT problem
    Domain dSat;
    for (Domain::fact_const_iterator it = d.facts_begin(); it != d.facts_end(); it++) {
        dSat.addFact(*it);
    }
    for (Domain::formula_const_iterator it = d.formulas_begin(); it != d.formulas_end(); it++) {
        ELSentence s = *it;
        s.setHasInfWeight(true);
        dSat.addFormula(s);
    }
    // perform UP
    Domain reduced;
    try {
        reduced = MCSat::applyUP(dSat);
    } catch (contradiction& c) {
        return models;  // can't continue, just return our models which has only one item.
    }

    // do some random restarts and hope for different models
    for (unsigned int i = 1; i <= walksatNumRandomRestarts_; i++) {
        Model iterInitModel = dSat.randomModel();   // TODO: better way to make random models
        Model iterModel = maxWalkSat(dSat, walksatIterations_, walksatRandomMoveProb_, &iterInitModel);
        if (dSat.isFullySatisfied(iterModel)) models.insert(iterModel);
    }
    return models;
}

MCSatSampleSegmentsStrategy::MCSatSampleSegmentsStrategy(const Domain& d)
    : formulaToSegment_() {
    for (Domain::fact_const_iterator it = d.facts_begin(); it != d.facts_end(); it++) {
        Proposition p = it->first;
        Sentence *a = new Atom(p.atom); // this should work for negation too,
                                        // since the segment collection is the same

        SISet sisetWhere = it->second;
        boost::unordered_set<SpanInterval> where;
        sisetWhere.collectSegments(std::inserter(where, where.end()));

        formulaToSegment_[a] = where;
    }
    for (Domain::formula_const_iterator it = d.formulas_begin(); it != d.formulas_end(); it++) {
        ELSentence s = *it;
        // collect all the atoms, union their sets and add it to the map
        AtomCollector atomCollect;

        s.sentence()->visit(atomCollect);
        assert(!atomCollect.atoms.empty());
        boost::unordered_set<SpanInterval> segments;
        for (boost::unordered_set<Atom>::const_iterator it2 = atomCollect.atoms.begin();
                it2 != atomCollect.atoms.end();
                it2++) {
            Atom atom = *it2;
            boost::unordered_set<SpanInterval> toMerge = formulaToSegment_.at(&atom);
            std::copy(toMerge.begin(), toMerge.end(), std::inserter(segments, segments.end()));
        }
        Sentence *sPtr = s.sentence()->clone();
        formulaToSegment_[sPtr] = segments;
    }
}

MCSatSampleSegmentsStrategy::MCSatSampleSegmentsStrategy(const MCSatSampleSegmentsStrategy& s)
    : formulaToSegment_() {
    for (boost::unordered_map<Sentence*, boost::unordered_set<SpanInterval> >::const_iterator it = s.formulaToSegment_.begin();
            it != s.formulaToSegment_.end();
            it++) {
        Sentence *sPtr = (it->first == 0 ? 0 : it->first->clone());
        formulaToSegment_.insert(std::make_pair(sPtr, it->second));
    }
}

MCSatSampleSegmentsStrategy::~MCSatSampleSegmentsStrategy() {
    std::vector<Sentence*> toDelete;
    for (boost::unordered_map<Sentence*, boost::unordered_set<SpanInterval> >::iterator it = formulaToSegment_.begin();
            it != formulaToSegment_.end();
            it++) {
        toDelete.push_back(it->first);
    }
    formulaToSegment_.clear();
    for (std::vector<Sentence*>::iterator it = toDelete.begin(); it != toDelete.end(); it++) {
        delete *it;
    }
    toDelete.clear();
}
