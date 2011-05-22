/*
 * fol.h
 *
 *  Created on: May 21, 2011
 *      Author: joe
 */

#ifndef FOL_H_
#define FOL_H_

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

/* include all the elements needed for FOL sentences */
#include "atom.h"
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
	typedef boost::tuple<boost::shared_ptr<Atom>, SpanInterval> EventTuple;
}

#endif /* FOL_H_ */
