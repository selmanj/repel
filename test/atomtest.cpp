#define BOOST_TEST_MODULE Atom
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include "logic/el_syntax.h"
#include <iostream>
#include <string>

BOOST_AUTO_TEST_CASE( atom )
{
	/*
  boost::shared_ptr<Term> c1(new Constant("a"));
  boost::shared_ptr<Term> c2(new Constant("b"));
  std::vector<boost::shared_ptr<Term> > vec;
	 */
	std::vector<Constant> vec;
	vec.push_back(Constant("a"));
	vec.push_back(Constant("b"));

	Atom x(std::string("blarg"), vec.begin(), vec.end());
}

