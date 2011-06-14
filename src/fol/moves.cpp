#include "moves.h"

#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <sstream>
#include "fol.h"
#include "domain.h"
#include "sentence.h"

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



Move findMovesFor(const Domain& d, const Model& m, const Sentence &s) {
	Move empty;
	if (dynamic_cast<const LiquidOp*>(&s)) {
		const LiquidOp* liq = dynamic_cast<const LiquidOp*>(&s);
		return findMovesForLiquid(d, m, *liq);
	}
	return empty;
}
