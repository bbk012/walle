/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_publisher.hpp
* Description: Defines class to be a base for all publishers in the communication middleware
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef PUBLISHER_HPP_
#define PUBLISHER_HPP_

#include "mw_notifier.hpp"
#include "mw_smart_ptr.hpp"
#include "wrp_queue.hpp"
#include "type.h"

/*
*********************************************************************************************************
* Name:                            cBasePublisher Class 
* 
* Description: Base class for the template of publisher which defines common publisher interface
*       
*
* *********************************************************************************************************
*/

class cBasePublisher
{
private:
	cBaseQueue* m_pSendQueue;//pointer to queue used to publish notifications
public:
	cBasePublisher(cBaseQueue* pSendQueue){ m_pSendQueue=pSendQueue;};
	cBaseQueue* GetSendQueue(){return m_pSendQueue;};
	BYTE Post(cSmartPtrBase &smartPtr);
};//cBasePublisher



/*
*********************************************************************************************************
* Name:                            cPublisher Class 
* 
* Description: Base template class to create Publisher with a send queue of specified size
*              Size is template input
*       
*          
**********************************************************************************************************
*/
template <BYTE Size> class cPublisher:public cBasePublisher
{
private:
	cQueue<Size> m_SendQueue;
public:
	cPublisher():cBasePublisher(static_cast<cBaseQueue*>(&m_SendQueue)){};
};//cPublisher
#endif /*PUBLISHER_HPP_*/
