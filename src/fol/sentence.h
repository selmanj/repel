#ifndef SENTENCE_H
#define SENTENCE_H

#include <string>
#include <boost/utility.hpp>
#include "sentencevisitor.h"

class Sentence : boost::noncopyable {
public:
	virtual ~Sentence() {};
	Sentence* clone() const { return doClone(); };
	bool operator==(const Sentence& b) const {return doEquals(b);};
	std::string toString() const {
		std::string str;
		doToString(str);
		return str;
	};
	int precedence() const { return doPrecedence(); };
	virtual void visit(SentenceVisitor& s) const = 0;
private:
	virtual void doToString(std::string& str) const = 0;
	virtual Sentence* doClone() const = 0;
	virtual bool doEquals(const Sentence& t) const = 0;
	virtual int doPrecedence() const = 0;
};

inline Sentence* new_clone(const Sentence& t) {
	return t.clone();
};


#endif
