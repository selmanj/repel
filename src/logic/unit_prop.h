/*
 * unit_prop.h
 *
 *  Created on: Dec 11, 2011
 *      Author: joe
 */

#ifndef UNIT_PROP_H_
#define UNIT_PROP_H_

#include <set>
#include <boost/shared_ptr.hpp>
#include <utility>
#include "el_syntax.h"
#include "../siset.h"

typedef std::set<boost::shared_ptr<Sentence> > CNFClause;
typedef std::pair<CNFClause, SISet> QCNFClause;
typedef std::pair<boost::shared_ptr<Sentence>, SISet> QCNFLiteral;

std::list<QCNFClause> propagate_literal(const QCNFLiteral& lit, const QCNFClause& c);
std::list<QCNFClause> propagate_literal(const QCNFLiteral& lit, const QCNFClause& c, const CNFClause::const_iterator& begin, const CNFClause::const_iterator& end);

#endif /* UNIT_PROP_H_ */
