/*
 * model.cpp
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */

#include <string>
#include <sstream>
#include "model.h"

Model::Model()
	: amap_() {

}

Model::Model(const std::vector<FOL::EventPair>& pairs)
	: amap_() {
	unsigned int smallest=UINT_MAX, largest=0;
	// find the max interval
	for (std::vector<FOL::EventPair>::const_iterator it = pairs.begin(); it != pairs.end(); it++) {
		SpanInterval interval = it->second;

		boost::optional<SpanInterval> norm = interval.normalize();
		if (!norm) {
			continue;
		}
		interval = norm.get();
		smallest = (std::min)(interval.start().start(), smallest);
		largest = (std::max)(interval.finish().finish(), largest);
	}

	Interval maxInterval = Interval(smallest, largest);

	// initialize observations
	for (std::vector<FOL::EventPair>::const_iterator it = pairs.begin(); it != pairs.end(); it++) {
		boost::shared_ptr<const Atom> atom = it->first;
		SpanInterval interval = it->second;

		// reinforce the max interval
		boost::optional<SpanInterval> opt = interval.setMaxInterval(maxInterval);
		if (!opt) continue;
		interval = opt.get();

		SISet set(true, maxInterval);

		set.add(interval);
		if (amap_.find(*atom) != amap_.end()) {
			set.add(amap_.find(*atom)->second);
			amap_.erase(*atom);
		}
		std::pair<Atom, SISet > pair(*atom, set);
		amap_.insert(pair);
	}
}

Model::Model(const Model& m)
	: amap_(m.amap_) {
}

Model::~Model() {
}

bool Model::hasAtom(const Atom& a) const {
	return amap_.find(a) != amap_.end();
}

SISet Model::getAtom(const Atom& a) const {
	if (!hasAtom(a)) {
		return SISet();
	}

	return amap_.at(a);
}


void Model::setAtom(const Atom& a, const SISet &set) {
	// check to see if atom is in the map
	if (hasAtom(a)) {
		SISet current = amap_[a];
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
	SISet current = amap_[a];
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
	atom_map resized;
	for (atom_map::iterator it = amap_.begin(); it != amap_.end(); it++) {
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

		if (toSubtract.amap_.find(a) != toSubtract.amap_.end()) {
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

		if (b.amap_.find(atom) != b.amap_.end()) {
			SISet intersect = intersection(set, b.amap_.find(atom)->second);
			if (intersect.size() != 0) amap_.insert(std::pair<Atom, SISet>(atom, intersect));
		}
	}
}

void Model::compliment(const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval) {
	Model newModel;

	BOOST_FOREACH(Atom atom, allAtoms) {
		if (amap_.find(atom) != amap_.end()) {
			Model::const_iterator it = amap_.find(atom);
			SISet set = it->second;
			if (set.compliment().size() != 0) {
				newModel.amap_.insert(std::pair<Atom, SISet>(atom, set.compliment()));
			}
		} else {
			SISet set(true, maxInterval);
			set.add(SpanInterval(maxInterval, maxInterval, maxInterval));
			newModel.amap_.insert(std::pair<Atom, SISet>(atom, set));
		}
	}

	swap(newModel);
}

unsigned long Model::size() const {
	unsigned long sum = 0;
	for (Model::const_iterator it = amap_.begin(); it != amap_.end(); it++) {
		sum += it->second.liqSize();
	}
	return sum;
}

std::string Model::toString() const {
	std::stringstream stream;

	for (atom_map::const_iterator it = amap_.begin(); it != amap_.end(); it++) {
		const Atom a = it->first;
		SISet set = it->second;
		stream << a.toString() << " @ " << set.toString() << std::endl;
	}
	return stream.str();
}


Model subtractModel(const Model& from, const Model& toSubtract) {
	Model mcopy(from);
	mcopy.subtract(toSubtract);
	return mcopy;
}

Model intersectModel(const Model& a, const Model& b) {
	Model mcopy(a);
	mcopy.intersect(b);
	return mcopy;
}

Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval) {
	Model mcopy(a);
	mcopy.compliment(allAtoms, maxInterval);
	return mcopy;
}
