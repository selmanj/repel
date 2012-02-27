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
    Model run(const Domain& d, URNG& u);

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

    struct WeightComparator : public std::binary_function<ELSentence, ELSentence, bool> {
        bool operator()(const ELSentence& l, const ELSentence& r) const {
            return l.weight()*l.quantification().size() < r.weight()*r.quantification().size();
        }
    };

    struct EnforceQuantification : public std::unary_function<ELSentence, void> {
        EnforceQuantification(const Domain& domain) : d(domain) {}

        void operator()(ELSentence& s) const {
            if (s.hasInfWeight()) {
                s.setQuantification(SISet(d.maxSpanInterval(), false));
            }
        }

        const Domain& d;
    };
}

class unsatisfiable_exception : public std::exception {
public:
    unsatisfiable_exception(const Model& m) throw() : std::exception(), partialModel(m) {};
    unsatisfiable_exception(const unsatisfiable_exception& e) throw() : std::exception(e), partialModel(e.partialModel) {};
    virtual ~unsatisfiable_exception() throw() {}
    Model partialModel;
};

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
Model MCSat::run(const Domain& d, URNG& rng) {
    // first, make a copy of the domain where all the formulas have quantification
    std::list<ELSentence> formulas(d.formulas_begin(), d.formulas_end());
    std::for_each(formulas.begin(), formulas.end(), EnforceQuantification(d));
    Domain dFixed = d;
    dFixed.clearFormulas();
    for(std::list<ELSentence>::const_iterator it = formulas.begin(); it != formulas.end(); it++) {
        dFixed.addFormula(*it);
    }

    // split out inf weighted formulas
    std::list<ELSentence> infForms;
    std::list<ELSentence> weightedForms;
    std::remove_copy_if(formulas.begin(), formulas.end(), std::back_inserter(infForms), std::not1(isInfWeightPred()));
    std::remove_copy_if(formulas.begin(), formulas.end(), std::back_inserter(weightedForms), isInfWeightPred());

    std::list<ELSentence> currentFormSet;
    if (!infForms.empty()) {
        std::cout << "adding all infinite weighted formulas to start with" << std::endl;
        std::cout << "infinite constraints: ";
        std::copy(infForms.begin(), infForms.end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
        std::cout << std::endl;
        // before starting, run a sat solver to try to get an initial model if we have hard constraints
        currentFormSet.insert(currentFormSet.end(), infForms.begin(), infForms.end());
    } else {
        if (weightedForms.empty()) {
            std::cerr << "no formulas to solve!";
            throw std::invalid_argument("no formulas to solve");
        }
        std::cout << "no infinite weighted formulas - choosing the best looking constraint to add" << std::endl;
        // add the best looking rule so far
        std::list<ELSentence>::iterator bestRuleIt = std::max_element(weightedForms.begin(), weightedForms.end(), WeightComparator());
        currentFormSet.push_back(*bestRuleIt);
        weightedForms.erase(bestRuleIt);
        std::cout << "added constraint: " << *bestRuleIt << std::endl;
    }
    Model currModel = dFixed.randomModel();
    std::cout << "Random model: " << currModel.toString() << std::endl;
    std::list<ELSentence> notSatisfiable;
    for (unsigned int iteration = 1; iteration <= numIterations_; iteration++) {
        std::cout << "** Starting iteration 1" << std::endl;
        // call sat solver on current formula set
        try {
            Model candidateModel = solveSATProblemWithFormulas(currModel, d, currentFormSet);
            std::cout << "candidateModel = " << candidateModel.toString() << std::endl;
            std::cout << "our model is fully satisfied (solved the sat problem).  finding a new rule to add" << std::endl;
            // find new formulas to add to the set
            std::list<ELSentence> formsSatisfied;
            std::list<ELSentence> formsNotSatisfied;

            splitSatisfiedRules(currModel, d, weightedForms.begin(), weightedForms.end(), std::back_inserter(formsSatisfied), std::back_inserter(formsNotSatisfied));


            std::cout << "newly satisfied: ";
            std::copy(formsSatisfied.begin(), formsSatisfied.end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
            std::cout << std::endl;
            std::cout << "newly unsatisfied: ";
            std::copy(formsNotSatisfied.begin(), formsNotSatisfied.end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
            std::cout << std::endl;

            // first, check to see if we are done
             if (formsNotSatisfied.empty()) {
                 std::cout << "nothing left to satisfy!  returning our answer..." << std::endl;
                 return candidateModel;
             }

            // add all the forms already satisfied to our set
            currentFormSet.insert(currentFormSet.end(), formsSatisfied.begin(), formsSatisfied.end());
            // update weighted forms with our unsatisfied set
            weightedForms = formsNotSatisfied;
            std::cout << "weightedForms now = ";
            std::copy(weightedForms.begin(), weightedForms.end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
            std::cout << std::endl;

            // add the best looking rule and update
            std::list<ELSentence>::iterator bestRuleIt = std::max_element(weightedForms.begin(), weightedForms.end(), WeightComparator());
            currentFormSet.push_back(*bestRuleIt);
            std::cout << "added constraint: " << *bestRuleIt << std::endl;
            weightedForms.erase(bestRuleIt);
            currModel = candidateModel;
        } catch (unsatisfiable_exception& e) {
            std::cout << "domain NOT fully satisfied, relaxing last rule and continuing...";
            // TODO: bug, assuming we can add or remove the entire rule, but what if its satisfied over some of the interval?

            // last rule added can't be satisfied (or at least we will assume it cannot).
            // add it to the list, remove and continue on
            ELSentence lastAdded = currentFormSet.back();
            currentFormSet.pop_back();
            notSatisfiable.push_back(lastAdded);

            if (weightedForms.empty()) {
                std::cout << "no other options to try, returning last model.";
                return currModel;
            }
            // try the next best
            std::list<ELSentence>::iterator bestRuleIt = std::max_element(weightedForms.begin(), weightedForms.end(), WeightComparator());
            currentFormSet.push_back(*bestRuleIt);
            weightedForms.erase(bestRuleIt);
        }
    }
    std::cout << "ran out of iterations - returning last model.";
    return currModel;   // ran out of iterations


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
            formula.setQuantification(SISet(d.maxSpanInterval(), false));
        }
        SISet quantification = formula.quantification();
        if (quantification.empty()) {
            continue;   // this formula applies nowhere?
        }
        SISet satisfiedAt = formula.dSatisfied(m, d);
        SISet unsatisfiedAt = formula.dNotSatisfied(m, d);

        if (!satisfiedAt.empty()) {
            // split into single spanning intervals
            for (SISet::const_iterator it2 = satisfiedAt.begin(); it2 != satisfiedAt.end(); it2++) {
                ELSentence newForm = formula;
                newForm.setQuantification(SISet(*it2, false));
                *satIt = newForm;
                satIt++;
            }
        }
        if (!unsatisfiedAt.empty()) {
            // split into single spanning intervals
            for (SISet::const_iterator it2 = unsatisfiedAt.begin(); it2 != unsatisfiedAt.end(); it2++) {
                ELSentence newForm = formula;
                newForm.setQuantification(SISet(*it2, false));
                *unsatIt = newForm;
                unsatIt++;
            }
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
    std::cout << "solving sat problem with rules: ";
    std::copy(newDomain.formulas_begin(), newDomain.formulas_end(), std::ostream_iterator<ELSentence>(std::cout, ", "));
    std::cout << std::endl;

    Model m = maxWalkSat(newDomain, walksatIterations_, walksatRandomMoveProb_, &initModel);
    //std::cout << "model after sat = " << m.toString() << std::endl;
    if (!newDomain.isFullySatisfied(m)) {
        std::cout << "warning, domain NOT fully satisfied" << std::endl;
        throw unsatisfiable_exception(m);
    }
    return m;
}


#endif /* MCSAT_H_ */
