#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "2darray.h"

/*	--------------------------------------------------------------------------------------------
	Creates a 2D array with contiguous memory blocks.

	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the 2D array.
*/
void** create2Darr(int rows, int cols, const int size)
{
	void* mem = malloc(rows*(cols*=size));
	void** arr;
	unsigned char**arrIt,*memIt;

	if(mem<=NULL) return NULL;
	
	arr = malloc(rows*sizeof(void*));

	if(arr<=NULL){free(mem); return NULL;}

	for(arrIt=(unsigned char**)arr,(unsigned char*)memIt=mem;rows--;memIt+=cols)
		*arrIt++ = memIt;

	return arr;
}

/*	--------------------------------------------------------------------------------------------
	Free a 2D array with contiguous memory blocks.

	@param arr array to be freed.

	@return Returns NULL.
*/
void** free2Darr(void** arr)
{
	if(arr>NULL)
	{
		if(*arr>NULL) free(*arr);
		free(arr);
	}
	return NULL;
}

/*	--------------------------------------------------------------------------------------------
	Copy a 2D array with contiguous memory blocks.

	@param src array to be copied.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.
	@param dst allocate memory if NULL.

	@return Returns the copied 2D array.
*/
void** copy2Darr(void**src, int rows, int cols, const int size,void** dst)
{
	if(dst<=NULL) dst = create2Darr(rows, cols, size);
	memcpy(*dst,*src,rows*cols*size);
	return dst;
}

/*	--------------------------------------------------------------------------------------------
	Create a 2D array from another 2D array with contiguous memory blocks.

	@param src 2D array to be cloned.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the cloned 2D array.
*/
void** clone2Darr(void**src, int rows, int cols, const int size)
{
	void** dst = create2Darr(rows, cols, size);
	memcpy(*dst,*src,rows*cols*size);
	return dst;
}

/*	--------------------------------------------------------------------------------------------
	Embed a 1D array into a 2D array. USE free INSTEAD OF free2Darr 
	if you don't want to free the 1D array while freeing the 2D one.

	@param src 1D array to be embeded; src must be of size rows*cols*size.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the 2D array version of the 1D array.
*/
void** embed1DarrInto2Darr(void*src,int rows, int cols, const int size)
{
	void** arr;
	unsigned char**arrIt,*srcIt;

	if(src<=NULL) return NULL;
	
	arr = malloc(rows*sizeof(void*));

	if(arr<=NULL)return NULL;

	cols*=size;
	for(arrIt=(unsigned char**)arr,srcIt=(unsigned char*)src;rows--;srcIt+=cols)
		*arrIt++ = srcIt;

	return arr;
}

/*	--------------------------------------------------------------------------------------------
	Copy a 1D array memory and put it in a 2D array with contiguous memory blocks.

	@param src 1D array to be copied; src must be of size rows*cols*size.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.
	@param dst allocate memory if NULL.

	@return Returns the copied 2D array.
*/
void** copy2DarrFrom1Darr(void*src, int rows, int cols, const int size,void** dst)
{
	if(dst<=NULL) dst = create2Darr(rows, cols, size);
	memcpy(*dst,src,rows*cols*size);
	return dst;
}

/*	--------------------------------------------------------------------------------------------
	Clone a 1D array memory and put it in a 2D array with contiguous memory blocks.

	@param src 1D array to be copied; src must be of size rows*cols*size.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the cloned 2D array.
*/
void** clone2DarrFrom1Darr(void*src, int rows, int cols, const int size)
{
	void** dst = create2Darr(rows, cols, size);
	memcpy(*dst,src,rows*cols*size);
	return dst;
}

/*	--------------------------------------------------------------------------------------------
	Save 2D array in a binary file

	@param filename name of the file.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns >0 value if everything is saved correctly.
*/
int save2Darr(const char* filename, const void** arr, const int rows, const int cols, const int size)
{
	FILE* fp = NULL;

	if((fp = fopen(filename,"wb+"))<=NULL)
		return 0;

	fwrite(&rows, sizeof(int), 1, fp );
	fwrite(&cols, sizeof(int), 1, fp );
	fwrite(&size, sizeof(int), 1, fp );
	fwrite(*arr, size, rows*cols, fp );
	

	fclose(fp);

	return 1;
}

/*	--------------------------------------------------------------------------------------------
	Load 2D array from a binary file

	@param filename name of the file.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns adress of the array.
*/
void** load2Darr(const char* filename, void*** arr, int* rows, int* cols, int* size)
{
	FILE* fp = NULL;
	int _rows,_cols,_size;
	void** _arr;

	if((fp = fopen(filename,"rb"))<=NULL)
		return 0;

	fread(&_rows, sizeof(int), 1, fp );
	fread(&_cols, sizeof(int), 1, fp );
	fread(&_size, sizeof(int), 1, fp );
	if((_rows*_cols*_size)<=0) return 0;
	_arr = create2Darr(_rows, _cols, _size);
	fread(*_arr, (unsigned int)size, (unsigned int)(_rows*_cols), fp );
	

	fclose(fp);
	if(arr>NULL) *arr = _arr;
	if(rows>NULL) *rows = _rows;
	if(cols>NULL) *cols = _cols;
	if(size>NULL) *size = _size;

	return _arr;
}