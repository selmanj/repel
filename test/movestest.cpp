/*
 * movestest.cpp
 *
 *  Created on: Jun 12, 2011
 *      Author: joe
 */

#define BOOST_TEST_MODULE Moves
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>
#include <cstdio>
#include <vector>
#include "logic/el_syntax.h"
#include "logic/moves.h"
#include "inference/maxwalksat.h"
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
    FormulaList formSet = d.formulas();
    ELSentence form1 = formSet.at(0);
    ELSentence form2 = formSet.at(1);
    ELSentence form3 = formSet.at(2);
    ELSentence form4 = formSet.at(3);


    std::vector<Move> moves;
    Move move;

    // initialize seed
    srand(0);
    moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_REQUIRE_EQUAL(moves.size(), 1);
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
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
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
    ELSentence form1 = d.formulas().at(0);
    srand(0);

    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_CHECK_EQUAL(moves.size(), 2);
    BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {}, toDel: {P(a, b) @ [3:5]}" );
    BOOST_CHECK_EQUAL(moves[1].toString(), "toAdd: {S(a) @ [3:5]}, toDel: {}" );

}

BOOST_AUTO_TEST_CASE(pelCNFAtomTest) {
    std::string facts("P(a,b) @ [1:5]\n"
            "S(a) @ [1:2]\n");
    std::string formulas("1: S(a)");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
    srand(0);

    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_CHECK_EQUAL(moves.size(), 1);
    BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {S(a) @ [3:5]}, toDel: {}");
}

BOOST_AUTO_TEST_CASE(pelCNFNegAtomTest) {
    std::string facts("P(a,b) @ [1:5]\n"
            "S(a) @ [1:2]\n");
    std::string formulas("1: !S(a)");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
    srand(0);

    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_CHECK_EQUAL(moves.size(), 1);
    BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {}, toDel: {S(a) @ [1:2]}");
}

BOOST_AUTO_TEST_CASE(pelCNFDisjunctionTest) {
    std::string facts("P(a,b) @ [1:5]\n"
            "S(a) @ [1:2]\n");
    std::string formulas("1: [P(a,b) v S(a)]");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setMaxInterval(Interval(1,10));
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
    srand(0);

    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_REQUIRE_EQUAL(moves.size(), 2);
    BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {P(a, b) @ [6:10]}, toDel: {}");
    BOOST_CHECK_EQUAL(moves[1].toString(), "toAdd: {S(a) @ [6:10]}, toDel: {}");

}

BOOST_AUTO_TEST_CASE(pelCNFDiamondTestTest) {
    std::string facts("Huddle(a)  @ [2:2]\n"
            "Dig(a) @ [4:5]\n"
            "Spike(a) @ [6:6]\n");
    std::string formulas("1: Huddle(a) -> !(<>{>} [Dig(a) v Spike(a)])");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
    srand(0);

    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_CHECK_EQUAL(moves.size(), 2);
    BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {}, toDel: {Dig(a) @ [4:6], Spike(a) @ [4:6]}");
    BOOST_CHECK_EQUAL(moves[1].toString(), "toAdd: {}, toDel: {Huddle(a) @ [2:2]}");
}

BOOST_AUTO_TEST_CASE(pelCNFDiamondConjTest) {
    std::string facts(  "GoingUp(a) @ [1:1]\n"
                        "GoingDown(a) @ [3:3]\n"
                        "GoingDown(a) @ [5:5]\n"
                        "GoingUp(a) @ [8:8]\n"
    );
    std::string formulas("1: GoingDown(a) ^{<} GoingDown(a) -> <>{d} GoingUp(a)");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
    SISet sat = form1.sentence()->dSatisfied(d.defaultModel(), d);
    std::cout << "sat = " << sat.toString() << std::endl;
    std::cout << "sat compliment is = " << sat.compliment().toString() << std::endl;

    srand(0);
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *form1.sentence());
    BOOST_REQUIRE_EQUAL(moves.size(), 3);

    BOOST_CHECK_EQUAL(moves[0].toString(), "toAdd: {GoingUp(a) @ [4:4]}, toDel: {}");
    BOOST_CHECK_EQUAL(moves[1].toString(), "toAdd: {}, toDel: {GoingDown(a) @ [3:3]}");
    BOOST_CHECK_EQUAL(moves[2].toString(), "toAdd: {}, toDel: {GoingDown(a) @ [5:5]}");
}

BOOST_AUTO_TEST_CASE(pelCNFDisjLiqTest) {
    std::string facts("D-Driving(t1) @ [35:50]\n"
                      "HasTrack(p1,t1) @ [1:50]\n"
                      "Driving(p1) @ [35:50]");
    std::string formulas("1: [!Driving(p1)] v [HasTrack(p1, t1) ^ D-Driving(t1)]");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    ELSentence form1 = d.formulas().at(0);
    SISet sat = form1.sentence()->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(sat.toString(), "{[1:34], [35:50]}");
}

BOOST_AUTO_TEST_CASE(maxWalkSatTest) {
    std::string facts(  "D_P(a) @ [1:5]\n"
                        "D_P(b) @ [4:7]\n");
    std::string formulas(
                            "1: [D_P(a) -> P(a)]\n"
                            "1: [D_P(b) -> P(b)]\n"
                            "100: [(P(a) & !P(b)) v (P(b) & !P(a)) v (!P(a) & !P(b))]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    srand(7);
    Model init = d.defaultModel();
    Model m = maxWalkSat(d, 100, 0.5, &init);
    std::cout << m.toString() << std::endl;
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm1mi) {
    std::string facts(  "P(a) @ [1:4]\n"
                        "P(b) @ [6:7]\n");
    std::string formulas("100: P(a) -> <>{mi} [P(a) v P(b)]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(2);

    const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_FOREACH(Move move, moves) {
        std::cout << "move: " << move.toString() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm1m) {
    std::string facts(  "P(a) @ [1:4]\n"
                        "P(b) @ [6:7]\n");
    std::string formulas("100: P(b) -> <>{m} [P(a) v P(b)]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(2);

    const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_FOREACH(Move move, moves) {
        std::cout << "move: " << move.toString() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm1f) {
    std::string facts(  "P(a) @ [1:4]\n"
                        "P(b) @ [6:7]\n");
    std::string formulas("100: P(b) -> <>{f} [P(a) v P(b)]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(2);

    const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_FOREACH(Move move, moves) {
        std::cout << "finish move: " << move.toString() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm1fi) {
    std::string facts(  "P(a) @ [1:4]\n"
                        "P(a) @ [7:7]\n"
                        "P(b) @ [6:8]\n");
    std::string formulas("100: P(b) -> <>{fi} [P(a) v P(b)]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(2);

    const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_FOREACH(Move move, moves) {
        std::cout << "finish move: " << move.toString() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm2) {
    std::string facts(  "P(a) @ [1:4]\n"
                        "P(b) @ [1:1]\n"
                        "P(b) @ [5:6]\n"
                        "P(c) @ [9:10]\n");
    std::string formulas("100: P(a) ; P(b) -> <>{mi} [P(b) v P(c)]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(2);

    const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_FOREACH(Move move, moves) {
        std::cout << "form2 move: " << move.toString() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(maxWalkSatTestForm3) {
    std::string facts(  "P(a) @ [1:3]\n"
                        "P(b) @ [1:8]\n"
                        "P(c) @ [4:5]\n"
                        "P(d) @ [6:8]");

    std::string formulas("100: P(b) -> !(P(a) ; P(c); P(d))\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(2);

    const Disjunction *s = dynamic_cast<const Disjunction *>(&(*d.formulas().at(0).sentence()));
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_FOREACH(Move move, moves) {
        std::cout << "form3 move: " << move.toString() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(diamondLessThan) {
    std::string facts(  "ElevatorEnter(a) @ [1:5]");
    std::string formulas("100: [ !ElevatorEnter(a) ] v <>{>} [ GateExit(a) ]");

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setDontModifyObsPreds(false);
    srand(0);
    d.setMaxInterval(Interval(1,10));

    const Sentence *s = &(*d.formulas().at(0).sentence());
    std::vector<Move> moves = findMovesFor(d, d.defaultModel(), *s);
    BOOST_CHECK(moves.size()!=0);
    BOOST_FOREACH(Move m, moves) {
        std::cout << "move = " << m.toString() << std::endl;
    }
}
