#ifndef MOVES_H
#define MOVES_H

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <istream>
#include <cstdlib>
#include <algorithm>
#include <iterator>

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
	Move findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s);
	Move findMovesForLiquidConjunction(const Domain& d, const Model& m, const Conjunction &c);
	std::vector<Move> findMovesForLiquidDisjunction(const Domain& d, const Model& m, const Disjunction &dis);
	std::vector<Move> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s);

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
		// find an interval to satisfy
		SISet sat = d.liqSatisfiedConjunction(c, m);
		sat.setForceLiquid(true);
		sat = sat.compliment();

		if (sat.size() == 0) return move;
		int idx = rand() % sat.set().size();
		SpanInterval si = set_at(sat.set(), idx);

		// we can only have literals in our conjunction!  collect them
		class LiteralCollector : public SentenceVisitor {
		public:
			std::vector<const Sentence*> lits;

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
					// the last literal we inserted is actually negation - pop it off the end
					lits.pop_back();
					lits.push_back(&s);
				} else {

					lits.push_back(&s);
				}
			}
		} litCollector;

		c.left()->visit(litCollector);
		c.right()->visit(litCollector);

		// find all the moves for each literal and combine it into one big move
		BOOST_FOREACH(const Sentence *s, litCollector.lits) {
			if (dynamic_cast<const Negation*>(s)) {
				const Negation* n = dynamic_cast<const Negation*>(s);
				const Atom* a = dynamic_cast<const Atom*>(&(*n->sentence()));
				Move::change change = boost::make_tuple(*a, si);
				move.toDel.push_back(change);
			} else {
				const Atom* a = dynamic_cast<const Atom*>(s);
				Move::change change = boost::make_tuple(*a, si);
				move.toAdd.push_back(change);
			}
		}
		return move;
	}

	std::vector<Move> findMovesForLiquidDisjunction(const Domain& d, const Model& m, const Disjunction &dis) {
		std::vector<Move> moves;
		std::vector<Move> movesL = findMovesForLiquid(d, m, *dis.left());
		std::vector<Move> movesR = findMovesForLiquid(d, m, *dis.right());

		moves.insert(moves.end(), movesL.begin(), movesL.end());
		moves.insert(moves.end(), movesR.begin(), movesR.end());

		return moves;
	}

	std::vector<Move> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s) {
		std::vector<Move> moves;
		if (dynamic_cast<const Negation *>(&s) || dynamic_cast<const Atom *>(&s))
			moves.push_back(findMovesForLiquidLiteral(d, m, s));
		else if (dynamic_cast<const Conjunction *>(&s)) {
			const Conjunction* c = dynamic_cast<const Conjunction *>(&s);
			moves.push_back(findMovesForLiquidConjunction(d, m, *c));
		} else if (dynamic_cast<const Disjunction *>(&s)) {
			const Disjunction* dis = dynamic_cast<const Disjunction *>(&s);
			std::vector<Move> disMoves = findMovesForLiquidDisjunction(d, m, *dis);
			moves.insert(moves.end(), disMoves.begin(), disMoves.end());
		}
		return moves;	// empty move
	}
}

std::vector<Move> findMovesFor(const Domain& d, const Model& m, const Sentence &s);
#endif
