#ifndef MCSATSAMPLEPERFECTLYSTRATEGY_H_
#define MCSATSAMPLEPERFECTLYSTRATEGY_H_

#include <boost/random/mersenne_twister.hpp>
#include "MCSatSampleStrategy.h"

class MCSatSamplePerfectlyStrategy : public MCSatSampleStrategy {
public:
    virtual MCSatSamplePerfectlyStrategy* clone() const;

    virtual void sampleSentences(const Model& m, const Domain& d, boost::mt19937& rng, std::vector<ELSentence>& sampled);
};

inline MCSatSamplePerfectlyStrategy* MCSatSamplePerfectlyStrategy::clone() const {
    return new MCSatSamplePerfectlyStrategy;    //no state
}



#endif
