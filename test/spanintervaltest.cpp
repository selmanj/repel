#define BOOST_TEST_MODULE SpanInterval 
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#ifdef HAVE_BOOST_UNIT_TEST_FRAMEWORK
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif
#include "../src/spaninterval.h"
#include "../src/interval.h"
#include "../src/siset.h"

#include <boost/foreach.hpp>
#include <iostream>
#include <set>

BOOST_AUTO_TEST_CASE( basic_test )
{
	{
		SpanInterval sp1(1,11,1,11);
		SpanInterval sp2(5,10,5,10);
		Interval start = sp1.start();
		Interval end = sp1.end();
		BOOST_CHECK_EQUAL(start.start(), 1);
		BOOST_CHECK_EQUAL(start.end(), 11);
		BOOST_CHECK_EQUAL(end.start(), 1);
		BOOST_CHECK_EQUAL(end.end(), 11);

		std::set<SpanInterval> removed;
		sp1.subtract(sp2,removed);

		BOOST_CHECK(removed.size() == 3);
		BOOST_CHECK(removed.find(SpanInterval(1,4,1,11)) != removed.end());
		BOOST_CHECK(removed.find(SpanInterval(11,11)) != removed.end());
		BOOST_CHECK(removed.find(SpanInterval(5,10,11,11)) != removed.end());
	}
	{
		SpanInterval sp1(1,11, 10,14);
		SpanInterval sp2(5,7, 5,9);
		std::set<SpanInterval> removed;
		sp1.subtract(sp2,removed);

		BOOST_CHECK(removed.size() == 3);
		BOOST_CHECK(removed.find(SpanInterval(1,4,10,14))!=removed.end());
		BOOST_CHECK(removed.find(SpanInterval(5,7,10,14))!=removed.end());
		BOOST_CHECK(removed.find(SpanInterval(8,11,10,14))!=removed.end());


	}

}

BOOST_AUTO_TEST_CASE( siset_test ) {
	SpanInterval sp1(0,10,0,10);
	SpanInterval sp2(8,11,8,11);
	SISet set;
	BOOST_CHECK(set.isDisjoint());
	set.add(sp1);
	BOOST_CHECK(set.isDisjoint());
	set.add(sp2);
	BOOST_CHECK(!set.isDisjoint());
}
