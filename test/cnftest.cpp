/*
 * cnftest.cpp
 *
 *  Created on: Jun 27, 2011
 *      Author: joe
 */

#define BOOST_TEST_MODULE CNFTest
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#ifdef HAVE_BOOST_UNIT_TEST_FRAMEWORK
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include "src/fol/fol.h"
#include "src/fol/moves.h"
#include "testutilities.h"

BOOST_AUTO_TEST_CASE(simpleNegations) {
	boost::shared_ptr<Sentence> s;
	boost::shared_ptr<Sentence> negsIn;

	s = getAsSentence("p(A)");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn, s);

	s = getAsSentence("!p(A)");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn, s);

	s = getAsSentence("!(!p(A))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK(negsIn);
	BOOST_CHECK_EQUAL(negsIn->toString(), "p(A)");

	s = getAsSentence("!(p(A) ^ p(B))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "!p(A) v !p(B)");

	s = getAsSentence("!(p(A) ^ !p(B))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "!p(A) v p(B)");

	s = getAsSentence("!(p(A) v p(B))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "!p(A) ^ !p(B)");

	s = getAsSentence("p(A) v p(B) v !(p(A) & p(B))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "p(A) v p(B) v !p(A) v !p(B)");

	s = getAsSentence("p(A) ^ p(B) ^ !(p(A) v p(B))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "p(A) ^ p(B) ^ !p(A) ^ !p(B)");

	s = getAsSentence("<> (!p(A) & !(p(a) v p(b)))");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "<> (!p(A) ^ !p(a) ^ !p(b))");

	s = getAsSentence("[ p(a) v p(b) v !(p(a) v p(b)) ]");
	negsIn = moveNegationsInward(s);
	BOOST_CHECK_EQUAL(negsIn->toString(), "[ p(a) v p(b) v !p(a) ^ !p(b) ]");


}
