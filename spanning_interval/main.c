#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

#include <cv.h>
#include <highgui.h>

#include "spanningInterval.h"
#include "parsing.h"
#include "pel.h"
#include "utils.h"
#include "2Darray.h"

#include "mainutils.h"

void viewer( EVENT_TYPE* events, int nEvents,INTERVAL* timeline, const char* filename, unsigned char view)
{
	int i,j;
	int length=timeline->t2-timeline->t1,l;
	IplImage* img = NULL;
	CvFont font;
	int dw=0,width,dh,height;
	SPAN_INTERVAL* si;
	SPAN_INTERVAL_SET * s;

	/* max char length */
	for(i=0;i<nEvents;i++)
	{
		l = (int)strlen(events[i].arg);
		if(dw<l)dw=l;
	}

	dw=dw*7;

	/* init image and font */
	dh = 20;
	height = nEvents*dh+25;
	width = length+dw+20;
	img = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	cvZero(img);
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, CV_AA);

	/* display intervals */
	cvRectangle(img,cvPoint(0,0),cvPoint(width,height),cvScalarAll(255),-1,8,0);
	for(i=0;i<nEvents;i++)
	{
		cvRectangle(img,cvPoint(0,i*dh),cvPoint(width,(i+1)*dh),cvScalarAll(220+35*(i%2)),-1,8,0);
		cvLine(img,cvPoint(0,i*dh),cvPoint(width,i*dh),cvScalarAll(0),1,8,0);
		cvPutText(img, events[i].arg, cvPoint(2,i*dh+15), &font, events[i].query?cvScalar(255, 0, 0, 0):events[i].locked?cvScalar(0, 0, 255, 0):cvScalar(0, 0, 0, 0));
	}
	cvLine(img,cvPoint(0,nEvents*dh),cvPoint(width,nEvents*dh),cvScalarAll(0),1,8,0);
	
	for(i=0;i<nEvents;i++)
	{
		s = &events[i].s;
		for(si = (SPAN_INTERVAL*)lklHead(s),j=0;si!=(SPAN_INTERVAL*)lklSTail(s);si = si->next,j++)
		{
			cvLine(img,cvPoint(dw+si->I1.t1,i*dh+dh/2),cvPoint(dw+si->I2.t2,i*dh+dh/2),getIdCol(i+1,nEvents),5,8,0);
		}
	}

	for(i=0;i<=length-length/4;i+=length/4)
	{
		cvLine(img,cvPoint(dw-1+i,0),cvPoint(dw-1+i,nEvents*dh+5),cvScalarAll(0),1,8,0);
		j = 9*(int)(log(i>0?i:1)/log(10)+1);	j/=2; j++;
		cvPutText(img, mkStr("%d",i+timeline->t1), cvPoint(dw-j+i,nEvents*dh+18), &font, cvScalar(0, 0, 0, 0));
	}
	cvPutText(img, mkStr("%d",length+timeline->t1), cvPoint(width - 14*(int)(log(length)/log(10)+1),nEvents*dh+18), &font, cvScalar(0, 0, 0, 0));
	
	cvLine(img,cvPoint(dw-1,0),cvPoint(dw-1,nEvents*dh),cvScalarAll(0),1,8,0);

	if(view) 
	{
		cvNamedWindow("intervals", CV_WINDOW_AUTOSIZE);
		//cvMoveWindow("intervals", 2, 2);
		cvShowImage("intervals",img);
		cvWaitKey(0);
	}

	if(filename>NULL)
	{
		cvSaveImage(filename,img);
	}

	cvReleaseImage(&img);
}

/*
  *****************************************************************************
	Main function.

	@param argc number of arguments.
	@param argv array of arguments, the first argument being the name of the program.
	
	@return Returns 0 if went well.
*/
int main2( int argc, char** argv )
{
	int i;
	int nFormulas = 0;
	PNODE** formulaTrees = NULL;
	double* weights = NULL;
	EVENT_TYPE* events = NULL;
	int nEvents = 0;
	//SPAN_INTERVAL_SET* s;
	INTERVAL timeline;
	//int cnt;
	double score;
	int nIters = 1000;
	double prob = 0.5;

	if(argc<2) argv[1] = "formulas.txt";
	if(argc<3) argv[2] = "observations.txt";
	if(argc>=4) nIters = atoi(argv[3]);
	if(argc>=5) prob = atof(argv[4]);

	pelLoadFormulas(argv[1], &formulaTrees, &nFormulas, &weights);

	printf("\n~~~~~~~~~~~ FORMULAS ~~~~~~~~~~~");
	pelPrintFormulas(formulaTrees, nFormulas,weights);

	pelLoadInterpretation(argv[2], &events, &nEvents);

	printf("\n\n\n~~~~~~~~~~~ OBSERVATIONS ~~~~~~~~~~~\n");
	//pelPrintIterpretation(events, nEvents);

	pelNormaliseInterpretation(events,nEvents);
	//pelPrintIterpretation(events, nEvents);

	pelFindTimeline(events, nEvents,&timeline);
	//timeline.t2 = 10;

	/* add hidden events */
	for(i=0;i<nFormulas;i++)
		pelCompleteEventList(formulaTrees[i], &events, &nEvents,& timeline);

	/* lock observations */
	pelLockEvents(events, nEvents, "d-");

	pelPrintIterpretation(events, nEvents);

	/*s = pelSAT(formulaTrees[0], events, nEvents, &timeline);

	cnt = pelSATSize(s);*/

	score = pelScore(formulaTrees, nFormulas, weights,
					 events, nEvents, &timeline);

	/*printf("\n\n-------------------\n\n");
	printf("> Formula: ");
	printTree(formulaTrees[0]);
	printf("\n\n> Timeline\n");
	printInterval(&timeline);
	printf("\n\n> SAT\n");
	printSpanIntervalSet(s);

	printf("\n\n> |SAT|: %d\n",cnt);*/

	printf("\n\n> Initial score: %lf\n",score);

	pelInference(	formulaTrees, nFormulas, weights,
					events, nEvents, &timeline, nIters, prob,0,1);

	printf("\n\n~~~~~~~~~~~ RESULTS ~~~~~~~~~~~\n");
	printf("\n> Timeline: [%d %d]\n",timeline.t1,timeline.t2);
	pelPrintIterpretation(events, nEvents);
	score = pelScore(formulaTrees, nFormulas, weights,
					 events, nEvents, &timeline);
	printf("\n\n> Total score: %lf\n",score);

	pelSaveInterpretation(mkStr("%sinterpretation.%s",pathname(argv[2],0),extname(argv[2],0)), events, nEvents);

	printf("\nPress enter to quit");
	getchar();
	return 0;
}

int main4( int argc, char** argv )
{
	int i,cnt=0;
	int nFormulas = 0;
	PNODE** formulaTrees = NULL;
	double* weights = NULL;
	EVENT_TYPE* _events = NULL;
	EVENT_TYPE* events = NULL;
	EVENT_TYPE** recEvents = NULL;
	int nRecEvents = 0;
	int nEvents = 0;
	INTERVAL timeline;
	double score1=0, score2=0;
	int nIters = 10000;
	double prob = 0.2;
	int id;
	char buf[1024] = {0};

	if(argc<2) argv[1] = "formulas_.txt";
	if(argc<3) argv[2] = "observations_.txt";
	if(argc>=4) nIters = atoi(argv[3]);
	if(argc>=5) prob = atof(argv[4]);

	/* load formulas */
	printf("> Load formulas\n");
	pelLoadFormulas(argv[1], &formulaTrees, &nFormulas, &weights);

	/* load observations */
	printf("> Load observations\n");
	pelLoadInterpretation(argv[2], &_events, &nEvents);

	/* perturbe events */
	printf("> Perturbe events\n");
	pelPerturbEvents(_events, nEvents, 
				     0, 0.3, 1);

	/* normalize observations */
	printf("> Normalize observations\n");
	pelNormaliseInterpretation(_events,nEvents);

	/* find timeline */
	pelFindTimeline(_events, nEvents,&timeline);

	/* add hidden events */
	printf("> Add hidden events\n");
	for(i=0;i<nFormulas;i++)
		pelCompleteEventList(formulaTrees[i], &_events, &nEvents,& timeline);

	//pelPrintFormulas(formulaTrees, nFormulas,weights);

	/* augment formulas */
	printf("> Augment formulas\n");
	pelAugmentFormulas(_events, nEvents,"d-", &formulaTrees, &nFormulas, &weights);

	//pelPrintFormulas(formulaTrees, nFormulas,weights);

	/* copy observation */
	events = pelCloneInterpretation(_events, nEvents);

	/* choose query */
	srand((unsigned int)time(0));
	for(i=0;i<nEvents;i++)
	{
		if(pelIsLockable(events+i,"d-"))continue;
		//events[i].query = rand()%2;
		events[i].query |= !strcmp(events[i].arg,"passing_PlayerYellow");
	}

	/* lock observations */
	//printf("> Lock observations\n");
	//pelLockEvents(events, nEvents, "d-");
	id = 0;
	events[id].locked = 1;
	for(i=0;i<nEvents;i++)
	{
		if(!events[i].locked && pelIsLockable(events+i,"d-"))
		{
			sisClear(&events[i].s);
			//sisAdd(&events[i].s,siCreate(&timeline,&timeline));
		}
		if(!pelIsLockable(events+i,"d-") && events[i].query)
		{
			sisClear(&events[i].s);
			sisAdd(&events[i].s,siCreate(&timeline,&timeline));
		}
	}

	/* score before PEL */
	/*printf("> Score observations\n");
	score1 = pelScore(formulaTrees, nFormulas, weights,
					  events, nEvents, &timeline);*/


	sprintf(buf,"%sobservation.png",pathname(argv[1],0));
	viewer( events, nEvents,&timeline,buf,0);

	while(++cnt)
	{
		for(i=0;i<nEvents;i++)
		{
			/*if(!events[i].locked && pelIsLockable(events+i,"d-"))
			{
				//sisClear(&events[i].s);
				//sisAdd(&events[i].s,siCreate(&timeline,&timeline));
			}
			if(!pelIsLockable(events+i,"d-"))
			{
				//sisClear(&events[i].s);
				//sisAdd(&events[i].s,siCreate(&timeline,&timeline));
			}*/
		}

		/* PEL */
		printf("> PEL\n");
		nRecEvents = pelInference(	formulaTrees, nFormulas, weights,
									events, nEvents, &timeline, nIters, prob,
									&recEvents,1);
		nRecEvents++;

		//pelPrintIterpretation(events, nEvents);
		sprintf(buf,"%sobservation_%i_%s_chosen.png",pathname(argv[1],0),cnt,events[id].arg);
		viewer( events, nEvents,&timeline,buf,0);

		/* get next event to detect */
		id = pelFindNextEventToDetect(events, nEvents, recEvents, nRecEvents,&timeline, "d-");
		//pelPrintRecEvents( recEvents, nRecEvents, nEvents);

		/* free recorded events */
		recEvents = pelFreeRecEvents(recEvents, nRecEvents, nEvents);

		if(id<0) break;
		events[id].locked = 1;
		sisClear(&events[id].s);
		sisCopy(&events[id].s, &_events[id].s);

		printf("\nnext detector is %s\n",events[id].arg);
	}

	/* score after PEL */
	/*printf("> Score PEL\n");
	score2 = pelScore(formulaTrees, nFormulas, weights,
					 events, nEvents, &timeline);*/

	/* free memory */
	pelReleaseInterpretation(&_events, nEvents);
	pelReleaseInterpretation(&events, nEvents);

	return 0;
}

int main5( int argc, char** argv )
{
	int i,cnt=0;
	int nFormulas = 0;
	PNODE** formulaTrees = NULL;
	double* weights = NULL;
	EVENT_TYPE* _events = NULL;
	EVENT_TYPE* events = NULL;
	EVENT_TYPE* events2 = NULL;
	EVENT_TYPE** recEvents = NULL;
	int nRecEvents = 0;
	int nEvents = 0;
	INTERVAL timeline;
	double score1=0, score2=0;
	int nIters = 10000;
	double prob = 0.2;
	char buf[1024] = {0};

	if(argc<2) argv[1] = "formulas_.txt";
	if(argc<3) argv[2] = "observations_.txt";
	if(argc>=4) nIters = atoi(argv[3]);
	if(argc>=5) prob = atof(argv[4]);

	/* load formulas */
	printf("> Load formulas\n");
	pelLoadFormulas(argv[1], &formulaTrees, &nFormulas, &weights);

	/* load observations */
	printf("> Load observations\n");
	pelLoadInterpretation(argv[2], &_events, &nEvents);

	/* perturbe events */
	printf("> Perturbe events\n");
	pelPerturbEvents(_events, nEvents, 
				     0, 0.3, 1);

	/* normalize observations */
	printf("> Normalize observations\n");
	pelNormaliseInterpretation(_events,nEvents);

	/* find timeline */
	pelFindTimeline(_events, nEvents,&timeline);

	/* add hidden events */
	printf("> Add hidden events\n");
	for(i=0;i<nFormulas;i++)
		pelCompleteEventList(formulaTrees[i], &_events, &nEvents,& timeline);

	//pelPrintFormulas(formulaTrees, nFormulas,weights);

	/* augment formulas */
	printf("> Augment formulas\n");
	pelAugmentFormulas(_events, nEvents,"d-", &formulaTrees, &nFormulas, &weights);

	//pelPrintFormulas(formulaTrees, nFormulas,weights);

	/* copy observation */
	events = pelCloneInterpretation(_events, nEvents);
	events2 = pelCloneInterpretation(_events, nEvents);

	/* choose query */
	srand((unsigned int)time(0));
	for(i=0;i<nEvents;i++)
	{
		if(pelIsLockable(events+i,"d-"))continue;
		//events[i].query = rand()%2;
		events[i].query |= !strcmp(events[i].arg,"passing_PlayerYellow");
	}

	/* lock observations */
	//printf("> Lock observations\n");
	//pelLockEvents(events, nEvents, "d-");

	/* init random */
	srand((unsigned int)time(0));
	for(i=0;i<nEvents;i++)
	{
		if(events[i].locked) continue;
		pelInitRandEvent(events+i, &timeline);
	}

	pelCopyInterpretation(events2, events, nEvents);

	sprintf(buf,"%sobservation.png",pathname(argv[1],0));
	viewer( events, nEvents,&timeline,buf,0);

	/* PEL */
	printf("> PEL\n");
	startTiming();
	pelInference(	formulaTrees, nFormulas, weights,
					events, nEvents, &timeline, nIters, prob,
					0,1);
	printTime(endTiming());

	/* view PEL results */
	sprintf(buf,"%sinterpretation.png",pathname(argv[1],0));
	viewer( events, nEvents,&timeline,buf,0);

	/* PEL full inference */
	printf("> PEL full inference\n");
	startTiming();
	pelInference(	formulaTrees, nFormulas, weights,
					events2, nEvents, &timeline, nIters, prob,
					0,0);
	printTime(endTiming());

	/* view PEL results */
	sprintf(buf,"%sinterpretation_full_inference.png",pathname(argv[1],0));
	viewer( events2, nEvents,&timeline,buf,0);

	printf("%d formulas, %d events, %d frames\n", nFormulas,nEvents,timeline.t2-timeline.t1+1);

	/* free memory */
	pelReleaseInterpretation(&_events, nEvents);
	pelReleaseInterpretation(&events, nEvents);
	pelReleaseInterpretation(&events2, nEvents);

	printf("DONE");
	getchar();

	return 0;
}

int main( int argc, char** argv )
{
	int i,j,k,l,id=0,cnt=0;
	int nFormulas = 0;
	PNODE** formulaTrees = NULL;
	double* weights = NULL;
	EVENT_TYPE* _events = NULL;
	EVENT_TYPE* events = NULL;
	EVENT_TYPE* events2 = NULL;
	EVENT_TYPE** recEvents = NULL;
	int nRecEvents = 0;
	int nEvents = 0;
	INTERVAL timeline;
	double score1=0, score2=0;
	int nIters = 10000;
	double prob = 0.2;
	char buf[1024] = {0};
	int nd1 = 5, nd2 = 5;
	int nd = nd1 * nd2;
	double ** d = NULL;
	double* score = NULL;

	if(argc<2) argv[1] = "formulas_.txt";
	if(argc<3) argv[2] = "observations_.txt";
	if(argc>=4) nIters = atoi(argv[3]);
	if(argc>=5) prob = atof(argv[4]);

	/* load formulas */
	printf("> Load formulas\n");
	pelLoadFormulas(argv[1], &formulaTrees, &nFormulas, &weights);

	/* load observations */
	printf("> Load observations\n");
	pelLoadInterpretation(argv[2], &_events, &nEvents);

	/* perturbe events */
	printf("> Perturbe events\n");
	pelPerturbEvents(_events, nEvents, 
				     0, 0.3, 1);

	/* normalize observations */
	printf("> Normalize observations\n");
	pelNormaliseInterpretation(_events,nEvents);

	/* find timeline */
	pelFindTimeline(_events, nEvents,&timeline);

	/* add hidden events */
	printf("> Add hidden events\n");
	for(i=0;i<nFormulas;i++)
		pelCompleteEventList(formulaTrees[i], &_events, &nEvents,& timeline);

	//pelPrintFormulas(formulaTrees, nFormulas,weights);

	/* augment formulas */
	printf("> Augment formulas\n");
	pelAugmentFormulas(_events, nEvents,"d-", &formulaTrees, &nFormulas, &weights);

	//pelPrintFormulas(formulaTrees, nFormulas,weights);

	/* choose query */
	srand((unsigned int)time(0));
	for(i=0;i<nEvents;i++)
	{
		if(pelIsLockable(_events+i,"d-"))continue;
		//events[i].query = rand()%2;
		_events[i].query |= !strcmp(_events[i].arg,"passing_PlayerYellow");
		_events[i].query |= !strcmp(_events[i].arg,"dribbling_PlayerYellow");
		/*_events[i].query |= !strcmp(_events[i].arg,"dribbling_PlayerRed");
		_events[i].query |= !strcmp(_events[i].arg,"jumping_PlayerBlack");
		_events[i].query |= !strcmp(_events[i].arg,"hasBall_PlayerGreen");*/
	}

	/* copy observation */
	events = pelCloneInterpretation(_events, nEvents);
	events2 = pelCloneInterpretation(_events, nEvents);

	/* alloc memory */
	d = (double**)create2Darr(nEvents, nd, sizeof(double));
	score = (double*)malloc(nd*sizeof(double));

	/* lock observations */
	//printf("> Lock observations\n");
	//pelLockEvents(events, nEvents, "d-");

	while(id>=0)
	{
		if(cnt)
			sprintf(buf,"%s%d-%s.png",pathname(argv[1],0),cnt,_events[id].arg);
		else
			sprintf(buf,"%sinit_config.png",pathname(argv[1],0));
		viewer( events, nEvents,&timeline,buf,0);
		startTiming();
		for(j=0;j<nd1;j++)
		{
			/* init random */
			k=0;
			srand((unsigned int)time(0));
			for(i=0;i<nEvents;i++)
			{
				if(events[i].locked) continue;
				pelInitRandEvent(events+i, &timeline);
			}

			//sprintf(buf,"%sobservation-%d.png",pathname(argv[1],0),j);
			//viewer( events, nEvents,&timeline,buf,0);

			pelCopyInterpretation(events2, events, nEvents);

			/* PEL */
			//printf("> PEL\n");
			//startTiming();
			pelInference(	formulaTrees, nFormulas, weights,
							events, nEvents, &timeline, nIters, prob,
							0,1);
			//printTime(endTiming());

			/* lock queries */
			for(i=0;i<nEvents;i++)
				if(events[i].query) events[i].locked = 1;	

			/* view PEL results */
			//sprintf(buf,"%sinterpretation-%d.png",pathname(argv[1],0),k);
			//viewer( events, nEvents,&timeline,buf,0);

			/* compute change */
			l = j*nd2+k;
			for(i=0;i<nEvents;i++)
				d[i][l] = sisCompare(&events[i].s, &events2[i].s, &timeline, events[i].liquid);

			score[l] = pelScore(formulaTrees, nFormulas,  weights, events, nEvents, &timeline);

			for(k=1;k<nd2;k++)
			{
				pelCopyInterpretation(events2, events, nEvents);
				for(i=0;i<nEvents;i++)
				{
					if(events2[i].query)
					{
						pelInitRandEvent(events2+i, &timeline);
					}
				}
				//sprintf(buf,"%sobservation-%d.png",pathname(argv[1],0),k);
				//viewer( events2, nEvents,&timeline,buf,0);

				/* PEL */
				//printf("> PEL\n");
				//startTiming();
				pelInference(	formulaTrees, nFormulas, weights,
								events2, nEvents, &timeline, nIters, prob,
								0,1);
				//printTime(endTiming());

				/* view PEL results */
				//sprintf(buf,"%sinterpretation-%d.png",pathname(argv[1],0),k);
				//viewer( events2, nEvents,&timeline,buf,0);

				/* compute change */
				l = j*nd2+k;
				for(i=0;i<nEvents;i++)
					d[i][l] = sisCompare(&events[i].s, &events2[i].s, &timeline, events[i].liquid);
				score[l] = pelScore(formulaTrees, nFormulas,  weights, events2, nEvents, &timeline);
			}

			/* unlock queries */
			for(i=0;i<nEvents;i++)
				if(events[i].query) events[i].locked = 0;
		}
		id =  pelMostCorrEvent(events, nEvents, d, score ,  nd1*nd2, "d-");
		if(id>=0) 
		{
			printTime(endTiming());
			_events[id].locked = 1;
			printf("%s chosen\n",_events[id].arg);
		}
		pelCopyInterpretation(events, _events, nEvents);
	}

	printf("%d formulas, %d events, %d frames\n", nFormulas,nEvents,timeline.t2-timeline.t1+1);

	/* free memory */
	pelReleaseInterpretation(&_events, nEvents);
	pelReleaseInterpretation(&events, nEvents);
	pelReleaseInterpretation(&events2, nEvents);
	free2Darr(d);

	printf("DONE");
	getchar();

	return 0;
}

int main3( int argc, char** argv )
{
	int i;
	int nFormulas = 0;
	PNODE** formulaTrees = NULL;
	double* weights = NULL;
	EVENT_TYPE* events = NULL;
	int nEvents = 0;
	EVENT_TYPE* events2 = NULL;
	int nEvents2 = 0;
	EVENT_TYPE* ref = NULL;
	INTERVAL timeline;
	double score1, score2,scoreRef;
	int nIters = 10000;
	double prob = 0.2;
	double perc;
	double interOverUnion,precision,recall,accuracy;
	int nFormulas2=0,cnt=0;
	FILE* fp;

	if(argc<2) argv[1] = "formulas.txt";
	if(argc<3) argv[2] = "observations.txt";
	if(argc>=4) nIters = atoi(argv[3]);
	if(argc>=5) prob = atof(argv[4]);

	/* load formulas */
	pelLoadFormulas(argv[1], &formulaTrees, &nFormulas, &weights);

	/* load observations */
	pelLoadInterpretation(argv[2], &events, &nEvents);

	/* normalize observations */
	pelNormaliseInterpretation(events,nEvents);

	/* find timeline */
	pelFindTimeline(events, nEvents,&timeline);


	/* ------------------------------------------------------------------------------------------ */
	/*                      test sensitivity with respect false negatif                           */
	/* ------------------------------------------------------------------------------------------ */
#if 1
	/* open file */
	fp = fopen("false_negatif.xls","w+");
	fprintf(fp,"Perc.	Score Obs.	Score Interp.	Seg Acc.	Prec.	Rec.	Acc.\n");

	for(perc=0;perc<=1;perc+=0.1)
	{
		printf("> Clone\n");
		nEvents2 = nEvents;
		events2 = pelCloneInterpretation( events, nEvents2);
		
		/* perturbe events */
		printf("> Perturbe events\n");
		pelPerturbEvents(events2, nEvents2, 
					     perc, 0, 0);

		/* add hidden events */
		printf("> Add hidden events\n");
		for(i=0;i<nFormulas;i++)
			pelCompleteEventList(formulaTrees[i], &events2, &nEvents2,& timeline);

		/* lock observations */
		printf("> Lock observations\n");
		pelLockEvents(events2, nEvents2, "d-");
		
		/* normalize observations */
		printf("> Normalize observations\n");
		pelNormaliseInterpretation(events2,nEvents2);

		/* score before PEL */
		printf("> Score observations\n");
		score1 = pelScore(formulaTrees, nFormulas, weights,
						 events2, nEvents2, &timeline);

		/* PEL */
		printf("> PEL\n");
		pelInference(	formulaTrees, nFormulas, weights,
						events2, nEvents2, &timeline, nIters, prob,0,1);

		/* score after PEL */
		printf("> Score PEL\n");
		score2 = pelScore(formulaTrees, nFormulas, weights,
						 events2, nEvents2, &timeline);

		if(!perc)
		{
			scoreRef = score2;
			if(scoreRef<=0) scoreRef = 1;
			ref = pelCloneInterpretation( events2, nEvents2);
		}

		/* display */
		score1/=scoreRef;
		score2/=scoreRef;

		interOverUnion = pelLiquidIntersectionOverUnion(ref, events2, nEvents2);

		pelLiquidStatistic(	ref,events2, nEvents2, &timeline,
							&precision, &recall, &accuracy);

		printf("perc: %.2g score: %.4g	->	%.4g	|	%.4g	%.4g	%.4g	%.4g\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);
		fprintf(fp,"%lf	%lf	%lf	%lf	%lf	%lf	%lf\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);

		/* free memory */
		pelReleaseInterpretation(&events2, nEvents2);
	}

	/* free memory */
	pelReleaseInterpretation(&ref, nEvents2);

	/* close file */
	fclose(fp);

	/* ------------------------------------------------------------------------------------------ */
#endif

	/* ------------------------------------------------------------------------------------------ */
	/*                      test sensitivity with respect false positif                           */
	/* ------------------------------------------------------------------------------------------ */
#if 0
	/* open file */
	fp = fopen("false_positif.xls","w+");
	fprintf(fp,"Perc.	Score Obs.	Score Interp.	Seg Acc.	Prec.	Rec.	Acc.\n");

	for(perc=0;perc<=1;perc+=0.1)
	{
		printf("> Clone\n");
		nEvents2 = nEvents;
		events2 = pelCloneInterpretation( events, nEvents2);
		
		/* perturbe events */
		printf("> Perturbe events\n");
		pelPerturbEvents(events2, nEvents2, 
					     0, perc, 1);

		/* add hidden events */
		printf("> Add hidden events\n");
		for(i=0;i<nFormulas;i++)
			pelCompleteEventList(formulaTrees[i], &events2, &nEvents2,& timeline);

		/* lock observations */
		printf("> Lock observations\n");
		pelLockEvents(events2, nEvents2, "d-");
		
		/* normalize observations */
		printf("> Normalize observations\n");
		pelNormaliseInterpretation(events2,nEvents2);

		/* score before PEL */
		printf("> Score observations\n");
		score1 = pelScore(formulaTrees, nFormulas, weights,
						 events2, nEvents2, &timeline);

		/* PEL */
		printf("> PEL\n");
		pelInference(	formulaTrees, nFormulas, weights,
						events2, nEvents2, &timeline, nIters, prob,0);

		/* score after PEL */
		printf("> Score PEL\n");
		score2 = pelScore(formulaTrees, nFormulas, weights,
						 events2, nEvents2, &timeline);

		if(!perc)
		{
			scoreRef = score2;
			if(scoreRef<=0) scoreRef = 1;
			ref = pelCloneInterpretation( events2, nEvents2);
		}

		/* display */
		score1/=scoreRef;
		score2/=scoreRef;

		interOverUnion = pelLiquidIntersectionOverUnion(ref, events2, nEvents2);
		pelLiquidStatistic(	ref,events2, nEvents2, &timeline,
							&precision, &recall, &accuracy);

		printf("perc: %.2g score: %.4g	->	%.4g	|	%.4g	%.4g	%.4g	%.4g\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);
		fprintf(fp,"%lf	%lf	%lf	%lf	%lf	%lf	%lf\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);

		/* free memory */
		pelReleaseInterpretation(&events2, nEvents2);
	}

	/* free memory */
	pelReleaseInterpretation(&ref, nEvents2);

	/* close file */
	fclose(fp);

	/* ------------------------------------------------------------------------------------------ */
#endif

	/* ------------------------------------------------------------------------------------------ */
	/*                           test sensitivity with respect noise                              */
	/* ------------------------------------------------------------------------------------------ */
#if 0
	/* open file */
	fp = fopen("noise.xls","w+");
	fprintf(fp,"Perc.	Score Obs.	Score Interp.	Seg Acc.	Prec.	Rec.	Acc.\n");

	for(perc=0;perc<=1;perc+=0.1)
	{
		printf("> Clone\n");
		nEvents2 = nEvents;
		events2 = pelCloneInterpretation( events, nEvents2);
		
		/* perturbe events */
		printf("> Perturbe events\n");
		pelPerturbEvents(events2, nEvents2, 
					     0, 1, perc);

		/* add hidden events */
		printf("> Add hidden events\n");
		for(i=0;i<nFormulas;i++)
			pelCompleteEventList(formulaTrees[i], &events2, &nEvents2,& timeline);

		/* lock observations */
		printf("> Lock observations\n");
		pelLockEvents(events2, nEvents2, "d-");
		
		/* normalize observations */
		printf("> Normalize observations\n");
		pelNormaliseInterpretation(events2,nEvents2);

		/* score before PEL */
		printf("> Score observations\n");
		score1 = pelScore(formulaTrees, nFormulas, weights,
						 events2, nEvents2, &timeline);

		/* PEL */
		printf("> PEL\n");
		pelInference(	formulaTrees, nFormulas, weights,
						events2, nEvents2, &timeline, nIters, prob,0);

		/* score after PEL */
		printf("> Score PEL\n");
		score2 = pelScore(formulaTrees, nFormulas, weights,
						 events2, nEvents2, &timeline);

		if(!perc)
		{
			scoreRef = score2;
			if(scoreRef<=0) scoreRef = 1;
			ref = pelCloneInterpretation( events2, nEvents2);
		}

		/* display */
		score1/=scoreRef;
		score2/=scoreRef;

		interOverUnion = pelLiquidIntersectionOverUnion(ref, events2, nEvents2);

		pelLiquidStatistic(	ref,events2, nEvents2, &timeline,
							&precision, &recall, &accuracy);

		printf("perc: %.2g score: %.4g	->	%.4g	|	%.4g	%.4g	%.4g	%.4g\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);
		fprintf(fp,"%lf	%lf	%lf	%lf	%lf	%lf	%lf\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);

		/* free memory */
		pelReleaseInterpretation(&events2, nEvents2);
	}

	/* free memory */
	pelReleaseInterpretation(&ref, nEvents2);

	/* close file */
	fclose(fp);

	/* ------------------------------------------------------------------------------------------ */
#endif

	/* ------------------------------------------------------------------------------------------ */
	/*                           test sensitivity with respect formulas                           */
	/* ------------------------------------------------------------------------------------------ */
#if 0
	/* open file */
	fp = fopen("formulas.xls","w+");
	fprintf(fp,"Perc.	Score Obs.	Score Interp.	Seg Acc.	Prec.	Rec.	Acc.\n");

	/* add hidden events */
	printf("> Add hidden events\n");
	for(i=0;i<nFormulas;i++)
		pelCompleteEventList(formulaTrees[i], &events, &nEvents,& timeline);

	/* lock observations */
	printf("> Lock observations\n");
	pelLockEvents(events, nEvents, "d-");
	
	/* normalize observations */
	printf("> Normalize observations\n");
	pelNormaliseInterpretation(events,nEvents);

	/* score before PEL */
	printf("> Score observations\n");
	score1 = pelScore(formulaTrees, nFormulas, weights,
					  events, nEvents, &timeline);

	/* PEL */
	printf("> PEL\n");
	pelInference(	formulaTrees, nFormulas, weights,
					events, nEvents, &timeline, nIters, prob,0);

	/* score after PEL */
	printf("> Score PEL\n");
	score2 = pelScore(formulaTrees, nFormulas, weights,
					  events, nEvents, &timeline);

	scoreRef = score2;

	/* display */
	score1/=scoreRef;
	score2/=scoreRef;

	interOverUnion = pelLiquidIntersectionOverUnion(events, events, nEvents);

	pelLiquidStatistic(	events,events, nEvents, &timeline,
						&precision, &recall, &accuracy);

	//printf("perc: %.2g score: %.4g	->	%.4g	|	%.4g	%.4g	%.4g	%.4g\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);
	//fprintf(fp,"%lf	%lf	%lf	%lf	%lf	%lf	%lf\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);

	printf("> Clone\n");
	ref = pelCloneInterpretation( events, nEvents);

	cnt = 0;
	for(i=0;i<nEvents;i++)
	{
		if(!events[i].locked)
			sisClear(&events[i].s);
		else
			cnt++;
	}

	/* perturbe events */
	printf("> Perturbe events\n");
	pelPerturbEvents(events, cnt, 
				     0.3, 0.7, 0.5);

	for(perc=0;perc<1;perc+=0.1)
	{
		printf("> Clone\n");
		nEvents2 = nEvents;
		events2 = pelCloneInterpretation( events, nEvents2);

		/* add hidden events */
		printf("> Add hidden events\n");
		for(i=0;i<nFormulas;i++)
			pelCompleteEventList(formulaTrees[i], &events2, &nEvents2,& timeline);

		/* lock observations */
		printf("> Lock observations\n");
		pelLockEvents(events2, nEvents2, "d-");
		
		/* normalize observations */
		printf("> Normalize observations\n");
		pelNormaliseInterpretation(events2,nEvents2);

		nFormulas2 = pelRemoveFormulas(formulaTrees, nFormulas,  weights,  events,  nEvents, perc, 1);
		if(nFormulas2<=0)nFormulas2=1;

		/* score before PEL */
		printf("> Score observations\n");
		score1 = pelScore(formulaTrees, nFormulas2, weights,
						 events2, nEvents2, &timeline);

		/* PEL */
		printf("> PEL\n");
		pelInference(	formulaTrees, nFormulas2, weights,
						events2, nEvents2, &timeline, nIters, prob,0);

		/* score after PEL */
		printf("> Score PEL\n");
		score2 = pelScore(formulaTrees, nFormulas2, weights,
						 events2, nEvents2, &timeline);

		/* display */
		score1/=scoreRef;
		score2/=scoreRef;

		interOverUnion = pelLiquidIntersectionOverUnion(ref, events2, nEvents2);

		pelLiquidStatistic(	ref,events2, nEvents2, &timeline,
							&precision, &recall, &accuracy);

		printf("perc: %.2g score: %.4g	->	%.4g	|	%.4g	%.4g	%.4g	%.4g\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);
		fprintf(fp,"%lf	%lf	%lf	%lf	%lf	%lf	%lf\n",perc,score1,score2,interOverUnion,precision,recall,accuracy);

		/* free memory */
		pelReleaseInterpretation(&events2, nEvents2);
	}
#endif

	/* free memory */
	pelReleaseInterpretation(&ref, nEvents);

	/* close file */
	fclose(fp);


	/* free memory */
	pelReleaseInterpretation(&events, nEvents);

	getchar();

	return 0;
}