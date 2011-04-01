#define BOOST_TEST_MODULE SpanInterval test
#include <boost/test/included/unit_test.hpp>
#include "spaninterval.h"
#include "interval.h"

BOOST_AUTO_TEST_CASE( constructors_test )
{
  SpanInterval sp(0,0,0,0);
  Interval start = sp.start();
  Interval end = sp.end();
  BOOST_CHECK_EQUAL(start.start(), 0);
  BOOST_CHECK_EQUAL(start.end(), 0);
  BOOST_CHECK_EQUAL(end.start(), 0);
  BOOST_CHECK_EQUAL(end.end(), 0);
}
