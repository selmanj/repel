/*
 * LiquidOp.cpp
 *
 *  Created on: Jan 30, 2012
 *      Author: joe
 */

#include "LiquidOp.h"

void LiquidOp::doToString(std::stringstream& str) const {
    str << "[ ";
    if (s_ != NULL) {
        str << s_->toString();
    }
    str << " ]";
};

SISet LiquidOp::satisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    SISet set = s_->satisfied(m, d, true); // override value of forceLiquid
    set.setForceLiquid(forceLiquid);
    return set;
}
