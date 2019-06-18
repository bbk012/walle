/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_display.hpp
* Description: Robot display manager
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*			   07-Oct-2017 - Added windows management for so named Wall-e Graphics Library
*********************************************************************************************************
*/
#ifndef MNG_DISPLAY_HPP_
#define MNG_DISPLAY_HPP_

#include "mng.hpp"
#include "lib_g_window.hpp"

//delay in number of ticks to keep Wall-e sys version information before it ic cleared by a first window
#define SYS_PROMPT_DELAY	300

//indexes for mWindows tables which refers to particular windows managed by Display Manager
#define 	WIN_DEAFULT_ID		0x00
#define 	WIN_SYS_STATUS_ID	0x01
#define		WIN_BAT_STATUS_ID	0x02
#define		WIN_TIME_ID			0x03


//manager basic parameters
#define  DISPLAY_PUBLISHER_SEND_Q_SIZE 	5      	//transmit queue size
#define  DISPLAY_SUBSCRIBER_REC_Q_SIZE 	5		//receive queue size
#define  DISPLAY_THREAD_STACK_SIZE		128		//manager stack
#define  DISPLAY_THREAD_PRIORITY   		30		//manager priority


#define  MAX_WINDOWS					5		//maximum number of windows managed by display

#define  DISPLAY_MODE_WAIT				0x01    //mode when manager is waiting just after Wall-e turn on when version information is displayed
#define  DISPLAY_MODE_NORMAL			0x02	//regular display manager operation
#define  DISPLAY_MODE_CRITICAL			0x03    //mode when criticla information is received and displayed

class cDisplayMngr:public cMngBasePublisherSubscriber<DISPLAY_PUBLISHER_SEND_Q_SIZE,DISPLAY_SUBSCRIBER_REC_Q_SIZE,DISPLAY_THREAD_STACK_SIZE,DISPLAY_THREAD_PRIORITY>
{
private:
	BYTE    mDisplayMode;//current display mode
	BYTE	mActiveWindow;//index of currently active Window
	cWindow * mWindows[MAX_WINDOWS];//storage for pointers to windows managed by manager
	BYTE	FindMaxIndex();//find index of the last window in the window list
	
	//used to process left and right keys from Wall-e front pannel
	void ProcessKey(cSmartPtr<cNotifier>  pNotifier);
	//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
    virtual void Run();
public:
	cDisplayMngr();
	//add or remove specified window to the set of windows managed by Display Manager
	void AddWindow(cWindow &inWnd);
	void RemoveWindow(cWindow &inWnd);
	cWindow *GetActiveWindow(){return mWindows[mActiveWindow];}; //get currently active window
	
	void Draw();//draw active window and it's all controls  
	void Clear();//clear whole active window from the LCD
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier

};//cDisplayMngr



#endif /*MNG_DISPLAY_HPP_*/
