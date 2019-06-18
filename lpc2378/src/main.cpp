/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        main.c
* Description: LPC 2378 project C level entry point - main()
* Author:      Bogdan Kowalczyk
* Date:        2-Aug-2008
* History:
* 2-Aug-2008 - Initial version created
*********************************************************************************************************
*/

#include "wrp_kernel.hpp"
#include "mng_brain.hpp"
#include "mng_display.hpp"
#include "mng_leftarm.hpp"
#include "mng_rightarm.hpp"
#include "mng_keypad.hpp"
#include "mng_indicator.hpp"
#include "mng_motion.hpp"
#include "mng_monitor.hpp"
#include "mng_rtc.hpp"
#include "mng_rmt.hpp"
#include "mng_vrm.hpp"

//Kernel must be initialized first before any OS object is constracted and use because OSInit() is call from constructor
//cKernel is singleton class and below its static member is initialized what calls private constructor and
//initialize uCOS-II through OSInit() call

cKernel cKernel::m_Kernel;

cBrainMngr 		BrainMngr;//contains cCtxMngr and cExeMngr
cDisplayMngr 	DisplayMngr;
cLeftArmMngr 	LeftArmMngr;
cRightArmMngr 	RightArmMngr; 
cKeyPadMngr  	KeyPadMngr;
cMotionMngr 	MotionMngr;
cMonitorMngr 	MonitorMngr;
cIndicatorMngr 	IndicatorMngr;
cRtcMngr 		RtcMngr;
cRmtMngr 		RmtMngr;
cVrmMngr 		VrmMngr;

//aplication level entry point
int	main (void)
{
//  This is debugger supporting do nothing loop just wait before run	
//	for (int j = 0; j < 5000000; j++ );//temporary loop looks help debuger to start and stop on the breakpoint

	
//IMPORTANT!
//adding Publishers/Subscribers assure right number of places in 
//dispatcher structures for publishers and subscribers in wp_kernel.hpp

	Kernel.Dispatcher.RegisterPublisher(BrainMngr);
	Kernel.Dispatcher.RegisterPublisher(BrainMngr.CtxMngr);
	Kernel.Dispatcher.RegisterPublisher(BrainMngr.ExeMngr);
	Kernel.Dispatcher.RegisterPublisher(LeftArmMngr);
	Kernel.Dispatcher.RegisterPublisher(RightArmMngr);
	Kernel.Dispatcher.RegisterPublisher(KeyPadMngr);
	Kernel.Dispatcher.RegisterPublisher(MonitorMngr);//publish EVT_SYS_ALIVE,EVT_SYS_RES,EVT_BATTERY...
	Kernel.Dispatcher.RegisterPublisher(MotionMngr);
	Kernel.Dispatcher.RegisterPublisher(IndicatorMngr);
	Kernel.Dispatcher.RegisterPublisher(RtcMngr);//publish EVT_TIME, EVT_ALARM, RSP_RTC, RSP_ALARM...
	Kernel.Dispatcher.RegisterPublisher(RmtMngr);
	Kernel.Dispatcher.RegisterPublisher(VrmMngr);

	Kernel.Dispatcher.RegisterSubscriber(BrainMngr,NONE_NOTIFIER);
	Kernel.Dispatcher.RegisterSubscriber(BrainMngr.CtxMngr,EVT_TIME|EVT_BATTERY|EVT_SYS_ALIVE);
	Kernel.Dispatcher.RegisterSubscriber(BrainMngr.ExeMngr,CMD_EXE_CMD|RSP_MOVE|RSP_MOVE_ARM|RSP_RTC|RSP_SCAN|RSP_CHECK|RSP_TURN_HEAD|RSP_INDICATOR);
	Kernel.Dispatcher.RegisterSubscriber(DisplayMngr,EVT_SYS_ALIVE|EVT_BATTERY|EVT_KEY|EVT_DAY_NIGHT|EVT_SYS_RES|EVT_TIME|EVT_ALARM|RSP_SCAN|EVT_DAY_NIGHT|EVT_DISPLAY_INFO);//display system status
	Kernel.Dispatcher.RegisterSubscriber(LeftArmMngr,CMD_MOVE_ARM);
	Kernel.Dispatcher.RegisterSubscriber(RightArmMngr,CMD_MOVE_ARM);
	
	Kernel.Dispatcher.RegisterSubscriber(MotionMngr,CMD_SCAN|CMD_MOVE|CMD_CHECK|CMD_TURN_HEAD);
	Kernel.Dispatcher.RegisterSubscriber(IndicatorMngr,EVT_SYS_ALIVE|CMD_INDICATOR);
	Kernel.Dispatcher.RegisterSubscriber(RtcMngr,CMD_RTC);
	Kernel.Dispatcher.RegisterSubscriber(RmtMngr,NONE_NOTIFIER);//!!!RmtMng is using dynamic subscribe unsubscribe
	Kernel.Dispatcher.RegisterSubscriber(VrmMngr,RSP_EXE_CMD);//EVT_SYS_ALIVE
	
	Kernel.Start();//start OS
	
	while(1);//forever loop - execution transfered to tasks as OS is started now
}//main






