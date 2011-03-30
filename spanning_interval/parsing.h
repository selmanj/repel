#ifndef __PARSING_H__
#define __PARSING_H__

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include "spanningInterval.h"

/*#define AND '&'
#define OR '|'
#define NOT '!'
#define LIQ '{'*/

#define N_OP 10
extern const char* OP[];
enum{ AND,OR,NOT,LIQ,IMP,DM,DMi,DF,DFi, XOR};

#define EVENT_ARG char arg[32]
#define ARG_EQUAL(arg1, arg2) (!strcmp(arg1,arg2))
#define EVENT_EQUAL(event1, event2) ARG_EQUAL(event1.arg,event2.arg)
#define DISPLAY_EVENT(event) printf("%s",event.arg)
#define printTree parsingTreeTest
#define copyEvent clearCopyEvent

typedef
struct
{
	EVENT_ARG;
	SPAN_INTERVAL_SET s;
	union
	{
		struct
		{
			unsigned char locked : 1;
			unsigned char liquid : 1;
			unsigned char query : 1;
		};
		unsigned char flags;
	};
}
EVENT_TYPE;

typedef
struct parsingNode
{
	struct parsingNode* arg1,*arg2;
	char op;
	
	EVENT_TYPE event;
		
}
PNODE;

char* skipChars(const char* c, const char* text, int length, int* dl);

PNODE* parsingFormula(const char* text);

void parsingTreeTest(PNODE* node);

void displayEventList(EVENT_TYPE* eventList, int nEvents);
void getEventList(PNODE* node, EVENT_TYPE** eventList, int* nEvents);

SPAN_INTERVAL_SET* parsingTree(PNODE* node, EVENT_TYPE* events, int nEvents,INTERVAL* timeline, unsigned char isLiquid);

void printEvent(EVENT_TYPE* event);
EVENT_TYPE* findEvent(PNODE* node,EVENT_TYPE* events, int nEvents);

EVENT_TYPE* cloneEvent(EVENT_TYPE* dst,EVENT_TYPE* src);
void clearCopyEvent(EVENT_TYPE* dst,EVENT_TYPE* src);

EVENT_TYPE* clearEvent(EVENT_TYPE* event);

#endif