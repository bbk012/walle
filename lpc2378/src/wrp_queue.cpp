/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_queue.cpp
* Description: objective wrapper to uCOS-II queues
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/

#include "wrp_queue.hpp"
#include "lib_error.h"

//------------------------------------------------------------------------------
//                     OsQueue Class - wraps uCOS Message Queue
//------------------------------------------------------------------------------
#if OS_Q_EN

cBaseQueue::cBaseQueue(void**pMessageStorage,WORD Size)
{
	m_OsEvent=::OSQCreate(pMessageStorage,Size);
	if(!m_OsEvent)UCOSII_RES_EXCEPTION;//Exception - when there is not uCOS-II event blocks availiable (uCOS-II resources are not availaible)
}//cBaseQueue::cBaseQueue

//Input:
//    TimeOut how long task should wait for the message
//Output:
//    pointer to received message or NULL when there is not message or timeout
//Description:
//IMPORTANT! Will not work correctly with NULL messages as for Accept() NULL result
//           means that there is not message
//get message from the message Queue 
//for TimeOut=OS_NO_WAIT the function returns immediately 
//the result is the pointer to received message or NULL on timeout or when there is not message
void* cBaseQueue::Receive(WORD TimeOut)
   {
   BYTE Result=OS_NO_ERR;//assume no arror when below called function does ont setup Result
   if(TimeOut==OS_INFINITE)
      return ::OSQPend(m_OsEvent,0x0000,&Result);//for uCOS-II the value 0x0000 means infinite
   else if(TimeOut==OS_NO_WAIT)
      return Accept();
   else
      return ::OSQPend(m_OsEvent,TimeOut,&Result);
   }//cBaseQueue::Receive
#endif //OS_Q_EN
