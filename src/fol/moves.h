#ifndef MOVES_H
#define MOVES_H

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <istream>
#include <cstdlib>

#include "atom.h"
#include "../siset.h"
#include "domain.h"
#include "sentence.h"
#include "../utils.h"

class LiquidOp;
class Sentence;

struct Move {
	typedef boost::tuple <Atom, SpanInterval> change;
	std::vector<change> toAdd;
	std::vector<change> toDel;

	std::string toString() const;
};

namespace {
	Move findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s) {
		Move move;
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
			// Our moves are single span intervals, so choose one randomly
			if (isNegation) {
				// we want to delete span intervals where its true

				if (sat.size() == 0) return move;

				int idx = rand() % sat.set().size();
				SpanInterval si = set_at(sat.set(), idx);
				Move::change change = boost::make_tuple(*a, si);
				move.toDel.push_back(change);
			} else {
				// work with the compliment
				SISet satComp = sat.compliment();
				if (satComp.size() == 0) return move;
				// we want to add span intervals where its false
				int idx = rand() % satComp.set().size();
				SpanInterval si = set_at(satComp.set(), idx);
				Move::change change = boost::make_tuple(*a, si);
				move.toAdd.push_back(change);
			}
		}
		return move;
	}

	Move findMovesForLiquidConjunction(const Domain& d, const Model& m, const Conjunction &c) {
		Move move;
		// we can only have literals in our conjunction!  collect them
		class LiteralCollector : public SentenceVisitor {
		public:
			LiteralCollector() : ignoreNextLiteral(false) {};
			std::vector<const Sentence*> lits;
			bool ignoreNextLiteral;

			void accept(const Sentence& s) {
				if (!dynamic_cast<const Negation*>(&s) && !dynamic_cast<const Atom*>(&s) ) {
					throw std::runtime_error("must have only literals inside conjunction when finding moves!");
				}
				if (dynamic_cast<const Negation*>(&s)) {
					const Negation* n = dynamic_cast<const Negation*>(&s);
					// ensure we are being applied to a literal
					if (!dynamic_cast<const Atom*>(&(*n->sentence()))) {
						throw std::runtime_error("negation can only be applied to an atom when finding moves");
					}
					lits.push_back(&s);
					ignoreNextLiteral = true;
				} else {
					if (ignoreNextLiteral) {
						ignoreNextLiteral = false;
						return;
					}
					lits.push_back(&s);
				}
			}
		} litCollector;

		c.visit(litCollector);


	}

	Move findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s) {
		return findMovesForLiquidLiteral(d, m, s);
	}
}

Move findMovesFor(const Domain& d, const Model& m, const Sentence &s);
#endif
