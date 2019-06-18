/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_exe.cpp
* Description: Runs high level Wall-e control algorithm
* Author:      Bogdan Kowalczyk
* Date:        2e-Nov-2013
* History:
* 23-Nov-2013 - Initial version created
* 07-Dec-2014 - AppMngr changed to ExeMngr intended to ba a part of Wall-e brain manager together with the context manager
*********************************************************************************************************
*/

#include "mng_exe.hpp"
#include "mng_ctx.hpp"
#include "wrp_kernel.hpp"
#include "hw_gpio.h"
#include "ctr_lcd.h"
#include "hw_pwm1.h"
#include "tsk_tracks.h"
#include "lib_std.h"
#include "hw_uart.h"
#include "lib_dbg.h"
#include "hw_sram.h"
#include "hw_wdt.h"

//Change to 0 for destination code to eliminate transimition to terminal
#define DEBUG_MNG_EXE 1
//Debug trace/stop label: ExeMng_n  n - number of the subsequent function

//delay in OS Ticks for infinite loop in which program is executed
#define EXE_INFINITE_LOOP_DELAY	100
//delay in OS Ticks before again executed enjoy program starts
#define EXE_ENJOY_DELAY 500

//sequence of commands executed by WALL-E when TEST program is executed
const static char *WalleTestSeqCmdTbl[]={
		"! HTR 933",//turn left 45 deg
		"! DLY 500",//wait 500ms
		"! HTR 483",//turn right 45 deg
		"! DLY 500",//wait 500ms
		"! HTR 708",//turn central
		"! DLY 500",//wait 500ms
		"! AON",//turn on arms servo
		"! LAM 521",//left arm max up
		"! DLY 500",//wait 500ms
		"! RAM 921",//right arm max up
		"! DLY 500",//wait 500ms
		"! AOF",//turn off arms servo
		"! MFD 30 2",//move forward 30 steps using LOW_PROFILE	
		"! MRD 30 2",//move backward 30 steps using LOW_PROFILE
		"! TRL",//turn left 90 degs
		"! TRR", //turn right 90 degs
		"! END"//terminate sequence
};

//sequence of commands executed by Wall-e for ENJOY program
const static char *WalleEnjoySeqCmdTbl[]={
		"l start",
		"! MFD 10 2", //move forward 10 steps using LOW_PROFILE	
		"! MRD 10 2", //move backward 10 steps using LOW_PROFILE
		"! AON",	  //turn on arms servo
		"! AOM 921",  //move oposit so right arm is max up
		"! AOM 610",  //move oposit so right arm is max down
		"* LAM 688",  //left arm horizontal position,
		"* RAM 746",  //right arm horizontal position
		"! AOF",      //turn off arms servo
		"! HTR 783",  //turn left 15 deg
		"! HTR 633",  //turn right 15 deg
		"! HTR 708",  //turn central
		"! TRR",      //turn Walle right 90 degs
		"! RPT start 4",//repeat 4 times to make full 360 deg movement
		"! END"//terminate sequence
};

//scans 3 deeper left side positions of Wall-e to find max light strength on Wall-e left side
//head servo needs to be on before ussage
WORD cExeMngr::FindLeftSideMaxLightValue()
{
	WORD MaxVal=0; 	//temporary max value
	WORD LeftDeg45; //storage for light at 45 degs
	WORD LeftDeg60; //storage for light at 60 degs
	WORD LeftMaxTurn; //storage for light at max left turn
	
	TurnHead(SYNC_HANDLING,TURN_HEAD_MOVE_SCMD_ID,HEAD_LEFT_45_DEG);
	LeftDeg45=SyncReadFrontLight();
		
	TurnHead(SYNC_HANDLING,TURN_HEAD_MOVE_SCMD_ID,HEAD_LEFT_60_DEG);
	LeftDeg60=SyncReadFrontLight();
		
	TurnHead(SYNC_HANDLING,TURN_HEAD_MOVE_SCMD_ID,HEAD_MAX_LEFT_TURN);
	LeftMaxTurn=SyncReadFrontLight();
		
	//select max from the 3 stored values
	if(LeftDeg45>MaxVal)MaxVal=LeftDeg45;
	if(LeftDeg60>MaxVal)MaxVal=LeftDeg60;
	if(LeftMaxTurn>MaxVal)MaxVal=LeftMaxTurn;
	
	return MaxVal; 	
	
}//cExeMngr::FindLeftSideMaxLightValue

//scans 3 deeper right side positions of Wall-e to find max light strength on Wall-e right side
//head servo needs to be on before ussage
WORD cExeMngr::FindRightSideMaxLightValue()
{
	WORD MaxVal=0;//temporary max value
	WORD RightDeg45; //storage for light at 45 degs
	WORD RightDeg60; //storage for light at 60 degs
	WORD RightMaxTurn; //storage for light at max right turn
	
	TurnHead(SYNC_HANDLING,TURN_HEAD_MOVE_SCMD_ID,HEAD_RIGHT_45_DEG);
	RightDeg45=SyncReadFrontLight();
		
	TurnHead(SYNC_HANDLING,TURN_HEAD_MOVE_SCMD_ID,HEAD_RIGHT_60_DEG);
	RightDeg60=SyncReadFrontLight();
		
	TurnHead(SYNC_HANDLING,TURN_HEAD_MOVE_SCMD_ID,HEAD_MAX_RIGHT_TURN);
	RightMaxTurn=SyncReadFrontLight();
			
	//select max from the 3 stored values
	if(RightDeg45>MaxVal)MaxVal=RightDeg45;
	if(RightDeg60>MaxVal)MaxVal=RightDeg60;
	if(RightMaxTurn>MaxVal)MaxVal=RightMaxTurn;
	
	return MaxVal; 	
}//cExeMngr::FindRightSideMaxLightValue
	  
//find max light value from the scans in input LightSrcTable		
WORD cExeMngr::FindMaxLightValue(WORD InLightSrcTable [LIGHT_SRC_TABLE_SIZE])
{
	WORD MaxVal=0;//temporary max value
	for (int i=0;i<LIGHT_SRC_TABLE_SIZE;i++)//check of light tables entries
		{
		if(InLightSrcTable[i]>MaxVal)
			{
			MaxVal=InLightSrcTable[i];
			}
		}
	return MaxVal;
}//cExeMngr::FindMaxLightValue
		 
//-----------------------------------------------------------------------------------------------------------
// STEP functions
// Set if supportive functions called by state functions

//check which turn select for StateCheckForDestination
//can be either TURN_LEFT_DIRECTION or TURN_RIGHT_DIRECTION
BYTE cExeMngr::StepDetermineTurnDirection()
{
	WORD LeftLightStrength;
	WORD RightLightStrength;

	TurnHead(SYNC_HANDLING,TURN_HEAD_ON_SCMD_ID,HEAD_CENTRAL);//turn on head servo
	LeftLightStrength =FindLeftSideMaxLightValue();//scan left side
	RightLightStrength=FindRightSideMaxLightValue();//scan right side
	TurnHead(SYNC_HANDLING,TURN_HEAD_OFF_SCMD_ID,HEAD_CENTRAL);//move to central head position and turn off head servo
	
	//determine direction which is checked
	if(RightLightStrength>LeftLightStrength)
		return TURN_RIGHT_DIRECTION;
	else
		return TURN_LEFT_DIRECTION;
}//cExeMngr::StepDetermineTurnDirection

//check if left movement strategy OK in StateCheckForDestination
//returns next state to be executed
BYTE cExeMngr::StepMoveLeftCheck()
{
	WORD InitialLeftLightStrength;//value of light strength just after turn, before movement
	WORD SectorLeftLightStrength;//value of light strength after moving one sector left
	
	Error=SyncTurnLeft90DegCmd();//make turn left
	if(Error)//if any movement error check cannot be executed make then any attempt to return back
		{
		SyncStepBackOnObstacle();//make a small step back
		SyncTurnRight90DegCmd();//and turn back towards prevoius light assuming we got destination
		return STATE_SIGNAL_AT_LIGHT_SOURCE;
		}
	
	SyncScanLight(LightStorageTable);//scan Wall-e seroundings
	InitialLeftLightStrength = FindMaxLightValue(LightStorageTable);//determine initial value as the max from scanned
	
	//move one sectot forward being in left position to check light strength
	Error=SyncMoveForwardCmd(SECTOR_LENGTH_COUNTS,LOW_PROFILE);
	if(Error)//if any movement error check cannot be executed make then an attempt to return back
		{
		SyncStepBackOnObstacle();//make a small step back (we do not know distance covered without error)
		SyncTurnRight90DegCmd();//and turn back towards prevoius light assuming we got destination
		return STATE_SIGNAL_AT_LIGHT_SOURCE;
		}
	
	SyncScanLight(LightStorageTable);//scan Wall-e seroundings
	SectorLeftLightStrength = FindMaxLightValue(LightStorageTable);//determine initial value as the max from scanned

	if(SectorLeftLightStrength>InitialLeftLightStrength)//if we move aside and light increase so we were not at destination
		{
		PreviousFrontLightLevel=SectorLeftLightStrength;//store light strength from last position
		return STATE_MOVE_TOWARDS_LIGHT; //further make movements towards that light
		}
	
	//if light decreases when moving aside return back to destination position which is the light source
	//move back to destination position any errors on a way back are ignored because we cannot handle those better
	SyncMoveReverseCmd(SECTOR_LENGTH_COUNTS,LOW_PROFILE); //move back
	SyncTurnRight90DegCmd();//and make turn left 
	//we got destination so next state is to make destination signalling
	return STATE_SIGNAL_AT_LIGHT_SOURCE;
}//cExeMngr::StepMoveLeftCheck

//check if right movement strategy OK in StateCheckForDestination;
BYTE cExeMngr::StepMoveRightCheck()
{
	WORD InitialRightLightStrength;//value of light strength just after turn, before movement
	WORD SectorRightLightStrength;//value of light strength after moving one sector right
	
	Error=SyncTurnRight90DegCmd();//make turn right
	if(Error)//if any movement error check cannot be executed make then any attempt to return back
		{
		SyncStepBackOnObstacle();//make a small step back
		SyncTurnLeft90DegCmd();//and turn back towards prevoius light assuming we got destination
		return STATE_SIGNAL_AT_LIGHT_SOURCE;
		}
	
	SyncScanLight(LightStorageTable);//scan Wall-e seroundings
	InitialRightLightStrength = FindMaxLightValue(LightStorageTable);//determine initial value as the max from scanned

	//move one sectot forward being in left position to check light strength
	Error=SyncMoveForwardCmd(SECTOR_LENGTH_COUNTS,LOW_PROFILE);
	if(Error)//if any movement error check cannot be executed make then an attempt to return back
		{
		SyncStepBackOnObstacle();//make a small step back (we do not know distance covered without error)
		SyncTurnLeft90DegCmd();//and turn back towards prevoius light assuming we got destination
		return STATE_SIGNAL_AT_LIGHT_SOURCE;
		}
	
	SyncScanLight(LightStorageTable);//scan Wall-e seroundings
	SectorRightLightStrength = FindMaxLightValue(LightStorageTable);//determine initial value as the max from scanned

	if(SectorRightLightStrength>InitialRightLightStrength)//if we move aside and light increase so we were not at destination
		{
		PreviousFrontLightLevel=SectorRightLightStrength;//store light strength from last position
		return STATE_MOVE_TOWARDS_LIGHT; //further make movements towards that light
		}
	
	//if light decreases when moving aside return back to destination position which is the light source
	//move back to destination position any errors on a way back are ignored because we cannot handle those better
	SyncMoveReverseCmd(SECTOR_LENGTH_COUNTS,LOW_PROFILE); //move back
	SyncTurnLeft90DegCmd();//and make turn left 
	//we got destination so next state is to make destination signalling
	return STATE_SIGNAL_AT_LIGHT_SOURCE;
}//cExeMngr::StepMoveRightCheck

//-----------------------------------------------------------------------------------------------------------
// MOVE COMMANDS

//inputs:
//       InDistancePulses - desired distance in pulses
//       InDirection - desired direction (MOTOR_FORWARD, MOTOR_REVERSE)
//       InSpeedProfile - desired profile of the movement (LOW_MID_HIGH_PROFILE, LOW_MID_PROFILE, LOW_PROFILE, MID_PROFILE, HIGH_PROFILE)
//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_FROWARD <>0 Error - movement cannot be realized none chnge in pulses
//       MOVE_BREAK_OBSTACLE
BYTE cExeMngr::SyncMoveForwardCmd(WORD InDistancePulses,BYTE InSpeedProfile)
{
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= FORWARD_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mDistancePulses=InDistancePulses;
	(pNotifier->GetData()).mSpeedProfile=InSpeedProfile;
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=FORWARD_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	

}//cExeMngr::SyncMoveForwardCmd

//inputs:
//       InDistancePulses - desired distance in pulses
//       InDirection - desired direction (MOTOR_FORWARD, MOTOR_REVERSE)
//       InSpeedProfile - desired profile of the movement (LOW_MID_HIGH_PROFILE, LOW_MID_PROFILE, LOW_PROFILE, MID_PROFILE, HIGH_PROFILE)
//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_REVERSE <>0 Error - movement cannot be realized none chnge in pulses
//       MOVE_BREAK_OBSTACLE
BYTE cExeMngr::SyncMoveReverseCmd(WORD InDistancePulses,BYTE InSpeedProfile)
{
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= REVERSE_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mDistancePulses=InDistancePulses;
	(pNotifier->GetData()).mSpeedProfile=InSpeedProfile;
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=REVERSE_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	
	
}//cExeMngr::SyncMoveReverseCmd

//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_LEFT - Error - movement cannot be realized none chnge in pulses
//       MOVE_ANGLE_LEFT  - Error - destination angle not achieved
BYTE cExeMngr::SyncTurnLeft90DegCmd()
{
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= LEFT90DEG_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=LEFT90DEG_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	

}//cExeMngr::SyncTurnLeft90DegCmd

//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_RIGHT - Error - movement cannot be realized none chnge in pulses
//       MOVE_ANGLE_RIGHT  - Error - destination angle not achieved
BYTE cExeMngr::SyncTurnRight90DegCmd()
{
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= RIGHT90DEG_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=RIGHT90DEG_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	
}//cExeMngr::SyncTurnRight90DegCmd

//inputs:
//       InDistancePulses - desired distance in pulses
//       AllowedPaths - mask which determines allowed paths to be executed for the movement
//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_FORWARD <>0 Error - movement cannot be realized none chnge in pulses when moving forward
//       MOVE_BREAK_OBSTACLE Error obstacle detected when moving forward
//       MOVE_BREAK_LEFT - Error - movement cannot be realized none chnge in pulses when truning left
//       MOVE_BREAK_RIGHT Error - movement cannot be realized none chnge in pulses when turning right
//       MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turning left
//       MOVE_ANGLE_RIGHT Error - destination angle not achieved when turning right
BYTE cExeMngr::SyncMoveOnPossiblePathCmd(WORD InDistancePulses,WORD AllowedPaths)
{

	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= RAND_PATH_MOVE_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mDistancePulses=InDistancePulses;
	(pNotifier->GetData()).mSpeedProfile=AllowedPaths;
	Post(pNotifier);//post command	
	

	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=RAND_PATH_MOVE_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	

}//cExeMngr::SyncMoveOnPossiblePathCmd


//inputs:
//      none
//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_LEFT - Error - movement cannot be realized none chnge in pulses when truning left
//       MOVE_BREAK_RIGHT Error - movement cannot be realized none chnge in pulses when turning right
//       MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turning left
//       MOVE_ANGLE_RIGHT Error - destination angle not achieved when turning right
//       MOVE_NONE_LIGHT_SRC - none meaningful light source was identified
BYTE cExeMngr::SyncFindMaxLightSourceCmd()
{

	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= FIND_MAX_LIGHT_SRC_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=FIND_MAX_LIGHT_SRC_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	

}//cExeMngr::SyncFindLightSourceCmd

//inputs:
//       none
//returns:
//       MOVE_OK - 0 - O.K. movement executed
//       MOVE_BREAK_LEFT - Error - movement cannot be realized none chnge in pulses when truning left
//       MOVE_BREAK_RIGHT Error - movement cannot be realized none chnge in pulses when turning right
//       MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turning left
//       MOVE_ANGLE_RIGHT Error - destination angle not achieved when turning right
//       MOVE_NONE_LIGHT_SRC - none meaningful light source was identified
BYTE cExeMngr::SyncPositionMaxLightScrFrd()
{
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifier = new cTypeNotifier<sMoveData>(CMD_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mMoveCmdId= POSITION_AT_MAX_LIGHT_FRD_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE) continue;//skip any notifier different to RSP_MOVE
	}while((pNotifierRSP->GetData()).mMoveCmdId!=POSITION_AT_MAX_LIGHT_FRD_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mResult;	

}//cExeMngr::SyncPositionMaxLightScrFrd
//END MOVE COMMANDS
//-----------------------------------------------------------------------------------------------------------
//SCAN COMMANDS


//inputs:
//       ObstacleTable - address of obstacle table where scanned data are stored
//       LightSrcTable - address of light table where light scaning data are stored 
//returns:
//		 none but storage places are updated with scanning results
//IMPORTANT! Should always be called with SYNC_HANDLING to assure correct execution result
//           stored for ObstacleTable and LightTable
void cExeMngr::SyncScanLightObstacle(BYTE ObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE], WORD LightSrcTable [LIGHT_SRC_TABLE_SIZE])
{
	cSmartPtr<cTypeNotifier<sScanData> > pNotifier = new cTypeNotifier<sScanData>(CMD_SCAN,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mScanDataCmdId= SCAN_LIGHT_OBSTACLE_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sScanData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_SCAN) continue;//skip any notifier different to RSP_SCAN
	}while((pNotifierRSP->GetData()).mScanDataCmdId!=SCAN_LIGHT_OBSTACLE_SCMD_ID);//untill we get response for the command
	for(int y=0;y<OBSTACLE_TABLE_Y_SIZE;y++)
		{
			for (int x=0;x<OBSTACLE_TABLE_X_SIZE;x++)
				{
					ObstacleTable[y][x]=(pNotifierRSP->GetData()).mObstacleTable[y][x];//copy obstacle data to destination
				}
		}
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++)
		{
			LightSrcTable[x]=(pNotifierRSP->GetData()).mLightSrcTable[x];//copy light scaning information to destination storage
		}
	
}//cExeMngr::SyncScanLightObstacle

//inputs:
//       LightSrcTable - address of light table where light scaning data are stored 
//returns:
//		 none but storage places are updated with scanning results
//IMPORTANT! Should always be called with SYNC_HANDLING to assure correct execution result
//           stored for LightTable
void cExeMngr::SyncScanLight(WORD LightSrcTable [LIGHT_SRC_TABLE_SIZE])
{
	cSmartPtr<cTypeNotifier<sScanData> > pNotifier = new cTypeNotifier<sScanData>(CMD_SCAN,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mScanDataCmdId= SCAN_LIGHT_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	

	cSmartPtr<cTypeNotifier<sScanData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_SCAN) continue;//skip any notifier different to RSP_SCAN
	}while((pNotifierRSP->GetData()).mScanDataCmdId!=SCAN_LIGHT_SCMD_ID);//untill we get response for the command
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++)
		{
		LightSrcTable[x]=(pNotifierRSP->GetData()).mLightSrcTable[x];//copy light scaning information to destination storage
		}

}//cExeMngr::SyncScanLight
//END SCAN COMMANDS

//-----------------------------------------------------------------------------------------------------------
//CHECK COMMANDS

//check ahead for SURFACE, CHASM, FAR, SHORT obstacle
//it uses US and IRED fusion and only has sense for stright forward view (HEAD CENTRAL POSITION)
BYTE cExeMngr::SyncCheckObstacleCmd()
{
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifier = new cTypeNotifier<sCheckData>(CMD_CHECK,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mCheckDataCmdId=CHECK_OBSTACLE_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_CHECK) continue;//skip any notifier different to RSP_CHECK
	}while((pNotifierRSP->GetData()).mCheckDataCmdId!=CHECK_OBSTACLE_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mType; 
	
}//cExeMngr::SyncCheckObstacleCmd

//check ahead for DARK, GRAY, BRIGHT day
BYTE cExeMngr::SyncCheckFrontLight()
{
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifier = new cTypeNotifier<sCheckData>(CMD_CHECK,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mCheckDataCmdId=CHECK_FRONT_LIGHT_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_CHECK) continue;//skip any notifier different to RSP_CHECK
	}while((pNotifierRSP->GetData()).mCheckDataCmdId!=CHECK_FRONT_LIGHT_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mType; 

}//cExeMngr::SyncCheckFrontLight

//reads current value from the head fototranzistor
WORD cExeMngr::SyncReadFrontLight()
{
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifier = new cTypeNotifier<sCheckData>(CMD_CHECK,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mCheckDataCmdId=CHECK_FRONT_LIGHT_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_CHECK) continue;//skip any notifier different to RSP_CHECK
	}while((pNotifierRSP->GetData()).mCheckDataCmdId!=CHECK_FRONT_LIGHT_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mRawData; 

}//cExeMngr::SyncCheckFrontLight

//check back for DARK, GRAY, BRIGHT day
BYTE cExeMngr::SyncCheckBackLight()
{
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifier = new cTypeNotifier<sCheckData>(CMD_CHECK,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mCheckDataCmdId=CHECK_BACK_LIGHT_SCMD_ID;//setup sub-command requested to be executed

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sCheckData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_CHECK) continue;//skip any notifier different to RSP_CHECK
	}while((pNotifierRSP->GetData()).mCheckDataCmdId!=CHECK_BACK_LIGHT_SCMD_ID);//untill we get response for the command
	return (pNotifierRSP->GetData()).mType; 

}//cExeMngr::SyncCheckBackLight
//END SCAN COMMANDS

//-----------------------------------------------------------------------------------------------------------
//HEAD TURN COMMANDS
//turn head servo on or off or move head to the InCount specified position

void cExeMngr::TurnHead(BYTE HandlingType,BYTE InCmdId, WORD InCount)
{
	cSmartPtr<cTypeNotifier<sTurnHeadData> > pNotifier = new cTypeNotifier<sTurnHeadData>(CMD_TURN_HEAD,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mTurnHeadCmdId=InCmdId;
	(pNotifier->GetData()).mTurnHeadPosition=InCount;//setup desired head position
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sTurnHeadData> > pNotifierRSP;//notifier to handle command response
		do{
			pNotifierRSP=Receive();//wait for notifier to arrive
			if((pNotifierRSP->GetNotifierId())!= RSP_TURN_HEAD) continue;//skip any notifier different to RSP_TURN_HEAD
		}while((pNotifierRSP->GetData()).mTurnHeadCmdId!=InCmdId);//untill we get response for the command
	}	
}//cExeMngr::TurnHead
//END HEAD TURN COMMANDS

//---------------------------------------------------------------------------------------------------------
// ARM COMMANDS
// turn on arm servos to be able to move arms
// HandlingType - synchronous or asynchronous type of command execution
void cExeMngr::ArmServoOnCmd(BYTE HandlingType)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= ARM_SERVO_ON_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//declare notifier to handle command response
		do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while((pNotifierRSP->GetData()).mArmCmdId!=ARM_SERVO_ON_SCMD_ID);//untill we get response for the command
	}
}//cExeMngr::ArmServoOnCmd

// turn off arm servos
// HandlingType - synchronous or asynchronous type of command execution
void cExeMngr::ArmServoOffCmd(BYTE HandlingType)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= ARM_SERVO_OFF_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//declare notifier to handle command response
		do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while((pNotifierRSP->GetData()).mArmCmdId!=ARM_SERVO_OFF_SCMD_ID);//untill we get response for the command
	}
}//cExeMngr::ArmServoOffCmd

//move left arm to specified count position
// HandlingType - synchronous or asynchronous type of command execution
void cExeMngr::MoveLeftArmCmd(BYTE HandlingType,WORD InCount)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= MOVE_LEFT_ARM_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mArmCount=InCount;//setup desired arm position
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//notifier to handle command response
		do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while((pNotifierRSP->GetData()).mArmCmdId!=MOVE_LEFT_ARM_SCMD_ID);//untill we get response for the command
	}
}//cExeMngr::MoveLeftArmCmd
		
//move left arm to the home position (horizontal position)
// HandlingType - synchronous or asynchronous type of command execution
void cExeMngr::MoveLeftArmHomeCmd(BYTE HandlingType)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= MOVE_LEFT_ARM_HOME_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//notifier to handle command response
		do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while((pNotifierRSP->GetData()).mArmCmdId!=MOVE_LEFT_ARM_HOME_SCMD_ID);//untill we get response for the command
	}
}//cExeMngr::MoveLeftArmHomeCmd

//get left arm current position - independent of handling type always executed as SYNC
WORD cExeMngr::GetLeftArmPosCmd()
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= GET_LEFT_ARM_POS_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
	}while((pNotifierRSP->GetData()).mArmCmdId!=GET_LEFT_ARM_POS_SCMD_ID);//untill we get response for the command
return (pNotifierRSP->GetData()).mArmCount;//return received count information
}//cExeMngr::GetLeftArmPosCmd

//move right arm to specified count position
// HandlingType - synchronous or asynchronous type of command execution
// InCount - destination position in counts
void cExeMngr::MoveRightArmCmd(BYTE HandlingType,WORD InCount)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= MOVE_RIGHT_ARM_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mArmCount=InCount;//setup desired arm position
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//notifier to handle command response
		do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while((pNotifierRSP->GetData()).mArmCmdId!=MOVE_RIGHT_ARM_SCMD_ID);//untill we get response for the command
	}
}//cExeMngr::MoveRightArmCmd
		
//move right arm to the home position (horizontal position)
// HandlingType - synchronous or asynchronous type of command execution
void cExeMngr::MoveRightArmHomeCmd(BYTE HandlingType)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= MOVE_RIGHT_ARM_HOME_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//notifier to handle command response
		do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while((pNotifierRSP->GetData()).mArmCmdId!=MOVE_RIGHT_ARM_HOME_SCMD_ID);//untill we get response for the command
	}
}//cExeMngr::MoveRightArmHomeCmd

//get right arm current position - independent of handling type always executed as SYNC
WORD cExeMngr::GetRightArmPosCmd()
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mArmCmdId= GET_RIGHT_ARM_POS_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP;//notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
	}while((pNotifierRSP->GetData()).mArmCmdId!=GET_RIGHT_ARM_POS_SCMD_ID);//untill we get response for the command
return (pNotifierRSP->GetData()).mArmCount;//return received count information
}//cExeMngr::GetRightArmPosCmd

//move both arm synchronously to the specified count position
// HandlingType - synchronous or asynchronous type of command execution
// InCount - destination position in counts (right arm as the base)
void cExeMngr::MoveArmSyncCmd(BYTE HandlingType,WORD InCount)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier1 = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier2 = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	
	(pNotifier1->GetData()).mArmCmdId= MOVE_LEFT_ARM_SYNC_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier1->GetData()).mArmCount=InCount;//setup desired arm position
		
	(pNotifier2->GetData()).mArmCmdId= MOVE_RIGHT_ARM_SYNC_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier2->GetData()).mArmCount=InCount;//setup desired arm position
	
	Post(pNotifier1);//post command	
	Post(pNotifier2);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP1;//notifier to handle command response
		do{
		pNotifierRSP1=Receive();//wait for notifier to arrive
		if((pNotifierRSP1->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while(((pNotifierRSP1->GetData()).mArmCmdId!=MOVE_LEFT_ARM_SYNC_SCMD_ID)&&
				((pNotifierRSP1->GetData()).mArmCmdId!=MOVE_RIGHT_ARM_SYNC_SCMD_ID)
				);//untill we get response for at least one of the the command
		
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP2;//notifier to handle command response
		do{
		pNotifierRSP2=Receive();//wait for notifier to arrive
		if((pNotifierRSP2->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while(((pNotifierRSP2->GetData()).mArmCmdId!=MOVE_LEFT_ARM_SYNC_SCMD_ID)&&
				((pNotifierRSP2->GetData()).mArmCmdId!=MOVE_RIGHT_ARM_SYNC_SCMD_ID)
				);//untill we get response for at least one of the the command
	}
}//cExeMngr::MoveArmSyncCmd

//move both arms oposit to each other at specified position
// HandlingType - synchronous or asynchronous type of command execution
// InCount - destination position of right arm in counts
void cExeMngr::MoveArmOpositCmd(BYTE HandlingType,WORD InCount)
{
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier1 = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifier2 = new cTypeNotifier<sMoveArmData>(CMD_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	
	(pNotifier1->GetData()).mArmCmdId= MOVE_LEFT_ARM_OPPOSITE_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier1->GetData()).mArmCount=InCount;//setup desired arm position
		
	(pNotifier2->GetData()).mArmCmdId= MOVE_RIGHT_ARM_OPPOSITE_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier2->GetData()).mArmCount=InCount;//setup desired arm position
	
	Post(pNotifier1);//post command	
	Post(pNotifier2);//post command	
	
	if (HandlingType==SYNC_HANDLING) //when synchronous call wait for cmd response
	{
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP1;//notifier to handle command response
		do{
		pNotifierRSP1=Receive();//wait for notifier to arrive
		if((pNotifierRSP1->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while(((pNotifierRSP1->GetData()).mArmCmdId!=MOVE_LEFT_ARM_OPPOSITE_SCMD_ID)&&
				((pNotifierRSP1->GetData()).mArmCmdId!=MOVE_RIGHT_ARM_OPPOSITE_SCMD_ID)
				);//untill we get response for at least one of the the command
		
		cSmartPtr<cTypeNotifier<sMoveArmData> > pNotifierRSP2;//notifier to handle command response
		do{
		pNotifierRSP2=Receive();//wait for notifier to arrive
		if((pNotifierRSP2->GetNotifierId())!= RSP_MOVE_ARM) continue;//skip any notifier different to RSP_MOVE_ARM
		}while(((pNotifierRSP2->GetData()).mArmCmdId!=MOVE_LEFT_ARM_OPPOSITE_SCMD_ID)&&
				((pNotifierRSP2->GetData()).mArmCmdId!=MOVE_RIGHT_ARM_OPPOSITE_SCMD_ID)
				);//untill we get response for at least one of the the command
	}
}//cExeMngr::MoveArmOpositCmd

//move random distance back away from an obstacle
BYTE cExeMngr::SyncStepBackOnObstacle()
{
	//move random distance back (from 4 to 6 counts)
	return SyncMoveReverseCmd((SECTOR_LENGTH_COUNTS/6)+(rand()%(SECTOR_LENGTH_COUNTS/8)),MID_PROFILE);
}//cExeMngr::SyncStepBackOnObstacle

//-----------------------------------------------------------------------------------------------------------
//TIME COMMANDS

//inputs:
//		pDateTime - pointer to tm structure where date and time to setup are stored 
//returns:
//		none
void cExeMngr::SyncSetTimeDateCmd(tmElements_t *pDateTime)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = SET_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mYear=(WORD)tmYearToCalendar(pDateTime->Year);//conver tm year to calendar year
	(pNotifier->GetData()).mMonth=pDateTime->Month;
	(pNotifier->GetData()).mDay=pDateTime->Day;
	(pNotifier->GetData()).mDayOfWeek=pDateTime->Wday;
	(pNotifier->GetData()).mHour=pDateTime->Hour;
	(pNotifier->GetData()).mMinute=pDateTime->Minute;
	(pNotifier->GetData()).mSecond=pDateTime->Second;	
	
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=SET_TIME_DATE_SCMD_ID);//untill we get response for the command

}//cExeMngr::SyncSetTimeDateCmd

//inputs:
//		pDateTime - pointer to tm structure where date and time are stored 
//returns:
//		none but tm structure pointed by pDateTime is setup
void cExeMngr::SyncGetTimeDateCmd(tmElements_t *pDateTime)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = GET_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=GET_TIME_DATE_SCMD_ID);//untill we get response for the command
	pDateTime->Year = CalendarYrToTm((pNotifierRSP->GetData()).mYear);//conver to tm year from calendar year
	pDateTime->Month = (pNotifierRSP->GetData()).mMonth;
	pDateTime->Day = (pNotifierRSP->GetData()).mDay;
	pDateTime->Wday =(pNotifierRSP->GetData()).mDayOfWeek;
	pDateTime->Hour =(pNotifierRSP->GetData()).mHour;
	pDateTime->Minute= (pNotifierRSP->GetData()).mMinute;
	pDateTime->Second =(pNotifierRSP->GetData()).mSecond;	
	
}//cExeMngr::SyncGetTimeDateCmd

//inputs:
//		pDateTime - pointer to tm structure where date and time to setup are stored 
//returns:
//		none
void cExeMngr::SyncSetAlarmTimeDateCmd(tmElements_t *pDateTime)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = SET_ALARM_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mYear=(WORD)tmYearToCalendar(pDateTime->Year);//conver tm year to calendar year
	(pNotifier->GetData()).mMonth=pDateTime->Month;
	(pNotifier->GetData()).mDay=pDateTime->Day;
	(pNotifier->GetData()).mDayOfWeek=pDateTime->Wday;
	(pNotifier->GetData()).mHour=pDateTime->Hour;
	(pNotifier->GetData()).mMinute=pDateTime->Minute;
	(pNotifier->GetData()).mSecond=pDateTime->Second;	
	
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=SET_ALARM_TIME_DATE_SCMD_ID);//untill we get response for the command

}//cExeMngr::SyncSetAlarmTimeDateCmd

//inputs:
//		pDateTime - pointer to tm structure where date and time are stored 
//returns:
//		none but tm structure pointed by pDateTime is setup
// IMPORTANT! To receive time and date information this command needs to be executed in SYNC_HANDLING mode
void cExeMngr::SyncGetAlarmTimeDateCmd(tmElements_t *pDateTime)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = GET_ALARM_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=GET_ALARM_TIME_DATE_SCMD_ID);//untill we get response for the command
	pDateTime->Year = CalendarYrToTm((pNotifierRSP->GetData()).mYear);//conver to tm year from calendar year
	pDateTime->Month = (pNotifierRSP->GetData()).mMonth;
	pDateTime->Day = (pNotifierRSP->GetData()).mDay;
	pDateTime->Wday =(pNotifierRSP->GetData()).mDayOfWeek;
	pDateTime->Hour =(pNotifierRSP->GetData()).mHour;
	pDateTime->Minute= (pNotifierRSP->GetData()).mMinute;
	pDateTime->Second =(pNotifierRSP->GetData()).mSecond;	
	
}//cExeMngr::SyncGetAlarmTimeDateCmd

//inputs:
//     none
//returns:
//		none
void cExeMngr::SyncClearAlarmTimeDateCmd()
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = CLR_ALARM_FOR_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=CLR_ALARM_FOR_TIME_DATE_SCMD_ID);//untill we get response for the command
	
}//cExeMngr::SyncClearAlarmTimeDateCmd
//END TIME COMMANDS
//-----------------------------------------------------------------------------------------------------------
//INDICATOR COMMANDS

//turn on or turn of Walles signaling LED
void cExeMngr::SyncCmdLED(BYTE InState)
{
	cSmartPtr<cTypeNotifier<sIndicatorData> > pNotifier = new cTypeNotifier<sIndicatorData>(CMD_INDICATOR,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mIndicatorCmdId=CTR_LED_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mState=InState;

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sIndicatorData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_INDICATOR) continue;//skip any notifier different to RSP_INDICATOR
	}while((pNotifierRSP->GetData()).mIndicatorCmdId!=CTR_LED_SCMD_ID);//untill we get response for the command
}//cExeMngr::SyncCmdLED

//turn on or turn off Walle BUZZER
void cExeMngr::SyncCmdBuzzer(BYTE InState)
{
	cSmartPtr<cTypeNotifier<sIndicatorData> > pNotifier = new cTypeNotifier<sIndicatorData>(CMD_INDICATOR,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mIndicatorCmdId=CTR_BUZZER_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mState=InState;

	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sIndicatorData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_INDICATOR) continue;//skip any notifier different to RSP_INDICATOR
	}while((pNotifierRSP->GetData()).mIndicatorCmdId!=CTR_BUZZER_SCMD_ID);//untill we get response for the command
}//cExeMngr::SyncCmdBuzzer

//turn off Walle
void cExeMngr::SyncCmdTurnOffWalle(WORD PowerOffReason)
{
	cSmartPtr<cTypeNotifier<sIndicatorData> > pNotifier = new cTypeNotifier<sIndicatorData>(CMD_INDICATOR,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mIndicatorCmdId=CTR_MAIN_POWER_SCMD_ID;//setup sub-command requested to be executed
	(pNotifier->GetData()).mState=PowerOffReason;//request main power off because of the certain reason

	Post(pNotifier);//post command	
	
	//I decided to add below to avoid command is exit while turn off message is not executed
	//in this way Walle will be turned off when we are waiting for RSP_INDICATOR to be received
	cSmartPtr<cTypeNotifier<sIndicatorData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_INDICATOR) continue;//skip any notifier different to RSP_INDICATOR
	}while((pNotifierRSP->GetData()).mIndicatorCmdId!=CTR_MAIN_POWER_SCMD_ID);//untill we get response for the command
}//cExeMngr::SyncCmdTurnOffWalle

//END INDICATOR COMMANDS

//send display request with critical information
void cExeMngr::EvtDisplayInfo(char *pText)
{
	//create EVT_DISPLAY_INFO notifier to request critical information to be displayed on LCD
	cSmartPtr<cTypeNotifier<sDspInfoEvt> > pNotifier = new cTypeNotifier<sDspInfoEvt>(EVT_DISPLAY_INFO,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mId=EVT_DISPLAY_CRITICAL_ID;//setup sub-command requested to be executed
	strncpy((pNotifier->GetData()).mText,pText,EVT_DISPLAY_INFO_TEXT_SIZE);//copy critical information text to notifier
	
	Post(pNotifier);//post command	
	
}//cExeMngr::EvtDisplayInfo

//-----------------------------------------------------------------------------------------------------------
// STATES
//scan 360 degs and position at light source 
//inputs:
//      none
//returns:
//		next state of the state machine
BYTE cExeMngr::StateFindLightSource()
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_1","\nTRC: ExeMngr: StateFindLightSource");
#endif		
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	mTime=pCtxMngr->GetNowTime();//get current time
	BreakTime(mTime, &mDateTime);//create date time structure from mTime
	if((mDateTime.Hour >ALARM_END_HOUR) && (mDateTime.Hour < ALARM_START_HOUR))//if not a time for BATH program do not execute it
		return STATE_SETUP_SLEEP;
	
	Error=SyncFindMaxLightSourceCmd();//find and position towards a light source
	
	switch(Error)
	{
	case MOVE_OK: //movement executed Wall-e positioned towards light
		break;
	case MOVE_BREAK_LEFT: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_ANGLE_LEFT: //Error - destination angle not achieved when turning somehow stopped by an obstacle
	case MOVE_BREAK_RIGHT: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_ANGLE_RIGHT: //Error - destination angle not achieved when turning somehow stopped by an obstacle
		SyncStepBackOnObstacle();
		MoveAttempts=MoveAttempts+1;//there is move eror count number of trials to resolve it
		return STATE_PASS_OBSTACLE_ANYHOW;
		break;
	case MOVE_NONE_LIGHT_SRC:// - none meaningful light source was identified
		return STATE_SETUP_SLEEP;
		break;
	default: //unknown result should not happen
		LCDDebugMessage("ERR: FndLgtSrc: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: StateFindLightSource: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}
	//Wall-e is corectly positioned towards light source
	//check if at his back there is DARK
	if(SyncCheckBackLight()!=DARK_DAY)
		return STATE_SETUP_SLEEP;//if there is not dark do not move go to sleep to wait for darknes around 
	PreviousFrontLightLevel=SyncReadFrontLight();//register front light level for future comparision
	return STATE_MOVE_TOWARDS_LIGHT;
}//cExeMngr::StateFindLightSource


//inform about weak health (for example low bat) for a while and turn off
BYTE cExeMngr::StateInformHelthState()
{

#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_2","\nTRC: ExeMngr: StateInformHealthState");
#endif	
	SyncClearAlarmTimeDateCmd();//turn off any alarms previously setup
	
	EvtDisplayInfo(EXE_MNG_CRITICAL_INFO_STR);//request critical info text to be displayed
	
	Kernel.TimeDlyHMSM(0,0,10,0);//wait for couple of seconds to make information visable
	PowerOffResons=HEALTH_STATE_TURN_OFF;//setup weak HEALTH (like low battery) as the reason for power off
	return STATE_TURN_OFF;
}//cExeMngr::StateInformHelthState

//stay sleep for a while whaiting for external condition change
BYTE cExeMngr::StateSetupSleep()
{

#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_3","\nTRC: ExeMngr: StateSetupSleep");
#endif	
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
		
	mTime=pCtxMngr->GetNowTime();//get current time
	BreakTime(mTime, &mDateTime);//create date time structure from mTime
	if((mDateTime.Hour >ALARM_END_HOUR) && (mDateTime.Hour < ALARM_START_HOUR))//if not a time for BATH program do not execute it
	{//but setup alarm for this day correct hour for bath program
		mTime=previousMidnight(mTime);//get time correcponding to current day start
		mTime+=SECS_PER_HOUR*ALARM_START_HOUR;//add seconds to ALARM_START_HOUR hour
		BreakTime(mTime,&mDateTime);//setup Date Time based on just calculated time of next alarm

	} else //that is time to execute bath program mean in that case WALLE_SLEEP_MINUTES sleep
	{
		//put Walle to sleep for WALLE_SLEEP_MINUTES 
		mTime=MakeTime(&mDateTime);//conver time to seconds since Jan 1 1970
		mTime+=MinutesToTime_t(WALLE_SLEEP_MINUTES);//setup WALLE_SLEEP_TIME
		BreakTime(mTime, &mDateTime);//create date time structure for alarm after sleep
	}

#if DEBUG_MNG_EXE	
	DbgTraceStrVal(2,"ExeMngr_3","Year:   ",mDateTime.Year);
	DbgTraceStrVal(2,"ExeMngr_3","Month:  ",mDateTime.Month);
	DbgTraceStrVal(2,"ExeMngr_3","Wday:   ",mDateTime.Wday);
	DbgTraceStrVal(2,"ExeMngr_3","Day:    ",mDateTime.Day);
	DbgTraceStrVal(2,"ExeMngr_3","Hour:   ",mDateTime.Hour);
	DbgTraceStrVal(2,"ExeMngr_3","Minute: ",mDateTime.Minute);
	DbgTraceStrVal(2,"ExeMngr_3","Second: ",mDateTime.Second);
#endif	
	SyncSetAlarmTimeDateCmd(&mDateTime);//and setup new alarm for next day
	Kernel.TimeDlyHMSM(0,0,10,0);//give some dalay to eventually switch to voice command
	PowerOffResons=ALARM_TURN_OFF;//setup ALARM as the reason for power off
	return STATE_TURN_OFF;
}//cExeMngr::StateSleep

//find any way to pass an obstacle
BYTE cExeMngr::StatePassObstackleAnyhow()
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_4","\nTRC: ExeMngr: StatePassObstackleAnyhow");
#endif
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	//select avaliable movements paths based on previous state movement error
	switch(Error)
	{
	case MOVE_BREAK_OBSTACLE: //cannot complete movement because of obstacle
		Error=SyncMoveOnPossiblePathCmd(3*SECTOR_LENGTH_COUNTS,BIT0|BIT1|BIT2|BIT3|BIT4|BIT6|BIT7|BIT8|BIT9|BIT10);
		break;
	case MOVE_BREAK_FORWARD: //movement cannot be realized none chnge in pulses, wall-e stoped
		Error=SyncMoveOnPossiblePathCmd(3*SECTOR_LENGTH_COUNTS,BIT3|BIT4|BIT7|BIT8|BIT9|BIT10);
		break;
	case MOVE_BREAK_RIGHT: //there is an obstacle on the right side
	case MOVE_ANGLE_RIGHT:
		//make an attempt to move max 3 sectors from current place using paths on left side only
		Error=SyncMoveOnPossiblePathCmd(3*SECTOR_LENGTH_COUNTS,BIT4|BIT8|BIT10);
		break;
	case MOVE_BREAK_LEFT: //there is an obstacle on the left side
	case MOVE_ANGLE_LEFT:
		//make an attempt to move max 3 sectors from current place using paths on right side only
		Error=SyncMoveOnPossiblePathCmd(3*SECTOR_LENGTH_COUNTS,BIT3|BIT7|BIT9);
		break;
	default://strange not supported error
		LCDDebugMessage("ERR: PasObstAny: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: PassObstackleAnyhow: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}//switch

	switch(Error)
	{
	case MOVE_OK: //movement executed go to state to find light source
		MoveAttempts=0;//we manage to pass an obstacle so clear attempts counter
		break;
	case MOVE_NONE_PATH: //cannot complete movement because of none path can be found
	case MOVE_BREAK_OBSTACLE: //cannot complete movement because of obstacle	
	case MOVE_BREAK_FORWARD: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_ANGLE_LEFT: //Error - destination angle not achieved when turning somehow stopped by an obstacle
	case MOVE_BREAK_LEFT: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_ANGLE_RIGHT: //Error - destination angle not achieved when turning somehow stopped by an obstacle
	case MOVE_BREAK_RIGHT: //movement cannot be realized none chnge in pulses, wall-e stoped
		SyncStepBackOnObstacle();
		if (MoveAttempts <= MAX_MOVE_ATTEMPT) //we cannot pass obstacle but number of attempts not exhausted 
			return STATE_FIND_LIGHT_SOURCE; //make attempt to resolve it
		else
			return STATE_SETUP_SLEEP;//cannot resolve movement error after number of attempts - go to sleep
		break;
	default: //unknown result should not happen
		LCDDebugMessage("ERR: PaObsAny2nd: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: PassObstackleAnyhow2nd: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}
	return STATE_FIND_LIGHT_SOURCE;
}//cExeMngr::StatePassObstackleAnyhow

//turn off Walle
BYTE cExeMngr::StateTurnOff()
{

#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_5","\nTRC: ExeMngr: StateTurnOff");
#endif
	
	SyncCmdTurnOffWalle(PowerOffResons);//request Walle power off
	return STATE_TURN_OFF;
}//cExeMngr::StateTurnOff

//move forward random distance towards light source
BYTE cExeMngr::StateMoveTowardsLight()
{
	WORD DistanceToMove;//calculated distance to be covered by movement
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_6","\nTRC: ExeMngr: StateMoveTowardsLight");
#endif
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	
	DistanceToMove = SECTOR_LENGTH_COUNTS + (rand()%MAX_FORWARD_ONCE_MOVE_DISTANCE);

	Error=SyncMoveForwardCmd(DistanceToMove,LOW_MID_HIGH_PROFILE);

	switch (Error)
	{
	case MOVE_OK:
		break;
	case MOVE_BREAK_FORWARD: //looks like an obstacle on a way forward
	case MOVE_BREAK_OBSTACLE:
		SyncStepBackOnObstacle();
		return STATE_PASS_OBSTACLES_FORWARD;//try to pass obstacle but still keeping track forward
		break;
	default: //unknown result should not happen
		LCDDebugMessage("ERR: MveToLgt: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: StateMoveTowardsLight: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}//switch (Error)
	return STATE_POSITION_AT_MAX_LIGHT;
}//cExeMngr::StateMoveTowardsLight

//being on a way towards light correct your position to the light source
BYTE cExeMngr::StatePositionAtMaxLight()
{
	WORD FrontLightStrength; //stores light readings
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_7","\nTRC: ExeMngr: StatePositionAtMaxLight");
#endif
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	Error=SyncPositionMaxLightScrFrd();
	switch(Error)
	{
	case MOVE_OK:
		break;
	case MOVE_BREAK_LEFT:
	case MOVE_ANGLE_LEFT:
	case MOVE_BREAK_RIGHT:
	case MOVE_ANGLE_RIGHT:
		SyncStepBackOnObstacle();
		return STATE_PASS_OBSTACLE_ANYHOW;
	case MOVE_NONE_LIGHT_SRC:
		return STATE_FIND_LIGHT_SOURCE;//cannot detect light source forward need to restart light searching
		break;
	default: //unknown result should not happen
		LCDDebugMessage("ERR: PosAtMaxLgt: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: StatePositionAtMaxLight: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}//switch(Error)
	FrontLightStrength=SyncReadFrontLight();
	if(FrontLightStrength<PreviousFrontLightLevel)//when we move forwards towards light source but light decreases
	{
		return STATE_FIND_LIGHT_SOURCE; //that is something wrong and we need to restart light searching
	}
	else
	{
		PreviousFrontLightLevel=FrontLightStrength;//store current position light strength
		return STATE_MOVE_TOWARDS_LIGHT;//light is stronger as we move so continue
	}
}//cExeMngr::StatePositionAtMaxLight

//pass an obstacle but only selecting forward paths
BYTE cExeMngr::StatePassObstacleForward()
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_8","\nTRC: ExeMngr: StatePassObstacleForward");
#endif	
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	PreviousFrontLightLevel=SyncReadFrontLight();//register front light level for future comparision
	
	//select avaliable movements paths based on previous state movement error
	switch(Error)
	{
	case MOVE_BREAK_FORWARD: //looks like an obstacle on a way forward so select left or right side only for further movement
		Error=SyncMoveOnPossiblePathCmd(3*SECTOR_LENGTH_COUNTS,BIT3|BIT4);
		break;
	case MOVE_BREAK_OBSTACLE: //scan ahead and move on avaliable path
		Error=SyncMoveOnPossiblePathCmd(3*SECTOR_LENGTH_COUNTS,BIT0|BIT1|BIT2|BIT3|BIT4|BIT6|BIT7|BIT8);
		break;
	default:
		LCDDebugMessage("ERR: PasObstFrd: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: ErrPassObstackleFrw: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}//switch
	
	switch(Error)
	{
	case MOVE_OK: //that was possible to pass obstacle and move forward
		return STATE_POSITION_AT_MAX_LIGHT; 
		break;
	case MOVE_NONE_PATH: //cannot complete movement because of none path can be found we are at light wall
		break;
	case MOVE_BREAK_OBSTACLE: //cannot complete movement because of obstacle
	case MOVE_BREAK_FORWARD: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_BREAK_LEFT: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_ANGLE_LEFT: //Error - destination angle not achieved when turning somehow stopped by an obstacle
	case MOVE_BREAK_RIGHT: //movement cannot be realized none chnge in pulses, wall-e stoped
	case MOVE_ANGLE_RIGHT: //Error - destination angle not achieved when turning somehow stopped by an obstacle
		SyncStepBackOnObstacle();
		return STATE_PASS_OBSTACLE_ANYHOW;//if we were stopped passing obstacle forward we need to try to find other way
		break;
	default: //unknown result should not happen
		LCDDebugMessage("ERR: PasObstFwd2: ",Error,20,1,1,0);
		Uart0Message("ERR: Exe: StatePassObstacleForward2nd: ",Error);
		UNEXPECTED_ERROR_VALUE;
		break;
	}
	return STATE_CHECK_FOR_DESTINATION;//looks we are at light wall so check for destination
}//cExeMngr::StatePassObstacleForward

//assuming you rach light source stay signaling for a while
BYTE cExeMngr::StateSignalAtLightSource()
{

#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_9","\nTRC: ExeMngr: StateSignalAtLightSource");
#endif	
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	for(int i=1;i<NUMBER_OF_SIGNALING_LOOPS;i++)
	{
		SyncCmdBuzzer(CTR_STATE_ON);
		Kernel.TimeDlyHMSM(0,0,0,500);
		SyncCmdBuzzer(CTR_STATE_OFF);
		Kernel.TimeDlyHMSM(0,0,0,500);
		if(SyncCheckFrontLight()==DARK_DAY)//light at which we are signalling is just turned off
			return STATE_MOVE_ASIDE;//prepare for next day wakeup
	}//for
	//once we finished signalling lets setup for next day alarm
	return STATE_MOVE_ASIDE; //signaling done prepare for next day check
}//cExeMngr::StateSignalAtLightSource

//check if final light source destination received
//that is when there is not any move forward and when any move aside generates lower light strength
BYTE cExeMngr::StateCheckForDestination()
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_10","\nTRC: ExeMngr: StateCheckForDestination");
#endif	
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	//identify that direction which has stronger light aside of Wall-e (to be checked)
	if(StepDetermineTurnDirection()==TURN_LEFT_DIRECTION)//if that is left direction, make left side check
	{
		return StepMoveLeftCheck();//make check and return next state machine state to be executed depending on check result
	}else //when right side of Wall-e has stronger light check that side
	{
		return StepMoveRightCheck();//make check and return next state machine state to be executed depending on check result
	}
}//cExeMngr::StateCheckForDestination

//setup alarm for next day
BYTE cExeMngr::StateSetupNextDayAlarm()
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_11","\nTRC: ExeMngr: StateSetupNextDayAlarm");
#endif
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	mTime=pCtxMngr->GetNowTime();
	mTime=MakeTime(&mDateTime);//convert to seconds since 1 Jan 1970
	
	if(mDateTime.Hour>=ALARM_START_HOUR) //if just before current day midnight
		mTime=nextMidnight(mTime);//get time correcponding to current day end
	if(mDateTime.Hour<ALARM_START_HOUR) //if just after current day midnight
		mTime=previousMidnight(mTime);//get time correcponding to current day start
	
	mTime+=SECS_PER_HOUR*ALARM_START_HOUR;//add seconds to ALARM_START_HOUR hour
	BreakTime(mTime,&mDateTime);//setup Date Time based on just calculated time of next alarm
#if DEBUG_MNG_EXE	
	DbgTraceStrVal(2,"ExeMngr_11","Year:   ",mDateTime.Year);
	DbgTraceStrVal(2,"ExeMngr_11","Month:  ",mDateTime.Month);
	DbgTraceStrVal(2,"ExeMngr_11","Wday:   ",mDateTime.Wday);
	DbgTraceStrVal(2,"ExeMngr_11","Day:    ",mDateTime.Day);
	DbgTraceStrVal(2,"ExeMngr_11","Hour:   ",mDateTime.Hour);
	DbgTraceStrVal(2,"ExeMngr_11","Minute: ",mDateTime.Minute);
	DbgTraceStrVal(2,"ExeMngr_11","Second: ",mDateTime.Second);
#endif	
	SyncSetAlarmTimeDateCmd(&mDateTime);//and setup new alarm for next day
	PowerOffResons=ALARM_TURN_OFF;//setup ALARM as the reason for power off
	return STATE_TURN_OFF;
}//cExeMngr::StateSetupNextDayAlarm

//move from the detected bath aside
BYTE cExeMngr::StateMoveAside()
{
	WORD DistanceToMove;
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_12","\nTRC: ExeMngr: StateMoveAside");
#endif
	
	//when cannot execute because of low batteries
	if(!(pCtxMngr->IsHealthOK()))
		return STATE_INFORM_HEALTH_STATE;
	
	if(rand()%2)//randomly select direction and turn around from light source
		{//and if right make turns right 180 degs
			if(SyncTurnRight90DegCmd())//turn if error break to next state
				return STATE_SETUP_NEXT_DAY_ALARM;
			if(SyncTurnRight90DegCmd())//turn if error break to next state
				return STATE_SETUP_NEXT_DAY_ALARM;
		}else // otherwise left turn 180 degs
		{
			if(SyncTurnLeft90DegCmd())//turn if error break to next state
				return STATE_SETUP_NEXT_DAY_ALARM;
			if(SyncTurnLeft90DegCmd())//turn if error break to next state
				return STATE_SETUP_NEXT_DAY_ALARM;
		}
	
	//move aside opposit to the light to be aside from possible opened doors
	DistanceToMove = 3*SECTOR_LENGTH_COUNTS + (rand()%MAX_FORWARD_ONCE_MOVE_DISTANCE);
	SyncMoveForwardCmd(DistanceToMove,LOW_MID_HIGH_PROFILE);//ignor any errors

	return STATE_SETUP_NEXT_DAY_ALARM;
}//cExeMngr::StateMoveAside

//execute Wall-e program to find Barbara in the bath
void cExeMngr::WalleProgramBath(void)
{
	BYTE State;//to store current state of the movement algorithm
	State=STATE_FIND_LIGHT_SOURCE;//when Walle Bath program is on start with light source identification
	
	Uart0PutStr("\nLOG: Exe: Walle Program Bath\n");

#if DEBUG_MNG_EXE
	DbgTraceStr(1,"ExeMngr_12","\nTRC: --- Bath Program State Machine Run ---\n");
#endif	
	for(;;)
	{
#if DEBUG_MNG_EXE		
	DbgTraceStrVal(1,"ExeMngr_12","\nTRC: ExeMngr State: ",State);
#endif		
	switch (State)
	{
	case STATE_FIND_LIGHT_SOURCE:
		State=StateFindLightSource();
		break;
	case STATE_INFORM_HEALTH_STATE:
		State=StateInformHelthState();
		break;
	case STATE_SETUP_SLEEP:
		State=StateSetupSleep();
		break;
	case STATE_PASS_OBSTACLE_ANYHOW:
		State=StatePassObstackleAnyhow();
		break;
	case STATE_TURN_OFF:
		State=StateTurnOff();
		break;
	case STATE_MOVE_TOWARDS_LIGHT:
		State=StateMoveTowardsLight();
		break;
	case STATE_POSITION_AT_MAX_LIGHT:
		State=StatePositionAtMaxLight();
		break;
	case STATE_SIGNAL_AT_LIGHT_SOURCE:
		State=StateSignalAtLightSource();
		break;
	case STATE_PASS_OBSTACLES_FORWARD:
		State=StatePassObstacleForward();
		break;
	case STATE_CHECK_FOR_DESTINATION:
		State=StateCheckForDestination();
		break;
	case STATE_SETUP_NEXT_DAY_ALARM:
		State=StateSetupNextDayAlarm();//setup alarm for next day
		break;
	case STATE_MOVE_ASIDE:
		State=StateMoveAside();
		break;
	default://unknown state that should not happen
		LCDDebugMessage("ERR: Wrng Sts: ",State,20,1,1,0);
		Uart0Message("ERR: Exe: Wrong state: ",State);
		NOT_ALLOWED_STATE;//generate an exception
		break;
	}//switch (State)
	//make 1 tick delay to allow for a context switch and to not exhaust uP by this task
	//that is for the case when there is not any delay inside a state
#if DEBUG_MNG_EXE
	DbgStopStrVal(1,"ExeMngr_12","\nTRC: Stopped before next state: ",State);
#endif
	Delay(1);
//	Kernel.TimeDlyHMSM(0,0,2,0);//After testing leave: Delay(1)		
	}//for(;;)
}//cExeMngr::WalleProgramBath

//starts from pIn to identify begining of the token string ignoring leading spaces if any
//returns pointer to token or null if not found (because of end of the processed string)
const char *cExeMngr::FindTokenStart(const char *pIn)
{
	while (*pIn) //as long as there is not a string end
	{
		if (*pIn!=' ')return pIn;//
		pIn++;
	}
	return NULL; //none parameter character found in the string (only spaces or \0)
}//cExeMngr::FindTokenStart

//find and copy token string to buffer which is next null terminated
//returns pointer to the first character after the token or null if no token found
const char *cExeMngr::ExtractTokenToBuffer(const char *pTokenStart, char *pTokenBuffer, int TokenBufferSize)
{
	*pTokenBuffer=0;//make token buffer empty at start
		
	pTokenStart=FindTokenStart(pTokenStart);//eliminate input space if any and point to token beginning
	if(pTokenStart)//if there is a token string
	{//copy characters as long as there is no space or end of the string or buffer size not excided
	while ((*pTokenStart!=' ') && (*pTokenStart!=0) && (TokenBufferSize > 1))
		{
		*pTokenBuffer=*pTokenStart;//copy character to the buffer
		pTokenStart++;//move to the next char
		pTokenBuffer++;//move to the next storage place
		TokenBufferSize--;
		}
	}//if
	*pTokenBuffer=0;//terminate extracted token string
	return pTokenStart; 
	
}//cExeMngr::ExtractTokenToBuffer

//finds Seuence Line Number coresponding to the label in the sequence table InSeqCmdTbl
//returns sequence line number or NO_LINE_NUMBER_FOR_LABEL
WORD cExeMngr::FindLineNoForLabel(const char *InSeqCmdTbl[], char *InLabel)
{
	const char *pSeqPos;//pointer to position in analysed sequence line
	WORD SeqLn=0;//start from sequence beginning
	
	for(;;)//check every line of the sequence to find corrsponding label if any
	{
		pSeqPos=ExtractTokenToBuffer(InSeqCmdTbl[SeqLn],TokenBuffer,sizeof(TokenBuffer));
		if(pSeqPos)//if there is not an empty line
		{
#if DEBUG_MNG_EXE
			DbgTraceStrVal(2,"ExeMngr_13","\nTRC: ExeMngr: FindLine LineNo: ",SeqLn);
			DbgTraceStr(2,"ExeMngr_13","\nTRC: ExeMngr: FindLine Line: ");
			DbgTraceStr(2,"ExeMngr_13",const_cast<char*>(pSeqPos));
			DbgTraceStr(2,"ExeMngr_13","\n");
#endif			
			if (!strcmp(TokenBuffer,SEQ_EXE_CODE_LABEL_STR))//if that is a label line
			{
				//extract label
				pSeqPos=ExtractTokenToBuffer(pSeqPos,TokenBuffer,sizeof(TokenBuffer));
				if(pSeqPos)//if label string extracted
				{
					if (!strcmp(TokenBuffer,InLabel))//if label found return it's line number
					{
#if DEBUG_MNG_EXE
						DbgTraceStr(2,"ExeMngr_13","\nTRC: ExeMngr: FindLine Line: Label found\n");
#endif						
						return SeqLn;
					}//if label found
				}//if label string extracted
			}//if label line found
			else //other not a label line, check if not end of sequence
			{
				//extract command
				pSeqPos=ExtractTokenToBuffer(pSeqPos,TokenBuffer,sizeof(TokenBuffer));
				if(pSeqPos)//if command string extracted
				{	
					if (!strcmp(TokenBuffer,SEQ_ACT_END_STR))//if sequence END
					{
#if DEBUG_MNG_EXE
						DbgTraceStr(2,"ExeMngr_13","\nTRC: ExeMngr: FindLine Line: Label not found\n");
#endif	
						return NO_LINE_NUMBER_FOR_LABEL;						
					}//if sequnce end
				}//if command string extracted
			}//other line
		}//no empty line
		SeqLn++;//get next line from sequence to analyse for label presence
		if(SeqLn==NO_LINE_NUMBER_FOR_LABEL)//protection in case of sequence not correctly ended
			return NO_LINE_NUMBER_FOR_LABEL;
	}//for
}//cExeMngr::FindTokenStart

//parse command sequence line
//return ID of the action to be executed and also setup execution code ExeCode
//which can be synchronous or asynchronous command call
//modifies internally member variable pCurPosInSeq and ExeCode 
BYTE cExeMngr::ParseSeq()
{
	ExeCode=SEQ_EXE_CODE_NO;//assume none execution code at the beginning
	//first extract execution code and move pointer to next token
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq) //if no token ifdentified there is no exe code specified like an empty line etc.
		return SEQ_ACT_NO_EXE_CODE;//so return anknown command because exe code not specified
	
	if (!strcmp(TokenBuffer,SEQ_EXE_CODE_LABEL_STR)) 
	{
		ExeCode=SEQ_EXE_CODE_LABEL;//in this case exe code is not used but it is setup here for consistency
		return SEQ_ACT_NO_EXE_CODE;//this is line with label so no execution code behind
	}
	
	if (!strcmp(TokenBuffer,SEQ_EXE_CODE_ASYNC_STR)) ExeCode=SEQ_EXE_CODE_ASYNC;
	
	if (!strcmp(TokenBuffer,SEQ_EXE_CODE_SYNC_STR)) ExeCode=SEQ_EXE_CODE_SYNC;
	
	//next extract action string if any
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));

	if(!pCurPosInSeq)//if there is not action string
		return SEQ_ACT_NO_EXE_CODE;//no action string specified in the sequence
	
	//check action string to identify command ID behind
	if (!strcmp(TokenBuffer,SEQ_ACT_DELAY_STR)) return SEQ_ACT_DELAY;
	if (!strcmp(TokenBuffer,SEQ_ACT_HALT_STR)) return SEQ_ACT_HALT;
	if (!strcmp(TokenBuffer,SEQ_ACT_RPT_STR)) return SEQ_ACT_RPT;
	if (!strcmp(TokenBuffer,SEQ_ACT_END_STR)) return SEQ_ACT_END;

	if (!strcmp(TokenBuffer,SEQ_ACT_MOVE_FRD_STR)) return SEQ_ACT_MOVE_FRD;
	if (!strcmp(TokenBuffer,SEQ_ACT_MOVE_BWR_STR)) return SEQ_ACT_MOVE_BWR;
	if (!strcmp(TokenBuffer,SEQ_ACT_TURN_LEFT_STR )) return SEQ_ACT_TURN_LEFT;
	if (!strcmp(TokenBuffer,SEQ_ACT_TURN_RIGHT_STR )) return SEQ_ACT_TURN_RIGHT;
	if (!strcmp(TokenBuffer,SEQ_ACT_TURN_AROUND_STR )) return SEQ_ACT_TURN_AROUND;
	if (!strcmp(TokenBuffer,SEQ_ACT_TURN_HEAD_STR )) return SEQ_ACT_TURN_HEAD;
	if (!strcmp(TokenBuffer,SEQ_ACT_LEFT_ARM_STR )) return SEQ_ACT_LEFT_ARM;
	if (!strcmp(TokenBuffer,SEQ_ACT_RIGHT_ARM_STR )) return SEQ_ACT_RIGHT_ARM;
	if (!strcmp(TokenBuffer,SEQ_ACT_ARMS_SYNC_MOVE_STR )) return SEQ_ACT_ARMS_SYNC_MOVE;
	if (!strcmp(TokenBuffer,SEQ_ACT_ARMS_OPOSIT_MOVE_STR )) return SEQ_ACT_ARMS_OPOSIT_MOVE;
	if (!strcmp(TokenBuffer,SEQ_ACT_ARMS_ON_STR )) return SEQ_ACT_ARMS_ON;
	if (!strcmp(TokenBuffer,SEQ_ACT_ARMS_OFF_STR )) return SEQ_ACT_ARMS_OFF;

	return SEQ_ACT_NO_EXE_CODE;//if TokenBuffer does not match to any sequence action return unknown action ID	
}//cExeMngr::ParseSeq

//function to parse and execute commands from the command sequance table
//InSeqCmdTbl - table with sequencies of commands which are parsed and executed
void cExeMngr::ExeCmdSeq(const char *InSeqCmdTbl[])
{
	ExeCode=SEQ_EXE_CODE_NO;	
	SeqLnCntr=0;//start from sequence beginning
	RptNo=0;//initialize to 0 number of repetition requested by parsed RPT command at sequence parsing start
	CurrentRprNo=0;//intialize current number of repetitions so far made to 0 at parsing start
	//process command sequence line by sequence line, 
	//can only be terminated by END command
	for(;;)
	{
		pCurPosInSeq=InSeqCmdTbl[SeqLnCntr];
#if DEBUG_MNG_EXE
		DbgTraceStrVal(2,"ExeMngr_14","\nTRC: ExeMngr: ExeCmdSeq SEQ: ",SeqLnCntr);
		DbgTraceStr(2,"ExeMngr_14","\nTRC: ExeMngr: ExeCmdSeq line: ");
		DbgTraceStr(2,"ExeMngr_14",const_cast<char*>(pCurPosInSeq));
		DbgTraceStr(2,"ExeMngr_14","\n");
#endif			

		switch (ParseSeq())//parse string to find execution code and action
		{
		case SEQ_ACT_DELAY://execute delay command
			SeqActDelay();
			break;
		case SEQ_ACT_HALT://halt sequence execution for infinite time
			SeqActHalt();
			break;
		case SEQ_ACT_RPT://repeat by jumping to the label
			SeqActRpt(InSeqCmdTbl);
			break;
		case SEQ_ACT_MOVE_FRD://when move forward requested execute it
			SeqActMoveFrd();
			break;
		case SEQ_ACT_MOVE_BWR://when move backward requested execute it
			SeqActMoveBwr();
			break;
		case SEQ_ACT_TURN_LEFT://turn left 90 deg
			SeqActTurnLeft();
			break;
		case SEQ_ACT_TURN_RIGHT://turn right 90 deg
			SeqActTurnRight();
			break;
		case SEQ_ACT_TURN_AROUND://turn 360 deg direction selected rundomly
			SeqActTurnAround();
			break;
		case SEQ_ACT_TURN_HEAD://turn head to sepcified position
			SeqActTurnHead();
			break;
		case SEQ_ACT_LEFT_ARM://trun left arm to specified position
			SeqActLeftArm();
			break;
		case SEQ_ACT_RIGHT_ARM://turn right arm to specified position
			SeqActRightArm();
			break;
		case SEQ_ACT_ARMS_SYNC_MOVE://turn both arms synchronously to specified position
			SeqActArmsSyncMove();
			break;
		case SEQ_ACT_ARMS_OPOSIT_MOVE://move both arm but oposit direction (one up other down)
			SeqActArmsOpositMove();
			break;
		case SEQ_ACT_ARMS_ON://turn on arms servo
			SeqActArmsOn();
			break;
		case SEQ_ACT_ARMS_OFF://turn off arms servo
			SeqActArmsOff();
			break;			
		case SEQ_ACT_NO_EXE_CODE://line without execution request for example empty line or label etc.
#if DEBUG_MNG_EXE
			DbgTraceStr(2,"ExeMngr_14","\nTRC: ExeMngr: ExeCmdSeq: Line without execution request");
#endif
			break;
		case SEQ_ACT_END: //terminate execution when sequence end
#if DEBUG_MNG_EXE
			DbgTraceStr(2,"ExeMngr_14","\nTRC: ExeMngr: ExeCmdSeq: End of sequence");
#endif
			return;//break execution of sequencies
		default://unknown command
			Uart0PutStr(SEQ_ACTION_UNKNOWN_MSG);//print on terminal information about unknown command
			break;
		}//switch (ParseSeq)
		SeqLnCntr++;//get next sequence
	}//for(;;)
}//cExeMngr::ExeCmdSeq

//execute delay sequence command
void cExeMngr::SeqActDelay(void)
{
	DWORD TimeDelay;//delay in miliseconds to execute
	
	//next extract delay value
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not delay specified do nothing
		return;
	TimeDelay=atol(TokenBuffer);
	TimeDelay=(TimeDelay*OS_TICKS_PER_SEC)/1000;//convert time in miliseconds to number of OS Ticks
#if DEBUG_MNG_EXE
	DbgTraceStrVal(2,"ExeMngr_15","\nTRC: ExeMngr: SeqActDelay: End of sequence Delay[ticks]: ",TimeDelay);
#endif	

	Delay((WORD)TimeDelay);
}//cExeMngr::SeqActDelay

//halt sequence execution for infinite time
void cExeMngr::SeqActHalt(void)
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_16","\nTRC: ExeMngr: SeqActHalt");
#endif
	for(;;)//infinie halt but with delays to release control to others
		Delay(SEQ_HALT_DELAY_TICKS);
}//cExeMngr::SeqActHalt(void)

//repeate sequencies by jumping to specified label optionally specified number of times
void cExeMngr::SeqActRpt(const char *InSeqCmdTbl[])
{
	WORD SeqLnNo;//keeps number of the line to jump into
	WORD RptNoInCmd;//stores number of repetitions as specified by command
	
	//extract label
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not label specified do nothing
		return;
	strcpy(LabelBuffer, TokenBuffer);//copy label to storage place
		
	//extract number of repetition
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not repetition specified
		RptNoInCmd=0;//default is no repetitions means pure infinite number jump
	else
		RptNoInCmd=atoi(TokenBuffer);
	
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_17","\nTRC: ExeMngr: SeqActRpt");
	DbgTraceStr(2,"ExeMngr_17","\nTRC: ExeMngr: SeqActRpt: Label: ");
	DbgTraceStr(2,"ExeMngr_17",LabelBuffer);
	DbgTraceStr(2,"ExeMngr_17","\n");
	DbgTraceStrVal(2,"ExeMngr_17","\nTRC: ExeMngr: SeqActRpt RPT Number of repetitions: ",RptNoInCmd);
	DbgTraceStrVal(2,"ExeMngr_17","\nTRC: ExeMngr: SeqActRpt RPT Number of repetitions to do: ",CurrentRprNo);
#endif	
	if(RptNoInCmd==0)//that is pure jump request without repetitions number at all
	{
		SeqLnNo=FindLineNoForLabel(InSeqCmdTbl,LabelBuffer);//find line number for label
		if(SeqLnNo==NO_LINE_NUMBER_FOR_LABEL)//if there is not specified label in sequence
		{
			return;//finish this command doing nothing
		}
		else//label identified jump to it
		{
			SeqLnCntr=SeqLnNo;//jump to the line with label repeating sequence of the code
			return;
		}
	}//if jump request
	
	if(RptNo==0 && CurrentRprNo==0)//there is RPT with number of repetition specified but not yet executed
	{//initialize repetition loop
		RptNo=RptNoInCmd;
		CurrentRprNo=RptNo;
	}
	CurrentRprNo-=1;//decrement repetition number
	if(CurrentRprNo)//if there is still anything to repeat
	{
		SeqLnNo=FindLineNoForLabel(InSeqCmdTbl,LabelBuffer);//find line number for label
		if(SeqLnNo==NO_LINE_NUMBER_FOR_LABEL)//if there is not specified label in sequence
		{
			return;//finish this command doing nothing
		}
		else
		{
			SeqLnCntr=SeqLnNo;//jump to the line with label repeating sequence of the code
			return;
		}
	}
	//end of repetition
	RptNo=0;
	CurrentRprNo=0;
	return;
}//cExeMngr::SeqActRpt

//execute move forward sequence command
void cExeMngr::SeqActMoveFrd(void)
{
	WORD DistancePulses;
	BYTE SpeedProfile;
	
	//extract distance
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not distance do nothing
		return;
	DistancePulses=atoi(TokenBuffer);
	
	//extract speed profile
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not speed profile do nothing
		return;
	SpeedProfile=atoi(TokenBuffer);
	
#if DEBUG_MNG_EXE
	DbgTraceStrVal(2,"ExeMngr_18","\nTRC: ExeMngr: SeqActMoveFrd Move forward distance: ",DistancePulses);
	DbgTraceStrVal(2,"ExeMngr_18","\nTRC: ExeMngr: SeqActMoveFrd Move forward speed profile: ",SpeedProfile);
#endif		
	SyncMoveForwardCmd(DistancePulses,SpeedProfile);
	
}//cExeMngr::SeqActMoveFrd

//execute move backword sequence command
void cExeMngr::SeqActMoveBwr(void)
{
	WORD DistancePulses;
	BYTE SpeedProfile;
	
	//extract distance
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not distance do nothing
		return;
	DistancePulses=atoi(TokenBuffer);
	
	//extract speed profile
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not speed profile do nothing
		return;
	SpeedProfile=atoi(TokenBuffer);
	
#if DEBUG_MNG_EXE
	DbgTraceStrVal(2,"ExeMngr_19","\nTRC: ExeMngr: SeqActMoveBwr Move backward distance: ",DistancePulses);
	DbgTraceStrVal(2,"ExeMngr_19","\nTRC: ExeMngr: SeqActMoveBwr Move backward speed profile: ",SpeedProfile);
#endif		
	SyncMoveReverseCmd(DistancePulses,SpeedProfile);

}//cExeMngr::SeqActMoveBwr

//execute sequence command to turn left 90 deg
void cExeMngr::SeqActTurnLeft(void)
{

#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_20","\nTRC: ExeMngr: SeqActTurnLeft");
#endif		
	SyncTurnLeft90DegCmd();
}//cExeMngr::SeqActTurnLeft

void cExeMngr::SeqActTurnRight(void)
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_21","\nTRC: ExeMngr: SeqActTurnRight");
#endif		
	
	SyncTurnRight90DegCmd();
	
}//cExeMngr::SeqActTurnRight

void cExeMngr::SeqActTurnAround(void)
{
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_22","\nTRC: ExeMngr: SeqActTurnAround");
#endif	
	if(rand()%2)//randomly select direction
	{//and if right make turns
		if(SyncTurnRight90DegCmd())//turn if error break
			return;
		if(SyncTurnRight90DegCmd())//turn if error break
			return;
		if(SyncTurnRight90DegCmd())//turn if error break
			return;
		if(SyncTurnRight90DegCmd())//turn if error break
			return;
	}else // otherwise left turn
	{
		if(SyncTurnLeft90DegCmd())//turn if error break
			return;
		if(SyncTurnLeft90DegCmd())//turn if error break
			return;
		if(SyncTurnLeft90DegCmd())//turn if error break
			return;
		if(SyncTurnLeft90DegCmd())//turn if error break
			return;
	}
}//cExeMngr::SeqActTurnAround

void cExeMngr::SeqActTurnHead(void)
{
	WORD HeadCount;//storage for number of counts corresponding to desired head position
	
	//extract number of counts corresponding to head position
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not counting for head position do nothing
		return;
	HeadCount=atoi(TokenBuffer);
	
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_23","\nTRC: ExeMngr: SeqActTurnHead asynchronous");
		DbgTraceStrVal(2,"ExeMngr_23","\nTRC: ExeMngr: SeqActTurnHead position: ",HeadCount);
#endif			
		TurnHead(ASYNC_HANDLING,TURN_HEAD_ON_SCMD_ID,HeadCount);//turn on head servo and move
		TurnHead(ASYNC_HANDLING,TURN_HEAD_OFF_SCMD_ID,HeadCount);//turn off head servo
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_23","\nTRC: ExeMngr: SeqActTurnHead synchronous");
		DbgTraceStrVal(2,"ExeMngr_23","\nTRC: ExeMngr: SeqActTurnHead position: ",HeadCount);
#endif
		TurnHead(SYNC_HANDLING,TURN_HEAD_ON_SCMD_ID,HeadCount); //turn on head servo and move
		TurnHead(SYNC_HANDLING,TURN_HEAD_OFF_SCMD_ID,HeadCount);//turn off head servo
	}
}//cExeMngr::SeqActTurnHead

//turn on arms servo
void cExeMngr::SeqActArmsOn(void)
{
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_24","\nTRC: ExeMngr: SeqActArmsOn asynchronous");
#endif
		ArmServoOnCmd(ASYNC_HANDLING);//turn on arm servos
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_24","\nTRC: ExeMngr: SeqActArmsOn synchronous");
#endif
		ArmServoOnCmd(SYNC_HANDLING);//turn on arm servos
	}	
	
}//cExeMngr::SeqActArmsOn

//turn off arms servo
void cExeMngr::SeqActArmsOff(void)
{
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_25","\nTRC: ExeMngr: SeqActArmsOff asynchronous");
#endif
		ArmServoOffCmd(ASYNC_HANDLING);//turn on arm servos
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_25","\nTRC: ExeMngr: SeqActArmsOff synchronous");
#endif
		ArmServoOffCmd(SYNC_HANDLING);//turn on arm servos
	}		
}//cExeMngr::SeqActArmsOff

void cExeMngr::SeqActLeftArm(void)
{
	WORD ArmCount;//storage for number of counts corresponding to desired arm position
	
	//extract number of counts corresponding to arm position
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not counting for arm position do nothing
		return;
	ArmCount=atoi(TokenBuffer);
	
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{

#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_26","\nTRC: ExeMngr: SeqActLeftArm asynchronous");
		DbgTraceStrVal(2,"ExeMngr_26","\nTRC: ExeMngr: SeqActLeftArm position: ",ArmCount);
#endif
		MoveLeftArmCmd(ASYNC_HANDLING,ArmCount);//move arm as requested
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_26","\nTRC: ExeMngr: SeqActLeftArm synchronous");
		DbgTraceStrVal(2,"ExeMngr_26","\nTRC: ExeMngr: SeqActLeftArm position: ",ArmCount);
#endif
		MoveLeftArmCmd(SYNC_HANDLING,ArmCount);//move arm as requested
	}	

}//cExeMngr::SeqActLeftArm

void cExeMngr::SeqActRightArm(void)
{
	WORD ArmCount;//storage for number of counts corresponding to desired arm position
	
	//extract number of counts corresponding to arm position
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not counting for arm position do nothing
		return;
	ArmCount=atoi(TokenBuffer);
	
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_27","\nTRC: ExeMngr: SeqActRightArm asynchronous");
		DbgTraceStrVal(2,"ExeMngr_27","\nTRC: ExeMngr: SeqActRightArm position: ",ArmCount);
#endif
		MoveRightArmCmd(ASYNC_HANDLING,ArmCount);//move arm as requested
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_27","\nTRC: ExeMngr: SeqActRightArm synchronous");
		DbgTraceStrVal(2,"ExeMngr_27","\nTRC: ExeMngr: SeqActRightArm position: ",ArmCount);
#endif
		MoveRightArmCmd(SYNC_HANDLING,ArmCount);//move arm as requested
	}	
	
}//cExeMngr::SeqActRightArm

void cExeMngr::SeqActArmsSyncMove(void)
{
	WORD ArmCount;//storage for number of counts corresponding to desired arm position
	
	//extract number of counts corresponding to arm position
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not counting for arm position do nothing
		return;
	ArmCount=atoi(TokenBuffer);
	
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_28","\nTRC: ExeMngr: SeqActArmsSyncMove asynchronous");
		DbgTraceStrVal(2,"ExeMngr_28","\nTRC: ExeMngr: SeqActArmsSyncMove position: ",ArmCount);
#endif
		MoveArmSyncCmd(ASYNC_HANDLING,ArmCount);//move arm as requested
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_28","\nTRC: ExeMngr: SeqActArmsSyncMove synchronous");
		DbgTraceStrVal(2,"ExeMngr_28","\nTRC: ExeMngr: SeqActArmsSyncMove position: ",ArmCount);
#endif
		MoveArmSyncCmd(SYNC_HANDLING,ArmCount);//move arm as requested
	}	
}//cExeMngr::SeqActArmsSyncMove

//execute move of both arms but oposit (one up on down)
void cExeMngr::SeqActArmsOpositMove(void)
{
	WORD ArmCount;//storage for number of counts corresponding to desired arm position
	
	//extract number of counts corresponding to arm position
	pCurPosInSeq=ExtractTokenToBuffer(pCurPosInSeq,TokenBuffer,sizeof(TokenBuffer));
	if(!pCurPosInSeq)//if there is not counting for arm position do nothing
		return;
	ArmCount=atoi(TokenBuffer);
	
	if(ExeCode==SEQ_EXE_CODE_ASYNC)//if asynchronous handling requested
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_29","\nTRC: ExeMngr: SeqActArmsOpositMove asynchronous");
		DbgTraceStrVal(2,"ExeMngr_29","\nTRC: ExeMngr: SeqActArmsOpositMove position: ",ArmCount);
#endif
		 MoveArmOpositCmd(ASYNC_HANDLING,ArmCount);//move arm as requested
	}else //execute command as synchronous otherwise
	{
#if DEBUG_MNG_EXE
		DbgTraceStr(2,"ExeMngr_29","\nTRC: ExeMngr: SeqActArmsOpositMove synchronous");
		DbgTraceStrVal(2,"ExeMngr_29","\nTRC: ExeMngr: SeqActArmsOpositMove position: ",ArmCount);
#endif
		 MoveArmOpositCmd(SYNC_HANDLING,ArmCount);//move arm as requested
	}		
}//cExeMngr::SeqActArmsOpositMove


//execute CMD_EXE_CMD command triggered by VOICE COMMANDS
//returns:
//       nothing but internally RSP_EXE_CMD notifier is posted
void cExeMngr::ProcessCmdExe(cSmartPtr<cNotifier> pNotifier)
{
	//extract requested command and its eventual parameter value
	BYTE CmdId = (static_cast<sCmdExeData*>(pNotifier->GetDataPtr()))->mCmdExeId;
	WORD ParameterValue = (static_cast<sCmdExeData*>(pNotifier->GetDataPtr()))->mParameterValue;
	BYTE Handling =(static_cast<sCmdExeData*>(pNotifier->GetDataPtr()))->mHandling;
#if DEBUG_MNG_EXE
	DbgTraceStr(2,"ExeMngr_30","\nTRC: ExeMngr: ProcessCmdExe triggered by Voice Command");
	DbgTraceStrVal(2,"ExeMngr_30","\nTRC: ExeMngr: ProcessCmdExe: Command: ",CmdId);
#endif
	
	switch(CmdId)
	{
	case CMD_EXE_TURN_HEAD_ID://head movement is requested
		TurnHead(Handling,TURN_HEAD_ON_SCMD_ID,ParameterValue);//turn on servo and move to desired position
		TurnHead(Handling,TURN_HEAD_OFF_SCMD_ID,ParameterValue);//turn servo off
		break;
	case CMD_EXE_LEFT_ARM_ID://left arem movement
		ArmServoOnCmd(SYNC_HANDLING);//turn on arm servos
		MoveLeftArmCmd(Handling,ParameterValue);//move arm as requested
		ArmServoOffCmd(SYNC_HANDLING);//turn off arm servos
		break;
	case CMD_EXE_RIGHT_ARM_ID://left arem movement
		ArmServoOnCmd(SYNC_HANDLING);//turn on arm servos
		MoveRightArmCmd(Handling,ParameterValue);//move arm as requested
		ArmServoOffCmd(SYNC_HANDLING);//turn off arm servos
		break;
	case CMD_EXE_ARM_SYNC_ID://when synchronous movement of both arms requested
		ArmServoOnCmd(SYNC_HANDLING);//turn on arm servos
		MoveArmSyncCmd(Handling,ParameterValue);
		ArmServoOffCmd(SYNC_HANDLING);//turn off arm servos
		break;
	case CMD_EXE_FORWARD_ID://when move forward requested
		SyncMoveForwardCmd(ParameterValue,LOW_MID_HIGH_PROFILE);
		break;
	case CMD_EXE_REVERSE_ID:
		SyncMoveReverseCmd(ParameterValue,LOW_MID_HIGH_PROFILE);
		break;
	case TURN_LEFT_90:
		SyncTurnLeft90DegCmd();
		break;
	case TURN_RIGHT_90:
		SyncTurnRight90DegCmd();
		break;
	case TURN_360:
		if(rand()%2)//randomly select direction
		{//and if right make turns
			if(SyncTurnRight90DegCmd())//turn if error break
				break;;
			if(SyncTurnRight90DegCmd())//turn if error break
				break;
		}else // otherwise left turn
		{
			if(SyncTurnLeft90DegCmd())//turn if error break
				break;
			if(SyncTurnLeft90DegCmd())//turn if error break
				break;
		}
		break;
	default:
		break;
	}//switch
	//send response once command executed	
	cSmartPtr<cTypeNotifier<sCmdExeData> > pRspNotifier = new cTypeNotifier<sCmdExeData>(RSP_EXE_CMD,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mCmdExeId=CmdId;
	(pRspNotifier->GetData()).mParameterValue=ParameterValue;
	(pRspNotifier->GetData()).mHandling=Handling;
	Post(pRspNotifier);//post response
}//cExeMngr::ProcessCmdExe


//Wall-e executes voice commands
void cExeMngr::WalleProgramVoiceCtrl(void)
{
	Uart0PutStr("\nLOG: Exe: Walle Program Voice Control\n");
	
 	for(;;)
	{
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		switch (pNotifier->GetNotifierId()) //execute manager command depending on the request
		{
		case  CMD_EXE_CMD://when remote execution of command requested process it
			ProcessCmdExe(pNotifier);
			break;
		default://do nothing when unknown request received
			break;
		}//switch
	}
}//cExeMngr::WalleProgramVoiceCtrl

//Wall-e executes its own test sequence to check basic behaviour
void cExeMngr::WalleProgramTest(void)
{
	Uart0PutStr("\nLOG: Exe: Walle Program Test\n");
	
 	for(;;)
	{
 		ExeCmdSeq(WalleTestSeqCmdTbl);//execute command sequence
 		for(;;)//once command sequence executed do nothing
 		{
 			Delay(EXE_INFINITE_LOOP_DELAY);//just delay to not exhoust all OS power when looping infinit
 		}
	}
}//cExeMngr::WalleProgramTest

//Wall-e executes its own enjoy sequence
void cExeMngr::WalleProgramEnjoy(void)
{
	Uart0PutStr("\nLOG: Exe: Walle Program Enjoy\n");
	
 	for(;;)
	{
 		ExeCmdSeq(WalleEnjoySeqCmdTbl);//execute command sequence
 		Delay(EXE_ENJOY_DELAY);//wait for a moment before Enjoy sequence starts again
 	}
}//cExeMngr::WalleProgramTest

void cExeMngr::Run()
{
	BYTE ProgramToExecute;//keeps Walle currently executed program 

#if DEBUG_MNG_EXE
	DbgTraceStr(1,"ExeMngr_31","\nTRC: --- ExeMngr Run() ---\n");
#endif	

	
	if(ReadLastResetReason()!= SW_RESET) //when it is not sw requested program change
	{//wait until mng_vrm wellcome message played	
		Kernel.TimeDlyHMSM(0,0,EXE_MNG_WAIT_TO_START,0);//do nothing for a moment to let Wall-e introduce
#if DEBUG_MNG_EXE
	DbgTraceStr(1,"ExeMngr_31","\nTRC: Not a SW RESET!");
#endif			
	}
	//get program to execute, program change is possible only through SW RESET of WALL-e
	ProgramToExecute=GetWalleProgramToExecute();
	for(;;)
	{
		switch(ProgramToExecute)//get setup program and execute it
		{
		case WALLE_PROGRAM_BATH: //find bath program
			WalleProgramBath();
			break;
		case WALLE_PROGRAM_VOICE_CTRL:
			WalleProgramVoiceCtrl();
			break;
		case WALLE_PROGRAM_TEST:
			WalleProgramTest();
			break;
		case WALLE_PROGRAM_ENJOY:
			WalleProgramEnjoy();
			break;
		default://undefined program should never happen
			LCDDebugMessage("ERR: Wrng Prg: ",ProgramToExecute,20,1,1,0);
			Uart0Message("ERR: Exe: Wrong program: ",ProgramToExecute);			
			NOT_ALLOWED_PROGRAM;//generate an exception for not allwed Wall-e program selected
			break;
		}//switch 
		
	}//for
}//cExeMngr::Run


