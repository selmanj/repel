#ifndef __SPANNING_INTERVAL_H__
#define __SPANNING_INTERVAL_H__

#include "linkedList.h"

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

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
#define IS_LIQUID(si) ( (si)->I1.t1 == (si)->I2.t1 && (si)->I1.t2 == (si)->I2.t2 )

typedef
struct interval
{
	int t1,t2;
}
INTERVAL;

typedef
struct spanningInterval
{
	struct spanningInterval* prev,*next;
	INTERVAL I1,I2;
}
SPAN_INTERVAL;

#define SPAN_INTERVAL_SET lkList
#define sisCreate lklCreateList
#define sisInit lklInitList

#define sisLiquidIntersection sisGeneralIntersection
#define sisLiquidDiamondm sisGeneralDiamondm
#define sisLiquidDiamondmi sisGeneralDiamondmi
#define sisLiquidDiamondf sisGeneralDiamondf
#define sisLiquidDiamondfi sisGeneralDiamondfi

inline void sisRelease(SPAN_INTERVAL_SET** s)
{
	lklReleaseList(s,0);
}

inline void sisClear(SPAN_INTERVAL_SET* s)
{
	lklClearList(s,0);
}

inline SPAN_INTERVAL * sisAdd(SPAN_INTERVAL_SET* s, SPAN_INTERVAL *si)
{
	return (SPAN_INTERVAL *)lklPushBack(s,si);
}

int iNormalizeInterval(INTERVAL* i);
int iIntervalRange(INTERVAL* i);
int iGetNSubIntervals(INTERVAL *I1, INTERVAL *I2);

SPAN_INTERVAL * siCreate(INTERVAL* I1, INTERVAL* I2);
SPAN_INTERVAL * siClone(SPAN_INTERVAL * si);
SPAN_INTERVAL * siLiquify(SPAN_INTERVAL * si);

SPAN_INTERVAL_SET * sisCopy(SPAN_INTERVAL_SET * dst, SPAN_INTERVAL_SET * src);
SPAN_INTERVAL_SET * sisClearAndCopy(SPAN_INTERVAL_SET * dst, SPAN_INTERVAL_SET * src);
SPAN_INTERVAL * siIntersection(SPAN_INTERVAL *si1,SPAN_INTERVAL *si2,SPAN_INTERVAL *si);
inline int siGetNSubIntervals(SPAN_INTERVAL *si)
{
	return iGetNSubIntervals(&si->I1, &si->I2);
}

SPAN_INTERVAL_SET * sisClone(SPAN_INTERVAL_SET * s);
SPAN_INTERVAL_SET* sisGeneralUnion(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2);
SPAN_INTERVAL_SET* sisGeneralIntersection(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2);
SPAN_INTERVAL_SET* sisGeneralComplement(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisGeneralDiamondm(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisGeneralDiamondmi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisGeneralDiamondf(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisGeneralDiamondfi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);

unsigned char sisIsLiquid(SPAN_INTERVAL_SET* s);
SPAN_INTERVAL_SET* sisLiquify(SPAN_INTERVAL_SET* s1);
SPAN_INTERVAL_SET* sisLiquidUnion(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2);
SPAN_INTERVAL_SET* sisLiquidComplement(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisLiquidDiamondm(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisLiquidDiamondmi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisLiquidDiamondf(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);
SPAN_INTERVAL_SET* sisLiquidDiamondfi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline);

SPAN_INTERVAL_SET* sisImply(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2, INTERVAL* timeline, unsigned char isLiquid);
SPAN_INTERVAL_SET* sisXUnion(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2, INTERVAL* timeline, unsigned char isLiquid);

inline SPAN_INTERVAL_SET* sisUnion(SPAN_INTERVAL_SET* s1, SPAN_INTERVAL_SET* s2,unsigned char isLiquid)
{
	return isLiquid?sisLiquidUnion(s1,s2):sisGeneralUnion(s1,s2);
}

inline SPAN_INTERVAL_SET* sisIntersection(SPAN_INTERVAL_SET* s1, SPAN_INTERVAL_SET* s2,unsigned char isLiquid)
{
	return isLiquid?sisLiquidIntersection(s1,s2):sisGeneralIntersection(s1,s2);
}

inline SPAN_INTERVAL_SET* sisComplement(SPAN_INTERVAL_SET* s1, INTERVAL* timeline,unsigned char isLiquid)
{
	return isLiquid?sisLiquidComplement(s1,timeline):sisGeneralComplement(s1,timeline);
}

inline SPAN_INTERVAL_SET* sisDiamondm(SPAN_INTERVAL_SET* s1, INTERVAL* timeline,unsigned char isLiquid)
{
	return isLiquid?sisLiquidDiamondm(s1,timeline):sisGeneralDiamondm(s1,timeline);
}

inline SPAN_INTERVAL_SET* sisDiamondmi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline,unsigned char isLiquid)
{
	return isLiquid?sisLiquidDiamondmi(s1,timeline):sisGeneralDiamondmi(s1,timeline);
}

inline SPAN_INTERVAL_SET* sisDiamondf(SPAN_INTERVAL_SET* s1, INTERVAL* timeline,unsigned char isLiquid)
{
	return isLiquid?sisLiquidDiamondf(s1,timeline):sisGeneralDiamondf(s1,timeline);
}

inline SPAN_INTERVAL_SET* sisDiamondfi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline,unsigned char isLiquid)
{
	return isLiquid?sisLiquidDiamondfi(s1,timeline):sisGeneralDiamondfi(s1,timeline);
}

int sisGetNSubIntervals(SPAN_INTERVAL_SET *s);
SPAN_INTERVAL_SET* sisLiquidMerge(SPAN_INTERVAL_SET* s1);
void sisMergingIntervals(SPAN_INTERVAL_SET* s, unsigned char isLiquid);
void sisNormalize(SPAN_INTERVAL_SET* s, unsigned char isLiquid);

int sisLiquidRange(SPAN_INTERVAL_SET* s);
double sisCompare(SPAN_INTERVAL_SET* s1, SPAN_INTERVAL_SET* s2, INTERVAL* timeline, unsigned char liquid);

void printInterval(INTERVAL *I);
void printSpanInterval(SPAN_INTERVAL *si);
void printSpanIntervalSet(SPAN_INTERVAL_SET *s);

#endif