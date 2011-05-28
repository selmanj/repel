#include "domain.h"
#include "fol.h"
#include "atom.h"
#include <boost/shared_ptr.hpp>
#include <stdexcept>

bool Domain::isLiquid(const std::string& predicate) const {
	// TODO: implement this sometime!
	return true;
}

SISet Domain::satisfied(const Sentence& s, const Model& m) const {
	// start with the base case
	const Atom* a = dynamic_cast<const Atom *>(&s);
	if (a != 0) return satisfiedAtom(*a, m);
	// negation
	const Negation* n = dynamic_cast<const Negation*>(&s);
	if (n != 0) return satisfiedNegation(*n, m);
	// made it this far, unimplemented!
	std::runtime_error e("Domain::satisfied not implemented yet!");
	throw e;
}

SISet Domain::satisfiedAtom(const Atom& a, const Model& m) const {
	if (a.isGrounded()) {
		// make sure its in model
		if (m.find(a) != m.end()){
			SISet set = m.at(a);
			// force non-liquidity
			set.setForceLiquid(false);
			return set;
		} else {
			return SISet(false);
		}
	}
	// grounding out of atoms currently not implemented yet!
	std::runtime_error e("Domain::satisfiedAtom grounding out of atoms not implemented!");
	throw e;
}

SISet Domain::satisfiedNegation(const Negation& n, const Model& m) const {
	// return the compliment of the set inside the negation
	SISet sat = satisfied(*(n.sentence()), m);

	return sat.compliment();
}
