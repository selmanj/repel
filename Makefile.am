ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_PROGRAM_OPTIONS_LIB)
dist_noinst_SCRIPTS=autogen.sh
bin_PROGRAMS = pel
pel_SOURCES = src/pel.cpp src/interval.cpp src/spaninterval.cpp src/fol/boollit.cpp src/fol/constant.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/variable.cpp src/siset.cpp src/fol/domain.cpp


PEL_BUILT_TESTS = test/spanintervaltest test/follexertest test/folparsertest test/atomtest test/domaintest test/movestest
TESTS = $(PEL_BUILT_TESTS)
check_PROGRAMS = $(PEL_BUILT_TESTS)

test_spanintervaltest_SOURCES = test/spanintervaltest.cpp src/interval.cpp src/spaninterval.cpp src/siset.cpp
test_follexertest_SOURCES = test/follexertest.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/boollit.cpp
test_atomtest_SOURCES = test/atomtest.cpp src/fol/constant.cpp 
test_folparsertest_SOURCES = test/folparsertest.cpp src/fol/follexer.cpp src/fol/boollit.cpp src/fol/foltoken.cpp src/fol/constant.cpp src/fol/variable.cpp src/spaninterval.cpp src/siset.cpp src/interval.cpp
test_domaintest_SOURCES = test/domaintest.cpp src/fol/boollit.cpp src/interval.cpp src/siset.cpp src/spaninterval.cpp src/fol/constant.cpp src/fol/variable.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/domain.cpp
test_movestest_SOURCES = test/domaintest.cpp src/fol/boollit.cpp src/interval.cpp src/siset.cpp src/spaninterval.cpp src/fol/constant.cpp src/fol/variable.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/domain.cpp src/fol/moves.cpp