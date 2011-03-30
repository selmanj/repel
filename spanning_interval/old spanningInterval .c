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

INTERVAL* iCreateInterval(int t1, int t2)
{
	INTERVAL*I = (INTERVAL*) malloc(sizeof(INTERVAL));
	I->prev = I->next = 0;
	I->t1 = t1;
	I->t2 = t2;

	return I;
}
/*

*/
SPAN_INTERVAL* siInitSpanningInterval(SPAN_INTERVAL* si)
{
	lklInitList(&si->I1);
	lklInitList(&si->I2);

	return si;
}
SPAN_INTERVAL* siCreateSpanningInterval()
{
	return  siInitSpanningInterval( (SPAN_INTERVAL*)malloc(sizeof(SPAN_INTERVAL)));;
}
void siFreeSpanningIntervalMem(SPAN_INTERVAL* si)
{
	lklClearList(&si->I1,0);
	lklClearList(&si->I2,0);
}
void siReleaseSpanningInterval(SPAN_INTERVAL** si)
{
	if(*si > NULL)
	{
		siFreeSpanningIntervalMem(*si);
		free(*si);
	}
	*si = NULL;
}
SPAN_INTERVAL * siIntersection(SPAN_INTERVAL *si1,SPAN_INTERVAL *si2,SPAN_INTERVAL *si)
{
	INTERVAL* Isi1,*Isi2;

	if(si<=NULL) si = siCreateSpanningInterval();
	else		 siFreeSpanningIntervalMem(si);

	for(Isi1 = (INTERVAL*)lklHead(&si1->I1);
		Isi1 != (INTERVAL*)lklSTail(&si1->I1);
		Isi1 = Isi1->next)
	{
		for(Isi2 = (INTERVAL*)lklHead(&si2->I1);
			Isi2 != (INTERVAL*)lklSTail(&si2->I1);
			Isi2 = Isi2->next)
		{
			if( OVERLAP(Isi1,Isi2) )
			{
				lklPushBack(&si->I1,iCreateInterval(max(Isi1->t1,Isi2->t1), min(Isi1->t2,Isi2->t2)));
			}
		}
	}

	for(Isi1 = (INTERVAL*)lklHead(&si1->I2);
		Isi1 != (INTERVAL*)lklSTail(&si1->I2);
		Isi1 = Isi1->next)
	{
		for(Isi2 = (INTERVAL*)lklHead(&si2->I2);
			Isi2 != (INTERVAL*)lklSTail(&si2->I2);
			Isi2 = Isi2->next)
		{
			if( OVERLAP(Isi1,Isi2) )
			{
				lklPushBack(&si->I1,iCreateInterval(max(Isi1->t1,Isi2->t1), min(Isi1->t2,Isi2->t2)));
			}
		}
	}

	return si;
}

SPAN_INTERVAL* siUnion(SPAN_INTERVAL *si1, SPAN_INTERVAL *si2, SPAN_INTERVAL *si)
{
	INTERVAL* timeline;
	SPAN_INTERVAL* c1,*c2,*i;

	if(si<=NULL) si = siCreateSpanningInterval();
	else		 siFreeSpanningIntervalMem(si);

	timeline = iCreateInterval(((INTERVAL*)lklHead(&si1->I1))->t1,((INTERVAL*)lklTail(&si1->I1))->t2);
	c1 = siComplement(si1, timeline, 0);
	c2 = siComplement(si2, timeline, 0);
	i = siIntersection(c1,c2,0);
	si = siComplement(i, timeline, si);

	free(timeline);
	siReleaseSpanningInterval(&c1);
	siReleaseSpanningInterval(&c2);
	siReleaseSpanningInterval(&i);

	return si;
}

SPAN_INTERVAL* siComplement(SPAN_INTERVAL *si1, INTERVAL* timeline, SPAN_INTERVAL *si)
{
	INTERVAL* Isi1,*Isi1next;
	unsigned char *A = NULL;

	if(si<=NULL) si = siCreateSpanningInterval();
	else		 siFreeSpanningIntervalMem(si);

	if(si1->I1.cnt<=0)
	{
		lklPushBack(&si->I1,iCreateInterval(timeline->t1,timeline->t2 ));
	}
	else
	{
		Isi1 = (INTERVAL*)lklHead(&si1->I1);
		Isi1next = Isi1->next;
		if(Isi1->t1>timeline->t1)
		{
			lklPushBack(&si->I1,iCreateInterval(timeline->t1,Isi1->t1-1 ));
		}

		while(Isi1next != (INTERVAL*)lklSTail(&si1->I1))
		{
			lklPushBack(&si->I1,iCreateInterval(Isi1->t2+1,Isi1next->t1-1 ));
			Isi1 = Isi1next;
			Isi1next = Isi1next->next;
		}
		if(Isi1->t2<timeline->t2)
		{
			lklPushBack(&si->I1,iCreateInterval(Isi1->t2+1,timeline->t2 ));
		}
	}

	if(si1->I2.cnt<=0)
	{
		lklPushBack(&si->I2,iCreateInterval(timeline->t1,timeline->t2 ));
	}
	else
	{
		Isi1 = (INTERVAL*)lklHead(&si1->I2);
		Isi1next = Isi1->next;
		if(Isi1->t1>timeline->t1)
		{
			lklPushBack(&si->I2,iCreateInterval(timeline->t1,Isi1->t1-1 ));
		}

		while(Isi1next != (INTERVAL*)lklSTail(&si1->I2))
		{
			lklPushBack(&si->I2,iCreateInterval(Isi1->t2+1,Isi1next->t1-1 ));
			Isi1 = Isi1next;
			Isi1next = Isi1next->next;
		}
		if(Isi1->t2<timeline->t2)
		{
			lklPushBack(&si->I2,iCreateInterval(Isi1->t2+1,timeline->t2 ));
		}
	}

	return si;
}