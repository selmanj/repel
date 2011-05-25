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
#include "../src/fol/fol.h"
#include "../src/spaninterval.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <utility>


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
	std::pair<boost::shared_ptr<Atom>, SpanInterval> e = FOLParse::parseEvent(tokens.begin(), tokens.end());
	BOOST_CHECK_EQUAL(e.first->name(), "mrManager");
	BOOST_CHECK_EQUAL(e.first->at(0)->name(), "georgeMichael");
	BOOST_CHECK_EQUAL(e.first->at(1)->name(), "bananaStand");

}

BOOST_AUTO_TEST_CASE( atom_test) 
{
	std::istringstream stream("love(?cats, meowmix)");
	std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
	boost::shared_ptr<Atom> a = FOLParse::parseAtom(tokens.begin(), tokens.end());
	BOOST_CHECK_EQUAL(a->name(), "love");
	BOOST_CHECK_EQUAL(a->arity(), 2);
	BOOST_CHECK_EQUAL(a->at(0)->name(), "cats");
	BOOST_CHECK_EQUAL(a->at(1)->name(), "meowmix");
	BOOST_CHECK(boost::dynamic_pointer_cast<Variable>(a->at(0)) != NULL);
	BOOST_CHECK(boost::dynamic_pointer_cast<Constant>(a->at(1)) != NULL);
}

BOOST_AUTO_TEST_CASE( static_formula_test )
{
	std::istringstream stream("p(?x,y)");	// simple atom case
	std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
	boost::shared_ptr<Sentence> s = FOLParse::parseStaticFormula(tokens.begin(), tokens.end());
	boost::shared_ptr<Atom> a = boost::dynamic_pointer_cast<Atom>(s);
	BOOST_CHECK(a != NULL);
	BOOST_CHECK_EQUAL(a->name(), "p");
	BOOST_CHECK(boost::dynamic_pointer_cast<Variable>(a->at(0)) != NULL);

	std::istringstream stream2("p(x) ^ q(x) -> r(x)");
	tokens = FOLParse::tokenize(&stream2);
	boost::shared_ptr<Sentence> s2 = FOLParse::parseStaticFormula(tokens.begin(), tokens.end());
	BOOST_CHECK_EQUAL(s2->toString(), "!(p(x) ^ q(x)) v r(x)");

	std::istringstream stream3("p(x) ^ (q(x) v r(x))");
	tokens = FOLParse::tokenize(&stream3);
	boost::shared_ptr<Sentence> s3 = FOLParse::parseStaticFormula(tokens.begin(), tokens.end());
	BOOST_CHECK_EQUAL(s3->toString(), "p(x) ^ (q(x) v r(x))");

	std::istringstream stream4("p(x) ^ q(x) v r(x) -> z(x)");
	tokens = FOLParse::tokenize(&stream4);
	boost::shared_ptr<Sentence> s4 = FOLParse::parseStaticFormula(tokens.begin(), tokens.end());
	BOOST_CHECK_EQUAL(s4->toString(), "!(p(x) ^ q(x) v r(x)) v z(x)");
}

BOOST_AUTO_TEST_CASE( weighted_formula_test ) {
	std::istringstream stream("10: p(?x,y)");	// simple atom case
	std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
	WSentence form = FOLParse::parseWeightedFormula(tokens.begin(), tokens.end());

	BOOST_CHECK_EQUAL(form.weight(), 10);
	BOOST_CHECK_EQUAL(form.sentence()->toString(), "p(?x, y)");

	std::istringstream stream2("55: [ p(?x,?y) ^ q(?x,?y) -> r(?x, ?y) ]");
	tokens = FOLParse::tokenize(&stream2);
	form = FOLParse::parseWeightedFormula(tokens.begin(), tokens.end());

	BOOST_CHECK_EQUAL(form.weight(), 55);
	BOOST_CHECK_EQUAL(form.sentence()->toString(), "[ !(p(?x, ?y) ^ q(?x, ?y)) v r(?x, ?y) ]");

	std::istringstream stream3("0: <>{m,mi} p(x) ^{o} q(x) ; r(x) ; <> z(x)");
	tokens = FOLParse::tokenize(&stream3);
	form = FOLParse::parseWeightedFormula(tokens.begin(), tokens.end());

	BOOST_CHECK_EQUAL(form.weight(), 0);
	BOOST_CHECK_EQUAL(form.sentence()->toString(), "<>{m, mi} (p(x) ^ q(x) ^ r(x) ^ <> z(x))");
}
