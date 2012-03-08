#ifndef PREDCOLLECTOR_H
#define PREDCOLLECTOR_H

#include <boost/unordered_set.hpp>
#include "syntax/sentencevisitor.h"
#include "syntax/atom.h"

class PredCollector : public SentenceVisitor {
public:
    PredCollector() {};
    virtual ~PredCollector() {};
    virtual void accept(const Sentence& s) {
        // only care about predicates
        // TODO: this should collect types, not atoms
        if (s.getTypeCode() == Atom::TypeCode) {
            preds.insert(static_cast<const Atom>(s));
        }
    }

    boost::unordered_set<Atom> preds;
};

#endif
