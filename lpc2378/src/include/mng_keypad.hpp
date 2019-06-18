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
#ifndef MNG_KEYPAD_HPP_
#define MNG_KEYPAD_HPP_

#include "mng.hpp"
#include "hw_gpio.h"

//manager basic parameters
#define  KEYPAD_PUBLISHER_SEND_Q_SIZE 	5      	//transmit queue size
#define  KEYPAD_SUBSCRIBER_REC_Q_SIZE 	5		//receive queue size
#define  KEYPAD_THREAD_STACK_SIZE		128		//manager stack
#define  KEYPAD_THREAD_PRIORITY   		20		//manager priority

//enum type to identify states of the key processing FSM (Finite State Machine)
enum tKeyFsmState {RELEASED, PRESSED_DEBAUNCE, PRESSED, RELEASE_DEBUNCED};

//timeout in uCOS-II ticks for debuncing
#define KEY_DEBAUNCE_TIMEOUT 	1

//frequency of InManager input scan in Kernrl ticks (for example every 5 OS ticks)
#define INPUT_SCAN_FREQUENCY 1

//timeout to assume robot OFF when J1CENTER is pressed in uCOS-II ticks
//100 ticks means 1s for 10ms TICK
#define ROBOT_TURN_OFF_TIMEOUT_TICKS    100

// STATUS CODING: 0 - means key pressed, 1 - means key released
// BIT0 - BUT1
// BIT1 - BUT2
// BIT2 - J1 DOWN
// BIT3 - J1 UP
// BIT4 - J1 LEFT
// BIT5 - J1 RIGHT
// BIT6 - J1 CENTER
//base class used by KeyFSM to process KeyStatus
class cKeyBase
{
private:
	tKeyFsmState mKeyState;//KeyFSM state
	BYTE mTimeout;//Key debaunce current timeout
public:
	cKeyBase(){mKeyState=RELEASED;mTimeout=0;};

	void SetKeyState(tKeyFsmState InState){mKeyState=InState;};
	tKeyFsmState GetKeyState(void){return mKeyState;};
	void SetKeyTimeout(BYTE InTimeout){mTimeout=InTimeout;};
	BYTE GetKeyTimeout(void){return mTimeout;};
	
	//key spcific functions which need to be overwriten for every key
	virtual BYTE GetKeyHwStatus(void){return 0xFF;};//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual void KeyStatusPressed(WORD* pKeyStatus){};//to clear BIT (key pressed) in the status WORD for the key see sUiInput structure
	virtual void KeyStatusReleased(WORD* pKeyStatus){};//to set BIT (key released)in the status WORD for the key
	virtual ~cKeyBase(){};//virtual destructor only to avoid warning
};//cKeyBase

//Key implementation for BUT1 key
class cKeyBUT1:public cKeyBase
{
public:
	cKeyBUT1():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetBut1()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT0);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT0;return;}
};//cKeyBUT1

//key implementation for BUT2 key
class cKeyBUT2:public cKeyBase
{
public:
	cKeyBUT2():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetBut2()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT1);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT1;return;}
};//cKeyBUT2

//key implementation for J1DOWN key
class cKeyJ1DOWN:public cKeyBase
{
public:
	cKeyJ1DOWN():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetJ1Down()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT2);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT2;return;}
};//cKeyJ1DOWN

//key implementation for J1UP key
class cKeyJ1UP:public cKeyBase
{
public:
	cKeyJ1UP():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetJ1Up()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT3);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT3;return;}
};//cKeyJ1UP

//key implementation for J1Left key
class cKeyJ1LEFT:public cKeyBase
{
public:
	cKeyJ1LEFT():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetJ1Left()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT4);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT4;return;}
};//cKeyJ1Left

//key implementation for J1RIGHT key
class cKeyJ1RIGHT:public cKeyBase
{
public:
	cKeyJ1RIGHT():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetJ1Right()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT5);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT5;return;}
};//cKeyJ1RIGHT

//key implementation for J1CENTER key
class cKeyJ1CENTER:public cKeyBase
{
public:
	cKeyJ1CENTER():cKeyBase(){};
	
	//to get current status of the key hw input "0" - pressed, <>"0" - released
	virtual BYTE GetKeyHwStatus(void){return GetJ1Center()?1:0;};
	
	//to clear BIT (key pressed) in the status WORD for the key see sKeyInput structure
	virtual void KeyStatusPressed(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus & ~(BIT6);return;};
	
	//to set BIT (key released)in the status WORD for the key
	virtual void KeyStatusReleased(WORD* pKeyStatus){*pKeyStatus = *pKeyStatus | BIT6;return;};
};//cKeyJ1CENTER


class cKeyPadMngr:public cMngBasePublisherSubscriber<KEYPAD_PUBLISHER_SEND_Q_SIZE,KEYPAD_SUBSCRIBER_REC_Q_SIZE,KEYPAD_THREAD_STACK_SIZE,KEYPAD_THREAD_PRIORITY>
{
public:
	cKeyPadMngr();
	//key processing Finite State Machine
	//inputs:
	//  rKey - pointer to the key which is processed
	//  pKeyStatus - pointer to the Key status which is updated by the KeyFSM
	//               in the KeyStatus every bit code a key as defined by sKeyInput structure
	void KeyFSM(cKeyBase* pKey, WORD* pKeyStatus);
private:
	//for key coding see struct sKeyInput
	WORD mPreviousKeyStatus;//byte coded status of keys as reported by previous notifier
	WORD mCurrentKeyStatus;//byte coded key status calculated by KeyFSM for current key check loop
	
	DWORD mKeyJ1CENTERPressedTicks;//number of sys TICKS when J1CENTER Key was pressed used to turn off robot
	
	//keys which are handled by cInManager
	cKeyBUT1 mKeyBUT1; 
	cKeyBUT2 mKeyBUT2;
	cKeyJ1DOWN mKeyJ1DOWN; 
	cKeyJ1UP   mKeyJ1UP;
	cKeyJ1LEFT mKeyJ1LEFT;
	cKeyJ1RIGHT mKeyJ1RIGHT;
	cKeyJ1CENTER mKeyJ1CENTER;
private:
	//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
    virtual void Run();
};//cKeyPadMngr

#endif /*MNG_KEYPAD_HPP_*/
