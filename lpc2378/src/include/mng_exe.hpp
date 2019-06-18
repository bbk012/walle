/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_exe.hpp
* Description: Runs high level Wall-e control algorithm
* Author:      Bogdan Kowalczyk
* Date:        23-Nov-2013
* History:
* 23-Nov-2013 - Initial version created
* 07-Dec-2014 - changed to exe manager with intention to be a part of Wall-e brain together with the context manager
*********************************************************************************************************
*/
#ifndef EXETHREAD_H_
#define EXETHREAD_H_

#include "mng.hpp"
#include "lib_time.h"

//number of seconds execution task waits with any actions to give Wall-e time to introduce
#define EXE_MNG_WAIT_TO_START	3

//manager parameters
#define  EXE_PUBLISHER_SEND_Q_SIZE 	5
#define  EXE_SUBSCRIBER_REC_Q_SIZE 	5
#define  EXE_THREAD_STACK_SIZE		128
#define  EXE_THREAD_PRIORITY   		23

//states constants for movement algorithm state machine
#define STATE_FIND_LIGHT_SOURCE			1
#define STATE_INFORM_HEALTH_STATE		2
#define STATE_SETUP_SLEEP				3
#define STATE_PASS_OBSTACLE_ANYHOW		4
#define	STATE_TURN_OFF					5
#define STATE_MOVE_TOWARDS_LIGHT		6
#define STATE_POSITION_AT_MAX_LIGHT		7
#define STATE_SIGNAL_AT_LIGHT_SOURCE	8
#define STATE_PASS_OBSTACLES_FORWARD	9
#define STATE_CHECK_FOR_DESTINATION		10
#define STATE_SETUP_NEXT_DAY_ALARM		11
#define STATE_MOVE_ASIDE				12


//maximum movement which can be once taken moving forward
//is from SECTOR_LENGTH_COUNTS to MAX_FORWARD_ONCE_MOVE_DISTANCE count pulses randomly generated
#define MAX_FORWARD_ONCE_MOVE_DISTANCE	150   

//defines number of signals issued at the destination light wall found
#define	NUMBER_OF_SIGNALING_LOOPS	30

//defines number of maximum movement attempts when trying to resolve move error for StatePassObstacleAnyhow
#define MAX_MOVE_ATTEMPT	3

//turn directions which are determined by StepDetermineTurnDirection()
#define TURN_LEFT_DIRECTION		10
#define TURN_RIGHT_DIRECTION	20

//maximum number of nose light strength readings which are averaged to determine turn direction for StepDetermineTurnDirection()
#define MAX_LIGHT_STRENGTH_CHECKS	10

//number of minutes Walle is in sleep
#define WALLE_SLEEP_MINUTES 	1

//hours interval when Wall-e wakes up to check for eventual busy bath i.e. BATH program execution hours
#define ALARM_START_HOUR		23
#define ALARM_END_HOUR			1


#define MAX_SEQ_TOKEN_LENGTH	20 //assumes longes sequence token is 19 characters plus terminating null

//constants related to sequence pasring and programming
//strings to define exe code types
#define SEQ_EXE_CODE_ASYNC_STR	"*" //used to request asycnchronous command call (if allowed)
#define SEQ_EXE_CODE_SYNC_STR	"!" //used to request synchronous command call (if allowed)
#define SEQ_EXE_CODE_LABEL_STR	"l" //l <label name> used to mark line with a label (not a command)

//exe code IDs
#define SEQ_EXE_CODE_NO		0 //undefined execution code
#define SEQ_EXE_CODE_ASYNC	1 //used to request asycnchronous command call (if allowed)
#define SEQ_EXE_CODE_SYNC	2 //used to request synchronous command call (if allowed)
#define SEQ_EXE_CODE_LABEL	3 //used to mark line with a label (not a command)

#define NO_LINE_NUMBER_FOR_LABEL 0xFFFF //when there is no line  found with label

#define SEQ_ACTION_UNKNOWN_MSG	"\nUNKNOWN SEQUENCE COMMAND\n"

#define SEQ_HALT_DELAY_TICKS 1000 //number of OS Ticks used by infinite delay loop in HALT sequence

//action token strings
#define SEQ_ACT_DELAY_STR				"DLY" //DLY <Time In Miliseconds> - delay sequence execution
#define SEQ_ACT_HALT_STR				"HLT" //HLT - halt sequence execution for infinite time
#define	SEQ_ACT_RPT_STR					"RPT" //RPT <Label Name> [<Number>] - repeat jumping to label otional number of times
#define SEQ_ACT_END_STR					"END" //mark end of sequence
#define SEQ_ACT_MOVE_FRD_STR			"MFD" //MFD <Distance> <Profile> - move forward
#define SEQ_ACT_MOVE_BWR_STR			"MRD" //MRD <Distance> <Profile> - move backward
#define SEQ_ACT_TURN_LEFT_STR			"TRL" //TRL - turn left 90 degrees
#define SEQ_ACT_TURN_RIGHT_STR			"TRR" //TRR - turn right 90 degrees
#define	SEQ_ACT_TURN_AROUND_STR			"TRA" //TRA - turn around 360 degrees with randomly selected direction
#define SEQ_ACT_TURN_HEAD_STR			"HTR" //HTR <PositionCount> - turn head as defined by position count
#define SEQ_ACT_LEFT_ARM_STR			"LAM" //LAM <PositionCount> - move left arm as specified by position count
#define SEQ_ACT_RIGHT_ARM_STR			"RAM" //RAM <PositionCount> - move right are as specified by position count
#define SEQ_ACT_ARMS_SYNC_MOVE_STR		"ASM" //ASM <PositionCount> - move both arms to position specified by position count
#define SEQ_ACT_ARMS_OPOSIT_MOVE_STR	"AOM" //AOM <PositionCount> - move both arme but oposit direction (one up other down)
#define SEQ_ACT_ARMS_ON_STR				"AON" //AON - turn on arms servo
#define SEQ_ACT_ARMS_OFF_STR			"AOF" //AOF - turn off arms servo


//sequence actions ID
#define SEQ_ACT_END					0 //end of sequence commands - terminate sequence processing
#define SEQ_ACT_DELAY       		1 //delay
#define SEQ_ACT_HALT				2 //halt sequence execution
#define	SEQ_ACT_RPT					3 //repeat
#define SEQ_ACT_MOVE_FRD			4 //move forward
#define SEQ_ACT_MOVE_BWR			5 //move backward
#define SEQ_ACT_TURN_LEFT			6 //turn left 90 deg
#define SEQ_ACT_TURN_RIGHT			7 //turn right 90 deg
#define	SEQ_ACT_TURN_AROUND			8 //turn 360 deg direction selected rundomly
#define SEQ_ACT_TURN_HEAD			9 //turn head to sepcified position
#define SEQ_ACT_LEFT_ARM			10//trun left ar to specified position
#define SEQ_ACT_RIGHT_ARM			11//turn right arm to specified position 
#define SEQ_ACT_ARMS_SYNC_MOVE		12//turn both arms synchronously to specified position
#define SEQ_ACT_ARMS_OPOSIT_MOVE 	13//move both arm but oposit direction (one up other down)
#define SEQ_ACT_ARMS_ON				14//turn on arms servo
#define SEQ_ACT_ARMS_OFF			15//turn off arms servo

#define SEQ_ACT_NO_EXE_CODE	100 //unknown command because exe code not specified


#define EXE_MNG_CRITICAL_INFO_STR	" LOW BATTERY!"

//forward declarations
class cBrainMngr;
class cCtxMngr;

class cExeMngr:public cMngBasePublisherSubscriber<EXE_PUBLISHER_SEND_Q_SIZE,EXE_SUBSCRIBER_REC_Q_SIZE,EXE_THREAD_STACK_SIZE,EXE_THREAD_PRIORITY>
{
private:
	  cBrainMngr* pBrainMngr;//pointer to brain manager this context is assigned into
	  cCtxMngr* pCtxMngr;//pointer to context manager tight within the brain to execution manager
	  
	  char TokenBuffer[MAX_SEQ_TOKEN_LENGTH];//storage for Walle program sequence tokens when processed by parser
	  char LabelBuffer[MAX_SEQ_TOKEN_LENGTH];//storage place for RPT command label
	  WORD RptNo;//number of repetition requested by parsed RPT command
	  WORD CurrentRprNo;//current number of repetitions so far made
	  WORD SeqLnCntr;//holds index of currently executed sequence from the table of comand sequencies
	  BYTE ExeCode;  //storage to keep execution code for currently executed command
	  const char *pCurPosInSeq;//pointer to the current position within the processed sequence
		
	  WORD PreviousFrontLightLevel;//stores front light level for future comparision
	  BYTE Error;//keeps error result usefull when moving from one state to the other to know reson of the transition
	  BYTE MoveAttempts;//counts move attempts when trying to pass anyhow in StatePassObstacleAnyhow
	  WORD LightStorageTable[LIGHT_SRC_TABLE_SIZE];//storage place for light scanning results
	  WORD PowerOffResons;//used to pass to Turn Off State reason why this turn of is requested
	  tmElements_t mDateTime;//structure which stores data and time
	  time_t mTime;//storage of time as seconds since Jan 1 1970 used for time operations
	  WORD FindLeftSideMaxLightValue();//servo must be on before -> scans 3 deeper left side positions of Wall-e to find max light strength
	  WORD FindRightSideMaxLightValue();//servo must be on before -> scans 3 deeper right side positions of Wall-e to find max light strength
	  WORD FindMaxLightValue(WORD InLightSrcTable [LIGHT_SRC_TABLE_SIZE]);//find max light value from the scans
	  BYTE StepDetermineTurnDirection();//check which turn select for StateCheckForDestination;
	  BYTE StepMoveLeftCheck();//check if left movement strategy OK in StateCheckForDestination;
	  BYTE StepMoveRightCheck();//check if right movement strategy OK in StateCheckForDestination;
	  
	  //finds Seuence Line Number coresponding to the label in the sequence table InSeqCmdTbl
	  //returns sequence line number or NO_LINE_NUMBER_FOR_LABEL
	  WORD FindLineNoForLabel(const char *InSeqCmdTbl[], char *InLabel);
	  //starts from pIn to identify begining of the token string ignoring leading spaces if any
      //returns pointer to token or null if not found (because of end of the processed string)
      const char *FindTokenStart(const char *pIn);
      //find and copy token string to buffer which is next null terminated
      //returns pointer to the first character after the token or null if no token found
      const char *ExtractTokenToBuffer(const char *pTokenStart, char *pTokenBuffer, int TokenBufferSize);
      //parse command sequence line
      //return ID of the action to be executed (command) and also setup execution code ExeCode
      //which can be synchronous or asynchronous command call
      //modifies internally member variable pCurPosInSeq and ExeCode
      BYTE ParseSeq();
      
      //pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
      virtual void Run();

public:
	cExeMngr(cBrainMngr* pBrain, cCtxMngr* pCtx )
	{
		pBrainMngr=pBrain;
		pCtxMngr=pCtx;
		PreviousFrontLightLevel=0;
		Error=MOVE_OK;//assume none error on start
		MoveAttempts=0;//none move attempts when manager created
	};
	
	//--------------------------------------------------------------------------------------
	BYTE SyncMoveForwardCmd(WORD InDistancePulses,BYTE InSpeedProfile);
	BYTE SyncMoveReverseCmd(WORD InDistancePulses,BYTE InSpeedProfile);
	BYTE SyncTurnLeft90DegCmd();
	BYTE SyncTurnRight90DegCmd();
	BYTE SyncMoveOnPossiblePathCmd(WORD InDistancePulses,WORD AllowedPaths);
	BYTE SyncFindMaxLightSourceCmd();
	BYTE SyncPositionMaxLightScrFrd();
	//---------------------------------------------------------------------------------------
	void SyncScanLightObstacle(BYTE ObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE], WORD LightSrcTable [LIGHT_SRC_TABLE_SIZE]);
	void SyncScanLight(WORD LightSrcTable [LIGHT_SRC_TABLE_SIZE]);
	//--------------------------------------------------------------------------------------
	//check ahead for SURFACE, CHASM, FAR, SHORT obstacle
	//it uses US and IRED fusion and only has sense for stright forward view (HEAD CENTRAL POSITION)
	BYTE SyncCheckObstacleCmd();
	BYTE SyncCheckFrontLight(); //check ahead for DARK, GRAY, BRIGHT day
	WORD SyncReadFrontLight();//reads raw value of light sensor at Walle head
	BYTE SyncCheckBackLight(); //check back for DARK, GRAY, BRIGHT day
	//--------------------------------------------------------------------------------------
	//turn head servo on or off or move head to the InCount specified position
	void TurnHead(BYTE HandlingType,BYTE InCmdId, WORD InCount); 
	//--------------------------------------------------------------------------------------
	// turn on arm servos to be able to move arms
	void ArmServoOnCmd(BYTE HandlingType);
	// turn of arm servos
	void ArmServoOffCmd(BYTE HandlingType);
	//move left arm to specified count position
	void MoveLeftArmCmd(BYTE HandlingType,WORD InCount);
	//move left arm to the home position (horizontal position)
	void MoveLeftArmHomeCmd(BYTE HandlingType);
	//get left arm current position - independent of handling type always executed as SYNC
	WORD GetLeftArmPosCmd();
	//move right arm to specified count position
	void MoveRightArmCmd(BYTE HandlingType,WORD InCount);
	//move right arm to the home position (horizontal position)
	void MoveRightArmHomeCmd(BYTE HandlingType);
	//get right arm current position - independent of handling type always executed as SYNC
	WORD GetRightArmPosCmd();
	//move both arm synchronously to the specified count position
	void MoveArmSyncCmd(BYTE HandlingType,WORD InCount);
	//move both arms oposit to each other at specified position
	void MoveArmOpositCmd(BYTE HandlingType,WORD InCount);

	//move random distance back away from an obstacle
	//return MOVE_OK or move back errors like BRAKE_REVERSE
	BYTE SyncStepBackOnObstacle();
	//--------------------------------------------------------------------------------------	
	void SyncCmdLED(BYTE InState);//turn on or turn of Walles signaling LED
	void SyncCmdBuzzer(BYTE InState);//turn on or turn off Walle BUZZER
	void SyncCmdTurnOffWalle(WORD PowerOffReason);//turn off Walle
	//---------------------------------------------------------------	
	void SyncSetTimeDateCmd(tmElements_t *pDateTime);
	void SyncGetTimeDateCmd(tmElements_t *pDateTime);
	void SyncSetAlarmTimeDateCmd(tmElements_t *pDateTime);
	void SyncGetAlarmTimeDateCmd(tmElements_t *pDateTime);
	void SyncClearAlarmTimeDateCmd();
	//---------------------------------------------------------------
	void EvtDisplayInfo(char *pText);//send display request with critical information
	//-----------------------------------------------------------------------------------------
	//state functions for algorithm state machine
	// state function returns next state of the state machine
	BYTE StateFindLightSource(); //scan 360 degs and position at light source 
	BYTE StateInformHelthState(); //inform about weak health (for example low bat) for a while and turn off
	BYTE StateSetupSleep(); //stay sleep for a while whaiting for external condition change
	BYTE StatePassObstackleAnyhow();//find any way to pass an obstacle
	BYTE StateTurnOff();//turn off Walle
	BYTE StateMoveTowardsLight();//move forward random distance towards light source
	BYTE StatePositionAtMaxLight();//being on a way towards light correct your position to the light source
	BYTE StateSignalAtLightSource();//assuming you rach light source stay signaling for a while
	BYTE StatePassObstacleForward();//pass an obstacle but only selecting forward paths
	BYTE StateCheckForDestination();//check if final light source destination received
	BYTE StateSetupNextDayAlarm();//setup alarm for next day
	BYTE StateMoveAside();//move from the detected bath aside
	//-----------------------------------------------------------------------------------------
	//   Walle Simply Sequencer Command Execution Functions
	void SeqActMoveFrd(void);//execute move forward sequence command
	void SeqActMoveBwr(void);//execute move backword sequence command
	void SeqActDelay(void);//execute delay sequence command
	void SeqActHalt(void);//halt sequence execution for infinite time
	void SeqActRpt(const char *InSeqCmdTbl[]); //repeate sequencies by jumping to specified label optionally specified number of times
	void ExeCmdSeq(const char *InSeqCmdTbl[]);//function to parse and execute commands from the command sequance table
	void SeqActTurnLeft(void);//execute 90 deg left turn
	void SeqActTurnRight(void);//execute 90 deg right turn
	void SeqActTurnAround(void);//execute 360 deg turn
	void SeqActTurnHead(void);//execute head move
	void SeqActArmsOn(void);//turn on arms servo
	void SeqActArmsOff(void);//turn off arms servo
	void SeqActLeftArm(void);//execute left arm move
	void SeqActRightArm(void);//execute right arme move
	void SeqActArmsSyncMove(void);//execute move of both arms at the same time
	void SeqActArmsOpositMove(void);//execute move of both arms but oposit (one up on down)

	//-----------------------------------------------------------------------------------------
	//functions to execute CMD_EXE_CMD commands
	//execute CMD_EXE_CMD command
	//returns:
	//       nothing but internally RSP_EXE_CMD notifier is posted
	void ProcessCmdExe(cSmartPtr<cNotifier> pNotifier);
	
	//-----------------------------------------------------------------------------------------
	//methods for handling various possible programs executed by Walle
	void WalleProgramBath(void);//execute Wall-e program to find Barbara in the bath
	void WalleProgramVoiceCtrl(void);//Wall-e executes voice commands
	void WalleProgramTest(void);//Wall-e executes its own test sequence to check basic behaviour
	void WalleProgramEnjoy(void);//Wall-e executes its own enjoy sequence
};//cExeManager


  
#endif /*EXETHREAD_H_*/
