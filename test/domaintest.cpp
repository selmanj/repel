#define BOOST_TEST_MODULE Domain
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <cstdlib>
#include "testutilities.h"
#include "logic/domain.h"
#include "logic/el_syntax.h"
#include "logic/folparser.h"


BOOST_AUTO_TEST_CASE( sat_test )
{
    std::stringstream facts;
    facts << "P(a,b) @ [1:10]";
    facts << "Q(a,b) @ [5:15]";


    std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
    std::vector<FOL::Event> factvec;
    FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
    //    std::vector<ELSentence> formulas;

    Domain d = loadDomainWithStreams(facts.str(), "");
    d.setMaxInterval(Interval(0,1000));
    boost::shared_ptr<Sentence> query = boost::dynamic_pointer_cast<Sentence>(factvec.front().atom());
    SISet trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10]}");

    // negation
    query = getAsSentence("!P(a,b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 0), (0, 1000)], [(1, 10), (11, 1000)], [11:1000]}");

    //lets try disjunction
    query = getAsSentence("P(a,b) v Q(a,b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10], [(5, 10), (11, 15)], [11:15]}");

    // a bit more complicated
    query = getAsSentence("!(P(a,b) -> Q(a,b))");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[(1, 4), (1, 10)]}");

    // liq op
    query = getAsSentence("[ !P(a,b) ]");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[0:0], [11:1000]}");

    query = getAsSentence("[ !Q(a,b) ^ P(a,b) ]");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:4]}");

    // lets try liq disjunction
    query = getAsSentence("[ !(P(a,b) -> Q(a,b)) ]");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:4]}");

    // liq conjunction
    query = getAsSentence("[ P(a,b) ^ Q(a,b) ]");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[5:10]}");

    query = getAsSentence("[ P(a,b) v Q(a,b) ]");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:15]}");

    // diamond op
    query = getAsSentence("<>{mi} Q(a,b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 14), (4, 14)]}");

    query = getAsSentence("<>{s,f} Q(a,b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[(0, 4), (5, 15)], [5:5], [(5, 15), (6, 1000)]}");

    // conjunction
    query = getAsSentence("P(a,b) ; Q(a,b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    trueAt.makeDisjoint();
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[(1, 10), (5, 10)], [(1, 10), (11, 15)]}");

    query = getAsSentence("P(a,b) ^ Q(a,b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[5:10]}");
}

BOOST_AUTO_TEST_CASE( sat_where ) {
    std::stringstream facts;
    facts << "P(a,b) @ [1:10]";
    facts << "Q(a,b) @ [5:15]";

    std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
    std::vector<FOL::Event> factvec;
    FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);

    std::vector<ELSentence> formulas;

    Domain d = loadDomainWithStreams(facts.str(), "");
    //d.setMaxInterval(Interval(0,1000));

    boost::shared_ptr<Sentence> query = getAsSentence("[Q(a,b) -> P(a,b)]");
    SISet trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:10]}");

    // now try only doing it at timepoints [5:7]
    SISet someTime(true, d.maxInterval());
    someTime.add(SpanInterval(5,7,5,7));

    trueAt = query->dSatisfied(d.defaultModel(), d, someTime);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[5:7]}");

    query = getAsSentence("[!(Q(a,b) -> P(a,b))]");
    trueAt = query->dSatisfied(d.defaultModel(), d, someTime);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

}

BOOST_AUTO_TEST_CASE( conjunctionIntervalTest ) {
    std::stringstream facts;
    facts << "A(a) @ [1:10]";
    facts << "B(b) @ [2:10]";

    std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
    std::vector<FOL::Event> factvec;
    FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
    std::vector<ELSentence> formulas;
    Domain d = loadDomainWithStreams(facts.str(), "");

    boost::shared_ptr<Sentence> query = getAsSentence("<>{mi} A(a)");
    SISet trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:9]}");

    query = getAsSentence("<>{mi} B(b)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:9]}");
}

BOOST_AUTO_TEST_CASE( conjunctionMeetsTest ) {
    std::stringstream facts;
    facts << "Q(a) @ [1:1]";
    facts << "R(a) @ [3:3]";

    std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
    std::vector<FOL::Event> factvec;
    FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
    std::vector<ELSentence> formulas;
    Domain d = loadDomainWithStreams(facts.str(), "");

    boost::shared_ptr<Sentence> query = getAsSentence("Q(a) ; R(a)");
    SISet trueAt = query->dSatisfied(d.defaultModel(), d);
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
    std::vector<ELSentence> formulas;
    Domain d = loadDomainWithStreams(facts.str(), "");

    boost::shared_ptr<Sentence> query = getAsSentence("R(a) ^{o} R(a)");
    SISet trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

    query = getAsSentence("R(a) ^{o} Q(a)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

    query = getAsSentence("Q(a) ^{o} R(a)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

    query = getAsSentence("Q(a) ^{o} Q(a)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

    query = getAsSentence("R(a) ^{o} S(a)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
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
    std::vector<ELSentence> formulas;
    Domain d = loadDomainWithStreams(facts.str(), "");

    boost::shared_ptr<Sentence> query;
    SISet trueAt;

    query = getAsSentence("true");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{[1:5]}");

    query = getAsSentence("false");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    BOOST_CHECK_EQUAL(trueAt.toString(), "{}");

    query = getAsSentence("S(a) ; S(a)");
    trueAt = query->dSatisfied(d.defaultModel(), d);
    trueAt.makeDisjoint();
    //BOOST_CHECK_EQUAL(trueAt.toString(), "{}");
}

BOOST_AUTO_TEST_CASE( randomModelTest ) {
    srand(0);
    std::stringstream facts;
    facts << "Q(a) @ [1:3]\n";
    facts << "!Q(a) @ [4:15]\n";
    facts << "R(a) @ [6:10]\n";
    facts << "S(a) @ [11:20]\n";

    std::vector<FOLToken> tokens = FOLParse::tokenize(&facts);
    std::vector<FOL::Event> factvec;
    FOLParse::parseEvents(tokens.begin(), tokens.end(), factvec);
    std::vector<ELSentence> formulas;
    Domain d = loadDomainWithStreams(facts.str(), "");

    Model randomModel = d.randomModel();

    std::cout << "atoms = ";
    std::set<Atom, atomcmp> atoms = randomModel.atoms();
    for (std::set<Atom, atomcmp>::const_iterator it = atoms.begin(); it != atoms.end(); it++) {
        std::cout << it->toString() << " at " << randomModel.getAtom(*it) << ", ";
    }
    std::cout << std::endl;

    BOOST_CHECK_EQUAL(randomModel.toString(), "Q(a) @ {[1:3], [16:17]}\n"
            "R(a) @ {[1:1], [5:13]}\n"
            "S(a) @ {[1:3], [6:7], [9:9], [11:20]}\n");
    //std::cout << "random model: " << randomModel.toString() << std::endl;
}




