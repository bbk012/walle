/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_vrm.hpp
* Description: Voice recognition module(EasyVR) manager
* Author:      Bogdan Kowalczyk
* Date:        11-Jan-2018
* Note:
* History:
*              11-Jan-2018 - Initial version created
*********************************************************************************************************
*/

#ifndef MNG_VRM_HPP_
#define MNG_VRM_HPP_
#include "mng.hpp"
#include "vrmcmd.h"
#include "cmd_def.h"

//maximum attempts to wake up/abort/restart EasyVR module
#define MAX_VR_INIT_ATTEMPTS 	5

//timeout for init (also named break) EasyVR command
#define VR_CMD_WAIT_TIMEOUT	50

//results of command execution by EasyVR module
#define VR_CMD_OK		0
#define VR_CMD_ERROR	1

//EasyVR manager states
#define STATE_EASY_VR_INIT			0
#define STATE_EASY_VR_INIT_ERROR	1
#define STATE_EASY_VR_SLEEP			2
#define STATE_EASY_VR_CMD_EXE		3

//delay time in seconds for the infinite do nothing loop when EasyVR cannot be initialized
#define VR_INFINITE_DO_NOTHING_DELAY	59

//play sound table command double volume setup value
#define SND_DOUBLE_GAIN_VOL			0x60

//VR_CMD_SLEEP command parameter to setup wake up
#define VR_SLEEP_WHISTLE_WAKE		VR_ARG_ZERO+1 //wake on whistle
#define VR_SLEEP_DOUBLE_CLAP_WAKE	VR_ARG_ZERO+5 //wake up on double clap
#define VR_SLEEP_TRIPLE_CLAP_WAKE	VR_ARG_ZERO+8 //wake up on tripel clap

//results for SleepAndWaitForWakeup
#define RESULT_SLEEP_ERROR			1 //error when executing go to sleep
#define RESULT_SLEEP_AWAKEN			0 //when correctly awaken from sleep

//delay to second waik up in OS Ticks
#define DELAY_SECOND_WAKE_UP		200 //200 <=> 2 seconds
//timeout in miliseconds for 2nd Sleep wake up
#define SECOND_WAKE_UP_TIMEOUT	1000

//PlaySnd function results
#define RESULT_SND_OK				0
#define RESULT_SND_ERROR			1

//GetVoiceCmd function results
#define RESULT_VR_CMD_OK				0
#define RESULT_VR_CMD_ERROR				1

//timeout in miliseconds for command to be said to Wall-e and received
//VR module is setup for infinite wait, but external timers of VRM manager are used to break
#define TIMEOUT_VR_CMD_GRP1			60000 // 300000 //for first command we wait 5 minutes
#define TIMEOUT_VR_CMD_GRP2_GRP3	5000 //for second group and 3 group we wait 5 seconds


//CheckVoiceCmd function results
#define VOICE_CMD_EXE 		1 // command assembled execute
#define VOICE_CMD_ASM 		2 // continue command assembly
#define VOICE_CMD_UNKNOWN 	3 // not recognized command

//ExecuteVoiceCmd method results
#define VOICE_EXE_RESULT_OK		0
#define VOICE_EXE_RESULT_SLEEP	1
#define VOICE_EXE_RESULT_ERROR	2

//state CMD timeout in minutes before eventual going to SLEEP when none voice command provided
#define STATE_CMD_EXE_TIMEOUT 	1 //5

#define  VRM_PUBLISHER_SEND_Q_SIZE 	5
#define  VRM_SUBSCRIBER_REC_Q_SIZE 	5
#define  VRM_THREAD_STACK_SIZE		128
#define  VRM_THREAD_PRIORITY   		42

class cVrmMngr:public cMngBasePublisherSubscriber<VRM_PUBLISHER_SEND_Q_SIZE,VRM_SUBSCRIBER_REC_Q_SIZE,VRM_THREAD_STACK_SIZE,VRM_THREAD_PRIORITY>
{
private:
		DWORD mVrAssembledCmd;//storage for assembled voice recognition command see comment above class
		BYTE PlaySnd(BYTE SndIndex, WORD Timeout);//play sound from sound table specified by index
		BYTE GetVoiceCmd(BYTE Group,DWORD TimeOut);// get command from specified group and code it on mVrAssembledCmd
		BYTE CheckVoiceCmd(BYTE Group);//check so far assembled command for correctnes and eventual farther processing
		void CmdWalleProgramVoiceCtrl(void);//execute program change to VOICE CTRL program
		void CmdWalleProgramTest(void);//execute program change to TEST program
		void CmdWalleProgramEnjoy(void);//execute program change to ENJOY program
		void CmdWalleProgramBath(void);//execute program change to BATH program
		BYTE ExecuteVoiceCmd(void);//execute assembled voice command
		BYTE InitVR(void);//initialize VR module
		BYTE StateEasyVRInit(void);//EasyVR module initialization state
		BYTE StateEasyVRInitError(void);//EasyVR module initialization erro state
		//go to sleep and wait infinite for wake up
		//returns RESULT_SLEEP_AWAKEN=0 when awaken or RESULT_SLEEP_ERROR=1 when execution error
		//InTimeOut - timeout in miliseconds or INFINITE_UART1_TIMEOUT for infinite wait
		BYTE SleepAndWaitForWakeup(DWORD InTimeOut);
		BYTE StateEasyVRSleep(void);//EasyVR module sleep state
		BYTE StateEasyVRCmdExe(void);//EasyVR module command execution state
		//request command execution by EXE MNG
		void ExeCmd(BYTE InCmdId, WORD InParameterValue);
		//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
		virtual void Run();
};//cVrmMngr


#endif /*MNG_VRM_HPP_*/
