/*
 * atom.cpp
 *
 *  Created on: Nov 27, 2011
 *      Author: joe
 */

#include "atom.h"
#include "term.h"
#include "../domain.h"
#include "../model.h"
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
    str << *this;
};

std::ostream& operator<<(std::ostream& out, const Atom& a) {
    out << a.pred << "(";
    for (boost::ptr_vector<Term>::const_iterator it = a.terms.begin();
            it != a.terms.end();
            it++) {
        const Term& t = *it;

        out << t.toString();
        if (it + 1 != a.terms.end()) {
            out << ", ";
        }
    }
    out << ")";
    return out;
}

SISet Atom::satisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    if (isGrounded()) {
        // make sure its in model
        if (m.hasAtom(*this)){
            SISet set = m.getAtom(*this);
            set.setForceLiquid(forceLiquid);
            return set;
        } else {
            return SISet(forceLiquid);
        }
    }
    // grounding out of atoms currently not implemented yet!
    std::runtime_error e("Domain::satisfiedAtom grounding out of atoms not implemented!");
    throw e;
}
