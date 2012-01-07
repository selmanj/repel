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
#include "logic/el_syntax.h"

BOOST_AUTO_TEST_CASE( qconstraints ) {
	boost::shared_ptr<Sentence> s = getAsSentence("<>{m} P(a)");
	boost::shared_ptr<DiamondOp> dia = boost::dynamic_pointer_cast<DiamondOp>(s);

	TQConstraints constraints;
	constraints.mustBeIn.add(SpanInterval(1,2,1,2));
	constraints.mustNotBeIn.add(SpanInterval(1,1,1,1));

	boost::shared_ptr<DiamondOp> diaCopy(new DiamondOp(dia->sentence(),dia->relations().begin(), dia->relations().end(), &constraints));
	BOOST_CHECK_EQUAL(diaCopy->toString(), "<>{m:&{[1:2]},\{[1:1]}} P(a)");
}

BOOST_AUTO_TEST_CASE( simple_lit ) {

	std::string facts = "video(a) @ [1:20]";
	std::string formulas = "P(a) @ [1:10]\n"
			"Q(a) @ [1:10]\n"
			"P(a) v Q(a) @ [1:20]";

	Domain d = loadDomainWithStreams(facts, formulas);

	FormulaList flist = d.formulas();
	ELSentence s1 = flist[0];
	ELSentence s2 = flist[1];
	ELSentence s3 = flist[2];

	QCNFClause qs1 = convertToQCNFClause(s1);
	QCNFClause qs2 = convertToQCNFClause(s2);
	QCNFClause qs3 = convertToQCNFClause(s3);


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
	/* TODO: why doesn't the below work?
	BOOST_CHECK(std::find(c.begin(), c.end(), pa) != c.end());
	BOOST_CHECK(std::find(c.begin(), c.end(), qa) != c.end());
	BOOST_CHECK(std::find(c.begin(), c.end(), nra) != c.end());
	BOOST_CHECK(std::find(c.begin(), c.end(), sa) != c.end());
	 */



}
