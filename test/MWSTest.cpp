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

BOOST_AUTO_TEST_CASE(MaxWalkSat) {
    return; //  TODO add unit tests

}
