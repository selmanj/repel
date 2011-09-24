ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_PROGRAM_OPTIONS_LIB)
dist_noinst_SCRIPTS=autogen.sh
bin_PROGRAMS = pel pelnoise query_sampling
pel_SOURCES = src/pel.cpp src/fol/namegenerator.cpp src/fol/moves.cpp src/fol/model.cpp src/interval.cpp src/spaninterval.cpp src/fol/boollit.cpp src/fol/constant.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/variable.cpp src/siset.cpp src/fol/domain.cpp
pelnoise_SOURCES = src/pelnoise.cpp src/fol/model.cpp src/fol/domain.cpp src/fol/namegenerator.cpp src/interval.cpp src/spaninterval.cpp src/fol/boollit.cpp src/fol/constant.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/variable.cpp src/siset.cpp 
query_sampling_SOURCES = queries/query_sampling.cpp 

PEL_BUILT_TESTS = test/spanintervaltest test/follexertest test/folparsertest test/atomtest test/domaintest test/movestest test/cnftest test/modeltest
TESTS = $(PEL_BUILT_TESTS)
check_PROGRAMS = $(PEL_BUILT_TESTS)

test_spanintervaltest_SOURCES = test/spanintervaltest.cpp src/interval.cpp src/spaninterval.cpp src/siset.cpp
test_follexertest_SOURCES = test/follexertest.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/boollit.cpp
test_atomtest_SOURCES = test/atomtest.cpp src/fol/constant.cpp 
test_folparsertest_SOURCES = test/folparsertest.cpp src/fol/model.cpp src/fol/namegenerator.cpp src/fol/follexer.cpp src/fol/boollit.cpp src/fol/foltoken.cpp src/fol/constant.cpp src/fol/variable.cpp src/spaninterval.cpp src/siset.cpp src/interval.cpp
test_domaintest_SOURCES = test/domaintest.cpp src/fol/model.cpp src/fol/namegenerator.cpp src/fol/boollit.cpp src/interval.cpp src/siset.cpp src/spaninterval.cpp src/fol/constant.cpp src/fol/variable.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/domain.cpp
test_movestest_SOURCES = test/movestest.cpp src/fol/model.cpp src/fol/namegenerator.cpp src/fol/boollit.cpp src/interval.cpp src/siset.cpp src/spaninterval.cpp src/fol/constant.cpp src/fol/variable.cpp src/fol/follexer.cpp src/fol/foltoken.cpp src/fol/domain.cpp src/fol/moves.cpp
test_cnftest_SOURCES = test/cnftest.cpp src/fol/model.cpp src/fol/moves.cpp src/interval.cpp src/spaninterval.cpp src/fol/domain.cpp src/siset.cpp src/fol/boollit.cpp src/fol/variable.cpp src/fol/constant.cpp src/fol/foltoken.cpp src/fol/namegenerator.cpp src/fol/follexer.cpp
test_modeltest_SOURCES = test/modeltest.cpp