/*
 * MWSTest.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: selman.joe@gmail.com
 */

#define BOOST_TEST_MODULE MWSTest
#define BOOST_TEST_MAIN
#include "../src/config.h"
#ifdef USE_DYNAMIC_UNIT_TEST
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include <boost/random/mersenne_twister.hpp>
#include "TestUtilities.h"
#include "../src/inference/MaxWalkSat.h"

BOOST_AUTO_TEST_CASE(MaxWalkSat_simple) {
    std::string facts(
            "D-P(a) @ [1:10]\n"
            "D-Q(a) @ [1:15]\n");

    std::string formulas(
            "1: [!D-P(a) v P(a)]\n"
            "1: [!P(a) v D-P(a)]\n"
            "1: [!D-Q(a) v Q(a)]\n"
            "1: [!Q(a) v D-Q(a)]\n");

    Domain d = loadDomainWithStreams(facts, formulas);
    MWSSolver solver(&d);
    boost::mt19937 rng(0);
    Model m = solver.run(rng);
    BOOST_CHECK_EQUAL(m.toString(), "D-P(a) @ {[1:10]}\n"
            "D-Q(a) @ {[1:15]}\n"
            "P(a) @ {[1:10]}\n"
            "Q(a) @ {[1:15]}\n");
}
