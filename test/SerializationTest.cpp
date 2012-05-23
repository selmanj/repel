/*
 * SerializationTest.cpp
 *
 *  Created on: May 23, 2012
 *      Author: joe
 */

#define BOOST_TEST_MODULE SerializationTests
#include "../src/config.h"
#define BOOST_TEST_MAIN
#ifdef USE_DYNAMIC_UNIT_TEST
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>
#include <sstream>
#include "../src/logic/syntax/BoolLit.h"

BOOST_AUTO_TEST_CASE(boolLitSerialization) {
    BoolLit bT(true), bF(false);

    std::stringstream stream;
    {
        boost::archive::text_oarchive oarch(stream);
        oarch << bT;
        oarch << bF;
    }
    BoolLit bTloaded(false), bFloaded(true), bSLoaded(true);
    {
        boost::archive::text_iarchive iarch(stream);
        iarch >> bTloaded;
        iarch >> bFloaded;
    }

    BOOST_CHECK(bT == bTloaded);
    BOOST_CHECK(bF == bFloaded);

}
