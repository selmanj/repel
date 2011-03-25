package edu.oregonstate.eecs.iis.pel.temporal;

public class Interval {
	
	private int start, end;
	private boolean startInclusive, endInclusive;
	
	public Interval(int start, int end, boolean startInclusive, boolean endInclusive) {
		this.start = start;
		this.end = end;
		this.startInclusive = startInclusive;
		this.endInclusive = endInclusive;
	}
	
	public String toString() {
		return (startInclusive ? "[" : "(") + start + ", "+ end + (endInclusive ? "]" : ")");
	}
	
	public int getStart() { return start; }
	public int getEnd() { return end; }
	public boolean isStartInclusive() { return startInclusive; }
	public boolean isEndInclusive() { return endInclusive; }
	
	public void setStart(int start) {this.start = start;}
	public void setEnd(int end) {this.end = end;}
	public void setIsStartInclusive(boolean startInclusive) {this.startInclusive = startInclusive;}
	public void setIsEndInclusive(boolean endInclusive) {this.endInclusive = endInclusive;}
	
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + end;
		result = prime * result + (endInclusive ? 1231 : 1237);
		result = prime * result + start;
		result = prime * result + (startInclusive ? 1231 : 1237);
		return result;
	}

	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		Interval other = (Interval) obj;
		if (end != other.end)
			return false;
		if (endInclusive != other.endInclusive)
			return false;
		if (start != other.start)
			return false;
		if (startInclusive != other.startInclusive)
			return false;
		return true;
	}
	
}
