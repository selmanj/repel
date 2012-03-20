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
        prevModel = *it;
        prevDomain = curDomain;
    }

    throw std::runtime_error("MCSat::run() unimplemented.");
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
        Model iterInitModel = dSat.randomModel();   // todo: better way to make random models
        Model iterModel = maxWalkSat(dSat, walksatIterations_, walksatRandomMoveProb_, &iterInitModel);
        if (dSat.isFullySatisfied(iterModel)) models.insert(iterModel);
    }
    return models;
}
