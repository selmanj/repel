#define BOOST_TEST_MODULE Domain
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#ifdef HAVE_BOOST_UNIT_TEST_FRAMEWORK
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include <sstream>
#include "../src/fol/domain.h"
#include "../src/fol/fol.h"
#include "../src/fol/folparser.h"

boost::shared_ptr<Sentence> getAsSentence(std::string str);

BOOST_AUTO_TEST_CASE( sat_test )
{
	std::stringstream facts;
	facts << "P(a,b) @ [1:10]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	FOL::EventPair pair = FOLParse::parseEvent(tokens.begin(), tokens.end());
	std::vector<FOL::EventPair> events;
	std::vector<WSentence> formulas;
	events.push_back(pair);

	Domain d(events.begin(), events.end(), formulas.begin(), formulas.end());
	d.setMaxInterval(Interval(0,1000));
	boost::shared_ptr<Sentence> query = boost::dynamic_pointer_cast<Sentence>(pair.first);
	SISet trueAt = d.satisfied(query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10]}");

	{
		boost::shared_ptr<Sentence> q1 = getAsSentence("P(a,b)");
		boost::shared_ptr<Sentence> q2 = getAsSentence("P(a,b)");
		BOOST_CHECK((*q1)==(*q2));

	}

	// wrap it in negation
	query = getAsSentence("!P(a,b)");
	trueAt = d.satisfied(query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[0:0], [11:1000]}");


}

boost::shared_ptr<Sentence> getAsSentence(std::string str) {
	std::istringstream stream(str);
	std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
	return FOLParse::parseStaticFormula(tokens.begin(),tokens.end());
}


