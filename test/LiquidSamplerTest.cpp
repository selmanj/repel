/*
 * liquidsamplertest.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#define BOOST_TEST_MODULE LiquidSampler
#define BOOST_TEST_MAIN
#include "../src/config.h"
#ifdef USE_DYNAMIC_UNIT_TEST
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include <vector>
#include <iterator>

#include "../src/inference/LiquidSampler.h"
#include "../src/Interval.h"
#include "../src/SpanInterval.h"

BOOST_AUTO_TEST_CASE( samplerTest )
{
    LiquidSampler sampler;
    std::vector<SpanInterval> sampled;

    boost::mt19937 rng;
    SpanInterval si(1,10,1,10);
    sampled = sampler(si, 0.99, rng);
    BOOST_REQUIRE_EQUAL(sampled.size(), 1);
    BOOST_CHECK_EQUAL(sampled[0], si);

    sampled = sampler(si, 0.001, rng);
    BOOST_CHECK(sampled.empty());

    sampled = sampler(si, 0.1, rng);
    BOOST_REQUIRE_EQUAL(sampled.size(), 3);
    BOOST_CHECK_EQUAL(sampled[0], SpanInterval(2,4));
    BOOST_CHECK_EQUAL(sampled[1], SpanInterval(7,8));
    BOOST_CHECK_EQUAL(sampled[2], SpanInterval(5,6));

}

BOOST_AUTO_TEST_CASE( samplerTest2 )
{
    boost::mt19937 rng;

    LiquidSampler sampler;
    std::vector<SpanInterval> sampled;

    std::cout << "S1: ";
    sampled = sampler(SpanInterval(1,50,1,50), 1.0-exp(-1.0), rng);
    std::copy(sampled.begin(), sampled.end(), std::ostream_iterator<SpanInterval>(std::cout, ", "));
    std::cout << std::endl;

    std::cout << "S2: ";
    sampled = sampler(SpanInterval(51,100,51,100), 1.0-exp(-1.0), rng);
    std::copy(sampled.begin(), sampled.end(), std::ostream_iterator<SpanInterval>(std::cout, ", "));
    std::cout << std::endl;
}
