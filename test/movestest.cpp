/*
 * movestest.cpp
 *
 *  Created on: Jun 12, 2011
 *      Author: joe
 */

#define BOOST_TEST_MODULE SpanInterval
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#ifdef HAVE_BOOST_UNIT_TEST_FRAMEWORK
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include <boost/optional.hpp>
#include "../src/fol/fol.h"
#include "../src/fol/moves.h"
#include "testutilities.h"

BOOST_AUTO_TEST_CASE(liquidMovesTest) {
	std::string facts("P(a,b) @ [1:5]");
	std::string formulas("1: [Q(a,b) ]\n 1: [!Q(a,b)]\n 1: [!P(a,b)]");

	Domain d = loadDomainWithStreams(facts, formulas);
	WSentence form1 = d.formulas().at(0);
	WSentence form2 = d.formulas().at(1);
	WSentence form3 = d.formulas().at(2);

	Moves moves;
	moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
	BOOST_CHECK_EQUAL(moves.toString(), "toAdd: {Q(a, b) @ [1:5]}, toDel: {}");

	moves = findMovesFor(d, d.defaultModel(), *form2.sentence());
	BOOST_CHECK_EQUAL(moves.toString(), "toAdd: {}, toDel: {}");

	moves = findMovesFor(d, d.defaultModel(), *form3.sentence());
	BOOST_CHECK_EQUAL(moves.toString(), "toAdd: {}, toDel: {P(a, b) @ [1:5]}");
}
