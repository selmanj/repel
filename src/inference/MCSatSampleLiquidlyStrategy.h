/*
 * MCSatSampleLiquidlyStrategy.h
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSATSAMPLELIQUIDLYSTRATEGY_H_
#define MCSATSAMPLELIQUIDLYSTRATEGY_H_

#include <boost/random/mersenne_twister.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/void_cast.hpp>
#include "MCSatSampleStrategy.h"


class MCSatSampleLiquidlyStrategy: public MCSatSampleStrategy {
public:
    MCSatSampleLiquidlyStrategy();
    virtual ~MCSatSampleLiquidlyStrategy();

    virtual MCSatSampleStrategy* clone() const;
    virtual void sampleSentences(const Model& m, const Domain& d, boost::mt19937& rng, std::vector<ELSentence>& sampled);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        // dont serialize the base class
        boost::serialization::void_cast_register<MCSatSampleLiquidlyStrategy, MCSatSampleStrategy>(
                static_cast<MCSatSampleLiquidlyStrategy*>(NULL),
                static_cast<MCSatSampleStrategy*>(NULL)
        );
        // no state to serialize
    }

    virtual bool doEquals(const MCSatSampleStrategy& s) const {
        // just see if its the same type
        return dynamic_cast<const MCSatSampleLiquidlyStrategy*>(&s) != NULL;
    }
};

#endif /* MCSATSAMPLELIQUIDLYSTRATEGY_H_ */
