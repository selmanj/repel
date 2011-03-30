#ifndef __2D_ARRAY_H__
#define __2D_ARRAY_H__

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

/**
	Creates a 2D array with contiguous memory blocks.

	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the 2D array.
*/
void** create2Darr(int rows, int cols, const int size);

/**
	Free a 2D array with contiguous memory blocks.

	@param arr array to be freed.

	@return Returns NULL.
*/
void** free2Darr(void** arr);

/**
	Copy a 2D array with contiguous memory blocks.

	@param src array to be copied.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.
	@param dst allocate memory if NULL.

	@return Returns the copied 2D array.
*/
void** copy2Darr(void**src, int rows, int cols, const int size,void** dst);

/**
	Create a 2D array from another 2D array with contiguous memory blocks.

	@param src 2D array to be cloned.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the cloned 2D array.
*/
void** clone2Darr(void**src, int rows, int cols, const int size);

/**
	Embed a 1D array into a 2D array. USE free INSTEAD OF free2Darr 
	if you don't want to free the 1D array while freeing the 2D one.

	@param src 1D array to be embeded; src must be of size rows*cols*size.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the 2D array version of the 1D array.
*/
void** embed1DarrInto2Darr(void*src,int rows, int cols, const int size);

/**
	Copy a 1D array memory and put it in a 2D array with contiguous memory blocks.

	@param src 1D array to be copied; src must be of size rows*cols*size.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.
	@param dst allocate memory if NULL.

	@return Returns the copied 2D array.
*/
void** copy2DarrFrom1Darr(void*src, int rows, int cols, const int size,void** dst);

/**
	Clone a 1D array memory and put it in a 2D array with contiguous memory blocks.

	@param src 1D array to be copied; src must be of size rows*cols*size.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns the cloned 2D array.
*/
void** clone2DarrFrom1Darr(void*src, int rows, int cols, const int size);

/**
	Save 2D array in a binary file

	@param filename name of the file.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns >0 value if everything is saved correctly.
*/
int save2Darr(const char* filename, const void** arr, const int rows, const int cols, const int size);

/**
	Load 2D array from a binary file

	@param filename name of the file.
	@param rows # of rows in the array.
	@param cols #of colums in the array.
	@param size size of an array element.

	@return Returns adress of the array.
*/
void** load2Darr(const char* filename, void*** arr, int* rows, int* cols, int* size);

#endif