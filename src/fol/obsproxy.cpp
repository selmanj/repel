/*
 * obsproxy.cpp
 *
 *  Created on: Sep 27, 2011
 *      Author: joe
 */

#include "obsproxy.h"
#include <map>

ObsProxy::ObsProxy(const Domain &d, const std::pair<Atom, SISet>& query)
: fullDomain_(d), masks_(), query_(query), numSamples_(8) {
	for (Model::atom_map::const_iterator it = d.defaultModel().begin(); it != d.defaultModel().end(); it++) {
		SISet allHidden(d.isLiquid(it->first), d.maxInterval());
		allHidden = allHidden.compliment();

		masks_.insert(std::pair<Atom, SISet>(it->first, allHidden));
	}
}


ObsProxy::~ObsProxy() {
	// TODO Auto-generated destructor stub
}

std::vector<boost::shared_ptr<Model> > ObsProxy::generateSamples() {
	// setup our domain
	Domain d(fullDomain_);
	d.setAssumeClosedWorld(false);

	std::vector<boost::shared_ptr<Model> > result;
	for (int count = 0; count < numSamples_; count++) {
		Model model = d.randomModel();
		model = maxWalkSat(d, 1000, .2, &model);
		boost::shared_ptr<Model> mptr(new Model(model));
		result.push_back(mptr);
		LOG(LOG_DEBUG) << "sample " << count << ": " << std::endl << model.toString();
	}

	return result;
}
