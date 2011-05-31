/*
 * domain.h
 *
 *  Created on: May 20, 2011
 *      Author: joe
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

#include <set>
#include <string>
#include <map>
#include <utility>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <boost/optional.hpp>
#include "fol.h"
#include "predcollector.h"
#include "../siset.h"

struct atomcmp {
	bool operator()(const Atom& a, const Atom& b) const {
		return a.toString() < b.toString();
	}
};

typedef std::map<Atom, SISet, atomcmp> Model;

class Domain {
public:
	template <class FactsForwardIterator, class FormForwardIterator>
	Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
			FormForwardIterator formulasBegin, FormForwardIterator formulasEnd)
			: maxInterval_(0,0) {
		// create a class for collecting predicate names
		PredCollector predCollector;

		// find the maximum interval of time
		if (factsBegin == factsEnd) {
			// what to do for time??
			std::runtime_error e("no facts given: currently need at least one fact to determine the interval to reason over!");
			throw e;
		}
		unsigned int smallest=UINT_MAX, largest=0;
		for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
			SpanInterval interval = it->second;

			boost::optional<SpanInterval> norm = interval.normalize();
			if (!norm) {
				continue;
			}
			interval = norm.get();
			smallest = std::min(interval.start().start(), smallest);
			largest = std::max(interval.end().end(), largest);
		}
		maxInterval_ = Interval(smallest, largest);

		// collect all fact predicates
		for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
			boost::shared_ptr<const Atom> s = it->first;
			it->first->visit(predCollector);
		}
		obsPreds_.insert(predCollector.preds.begin(), predCollector.preds.end());
		// now collect all unobserved preds
		predCollector.preds.clear();
		for (FormForwardIterator it = formulasBegin; it != formulasEnd; it++) {
			it->sentence()->visit(predCollector);
		}

		// remove the predicates we know are observed
		std::set_difference(predCollector.preds.begin(), predCollector.preds.end(),
				obsPreds_.begin(), obsPreds_.end(),
				std::inserter(unobsPreds_, unobsPreds_.end()));

		// initialize observations
		for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
			boost::shared_ptr<const Atom> atom = it->first;
			SpanInterval interval = it->second;

			// reinforce the max interval
			interval.setMaxInterval(maxInterval_);
			// TODO: we are hardwired for liquidity, come back and fix this later
			SISet set(true, maxInterval_);

			set.add(interval);
			std::pair<Atom, SISet > pair(*atom, set);
			observations_.insert(pair);
		}

	};
	virtual ~Domain() {};

	bool isLiquid(const std::string& predicate) const;
	Model defaultModel() const {return observations_;};
	Interval maxInterval() const {return maxInterval_;};
	void setMaxInterval(const Interval& maxInterval) {
		maxInterval_ = Interval(maxInterval);
		Model resized;
		for (std::map<const Atom, SISet>::iterator it = observations_.begin(); it != observations_.end(); it++) {
			const Atom atom = it->first;
			SISet set = it->second;
			set.setMaxInterval(maxInterval);
			resized.insert(std::pair<const Atom, SISet>(atom,set));
		}
		observations_.swap(resized);
	}

	SISet satisfied(const Sentence& s, const Model& m) const;
	SISet satisfiedAtom(const Atom& a, const Model& m) const;
	SISet satisfiedNegation(const Negation& n, const Model& m) const;
	SISet satisfiedDisjunction(const Disjunction& d, const Model& m) const;

	SISet liqSatisfied(const Sentence& s, const Model& m) const;
	SISet liqSatisfiedAtom(const Atom& a, const Model& m) const;
	SISet liqSatisfiedNegation(const Negation& a, const Model& m) const;
	SISet liqSatisfiedDisjunction(const Disjunction& d, const Model& m) const;
	SISet liqSatisfiedConjunction(const Conjunction& c, const Model& m) const;


private:
	std::set<std::string> obsPreds_;
	std::set<std::string> unobsPreds_;
	std::set<std::string> constants_;
	Interval maxInterval_;

	std::vector<WSentence> formulas_;
	Model observations_;
};

#endif /* DOMAIN_H_ */
