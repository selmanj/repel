#ifndef __MAIN_UTILS_H__
#define __MAIN_UTILS_H__

#define _CRT_SECURE_NO_WARNINGS

#define MAT_TYPE double

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

#ifndef inline
#define inline __inline
#endif

CvScalar getIdCol(int id,int maxId);

MAT_TYPE* convertIplImage2Type(IplImage* img,int channel,MAT_TYPE* arr);
void convertIplImage2Types(IplImage* img,MAT_TYPE* arr1,MAT_TYPE* arr2,MAT_TYPE* arr3);
IplImage* convertType2IplImage(MAT_TYPE* img, int width, int height, IplImage* dst);

IplImage* BGR2HSV(IplImage* bgr,IplImage* hsv);
IplImage* RGB2HSV(IplImage* rgb,IplImage* hsv);
IplImage* cvMedian(IplImage* src,IplImage* dst, int radius);

void swapMem(void** mem1, void** mem2);

extern CvRect _selection;
extern unsigned char _selecting;
extern unsigned char _selected;
extern CvPoint _origin;
void onMouse( int event, int x, int y, int flags, void* param );
int getMouseSelection(CvRect* selection);
void resetMouseSelection();
int isMouseSelection();
int isMouseSelecting();
int drawMouseSelection(IplImage* img);

#endif