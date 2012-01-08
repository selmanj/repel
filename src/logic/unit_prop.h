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

typedef std::list<boost::shared_ptr<Sentence> > CNFClause;
typedef std::pair<CNFClause, SISet> QCNFClause;
typedef std::list<QCNFClause> QCNFClauseList;

typedef boost::shared_ptr<Sentence> CNFLiteral;
typedef std::pair<boost::shared_ptr<Sentence>, SISet> QCNFLiteral;
typedef std::list<QCNFLiteral> QCNFLiteralList;


//QCNFClauseList performUnitPropagation(const QCNFClauseList sentences);
QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c);
//QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c, const CNFClause::const_iterator& begin, const CNFClause::const_iterator& end);

CNFClause convertToCNFClause(boost::shared_ptr<Sentence> s);
QCNFClause convertToQCNFClause(const ELSentence& el);

namespace {
	bool isSimpleLiteral(const boost::shared_ptr<Sentence>& lit);

	// anonymous struct for providing a sorting order for QCNFClauseList iterators
	struct iterator_cmp {
		bool operator()(const QCNFClauseList::iterator& a, const QCNFClauseList::iterator& b) const {
			CNFClause aClause = a->first;
			CNFClause bClause = b->first;

			for (CNFClause::const_iterator aIt = aClause.begin(); aIt != aClause.end(); aIt++) {
				for (CNFClause::const_iterator bIt = bClause.begin(); bIt != bClause.end(); bIt++) {
					if ((*aIt)->toString() < (*bIt)->toString()) return true;
					if ((*aIt)->toString() > (*bIt)->toString()) return false;
				}
				return false;
			}
			return true;
		}
	};

}
#endif /* UNIT_PROP_H_ */
