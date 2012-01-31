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
	BoolLit(bool value);
	BoolLit(const BoolLit& other);
	virtual ~BoolLit();

	BoolLit& operator=(const BoolLit& other);
	bool value() const;

	virtual void visit(SentenceVisitor& s) const;
private:
	bool val_;

	virtual void doToString(std::stringstream& str) const;
	virtual Sentence* doClone() const;
	virtual bool doEquals(const Sentence& t) const;
	virtual int doPrecedence() const;
    virtual bool doContains(const Sentence& s) const;
};

// IMPLEMENTATION
inline BoolLit::BoolLit(bool value) : val_(value) {}
inline BoolLit::BoolLit(const BoolLit& other) : val_(other.val_) {}
inline BoolLit::~BoolLit() {}

inline BoolLit& BoolLit::operator=(const BoolLit& other) {if (this != &other) val_ = other.val_; return *this;}

inline bool BoolLit::value() const {return val_;};
inline void BoolLit::visit(SentenceVisitor& s) const {s.accept(*this);}

// private members

inline void BoolLit::doToString(std::stringstream& str) const {
	 (val_) ? str << "true" : str << "false";
}
inline Sentence* BoolLit::doClone() const { return new BoolLit(*this);};
inline int BoolLit::doPrecedence() const { return 0;}
inline bool BoolLit::doContains(const Sentence& s) const {return *this == s;}

#endif /* BOOLLIT_H_ */
