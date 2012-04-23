/*
 * liquidsampler.cpp
 *
 *  Created on: Apr 18, 2012
 *      Author: selman.joe@gmail.com
 */


#include <cstdlib>
#include <vector>

#include "liquidsampler.h"
#include "../spaninterval.h"
#include "../siset.h"

std::vector<SpanInterval> LiquidSampler::operator ()(const SpanInterval& si, double p) const {
    std::vector<SpanInterval> sampled;

    if (p <= 0.0) return sampled;
    if (p >= 1.0) {
        sampled.push_back(si);
        return sampled;
    }

    if (si.isEmpty()) return sampled;


    // loop over intervals ranging from the width of si, down to 1
    std::list<Interval> toSample;
    toSample.push_front(Interval(si.start().start(), si.finish().finish()));
    do {
        Interval curInterval = toSample.front();
        toSample.pop_front();
        bool sampleCurInterval = ((double)rand() / (double)RAND_MAX) <= p;
        if (sampleCurInterval) {
            sampled.push_back(SpanInterval(curInterval));
            // now make sure none of our intervals overlap the sampled interval
            for (std::list<Interval>::iterator it = toSample.begin();
                    it != toSample.end();) {
                if (intersection(curInterval, *it)) {
                    std::vector<Interval> subtracted = it->subtract(curInterval);
                    // put what we just processed in there and erase it
                    for (std::vector<Interval>::const_iterator it2 = subtracted.begin();
                            it2 != subtracted.end();
                            it2++) {
                        toSample.insert(it, *it2);
                    }
                    it = toSample.erase(it);
                } else {
                    it++;
                }
            }
        } else {
            // didn't sample, so lets put the smaller size of intervals next in to sample
            if (curInterval.size() != 1) {
                Interval left = Interval(curInterval.start(), curInterval.finish()-1);
                Interval right = Interval(curInterval.start()+1, curInterval.finish());
                toSample.push_front(right);
                toSample.push_front(left);
            }
        }
    } while (!toSample.empty());

    return sampled;
}
