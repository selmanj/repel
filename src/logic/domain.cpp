#include "domain.h"
#include "el_syntax.h"
#include "model.h"
#include "../log.h"

#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <sstream>
#include <vector>

void Domain::addObservedPredicate(const Atom& a) {
	if (observations_.hasAtom(a)) return;
	SISet newSet(true, maxInterval_);
	obsPreds_.insert(std::pair<std::string, SISet>(a.name(), newSet));
	observations_.setAtom(a, newSet);
}

SISet Domain::getModifiableSISet(const std::string& name) const {
	SISet everywhere(isLiquid(name), maxInterval_);
	everywhere = everywhere.compliment();
	return getModifiableSISet(name, everywhere);
}

SISet Domain::getModifiableSISet(const std::string& name, const SISet& where) const {
	// check to see if its an obs predicate
	if (obsPreds_.find(name) == obsPreds_.end() || !dontModifyObsPreds()) {
		return where;
	}

	SISet modifiable(where.forceLiquid(), where.maxInterval());
	if (assumeClosedWorld()) return modifiable;	// if its a closed world, can't change any obs predicates
	modifiable = where;

	modifiable.subtract(obsPreds_.find(name)->second);
	return modifiable;
}

void Domain::unsetAtomAt(const std::string& name, const SISet& where) {
	SISet newSet = obsPreds_.at(name);
	newSet.subtract(where);
	obsPreds_.erase(name);
	obsPreds_.insert(std::pair<std::string, SISet>(name, newSet));

	observations_.unsetAtom(name, where);


}


Model Domain::randomModel() const {
	// first check to see if we are even allowed to modify obs preds.  if not, just return the default model
	if (assumeClosedWorld()) {
		return defaultModel();
	}
	Model newModel;
	std::set<Atom, atomcmp> atoms = observations_.atoms();
	BOOST_FOREACH(Atom atom, atoms) {
		SISet random = SISet::randomSISet(isLiquid(atom.name()), maxInterval_);
		// intersect it with the places that are currently unset
		SISet unsetAt = getModifiableSISet(atom.name());
		random = intersection(random, unsetAt);
		// add in the set parts

		SISet setAt = unsetAt.compliment();
		SISet trueVals = intersection(setAt, observations_.getAtom(atom));
		random.add(trueVals);

		//newModel.clearAtom(obsPair->first);
		newModel.setAtom(atom, random);
	}
	return newModel;
}


void Domain::setMaxInterval(const Interval& maxInterval) {
	maxInterval_ = Interval(maxInterval);
	Model resized;

	observations_.setMaxInterval(maxInterval);
}

bool Domain::isLiquid(const std::string& predicate) const {
	// TODO: implement this sometime!
	return true;
}

unsigned long Domain::score(const ELSentence& w, const Model& m) const {
	const Sentence& s = *(w.sentence());
	SISet sat = satisfied(s, m);
	if (!sat.isDisjoint()) sat.makeDisjoint();
	return sat.size() * w.weight();
}

unsigned long Domain::score(const Model& m) const {
	unsigned long sum = 0;
	for (std::vector<ELSentence>::const_iterator it = formulas_.formulas().begin(); it != formulas_.formulas().end(); it++) {
		sum += score(*it, m);
	}
	return sum;
}

SISet Domain::satisfied(const Sentence& s, const Model& m, const SISet* where) const {

	// check in our cache first!
	ModelSentencePair pair(&m,&s);
	if (cache_.count(pair) == 1) {
		// return that instead!
		return cache_.get(pair);
	}

	SISet toReturn;
	// start with the base case
	if (dynamic_cast<const Atom *>(&s) != 0) {
		const Atom* a = dynamic_cast<const Atom *>(&s);
		toReturn = satisfiedAtom(*a, m);
	} else if (dynamic_cast<const Negation*>(&s) != 0) {
		const Negation* n = dynamic_cast<const Negation *>(&s);
		toReturn = satisfiedNegation(*n, m);
	} else if (dynamic_cast<const Disjunction*>(&s) != 0) {
		const Disjunction* d = dynamic_cast<const Disjunction *>(&s);
		toReturn = satisfiedDisjunction(*d, m);
	} else if (dynamic_cast<const LiquidOp*>(&s) != 0) {
		const LiquidOp* l = dynamic_cast<const LiquidOp *>(&s);
		toReturn = liqSatisfied(*(l->sentence()), m);
		// remove liquid restriction
		toReturn.setForceLiquid(false);
	} else if (dynamic_cast<const DiamondOp*>(&s) != 0) {
		const DiamondOp* d = dynamic_cast<const DiamondOp *>(&s);
		toReturn = satisfiedDiamond(*d, m);
	} else if (dynamic_cast<const Conjunction*>(&s) != 0) {
		const Conjunction* c = dynamic_cast<const Conjunction *>(&s);
		toReturn = satisfiedConjunction(*c, m);
	} else if (dynamic_cast<const BoolLit *>(&s) != 0) {
		const BoolLit* b = dynamic_cast<const BoolLit *>(&s);
		toReturn = satisfiedBoolLit(*b, m);
	} else {
		// made it this far, unimplemented!
		std::runtime_error e("Domain::satisfied not implemented yet!");
		throw e;
	}
	// for now, force it to be disjoint
	toReturn.makeDisjoint();
//	LOG_PRINT(LOG_DEBUG) << "sentence: " << s.toString() << " satisfied at " << toReturn.toString() << std::endl;
	// mask it with where
	if (where != NULL) {
		toReturn = intersection(toReturn, *where);
	}
	// add set to cache
	cache_.insert(pair, toReturn);
	return toReturn;
}

SISet Domain::satisfiedAtom(const Atom& a, const Model& m) const {
	if (a.isGrounded()) {
		// make sure its in model
		if (m.hasAtom(a)){
			SISet set = m.getAtom(a);
			// force non-liquidity
			set.setForceLiquid(false);
			return set;
		} else {
			return SISet(false, maxInterval_);
		}
	}
	// grounding out of atoms currently not implemented yet!
	std::runtime_error e("Domain::satisfiedAtom grounding out of atoms not implemented!");
	throw e;
}

SISet Domain::satisfiedNegation(const Negation& n, const Model& m) const {
	// return the compliment of the set inside the negation
	SISet sat = satisfied(*(n.sentence()), m);
	sat = sat.compliment();
	return sat;
}

SISet Domain::satisfiedDisjunction(const Disjunction& d, const Model& m) const {
	// union the left and the right
	SISet leftSat = satisfied(*(d.left()),m);
	SISet rightSat = satisfied(*(d.right()), m);
	leftSat.add(rightSat);
	return leftSat;
}

SISet Domain::satisfiedDiamond(const DiamondOp& d, const Model& m) const {
	SISet sat = satisfied(*(d.sentence()), m);
	SISet newsat(false, sat.maxInterval());
	BOOST_FOREACH(SpanInterval sp, sat.intervals()) {
		BOOST_FOREACH(Interval::INTERVAL_RELATION rel, d.relations()) {
			boost::optional<SpanInterval> spR = sp.satisfiesRelation(rel);
			if (spR) newsat.add(spR.get());
		}
	}
	return newsat;
}

SISet Domain::satisfiedConjunction(const Conjunction& c, const Model& m) const {
	SISet leftSat = satisfied(*(c.left()), m);
	SISet rightSat = satisfied(*(c.right()), m);

	SISet result(false, leftSat.maxInterval());

	BOOST_FOREACH(SpanInterval i, leftSat.intervals()) {
		BOOST_FOREACH(SpanInterval j, rightSat.intervals()) {
			BOOST_FOREACH(Interval::INTERVAL_RELATION rel, c.relations()) {
				result.add(composedOf(i, j, rel));
			}
		}
	}

	return result;
}

SISet Domain::satisfiedBoolLit(const BoolLit& b, const Model& m) const {
	SISet toReturn(false, maxInterval_);
	if (b.value()) {
		// max interval
		SpanInterval max(maxInterval_.start(), maxInterval_.finish(), maxInterval_.start(), maxInterval_.finish(), maxInterval_);
		toReturn.add(max);
	}
	return toReturn;
}


SISet Domain::liqSatisfied(const Sentence& s, const Model& m) const {
	SISet toReturn;
	if (dynamic_cast<const Atom *>(&s) != 0) {
		const Atom* a = dynamic_cast<const Atom *>(&s);
		toReturn = liqSatisfiedAtom(*a, m);
	} else if (dynamic_cast<const Negation *>(&s) != 0) {
		const Negation* n = dynamic_cast<const Negation *>(&s);
		toReturn = liqSatisfiedNegation(*n, m);
	} else if (dynamic_cast<const Disjunction *>(&s) != 0) {
		const Disjunction* d = dynamic_cast<const Disjunction *>(&s);
		toReturn = liqSatisfiedDisjunction(*d, m);
	} else if (dynamic_cast<const Conjunction *>(&s) != 0) {
		const Conjunction* c = dynamic_cast<const Conjunction *>(&s);
		toReturn = liqSatisfiedConjunction(*c, m);
	} else if (dynamic_cast<const BoolLit *>(&s) != 0) {
		const BoolLit* b = dynamic_cast<const BoolLit *>(&s);
		toReturn = liqSatisfiedBoolLit(*b, m);
	} else {
		// made it this far, unimplemented!
		std::runtime_error e("Domain::liqSatisfied not implemented yet!");
		throw e;
	}
	//LOG_PRINT(LOG_DEBUG) << "in liqSatisfied, sentence " << s.toString() << " is satisfied at " << toReturn.toString() << std::endl;
	return toReturn;
}

SISet Domain::liqSatisfiedAtom(const Atom& a, const Model& m) const {
	if (!a.isGrounded()) {
		std::runtime_error e("Domain::liqSatisfiedAtom currently doesn't support variables!");
		throw e;
	}
	// make sure its in model
	if (m.hasAtom(a)){
	//	LOG_PRINT(LOG_DEBUG) << "atom " << a.toString() << " true in model." << std::endl;
		SISet set = m.getAtom(a);
		set.setForceLiquid(true);
		return set;
	} else {
	//	LOG_PRINT(LOG_DEBUG) << "atom " << a.toString() << " NOT true in model." << std::endl;
		return SISet(true, maxInterval_);
	}
}

SISet Domain::liqSatisfiedNegation(const Negation& n, const Model& m) const {
	SISet sat = liqSatisfied(*(n.sentence()), m);
	sat = sat.compliment();
	return sat;
}

SISet Domain::liqSatisfiedDisjunction(const Disjunction& d, const Model& m) const {
	SISet leftSat = liqSatisfied(*(d.left()),m);
	SISet rightSat = liqSatisfied(*(d.right()), m);
	leftSat.add(rightSat);
	return leftSat;
}

SISet Domain::liqSatisfiedConjunction(const Conjunction& c, const Model& m) const {
	SISet leftSat = liqSatisfied(*(c.left()), m);
	SISet rightSat = liqSatisfied(*(c.right()), m);
	// intersection now!
	return intersection(leftSat, rightSat);
}

SISet Domain::liqSatisfiedBoolLit(const BoolLit& b, const Model& m) const {
	SISet toReturn(true, maxInterval_);
	if (b.value()) {
		// max interval
		SpanInterval max(maxInterval_.start(), maxInterval_.finish(), maxInterval_.start(), maxInterval_.finish(), maxInterval_);
		toReturn.add(max);
	}
	return toReturn;
}
