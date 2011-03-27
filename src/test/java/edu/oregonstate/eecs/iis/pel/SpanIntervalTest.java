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
