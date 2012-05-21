/*
 * modeltest.cpp
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */


#define BOOST_TEST_MODULE Model
#include "../src/config.h"
#define BOOST_TEST_MAIN
#ifdef USE_DYNAMIC_UNIT_TEST
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include "logic/Model.h"
#include "SISet.h"
#include "logic/ELSyntax.h"

// TODO: write some test cases fool!
BOOST_AUTO_TEST_CASE(basicModelTest) {

}
