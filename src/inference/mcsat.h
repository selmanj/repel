/*
 * mcsat.h
 *
 *  Created on: Feb 7, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSAT_H_
#define MCSAT_H_

#include <cmath>
#include <boost/random.hpp>
#include <algorithm>
#include "maxwalksat.h"
#include "../logic/domain.h"

class MCSat {
public:
    static const unsigned int defNumIterations = 1000;
    static const unsigned int defWalksatIterations = 1000;
    static const double defWalksatRandomMoveProb = 0.2;

    MCSat();

    template<typename URNG>
    void run(const Domain& d, URNG& u);

    unsigned int numIterations() const;
    unsigned int walksatIterations() const;
    double walksatRandomMoveProb() const;

    void setNumIterations(unsigned int numIterations);
    void setWalksatIterations(unsigned int walksatIterations);
    void setWalksatRandomMoveProb(double walksatRandomMoveProb);

    template<typename URNG>
    Model performIteration(const Model& m, const Domain& d, URNG& rng) const;

private:
    unsigned int numIterations_;
    unsigned int walksatIterations_;
    double walksatRandomMoveProb_;

    /**
     * Given a list of formulas, find where they are satisfied in the model
     * and split them into two different lists.
     */
    template<class InputIterator, class satisfiedOutIt, class unsatisfiedOutIt>
    void splitSatisfiedRules(const Model& m,
            const Domain& d,
            InputIterator begin,
            InputIterator end,
            satisfiedOutIt satIt,
            unsatisfiedOutIt unsatIt) const;

    Model solveSATProblemWithFormulas(const Model& initModel, const Domain& d, const std::list<ELSentence> forms) const;

};

// private namespace
namespace {
    struct isInfWeightPred : public std::unary_function<ELSentence, bool> {
        bool operator()(const ELSentence& s) const {
            return s.hasInfWeight();
        }
    };
}

// IMPLEMENTATION
inline MCSat::MCSat()
: numIterations_(MCSat::defNumIterations), walksatIterations_(MCSat::defWalksatIterations), walksatRandomMoveProb_(MCSat::defWalksatRandomMoveProb) {};
inline unsigned int MCSat::numIterations() const {return numIterations_;}
inline unsigned int MCSat::walksatIterations() const { return walksatIterations_;}
inline double MCSat::walksatRandomMoveProb() const {return walksatRandomMoveProb_;}

inline void MCSat::setNumIterations(unsigned int num) {numIterations_ = num;}
inline void MCSat::setWalksatIterations(unsigned int walksatIterations) {walksatIterations_ = walksatIterations;}
inline void MCSat::setWalksatRandomMoveProb(double walksatRandomMoveProb) {walksatRandomMoveProb_ = walksatRandomMoveProb;}

template<typename URNG>
void MCSat::run(const Domain& d, URNG& rng) {
    // first, use the default model as the initial model
    //Model initialmodel = d.defaultModel();
    Model currModel = d.randomModel();
    std::cout << currModel.toString() << std::endl;
    // split out inf weighted formulas
    std::list<ELSentence> infForms;
    std::list<ELSentence> weightedForms;
    std::remove_copy_if(d.formulas_begin(), d.formulas_end(), std::back_inserter(infForms), std::not1(isInfWeightPred()));
    std::remove_copy_if(d.formulas_begin(), d.formulas_end(), std::back_inserter(weightedForms), isInfWeightPred());

    // before starting, run a sat solver to try to get an initial model if we have hard constraints
    if (!infForms.empty()) {
        Model solution = solveSATProblemWithFormulas(currModel, d, infForms);
    }

    // find rules currently satisfied, add them to the set
    std::list<ELSentence> formsSatisfied;
    std::list<ELSentence> formsNotSatisfied;

    splitSatisfiedRules(currModel, d, weightedForms.begin(), weightedForms.end(), std::back_inserter(formsSatisfied), std::back_inserter(formsNotSatisfied));

    std::cout << "satisfied: ";
    std::copy(formsSatisfied.begin(), formsSatisfied.end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
    std::cout << std::endl;
    std::cout << "unsatisfied: ";
    std::copy(formsNotSatisfied.begin(), formsNotSatisfied.end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
    std::cout << std::endl;



    /*
    for (unsigned int i = 0; i < numIterations_; i++ ) {
        currModel = performIteration(currModel, d, rng);
    }
    */
    throw std::runtime_error("MCSat::run not implemented");
}

template<class InputIterator, class satisfiedOutIt, class unsatisfiedOutIt>
void MCSat::splitSatisfiedRules(const Model& m,
        const Domain& d,
        InputIterator begin,
        InputIterator end,
        satisfiedOutIt satIt,
        unsatisfiedOutIt unsatIt) const {
    for (InputIterator it = begin; it != end; it++) {
        ELSentence formula = *it;
        if (!formula.isQuantified()) {
            formula.setQuantification(SISet(d.maxSpanInterval(), false, d.maxInterval()));
        }
        SISet quantification = formula.quantification();
        if (quantification.empty()) {
            continue;   // this formula applies nowhere?
        }
        SISet satisfiedAt = formula.dSatisfied(m, d);
        SISet unsatisfiedAt = formula.dNotSatisfied(m, d);

        if (!satisfiedAt.empty()) {
            ELSentence newForm = formula;
            newForm.setQuantification(satisfiedAt);
            *satIt = newForm;
            satIt++;
        }
        if (!unsatisfiedAt.empty()) {
            ELSentence newForm = formula;
            newForm.setQuantification(unsatisfiedAt);
            *unsatIt = newForm;
            unsatIt++;
        }
    }
}

template<typename URNG>
Model MCSat::performIteration(const Model& m, const Domain& d, URNG& rng) const {
    /*
    // first, select a subset of the rules in the domain to enforce as hard,
    // based on their weight
    FormulaList formsToEnforce;
    for(FormulaList::const_iterator it = d.formulas().begin(); it != d.formulas().end(); it++) {
        ELSentence curForm = *it;
        if (curForm.hasInfWeight()) {
            // have to include it!
            formsToEnforce.push_back(curForm);
            continue;
        }
        SISet where = (curForm.isQuantified() ? curForm.quantification() : SISet(d.maxSpanInterval(), false, d.maxInterval()));
        // find where it's satisfied
        SISet trueAt = curForm.sentence()->dSatisfied(m, d, where);
        if (trueAt == where) {
            // keep enforcing it for now, it's satisfied everywhere
            formsToEnforce.push_back(curForm);
        } else {
            SISet violations = intersection(trueAt.compliment(), where);
            for (SISet::const_iterator it2 = trueAt.begin(); it2 != trueAt.end(); it2++) {
                SpanInterval violation = *it2;
                // calculate the probability of selecting a single interval
                double p = 1.0 - exp(-static_cast<double>(curForm.weight()));

                boost::binomial_distribution<unsigned int> distro(violation.size(), p);
                unsigned int toSelect = distro(rng);
                std::cout << "drew from binomial distribution with t = " << violation.size() << ", p = " << p << std::endl;
                std::cout << "selected " << toSelect << std::endl;
            }
        }
    }
    */

    throw std::runtime_error("MCSat::performIteration not implemented");
}

Model MCSat::solveSATProblemWithFormulas(const Model& initModel, const Domain& d, const std::list<ELSentence> forms) const {
    // TODO: add unit prop
    Domain newDomain = d;
    newDomain.clearFormulas();
    for(std::list<ELSentence>::const_iterator it = forms.begin(); it != forms.end(); it++) {
        ELSentence s = *it;
        s.setWeight(1); // hard coded to 1
        newDomain.addFormula(s);
    }

    Model m = maxWalkSat(newDomain, walksatIterations_, walksatRandomMoveProb_, &initModel);
    std::cout << "model after sat = " << m.toString() << std::endl;
    if (!newDomain.isFullySatisfied(m)) {
        std::cout << "warning, domain NOT fully satisfied" << std::endl;
    }
    return m;
}


#endif /* MCSAT_H_ */
