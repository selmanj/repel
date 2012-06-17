/*
 * MaxWalkSat.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: selman.joe@gmail.com
 */

#include "MaxWalkSat.h"
#include "../logic/syntax/ELSentence.h"
#include "../logic/Domain.h"
#include "../logic/Moves.h"
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/unordered_map.hpp>
#include <limits>

const unsigned int MWSSolver::defNumIterations = 1000;
const double MWSSolver::defProbOfRandomMove = 0.2;

Model MWSSolver::run(boost::mt19937& rng) {
    if (domain_ == NULL) {
        std::logic_error e("unable to run MWSSolver with Domain set to null ptr");
        throw e;
    }
    return run(rng, domain_->defaultModel());
}

namespace {
// we will calculate the resulting model and score for all
// moves, and choose the highest scoring one to move to.  This struct
// makes it a lot more convenient to do.
struct MWSState {
    Move move;
    Model model;
    double score;
    // the three below here are copies of the versions above
    std::vector<bool> localFormNeedUpdates;
    std::vector<double> localScores;
    std::vector<bool> localFormFullySat;
};
}

Model MWSSolver::run(boost::mt19937& rng, const Model& initialModel) {
    // validate domain
    if (domain_ == NULL) {
        std::logic_error e("unable to run MWSSolver with Domain set to null ptr");
        throw e;
    }
    // copy the domain's sentences into our own
    std::vector<ELSentence> formulas;
    std::copy(domain_->formulas_begin(), domain_->formulas_end(), std::back_inserter(formulas));

    if (formulas.size() == 0) {
        LOG(LOG_WARN) << "no formulas given - returning initial model";
        return initialModel;
    }

    // now note which sentences have valid moves
    std::vector<bool> formWithMoves;
    for (std::vector<ELSentence>::size_type i = 0;
            i < formulas.size();
            i++) {
        // infinite weight domains are invalid
        if (formulas[i].hasInfWeight()) {
            std::logic_error e("unable to deal with infinitely weighted sentences.");
            throw e;
        }
        if (!canFindMovesFor(*formulas[i].sentence(), *domain_)) {
            LOG(LOG_WARN) << "currently cannot generate moves for sentence: \"" << formulas[i].sentence()->toString() << "\".  ignoring it for generating moves";
            formWithMoves[i] = false;
        } else {
            formWithMoves[i] = true;
        }
    }

    // now record which sentences contain each atom - this is used to cache
    // the results of previous satisfied sentences.
    boost::unordered_map<Atom, std::vector<std::vector<ELSentence>::size_type > > atomToSentence;
    for (std::vector<ELSentence>::size_type i = 0;
            i != formulas.size();
            i++) {
        AtomCollector collector;
        formulas[i].sentence()->visit(collector);
        for (AtomCollector::atom_set::const_iterator atomit = collector.atoms.begin();
                atomit != collector.atoms.end();
                atomit++) {
            atomToSentence[*atomit].push_back(i);
        }
    }

    // setup stores for the score as well as whether each sentence is fully satisfied
    std::vector<double> formScores(formulas.size(), 0.0);
    std::vector<bool> formFullySat(formulas.size(), false);
    // also setup a vector we will use to mark which scores need updating
    std::vector<bool> formNeedUpdates(formulas.size(), true);

    updateScores(formulas, initialModel, formNeedUpdates, formScores, formFullySat);

    double currentScore = std::accumulate(formScores.begin(), formScores.end(), 0.0);
    double bestScore = currentScore;
    Model currentModel = initialModel;
    Model bestModel = initialModel;

    unsigned int showPeriodMod = (numIterations_ < 20 ? 1 : numIterations_/20); // TODO: make this configurable
    for (unsigned int iteration=1; iteration <= numIterations_; iteration++) {
        if (iteration % showPeriodMod == 0) {
            std::cout << ".";
            std::cout.flush();
        }
        LOG(LOG_DEBUG) << "currentModel: " << currentModel;
        LOG(LOG_DEBUG) << "current score: " << currentScore;

        // first, check to see if all our formulas are fully satisfied - if
        // that's the case, there's no real improvement left

        // At the same time build a list of formulas we can find moves for
        // that are not fully satisfied
        bool noImprovementLeft = true;
        std::vector<std::size_t> formCandidates;
        for (std::vector<bool>::size_type i = 0;
                i < formulas.size();
                i++) {
            if (!formFullySat[i]) {
                noImprovementLeft = false;
                if (formWithMoves[i]) {
                    formCandidates.push_back(i);
                }
            }
        }
        if (noImprovementLeft) {
            LOG(LOG_DEBUG) << "exiting early, no more sentences to satisfy!";
            return currentModel;
        }
        // if formCandidates is empty, then we aren't fully satisfied but we
        // can't generate moves for any valid sentences right now.  log a
        // warning and generate a random move
        Move nextMove;
        if (formCandidates.empty()) {
            LOG(LOG_WARN) << "cannot generate any moves for the current model (not all formulas are currently handled) - generating a random move";
            // pick a random atom
            boost::uniform_int<std::size_t> atomPick(0, domain_->atoms_size()-1);
            std::size_t atomInd = atomPick(rng);
            Domain::atom_const_iterator atomIt = domain_->atoms_begin();
            while (atomInd > 0) {
                atomIt++;
                atomInd--;
            }

            Atom atom = *atomIt;
            // now pick a spanning interval

            Interval maxInterval = domain_->maxInterval();
            boost::uniform_int<unsigned int> momentPick(maxInterval.start(), maxInterval.finish());
            unsigned int start = momentPick(rng);
            unsigned int finish = momentPick(rng);
            if (start > finish) std::swap(start, finish);

            SpanInterval si;
            if (domain_->isLiquid(atom.name())) {
                si = SpanInterval(start, finish);
            } else {
                si = SpanInterval(start, start, finish, finish);
            }
            // make a random flip to add/subtract
            Move::change ch(atom, si);
            boost::bernoulli_distribution<> flip(0.5);
            if (flip(rng)) {
                nextMove.toAdd.push_back(ch);
            } else {
                nextMove.toDel.push_back(ch);
            }
        } else {
            // choose a formula to improve at random
            boost::uniform_int<std::size_t> formulaPick(0, formCandidates.size()-1);
            std::size_t formChoseInd = formulaPick(rng);
            ELSentence formula = formulas[formChoseInd];
            LOG(LOG_DEBUG) << "choosing formula: " << formula << " to improve.";

            // find the moves for it
            std::vector<Move> moves = findMovesFor(*domain_, currentModel, formula, rng);
            if (moves.size() == 0) {
                LOG(LOG_WARN) << "WARNING: unable to find moves for sentence " << formula.sentence()->toString()
                        << " but couldn't find any (even though its violated)!  continuing (with a null iteration)...";
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

            boost::bernoulli_distribution<> randMovePick(probOfRandomMove_);
            if (randMovePick(rng)) {
                // take a random move
                boost::uniform_int<std::size_t> movesPick(0, moves.size()-1);
                Move aMove = moves[movesPick(rng)];
                LOG(LOG_DEBUG) << "taking random move: " << aMove.toString();

                currentModel = updateWithMove(aMove, currentModel, atomToSentence, formNeedUpdates);
                // update scores
                updateScores(formulas, currentModel, formNeedUpdates, formScores, formFullySat);
                currentScore = std::accumulate(formScores.begin(), formScores.end(), 0.0);
            } else {
                // instead of choosing a random move, choose the move that leads to the
                // highest scoring move.

                // we will calculate the resulting model and score for all
                // moves, and choose the highest scoring one to move to
                MWSState bestMWSState;

                bestMWSState.score = std::numeric_limits<double>::min();    // the lowest value possible
                // also have a vector of best models in case of ties
                std::vector<MWSState> ties;

                //std::vector<std::pair<Model, double> > nearbyModels;
                for (std::vector<Move>::const_iterator it = moves.begin(); it != moves.end(); it++) {
                    Move m = *it;
                    std::vector<bool> localFormNeedUpdates(formNeedUpdates);
                    Model nearbyModel = updateWithMove(m, currentModel, atomToSentence, localFormNeedUpdates);

                    std::vector<double> localScores(formScores);
                    std::vector<bool> localFormFullySat(formFullySat);
                    updateScores(formulas, currentModel, localFormNeedUpdates, localScores, localFormFullySat);
                    double nearbyScore = std::accumulate(localScores.begin(), localScores.end(), 0.0);

                    if (nearbyScore > bestMWSState.score) {
                        // save it
                        bestMWSState.move = m;
                        bestMWSState.model = nearbyModel;
                        bestMWSState.score = nearbyScore;
                        bestMWSState.localFormNeedUpdates = localFormNeedUpdates;
                        bestMWSState.localScores = localScores;
                        bestMWSState.localFormFullySat = localFormFullySat;

                        ties.clear();
                        ties.push_back(bestMWSState);
                    } else if (nearbyScore == bestMWSState.score) {
                        // found a tie
                        MWSState tieState;
                        tieState.move = m;
                        tieState.model = nearbyModel;
                        tieState.score = nearbyScore;
                        tieState.localFormNeedUpdates = localFormNeedUpdates;
                        tieState.localScores = localScores;
                        tieState.localFormFullySat = localFormFullySat;

                        ties.push_back(tieState);
                    }
                }
                assert(bestMWSState.score > std::numeric_limits<double>::min());
                if (ties.size() > 1) {
                    // pick a choice randomly
                    boost::uniform_int<std::size_t> tieChoice(0, ties.size()-1);
                    bestMWSState = ties[tieChoice(rng)];
                }
                LOG(LOG_DEBUG) << "taking move " << bestMWSState.move.toString();

                currentModel = bestMWSState.model;
                currentScore = bestMWSState.score;
                formNeedUpdates = bestMWSState.localFormNeedUpdates;
                formScores = bestMWSState.localScores;
                formFullySat = bestMWSState.localFormFullySat;
            }
        }
        // check to see if its the best score foudn so far
        if (currentScore > bestScore) {
            LOG(LOG_DEBUG) << "remembering this model as the best one seen so far.";
            bestScore = currentScore;
            bestModel = currentModel;
        }

    }
    LOG(LOG_INFO) << "returning the best model found with a score of " << bestScore;
    return bestModel;

//    for (int iteration=1; iteration <= numIterations; iteration++) {
//        if (iteration % showPeriodMod == 0) {
//            std::cout << ".";
//            std::cout.flush();
//        }
//        LOG(LOG_DEBUG) << "currentModel: " << currentModel;
//        LOG(LOG_DEBUG) << "current score: " << currentScore;
//
//        datalog << currentScore;
//
//        // make a list of the current unsatisfied formulas we can calc moves for
//        std::vector<int> notFullySatisfied = validForms;
//        std::vector<ELSentence> curFormulas = formulas;
//
//        for (std::vector<int>::iterator it = notFullySatisfied.begin(); it != notFullySatisfied.end(); ) {
//            int i = *it;
//
//            ELSentence wsent = curFormulas.at(i);
//            //const WSentence *wsentence = *it;
//            if (wsent.fullySatisfied(currentModel, d)) {
//                it = notFullySatisfied.erase(it);
//            } else {
//                it++;
//            }
//        }
//
//        if (notFullySatisfied.size()==0) {
//            // can't really improve on this
//            LOG(LOG_INFO) << "no more sentences to satisfy!  exiting early after "<< iteration-1 << " iterations";
//            return currentModel;
//        }
//
//        // pick one at random
//        boost::uniform_int<std::size_t> curFormUniformPick(0, notFullySatisfied.size()-1);
//        ELSentence toImprove = curFormulas.at(notFullySatisfied.at(curFormUniformPick(rng)));
//        LOG(LOG_DEBUG) << "choosing formula: " << toImprove << " to improve.";
//        // find the set of moves that improve it
//        std::vector<Move> moves = findMovesFor(d, currentModel, toImprove, rng);
//        if (moves.size() == 0) {
//            LOG(LOG_WARN) << "WARNING: unable to find moves for sentence " << toImprove.sentence()->toString()
//                    << " but couldn't find any (even though its violated)!  continuing...";
//            continue; // TODO: this shouldn't happen, right?
//        }
//        if (FileLog::globalLogLevel() <= LOG_DEBUG) {
//            std::ostringstream vecStream;
//            for (std::vector<Move>::const_iterator it = moves.begin(); it != moves.end(); it++) {
//                if (it != moves.begin()) vecStream << ", ";
//                vecStream << "(" << it->toString() << ")";
//            }
//            LOG(LOG_DEBUG) << "moves to consider: " << vecStream.str();
//        }
//        boost::bernoulli_distribution<> randMovePick(probOfRandomMove);
//        if (randMovePick(rng)) {
//            // take a random move
//            boost::uniform_int<std::size_t> movesPick(0, moves.size()-1);
//            Move aMove = moves[movesPick(rng)];
//            LOG(LOG_DEBUG) << "taking random move: " << aMove.toString();
//            currentModel = executeMove(d, aMove, currentModel);
//            score_pair scorePair = computeScoresForMove(d, currentModel, aMove, currentScore, formScores, occurs);
//            currentScore = scorePair.totalScore;
//            formScores = scorePair.formScores;
//        } else {
//            // find the models resulting from each move, and choose the highest scoring model as our next model
//            double bestLocalScore = 0.0;
//            std::vector<Model> bestLocalModels;
//            std::vector<Move> bestLocalMoves;
//            std::vector<score_pair> bestLocalScorePairs;
//
//            //bestLocalModels.push_back(currentModel);
//            for (std::vector<Move>::const_iterator it=moves.begin(); it != moves.end(); it++) {
//                Model nextModel = executeMove(d, *it, currentModel);
//                score_pair scorePair = computeScoresForMove(d, nextModel, *it, currentScore, formScores, occurs);
//                double nextScore = scorePair.totalScore;
//                if (nextScore > bestLocalScore) {
//                    bestLocalModels.clear();
//                    bestLocalMoves.clear();
//                    bestLocalScorePairs.clear();
//
//                    bestLocalScore = nextScore;
//                    bestLocalModels.push_back(nextModel);
//                    bestLocalMoves.push_back(*it);
//                    bestLocalScorePairs.push_back(scorePair);
//                } else if (nextScore == bestLocalScore) {
//                    bestLocalModels.push_back(nextModel);
//                    bestLocalMoves.push_back(*it);
//                    bestLocalScorePairs.push_back(scorePair);
//                }
//            }
//            boost::uniform_int<std::size_t> modelPick(0, bestLocalModels.size()-1);
//            int idx = modelPick(rng);  // choose one at random
//            currentModel = bestLocalModels[idx];
//            score_pair scorePair = bestLocalScorePairs[idx];
//            currentScore = scorePair.totalScore;
//            formScores = scorePair.formScores;
//            LOG(LOG_DEBUG) << "choosing best local move: " << bestLocalMoves[idx].toString();
//        }
//        // evaluate and see if our model is better than any found so far
//        if (currentScore > bestScore) {
//            LOG(LOG_DEBUG) << "remembering this model as best scoring so far";
//            bestModel = currentModel;
//            bestScore = currentScore;
//        }
//    }
//
//    return bestModel;


    std::runtime_error e("MWSSolver::run() not implemented.");
    throw e;
}

Model MWSSolver::updateWithMove(const Move& m,
        const Model& currentModel,
        const boost::unordered_map<Atom, std::vector<std::vector<ELSentence>::size_type > >& atomMap,
        std::vector<bool>& formsNeedUpdate) {
    // scan over all atoms in the move - if its being modified, mark the formula as needing update
    boost::unordered_set<Atom> moveAtoms;
    for (std::vector<Move::change>::const_iterator it = m.toAdd.begin();
            it != m.toAdd.end();
            it++) {
        moveAtoms.insert(it->get<0>());
    }
    for (std::vector<Move::change>::const_iterator it = m.toDel.begin();
            it != m.toDel.end();
            it++) {
        moveAtoms.insert(it->get<0>());
    }

    for (boost::unordered_set<Atom>::const_iterator it = moveAtoms.begin();
            it != moveAtoms.end();
            it++) {
        Atom a = *it;
        if (atomMap.count(a)) {
            // TODO: loop over all sentences
            std::vector<std::vector<ELSentence>::size_type > formsWithAtom = atomMap.at(a);
            for (std::vector<std::vector<ELSentence>::size_type >::const_iterator atomIt = formsWithAtom.begin();
                    atomIt != formsWithAtom.end();
                    atomIt++) {
            formsNeedUpdate[*atomIt] = true;
            }
        }
    }

    // now execute the move
    return executeMove(*domain_, m, currentModel);
}

/*
Model maxWalkSat(Domain& d,
        int numIterations,
        double probOfRandomMove,
        boost::mt19937& rng,
        const Model* initialModel,
        std::ostream* dataout) {
    row_out datalog(dataout);

    Model currentModel(d.maxInterval());
    if (initialModel==0) currentModel = d.defaultModel();
    else currentModel = *initialModel;

    // filter out sentences we can't currently generate moves for
    std::vector<int> validForms;
    //std::vector<int> validNorm;
    std::vector<ELSentence> formulas(d.formulas_begin(), d.formulas_end());
    //std::vector<ELSentence> formulas = formSet.formulas();
    for (std::vector<ELSentence>::size_type i = 0; i < formulas.size(); i++) {
        ELSentence form = formulas[i];
        if (form.hasInfWeight()) {
            throw std::invalid_argument("maxWalkSat(): can't solve a problem with infinite weights - rewrite first");
        }
        if (canFindMovesFor(*(form.sentence()), d)) {
            validForms.push_back(i);
        } else {
            // TODO: use a logging warning instead of stderr
            //std::cerr << "WARNING: currently cannot generate moves for sentence: \"" << d.formulas().at(i).sentence()->toString() << "\"." << std::endl;
            LOG(LOG_WARN) << "currently cannot generate moves for sentence: \"" <<form.sentence()->toString() << "\".";
        }
    }
    if (validForms.size() ==0) {
        // TODO: log an error
        std::cerr << "ERROR: no valid sentences to generate moves for!" << std::endl;
        return currentModel;
    }

    AtomOccurences occurs = findAtomOccurences(formulas);
    std::vector<double> formScores;
    double currentScore = 0.0;
    for (unsigned int i = 0; i < formulas.size(); i++) {
        ELSentence formula = formulas[i];
        double localScore = d.score(formula, currentModel);
        formScores.push_back(localScore);
        currentScore += localScore;
    }

    // initialize best score to the current score
    double bestScore = currentScore;
    Model bestModel = currentModel;


    unsigned int showPeriodMod = (numIterations < 20 ? 1 : numIterations/20);

    for (int iteration=1; iteration <= numIterations; iteration++) {
        if (iteration % showPeriodMod == 0) {
            std::cout << ".";
            std::cout.flush();
        }
        LOG(LOG_DEBUG) << "currentModel: " << currentModel;
        LOG(LOG_DEBUG) << "current score: " << currentScore;

        datalog << currentScore;

        // make a list of the current unsatisfied formulas we can calc moves for
        std::vector<int> notFullySatisfied = validForms;
        std::vector<ELSentence> curFormulas = formulas;

        for (std::vector<int>::iterator it = notFullySatisfied.begin(); it != notFullySatisfied.end(); ) {
            int i = *it;

            ELSentence wsent = curFormulas.at(i);
            //const WSentence *wsentence = *it;
            if (wsent.fullySatisfied(currentModel, d)) {
                it = notFullySatisfied.erase(it);
            } else {
                it++;
            }
        }

        if (notFullySatisfied.size()==0) {
            // can't really improve on this
            LOG(LOG_INFO) << "no more sentences to satisfy!  exiting early after "<< iteration-1 << " iterations";
            return currentModel;
        }

        // pick one at random
        boost::uniform_int<std::size_t> curFormUniformPick(0, notFullySatisfied.size()-1);
        ELSentence toImprove = curFormulas.at(notFullySatisfied.at(curFormUniformPick(rng)));
        LOG(LOG_DEBUG) << "choosing formula: " << toImprove << " to improve.";
        // find the set of moves that improve it
        std::vector<Move> moves = findMovesFor(d, currentModel, toImprove, rng);
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
        boost::bernoulli_distribution<> randMovePick(probOfRandomMove);
        if (randMovePick(rng)) {
            // take a random move
            boost::uniform_int<std::size_t> movesPick(0, moves.size()-1);
            Move aMove = moves[movesPick(rng)];
            LOG(LOG_DEBUG) << "taking random move: " << aMove.toString();
            currentModel = executeMove(d, aMove, currentModel);
            score_pair scorePair = computeScoresForMove(d, currentModel, aMove, currentScore, formScores, occurs);
            currentScore = scorePair.totalScore;
            formScores = scorePair.formScores;
        } else {
            // find the models resulting from each move, and choose the highest scoring model as our next model
            double bestLocalScore = 0.0;
            std::vector<Model> bestLocalModels;
            std::vector<Move> bestLocalMoves;
            std::vector<score_pair> bestLocalScorePairs;

            //bestLocalModels.push_back(currentModel);
            for (std::vector<Move>::const_iterator it=moves.begin(); it != moves.end(); it++) {
                Model nextModel = executeMove(d, *it, currentModel);
                score_pair scorePair = computeScoresForMove(d, nextModel, *it, currentScore, formScores, occurs);
                double nextScore = scorePair.totalScore;
                if (nextScore > bestLocalScore) {
                    bestLocalModels.clear();
                    bestLocalMoves.clear();
                    bestLocalScorePairs.clear();

                    bestLocalScore = nextScore;
                    bestLocalModels.push_back(nextModel);
                    bestLocalMoves.push_back(*it);
                    bestLocalScorePairs.push_back(scorePair);
                } else if (nextScore == bestLocalScore) {
                    bestLocalModels.push_back(nextModel);
                    bestLocalMoves.push_back(*it);
                    bestLocalScorePairs.push_back(scorePair);
                }
            }
            boost::uniform_int<std::size_t> modelPick(0, bestLocalModels.size()-1);
            int idx = modelPick(rng);  // choose one at random
            currentModel = bestLocalModels[idx];
            score_pair scorePair = bestLocalScorePairs[idx];
            currentScore = scorePair.totalScore;
            formScores = scorePair.formScores;
            LOG(LOG_DEBUG) << "choosing best local move: " << bestLocalMoves[idx].toString();
        }
        // evaluate and see if our model is better than any found so far
        if (currentScore > bestScore) {
            LOG(LOG_DEBUG) << "remembering this model as best scoring so far";
            bestModel = currentModel;
            bestScore = currentScore;
        }
    }

    return bestModel;
}


namespace {
    AtomOccurences findAtomOccurences(const std::vector<ELSentence>& sentences) {
        // set up a mapping from atom to formula index.  this represents formulas where the atom occurs
        AtomCollector collector;
        AtomOccurences occurs;
        for (std::vector<ELSentence>::size_type i = 0; i < sentences.size(); i++) {
            ELSentence formula = sentences.at(i);

            collector.atoms.clear();
            formula.sentence()->visit(collector);
            // add this index to all occurrences of our atom
            BOOST_FOREACH(Atom a, collector.atoms) {
                FormSet set = occurs[a];
                if (set.count(i) == 0) {
                    set.insert(i);
                    occurs[a] = set;
                }
            }
        }

        return occurs;
    }

    score_pair computeScoresForMove(const Domain& d,
            const Model& m,
            const Move& move,
            double currentScore,
            const std::vector<double>& curFormScores,
            const AtomOccurences& occurs) {
        // first, find the formulas we need to recompute
        std::set<int> formsToRescore;
        std::vector<Move::change> allchanges(move.toAdd);
        std::copy(move.toDel.begin(), move.toDel.end(), std::back_inserter(allchanges));

        BOOST_FOREACH(Move::change change, allchanges) {
            Atom a = change.get<0>();
            if (occurs.count(a) > 0) {
                AtomOccurences::const_iterator it = occurs.find(a);
                FormSet changedForms = it->second;
                std::copy(changedForms.begin(), changedForms.end(), std::inserter(formsToRescore, formsToRescore.end()));
            }
        }

        score_pair pair;
        pair.formScores = curFormScores;
        pair.totalScore = currentScore;

        // start recomputing, adjusting the total score as necessary
        std::vector<ELSentence> formulas(d.formulas_begin(), d.formulas_end());
        for (std::set<int>::const_iterator it = formsToRescore.begin(); it != formsToRescore.end(); it++) {
            int formNum = *it;
            ELSentence sentence = formulas[formNum];

            double score = d.score(sentence, m);
            if (score != curFormScores.at(formNum)) {   // TODO: require some sort of epsilon check?
                double difference = score - curFormScores.at(formNum);
                pair.formScores[formNum] = score;
                pair.totalScore += difference;
            }
        }

        return pair;
    }
}
*/

void MWSSolver::updateScores(const std::vector<ELSentence>& formulas,
        const Model& model,
        std::vector<bool>& whichToUpdate,
        std::vector<double>& scores,
        std::vector<bool>& fullySatisfied) {
    // first, calculate all the SISets for our sentences
    for (std::size_t i = 0;
            i < whichToUpdate.size();
            i++) {
        if (!whichToUpdate[i]) continue;    // skip elements that don't need updating

        ELSentence formula = formulas[i];
        // find the quantification for the current sentence
        SISet quantification(domain_->maxSpanInterval(), false, domain_->maxInterval());
        if (formula.isQuantified()) {
            quantification = formula.quantification();
        }

        SISet formSat = formula.dSatisfied(model, *domain_);
        // next, overwrite the score for the model
        scores[i] = ((double)formSat.size()) * formula.weight();
        // finally, mark if its completely satisfied
        if (quantification.includes(formSat) && formSat.includes(quantification)) {
            fullySatisfied[i] = true;
        } else {
            fullySatisfied[i] = false;
        }
        // done updating!  make a note
        whichToUpdate[i] = false;
    }
}
