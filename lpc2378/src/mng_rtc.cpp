/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_rtc.cpp
* Description: Robot's Real Time Clock manager
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/

#include "mng_rtc.hpp"
#include "wrp_kernel.hpp"

	

//when CMD_RTC notifier received execute required actions behind it and provide respons
void cRtcMngr::ProcessCmdRtc(cSmartPtr<cNotifier> pNotifier)
{
	
	switch(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mRtcCmdId)//depending on sub-command type execute proper RTC function
	{
	case SET_TIME_DATE_SCMD_ID:
		//prepare tm structure with data received in set time date command
		mRtcDateTime.Year=(BYTE)CalendarYrToTm(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mYear);
		mRtcDateTime.Month=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMonth;
		mRtcDateTime.Day=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDay;
		mRtcDateTime.Wday=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDayOfWeek;
		mRtcDateTime.Hour=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour;
		mRtcDateTime.Minute=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute;
		mRtcDateTime.Second=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mSecond;
		RTCSetupDateTime(&mRtcDateTime);
		break;
	case GET_TIME_DATE_SCMD_ID:
		RTCGetDateTime(&mRtcDateTime);
		break;
	case SET_ALARM_TIME_DATE_SCMD_ID:
		//prepare tm structure with data and received in set alarm time date command
		mRtcDateTime.Year=(BYTE)CalendarYrToTm(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mYear);
		mRtcDateTime.Month=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMonth;
		mRtcDateTime.Day=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDay;
		mRtcDateTime.Wday=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDayOfWeek;
		mRtcDateTime.Hour=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour;
		mRtcDateTime.Minute=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute;
		mRtcDateTime.Second=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mSecond;
		RTCSetupDateTimeAlarmValue(&mRtcDateTime);
		RTCSetAlarmState(TURN_ON_DATE_ALARM,TURN_ON_TIME_ALARM);
		break;
	case GET_ALARM_TIME_DATE_SCMD_ID:
		RTCGetDateTimeAlarmValue(&mRtcDateTime);
		break;
	case CLR_ALARM_FOR_TIME_DATE_SCMD_ID:
		RTCSetAlarmState(TURN_OFF_DATE_ALARM,TURN_OFF_TIME_ALARM);
		break;
	default:
		return;//when unclear sub-command just return without any response
	}//switch
	//when this place is received it means requested command was executed so send RSP_RTC response
	//create and issue response notifier
	cSmartPtr<cTypeNotifier<sRtcData> > pRspNotifier = new cTypeNotifier<sRtcData>(RSP_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mRtcCmdId=static_cast<sRtcData*>(pNotifier->GetDataPtr())->mRtcCmdId;
	(pRspNotifier->GetData()).mIsAlarmSetupInRTC=IsAlarmSetupInRTC();//check and report is alarm setup in RTC
	(pRspNotifier->GetData()).mIsAlarmTriggeredByRTC=FALSE;//EVT_ALARM reports alarm if any not RSP_RTC	
	(pRspNotifier->GetData()).mYear=tmYearToCalendar(mRtcDateTime.Year);
	(pRspNotifier->GetData()).mMonth=mRtcDateTime.Month;
	(pRspNotifier->GetData()).mDay=mRtcDateTime.Day;
	(pRspNotifier->GetData()).mDayOfWeek=mRtcDateTime.Wday;
	(pRspNotifier->GetData()).mHour=mRtcDateTime.Hour;
	(pRspNotifier->GetData()).mMinute=mRtcDateTime.Minute;
	(pRspNotifier->GetData()).mSecond=mRtcDateTime.Second;	
	Post(pRspNotifier);//post RSP_RTC response	
}//cRtcMngr::ProcessCmdRtc

//report time and alarm state ones per about 1 second
void cRtcMngr::ProcessRtcTimeAlarm(void)
{
	RTCGetDateTime(&mRtcDateTime);//get most up to date time & date 
	
	//create EVT_TIME notifier fill with time date data and post it to all subscribers
	cSmartPtr< cTypeNotifier<sRtcData> > pEvtNotifier = new cTypeNotifier<sRtcData>(EVT_TIME,GetThreadId(),NT_HND_NORMAL_PRT);

	(pEvtNotifier->GetData()).mRtcCmdId=RTC_NONE_SCMD_ID;
	(pEvtNotifier->GetData()).mIsAlarmSetupInRTC=IsAlarmSetupInRTC();//TRUE when alarm is setup in RTC
	(pEvtNotifier->GetData()).mIsAlarmTriggeredByRTC=FALSE;//EVT_TIME is not used to report alarm see EVT_ALARM below
	(pEvtNotifier->GetData()).mYear=tmYearToCalendar(mRtcDateTime.Year);
	(pEvtNotifier->GetData()).mMonth=mRtcDateTime.Month;
	(pEvtNotifier->GetData()).mDay=mRtcDateTime.Day;
	(pEvtNotifier->GetData()).mDayOfWeek=mRtcDateTime.Wday;
	(pEvtNotifier->GetData()).mHour=mRtcDateTime.Hour;
	(pEvtNotifier->GetData()).mMinute=mRtcDateTime.Minute;
	(pEvtNotifier->GetData()).mSecond=mRtcDateTime.Second;		

	Post(pEvtNotifier);//post EVT_TIME periodic event

	//create EVT_ALARM notifier fill with data
	//when alarm was generated it contains member mIsAlarmTriggeredByRTC set to TRUE and date time of alarm
	//when not it is periodic ALARM state information if Alarm is setup in RTC and for what time
	cSmartPtr< cTypeNotifier<sRtcData> > pAlarmNotifier = new cTypeNotifier<sRtcData>(EVT_ALARM,GetThreadId(),NT_HND_NORMAL_PRT);
	
	if(GetAlarmState())//when alarm was triggered post EVT_ALARM event to all its subscribers
	{
		RTCGetAlarmStateDateTime(&mRtcDateTime);//get information about when alarm was triggered
		ClrAlarmState();//clear information about triggered alarm to be able to report eventual new one

		(pAlarmNotifier->GetData()).mRtcCmdId=RTC_NONE_SCMD_ID;
		(pAlarmNotifier->GetData()).mIsAlarmSetupInRTC=IsAlarmSetupInRTC();//TRUE when alarm is setup in RTC
		(pAlarmNotifier->GetData()).mIsAlarmTriggeredByRTC=TRUE;//TRUE when alarm is just triggered by RTC
		(pAlarmNotifier->GetData()).mYear=tmYearToCalendar(mRtcDateTime.Year);
		(pAlarmNotifier->GetData()).mMonth=mRtcDateTime.Month;
		(pAlarmNotifier->GetData()).mDay=mRtcDateTime.Day;
		(pAlarmNotifier->GetData()).mDayOfWeek=mRtcDateTime.Wday;
		(pAlarmNotifier->GetData()).mHour=mRtcDateTime.Hour;
		(pAlarmNotifier->GetData()).mMinute=mRtcDateTime.Minute;
		(pAlarmNotifier->GetData()).mSecond=mRtcDateTime.Second;	

	} else //alarm was not triggered issue periodic EVT_ALARM event to reflect what is current alarm setup in RTC
	{
		RTCGetDateTimeAlarmValue(&mRtcDateTime);//get date and time setup in RTC for Alarm
		
		(pAlarmNotifier->GetData()).mRtcCmdId=RTC_NONE_SCMD_ID;
		(pAlarmNotifier->GetData()).mIsAlarmSetupInRTC=IsAlarmSetupInRTC();//TRUE when alarm is setup in RTC
		(pAlarmNotifier->GetData()).mIsAlarmTriggeredByRTC=FALSE;//alarm is not triggered when EVT_ALARM is now assembled
		(pAlarmNotifier->GetData()).mYear=tmYearToCalendar(mRtcDateTime.Year);
		(pAlarmNotifier->GetData()).mMonth=mRtcDateTime.Month;
		(pAlarmNotifier->GetData()).mDay=mRtcDateTime.Day;
		(pAlarmNotifier->GetData()).mDayOfWeek=mRtcDateTime.Wday;
		(pAlarmNotifier->GetData()).mHour=mRtcDateTime.Hour;
		(pAlarmNotifier->GetData()).mMinute=mRtcDateTime.Minute;
		(pAlarmNotifier->GetData()).mSecond=mRtcDateTime.Second;		
	}//else
	
	Post(pAlarmNotifier);//post EVT_ALARM periodic notifier
	
}//cRtcMngr::ProcessRtcTimeAlarm

void cRtcMngr::Run()
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier;//received notifier if any
	WORD TicksCounter=0;//counts task ticks to counts 1s time to check alarm state and to send time event every 1s	

	for(;;)
	{
		pNotifier = Receive(RTC_PROCESSING_TICKS);//wait max RTC_PROCESSING_TICKS tick for notifier to arrive
		if (pNotifier.isValid())//when Notifier received
		{
			if((pNotifier->GetNotifierId())== CMD_RTC )
			{
				ProcessCmdRtc(pNotifier);//execute RTC command as defined by received notifier	
			}

			continue;//unexpected command received skip it
		}
		else //none Notifier received after RTC_PROCESSING_TICKS
		{
			TicksCounter+=1;//increment number of counted loops
			if(TicksCounter >= RTC_MAX_TO_CHECK_TIME_ALARM)//report time and alarm state (evry 1s)
			{
				TicksCounter=0;//lets strat counting another period
				ProcessRtcTimeAlarm();//report time and alarm
			}
		}	
	}//for
}//cRtcMngr::Run()
