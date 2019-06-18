/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_new.cpp
* Description: operator new() and operator delete() implementation to not use lstdc++ lib to avoid overhead
* Author:      Bogdan Kowalczyk
* Date:        16-October-2008
* Note:
*       In C++ new calls operator new() (if class specific operator new() is not defined
*       and delete calls operator delete (if class specific operator delete() is not defined)
*       In addition for both in lstdc++ exception thrownig is used by default what generates big overhead.
*       In this implementation exceptions are not geerated
* History:
*              16-Oct-2008 - Initial version created
*********************************************************************************************************
*/
#include "lib_std.h"                   // for prototypes of malloc() and free()
#include "lib_memalloc.h"
#include "lib_new.hpp"
#include "wrp_kernel.hpp"

//extern cMutex MemMutex;
/*
*********************************************************************************************************
* Name:                                   operator new() - thread safe
* 
* Description: Global C++ operator new
*       
*
* Arguments:   size - size (amount of bytes) to be allocated from the heap
*
* Returns:     pointer to allocated memory
*
* Note(s):     
* 		This function (operator new()) is called by C++ new when object is created to allocate memory.
*       This is special version which does not throw any exceptions.
* *********************************************************************************************************
*/
void *operator new(size_t size) throw() 
{
	
	void* p;//temporary pointer
	Kernel.MemMutex.Acquire();
	p=malloc(size);
	Kernel.MemMutex.Release();
    return p; 
}//*operator new

/*
*********************************************************************************************************
* Name:                                   operator delete() - thread safe
* 
* Description: Global C++ operator new
*       
*
* Arguments:    pointer to heap memory to free 
*
* Returns:    none
*
* Note(s):     
* 		This function (operator delete()) is called by C++ delete when object is deleted from the heap.
*       This is special version which does not throw any exceptions.
* *********************************************************************************************************
*/
void operator delete(void *p) throw() 
{
	Kernel.MemMutex.Acquire();
    free(p);
	Kernel.MemMutex.Release();
}//operator delete



