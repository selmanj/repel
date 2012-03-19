/*
 * mcsat.cpp
 *
 *  Created on: Feb 8, 2012
 *      Author: selman.joe@gmail.com
 */

#include "mcsat.h"
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
    Domain reduced = applyUP(*d_);
    // default model is our initial sample
    samples_.push_back(reduced.defaultModel());

    for (unsigned int iteration = 1; iteration < numSamples_; iteration++) {
        std::vector<ELSentence> newSentences;

    }

    throw std::runtime_error("MCSat::run() unimplemented.");
}

Domain MCSat::applyUP(const Domain& d) const {
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
