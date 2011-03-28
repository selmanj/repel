package edu.oregonstate.eecs.iis.pel.temporal;

import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

// TODO: totally forgot about Math.max and Math.min in some places, code could be cleaned up here
public class SpanInterval {
	private int startFrom, startTo;
	private boolean startFromInc, startToInc;
	private int endFrom, endTo;
	private boolean endFromInc, endToInc;
	private boolean startInc;
	private boolean endInc;
	
	public static final int POSITIVE_INF = Integer.MAX_VALUE;
	public static final int NEGATIVE_INF = Integer.MIN_VALUE;
	
	// TODO: doesn't account for negative numbers, needed?
	private static final Pattern SPAN_PATTERN = 
		Pattern.compile("([\\[\\(])\\s*([\\[\\(])\\s*(\\d+|-?inf)\\s*,\\s*(\\d+|-?inf)\\s*([\\]\\)])\\s*,\\s*([\\[\\(])\\s*(\\d+|-?inf)\\s*,\\s*(\\d+|-?inf)\\s*([\\]\\)])\\s*([\\]\\)])");
	
	public SpanInterval(Interval start, Interval end, boolean startInclusive, boolean endInclusive) {
		
		startFrom = start.getStart();
		startTo = start.getEnd();
		startFromInc = start.isStartInclusive();
		startToInc = start.isEndInclusive();
		
		endFrom = end.getStart();
		endTo = end.getEnd();
		endFromInc = end.isStartInclusive();
		endToInc = end.isEndInclusive();
		
		this.startInc = startInclusive;
		this.endInc = endInclusive;
	}
	
	public SpanInterval(int startFrom, int startTo, boolean startFromInc, boolean startToInc,
			int endFrom, int endTo, boolean endFromInc, boolean endToInc, 
			boolean startInclusive, boolean endInclusive) {
		this.startFrom = startFrom;
		this.startTo = startTo;
		this.startFromInc = startFromInc;
		this.startToInc = startToInc;
		
		this.endFrom = endFrom;
		this.endTo = endTo;
		this.endFromInc = endFromInc;
		this.endToInc = endToInc;
		
		this.startInc = startInclusive;
		this.endInc = endInclusive;
	}
	
	public SpanInterval(SpanInterval copyFrom) {
		this.startFrom = copyFrom.startFrom;
		this.startTo = copyFrom.startTo;
		this.startFromInc = copyFrom.startFromInc;
		this.startToInc = copyFrom.startToInc;
		
		this.endFrom = copyFrom.endFrom;
		this.endTo = copyFrom.endTo;
		this.endFromInc = copyFrom.endFromInc;
		this.endToInc = copyFrom.endToInc;
		
		this.startInc = copyFrom.startInc;
		this.endInc = copyFrom.endInc;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		
		sb.append(isStartInclusive() ? "[" : "(");
		sb.append(rangeAsString(startFrom, startTo, startFromInc, startToInc));
		sb.append(", ");
		sb.append(rangeAsString(endFrom, endTo, endFromInc, endToInc));
		sb.append(isEndInclusive() ? "]" : ")");
		
		return sb.toString();
	}
	
	private String rangeAsString(int start, int end, boolean startInclusive, boolean endInclusive) {
		StringBuilder sb = new StringBuilder();
		
		sb.append((startInclusive ? "[" : "("));
		switch (start) {
		case NEGATIVE_INF:
			sb.append("-");
		case POSITIVE_INF:
			sb.append("inf");
			break;
		default:
			sb.append(Integer.valueOf(start));
			break;
		}
		
		sb.append(", ");
		switch (end) {
		case NEGATIVE_INF:
			sb.append("-");
		case POSITIVE_INF:
			sb.append("inf");
			break;
		default:
			sb.append(Integer.valueOf(end));
			break;
		}
		sb.append((endInclusive ? "]" : ")"));
		
		return sb.toString();
	}
	
	public Set<SpanInterval> normalize() {
		Set<SpanInterval> result = new HashSet<SpanInterval>();
		
		SpanInterval copy = new SpanInterval(this);
		copy.setStartTo(getStartTo() < getEndTo() ? getStartTo() : getEndTo());	// j' = min(j,l))
		copy.setEndFrom(getEndFrom() > getStartFrom() ? getEndFrom() : getStartFrom()); // k' = max(k,i)
		if (isStartFromInclusive() && getStartFrom() == NEGATIVE_INF) {
			copy.setStartFromInclusive(false);
		}
		if (isEndToInclusive() && getEndTo() == POSITIVE_INF) {
			copy.setEndToInclusive(false);
		}
	    if (isStartToInclusive() 
	    		&& copy.getStartTo() != POSITIVE_INF
	    		&& (getStartTo() < getEndTo() || isEndToInclusive() && isStartInclusive() && isEndInclusive())) {
	    	copy.setStartToInclusive(true);
	    } else {
	    	copy.setStartToInclusive(false);
	    }
	    
	    if (isEndFromInclusive()
	    		&& copy.getEndFrom() != NEGATIVE_INF
	    		&& (getEndFrom() >  getStartFrom() || isStartFromInclusive() && isStartInclusive() && isEndInclusive())) {
	    	copy.setEndFromInclusive(true);
	    } else {
	    	copy.setEndFromInclusive(false);
	    }

		
	    if (copy.getStartFrom() > copy.getStartTo()) {	// this and the following test are self-explanatory
	    	return result;
	    }
	    if (copy.getEndFrom() > copy.getEndTo()) {
	    	return result;
	    }
	    
	    // if the start range is over exactly 1 time unit (instaneous), require it to be inclusive
	    if (copy.getStartFrom() == copy.getStartTo() && (!copy.isStartFromInclusive() || !copy.isStartToInclusive())) {
	    	return result;
	    }
	    // same with end range
	    if (copy.getEndFrom() == copy.getEndTo() && (!copy.isEndFromInclusive() || !copy.isEndToInclusive())) {
	    	return result;
	    }
	    // if startFrom is the same as endTo, require it to be inclusive
	    if (copy.getStartFrom() == copy.getEndTo() && (!copy.isStartInclusive() || !copy.isEndInclusive())) {
	    	return result;
	    }
	    // disqualify invalid infinities
	    if (copy.getStartFrom() == POSITIVE_INF || copy.getStartTo() == NEGATIVE_INF 
	    		|| copy.getEndFrom() == POSITIVE_INF || copy.getEndTo() == NEGATIVE_INF) {
	    	return result;
	    }
	    		
	    // we passed all the invalid tests, add it to the set
	    result.add(copy);
		return result;
	}
	
	public Set<SpanInterval> intersect(SpanInterval other) {
		Set<SpanInterval> result = new HashSet<SpanInterval>();
		
		if (isStartInclusive() != other.isStartInclusive()
				|| isEndInclusive() != other.isEndInclusive()) {
			return result;
		}
		int sf = Math.max(getStartFrom(), other.getStartFrom());
		int st = Math.min(getStartTo(), other.getStartTo());
		int ef = Math.max(getEndFrom(), other.getEndFrom());
		int et = Math.min(getEndTo(), other.getEndTo());
		
		boolean sfi;
		if (getStartFrom() > other.getStartFrom()) {
			sfi = isStartFromInclusive();
		} else if (getStartFrom() == other.getStartFrom()) {
			sfi = isStartFromInclusive() && other.isStartFromInclusive();
		} else {
			sfi = other.isStartFromInclusive();
		}
		
		boolean sei;
		//if ()
		
		return null;
	}
	
	public static SpanInterval parseSpanInterval(String s) throws SpanIntervalFormatException {
		Matcher matcher = SPAN_PATTERN.matcher(s);
		if (!matcher.matches()) {
			throw new SpanIntervalFormatException("unable to parse string as interval: "+ s);
		}
		boolean startInc = (matcher.group(1).equals("[") ? true : false);
		boolean startFromInc = (matcher.group(2).equals("[") ? true : false);
		int startFrom = parseSpanIntervalInt(matcher.group(3));
		int startTo = parseSpanIntervalInt(matcher.group(4));
		boolean startToInc = (matcher.group(5).equals("]") ? true : false);
		boolean endFromInc = (matcher.group(6).equals("[") ? true : false);
		int endFrom = parseSpanIntervalInt(matcher.group(7));
		int endTo = parseSpanIntervalInt(matcher.group(8));
		boolean endToInc = (matcher.group(9).equals("]") ? true : false);
		boolean endInc = (matcher.group(10).equals("]") ? true : false);

		return new SpanInterval(startFrom, startTo, startFromInc, startToInc, 
				endFrom, endTo, endFromInc, endToInc, 
				startInc, endInc);	
	}
	
	private static int parseSpanIntervalInt(String s) {
		if (s.equals("inf")) { return POSITIVE_INF; }
		if (s.equals("-inf")) { return NEGATIVE_INF; }
		return Integer.parseInt(s);
	}
	
	public int getStartFrom() {
		return startFrom;
	}

	public void setStartFrom(int startFrom) {
		this.startFrom = startFrom;
	}

	public int getStartTo() {
		return startTo;
	}

	public void setStartTo(int startTo) {
		this.startTo = startTo;
	}

	public boolean isStartFromInclusive() {
		return startFromInc;
	}

	public void setStartFromInclusive(boolean startFromInc) {
		this.startFromInc = startFromInc;
	}

	public boolean isStartToInclusive() {
		return startToInc;
	}

	public void setStartToInclusive(boolean startToInc) {
		this.startToInc = startToInc;
	}

	public int getEndFrom() {
		return endFrom;
	}

	public void setEndFrom(int endFrom) {
		this.endFrom = endFrom;
	}

	public int getEndTo() {
		return endTo;
	}

	public void setEndTo(int endTo) {
		this.endTo = endTo;
	}

	public boolean isEndFromInclusive() {
		return endFromInc;
	}

	public void setEndFromInclusive(boolean endFromInc) {
		this.endFromInc = endFromInc;
	}

	public boolean isEndToInclusive() {
		return endToInc;
	}

	public void setEndToInclusive(boolean endToInc) {
		this.endToInc = endToInc;
	}

	public boolean isStartInclusive() {
		return startInc;
	}

	public void setStartInclusive(boolean startInc) {
		this.startInc = startInc;
	}

	public boolean isEndInclusive() {
		return endInc;
	}

	public void setEndInclusive(boolean endInc) {
		this.endInc = endInc;
	}

	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + endFrom;
		result = prime * result + (endFromInc ? 1231 : 1237);
		result = prime * result + (endInc ? 1231 : 1237);
		result = prime * result + endTo;
		result = prime * result + (endToInc ? 1231 : 1237);
		result = prime * result + startFrom;
		result = prime * result + (startFromInc ? 1231 : 1237);
		result = prime * result + (startInc ? 1231 : 1237);
		result = prime * result + startTo;
		result = prime * result + (startToInc ? 1231 : 1237);
		return result;
	}

	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		SpanInterval other = (SpanInterval) obj;
		if (endFrom != other.endFrom)
			return false;
		if (endFromInc != other.endFromInc)
			return false;
		if (endInc != other.endInc)
			return false;
		if (endTo != other.endTo)
			return false;
		if (endToInc != other.endToInc)
			return false;
		if (startFrom != other.startFrom)
			return false;
		if (startFromInc != other.startFromInc)
			return false;
		if (startInc != other.startInc)
			return false;
		if (startTo != other.startTo)
			return false;
		if (startToInc != other.startToInc)
			return false;
		return true;
	}
}
