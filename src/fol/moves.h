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
#include "../log.h"
#include "fol.h"

class LiquidOp;
class Sentence;

struct Move {
	typedef boost::tuple <Atom, SpanInterval> change;
	std::vector<change> toAdd;
	std::vector<change> toDel;

	std::string toString() const;
	bool isEmpty() const;
};


bool canFindMovesFor(const Sentence &s, const Domain &d);
bool isFormula1Type(const Sentence &s, const Domain &d);
bool isFormula2Type(const Sentence &s, const Domain &d);
bool isFormula3Type(const Sentence &s, const Domain &d);
std::vector<Move> findMovesFor(const Domain& d, const Model& m, const Sentence &s);
std::vector<Move> findMovesForForm1(const Domain& d, const Model& m, const Disjunction &dis);
std::vector<Move> findMovesForForm2(const Domain& d, const Model& m, const Disjunction &dis);
std::vector<Move> findMovesForForm3(const Domain& d, const Model& m, const Disjunction &dis);

Move findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si);
Move findMovesForLiquidConjunction(const Domain& d, const Model& m, const Conjunction &c, const SpanInterval &si);
std::vector<Move> findMovesForLiquidDisjunction(const Domain& d, const Model& m, const Disjunction &dis, const SpanInterval &si);
std::vector<Move> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s, const SpanInterval &si);

bool isDisjunctionOfCNFLiterals(const boost::shared_ptr<const Sentence>& sentence);
bool isPELCNFLiteral(const Sentence& sentence);
bool isPELCNFLiteral(const boost::shared_ptr<const Sentence>& sentence);

std::vector<Move> findMovesForPELCNFLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si);

Model executeMove(const Domain& d, const Move& move, const Model& model);
Model maxWalkSat(const Domain& d, int numIterations, double probOfRandomMove, const Model* initialModel=0);

boost::shared_ptr<Sentence> convertToPELCNF(const boost::shared_ptr<const Sentence>& sentence, std::vector<boost::shared_ptr<Sentence> >& supportSentences, Domain &d);
boost::shared_ptr<Sentence> moveNegationsInward(const boost::shared_ptr<Sentence>& sentence);

namespace {
	boost::shared_ptr<Sentence> convertToPELCNF_(const boost::shared_ptr<Sentence>& curSentence, std::vector<boost::shared_ptr<Sentence> >& additionalSentences, Domain& d);
	boost::shared_ptr<Atom> rewriteAsLiteral(boost::shared_ptr<Sentence> sentence, std::vector<boost::shared_ptr<Sentence> >& additionalSentences, Domain& d);

	boost::shared_ptr<Sentence> convertToPELCNF_(const boost::shared_ptr<Sentence>& curSentence, std::vector<boost::shared_ptr<Sentence> >& additionalSentences, Domain& d) {
		if (isPELCNFLiteral(curSentence)) {
			// if curSentence is a literal, we're good!
			return curSentence;
		} else if (boost::dynamic_pointer_cast<Disjunction>(curSentence)) {
			if (isDisjunctionOfCNFLiterals(curSentence)) {
				return curSentence;
			}
			boost::shared_ptr<Disjunction> dis = boost::dynamic_pointer_cast<Disjunction>(curSentence);

			dis->left() = convertToPELCNF_(dis->left(), additionalSentences, d);
			dis->right() = convertToPELCNF_(dis->right(), additionalSentences, d);

			if (!isDisjunctionOfCNFLiterals(dis)) {
				// if we made it here, something must have gone wrong!
				LOG_PRINT(LOG_ERROR) << "got a disjunction but it wasn't a disjunction of lits! :" << dis->toString();
				return dis;
			}
			return dis;
		}
		// OK, it needs to be fixed.  find the element immediately below this operation
		if (boost::dynamic_pointer_cast<Negation>(curSentence)) {
			boost::shared_ptr<Negation> neg = boost::dynamic_pointer_cast<Negation>(curSentence);
			assert(isPELCNFLiteral(neg->sentence()) || isDisjunctionOfCNFLiterals(neg->sentence()));

			boost::shared_ptr<Sentence> newLit = rewriteAsLiteral(neg->sentence(), additionalSentences, d);
			neg->sentence() = newLit;
			return neg;
		} else if (boost::dynamic_pointer_cast<Conjunction>(curSentence)) {
			boost::shared_ptr<Conjunction> con = boost::dynamic_pointer_cast<Conjunction>(curSentence);
			con->left() = convertToPELCNF_(con->left(), additionalSentences, d);
			con->right() = convertToPELCNF_(con->right(), additionalSentences, d);

			// if left/right is not an atom/boolit, replace with a new atom
			if (!boost::dynamic_pointer_cast<Atom>(con->left())
					&& !boost::dynamic_pointer_cast<BoolLit>(con->left())) {
				boost::shared_ptr<Sentence> newLit = rewriteAsLiteral(con->left(), additionalSentences, d);
				con->left() = newLit;
			}
			if (!boost::dynamic_pointer_cast<Atom>(con->right())
								&& !boost::dynamic_pointer_cast<BoolLit>(con->right())) {
				boost::shared_ptr<Sentence> newLit = rewriteAsLiteral(con->right(), additionalSentences, d);
				con->right() = newLit;
			}
			return con;
		} else if (boost::dynamic_pointer_cast<DiamondOp>(curSentence)) {
			boost::shared_ptr<DiamondOp> dia = boost::dynamic_pointer_cast<DiamondOp>(curSentence);
			dia->sentence() = convertToPELCNF_(dia->sentence(), additionalSentences, d);

			if (!boost::dynamic_pointer_cast<Atom>(dia->sentence())
					|| !boost::dynamic_pointer_cast<BoolLit>(dia->sentence())) {
				boost::shared_ptr<Sentence> newLit = rewriteAsLiteral(dia->sentence(), additionalSentences, d);
				dia->sentence() = newLit;
			}
			return dia;
		}

		LOG_PRINT(LOG_ERROR) << "got a sentence we don't know what to do with! :" << curSentence->toString();
		return curSentence;

	}

	boost::shared_ptr<Atom> rewriteAsLiteral(boost::shared_ptr<Sentence> sentence, std::vector<boost::shared_ptr<Sentence> >& additionalSentences, Domain& d) {
		// introduce a new atom that represents this non-lit
		std::string newLitName = d.nameGenerator().getUniqueName();
		boost::shared_ptr<Atom> newLit(new Atom(newLitName));
		boost::shared_ptr<Sentence> newLitNegated(new Negation(newLit));

		boost::shared_ptr<Sentence> newDisj1(new Disjunction(newLitNegated, sentence));
		additionalSentences.push_back(newDisj1);

		// make the opposing sentence, one for each literal in the disjunction
		if (isDisjunctionOfCNFLiterals(sentence)) {
			const Disjunction* dis = dynamic_cast<const Disjunction*>(&*sentence);
			std::vector<const Sentence*> args = getDisjunctionArgs(*dis);
			BOOST_FOREACH(const Sentence* sPtr, args) {
				boost::shared_ptr<Sentence> negatedLit;
				if (dynamic_cast<const Negation*>(sPtr)) {
					// don't negate it, just remove the negation
					const Negation* negSPtr = dynamic_cast<const Negation*>(sPtr);
					negatedLit = boost::shared_ptr<Sentence>(negSPtr->sentence()->clone());
				} else {
					// wrap it in negation
					boost::shared_ptr<Sentence> sSharedPtr(sPtr->clone());
					negatedLit = boost::shared_ptr<Sentence>(new Negation(sSharedPtr));
				}

				boost::shared_ptr<Sentence> newDisj2(new Disjunction(newLit, negatedLit));
				additionalSentences.push_back(newDisj2);
			}
		} else {
			// again, only negate it if it needs it
			boost::shared_ptr<Sentence> negatedLit = sentence;
			if (!boost::dynamic_pointer_cast<Negation>(negatedLit)) {
				negatedLit = boost::shared_ptr<Sentence>(new Negation(negatedLit));
			} else {
				boost::shared_ptr<Negation> negTmp = boost::dynamic_pointer_cast<Negation>(negatedLit);
				negatedLit = negTmp->sentence();
			}

			boost::shared_ptr<Sentence> newDisj2(new Disjunction(newLit, negatedLit));
			additionalSentences.push_back(newDisj2);
		}
		return newLit;

	}
}


#endif
