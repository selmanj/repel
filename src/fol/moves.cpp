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

bool canFindMovesFor(const Sentence &s) {
	if (dynamic_cast<const LiquidOp *>(&s)) {
		return true;		// this isn't really fair: TODO write a better test on liquid ops
	}
	return false;
}

std::vector<Move> findMovesForForm1(const Domain& d, const Model& m, const Disjunction &dis) {
	LOG(LOG_DEBUG) << "inside findMovesForForm1()";
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
	std::cout << "PHI2 = " << phi2->toString() << std::endl;
	//boost::shared_ptr<Disjunction> disWithoutPrecedent =



	// TODO:  factor these out!  sheesh
	if (headDia->relations().find(Interval::MEETSI) != headDia->relations().end()) {
		boost::shared_ptr<DiamondOp> headDiaSingle(new DiamondOp(headDia->sentence(), Interval::MEETSI));
		boost::shared_ptr<Disjunction> disSingle(new Disjunction(headDiaSingle, body));
		// find where this statement is not true
		SISet falseAt = d.satisfied(disSingle, m).compliment();
		std::cout << "false at :" << falseAt.toString() << std::endl;
		LOG(LOG_DEBUG) << "false at :" << falseAt.toString();

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
					std::cout << "b = " << b << " t = " << t << std::endl;
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
					std::cout << "now b = " << b << " t = " << t << std::endl;
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
	// now do meets (similar)
	if (headDia->relations().find(Interval::MEETS) != headDia->relations().end()) {
		boost::shared_ptr<DiamondOp> headDiaSingle(new DiamondOp(headDia->sentence()->clone(), Interval::MEETS));
		boost::shared_ptr<Disjunction> disSingle(new Disjunction(headDiaSingle, body));
		// find where this statement is not true
		SISet falseAt = d.satisfied(disSingle, m).compliment();
		std::cout << "false at :" << falseAt.toString() << std::endl;
		LOG(LOG_DEBUG) << "false at :" << falseAt.toString();
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
					std::cout << "b = " << b << " t = " << t << std::endl;
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
					std::cout << "now b = " << b << " t = " << t << std::endl;
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
	if (headDia->relations().find(Interval::FINISHESI)) {

	}

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
		if (canFindMovesFor(*(d.formulas().at(i).sentence()))) {
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
