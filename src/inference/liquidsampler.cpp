/*
 * liquidsampler.cpp
 *
 *  Created on: Apr 18, 2012
 *      Author: selman.joe@gmail.com
 */


#include <cstdlib>

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
    Interval curInterval(si.start().start(), si.finish().finish());
    std::list<Interval> toSample;
    do {
        bool sampleCurInterval = ((double)rand() / (double)RAND_MAX) <= p;
        if (sampleCurInterval) {
            sampled.push_back(SpanInterval(curInterval));
            // now make sure none of our intervals overlap the sampled interval
            for (std::list<Interval>::iterator it = toSample.begin();
                    it != toSample.end();
                    it++) {
                if (starts(curInterval, *it)) {
                    it->setStart(curInterval.finish()+1);
                } else if (finishes(curInterval, *it)) {
                    it->setFinish(curInterval.start()-1);
                } else if (overlaps(curInterval, *it)) {

                }
               // if ()
                //boost::optional<SpanInterval> intersect = intersection(*it, curInterval);
                //if (intersect) {

                //}
            }
        }
    } while (!toSample.empty());

    return sampled;
}
