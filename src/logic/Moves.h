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

#include "../siset.h"
#include "domain.h"
#include "../utils.h"
#include "../log.h"
#include "el_syntax.h"

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
std::vector<Move> findMovesFor(const Domain& d, const Model& m, const ELSentence &s);
std::vector<Move> findMovesForForm1(const Domain& d, const Model& m, const Disjunction &dis);
std::vector<Move> findMovesForForm2(const Domain& d, const Model& m, const Disjunction &dis);
std::vector<Move> findMovesForForm3(const Domain& d, const Model& m, const Disjunction &dis);

Move findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si);
std::vector<Move> findMovesForLiquidDisjunction(const Domain& d, const Model& m, const Disjunction &dis, const SpanInterval &si);
std::vector<Move> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s, const SpanInterval &si);
std::vector<Move> findMovesForPELCNFLiteral(const Domain& d, const Model& m, const Sentence &s, const SpanInterval& si);
std::vector<Move> findMovesForPELCNFDisjunction(const Domain &d, const Model& m, const Disjunction &dis, const SpanInterval& si);
Model executeMove(const Domain& d, const Move& move, const Model& model);

boost::shared_ptr<Sentence> convertToPELCNF(const boost::shared_ptr<const Sentence>& sentence, std::vector<boost::shared_ptr<Sentence> >& supportSentences, Domain &d);
boost::shared_ptr<Sentence> moveNegationsInward(const boost::shared_ptr<Sentence>& sentence);

bool moveContainsObservationPreds(const Domain& d, const Move& m);

namespace {
    boost::shared_ptr<Sentence> convertToPELCNF_(const boost::shared_ptr<Sentence>& curSentence, std::vector<boost::shared_ptr<Sentence> >& additionalSentences, Domain& d);
    boost::shared_ptr<Atom> rewriteAsLiteral(boost::shared_ptr<Sentence> sentence, std::vector<boost::shared_ptr<Sentence> >& additionalSentences, Domain& d);
}


#endif
