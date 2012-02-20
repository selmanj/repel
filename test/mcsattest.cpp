/*
 * mcsattest.cpp
 *
 *  Created on: Feb 12, 2012
 *      Author: selman.joe@gmail.com
 */

#define BOOST_TEST_MODULE MCSat
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/random.hpp>
#include <string>
#include "../src/inference/mcsat.h"
#include "testutilities.h"

BOOST_AUTO_TEST_CASE( mcsat_test)
{
    std::string facts("P(a) @ [1:10]\n");
    std::string formulas("1: [ P(a) ] @ [1:15]\n"
            "inf: P(a) -> Q(a) \n");


    Domain d = loadDomainWithStreams(facts, formulas);

    MCSat mcSatSolver;
    mcSatSolver.setNumIterations(1000);
    boost::mt19937 gen(0);
    mcSatSolver.run(d, gen);

}
