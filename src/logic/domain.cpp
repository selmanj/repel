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
    obsPredsFixedAt_.insert(std::pair<std::string, SISet>(a.name(), newSet));
    observations_.setAtom(a, newSet);
}

SISet Domain::getModifiableSISet(const std::string& name) const {
    SISet everywhere(isLiquid(name), maxInterval_);
    everywhere = everywhere.compliment();
    return getModifiableSISet(name, everywhere);
}

SISet Domain::getModifiableSISet(const std::string& name, const SISet& where) const {
    // check to see if its an obs predicate
    if (obsPredsFixedAt_.find(name) == obsPredsFixedAt_.end() || !dontModifyObsPreds()) {
        return where;
    }

    SISet modifiable(where.forceLiquid(), where.maxInterval());
    if (assumeClosedWorld()) return modifiable; // if its a closed world, can't change any obs predicates
    modifiable = where;

    modifiable.subtract(obsPredsFixedAt_.find(name)->second);
    return modifiable;
}

void Domain::unsetAtomAt(const std::string& name, const SISet& where) {
    SISet newSet = obsPredsFixedAt_.at(name);
    newSet.subtract(where);
    obsPredsFixedAt_.erase(name);
    obsPredsFixedAt_.insert(std::pair<std::string, SISet>(name, newSet));

    observations_.unsetAtom(name, where);


}


Model Domain::randomModel() const {
    Model newModel;
    //std::set<Atom, atomcmp> atoms = observations_.atoms();
    boost::unordered_set<Atom> atoms = obsPreds_;
    std::copy(unobsPreds_.begin(), unobsPreds_.end(), std::inserter(atoms, atoms.end()));

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
    SISet quantification = SISet(maxSpanInterval(), false, maxInterval());
    if (w.isQuantified()) quantification = w.quantification();

    SISet sat = w.sentence()->dSatisfied(m, *this, quantification);
    if (!sat.isDisjoint()) sat.makeDisjoint();
    return sat.size() * w.weight();
}

unsigned long Domain::score(const Model& m) const {
    unsigned long sum = 0;
    for (std::vector<ELSentence>::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        sum += score(*it, m);
    }
    return sum;
}

bool Domain::isFullySatisfied(const Model& m) const {
    for (formula_const_iterator it = formulas_begin(); it != formulas_end(); it++) {
        if (!it->fullySatisfied(m, *this)) return false;
    }
    return true;
}
