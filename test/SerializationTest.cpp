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
#include "../src/inference/MCSat.h"
#include "../src/AllSerializationExports.h"
#include "TestUtilities.h"

template <class T>
void checkSerializationPtr(T s) {
    std::stringstream stream;
    {
        boost::archive::text_oarchive oarch(stream);
        registerAllPELTypes(oarch);
        oarch << s;
    }
    T t;
    {
        boost::archive::text_iarchive iarch(stream);
        registerAllPELTypes(iarch);
        iarch >> t;
    }

    BOOST_CHECK(*s == *t);
}

template <class T>
void checkSerialization(T s) {
    std::stringstream stream;
    {
        boost::archive::text_oarchive oarch(stream);
        registerAllPELTypes(oarch);
        oarch << s;
    }
    T t;
    {
        boost::archive::text_iarchive iarch(stream);
        registerAllPELTypes(iarch);
        iarch >> t;
    }

    BOOST_CHECK(s == t);
}

BOOST_AUTO_TEST_CASE(generalSentenceSerialization) {
    checkSerializationPtr(getAsSentence("P(a)"));
    checkSerializationPtr(getAsSentence("true"));
    checkSerializationPtr(getAsSentence("false"));
    checkSerializationPtr(getAsSentence("!P(a)"));
    checkSerializationPtr(getAsSentence("P(a) v Q(a)"));
    checkSerializationPtr(getAsSentence("P(a) ^{m} R(a)"));
    checkSerializationPtr(getAsSentence("[P(a)]"));
    checkSerializationPtr(getAsSentence("<>{m} P(a)"));
    checkSerializationPtr(getAsSentence("P(a) v [!R(a) ^ S(a)] v <>{s} T(a)"));
}

BOOST_AUTO_TEST_CASE(domainSerialization) {
    std::string facts("P(a) @ [1:10]\n"
            "Q(A) @ [5:6]");
    std::string formulas("1: [P(a) -> Q(a)]\n"
            "[Q(a) -> P(a)]");

    Domain d = loadDomainWithStreams(facts, formulas);
    checkSerialization(d);
}

BOOST_AUTO_TEST_CASE(mcSatSerialization) {
    std::string facts("P(a) @ [1:10]\n"
            "Q(A) @ [5:6]");
    std::string formulas("1: [P(a) -> Q(a)]\n"
            "[Q(a) -> P(a)]");

    Domain d = loadDomainWithStreams(facts, formulas);
    MCSat sat(&d);
    // don't actually run it, it takes too long. just test it
    checkSerialization(sat);
}
