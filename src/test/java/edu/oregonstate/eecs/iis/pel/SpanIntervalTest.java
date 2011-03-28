package edu.oregonstate.eecs.iis.pel;

import static org.junit.Assert.*;

import org.junit.Test;

import edu.oregonstate.eecs.iis.pel.temporal.SpanInterval;
import edu.oregonstate.eecs.iis.pel.temporal.SpanIntervalFormatException;

public class SpanIntervalTest {

	@Test
	public void testNormalize() {
		try {
			SpanInterval i;
			// ensure these infinites are empty
			assertTrue(SpanInterval.parseSpanInterval("[(inf, 5), (9,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, -inf), (9,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 5), (inf,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 5), (9,-inf])").normalize().isEmpty());
			// these should be fine (although they might get normalized
			assertTrue(!SpanInterval.parseSpanInterval("[(-inf, 5), (9,10])").normalize().isEmpty());
			assertTrue(!SpanInterval.parseSpanInterval("[(1, inf), (9,10])").normalize().isEmpty());
			assertTrue(!SpanInterval.parseSpanInterval("[(1, 5), (-inf,10])").normalize().isEmpty());
			assertTrue(!SpanInterval.parseSpanInterval("[(1, 5), (9,inf])").normalize().isEmpty());
			// make sure the endpoints are non inclusive for infinity
			assertEquals(SpanInterval.parseSpanInterval("[(-inf, 5), (9, 10])").toString(),
					SpanInterval.parseSpanInterval("[[-inf, 5), (9,10])").normalize().iterator().next().toString());
			assertEquals(SpanInterval.parseSpanInterval("[(1, inf), (9, inf))").toString(),
					SpanInterval.parseSpanInterval("[(1, inf], (9,inf])").normalize().iterator().next().toString());
			// TODO: could add more tests for this case
			// make sure that if j > l, j reduced to l
			assertEquals(SpanInterval.parseSpanInterval("[(1, 10), (9, 10])").toString(),
					SpanInterval.parseSpanInterval("[(1, 100), (9,10])").normalize().iterator().next().toString());
			assertEquals(SpanInterval.parseSpanInterval("[(1, 5), (1, 10])").toString(),
					SpanInterval.parseSpanInterval("[(1, 5), (0,10])").normalize().iterator().next().toString());
			// more empty intervals
			assertTrue(SpanInterval.parseSpanInterval("[(9, 5), (9,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 5), (15,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 1), (9,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[[1, 1), (9,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 1], (9,10])").normalize().isEmpty());
			assertTrue(!SpanInterval.parseSpanInterval("[[1, 1], (9,10])").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 5), (10,10))").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 5), [10,10))").normalize().isEmpty());
			assertTrue(SpanInterval.parseSpanInterval("[(1, 5), (10,10])").normalize().isEmpty());
			assertTrue(!SpanInterval.parseSpanInterval("[(1, 5), [10,10])").normalize().isEmpty());
			// TODO: more cases to test!!!



			i = SpanInterval.parseSpanInterval("[(5,10), (2,3)]");
			assertTrue(i.normalize().isEmpty());
			i = SpanInterval.parseSpanInterval("[(2,3), (5,10)]");
			assertTrue(!i.normalize().isEmpty());
		} catch (SpanIntervalFormatException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testParseSpanInterval() {
		try{
			assertEquals("[(1, 2), [3, 4))", 
					SpanInterval.parseSpanInterval("[(1,2), [3 ,4))").toString());
			assertEquals("[(inf, 224), [-inf, 411]]", 
					SpanInterval.parseSpanInterval("[(  inf,224) , [-inf  ,411]]").toString());
			// TODO: add more tests (boring i know)

		} catch(SpanIntervalFormatException e) {
			fail(e.toString());
		}
	}

}
