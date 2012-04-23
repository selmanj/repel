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

SISet Disjunction::satisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    SISet leftSat = left_->satisfied(m, d, forceLiquid);
    SISet rightSat = right_->satisfied(m, d, forceLiquid);
    if (!forceLiquid) {
        leftSat.setForceLiquid(false);
        rightSat.setForceLiquid(false);
    } else {
        leftSat.setForceLiquid(true);
        rightSat.setForceLiquid(true);
    }
    leftSat.add(rightSat);
    return leftSat;
}
