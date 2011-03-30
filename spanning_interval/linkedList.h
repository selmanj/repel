#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#ifndef inline
#define inline __inline
#endif

/*	
	-------------------------------------------------------------------
								Link macros
	-------------------------------------------------------------------
*/

#define LINK_STRUCTURE	\
struct {struct _lk* prev,*next;}

#define lkRemove lkDisconnect
#define lkDelete lkDisconnectAndFree

#define LK(link_pt)						((_lk*)link_pt)

#define NEXT_LK(link_pt)				((void*)LK(link_pt)->next)

#define PREV_LK(link_pt)				((void*)LK(link_pt)->prev)

#define SET_NEXT_LK(link_pt,next_pt)	(LK(link_pt)->next=next_pt)

#define SET_PREV_LK(link_pt,prev_pt)	(LK(link_pt)->prev=prev_pt)

#define lkCpyLink(dst,src)				LK(memcpy(dst,src,sizeof(_lk)))
#define lkDataPtr(link_pt,PTR_TYPE)		((PTR_TYPE)(&((LK(link_pt)->next) + sizeof(_lk*))))
#define lkCpyData(link_pt,data_pt,TYPE)	((TYPE*)(memcpy(data_pt,&((LK(link_pt)->next) + sizeof(_lk*)),sizeof(TYPE))))

/*	
	-------------------------------------------------------------------
							Linked list macros
	-------------------------------------------------------------------
*/
#define LINKED_LIST_STRUCTURE		\
struct								\
{									\
	_lk headSentinel,tailSentinel;	\
	long cnt;						\
}

#define LKL(list_pt) ((lkList*)list_pt)

#define lklRemove		lklDisconnect
#define lklDelete		lklDisconnectAndFree
#define lklAdd			lklPushFront
#define lklCleanList	lklClearList

#define lklIsEmpty(list_pt)		(LKL(list_pt)->headSentinel.next == &LKL(list_pt)->tailSentinel)
#define lklIsNotEmpty(list_pt)	(LKL(list_pt)->headSentinel.next != &LKL(list_pt)->tailSentinel)

#define lklHead(list_pt)		(LKL(list_pt)->headSentinel.next)
#define lklTail(list_pt)		(LKL(list_pt)->tailSentinel.prev)

#define lklSHead(list_pt)		(&(LKL(list_pt)->headSentinel))
#define lklSTail(list_pt)		(&(LKL(list_pt)->tailSentinel))

/*	
	-------------------------------------------------------------------
							Iterator macros
	-------------------------------------------------------------------
*/
#define LINKED_LIST_ITERATOR_STRUCTURE	\
struct{_lk* start, * end, * cur;}

#define LKI(iter_pt) ((lkIterator*)iter_pt)

#define lkiNext(iter_pt)					(LKI(iter_pt)->cur = LKI(iter_pt)->cur->next)
#define lkiPrev(iter_pt)					(LKI(iter_pt)->cur = LKI(iter_pt)->cur->prev)
#define lkiIsNotDone(iter_pt)				(LKI(iter_pt)->cur!= LKI(iter_pt)->end)
#define lkiBegin(iter_pt)					(LKI(iter_pt)->cur = LKI(iter_pt)->start)
#define lkiDataPtr(iter_pt,PTR_TYPE)		((PTR_TYPE)(&((LKI(iter_pt)->cur->next) + sizeof(_lk*))))
#define lkiCpyData(iter_pt,data_pt,TYPE)	memcpy(data_pt,&((LKI(iter_pt)->cur->next) + sizeof(_lk*)),sizeof(TYPE))

/*	
	-------------------------------------------------------------------
								Structures
	-------------------------------------------------------------------
*/

/*
	------------ Default link structure -----------
*/
typedef
struct _lk
{
	LINK_STRUCTURE;
}
_lk;

/*
	-------- Default linked list structure --------
*/
typedef
struct lkList
{
	LINKED_LIST_STRUCTURE;
}
lkList;

/*
	---------- Default iterator structure ---------
*/
typedef
struct lkIterator
{
	LINKED_LIST_ITERATOR_STRUCTURE;
}
lkIterator;

/*	
	-------------------------------------------------------------------
							Link functions
	-------------------------------------------------------------------
*/

/**
	Insert a link after a reference link

	@param linkref_pt reference link
	@param link_pt link to be inserted

	@return Returns the inserted link.
*/
inline void* lkInsertAfter(void* linkref_pt, void* link_pt)
{
	LK(link_pt)->prev = LK(linkref_pt);
	LK(link_pt)->next = LK(linkref_pt)->next;
	LK(linkref_pt)->next->prev = LK(link_pt);
	LK(linkref_pt)->next = LK(link_pt);

	return link_pt;
}

/**
	Insert a link before a reference link

	@param linkref_pt reference link
	@param link_pt link to be inserted

	@return Returns the inserted link.
*/
inline void* lkInsertBefore(void* linkref_pt, void* link_pt)
{
	LK(link_pt)->prev = LK(linkref_pt)->prev;
	LK(link_pt)->next = LK(linkref_pt);
	LK(linkref_pt)->prev->next = LK(link_pt);
	LK(linkref_pt)->prev = LK(link_pt);

	return link_pt;
}

/**
	Disconnect a link.

	@param link_pt link to be disconnected

	@return Returns the disconnected link
*/
inline void* lkDisconnect(void* link_pt)
{
	LK(link_pt)->prev->next = LK(link_pt)->next;
	LK(link_pt)->next->prev = LK(link_pt)->prev;
	return link_pt;
}

/**
	Disconnect a link, free inside data and free the link.

	@param link_pt link to be deleted
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL

	@return Returns NULL
*/
inline void* lkDisconnectAndFree(void* link_pt,int (*freeLinkData)(void* link_pt))
{
	lkDisconnect(link_pt);
	if(freeLinkData>NULL)(*freeLinkData)(link_pt);
	free(link_pt);

	return NULL;
}

/**
	Free inside data and free the link.

	@param link_pt link to be deleted
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL

	@return Returns NULL
*/
inline void* lkFree(void* link_pt,int (*freeLinkData)(void* link_pt))
{
	if(freeLinkData>NULL)(*freeLinkData)(link_pt);
	free(link_pt);

	return NULL;
}

/**
	Swap two links.

	@param link_pt1 link to be swapped
	@param link_pt2 link to be swapped
*/
inline void lkSwap(void* link_pt1,void* link_pt2)
{
	_lk* tmp = LK(link_pt1)->prev;
	lkDisconnect(link_pt1);
	lkDisconnect(link_pt2);
	lkInsertAfter(LK(link_pt2)->prev, link_pt1);
	lkInsertAfter(tmp, link_pt2);
}

/*	
	-------------------------------------------------------------------
							Linked list functions
	-------------------------------------------------------------------
*/

/**
	Initialize a linked list.

	@param list_pt pointer to a list

	@return Returns the pointer to the list.
*/
inline void* lklInitList(void* list_pt)
{
	memset(list_pt,0,sizeof(lkList));
	LKL(list_pt)->headSentinel.next = &LKL(list_pt)->tailSentinel;
	LKL(list_pt)->tailSentinel.prev = &LKL(list_pt)->headSentinel;

	return list_pt;
}

/**
	Create and initialize a linked list.

	@return Returns the pointer to the list.
*/
inline lkList* lklCreateList()
{
	return LKL(lklInitList(malloc(sizeof(lkList))));
}

/**
	Disconnect the head from the list.

	@param list_pt pointer to the list

	@return Returns the disconnected link.
*/
inline void* lklPopFront(void* list_pt)
{
	LKL(list_pt)->cnt--;
	return lkDisconnect(LKL(list_pt)->headSentinel.next);
}

/**
	Disconnect the tail from the list.

	@param list_pt pointer to the list

	@return Returns the disconnected link.
*/
inline void* lklPopBack(void* list_pt)
{
	LKL(list_pt)->cnt--;
	return lkDisconnect(LKL(list_pt)->tailSentinel.prev);
}

/**
	Add link to the head of the list.

	@param list_pt pointer to the list
	@param link_pt link to be added

	@return Returns the added link.
*/
inline void* lklPushFront(void* list_pt,void* link_pt) 
{
	LKL(list_pt)->cnt++;
	return lkInsertAfter(&LKL(list_pt)->headSentinel,link_pt);
}

/**
	Add link to the tail of the list.

	@param list_pt pointer to the list
	@param link_pt link to be added

	@return Returns the added link.
*/
inline void* lklPushBack(void* list_pt,void* link_pt) 
{
	LKL(list_pt)->cnt++;
	return lkInsertBefore(&LKL(list_pt)->tailSentinel,link_pt);
}

/**
	Add link after reference link.

	@param list_pt pointer to the list
	@param link_pt link to be added

	@return Returns the added link.
*/
inline void* lklInsertAfter(void* list_pt,void* linkref_pt,void* link_pt) 
{
	LKL(list_pt)->cnt++;
	return lkInsertAfter(linkref_pt,link_pt);
}

/**
	Add link before reference link.

	@param list_pt pointer to the list
	@param link_pt link to be added

	@return Returns the added link.
*/
inline void* lklInsertBefore(void* list_pt,void* linkref_pt,void* link_pt) 
{
	LKL(list_pt)->cnt++;
	return lkInsertBefore(linkref_pt,link_pt);
}

/**
	Disconnect a link from the list.

	@param list_pt pointer to the list
	@param link_pt link to be disconnected

	@return Returns the disconnected link.
*/
inline void* lklDisconnect(void* list_pt,void* link_pt)
{
	LKL(list_pt)->cnt--;
	return lkDisconnect(link_pt);
}

/**
	Disconnect a link from the list, free embeded data and free the link.

	@param list_pt pointer to the list
	@param link_pt link to be disconnected
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL

	@return Returns NULL.
*/
inline void* lklDisconnectAndFree(void* list_pt,void* link_pt,int (*freeLinkData)(void* link_pt))
{
	LKL(list_pt)->cnt--;
	return lkDisconnectAndFree(link_pt,freeLinkData);
}

/**
	Free all the links in the list and their embeded data.

	@param list_pt pointer to the list
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL

   @return Returns the pointer to the list.
*/
void* lklFreeListMem(void* list_pt,int (*freeLinkData)(void* link_pt));

/**
	Free the list memory, then free the list and set it to NULL.

	@param list_pt pointer to the list
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL
*/
inline void lklReleaseList(void** list_pt,int (*freeLinkData)(void* link_pt))
{
	if(list_pt && lklFreeListMem(*list_pt,freeLinkData))
	{
		free(*list_pt);
		*list_pt = NULL;
	}
}

/**
	Free the list memory, and reset parameters to initiale state.

	@param list_pt pointer to the list
	@param freeLinkData function to delete embeded data
		   does not free anything if <= NULL
*/
inline void* lklClearList(void* list_pt,int (*freeLinkData)(void* link_pt))
{
	return lklFreeListMem(list_pt,freeLinkData)? lklInitList(list_pt):NULL;
}

/*	
	-------------------------------------------------------------------
							Iterator functions
	-------------------------------------------------------------------
*/

/**
	Initialize iterator.

	@param iter_pt iterator pointer to be initialized
	@param start starting link
	@param end end link

	@return Return iterator pointer.
*/
inline void* lkiInitIterator(void* iter_pt, void* start, void* end)
{
	LKI(iter_pt)->cur = LKI(iter_pt)->start = LK(start);
	LKI(iter_pt)->end = LK(end);

	return iter_pt;
}

/**
	Initialize iterator from list.

	@param iter_pt iterator pointer to be initialized
	@param list_pt list pointer

	@return Return iterator pointer.
*/
inline void* lkiInitIteratorFromList(void* iter_pt, void* list_pt)
{
	LKI(iter_pt)->cur = LKI(iter_pt)->start = lklHead(list_pt);
	LKI(iter_pt)->end = lklSTail(list_pt);

	return iter_pt;
}

/**
	Create iterator and initialize it.

	@param start starting link
	@param end end link

	@return Return iterator pointer.
*/
inline void* lkiCreateIterator(void* start, void* end)
{
	return lkiInitIterator(malloc(sizeof(lkIterator)), start, end);
}

/**
	Create iterator from list and initialize it.

	@param list_pt list pointer

	@return Return iterator pointer.
*/
inline void* lkiCreateIteratorFromList(void* list_pt)
{
	return lkiInitIteratorFromList(malloc(sizeof(lkIterator)), list_pt);
}

/**
	Free iterator and set it to NULL

	@param iter_pt adress of iterator pointer
*/
inline void lkiReleaseIterator(void** iter_pt)
{
	if(iter_pt)
	{
		if(*iter_pt>NULL)free(*iter_pt);
		*iter_pt = NULL;
	}
}
 

#endif