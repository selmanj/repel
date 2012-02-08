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
    Model initialmodel = d.defaultModel();

    throw std::runtime_error("MCSat::run not implemented");
}
