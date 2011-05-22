#ifndef PREDCOLLECTOR_H
#define PREDCOLLECTOR_H

#include "sentencevisitor.h"

class Sentence;
class Atom;

class PredCollector : public SentenceVisitor {
public:
	PredCollector() {};
	virtual ~PredCollector() {};
	virtual void accept(const Sentence& s) {
		// only care about atoms
		const Atom* a = dynamic_cast<const Atom*>(&s);
		if (a != NULL) {
			preds.insert(a->name());
		}
	}

	std::set<std::string> preds;
};

#endif
