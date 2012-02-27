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

SISet Sentence::dNotSatisfied(const Model& m, const Domain& d) const {
    SISet set = satisfied(m, d, false);
    set = set.compliment(d.maxSISet());
    set.makeDisjoint();
    return set;
}

SISet Sentence::dNotSatisfied(const Model& m, const Domain& d, const SISet& where) const {
    SISet set = satisfied(m, d, false);
    set = intersection(set.compliment(d.maxSISet()), where);
    set.makeDisjoint();
    return set;
}
