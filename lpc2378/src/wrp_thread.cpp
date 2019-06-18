/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_thread.cpp
* Description: objective wrapper to uCOS-II tasks
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/
#include "wrp_thread.hpp"

//------------------------------------------------------------------------------
//                 Thread Related Member Functions
//------------------------------------------------------------------------------


//Input:
//    pThread - pointer to thread for which Execute is called
//Output:
//    none
//Description:
//    This function is used to call virtual Thread::Run when task is created
//    The trick is required as there is not possibility to pass method to OSTaskCreate and
//    to OSTaskCreateExt 
void cThread::Execute(void *pThread)
   {
   (static_cast<cThread*>(pThread))->Run();//Start() passes to Execute() pointer to Thread class
   }//Thread::Execute

#if OS_TASK_CREATE_EN
//	run thread
//    pStackBot - pointer to stack bottom - required only by CheckStack() method
//    pStackTop - pointer to the stack top i.e. (OS_STK *)&Task1Stk[TASK_STK_SIZE-1] because in ARM stack
//             grows from high adresses to low one (from top to bottom)
//    Priority - task priority
//Example:
//
//      OS_STK Task1Stk[TASK_STK_SIZE];
//
//      
//      Thread1.Create((OS_STK *)&Task1Stk[0],(OS_STK *)&Task1Stk[TASK_STK_SIZE-1],(INT8U)5);

void cThread::Create(OS_STK *pStackBot, OS_STK *pStackTop,BYTE Priority)
{
	OS_STK *pStack;//temporary pointer to stack entry
	//when stack not specified raise an exception
	if(!pStackTop) THREAD_CREATE_EXCEPTION;
	if(!pStackBot) THREAD_CREATE_EXCEPTION;
	if(!(pStackBot < pStackTop)) THREAD_CREATE_EXCEPTION;
	pStack=pStackBot;
	//clear stack before task creation
	while(pStack <= pStackTop)
	{
		*pStack=0;
		pStack++;
	}//while
	
	//when thread not created rise an exception
    if(::OSTaskCreate(&cThread::Execute,this,pStackTop,Priority)!= OS_NO_ERR) THREAD_CREATE_EXCEPTION;
    
    m_pStackBot = pStackBot;//pointer to stack bottom - this pointer is not required by uCOS-II but is used by wrapper to check stack usage
    m_pStackTop = pStackTop;//pointer to used stack top (passed to Create function)
    m_Id=Priority;//setup task priority to preserve it
   }//Thread::Create
#endif //OS_TASK_CREATE_EN

// Return number of free bytes on stack
WORD cThread::CheckStack(void)
{
	OS_STK *pStack=m_pStackBot;//temporary pointer to stack entry
	WORD free=0;//holds number of stack free entries
	while(pStack <= m_pStackTop && *pStack == 0)
	{
		free++;//count not used stack entries
		pStack++;
	}//while
	return free*sizeof(OS_STK);//translate number of free stack entries into number of free bytes
}//cThread::CheckStack

//return number of bytes reserved for a stack
WORD cThread::StackSize(void)
{
	return (sizeof(OS_STK)*(m_pStackTop-m_pStackBot+1));//count number of OS_STK entries and multiply bu OS_STK size
	
}//cThread::StackSize

