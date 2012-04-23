/*
 * MCSatSamplePerfectlyStrategy.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#include <cstdlib>
#include "MCSatSamplePerfectlyStrategy.h"
#include "../logic/Domain.h"

void MCSatSamplePerfectlyStrategy::sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled) {
    // sample on an interval basis for each formula
    for (Domain::formula_const_iterator it = d.formulas_begin(); it != d.formulas_end(); it++) {
        ELSentence curSentence = *it;
        std::cout << "working on sentence: " << curSentence << std::endl;

        if (curSentence.hasInfWeight()) {
            sampled.push_back(curSentence); // have to take it
            continue;
        }
        SISet satisfied = curSentence.dSatisfied(m, d);
        double prob = 1.0 - exp(-(double)(curSentence.weight()));   // probability to sample an interval
        SISet where(false, d.maxInterval());
        // iterate over each interval, sampling!
        for (SISet::const_iterator sisetIt = satisfied.begin(); sisetIt != satisfied.end(); sisetIt++) {
            SpanInterval si = *sisetIt;
            for (SpanInterval::const_iterator siIt = si.begin(); siIt != si.end(); siIt++) {
                Interval interval = *siIt;
                // sample it with probability 1-exp(-w)
                if (((double)rand()) / ((double)RAND_MAX) <= prob) {
                    where.add(SpanInterval(interval.start(), interval.start(), interval.finish(), interval.finish()));
                }
            }
        }
        if (!where.empty()) {
            curSentence.setQuantification(where);
            sampled.push_back(curSentence);
        }
    }
}
