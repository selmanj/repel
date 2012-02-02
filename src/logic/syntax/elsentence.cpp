/*
 * elsentence.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: joe
 */

#include "elsentence.h"

bool ELSentence::operator==(const ELSentence& b) const {
    if (*s_ != *b.s_) return false;
    if (hasInfWeight_ != b.hasInfWeight_) return false;
    if (!hasInfWeight_ && w_ != b.w_) return false;
    if (isQuantified_ != b.isQuantified_) return false;
    if (isQuantified_ && quantification_ != b.quantification_) return false;
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
