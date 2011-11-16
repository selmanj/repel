/*
 * si_histogramtest.cpp
 *
 *  Created on: Sep 27, 2011
 *      Author: joe
 */

#define BOOST_TEST_MODULE SIHistogram
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>
#include "si_histogram.h"

BOOST_AUTO_TEST_CASE( si_histogram )
{
	Interval maxInterval(0,100);
	SIHistogram hist(false, maxInterval);
	hist.add(SpanInterval(0,2,3,4,maxInterval));
	hist.add(SpanInterval(0,2,3,4,maxInterval));
	hist.add(SpanInterval(2,3,3,5,maxInterval));
	//std::cout << "histogram = " << hist.toString() << std::endl;
	BOOST_CHECK_EQUAL(hist.toString(), "{\n"
			"[(0, 1), (3, 4)] = 2\n"
			"[(2, 2), (3, 4)] = 3\n"
			"[(2, 2), (5, 5)] = 1\n"
			"[(3, 3), (3, 5)] = 1\n"
			"}\n");
}

BOOST_AUTO_TEST_CASE( si_histogram_liquid )
{
	Interval maxInterval(0,100);
	SIHistogram hist(true, maxInterval);
	hist.add(SpanInterval(0,2,0,2,maxInterval));
	hist.add(SpanInterval(0,2,0,2,maxInterval));
	hist.add(SpanInterval(2,3,2,3,maxInterval));
	//std::cout << "liqhistogram = " << hist.toString() << std::endl;
	BOOST_CHECK_EQUAL(hist.toString(), "{\n"
			"[0:1] = 2\n"
			"[2:2] = 3\n"
			"[3:3] = 1\n"
			"}\n");
}

