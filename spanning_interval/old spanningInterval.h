#ifndef SPANNING_INTERVAL
#define SPANNING_INTERVAL

#include "linkedList.h"

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#define intervalSet lkList

#define EQUAL(I1,I2) ( (I1)->t1 == (I2)->t1 && (I1)->t2 == (I2)->t2  )
#define STRICTLY_AFTER(I1,I2) ( (I1)->t1 > (I2)->t2 )
#define STRICTLY_BEFORE(I1,I2) ( (I1)->t2 < (I2)->t1 )
#define STRICTLY_AFTER(I1,I2) ( (I1)->t1 > (I2)->t2 )
#define BEFORE(I1,I2) ( (I1)->t2 <= (I2)->t1 )
#define AFTER(I1,I2) ( (I1)->t1 >= (I2)->t2 )
#define MEET_AFTER(I1,I2) ( (I1)->t1 == (I2)->t2+1 )
#define MEET_BEFORE(I1,I2) ( (I1)->t2+1 == (I2)->t1 )
#define MEET(I1,I2) ( MEET_AFTER(I1,I2) || MEET_BEFORE(I1,I2) )
#define STRICTLY_INSIDE(I1,I2) ( (I1)->t1 > (I2)->t1 && (I1)->t1 < (I2)->t2 && (I1)->t2 > (I2)->t1 && (I1)->t2 < (I2)->t2 )
#define INSIDE(I1,I2) ( (I1)->t1 >= (I2)->t1 && (I1)->t1 <= (I2)->t2 && (I1)->t2 >= (I2)->t1 && (I1)->t2 <= (I2)->t2 )
#define OVERLAP(I1,I2) ( ((I1)->t1 >= (I2)->t1 && (I1)->t1 <= (I2)->t2) || ((I1)->t2 >= (I2)->t1 && (I1)->t2 <= (I2)->t2) || INSIDE(I2,I1) )

typedef
struct interval
{
	struct interval* prev,*next;
	int t1,t2;
}
INTERVAL;

typedef
struct spanningInterval
{
	intervalSet I1,I2;
}
SPAN_INTERVAL;

int iNormalizeInterval(INTERVAL* i);
int iIntervalRange(INTERVAL *i);
int iGetNSubIntervals(INTERVAL *I1, INTERVAL *I2);
INTERVAL* iCreateInterval(int t1, int t2);

SPAN_INTERVAL* siInitSpanningInterval(SPAN_INTERVAL* si);
SPAN_INTERVAL* siCreateSpanningInterval();
void siFreeSpanningIntervalMem(SPAN_INTERVAL* si);
void siReleaseSpanningInterval(SPAN_INTERVAL** si);
int siGetNSubIntervals(SPAN_INTERVAL *si);
SPAN_INTERVAL * siIntersection(SPAN_INTERVAL *si1,SPAN_INTERVAL *si2,SPAN_INTERVAL *si);
SPAN_INTERVAL * siUnion(SPAN_INTERVAL *si1,SPAN_INTERVAL *si2,SPAN_INTERVAL *si);
SPAN_INTERVAL * siComplement(SPAN_INTERVAL *si1, INTERVAL* timeline,SPAN_INTERVAL *si);


#endif