/*
 * cnftest.cpp
 *
 *  Created on: Jun 27, 2011
 *      Author: joe
 */

#define BOOST_TEST_MODULE CNFTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <vector>
#include "logic/fol.h"
#include "logic/moves.h"
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
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!p(a)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!p(a) v p(b)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("<> p(a)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!(<> p(a))");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("p(a) ^ p(b)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!(p(a) ^ p(b))");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK(*s == *rewrite);
	BOOST_CHECK_EQUAL(support.size(), 0);

	s = getAsSentence("!(p(a) v p(b)) v p(c)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(rewrite->toString(), "!__anonPred0() v p(c)");
	BOOST_CHECK_EQUAL(support.size(), 3);
	BOOST_CHECK_EQUAL(support[0]->toString(), "!__anonPred0() v p(a) v p(b)");
	BOOST_CHECK_EQUAL(support[1]->toString(), "__anonPred0() v !p(a)");
	BOOST_CHECK_EQUAL(support[2]->toString(), "__anonPred0() v !p(b)");

	s = getAsSentence("(p(a) ; p(b)) ^ p(c)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(rewrite->toString(), "__anonPred1() ^ p(c)");
	BOOST_CHECK_EQUAL(support.size(), 2);
	BOOST_CHECK_EQUAL(support[0]->toString(), "!__anonPred1() v p(a) ; p(b)");
	BOOST_CHECK_EQUAL(support[1]->toString(), "__anonPred1() v !(p(a) ; p(b))");

	s = getAsSentence("<> !p(a)");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(rewrite->toString(), "<> __anonPred2()");
	BOOST_CHECK_EQUAL(support.size(), 2);
	BOOST_CHECK_EQUAL(support[0]->toString(), "!__anonPred2() v !p(a)");
	BOOST_CHECK_EQUAL(support[1]->toString(), "__anonPred2() v p(a)");


	s = getAsSentence("dribbling(man1) ; dribbling(man2) -> <>{mi} (dribbling(man2) v flag(ref))");
	support.clear();
	rewrite = convertToPELCNF(s, support, d);
	BOOST_CHECK_EQUAL(rewrite->toString(), "!(dribbling(man1) ; dribbling(man2)) v <>{mi} __anonPred3()");
	BOOST_CHECK_EQUAL(support.size(), 3);
	BOOST_CHECK_EQUAL(support[0]->toString(), "!__anonPred3() v dribbling(man2) v flag(ref)");
	BOOST_CHECK_EQUAL(support[1]->toString(), "__anonPred3() v !dribbling(man2)");
	BOOST_CHECK_EQUAL(support[2]->toString(), "__anonPred3() v !flag(ref)");
}
