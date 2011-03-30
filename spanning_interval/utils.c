#include "utils.h"
#include <sys/stat.h>

clock_t ___t___;
char ____string____[1024] = {0};
char ____base____[512] = {0};
char ____path____[1024] = {0};
char ____ext____[32] = {0};

/*
	Prints an error message and aborts the program.  The error message is
	of the form "ERROR ... file function line", where the ... is specified
	by the \a format argument. The macro PRINT_ERROR_MESSAGES must by defined
	to print the error.
  
	@param format an error message format string (as with \c printf(3)).
*/
void _printError(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
	fprintf(stderr, "ERROR ");
    vfprintf(stderr, fmt, args);
	fprintf(stderr, "\nPRESS ENTER TO EXIT\n");
	getchar();
    va_end(args);
    exit(1);
}

/*
	Prints a warning message.  The warning message is of the form 
	"WARNING ... file function line", where the ... is specified
	by the \a format argument. The macro PRINT_WARNING_MESSAGES must 
	by defined to print the warning.
  
	@param format a warning message format string (as with \c printf(3)).
*/
void _printWarning(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
	fprintf(stderr, "WARNING ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/*
	Replace the '\' by '/' in the string

	@param s a string.

	@return Return the modified string
*/
const char* bslash2slash( const char* s, char* news)
{
	char *ptr=NULL;
	if(news<=NULL) news = (char*)____string____;
	strcpy(news,s);
	ptr = strchr( news, 92 );
	
	while(ptr>NULL)
	{
		*ptr = '/';
		ptr = strchr( ++ptr, 92 );
	}

	return (const char*)news;
}

/*
	A function that removes the path from a filename.  Similar to the Unix
	basename command.

	@param filename a (full) path name
	@param base a string to store the base name (optional).

	@return Returns the basename of \a filename and fill base if not null.
*/
const char* basename( const char* filename, char* base )
{
	char * last_slash,*_base = base>NULL?base:(char*)____base____;

	last_slash = (char*)strrchr( filename, '/' );
	if(! last_slash) last_slash = (char*)strrchr( filename, 92 );

	if( ! last_slash )
	{
		strcpy( _base, filename );
	}
	else
	{
		sprintf(_base,"%s\0",++last_slash);
	}
	last_slash = strrchr( _base, '.' );
	if(last_slash>NULL)*last_slash = '\0';

	return (const char*)_base;
}

/*
	A function that return the extension of a filename.

	@param filename a filename or a (full) path name
	@param ext a string to store the extension (optional).

	@return Returns the path of \a filename and fill path if not null.
*/
const char* extname( const char* filename, char* ext )
{
	char * ptr,*_ext = ext>NULL?ext:(char*)____ext____;

	ptr = (char*)strrchr( filename, '.' );

	if( ! ptr )
	{
		_ext[0] = '\0';
	}
	else
	{
		sprintf(_ext,"%s",++ptr);
	}

	return (const char*)_ext;
}

/*
	A function that return the path from a filename.

	@param filename a (full) path name
	@param path a string to store the path (optional).

	@return Returns the path of \a filename and fill path if not null.
*/
const char* pathname( const char* filename, char* path )
{
	char * last_slash,*_path = path>NULL?path:(char*)____path____;

	filename = bslash2slash(filename,NULL);
	last_slash = (char*)strrchr( filename, '/' );
	if(! last_slash) last_slash = (char*)strrchr( filename, 92 );

	if( ! last_slash )
	{
		_path[0] = '\0';
	}
	else
	{
		last_slash++;
		strncpy( _path, filename ,last_slash-filename);
		_path[last_slash-filename] = '\0';
	}

	return (const char*)_path;
}

/*
	A function that return the filename with a new extension.

	@param filename a filename or a (full) path name
	@param newExt new extension.
	@param newFilename a string to store the new filename (optional).

	@return Returns the path of \a filename and fill path if not null.
*/
const char* changeExt( const char* filename, const char* newExt,char* newFilename )
{
	char * ptr,*_newFilename = newFilename>NULL?newFilename:(char*)____path____;
	size_t l = strlen(filename);
	memset(_newFilename,0,1024);
	memcpy(_newFilename,filename,l);
	ptr = strrchr( _newFilename, '.' );

	if( ! ptr )
	{
		ptr=_newFilename+l+1;
		*ptr='.';
	}
	
	
	sprintf(++ptr,"%s\0",newExt);
	

	return (const char*)_newFilename;
}


/*
	Displays progress in the console with dots.  Every time this
	function is called, the state of dots is incremented.  The dots
	have four states that loop indefinitely: ' ', '.', '..', '...'.

	@param done if 0, this function simply increments the state of the pinwheel;
	otherwise it prints "done"
*/
const char* ___dots___[4] = {"   ", ".  ", ".. ", "..." };
void progress(int done)
{
	static int cur = -1;

	fprintf( stderr, "\b\b\b\b\b");
	if(!done)
	{
		cur = cur<3?cur + 1:0;
		fprintf( stderr, " %s ", ___dots___[cur] );
		fflush( stderr );
	}
}
void progressPerc(int done,float perc)
{
	static int cur = -1;

	if(!done)
	{
		fprintf( stderr, "\b\b\b\b\b\b\b\b\b\b");
		cur = cur<3?cur + 1:0;
		fprintf( stderr, " %2.2f%c%s ", perc,'%',___dots___[cur] );
		fflush( stderr );
	}
}
void progressMessage( int done,char *fmt, ...)
{
	static int cur = -1;
	va_list args;

	fprintf( stderr, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

	if( done )
	{
		if(fmt>NULL)
		{
			va_start(args, fmt);
			vfprintf(stderr, fmt, args);
			va_end(args);
		}

		//fprintf( stderr, "\b\bdone\n");
		cur = -1;
	}
	else
	{
		if(fmt>NULL)
		{
			va_start(args, fmt);
			vfprintf(stderr, fmt, args);
			va_end(args);
		}

		cur = cur<3?cur + 1:0;
		fprintf( stderr, " %s ", ___dots___[cur] );
		fflush( stderr );
	}
}


/*
	Erases a specified number of characters from a stream.

	@param stream the stream from which to erase characters
	@param n the number of characters to erase
*/
void eraseFromStream( FILE* stream, int n )
{
	int j;
	for( j = 0; j < n; j++ )
		fprintf( stream, "\b" );
}

/*
	Create a string ..., where the ... is specified
	by the \a format argument..

	@param format a message format string (as with \c printf(3)).
	@return Returns the string.
*/
const char* mkStr(char *fmt, ...)
{
    va_list args;
	//memset(____string____,0,1024);
    va_start(args, fmt);
    vsprintf(____string____, fmt, args);
    va_end(args);
	return ____string____;
}


/*
	Doubles the size of an array with error checking

	@param arr pointer to an array whose size is to be doubled
	@param nElmts number of elements allocated for \a array
	@param elmtSize size in bytes of elements in \a array

	@return Returns the new number of elements allocated for \a array.  If no
	memory is available, returns 0.
*/
int doubleArray( void** arr, int nElmts, int elmtSize )
{
	void* tmp;

	nElmts = nElmts>0?nElmts*2:1;

	tmp = realloc( *arr, nElmts * elmtSize );
	if( ! tmp )
	{
		printWarning("unable to allocate memory");
		if( *arr )
			free( *arr );
		*arr = NULL;
		return 0;
	}
	*arr = tmp;
	return nElmts;
}

/*
	Wait during m milliseconds

	@param milliseconds time to wait
*/
void wait ( int milliseconds )
{
	clock_t endwait;
	endwait = clock () + milliseconds * CLOCKS_PER_MILLISEC ;
	while (clock() < endwait);
}

/*
	Start the timing for benchmarking
*/
void startTiming()
{
	___t___ = clock();
}

/*
	End the timing for benchmarking

	@return Return the time taken between startTiming and endTiming
	in milliseconds
*/
long long endTiming()
{
	___t___ = clock() - ___t___ ;

	return (long long)(___t___/CLOCKS_PER_MILLISEC);
}

/*
	Print the timing for benchmarking

	@param t time in milliseconds

	@return Return the time in milliseconds
*/
long long printTime(long long t)
{
	long long d,h,m,s,ms;

	s = t/1000;
	ms = t-1000*s;
	m = s/60;
	s -= 60*m;
	h = m/60;
	m -= 60*h;
	d = h/24;
	h -= d*24;

	printf("time: ");
	if(d)printf("%d day%s ",(int)d,d>1?"s":"");
	if(h)printf("%d hour%s ",(int)h,h>1?"s":"");
	if(m)printf("%d minute%s ",(int)m,m>1?"s":"");
	if(s)printf("%d second%s ",(int)s,s>1?"s":"");
	if(ms)printf("%d millisecond%s",(int)ms,ms>1?"s":"");
	printf("\n");
	return t;
}

/*

*/

#ifndef S_ISREG
#define _IFMT		0170000
#define _IFREG		0100000
#define _IFDIR		0040000
#define S_ISREG(m)  (((m)&_IFMT) == _IFREG)
#define S_ISDIR(m)  (((m)&_IFMT) == _IFDIR)
#endif

int  fexist(const char *filename)
{
    struct stat s;

    if (stat (filename, &s))
        return 0;
    return S_ISREG(s.st_mode);
}

long  fsize(const char *filename)
{
    struct stat s;

    if (stat (filename, &s))
        return -1L;
    return s.st_size;
}

int direxist(const char *directory)
{
    struct stat s;
    int rc;

#if !defined(__WATCOMC__) && !defined(__MSVC__) && !defined(__MINGW32__)
    rc = stat (directory, &s);
#else
    char *tempstr, *p;
    size_t l;
    tempstr = strdup(directory);
    if (tempstr == NULL) {
        return 0;
    }

    /* Root directory of any drive always exists! */

    if ((isalpha((int)tempstr[0]) && tempstr[1] == ':' && (tempstr[2] == '\\' || tempstr[2] == '/') &&
      !tempstr[3]) || eqstr(tempstr, "\\")) {
        free(tempstr);
        return 1;
    }

    l = strlen(tempstr);
    if (tempstr[l - 1] == '\\' || tempstr[l - 1] == '/')
    {
        /* remove trailing backslash */
        tempstr[l - 1] = '\0';
    }

    for (p=tempstr; *p; p++)
    {
        if (*p == '/')
          *p='\\';
    }

    rc = stat (tempstr, &s);

    free(tempstr);
#endif
    if (rc)
        return 0;
    return S_ISDIR(s.st_mode);
}

int makedir(const char *directory)
{
	char buf[1024] = {0};
	sprintf(buf,"mkdir \"%s\"",directory);
	system(buf);

	return direxist(directory);
}

double _getElmt (const void *elmt) {return *(double*)elmt;}
int saveArray(const char* filename, const void* arr,int n, int elmtSize, double (*getElmt) (void const *elmt))
{
	char* pt = (char*)arr;
	FILE* fp = NULL;
	if(getElmt<=NULL)getElmt=_getElmt;

	if((fp = fopen(filename,"w+"))<=NULL)
		return 0;

	fprintf(fp,"size: %d\n",n);
	for(;n--;pt+=elmtSize)
	{
		fprintf(fp,"%lf\n",getElmt((const void*)pt));
	}

	fclose(fp);

	return 1;
}