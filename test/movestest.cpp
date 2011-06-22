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
#include <cstdio>
#include <vector>
#include "../src/fol/fol.h"
#include "../src/fol/moves.h"
#include "testutilities.h"

BOOST_AUTO_TEST_CASE(liquidLitMovesTest) {
	std::string facts("P(a,b) @ [1:5]\n"
			"S(a) @ [1:2]\n"
			"S(a) @ [4:4]\n");
	std::string formulas("1: [Q(a,b) ]\n"
			"1: [!Q(a,b)]\n"
			"1: [!P(a,b)]\n"
			"1: [S(a)]");

	Domain d = loadDomainWithStreams(facts, formulas);
	d.setDontModifyObsPreds(false);
	WSentence form1 = d.formulas().at(0);
	WSentence form2 = d.formulas().at(1);
	WSentence form3 = d.formulas().at(2);
	WSentence form4 = d.formulas().at(3);


	std::vector<Move> moves;
	Move move;

	// initialize seed
	srand(0);
	moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
	BOOST_CHECK_EQUAL(moves.size(), 1);
	move = moves[0];
	BOOST_CHECK_EQUAL(move.toString(), "toAdd: {Q(a, b) @ [1:5]}, toDel: {}");

	moves = findMovesFor(d, d.defaultModel(), *form2.sentence());
	BOOST_CHECK_EQUAL(moves.size(), 0);

	moves = findMovesFor(d, d.defaultModel(), *form3.sentence());
	BOOST_CHECK_EQUAL(moves.size(), 1);
	move = moves[0];
	BOOST_CHECK_EQUAL(move.toString(), "toAdd: {}, toDel: {P(a, b) @ [1:5]}");

	moves = findMovesFor(d, d.defaultModel(), *form4.sentence());
	BOOST_CHECK_EQUAL(moves.size(), 1);
	move = moves[0];
	BOOST_CHECK_EQUAL(move.toString(), "toAdd: {S(a) @ [5:5]}, toDel: {}");
}

BOOST_AUTO_TEST_CASE(liquidConjMovesTest) {
	std::string facts("P(a,b) @ [1:5]\n"
			"S(a) @ [1:2]\n");
	std::string formulas("1: [P(a,b) & !S(a)]");

	Domain d = loadDomainWithStreams(facts, formulas);
	d.setMaxInterval(Interval(1,10));
	WSentence form1 = d.formulas().at(0);
	srand(0);

	Move move;
	move = findMovesFor(d, d.defaultModel(), *form1.sentence())[0];
	BOOST_CHECK_EQUAL(move.toString(), "toAdd: {P(a, b) @ [6:10]}, toDel: {S(a) @ [6:10]}" );
}

BOOST_AUTO_TEST_CASE(liquidDisjMovesTest) {
	std::string facts("P(a,b) @ [1:5]\n"
			"S(a) @ [1:2]\n");
	std::string formulas("1: [!P(a,b) v S(a)]");

	Domain d = loadDomainWithStreams(facts, formulas);
	d.setMaxInterval(Interval(1,10));
	d.setDontModifyObsPreds(false);
	WSentence form1 = d.formulas().at(0);
	srand(0);

	std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
	BOOST_CHECK_EQUAL(moves.size(), 2);
	BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {}, toDel: {P(a, b) @ [3:5]}" );
	BOOST_CHECK_EQUAL(moves[1].toString(), "toAdd: {S(a) @ [3:5]}, toDel: {}" );

}

BOOST_AUTO_TEST_CASE(maxWalkSatTest) {
	std::string facts(	"D_P(a) @ [1:5]\n"
						"D_P(b) @ [4:7]\n");
	std::string formulas(
							"1: [D_P(a) -> P(a)]\n"
							"1: [D_P(b) -> P(b)]\n"
							"100: [(P(a) & !P(b)) v (P(b) & !P(a)) v (!P(a) & !P(b))]\n");

	Domain d = loadDomainWithStreams(facts, formulas);
	srand(7);
	Model m = maxWalkSat(d, 100, 0.5);
	std::cout << modelToString(m) << std::endl;
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm1mi) {
	std::string facts(	"P(a) @ [1:4]\n"
						"P(b) @ [6:7]\n");
	std::string formulas("100: P(a) -> <>{mi} [P(a) v P(b)]\n");

	Domain d = loadDomainWithStreams(facts, formulas);
	d.setDontModifyObsPreds(false);
	srand(2);

	const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
	std::vector<Move> moves = findMovesForForm1(d, d.defaultModel(), *s);
	BOOST_FOREACH(Move move, moves) {
		std::cout << "move: " << move.toString() << std::endl;
	}
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm1m) {
	std::string facts(	"P(a) @ [1:4]\n"
						"P(b) @ [6:7]\n");
	std::string formulas("100: P(b) -> <>{m} [P(a) v P(b)]\n");

	Domain d = loadDomainWithStreams(facts, formulas);
	d.setDontModifyObsPreds(false);
	srand(2);

	const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
	std::vector<Move> moves = findMovesForForm1(d, d.defaultModel(), *s);
	BOOST_FOREACH(Move move, moves) {
		std::cout << "move: " << move.toString() << std::endl;
	}
}
