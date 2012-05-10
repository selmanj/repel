/*
 * MCSatSampleLiquidlyStrategy.h
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSATSAMPLELIQUIDLYSTRATEGY_H_
#define MCSATSAMPLELIQUIDLYSTRATEGY_H_

#include <boost/random/mersenne_twister.hpp>
#include "MCSatSampleStrategy.h"


class MCSatSampleLiquidlyStrategy: public MCSatSampleStrategy {
public:
    MCSatSampleLiquidlyStrategy();
    virtual ~MCSatSampleLiquidlyStrategy();

    virtual MCSatSampleStrategy* clone() const;
    virtual void sampleSentences(const Model& m, const Domain& d, boost::mt19937& rng, std::vector<ELSentence>& sampled);
};

#endif /* MCSATSAMPLELIQUIDLYSTRATEGY_H_ */
