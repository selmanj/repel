#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parsing.h"

#define SEP " \t()}\n\0"
const char* OP[] = {"&","|","!","{","=>","Dm","Dmi","Df","Dfi","x"};

char* skipChars(const char* c, const char* text, int length, int* dl)
{
	char* pt = (char*)text;
	unsigned char flag = 1;
	int l = (int)strlen(c);
	int i;
	
	while(flag && length>0)
	{
		flag = 0;
		for(i=0;i<l;i++)
		{
			if(c[i] == *pt)
			{
				flag = 1;
				pt++;
				length--;
				break;
			}
		}
	}

	if(dl>NULL) *dl = pt-text;
	return pt;
}

char* tokenizeFormula(const char* text)
{
	char* p,*p2;
	int l,len = strlen(text)+1;
	char* _text = (char*)calloc(len,sizeof(char));
	char* buf = (char*)malloc(len*sizeof(char));
	memset(buf,0,len*sizeof(char));
	strncpy(_text,text,len-1);

	p2 = buf;
	p = strtok(_text,SEP);
	l = strlen(p);
	strncpy(p2,p,l);
	p2+=l;
	*p2 = ' ';
	p2++;

	do
	{
		p = strtok(NULL,SEP);
		if(p)
		{
			l = strlen(p);
			strncpy(p2,p,l);
			p2+=l;
			*p2 = ' ';
			p2++;
		}
	}
	while(p);

	free(_text);

	return buf;
}

char* _parsingFormula(char* pt, PNODE* node)
{
	int i;
	char c = -1;
	char* start = (char*)pt;
	char*temp;
	char buf[512] = {0};
	char* next = strchr(pt,' ');

	if(next<=NULL) next = pt+strlen(pt)-1;

	for(i=0;i<N_OP;i++)
	{
		if( !strncmp(OP[i],pt,next-pt))
		{
			c = i;
			break;
		}
	}
	
	/* evaluation */
	switch(c)
	{
	case AND:
	case OR:
	case XOR:
	case IMP:
		node->op = c;

		/* evaluate first argument */
		pt = next+1;
		node->arg1 = (PNODE*)calloc(1, sizeof(PNODE));
		pt = _parsingFormula(pt,node->arg1);

		/* evaluate second argument */
		node->arg2 = (PNODE*)calloc(1, sizeof(PNODE));
		pt = _parsingFormula(pt,node->arg2);

		strncpy(buf,start,pt-start);
		printf("%s\n",buf);

		break;
	
	case DM:
	case DMi:
	case DF:
	case DFi:
	case LIQ:
	case NOT:
		node->op = c;

		/* evaluate first argument */
		pt = next+1;
		node->arg1 = (PNODE*)calloc(1, sizeof(PNODE));
		pt = _parsingFormula(pt,node->arg1);

		strncpy(buf,start,pt-start);
		printf("%s\n",buf);
		
		break;
	default:
		node->op = c;
		temp = next;
		if(temp) pt = temp;

		strncpy(node->event.arg,start,pt-start);
		strncpy(buf,start,pt-start);
		printf("%s\n",buf);

		pt++;
	}
	

	return pt;
}

#if 0
char* _parsingFormula2(char* pt, PNODE* node)
{
	char c = 0;
	char* start = (char*)pt;
	char*temp;
	char buf[512] = {0};
	
	/* evaluation */
	switch(*pt)
	{
	case AND:
	case OR:
		node->op = *pt;

		/* evaluate first argument */
		pt = strchr(pt,' ');	pt++;
		node->arg1 = (PNODE*)calloc(1, sizeof(PNODE));
		pt = _parsingFormula(pt,node->arg1);

		/* evaluate second argument */
		node->arg2 = (PNODE*)calloc(1, sizeof(PNODE));
		pt = _parsingFormula(pt,node->arg2);

		strncpy(buf,start,pt-start);
		printf("%s\n",buf);

		break;
	
	case LIQ:
	case NOT:
		node->op = *pt;

		/* evaluate first argument */
		pt = strchr(pt,' ');	pt++;
		node->arg1 = (PNODE*)calloc(1, sizeof(PNODE));
		pt = _parsingFormula(pt,node->arg1);

		strncpy(buf,start,pt-start);
		printf("%s\n",buf);
		
		break;
	default:
		temp = strchr(pt,' ');
		if(temp) pt = temp;

		strncpy(node->event.arg,start,pt-start);
		strncpy(buf,start,pt-start);
		printf("%s\n",buf);

		pt++;
	}
	

	return pt;
}
#endif
PNODE* parsingFormula(const char* text)
{
	char* _text = tokenizeFormula(text);
	char* res;
	PNODE* node = (PNODE*)calloc(1, sizeof(PNODE));

	res = _parsingFormula(_text,node);

	free(_text);

	return node;
}

void parsingTreeTest(PNODE* node)
{
	/* evaluation */
	switch(node->op)
	{
	case AND:
	case OR:
	case XOR:
	case IMP:

		printf("( %s ",OP[node->op]);

		/* evaluate first argument */
		parsingTreeTest(node->arg1);

		/* evaluate second argument */
		parsingTreeTest(node->arg2);

		printf(") ");
		break;
	
	case NOT:
	case DM:
	case DMi:
	case DF:
	case DFi:

		printf("( %s ",OP[node->op]);

		/* evaluate first argument */
		parsingTreeTest(node->arg1);

		printf(") ");
		break;

	case LIQ:

		printf("{ ");

		/* evaluate first argument */
		parsingTreeTest(node->arg1);

		printf("} ");
		break;

	default:
		DISPLAY_EVENT(node->event);
		printf(" ");

		break;
	}
}

void getEventList(PNODE* node, EVENT_TYPE** eventList, int* nEvents)
{
	int i;
	unsigned char flag = 1;
	if(!node->arg1 && !node->arg2)
	{
		for(i=0;i<*nEvents;i++)
		{
			if(EVENT_EQUAL((*eventList)[i], node->event))
			{
				flag = 0;
				break;
			}
		}
		if(flag)
		{
			*eventList = (EVENT_TYPE*)realloc(*eventList,((*nEvents) + 1) *sizeof(EVENT_TYPE));
			memcpy(&(*eventList)[*nEvents],&node->event,sizeof(EVENT_TYPE));
			(*nEvents)++;
		}
	}
	else
	{
		if(node->arg1)
			getEventList(node->arg1, eventList, nEvents);

		if(node->arg2)
			getEventList(node->arg2, eventList, nEvents);
	}
}

void displayEventList(EVENT_TYPE* eventList, int nEvents)
{
	int i;
	printf("EVENT LIST\n-------------\n");
	for(i=0;i<nEvents;i++)
	{
		DISPLAY_EVENT(eventList[i]);
		printf("\n");
	}
}

EVENT_TYPE* findEvent(PNODE* node,EVENT_TYPE* events, int nEvents)
{
	int i;
	for(i=0;i<nEvents;i++)
	{
		if(EVENT_EQUAL(node->event,events[i]))
		{
			return events+i;
		}
	}
	return NULL;
}

SPAN_INTERVAL_SET* parsingTree(PNODE* node, EVENT_TYPE* events, int nEvents,INTERVAL* timeline, unsigned char isLiquid)
{
	EVENT_TYPE* e;
	SPAN_INTERVAL_SET* s = NULL,*s1 = NULL,*s2 = NULL;

	/* evaluation */
	switch(node->op)
	{
	case AND:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		/* evaluate second argument */
		s2 = parsingTree(node->arg2,events,nEvents,timeline,isLiquid);

		s = sisIntersection(s1,s2,isLiquid);
		sisRelease(&s1);
		sisRelease(&s2);

		break;

	case OR:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		/* evaluate second argument */
		s2 = parsingTree(node->arg2,events,nEvents,timeline,isLiquid);

		s = sisUnion(s1,s2,isLiquid);
		sisRelease(&s1);
		sisRelease(&s2);

		break;

	case XOR:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		/* evaluate second argument */
		s2 = parsingTree(node->arg2,events,nEvents,timeline,isLiquid);

		s = sisXUnion(s1,s2,timeline,isLiquid);
		sisRelease(&s1);
		sisRelease(&s2);

		break;

	case IMP:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		/* evaluate second argument */
		s2 = parsingTree(node->arg2,events,nEvents,timeline,isLiquid);

		s = sisImply(s1,s2,timeline,isLiquid);

		/*printf("***********\n");
		printSpanIntervalSet(s1);
		printf("...........\n");
		printSpanIntervalSet(s2);
		printf("...........\n");
		printSpanIntervalSet(s);
		printf("***********\n");*/

		sisRelease(&s1);
		sisRelease(&s2);
		
		break;
	
	case NOT:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		s = sisComplement(s1,timeline,isLiquid);

		/*printf("***********\n");
		printSpanIntervalSet(s1);
		printf("...........\n");
		printSpanIntervalSet(s);
		printf("***********\n");*/

		sisRelease(&s1);
		
		break;

	case DM:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		if(isLiquid)
		{
			fprintf(stderr,"WARNING: using liquid operator on a non-liquid formula (%s)!\n",OP[node->op]);
		}

		s = sisDiamondm(s1,timeline,isLiquid);
		sisRelease(&s1);
		
		break;

	case DMi:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		if(isLiquid)
		{
			fprintf(stderr,"WARNING: using liquid operator on a non-liquid formula (%s)!\n",OP[node->op]);
		}

		s = sisDiamondmi(s1,timeline,isLiquid);

		/*printf("***********\n");
		printSpanIntervalSet(s1);
		printf("...........\n");
		printSpanIntervalSet(s);
		printf("***********\n");*/

		sisRelease(&s1);

		break;

	case DF:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		if(isLiquid)
		{
			fprintf(stderr,"WARNING: using liquid operator on a non-liquid formula (%s)!\n",OP[node->op]);
		}

		s = sisDiamondf(s1,timeline,isLiquid);
		sisRelease(&s1);
		
		break;

	case DFi:

		/* evaluate first argument */
		s1 = parsingTree(node->arg1,events,nEvents,timeline,isLiquid);

		if(isLiquid)
		{
			fprintf(stderr,"WARNING: using liquid operator on a non-liquid formula (%s)!\n",OP[node->op]);
		}

		s = sisDiamondfi(s1,timeline,isLiquid);
		sisRelease(&s1);
		
		break;

	case LIQ:

		/* evaluate first argument */
		s = parsingTree(node->arg1,events,nEvents,timeline,1);
		
		break;

	default:
		e = findEvent(node,events,nEvents);
		if(e) s = sisClone(&e->s);
		break;
	}

	/*printf("\n=================\nop: %s\n",node->op=='!'?"NOT":node->op=='&'?"AND":node->op=='|'?"OR ":node->event.arg);
	if(s) printSpanIntervalSet(s);*/

	return s;
}
void printEvent(EVENT_TYPE* event)
{
	printf("\n--------------------\n%s %s\n--------------------\n",event->arg,event->locked?"> locked":"");
	printSpanIntervalSet(&event->s);
}
EVENT_TYPE* cloneEvent(EVENT_TYPE* dst,EVENT_TYPE* src)
{
	if(dst<=NULL) dst = (EVENT_TYPE*)malloc(sizeof(EVENT_TYPE));
	memcpy(dst,src,sizeof(EVENT_TYPE));
	//memset(dst,0,sizeof(EVENT_TYPE));
	//memcpy(dst->arg,src->arg,sizeof(src->arg));
	sisInit(&dst->s);
	sisCopy(&dst->s,&src->s);

	return dst;
}

void clearCopyEvent(EVENT_TYPE* dst,EVENT_TYPE* src)
{
	sisClear(&dst->s);

	memcpy(dst,src,sizeof(EVENT_TYPE));
	//memset(dst,0,sizeof(EVENT_TYPE));
	//memcpy(dst->arg,src->arg,sizeof(src->arg));
	sisInit(&dst->s);
	sisCopy(&dst->s,&src->s);
}

EVENT_TYPE* clearEvent(EVENT_TYPE* event)
{
	sisClear(&event->s);

	return event;
}