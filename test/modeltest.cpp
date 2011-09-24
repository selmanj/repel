/*
 * modeltest.cpp
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */


#define BOOST_TEST_MODULE Model
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#ifdef HAVE_BOOST_UNIT_TEST_FRAMEWORK
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include "../src/fol/model.h"
#include "../src/siset.h"
#include "../src/fol/fol.h"

// TODO: write some test cases fool!
BOOST_AUTO_TEST_CASE(basicModelTest) {

}
