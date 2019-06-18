/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_mbox.cpp
* Description: objective wrapper to uCOS-II - messages boxes
* Author:      Bogdan Kowalczyk
* Date:        17-December-2008
* Note:
* History:
*              17-December-2008 - Initial version created
*********************************************************************************************************
*/
#include "wrp_mbox.hpp"
#include "lib_error.h"

//------------------------------------------------------------------------------
//                     MailBox Class - wraps uCOS Mail Box
//------------------------------------------------------------------------------
#if OS_MBOX_EN
cMailBox::cMailBox(void *Message)
{
	m_OsEvent=::OSMboxCreate(Message);
	if(!m_OsEvent)UCOSII_RES_EXCEPTION;//Exception - when there is not uCOS-II event blocks availiable (uCOS-II resources are not availaible)
}//cMailBox::cMailBox


//Input:
//    TimeOut how long task should wait for the message
//Output:
//    pointer to received message or NULL when there is not message or timeout
//Description:
//to get message from the mailbox 
//IMPORTANT! Will not work correctly with NULL messages as for Accept() NULL result
//           means that there is not message
//for TimeOut=OS_NO_WAIT the function returns immediately 
//the result is the pointer to received message or NULL on timeout or when there is not message
void* cMailBox::Receive(WORD TimeOut)
   {
   BYTE Result;
   if(TimeOut==OS_INFINITE)
      return ::OSMboxPend(m_OsEvent,0x0000,&Result);//for uCOS-II the value 0x0000 means infinite
   else if(TimeOut==OS_NO_WAIT)
      return Accept();
   else
      return ::OSMboxPend(m_OsEvent,TimeOut,&Result);
   }
#endif //OS_MBOX_EN
