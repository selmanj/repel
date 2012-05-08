/*
 * RNG.h
 *
 *  Created on: May 7, 2012
 *      Author: joe
 */

#ifndef RNG_H_
#define RNG_H_

#include <boost/function.hpp>
#include <boost/random/mersenne_twister.hpp>
/**
 * Type erasure class for boost's random generators.
 */

template <typename ResultType>
class RNG {
public:
    template <class T>
    explicit RNG(T& gen)
        : min_(gen.min()), max_(gen.max()), func_(gen) {};

    typedef ResultType result_type;

    ResultType min() const {return min_;}
    ResultType max() const {return max_;}

    ResultType operator()() { return func_();}

    template <typename R, class T>
    friend RNG<R> rngFromRef(T& gen);

private:
    RNG() {};

    ResultType min_, max_;
    boost::function<ResultType ()> func_;

};

template <typename ResultType, class T>
RNG<ResultType> rngFromRef(T& gen) {
    RNG<ResultType> rng;
    rng.min_ = gen.min();
    rng.max_ = gen.max();
    rng.func_ = boost::ref(gen);
    return rng;
}


#endif /* RNG_H_ */
