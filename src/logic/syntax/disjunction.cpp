/*
 * disjunction.cpp
 *
 *  Created on: Jan 30, 2012
 *      Author: joe
 */
#include "disjunction.h"

void Disjunction::doToString(std::stringstream& str) const {
    if (left_->precedence() > precedence()) {
        str << "(";
        str << left_->toString();
        str << ")";
    } else {
        str << left_->toString();
    }
    str << " v ";
    if (right_->precedence() > precedence()) {
        str << "(";
        str << right_->toString();
        str << ")";
    } else {
        str << right_->toString();
    }
};

SISet Disjunction::doSatisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    SISet leftSat = left_->satisfied(m, d, forceLiquid);
    SISet rightSat = right_->satisfied(m, d, forceLiquid);
    leftSat.add(rightSat);
    return leftSat;
}
