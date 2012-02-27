/*
 * elsentence.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: joe
 */

#include "elsentence.h"
#include "../domain.h"

bool operator==(const ELSentence& a, const ELSentence& b) {
    if (*a.s_ != *b.s_) return false;
    if (a.hasInfWeight_ != b.hasInfWeight_) return false;
    if (!a.hasInfWeight_ && a.w_ != b.w_) return false;
    if (a.isQuantified_ != b.isQuantified_) return false;
    if (a.isQuantified_ && a.quantification_ != b.quantification_) return false;
    return true;
};

std::string ELSentence::toString() const {
    std::stringstream str;

    if (hasInfWeight_) {
        str << "inf: ";
    } else {
        str << w_ << ": ";
    }
    str << s_->toString();
    if (isQuantified_) {
        str << " @ " << quantification_.toString();
    }

    return str.str();
};

SISet ELSentence::dSatisfied(const Model& m, const Domain& d) const {
    if (isQuantified()) return s_->dSatisfied(m, d, quantification_);
    else return s_->dSatisfied(m, d);
}

SISet ELSentence::dNotSatisfied(const Model& m, const Domain& d) const {
    if (isQuantified()) return s_->dNotSatisfied(m, d, quantification_);
    else return s_->dNotSatisfied(m, d);
}

bool ELSentence::fullySatisfied(const Model& m, const Domain& d) const {
    SISet satisfiedAt = dSatisfied(m, d);
    SISet toSatisfyAt;
    if (isQuantified_) {
        toSatisfyAt = quantification_;
    } else {
        toSatisfyAt = SISet(d.maxSpanInterval(), false);
    }

    toSatisfyAt.subtract(satisfiedAt);
    if (toSatisfyAt.empty()) return true;
    return false;
}
