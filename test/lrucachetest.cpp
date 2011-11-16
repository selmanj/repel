#define BOOST_TEST_MODULE LRUCache
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
#include <string>
#include "lrucache.h"

BOOST_AUTO_TEST_CASE( lrucache_basic_test)
{
	typedef std::pair<std::string, int> StringIntPair;
	LRUCache<std::string, int> cache(5);

	BOOST_CHECK_EQUAL(cache.count("test"),0);
	cache.insert(StringIntPair("test", 1));
	BOOST_CHECK_EQUAL(cache.count("test"),1);
	cache.insert(StringIntPair("test2", 2));
	cache.insert(StringIntPair("test3", 3));
	cache.insert(StringIntPair("test4", 4));
	cache.insert(StringIntPair("test5", 5));
	// now use them all except for test
	BOOST_CHECK_EQUAL(cache.get("test2"), 2);
	BOOST_CHECK_EQUAL(cache.get("test3"), 3);
	BOOST_CHECK_EQUAL(cache.get("test4"), 4);
	BOOST_CHECK_EQUAL(cache.get("test5"), 5);
	// add test 6
	cache.insert(StringIntPair("test6", 6));
	BOOST_CHECK_EQUAL(cache.size(), 5);
	BOOST_CHECK_EQUAL(cache.count("test"), 0);


}
