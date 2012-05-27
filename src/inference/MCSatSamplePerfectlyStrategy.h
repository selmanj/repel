#ifndef MCSATSAMPLEPERFECTLYSTRATEGY_H_
#define MCSATSAMPLEPERFECTLYSTRATEGY_H_

#include <boost/random/mersenne_twister.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/void_cast.hpp>
#include "MCSatSampleStrategy.h"

class MCSatSamplePerfectlyStrategy : public MCSatSampleStrategy {
public:
    virtual MCSatSamplePerfectlyStrategy* clone() const;

    virtual void sampleSentences(const Model& m, const Domain& d, boost::mt19937& rng, std::vector<ELSentence>& sampled);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    virtual bool doEquals(const MCSatSampleStrategy& s) const {
        // just see if its the same type
        return dynamic_cast<const MCSatSamplePerfectlyStrategy*>(&s) != NULL;
    }
};

inline MCSatSamplePerfectlyStrategy* MCSatSamplePerfectlyStrategy::clone() const {
    return new MCSatSamplePerfectlyStrategy;    //no state
}

template <class Archive>
void MCSatSamplePerfectlyStrategy::serialize(Archive& ar, const unsigned int version) {
    // explicitly dont call the base class
    boost::serialization::void_cast_register<MCSatSamplePerfectlyStrategy, MCSatSampleStrategy>(
            static_cast<MCSatSamplePerfectlyStrategy*>(NULL),
            static_cast<MCSatSampleStrategy*>(NULL)
    );
    // no state
}



#endif
