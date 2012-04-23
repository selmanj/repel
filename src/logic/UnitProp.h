/*
 * UnitProp.h
 *
 *  Created on: Dec 11, 2011
 *      Author: joe
 */

#ifndef UNIT_PROP_H_
#define UNIT_PROP_H_

#include <boost/shared_ptr.hpp>
#include <utility>
#include <queue>
#include <iostream>
#include "logic/syntax/Sentence.h"
#include "logic/Domain.h"
#include "../SISet.h"

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
 * @param d the domain containing formulas to propagate
 * @return a copy of the domain after unit prop is applied
 */
Domain performUnitPropagation(const Domain& d);

/**
 * Perform unit propagation with the given QCNFClauseList.
 *
 * @param sentences a list of quantified clauses in CNF form
 * @return a pairing of unit clauses and formulas after propagation
 */
QUnitsFormulasPair performUnitPropagation(const QCNFClauseList& sentences);

/**
 * Propagate a single literal into a clause.
 *
 * @param lit the literal to propagate
 * @param c the clause to propagate lit into
 * @return a list of resulting sentences after lit is propagated into c
 */
QCNFClauseList propagateLiteral(const QCNFLiteral& lit, const QCNFClause& c);

/**
 * Convert a sentence that is already in CNF form into a CNFClause.
 *
 * Note that the sentence MUST already be in CNF form, or an exception will
 * result.  CNF is either a single literal or a disjunction of literals.
 *
 * @param s the sentence to convert
 * @return CNFClause representing s
 */
CNFClause convertToCNFClause(boost::shared_ptr<Sentence> s);

/**
 * Converts all infinitely-weighted clauses into QCNF form.  Non-infinitely
 * weighted clauses are ignored.
 *
 * Note that sentences MUST already be in CNF form, or an exception will
 * result.  CNF is either a single literal or a disjunction of literals.
 *
 * @param list FormulaList to convert
 * @return a QCNFClauseList representing the converted formulas
 */
QCNFClauseList convertToQCNFClauseList(const std::vector<ELSentence>& list);

/**
 * Convert an ELSentence into QCNF form.
 *
 * Note that the ELSentence MUST already be in CNF form, or an exception will
 * result.  CNF is either a single literal or a disjunction of literals.
 *
 * @param el the ELSentence to convert
 * @return a QCNFClause representing the sentence.
 */
QCNFClause convertToQCNFClause(const ELSentence& el);

/**
 * Convert a QCNFClause into an ELSentence.
 *
 * @param c the QCNFLiteral to convert
 * @return an ELSentence representing c
 */
ELSentence convertFromQCNFClause(const QCNFLiteral& c);

/**
 * Convert a QCNFClause into an ELSentence.
 *
 * @param c the QCNFClause to convert
 * @return an ELSentence representing c
 */
ELSentence convertFromQCNFClause(const QCNFClause& c);

/**
 * Convert a CNFClause into a shared_ptr Sentence.
 *
 * @param c the CNFClause to convert
 * @return shared_ptr holding the sentence structure representing c.
 */
boost::shared_ptr<Sentence> convertFromCNFClause(const CNFClause& c);

/**
 * Overload of operator<< for QCNFClause.
 */
template <class traits>
std::basic_ostream<char,traits>& operator<< (std::basic_ostream<char,traits>& os, const QCNFClause& c );

/**
 * Overload of operator<< for QCNFLiteral.
 */
template <class traits>
std::basic_ostream<char,traits>& operator<< (std::basic_ostream<char,traits>& os, const QCNFLiteral& c );

/**
 * Exception representing a contradiction.
 */
class contradiction : public std::exception {
public:
    contradiction() throw()
        : what_("Contradiction found in constraints.") {};
    contradiction(const contradiction& o) throw()
        : what_(o.what_) {}
    contradiction(const std::string& what) throw()
        : what_(what) {}
    //contradiction& operator=(const contradiction& o) throw() {ifreturn *this;}
    virtual const char* what() const throw() { return what_.c_str();}
    //friend std::ostream& operator<<(std::ostream& o, const contradiction& c) { o << "contradiction: " << c.what(); return o;}
private:
    const std::string& what_;
};

namespace {
    /**
    * After this function is called, unit clauses will be removed from
    * sentences and placed in unitClauses.
    */
    void splitUnitClauses(QCNFClauseList& sentences, QCNFLiteralList& unitClauses);
    /*
     * Enforce unit clauses in a partial model (throws exception if a
     * contradiction is detected - also meant to be called incrementally).
     */
    void enforceUnitProps(const QCNFLiteralList& unitClauses, boost::unordered_map<Proposition, SISet>& partialModel);
    /**
     * Convert a literal into a proposition.
     */
    Proposition convertToProposition(const QCNFLiteral& lit);

    /**
     * Check to see if left or right is the negated version of the opposite (e.g. P(a) is negated of !P(a)).
     */
    bool isNegatedLiteral(boost::shared_ptr<Sentence> left, boost::shared_ptr<Sentence> right);

    /**
     * Propagate a simple literal (e.g. P(a) or its negation)   clause is the
     * clause we are working on, lit is the actual lit we want to propagate
     * into (in the clause) and newSentences is for any new sentences we
     * create.
     */
    bool propagateSimpleLitToSimpleLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);
    bool propagateNegSimpleLitToSimpleLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);
    bool propagateSimpleLitToDiamond(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);
    bool propagateSimpleLitToLiquidLit(const QCNFLiteral& unit, QCNFClause& clause, CNFClause::iterator& lit, std::queue<QCNFClause>& newSentences);

    // anonymous struct for providing a sorting order for QCNFClauseList iterators
    struct iterator_cmp {
        bool operator()(const QCNFClauseList::iterator& a, const QCNFClauseList::iterator& b) const;
    };

}

// IMPLEMENTATION
template <class traits>
std::basic_ostream<char,traits>& operator<< (std::basic_ostream<char,traits>& os, const QCNFClause& c ) {
    for (CNFClause::const_iterator it = c.first.begin(); it != c.first.end(); it++) {
        if (it != c.first.begin()) os << ", ";
        os << (*it)->toString();
    }
    os << " @ " << c.second.toString();
    return os;
}

template <class traits>
inline std::basic_ostream<char,traits>& operator<< (std::basic_ostream<char,traits>& os, const QCNFLiteral& c ) {
    os << c.first->toString() << " @ " << c.second.toString();
    return os;
}

inline bool iterator_cmp::operator()(const QCNFClauseList::iterator& a, const QCNFClauseList::iterator& b) const {
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

#endif /* UNIT_PROP_H_ */
