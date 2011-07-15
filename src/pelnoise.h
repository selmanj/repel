/*
 * pelnoise.h
 *
 *  Created on: Jul 6, 2011
 *      Author: joe
 */

#ifndef PELNOISE_H_
#define PELNOISE_H

#include <boost/tuple/tuple.hpp>
#include "fol/fol.h"
#include "fol/domain.h"

struct atompair_cmp {
	bool operator()(const std::pair<Atom, Atom>& a, const std::pair<Atom, Atom>& b) const {
		if (a.first.toString() == b.first.toString())
			return (a.second.toString() < b.second.toString());
		else return (a.first.toString() < b.first.toString());
	}
};

typedef std::map<std::pair<Atom, Atom>, unsigned int*, atompair_cmp> ConfMatrix;

std::set<Atom, atomcmp> generateValidPreds();
ConfMatrix confusionMatrix(const Model& groundTruth, const Model& model, double threshhold);
boost::tuple<unsigned int, unsigned int, unsigned int, unsigned int> getThreshholdAccuracy(const Model& groundTruth, const Model& model, double threshhold);
std::pair<Model,Interval> constructModel(const std::vector<FOL::EventPair>& pairs);
Model subtractModel(const Model& from, const Model& toSubtract);
Model intersectModel(const Model& a, const Model& b);
Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);

unsigned long sizeModel(const Model& a);
Model rewritePELOutputAsConcreteModel(const Model& a);

#endif /* PELNOISE_H_ */
