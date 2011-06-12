/*
 * boollit.h
 *
 *  Created on: Jun 8, 2011
 *      Author: joe
 */

#ifndef BOOLLIT_H_
#define BOOLLIT_H_

#include "sentence.h"

class BoolLit: public Sentence {
public:
	BoolLit(bool value) : val_(value) {};
	BoolLit(const BoolLit& other) : val_(other.val_) {};
	virtual ~BoolLit();

	bool value() const {return val_;};

	virtual void visit(SentenceVisitor& s) const;
private:
	bool val_;

	virtual void doToString(std::string& str) const;
	virtual Sentence* doClone() const { return new BoolLit(*this);};
	virtual bool doEquals(const Sentence& t) const;
	virtual int doPrecedence() const;
};

#endif /* BOOLLIT_H_ */
