/*
 * domain.h
 *
 *  Created on: May 20, 2011
 *      Author: joe
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <boost/tuple/tuple.hpp>
#include <set>
#include <string>
#include "fol.h"
#include "predcollector.h"

class Domain {
public:
	template <class FactsForwardIterator, class FormForwardIterator>
	Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
			FormForwardIterator formulasBegin, FormForwardIterator formulasEnd) {
		// create a class for collecting predicate names
		PredCollector predCollector;

		// collect all fact predicates
		for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
			boost::shared_ptr<Sentence> s = it->first;
			//it->get<0>()->visit(predCollector);
		}
		obsPreds_.insert(predCollector.preds.begin(), predCollector.preds.end());

		// now collect all unobserved preds
		predCollector.preds.clear();
		for (FormForwardIterator it = formulasBegin; it != formulasEnd; it++) {
			it->sentence()->visit(predCollector);
		}
		unobsPreds_.insert(predCollector.preds.begin(), predCollector.preds.end());
		unobsPreds_.erase(obsPreds_.begin(), obsPreds_.end());
	};
	virtual ~Domain() {};


private:
	std::set<std::string> obsPreds_;
	std::set<std::string> unobsPreds_;
	std::set<std::string> constants_;
};

#endif /* DOMAIN_H_ */
