/*
 * MCSatSampleStrategy.h
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSATSAMPLERSTRATEGY_H_
#define MCSATSAMPLERSTRATEGY_H_

#include <boost/utility.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <vector>

class Model;
class Domain;
class ELSentence;

/**
 * Abstract Class representing the strategy used for sampling constraints.
 */
class MCSatSampleStrategy : public boost::noncopyable {
public:
    virtual ~MCSatSampleStrategy() {}

    virtual MCSatSampleStrategy* clone() const = 0;
    virtual void sampleSentences(const Model& m, const Domain& d, boost::mt19937& rng, std::vector<ELSentence>& sampled) = 0;
};


#endif /* MCSATSAMPLERSTRATEGY_H_ */
