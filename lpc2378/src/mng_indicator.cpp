/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_indicator.cpp
* Description: Manager which controls all robot's simply indicators like Led and Buzzer
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/

#include "mng_indicator.hpp"
#include "wrp_kernel.hpp"
#include "hw_gpio.h"

//process system alive command EVT_SYS_ALIVE - there is not response for the event
void  cIndicatorMngr::EvtSysAlive(void)
{
	if(mLedState==CTR_STATE_ON)//when Led is on turn it off
	{
		LedOff();//turn led off
		mLedState=CTR_STATE_OFF;//mark Led is OFF now
	}else//when led is off turn it on
	{
		LedOn();//turn led on
		mLedState=CTR_STATE_ON;//mark Led is ON now
	}
}//cIndicatorMngr::EvtSysAlive

//process LED command CMD_LED when executed provide RSP_LED response
void  cIndicatorMngr::CmdLed(void)
{
	switch (mIndicatorData.mState)
	{
	case CTR_STATE_OFF://when led off requested
			LedOff();//turn led off
			mLedState=CTR_STATE_OFF;//mark Led is OFF now
		break;
	case CTR_STATE_ON://when led on requested
	default:
			LedOn();//turn led on
			mLedState=CTR_STATE_ON;//mark Led is ON now
		break;
	}//switch
}//cIndicatorMngr::CmdLed

//process buzzer command CMD_BUZZER and provide RSP_BUZZER when command completed
void  cIndicatorMngr::CmdBuzzer(void)
{
	switch (mIndicatorData.mState)
	{
	case CTR_STATE_OFF://when led off requested
		BuzzerOff();//turn buzzer off
		mBuzzerState=CTR_STATE_OFF;//mark Led is OFF now
		break;
	case CTR_STATE_ON://when led on requested
	default:
		BuzzerOn();//turn buzzer on
		mBuzzerState=CTR_STATE_ON;//mark Led is ON now
		break;
	}//switch
}//cIndicatorMngr::CmdBuzzer

//process main power off sub-command
void  cIndicatorMngr::CmdMainPower(void)
{
	uPBoardOff(mIndicatorData.mState);//turn of main board power supply
}//cIndicatorMngr::CmdMainPower

void  cIndicatorMngr::Run(void)
{
for(;;)
	{
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		if(pNotifier->GetNotifierId()==EVT_SYS_ALIVE)//when sys alive event received
		{
			EvtSysAlive();//togle led indicator
		}
		else //process CMD_INDICATOR command
		{
			mIndicatorData=*(static_cast<sIndicatorData*>(pNotifier->GetDataPtr()));//copy notifier data to mMoveData
			switch(mIndicatorData.mIndicatorCmdId)//depending on sub-command type execute proper action
			{
			case CTR_LED_SCMD_ID://control led as defined by sub-command
				CmdLed();
				break;
			case CTR_BUZZER_SCMD_ID://contol buzzer as defined by sub-command
				CmdBuzzer();
				break;
			case CTR_MAIN_POWER_SCMD_ID://control main power as defined by sub-command
				CmdMainPower();
				break;
			default:
				break;
			}//switch
			//create and issue response notifier
			cSmartPtr<cTypeNotifier<sIndicatorData> > pRspNotifier = new cTypeNotifier<sIndicatorData>(RSP_INDICATOR,GetThreadId(),NT_HND_NORMAL_PRT);
			(pRspNotifier->GetData()).mIndicatorCmdId=mIndicatorData.mIndicatorCmdId;
			(pRspNotifier->GetData()).mState=mIndicatorData.mState;	
				
			Post(pRspNotifier);//post RSP_INDICATOR response
		}//if
	}//for
}//cIndicatorMngr::Run()
