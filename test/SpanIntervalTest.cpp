#define BOOST_TEST_MODULE SpanInterval 
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>
#include <boost/assign/list_of.hpp>
#include "spaninterval.h"
#include "interval.h"
#include "siset.h"

#include <boost/foreach.hpp>
#include <iostream>
#include <list>
#include <iterator>
#include <algorithm>

using boost::assign::list_of;

BOOST_AUTO_TEST_CASE( basic_test )
{
    {
        SpanInterval sp1(1,11,1,11);
        SpanInterval sp2(5,10,5,10);
        Interval start = sp1.start();
        Interval end = sp1.finish();
        BOOST_CHECK_EQUAL(start.start(), 1);
        BOOST_CHECK_EQUAL(start.finish(), 11);
        BOOST_CHECK_EQUAL(end.start(), 1);
        BOOST_CHECK_EQUAL(end.finish(), 11);

        std::list<SpanInterval> removed;
        sp1.subtract(sp2,back_inserter(removed));

        BOOST_CHECK(removed.size() == 3);
        BOOST_CHECK(std::find(removed.begin(), removed.end(), SpanInterval(1,4,1,11)) != removed.end());
        BOOST_CHECK(std::find(removed.begin(), removed.end(), SpanInterval(11,11,11,11)) != removed.end());
        BOOST_CHECK(std::find(removed.begin(), removed.end(), SpanInterval(5,10,11,11)) != removed.end());
    }
    {
        SpanInterval sp1(1,11, 10,14);
        SpanInterval sp2(5,7, 5,9);
        std::list<SpanInterval> removed;
        sp1.subtract(sp2,back_inserter(removed));

        BOOST_CHECK(removed.size() == 1);
        BOOST_CHECK(std::find(removed.begin(), removed.end(), SpanInterval(1,11,10,14))!=removed.end());
    }

    {
        std::list<SpanInterval> shouldBeEmpty;
        SpanInterval sp(1,10,1,10);
        sp.subtract(sp, std::back_inserter(shouldBeEmpty));

        BOOST_CHECK_EQUAL(shouldBeEmpty.size(), 0);
    }

}

BOOST_AUTO_TEST_CASE( siset_test ) {
    SpanInterval sp1(1,10,1,10);
    SpanInterval sp2(8,11,8,11);
    SISet set(false, Interval(1,11));
    BOOST_CHECK(set.isDisjoint());
    set.add(sp1);
    BOOST_CHECK(set.isDisjoint());
    set.add(sp2);
    BOOST_CHECK(!set.isDisjoint());
    set.makeDisjoint();
    BOOST_CHECK(set.isDisjoint());


    // TODO turn this part into a test
    //std::cout << "compliment of " << set.toString() << " is " << set.compliment().toString() << std::endl;

    SISet dcompliment = set.compliment().compliment();
    dcompliment.makeDisjoint(); //
    //std::cout << "double compliment is " << dcompliment.toString() << std::endl;
}

BOOST_AUTO_TEST_CASE( hammingliq_dist_test) {
    Interval maxInterval(0,50);
    SISet a(true, maxInterval);
    SISet b(true, maxInterval);

    a.add(SpanInterval(1,5,1,5));
    b.add(SpanInterval(2,5,2,5));

    BOOST_CHECK_EQUAL(hammingDistance(a,b), 1);
}

BOOST_AUTO_TEST_CASE( hamming_dist_test) {
    Interval maxInterval(0,50);
    SISet a(false, maxInterval);
    SISet b(false, maxInterval);

    a.add(SpanInterval(1,5,1,5));
    b.add(SpanInterval(2,5,2,5));

    BOOST_CHECK_EQUAL(hammingDistance(a,b), 5);
}

BOOST_AUTO_TEST_CASE( rand_siset_test ) {
    Interval maxInterval(0,100);
    srand(0);
    SISet set = SISet::randomSISet(true, maxInterval);
    BOOST_WARN_EQUAL(set.toString(), "{[0:2], [5:6], [8:8], [10:10], [14:14], [16:17], [21:24], [28:28], [30:30], [33:33], [35:36], [41:42], [45:46], [49:51], [56:56], [59:60], [64:64], [66:66], [70:72], [74:82], [85:87], [89:89], [91:91], [93:93], [96:96]}");
}

BOOST_AUTO_TEST_CASE( sisetliq_test ) {
    SpanInterval sp1(1,10,1,10);
    SpanInterval sp2(6,11,6,11);
    SISet set(true, Interval(1,11));

    set.add(sp1);
    set.add(sp2);

    BOOST_CHECK(set.isDisjoint());
}

BOOST_AUTO_TEST_CASE( spanInterval_relations ) {
    Interval maxInterval(0, 1000);
    SpanInterval universe(maxInterval);
    SpanInterval sp1(1,5,6,10);
    boost::optional<SpanInterval> sp2 = sp1.satisfiesRelation(Interval::MEETS, universe);
    BOOST_CHECK(sp2);
    SpanInterval si = sp2.get();
    BOOST_CHECK_EQUAL(si.toString(), "[(7, 11), (7, 1000)]");
    sp1 = SpanInterval(999, 1000, 1000, 1000);
    sp2 = sp1.satisfiesRelation(Interval::MEETS, universe);
    BOOST_CHECK(!sp2);

    sp1 = SpanInterval(1,4,7,9);
    sp2 = sp1.satisfiesRelation(Interval::OVERLAPS, universe);
    BOOST_CHECK(sp2);
    si = sp2.get();
    BOOST_CHECK_EQUAL(si.toString(), "[(2, 9), (8, 1000)]");

    sp2 = sp1.satisfiesRelation(Interval::OVERLAPSI, universe);
    BOOST_CHECK(sp2);
    si = sp2.get();
    BOOST_CHECK_EQUAL(si.toString(), "[(0, 3), (1, 8)]");

    sp2 = sp1.satisfiesRelation(Interval::STARTS, universe);
    BOOST_CHECK(sp2);
    si = sp2.get();
    BOOST_CHECK_EQUAL(si.toString(), "[(1, 4), (8, 1000)]");

    sp2 = sp1.satisfiesRelation(Interval::STARTSI, universe);
    BOOST_CHECK(sp2);
    si = sp2.get();
    BOOST_CHECK_EQUAL(si.toString(), "[(1, 4), (1, 8)]");
}

BOOST_AUTO_TEST_CASE( spanIntervalspan ) {
    SpanInterval sp1(1,5,6,10);
    SpanInterval sp2(3,7,9,11);

    SISet set = span(sp1, sp2, Interval(1,11));
    /*
    BOOST_FOREACH(SpanInterval sp, set.set()) {
        std::cout << "si = " << sp.toString() << std::endl;
    }
    set.makeDisjoint();

    BOOST_FOREACH(SpanInterval sp, set.set()) {
        std::cout << "si = " << sp.toString() << std::endl;
    }
    */
}

BOOST_AUTO_TEST_CASE( spanIntervalSize ) {
    SpanInterval sp1(5,10,5,10);
    SpanInterval sp2(1,5,3,6);
    SpanInterval sp3(5,7,1,2);

    BOOST_CHECK_EQUAL(sp1.size(), 21);
    BOOST_CHECK_EQUAL(sp2.size(), 17);
    BOOST_CHECK_EQUAL(sp3.size(), 0);
}

BOOST_AUTO_TEST_CASE ( subtractTest) {
    SpanInterval sp1(1,10,1,10);
    SpanInterval sp2(5,5,5,5);

    std::vector<SpanInterval> rVec;
    sp1.subtract(sp2, back_inserter(rVec));

    BOOST_REQUIRE_EQUAL(rVec.size(),3);
    BOOST_CHECK_EQUAL(rVec[0].toString(), "[(1, 4), (1, 10)]");
    BOOST_CHECK_EQUAL(rVec[1].toString(), "[(5, 5), (6, 10)]");
    BOOST_CHECK_EQUAL(rVec[2].toString(), "[6:10]");

}

BOOST_AUTO_TEST_CASE ( siSetSubtractTest) {
    SpanInterval sp1(1,10,1,10);
    SpanInterval sp2(11,15,13,15);
    SpanInterval sp3(5,5,5,5);
    SpanInterval sp4(11,15,13,14);

    SISet set1(false, Interval(1,15));
    SISet set2(false, Interval(1,15));

    set1.add(sp1);
    set1.add(sp2);
    set2.add(sp3);
    set2.add(sp4);

    set1.subtract(set2);

    BOOST_CHECK_EQUAL(set1.toString(), "{[(1, 4), (1, 10)], [(5, 5), (6, 10)], [6:10], [(11, 14), (15, 15)], [15:15]}");
}

BOOST_AUTO_TEST_CASE( siSetComplimentTest) {
    // [(1, 9), (1, 20)], [(1, 18), (20, 20)], [(10, 10), (10, 19)], [11:20]
    Interval maxInt = Interval(1,20);
    SpanInterval sp1(1,9,1,20);
    SpanInterval sp2(1,18,20,20);
    SpanInterval sp3(10,10,10,19);
    SpanInterval sp4(11,20,11,20);
    SISet set(false, maxInt);

    set.add(sp1);
    set.add(sp2);
    set.add(sp3);
    set.add(sp4);
    set.makeDisjoint();
    BOOST_CHECK_EQUAL(set.toString(), "{[(1, 9), (1, 20)], [(10, 10), (10, 19)], [(10, 18), (20, 20)], [(11, 18), (11, 19)], [19:20]}");
    SISet compliment = set.compliment();
    BOOST_CHECK_EQUAL(compliment.toString(), "{}");
}

BOOST_AUTO_TEST_CASE ( siIteratorTest) {
    SpanInterval sp(1,10,1,10);
    std::vector<Interval> intervals;

    std::copy(sp.begin(), sp.end(), std::inserter(intervals, intervals.begin()));
    BOOST_CHECK_EQUAL(sp.size(), intervals.size());

    // make sure that every interval is in there
    for (int start = 1; start <= 10; start++) {
        for (int finish = start; finish <= 10; finish++) {
            Interval interval(start, finish);
            BOOST_CHECK(std::find(intervals.begin(), intervals.end(), interval) != intervals.end());
        }
    }
}

BOOST_AUTO_TEST_CASE (siSetOutputIteratorTest ) {
    std::list<SpanInterval> list = list_of(SpanInterval(0,1,0,1))(SpanInterval(3,6,3,6))(SpanInterval(9,10,9,10))(SpanInterval(11,11,11,11));
    SISet siset(false, Interval(0,15));
    std::copy(list.begin(), list.end(), SISetInserter(siset));
    BOOST_CHECK_EQUAL(siset.toString(), "{[0:1], [3:6], [9:10], [11:11]}");
}

BOOST_AUTO_TEST_CASE( siSetEndpointTest) {
    {
        SISet set(true, Interval(0,10));
        std::vector<SpanInterval> segments;
        std::vector<SpanInterval> expected = list_of(SpanInterval(0,10,0,10));
        set.collectSegments(std::back_inserter(segments));

        BOOST_CHECK_EQUAL_COLLECTIONS(segments.begin(), segments.end(), expected.begin(), expected.end());
    }
    {
        SISet set(true, Interval(0,10));
        set.add(SpanInterval(2,4,2,4));
        set.add(SpanInterval(7,7,7,7));
        std::set<SpanInterval> segments;
        std::set<SpanInterval> expected = list_of(SpanInterval(0,1,0,1))(SpanInterval(2,4,2,4))(SpanInterval(5,6,5,6))(SpanInterval(7,7,7,7))(SpanInterval(8,10,8,10));
        set.collectSegments(std::inserter(segments, segments.end()));

        BOOST_CHECK_EQUAL_COLLECTIONS(segments.begin(), segments.end(), expected.begin(), expected.end());
    }
}

BOOST_AUTO_TEST_CASE( intervalSubtract ) {
    Interval a(1,5);
    Interval b(6,10);
    Interval c(4,8);
    Interval d(3,4);

    BOOST_REQUIRE_EQUAL(a.subtract(b).size(), 1);
    BOOST_CHECK_EQUAL(a.subtract(b)[0], Interval(1,5));
    BOOST_REQUIRE_EQUAL(b.subtract(a).size(), 1);
    BOOST_CHECK_EQUAL(b.subtract(a)[0], Interval(6,10));

    BOOST_REQUIRE_EQUAL(a.subtract(c).size(), 1);
    BOOST_CHECK_EQUAL(a.subtract(c)[0], Interval(1,3));
    BOOST_REQUIRE_EQUAL(b.subtract(c).size(), 1);
    BOOST_CHECK_EQUAL(b.subtract(c)[0], Interval(9,10));

    BOOST_REQUIRE(d.subtract(a).empty());
    BOOST_REQUIRE_EQUAL(a.subtract(d).size(), 2);
    BOOST_CHECK_EQUAL(a.subtract(d)[0], Interval(1,2));
    BOOST_CHECK_EQUAL(a.subtract(d)[1], Interval(5,5));
}
