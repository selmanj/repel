/*
 * RNG.h
 *
 *  Created on: May 7, 2012
 *      Author: joe
 */

#ifndef RNG_H_
#define RNG_H_

#include <boost/function.hpp>

/**
 * Type erasure class for boost's random generators.
 */

template <typename ResultType>
class RNG {
public:
    template <class T>
    explicit RNG(T& rng)
        : min_(rng.min()), max_(rng.max()), func_(boost::ref(rng)) {};

    typedef ResultType result_type;

    ResultType min() const {return min_;}
    ResultType max() const {return max_;}

    ResultType operator()() { return func_();}
private:
    ResultType min_, max_;
    boost::function<ResultType ()> func_;
};


#endif /* RNG_H_ */
