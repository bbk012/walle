/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_indicator.cpp
* Description: Manager which controls all robot's simply indicators like Led and Buzzeer
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/
#ifndef MNG_INDICATOR_HPP_
#define MNG_INDICATOR_HPP_

#include "mng.hpp"

#define  INDICATOR_PUBLISHER_SEND_Q_SIZE 	5
#define  INDICATOR_SUBSCRIBER_REC_Q_SIZE 	5
#define  INDICATOR_THREAD_STACK_SIZE		128
#define  INDICATOR_THREAD_PRIORITY   		35



//indication manager - manages led and buzzer indications
class cIndicatorMngr:public cMngBasePublisherSubscriber<INDICATOR_PUBLISHER_SEND_Q_SIZE,INDICATOR_SUBSCRIBER_REC_Q_SIZE,INDICATOR_THREAD_STACK_SIZE,INDICATOR_THREAD_PRIORITY>
{
public:
	cIndicatorMngr(){mLedState=CTR_STATE_OFF;mBuzzerState=CTR_STATE_OFF;};//initialize state variable to indicate both indicators are off
private:
	sIndicatorData mIndicatorData;//used to preserve notifier received CMD_INDICATOR data
	BYTE mLedState;//this variable is used to keep track of current Led state to togle it for EVT_SYS_ALLIVE initialized to signal Led is Off
	BYTE mBuzzerState;//to keep track of current Buzzer state
	void EvtSysAlive(void);//process system alive command EVT_SYS_ALIVE
	void CmdLed(void);//process LED sub-command 
	void CmdBuzzer(void);//process buzzer sub-command 
	void CmdMainPower(void);//process main power off sub-command
	//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
    virtual void Run();
}; //cIndicatorMngr


#endif /*MNG_INDICATOR_HPP_*/
