/*
 * maxwalksat.h
 *
 *  Created on: Nov 10, 2011
 *      Author: joe
 */

#ifndef MAXWALKSAT_H_
#define MAXWALKSAT_H_

#include <cstdlib>
#include "../log.h"

Model maxWalkSat(Domain& d, int numIterations, double probOfRandomMove, const Model& initialModel);

Model maxWalkSat(Domain& d, int numIterations, double probOfRandomMove, const Model* initialModel) {
	Model currentModel;
	if (initialModel==0) currentModel = d.defaultModel();
	else currentModel = *initialModel;

	// filter out sentences we can't currently generate moves for
	std::vector<int> validForms;
	//std::vector<int> validNorm;
	FormulaSet formSet = d.formulaSet();
	for (int i = 0; i < formSet.formulas().size(); i++) {
		WSentence form = formSet.formulas().at(i);
		if (canFindMovesFor(*(form.sentence()), d)) {
			validForms.push_back(i);
		} else {
			// TODO: use a logging warning instead of stderr
			//std::cerr << "WARNING: currently cannot generate moves for sentence: \"" << d.formulas().at(i).sentence()->toString() << "\"." << std::endl;
			LOG(LOG_WARN) << "currently cannot generate moves for sentence: \"" <<form.sentence()->toString() << "\".";
		}
	}

	// set up a mapping from atom to formula index.  this represents formulas where the atom occurs
	PredCollector collector;
	for (int i = 0; i < formSet.size(); i++) {
		WSentence formula = formSet.formulas().at(i);
		formula.sentence()->visit(collector);
	}

	/*
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
	*/
	if (validForms.size() ==0) {
		// TODO: log an error
		std::cerr << "ERROR: no valid sentences to generate moves for!" << std::endl;
		return currentModel;
	}

	SpanInterval maxSI = SpanInterval(d.maxInterval().start(), d.maxInterval().finish(), d.maxInterval().start(), d.maxInterval().finish(), d.maxInterval());
	unsigned long maxSize = maxSI.size();
	unsigned long bestScore = d.score(currentModel);
	Model bestModel = currentModel;

	unsigned int showPeriodMod = (numIterations < 20 ? 1 : numIterations/20);

	for (int iteration=1; iteration <= numIterations; iteration++) {
		if (iteration % showPeriodMod == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		LOG(LOG_DEBUG) << "currentModel: " << currentModel.toString();
		LOG(LOG_DEBUG) << "current score: " << d.score(currentModel);
		// make a list of the current unsatisfied formulas we can calc moves for
		std::vector<int> notFullySatisfied = validForms;
		std::vector<WSentence> curFormulas = d.formulaSet().formulas();

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



#endif /* MAXWALKSAT_H_ */
