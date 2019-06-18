/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_rtc.hpp
* Description: Robot's Real Time Clock manager
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/

#ifndef MNG_RTC_HPP_
#define MNG_RTC_HPP_
#include "mng.hpp"
#include "hw_rtc.h"


#define  RTC_PROCESSING_TICKS      		10 //number of OS ticks rtc manager is waiting for a command before it starts its bacground task
#define  RTC_MAX_TO_CHECK_TIME_ALARM	10 //how often in RTC_PROCESSING_TICK*OS_TICK time ALARAM status and TIME is reported 10 means 1 about second

#define  RTC_PUBLISHER_SEND_Q_SIZE 	5
#define  RTC_SUBSCRIBER_REC_Q_SIZE 	5
#define  RTC_THREAD_STACK_SIZE		128
#define  RTC_THREAD_PRIORITY   		15


class cRtcMngr:public cMngBasePublisherSubscriber<RTC_PUBLISHER_SEND_Q_SIZE,RTC_SUBSCRIBER_REC_Q_SIZE,RTC_THREAD_STACK_SIZE,RTC_THREAD_PRIORITY>
{
private:
		tmElements_t mRtcDateTime;//storage place for date and time 
		
		//when time notifier received execute required actions behind it
		void ProcessCmdRtc(cSmartPtr<cNotifier> pNotifier);

		//report time and alarm state every 1s
		void ProcessRtcTimeAlarm(void);
		//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
		virtual void Run();
};//cRtcManager


#endif /*MNG_RTC_HPP_*/
