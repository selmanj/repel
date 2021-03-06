/*
 * MCSat.cpp
 *
 *  Created on: Feb 8, 2012
 *      Author: selman.joe@gmail.com
 */

#include <cmath>
#include <boost/random/bernoulli_distribution.hpp>
#include "MCSat.h"
#include "MaxWalkSat.h"
#include "../logic/UnitProp.h"
#include "../logic/Domain.h"
#include "../logic/syntax/ELSentence.h"

const unsigned int MCSat::defNumSamples = 1000;
const unsigned int MCSat::defBurnInIterations = 1000;
const unsigned int MCSat::defWalksatIterations = 1000;
const double MCSat::defWalksatRandomMoveProb = 0.2;
const unsigned int MCSat::defWalksatNumRandomRestarts = 4;
const bool MCSat::defUseRandomInitialModels = true;
const bool MCSat::defUseUnitPropagation = true;

void MCSat::run(boost::mt19937& rng) { // TODO: setup using random initial models
    if (d_ == 0) {
        throw std::logic_error("MCSat::run() - Domain not set");
    }
    if (sampleStrategy_ == 0) {
        throw std::logic_error("MCSat::run() - SampleStrategy not set");
    }
    samples_.clear();
    samples_.reserve(numSamples_);

    //std::cout << "initial domain: ";
    //d_->printDebugDescription(std::cout);

    // first, run unit propagation on our domain to get a new reduced one.
    Domain reduced;
    if (useUnitPropagation_) {
        reduced = MCSat::applyUP(*d_);
    } else {
        reduced = *d_;
    }
    //std::cout << "reduced domain: ";
    //reduced.printDebugDescription(std::cout);


    Model prevModel = (useRandomInitialModels_ ? reduced.randomModel(rng) : reduced.defaultModel());

    // do a starting run on the whole problem as our initial sample
    //boost::unordered_set<Model> initModels = sampleSat(prevModel, reduced);
    //prevModel = *initModels.begin();
    Domain prevDomain = reduced;

    if (burnInIterations_ == 0) samples_.push_back(prevModel);
    unsigned int totalIterations = numSamples_+burnInIterations_;


    for (unsigned int iteration = 1; iteration < totalIterations; iteration++) {
        std::vector<ELSentence> newSentences;

        if ( totalIterations < 20 ||
                iteration % (totalIterations / 20) == 0) {
            std::cout << (((double)iteration) / ((double) totalIterations))*100 << "% done." << std::endl;
        }

        sampleStrategy_->sampleSentences(prevModel, reduced, rng, newSentences);
//        if (iteration == 1) {
//            std::cout << "initial sampled sentences: ";
//            std::copy(newSentences.begin(), newSentences.end(), std::ostream_iterator<ELSentence>(std::cout, "\n"));
//            std::cout << "initial model:";
//            std::cout << prevModel;
//            for (Domain::formula_const_iterator it = prevDomain.formulas_begin();
//                    it != prevDomain.formulas_end();
//                    it++) {
//                std::cout << "formula " << *it << " is satisfied at: " << it->dSatisfied(prevModel, prevDomain) << std::endl;
//            }
//            std::cin.get();
//        }

        // make a new domain using new Sentences
        Domain curDomain;
        curDomain.setMaxInterval(prevDomain.maxInterval());
        for (Domain::fact_const_iterator it = prevDomain.facts_begin(); it != prevDomain.facts_end(); it++) {
            curDomain.addFact(*it);
        }
        for (std::vector<ELSentence>::const_iterator it = newSentences.begin(); it != newSentences.end(); it++) {
            curDomain.addFormula(*it);
        }
        curDomain.addAtoms(prevDomain.atoms_begin(), prevDomain.atoms_end());
//
//        if (iteration == burnInIterations_ + numSamples_/2) {
//            std::cout << "ITERATION: " << iteration << std::endl;
//            std::cout << "curDomain";
//            curDomain.printDebugDescription(std::cout);
//            std::cout << "sampled sentences: ";
//            std::copy(newSentences.begin(), newSentences.end(), std::ostream_iterator<ELSentence>(std::cout, "\n"));
//        }


        boost::unordered_set<Model> curModels = sampleSat(prevModel, curDomain, rng);
        assert(!curModels.empty());
        // choose a random model
        boost::uniform_int<std::size_t> pickModel(0, curModels.size()-1);
        boost::unordered_set<Model>::size_type index = pickModel(rng);
        boost::unordered_set<Model>::const_iterator it = curModels.begin();
        while (index > 0) {
            it++;
            index--;
        }
        // add the model
        if (iteration >= burnInIterations_) samples_.push_back(*it);
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

/*

void MCSatSampleSegmentsStrategy::sampleSentences(const Model& m, const Domain& d, boost::mt19937& rng, std::vector<ELSentence>& sampled) {
    for (Domain::formula_const_iterator it = d.formulas_begin(); it != d.formulas_end(); it++) {
        ELSentence currentSentence = *it;

        // if it has hard weight, it must be included
        if (currentSentence.hasInfWeight()) {
            sampled.push_back(currentSentence);
            continue;
        }
        SISet satisfied = currentSentence.dSatisfied(m, d);
        if (satisfied.empty()) continue;

        // find the segments for this sentence
        boost::unordered_set<SpanInterval> segments = formulaToSegment_.at(currentSentence.sentence().get());
        SISet toEnforceAt(false, d.maxInterval());
        for (boost::unordered_set<SpanInterval>::const_iterator sIt = segments.begin(); sIt != segments.end(); sIt++) {
            // first, require that the sentence be fully satisfied at the spanning interval
            if (!satisfied.includes(*sIt)) continue;
            // sample it.  with probability 1 - e^-score, add it to be enforced
            double prob = 1.0 - exp((double)-(currentSentence.weight()*sIt->size()));
            boost::bernoulli_distribution<double> flip(prob);
            bool addit = flip(rng);
            //bool addit = (((double)rand()) / ((double)RAND_MAX)) <= prob;
            if (addit) toEnforceAt.add(*sIt);
        }
        if (!toEnforceAt.empty()) {
            currentSentence.setQuantification(toEnforceAt);
            sampled.push_back(currentSentence);
        }
    }
}
*/

boost::unordered_set<Model> MCSat::sampleSat(const Model& initialModel, const Domain& d, boost::mt19937& rng) {
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
    dSat.addAtoms(d.atoms_begin(), d.atoms_end());
    // perform UP (if possible)
    Domain reduced;
    if (useUnitPropagation_) {
        try {
            reduced = MCSat::applyUP(dSat);
        } catch (contradiction& c) {
            return models;  // can't continue, just return our models which has only one item.
        }
    } else {
        reduced = dSat;
    }

    // rewrite infinite weighted formulas so they have singular weight
    reduced = reduced.replaceInfForms();

    // do some random restarts and hope for different models
    MWSSolver walksatSolver(walksatIterations_, walksatRandomMoveProb_, &reduced);
    for (unsigned int i = 1; i <= walksatNumRandomRestarts_; i++) {
        Model iterInitModel = reduced.randomModel(rng);   // TODO: better way to make random models
        if (reduced.formulas_size() == 0) {
            // just add the random model and continue
            models.insert(reduced.randomModel(rng));
            continue;
        }

        //std::cout << "--\nFormulas for maxwalksat: ";
        //std::copy(reduced.formulas_begin(), reduced.formulas_end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
        //std::cout << std::endl;

        Model iterModel = walksatSolver.run(rng, iterInitModel);
        if (reduced.isFullySatisfied(iterModel)) models.insert(iterModel);
    }
    return models;
}

double MCSat::estimateProbability(const Proposition& prop, const Interval& where) const {
    return ((double)countProps(prop, where)) / ((double) numSamples_);
}

unsigned int MCSat::countProps(const Proposition& prop, const Interval& where) const {
    assert(samples_.size() == numSamples_);
    unsigned int count = 0;

    for (std::vector<Model>::const_iterator it = samples_.begin(); it != samples_.end(); it++) {
        SISet trueAt = it->getAtom(prop.atom());
        if ((trueAt.includes(where) && prop.sign())
                || (!trueAt.includes(where) && !prop.sign())) {
            count++;
        }
    }

    return count;
}

/*
MCSatSampleSegmentsStrategy::MCSatSampleSegmentsStrategy(const Domain& d)
    : formulaToSegment_() {
    for (Domain::fact_const_iterator it = d.facts_begin(); it != d.facts_end(); it++) {
        Proposition p = it->first;
        Sentence *a = new Atom(p.atom()); // this should work for negation too,
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
            if (formulaToSegment_.count(&atom) == 0) continue;
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
*/

bool operator==(const MCSat& l, const MCSat& r) {
    return (
            (l.d_ == r.d_ || (l.d_ != NULL && r.d_ != NULL && *l.d_ == *r.d_) ) &&
            l.numSamples_ == r.numSamples_ &&
            l.burnInIterations_ == r.burnInIterations_ &&
            l.walksatIterations_ == r.walksatIterations_ &&
            l.walksatRandomMoveProb_ == r.walksatRandomMoveProb_ &&
            l.walksatNumRandomRestarts_ == r.walksatNumRandomRestarts_ &&
            l.useRandomInitialModels_ == r.useRandomInitialModels_ &&
            l.useUnitPropagation_ == r.useUnitPropagation_ &&
            l.samples_ == r.samples_ &&
            (l.sampleStrategy_ == r.sampleStrategy_ || (l.sampleStrategy_ != NULL && r.sampleStrategy_ != NULL && *l.sampleStrategy_ == *r.sampleStrategy_))
            );
//    ar & d_;
//    ar & numSamples_;
//    ar & burnInIterations_;
//    ar & walksatIterations_;
//    ar & walksatRandomMoveProb_;
//    ar & walksatNumRandomRestarts_;
//    ar & useRandomInitialModels_;
//    ar & samples_;
//    ar & sampleStrategy_;
}
