#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#define PEL_LOG_FILE "log.txt"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "pel.h"

int pelLoadFormulas(const char* filename, PNODE*** formulaTrees, int* nFormulas, double** weights)
{
	char buf[1024] = {0};
	char txt[1024] = {0};
	double w;
	FILE* fp = fopen(filename,"r");

	if(fp<=NULL)
	{
		fprintf(stderr,"ERROR: Could not load %s\n",filename);
		return 0;
	}

	*nFormulas = 0;
	*formulaTrees = NULL;

	while(!feof(fp))
	{
		memset(buf,0,1024*sizeof(char));
		fscanf(fp,"%lf",&w);
		fgets(buf,1024,fp);
		*weights = (double*)realloc(*weights,((*nFormulas)+1)*sizeof(double));
		*formulaTrees = (PNODE**)realloc(*formulaTrees,((*nFormulas)+1)*sizeof(PNODE*));

		//sscanf(buf,"%d	%s",&w,txt);
		(*weights)[*nFormulas] = w;
		(*formulaTrees)[*nFormulas] = parsingFormula(buf);
		(*nFormulas)++;
	}

	fclose(fp);

	return 1;
}

int pelAugmentFormulas(EVENT_TYPE* events, int nEvents, const char* ref,  
					   PNODE*** formulaTrees, int* nFormulas, double** weights)
{
	int i,j,k;
	char* pt,*pt2;
	PNODE* node;
	int refl = (int)strlen(ref);
	int ptl,ptl2;
	for(i=0;i<nEvents;i++)
	{
		k=i;
		pt = skipChars(" \t()\n\0", events[i].arg, strlen(events[i].arg), 0);
		ptl = strlen(pt);
		if(ptl<refl) continue;

		for(j=0;j<refl;j++)
		{
			if(tolower(ref[j]) != tolower(pt[j]))
			{
				k = -1;
				break;
			}
		}
		if(k<0) continue;

		/* augment formulas */
		pt2 = pt + refl;
		pt2 = skipChars(" \t()\n\0", pt2, ptl-refl, 0);
		ptl2 = strlen(pt2);
		(*formulaTrees) = (PNODE**)realloc(*formulaTrees,((*nFormulas)+1)*sizeof(PNODE*));
		node = (*formulaTrees)[*nFormulas] = (PNODE*)calloc(1,sizeof(PNODE));
		node->op = IMP;
		node->arg1 = (PNODE*)calloc(1,sizeof(PNODE));
		strncpy(node->arg1->event.arg,pt2,ptl2);
		node->arg1->op = -1;
		node->arg2 = (PNODE*)calloc(1,sizeof(PNODE));
		strncpy(node->arg2->event.arg,pt,ptl);
		node->arg2->op = -1;
		(*weights) = (double*)realloc(*weights,((*nFormulas)+1)*sizeof(double));
		(*weights)[*nFormulas] = 1;
		(*nFormulas)++;
	}

	return 1;
}

int pelLoadInterpretation(const char* filename, EVENT_TYPE** events, int* nEvents)
{
	int i;
	char buf[1024] = {0};
	FILE* fp = fopen(filename,"r");
	INTERVAL I;
	EVENT_ARG = {0};
	unsigned char flag;

	if(fp<=NULL)
	{
		fprintf(stderr,"ERROR: Could not load %s\n",filename);
		return 0;
	}

	*events = NULL;
	*nEvents = 0;

	while(!feof(fp))
	{
		fgets(buf,1024,fp);
		(*nEvents)++;
	}

	*events = (EVENT_TYPE*)malloc((*nEvents)*sizeof(EVENT_TYPE));
	rewind(fp);
	*nEvents = 0;

	while(!feof(fp))
	{
		memset(buf,0,1024*sizeof(char));
		fgets(buf,1024,fp);
		
		sscanf(buf,"%s @ [%d : %d]",arg,&I.t1,&I.t2);

		flag = 1;
		for(i=0;i<*nEvents;i++)
		{
			if(ARG_EQUAL((*events)[i].arg,arg))
			{
				flag = 0;
				sisAdd(&(*events)[i].s,siCreate(&I,&I));
				//printSpanIntervalSet(&(*events)[i].s);
				//printEvent(&(*events)[i]);
				break;
			}
		}

		if(flag)
		{
			memcpy((*events)[*nEvents].arg,arg,sizeof(arg));
			(*events)[*nEvents].flags = 0;
			(*events)[*nEvents].liquid = 1;
			sisInit(&(*events)[*nEvents].s);
			sisAdd(&(*events)[*nEvents].s,siCreate(&I,&I));
			//printSpanIntervalSet(&(*events)[*nEvents].s);
			//printEvent(&(*events)[*nEvents]);
			(*nEvents)++;
		}
	}

	*events = (EVENT_TYPE*)realloc(*events,(*nEvents)*sizeof(EVENT_TYPE));
	//printEvent(&(*events)[0]);
	//printIterpretation(*events, *nEvents);

	fclose(fp);

	return 1;
}

int pelSaveInterpretation(const char* filename, EVENT_TYPE* events, int nEvents)
{
	int i;
	char buf[1024] = {0};
	FILE* fp = fopen(filename,"w+");
	SPAN_INTERVAL* si;
	EVENT_ARG = {0};

	if(fp<=NULL)
	{
		fprintf(stderr,"ERROR: Could not load %s\n",filename);
		return 0;
	}

	for(i=0;i<nEvents;i++)
	{
		for(si = (SPAN_INTERVAL*)lklHead(&events[i].s);si!=(SPAN_INTERVAL*)lklSTail(&events[i].s);si = si->next)
		{
			if(IS_LIQUID(si))
				fprintf(fp,"%s @ [%d : %d]\n",events[i].arg,si->I1.t1,si->I1.t2);
			else
				fprintf(fp,"%s @ [	[%d  %d]	[%d  %d]	]\n",events[i].arg,si->I1.t1,si->I1.t2,si->I2.t1,si->I2.t2);
		}
	}

	fclose(fp);

	return 1;
}

int pelLogIter(const char* filename,int iter, unsigned char createFile)
{
	FILE* fp = fopen(filename,(createFile?"w+":"a+"));

	if(fp<=NULL)
	{
		fprintf(stderr,"ERROR: Could not load %s\n",filename);
		return 0;
	}

	fprintf(fp,"Iteration: %d\n\n",iter);

	fclose(fp);

	return 1;
}

int pelLogInterpretation(const char* filename, EVENT_TYPE* events, int nEvents, double score, unsigned char isBest, unsigned char createFile)
{
	int i;
	FILE* fp = fopen(filename,(createFile?"w+":"a+"));
	SPAN_INTERVAL* si;
	EVENT_ARG = {0};

	if(fp<=NULL)
	{
		fprintf(stderr,"ERROR: Could not load %s\n",filename);
		return 0;
	}

	for(i=0;i<nEvents;i++)
	{
		for(si = (SPAN_INTERVAL*)lklHead(&events[i].s);si!=(SPAN_INTERVAL*)lklSTail(&events[i].s);si = si->next)
		{
			fprintf(fp,"%s @ [	[%d  %d]	[%d  %d]	]\n",events[i].arg,si->I1.t1,si->I1.t2,si->I2.t1,si->I2.t2);
		}
		if(events[i].s.cnt<=0)
			fprintf(fp,"%s @ (empty)\n",events[i].arg);
	}

	fprintf(fp,"\nScore: %lf %s\n",score,isBest?"BEST SCORE":"");

	fprintf(fp,"-----------------------------------------------\n");

	fclose(fp);

	return 1;
}

int pelLogMove(const char* filename, EVENT_TYPE* e, SPAN_INTERVAL* si, const char* action, unsigned char createFile)
{
	char buf[1024] = {0};
	FILE* fp = fopen(filename,(createFile?"w+":"a+"));
	EVENT_ARG = {0};

	if(fp<=NULL)
	{
		fprintf(stderr,"ERROR: Could not load %s\n",filename);
		return 0;
	}

	fprintf(fp,"%s %s [%d %d]	[%d %d]\n",action,e->arg,si->I1.t1,si->I1.t2,si->I2.t1,si->I2.t2);

	fclose(fp);

	return 1;
}

void pelNormaliseInterpretation(EVENT_TYPE* events, int nEvents)
{
	int i;

	for(i=0;i<nEvents;i++)
	{
		sisNormalize(&events[i].s, events[i].liquid);
	}
}

void pelPrintIterpretation(EVENT_TYPE* events, int nEvents)
{
	int i;

	for(i=0;i<nEvents;i++)
	{
		printEvent(&events[i]);
	}
}

void pelFindTimeline(EVENT_TYPE* events, int nEvents,INTERVAL* timeline)
{
	int i;
	SPAN_INTERVAL* si;

	timeline->t1 = INT_MAX;
	timeline->t2 = -INT_MAX;

	for(i=0;i<nEvents;i++)
	{
		for(si = (SPAN_INTERVAL*)lklHead(&events[i].s); si!=(SPAN_INTERVAL*)lklSTail(&events[i].s); si = si->next)
		{
			timeline->t1 = min(timeline->t1,si->I1.t1);
			timeline->t1 = min(timeline->t1,si->I2.t1);
			timeline->t2 = max(timeline->t2,si->I1.t2);
			timeline->t2 = max(timeline->t2,si->I2.t2);
		}
	}
}

void pelCompleteEventList(PNODE* node, EVENT_TYPE** events, int* nEvents,INTERVAL* timeline)
{
	EVENT_TYPE* e;
	int i;

	/* evaluation */
	switch(node->op)
	{
	case -1:
		e = findEvent(node,*events,*nEvents);
		if(!e)
		{
			e = (EVENT_TYPE*)malloc(((*nEvents)+1)*sizeof(EVENT_TYPE));
			for(i=0;i<*nEvents;i++)
				cloneEvent(e+i,(*events)+i);
			//cloneEvent(e+(*nEvents),&node->event);
			memset(e+(*nEvents),0,sizeof(EVENT_TYPE));
			memcpy(e+(*nEvents),&node->event,sizeof(EVENT_TYPE));
			sisInit(&e[*nEvents].s);
			e[*nEvents].liquid = 1;
			//sisAdd(&e[*nEvents].s,siCreate(timeline,timeline));
			pelReleaseInterpretation(events, *nEvents);
			*events = e;
			(*nEvents)++;
		}
		break;
	default:
		/* evaluate first argument */
		if(node->arg1)pelCompleteEventList(node->arg1,events,nEvents,timeline);

		/* evaluate second argument */
		if(node->arg2)pelCompleteEventList(node->arg2,events,nEvents,timeline);

		break;
	}
}

void pelLockEvents(EVENT_TYPE* events, int nEvents, const char* ref)
{
	int i,j;
	char* pt;
	int refl = (int)strlen(ref);
	int ptl;
	for(i=0;i<nEvents;i++)
	{
		pt = skipChars(" \t()\n\0", events[i].arg, strlen(events[i].arg), 0);
		ptl = strlen(pt);
		if(ptl<refl)
		{
			events[i].locked = 0;
			continue;
		}

		events[i].locked = 1;
		for(j=0;j<refl;j++)
		{
			if(tolower(ref[j]) != tolower(pt[j]))
			{
				events[i].locked = 0;
				break;
			}
		}
	}
}

unsigned char pelIsLockable(EVENT_TYPE* e, const char* ref)
{
	int j;
	char* pt;
	int refl = (int)strlen(ref);
	int ptl;
	unsigned char flag = 1;
	
	pt = skipChars(" \t()\n\0", e->arg, strlen(e->arg), 0);
	ptl = strlen(pt);
	if(ptl<refl)
	{
		return 0;
	}

	for(j=0;j<refl;j++)
	{
		if(tolower(ref[j]) != tolower(pt[j]))
		{
			flag = 0;
			break;
		}
	}

	return flag;
}

SPAN_INTERVAL_SET* pelNOTSAT(PNODE* node, EVENT_TYPE* events, int nEvents,INTERVAL* timeline)
{
	SPAN_INTERVAL_SET* s;
	PNODE* root = (PNODE*)calloc(1, sizeof(PNODE));
	root->op = NOT;
	root->arg1 = node;

	s = pelSAT(root, events, nEvents, timeline,(node->op == LIQ));

	free(root);

	return s;
}

double pelScore(PNODE** formulaTrees, int nFormulas, double* weights,
				EVENT_TYPE* events, int nEvents,INTERVAL* timeline)
{
	int i;
	SPAN_INTERVAL_SET* s,*s2=NULL;
	SPAN_INTERVAL* si1,* si2,*si ;
	double score = 0,scorei;

	for(i=0;i<nFormulas;i++)
	{
		s = pelSAT(formulaTrees[i], events, nEvents, timeline,0);

		/*if(s) 
		{
			printf("-----\n");
			printSpanIntervalSet(s);
		}*/

		scorei = (weights>NULL?weights[i]:1) * (s>NULL?pelSATSize(s):0);

		/* remove intersection */
		if(s>NULL)
		{
			for(si1 = (SPAN_INTERVAL*)lklHead(s);si1!=(SPAN_INTERVAL*)lklSTail(s);si1 = si1->next)
			{
				for(si2 = si1->next ;si2!=(SPAN_INTERVAL*)lklSTail(s);si2 = si2->next)
				{
					si = siIntersection(si1,si2,0);
					if(si>NULL)
					{
						if(s2<=NULL) s2 = sisCreate();
						sisAdd(s2, si);
					}
				}
			}
			scorei -= (weights>NULL?weights[i]:1) * (s2>NULL?pelSATSize(s2):0);
		}

		//printf("score %d: %lf\n",i,scorei);

		score += scorei;

		if(s>NULL) sisRelease(&s);
		if(s2>NULL) sisRelease(&s2);
	}

	return score;
}

int pelFindId(PNODE* node,EVENT_TYPE* events, int nEvents)
{
	int i;
	for(i=0;i<nEvents;i++)
	{
		if(EVENT_EQUAL(node->event,events[i]))
		{
			return i;
		}
	}
	return -1;
}

void pelAdd(EVENT_TYPE* e,SPAN_INTERVAL* si,INTERVAL* timeline)
{
#ifdef PEL_LOG_FILE
			pelLogMove(PEL_LOG_FILE, e, si, "ADD", 0);
#endif
			sisAdd(&(e->s),siClone(si));
			sisNormalize(&(e->s),e->liquid);
}

void pelDel(EVENT_TYPE* e,SPAN_INTERVAL* si,INTERVAL* timeline)
{
	SPAN_INTERVAL_SET *s,s2,*s3;
#ifdef PEL_LOG_FILE
		pelLogMove(PEL_LOG_FILE, e, si, "DEL", 0);
#endif
		sisInit(&s2);
		sisAdd(&s2,siClone(si));
		s3 = sisComplement(&s2, timeline,e->liquid);
		s = sisIntersection(&(e->s),s3,e->liquid);
		sisClearAndCopy(&(e->s),s);
		sisClear(&s2);
		sisRelease(&s);
		sisRelease(&s3);
		sisNormalize(&(e->s),e->liquid);
}

void pelExecuteMove(SPAN_INTERVAL* si, EVENT_TYPE* events, int nEvents, INTERVAL* timeline, int* move)
{
	int i;
	SPAN_INTERVAL* _si;

	for(i=0;i<nEvents;i++)
	{
		if(events[i].locked) continue;
		_si = events[i].liquid?siLiquify(siClone(si)):si;

		if(move[i]>0)		/* ADD */
		{
			pelAdd(events+i,_si,timeline);
		}
		else if(move[i]<0)	/* DEL */
		{
			pelDel(events+i,_si,timeline);
		}
		else
		{
			
		}

		if(_si!=si) free(_si);
	}
}

int pelTree(SPAN_INTERVAL* si,PNODE* node, EVENT_TYPE* events, int nEvents, INTERVAL* timeline, unsigned char notFlag, int* move)
{
	int id;
	int satisfied = 0;
	int* move2;
	SPAN_INTERVAL si2;

	/* evaluation */
	switch(node->op)
	{
	case AND:

		if(notFlag)
		{
			/* evaluate one the two arguments */
			pelTree(si,(rand()%2?node->arg1:node->arg2),events,nEvents,timeline,notFlag,move);
		}
		else
		{
			/* evaluate first argument */
			pelTree(si,node->arg1,events,nEvents,timeline,notFlag,move);

			/* evaluate second argument */
			pelTree(si,node->arg2,events,nEvents,timeline,notFlag,move);
		}

		break;

	case OR:

		if(notFlag)
		{
			/* evaluate first argument */
			pelTree(si,node->arg1,events,nEvents,timeline,notFlag,move);

			/* evaluate second argument */
			pelTree(si,node->arg2,events,nEvents,timeline,notFlag,move);
		}
		else
		{
			/* evaluate one the two arguments */
			pelTree(si,(rand()%2?node->arg1:node->arg2),events,nEvents,timeline,notFlag,move);
		}

		break;

	case XOR:

		if(notFlag)
		{
			/* evaluate first argument */
			id = rand()%2;
			pelTree(si,(id?node->arg1:node->arg2),events,nEvents,timeline,(id?!notFlag:notFlag),move);

			/* evaluate second argument */
			id = rand()%4;
			pelTree(si,(id?node->arg1:node->arg2),events,nEvents,timeline,(id?notFlag:!notFlag),move);
		}
		else
		{
			if(rand()%2)
			{
				/* evaluate first argument */
				pelTree(si,node->arg1,events,nEvents,timeline,notFlag,move);

				/* evaluate second argument */
				pelTree(si,node->arg2,events,nEvents,timeline,!notFlag,move);
			}
			else
			{
				/* evaluate first argument */
				pelTree(si,node->arg1,events,nEvents,timeline,!notFlag,move);

				/* evaluate second argument */
				pelTree(si,node->arg2,events,nEvents,timeline,notFlag,move);
			}
		}

		break;

	case IMP:

		if(notFlag)
		{
			/* evaluate first argument */
			pelTree(si,node->arg1,events,nEvents,timeline,!notFlag,move);

			/* evaluate second argument */
			pelTree(si,node->arg2,events,nEvents,timeline,notFlag,move);
		}
		else
		{
			/* evaluate one the two arguments */
			id = rand()%2;
			pelTree(si,(id?node->arg1:node->arg2),events,nEvents,timeline,(id?!notFlag:notFlag),move);
		}

		break;
	
	case NOT:

		notFlag = !notFlag;

		/* evaluate first argument */
		pelTree(si,node->arg1,events,nEvents,timeline,notFlag,move);
		
		break;

	case DM:

		/* init the move */
		move2 = (int*)calloc(nEvents,sizeof(int));
		/*si2.I1.t1 = max(si->I1.t1 - 1,timeline->t1);
		si2.I1.t2 = min(si->I1.t2,max(si->I2.t1,si->I1.t1)) - 1;
		si2.I2 = si2.I1;*/
		si2.I1.t1 = max(si->I1.t1-1,timeline->t1);
		si2.I1.t2 = max(si->I1.t1-1,si2.I1.t1);
		si2.I2 = si2.I1;

		if(si2.I1.t1<=si2.I1.t2)
		{
			/* evaluate first argument */
			pelTree(&si2,node->arg1,events,nEvents,timeline,notFlag,move2);

			/* make the move */
			pelExecuteMove(&si2, events, nEvents, timeline, move2);
		}
		
		free(move2);
		satisfied = 1;
		break;

	case DMi:

		/* init the move */
		move2 = (int*)calloc(nEvents,sizeof(int));
		/*si2.I1.t1 = max(si->I2.t1,min(si->I1.t2,si->I2.t2)) + 1;
		si2.I1.t2 = min(si->I2.t2 + 1,timeline->t2);
		si2.I2 = si2.I1;*/
		si2.I1.t2 = min(si->I2.t2+1,timeline->t2);
		si2.I1.t1 = min(si->I2.t1+1,si2.I1.t2);
		si2.I2 = si2.I1;

		if(si2.I1.t1<=si2.I1.t2)
		{
			/* evaluate first argument */
			pelTree(&si2,node->arg1,events,nEvents,timeline,notFlag,move2);

			/* make the move */
			pelExecuteMove(&si2, events, nEvents, timeline, move2);
		}
		
		free(move2);
		satisfied = 1;
		break;

	case DF:

		/* init the move */
		move2 = (int*)calloc(nEvents,sizeof(int));
		/*si2.I1.t1 = max(si->I2.t1,min(si->I1.t2,si->I2.t2));
		si2.I1.t2 = si->I2.t2;
		si2.I2 = si2.I1;*/
		si2.I2 = si2.I1 = si->I2;

		/* evaluate first argument */
		pelTree(&si2,node->arg1,events,nEvents,timeline,notFlag,move2);

		/* make the move */
		pelExecuteMove(&si2, events, nEvents, timeline, move2);
		
		free(move2);
		satisfied = 1;
		break;

	case DFi:

		/* init the move */
		move2 = (int*)calloc(nEvents,sizeof(int));
		/*si2.I1.t1 = si->I1.t1;
		si2.I1.t2 = min(si->I1.t2,max(si->I2.t1,si->I1.t1));
		si2.I2 = si2.I1;*/
		si2.I2 = si2.I1 = si->I1;

		/* evaluate first argument */
		pelTree(&si2,node->arg1,events,nEvents,timeline,notFlag,move2);

		/* make the move */
		pelExecuteMove(&si2, events, nEvents, timeline, move2);
		
		free(move2);
		satisfied = 1;
		break;

	case LIQ:

		/* evaluate first argument */
		pelTree(si,node->arg1,events,nEvents,timeline,notFlag,move);
		
		break;

	default:
		id = pelFindId(node,events,nEvents);
		if(id>=0)
			move[id] += notFlag?-1:1;
		break;
	}

	return satisfied;
}

void pelMove(SPAN_INTERVAL* si,PNODE* formula, EVENT_TYPE* events, int nEvents, INTERVAL* timeline)
{
	int* move = (int*)calloc(nEvents,sizeof(int));
	
	/* find the set of ADD and DEL for each event */
	pelTree(si, formula, events, nEvents, timeline,0, move);

	/* execute the moves */
	pelExecuteMove(si, events, nEvents,timeline, move);
	
	free(move);
}

EVENT_TYPE* pelCloneInterpretation(EVENT_TYPE* events, int nEvents)
{
	int i;
	EVENT_TYPE* events2 = (EVENT_TYPE*)malloc(nEvents*sizeof(EVENT_TYPE));

	for(i=0;i<nEvents;i++)
	{
		cloneEvent(events2+i,events+i);
	}

	return events2;
}

EVENT_TYPE* pelCopyInterpretation(EVENT_TYPE*dst, EVENT_TYPE* src, int nEvents)
{
	int i;
	if(dst<=NULL) dst = (EVENT_TYPE*)malloc(nEvents*sizeof(EVENT_TYPE));

	for(i=0;i<nEvents;i++)
	{
		copyEvent(dst+i,src+i);
	}

	return dst;
}

void pelReleaseInterpretation(EVENT_TYPE** events, int nEvents)
{
	int i;

	for(i=0;i<nEvents;i++)
	{
		clearEvent((*events)+i);
	}

	free(*events);
	*events = NULL;
}

int pelInference(	PNODE** formulaTrees, int nFormulas, double* weights,
					EVENT_TYPE* events, int nEvents,INTERVAL* timeline, int maxIters, double prob,
					EVENT_TYPE*** recEvents, unsigned char flag)
{
	PNODE* formula;
	SPAN_INTERVAL_SET* s;
	EVENT_TYPE* temp,*cur,*prev,*best,*e;
	double p;
	SPAN_INTERVAL* si;
	double score, bestScore, curScore;
	int i,j,iter=0;

	srand((int)time(0));

	best = pelCloneInterpretation( events, nEvents);
	bestScore = pelScore(formulaTrees, nFormulas, weights,
								 best, nEvents, timeline);
	prev = pelCloneInterpretation( events, nEvents);

	if(recEvents>NULL)
	{
		(*recEvents) = (EVENT_TYPE**)malloc((maxIters+1)*sizeof(EVENT_TYPE*));
		memset(*recEvents,0,(maxIters+1)*sizeof(EVENT_TYPE*));
		(*recEvents)[iter] = pelCloneInterpretation( events, nEvents);
	}

	//pelPrintIterpretation(best, nEvents);

#ifdef PEL_LOG_FILE
		pelLogInterpretation(PEL_LOG_FILE, prev, nEvents, bestScore,0, 1);
#endif

	while(maxIters--)
	{
		p = (double)rand()/(double)RAND_MAX;
		if(p<prob)
		{
			/* choose a formula */
			formula = formulaTrees[/*0*/rand()%nFormulas];

			/* compute violating spanning interval */
			s = pelNOTSAT(formula, prev, nEvents, timeline);
			if(s<=NULL || s->cnt<=0)
			{
				// ----------------
				/*printf("\nVIOLATING INTERVAL\n");
				printSpanIntervalSet(s);
				printf("\nINTERPRETATION\n");
				pelPrintIterpretation(prev, nEvents);
				printf("\n~~~~~~~~~~~~~~~~~\n");*/
				// ----------------
				sisRelease(&s);
				continue;
			}
			i = rand()%s->cnt;
			for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s) && i-- ;si = si->next);
			cur = pelCloneInterpretation( prev, nEvents);
			pelMove(si,formula, cur, nEvents, timeline);
			curScore = pelScore(formulaTrees, nFormulas, weights,
								cur, nEvents, timeline);

			sisRelease(&s);
		}
		else
		{
			cur = NULL;
			curScore = -1e20;
			for(j=0;j<nFormulas;j++)
			{
				formula = formulaTrees[j];
				if(flag)
				{
					j = rand()%nFormulas;
					formula = formulaTrees[j];
					j = nFormulas;
				}

				/* compute violating spanning interval */
				s = pelNOTSAT(formula, prev, nEvents, timeline);
				if(s<=NULL || s->cnt<=0)
				{
					// ----------------
					/*printf("\nVIOLATING INTERVAL\n");
					printSpanIntervalSet(s);
					printf("\nINTERPRETATION\n");
					pelPrintIterpretation(prev, nEvents);
					printf("\n~~~~~~~~~~~~~~~~~\n");*/
					// ----------------
					sisRelease(&s);
					continue;
				}

				

				// ----------------
				//printf("\n~~~~~~~~~~~~~~~~~\n");
				//printSpanIntervalSet(s);
				//printf("\n~~~~~~~~~~~~~~~~~\n");
				// ----------------

				for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next)
				{
					// ----------------
					//printf("\n~~~~~~~~~~~~~~~~~\n");
					//printSpanIntervalSet(s);
					//printf("\n~~~~~~~~~~~~~~~~~\n");
					// ----------------

					temp = pelCloneInterpretation( prev, nEvents);

					//pelPrintIterpretation(temp, nEvents);

					pelMove(si,formula, temp, nEvents, timeline);

					//pelPrintIterpretation(temp, nEvents);

					score = pelScore(formulaTrees, nFormulas, weights,
									 temp, nEvents, timeline);
					if(cur==NULL || score>curScore)
					{
						curScore = score;
						e = temp;
						temp = cur;
						cur = e;
					}

					if(temp)
						pelReleaseInterpretation(&temp, nEvents);

					//pelPrintIterpretation(cur, nEvents);
				}
				sisRelease(&s);
			}
		}

		if(!cur) continue;

		//pelPrintIterpretation(cur, nEvents);

		e = prev;
		prev = cur;
		cur = e;

		if(cur)
			pelReleaseInterpretation(&cur, nEvents);

#ifdef PEL_LOG_FILE
		pelLogInterpretation(PEL_LOG_FILE, prev, nEvents, curScore,(curScore>bestScore), 0);
#endif

		iter+= (prev>NULL);

		/* record spanning intervals */
		if(recEvents>NULL && prev)
		{
			(*recEvents)[iter] = pelCloneInterpretation( prev, nEvents);
		}

		if(curScore>bestScore)
		{
			pelReleaseInterpretation(&best, nEvents);
			best = pelCloneInterpretation( prev, nEvents);
			bestScore = curScore;

			//pelPrintIterpretation(best, nEvents);
		}

	}

	pelReleaseInterpretation(&prev, nEvents);

	/*pelPrintIterpretation(best, nEvents);
	bestScore = pelScore(formulaTrees, nFormulas, weights,
						 best,  nEvents, timeline);*/
	//printf("Best score: %lf\n",bestScore);

	for(i=0;i<nEvents;i++)
	{
		clearEvent(events+i);
		clearCopyEvent(events+i,best+i);
	}

	if(best)
		pelReleaseInterpretation(&best, nEvents);

	return iter;
}

EVENT_TYPE** pelFreeRecEvents(EVENT_TYPE** recEvents, int nRecEvents, int nEvents)
{
	int i;
	if(recEvents<=NULL) return NULL;
	for(i=0;i<nRecEvents;i++)
	{
		if(recEvents[i]>NULL)
			pelReleaseInterpretation(recEvents+i, nEvents);
	}
	free(recEvents);
	return NULL;
}

void pelPrintRecEvents(EVENT_TYPE** recEvents, int nRecEvents, int nEvents)
{
	int i;
	if(recEvents<=NULL) return;
	for(i=0;i<nRecEvents;i++)
	{
		if(recEvents[i]>NULL)
			pelPrintIterpretation(recEvents[i], nEvents);
	}
}

void pelPerturbEvents(EVENT_TYPE* events, int nEvents, 
					  double falseNegPerc, double falsePosPerc, double noisePerc)
{
	int i,j,k,n;
	int cnt=0;
	unsigned char* flag;
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET* s;
	INTERVAL timeline,I;

	typedef
	struct
	{
		int eId,siId,n;
	}_ids;

	_ids* ids,tmp;

	srand((int)time(0));

	/* find timeline */
	pelFindTimeline(events, nEvents,&timeline);

	/* init random selection */
	for(i=0;i<nEvents;i++)
		cnt += events[i].s.cnt;
	
	ids = (_ids*)malloc(cnt*sizeof(_ids));
	flag = (unsigned char*)malloc(cnt*sizeof(unsigned char));
	
	for(i=0,n=0;i<nEvents;i++)
	{
		for(j=0;j<events[i].s.cnt;j++,n++)
		{
			ids[n].eId = i;
			ids[n].siId = j;
			ids[n].n = n;
		}
	}

	/* false negative: delete files */
	for(i=0,n=0;i<nEvents;i++)
	{
		for(j=0;j<events[i].s.cnt;j++,n++)
		{
			k = rand()%(cnt-n)+n;
			tmp = ids[n];
			ids[n] = ids[k];
			ids[k] = tmp;
		}
	}
	memset(flag,0,cnt*sizeof(unsigned char));
	for(n=0;n<cnt*falseNegPerc;n++)
		flag[ids[n].n] = 1;

	for(i=0,n=0;i<nEvents;i++)
	{
		s = &events[i].s;
		for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next,n++)
		{
			if(flag[n])
			{
				si = si->prev;
				if(si!=(SPAN_INTERVAL*)lklSTail(s))
				{
					lklDisconnectAndFree(s,si->next,0);
				}
			}
		}
	}


	/* false positive: change files */
	for(i=0,n=0;i<nEvents;i++)
	{
		for(j=0;j<events[i].s.cnt;j++,n++)
		{
			k = rand()%(cnt-n)+n;
			tmp = ids[n];
			ids[n] = ids[k];
			ids[k] = tmp;
		}
	}
	memset(flag,0,cnt*sizeof(unsigned char));
	for(n=0;n<cnt*falsePosPerc;n++)
		flag[ids[n].n] = 1;
 
	for(i=0,n=0;i<nEvents;i++)
	{
		s = &events[i].s;
		for(si = (SPAN_INTERVAL*)lklHead(s);si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next,n++)
		{
			if(flag[n])
			{
				I.t1 = timeline.t1 + rand()%(abs(timeline.t2-timeline.t1));
				I.t2 = timeline.t1 + rand()%(abs(timeline.t2-timeline.t1));
				if(I.t1>I.t2)
				{
					k = I.t1;
					I.t1 = I.t2;
					I.t2 = k;
				}

				si->I1.t1 = (int)((1-noisePerc)*si->I1.t1 + noisePerc*I.t1);
				si->I1.t2 = (int)((1-noisePerc)*si->I1.t2 + noisePerc*I.t2);

				if(events[i].liquid)
					si->I2 = si->I1;
				else
				{
					I.t1 = timeline.t1 + rand()%(abs(timeline.t2-timeline.t1));
					I.t2 = timeline.t1 + rand()%(abs(timeline.t2-timeline.t1));
					if(I.t1>I.t2)
					{
						k = I.t1;
						I.t1 = I.t2;
						I.t2 = k;
					}
					si->I2.t1 = (int)((1-noisePerc)*si->I2.t1 + noisePerc*I.t1);
					si->I2.t2 = (int)((1-noisePerc)*si->I2.t2 + noisePerc*I.t2);
				}
			}
		}
	}

	free(flag);
	free(ids);
}

double pelLiquidIntersectionOverUnion(EVENT_TYPE* events1,EVENT_TYPE* events2, int nEvents)
{
	int i=0,rangeInter,rangeUnion;
	SPAN_INTERVAL_SET* sInter,*sUnion;
	double interOverUnion=0;
	double cnt=0;

	for(i=0;i<nEvents;i++)
	{
		if(events1[i].locked) continue;

		cnt++;
		sInter = sisLiquidIntersection(&events1[i].s,&events2[i].s);
		sUnion = sisLiquidUnion(&events1[i].s,&events2[i].s);

		rangeInter = sisLiquidRange(sInter);
		rangeUnion = sisLiquidRange(sUnion);
		
		interOverUnion += rangeUnion>0?(double)rangeInter/(double)rangeUnion:1;
		sisRelease(&sInter);
		sisRelease(&sUnion);
	}

	return cnt>0?interOverUnion/(double)cnt:1;
}

void pelLiquidStatistic(EVENT_TYPE* groundTruth,EVENT_TYPE* events, int nEvents,INTERVAL* timeline,
						double* precision, double* recall, double* accuracy)
{
	int i=0;
	SPAN_INTERVAL_SET* sGTPos,*sGTNeg,*sEPos,*sENeg;
	SPAN_INTERVAL_SET* sTP,*sTN,*sFP,*sFN;
	SPAN_INTERVAL_SET* stemp;
	double cnt=0;
	double tp=0,tn=0,fp=0,fn=0;
	double defaultVal = 1;

	for(i=0;i<nEvents;i++)
	{
		if(groundTruth[i].locked) continue;
		cnt++;
		
		sGTPos  = &groundTruth[i].s;
		sGTNeg = sisLiquidComplement(sGTPos,timeline);
		sEPos = &events[i].s;
		sENeg = sisLiquidComplement(sEPos,timeline);

		sTP = sisLiquidIntersection(sGTPos,sEPos);
		sTN = sisLiquidIntersection(sGTNeg,sENeg);

		stemp = sisLiquidComplement(sTP,timeline);
		sFP = sisLiquidIntersection(stemp,sEPos);
		sisRelease(&stemp);

		stemp = sisLiquidComplement(sTN,timeline);
		sFN = sisLiquidIntersection(stemp,sENeg);
		sisRelease(&stemp);

		tp += sisLiquidRange(sTP);
		tn += sisLiquidRange(sTN);
		fp += sisLiquidRange(sFP);
		fn += sisLiquidRange(sFN);

		sisRelease(&sGTNeg);
		sisRelease(&sENeg);
		sisRelease(&sTP);
		sisRelease(&sTN);
		sisRelease(&sFP);
		sisRelease(&sFN);
	}

	*precision = (tp + fp)?tp / (tp + fp):defaultVal;
	*recall = (tp + fn)?tp / (tp + fn):defaultVal;
	*accuracy = (tp + tn + fp + fn)?(tp + tn) / (tp + tn + fp + fn):defaultVal;
}

int pelCreateNoisyObservationFile(const char* input, const char* output,
								  double falseNegPerc, double falsePosPerc, double noisePerc)
{
	EVENT_TYPE* events;
	int nEvents;

	/* load file */
	pelLoadInterpretation(input, &events, &nEvents);

	/* perturb events */
	pelPerturbEvents(events, nEvents, 
					 falseNegPerc, falsePosPerc, noisePerc);

	/* save file */
	pelSaveInterpretation(output, events, nEvents);

	/* free memory */
	pelReleaseInterpretation(&events, nEvents);

	return 1;
}
unsigned char pelHasLockedExpresssion(PNODE* node, EVENT_TYPE* events, int nEvents)
{
	int id;
	if(node->op >= 0)
	{
		if(node->arg1 > NULL)
			if(pelHasLockedExpresssion(node->arg1,events, nEvents)) return 1;

		if(node->arg2 > NULL)
			if(pelHasLockedExpresssion(node->arg2,events, nEvents)) return 1;

		return 0;
	}
	else
	{
		id = pelFindId(node,events,nEvents);
		return (id>=0 && events[id].locked);
	}
}

int pelRemoveFormulas(PNODE** formulaTrees, int nFormulas, double* weights, 
				   EVENT_TYPE* events, int nEvents, 
				   double perc, unsigned char onlyConstraints)
{
	PNODE* ntmp;
	double wtmp;
	int cnt=0;
	int i,id;

	if(onlyConstraints)
	{
		for(i=0;i<nFormulas;i++)
		{
			if(pelHasLockedExpresssion(formulaTrees[i], events, nEvents))
			{
				ntmp = formulaTrees[cnt];
				wtmp = weights[cnt];
				formulaTrees[cnt] = formulaTrees[i];
				weights[cnt] = weights[i];
				formulaTrees[i] = ntmp;
				weights[i] = wtmp;
				cnt++;
			}
		}
	}

	/* mix randomly */
	for(i=cnt;i<nFormulas;i++)
	{
		id = rand()%(nFormulas-i)+i;
		ntmp = formulaTrees[id];
		wtmp = weights[id];
		formulaTrees[id] = formulaTrees[i];
		weights[id] = weights[i];
		formulaTrees[i] = ntmp;
		weights[i] = wtmp;
	}

	return (int)(cnt + (nFormulas - cnt)*(1-perc));

}
void pelPrintFormulas(PNODE** formulaTrees, int nFormulas, double* weights)
{
	int i;
	for(i=0;i<nFormulas;i++)
	{
		printf("\n\n------------------------\nFormula %d	weight: %g\n------------------------\n",i+1,weights>NULL?weights[i]:0);
		printTree(formulaTrees[i]);
	}
	printf("\n\n");
}

int pelFindNextEventToDetect(EVENT_TYPE* events, int nEvents, EVENT_TYPE** recEvents, int nRecEvents, INTERVAL* timeline, const char* ref)
{
	int idmax = -1,idmin = -1;
	int i,j,k,rangeInter;
	EVENT_TYPE* e, *enext;
	SPAN_INTERVAL_SET* sInter,*sComp;

	double** d;
	double* mean;
	double* sigma;
	double ccmax=-1e20, ccmin = 1e20,cc,cck;

	if(nRecEvents<=1) return -1;

	d = (double**)calloc(nEvents,sizeof(double*));
	mean = (double*)calloc(nEvents,sizeof(double));
	sigma = (double*)calloc(nEvents,sizeof(double));

	for(i=0;i<nEvents;i++)
	{
		if(events[i].locked) continue;
		d[i] = (double*)calloc(nRecEvents,sizeof(double));

		for(j=0;j<nRecEvents-1;)
		{
			e = recEvents[j]+i;
			j++;
			enext = recEvents[j]+i;

			sComp = sisComplement(&e->s,timeline,events[i].liquid);
			sInter = sisIntersection(sComp,&enext->s,events[i].liquid);

			rangeInter = sisGetNSubIntervals(sInter);
			
			sisRelease(&sInter);
			sisRelease(&sComp);

			sComp = sisComplement(&enext->s, timeline,events[i].liquid);
			sInter = sisIntersection(sComp,&e->s,events[i].liquid);

			rangeInter += sisGetNSubIntervals(sInter);
			
			sisRelease(&sInter);
			sisRelease(&sComp);

			d[i][j] = rangeInter;
			mean[i] += d[i][j];
		}
		mean[i] /= nRecEvents-1;
	}

	for(i=0;i<nEvents;i++)
	{
		if(!d[i]) continue;

		for(j=0;j<nRecEvents-1;j++)
		{
			d[i][j] -= mean[i];
			sigma[i] += d[i][j]*d[i][j];
		}
		sigma[i]/=nRecEvents-1;
		sigma[i] = sqrt(sigma[i]);
	}

	for(i=0;i<nEvents;i++)
	{
		if(!d[i] || !pelIsLockable(events+i,ref))continue;
		cc = 0;
		for(k=0;k<nEvents;k++)
		{
			if(!d[k] || !events[k].query)continue;

			/* cross corelation */
			cck = 0;
			for(j=0;j<nRecEvents-1;j++)
			{
				cck += d[i][j]*d[k][j];
			}
			if(sigma[i] && sigma[k]) cck /= sigma[i]*sigma[k]*(nRecEvents-1);
			cc += fabs(cck);
		}

		if(ccmax<cc)
		{
			idmax = i;
			ccmax = cc;
		}

		if(ccmin>cc)
		{
			idmin = i;
			ccmin = cc;
		}
	}

	/* free memory */
	for(i=0;i<nEvents;i++)
	{
		if(d[i]) free(d[i]);
	}

	free(d);
	free(mean);
	free(sigma);

	return idmax;
}

int pelMostCorrEvent(EVENT_TYPE* events, int nEvents, double**d, double* score , int nd, const char* ref)
{
	int idmax = -1,idmin = -1;
	int i,j,k;
	double cnt;

	double* mean;
	double* sigma;
	double ccmax=-1e20, ccmin = 1e20,*cc,cck;
	FILE* fp;
	char filename[512] = {0};
	double sumScore = 0;

	if(nd<=0) return -1;

	mean = (double*)calloc(nEvents,sizeof(double));
	sigma = (double*)calloc(nEvents,sizeof(double));
	cc = (double*)calloc(nEvents,sizeof(double));

	for(j=0;j<nd;j++) sumScore += score[j];
	for(j=0;j<nd;j++) score[j]/=sumScore;

	for(i=0;i<nEvents;i++)
	{
		if(events[i].locked) continue;

		for(j=0;j<nd;j++)
		{
			mean[i] += d[i][j];
		}
		mean[i] /= nd;
	}

	for(i=0;i<nEvents;i++)
	{
		for(j=0;j<nd;j++)
		{
			d[i][j] -= mean[i];
			/************ test **************/
			d[i][j] = fabs(d[i][j]);
			/********************************/
			sigma[i] += d[i][j]*d[i][j];
		}
		sigma[i]/=nd;
		sigma[i] = sqrt(sigma[i]);
	}

	for(i=0;i<nEvents;i++)
	{
		if( !pelIsLockable(events+i,ref))continue;
		cc[i] = 0;
		cnt = 0;
		for(k=0;k<nEvents;k++)
		{
			if(!events[k].query)continue;

			/* cross corelation */
			cck = 0;
			for(j=0;j<nd;j++)
			{
				cck += score[j]*d[i][j]*d[k][j];
				//cck += d[i][j]*d[k][j];
			}
			if(sigma[i] && sigma[k]) cck /= sigma[i]*sigma[k]*(nd);
			//cc += fabs(cck);
			cc[i] += cck;
			//cc[i] = cnt?max(cc[i],cck):cck;
			cnt++;
		}

		if(cnt) cc[i]/=cnt;

		if(ccmax<cc[i])
		{
			idmax = i;
			ccmax = cc[i];
		}

		if(ccmin>cc[i])
		{
			idmin = i;
			ccmin = cc[i];
		}
	}

	sprintf(filename,"coor_%s.xls",events[idmax].arg);
	fp = fopen(filename,"w+");

	for(i=0;i<nEvents;i++)
	{
		if( !pelIsLockable(events+i,ref))continue;
		fprintf(fp,"%s	%lf\n",events[i].arg,cc[i]);
	}

	fclose(fp);

	/* free memory */
	free(mean);
	free(sigma);
	free(cc);

	return idmax;
}
void pelInitRandEvent(EVENT_TYPE* e,INTERVAL* timeline)
{
	int i,j;
	INTERVAL I;
	int range = timeline->t2-timeline->t1;

	i = rand()%(min(10,range))+1;

	sisClear(&e->s);
	while(i--)
	{
		I.t1 = rand()%range + timeline->t1;
		j = max(min(range/2,timeline->t2-I.t1),1);
		I.t2 = rand()%j + I.t1;
		sisAdd(&e->s,siCreate(&I,&I));
	}
	sisNormalize(&e->s, e->liquid);
}