#ifndef __PEL_H__
#define __PEL_H__

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include "parsing.h"

#define pelSAT parsingTree
#define pelSATSize sisGetNSubIntervals

int pelLoadFormulas(const char* filename, PNODE*** formulaTrees, int* nFormulas, double** weights);
int pelLoadInterpretation(const char* filename, EVENT_TYPE** events, int* nEvents);
int pelSaveInterpretation(const char* filename, EVENT_TYPE* events, int nEvents);
void pelNormaliseInterpretation(EVENT_TYPE* events, int nEvents);
void pelPrintIterpretation(EVENT_TYPE* events, int nEvents);
void pelFindTimeline(EVENT_TYPE* events, int nEvents,INTERVAL* timeline);
void pelCompleteEventList(PNODE* node, EVENT_TYPE** events, int* nEvents,INTERVAL* timeline);
void pelLockEvents(EVENT_TYPE* events, int nEvents, const char* ref);
unsigned char pelIsLockable(EVENT_TYPE* e, const char* ref);
int pelAugmentFormulas(EVENT_TYPE* events, int nEvents, const char* ref,  
					   PNODE*** formulaTrees, int* nFormulas, double** weights);

SPAN_INTERVAL_SET* pelNOTSAT(PNODE* node, EVENT_TYPE* events, int nEvents,INTERVAL* timeline);

double pelScore(PNODE** formulaTrees, int nFormulas, double* weights,
				EVENT_TYPE* events, int nEvents,INTERVAL* timeline);

EVENT_TYPE* pelCloneInterpretation(EVENT_TYPE* events, int nEvents);
EVENT_TYPE* pelCopyInterpretation(EVENT_TYPE*dst, EVENT_TYPE* src, int nEvents);
void pelReleaseInterpretation(EVENT_TYPE** events, int nEvents);


int pelFindId(PNODE* node,EVENT_TYPE* events, int nEvents);
int pelTree(SPAN_INTERVAL* si,PNODE* node, EVENT_TYPE* events, int nEvents, INTERVAL* timeline, unsigned char notFlag, int* move);
void pelMove(SPAN_INTERVAL* si,PNODE* formula, EVENT_TYPE* events, int nEvents, INTERVAL* timeline);
int pelInference(	PNODE** formulaTrees, int nFormulas, double* weights,
					EVENT_TYPE* events, int nEvents,INTERVAL* timeline, int maxIters, double prob,
					EVENT_TYPE*** recEvents, unsigned char flag);

EVENT_TYPE** pelFreeRecEvents(EVENT_TYPE** recEvents, int nRecEvents, int nEvents);
void pelPrintRecEvents(EVENT_TYPE** recEvents, int nRecEvents, int nEvents);

void pelPerturbEvents(EVENT_TYPE* events, int nEvents, 
					  double falseNegPerc, double falsePosPerc, double noisePerc);

int pelCreateNoisyObservationFile(const char* input, const char* output,
								  double falseNegPerc, double falsePosPerc, double noisePerc);

double pelLiquidIntersectionOverUnion(EVENT_TYPE* events1, EVENT_TYPE* events2, int nEvents);
void pelLiquidStatistic(EVENT_TYPE* groundTruth,EVENT_TYPE* events, int nEvents,INTERVAL* timeline,
						double* precision, double* recall, double* accuracy);

unsigned char pelHasLockedExpresssion(PNODE* node, EVENT_TYPE* events, int nEvents);
int pelRemoveFormulas(PNODE** formulaTrees, int nFormulas, double* weights, 
				   EVENT_TYPE* events, int nEvents, 
				   double perc, unsigned char onlyConstraints);

void pelPrintFormulas(PNODE** formulaTrees, int nFormulas, double* weights);

int pelFindNextEventToDetect(EVENT_TYPE* events, int nEvents, EVENT_TYPE** recEvents, int nRecEvents, INTERVAL* timeline, const char* ref);
int pelMostCorrEvent(EVENT_TYPE* events, int nEvents, double**d, double* score , int nd, const char* ref);
void pelInitRandEvent(EVENT_TYPE* e,INTERVAL* timeline);

#endif