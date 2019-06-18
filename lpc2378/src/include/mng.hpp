/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng.hpp
* Description: This defines manager base clase which is a thread (cThread) cabable to subscribe (cSubscribe)
*              and publish (cPublish) notifiers
* Author:      Bogdan Kowalczyk
* Date:        17-Dec-2008
* Notes:
* History:
* 	17-Dec-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef MNG_HPP_
#define MNG_HPP_
#include "type.h"
#include "wrp_thread.hpp"
#include "mw_publisher.hpp"
#include "mw_subscriber.hpp"

/*
*********************************************************************************************************
* Name:                            cMngBasePublisher Class 
* 
* Description: Base class to create managers which are only publishers
*       
*
* *********************************************************************************************************
*/
template <BYTE PublisherSendQSize,WORD ThreadStackSize, BYTE ThreadPriority> 
class cMngBasePublisher: public cThread, public cPublisher<PublisherSendQSize>
{
private:
	OS_STK m_ThreadStack[ThreadStackSize];
public:
	cMngBasePublisher();
};//class cMngBasePublisher
	
template <BYTE PublisherSendQSize,WORD ThreadStackSize, BYTE ThreadPriority> 
cMngBasePublisher<PublisherSendQSize,ThreadStackSize,ThreadPriority>::cMngBasePublisher()
{
	Create((OS_STK *)&m_ThreadStack[0],(OS_STK *)&m_ThreadStack[ThreadStackSize-1],ThreadPriority);	
}

/*
*********************************************************************************************************
* Name:                            cMngBaseSubscriber Class 
* 
* Description: Base class to create managers which are only subscribers
*       
*
* *********************************************************************************************************
*/
template <BYTE SubscriberReceiveQSize,WORD ThreadStackSize, BYTE ThreadPriority> 
class cMngBaseSubscriber: public cThread, public cSubscriber<SubscriberReceiveQSize>
{
private:
	OS_STK m_ThreadStack[ThreadStackSize];
public:
	cMngBaseSubscriber();
};//class cMngBaseSubscriber
	
template <BYTE SubscriberReceiveQSize,WORD ThreadStackSize, BYTE ThreadPriority> 
cMngBaseSubscriber<SubscriberReceiveQSize,ThreadStackSize,ThreadPriority>::cMngBaseSubscriber()
{
	Create((OS_STK *)&m_ThreadStack[0],(OS_STK *)&m_ThreadStack[ThreadStackSize-1],ThreadPriority);	
}


/*
*********************************************************************************************************
* Name:                            cMngBasePublisherSubscriber Class 
* 
* Description: Base class to create managers which are both publishers and subscribers
*       
*
* *********************************************************************************************************
*/
template <BYTE PublisherSendQSize,BYTE SubscriberReceiveQSize,WORD ThreadStackSize, BYTE ThreadPriority> 
class cMngBasePublisherSubscriber: public cThread, public cPublisher<PublisherSendQSize>, public cSubscriber<SubscriberReceiveQSize>
{
private:
	OS_STK m_ThreadStack[ThreadStackSize];
public:
	cMngBasePublisherSubscriber();
};//class cMngBasePublisherSubscriber
	
template <BYTE PublisherSendQSize,BYTE SubscriberReceiveQSize,WORD ThreadStackSize, BYTE ThreadPriority> 
cMngBasePublisherSubscriber<PublisherSendQSize,SubscriberReceiveQSize,ThreadStackSize,ThreadPriority>::cMngBasePublisherSubscriber()
{
	Create((OS_STK *)&m_ThreadStack[0],(OS_STK *)&m_ThreadStack[ThreadStackSize-1],ThreadPriority);	
}



#endif /*MNG_HPP_*/

