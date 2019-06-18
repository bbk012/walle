/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_kernel.hpp
* Description: objective wrapper to uCOS-II - defines singleton kernel which is OS for the system
* Author:      Bogdan Kowalczyk
* Date:        4-November-2008
* Note:
* History:
*              4-November-2008 - Initial version created
*********************************************************************************************************
*/
#ifndef WRP_KERNEL_HPP_
#define WRP_KERNEL_HPP_

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"
#include "wrp_thread.hpp"
#include "wrp_sem.hpp"
#include "mw_dispatcher.hpp"


#define WALLE_RUN			"WALL-e SW RUN" 
#define WALLE_OS_VERSION	"uC/OS-II 2.0"
#define WALLE_VERSION		"SW VERSION: 6.0.1" 
#define WALLE_PRG_CHANGE	"PROGRAM CHANGED"
#define WALLE_PRG_NO		"PROGRAM ID: "

//define to access cKernel singleton through the Kernel keyword
//for example: 
//          Kernel.Version()
#define Kernel cKernel::Api()

//delay befor Timer3 ticks are taken to setup random number generator seed
#define RAND_SEED_SETUP_DELAY_TCIKS 10

//defines period in OS Ticks for which Dispatchers works
#define DISPATCH_PERIOD_IN_OS_TICKS 1

//defines priority of the thread which dispatches notifiers - it is the highest priority in the system
#define DISPATCHER_THREAD_PRIORITY  3

//define max number of publishers in the system
#define NO_OF_PUBLISHERS  	12

//define max number of subscribers in the system
#define NO_OF_SUBSCRIBERS	12

//size of the stack for dispatching thread
#define DISPATCHER_THREAD_STACK_SIZE OS_TASK_STACK_SIZE

//------------------------------------------------------------------------------
//                     Singleton Kernel Class - wraps uCOS API
//------------------------------------------------------------------------------
//IMPORTANT! The Kernel is singleton created as global.


//IMPORTANT!!
//m_KernelInit this is empty initializer  class used to assure ::OSInit() is called before any other Kernel member
//object is constructed - otherwise cMutex below will throw an exception as uCOS-II is not initilaized
//We use trick: member objects constructor are called in the order of declaration
class cKernelInit
	{
	public:
		cKernelInit();
	};

//this is class used to instantinate dispatching thread for Kernel
class cDispatchThread:public cThread
      {
    	  virtual void Run();
      };
      
class cKernel
   {
	private:
	  cKernelInit m_KernelInit;//IMPORTANT! need to be first member of cKernel - see comment above for cKernelInit class
	  OS_STK m_DispatcherThreadStack[DISPATCHER_THREAD_STACK_SIZE];//stack for dispatching thread
	  static cKernel m_Kernel;//required to make the class a singleton one
	  cDispatchThread m_DispatcherThread;//dispatcher thread to periodically dispatch notifiers from publishers to subscribers
	  //private(!!!!) constructor to have singleton
	  cKernel();
   
   public:
	  cDispatcher<NO_OF_PUBLISHERS,NO_OF_SUBSCRIBERS> Dispatcher;//one Kernel has one Dispatcher
      static cKernel& Api(){return m_Kernel;}//function to get access to kernel API
      void Start(){::OSStart();};//call to start uCOS multitasking
#if OS_TASK_CHANGE_PRIO_EN      
      //change priority
      BYTE ThreadChangePriority(BYTE OldPriority,BYTE NewPriority){return ::OSTaskChangePrio(OldPriority,NewPriority);}//change thread priority
#endif //OS_TASK_CHANGE_PRIO_EN 
      
#if OS_TASK_DEL_EN      
      //delete thread
      BYTE ThreadDelete(BYTE Priority){return ::OSTaskDel(Priority);}

      //request thread to delete itself
      BYTE ThreadDeleteRequest(BYTE Priority){return ::OSTaskDelReq(Priority);}
#endif //OS_TASK_DEL_EN

      //obtain information about a task and stor it in the place pointed by pData
      BYTE ThreadQuery(BYTE Priority,OS_TCB* pData){return ::OSTaskQuery(Priority,pData);}

#if OS_TASK_SUSPEND_EN
      //block thread execution
      BYTE ThreadSuspend(BYTE Priority){return ::OSTaskSuspend(Priority);}
      //ressume thread suspended by ThreadSuspend
      BYTE ThreadResume(BYTE Priority){return ::OSTaskResume(Priority);}
#endif //OS_TASK_SUSPEND_EN

      //delay calling thread for the number of ticks
      void Delay( INT16U  Tcks){::OSTimeDly(Tcks);}//delay thread
      
      //ressume thread delayed by cThread::Delay or by cKernel::Delay
      BYTE ThreadWake(BYTE Priority){return ::OSTimeDlyResume(Priority);}
      
      //delay execution of the currently running task until some time expires
      INT8U TimeDlyHMSM(INT8U Hours, INT8U Minutes, INT8U Seconds, INT16U Milli){return ::OSTimeDlyHMSM(Hours,Minutes,Seconds,Milli);}
      
      //obtain current value of the Kernel ticks
      DWORD Ticks(){return ::OSTimeGet();}

      //get number of ticks per second for RTOS
      WORD  TicksPerSecond(){return OS_TICKS_PER_SEC;}
      
      //prevent task rescheduling
      void LockScheduler(){::OSSchedLock();}

      //re-enable task scheduling
      void UnlockScheduler(){::OSSchedUnlock();}

      //get uCOS-II version number
      WORD Version(){return ::OSVersion();}

      //return the OS running state
      static BYTE IsRunning() {return ::OSRunning;}
               
      //this is global mutext to protect new and delete and make them thread safety
      //this mutex is acquired by function operator new before memory allocation and released after that
      //the same is for function operator delete see newlpc.cpp for details.
      cMutex MemMutex; 
      
      //this is mutext which protects reference counting operation for all objects (Notifiers)
      //which derivate from cMemMgrBase
      cMutex MemMgrMutex;
      
      friend class cDispatchThread;//DispatchThread is allowed to get access to all cKernel data

   };//cKernel

#endif /*WRP_KERNEL_HPP_*/

