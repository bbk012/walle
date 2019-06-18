/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_thread.hpp
* Description: objective wrapper to uCOS-II tasks
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef WRP_THREAD_HPP_
#define WRP_THREAD_HPP_

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"
#include "lib_error.h"
#include <stddef.h>

/*
*********************************************************************************************************
* Name:                                    cThread             
* 
* Description: 	Thread Virtual Base Class - wraps uCOS task
*   			virtual base class to create threads
*				Every thread must be derived from this base class and overwrite cThread::Run()
*				virtual function which is the thread body.    
*
* Arguments:   
*
* Returns:  
*
* Note(s):     
*
* *********************************************************************************************************
*/ 

class cThread
{
   public:
      //default constructor put thread into default state without running
	  //m_pStack is generated when Create is called
      cThread(){m_pStackTop=static_cast<OS_STK*>(NULL);m_pStackBot=static_cast<OS_STK*>(NULL);m_Id=0;};
#if OS_TASK_CREATE_EN
      //create thread and run it
      //       pStackBot - pointer to stack bottom (Stack[0]) - used only by cThread's CheckStack() method
      //       pStackTop - pointer to the stack top (Stack[Max-1]) - required by uCOSII
      //       Priority - task priority
      cThread(OS_STK *pStackBot,OS_STK *pStackTop,BYTE Priority){Create(pStackBot,pStackTop,Priority);}
#endif //OS_TASK_CREATE_EN
      virtual ~cThread(){};//virtual destructor
#if OS_TASK_CREATE_EN
      //	run thread
      //    pStackBot - pointer to stack bottom - required only by CheckStack() method
      //    pStackTop - pointer to the stack top i.e. (OS_STK *)&Task1Stk[TASK_STK_SIZE-1] because in ARM stack
      //             grows from high adresses to low one (from top to bottom)
      //    Priority - task priority
      //    When not created a SWI exception error is generated
      void Create(OS_STK *pStackBot,OS_STK *pStackTop,BYTE Priority);
#endif //OS_TASK_CREATE_EN
      
      // Count number of thread's stack free bytes
      // Return:
      //       - number of free bytes on the thread stack
      WORD CheckStack(void);
      // return number of bytes of the stack
      WORD StackSize(void);
      //return task Id or its pririty as they are the same
      BYTE GetThreadId(){return m_Id;};
      BYTE GetThreadPriority(){return m_Id;};
      
   protected:
      //delay task for the number of ticks
      void Delay(WORD Ticks){::OSTimeDly(Ticks);}//delay thread   
   private:
      //trick - not implemented private operator new() disables heap instantination
	   static void* operator new(size_t)throw(){return NULL;};
   
      //pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
      virtual void Run(){THREAD_NO_RUN_DEFINED;};

      //wraper which is passed to uCOS OSTaskCreate and calls virtual Run()
      static void Execute(void *pThread);
      
      OS_STK *m_pStackTop;//pointer to used stack top (passed to Create function)
      OS_STK *m_pStackBot;//pointer to stack bottom - this pointer is not required by uCOS-II but is used by wrapper to check stack usage
      BYTE m_Id;//task ID which in uCOS is the task priority
};//class cThread

#endif /*WRP_THREAD_HPP_*/
