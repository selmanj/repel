/*
 * mcsattest.cpp
 *
 *  Created on: Feb 12, 2012
 *      Author: selman.joe@gmail.com
 */

#define BOOST_TEST_MODULE MCSat
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/random.hpp>
#include <string>
#include "../src/inference/MCSat.h"
#include "../src/inference/MCSatSamplePerfectlyStrategy.h"
#include "../src/inference/MCSatSampleLiquidlyStrategy.h"
#include "../src/logic/syntax/Proposition.h"
#include "TestUtilities.h"

BOOST_AUTO_TEST_CASE( mcsat_test)
{
    boost::mt19937 rng;
    // disable debug logging
    FileLog::globalLogLevel() = LOG_ERROR;
    std::string facts("D-P(a) @ {[1:50]}\n");

    std::string formulas("0.1: [ P(a) -> D-P(a) ] @ [1:100]\n"
            "0.1: [ D-P(a) -> P(a) ] @ [1:100]\n");
    Interval maxInterval(1,100);
    int numRuns = 5;


    unsigned int counts[maxInterval.finish() - maxInterval.start() + 1][numRuns];

    Domain d = loadDomainWithStreams(facts, formulas);
    d.setMaxInterval(maxInterval);

    //mcSatSolver.setBurnInIterations(10000);
    for (int i = 0; i < numRuns; i++) {
        std::cout << "- Run " << i+1 << std::endl;
        MCSat mcSatSolver(&d);
        mcSatSolver.setBurnInIterations(5000);
        mcSatSolver.setSampleStrategy(new MCSatSampleLiquidlyStrategy());
        mcSatSolver.run(rng);
        BOOST_CHECK_EQUAL(mcSatSolver.size(), MCSat::defNumSamples);
        boost::shared_ptr<Sentence> pa = getAsSentence("P(a)");
        Proposition propPa(static_cast<const Atom&>(*pa), true);

        std::cout << "counts:" << std::endl;
        for (unsigned int j = maxInterval.start(); j <= maxInterval.finish(); j++) {
            counts[j][i] = mcSatSolver.countProps(propPa, Interval(j,j));
            std::cout << counts[j][i];
            if (j != maxInterval.finish()) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "- Average counts:" << std::endl;
    for (unsigned int j = maxInterval.start(); j <= maxInterval.finish(); j++) {
        double avg = 0.0;
        for (int i = 0; i < numRuns; i++) {
            avg += counts[j][i];
        }
        avg = avg/ (double)numRuns;
        std::cout << avg;
        if (j != maxInterval.finish()) std::cout << ", ";
    }
    std::cout << std::endl;
    //BOOST_CHECK_CLOSE(probPaAt11, .99, .2);
}

BOOST_AUTO_TEST_CASE( mcsat_test2)
{
    boost::mt19937 rng;
    // disable debug logging
    FileLog::globalLogLevel() = LOG_ERROR;
    std::string facts("D-P(a) @ {[1:10]}\n");
    std::string formulas("1: [ D-P(a) -> !P(a) ] @ [1:10]\n"
            "1.2: [ D-P(a) -> P(a) ] @ [1:10]\n");

    Domain d = loadDomainWithStreams(facts, formulas);

    MCSat mcSatSolver(&d);
    mcSatSolver.setSampleStrategy(new MCSatSampleLiquidlyStrategy());
    //mcSatSolver.setBurnInIterations(10000);
    mcSatSolver.run(rng);
    BOOST_CHECK_EQUAL(mcSatSolver.size(), MCSat::defNumSamples);
    boost::shared_ptr<Sentence> pa = getAsSentence("P(a)");
    Proposition propPa(static_cast<const Atom&>(*pa), true);
    double probPaAt11 =  mcSatSolver.estimateProbability(propPa, Interval(1,1));
    std::cout << "probPaAt11 = " << probPaAt11 << std::endl;

    std::cout << "counts:" << std::endl;
    for (unsigned int i = 1; i <= 10; i++) {
        std::cout << mcSatSolver.countProps(propPa, Interval(i,i));
        if (i != 10) std::cout << ", ";
    }
    std::cout << std::endl;
    //BOOST_CHECK_CLOSE(probPaAt11, .99, .2);
}

BOOST_AUTO_TEST_CASE( mcsatSamplePerfectlyStrategy) {
    boost::mt19937 rng;

    std::string facts("D-P(a) @ {[1:25]}\n");
    std::string formulas("100: [ P(a) -> D-P(a) ] @ [1:15]\n");
    Domain d = loadDomainWithStreams(facts, formulas);
    MCSatSamplePerfectlyStrategy strategy;
    std::vector<ELSentence> sampled;
    strategy.sampleSentences(d.defaultModel(), d, rng, sampled);
    BOOST_REQUIRE(sampled.size() == 1);
    std::cout << sampled[0].quantification().toString();

}



