ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
dist_noinst_SCRIPTS=autogen.sh
bin_PROGRAMS = pel
pel_SOURCES = src/pel.cpp src/interval.cpp src/spaninterval.cpp


PEL_BUILT_TESTS = test/spanintervaltest test/follexertest
TESTS = $(PEL_BUILT_TESTS)
check_PROGRAMS = $(PEL_BUILT_TESTS)

test_spanintervaltest_SOURCES = test/spanintervaltest.cpp src/interval.cpp src/spaninterval.cpp
test_follexertest_SOURCES = test/follexertest.cpp src/fol/follexer.cpp src/fol/foltoken.cpp
