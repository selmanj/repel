#include "mainutils.h"

/*
  *****************************************************************************
	Get a random color
*/
CvScalar* _col = NULL;
int _ncol = 0;
CvScalar getIdCol(int id,int maxId)
{
	int i;
	double r,g,b,c;
	//maxId--;
	//id++;
	if(_ncol<maxId+1)
	{
		srand(1100);
		_col = (CvScalar*)realloc(_col,(maxId+1)*sizeof(CvScalar));
		for(i=_ncol;i<maxId+1;i++)
		{
			c = (0.99*(double)(rand())/(double)(RAND_MAX) + 0.01) * (256.*256.*256.);
			r = (int)(c/(255.*255.));
			g = (int)((c-r*255*255)/255.);
			b = (int)(c - r*255.*255. - g*255.);
			_col[i] = CV_RGB(((int)r+255)%255 ,((int)g+255)%255,((int)b+255)%255);
		}
		_ncol=maxId+1;
	}

	//return CV_RGB(63*(id%4 + 1),85*(id%3+1),255*(id%2));
	//return CV_RGB(63*(id%4 + 1),63*(id%5),30*(id%7)+1);
	return id>0?_col[id]:CV_RGB(0,0,0);
}



/*
  *****************************************************************************
	Convert IplImage data into an array
	
	@param img input IplImage
	@param channel which channel to convert
	@param arr adress of the output array (will be allocated inside if <= NULL)

	@return Returns the adress of the output array.
*/
MAT_TYPE* convertIplImage2Type(IplImage* img,int channel,MAT_TYPE* arr)
{
	int i,j;
	MAT_TYPE* it;
	if(arr<=NULL) arr = (MAT_TYPE*)malloc(img->width * img->height*sizeof(MAT_TYPE));
	it = arr;

	for(i=0;i<img->height;i++)
	{
		for(j=0;j<img->width;j++)
		{
			*it++ = (MAT_TYPE)(cvGet2D(img,i,j)).val[channel]/(MAT_TYPE)255;
		}
	}
	
	return arr;
}
/*
  *****************************************************************************
	Convert IplImage data into arrays
	
	@param img input IplImage
	@param channel which channel to convert
	@param arr1 adress of the output array for the first channel
	@param arr2 adress of the output array for the second channel
	@param arr3 adress of the output array for the third channel
*/
void convertIplImage2Types(IplImage* img,MAT_TYPE* arr1,MAT_TYPE* arr2,MAT_TYPE* arr3)
{
	int i,j;
	MAT_TYPE* it1=arr1,*it2=arr2,*it3=arr3;
	CvScalar col;

	for(i=0;i<img->height;i++)
	{
		for(j=0;j<img->width;j++)
		{
			col = cvGet2D(img,i,j);
			if(arr1) *it1++ = col.val[0]/(MAT_TYPE)255;
			if(arr2) *it2++ = col.val[1]/(MAT_TYPE)255;
			if(arr3) *it3++ = col.val[2]/(MAT_TYPE)255;
		}
	}
}
/*
  *****************************************************************************
	Convert an array into IplImage
	
	@param img input array
	@param width image width
	@param height image height
	@param dst adress of the output IplImage (will be allocated inside if <= NULL)

	@return Returns the adress of the output IplImage.
*/
IplImage* convertType2IplImage(MAT_TYPE* img, int width, int height, IplImage* dst)
{
	int i,j;
	IplImage* _dst = dst>NULL?dst:cvCreateImage( cvSize(width,height), IPL_DEPTH_8U, 1 );
	MAT_TYPE* it = img;
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++,it++)
		{
			cvSet2D(_dst,i,j,cvScalarAll(*it * (MAT_TYPE)255));
		}
	}
	return _dst;
}

/*
  *****************************************************************************
	Convert the BGR color space into a modified HSV color space
	
	@param bgr input image
	@param hsv output image (will be allocated inside if <= NULL)

	@return Returns the output image.
*/

IplImage* BGR2HSV(IplImage* bgr,IplImage* hsv)
{
	#ifndef M_2PI_180
	#define M_2PI_180 0.034906585039886591538473815369772
	#endif

	int i,j;
	CvScalar col,col2;

	if(hsv<=NULL) hsv = cvCreateImage( cvSize(bgr->width,bgr->height), IPL_DEPTH_8U, 3 );
	cvCvtColor( bgr, hsv, CV_BGR2HSV );
	for(i=0;i<bgr->height;i++)
	{
		for(j=0;j<bgr->width;j++)
		{
			col = cvGet2D(hsv,i,j);
			col.val[0] *= M_2PI_180;
			col.val[2] = 255-col.val[2];
			col.val[1] = col.val[1]/255;
			
			col2.val[2] = sqrt(255*(1-col.val[1])*col.val[2]);
			col2.val[0] = (col.val[1]*cos(col.val[0])+1)*127.5;
			col2.val[1] = (col.val[1]*sin(col.val[0])+1)*127.5;
			//col2.val[2] = (1-col.val[1])*col.val[2]/2;

			cvSet2D(hsv,i,j,col2);
		}
	}/**/

	return hsv;
}

/*
  *****************************************************************************
	Convert the RGB color space into a modified HSV color space
	
	@param bgr input image
	@param hsv output image (will be allocated inside if <= NULL)

	@return Returns the output image.
*/
IplImage* RGB2HSV(IplImage* rgb,IplImage* hsv)
{
	#ifndef M_2PI_180
	#define M_2PI_180 0.034906585039886591538473815369772
	#endif

	int i,j;
	CvScalar col,col2;

	if(hsv<=NULL) hsv = cvCreateImage( cvSize(rgb->width,rgb->height), IPL_DEPTH_8U, 3 );
	cvCvtColor( rgb, hsv, CV_RGB2HSV );
	for(i=0;i<rgb->height;i++)
	{
		for(j=0;j<rgb->width;j++)
		{
			col = cvGet2D(hsv,i,j);
			col.val[0] *= M_2PI_180;
			col.val[2] = 255-col.val[2];
			col.val[1] = col.val[1]/255;
			
			col2.val[2] = (1-col.val[1])*col.val[2];
			//col2.val[2] = sqrt(255*(1-col.val[1])*col.val[2]);
			col2.val[0] = (col.val[1]*cos(col.val[0])+1)*127.5;
			col2.val[1] = (col.val[1]*sin(col.val[0])+1)*127.5;
			//col2.val[2] = (1-col.val[1])*col.val[2]/2;

			cvSet2D(hsv,i,j,col2);
		}
	}/**/

	return hsv;
}
/*
  *****************************************************************************
	Median filter
	
	@param src input image
	@param dst output image (will be allocated inside if <= NULL)
	@param radius median filter radius

	@return Returns the output image.
*/
IplImage* cvMedian(IplImage* src,IplImage* dst, int radius)
{
	IplImage* _dst = cvCreateImage( cvSize(src->width,src->height), src->depth, src->nChannels );
	radius = 2*radius + 1;

	cvSmooth( src, _dst, CV_MEDIAN,radius,0,0,0 );

	if(dst<=NULL) dst = _dst;
	else {cvCopyImage(_dst,dst); cvReleaseImage(&_dst);}

	return dst;
}
/*
  *****************************************************************************
	Swap memory
	
	@param mem1 input adress
	@param mem2 input adress
*/

void swapMem(void** mem1, void** mem2)
{
	void* temp = *mem1;
	*mem1 = *mem2;
	*mem2 = temp;
}

/*

*/
CvRect _selection={0};
unsigned char _selecting = 0,_selected=0;
CvPoint _origin={0};
void onMouse( int event, int x, int y, int flags, void* param )
{
	IplImage* img = (IplImage*)param;
    if( !img )
        return;

    if( img->origin )
        y = img->height-1 - y;

	if(x>60000)x=0;
	if(y>60000)y=0;
	x = min(max(x,0),img->width-1);
	y = min(max(y,0),img->height-1);

    if( _selecting )
    {
        _selection.x = min(x,_origin.x);
        _selection.y = min(y,_origin.y);
        _selection.width = CV_IABS(x - _origin.x);
        _selection.height = CV_IABS(y - _origin.y);
    }

    switch( event )
    {
		case CV_EVENT_LBUTTONDOWN:		
			_origin = cvPoint(x,y);
			_selection = cvRect(x,y,0,0);
			_selecting = 1;
			break;

		case CV_EVENT_LBUTTONUP:
			_selected = _selecting*( _selection.width > 0 && _selection.height > 0 );
			_selecting = 0;
			break;
    }
}
int getMouseSelection(CvRect* selection)
{
	*selection = _selection;
	return _selected;
}
void resetMouseSelection()
{
	_selected = 0;
}
int isMouseSelection()
{
	return _selected;
}
int isMouseSelecting()
{
	return _selecting;
}
int drawMouseSelection(IplImage* img)
{
	if( img>NULL && _selecting && _selection.width > 0 && _selection.height > 0 ) 
	{
		cvSetImageROI( img, _selection );
		cvXorS( img, cvScalarAll(255), img, 0 );
		cvResetImageROI( img );

		cvRectangle( img, 
					 cvPoint( _selection.x, _selection.y ), 
					 cvPoint( _selection.x + _selection.width, _selection.y + _selection.height ),
					 cvScalar( 0, 255, 0, 0 ), 1, 0, 0 );

		return 1;
	}
	return 0;
}