/*
 * obsproxy.cpp
 *
 *  Created on: Sep 27, 2011
 *      Author: joe
 */

#include "obsproxy.h"
#include "../log.h"
#include "moves.h"
#include <map>
#include <vector>
#include <list>
#include <cstdlib>
#include <climits>
#include "../si_histogram.h"



ObsProxy::ObsProxy(const Domain &d, const Atom& queryAtom, const SISet& querySet)
: fullDomain_(d), masks_(), queryAtom_(queryAtom), querySet_(querySet), numSamples_(1) {
	LOG_PRINT(LOG_DEBUG) << "defaultModel = " << d.defaultModel().toString();
	std::set<Atom, atomcmp> atoms = d.defaultModel().atoms();
	BOOST_FOREACH(Atom atom, atoms) {
		SISet allHidden(d.isLiquid(atom.name()), d.maxInterval());
		allHidden = allHidden.compliment();

		masks_.insert(std::pair<Atom, SISet>(atom, allHidden));
	}
}


ObsProxy::~ObsProxy() {
	// TODO Auto-generated destructor stub
}

std::vector<Model> ObsProxy::generateSamples() {
	// setup our domain
	Domain d(fullDomain_);
	d.setAssumeClosedWorld(false);
	for(std::map<Atom, SISet, atomcmp>::const_iterator it = masks_.begin(); it != masks_.end(); it++) {
		d.unsetAtomAt(it->first.name(), it->second);
	}

	std::vector<Model > result;
	for (int count = 0; count < numSamples_; count++) {
		Model model = d.randomModel();
		model = maxWalkSat(d, 200, .25, &model);
		//boost::shared_ptr<Model> mptr(new Model(model));
		result.push_back(model);
		LOG_PRINT(LOG_DEBUG) << "sample " << count << ": " << std::endl << model.toString();
	}

	return result;
}

std::map<Model, int, modelcmp > ObsProxy::kMedoid(unsigned int k, const std::vector<Model>& models) const {
	LOG(LOG_DEBUG) << "entered kMedoid!";
	std::map<Model, int, modelcmp > assignments;
	std::map<int, boost::shared_ptr<std::list<Model> > > clusterToModels;
	std::vector<boost::shared_ptr<SISet> > medoids;
	std::vector<boost::shared_ptr<SIHistogram> > histograms;

	// initialize dynamic ptrs
	for (int i = 0; i < k; i++) {
		boost::shared_ptr<SISet> set(new SISet(false, fullDomain_.maxInterval()));
		boost::shared_ptr<SIHistogram> hist(new SIHistogram(false, fullDomain_.maxInterval()));
		boost::shared_ptr<std::list<Model> > list(new std::list<Model>());
		medoids.push_back(set);
		histograms.push_back(hist);
		clusterToModels[i] = list;
	}
	// initialize points
	BOOST_FOREACH(Model model, models) {
		int assignment = (rand() % k);
		assignments.insert(std::pair<Model, int>(model, assignment));
		clusterToModels[assignment]->push_back(model);
	}

	// ok time for the while loop
	bool changesMade=true;
	while (changesMade) {
		changesMade = false;
		// compute new medoid
		for (int i = 0; i < k; i++) {
			histograms[i]->clear();
		}
		for (std::map<Model, int >::const_iterator it = assignments.begin(); it != assignments.end(); it++) {
			Model model = it->first;
			int assignment = it->second;

			SISet whereTrue;
			if (!model.hasAtom(queryAtom_)) {
				whereTrue = SISet(false, fullDomain_.maxInterval());
			} else {
				whereTrue = intersection(model.getAtom(queryAtom_), querySet_);
			}
			histograms[assignment]->add(whereTrue);
		}
		for (int i = 0; i < k; i++) {
			medoids[i]->clear();
			std::map<SpanInterval, int> counts = histograms[i]->counts();
			for (std::map<SpanInterval, int>::const_iterator it = counts.begin(); it != counts.end(); it++) {
				if (it->second >= clusterToModels[i]->size()/2.0) {
					medoids[i]->add(it->first);
				}
			}
		}
		// now that we have the medoids, for each model reassign it to the closest medoid
		std::map<Model, int, modelcmp> newAssignments;
		for (std::map<Model, int>::const_iterator it = assignments.begin(); it != assignments.end(); it++) {
			Model model = it->first;
			int prevAssignment = it->second;

			SISet whereTrue;
			if (!model.hasAtom(queryAtom_)) {
				whereTrue = SISet(false, fullDomain_.maxInterval());
			} else {
				whereTrue = intersection(model.getAtom(queryAtom_), querySet_);
			}

			int bestFound = 0;
			unsigned long bestScore = ULONG_MAX;
			for (int i = 0; i < k; i++) {
				unsigned long score = hammingDistance(whereTrue, *medoids[i]);
				if (score < bestScore) {
					bestFound = i;
					bestScore = score;
				}
			}
			// check for reassignment
			if (bestFound != prevAssignment) {
				changesMade = true;
				clusterToModels[prevAssignment]->remove(model);
				clusterToModels[bestFound]->push_back(model);

			}
			newAssignments.insert(std::pair<Model, int>(model, bestFound));
		}
		assignments = newAssignments;
	}
	LOG(LOG_DEBUG) << "leaving kMedoid";
	return assignments;

}

