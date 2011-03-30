#ifndef __UTILS_H__
#define __UTILS_H__

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#define PRINT_ALL_MESSAGES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* ------------------- Global variables ------------------- */

#ifndef CLOCKS_PER_MILLISEC
#define CLOCKS_PER_MILLISEC (CLOCKS_PER_SEC / 1000)
#endif

#ifndef __FUNC__
#ifdef __func__
#define __FUNC__ __func__
#else
#ifdef __FUNCTION__
#define __FUNC__ __FUNCTION__
#endif
#endif
#endif

#ifndef inline
#define inline __inline
#endif

/*--------------------- Math functions -------------------- */

#ifndef POW2
#define POW2(n) (1<<n)
#endif

#ifndef SQR
#define SQR(a) ((a)*(a))
#endif

#ifndef ROUND
#define ROUND(a) ((int)((a)+0.5))
#endif

#ifndef NORM2D2
#define NORM2D2(x,y) (SQR(x) + SQR(y))
#endif

#ifndef NORM2D
#define NORM2D(x,y) sqrt(NORM2D2(x,y))
#endif

#ifndef NORM3D2
#define NORM3D2(x,y,z) (SQR(x) + SQR(y) + SQR(z))
#endif

#ifndef NORM3D
#define NORM3D(x,y,z) sqrt(NORM3D2(x,y,z))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef ABS
#define ABS(a) ((a)<0?-(a):(a))
#endif

#ifndef SIGN
#define SIGN(a) ((a)<0?-1:1)
#endif

#ifndef IS_NULL
#define IS_NULL(a) ((double)(a)=<0.0000000000000001 && (double)(a)>=-0.0000000000000001)
#endif

#ifndef IS_NOT_NULL
#define IS_NOT_NULL(a) ((double)(a)>0.0000000000000001 || (double)(a)<-0.0000000000000001)
#endif

#ifndef SWAP
#define SWAP(a,b,tmp) tmp=a; a=b; b=tmp
#endif

#ifndef POS_PADDING
#define POS_PADDING(x,lengthArray) ((x)<0?0:((x)<lengthArray?(x):lengthArray-1))
#endif

#ifndef IS_POS_IN_ARR
#define IS_POS_IN_ARR(i,n) ((i)>=0 && (i)<n)
#endif

#ifndef IS_POS_IN_2DARR
#define IS_POS_IN_2DARR(i,j,r,c) (((i)>=0 && (i)<r) && ((j)>=0 && (j)<c))
#endif

#ifndef IS_POS_IN_IMG
#define IS_POS_IN_IMG(x,y,w,h) IS_POS_IN_2DARR(y,x,h,w)
#endif

#ifndef COIN_TOSS
#define COIN_TOSS(p) (((double)rand()/RAND_MAX) < (double)(p))
#endif

/*--------------------- Flag functions -------------------- */

#ifndef ENABLE_FLAG
#define ENABLE_FLAG(flags,flag) flags |= flag
#endif

#ifndef DISABLE_FLAG
#define DISABLE_FLAG(flags,flag) flags &= ~(flag)
#endif

#ifndef SWITCH_FLAG
#define SWITCH_FLAG(flags,flag) flags = (flags & ~(flag)) | (~(flags) & flag)
#endif

#ifndef IS_FLAG_ENABLED
#define IS_FLAG_ENABLED(flags,flag) (((flags) & (flag)) == (flag))
#endif

#ifndef IS_FLAG_DISABLED
#define IS_FLAG_DISABLED(flags,flag) (((flags) & (flag)) != (flag))
#endif

/*--------- Message printting variables and macros -------- */

#ifdef __FUNC__
#define printMsgInfo()	\
	fprintf(stderr,"In %s, %s function, line %d:\n", __FILE__, __FUNC__,__LINE__ )
#else
#define printMsgInfo()	\
	fprintf(stderr,"In %s, line %d:\n", __FILE__, __LINE__ )
#endif

#ifdef PRINT_ERROR_WARNING_MESSAGES
#define PRINT_ERRORS_MESSAGES
#define PRINT_WARNING_MESSAGES
#endif

#ifdef PRINT_ERROR_WARNING_DEBUG_MESSAGES
#define PRINT_ERROR_MESSAGES
#define PRINT_WARNING_MESSAGES
#define PRINT_DEBUG_MESSAGES
#endif

#ifdef PRINT_ERROR_DEBUG_MESSAGES
#define PRINT_ERROR_MESSAGES
#define PRINT_DEBUG_MESSAGES
#endif

#ifdef PRINT_ALL_MESSAGES
#define PRINT_ERROR_MESSAGES
#define PRINT_WARNING_MESSAGES
#define PRINT_DEBUG_MESSAGES
#define PRINT_MESSAGES
#endif

#ifdef PRINT_ERROR_MESSAGES
#define printError printMsgInfo();_printError
#else
#define printError
#endif

#ifdef PRINT_WARNING_MESSAGES
#define printWarning printMsgInfo();_printWarning
#else
#define printWarning
#endif

#ifdef PRINT_DEBUG_MESSAGES
#define printDebug printMsgInfo(); printf
#else
#define printDebug
#endif

#ifdef PRINT_MESSAGES
#define printMessage printf
#else
#define printMessage
#endif

/* ----------------- Function declaration ----------------- */

#define print_error printError
#define print_warning printWarning
#define print_debug printDebug
#define print_message printMessage
#define erase_from_stream eraseFromStream
#define double_array doubleArray
#define start_timming startTimming
#define end_timming endTimming


/**
	Prints an error message and aborts the program.  The error message is
	of the form "ERROR ... file function line", where the ... is specified
	by the \a format argument. The macro PRINT_ERROR_MESSAGES must by defined
	to print the error.
  
	@param format an error message format string (as with \c printf(3)).
*/
void _printError(char *fmt, ...);

/**
	Prints a warning message.  The warning message is of the form 
	"WARNING ... file function line", where the ... is specified
	by the \a format argument. The macro PRINT_WARNING_MESSAGES must 
	by defined to print the warning.
  
	@param format a warning message format string (as with \c printf(3)).
*/
void _printWarning(char *fmt, ...);

/**
	Replace the '\' by '/' in the string

	@param s a string.

	@return Return the modified string
*/
const char* bslash2slash( const char* s, char* news);

/**
  A function that removes the path from a filename.  Similar to the Unix
  basename command.
  
  @param filename a (full) path name
  @param base a string to store the base name (optional).
  
  @return Returns the basename of \a filename and fill base if not null.
*/
const char* basename( const char* filename, char* base );

/**
	A function that return the extension of a filename.

	@param filename a filename or a (full) path name
	@param ext a string to store the extension (optional).

	@return Returns the path of \a filename and fill path if not null.
*/
const char* extname( const char* filename, char* ext );

/**
	A function that return the filename with a new extension.

	@param filename a filename or a (full) path name
	@param newExt new extension.
	@param newFilename a string to store the new filename (optional).

	@return Returns the path of \a filename and fill path if not null.
*/
const char* changeExt( const char* filename, const char* newExt,char* newFilename );

/**
  A function that return the path from a filename.
  
  @param filename a (full) path name
  @param path a string to store the path (optional).
  
  @return Returns the path of \a filename and fill path if not null.
*/
const char* pathname( const char* filename, char* path );


/**
	Displays progress in the console with dots.  Every time this
	function is called, the state of dots is incremented.  The dots
	have four states that loop indefinitely: ' ', '.', '..', '...'.

	@param done if 0, this function simply increments the state of the dots;
*/
void progress(int done);
void progressPerc(int done,float perc);
void progressMessage( int done ,char *fmt, ...);
/**
  Erases a specified number of characters from a stream.

  @param stream the stream from which to erase characters
  @param n the number of characters to erase
*/
void eraseFromStream( FILE* stream, int n );

/**
	Create a string ..., where the ... is specified
	by the \a format argument..

	@param format a message format string (as with \c printf(3)).
	@return Returns the string.
*/
const char* mkStr(char *fmt, ...);

/**
	Doubles the size of an array with error checking

	@param arr pointer to an array whose size is to be doubled
	@param nElmts number of elements allocated for \a array
	@param elmtSize size in bytes of elements in \a array

	@return Returns the new number of elements allocated for \a array.  If no
	memory is available, returns 0.
*/
int doubleArray( void** arr, int nElmts, int elmtSize );

/**
	Wait during m milliseconds

	@param milliseconds time to wait
*/
void wait ( int milliseconds );

/**
	Start the timing for benchmarking
*/
void startTiming();

/**
	End the timing for benchmarking

	@return Return the time taken between startTiming and endTiming
	in milliseconds
*/
long long endTiming();

/*
	Print the timing for benchmarking

	@param t time in milliseconds

	@return Return the time in milliseconds
*/
long long printTime(long long t);

int  fexist(const char *filename);
long  fsize(const char *filename);
int direxist(const char *directory);
int makedir(const char *directory);

int saveArray(const char* filename, const void* arr,int n, int elmtSize, double (*getElmt) (void const *elmt));

#endif