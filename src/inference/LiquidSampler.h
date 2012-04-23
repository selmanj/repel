/*
 * liquidsampler.h
 *
 *  Created on: Apr 18, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef LIQUIDSAMPLER_H_
#define LIQUIDSAMPLER_H_

#include <functional>
#include <vector>

class SpanInterval;

struct LiquidSampler : std::binary_function<SpanInterval, double, std::vector<SpanInterval> > {
    /**
     * Sample all intervals in a SpanInterval with probability p, obeying
     * liquid constraints.
     *
     * @param si SpanInterval representing the set of intervals to sample
     * @param p probabiliy of sampling an interval (0 <= p <= 1)
     * @return vector of liquid spanintervals sampled
     */
    std::vector<SpanInterval> operator()(const SpanInterval& si, double p) const;
};



#endif /* LIQUIDSAMPLER_H_ */
