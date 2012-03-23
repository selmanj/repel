/*
 * predicatetype.cpp
 *
 *  Created on: Mar 22, 2012
 *      Author: joe
 */
#include "predicatetype.h"
#include <boost/functional/hash.hpp>

PredicateType::PredicateType() : name_(), arity_(0) {};
PredicateType::PredicateType(const std::string& predname, unsigned int numArgs)
        : name_(predname), arity_(numArgs) {}

std::string PredicateType::name() const {
    return name_;
}

unsigned int PredicateType::arity() const {
    return arity_;
}

std::size_t hash_value(const PredicateType& t) {
    std::size_t seed = 0;
    boost::hash_combine(seed, t.name_);
    boost::hash_combine(seed, t.arity_);
    return seed;
}

bool operator==(const PredicateType& l, const PredicateType& r) {
    return (l.name_ == r.name_) && (l.arity_ == r.arity_);
}
bool operator!=(const PredicateType& l, const PredicateType& r) {
    return !operator==(l, r);
}
