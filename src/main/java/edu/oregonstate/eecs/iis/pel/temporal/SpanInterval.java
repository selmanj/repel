package edu.oregonstate.eecs.iis.pel.temporal;

import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class SpanInterval {
	private int startFrom, startTo;
	private boolean startFromInc, startToInc;
	private int endFrom, endTo;
	private boolean endFromInc, endToInc;
	private boolean startInc;
	private boolean endInc;
	
	public static final int POSITIVE_INF = Integer.MAX_VALUE;
	public static final int NEGATIVE_INF = Integer.MIN_VALUE;
	
	private static final Pattern SPAN_PATTERN = Pattern.compile("([\\[\\(])");
	
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
		
	    int newStartTo = (getStartTo() < getEndTo() ? getStartTo() : getEndTo());	// j' = min(j,l)
	    if (getStartTo() <= newStartTo) {
	    	SpanInterval normalized = new SpanInterval(getStartFrom(),
	    			getStartTo(),
	    			isStartFromInclusive(),
	    			isStartToInclusive(),
	    			getEndFrom(),
	    			getEndTo(),
	    			isEndFromInclusive(),
	    			isEndToInclusive(),
	    			isStartInclusive(),
	    			isEndInclusive());
	    	result.add(normalized);
	    }
	    
		return result;
	}
	
	public static SpanInterval parseSpanInterval(String s) throws SpanIntervalFormatException {
		Matcher matcher = SPAN_PATTERN.matcher(s);
		if (matcher.matches()) {
			System.out.println("matches!");
			System.out.println("group 1 is "+ matcher.group(1));
		}
		return null;
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
