#include "moves.h"

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <stdexcept>
#include "fol.h"
#include "domain.h"
#include "sentence.h"

Moves findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s) {

}

Moves findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s) {
	if (dynamic_cast<const Atom*>(&s) || dynamic_cast<const Negation*>(&s)) {
		const Atom* a;
		bool isNegation=false;
		if (dynamic_cast<const Atom*>(&s)) {
			a = dynamic_cast<const Atom *>(&s);
		} else {
			const Negation* n = dynamic_cast<const Negation *>(&s);
			isNegation = true;
			a = dynamic_cast<const Atom *>(&(*n->sentence()));
			if (!a) {
				throw std::runtime_error("negation applied to something that is not an atom!");
			}
		}
		if (!a->isGrounded()) {
			throw std::runtime_error("cannot handle atoms with variables at the moment!");
		}
		// now that those checks are out of the way, find where it's not satisfied
		SISet sat = d.satisfied(s, m);
		sat = sat.compliment();	// compliment is where its not satisfied at
		// Our moves are single span intervals, so split them out into a list of moves


	}

	// find where it's satisfied

}

boost::optional<Moves> findMovesFor(const Domain& d, const Model& m, const Sentence &s) {
	if (dynamic_cast<const LiquidOp*>(&s)) {
		const LiquidOp* liq = dynamic_cast<const LiquidOp*>(&s);
		return findMovesFor(d, m, *(liq->sentence()));
	}
	return boost::optional<Moves>();
}
