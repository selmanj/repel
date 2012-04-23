/*
 * Negation.cpp
 *
 *  Created on: Jan 30, 2012
 *      Author: joe
 */

#include "Negation.h"

void Negation::doToString(std::stringstream& str) const {
    str << "!";
    if (s_ != NULL) {
        if (s_->precedence() > precedence()) {
            str << "(";
            str << s_->toString();
            str << ")";
        } else {
            str << s_->toString();
        }
    }
}

SISet Negation::satisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    // return the compliment of the set inside the negation
    SISet sat = s_->satisfied(m, d, forceLiquid);
    sat.setForceLiquid(forceLiquid);
    return sat.compliment();
}


