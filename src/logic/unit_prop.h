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
#include <queue>
#include <logic/domain.h>
#include "el_syntax.h"
#include "../siset.h"

typedef std::list<boost::shared_ptr<Sentence> > CNFClause;
typedef std::pair<CNFClause, SISet> QCNFClause;
typedef std::list<QCNFClause> QCNFClauseList;

typedef boost::shared_ptr<Sentence> CNFLiteral;
typedef std::pair<boost::shared_ptr<Sentence>, SISet> QCNFLiteral;
typedef std::list<QCNFLiteral> QCNFLiteralList;

typedef std::pair<QCNFLiteralList, QCNFClauseList> QUnitsFormulasPair;

/**
 * Perform unit propagation on a domain.  All infinitely-weighted formulas (as
 * well as facts) are propagated or treated as candidates for propagation.
 *
 * Note that only closed-world domains are supported at this time.
 *
 * @param d The domain containing formulas to propagate
 * @return A pairing of unit clauses and formulas
 */
QUnitsFormulasPair performUnitPropagation(const Domain& d);
QUnitsFormulasPair performUnitPropagation(const QCNFClauseList& sentences);
QCNFClauseList propagateLiteral(const QCNFLiteral& lit, const QCNFClause& c);
//QCNFClauseList propagate_literal(const QCNFLiteral& lit, const QCNFClause& c, const CNFClause::const_iterator& begin, const CNFClause::const_iterator& end);

CNFClause convertToCNFClause(boost::shared_ptr<Sentence> s);

/**
 * Converts all infinitely-weighted clauses into QCNF form.
 */
QCNFClauseList convertToQCNFClauseList(const FormulaList& list);
QCNFClause convertToQCNFClause(const ELSentence& el);
ELSentence convertFromQCNFClause(const QCNFLiteral& c);
ELSentence convertFromQCNFClause(const QCNFClause& c);

namespace {
	/**
	* After this function is called, unit clauses will be removed from
	* sentences and placed in unitClauses.
	*/
	void splitUnitClauses(QCNFClauseList& sentences, QCNFLiteralList& unitClauses);

	bool isSimpleLiteral(const boost::shared_ptr<Sentence>& lit);
	bool isNegatedLiteral(const boost::shared_ptr<Sentence>& left, const boost::shared_ptr<Sentence>& right);

	bool propagateSimpleLitToSimpleLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);
	bool propagateNegSimpleLitToSimpleLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);
	bool propagateSimpleLitToDiamond(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);

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
