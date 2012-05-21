/*
 * Model.cpp
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */

#include <string>
#include <sstream>
#include "Model.h"
#include "ELSyntax.h"

Model::Model(const std::vector<FOL::Event>& pairs, const Interval& maxInterval)
    : amap_(), maxInterval_(maxInterval) {
    /*
    unsigned int smallest=UINT_MAX, largest=0;
    // find the max interval
    for (std::vector<FOL::Event>::const_iterator it = pairs.begin(); it != pairs.end(); it++) {

        SpanInterval interval = it->where();

        boost::optional<SpanInterval> norm = interval.normalize();
        if (!norm) {
            continue;
        }
        interval = norm.get();
        smallest = (std::min)(interval.start().start(), smallest);
        largest = (std::max)(interval.finish().finish(), largest);
    }

    Interval maxInterval = Interval(smallest, largest);
    */

    // initialize observations
    for (std::vector<FOL::Event>::const_iterator it = pairs.begin(); it != pairs.end(); it++) {
        boost::shared_ptr<const Atom> atom = it->atom();
        SpanInterval interval = it->where();
        bool truthVal = it->truthVal();

        SISet set(true, maxInterval_);

        if (truthVal) set.add(interval);
        if (amap_.count(*atom) == 1) {
            set.add(amap_.find(*atom)->second);
            amap_.erase(*atom);
        }
        std::pair<Atom, SISet > pair(*atom, set);
        amap_.insert(pair);
    }
}

Model::Model(const boost::unordered_map<Proposition, SISet>& partialModel, const Interval& maxInterval)
    : amap_(), maxInterval_(maxInterval) {
    for(boost::unordered_map<Proposition, SISet>::const_iterator it = partialModel.begin();
            it != partialModel.end(); it++) {
        if (amap_.count(it->first.atom()) == 0) {
            SISet empty(it->second.forceLiquid(), maxInterval_);
            amap_.insert(std::pair<const Atom, SISet>(it->first.atom(), empty));
        }
        if (it->first.sign()) {
            amap_.at(it->first.atom()).add(it->second);
        } else {
            amap_.at(it->first.atom()).subtract(it->second);
        }
    }
}

/*
Model::Model(const Model& m)
    : amap_(m.amap_) {
}
*/
/*
Model::~Model() {
}
*/

bool Model::hasAtom(const Atom& a) const {
    return amap_.count(a) == 1;
}

SISet Model::getAtom(const Atom& a) const {
    if (!hasAtom(a)) {
        return SISet(false, maxInterval_);
    }

    return amap_.at(a);
}


void Model::setAtom(const Atom& a, const SISet &set) {
    // check to see if atom is in the map
    if (hasAtom(a)) {
        SISet current = amap_.at(a);
        current.add(set);
        // replace the previous element
        amap_.erase(a);
        amap_.insert(std::pair<const Atom, SISet>(a,current));
    } else {
        amap_.insert(std::pair<const Atom, SISet>(a,set));
    }
}

void Model::unsetAtom(const Atom& a, const SISet &set) {
    if (!hasAtom(a)) return;
    SISet current = amap_.at(a);
    current.subtract(set);
    amap_.erase(a);
    if (current.size() != 0) {
        amap_.insert(std::pair<const Atom, SISet>(a, current));
    }
}

void Model::clearAtom(const Atom& a) {
    amap_.erase(a);
}


void Model::setMaxInterval(const Interval& maxInterval) {
    maxInterval_ = maxInterval;
    boost::unordered_map<Atom, SISet> resized;
    for (boost::unordered_map<Atom, SISet>::iterator it = amap_.begin(); it != amap_.end(); it++) {
        const Atom a = it->first;
        SISet b = it->second;

        b.setMaxInterval(maxInterval);
        resized.insert(std::pair<const Atom, SISet>(a,b));
    }

    amap_.swap(resized);
}

void Model::subtract(const Model& toSubtract) {
    for (Model::const_iterator it = amap_.begin(); it != amap_.end(); it++) {
        Atom a = it->first;
        SISet set = it->second;

        if (toSubtract.amap_.count(a) == 1) {
            SISet setSubtract = toSubtract.amap_.find(a)->second;
            set.subtract(setSubtract);
        }
        if (set.size() != 0) amap_.insert(std::pair<Atom, SISet>(a, set));
    }
}

void Model::intersect(const Model& b) {

    for (Model::const_iterator it = amap_.begin(); it != amap_.end(); it++) {
        Atom atom = it->first;
        SISet set = it->second;

        if (b.amap_.count(atom) == 1) {
            SISet intersect = intersection(set, b.amap_.find(atom)->second);
            if (intersect.size() != 0) amap_.insert(std::pair<Atom, SISet>(atom, intersect));
        }
    }
}

unsigned long Model::size() const {
    unsigned long sum = 0;
    for (Model::const_iterator it = amap_.begin(); it != amap_.end(); it++) {
        sum += it->second.liqSize();
    }
    return sum;
}

std::ostream& operator<<(std::ostream& out, const Model& m) {
    // collect the keys, sort them, then print
    std::list<Atom> atoms;

    for (boost::unordered_map<Atom, SISet>::const_iterator it = m.amap_.begin(); it != m.amap_.end(); it++) {
        std::pair<Atom, SISet> pair = *it;
        atoms.push_back(pair.first);
    }
    atoms.sort(AtomStringCompare());

    for (std::list<Atom>::const_iterator it = atoms.begin(); it != atoms.end(); it++) {
        out << it->toString() << " @ " << m.amap_.at(*it) << "\n";
    }
    return out;
}

std::string Model::toString() const {
    std::stringstream strstr;
    strstr << *this;
    return strstr.str();
}

