#include "sentence.h"
#include "disjunction.h"
#include "atom.h"
#include "boollit.h"
#include "conjunction.h"
#include "diamondop.h"
#include "liquidop.h"
#include "negation.h"
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

bool isDisjunctionOfPELCNFLiterals(const Sentence& s) {
    if (s.getTypeCode() != Disjunction::TypeCode) return false;
    const Disjunction& dis = static_cast<const Disjunction&>(s);

    if ((isPELCNFLiteral(*dis.left()) || isDisjunctionOfPELCNFLiterals(*dis.left()))
            && (isPELCNFLiteral(*dis.right()) || isDisjunctionOfPELCNFLiterals(*dis.right()))) {
        return true;
    }
    return false;
}

bool isPELCNFLiteral(const Sentence& sentence) {
    if (sentence.getTypeCode() == Atom::TypeCode
            || sentence.getTypeCode() == BoolLit::TypeCode
            || sentence.getTypeCode() == LiquidOp::TypeCode) {
        return true;
    }
    if (sentence.getTypeCode() == Negation::TypeCode) {
        const Negation& neg = static_cast<const Negation&>(sentence);

        // TODO: necessary to check for double negation?
        if (neg.sentence()->getTypeCode() == Negation::TypeCode) return false;
        return isPELCNFLiteral(*neg.sentence());
    }
    if (sentence.getTypeCode() == DiamondOp::TypeCode) {
        const DiamondOp& dia = static_cast<const DiamondOp&>(sentence);
        if (       dia.sentence()->getTypeCode()    == Atom::TypeCode
                || dia.sentence()->getTypeCode()    == BoolLit::TypeCode
                || dia.sentence()->getTypeCode()    == LiquidOp::TypeCode) {  // TODO add liquidop
            return true;
        }
        return false;
    }
    if (sentence.getTypeCode() == Conjunction::TypeCode) {
        const Conjunction& con = static_cast<const Conjunction&>(sentence);
        if ((con.left()->getTypeCode() == Atom::TypeCode
                || con.left()->getTypeCode() == BoolLit::TypeCode)
              && (con.right()->getTypeCode() == Atom::TypeCode
                || con.right()->getTypeCode() == BoolLit::TypeCode)) {
            return true;
        }
        return false;
    }

    return false;
}


bool isSimpleLiteral(const Sentence& lit) {
    if (lit.getTypeCode() == Atom::TypeCode) return true;
    if (lit.getTypeCode() == Negation::TypeCode) {
        const Negation& neg = static_cast<const Negation&>(lit);
        if (neg.sentence()->getTypeCode() == Atom::TypeCode) return true;
    }
    return false;
}
