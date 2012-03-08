#include "domain.h"
#include "el_syntax.h"
#include "model.h"
#include "../log.h"

#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <sstream>
#include <vector>

/*
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
*/

void Domain::addFormula(const ELSentence& e) {
    ELSentence toAdd = e;
    if (toAdd.isQuantified()) {
        SISet set = toAdd.quantification();
        Interval toAddMaxInt = set.maxInterval();
        growMaxInterval(toAddMaxInt);
        //if (toAddMaxInt != maxInterval_) {
         //   set.setMaxInterval(maxInterval_);
        //    toAdd.setQuantification(set);
       // }
    }
    PredicateTypeCollector pcollect;
    toAdd.sentence()->visit(pcollect);
    predTypes_.insert(pcollect.types.begin(), pcollect.types.end());
    AtomCollector acollect;
    toAdd.sentence()->visit(acollect);
    allAtoms_.insert(acollect.atoms.begin(), acollect.atoms.end());
    // update our list of unobs preds
    /*
    PredCollector collect;
    toAdd.sentence()->visit(collect);
    for (std::set<Atom, atomcmp>::const_iterator it = collect.preds.begin(); it != collect.preds.end(); it++) {
        if (obsPreds_.count(*it) == 0) unobsPreds_.insert(*it);
    }
    */
    formulas_.push_back(e);
}

Model Domain::randomModel() const {
    Model newModel;
    //std::set<Atom, atomcmp> atoms = observations_.atoms();
    for (boost::unordered_set<Atom>::const_iterator it = predTypes_.begin(); it != predTypes_.end(); it++) {
        SISet random = SISet::randomSISet(isLiquid(it->name()), maxInterval_);
        // enforce our partial model
        random.add(     partialModel_[Proposition(*it, true)]);
        random.subtract(partialModel_[Proposition(*it, false)]);
        random.makeDisjoint();
        //newModel.clearAtom(obsPair->first);
        newModel.setAtom(*it, random);
    }
    return newModel;
}


void Domain::setMaxInterval(const Interval& maxInterval) {
    maxInterval_ = maxInterval;
    // resize formulas
    for (std::vector<ELSentence>::iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        if (it->isQuantified()) {
            SISet copy(it->quantification());
            copy.setMaxInterval(maxInterval);
            it->setQuantification(copy);
        }
    }
    for (PropMap::iterator it = partialModel_.begin(); it != partialModel_.end(); it++) {
        it->second.setMaxInterval(maxInterval);
    }
}

bool Domain::isLiquid(const std::string& predicate) const {
    // TODO: implement this sometime!
    return true;
}

score_t Domain::score(const ELSentence& w, const Model& m) const {
    SISet quantification = SISet(maxSpanInterval(), false, maxInterval());
    if (w.isQuantified()) quantification = w.quantification();

    SISet sat = w.sentence()->dSatisfied(m, *this, quantification);

    // check for overflow.
    if (sat.size() > std::numeric_limits<score_t>::max() / w.weight()) {
        throw std::runtime_error("integer overflow when scoring domain!");
    }
    return sat.size() * w.weight();
}

score_t Domain::score(const Model& m) const {
    score_t sum = 0;
    for (std::vector<ELSentence>::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        score_t x = score(*it, m);
        if (std::numeric_limits<score_t>::max() - x < sum) {
            throw std::runtime_error("integer overflow when scoring domain!");
        }
        sum += x;
    }
    return sum;
}

bool Domain::isFullySatisfied(const Model& m) const {
    for (formula_const_iterator it = formulas_begin(); it != formulas_end(); it++) {
        if (!it->fullySatisfied(m, *this)) return false;
    }
    return true;
}
