#include "linkedList.h"

/*	
	-------------------------------------------------------------------
							Linked list functions
	-------------------------------------------------------------------
*/

/*
	Free all the links in the list and their embeded data.

	@param list_pt pointer to the list
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL

   @return Returns the pointer to the list.
*/
void* lklFreeListMem(void* list_pt,int (*freeLinkData)(void* link_pt))
{
	_lk* cur_pt;
	_lk* next_pt;

	if(list_pt<=NULL || lklHead(list_pt)<=NULL) return NULL;

	cur_pt  = lklHead(list_pt);
	next_pt = cur_pt->next;

	if(freeLinkData>NULL)
	{
		while(cur_pt>NULL && cur_pt!=lklSTail(list_pt))
		{
			if(cur_pt>NULL)
			{
				next_pt = cur_pt->next;
				(*freeLinkData)(cur_pt);
				free(cur_pt);
				cur_pt = next_pt;
			}
		}
	}
	else
	{
		while(cur_pt!=lklSTail(list_pt))
		{
			if(cur_pt>NULL)
			{
				next_pt = cur_pt->next;
				free(cur_pt);
				cur_pt = next_pt;
			}
		}
	}

	return list_pt;
}