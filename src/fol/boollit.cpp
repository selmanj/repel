/*
 * boollit.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: joe
 */

#include "boollit.h"

BoolLit::~BoolLit() {
	// TODO Auto-generated destructor stub
}

void BoolLit::visit(SentenceVisitor& s) const {
	s.accept(*this);
}

void BoolLit::doToString(std::string& str) const {
	if (val_) {
		str += "true";
	} else {
		str += "false";
	}
}

bool BoolLit::doEquals(const Sentence& t) const {
	const BoolLit *bt = dynamic_cast<const BoolLit*>(&t);
	if (bt == NULL) {
		return false;
	}

	return bt->val_ == val_;
}

int BoolLit::doPrecedence() const {
	return 0;
}

