#ifndef MOVES_H
#define MOVES_H

#include <vector>
#include <utility>
#include <boost/optional.hpp>

#include "atom.h"
#include "../siset.h"
#include "domain.h"

class LiquidOp;
class Sentence;

struct Moves {
	typedef std::pair <Atom, SISet> move;
	std::vector<move> toAdd;
	std::vector<move> toDel;
};

namespace {
	boost::optional<Moves> findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s);
	boost::optional<Moves> findMovesForLiquidAtom(const Domain& d, const Model& m, const Atom &a);

}

boost::optional<Moves> findMovesFor(const Domain& d, const Model& m, const Sentence &s);
#endif
