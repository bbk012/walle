/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_subscriber.hpp
* Description: Defines class to be a base for all notifier subscribers in the communication middleware
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef SUBSCRIBER_HPP_
#define SUBSCRIBER_HPP_

#include "type.h"
#include "wrp_queue.hpp"
#include "mw_notifier.hpp"

/*
*********************************************************************************************************
* Name:                            cBaseSubscriber Class 
* 
* Description: Base class for the template of subscriber which defines common subscriber interface
*       
*
* *********************************************************************************************************
*/
class cBaseSubscriber
{
private:
	cBaseQueue* m_pReceiveQueue;//pointer to queue used to receive notifications by subscriber
public:
	 cBaseSubscriber(cBaseQueue* pReceiveQueue){ m_pReceiveQueue=pReceiveQueue;};
	 cBaseQueue* GetReceiveQueue(){return m_pReceiveQueue;}; 
	 cSmartPtrBase Receive(WORD TimeOut=OS_INFINITE);//receive notifier
};//cBaseSubscriber

/*
*********************************************************************************************************
* Name:                            cSubscriber Class 
* 
* Description: Base template class to create Subscriber with a receive queue of specified size.
*              Size is template input.
*       
*          
**********************************************************************************************************
*/
template <BYTE Size> class cSubscriber:public cBaseSubscriber
{
private:
	cQueue<Size> m_ReceiveQueue;
public:
	cSubscriber():cBaseSubscriber(static_cast<cBaseQueue*>(&m_ReceiveQueue)){};
};//cSubscriber

#endif /*SUBSCRIBER_HPP_*/
