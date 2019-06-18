/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_subscriber.cpp
* Description: Defines class to be a base for all notifier subscribers in the communication middleware
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/
#include "mw_subscriber.hpp"

//wait for notifier and if received return it
 cSmartPtrBase cBaseSubscriber::Receive(WORD TimeOut)
 {
	 cSmartPtrBase smartPtr = static_cast<cMemMgrBase*>(m_pReceiveQueue->Receive(TimeOut));
	 if(smartPtr)//when message received smartPtr is initialized with pointer to memory so
		 smartPtr.m_pClass->Dec();//Decrement referencies because we got Notifier out of receive queue
	 return smartPtr;//return valid or invalid smart pointer
 }//cBaseSubscriber::Receive()
