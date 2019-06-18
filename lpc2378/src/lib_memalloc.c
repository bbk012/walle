/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_memalloc.c
* Description: Functions to control dynamic memory allocations
* Author:      Bogdan Kowalczyk
* Date:        30-Sep-2008
* Note:
* 	This is based on the source code from article "memory allocation in C"
*   by Leslie Aldridge avaliable on embedded.com
* History:
*              30-Sep-2008 - Initial version created
* 				2-Nov-2008 - Added critical mem exception generation when allocations fail
*********************************************************************************************************
*/

/*
 * Produced by Programming ARTS
 * 8/18/88
 * Programmers: Les Aldridge, Travis I. Seay
 */

#include "type.h"
#include "lib_memalloc.h"
#include "lib_error.h"


//HEADER occupy 8 bytes
typedef struct hdr {
	struct hdr	*ptr; // 4 byte long pointer (in ARM)to other free blocks (NULL if there are not any free blocks - end of free block list) 
	unsigned int size; //4 byte long size of allocated space in HEADER size units
} HEADER;

/*	Defined in the linker file. _heapstart is the first byte allocated to the heap; _heapend
is the last. */

extern HEADER _heapstart, _heapend;

// this function is removed extern void	warm_boot(char *str);

static HEADER 	*frhd=NULL; //head of the list of free blocks
static volatile unsigned int	memleft;	/* memory left on the heap in HEADER size units*/
static volatile unsigned int	minmemleft; /* the smallest amount of free memory noticed measured in HEADER size units*/


/*
*********************************************************************************************************
* Name:                                   freeleft
* 
* Description: returns number of free bytes on the heap
*  
*
* Arguments:   none
*
* Returns:     number of free bytes on the heap
*
* 			
* Note(s):     
* 
* *********************************************************************************************************
*/
unsigned int freeleft(void)
{
	return  memleft*sizeof(HEADER);//calculate number of free bytes on the heap
}//freeleft

/*
*********************************************************************************************************
* Name:                                   minfreeleft
* 
* Description: returns registered smallest number of free bytes on the heap
*  
*
* Arguments:   none
*
* Returns:     smallest number of free bytes on the heap so far
*
* 			
* Note(s):     
* 
* *********************************************************************************************************
*/
unsigned int minfreeleft(void)
{
	return minmemleft*sizeof(HEADER);//return calculated number of minimum free bytes on the heap
}//minfreeleft

/*
*********************************************************************************************************
* Name:                                    free 
* 
* Description: Release heap space allocated by malloc
*       IMPORTANT! This function is not multi-thread safe and require external protection to be safe
*
* Arguments:   pointer to the allocated space intended to be free
*
* Returns:     none
*
* 			
* Note(s):     
* 
* *********************************************************************************************************
*/
void free(void *ap)
{

/*	Return memory to free list. Where possible, make contiguous blocks of free memory. (Assumes that 0 is not a valid address for allocation. Also, i_alloc() must be called prior to using either free() or malloc(); otherwise, the free list will be null.) */

	HEADER *nxt, *prev, *f;

	//it is assumed ap cannot be NULL
	
	f = (HEADER *)ap - 1;	/* Point to header of block being returned. */
	memleft += f->size;	
	
	if(frhd==NULL)//NULL here means all heap is allocated (we assume i_alloc was called before first free call)
	{//returned block will be the only on free list
		frhd=f;//make free blocks pointer to point to the released block
		f->ptr=NULL;//mark that this has to be the only free block on the list
		return;
	}

/*	frhd is never null unless i_alloc() wasn't called to initialize package. */

if (frhd > f)
	{

	/* Free-space head is higher up in memory than returnee. */

	nxt = frhd; 	/* old head */
	frhd = f; 	/* new head */
	prev = f + f->size; 	/* right after new head */

	if (prev==nxt)  /* Old and new are contiguous. */
		{//B.K. looks this was missed in origial implementation so I add it but this need to be checked
		f->size += nxt->size;
		f->ptr = nxt->ptr; 	/* Form one block. */
		}
	else 
		{
		f->ptr = nxt;
		}
	return;
	}

/*	Otherwise, current free-space head is lower in memory. Walk down free-space list looking for the block being returned. If the next pointer points past the block, make a new entry and link it. If next pointer plus its size points to the block, form one contiguous block. */

nxt = frhd;
for (nxt=frhd; nxt && nxt < f; prev=nxt,nxt=nxt->ptr)
	{
		if (nxt+nxt->size == f)
		{
			nxt->size += f->size; 	/* They're contiguous. */
			f = nxt + nxt->size; 	/* Form one block. */
			if (f==nxt->ptr)
			{

			/* The new, larger block is contiguous to the next free block, so form a larger block.There's no need to continue this checking since if the block following this free one were free, the two would already have been combined. */

			nxt->size += f->size;
			nxt->ptr = f->ptr;
			}
		return;
		}
	}

/*	The address of the block being returned is greater than one in the free queue (nxt) or the end of the queue was reached. If at end, just link to the end of the queue. Therefore, nxt is null or points to a block higher up in memory than the one being returned. */

prev->ptr = f; 	/* link to queue */
prev = f + f->size; 	/* right after space to free */
if (prev == nxt) 	/* 'f' and 'nxt' are contiguous. */
	{
	f->size += nxt->size;
	f->ptr = nxt->ptr; 	/* Form a larger, contiguous block. */
	}
else 
	f->ptr = nxt;
return;
}//void free

/*
*********************************************************************************************************
* Name:                                    malloc 
* 
* Description: Request allocation of nbytes from the Heap
*       IMPORTANT! This function is not multi-thread safe and require external protection to be safe
*
* Arguments:   nbytes - requested number of bytes from the heap
* 		IMPORTANT! Works correctly only if nbytes > 0 otherwise exception is generated (MEM_ALLOC_EXCEPTION)
*
* Returns:     adress of the allocated space or NULL when there is not space avaliable
*
* Note(s):     
* 		IMPORTANT! When memory cannot be allocated  MEM_ALLOC_EXCEPTION exception is generated through SWI
* 
* *********************************************************************************************************
*/

void *malloc(size_t nbytes) 	/* bytes to allocate */
{
HEADER *nxt, *prev;
int 		nunits;


if (!nbytes) MEM_ALLOC_EXCEPTION; //B.K. do not allocate for zero because for such a case malloc returned pointer points to not allocated area on the heap

nunits = (nbytes+sizeof(HEADER)-1) / sizeof(HEADER) + 1;

for (prev=NULL,nxt=frhd; nxt; prev=nxt,nxt = nxt->ptr)
{
	if (nxt->size >= nunits) 	/* big enough */
	{
		if (nxt->size > nunits) //if free block avaliable is larger than requested block
		{
			nxt->size -= nunits; 	/* Allocate requested block from the avalilable free block end. */
			nxt += nxt->size;       //advance poiter to the block which will be allocated 
			nxt->size = nunits; 	/* nxt now == pointer to be alloc'd. so update allocated block size value */
		}
		else //requested size in HEADER units exactly correspond to the released block
		{
			if (prev==NULL) frhd = nxt->ptr; //if we allocate first block from the free list let frhd point to next block if any or NULL if no more frr blocks
			else prev->ptr = nxt->ptr; //if there is procedding block join it to the next block behind the one which is allocated
		}
		memleft -= nunits;
		if(memleft < minmemleft) minmemleft=memleft;//if current memory left on heap smallet than lowest valus so far update lowest value so far to the new value

		/* Return a pointer past the header to the actual space requested. */

		return((void *)(nxt+1));
	}
}
//memory cannot be allocated generate exception
MEM_ALLOC_EXCEPTION;
/* This function that explains what catastrophe befell us before resetting the system. */
//warm_boot("Allocation Failed!");
return(NULL);
}//void *malloc


/*
*********************************************************************************************************
* Name:                                    i_alloc 
* 
* Description: Initialize Heap (the list of free blocks) in the memory space dedicated for the heap
*       IMPORTANT! This function is not multi-thread safe and require external protection to be safe
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
void i_alloc(void)
{
frhd = &_heapstart; 	/* Initialize the allocator. */
frhd->ptr = NULL;
//cast to (char*) to count numbers of bytes
frhd->size = ((char *)&_heapend - (char *)&_heapstart) / sizeof(HEADER);
memleft = frhd->size; 	/* initial size in HEADER size units */
minmemleft=memleft;/*smallest value at the beginning is same as mem left*/
}//void i_alloc
