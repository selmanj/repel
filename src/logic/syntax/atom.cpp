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

Atom::term_const_iterator Atom::term_begin() const {
    return terms.begin();
}

Atom::term_const_iterator Atom::term_end() const {
    return terms.end();
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
            return SISet(forceLiquid, d.maxInterval());
        }
    }
    // grounding out of atoms currently not implemented yet!
    std::runtime_error e("Domain::satisfiedAtom grounding out of atoms not implemented!");
    throw e;
}

bool AtomStringCompare::operator()(const Atom& a, const Atom& b) const {
    if (a.name() < b.name()) return true;
    if (a.name() > b.name()) return false;

    Atom::term_const_iterator aIt = a.term_begin();
    Atom::term_const_iterator bIt = b.term_begin();
    bool reachedEndOfA = (aIt == a.term_end());
    bool reachedEndOfB = (bIt == b.term_end());
    while (!reachedEndOfA && !reachedEndOfB) {
        if (aIt->name() < bIt->name()) return true;
        if (aIt->name() > bIt->name()) return false;
        aIt++;
        bIt++;
        reachedEndOfA = (aIt == a.term_end());
        reachedEndOfB = (bIt == b.term_end());
    }
    if (reachedEndOfA) return true; // if a is shorter (or they're the same) return true
    return false;   // must have reached end of b
}

