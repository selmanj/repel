#include "sentence.h"
#include "disjunction.h"
#include "../moves.h"
#include <sstream>
#include <boost/shared_ptr.hpp>

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

bool isDisjunctionOfCNFLiterals(const boost::shared_ptr<const Sentence>& sentence) {
    boost::shared_ptr<const Disjunction> dis = boost::dynamic_pointer_cast<const Disjunction>(sentence);
    if (!dis) return false;
    if ((isPELCNFLiteral(dis->left()) || isDisjunctionOfCNFLiterals(dis->left()))
            && (isPELCNFLiteral(dis->right()) || isDisjunctionOfCNFLiterals(dis->right()))) {
        return true;
    }
    return false;
}

/*
SISet Sentence::satisfied(const Model& m, const Domain& d, bool forceLiquid, const SISet* where) const {
    SISet set = doSatisfied(m, d, forceLiquid);
    if (where) return intersection(set, *where);
    return set;
}
*/
