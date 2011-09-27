/*
 * obsproxy.h
 *
 *  Created on: Sep 27, 2011
 *      Author: joe
 */

#ifndef OBSPROXY_H_
#define OBSPROXY_H_

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "fol.h"
#include "domain.h"
#include "model.h"

struct modelcmp {
	bool operator()(const Model& a, const Model& b) const {
		return a.toString() < b.toString();
	}
};

class ObsProxy {
public:
	ObsProxy(const Domain &d, const Atom& queryAtom, const SISet& querySet);
	virtual ~ObsProxy();

	std::vector<Model> generateSamples();
	std::map<Model, int, modelcmp > kMedoid(unsigned int k, const std::vector<Model>& models) const;
private:
	Domain fullDomain_;
	std::map<Atom, SISet, atomcmp> masks_;
	Atom queryAtom_;
	SISet querySet_;
	unsigned int numSamples_;
};

#endif /* OBSPROXY_H_ */
