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

#ifndef MEMALLOC_H_
#define MEMALLOC_H_

#ifdef __cplusplus
   extern "C" {
#endif
	   
#include <stddef.h>

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
	   
extern unsigned int freeleft(void);

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
extern unsigned int minfreeleft(void);

/*
*********************************************************************************************************
* Name:                                    i_alloc 
* 
* Description: Initialize Heap (the list of free blocks) in the mem space dedicated for the heap
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
extern void i_alloc(void);

/*
*********************************************************************************************************
* Name:                                    malloc 
* 
* Description: Request allocation of nbytes from the Heap
*       IMPORTANT! This function is not multi-thread safe and require external protection to be asfe
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
extern void *malloc(size_t nbytes);

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
* Note(s):     
* 
* *********************************************************************************************************
*/
extern void free(void *aptr);

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*MEMALLOC_H_*/
