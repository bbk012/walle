/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_kaypady.cpp
* Description: Manager which controls robot's keypad
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/

#include "mng_keypad.hpp"
#include "wrp_kernel.hpp"
#include "hw_rtc.h"

//put cKeyPadMngr into default entry state
cKeyPadMngr::cKeyPadMngr()
{
	mPreviousKeyStatus=0xFFFF;//byte coded status of keys as reported by previous notifier - no key pressed at init
	mCurrentKeyStatus=0xFFFF;//byte coded key status calculated by KeyFSM for current key check loop - no key pressed at init

}//cKeyPadMngr::cKeyPadMngr

//KeyFSM is a function which executes Finite State Machine for every key.
//Based on the current Key State and based on the current status of the hardware key button 
//      both next Key State as well as Key Status are evaluated
//KeyFSM - statechart
//
//	                  on start
//                    |
//       +-------+    |
//	"1"  |       V    V
//       +------RELEASED<----------------------------------------+
//               |    ^                                          |
//   "0"/        |    | "1"                                      |
//   Start       |    |                                          |
//  Timeout      V    |                                          |
//             PRESSED_DEBAUNCE<----+                            |
//                |         |       | No Timeout & "0"           |
//                |         +-------+                            |
//   Timeout &    |                                              | Timeout & "1"/
//    "0"/        |                                              | Send Notifier "released"
// Send Notifier  V                                              |
//  "pressed"    PRESSED<----+                                   |
//               |   ^ |     | "0"                               |
//   "1"/        |   | +-----+                                   |
// Start         |   |                                           |
// Timeout       |   | "0"                                       |
//               V   |                                           |
//             RELEASE_DEBUNCED----------------------------------+
//               ^        |
//               |        | No Timeout & "1"
//               +--------+
//
//
void cKeyPadMngr::KeyFSM(cKeyBase* pKey,WORD* pKeyStatus )
{
	switch(pKey->GetKeyState())
	{
	case RELEASED:
		if(pKey->GetKeyHwStatus())//"1" when key is not pressed
		{
			pKey->KeyStatusReleased(pKeyStatus);//set key is still released
			pKey->SetKeyState(RELEASED);//next state for FSM is RELEASED
		}else//"0" i.e. when key is pressed
		{
			pKey->KeyStatusReleased(pKeyStatus);//set key is still released until confiremed by debaunce
			pKey->SetKeyTimeout(0);//clear key timeout counter as we will debaunce
			pKey->SetKeyState(PRESSED_DEBAUNCE);//next state for FSM is PRESSED_DEBAUNCE
		}
		break;
	case PRESSED_DEBAUNCE:
		if(pKey->GetKeyHwStatus())//"1" when key is not pressed
		{
			pKey->KeyStatusReleased(pKeyStatus);//set key is still released
			pKey->SetKeyState(RELEASED);//next state for FSM is RELEASED
		}else//"0" i.e. when key is pressed
		{
			if(pKey->GetKeyTimeout()<KEY_DEBAUNCE_TIMEOUT)//when not timeout
			{
				pKey->SetKeyTimeout(pKey->GetKeyTimeout()+1);//increment timeout by one call tick
				pKey->KeyStatusReleased(pKeyStatus);//set key is still released as not fully debaunced
				pKey->SetKeyState(PRESSED_DEBAUNCE);//next state for FSM is PRESSED_DEBAUNCE
			}
			else//when timeout and still pressed i.e. when fully debaunced
			{
				pKey->KeyStatusPressed(pKeyStatus);//set key is pressed
				pKey->SetKeyState(PRESSED);//next state for FSM is PRESSED
			}
		}
		break;
	case PRESSED:
		if(pKey->GetKeyHwStatus())//"1" when key is not pressed
		{
			pKey->KeyStatusPressed(pKeyStatus);//set key is still pressed until confiremed by debaunce
			pKey->SetKeyTimeout(0);//clear key timeout counter as we will debaunce
			pKey->SetKeyState(RELEASE_DEBUNCED);//next state for FSM is RELEASE_DEBUNCED
		}else//"0" i.e. when key is still pressed
		{
			pKey->KeyStatusPressed(pKeyStatus);//set key is still released
			pKey->SetKeyState(PRESSED);//next state for FSM is RELEASED
		}
		break;
	case RELEASE_DEBUNCED:
		if(pKey->GetKeyHwStatus())//"1" when key is not pressed
		{
			if(pKey->GetKeyTimeout()<KEY_DEBAUNCE_TIMEOUT)//when not timeout
				{
					pKey->SetKeyTimeout(pKey->GetKeyTimeout()+1);//increment timeout by one call tick
					pKey->KeyStatusPressed(pKeyStatus);//set key is still pressed as not fully debaunced
					pKey->SetKeyState(RELEASE_DEBUNCED);//next state for FSM is RELEASE_DEBAUNCE
				}
			else//when timeout and still released i.e. when fully debaunced
				{
					pKey->KeyStatusReleased(pKeyStatus);//set key is released
					pKey->SetKeyState(RELEASED);//next state for FSM is PRESSED
				}
		}else//"0" i.e. when key is pressed
		{
			pKey->KeyStatusPressed(pKeyStatus);//set key is still pressed
			pKey->SetKeyState(PRESSED);//next state for FSM is PRESSED
		}
		break;
	}//switch()
}//cKeyPadMngr::KeyFSM

//IMPORTANT! This function can turn off robot when J1CENTER is pressed sufficently long
void  cKeyPadMngr::Run()
{
	sKeyInput KeyInput;//structure to capture user interface input data
	for(;;)
	{
		Kernel.Delay(INPUT_SCAN_FREQUENCY);
		KeyFSM(&mKeyBUT1, &mCurrentKeyStatus);
		KeyFSM(&mKeyBUT2, &mCurrentKeyStatus);
		KeyFSM(&mKeyJ1DOWN, &mCurrentKeyStatus);
		KeyFSM(&mKeyJ1UP, &mCurrentKeyStatus);
		KeyFSM(&mKeyJ1LEFT, &mCurrentKeyStatus);
		KeyFSM(&mKeyJ1RIGHT, &mCurrentKeyStatus);
		KeyFSM(&mKeyJ1CENTER, &mCurrentKeyStatus);
		
		//when J1CENTER changed and it is pressed
		if (((mCurrentKeyStatus&BIT6)^(mPreviousKeyStatus&BIT6))&&(!(mCurrentKeyStatus&BIT6)))
		{
			mKeyJ1CENTERPressedTicks=Kernel.Ticks();//get number of ticks when J1CENTER pressed
		}
		if(!(mCurrentKeyStatus&BIT6))//when J1CENTER is pressed
		{
			//if time of J1CENTER pressed larger than ROBOT_TURN_OFF_TIMEOUT_TICKS turn robot OFF
			if(Kernel.Ticks() >= (mKeyJ1CENTERPressedTicks+ROBOT_TURN_OFF_TIMEOUT_TICKS))
			{
				//when Wall-e is turned off by keyboard long press any RTC alarm is clear to protect against SRAM battery discharge
				RTCSetAlarmState(TURN_OFF_DATE_ALARM,TURN_OFF_TIME_ALARM);
				uPBoardOff(KEYPAD_TURN_OFF);//turn OFF uP board power when J1CENTER pressed longer than ROBOT_TURN_OFF_TIMEOUT_TICKS
			}
		}
		else//when J1 CENTER is not pressed move to current moment with the key press time to avoid uP Board turn off
		{
			mKeyJ1CENTERPressedTicks=Kernel.Ticks();
		}
		//if current calculate status different to the previous reported one - send out key status notifer
		if(mCurrentKeyStatus^mPreviousKeyStatus)
		{
			//Create new empty key event Notifier 
			cSmartPtr<cTypeNotifier<sKeyInput> > pNotifier = new cTypeNotifier<sKeyInput>(EVT_KEY,GetThreadId(),NT_HND_NORMAL_PRT,KeyInput);

			//setup Notifier data
			(pNotifier->GetData()).mCurKeyStatus=mCurrentKeyStatus;//send current key-pad status
			(pNotifier->GetData()).mPrevKeyStatus=mPreviousKeyStatus;//send previous key-pad status
			(pNotifier->GetData()).mTimeStamp=Kernel.Ticks();//get key status change timestamp
			
			Post(pNotifier);//issue key event Notifier for every key status change
			mPreviousKeyStatus=mCurrentKeyStatus;//change status send so start key change tracking again
		}
	}//for
}//cKeyPadMngr::Run()
