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

class ObsProxy {
public:
	ObsProxy(const Domain &d, const std::pair<Atom, SISet>& query);
	virtual ~ObsProxy();

	std::vector<boost::shared_ptr<Model> > generateSamples();
private:
	Domain fullDomain_;
	std::map<Atom, SISet> masks_;
	std::pair<Atom, SISet> query_;
	unsigned int numSamples_;
};

#endif /* OBSPROXY_H_ */
