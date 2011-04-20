#define BOOST_TEST_MODULE FOLParser
#ifdef HAVE_CONFIG_H
  #include "../config.h"
#endif
#ifdef HAVE_BOOST_UNIT_TEST_FRAMEWORK
  #define BOOST_TEST_DYN_LINK
  #include <boost/test/unit_test.hpp>
#else
  #include <boost/test/included/unit_test.hpp>
#endif
#include <boost/test/unit_test.hpp>
#include "../src/fol/folparser.h"
#include "../src/fol/follexer.h"
#include "../src/fol/foltoken.h"
#include "../src/fol/constant.h"
#include "../src/spaninterval.h"
#include <iostream>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE( parser_test )
{
  std::istringstream stream("flying(a,theAir) @ [1,11] # this is a test of a comment \n");
 
  std::vector<FOLToken> tokens = FOLParse::tokenize(&stream); 
  boost::shared_ptr<Atom> a = FOLParse::parseGroundAtom(tokens.begin(), tokens.end());
  BOOST_CHECK_EQUAL(a->name(), "flying");
  BOOST_CHECK_EQUAL(a->arity(), 2);
}

BOOST_AUTO_TEST_CASE( interval_test )
{
  std::istringstream stream("[1,11]");
  std::vector<FOLToken> tokens = FOLParse::tokenize(&stream); 
  SpanInterval interval = FOLParse::parseInterval(tokens.begin(), tokens.end());
  BOOST_CHECK_EQUAL(interval.start().start(), 1);
  BOOST_CHECK_EQUAL(interval.start().end(), 1);
  BOOST_CHECK_EQUAL(interval.end().start(), 11);
  BOOST_CHECK_EQUAL(interval.end().end(), 11);
}

BOOST_AUTO_TEST_CASE( event_test) 
{
  std::istringstream stream("mrManager(georgeMichael, bananaStand) @ [10:20]");
  std::vector<FOLToken> tokens = FOLParse::tokenize(&stream); 
  boost::shared_ptr<Event> e = FOLParse::parseEvent(tokens.begin(), tokens.end());
  BOOST_CHECK_EQUAL(e->fact()->name(), "mrManager");
  BOOST_CHECK_EQUAL(e->fact()->at(0).name(), "georgeMichael");
  BOOST_CHECK_EQUAL(e->fact()->at(1).name(), "bananaStand");

}

BOOST_AUTO_TEST_CASE( atom_test) 
{
  std::istringstream stream("love(?cats, meowmix)");
  std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
  boost::shared_ptr<Atom> a = FOLParse::parseAtom(tokens.begin(), tokens.end());
  BOOST_CHECK_EQUAL(a->name(), "love");
  BOOST_CHECK_EQUAL(a->arity(), 2);
  BOOST_CHECK_EQUAL((*a)[0].name(), "cats");
  BOOST_CHECK_EQUAL((*a)[1].name(), "meowmix");
  BOOST_CHECK(dynamic_cast<Variable*>(&(*a)[0]) != NULL);
  BOOST_CHECK(dynamic_cast<Constant*>(&(*a)[1]) != NULL);
}
