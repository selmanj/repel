/*
 * fol.h
 *
 *  Created on: May 21, 2011
 *      Author: joe
 */

#ifndef FOL_H_
#define FOL_H_

#include <boost/shared_ptr.hpp>
#include <utility>
#include <vector>
#include <deque>

/* include all the elements needed for FOL sentences */
#include "atom.h"
#include "boollit.h"
#include "conjunction.h"
#include "constant.h"
#include "diamondop.h"
#include "disjunction.h"
#include "liquidop.h"
#include "negation.h"
#include "sentence.h"
#include "term.h"
#include "variable.h"
#include "wsentence.h"
#include "sentencevisitor.h"
#include "../spaninterval.h"

namespace FOL {
	typedef std::pair<boost::shared_ptr<Atom>, SpanInterval> EventPair;
}

inline std::vector<const Sentence*> getDisjunctionArgs(const Disjunction& d) {
	std::vector<const Sentence*> vec;

	const Disjunction *dLeft = dynamic_cast<const Disjunction *>(&*d.left());
	const Disjunction *dRight = dynamic_cast<const Disjunction *>(&*d.right());
	if (dLeft != 0) {
		std::vector<const Sentence*> vecL = getDisjunctionArgs(*dLeft);
		vec.insert(vec.end(), vecL.begin(), vecL.end());
	} else {
		vec.push_back(&*d.left());
	}

	if (dRight != 0) {
		std::vector<const Sentence*> vecR = getDisjunctionArgs(*dRight);
		vec.insert(vec.end(), vecR.begin(), vecR.end());
	} else {
		vec.push_back(&*d.right());
	}

	return vec;
};

inline boost::shared_ptr<Disjunction> wrapInDisjunction(const std::vector<const Sentence*>& sentences) {
	assert(sentences.size() >= 2);
	std::deque<boost::shared_ptr<Sentence> > copies;
	// first, make a copy of all terms
	for(std::vector<const Sentence*>::const_iterator it = sentences.begin(); it != sentences.end(); it++) {
		boost::shared_ptr<Sentence> copy((*it)->clone());
		copies.push_back(copy);
	}

	// wrap them in a disjunction
	boost::shared_ptr<Disjunction> dis(new Disjunction(copies[0], copies[1]));
	copies.pop_front();
	copies.pop_front();
	while (copies.size() > 0) {
		boost::shared_ptr<Disjunction> newDis(new Disjunction(dis, copies[0]));
		copies.pop_front();
		dis = newDis;
	}

	return dis;
}

#endif /* FOL_H_ */
