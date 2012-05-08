/*
 * UtilTest.cpp
 *
 *  Created on: May 7, 2012
 *      Author: joe
 */

#define BOOST_TEST_MODULE UtilTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <iostream>
#include "../src/util/RNG.h"

BOOST_AUTO_TEST_CASE( rng ) {
    boost::mt19937 gen;
    RNG<boost::mt19937::result_type> rng = rngFromRef<boost::mt19937::result_type>(gen);

    BOOST_CHECK_EQUAL(gen(), 3499211612);
    BOOST_CHECK_EQUAL(rng(), 581869302);
    BOOST_CHECK_EQUAL(gen(), 3890346734);
    BOOST_CHECK_EQUAL(rng(), 3586334585);
}
