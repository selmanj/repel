#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

#include "spanningInterval.h"

int iNormalizeInterval(INTERVAL* i)
{
	int t;
	if(i->t1>i->t2)
	{
		t = i->t1;
		i->t1 = i->t2;
		i->t2 = t;
		return 1;
	}
	return 0;
}

int iIntervalRange(INTERVAL *i)
{
	return i->t2-i->t1+1;
}
int iGetNSubIntervals(INTERVAL *I1, INTERVAL *I2)
{
	int n, n2;
	INTERVAL I,II;

	iNormalizeInterval(I1);
	iNormalizeInterval(I2);

	/*
		I1	      ---
		I2	 ---
	*/
	if(STRICTLY_AFTER(I1,I2))
	{
		return 0;
	}
	
	/*
		I1	 ---
		I2	     ---
	*/
	if(STRICTLY_BEFORE(I1,I2))
	{
		return iIntervalRange(I1) * iIntervalRange(I2);
	}

	/*
		I1	 ---
		I2	 ---
	*/
	if(EQUAL(I1,I2))
	{
		n = iIntervalRange(I1);
		return n*(n+1)/2;
	}
	
	/*
		I1	-------------
		I2	     ---
	*/
	if(INSIDE(I2,I1))
	{
		n = iIntervalRange(I2);
		I.t1 = I1->t1;
		I.t2 = I2->t1-1;
		return iIntervalRange(&I) * n + n*(n+1)/2;
	}

	/*
		I1	     ---
		I2	-------------
	*/
	if(INSIDE(I1,I2))
	{
		n = iIntervalRange(I1);
		I.t1 = I1->t2+1;
		I.t2 = I2->t2;
		return n*(n+1)/2 + iIntervalRange(&I) * n;
	}

	/*
		I1	------
		I2	    ------
	*/
	if(BEFORE(I1,I2))
	{
		I.t1 = I1->t1;
		I.t2 = I1->t1-1;
		n = iIntervalRange(&I) * iIntervalRange(I2);
		I.t1 = I2->t1;
		I.t2 = I1->t2;
		n2 = iIntervalRange(&I);
		II.t1 = I1->t2+1;
		II.t2 = I2->t2;
		return n + n2*(n2+1)/2 + iIntervalRange(&I) * iIntervalRange(&II);
	}

	/*
		I1	    ------
		I2	------
	*/
	if(AFTER(I1,I2))
	{
		I.t1 = I1->t1;
		I.t2 = I2->t2;
		n = iIntervalRange(&I);
		return n*(n+1)/2;
	}

	return 0;
}

/*

*/
SPAN_INTERVAL * siCreate(INTERVAL* I1, INTERVAL* I2)
{
	SPAN_INTERVAL *si = (SPAN_INTERVAL*)calloc(1,sizeof(SPAN_INTERVAL));
	if(I1>NULL) si->I1 = *I1;
	if(I2>NULL) si->I2 = *I2;

	return si;
}

SPAN_INTERVAL * siClone(SPAN_INTERVAL * si)
{
	SPAN_INTERVAL *si2 = (SPAN_INTERVAL*)malloc(sizeof(SPAN_INTERVAL));
	memcpy(si2,si,sizeof(SPAN_INTERVAL));

	return si2;
}

SPAN_INTERVAL * siIntersection(SPAN_INTERVAL *si1,SPAN_INTERVAL *si2,SPAN_INTERVAL *si)
{	
	if( !(OVERLAP(&si1->I1,&si2->I1) && OVERLAP(&si1->I2,&si2->I2)) ) return NULL;

	if(si<=NULL) si = (SPAN_INTERVAL*)malloc(sizeof(SPAN_INTERVAL));
	memset(si,0,sizeof(SPAN_INTERVAL));
	
	si->I1.t1 = max(si1->I1.t1,si2->I1.t1);
	si->I1.t2 = min(si1->I1.t2,si2->I1.t2);
	si->I2.t1 = max(si1->I2.t1,si2->I2.t1);
	si->I2.t2 = min(si1->I2.t2,si2->I2.t2);

	return si;
}

SPAN_INTERVAL * siLiquify(SPAN_INTERVAL * si)
{
	si->I1.t2 = si->I2.t2;
	si->I2 = si->I1;

	return si;
}
/*

*/
SPAN_INTERVAL_SET * sisCopy(SPAN_INTERVAL_SET * dst, SPAN_INTERVAL_SET * src)
{
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET* s2 = sisCreate();

	if(dst<=NULL) dst = sisCreate();
	else sisInit(dst);

	if(src>NULL)
	{
		for(si = (SPAN_INTERVAL*)lklHead(src);si!=(SPAN_INTERVAL*)lklSTail(src);si = si->next)
		{
			sisAdd(dst, siClone(si));
		}
	}

	return dst;
}

SPAN_INTERVAL_SET * sisClearAndCopy(SPAN_INTERVAL_SET * dst, SPAN_INTERVAL_SET * src)
{
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET* s2 = sisCreate();

	if(dst<=NULL) dst = sisCreate();
	else sisClear(dst);

	if(src>NULL)
	{
		for(si = (SPAN_INTERVAL*)lklHead(src);si!=(SPAN_INTERVAL*)lklSTail(src);si = si->next)
		{
			sisAdd(dst, siClone(si));
		}
	}

	return dst;
}

SPAN_INTERVAL_SET * sisClone(SPAN_INTERVAL_SET * s)
{
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET* s2 = sisCreate();

	if(s<=NULL) 
	{
		sisRelease(&s2);
		return NULL;
	}
	for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next)
	{
		sisAdd(s2, siClone(si));
	}

	return s2;
}

SPAN_INTERVAL_SET* sisGeneralUnion(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2)
{
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET* s = NULL;

	if(s1<=NULL && s2<=NULL) return s;
	if(s1<=NULL) return sisClone(s2);
	if(s2<=NULL) return sisClone(s1);

	s = sisCreate();

	for(si = (SPAN_INTERVAL*)lklHead(s1);si!=(SPAN_INTERVAL*)lklSTail(s1);si = si->next)
	{
		sisAdd(s, siClone(si));
	}

	for(si = (SPAN_INTERVAL*)lklHead(s2);si!=(SPAN_INTERVAL*)lklSTail(s2);si = si->next)
	{
		sisAdd(s, siClone(si));
	}

	sisNormalize(s,0);

	return s;
}

SPAN_INTERVAL_SET* sisGeneralIntersection(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2)
{
	SPAN_INTERVAL* si1, *si2, *si;
	SPAN_INTERVAL_SET* s = NULL;

	if(s1<=NULL || s2<=NULL) return s;

	s = sisCreate();
	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next)
	{
		for(si2 = (SPAN_INTERVAL*)lklHead(s2);si2!=(SPAN_INTERVAL*)lklSTail(s2);si2 = si2->next)
		{
			si = siIntersection(si1,si2,0);
			if(si>NULL)
				sisAdd(s, si);
		}
	}

	return s;
}

SPAN_INTERVAL_SET* _sisGeneralComplement(SPAN_INTERVAL* si, SPAN_INTERVAL* ref)
{
	SPAN_INTERVAL_SET* s = NULL;

	SPAN_INTERVAL*si2;
	
	if(si->I1.t1>ref->I1.t1)
	{
		if(!s) s = sisCreate();
		si2 = siClone(ref);
		si2->I1.t2 = min(si->I1.t1-1,ref->I1.t2);
		sisAdd(s, si2);
	}

	if(si->I1.t2<ref->I1.t2)
	{
		if(!s) s = sisCreate();
		si2 = siClone(ref);
		si2->I1.t1 = max(si->I1.t2+1,ref->I1.t1);
		sisAdd(s, si2);
	}
	
	if(si->I2.t1>ref->I2.t1)
	{
		if(!s) s = sisCreate();
		si2 = siClone(ref);
		si2->I2.t2 = min(si->I2.t1-1,ref->I2.t2);
		sisAdd(s, si2);
	}

	if(si->I2.t2<ref->I2.t2)
	{
		if(!s) s = sisCreate();
		si2 = siClone(ref);
		si2->I2.t1 = max(si->I2.t2+1,ref->I2.t1);
		sisAdd(s, si2);
	}

	return s;
}

SPAN_INTERVAL_SET* sisGeneralComplement(SPAN_INTERVAL_SET* s1, INTERVAL* timeline)
{
	SPAN_INTERVAL_SET *s2 = sisCreate(),*s3,*temp;

	SPAN_INTERVAL* si,*si2,*tempi;

	sisAdd(s2,siCreate(timeline, timeline));

	if(s1<=NULL) return s2;
	
	for(si = (SPAN_INTERVAL*)lklHead(s1); si!=(SPAN_INTERVAL*)lklSTail(s1); si = si->next)
	{
		/* satisfy non overlapping */
		s3 = NULL;
		for(si2 = (SPAN_INTERVAL*)lklHead(s2); si2!=(SPAN_INTERVAL*)lklSTail(s2); si2 = si2->next)
		{
			temp = _sisGeneralComplement(si, si2);
			if(temp)
			{
				if(!s3) s3 = sisCreate();
				for(tempi = (SPAN_INTERVAL*)lklHead(temp); tempi!=(SPAN_INTERVAL*)lklSTail(temp); tempi = tempi->next)
				{
					sisAdd(s3, siClone(tempi));
					//printSpanInterval(tempi);	printf("\n");
				}
				sisRelease(&temp);
			}
		}

		/*if(s3 && s3->cnt>30)
		{
			printSpanIntervalSet(s3);
			printf("$$$$$$$$$$$$$$$$$$$\n");
		}*/

		sisRelease(&s2);
		s2 = s3;
		if(!s2) return NULL;
		else sisNormalize(s2,0);

		/* ------------ */
		/*printf("------------\n");
		printSpanIntervalSet(s2);
		printf("------------\n");*/
		/* ------------ */
	}

	return s2;
}
SPAN_INTERVAL_SET* sisGeneralDiamondm(SPAN_INTERVAL_SET* s1, INTERVAL* timeline)
{
	SPAN_INTERVAL_SET* s = NULL;
	SPAN_INTERVAL* si1, *si;
	INTERVAL I = {0};

	if(s1<=NULL) return s;

	s = sisCreate();
	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next)
	{
		I.t1 = si1->I2.t1+1;
		I.t2 = min(si1->I2.t2+1,timeline->t2);
		if(I.t1<=I.t2)
		{
			si = siCreate(&I,timeline);
			sisAdd(s, si);
		}
	}
	
	sisNormalize(s,0);

	return s;
}

SPAN_INTERVAL_SET* sisGeneralDiamondmi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline)
{
	SPAN_INTERVAL_SET* s = NULL;
	SPAN_INTERVAL* si1, *si;
	INTERVAL I = {0};

	if(s1<=NULL) return s;

	s = sisCreate();
	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next)
	{
		I.t1 = max(si1->I1.t1-1,timeline->t1);
		I.t2 = si1->I1.t2-1;
		if(I.t1<=I.t2)
		{
			si = siCreate(timeline,&I);
			sisAdd(s, si);
		}
	}
	
	sisNormalize(s,0);

	return s;
}

SPAN_INTERVAL_SET* sisGeneralDiamondf(SPAN_INTERVAL_SET* s1, INTERVAL* timeline)
{
	SPAN_INTERVAL_SET* s = NULL;
	SPAN_INTERVAL* si1, *si;

	if(s1<=NULL) return s;

	s = sisCreate();
	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next)
	{
		si = siCreate(timeline,&si1->I2);
		sisAdd(s, si);
	}
	
	sisNormalize(s,0);

	return s;
}

SPAN_INTERVAL_SET* sisGeneralDiamondfi(SPAN_INTERVAL_SET* s1, INTERVAL* timeline)
{
	SPAN_INTERVAL_SET* s = NULL;
	SPAN_INTERVAL* si1, *si;

	if(s1<=NULL) return s;

	s = sisCreate();
	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next)
	{
		si = siCreate(&si1->I1,timeline);
		sisAdd(s, si);
	}
	
	sisNormalize(s,0);

	return s;
}
//
unsigned char sisIsLiquid(SPAN_INTERVAL_SET* s)
{
	SPAN_INTERVAL* si;

	for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next)
	{
		if(!IS_LIQUID(si))
		{
			return 0;
		}
	}
	return 1;
}

int compare (const void * a, const void * b)
{
	return ( ((SPAN_INTERVAL*)a)->I1.t1 - ((SPAN_INTERVAL*)b)->I1.t1 );
}

SPAN_INTERVAL_SET* sisLiquidMerge(SPAN_INTERVAL_SET* s1)
{
	unsigned char flag;
	int cnt = 0,i,j;
	SPAN_INTERVAL* stemp;
	SPAN_INTERVAL_SET* s;
	SPAN_INTERVAL* si1;

	if(s1<=NULL) return NULL;
	
	s = sisCreate();
	if(s1->cnt<=0) return s;

	stemp = (SPAN_INTERVAL*)malloc(s1->cnt*sizeof(SPAN_INTERVAL));
	memset(stemp,0,s1->cnt*sizeof(SPAN_INTERVAL));

	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next,cnt++)
		stemp[cnt] = *si1;
	
	/* merge */
	for(i=0;i<cnt;i++)
	{
		flag = 0;
		for(j=i+1;j<cnt;j++)
		{
			if( OVERLAP(&stemp[i].I1,&stemp[j].I1) || MEET(&stemp[i].I1,&stemp[j].I1) )
			{
				stemp[i].I1.t1 = min(stemp[i].I1.t1,stemp[j].I1.t1);
				stemp[i].I1.t2 = max(stemp[i].I1.t2,stemp[j].I1.t2);
				stemp[i].I2.t1 = min(stemp[i].I2.t1,stemp[j].I2.t1);
				stemp[i].I2.t2 = max(stemp[i].I2.t2,stemp[j].I2.t2);
				
				cnt--;
				stemp[j] = stemp[cnt];
				flag = 1;
			}
		}
		i -= flag;
	}
	
	/* sort */
	qsort ( stemp, cnt, sizeof(SPAN_INTERVAL), compare);

	for(i=0;i<cnt;i++) sisAdd(s,siClone(stemp+i));

	free(stemp);

	return s;
}

SPAN_INTERVAL_SET* sisLiquify(SPAN_INTERVAL_SET* s1)
{
	SPAN_INTERVAL_SET* s,*s2;
	SPAN_INTERVAL* si1;

	if(s1<=NULL) return NULL;
	
	s = sisCreate();
	if(s1->cnt<=0) return s;
	
	for(si1 = (SPAN_INTERVAL*)lklHead(s1);si1!=(SPAN_INTERVAL*)lklSTail(s1);si1 = si1->next)
		sisAdd(s,siLiquify(siClone(si1)));

	s2 = sisLiquidMerge(s);
	sisRelease(&s);

	return s2;
}
SPAN_INTERVAL_SET* sisLiquidUnion(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2)
{
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET* s = NULL;

	if(s1<=NULL && s2<=NULL) return s;
	if(s1<=NULL) return sisClone(s2);
	if(s2<=NULL) return sisClone(s1);

	s = sisCreate();

	for(si = (SPAN_INTERVAL*)lklHead(s1);si!=(SPAN_INTERVAL*)lklSTail(s1);si = si->next)
	{
		sisAdd(s, siClone(si));
	}

	for(si = (SPAN_INTERVAL*)lklHead(s2);si!=(SPAN_INTERVAL*)lklSTail(s2);si = si->next)
	{
		sisAdd(s, siClone(si));
	}

	sisNormalize(s,1);

	return s;
}
SPAN_INTERVAL_SET* sisLiquidComplement(SPAN_INTERVAL_SET* s1, INTERVAL* timeline)
{
	SPAN_INTERVAL_SET* s = NULL,*stemp;
	SPAN_INTERVAL* si1;
	INTERVAL I = {0};

	s = sisCreate();

	if(s1<=NULL || s1->cnt<=0)
	{
		sisAdd(s, siCreate(timeline,timeline));
		return s;
	}

	stemp =  sisLiquidMerge( s1);

	si1 = (SPAN_INTERVAL*)lklHead(stemp);
	if(si1->I1.t1>timeline->t1)
	{
		I.t1 = timeline->t1;
		I.t2 = si1->I1.t1-1;
		sisAdd(s,siCreate(&I,&I));
	}
	
	for(si1 = (SPAN_INTERVAL*)lklHead(stemp);si1!=(SPAN_INTERVAL*)lklTail(stemp);si1 = si1->next)
	{
		I.t1 = si1->I1.t2+1;
		I.t2 = si1->next->I1.t1-1;
		sisAdd(s,siCreate(&I,&I));
	}

	if(si1->I1.t2<timeline->t2)
	{
		I.t1 = si1->I1.t2 + 1;
		I.t2 = timeline->t2;
		sisAdd(s,siCreate(&I,&I));
	}

	/*printf("\n*********\n");
	printSpanIntervalSet(stemp);
	printf("\n---\n");
	printSpanIntervalSet(s);
	printf("\n*********\n");*/

	sisRelease(&stemp);

	return s;
}

//
SPAN_INTERVAL_SET* sisImply(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2, INTERVAL* timeline, unsigned char isLiquid)
{
	SPAN_INTERVAL_SET* _s1 = sisComplement(s1, timeline, isLiquid);
	SPAN_INTERVAL_SET* s = sisUnion(_s1, s2, isLiquid);

	if(_s1>NULL) sisRelease(&_s1);

	return s;
}

SPAN_INTERVAL_SET* sisXUnion(SPAN_INTERVAL_SET* s1,SPAN_INTERVAL_SET* s2, INTERVAL* timeline, unsigned char isLiquid)
{
	SPAN_INTERVAL_SET* _s1 = sisComplement(s1, timeline, isLiquid);
	SPAN_INTERVAL_SET* _s2 = sisComplement(s2, timeline, isLiquid);
	SPAN_INTERVAL_SET* s1_s2 = sisIntersection(s1,_s2, isLiquid);
	SPAN_INTERVAL_SET* s2_s1 = sisIntersection(s2,_s1, isLiquid);
	SPAN_INTERVAL_SET* s = sisUnion(s1_s2, s2_s1, isLiquid);

	if(_s1>NULL) sisRelease(&_s1);
	if(_s2>NULL) sisRelease(&_s2);
	if(s1_s2>NULL) sisRelease(&s1_s2);
	if(s2_s1>NULL) sisRelease(&s2_s1);

	return s;
}

int sisGetNSubIntervals(SPAN_INTERVAL_SET *s)
{
	SPAN_INTERVAL* si;
	int cnt = 0;

	for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next)
		cnt += siGetNSubIntervals(si);
	
	return cnt;
}

void sisMergingIntervals(SPAN_INTERVAL_SET* s, unsigned char isLiquid)
{
	SPAN_INTERVAL* si1,*si2;
	unsigned char flag = 1;


	if(s<=NULL) return;

	while(flag)
	{
		flag = 0;
		for(si1 = (SPAN_INTERVAL*)lklHead(s);si1!=(SPAN_INTERVAL*)lklSTail(s);si1 = si1->next)
		{
			for(si2 = si1->next ;si2!=(SPAN_INTERVAL*)lklSTail(s);si2 = si2->next)
			{
				if(	(isLiquid && (OVERLAP(&si1->I1,&si2->I1) || MEET(&si1->I1,&si2->I1))) || 
					(!isLiquid &&
						(	( EQUAL(&si1->I1,&si2->I1) && ( OVERLAP(&si1->I2,&si2->I2) || MEET(&si1->I2,&si2->I2) ) ) ||
							( EQUAL(&si1->I2,&si2->I2) && ( OVERLAP(&si1->I1,&si2->I1) || MEET(&si1->I1,&si2->I1) ) ) ||
							( INSIDE(&si1->I1,&si2->I1) && INSIDE(&si1->I2,&si2->I2) ) || 
							( INSIDE(&si2->I1,&si1->I1) && INSIDE(&si2->I2,&si1->I2) )
						)
					)
				  )
				{
					si1->I1.t1 = min(si1->I1.t1,si2->I1.t1);
					si1->I1.t2 = max(si1->I1.t2,si2->I1.t2);
					si1->I2.t1 = min(si1->I2.t1,si2->I2.t1);
					si1->I2.t2 = max(si1->I2.t2,si2->I2.t2);
					free(lklDisconnect(s,si2));
					flag = 1;
					break;
				}
			}
			if(flag) break;
		}
	}
}

void sisNormalize(SPAN_INTERVAL_SET* s,unsigned char isLiquid)
{
	SPAN_INTERVAL* si;

	if(s<=NULL) return;

	sisMergingIntervals(s,isLiquid);

	if(!isLiquid)
	{
		for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next)
		{
			si->I1.t2 = min(si->I1.t2,si->I2.t2);
			si->I2.t1 = max(si->I1.t1,si->I2.t1);
		}
	}

	/*if(!isLiquid)
	{
		printf("AAAAAAAAAAA\n");
	}*/
}

int sisLiquidRange(SPAN_INTERVAL_SET* s)
{
	SPAN_INTERVAL* si;
	int range = 0;
	for(si = (SPAN_INTERVAL*)lklHead(s); si!=(SPAN_INTERVAL*)lklSTail(s); si = si->next)
			range += si->I1.t2 - si->I1.t1+1;

	return range;
}

double sisCompare(SPAN_INTERVAL_SET* s1, SPAN_INTERVAL_SET* s2, INTERVAL* timeline, unsigned char liquid)
{
	double rangeInter = 0;
	SPAN_INTERVAL_SET* sInter,*sComp;

	sComp = sisComplement(s1,timeline,liquid);
	sInter = sisIntersection(sComp,s2,liquid);

	rangeInter = sisGetNSubIntervals(sInter);
	
	sisRelease(&sInter);
	sisRelease(&sComp);

	sComp = sisComplement(s2, timeline, liquid);
	sInter = sisIntersection(sComp,s1, liquid);

	rangeInter += sisGetNSubIntervals(sInter);
	
	sisRelease(&sInter);
	sisRelease(&sComp);

	return rangeInter;
}
/*

*/

void printInterval(INTERVAL *I)
{
	if(I<=NULL) return;
	printf("[%d %d]", I->t1, I->t2);
}

void printSpanInterval(SPAN_INTERVAL *si)
{
	if(si<=NULL) return;

	printInterval(&si->I1);
	printf("	");
	printInterval(&si->I2);
	printf("\n");
}

void printSpanIntervalSet(SPAN_INTERVAL_SET *s)
{
	SPAN_INTERVAL* si;

	if(s<=NULL || !s->cnt)
	{
		printf("(empty)\n");
		return;
	}
	for(si = (SPAN_INTERVAL*)lklHead(s); si!=(SPAN_INTERVAL*)lklSTail(s); si = si->next)
		printSpanInterval(si);
}