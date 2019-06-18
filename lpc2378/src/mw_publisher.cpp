/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_publisher.cpp
* Description: Defines class to be a base for all publishers in the communication middleware
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/

#include "mw_publisher.hpp"


BYTE cBasePublisher::Post(cSmartPtrBase& smartPtr)
{
	BYTE Result;//temporary result holder
	
	if(!smartPtr.isValid())return OS_ERR_EVENT_TYPE;//if smartPtr does not point any valid managed memory do nothing
	
	smartPtr.m_pClass->Inc();//increment referencies to Notifier befor we place it into Send queue
	
	if ((static_cast<cNotifier*>(smartPtr.m_pClass))->GetHandling() & NT_HND_HIGH_PRT)//if high priority Notifier handling requested
	{
		Result = m_pSendQueue->SendFront(smartPtr.m_pClass);
	}else
	{
		Result = m_pSendQueue->Send(smartPtr.m_pClass);
	}
	if(Result != OS_NO_ERR ) //notifier was not placed in the send queue because of error
	{
		smartPtr.m_pClass->Dec();//return back to original number of referencies to avoid mem leak
	}
	return Result;
	
}//cBasePublisher::Post

