/*
 * mcsat.cpp
 *
 *  Created on: Feb 8, 2012
 *      Author: selman.joe@gmail.com
 */

#include "mcsat.h"
#include "../logic/syntax/elsentence.h"

void MCSat::run() {
    if (d_ == 0) {
        throw std::logic_error("MCSat::run() - Domain not set");
    }

    // first, run unit propagation on our domain to get a new reduced one.
    // note that we don't catch the possible unsatisfiability exception, since
    // we can't really recover from that.
    // TODO: handle it somehow?
    Domain reduced = performUnitPropagation(*d_);

    // default model is guaranteed to satisfy the facts
    Model m = reduced.defaultModel();
    // check to make sure hard clauses are satisfied
    std::vector<ELSentence> hardClauses;
    std::remove_copy_if(reduced.formulas_begin(), reduced.formulas_end(), std::back_inserter(hardClauses), std::not1(IsHardClausePred()));

    throw std::runtime_error("MCSat::run() unimplemented.");
}
