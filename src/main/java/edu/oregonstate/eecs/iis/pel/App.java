package edu.oregonstate.eecs.iis.pel;

import edu.oregonstate.eecs.iis.pel.temporal.Interval;
import edu.oregonstate.eecs.iis.pel.temporal.SpanInterval;

/**
 * Hello world!
 *
 */
public class App 
{
    public static void main( String[] args )
    {
    	
    	Interval start = new Interval(Integer.MIN_VALUE,5,true, false);
    	Interval end = new Interval(6,Integer.MAX_VALUE,false, false);
    	SpanInterval sp = new SpanInterval(start, end, true, false);
    	System.out.println("it = "+ sp.toString());
    }
}
