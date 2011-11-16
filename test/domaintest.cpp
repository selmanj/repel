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
#include <cstdlib>
#include "testutilities.h"
#include "fol/domain.h"
#include "fol/fol.h"
#include "fol/folparser.h"
#include "fol/formulaset.h"


BOOST_AUTO_TEST_CASE( sat_test )
{
	std::stringstream facts;
	facts << "P(a,b) @ [1:10]";
	facts << "Q(a,b) @ [5:15]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::Event> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);

	FormulaSet formulas;

	Domain d(factvec.begin(), factvec.end(), formulas);
	d.setMaxInterval(Interval(0,1000));
	boost::shared_ptr<Sentence> query = boost::dynamic_pointer_cast<Sentence>(factvec.front().atom());
	SISet trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10]}");

	// negation
	query = getAsSentence("!P(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 0), (0, 1000)], [(1, 10), (11, 1000)], [11:1000]}");

	//lets try disjunction
	query = getAsSentence("P(a,b) v Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10], [(5, 10), (11, 15)], [11:15]}");

	// a bit more complicated
	query = getAsSentence("!(P(a,b) -> Q(a,b))");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(1, 4), (1, 10)]}");

	// liq op
	query = getAsSentence("[ !P(a,b) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[0:0], [11:1000]}");

	query = getAsSentence("[ !Q(a,b) ^ P(a,b) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:4]}");

	// lets try liq disjunction
	query = getAsSentence("[ !(P(a,b) -> Q(a,b)) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:4]}");

	// liq conjunction
	query = getAsSentence("[ P(a,b) ^ Q(a,b) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[5:10]}");

	query = getAsSentence("[ P(a,b) v Q(a,b) ]");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:15]}");

	// diamond op
	query = getAsSentence("<>{mi} Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 14), (4, 14)]}");

	query = getAsSentence("<>{s,f} Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 4), (5, 15)], [5:5], [(5, 15), (6, 1000)]}");

	// conjunction
	query = getAsSentence("P(a,b) ; Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	trueAt.makeDisjoint();
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(1, 10), (5, 10)], [(1, 10), (11, 15)]}");

	query = getAsSentence("P(a,b) ^ Q(a,b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[5:10]}");
}

BOOST_AUTO_TEST_CASE( conjunctionIntervalTest ) {
	std::stringstream facts;
	facts << "A(a) @ [1:10]";
	facts << "B(b) @ [2:10]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::Event> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
	FormulaSet formulas;
	Domain d(factvec.begin(), factvec.end(), formulas);

	boost::shared_ptr<Sentence> query = getAsSentence("<>{mi} A(a)");
	SISet trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:9]}");

	query = getAsSentence("<>{mi} B(b)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:9]}");
}

BOOST_AUTO_TEST_CASE( conjunctionMeetsTest ) {
	std::stringstream facts;
	facts << "Q(a) @ [1:1]";
	facts << "R(a) @ [3:3]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::Event> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
	FormulaSet formulas;
	Domain d(factvec.begin(), factvec.end(), formulas);

	boost::shared_ptr<Sentence> query = getAsSentence("Q(a) ; R(a)");
	SISet trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{}");
}

BOOST_AUTO_TEST_CASE( conjunctionOverlapsTest ) {
	std::stringstream facts;
	facts << "Q(a) @ [1:1]";
	facts << "R(a) @ [1:2]";
	facts << "S(a) @ [2:3]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::Event> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
	FormulaSet formulas;
	Domain d(factvec.begin(), factvec.end(), formulas);

	boost::shared_ptr<Sentence> query = getAsSentence("R(a) ^{o} R(a)");
	SISet trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

	query = getAsSentence("R(a) ^{o} Q(a)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

	query = getAsSentence("Q(a) ^{o} R(a)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

	query = getAsSentence("Q(a) ^{o} Q(a)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

	query = getAsSentence("R(a) ^{o} S(a)");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[(1, 1), (3, 3)]}");
}

BOOST_AUTO_TEST_CASE( trueFalseTest ) {
	std::stringstream facts;
	facts << "Q(a) @ [1:3]";
	facts << "R(a) @ [3:5]";
	facts << "S(a) @ [1:3]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::Event> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
	FormulaSet formulas;
	Domain d(factvec.begin(), factvec.end(), formulas);

	boost::shared_ptr<Sentence> query;
	SISet trueAt;

	query = getAsSentence("true");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:5]}");

	query = getAsSentence("false");
	trueAt = d.satisfied(*query, d.defaultModel());
	BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

	query = getAsSentence("S(a) ; S(a)");
	trueAt = d.satisfied(*query, d.defaultModel());
	trueAt.makeDisjoint();
	//BOOST_CHECK_EQUAL(trueAt.toString(), "{}");
}

BOOST_AUTO_TEST_CASE( randomModelTest ) {
	srand(0);
	std::stringstream facts;
	facts << "Q(a) @ [1:3]";
	facts << "!Q(a) @ [4:15]";
	facts << "R(a) @ [6:10]";
	facts << "S(a) @ [11:20]";

	std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
	std::vector<FOL::Event> factvec;
	FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
	FormulaSet formulas;
	Domain d(factvec.begin(), factvec.end(), formulas, false);

	Model randomModel = d.randomModel();
	BOOST_CHECK_EQUAL(randomModel.toString(), "Q(a) @ {[1:3], [17:18]}\n"
			"R(a) @ {[1:1], [4:4], [6:10], [12:13], [16:17]}\n"
			"S(a) @ {[1:1], [5:5], [7:7], [11:20]}\n");
	//std::cout << "random model: " << randomModel.toString() << std::endl;

}


