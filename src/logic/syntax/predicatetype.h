/*
 * predicatetype.h
 *
 *  Created on: Mar 22, 2012
 *      Author: joe
 */

#ifndef PREDICATETYPE_H_
#define PREDICATETYPE_H_

#include <string>

/**
 * Class describing the type of an atom.  Right now its just the predicate name and
 * number of arguments.
 */
class PredicateType {
public:
    PredicateType();
    PredicateType(const std::string& predname, unsigned int numArgs);

    std::string name() const;
    unsigned int arity() const;

    friend std::size_t hash_value(const PredicateType& t);

    friend bool operator==(const PredicateType& l, const PredicateType& r);
    friend bool operator!=(const PredicateType& l, const PredicateType& r);
private:
    std::string name_;
    unsigned int arity_;
};


#endif /* PREDICATETYPE_H_ */
