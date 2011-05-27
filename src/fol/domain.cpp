#include "domain.h"
#include "fol.h"
#include "atom.h"
#include <boost/shared_ptr.hpp>
#include <stdexcept>

bool Domain::isLiquid(const std::string& predicate) const {
	// TODO: implement this sometime!
	return true;
}

SISet Domain::satisfied(const boost::shared_ptr<const Sentence>& s, const Model& m) const {
	// start with the base case
	boost::shared_ptr<const Atom> a = boost::shared_dynamic_cast<const Atom>(s);
	if (a.get() != 0) {
		if (a->isGrounded()) {
			// make sure its in model
			if (m.find(*a) != m.end()){
				return m.at(*a);
			} else {
				// return an empty SISet, liquid or not depending on a
				if (isLiquid(a->name())) {
					return SISet(true);
				}
				return SISet(false);
			}
		}
	}
	// negation
	boost::shared_ptr<const Negation> n = boost::shared_dynamic_cast<const Negation>(s);
	if (n.get() != 0) {
		// return the compliment of the set inside the negation
		SISet sat = satisfied(n->sentence(), m);
		return sat.compliment();
	}
	// made it this far, unimplemented!
	std::runtime_error e("Domain::satisfied not implemented yet!");
	throw e;
}
