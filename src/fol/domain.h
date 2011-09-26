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
#include "model.h"
#include "../siset.h"
#include "namegenerator.h"


std::string modelToString(const Model& m);

class Domain {
public:
	Domain() : dontModifyObsPreds_(true), maxInterval_(0,0), formulas_(), generator_() {}
	template <class FactsForwardIterator, class FormForwardIterator>
	Domain(FactsForwardIterator factsBegin, FactsForwardIterator factsEnd,
			FormForwardIterator formulasBegin, FormForwardIterator formulasEnd,
			bool assumeClosedWorld=true)
			: assumeClosedWorld_(assumeClosedWorld), dontModifyObsPreds_(true), maxInterval_(0,0), formulas_(formulasBegin, formulasEnd), generator_() {

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
			smallest = (std::min)(interval.start().start(), smallest);
			largest = (std::max)(interval.finish().finish(), largest);
		}
		maxInterval_ = Interval(smallest, largest);

		// collect all fact predicates
		for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
			boost::shared_ptr<const Atom> a = it->first;
			SpanInterval si = it->second;
			if (obsPreds_.find(a->name()) == obsPreds_.end()) {
				SISet newSet(true, maxInterval_);		// TODO: assumes all unobs are liquid!!!
				newSet.add(si);
				obsPreds_.insert(std::pair<std::string, SISet>(a->name(), newSet));
			} else {
				SISet curSet = obsPreds_.find(a->name())->second;
				curSet.add(si);
				obsPreds_.erase(a->name());
				obsPreds_.insert(std::pair<std::string, SISet>(a->name(), curSet));
			}
			//it->first->visit(predCollector);
		}
	//	obsPreds_.insert(predCollector.preds.begin(), predCollector.preds.end());
		// now collect all unobserved preds
		//predCollector.preds.clear();
		for (FormForwardIterator it = formulasBegin; it != formulasEnd; it++) {
			it->sentence()->visit(predCollector);
		}

		// remove the predicates we know are observed
		std::set<std::string> obsJustPreds;
		for (std::map<std::string, SISet>::const_iterator it = obsPreds_.begin();
				it != obsPreds_.end();
				it++) {
			obsJustPreds.insert(it->first);
		}
		std::set_difference(predCollector.preds.begin(), predCollector.preds.end(),
				obsJustPreds.begin(), obsJustPreds.end(),
				std::inserter(unobsPreds_, unobsPreds_.end()));

		// initialize observations
		for (FactsForwardIterator it = factsBegin; it != factsEnd; it++) {
			boost::shared_ptr<const Atom> atom = it->first;
			SpanInterval interval = it->second;

			// reinforce the max interval
			boost::optional<SpanInterval> opt = interval.setMaxInterval(maxInterval_);
			if (!opt) continue;
			interval = opt.get();
			// TODO: we are hardwired for liquidity, come back and fix this later
			SISet set(true, maxInterval_);

			set.add(interval);
			/*
			if (observations_.find(*atom) != observations_.end()) {
				set.add(observations_.find(*atom)->second);
				observations_.erase(*atom);
			}
			std::pair<Atom, SISet > pair(*atom, set);
			observations_.insert(pair);
			*/
			observations_.setAtom(*atom, set);
		}

	};
	virtual ~Domain() {};

	const std::vector<WSentence>& formulas() const {return formulas_;};
	const std::map<std::string, SISet>& observedPredicates() const {return obsPreds_;};
	SISet getModifiableSISet(const std::string& name, const SISet& where) const;
	NameGenerator& nameGenerator() {return generator_;};
	Model defaultModel() const {return observations_;};
	Model randomModel() const;
	Interval maxInterval() const {return maxInterval_;};
	void setMaxInterval(const Interval& maxInterval);

	bool isLiquid(const std::string& predicate) const;
	bool dontModifyObsPreds() const {return dontModifyObsPreds_;};
	bool assumeClosedWorld() const {return assumeClosedWorld_;};
	void setDontModifyObsPreds(bool b) {dontModifyObsPreds_ = b;};
	void setAssumeClosedWorld(bool b) {assumeClosedWorld_ = b;};

	unsigned long score(const WSentence& s, const Model& m) const;
	unsigned long score(const Model& m) const;

	SISet satisfied(const Sentence& s, const Model& m) const;
	SISet satisfiedAtom(const Atom& a, const Model& m) const;
	SISet satisfiedNegation(const Negation& n, const Model& m) const;
	SISet satisfiedDisjunction(const Disjunction& d, const Model& m) const;
	SISet satisfiedDiamond(const DiamondOp& d, const Model& m) const;
	SISet satisfiedConjunction(const Conjunction& c, const Model& m) const;
	SISet satisfiedBoolLit(const BoolLit& b, const Model& m) const;

	SISet liqSatisfied(const Sentence& s, const Model& m) const;
	SISet liqSatisfiedAtom(const Atom& a, const Model& m) const;
	SISet liqSatisfiedNegation(const Negation& a, const Model& m) const;
	SISet liqSatisfiedDisjunction(const Disjunction& d, const Model& m) const;
	SISet liqSatisfiedConjunction(const Conjunction& c, const Model& m) const;
	SISet liqSatisfiedBoolLit(const BoolLit& b, const Model& m) const;

private:
	bool dontModifyObsPreds_;
	bool assumeClosedWorld_;

	std::map<std::string, SISet> obsPreds_;
	std::set<std::string> unobsPreds_;
	std::set<std::string> constants_;
	Interval maxInterval_;

	std::vector<WSentence> formulas_;
	Model observations_;

	NameGenerator generator_;
};

#endif /* DOMAIN_H_ */
