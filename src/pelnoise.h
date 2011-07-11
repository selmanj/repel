/*
 * pelnoise.h
 *
 *  Created on: Jul 6, 2011
 *      Author: joe
 */

#ifndef PELNOISE_H_
#define PELNOISE_H

#include "fol/fol.h"
#include "fol/domain.h"

std::pair<Model,Interval> constructModel(const std::vector<FOL::EventPair>& pairs);
Model subtractModel(const Model& from, const Model& toSubtract);
Model intersectModel(const Model& a, const Model& b);
Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);

unsigned long sizeModel(const Model& a);
Model rewritePELOutputAsConcreteModel(const Model& a);

#endif /* PELNOISE_H_ */
