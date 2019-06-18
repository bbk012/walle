/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_mbox.hpp
* Description: objective wrapper to uCOS-II - messages boxes
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/
#ifndef WRP_MBOX_HPP_
#define WRP_MBOX_HPP_

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"

/*
*********************************************************************************************************
* Name:                            cMailBox Class 
* 
* Description: 	MailBox Class - wraps uCOS Message Mail Box
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
#if OS_MBOX_EN
class cMailBox
   {
   public:
      //default constructor
      cMailBox(void *Message = NULL);
      //checks the mailbox to see if a message is available
      //returns pointer to message or NULL when there is not message
      //IMPORTANT! Will not work with NULL pointers as NULL means no message
      void* Accept(){return ::OSMboxAccept(m_OsEvent);};

      //to send message to the mailbox
      BYTE Send(void *Message){return ::OSMboxPost(m_OsEvent,Message);}

      //to get message from the mailbox 
      //   	TimeOut - timeout period (in clock ticks)
      //	for TimeOut=OS_NO_WAIT the function returns immediately
      // 
      //the result is the pointer to received message or NULL on timeout or when there is not message
      //IMPORTANT! Will not work correctly with NULL messages as for Accept() NULL result
      //           means that there is not message
      void* Receive(WORD TimeOut=OS_INFINITE);

      //obtain information about message mailbox
      BYTE Query(OS_MBOX_DATA* pData){return ::OSMboxQuery(m_OsEvent,pData);}
      
   private:
      OS_EVENT* m_OsEvent;//pointer to the Event Control Block allocated to the mailbox by uCOS
   };//class cMailBox
#endif //OS_MBOX_EN

#endif /*WRP_MBOX_HPP_*/
