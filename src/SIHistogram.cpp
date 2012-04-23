/*
 * SIHistogram.cpp
 *
 *  Created on: Sep 26, 2011
 *      Author: joe
 */
#include <boost/foreach.hpp>
#include <sstream>
#include "SIHistogram.h"
#include "SpanInterval.h"

SIHistogram::SIHistogram(bool forceLiquid, const Interval& maxInterval)
    : forceLiquid_(forceLiquid), sis_(), counts_(), maxInterval_(maxInterval) {
    //SpanInterval everything(maxInterval.start(), maxInterval.finish(), maxInterval.start(), maxInterval.finish(), maxInterval);
    //sis_.insert(everything);
    //counts_.insert(std::pair<SpanInterval, int>(everything,0));

}

void SIHistogram::add(const SpanInterval& si) {
    SISet set(forceLiquid_, maxInterval_);
    set.add(si);
    add(set);
}


void SIHistogram::add(const SISet& siset) {
    // TODO: check for max interval
    std::set<SpanInterval> sisNew;
    std::map<SpanInterval, int> countsNew;
    SISet leftToAdd = siset;
    for (std::set<SpanInterval>::const_iterator it = sis_.begin(); it != sis_.end(); it++) {
        SISet inter = intersection(leftToAdd, *it);
        if (inter.size() > 0) {
            // BREAK IT UP YOU TWO
            SISet before(forceLiquid_, maxInterval_);
            before.add(*it);
            before.subtract(inter);
            BOOST_FOREACH(SpanInterval beforesi, before.intervals()) {
                sisNew.insert(beforesi);
                countsNew.insert(std::pair<SpanInterval, int>(beforesi, counts_[*it]));
            }
            BOOST_FOREACH(SpanInterval intersi, inter.intervals()) {
                sisNew.insert(intersi);
                countsNew.insert(std::pair<SpanInterval, int>(intersi, counts_[*it]+1));
            }
            leftToAdd.subtract(inter);
        } else {
            // just copy it
            sisNew.insert(*it);
            countsNew.insert(std::pair<SpanInterval, int>(*it, counts_[*it]));
        }
    }
    if (leftToAdd.size() > 0) {
        BOOST_FOREACH(SpanInterval si, leftToAdd.intervals()) {
            sisNew.insert(si);
            countsNew.insert(std::pair<SpanInterval, int>(si, 1));
        }
    }
    sis_ = sisNew;
    counts_ = countsNew;
}

void SIHistogram::clear() {
    sis_.clear();
    counts_.clear();
}


std::string SIHistogram::toString() const {
    std::stringstream sstream;
    sstream << "{" << std::endl;
    BOOST_FOREACH(SpanInterval si, sis_) {
        sstream << si.toString() << " = " << counts_.at(si) << std::endl;
    }
    sstream << "}" << std::endl;
    return sstream.str();
}
