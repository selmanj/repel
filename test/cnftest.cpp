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
#include <vector>
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

BOOST_AUTO_TEST_CASE(simplePELCNF) {
	boost::shared_ptr<Sentence> s;
	boost::shared_ptr<Sentence> rewrite;
	std::vector<boost::shared_ptr<Sentence> > support;
	Domain d;

	s = getAsSentence("p(a)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!p(a)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!p(a) v p(b)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("<> p(a)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!(<> p(a))");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("p(a) ^ p(b)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!(p(a) ^ p(b))");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s, rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!(p(a) v p(b)) v p(c)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(s->toString(), "!__anonPred0() v p(c)");
	BOOST_CHECK_EQUAL(support.size(), 3);
	BOOST_CHECK_EQUAL(support[0]->toString(), "!__anonPred0() v p(a) v p(b)");
	BOOST_CHECK_EQUAL(support[1]->toString(), "__anonPred0() v !p(a)");
	BOOST_CHECK_EQUAL(support[2]->toString(), "__anonPred0() v !p(b)");


}
