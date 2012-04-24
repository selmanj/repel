/*
 * MCSatSampleLiquidlyStrategy.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#include <vector>
#include "MCSatSampleLiquidlyStrategy.h"
#include "../logic/Domain.h"
#include "../logic/ELSyntax.h"
#include "LiquidSampler.h"
#include "../SpanInterval.h"

MCSatSampleLiquidlyStrategy::MCSatSampleLiquidlyStrategy() {
    // TODO Auto-generated constructor stub

}

MCSatSampleLiquidlyStrategy::~MCSatSampleLiquidlyStrategy() {
    // TODO Auto-generated destructor stub
}

void MCSatSampleLiquidlyStrategy::sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled) {
    // sample on an interval basis for each formula, enforcing liquid constraints
    for (Domain::formula_const_iterator it = d.formulas_begin(); it != d.formulas_end(); it++) {
        ELSentence curSentence = *it;

        if (curSentence.hasInfWeight()) {
            sampled.push_back(curSentence); // have to take it
            continue;
        }
        SISet satisfied = curSentence.dSatisfied(m, d);
        double prob = 1.0 - exp(-(double)(curSentence.weight()));   // probability to sample an interval
        SISet where(true, d.maxInterval());

        // assume its liquid for now!!
        LiquidSampler sampler;
        for (SISet::const_iterator siIt = satisfied.begin(); siIt != satisfied.end(); siIt++) {
            std::vector<SpanInterval> samples = sampler(*siIt, prob);
            for (std::vector<SpanInterval>::const_iterator sampleIt = samples.begin();
                    sampleIt != samples.end();
                    sampleIt++) {
                where.add(*sampleIt);
            }
        }
        if (!where.empty()) {
            curSentence.setQuantification(where);
            sampled.push_back(curSentence);
        }
    }

}

MCSatSampleStrategy* MCSatSampleLiquidlyStrategy::clone() const {
    return new MCSatSampleLiquidlyStrategy();
}


