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
#include "../src/logic/syntax/SentenceSerializationExport.h"
#include "../src/logic/syntax/TermSerializationExports.h"
#include "TestUtilities.h"

void checkSentenceSerialization(boost::shared_ptr<Sentence> s) {
    std::stringstream stream;
    {
        boost::archive::text_oarchive oarch(stream);
        oarch << s;
    }
    boost::shared_ptr<Sentence> t;
    {
        boost::archive::text_iarchive iarch(stream);
        iarch >> t;
    }

    BOOST_CHECK(*s == *t);
}

BOOST_AUTO_TEST_CASE(generalSentenceSerialization) {
    checkSentenceSerialization(getAsSentence("P(a)"));
    checkSentenceSerialization(getAsSentence("true"));
    checkSentenceSerialization(getAsSentence("false"));
    checkSentenceSerialization(getAsSentence("!P(a)"));
    checkSentenceSerialization(getAsSentence("P(a) v Q(a)"));
    checkSentenceSerialization(getAsSentence("P(a) ^{m} R(a)"));
    checkSentenceSerialization(getAsSentence("[P(a)]"));
    checkSentenceSerialization(getAsSentence("<>{m} P(a)"));
    checkSentenceSerialization(getAsSentence("P(a) v [!R(a) ^ S(a)] v <>{s} T(a)"));
}
