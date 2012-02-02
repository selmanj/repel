/*
 * si_histogram.h
 *
 *  Created on: Sep 26, 2011
 *      Author: joe
 */

#ifndef SI_HISTOGRAM_H_
#define SI_HISTOGRAM_H_

#include <set>
#include <map>
#include <string>

#include "interval.h"
#include "spaninterval.h"
#include "siset.h"



class SIHistogram {
public:
    SIHistogram(bool forceLiquid_, const Interval& maxInterval);
    void add(const SpanInterval& si);
    void add(const SISet& si);
    void clear();
    std::string toString() const;
    const std::map<SpanInterval, int>& counts() const {return counts_;}
private:
    bool forceLiquid_;
    std::set<SpanInterval> sis_;
    std::map<SpanInterval, int> counts_;
    Interval maxInterval_;

};

#endif /* SI_HISTOGRAM_H_ */
