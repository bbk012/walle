/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_ctx.cpp
* Description: Keeps all internal and external context of Wall-e operations
* Author:      Bogdan Kowalczyk
* Date:        25-Jan-2015
* History:
* 25-Jan-2015 - Initial version created
*********************************************************************************************************
*/

#include "mng_ctx.hpp"
#include "wrp_kernel.hpp"
#include "ctr_lcd.h"

void cCtxMngr::UpadateTimeContext(cSmartPtr<cNotifier>  p_smartNotifier)
{
	sRtcData *pRtcData=static_cast<sRtcData*>(p_smartNotifier->GetDataPtr());//pointer to processed notifier data
	
	mCtxMutext.Acquire();//assure exclusive access to time data and update them
	
	mTimeDateNow.Year=CalendarYrToTm(pRtcData->mYear);
	mTimeDateNow.Month=pRtcData->mMonth;
	mTimeDateNow.Day=pRtcData->mDay;
	mTimeDateNow.Wday=pRtcData->mDayOfWeek;
	mTimeDateNow.Hour=pRtcData->mHour;
	mTimeDateNow.Minute=pRtcData->mMinute;
	mTimeDateNow.Second=pRtcData->mSecond;
	
	mTimeNow=MakeTime(&mTimeDateNow);//convert mTimeDateNow to number of seconds since Jan 1 1970
	mTimeContextUpdated=TRUE;//mark time updated
	mCtxMutext.Release();


}//cCtxMngr::UpadateTimeContext

//IMPORTANT! It waits internally until time context first time updated
time_t cCtxMngr::GetNowTime(void)
{
	time_t Time;
	
	while(!mTimeContextUpdated)//wait if time context was not yet updated
		Delay(CTX_CONTEXT_UPDATE_DELAY);//keep OS running so context switch is possible when waiting and checking flag
	
	mCtxMutext.Acquire();//assure exclusive access
	Time=mTimeNow;
	mCtxMutext.Release();
	return Time;
}//cCtxMngr::GetNowTime


//update battery state context
void cCtxMngr::UpdateBatteryContext(cSmartPtr<cNotifier> pNotifier)
{
	sBatteryStatus *pBatteryData=static_cast<sBatteryStatus*>(pNotifier->GetDataPtr());//pointer to processed notifier data
	
	mCtxMutext.Acquire();//assure exclusive access to the battery data
  
	mMainSupplyState=pBatteryData->mMainSupplyState;//normal, warn or low state of the main power supply
	mMotorSupplyState=pBatteryData->mMotorSupplyState;//normal, warn or low state of the motor supply
	mServoSupplyState=pBatteryData->mServoSupplyState;//normal, warn or low state of the servo supply
	mBatteryContextUpdated=TRUE;//set TRUE after first context update

	mCtxMutext.Release();//mark context updated	
	
}//cCtxMngr::UpdateBatteryContext

//OK or NOK state of any of battery sub-systems
//IMPORTANT! It waits internally until battery context first time updated
BYTE cCtxMngr::GetBatteryOKState(void)
{
	BYTE BatteryState;//temporary to keep read battery state
	
	while(!mBatteryContextUpdated)//wait if buttery context was not yet updated
		Delay(CTX_CONTEXT_UPDATE_DELAY);//keep OS running so context switch is possible when waiting and checking flag
	
	mCtxMutext.Acquire();//assure exclusive access to the battery data
	
	if(mMainSupplyState==BOARD_BATT_EMPTY ||
	mMotorSupplyState== TRACK_BATT_EMPTY || 
	mServoSupplyState==SERVO_BATT_EMPTY)
		BatteryState=NOK_STATE; //at least one of batteries report empty state
	else
		BatteryState=OK_STATE;//all batteries are operational
	
	mCtxMutext.Release();//release mutex because context is updated	
	
	return BatteryState;
}//cCtxMngr::GetBatteryState

void cCtxMngr::UpdateSysStateContext(cSmartPtr<cNotifier> pNotifier)
{
	sSysAliveEvt *pSysAlive=static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr());//pointer to processed notifier data
	
	mCtxMutext.Acquire();//assure exclusive access to the battery data
  
	mRAMContentPreserved=pSysAlive->mRAMContentPreserved;
	mSysStateContextUpdated=TRUE;//set TRUE after first context update
	
	mCtxMutext.Release();//release mutex when context updated		
}//cCtxMngr::UpdateSysStateContext

//OK or NOK state depending if SRAM content preserved or not
//IMPORTANT! It waits internally until battery context first time updated
BYTE cCtxMngr::GetSRAMBatteryOKState(void)
{
	BYTE BatteryState;//temporary to keep read battery state
	
	while(!mSysStateContextUpdated)//wait context was not yet updated
		Delay(CTX_CONTEXT_UPDATE_DELAY);//keep OS running so context switch is possible when waiting and checking flag
	
	mCtxMutext.Acquire();//assure exclusive access to the battery data
	
	if(mRAMContentPreserved)
		BatteryState= OK_STATE;
	else
		BatteryState=NOK_STATE;	
	
	mCtxMutext.Release();//release mutex because context is updated	
	
	return BatteryState;
}//cCtxMngr::GetBatteryState

void cCtxMngr::Run()
{
	for(;;)
	{
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		switch(pNotifier->GetNotifierId())//depending on received information update context
		{
		case EVT_TIME://when time information received update time context
			UpadateTimeContext(pNotifier);
			break;
		case EVT_BATTERY://get battery state information and update battery context
			UpdateBatteryContext(pNotifier);
			break;
		case EVT_SYS_ALIVE://get system basic system information
			UpdateSysStateContext(pNotifier);
			break;
		default:
			break;
		}//switch
	}//for
}//cCtxMngr::Run()
