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


std::vector<Move> findMovesFor(const Domain& d, const Model& m, const Sentence &s) {
	std::vector<Move> moves;
	if (dynamic_cast<const LiquidOp*>(&s)) {
		// pick an si to satisfy
		SISet notSat = d.satisfied(s, m);
		notSat.setForceLiquid(true);
		notSat = notSat.compliment();
		if (notSat.size() == 0) return moves;

		SpanInterval si = set_at(notSat.set(), rand() % notSat.set().size());

		const LiquidOp* liq = dynamic_cast<const LiquidOp*>(&s);
		moves = findMovesForLiquid(d, m, *liq->sentence(), si);
	} else if (isFormula1Type(s, d)) {
		return findMovesForForm1(d, m, dynamic_cast<const Disjunction&>(s));
	} else if (isFormula2Type(s, d)) {
		return findMovesForForm2(d, m, dynamic_cast<const Disjunction&>(s));
	} else if (isFormula3Type(s, d)) {
		return findMovesForForm3(d, m, dynamic_cast<const Disjunction&>(s));
	} else {
		LOG_PRINT(LOG_ERROR) << "given sentence \"" << s.toString() << "\" but it doesn't match any moves function we know about!";
	}
	return moves;
}

Model executeMove(const Domain& d, const Move& move, const Model& model) {
	Model currentModel = model;
	// handle toadd
	// TODO: make model a full blown class/struct so it has its own operators that modify it
	for (std::vector<Move::change>::const_iterator it = move.toAdd.begin(); it != move.toAdd.end(); it++) {
		// check to see if atom is in the map; if not we will add it
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
	}
	// handle toDel
	for (std::vector<Move::change>::const_iterator it = move.toDel.begin(); it != move.toDel.end(); it++) {
		// check to see if atom is in the map; if not we are done
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
	}
	return currentModel;
}

Model maxWalkSat(const Domain& d, int numIterations, double probOfRandomMove, const Model* initialModel) {
	Model currentModel;
	if (initialModel==0) currentModel = d.defaultModel();
	else currentModel = *initialModel;

	// filter out sentences we can't currently generate moves for
	std::vector<int> movesForSentences;
	for (int i = 0; i < d.formulas().size(); i++) {
		if (canFindMovesFor(*(d.formulas().at(i).sentence()), d)) {
			movesForSentences.push_back(i);
		} else {
			// TODO: use a logging warning instead of stderr
			//std::cerr << "WARNING: currently cannot generate moves for sentence: \"" << d.formulas().at(i).sentence()->toString() << "\"." << std::endl;
			LOG(LOG_WARN) << "currently cannot generate moves for sentence: \"" << d.formulas().at(i).sentence()->toString() << "\".";
		}
	}
	if (movesForSentences.size()==0) {
		// TODO: log an error
		std::cerr << "ERROR: no valid sentences to generate moves for!" << std::endl;
		return currentModel;
	}

	SpanInterval maxSI = SpanInterval(d.maxInterval().start(), d.maxInterval().finish(), d.maxInterval().start(), d.maxInterval().finish());
	unsigned long maxSize = maxSI.size();
	unsigned long bestScore = d.score(currentModel);
	Model bestModel = currentModel;

	for (int iteration=1; iteration < numIterations+1; iteration++) {
		LOG(LOG_DEBUG) << "currentModel: " << modelToString(currentModel);
		// make a list of the current unsatisfied formulas we can calc moves for
		std::vector<int> notFullySatisfied = movesForSentences;
		for (std::vector<int>::iterator it = notFullySatisfied.begin(); it != notFullySatisfied.end(); ) {
			int i = *it;

			WSentence wsent = d.formulas().at(i);
			//const WSentence *wsentence = *it;

			if (maxSize*wsent.weight() == d.score(wsent, currentModel)) {
				it = notFullySatisfied.erase(it);
			} else {
				it++;
			}

		}

		if (notFullySatisfied.size()==0) {
			// can't really improve on this
			return currentModel;
		}

		// pick one at random
		WSentence toImprove = d.formulas().at(notFullySatisfied[rand() % notFullySatisfied.size()]);
		LOG(LOG_DEBUG) << "choosing sentence: " << toImprove.sentence()->toString() << " to improve.";
		// find the set of moves that improve it
		std::vector<Move> moves = findMovesFor(d, currentModel, *(toImprove.sentence()));
		if (moves.size() == 0) {
			std::cerr << "WARNING: unable to find moves for sentence " << toImprove.sentence()->toString()
					<< " but couldn't find any (even though its violated)!  continuing..." << std::endl;
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
