#ifndef MOVES_H
#define MOVES_H

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <istream>

#include "atom.h"
#include "../siset.h"
#include "domain.h"

class LiquidOp;
class Sentence;

struct Moves {
	typedef boost::tuple <Atom, SpanInterval> move;
	std::vector<move> toAdd;
	std::vector<move> toDel;

	std::string toString() const;
};

namespace {
	Moves findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s) {
		Moves moves;
		if (dynamic_cast<const Atom*>(&s) || dynamic_cast<const Negation*>(&s)) {
			const Atom* a;
			bool isNegation=false;
			if (dynamic_cast<const Atom*>(&s)) {
				a = dynamic_cast<const Atom *>(&s);
			} else {
				const Negation* n = dynamic_cast<const Negation *>(&s);
				isNegation = true;
				a = dynamic_cast<const Atom *>(&(*n->sentence()));		// joseph!  this is ugly!  TODO fix it
				if (!a) {
					throw std::runtime_error("negation applied to something that is not an atom!");
				}
			}
			if (!a->isGrounded()) {
				throw std::runtime_error("cannot handle atoms with variables at the moment!");
			}
			// now that those checks are out of the way, find where it's not satisfied
			SISet sat = d.satisfied(*a, m);
			// ensure sat stays liquid
			sat.setForceLiquid(true);
			// Our moves are single span intervals, so split them out into a list of moves
			if (isNegation) {
				// we want to delete span intervals where its true
				BOOST_FOREACH(SpanInterval sp, sat.set()) {
					Moves::move aMove = boost::make_tuple(*a, sp);
					moves.toDel.push_back(aMove);
				}
			} else {
				// work with the compliment
				SISet satComp = sat.compliment();
				// we want to add span intervals where its false
				BOOST_FOREACH(SpanInterval sp, satComp.set()) {
					Moves::move aMove = boost::make_tuple(*a, sp);
					moves.toAdd.push_back(aMove);
				}
			}
		}
		return moves;
	}

	Moves findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s) {
		return findMovesForLiquidLiteral(d, m, s);
	}
}

Moves findMovesFor(const Domain& d, const Model& m, const Sentence &s);
#endif
