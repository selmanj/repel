/*
 * movestest.cpp
 *
 *  Created on: Jun 12, 2011
 *      Author: joe
 */

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
#include <boost/optional.hpp>
#include "../src/fol/fol.h"
#include "../src/fol/moves.h"

BOOST_AUTO_TEST_CASE(liquidMovesTest) {

}
