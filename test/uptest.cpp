/*
 * uptest.cpp
 *
 *  Created on: Dec 12, 2011
 *      Author: joe
 */

#define BOOST_TEST_MODULE UPTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <algorithm>
#include "logic/unit_prop.h"
#include "testutilities.h"
#include "siset.h"
#include "spaninterval.h"
#include "interval.h"

BOOST_AUTO_TEST_CASE( simple_lit ) {

	boost::shared_ptr<Sentence> pa = getAsSentence("P(a)");
	boost::shared_ptr<Sentence> pb = getAsSentence("P(b)");


/*
	boost::shared_ptr<Sentence> singleLit = pa;

	CNFClause sentence;
	sentence.push_back(pa);
	sentence.push_back(pb);

	Interval maxInterval(1,10);
	SISet halfway(false, maxInterval);
	halfway.add(SpanInterval(1,5,1,5,maxInterval));

	SISet all(false, maxInterval);
	all.add(SpanInterval(1,10,1,10,maxInterval));

	QCNFLiteral qsingleLit;
	qsingleLit.first = singleLit;
	qsingleLit.second = halfway;

	QCNFClause qClause;
	qClause.first = sentence;
	qClause.second = all;

	// phew!
	std::list<QCNFClause> newClauses = propagate_literal(qsingleLit, qClause);
	BOOST_CHECK_EQUAL(newClauses.size(), 1);
	QCNFClause newClause = newClauses.front();
	BOOST_CHECK_EQUAL_COLLECTIONS(newClause.first.begin(), newClause.first.end(), sentence.begin(), sentence.end());
	//BOOST_CHECK_EQUAL(newClause.first, sentence);
	BOOST_CHECK_EQUAL(newClause.second.toString(), "{[(1, 5), (6, 10)], [6:10]}");
	*/
}

BOOST_AUTO_TEST_CASE( cnfConvertBasic ) {

	boost::shared_ptr<Sentence> a = getAsSentence("P(a) v Q(a) v !R(a) v S(a)");
	CNFClause c = convertToCNFClause(a);

	BOOST_REQUIRE_EQUAL(c.size(), 4);
	boost::shared_ptr<Sentence> pa = getAsSentence("P(a)");
	boost::shared_ptr<Sentence> qa = getAsSentence("Q(a)");
	boost::shared_ptr<Sentence> nra = getAsSentence("!R(a)");
	boost::shared_ptr<Sentence> sa = getAsSentence("S(a)");
	BOOST_CHECK(std::find(c.begin(), c.end(), pa) != c.end());
	BOOST_CHECK(std::find(c.begin(), c.end(), qa) != c.end());
	BOOST_CHECK(std::find(c.begin(), c.end(), nra) != c.end());
	BOOST_CHECK(std::find(c.begin(), c.end(), sa) != c.end());




}
