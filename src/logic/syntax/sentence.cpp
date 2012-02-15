#include "sentence.h"
#include <sstream>

std::string TQConstraints::toString() const {
    std::stringstream str;

    if (mustBeIn.size() != 0) {
        str << "&" << mustBeIn.toString();
    }

    if (mustNotBeIn.size() != 0) {
        // only print a comma if we printed something before
        if (mustBeIn.size() != 0) {
            str << ",";
        }
        str << "\\" << mustNotBeIn.toString();
    }

    return str.str();
}
/*
SISet Sentence::satisfied(const Model& m, const Domain& d, bool forceLiquid, const SISet* where) const {
    SISet set = doSatisfied(m, d, forceLiquid);
    if (where) return intersection(set, *where);
    return set;
}
*/
