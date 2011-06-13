#include "moves.h"

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <stdexcept>
#include "fol.h"
#include "domain.h"
#include "sentence.h"

boost::optional<Moves> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s) {
	return boost::optional<Moves>();
}

boost::optional<Moves> findMovesForLiquidAtom(const Domain& d, const Model& m, const Atom &a) {
	if (!a.isGrounded()) {
		throw std::runtime_error("cannot handle atoms with variables at the moment!");
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
