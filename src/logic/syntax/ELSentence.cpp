/*
 * ELSentence.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: joe
 */

#include "ELSentence.h"
#include "../Domain.h"

bool operator==(const ELSentence& a, const ELSentence& b) {
    if (*a.s_ != *b.s_) return false;
    if (a.hasInfWeight_ != b.hasInfWeight_) return false;
    if (!a.hasInfWeight_ && a.w_ != b.w_) return false;
    if ((a.quantification_ == 0 && b.quantification_ != 0)
            || (a.quantification_ == 0 && b.quantification_ != 0)) return false;
    if (a.quantification_ != 0 && b.quantification_ != 0 && *a.quantification_ != *b.quantification_) return false;
    return true;
};

std::string ELSentence::toString() const {
    std::stringstream str;
    str << *this;
    return str.str();
};

SISet ELSentence::dSatisfied(const Model& m, const Domain& d) const {
    if (isQuantified()) return s_->dSatisfied(m, d, *quantification_);
    else return s_->dSatisfied(m, d);
}

SISet ELSentence::dNotSatisfied(const Model& m, const Domain& d) const {
    if (isQuantified()) return s_->dNotSatisfied(m, d, *quantification_);
    else return s_->dNotSatisfied(m, d);
}

bool ELSentence::fullySatisfied(const Model& m, const Domain& d) const {
    SISet satisfiedAt = dSatisfied(m, d);
    SISet toSatisfyAt(false, d.maxInterval());
    if (quantification_ != 0) {
        toSatisfyAt = *quantification_;
    } else {
        toSatisfyAt = SISet(d.maxSpanInterval(), false, d.maxInterval());
    }

    toSatisfyAt.subtract(satisfiedAt);
    if (toSatisfyAt.empty()) return true;
    return false;
}
