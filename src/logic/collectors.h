#ifndef COLLECTORS_H
#define COLLECTORS_H

#include <boost/unordered_set.hpp>
#include "syntax/sentencevisitor.h"
#include "syntax/atom.h"
#include "syntax/predicatetype.h"

struct AtomCollector : public SentenceVisitor {
    virtual void accept(const Sentence& s) {
        // only care about predicates
        if (s.getTypeCode() == Atom::TypeCode) {
            atoms.insert(static_cast<const Atom&>(s));
        }
    }

    boost::unordered_set<Atom> atoms;
};


struct PredicateTypeCollector : public SentenceVisitor {
    virtual void accept(const Sentence& s) {
        if (s.getTypeCode() == Atom::TypeCode) {
            const Atom& a = static_cast<const Atom&>(s);
            types.insert(a.predicateType());
        }
    }

    boost::unordered_set<PredicateType> types;
};

//struct
#endif
