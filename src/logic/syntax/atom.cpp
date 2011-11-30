/*
 * atom.cpp
 *
 *  Created on: Nov 27, 2011
 *      Author: joe
 */

#include "atom.h"

bool Atom::isGrounded() const {
	for (boost::ptr_vector<Term>::const_iterator it = terms.begin(); it != terms.end(); it++) {
		if (dynamic_cast<const Constant*>(&(*it)) == NULL) return false;
	}
	return true;
};

Atom& Atom::operator=(const Atom& b) {
	pred = b.pred;
	terms = b.terms;
	return *this;
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
			str << it->toString();
			if (it + 1 != terms.end()) {
				str << ", ";
			}
		}
		str << ")";
	};
