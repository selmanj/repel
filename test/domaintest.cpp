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
	facts << "Q(a,b) @ [5:15]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::EventPair> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);

	std::vector<WSentence> formulas;

	Domain d(factvec.begin(), factvec.end(), formulas.begin(), formulas.end());
	d.setMaxInterval(Interval(0,1000));
	boost::shared_ptr<Sentence> query = boost::dynamic_pointer_cast<Sentence>(factvec.front().first);
	SISet trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10]}");

	// negation
	query = getAsSentence("!P(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 0), (0, 1000)], [(1, 10), (11, 1000)], [11:1000]}");

	//lets try disjunction
	query = getAsSentence("P(a,b) v Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10], [5:15]}");

	// a bit more complicated
	query = getAsSentence("!(P(a,b) -> Q(a,b))");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(1, 4), (1, 10)]}");

	// liq op
	query = getAsSentence("[ !P(a,b) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[0:0], [11:1000]}");

	// lets try liq disjunction
	query = getAsSentence("[ !(P(a,b) -> Q(a,b)) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:4]}");

	// liq conjunction
	query = getAsSentence("[ P(a,b) ^ Q(a,b) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[5:10]}");

	// diamond op
	query = getAsSentence("<>{mi} Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 14), (4, 14)]}");

	query = getAsSentence("<>{s,f} Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 14), (5, 15)], [(5, 15), (6, 1000)]}");
}

boost::shared_ptr<Sentence> getAsSentence(std::string str) {
	std::istringstream stream(str);
	std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
	return FOLParse::parseFormula(tokens.begin(),tokens.end());
}


