/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_sem.hpp
* Description: objective wrapper to uCOS-II semaphores
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef WRP_SEM_HPP_
#define WRP_SEM_HPP_

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"

/*
*********************************************************************************************************
* Name:                            CountedSemaphore Class 
* 
* Description: Objective wraper to uCOS Semaphore
*       
*
* Arguments:   
*
* Returns:  
*
* Note(s):     
*
* *********************************************************************************************************
*/
#if OS_SEM_EN
class cCountedSemaphore
   {
   public:
      //create and initialize semaphore with a Value
      cCountedSemaphore(WORD Value);
      
      //checks the semaphore to see if a resource is available
      //returns:
      //       0 - resource not availiable
      //      >0 - resource availiable
      WORD Accept(){return ::OSSemAccept(m_OsEvent);}

      //take semaphore - wait on resource or event
      //returns:
      //   OS_NO_ERROR - when semaphore is acquired      
      //   or other values for error same as defined for OSSemPend()
      //   TimeOut - timeout period (in clock ticks) 
      BYTE Acquire(WORD TimeOut=OS_INFINITE);
      
      //release semaphore
      BYTE Release(){return ::OSSemPost(m_OsEvent);}
      
      //obtain information about message queue
      BYTE Query(OS_SEM_DATA* pData){return ::OSSemQuery(m_OsEvent,pData);}
   protected:
      //resets semaphore with a new Value
      //returns:
      //   OS_NO_ERROR - when semaphore value is set
      //   OS_ERR_EVENT_TYPE - when called for no semaphore (theoretically not possible)
      BYTE Set(WORD Value){ return ::OSSemSet(m_OsEvent,Value);}
   private:
      OS_EVENT* m_OsEvent;//pointer to the Event Control Block allocated to the semaphore by uCOS
   };//class cCountedSemaphore

/*
*********************************************************************************************************
* Name:                            cMutex Class 
* 
* Description: 	Mutex is simply a semaphore initialized with value equal one
*       
*
* Arguments:   
*
* Returns:  
*
* Note(s):     
*
* *********************************************************************************************************
*/
class cMutex : public cCountedSemaphore
   {
   public:
      cMutex():cCountedSemaphore(1){}
   };//class Mutex

/*
*********************************************************************************************************
* Name:                            cEvent Class 
* 
* Description: 	Event can be also realized with semaphore which is busy at beginning
*       
*
* Arguments:   
*
* Returns:  
*
* Note(s):     
*
* *********************************************************************************************************
*/ 
class cEvent : public cCountedSemaphore
   {
   public:
      cEvent():cCountedSemaphore(0){}

      //wait for an event
      //TimeOut - timeout period (in clock ticks) 
      BYTE Wait(WORD TimeOut=OS_INFINITE){return Acquire(TimeOut);}

      //signal that event occured
      BYTE Signal(){return Release();}

      //reset Event - gives exaclty same result as creating new cEvent
      //i.e. sets semaphore count to zero.
      void Reset(){Set(0);}
   };//class cEvent
#endif //OS_SEM_EN

#endif /*WRP_SEM_HPP_*/
