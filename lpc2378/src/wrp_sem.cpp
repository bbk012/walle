
/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_sem.cpp
* Description: objective wrapper to uCOS-II semaphores
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/

#include "wrp_sem.hpp"
#include "lib_error.h"

//------------------------------------------------------------------------------
//                     CountedSemaphore Class - wraps uCOS Semaphore
//------------------------------------------------------------------------------
#if OS_SEM_EN
cCountedSemaphore::cCountedSemaphore(WORD Value)
{
	m_OsEvent=::OSSemCreate(Value);
	if(!m_OsEvent)UCOSII_RES_EXCEPTION;//Exception - when there is not uCOS-II event blocks availiable (uCOS-II resources are not availaible) 
}//cCountedSemaphore::cCountedSemaphore


BYTE cCountedSemaphore::Acquire(WORD TimeOut)
{
   BYTE Result;

   if(TimeOut==OS_INFINITE)
      {
      ::OSSemPend(m_OsEvent,0x0000,&Result);//for uCOS-II the value 0x0000 means infinite
      return Result;
      }
   else if(TimeOut==OS_NO_WAIT)
      {
      if(Accept()) //if semaphore can be accepted
         return OS_NO_ERR;
      else //semaphore is not availiable
         return OS_TIMEOUT;
      }
   else
      {
      ::OSSemPend(m_OsEvent,TimeOut,&Result);
      return Result;
      }
}//take semaphore with error
#endif //OS_SEM_EN
