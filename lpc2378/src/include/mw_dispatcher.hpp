/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_dispatcher.hpp
* Description: Key communication middleware class.
*              Defines dispatcher capable to dispatch notifiers from registered publishers to registered
*              subscribers without subscribers knowing publishers and vice versa.
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/
#ifndef DISPATCHER_HPP_
#define DISPATCHER_HPP_

#include "type.h"
#include "lib_error.h"
#include "mw_notifier.hpp"
#include "wrp_sem.hpp"
#include "mw_publisher.hpp"
#include "mw_subscriber.hpp"

//forward declaration
class cDispatchThread;

//defines for cDispatcher return errors
#define DISPATCHER_NO_ERROR			0
#define PUBLISHER_REGISTER_ERROR	2
#define SUBSCRIBER_REGISTER_ERROR	4
#define PUBLISHER_UNREGISTER_ERROR	8
#define SUBSCRIBER_UNREGISTER_ERROR	10

/*
*********************************************************************************************************
* Name:                            cDispatcher Class 
* 
* Description: Key class of middleware keeps list of publishers and subscribers and
*              dispatches notifiers from publisher to their subscribers
*       
*
* *********************************************************************************************************
*/
template <BYTE PublisherTableSize,BYTE SubscriberTableSize> 
class cDispatcher
{
private:
	// NotifierIdFlags
	//           |31|30|29|......................|2|1|0|
	//           Bit0=1 - Notifier ID = 1 to be received by subscriber
	//           Bit1=1 - Notifier ID = 2 to be receved by subscriber
	//
	//           Bit31=1 - Notifier ID = 32 to be received by subscriber
	struct sSubscriberEntry
	{
		cBaseSubscriber* m_pSubscriber;//address of subscriber for notifiers defined by m_IdFilags 
		DWORD m_NotifierIdFlags;// list of Notifiers m_pSubscribers wants to receive
		WORD m_Error_Counter;//counts errors when dispatcher cannot put Notifier to the receiver queue
	};
	WORD m_Total_Dispatch_Error_Counter;//counts errors total number of Notifier dipstacher errors not related to particular subscriber
	cBasePublisher* m_PublisherTable[PublisherTableSize];
	sSubscriberEntry m_SubscriberTable[SubscriberTableSize];
	cMutex m_PublisherMutex, m_SubscriberMutex; //mutexes to synchronize access to subscriber and publisher tables
	void DispatchNotifier(cNotifier* pNotifier); //dispatch Notifier pointed by pNotifier to its all subscribers
	
	//makes one dispatching sequence i.e. polls all publisher send queues
	//and get notifiers out of them dispatching to receive queues of all registered subscribers
	void Dispatch();
public:
	
	//constructor - initialize empty publisher and subscribers lists
	cDispatcher();
	
	//return number of counted cases when notifier was not dispatched because of subscriber receive queue problems
	inline WORD GetTotalDispatchErrorCounter(void){return m_Total_Dispatch_Error_Counter;};
	
	//register publishers to know what send queue need to be polled
	//IMPORTANT! Exception is generated when publisher cannot be registered
	void RegisterPublisher(cBasePublisher& rPublisher);
	
	//remove publisher from the list of registered publishers
	void UnregisterPublisher(cBasePublisher& rPublisher);
	
	//pSubscriber subscribes for specified notifiers usung notifiers mask
	//
	//input:
	//	pSubscriber - adress of subscriber which subscribes for notifiers
	//  NotifierIdMask - mask of notifiers ID subscriber subscribes
	//IMPORTANT! Exception is generated when subscriber cannot be registered
	void RegisterSubscriber(cBaseSubscriber& rSubscriber, DWORD NotifierIdMask);
	
	// rSubscriber unsubscribes for specified notifiers usung notifiers mask or completely unsubscribe
	//
	//input:
	//	rSubscriber - subscriber which unsubscribes for notifiers
	//  NotifierIdMask - mask of notifiers ID subscriber unsubscribes - NT_NONE means remove subscriber form the list
	void UnregisterSubscriber(cBaseSubscriber& pSubscriber, DWORD NotifierIdMask=NT_NONE);
	
	//get mask of notifiers subscriber subscribe for
	DWORD GetSubscriberNotifiers(BYTE InSubscriberNo);
	//get counter of not received notifiers of the subscriber
	WORD GetNotifierErrorCounter(BYTE InSubscriberNo);
	
	friend class cDispatchThread;//only cKernel's cDispatchThread is allowed to dispatch notifiers
};//cDispatcher

//create dispatcher and setup publisher and subscriber lists as empty
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
cDispatcher<PublisherTableSize,SubscriberTableSize>::cDispatcher()
{
	BYTE i;

	for(i=0;i<PublisherTableSize;i++)
	{
		m_PublisherMutex.Acquire();//get O.K. for access
		m_PublisherTable[i]=static_cast<cBasePublisher*>(NULL);
		m_PublisherMutex.Release();//release mutex to signal that resource is free
	};
	for(i=0;i<SubscriberTableSize;i++)
	{
		m_SubscriberMutex.Acquire();//get O.K. for access
		m_SubscriberTable[i].m_pSubscriber=static_cast<cBaseSubscriber*>(NULL);
		m_SubscriberTable[i].m_NotifierIdFlags = NT_NONE;
		m_SubscriberTable[i].m_Error_Counter = 0;//none errors when initialized
		m_SubscriberMutex.Release();//release mutex to signal that resource is free
	};
	m_Total_Dispatch_Error_Counter=0;//none errors when initialized
}//cDispatcher


//register publishers to know what send queue need to be polled
//return:
//	DISPATCHER_NO_ERROR - when no error and Publisher is registered in dispatcher
//  PUBLISHER_REGISTER_ERROR - when cannot register publsher no place for it
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
void cDispatcher<PublisherTableSize,SubscriberTableSize>::RegisterPublisher(cBasePublisher& rPublisher)
{
	BYTE i;
	//check if publisher not already registered
	for(i=0;i<PublisherTableSize;i++)
	{
		m_PublisherMutex.Acquire();//get O.K. for access
		if(m_PublisherTable[i]==&rPublisher)//check if we do not have publisher already registered
			{
			m_PublisherMutex.Release();//release mutex to signal that resource is free
			return; 
			};
		m_PublisherMutex.Release();//release mutex to signal that resource is free
	};
	//publisher not already registered find free place and register
	for(i=0;i<PublisherTableSize;i++)
	{
		m_PublisherMutex.Acquire();//get O.K. for access
		//find first empty entry in m_PublisherTable and register publisher
		if(m_PublisherTable[i]==static_cast<cBasePublisher*>(NULL))
			{
			m_PublisherTable[i]=&rPublisher;
			m_PublisherMutex.Release();//release mutex to signal that resource is free
			return;
			};
		m_PublisherMutex.Release();//release mutex to signal that resource is free
	};
	//if we got this place there is not free place in m_PublisherTable to register publisher so generate an exception
	PUBLISHER_REGISTER_EXCEPTION;
}// cDispatcher::RegisterPublisher


//remove publisher from the list of registered publishers
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
void cDispatcher<PublisherTableSize,SubscriberTableSize>::UnregisterPublisher(cBasePublisher& rPublisher)
{
	BYTE i;
	//check if publisher registered
	for(i=0;i<PublisherTableSize;i++)
	{
		m_PublisherMutex.Acquire();//get O.K. for access
		if(m_PublisherTable[i]==&rPublisher)//remove it when found
			{
			m_PublisherTable[i]=static_cast<cBasePublisher*>(NULL);
			m_PublisherMutex.Release();//release mutex to signal that resource is free
			return; 
			};
		m_PublisherMutex.Release();//release mutex to signal that resource is free
	};
	//if we get this place there was not publisher subscribed so it is unsubscribed anyhow
	return; 
}//cDispatcher::UnregisterPublisher


//rSubscriber subscribes for specified notifiers usung notifiers mask
//
//input:
//	rSubscriber - subscriber which subscribes for notifiers
//  NotifierIdMask - mask of notifiers ID subscriber subscribes
//return:
//	DISPATCHER_NO_ERROR - when no error and subscriber is registered in dispatcher for notifiers
//  SUBSCRIBER_REGISTER_ERROR - when cannot register subscriber for notifiers
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
void cDispatcher<PublisherTableSize,SubscriberTableSize>::RegisterSubscriber(cBaseSubscriber& rSubscriber, DWORD NotifierIdMask)
{
	BYTE i;
	//check if subscriber not already registered
	for(i=0;i<SubscriberTableSize;i++)
	{
		m_SubscriberMutex.Acquire();//get O.K. for access
		if(m_SubscriberTable[i].m_pSubscriber==&rSubscriber)//if subscriber already registered update its mask
			{
			m_SubscriberTable[i].m_NotifierIdFlags|=NotifierIdMask;//set mask bits as requested by NotifierIdMask
			m_SubscriberMutex.Release();//release mutex to signal that resource is free
			return; 
			};
		m_SubscriberMutex.Release();//release mutex to signal that resource is free
	};
	//subscriber not already registered find free place and register
	for(i=0;i<SubscriberTableSize;i++)
	{
		m_SubscriberMutex.Acquire();//get O.K. for access
		//find first empty entry in m_PublisherTable and register subscriber
		if(m_SubscriberTable[i].m_pSubscriber==static_cast<cBaseSubscriber*>(NULL))
			{
			m_SubscriberTable[i].m_pSubscriber=&rSubscriber;//register subscriber
			m_SubscriberTable[i].m_NotifierIdFlags=NotifierIdMask;//set mask bits as requested by NotifierIdMask
			m_SubscriberTable[i].m_Error_Counter = 0;//none errors when initialized
			m_SubscriberMutex.Release();//release mutex to signal that resource is free
			return;
			};
		m_SubscriberMutex.Release();//release mutex to signal that resource is free
	};
	//if we got this place there is not free place in m_SubscriberTable to register subscriber so generate an exception
	SUBSCRIBER_REGISTER_EXCEPTION;	
}//cDispatcher::RegisterSubscriber


// rSubscriber unsubscribes for specified notifiers usung notifiers mask or completely unsubscribe
//
//input:
//	rSubscriber - subscriber which unsubscribes for notifiers
//  NotifierIdMask - mask of notifiers ID subscriber unsubscribes - NT_NONE means remove subscriber form the list
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
void cDispatcher<PublisherTableSize,SubscriberTableSize>::UnregisterSubscriber(cBaseSubscriber& rSubscriber, DWORD NotifierIdMask)
{
	BYTE i;
	//check if subscriber not already registered
	for(i=0;i<SubscriberTableSize;i++)
	{
		m_SubscriberMutex.Acquire();//get O.K. for access
		if(m_SubscriberTable[i].m_pSubscriber==&rSubscriber)//if subscriber registered update its mask
		{
			if(NotifierIdMask==NT_NONE)//if request to unsubscribe completely
			{
				m_SubscriberTable[i].m_NotifierIdFlags=NT_NONE;//clear mask bits
				m_SubscriberTable[i].m_pSubscriber=static_cast<cBaseSubscriber*>(NULL);//mark entry in subscriber table as empty
				m_SubscriberTable[i].m_Error_Counter = 0;//none errors when unsubscribed
			}else
			{
				m_SubscriberTable[i].m_NotifierIdFlags&=(~NotifierIdMask);//clear mask bits as requested by NotifierIdMask&=(~NotifierIdMask)	
			}
		m_SubscriberMutex.Release();//release mutex to signal that resource is free
		return; 
		};
		m_SubscriberMutex.Release();//release mutex to signal that resource is free
	};
	//if we got this place there is not subscriber registered so there is nothing to unregister
	return;		
}//cDispatcher::UnregisterSubscriber

//pNotifier is the address of Notifier to be dispatched to all its subscribers receive queue
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
void cDispatcher<PublisherTableSize,SubscriberTableSize>::DispatchNotifier(cNotifier* pNotifier)
{
	BYTE i;
	BYTE Result;//hold result of Send operation
	//go through whole subscriber list
	for(i=0;i<SubscriberTableSize;i++)
	{
		m_SubscriberMutex.Acquire();//get O.K. for access the subscriber lists

		if((m_SubscriberTable[i].m_NotifierIdFlags & (pNotifier->GetNotifierId()))&& (m_SubscriberTable[i].m_pSubscriber))//if subscriber wants to received this notifier ID send it
		{
			pNotifier->Inc();//increment referencies to Notifier befor we place it into receiver queue of subscriber
			if(pNotifier->GetHandling() & NT_HND_HIGH_PRT)//if high priority notifier handling set
			{//put notifier on the front of the queue if high priority treatment requested
				Result=((m_SubscriberTable[i].m_pSubscriber)->GetReceiveQueue())->SendFront(pNotifier);	
			}else
			{//standard handling of notifier requested
				Result=((m_SubscriberTable[i].m_pSubscriber)->GetReceiveQueue())->Send(pNotifier);
			}
			if (Result!=OS_NO_ERR)//if Notifier was not able to be delivered
			{
			//may DO: Consider to empty Queue when full
			//To make full queue empty you cannot flash it but only to Receive notifier by notifier to release	
			//occupied memory via SmartPointer automatically	
			m_SubscriberTable[i].m_Error_Counter += 1;//increment error count as Notifier was not transfered
			m_Total_Dispatch_Error_Counter+=1; //not only individual subscriber erro counter is increased but also total one

			pNotifier->Dec();//decrement referencies because Notifier was finally not placed into receiver queue of subscriber
			};
		}
		m_SubscriberMutex.Release();//release mutex to signal that resource is free
	}
}//cDispatcher::DispatchNotifier


//makes one dispatching sequence i.e. polls all publisher send queues
//and get notifiers out of them dispatching to receive queues of all registered subscribers
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
void cDispatcher<PublisherTableSize,SubscriberTableSize>::Dispatch()
{
	BYTE i;
	//check if publisher registered
	cNotifier* pNotifier;//temporary message holder

	//go through all publishers
	for(i=0;i<PublisherTableSize;i++)
	{
		m_PublisherMutex.Acquire();//get O.K. for access
		if (m_PublisherTable[i])//if there is any publisher registered at this table position
		{
			pNotifier=static_cast<cNotifier*>((m_PublisherTable[i]->GetSendQueue())->Accept());//get message from the publisher's send queue
			if(pNotifier)//if Notifier received
			{
				DispatchNotifier(pNotifier);//dispatch Notifier to all subscribers queues
				pNotifier->Dec();//decrement reference count because pNotifier is not any longer kept in Send queue
			}
		}
		m_PublisherMutex.Release();//release mutex to signal that resource is free
	}	
}//cDispatcher<PublisherTableSize,SubscriberTableSize>::Dispatch

//get mask of notifiers subscriber subscribe for
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
DWORD cDispatcher<PublisherTableSize,SubscriberTableSize>::GetSubscriberNotifiers(BYTE InSubscriberNo)
{
	if(InSubscriberNo < SubscriberTableSize)//if that is from the range of allwed subscribes
		return m_SubscriberTable[InSubscriberNo].m_NotifierIdFlags;
	else
		return NT_NONE;
}//cDispatcher<PublisherTableSize,SubscriberTableSize>::GetSubscriberNotifiers(BYTE InSubscriberNo)

 
//get counter of not received notifiers of the subscriber
template <BYTE PublisherTableSize,BYTE SubscriberTableSize>
WORD cDispatcher<PublisherTableSize,SubscriberTableSize>::GetNotifierErrorCounter(BYTE InSubscriberNo)
{
	if(InSubscriberNo < SubscriberTableSize)//if that is from the range of allwed subscribes
		return m_SubscriberTable[InSubscriberNo].m_Error_Counter;
	else
		return 0;
}//cDispatcher<PublisherTableSize,SubscriberTableSize>::GetNotifierErrorCounter(BYTE InSubscriberNo)

#endif /*DISPATCHER_HPP_*/

