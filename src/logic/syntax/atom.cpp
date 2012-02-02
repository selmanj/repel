/*
 * atom.cpp
 *
 *  Created on: Nov 27, 2011
 *      Author: joe
 */

#include "atom.h"
#include "term.h"
#include <boost/ptr_container/ptr_vector.hpp>

bool Atom::isGrounded() const {
    for (boost::ptr_vector<Term>::const_iterator it = terms.begin(); it != terms.end(); it++) {
        if (dynamic_cast<const Constant*>(&(*it)) == NULL) return false;
    }
    return true;
}

bool Atom::doEquals(const Sentence& t) const {
    const Atom *at = dynamic_cast<const Atom*>(&t);
    if (at == NULL) {
        return false;
    }

    return (pred == at->pred)
            && (terms == at->terms);
}

void Atom::doToString(std::stringstream& str) const {
    str << pred << "(";
    for (boost::ptr_vector<Term>::const_iterator it = terms.begin();
            it != terms.end();
            it++) {
        const Term *t = &(*it);

        str << t->toString();
        if (it + 1 != terms.end()) {
            str << ", ";
        }
    }
    str << ")";
};
