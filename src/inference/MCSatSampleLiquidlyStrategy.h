/*
 * MCSatSampleLiquidlyStrategy.h
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSATSAMPLELIQUIDLYSTRATEGY_H_
#define MCSATSAMPLELIQUIDLYSTRATEGY_H_

#include "MCSatSampleStrategy.h"

class MCSatSampleLiquidlyStrategy: public MCSatSampleStrategy {
public:
    MCSatSampleLiquidlyStrategy();
    virtual ~MCSatSampleLiquidlyStrategy();

    virtual MCSatSampleStrategy* clone() const;
    virtual void sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled);
};

#endif /* MCSATSAMPLELIQUIDLYSTRATEGY_H_ */
