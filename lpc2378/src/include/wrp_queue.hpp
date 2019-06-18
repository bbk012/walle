/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_queue.hpp
* Description: objective wrapper to uCOS-II queues
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef WRP_QUEUE_HPP_
#define WRP_QUEUE_HPP_

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"

/*
*********************************************************************************************************
* Name:                            cBaseQueue Class 
* 
* Description: 	Base clase needed to create uCOS wraper of its Message Queue
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
#if OS_Q_EN
class cBaseQueue
   {
   public:
      //create queue tied with specified storage area
      cBaseQueue(void**pMessageStorage,WORD Size);  

      //to send message to the queue end
      BYTE Send(void *Message){return ::OSQPost(m_OsEvent,Message);}

      //to send message to the queue front
      BYTE SendFront(void *Message){return ::OSQPostFront(m_OsEvent,Message);}

      //checks the queue to see if a message is available
      //returns pointer to message or NULL when there is not message
      //IMPORTANT! Will not work with NULL pointers as NULL means no message
      void* Accept(){return ::OSQAccept(m_OsEvent);}

      //get message from the message Queue
      //   	TimeOut - timeout period (in clock ticks)
      //returns pointer to message or NULL when there is timeout or ther is not message
      //IMPORTANT! Will not work correctly with NULL messages as for Accept() NULL result
      //           means that there is not message
      void* Receive(WORD TimeOut=OS_INFINITE);

      //flush the contents of the message queue.
      BYTE Flush(){return ::OSQFlush(m_OsEvent);}
      
   protected:
      OS_EVENT* m_OsEvent;//pointer to the Event Control Block allocated to the queue by uCOS
   };//cBaseQueue

template <BYTE Size>//Size number of messages which can be stored in queue
class cQueue:public cBaseQueue
   {
   public:
      cQueue():cBaseQueue(m_pMessageStorage,Size){}//default constructor
   private:
      void *m_pMessageStorage[Size];
   };//class cQueue

#endif //OS_Q_EN

#endif /*WRP_QUEUE_HPP_*/
