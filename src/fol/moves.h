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
	bool isEmpty() const;
};

namespace {
	Move findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si);
	Move findMovesForLiquidConjunction(const Domain& d, const Model& m, const Conjunction &c, const SpanInterval &si);
	std::vector<Move> findMovesForLiquidDisjunction(const Domain& d, const Model& m, const Disjunction &dis, const SpanInterval &si);
	std::vector<Move> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s, const SpanInterval &si);

	Move findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval &si) {
		Move move;
		if (si.size() == 0) return move;

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
			if (d.dontModifyObsPreds()
					&& d.observedPredicates().find(a->name()) != d.observedPredicates().end()) {
				// this predicate is an observed predicate; we can't change it.  return an empty move
				return move;
			}
			if (isNegation) {
				// we want to delete span intervals where its true
				Move::change change = boost::make_tuple(*a, si);
				move.toDel.push_back(change);
			} else {
				// we want to add span intervals where its false
				Move::change change = boost::make_tuple(*a, si);
				move.toAdd.push_back(change);
			}
		}
		return move;
	}

	Move findMovesForLiquidConjunction(const Domain& d, const Model& m, const Conjunction &c, const SpanInterval& si) {
		Move move;

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

	std::vector<Move> findMovesForLiquidDisjunction(const Domain& d, const Model& m, const Disjunction &dis, const SpanInterval& si) {
		std::vector<Move> moves;
		std::vector<Move> movesL = findMovesForLiquid(d, m, *dis.left(), si);
		std::vector<Move> movesR = findMovesForLiquid(d, m, *dis.right(), si);

		moves.insert(moves.end(), movesL.begin(), movesL.end());
		moves.insert(moves.end(), movesR.begin(), movesR.end());

		return moves;
	}

	std::vector<Move> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si) {
		std::vector<Move> moves;
		if (dynamic_cast<const Negation *>(&s) || dynamic_cast<const Atom *>(&s)) {
			Move move = findMovesForLiquidLiteral(d, m, s, si);
			if (!move.isEmpty()) moves.push_back(move);
		} else if (dynamic_cast<const Conjunction *>(&s)) {
			const Conjunction* c = dynamic_cast<const Conjunction *>(&s);
			Move move = findMovesForLiquidConjunction(d, m, *c, si);
			if (!move.isEmpty()) moves.push_back(move);
		} else if (dynamic_cast<const Disjunction *>(&s)) {
			const Disjunction* dis = dynamic_cast<const Disjunction *>(&s);
			std::vector<Move> disMoves = findMovesForLiquidDisjunction(d, m, *dis, si);
			moves.insert(moves.end(), disMoves.begin(), disMoves.end());
		}
		return moves;
	}
}

bool canFindMovesFor(const Sentence &s, const Domain &d);
bool isFormula1Type(const Sentence &s, const Domain &d);
bool isFormula2Type(const Sentence &s, const Domain &d);
bool isFormula3Type(const Sentence &s, const Domain &d);
std::vector<Move> findMovesFor(const Domain& d, const Model& m, const Sentence &s);
std::vector<Move> findMovesForForm1(const Domain& d, const Model& m, const Disjunction &dis);
std::vector<Move> findMovesForForm2(const Domain& d, const Model& m, const Disjunction &dis);
std::vector<Move> findMovesForForm3(const Domain& d, const Model& m, const Disjunction &dis);

Model executeMove(const Domain& d, const Move& move, const Model& model);
Model maxWalkSat(const Domain& d, int numIterations, double probOfRandomMove, const Model* initialModel=0);

std::vector<WSentence> convertToPELCNF(const std::vector<WSentence>, const Domain &d);
#endif
