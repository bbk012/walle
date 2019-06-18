/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_vrm.cpp
* Description: Voice recognition module (EasyVR) manager
* Author:      Bogdan Kowalczyk
* Date:        11-Jan-2018
* Note:
* History:
*              11-Jan-2018 - Initial version created
*********************************************************************************************************
*/
#include "mng_vrm.hpp"
#include "wrp_kernel.hpp"

#include "hw_uart.h"
#include "ctr_lcd.h"
#include "lib_dbg.h"
#include "lib_std.h"
#include "hw_uart1.h"
#include "lib_e_tmr.hpp"
#include "hw_sram.h"
#include "hw_wdt.h"
#include "hw_gpio.h"
#include "hw_pwm1.h"

//VRM module debug turn on (1) turn off(0) define
#define DEBUG_VRM 1

//table of all Easy VR command sequencies recognized by Wall-e
const DWORD VoiceCmdSeqList[]=
{
		VR_CMD_SEQ1,
		VR_CMD_SEQ2,
		VR_CMD_SEQ3,
		VR_CMD_SEQ4,
		VR_CMD_SEQ5,
		VR_CMD_SEQ6,
		VR_CMD_SEQ7,
		VR_CMD_SEQ8,
		VR_CMD_SEQ9,
		VR_CMD_SEQ10,
		VR_CMD_SEQ11,
		VR_CMD_SEQ12,
		VR_CMD_SEQ13,
		VR_CMD_SEQ14,
		VR_CMD_SEQ15,
		VR_CMD_SEQ16,
		VR_CMD_SEQ17,
		VR_CMD_SEQ18,
		VR_CMD_SEQ19,
		VR_CMD_SEQ20,
		VR_CMD_SEQ21,
		VR_CMD_SEQ22,
		VR_CMD_SEQ23,
		VR_CMD_SEQ24,
		VR_CMD_SEQ25,
		VR_CMD_SEQ26,
		VR_CMD_SEQ27,
		VR_CMD_SEQ28,
		VR_CMD_SEQ29,
		VR_CMD_SEQ30,
		VR_CMD_SEQ31,
		VR_CMD_SEQ32,
		VR_CMD_SEQ33,
		VR_CMD_SEQ34,
		VR_CMD_SEQ35,
		VR_CMD_SEQ36,
		VR_CMD_SEQ37,
		VR_CMD_SEQ38,
		VR_CMD_END//this is marker of end of the table BIT31=1
};



//play sound from sound table specified by index
//   RESULT_SND_OK = 0 when sound played
//   RESULT_SND_ERROR <> 0  when an error occured
BYTE cVrmMngr::PlaySnd(BYTE SndIndex, WORD Timeout)
{
	int Result;//temp result storage
	
	Uart1PutChar(VR_CMD_BREAK);//issue break command for anything ongoing
	Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);
	if(!Result)//if none response from VR module
	{
#if DEBUG_VRM
		DbgTraceStr(2,"VrmMngr_1","\nTRC: VrmMngr: PlaySnd: ERR: Cannot execute CMD_BREAK!");
#endif//DEBUG_VRM
		return RESULT_SND_ERROR;
	}
	
	Uart1PutChar(VR_CMD_PLAY_SX);
	Uart1PutChar(VR_ARG_ZERO);
	Uart1PutChar(VR_ARG_ZERO+SndIndex);
	Uart1PutChar(SND_DOUBLE_GAIN_VOL);

	Result=Uart1GetCharWithTimeout(Timeout);
	if(Result!=VR_STS_SUCCESS)
	{
		return RESULT_SND_ERROR;//in case of snd play error or timeout report an error outside
	}
	//when snd play corectly return OK
	return RESULT_SND_OK;
}//cVrmMngr::PlaySnd

// get command from specified group (1, 2 or 3) and code it on mVrAssembledCmd
// TimeOut - time in ms for how long we wait for voice command to be received
// returns RESULT_VR_CMD_OK				0 when command recognized
// or RESULT_VR_CMD_ERROR				1 when command unrecognized or error
// important when command recognized mVrAssembledCmd is setup accordingly

BYTE cVrmMngr::GetVoiceCmd(BYTE Group, DWORD TimeOut)
{
	int Result;//temp result storage
	
	Uart1PutChar(VR_CMD_BREAK);//issue break command just for anything ongoing
	Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);
	if(!Result)//if none response from VR module
	{
#if DEBUG_VRM
	DbgTraceStr(2,"VrmMngr_2","\nTRC: VrmMngr: GetVoiceCmd: ERR: Cannot execute CMD_BREAK!");
#endif//DEBUG_VRM
	return RESULT_VR_CMD_ERROR;
	}
	
	Uart1PutChar(VR_CMD_RECOG_SD);//start recognition	
	Uart1PutChar(VR_ARG_ZERO+Group);//in the specified group
	Result=Uart1GetCharWithTimeout(TimeOut);//wait for reply
	if(Result!= VR_STS_RESULT) //when error or timeout
	{
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_2","\nTRC: VrmMngr: GetVoiceCmd: ERR: Triggered recognition NOK: ",Result);
#endif//DEBUG_VRM
		return RESULT_VR_CMD_ERROR;
	}
	Uart1PutChar(VR_ARG_ACK);//result was OK get more parameters
	Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);//wait for reply
	if(!Result)//when timeout to get command index
	{
#if DEBUG_VRM
		DbgTraceStr(2,"VrmMngr_2","\nTRC: VrmMngr: GetVoiceCmd: Timeout to get VR command Index!");
#endif//DEBUG_VRM
		return RESULT_VR_CMD_ERROR;		
	}
	Result=Result-VR_ARG_ZERO;//transform response to index
	if((Group!=1) && (!Result))//if group 2 or 3 and index==0  means break command
	{
#if DEBUG_VRM
		DbgTraceStr(2,"VrmMngr_2","\nTRC: VrmMngr: GetVoiceCmd: Requested BREAK!");
#endif//DEBUG_VRM
		return RESULT_VR_CMD_ERROR;		
	}
	switch (Group)//depending on the group process, code result accordingly
	{
		case 1:
			mVrAssembledCmd|=(DWORD)Result;
			break;
		case 2:
			mVrAssembledCmd = mVrAssembledCmd | (((DWORD)Result)<<8);//shift index and place it in right place
			break;
		case 3:
			mVrAssembledCmd = mVrAssembledCmd | (((DWORD)Result)<<16);//shift index and place it in right place
			break;
		default://that is completely wrong group - this should never happen
#if DEBUG_VRM
			DbgTraceStr(2,"VrmMngr_2","\nTRC: VrmMngr: GetVoiceCmd: Wrong group specified!");
#endif//DEBUG_VRM
			return RESULT_VR_CMD_ERROR;	
			break;
	}
#if DEBUG_VRM
	DbgTraceStrVal(2,"VrmMngr_2","\nTRC: VrmMngr: GetVoiceCmd: Command Assembled: ",mVrAssembledCmd);
#endif//DEBUG_VRM

	return RESULT_VR_CMD_OK;	
}//cVrmMngr::GetVoiceCmd

//check so far assembled command for correctnes and eventual farther processing
//Group (1, 2 or 3) points to the group so far assembled
//result: VOICE_CMD_EXE - execute, VOICE_CMD_ASM - continue command assembly, VOICE_CMD_UNKNOWN - not recognized command
BYTE cVrmMngr::CheckVoiceCmd(BYTE Group)
{
	DWORD AnalyzedCmd;//temporary storage for the assembled command being analyzed
	DWORD AllowedCmd;//temporary storage for the allowed commands from VoiceCmdSeqList
	
	BYTE Index=0;//used to select subsequent commands from the VoiceCmdSeqList
	
	AnalyzedCmd=mVrAssembledCmd;//get so far assembled command and check its correctnes
	while (!(VoiceCmdSeqList[Index]&BIT31))//as long as there is not last table entry
	{
		if(Group==1)//so far group 1 is assembled analyze it
		{
			AnalyzedCmd&=VR_CMD_MASK_GRP1;//mask so Group1 is visible only
			AllowedCmd=VoiceCmdSeqList[Index]&VR_CMD_MASK_GRP1;
			if(AnalyzedCmd==AllowedCmd)//when assembled command match allowed one
			{
				if(VoiceCmdSeqList[Index]&VR_CMD_MASK_GRP2)//if additional input required
					return VOICE_CMD_ASM;
				else //no more voice input required execute
					return VOICE_CMD_EXE;
			}
		}
		else if (Group==2) //if checking voice command assembled from group 1 and group 2
		{
			AnalyzedCmd&=VR_CMD_MASK_GRP1_GRP2;//mask so Group1 and Group 2 is visible only
			AllowedCmd=VoiceCmdSeqList[Index]&VR_CMD_MASK_GRP1_GRP2;
			if(AnalyzedCmd==AllowedCmd)//when assembled command match allowed one
			{
				if(VoiceCmdSeqList[Index]&VR_CMD_MASK_GRP3 )//if additional input required
					return VOICE_CMD_ASM;
				else //no more voice input required execute
					return VOICE_CMD_EXE;
			}
		}
		else if (Group==3)//if checking voice command assembled from group 1, 2 and 3
		{
			AnalyzedCmd&=VR_CMD_MASK_GRP1_GRP2_GRP3;//mask so Group1 and Group 2 and 3 is visible only
			AllowedCmd=VoiceCmdSeqList[Index]&VR_CMD_MASK_GRP1_GRP2_GRP3;
			if(AnalyzedCmd==AllowedCmd)//when assembled command match allowed one
			{
				return VOICE_CMD_EXE;//correct command assembled
			}
		}else //when wrong group
		{
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_3","\nTRC: VrmMngr: CheckVoiceCmd: ERR: Wrong group: ",Group);
#endif//DEBUG_VRM
		return VOICE_CMD_UNKNOWN;//wrong group requested so command cannot be recognized
		}
	Index++;//check next position from the allowed commands defined by VoiceCmdSeqList	
	}//while

	return VOICE_CMD_UNKNOWN;//all list checked and assembled command does not match to anything
	
}//cVrmMngr::CheckVoiceCmd

//initialize VR module or restart at any moment
BYTE  cVrmMngr::InitVR(void)
{
	int InitAttempts;//counter for initiallization attempts
	int Result;//result of communication attempts to EasyVR module
	for (InitAttempts=0;InitAttempts<MAX_VR_INIT_ATTEMPTS;InitAttempts++)
	{
		Uart1PutChar(VR_CMD_BREAK);//request immediate EasyVR module initialization
		Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);//get response
		if(Result==VR_STS_SUCCESS ||Result==VR_STS_AWAKEN )
		{
			Uart1PutChar(VR_CMD_TIMEOUT);//setup command timeout to infinite (VR module is not used for timeouts)
			Uart1PutChar(VR_ARG_ZERO);//timeout is setup via external timers of vr manager
			Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);//get response	
			if(Result==VR_STS_SUCCESS)
				return VR_CMD_OK;//module initialized
		}
	}
	return VR_CMD_ERROR;//cannot get module initialized
}//cVrmMngr::InitVR

//EasyVR module initialization state
BYTE  cVrmMngr::StateEasyVRInit(void)
{
#if DEBUG_VRM
	DbgTraceStr(2,"VrmMngr_3","\nTRC: VrmMngr: StateEasyVRInit: State: Init!");
#endif//DEBUG_VRM	
		
	if(InitVR())//if initialization error and EasyVR cannot be used
	{//display error information and stay in do nothing loop
#if DEBUG_VRM		
		DbgTraceStr(2,"VrmMngr_3","\nTRC: VrmMngr: StateEasyVRInit: Init Error: Cannot initialize!");
#endif //DEBUG_VRM
		return STATE_EASY_VR_INIT_ERROR;//when cannot initialize next state is ERROR state	
	}
	if(ReadLastResetReason()!= SW_RESET) //when it is not sw requested program change
	{//play welcome sentence
		PlaySnd(SND_HELLO, TIMEOUT_SND_HELLO);//play hello invitation to the world
		PlaySnd(SND_WALLE, TIMEOUT_SND_WALLE);
	}
	if(GetWalleProgramToExecute()==WALLE_PROGRAM_VOICE_CTRL)//when voice control requested
		return STATE_EASY_VR_CMD_EXE;//go directly to command execution
	else //otherwise go to sleep mode
		return STATE_EASY_VR_SLEEP;
}//cVrmMngr::StateEasyVRInit

//Infinite wait state when initialization failed
BYTE  cVrmMngr::StateEasyVRInitError(void)
{
#if DEBUG_VRM
		DbgTraceStr(2,"VrmMngr_4","\nTRC: VrmMngr: StateEasyVRInitError: Init Error!");
#endif//DEBUG_VRM	
		
	for(;;)//stay in infinite loop
	{
		Kernel.TimeDlyHMSM(0,0,VR_INFINITE_DO_NOTHING_DELAY,0);
	}
	return STATE_EASY_VR_INIT_ERROR;//never achived because of infinite wait above only to avoid warning
}//cVrmMngr::StateEasyVRInitError

//go to sleep and wait infinite for wake up
//returns RESULT_SLEEP_AWAKEN=0 when awaken or RESULT_SLEEP_ERROR=1 when execution error
//InTimeOut - timeout in miliseconds or INFINITE_UART1_TIMEOUT for infinite wait
BYTE cVrmMngr::SleepAndWaitForWakeup(DWORD InTimeOut)
{
	int Result;//keep VRM function results for subsequent executed commands
	
	Uart1PutChar(VR_CMD_BREAK);//issue break command for anything ongoing
	Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);
	if(!Result)//if none response from VR module i.e. timeout
		return RESULT_SLEEP_ERROR;//return error
	
	Uart1PutChar(VR_CMD_SLEEP);//issue go to sleep command
	Uart1PutChar(VR_SLEEP_WHISTLE_WAKE);//with wake specified by see: #define description
	
	Result=Uart1GetCharWithTimeout(VR_CMD_WAIT_TIMEOUT);
	if(Result!=VR_STS_SUCCESS)
		return RESULT_SLEEP_ERROR;//return error

	Result=Uart1GetCharWithTimeout(InTimeOut);//wait infinite for wake up
	if(Result==VR_STS_AWAKEN)
		return RESULT_SLEEP_AWAKEN;//correctly awaken
	else
		return RESULT_SLEEP_ERROR;//return error
	
}//cVrmMngr::SleepAndWaitForWakeup


//slee waiting for double clap
BYTE  cVrmMngr::StateEasyVRSleep(void)
{
	int Result;//result of go to sleep attempts

#if DEBUG_VRM
	DbgTraceStr(2,"VrmMngr_5","\nTRC: VrmMngr: StateEasyVRSleep: Sleep state!");
#endif//DEBUG_VRM	

	Result=SleepAndWaitForWakeup(INFINITE_UART1_TIMEOUT);//go to sleep and wait infinite for an wakeup
	if(Result==RESULT_SLEEP_ERROR)//if none response from VR module
	{
#if DEBUG_VRM
	DbgTraceStr(2,"VrmMngr_5","\nTRC: VrmMngr: StateEasyVRSleep: ERR:  Cannot execute GO TO SLEEP!");
#endif//DEBUG_VRM
	return STATE_EASY_VR_INIT;//if cannot put to sleep try to break and initialize again
	}	
	//if awaken correctly
	if(GetWalleProgramToExecute()==WALLE_PROGRAM_VOICE_CTRL)//when voice control mode
	{
		return STATE_EASY_VR_CMD_EXE;//go to CMD state
	}
	//when not in voice control mode request another sleep and wakeup to protect against noice triggering
	//but after some time to debaunce noice
	Delay(DELAY_SECOND_WAKE_UP);	
	PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//triple beep to request one more sleep and whistle
	PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//
	PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//
	Result=SleepAndWaitForWakeup(SECOND_WAKE_UP_TIMEOUT);
	if(Result==RESULT_SLEEP_ERROR)//if none response from VR module
	{
#if DEBUG_VRM
	DbgTraceStr(2,"VrmMngr_5","\nTRC: VrmMngr: StateEasyVRSleep: ERR:  Cannot execute 2nd GO TO SLEEP!"); 
#endif//DEBUG_VRM
	InitVR();//reinitialize VR mode to avoid it is on hold before new command is issued
	return STATE_EASY_VR_SLEEP;//if cannot execute second sleep correctly continue sleep state
	}
	//beeng there means two whistle correctly received so program change authorized
	SetWalleProgramToExecute(WALLE_PROGRAM_VOICE_CTRL);//setup VOICE CTR MODE
	uPResetByWatchdog(SW_RESET);//and RESET Wall-e to start it 
	for(;;)//infinite loop to disable further program execution until reset
		{
		}//infinite for
	return STATE_EASY_VR_INIT;//this line should never be reached is only to avoid warning
}//cVrmMngr::StateEasyVRSleep

//state to assemble voice command and execute it if correctly assembled
BYTE  cVrmMngr::StateEasyVRCmdExe(void)
{
	BYTE Result;//result of communication attempts to EasyVR module
	BYTE GroupProcessed;//index of voice group currently processed
	cEasyTimer CmdTimer;//setup timer for CMD state

#if DEBUG_VRM
	DbgTraceStr(2,"VrmMngr_6","\nTRC: VrmMngr: StateEasyVRCmdExe: Exe state!");
#endif//DEBUG_VRM
		
	CmdTimer.StartDelayInMinutes(STATE_CMD_EXE_TIMEOUT);//timer before going to sleep
	
	while(!CmdTimer.isExpired())//as long as CMD state timer not expired
	{
		mVrAssembledCmd=VR_IDX_CMD_EMPTY;//none command at start
		GroupProcessed=1;//start with first group
		PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//play beep to indicate wait for command sequence
		while(TRUE)
		{
#if DEBUG_VRM
			DbgTraceStrVal(2,"VrmMngr_6","\nTRC: VrmMngr: StateEasyVRCmdExe: Exe State Group: ",GroupProcessed);
#endif//DEBUG_VRM				
			if(GroupProcessed==1)
				Result=GetVoiceCmd(GroupProcessed,TIMEOUT_VR_CMD_GRP1);
			else
				Result=GetVoiceCmd(GroupProcessed,TIMEOUT_VR_CMD_GRP2_GRP3);
			if(Result==RESULT_VR_CMD_ERROR)
			{
				PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//play beep twice to indicate problem
				break;//break loop when no correct voice command provided
			}
			Result=CheckVoiceCmd(GroupProcessed);//check how process provided command
			if(Result==VOICE_CMD_UNKNOWN)
			{
				PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//play beep twice to indicate problem
				break;//break voice cmd sequence when unknown command
			}
			if(Result==VOICE_CMD_ASM)
			{
				GroupProcessed+=1;//get to next group
				PlaySnd(SND_BEEP, TIMEOUT_SND_BEEP);//play beep to ask for another command
				continue;
			}
			//getting this place means CheckVoiceCmd result is execute command 
			Result=ExecuteVoiceCmd();//execute assembled voice command
			if(Result==VOICE_EXE_RESULT_SLEEP)//when command sleep requested
				CmdTimer.Expire();//force timer to expire so switch to command sleep mode is made
			else
				CmdTimer.Reset();//start timer always again when voice command provided
			//continue command processing
			break;
		}//while(TRUE)
	}//while(!CmdTimer.isExpired())
	PlaySnd(SND_CMD_SLEEP, TIMEOUT_SND_CMD_SLEEP);//play info about going to voice sleep	
	return STATE_EASY_VR_SLEEP;
}//cVrmMngr::StateEasyVRCmdExe

//execute program change to VOICE CTRL program
void cVrmMngr::CmdWalleProgramVoiceCtrl(void)
{
	if(GetWalleProgramToExecute()==WALLE_PROGRAM_VOICE_CTRL)
		return;//do nothing if already in VOICE CTRL mode
	SetWalleProgramToExecute(WALLE_PROGRAM_VOICE_CTRL);//setup VOICE CTR MODE
	uPResetByWatchdog(SW_RESET);//and RESET Wall-e to start it 
	for(;;)//infinite loop to disable further program execution until reset
	{
	}//infinite for	
}//cVrmMngr::CmdWalleProgramVoiceCtrl

//execute program change to TEST program
void cVrmMngr::CmdWalleProgramTest(void)
{
	if(GetWalleProgramToExecute()==WALLE_PROGRAM_TEST)
		return;//do nothing if already WALLE TEST program is executed
	SetWalleProgramToExecute(WALLE_PROGRAM_TEST);//setup WALLE TEST program
	uPResetByWatchdog(SW_RESET);//and RESET Wall-e to start it 
	for(;;)//infinite loop to disable further program execution until reset
	{
	}//infinite for	
}//cVrmMngr::CmdWalleProgramTest

//execute program change to ENJOY program
void cVrmMngr::CmdWalleProgramEnjoy(void)
{
	if(GetWalleProgramToExecute()==WALLE_PROGRAM_ENJOY)
		return;//do nothing if already WALLE ENJOY program is executed
	SetWalleProgramToExecute(WALLE_PROGRAM_ENJOY);//setup WALLE ENJOY program
	uPResetByWatchdog(SW_RESET);//and RESET Wall-e to start it 
	for(;;)//infinite loop to disable further program execution until reset
	{
	}//infinite for		
}//cVrmMngr::CmdWalleProgramEnjoy

//execute program change to BATH program
void cVrmMngr::CmdWalleProgramBath(void)
{
	if(GetWalleProgramToExecute()==WALLE_PROGRAM_BATH)
		return;//do nothing if already WALLE BATH program is executed
	SetWalleProgramToExecute(WALLE_PROGRAM_BATH);//setup WALLE BATH program
	uPResetByWatchdog(SW_RESET);//and RESET Wall-e to start it 
	for(;;)//infinite loop to disable further program execution until reset
	{
	}//infinite for			
}//cVrmMngr::CmdWalleProgramBath

//request command execution by EXE MNG
void cVrmMngr::ExeCmd(BYTE InCmdId, WORD InParameterValue)
{
	cSmartPtr<cTypeNotifier<sCmdExeData> > pNotifier = new cTypeNotifier<sCmdExeData>(CMD_EXE_CMD,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mCmdExeId=InCmdId; //Id of requested command
	(pNotifier->GetData()).mParameterValue=InParameterValue;//value of a command paramiter which depends on requested command
	(pNotifier->GetData()).mHandling=SYNC_HANDLING;//synchronous or asynchronous handling type
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sCmdExeData> > pNotifierRSP;//notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_EXE_CMD) continue;//skip any notifier different to RSP_EXE_CMD
	}while((pNotifierRSP->GetData()).mCmdExeId!=InCmdId);//untill we get response for the command
	
}//cVrmMngr::ExeCmd


//execute command assembled by StateEasyVRCmdExe
BYTE  cVrmMngr::ExecuteVoiceCmd(void)
{
	switch(mVrAssembledCmd&VR_CMD_MASK_GRP1_GRP2_GRP3)
	{
	case VR_CMD_SEQ1://who are you
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM		
		PlaySnd(SND_WALLE, TIMEOUT_SND_WALLE);//play hello invitation to the world
		break;
	case VR_CMD_SEQ2://do you know me
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		PlaySnd(SND_BOGDAN, TIMEOUT_SND_BOGDAN);//play creator is known by Walle :-) 
		break;
	case VR_CMD_SEQ3://walle turn off
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		PlaySnd(SND_TURN_OFF, TIMEOUT_SND_TURN_OFF);//play Turn off to the world
		uPBoardOff(REMOTE_TURN_OFF); //turn off Wall-e
		break;
	case VR_CMD_SEQ4://voice command sleep mode
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		//PlaySnd(SND_SLEEP, TIMEOUT_SND_SLEEP);//play hello invitation to the world
		return VOICE_EXE_RESULT_SLEEP;//signal voice command sleep mode requested
		break;
	case VR_CMD_SEQ5://walle backward short movement
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_REVERSE_ID,MOVE_SHORT_DISTANCE);
		break;
	case VR_CMD_SEQ6://walle backward little movement
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_REVERSE_ID,MOVE_LITTLE_DISTANCE);
		break;
	case VR_CMD_SEQ7://walle backward far movement
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_REVERSE_ID,MOVE_FAR_DISTANCE);
		break;
	case VR_CMD_SEQ8://walle forward short movement
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_FORWARD_ID,MOVE_SHORT_DISTANCE);
		break;
	case VR_CMD_SEQ9://walle forward little movement
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_FORWARD_ID,MOVE_LITTLE_DISTANCE);
		break;
	case VR_CMD_SEQ10://walle forward far movement
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_FORWARD_ID,MOVE_FAR_DISTANCE);
		break;
	case VR_CMD_SEQ11://walle turn left
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(TURN_LEFT_90,0);
		break;
	case VR_CMD_SEQ12://walle turn right
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(TURN_RIGHT_90,0);
		break;
	case VR_CMD_SEQ13://walle trun around
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(TURN_360,0);
		break;
	case VR_CMD_SEQ14://walle program voice control
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		CmdWalleProgramVoiceCtrl();
		break;
	case VR_CMD_SEQ15://walle program test
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		CmdWalleProgramTest();
		break;
	case VR_CMD_SEQ16://walle program enjoy
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		CmdWalleProgramEnjoy();
		break;	
	case VR_CMD_SEQ17://walle program barth
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		CmdWalleProgramBath();
		break;	
	case VR_CMD_SEQ18://head left some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_LEFT_30_DEG);
		break;		
	case VR_CMD_SEQ19://head left a lot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_LEFT_60_DEG);
		break;
	case VR_CMD_SEQ20://head right soem
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_RIGHT_30_DEG);
		break;		
	case VR_CMD_SEQ21://head right a lot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_RIGHT_60_DEG);
		break;
	case VR_CMD_SEQ22://left arm up some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_LEFT_ARM_ID,LEFT_ARM_SOME_UP);
		break;
	case VR_CMD_SEQ23://left arm up a lot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_LEFT_ARM_ID,LEFT_ARM_MAX_UP);
		break;	
	case VR_CMD_SEQ24://left arm down some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_LEFT_ARM_ID,LEFT_ARM_SOME_DOWN);
		break;	
	case VR_CMD_SEQ25://left arm down alot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_LEFT_ARM_ID,LEFT_ARM_MAX_DOWN);
		break;
	case VR_CMD_SEQ26://right arm up some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_RIGHT_ARM_ID,RIGHT_ARM_SOME_UP);
		break;
	case VR_CMD_SEQ27://right arm up a lot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_RIGHT_ARM_ID,RIGHT_ARM_MAX_UP);
		break;	
	case VR_CMD_SEQ28://right arm down some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_RIGHT_ARM_ID,RIGHT_ARM_SOME_DOWN);
		break;	
	case VR_CMD_SEQ29://right arm down alot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_RIGHT_ARM_ID,RIGHT_ARM_MAX_DOWN);
		break;
	case VR_CMD_SEQ30://arms up some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_ARM_SYNC_ID,RIGHT_ARM_SOME_UP);
		break;
	case VR_CMD_SEQ31://arms up alot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_ARM_SYNC_ID,RIGHT_ARM_MAX_UP);
		break;
	case VR_CMD_SEQ32://arms down some
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_ARM_SYNC_ID,RIGHT_ARM_SOME_DOWN);
		break;
	case VR_CMD_SEQ33://arms down a lot
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_ARM_SYNC_ID,RIGHT_ARM_MAX_DOWN);
		break;
	case VR_CMD_SEQ34://head shake
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_CENTRAL);
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_LEFT_30_DEG);
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_RIGHT_30_DEG);
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_CENTRAL);
		break;
	case VR_CMD_SEQ35://head center
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_TURN_HEAD_ID,HEAD_CENTRAL);
		break;
	case VR_CMD_SEQ36://left arm center
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_LEFT_ARM_ID,LEFT_ARM_HORIZONTAL);
		break;
	case VR_CMD_SEQ37://right arm center
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_RIGHT_ARM_ID,RIGHT_ARM_HORIZONTAL);
		break;
	case VR_CMD_SEQ38://arms center	
#if DEBUG_VRM
		DbgTraceStrVal(2,"VrmMngr_7","\nTRC: VrmMngr: ExecuteVoiceCmd: ",mVrAssembledCmd);
#endif//DEBUG_VRM
		ExeCmd(CMD_EXE_ARM_SYNC_ID,RIGHT_ARM_HORIZONTAL);
		break;
	default:
		break;
		
	}
	return VOICE_EXE_RESULT_OK;
}//cVrmMngr::ExecuteVoiceCmd

//Voice Recognition Module thread execution function
void cVrmMngr::Run()
{
	BYTE State;//keeps current state of the EasyVR manager state
	
	State=STATE_EASY_VR_INIT;//first state is initialization state
#if DEBUG_VRM
	DbgTraceStr(1,"VrmMngr_7","\nTRC: VrmMngr: Run: Run!");
#endif//DEBUG_VRM	
	
	for(;;)
	{
		switch(State)
		{
		case STATE_EASY_VR_INIT:
			State=StateEasyVRInit();
			break;
		case STATE_EASY_VR_INIT_ERROR:
			State=StateEasyVRInitError();
			break;
		case STATE_EASY_VR_SLEEP:
			State=StateEasyVRSleep();
			break;
		case STATE_EASY_VR_CMD_EXE:
			State=StateEasyVRCmdExe();
			break;
		default:
			LCDDebugMessage("ERR: Wrng Sts: ",State,20,1,1,0);
			Uart0Message("ERR: Vrm: Wrong state: ",State);
			NOT_ALLOWED_STATE;
			break;
		}//switch(State)
	}//for(;;)
}//cVrmMngr::Run
