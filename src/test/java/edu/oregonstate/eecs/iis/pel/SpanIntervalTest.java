package edu.oregonstate.eecs.iis.pel;

import static org.junit.Assert.*;

import org.junit.Test;

import edu.oregonstate.eecs.iis.pel.temporal.SpanInterval;
import edu.oregonstate.eecs.iis.pel.temporal.SpanIntervalFormatException;

public class SpanIntervalTest {

	@Test
	public void testNormalize() {
		fail("Not yet implemented");
	}

	@Test
	public void testParseSpanInterval() {
		try{
			SpanInterval.parseSpanInterval("[(1,2), [3 ,4))");
		} catch(SpanIntervalFormatException e) {
			fail(e.toString());
		}
	}

}
