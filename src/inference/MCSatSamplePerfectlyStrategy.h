#ifndef MCSATSAMPLEPERFECTLYSTRATEGY_H_
#define MCSATSAMPLEPERFECTLYSTRATEGY_H_

#include "MCSatSampleStrategy.h"

class MCSatSamplePerfectlyStrategy : public MCSatSampleStrategy {
public:
    virtual MCSatSamplePerfectlyStrategy* clone() const;

    virtual void sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled);
};

inline MCSatSamplePerfectlyStrategy* MCSatSamplePerfectlyStrategy::clone() const {
    return new MCSatSamplePerfectlyStrategy;    //no state
}



#endif
