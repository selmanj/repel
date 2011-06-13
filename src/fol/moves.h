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
	typedef std::pair <Atom, SpanInterval> move;
	std::vector<move> toAdd;
	std::vector<move> toDel;
};

namespace {
	Moves findMovesForLiquid(const Domain& d, const Model& m, const Sentence &s);
	Moves findMovesForLiquidLiteral(const Domain& d, const Model& m, const Sentence &s);

}

boost::optional<Moves> findMovesFor(const Domain& d, const Model& m, const Sentence &s);
#endif
