/*
 * NameGenerator.cpp
 *
 *  Created on: Jun 27, 2011
 *      Author: joe
 */
#include <string>
#include <sstream>
#include "NameGenerator.h"

NameGenerator::NameGenerator() : counter_(0) {}

std::string NameGenerator::getUniqueName() {
    std::ostringstream stream;
    stream << NAME_PREFIX << counter_++;
    return stream.str();
}
