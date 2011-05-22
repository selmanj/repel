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
#include "fol.h"

class Domain {
public:
	template <class ForwardIterator>
	Domain(ForwardIterator factsBegin, ForwardIterator factsEnd, ForwardIterator formulasBegin, ForwardIterator formulasEnd) {
		// create a class for collecting predicate names
		class PredCollector : SentenceVisitor {
			std::set<std::string> preds;
			void accept(const Sentence& s) {
				// only care about atoms
				Atom* a = dynamic_cast<Atom*>(&s);
				if (a != NULL) {
					preds.insert(a->pred);
				}
			}
		} predCollector;

		// collect all fact predicates
		for (ForwardIterator it = factsBegin; it != factsEnd; it++) {
			it->visit(predCollector);
		}
		obsPreds_.insert(predCollector.preds.begin(), predCollector.preds.end());

		// now collect all unobserved preds
		predCollector.preds.clear();
		for (ForwardIterator it = formulasBegin; it != formulasEnd; it++) {
			it->visit(predCollector);
		}
		unobsPreds_.insert(predCollector.preds.begin(), predCollector.preds.end());
		unobsPreds_.erase(obsPreds_.begin(), obsPreds_.end());
	}
	virtual ~Domain();
private:
	std::set<std::string> obsPreds_;
	std::set<std::string> unobsPreds_;
	std::set<std::string> constants_;
};

#endif /* DOMAIN_H_ */
