/*
 * liquidop.cpp
 *
 *  Created on: Jan 30, 2012
 *      Author: joe
 */

#include "liquidop.h"

void LiquidOp::doToString(std::stringstream& str) const {
    str << "[ ";
    if (s_ != NULL) {
        str << s_->toString();
    }
    str << " ]";
};
