/*
 * mcsat.cpp
 *
 *  Created on: Feb 8, 2012
 *      Author: selman.joe@gmail.com
 */

#include <stdexcept>
#include "mcsat.h"

void MCSat::run(const Domain& d) {
    // first, use the default model as the initial model
    //Model initialmodel = d.defaultModel();

    Model currModel = d.defaultModel();
    for (unsigned int i = 0; i < numIterations_; i++ ) {
        currModel = performIteration(currModel, d);
    }

    throw std::runtime_error("MCSat::run not implemented");
}

Model MCSat::performIteration(const Model& m, const Domain& d) const {
    // first, select a subset of the rules in the domain to enforce as hard,
    // based on their weight
    for(FormulaList::const_iterator it = d.formulas().begin(); it != d.formulas().end(); it++) {
        ELSentence curForm = *it;
        SISet where = (curForm.isQuantified() ? curForm.quantification() : SISet(d.maxSpanInterval(), false, d.maxInterval()));
        // find where it's satisfied
        SISet trueAt = d.satisfied(curForm.sentence(), m, (curForm.isQuantified() ? &curForm.quantification() : NULL));

    }

    throw std::runtime_error("MCSat::performIteration not implemented");
}
