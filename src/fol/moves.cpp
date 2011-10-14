#include "moves.h"

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "fol.h"
#include "domain.h"
#include "sentence.h"
#include "model.h"
#include "../siset.h"
#include "../log.h"

std::string Move::toString() const {
	std::stringstream str;

	str << "toAdd: {";
	for (std::vector<Move::change>::const_iterator it = toAdd.begin(); it != toAdd.end(); it++){
		if (it != toAdd.begin()) str << ", ";
		str << it->get<0>().toString() << " @ " << it->get<1>().toString();
	}
	str << "}, ";

	str << "toDel: {";
	for (std::vector<Move::change>::const_iterator it = toDel.begin(); it != toDel.end(); it++){
		if (it != toDel.begin()) str << ", ";
		str << it->get<0>().toString() << " @ " << it->get<1>().toString();
	}
	str << "}";

	return str.str();
}

bool Move::isEmpty() const {
	return (toAdd.size() == 0 && toDel.size() == 0);
}

bool canFindMovesFor(const Sentence &s, const Domain &d) {
	if (dynamic_cast<const LiquidOp *>(&s)) {
		return true;		// this isn't really fair: TODO write a better test on liquid ops
	} else if (isFormula1Type(s, d)) {
		return true;
	} else if (isFormula2Type(s, d)) {
		return true;
	} else if (isFormula3Type(s, d)) {
		return true;
	} else if (isPELCNFLiteral(s) || isDisjunctionOfCNFLiterals(s)) {
		return true;
	}
	return false;
}

bool isFormula1Type(const Sentence &s, const Domain &d) {
	// ensure it's a disjunction
	const Disjunction *disjunc = dynamic_cast<const Disjunction *>(&s);
	if (!disjunc) return false;
	// ensure one side is a negation and the other side is not
	const Sentence *consequent;
	const Negation *precedent;
	if (dynamic_cast<const Negation*>(&*disjunc->left()) && !dynamic_cast<const Negation*>(&*disjunc->right())) {
		precedent = dynamic_cast<const Negation*>(&*disjunc->left());
		consequent = &*disjunc->right();
	} else if (!dynamic_cast<const Negation*>(&*disjunc->left()) && dynamic_cast<const Negation*>(&*disjunc->right())) {
		precedent = dynamic_cast<const Negation*>(&*disjunc->right());
		consequent = &*disjunc->left();
	} else {
		return false;
	}

	// ensure precedent is either liquid or a liq atom (or negation)
	if (!dynamic_cast<const LiquidOp*>(&*precedent->sentence())) {
		// its ok!  it could be an atom that is liquid
		const Atom *atom = dynamic_cast<const Atom*>(&*precedent->sentence());
		if (atom && !d.isLiquid(atom->name())) return false;
		else if (!atom) return false;
	}

	// now make sure the consequent is diamond operator applied to a liq item
	const DiamondOp *diamondop = dynamic_cast<const DiamondOp *>(consequent);
	if (!diamondop) return false;
	const LiquidOp *liqCons = dynamic_cast<const LiquidOp*>(&*diamondop->sentence());
	if (!liqCons) return false;
	const Disjunction *disCons = dynamic_cast<const Disjunction*>(&*liqCons->sentence());
	if (!disCons) return false;

	// now make sure the precedent appears in the body somewhere
	std::vector<const Sentence *> args = getDisjunctionArgs(*disCons);
	BOOST_FOREACH(const Sentence *subS, args) {
		if (*subS == *precedent->sentence()) return true;
	}
	return false;
}

bool isFormula2Type(const Sentence &s, const Domain &d) {
	// ensure it's a disjunction
	const Disjunction *disjunc = dynamic_cast<const Disjunction *>(&s);
	if (!disjunc) return false;
	// ensure one side is a negation and the other side is not
	const Sentence *consequent;
	const Negation *precedent;
	if (dynamic_cast<const Negation*>(&*disjunc->left()) && !dynamic_cast<const Negation*>(&*disjunc->right())) {
		precedent = dynamic_cast<const Negation*>(&*disjunc->left());
		consequent = &*disjunc->right();
	} else if (!dynamic_cast<const Negation*>(&*disjunc->left()) && dynamic_cast<const Negation*>(&*disjunc->right())) {
		precedent = dynamic_cast<const Negation*>(&*disjunc->right());
		consequent = &*disjunc->left();
	} else {
		return false;
	}

	// ensure precedent is a conjunction of meets
	const Conjunction *precConj = dynamic_cast<const Conjunction*>(&*precedent->sentence());
	if (!precConj) {
		return false;
	}
	std::set<Interval::INTERVAL_RELATION> justMeets;
	justMeets.insert(Interval::MEETS);
	if (precConj->relations() != justMeets) return false;

	std::vector<const Sentence *> conjArgs = getMeetsConjunctionArgs(*precConj);
	if (conjArgs.size() < 2) return false;
	const Sentence *phik = conjArgs.back();

	// now make sure the consequent is diamond operator applied to a liq item
	const DiamondOp *diamondop = dynamic_cast<const DiamondOp *>(consequent);
	if (!diamondop) return false;
	const LiquidOp *liqCons = dynamic_cast<const LiquidOp*>(&*diamondop->sentence());
	if (!liqCons) return false;
	const Disjunction *disCons = dynamic_cast<const Disjunction*>(&*liqCons->sentence());
	if (!disCons) return false;

	// now make sure that phik appears in the body somewhere
	std::vector<const Sentence *> args = getDisjunctionArgs(*disCons);
	BOOST_FOREACH(const Sentence *subS, args) {
		if (*subS == *phik) return true;
	}
	return false;
}

bool isFormula3Type(const Sentence &s, const Domain &d) {
	// ensure it's a disjunction
	const Disjunction *disjunc = dynamic_cast<const Disjunction *>(&s);
	if (!disjunc) return false;

	const Negation *leftAsNeg = dynamic_cast<const Negation *>(&*disjunc->left());
	const Negation *rightAsNeg = dynamic_cast<const Negation *>(&*disjunc->right());
	if (!leftAsNeg || !rightAsNeg) return false;
	// ensure both sides are negation
	const Negation *consequent;
	const Negation *precedent;
	// consequent has the conjunction subtype
	if (dynamic_cast<const Conjunction *>(&*leftAsNeg->sentence())
			&& (dynamic_cast<const LiquidOp *>(&*rightAsNeg->sentence()) || dynamic_cast<const Atom*>(&*rightAsNeg->sentence()))) {
		precedent = rightAsNeg;
		consequent = leftAsNeg;
	} else if (dynamic_cast<const Conjunction *>(&*rightAsNeg->sentence())
			&& (dynamic_cast<const LiquidOp *>(&*leftAsNeg->sentence()) || dynamic_cast<const Atom*>(&*leftAsNeg->sentence()))) {
		precedent = leftAsNeg;
		consequent = rightAsNeg;
	} else {
		return false;
	}

	// ensure precedent is either liquid or a liq atom (or negation)
	if (!dynamic_cast<const LiquidOp*>(&*precedent->sentence())) {
		// its ok!  it could be an atom that is liquid
		const Atom *atom = dynamic_cast<const Atom*>(&*precedent->sentence());
		if (atom && !d.isLiquid(atom->name())) return false;
		else if (!atom) return false;
	}

	// now ensure consequent is a meets conjunction
	const Conjunction *consConj = dynamic_cast<const Conjunction *>(&*consequent->sentence());
	std::set<Interval::INTERVAL_RELATION> justMeets;
	justMeets.insert(Interval::MEETS);
	if (consConj->relations() != justMeets) return false;

	std::vector<const Sentence *> conjArgs = getMeetsConjunctionArgs(*consConj);
	if (conjArgs.size() < 2) return false;
	return true;
}

// TODO: not sure if below function works correctly when deleting phi1 as it may delete more than necessary... does this matter??
std::vector<Move> findMovesForForm1(const Domain& d, const Model& m, const Disjunction &dis) {
	LOG(LOG_DEBUG) << "inside findMovesForForm1() with sentence: " << dis.toString();
	std::vector<Move> moves;
	// TODO: ensure sentence s is of the proper form
	const Sentence *head;
	const Negation *body;
	if (dynamic_cast<const Negation*>(&(*dis.left()))) {
		body = dynamic_cast<const Negation*>(&(*dis.left()));
		head = &(*dis.right());
	} else {
		body = dynamic_cast<const Negation*>(&(*dis.right()));
		head = &(*dis.left());
	}

	// we will assume the head side is a diamond op applied to a liquid op applied to a disjunction
	const DiamondOp* headDia = dynamic_cast<const DiamondOp*>(head);
	if (!headDia) {
		LOG(LOG_ERROR) << "incorrect sentence given for findMovesForm1: "+ dis.toString();
		return moves;
	}
	const LiquidOp* headLiq = dynamic_cast<const LiquidOp*>(&*headDia->sentence());
	if (!headLiq) {
		LOG(LOG_ERROR) << "incorrect sentence given for findMovesForm1: "+ dis.toString();
		return moves;
	}
	const Disjunction* headDis = dynamic_cast<const Disjunction*>(&*headLiq->sentence());
	if (!headDis) {
		LOG(LOG_ERROR) << "incorrect sentence given for findMovesForm1: "+ dis.toString();
		return moves;
	}

	// get the list of items in the disjunction
	std::vector<const Sentence*> disItems = getDisjunctionArgs(*headDis);
	size_t disSizeBefore = disItems.size();
	assert(disSizeBefore >= 2);


	// finally, ensure that our body exists in the disjunction items (and remove it)
	for (std::vector<const Sentence *>::iterator it = disItems.begin(); it != disItems.end();) {
		// TODO: acquire summer grifter
		if (**it == *body->sentence()) {
			it = disItems.erase(it);
		} else {
			it++;
		}
	}

	if (disSizeBefore == disItems.size()) {
		LOG(LOG_ERROR) << "incorrect sentence given for findMovesForm1: "+ dis.toString();
		return moves;
	}

	boost::shared_ptr<Sentence> phi2;
	if (disItems.size() > 1) {
		boost::shared_ptr<Sentence> newDis = wrapInDisjunction(disItems);
		phi2 = boost::shared_ptr<Sentence>(new LiquidOp(newDis));
	} else {
		boost::shared_ptr<Sentence> newItem(disItems[0]->clone());
		phi2 = boost::shared_ptr<Sentence>(new LiquidOp(newItem));
	}
	const LiquidOp* phi2Liq = dynamic_cast<const LiquidOp*>(&*phi2);
	assert(phi2Liq);
	//boost::shared_ptr<Disjunction> disWithoutPrecedent =



	// TODO:  factor these out!  sheesh
	if (headDia->relations().find(Interval::MEETSI) != headDia->relations().end()) {
		SISet falseAt;
		{
			boost::shared_ptr<Sentence> insideDiamondSentence(headDia->sentence()->clone());
			boost::shared_ptr<DiamondOp> headDiaSingle(new DiamondOp(insideDiamondSentence, Interval::MEETSI));
			boost::shared_ptr<Sentence> bodyCopy(body->clone());
			boost::shared_ptr<Sentence> disSingle(new Disjunction(headDiaSingle, bodyCopy));
			// find where this statement is not true
			falseAt = d.satisfied(*disSingle, m).compliment();
			LOG(LOG_DEBUG) << "false at :" << falseAt.toString();
		}
		if (falseAt.size() != 0) {
			// pick a span interval at random
			SpanInterval toSatisfy = set_at(falseAt.set(), rand() % falseAt.set().size());

			unsigned int b = toSatisfy.finish().start();
			// case 1 and 2:  extend the precedent (or consequent) so that it meets with the next spot the consequent is true at
			{
				SISet phi2TrueAt = d.satisfied(*phi2, m);
				if (b != d.maxInterval().finish()) {
					// find the point after b where phi2 is true at
					SpanInterval toIntersect(b+1, d.maxInterval().finish(), b+1, d.maxInterval().finish(), d.maxInterval());
					SISet toScan(true, d.maxInterval());
					toScan.add(toIntersect);
					toScan = intersection(phi2TrueAt, toScan);

					unsigned int t;
					if (toScan.size() != 0) {
						t = set_at(toScan.set(), 0).start().start()-1;
						// satisfy precedent over that expinterval
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body->sentence(), SpanInterval(b,t,b,t,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
					// option 2, add the consequent
					if (toScan.size() == 0) {
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *phi2Liq->sentence(), SpanInterval(b,b+1,b,b+1,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					} else {
						t = set_at(toScan.set(), 0).start().start()-1;
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *phi2Liq->sentence(), SpanInterval(b,b+1,b,b+1,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
				}
				if (b != d.maxInterval().start()) {
					// option 3, delete precedent all the way until consequent is true
					SpanInterval toIntersect(d.maxInterval().start(), b, d.maxInterval().start(), b, d.maxInterval());
					SISet toScan(true, d.maxInterval());
					toScan.add(toIntersect);
					toScan = intersection(phi2TrueAt, toScan);

					unsigned int t;
					if (toScan.size() != 0) {
						t = set_at(toScan.set(), toScan.set().size()-1).finish().finish();
						// delete precedent over that interval
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body, SpanInterval(t,b,t,b,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					} else {
						// just delete it all the way to maxInterval.begin()
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body,
								SpanInterval(d.maxInterval().start(), b, d.maxInterval().start(), b, d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
				}
			}
		}
	}
	// now do meets (similar)
	if (headDia->relations().find(Interval::MEETS) != headDia->relations().end()) {
		SISet falseAt;
		{
			boost::shared_ptr<Sentence> insideDiamondSentence(headDia->sentence()->clone());
			boost::shared_ptr<DiamondOp> headDiaSingle(new DiamondOp(insideDiamondSentence, Interval::MEETS));
			boost::shared_ptr<Sentence> bodyCopy(body->clone());
			boost::shared_ptr<Sentence> disSingle(new Disjunction(headDiaSingle, bodyCopy));
			// find where this statement is not true
			falseAt = d.satisfied(*disSingle, m).compliment();
			LOG(LOG_DEBUG) << "false at :" << falseAt.toString();
		}
		if (falseAt.size() != 0) {
			// pick a span interval at random
			SpanInterval toSatisfy = set_at(falseAt.set(), rand() % falseAt.set().size());
			unsigned int b = toSatisfy.start().start();
			// case 1 and 2:  extend the precedent (or consequent) so that it meets with the next spot the consequent is true at
			{
				SISet phi2TrueAt = d.satisfied(*phi2, m);
				if (b != d.maxInterval().start()) {
					// find the point before b where phi2 is true at
					SpanInterval toIntersect(d.maxInterval().start(), b-1, d.maxInterval().start(),b-1, d.maxInterval());
					SISet toScan(true, d.maxInterval());
					toScan.add(toIntersect);
					toScan = intersection(phi2TrueAt, toScan);

					unsigned int t;
					if (toScan.size() != 0) {
						t = set_at(toScan.set(), toScan.set().size()-1).finish().finish()+1;
						// satisfy precedent over that expinterval
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body->sentence(), SpanInterval(t,b-1,t,b-1,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
					// option 2, add the consequent
					if (toScan.size() == 0) {
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *phi2Liq->sentence(), SpanInterval(b-2,b-1,b-2,b-1,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					} else {
						t = set_at(toScan.set(), toScan.set().size()-1).finish().finish()+1;
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *phi2Liq->sentence(), SpanInterval(t,b-1,t,b-1,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
				}
				if (b != d.maxInterval().finish()) {
					// option 3, delete precedent all the way until consequent is true
					SpanInterval toIntersect(b, d.maxInterval().finish(), b, d.maxInterval().finish(), d.maxInterval());
					SISet toScan(true, d.maxInterval());
					toScan.add(toIntersect);
					toScan = intersection(phi2TrueAt, toScan);

					unsigned int t;
					if (toScan.size() != 0) {
						t = set_at(toScan.set(), 0).start().start()-1;
						// delete precedent over that interval
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body, SpanInterval(b,t,b,t,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					} else {
						// just delete it all the way to maxInterval.end()
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body,
								SpanInterval(b, d.maxInterval().finish(), b, d.maxInterval().finish(), d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
				}
			}
		}
	}
	if (headDia->relations().find(Interval::FINISHES) != headDia->relations().end()) {
		SISet falseAt;
		{
			boost::shared_ptr<Sentence> insideDiamondSentence(headDia->sentence()->clone());
			boost::shared_ptr<DiamondOp> headDiaSingle(new DiamondOp(insideDiamondSentence, Interval::FINISHES));
			boost::shared_ptr<Sentence> bodyCopy(body->clone());
			boost::shared_ptr<Sentence> disSingle(new Disjunction(headDiaSingle, bodyCopy));
			// find where this statement is not true
			falseAt = d.satisfied(*disSingle, m).compliment();
			LOG(LOG_DEBUG) << "false at :" << falseAt.toString();
		}
		if (falseAt.size() != 0) {
			// pick a span interval at random
			SpanInterval toSatisfy = set_at(falseAt.set(), rand() % falseAt.set().size());
			unsigned int b = toSatisfy.finish().finish();
			// case 1: extend phi1 to satisfy violation at b
			SISet phi2TrueAt = d.satisfied(*phi2, m);
			{
				if (b != d.maxInterval().finish()) {
					// find the point after b where phi2 is true at
					SpanInterval toIntersect(b+1, d.maxInterval().finish(), b+1, d.maxInterval().finish(), d.maxInterval());
					SISet toScan(true, d.maxInterval());
					toScan.add(toIntersect);
					toScan = intersection(phi2TrueAt, toScan);

					unsigned int t;
					if (toScan.size() != 0) {
						t = set_at(toScan.set(), 0).finish().finish();
						// satisfy precedent over that expinterval
						std::vector<Move> localMoves = findMovesForLiquid(d, m, *body->sentence(), SpanInterval(b,t,b,t,d.maxInterval()));
						moves.insert(moves.end(), localMoves.begin(), localMoves.end());
					}
				}
			}
			// case 2: delete phi1 until phi2 is true
			{
				if (b != d.maxInterval().start()) {
					// find the point before b where phi2 is true at
					SpanInterval toIntersect(d.maxInterval().start(), b-1, d.maxInterval().start(), b-1, d.maxInterval());
					SISet toScan(true, d.maxInterval());
					toScan.add(toIntersect);
					toScan = intersection(phi2TrueAt, toScan);

					unsigned int t;
					if (toScan.size() == 0) {
						// delete phi1 completely!
						t = 0;
					} else {
						t = set_at(toScan.set(), toScan.set().size()-1).finish().finish()+1;
					}
					std::vector<Move> localMoves = findMovesForLiquid(d, m, *body, SpanInterval(t,b,t,b,d.maxInterval()));
					moves.insert(moves.end(), localMoves.begin(), localMoves.end());

					// case 3: extend phi2 until phi1 is true
					if (toScan.size() == 0) {
						t = b-1;	// instead of adding it at 0, we will consider adding for the previous step
					}
					localMoves = findMovesForLiquid(d, m, *phi2Liq->sentence(), SpanInterval(t,b,t,b,d.maxInterval()));
					moves.insert(moves.end(), localMoves.begin(), localMoves.end());
				}
			}
		}
	}
	if (headDia->relations().find(Interval::FINISHESI) != headDia->relations().end()) {
			// TODO:  COME BACK TO THIS< IT BROKEN
		LOG_PRINT(LOG_WARN) << "Interval::FINISHESI is not correctly working yet!  Moves likely wrong...";
			SISet falseAt;
			{
				boost::shared_ptr<Sentence> insideDiamondSentence(headDia->sentence()->clone());
				boost::shared_ptr<DiamondOp> headDiaSingle(new DiamondOp(insideDiamondSentence, Interval::FINISHESI));
				boost::shared_ptr<Sentence> bodyCopy(body->clone());
				boost::shared_ptr<Sentence> disSingle(new Disjunction(headDiaSingle, bodyCopy));
				// find where this statement is not true
				falseAt = d.satisfied(*disSingle, m).compliment();
				LOG(LOG_DEBUG) << "false at :" << falseAt.toString();
			}
			if (falseAt.size() != 0) {
				// TODO: only two moves??
				// pick a span interval at random
				SpanInterval toSatisfy = set_at(falseAt.set(), rand() % falseAt.set().size());
				unsigned int b = toSatisfy.finish().finish();
				// case 1: extend phi2 to satisfy violation ending at b
				SISet phi2TrueAt = d.satisfied(*phi2, m);

				// find the point before b where phi2 is true at
				SpanInterval toIntersect(toSatisfy.start().start(), b, toSatisfy.start().start(), b, d.maxInterval());
				SISet toScan(true, d.maxInterval());
				toScan.add(toIntersect);
				toScan = intersection(phi2TrueAt, toScan);

				unsigned int t;
				if (toScan.size() != 0) {
					t = set_at(toScan.set(), toScan.set().size()-1).finish().finish()+1;
				} else {
					t = b-1;
				}
				// satisfy phi2 over that interval
				std::vector<Move> localMoves = findMovesForLiquid(d, m, *phi2Liq, SpanInterval(t,b,t,b,d.maxInterval()));
				moves.insert(moves.end(), localMoves.begin(), localMoves.end());

				// case 2: delete phi1 until phi2 is true
				if (toScan.size() == 0) {
					t = toSatisfy.start().start();
				}
				localMoves = findMovesForLiquid(d, m, *body, SpanInterval(t,b,t,b,d.maxInterval()));
				moves.insert(moves.end(), localMoves.begin(), localMoves.end());

			}
		}

	return moves;
}

std::vector<Move> findMovesForForm2(const Domain& d, const Model& m, const Disjunction &dis) {
	std::vector<Move> moves;
	LOG(LOG_DEBUG) << "inside findMovesForForm2() with sentence: " << dis.toString();

	// extract our precedent and consequent from dis
	const Negation *precedent;
	const Sentence *consequent;
	if (dynamic_cast<const Negation*>(&*dis.left())
			&& !dynamic_cast<const Negation*>(&*dis.right())) {
		precedent = dynamic_cast<const Negation*>(&*dis.left());
		consequent = &*dis.right();
	} else if (!dynamic_cast<const Negation*>(&*dis.left())
			&& dynamic_cast<const Negation*>(&*dis.right())) {
		precedent = dynamic_cast<const Negation*>(&*dis.right());
		consequent = &*dis.left();
	} else {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}

	const Conjunction *precConj = dynamic_cast<const Conjunction*>(&*precedent->sentence());
	if (!precConj) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}
	// extract all the sentences used in the meets conjunction
	std::vector<const Sentence *> meetsArgs = getMeetsConjunctionArgs(*precConj);

	// phik is the last element in the vector
	const Sentence *phik = meetsArgs.back();
	LOG(LOG_DEBUG) << "phik = " << phik->toString();

	// make sure the consequent is diamond ops applied to a liq disjunction, containing phik
	const DiamondOp *consDiamond = dynamic_cast<const DiamondOp*>(consequent);
	if (!consDiamond) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}
	if (consDiamond->relations().find(Interval::MEETSI) == consDiamond->relations().end()) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}
	const LiquidOp *consLiquid = dynamic_cast<const LiquidOp*>(&*consDiamond->sentence());
	if (!consLiquid) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}
	const Disjunction *consDisj = dynamic_cast<const Disjunction*>(&*consLiquid->sentence());
	if (!consDisj) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}

	// now extract the arguments
	std::vector<const Sentence*> disjArgs = getDisjunctionArgs(*consDisj);
	bool phikFound = false;
	// double check that one of them is equal to phik (and remove it)
	for(std::vector<const Sentence*>::iterator it = disjArgs.begin(); it != disjArgs.end();) {
		if (**it == *phik) {
			phikFound = true;
			it = disjArgs.erase(it);
		} else {
			it++;
		}
	}
	if (!phikFound) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 2: " << dis.toString();
		return moves;
	}
	// finally, wrap the leftover arguments as phiPrime
	boost::shared_ptr<Sentence> phiPrime;
	if (disjArgs.size() == 1) {
		phiPrime = boost::shared_ptr<Sentence>(disjArgs.front()->clone());
	} else {
		phiPrime = wrapInDisjunction(disjArgs);
	}

	/////// DONE PARSING OUT IMPORTANT PIECES /////////

	// pick an interval to satisfy where this sentence is violated
	SISet violations = d.satisfiedDisjunction(dis, m);
	violations = violations.compliment();

	if (violations.size() == 0) {
		LOG_PRINT(LOG_WARN) << "no moves to calculate for sentence \"" << dis.toString() << "\" - this probably shouldn't happen...";
		return moves;
	}
	// pick at random
	SpanInterval toSatisfy = set_at(violations.set(), rand() % violations.set().size());
	LOG(LOG_DEBUG) << "choosing to satisfy spaninterval " << toSatisfy.toString();

	unsigned int b = toSatisfy.finish().finish();

	// CASE 1: extend phik to satisfy violation at b
	// find the next point of time that phiPrime is true at
	SISet phiPrimeTrueAt = d.satisfied(*phiPrime, m);
	// we only care about time points after b
	SISet toIntersect(true, d.maxInterval());
	toIntersect.add(SpanInterval(b+1, d.maxInterval().finish(), b+1, d.maxInterval().finish()));
	SISet toScan = intersection(phiPrimeTrueAt, toIntersect);
	unsigned int t;
	if (toScan.size() != 0) {
		// pick the first element
		t = set_at(toScan.set(), 0).start().start()-1;
		// choose local moves adding phik between b and t
		std::vector<Move> localMoves = findMovesForLiquid(d, m, *phik, SpanInterval(b+1,t,b+1,t, d.maxInterval()));
		moves.insert(moves.end(), localMoves.begin(), localMoves.end());
	}

	// CASE 2: extend phiPrime to satisfy violation at b
	if (toScan.size() == 0) {
		t = b+1;
	}
	std::vector<Move> localMoves = findMovesForLiquid(d, m, *phiPrime, SpanInterval(b+1,t,b+1,t, d.maxInterval()));
	moves.insert(moves.end(), localMoves.begin(), localMoves.end());

	// CASE 3: delete phik until met by phiPrime (optionally deleting all of phik
	SISet phikTrueAt = d.satisfied(*phik, m);
	toIntersect = SISet(true, d.maxInterval());
	toIntersect.add(SpanInterval(d.maxInterval().start(), b, d.maxInterval().start(), b, d.maxInterval()));
	toScan = intersection(phikTrueAt, toIntersect);
	// choose the last element
	unsigned int phikLowerBound = set_at(toScan.set(), toScan.set().size()-1).start().start();
	if (phikLowerBound >= b) return moves;	// TODO probably shouldn't happen?

	// now find where phiPrime is true within phikLowerBound and b
	toIntersect = SISet(true, d.maxInterval());
	toIntersect.add(SpanInterval(phikLowerBound, b, phikLowerBound, b, d.maxInterval()));
	toScan = intersection(phiPrimeTrueAt, toIntersect);
	if (toScan.size() == 0) {
		t = phikLowerBound;
	} else {
		t = set_at(toScan.set(), toScan.set().size()-1).finish().finish()-1;
	}
	boost::shared_ptr<Sentence> phikCopy(phik->clone());
	boost::shared_ptr<Sentence> negatedPhik(new Negation(phikCopy));
	localMoves = findMovesForLiquid(d, m, *negatedPhik, SpanInterval(t,b,t,b, d.maxInterval()));
	moves.insert(moves.end(), localMoves.begin(), localMoves.end());

	return moves;
}

std::vector<Move> findMovesForForm3(const Domain& d, const Model& m, const Disjunction &dis) {
	std::vector<Move> moves;
	LOG(LOG_DEBUG) << "inside findMovesForForm3() with sentence: " << dis.toString();

	// extract our precedent and consequent from dis
	const Negation *leftAsNeg = dynamic_cast<const Negation *>(&*dis.left());
	const Negation *rightAsNeg = dynamic_cast<const Negation *>(&*dis.right());

	if (!leftAsNeg || !rightAsNeg) {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 3: " << dis.toString();
		return moves;
	}
	const Negation *precedent;
	const Negation *consequent;

	// consequent has the conjunction subtype
	if (dynamic_cast<const Conjunction *>(&*leftAsNeg->sentence())
			&& (dynamic_cast<const LiquidOp *>(&*rightAsNeg->sentence()) || dynamic_cast<const Atom*>(&*rightAsNeg->sentence()))) {
		precedent = rightAsNeg;
		consequent = leftAsNeg;
	} else if (dynamic_cast<const Conjunction *>(&*rightAsNeg->sentence())
			&& (dynamic_cast<const LiquidOp *>(&*leftAsNeg->sentence()) || dynamic_cast<const Atom*>(&*leftAsNeg->sentence()))) {
		precedent = leftAsNeg;
		consequent = rightAsNeg;
	} else {
		LOG_PRINT(LOG_WARN) << "given a sentence that doesn't match form 3: " << dis.toString();
		return moves;
	}
	// collect all the arguments of the conjunction
	const Conjunction *consConj = dynamic_cast<const Conjunction *>(&*consequent->sentence());
	std::vector<const Sentence *> consArgs = getMeetsConjunctionArgs(*consConj);

	// find an interval to satisfy this over
	SISet violations = d.satisfied(dis, m);
	violations = violations.compliment();
	if (violations.size() == 0) {
		LOG_PRINT(LOG_WARN) << "given a sentence that has no violations for form 3! sentence: " << dis.toString();
		return moves;
	}
	// choose randomly
	SpanInterval toSatisfyOrig = set_at(violations.set(), rand() % violations.set().size());
	// try to satisfy it over its liquid interval
	SpanInterval toSatisfy = SpanInterval(toSatisfyOrig.start().start(), toSatisfyOrig.finish().finish(),
							 toSatisfyOrig.start().start(), toSatisfyOrig.finish().finish(), toSatisfyOrig.maxInterval());
	SISet toSatisfySet(true, d.maxInterval());
	toSatisfySet.add(toSatisfy);

	// CASE 1: create a move for deleting every single element of the conjunction
	for(std::vector<const Sentence*>::iterator it = consArgs.begin(); it != consArgs.end(); it++) {
		// find the elements before and after
		std::vector<const Sentence*> eleBefore(consArgs.begin(), it);
		std::vector<const Sentence*>::iterator next(it);
		next++;
		std::vector<const Sentence*> eleAfter(next, consArgs.end());

		SISet beforeTrueAt(false, d.maxInterval());
		bool elementsBefore=true;
		if (eleBefore.size() == 1) {
			beforeTrueAt = d.satisfied(*eleBefore[0], m);
			beforeTrueAt = intersection(toSatisfySet, beforeTrueAt);
		} else if (eleBefore.size() > 1) {
			beforeTrueAt = d.satisfied(*wrapInMeetsConjunction(eleBefore), m);
			beforeTrueAt = intersection(toSatisfySet, beforeTrueAt);
		} else {
			elementsBefore = false;
		}
		SISet afterTrueAt(false, d.maxInterval());
		bool elementsAfter=true;
		if (eleAfter.size() == 1) {
			afterTrueAt = d.satisfied(*eleAfter[0], m);
			afterTrueAt = intersection(toSatisfySet, afterTrueAt);
		} else if (eleAfter.size() > 1) {
			afterTrueAt = d.satisfied(*wrapInMeetsConjunction(eleAfter), m);
			afterTrueAt = intersection(toSatisfySet, afterTrueAt);
		} else {
			elementsAfter = false;
		}

		SISet currTrueAt = d.satisfied(**it, m);

		currTrueAt = intersection(currTrueAt, toSatisfySet);


		// now, for each possible pairing, we will compute a move if it is valid
		if (elementsBefore && elementsAfter) {
			BOOST_FOREACH(SpanInterval spBefore, beforeTrueAt.set()) {
				BOOST_FOREACH(SpanInterval spCurr, currTrueAt.set()) {
					BOOST_FOREACH(SpanInterval spAfter, afterTrueAt.set()) {
						boost::optional<SpanInterval> spBeforeRelOpt = spBefore.satisfiesRelation(Interval::MEETS);
						boost::optional<SpanInterval> spAfterRelOpt = spAfter.satisfiesRelation(Interval::MEETSI);
						if (spBeforeRelOpt && spAfterRelOpt) {
							SpanInterval leftover = intersection(intersection(spBeforeRelOpt.get(), spAfterRelOpt.get()), spCurr);
							if (leftover.size() > 0) {
								// generate a move deleting it here
								boost::shared_ptr<Sentence> itSentenceCopy((*it)->clone());
								boost::shared_ptr<Sentence> negatedIt(new Negation(itSentenceCopy));
								std::vector<Move> localMoves = findMovesForLiquid(d, m, *negatedIt, spCurr);
								moves.insert(moves.end(), localMoves.begin(), localMoves.end());
							}
						}
					}
				}
			}
		} else if (elementsBefore) {
			BOOST_FOREACH(SpanInterval spBefore, beforeTrueAt.set()) {
				BOOST_FOREACH(SpanInterval spCurr, currTrueAt.set()) {
					boost::optional<SpanInterval> spBeforeRelOpt = spBefore.satisfiesRelation(Interval::MEETS);
					if (spBeforeRelOpt) {
						SpanInterval leftover = intersection(spBeforeRelOpt.get(), spCurr);
						if (leftover.size() > 0) {
							// generate a move deleting it here
							boost::shared_ptr<Sentence> itSentenceCopy((*it)->clone());
							boost::shared_ptr<Sentence> negatedIt(new Negation(itSentenceCopy));
							std::vector<Move> localMoves = findMovesForLiquid(d, m, *negatedIt, spCurr);
							moves.insert(moves.end(), localMoves.begin(), localMoves.end());

						}
					}
				}
			}
		} else {
			BOOST_FOREACH(SpanInterval spAfter, afterTrueAt.set()) {
				BOOST_FOREACH(SpanInterval spCurr, currTrueAt.set()) {
					boost::optional<SpanInterval> spAfterRelOpt = spAfter.satisfiesRelation(Interval::MEETSI);
					if (spAfterRelOpt) {
						SpanInterval leftover = intersection(spAfterRelOpt.get(), spCurr);
						if (leftover.size() > 0) {
							// generate a move deleting it here
							boost::shared_ptr<Sentence> itSentenceCopy((*it)->clone());
							boost::shared_ptr<Sentence> negatedIt(new Negation(itSentenceCopy));
							std::vector<Move> localMoves = findMovesForLiquid(d, m, *negatedIt, spCurr);
							moves.insert(moves.end(), localMoves.begin(), localMoves.end());

						}
					}
				}
			}
		}
	}
	// NO we are still not done, now we consider deleting the head at the beginning/end
	SpanInterval a(toSatisfyOrig.start().start(), toSatisfyOrig.start().finish(),
					toSatisfyOrig.start().start(), toSatisfyOrig.start().finish(), d.maxInterval());
	SpanInterval b(toSatisfyOrig.finish().start(), toSatisfyOrig.finish().finish(),
					toSatisfyOrig.finish().start(), toSatisfyOrig.finish().finish(), d.maxInterval());
	std::vector<Move> localMoves = findMovesForLiquid(d, m, *precedent, a);
	moves.insert(moves.end(), localMoves.begin(), localMoves.end());
	localMoves = findMovesForLiquid(d, m, *precedent, b);
	moves.insert(moves.end(), localMoves.begin(), localMoves.end());


	return moves;


}


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
		SISet original(true, si.maxInterval());
		original.add(si);
		SISet toModify = d.getModifiableSISet(a->name(), original);
		if (toModify.size() == 0) {
			// this predicate can't be changed.  return an empty move
			return move;
		}
		if (isNegation) {
			// we want to delete span intervals where its true
			BOOST_FOREACH(SpanInterval toModifySi, toModify.set()) {
				Move::change change = boost::make_tuple(*a, toModifySi);
				move.toDel.push_back(change);
			}
		} else {
			BOOST_FOREACH(SpanInterval toModifySi, toModify.set()) {
				// we want to add span intervals where its false
				Move::change change = boost::make_tuple(*a, toModifySi);
				move.toAdd.push_back(change);
			}
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
			if (!dynamic_cast<const Negation*>(&s) && !dynamic_cast<const Atom*>(&s) && !dynamic_cast<const Conjunction*>(&s)) {
				LOG_PRINT(LOG_DEBUG) << "can only have literals inside conjunction when working with moves: " << s.toString();
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
			} else if (dynamic_cast<const Atom*>(&s)) {
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


std::vector<Move> findMovesFor(const Domain& d, const Model& m, const Sentence &s) {
	std::vector<Move> moves;
	if (dynamic_cast<const LiquidOp*>(&s)) {
		// pick an si to satisfy
		SISet notSat = d.satisfied(s, m);
		notSat.setForceLiquid(true);
		LOG(LOG_DEBUG) << "sentence " << s.toString() << " satisfied at " << notSat.toString();
		notSat = notSat.compliment();
		if (notSat.size() == 0) return moves;

		SpanInterval si = set_at(notSat.set(), rand() % notSat.set().size());
		LOG(LOG_DEBUG) << "choosing " << si.toString() << " as the interval to satisfy";
		const LiquidOp* liq = dynamic_cast<const LiquidOp*>(&s);
		moves = findMovesForLiquid(d, m, *liq->sentence(), si);
	} else if (isFormula1Type(s, d)) {
		moves = findMovesForForm1(d, m, dynamic_cast<const Disjunction&>(s));
	} else if (isFormula2Type(s, d)) {
		moves = findMovesForForm2(d, m, dynamic_cast<const Disjunction&>(s));
	} else if (isFormula3Type(s, d)) {
		moves = findMovesForForm3(d, m, dynamic_cast<const Disjunction&>(s));
	} else if (isPELCNFLiteral(s)) {
		// pick an si to satisfy
		SISet notSat = d.satisfied(s, m);
		notSat = notSat.compliment();
		if (notSat.size() == 0) return moves;

		SpanInterval si = set_at(notSat.set(), rand() % notSat.set().size());
		moves = findMovesForPELCNFLiteral(d, m, s, si);
	} else if (isDisjunctionOfCNFLiterals(s)) {
		// instead of choosing just one si, we'll try them all
		SISet notSat = d.satisfied(s, m);
		LOG(LOG_DEBUG) << "sentence true at :" << notSat.toString();
		notSat = notSat.compliment();
		LOG(LOG_DEBUG) << "sentence NOT true at :" << notSat.toString();

		if (notSat.size() == 0) return moves;

		BOOST_FOREACH(SpanInterval si, notSat.set()) {
			std::vector<Move> localMoves = findMovesForPELCNFDisjunction(d, m, dynamic_cast<const Disjunction&>(s), si);
			moves.insert(moves.end(), localMoves.begin(), localMoves.end());
		}

	} else {
		LOG_PRINT(LOG_ERROR) << "given sentence \"" << s.toString() << "\" but it doesn't match any moves function we know about!";
	}
	// ensure that if we aren't allowed to modify observation predicates, we don't!
	if (d.dontModifyObsPreds()) {
		for (std::vector<Move>::iterator it = moves.begin(); it != moves.end();) {
			if (moveContainsObservationPreds(d, *it)) {
				LOG(LOG_DEBUG) << "removing invalid move: " << it->toString();
				it = moves.erase(it);
			} else {
				it++;
			}
		}
	}

	return moves;
}

bool isDisjunctionOfCNFLiterals(const Sentence& s) {
	// TODO: rewrite this!  stupid and slow!
	boost::shared_ptr<Sentence> copy(s.clone());
	return isDisjunctionOfCNFLiterals(copy);
}

bool isDisjunctionOfCNFLiterals(const boost::shared_ptr<const Sentence>& sentence) {
	boost::shared_ptr<const Disjunction> dis = boost::dynamic_pointer_cast<const Disjunction>(sentence);
	if (!dis) return false;
	if ((isPELCNFLiteral(dis->left()) || isDisjunctionOfCNFLiterals(dis->left()))
			&& (isPELCNFLiteral(dis->right()) || isDisjunctionOfCNFLiterals(dis->right()))) {
		return true;
	}
	return false;
}

bool isPELCNFLiteral(const Sentence& sentence) {	// TODO: this is stupid and slow!  rewrite this!
	boost::shared_ptr<Sentence> copy(sentence.clone());
	return isPELCNFLiteral(copy);
}

bool isPELCNFLiteral(const boost::shared_ptr<const Sentence>& sentence) {
	if (boost::dynamic_pointer_cast<const Atom>(sentence)
			|| boost::dynamic_pointer_cast<const BoolLit>(sentence)) {
		return true;
	}
	if (boost::dynamic_pointer_cast<const Negation>(sentence)) {
		boost::shared_ptr<const Negation> neg = boost::dynamic_pointer_cast<const Negation>(sentence);
		// TODO: necessary?
		if (boost::dynamic_pointer_cast<const Negation>(neg->sentence())) {
			return false;
		}
		return isPELCNFLiteral(neg->sentence());
	}
	if (boost::dynamic_pointer_cast<const DiamondOp>(sentence)) {
		boost::shared_ptr<const DiamondOp> dia = boost::dynamic_pointer_cast<const DiamondOp>(sentence);
		if (boost::dynamic_pointer_cast<const Atom>(dia->sentence())
				|| boost::dynamic_pointer_cast<const BoolLit>(dia->sentence())
				|| boost::dynamic_pointer_cast<const LiquidOp>(dia->sentence())) {	// TODO add liquidop
			return true;
		}
		return false;
	}
	if (boost::dynamic_pointer_cast<const Conjunction>(sentence)) {
		boost::shared_ptr<const Conjunction> con = boost::dynamic_pointer_cast<const Conjunction>(sentence);
		if ((boost::dynamic_pointer_cast<const Atom>(con->left())
			 || boost::dynamic_pointer_cast<const BoolLit>(con->left()))
			&& (boost::dynamic_pointer_cast<const Atom>(con->right())
			 || boost::dynamic_pointer_cast<const BoolLit>(con->right()))) {
			return true;
		}
		return false;
	}

	return false;
}

std::vector<Move> findMovesForPELCNFLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si) {
	std::vector<Move> moves;
	// check for simple literal - either an atom or a negation applied to an atom
	if (const Atom* a = dynamic_cast<const Atom*>(&s)) {
		// easy, just add it to si
		Move move;
		if (d.isLiquid(a->name()) && !si.isLiquid()) {
			// need to add it to a liquid spaninterval
			SpanInterval si2(si.start().start(), si.finish().finish(), si.start().start(), si.finish().finish(), si.maxInterval());
			move.toAdd.push_back(boost::make_tuple(*a, si2));
		} else {
			move.toAdd.push_back(boost::make_tuple(*a, si));
		}
		moves.push_back(move);
		return moves;
	}
	if (const LiquidOp* liq = dynamic_cast<const LiquidOp*>(&s)) {
		// if it's a liquid operator, just use the moves for liquid operators
		return findMovesForLiquid(d, m, s, si);
	}
	if (const Negation* n = dynamic_cast<const Negation*>(&s)) {
		if (const Atom* a = dynamic_cast<const Atom*>(&*n->sentence())) {
			// delete it from si
			Move move;
			if (d.isLiquid(a->name()) && !si.isLiquid()) {
				SpanInterval si2(si.start().start(), si.finish().finish(), si.start().start(), si.finish().finish(), si.maxInterval());
				move.toDel.push_back(boost::make_tuple(*a, si2));
			} else {
				move.toDel.push_back(boost::make_tuple(*a, si));
			}
			moves.push_back(move);
			return moves;
		}
		if (const DiamondOp* dia = dynamic_cast<const DiamondOp*>(&*n->sentence())) {
			// TODO: implement code that handles multiple diamond operators!
			if (dia->relations().size() == 13) {
				const Atom* a = dynamic_cast<const Atom*>(&*dia->sentence());
				if (a) {
					// We've got !<>{*}, this is easy
					Move move;
					SpanInterval everywhere(d.maxInterval(), d.maxInterval(), d.maxInterval());
					move.toDel.push_back(boost::make_tuple(*a, everywhere));
					moves.push_back(move);
					return moves;
				}
			}
			if (dia->relations().size() > 1) {
				LOG_PRINT(LOG_ERROR) << "currently cannot handle moves for diamond ops with multiple relations!";
				throw std::runtime_error("unimplemented moves found");
			}
			if (dia->relations().size() == 0) return moves;
			// OK, we've got !<>{r} (phi), whether phi is liquid or an atom it should be relatively the same
			boost::optional<SpanInterval> si2Opt = si.satisfiesRelation(inverseRelation(*dia->relations().begin()));
			if (!si2Opt) return moves;		// no moves available
			SpanInterval si2 = si2Opt.get();
			if (const Atom* a = dynamic_cast<const Atom*>(&*dia->sentence())) {
				Move move;
				if (d.isLiquid(a->name()) && !si2.isLiquid()) {
					SpanInterval si3(si2.start().start(), si2.finish().finish(), si2.start().start(), si2.finish().finish(), si2.maxInterval());
					move.toDel.push_back(boost::make_tuple(*a, si3));
				} else {
					move.toDel.push_back(boost::make_tuple(*a, si2));
				}
				moves.push_back(move);
				return moves;
			} else if (const LiquidOp* liq = dynamic_cast<const LiquidOp*>(&*dia->sentence())) {
				// normally we can't do this, but slip a negation inside the liquid op since its equivalent
				boost::shared_ptr<Sentence> insideLiq(liq->sentence()->clone());
				boost::shared_ptr<Sentence> negInsideLiq(new Negation(insideLiq));
				// by adding this negation we need to ensure it's moved in
				negInsideLiq = moveNegationsInward(negInsideLiq);
		//		boost::shared_ptr<Sentence> newLiq(new LiquidOp(negInsideLiq));
				// satisfy this liq operation now
				std::vector<Move> localMoves = findMovesForLiquid(d, m, *negInsideLiq, si2);
				moves.insert(moves.begin(), localMoves.begin(), localMoves.end());
				return moves;
			}
		}
		if (const Conjunction* con = dynamic_cast<const Conjunction*>(&*n->sentence())) {
			// TODO: implement code that handles multiple conjunction operators
			if (con->relations().size() > 1) {
				LOG_PRINT(LOG_ERROR) << "currently cannot handle conjunctions with multiple relations! :" << s.toString();
				throw std::runtime_error("can't find moves inside findMovesForPELCNFLiteral for a conjunction with multiple relations");
			}
			if (con->relations().size() == 0) return moves;
			// TODO: handle more than just the case where it is two atoms (ie liquid ops)
			const Atom* leftAtom = dynamic_cast<const Atom*>(&*con->left());
			const Atom* rightAtom = dynamic_cast<const Atom*>(&*con->right());

			if (!leftAtom || !rightAtom) {
				LOG_PRINT(LOG_ERROR) << "conjunction operator can only be applied to simple atoms right now! :" << s.toString();
				throw std::runtime_error("unimplemented moves for conjunction operator");
			}

			// find where leftAtom is true, intersected with si
			SISet toIntersect(false, si.maxInterval());
			toIntersect.add(SpanInterval(si.start().start(), si.finish().finish(), si.start().start(), si.finish().finish(), si.maxInterval()));
			SISet leftTrueAt = d.satisfied(*leftAtom, m);
			leftTrueAt = intersection(leftTrueAt, toIntersect);
			SISet rightTrueAt = d.satisfied(*rightAtom, m);
			rightTrueAt = intersection(rightTrueAt, toIntersect);

			if (con->relations().find(Interval::LESSTHAN) != con->relations().end()) {
				{
					// two cases: 1 delete any spanintervals that start at si
					Move move;
					BOOST_FOREACH(SpanInterval leftSi, leftTrueAt.set()) {
						boost::optional<Interval> interOpt = intersection(leftSi.start(), si.start());
						if (interOpt) {
							Interval inter = interOpt.get();
							// remove that part of the spanning interval
							SpanInterval siToRemove(inter.start(), leftSi.finish().finish(), inter.start(), leftSi.finish().finish(), leftSi.maxInterval());
							move.toDel.push_back(boost::make_tuple(*leftAtom, siToRemove));
						}
					}
					if (!move.isEmpty()) moves.push_back(move);
				}
				{
					// case 2: delete any spanintervals that end at si
					Move move;
					BOOST_FOREACH(SpanInterval rightSi, rightTrueAt.set()) {
						boost::optional<Interval> interOpt = intersection(rightSi.start(), si.finish());
						if (interOpt) {
							Interval inter = interOpt.get();
							// remove that part of the spanning interval
							SpanInterval siToRemove(rightSi.start().start(), inter.finish(), rightSi.start().start(), inter.finish(), rightSi.maxInterval());
							move.toDel.push_back(boost::make_tuple(*rightAtom, siToRemove));
						}
					}
					if (!move.isEmpty()) moves.push_back(move);
				}
				return moves;
			} else {
				LOG_PRINT(LOG_ERROR) << "currently don't support the relation given: " << s.toString();
				throw std::runtime_error("unimplemented moves for conjunction operator");
			}
		}
		// TODO: implement moves for ![phi]
	}
	if (const DiamondOp* dia = dynamic_cast<const DiamondOp*>(&s)) {
		const Atom* a = dynamic_cast<const Atom*>(&*dia->sentence());

		if (!a) {
			LOG_PRINT(LOG_ERROR) << "must be an atom inside the diamond op! :" << s.toString();
			throw std::runtime_error("unable to calculate moves");
		}
		if (dia->relations().size() > 1) {
			// calculate the moves for each one
			BOOST_FOREACH(Interval::INTERVAL_RELATION rel, dia->relations()) {
				boost::shared_ptr<Sentence> diaSentenceCopy(dia->sentence()->clone());
				boost::shared_ptr<Sentence> diaCopy(new DiamondOp(diaSentenceCopy, rel));
				std::vector<Move> localMoves = findMovesForPELCNFLiteral(d, m, *diaCopy, si);
				moves.insert(moves.end(), localMoves.begin(), localMoves.end());
			}
			return moves;
		}
		// TODO: implement moves for diamond operator
		if (dia->relations().size() == 0) return moves;
		if (dia->relations().find(Interval::DURING) != dia->relations().end()) {
			boost::optional<SpanInterval> durIntOpt = si.satisfiesRelation(Interval::DURINGI);
			if (!durIntOpt) return moves;
			SpanInterval durInt = durIntOpt.get();
			// how to choose where to add it?  ideally we'd choose a transition point
			// TODO: choose transition points!!!

			// instead we will choose a random point in the interval and add it there
			unsigned int point = durInt.start().start() + (rand() % durInt.start().size());
			Move move;
			move.toAdd.push_back(boost::make_tuple(*a, SpanInterval(point, point, point, point, si.maxInterval())));
			moves.push_back(move);
			return moves;
		}
		if (dia->relations().find(Interval::MEETS) != dia->relations().end()) {
			// unfortunately we can only add (or extend) phi to satisfy this relation
			if (!si.satisfiesRelation(Interval::MEETSI)) return moves;
			SpanInterval whereToSat = si.satisfiesRelation(Interval::MEETSI).get();
			SISet insideSatisfiedAt = d.satisfiedAtom(*a,m);
			insideSatisfiedAt = intersection(insideSatisfiedAt, whereToSat);
			unsigned int j = whereToSat.finish().finish();

			if (insideSatisfiedAt.size() == 0) {
				// just add it at the beginning
				Move move;
				move.toAdd.push_back(boost::make_tuple(*a,
						SpanInterval(j, j, j, j, d.maxInterval())));
				moves.push_back(move);
				return moves;
			} else {
				SpanInterval mostRecent = set_at(insideSatisfiedAt.set(), insideSatisfiedAt.set().size()-1);
				Move move;
				move.toAdd.push_back(boost::make_tuple(*a,
						SpanInterval(mostRecent.finish().finish()+1,
								j,
								mostRecent.finish().finish()+1,
								j,
								d.maxInterval())));
				moves.push_back(move);
				return moves;
			}
		}
		if (dia->relations().find(Interval::MEETSI) != dia->relations().end()) {
			// unfortunately we can only add (or extend) phi to satisfy this relation
			if (!si.satisfiesRelation(Interval::MEETS)) return moves;
			SpanInterval whereToSat = si.satisfiesRelation(Interval::MEETS).get();
			SISet insideSatisfiedAt = d.satisfiedAtom(*a,m);
			insideSatisfiedAt = intersection(insideSatisfiedAt, whereToSat);
			unsigned int j = whereToSat.start().start();

			if (insideSatisfiedAt.size() == 0) {
				// just add it at the end
				Move move;
				move.toAdd.push_back(boost::make_tuple(*a,
						SpanInterval(j, j, j, j, d.maxInterval())));
				moves.push_back(move);
				return moves;
			} else {
				SpanInterval mostRecent = set_at(insideSatisfiedAt.set(), 0);
				Move move;
				move.toAdd.push_back(boost::make_tuple(*a,
						SpanInterval(j,
								mostRecent.start().start(),
								j,
								mostRecent.start().start(),
								d.maxInterval())));
				moves.push_back(move);
				return moves;
			}
		}
	}

	// NO MOVE FOUND
	LOG_PRINT(LOG_ERROR) << "inside findMovesForPELCNFLiteral: given a sentence we don't recognize! :" << s.toString();
	throw std::runtime_error("unable to calculate moves");

}

std::vector<Move> findMovesForPELCNFDisjunction(const Domain &d, const Model& m, const Disjunction &dis, const SpanInterval& si) {
	std::vector<Move> moves;
	// consider satisfying either the left, or the right
	if (const Disjunction *disLeft  = dynamic_cast<const Disjunction*>(&*dis.left())) {
		std::vector<Move> localMoves = findMovesForPELCNFDisjunction(d, m, *disLeft, si);
		moves.insert(moves.begin(), localMoves.begin(), localMoves.end());
	} else {
		std::vector<Move> localMoves = findMovesForPELCNFLiteral(d, m, *dis.left(), si);
		moves.insert(moves.begin(), localMoves.begin(), localMoves.end());
	}

	if (const Disjunction *disRight  = dynamic_cast<const Disjunction*>(&*dis.right())) {
		std::vector<Move> localMoves = findMovesForPELCNFDisjunction(d, m, *disRight, si);
		moves.insert(moves.begin(), localMoves.begin(), localMoves.end());
	} else {
		std::vector<Move> localMoves = findMovesForPELCNFLiteral(d, m, *dis.right(), si);
		moves.insert(moves.begin(), localMoves.begin(), localMoves.end());
	}

	return moves;
}

Model executeMove(const Domain& d, const Move& move, const Model& model) {
	Model currentModel = model;
	// handle toadd
	for (std::vector<Move::change>::const_iterator it = move.toAdd.begin(); it != move.toAdd.end(); it++) {
		// check to see if atom is in the map; if not we will add it
		/*
		Model::iterator modelIt = currentModel.find(it->get<0>());
		if (modelIt != currentModel.end()) {
			const Atom a = modelIt->first;
			SISet trueAt = modelIt->second;
			trueAt.add(it->get<1>());
			currentModel.erase(modelIt);
			currentModel.insert(std::pair<const Atom, SISet>(a, trueAt));
		} else {
			bool isLiquid = d.isLiquid(it->get<0>().name());
			SISet trueAt(isLiquid, d.maxInterval());
			trueAt.add(it->get<1>());
			currentModel.insert(std::pair<const Atom, SISet>(it->get<0>(), trueAt));
		}
		*/
		bool isLiquid = d.isLiquid(it->get<0>().name());
		SISet trueAt(isLiquid, d.maxInterval());
		trueAt.add(it->get<1>());
		currentModel.setAtom(it->get<0>(), trueAt);
	}
	// handle toDel
	for (std::vector<Move::change>::const_iterator it = move.toDel.begin(); it != move.toDel.end(); it++) {
		// check to see if atom is in the map; if not we are done
		/*
		Model::iterator modelIt = currentModel.find(it->get<0>());
		if (modelIt != currentModel.end()) {
			const Atom a = modelIt->first;
			SISet trueAt = modelIt->second;

			SISet toRemoveComp(trueAt.forceLiquid(), trueAt.maxInterval());
			toRemoveComp.add(it->get<1>());
			toRemoveComp = toRemoveComp.compliment();
			trueAt = intersection(trueAt, toRemoveComp);

			currentModel.erase(modelIt);
			if (trueAt.size() > 0) currentModel.insert(std::pair<const Atom, SISet>(a, trueAt));
		}
		*/
		if (currentModel.hasAtom(it->get<0>())) {
			SISet toRemove(d.isLiquid(it->get<0>().name()), d.maxInterval());
			toRemove.add(it->get<1>());
			currentModel.unsetAtom(it->get<0>(), toRemove);
		}
	}
	return currentModel;
}

Model maxWalkSat(Domain& d, int numIterations, double probOfRandomMove, const Model* initialModel) {
	Model currentModel;
	if (initialModel==0) currentModel = d.defaultModel();
	else currentModel = *initialModel;

	// filter out sentences we can't currently generate moves for
	std::vector<int> validInit;
	std::vector<int> validNorm;
	FormulaSet formSet = d.formulas();
	for (int i = 0; i < formSet.primaryFormulas().size(); i++) {
		WSentence form = formSet.primaryFormulas().at(i);
		if (canFindMovesFor(*(form.sentence()), d)) {
			validInit.push_back(i);
		} else {
			// TODO: use a logging warning instead of stderr
			//std::cerr << "WARNING: currently cannot generate moves for sentence: \"" << d.formulas().at(i).sentence()->toString() << "\"." << std::endl;
			LOG(LOG_WARN) << "currently cannot generate moves for sentence: \"" <<form.sentence()->toString() << "\".";
		}
	}

	for (int i = 0; i < formSet.secondaryFormulas().size(); i++) {
		WSentence form = formSet.secondaryFormulas().at(i);
		if (canFindMovesFor(*(form.sentence()), d)) {
			validNorm.push_back(i);
		} else {
			// TODO: use a logging warning instead of stderr
			//std::cerr << "WARNING: currently cannot generate moves for sentence: \"" << d.formulas().at(i).sentence()->toString() << "\"." << std::endl;
			LOG(LOG_WARN) << "currently cannot generate moves for sentence: \"" <<form.sentence()->toString() << "\".";
		}
	}
	if (validInit.size() + validNorm.size() ==0) {
		// TODO: log an error
		std::cerr << "ERROR: no valid sentences to generate moves for!" << std::endl;
		return currentModel;
	}

	SpanInterval maxSI = SpanInterval(d.maxInterval().start(), d.maxInterval().finish(), d.maxInterval().start(), d.maxInterval().finish(), d.maxInterval());
	unsigned long maxSize = maxSI.size();
	unsigned long bestScore = d.score(currentModel);
	Model bestModel = currentModel;

	bool workingOnInit = (validInit.size() > 0 ? true : false);
	if (workingOnInit) {
		d.setScoringInit(true);
		LOG(LOG_DEBUG) << "working on initialization formulas.";
	}
	unsigned int showPeriodMod = (numIterations < 20 ? 1 : numIterations/20);

	for (int iteration=1; iteration <= numIterations; iteration++) {
		if (iteration % showPeriodMod == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		LOG(LOG_DEBUG) << "currentModel: " << currentModel.toString();
		LOG(LOG_DEBUG) << "current score: " << d.score(currentModel);
		// make a list of the current unsatisfied formulas we can calc moves for
		std::vector<int> notFullySatisfied = (workingOnInit ? validInit : validNorm);
		std::vector<WSentence> curFormulas = (workingOnInit ? d.formulas().primaryFormulas() : d.formulas().secondaryFormulas());

		for (std::vector<int>::iterator it = notFullySatisfied.begin(); it != notFullySatisfied.end(); ) {
			int i = *it;

			WSentence wsent = curFormulas.at(i);
			//const WSentence *wsentence = *it;
			if (maxSize*wsent.weight() == d.score(wsent, currentModel)) {
				it = notFullySatisfied.erase(it);
			} else {
				it++;
			}

		}

		if (notFullySatisfied.size()==0 && workingOnInit) {
			LOG_PRINT(LOG_INFO) << "done initializing, switching to normal formulas";
			workingOnInit = false;
			d.setScoringInit(false);
			continue;
		}

		if (notFullySatisfied.size()==0) {
			// can't really improve on this
			LOG_PRINT(LOG_INFO) << "no more sentences to satisfy!  exiting early after "<< iteration-1 << " iterations";
			return currentModel;
		}

		// pick one at random
		WSentence toImprove = curFormulas.at(notFullySatisfied[rand() % notFullySatisfied.size()]);
		LOG(LOG_DEBUG) << "choosing sentence: " << toImprove.sentence()->toString() << " to improve.";
		// find the set of moves that improve it
		std::vector<Move> moves = findMovesFor(d, currentModel, *(toImprove.sentence()));
		if (moves.size() == 0) {
			LOG(LOG_WARN) << "WARNING: unable to find moves for sentence " << toImprove.sentence()->toString()
					<< " but couldn't find any (even though its violated)!  continuing...";
			continue; // TODO: this shouldn't happen, right?
		}
		if (FileLog::globalLogLevel() <= LOG_DEBUG) {
			std::ostringstream vecStream;
			for (std::vector<Move>::const_iterator it = moves.begin(); it != moves.end(); it++) {
				if (it != moves.begin()) vecStream << ", ";
				vecStream << "(" << it->toString() << ")";
			}
			LOG(LOG_DEBUG) << "moves to consider: " << vecStream.str();
		}
		if (((double)rand()) / RAND_MAX < probOfRandomMove) {
			// take a random move
			Move aMove = moves[rand() % moves.size()];
			LOG(LOG_DEBUG) << "taking random move: " << aMove.toString();
			currentModel = executeMove(d, aMove, currentModel);
		} else {
			// find the models resulting from each move, and choose the highest scoring model as our next model
			unsigned long bestLocalScore = 0;
			std::vector<Model> bestLocalModels;
			std::vector<Move> bestLocalMoves;
			bestLocalModels.push_back(currentModel);
			for (std::vector<Move>::const_iterator it=moves.begin(); it != moves.end(); it++) {
				Model nextModel = executeMove(d, *it, currentModel);
				unsigned long nextScore = d.score(nextModel);
				if (nextScore > bestLocalScore) {
					bestLocalModels.clear();
					bestLocalMoves.clear();
					bestLocalScore = nextScore;
					bestLocalModels.push_back(nextModel);
					bestLocalMoves.push_back(*it);
				} else if (nextScore == bestLocalScore) {
					bestLocalModels.push_back(nextModel);
					bestLocalMoves.push_back(*it);
				}
			}
			int idx = rand() % bestLocalModels.size();	// choose one at random
			currentModel = bestLocalModels[idx];
			LOG(LOG_DEBUG) << "choosing best local move: " << bestLocalMoves[idx].toString();
		}
		d.clearCache();
		// evaluate and see if our model is better than any found so far
		unsigned long newScore = d.score(currentModel);
		if (newScore > bestScore) {
			LOG(LOG_DEBUG) << "remembering this model as best scoring so far";
			bestModel = currentModel;
			bestScore = newScore;
		}
	}

	return bestModel;
}

boost::shared_ptr<Sentence> convertToPELCNF(const boost::shared_ptr<const Sentence>& sentence, std::vector<boost::shared_ptr<Sentence> >& supportSentences,  Domain &d) {
	// make a copy so we can modify it
	boost::shared_ptr<Sentence> copy(sentence->clone());
	return convertToPELCNF_(copy, supportSentences, d);
}

boost::shared_ptr<Sentence> moveNegationsInward(const boost::shared_ptr<Sentence>& sentence) {
	if (boost::dynamic_pointer_cast<Atom>(sentence)
			|| boost::dynamic_pointer_cast<BoolLit>(sentence)) {
		return sentence;	// do nothing!
	}
	if (boost::dynamic_pointer_cast<Negation>(sentence)) {
		boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(sentence);
		// now see if we can move the negation inwards

		if (boost::dynamic_pointer_cast<Negation>(neg->sentence())) {
			boost::shared_ptr<Negation> neg2 = boost::dynamic_pointer_cast<Negation>(neg->sentence());
			// the negations cancel out!
			return moveNegationsInward(neg2->sentence());
		}
		if (boost::dynamic_pointer_cast<Conjunction>(neg->sentence())) {
			boost::shared_ptr<Conjunction> con = boost::dynamic_pointer_cast<Conjunction>(neg->sentence());
			// ONLY can move it in if the conjunction is of the equals variety
			std::set<Interval::INTERVAL_RELATION> justEquals;
			justEquals.insert(Interval::EQUALS);
			if (con->relations() == justEquals) {
				boost::shared_ptr<Sentence> conLeft = con->left();
				boost::shared_ptr<Sentence> conRight = con->right();

				// wrap both left and right with a negation
				boost::shared_ptr<Sentence> conLeftNeg(new Negation(conLeft));
				boost::shared_ptr<Sentence> conRightNeg(new Negation(conRight));

				// make sure they have negations moved in
				conLeftNeg = moveNegationsInward(conLeftNeg);
				conRightNeg = moveNegationsInward(conRightNeg);

				// construct our disjunction
				boost::shared_ptr<Sentence> disj(new Disjunction(conLeftNeg, conRightNeg));
				return disj;
			}
		}
		if (boost::dynamic_pointer_cast<Disjunction>(neg->sentence())) {
			boost::shared_ptr<Disjunction> dis = boost::dynamic_pointer_cast<Disjunction>(neg->sentence());
			// turn it into a conjunction
			boost::shared_ptr<Sentence> disLeft = dis->left();
			boost::shared_ptr<Sentence> disRight = dis->right();

			// wrap both left and right with a negation
			boost::shared_ptr<Sentence> disLeftNeg(new Negation(disLeft));
			boost::shared_ptr<Sentence> disRightNeg(new Negation(disRight));

			// make sure they have negations moved in
			disLeftNeg = moveNegationsInward(disLeftNeg);
			disRightNeg = moveNegationsInward(disRightNeg);

			// construct conjunction
			std::set<Interval::INTERVAL_RELATION> justEquals;
			boost::shared_ptr<Sentence> con(new Conjunction(disLeftNeg, disRightNeg, Interval::EQUALS));
			return con;
		}
		// can't move it in farther, just return our negation wrapped around it
		boost::shared_ptr<Sentence> newCon = moveNegationsInward(neg->sentence());
		if (newCon != neg->sentence()) {
			boost::shared_ptr<Sentence> newNeg(new Negation(newCon));
			return newNeg;
		}
		return neg;
	}
	if (boost::dynamic_pointer_cast<Disjunction>(sentence)) {
		boost::shared_ptr<Disjunction> dis = boost::dynamic_pointer_cast<Disjunction>(sentence);
		boost::shared_ptr<Sentence> disLeft = moveNegationsInward(dis->left());
		boost::shared_ptr<Sentence> disRight = moveNegationsInward(dis->right());

		if (disLeft == dis->left() & disRight == dis->right()) {
			// just return the original sentence
			return sentence;
		}
		// return a new disjunction obj
		boost::shared_ptr<Sentence> newDis(new Disjunction(disLeft, disRight));
		return newDis;
	}
	if (boost::dynamic_pointer_cast<Conjunction>(sentence)) {
		boost::shared_ptr<Conjunction> con = boost::dynamic_pointer_cast<Conjunction>(sentence);
		boost::shared_ptr<Sentence> conLeft = moveNegationsInward(con->left());
		boost::shared_ptr<Sentence> conRight = moveNegationsInward(con->right());

		if (conLeft == con->left() & conRight == con->right()) {
			// just return the original sentence
			return sentence;
		}
		// return a new conjunction obj
		std::set<Interval::INTERVAL_RELATION> rels = con->relations();
		boost::shared_ptr<Sentence> newCon(new Conjunction(conLeft, conRight, rels.begin(), rels.end()));
		return newCon;
	}
	if (boost::dynamic_pointer_cast<DiamondOp>(sentence)) {
		boost::shared_ptr<DiamondOp> dia = boost::dynamic_pointer_cast<DiamondOp>(sentence);
		boost::shared_ptr<Sentence> diaSent = moveNegationsInward(dia->sentence());

		if (diaSent == dia->sentence()) {
			return sentence;
		}
		std::set<Interval::INTERVAL_RELATION> rels = dia->relations();
		boost::shared_ptr<Sentence> newDia(new DiamondOp(diaSent, rels.begin(), rels.end()));
		return newDia;
	}
	if (boost::dynamic_pointer_cast<LiquidOp>(sentence)) {
		boost::shared_ptr<LiquidOp> liq = boost::dynamic_pointer_cast<LiquidOp>(sentence);
		boost::shared_ptr<Sentence> liqSent = moveNegationsInward(liq->sentence());

		if (liqSent == liq->sentence()) {
			return sentence;
		}
		boost::shared_ptr<Sentence> newLiq(new LiquidOp(liqSent));
		return newLiq;
	}

	// MADE IT THIS FAR, show an error!
	LOG_PRINT(LOG_ERROR) << "unknown type: " << sentence->toString() << std::endl;
	return sentence;

}

bool moveContainsObservationPreds(const Domain& d, const Move &m) {
	// TODO: this is a bad solution!  come back and fix this!

	for (std::vector<Move::change>::const_iterator it = m.toAdd.begin();
			it != m.toAdd.end();
			it++) {
	    //if (d.observedPredicates().find(it->get<0>().name()) != d.observedPredicates().end()) {
	    //  return true;
	    //}
		SISet original(d.isLiquid(it->get<0>().name()), it->get<1>().maxInterval());
		original.add(it->get<1>());
		SISet modifiable = d.getModifiableSISet(it->get<0>().name(), original);
		if (modifiable.size() != original.size()) return true;

	   // if (it->get<0>().name().find("D-") == 0) return true;
	}

	for (std::vector<Move::change>::const_iterator it = m.toDel.begin();
			it != m.toDel.end();
			it++) {
		/*
	    if (d.observedPredicates().find(it->get<0>().name()) != d.observedPredicates().end()) {
	      return true;
	    }
	    if (it->get<0>().name().find("D-") == 0) return true;
	    */
		SISet original(d.isLiquid(it->get<0>().name()), it->get<1>().maxInterval());
		original.add(it->get<1>());
		SISet modifiable = d.getModifiableSISet(it->get<0>().name(), original);
		if (modifiable.size() != original.size()) return true;
	}


	return false;
}
