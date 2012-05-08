#include "Domain.h"
#include "ELSyntax.h"
#include "Model.h"
#include "../Log.h"

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
*/
void swap(Domain& a, Domain& b) {
    using std::swap;

    swap(a.dontModifyObsPreds_, b.dontModifyObsPreds_);
    swap(a.maxInterval_, b.maxInterval_);
    swap(a.formulas_, b.formulas_);
    swap(a.partialModel_, b.partialModel_);
    swap(a.predTypes_, b.predTypes_);
    swap(a.allAtoms_, b.allAtoms_);
    swap(a.generator_, b.generator_);
}

std::size_t Domain::formulas_size() const {
    return formulas_.size();
}

SISet Domain::getModifiableSISet(const Atom& a) const {
    return getModifiableSISet(a, SISet(maxSpanInterval(), true, maxInterval_));
}

SISet Domain::getModifiableSISet(const Atom& a, const SISet& where) const {
    if (!dontModifyObsPreds_) {
        return where;
    }
    SISet modifiable = where;
    Proposition trueAt(a, true);
    Proposition falseAt(a, false);
    if (partialModel_.count(trueAt) != 0) modifiable.subtract(partialModel_.at(trueAt));
    if (partialModel_.count(falseAt) != 0) modifiable.subtract(partialModel_.at(falseAt));

    return modifiable;
}

void Domain::addFact(const ELSentence& e) {
    if (!e.hasInfWeight()) throw std::invalid_argument("Cannot enforce facts that have finite weight");
    if (e.sentence()->getTypeCode() == Atom::TypeCode) {
        boost::shared_ptr<const Atom> atom = boost::static_pointer_cast<const Atom>(e.sentence());
        addFact(Proposition(*atom, true), e.quantification());
    } else if (e.sentence()->getTypeCode() == Negation::TypeCode
            && (boost::static_pointer_cast<const Negation>(e.sentence())->sentence()->getTypeCode() == Atom::TypeCode)) {
        boost::shared_ptr<const Negation> neg = boost::static_pointer_cast<const Negation>(e.sentence());
        addFact(Proposition(*(boost::static_pointer_cast<const Atom>(neg->sentence())), false), e.quantification());
    } else {
        throw std::invalid_argument("unable to add fact because it's not a simple atom or its negation: "+ e.toString());
    }
}

void Domain::addFact(const Proposition& p, const SISet& where) {
    // resize where
    SISet newSet(where);
    if (!maxInterval_.isNull()) newSet.setMaxInterval(span(where.maxInterval(), maxInterval_));

    if (partialModel_.count(p) == 0) {
        partialModel_.insert(std::make_pair(p, newSet));
    } else {
        partialModel_.at(p).setMaxInterval(newSet.maxInterval());
        partialModel_.at(p).add(newSet);
    }
    predTypes_.insert(p.atom().predicateType());
    allAtoms_.insert(p.atom());
    growMaxInterval(where.maxInterval());
}
/*
void Domain::unsetAtomAt(const std::string& name, const SISet& where) {
    SISet newSet = obsPredsFixedAt_.at(name);
    newSet.subtract(where);
    obsPredsFixedAt_.erase(name);
    obsPredsFixedAt_.insert(std::pair<std::string, SISet>(name, newSet));

    observations_.unsetAtom(name, where);


}
*/

void Domain::addFormula(const ELSentence& e) {
    //ELSentence toAdd = e;
    formulas_.push_back(e);

    if (e.isQuantified()) {
        SISet set = e.quantification();
        Interval toAddMaxInt = set.maxInterval();
        growMaxInterval(toAddMaxInt);
        //if (toAddMaxInt != maxInterval_) {
         //   set.setMaxInterval(maxInterval_);
        //    toAdd.setQuantification(set);
       // }
    }
    PredicateTypeCollector pcollect;
    e.sentence()->visit(pcollect);
    predTypes_.insert(pcollect.types.begin(), pcollect.types.end());
    AtomCollector acollect;
    e.sentence()->visit(acollect);
    allAtoms_.insert(acollect.atoms.begin(), acollect.atoms.end());
    // update our list of unobs preds
    /*
    PredCollector collect;
    toAdd.sentence()->visit(collect);
    for (std::set<Atom, atomcmp>::const_iterator it = collect.preds.begin(); it != collect.preds.end(); it++) {
        if (obsPreds_.count(*it) == 0) unobsPreds_.insert(*it);
    }
    */
}

void Domain::addAtom(const Atom& a) {
    predTypes_.insert(a.predicateType());
    allAtoms_.insert(a);
}

Model Domain::randomModel() const {
    Model newModel(maxInterval_);
    //std::set<Atom, atomcmp> atoms = observations_.atoms();
    for (boost::unordered_set<Atom>::const_iterator it = allAtoms_.begin(); it != allAtoms_.end(); it++) {
        SISet random = SISet::randomSISet(isLiquid(it->name()), maxInterval_);
        // enforce our partial model
        Proposition trueProp(*it, true);
        Proposition falseProp(*it, false);
        if (partialModel_.count(trueProp) != 0) random.add(     partialModel_.at(trueProp));
        if (partialModel_.count(falseProp) != 0) random.subtract(partialModel_.at(falseProp));

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
        partialModel_.at(it->first).setMaxInterval(maxInterval);
    }
}

bool Domain::isLiquid(const std::string& predicate) const {
    // TODO: implement this sometime!
    return true;
}

double Domain::score(const ELSentence& w, const Model& m) const {
    SISet quantification = SISet(maxSpanInterval(), false, maxInterval());
    if (w.isQuantified()) quantification = w.quantification();

    SISet sat = w.sentence()->dSatisfied(m, *this, quantification);

    return (double)sat.size() * w.weight();
}

double Domain::score(const Model& m) const {
    double sum = 0.0;
    for (std::vector<ELSentence>::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        double x = score(*it, m);
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

Domain Domain::replaceInfForms() const {
    Domain d = *this;

    // find the highest weighted rule in the domain
    double maxWeight = 0.0;
    for (std::vector<ELSentence>::const_iterator it = d.formulas_.begin(); it != d.formulas_.end(); it++) {
       if (!it->hasInfWeight() && it->weight() > maxWeight) {
           maxWeight = it->weight();
       }
    }
    double hardWeight = 0.0;
    if (maxWeight == 0.0) hardWeight = 1.0;  // if nothing is weighted, a weight of 1 is all that's needed.
    else {
        hardWeight = maxWeight * Domain::hardFormulaFactor;
    }

    // now set all the infinite forms to have this new weight
    for (std::vector<ELSentence>::iterator it = d.formulas_.begin(); it != d.formulas_.end(); it++) {
        if (it->hasInfWeight()) it->setWeight(hardWeight);
    }

    return d;
}

void Domain::growMaxInterval(const Interval& maxInterval) {
    if (maxInterval_.isNull()) setMaxInterval(maxInterval);
    if (maxInterval.start() < maxInterval_.start()
            || maxInterval.finish() > maxInterval_.finish()) {
        setMaxInterval(span(maxInterval, maxInterval_));
    }
}

void Domain::printDebugDescription(std::ostream& out) const {
    out << "Domain at memory location: " << (void *)this << "\n";
    out << "  dontModifyObsPreds: " << dontModifyObsPreds_ << "\n";
    out << "  MaxInterval: " << maxInterval_ << "\n";
    out << "  Formulas:\n";
    for (std::vector<ELSentence>::const_iterator it = formulas_.begin(); it != formulas_.end(); it++) {
        out << "    " << *it << "\n";
    }
    out << "  Facts:\n";
    for (PropMap::const_iterator it = partialModel_.begin(); it != partialModel_.end(); it++) {
        std::pair<Proposition, SISet> pair = *it;
        out << "    " << pair.first << " @ " << pair.second << "\n";
    }
    // ignore atoms and generator for now
    out << "  allAtoms: ";
    std::copy(allAtoms_.begin(),  allAtoms_.end(), std::ostream_iterator<Atom>(out, ", "));
    out << std::endl;
}
