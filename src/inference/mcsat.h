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
#include "../logic/domain.h"

class MCSat {
public:
    MCSat();

    template<typename URNG>
    void run(const Domain& d, URNG& u);

    unsigned int numIterations() const;

    template<typename URNG>
    Model performIteration(const Model& m, const Domain& d, URNG& rng) const;
    void setNumIterations(unsigned int num);
private:
    unsigned int numIterations_;
};

// IMPLEMENTATION
inline MCSat::MCSat() : numIterations_(1000) {};
inline unsigned int MCSat::numIterations() const {return numIterations_;}
inline void MCSat::setNumIterations(unsigned int num) {numIterations_ = num;}

template<typename URNG>
void MCSat::run(const Domain& d, URNG& rng) {
    // first, use the default model as the initial model
    //Model initialmodel = d.defaultModel();

    Model currModel = d.defaultModel();
    for (unsigned int i = 0; i < numIterations_; i++ ) {
        currModel = performIteration(currModel, d, rng);
    }

    throw std::runtime_error("MCSat::run not implemented");
}

template<typename URNG>
Model MCSat::performIteration(const Model& m, const Domain& d, URNG& rng) const {
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

    throw std::runtime_error("MCSat::performIteration not implemented");
}


#endif /* MCSAT_H_ */
