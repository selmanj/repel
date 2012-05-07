/*
 * liquidsamplertest.cpp
 *
 *  Created on: Apr 23, 2012
 *      Author: selman.joe@gmail.com
 */

#define BOOST_TEST_MODULE LiquidSampler
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iterator>

#include "../src/inference/LiquidSampler.h"
#include "../src/Interval.h"
#include "../src/SpanInterval.h"

BOOST_AUTO_TEST_CASE( samplerTest )
{
    LiquidSampler sampler;
    std::vector<SpanInterval> sampled;

    srand(0);
    SpanInterval si(1,10,1,10);
    sampled = sampler(si, 0.99);
    BOOST_REQUIRE_EQUAL(sampled.size(), 1);
    BOOST_CHECK_EQUAL(sampled[0], si);

    sampled = sampler(si, 0.001);
    BOOST_CHECK(sampled.empty());

    sampled = sampler(si, 0.1);
    BOOST_REQUIRE_EQUAL(sampled.size(), 5);
    BOOST_CHECK_EQUAL(sampled[0], SpanInterval(2,3));
    BOOST_CHECK_EQUAL(sampled[1], SpanInterval(4,4));
    BOOST_CHECK_EQUAL(sampled[2], SpanInterval(5,5));
    BOOST_CHECK_EQUAL(sampled[3], SpanInterval(6,6));
    BOOST_CHECK_EQUAL(sampled[4], SpanInterval(7,8));
}

BOOST_AUTO_TEST_CASE( samplerTest2 )
{
    srand(120);

    LiquidSampler sampler;
    std::vector<SpanInterval> sampled;

    std::cout << "S1: ";
    sampled = sampler(SpanInterval(1,50,1,50), 1.0-exp(-1.0));
    std::copy(sampled.begin(), sampled.end(), std::ostream_iterator<SpanInterval>(std::cout, ", "));
    std::cout << std::endl;

    std::cout << "S2: ";
    sampled = sampler(SpanInterval(51,100,51,100), 1.0-exp(-1.0));
    std::copy(sampled.begin(), sampled.end(), std::ostream_iterator<SpanInterval>(std::cout, ", "));
    std::cout << std::endl;
}
