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

bool isDisjunctionOfCNFLiterals(const Sentence& s) {
    if (s.getTypeCode() != Disjunction::TypeCode) return false;
    const Disjunction& dis = static_cast<const Disjunction&>(s);

    if ((isPELCNFLiteral(*dis.left()) || isDisjunctionOfCNFLiterals(*dis.left()))
            && (isPELCNFLiteral(*dis.right()) || isDisjunctionOfCNFLiterals(*dis.right()))) {
        return true;
    }
    return false;
}

bool isPELCNFLiteral(const Sentence& sentence) {    // TODO: this is stupid and slow!  rewrite this!
    boost::shared_ptr<Sentence> copy(sentence.clone());
    return isPELCNFLiteral(copy);
}

bool isPELCNFLiteral(const boost::shared_ptr<const Sentence>& sentence) {
    if (boost::dynamic_pointer_cast<const Atom>(sentence)
            || boost::dynamic_pointer_cast<const BoolLit>(sentence)) {
        return true;
    }
    if (boost::dynamic_pointer_cast<const Negation>(sentence)) {
        boost::shared_ptr<const Negation> neg = boost::dynamic_pointer_cast<const Negation>(sentence);
        // TODO: necessary?
        if (boost::dynamic_pointer_cast<const Negation>(neg->sentence())) {
            return false;
        }
        return isPELCNFLiteral(neg->sentence());
    }
    if (boost::dynamic_pointer_cast<const DiamondOp>(sentence)) {
        boost::shared_ptr<const DiamondOp> dia = boost::dynamic_pointer_cast<const DiamondOp>(sentence);
        if (boost::dynamic_pointer_cast<const Atom>(dia->sentence())
                || boost::dynamic_pointer_cast<const BoolLit>(dia->sentence())
                || boost::dynamic_pointer_cast<const LiquidOp>(dia->sentence())) {  // TODO add liquidop
            return true;
        }
        return false;
    }
    if (boost::dynamic_pointer_cast<const Conjunction>(sentence)) {
        boost::shared_ptr<const Conjunction> con = boost::dynamic_pointer_cast<const Conjunction>(sentence);
        if ((boost::dynamic_pointer_cast<const Atom>(con->left())
             || boost::dynamic_pointer_cast<const BoolLit>(con->left()))
            && (boost::dynamic_pointer_cast<const Atom>(con->right())
             || boost::dynamic_pointer_cast<const BoolLit>(con->right()))) {
            return true;
        }
        return false;
    }
    if (boost::dynamic_pointer_cast<const LiquidOp>(sentence)) {
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
