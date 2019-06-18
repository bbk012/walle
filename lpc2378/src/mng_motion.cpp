/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2010, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_motion.cpp
* Description: Manager which performs overall motion control of robot (track motors)
* Author:      Bogdan Kowalczyk
* Date:        10-January-2010
* Note:
* History:
*              10-Jan-2010 - Initial version created
*********************************************************************************************************
*/

#include "mng_motion.hpp"
#include "wrp_kernel.hpp"
#include "ctr_lcd.h"
#include "hw_timer.h"
#include "hw_spi.h"
#include "hw_rtc.h"
#include "lib_std.h"
#include "ctr_f_sens.h"

#include "ctr_gp2d12.h"
#include "hw_hcsr04.h"

#include "hw_pwm1.h"
#include "ctr_lcd.h"
#include "hw_adc.h"
#include "hw_uart.h"

BYTE ObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE];//used to store obstacles information for scanning
WORD IdentifiedPossiblePaths;//used to store possible paths after obstacle table analysis by FindMovePaths function 

WORD LightSrcTable[LIGHT_SRC_TABLE_SIZE];//storage of light source scaning results

//---------------------------------------------------------------------------------------------------------------
//Structures filled by ScanLightSurroundings function
WORD FrdLightSrcTable[LIGHT_SRC_TABLE_SIZE];//storage of light source scaning results for forward direction
WORD LftLightSrcTable[LIGHT_SRC_TABLE_SIZE];//storage of light source scaning results for left direction
WORD RgtLightSrcTable[LIGHT_SRC_TABLE_SIZE];//storage of light source scaning results for right direction
WORD RevLightSrcTable[LIGHT_SRC_TABLE_SIZE];//storage of light source scaning results for reverse direction

//IMPORTANT! Not that fotorezistor is monted on the Wall-e back
//when Wall-e is pointed with his head forward we measure reverse day night reading and store it
//in the RevDayNight variable
WORD FrdDayNight; //Day night fotoresistor value for the forward direction
WORD LftDayNight; //Day night fotoresistor value for the left direction
WORD RgtDayNight; //Day night fotoresistor value for the right direction
WORD RevDayNight; //Day night fotoresistor value for the reverse direction

//---------------------------------------------------------------------------------------------------------------
//move specified number of puleses toward specified direction with specified speed profile
//inputs:
//       InDistancePulses - desired distance in pulses
//       InDirection - desired direction (MOTOR_FORWARD, MOTOR_REVERSE)
//       InSpeedProfile - desired profile of the movement (LOW_MID_HIGH_PROFILE, LOW_MID_PROFILE, LOW_PROFILE, MID_PROFILE, HIGH_PROFILE)    		
//returns:
//	MOVE_OK - O.K movement executed with none problems
//	MOVE_BREAK_FORWARD	Error - movement cannot be realized none chnge in pulses when moving forward
//	MOVE_BREAK_REVERSE	Error - movement cannot be realized none chnge in pulses when moving reverse
//	MOVE_BREAK_OBSTACLE Error - distance detector detected an obstacle
BYTE cMotionMngr::Move(WORD InDistancePulses,BYTE InDirection,BYTE InSpeedProfile)
{
	WORD RightTrackPulses=0;// assume none pulse at start
	WORD LeftTrackPulses=0;//assume none pulses at start
	BYTE ObstacleStatus; //handles result of CheckObstacle call
	WORD RawIredData;//tmp storage for IRED Raw Data returned by CheckForObstacleIRED
	WORD RawUSData;//tmp storage for US Raw Data returned by CheckForObstacleUS
	
	mCoveredDistancePulses=0;//stores really covered distance in pulses at begining none movement is executed
	if(!InDistancePulses)return MOVE_OK;//requested none movement so OK immediately
	
	//move arms to max up position which is save for moving so any obstacle hited cannot damage arm 
	ArmServoOn();
	RunPWMLeftArm(LEFT_ARM_MAX_UP_PHY);
	OSTimeDly(PWM1_MR0_MATCH);//work around to get arms moved
	RunPWMRightArm(RIGHT_ARM_MAX_UP_PHY);
	Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
	ArmServoOff();
	
	//assure Head is directed forward central to track obstacle during the movement
	HeadServoOn();
	RunPWMHead(HEAD_CENTRAL);
	Kernel.TimeDlyHMSM(0,0,0,TIME_TO_GET_HEAD_CENTRAL);

	HeadServoOff();
	
	//start movement
	RunTracksPulses(InDistancePulses,InDirection,InDistancePulses,InDirection,InSpeedProfile);
	while(IsTrackMoving())//when movement check left and right track pulse change
	{
//		TrackPulseTimeoutInTciks+=1;//increase timeout counter commented out because CheckForObstacleIRED()
//      introduces enough delay (about 100 ms) to not have additional code checking movement progress after some time
		if(InDirection==MOTOR_FORWARD)//check obstacle but only for forward movement
		{
			ObstacleStatus=CheckForObstacleUS(&RawUSData);//first check US for very close object
			//if there is not close obstacle use IR and fusion
			if(ObstacleStatus!=OBSTACLE_VERY_SHORT_DISTANCE)
				//inside CheckForObstacleIRED there is delay of about 0.5 sec
				ObstacleStatus=DistanceSensorsFusion(ObstacleStatus, CheckForObstacleIRED(&RawIredData));		

			if(ObstacleStatus==OBSTACLE_CHASM ||ObstacleStatus==OBSTACLE_VERY_SHORT_DISTANCE)//if obstacle detected close to Wall-e
			{
				mCoveredDistancePulses=GetCurrentTrackPulses();//get final number of pulses managed to be run
				StopTracks(); //stop movement immediately
				BuzzerOn();
				Delay(100);
				BuzzerOff();
				return MOVE_BREAK_OBSTACLE;//signal failure in movement	
			}// if obstacle
			
			//check if during movement Wall-e is not moving over an obstacle
			ObstacleStatus = CheckForTilt();
			if(ObstacleStatus!=NO_TILT) //if it looks Wall-e is tilt stop and return an error
			{
				mCoveredDistancePulses=GetCurrentTrackPulses();//get final number of pulses managed to be run
				StopTracks(); //stop movement immediately
				BuzzerOn();
				Delay(100);
				BuzzerOff();
				return MOVE_BREAK_OBSTACLE;//signal failure in movement		
			}//if TILT
		}//end if moving forward
		else //if we move reverse there is not delay generated by CheckForObstacleIRED 
			//so to get some changes in pulses delay is added
			
		{
			//check if during movement Wall-e is not moving over an obstacle
			ObstacleStatus = CheckForTilt();
			if(ObstacleStatus!=NO_TILT) //if it looks Wall-e is tilt stop and return an error
			{
				mCoveredDistancePulses=GetCurrentTrackPulses();//get final number of pulses managed to be run
				StopTracks(); //stop movement immediately
				BuzzerOn();
				Delay(100);
				BuzzerOff();
				return MOVE_BREAK_OBSTACLE;//signal failure in movement		
			}
			Kernel.TimeDlyHMSM(0,0,0,REVERSE_PULSE_TIMEOUT);//we need to wait for a moment 
		}
		//check for changes in track pulses (to see if tracks are not blocked)
		if((RightTrackPulses==GetRightTrackPulses())||(LeftTrackPulses==GetLeftTrackPulses()))
		{
			if(IsTrackMoving())//if we should move but there is not any change in track pulses
			{
				mCoveredDistancePulses=GetCurrentTrackPulses();//get final number of pulses managed to be run
				StopTracks(); //stop movement immediately
				if(InDirection==MOTOR_FORWARD)
				{
					return MOVE_BREAK_FORWARD;//signal failure in movement forward
				}
				else
				{
					return MOVE_BREAK_REVERSE;//signal failure in movement reverse
				}
			}
		}
     	//setup new number of pulses for left and right track for future comparision
		RightTrackPulses=GetRightTrackPulses();
		LeftTrackPulses=GetLeftTrackPulses();
//		}//if track timeout
	}//while in movement
	mCoveredDistancePulses=InDistancePulses;//no error desired distance fully covered
	return MOVE_OK;
}//cMotionMngr::Move


//turn left 90 degrees
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::TurnLeft90Deg(void)
{
	return TurnLeftTo90Deg(ANGLE_90);
}//cMotionMngr::TurnLeft90Deg

//turn left from 0 to 90 degrees
//input:      Degree of left turn (positive value from 0 degs to 90 degs)
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::TurnLeftTo90Deg(long AngleInDegs)
{
	WORD RightTrackPulses=0;// assume none pulse at start
	WORD LeftTrackPulses=0;//assume none pulses at start
	WORD TrackPulseTimeoutInTciks=0;//timeout counter to zero
	
	//move arms to max up position which is save for moving so any obstacle hited cannot damage arm 
	ArmServoOn();
	RunPWMLeftArm(LEFT_ARM_MAX_UP_PHY);
	OSTimeDly(PWM1_MR0_MATCH);//work around to get arms moved
	RunPWMRightArm(RIGHT_ARM_MAX_UP_PHY);
	Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
	ArmServoOff();
	

	SetArsTargetAngle((AngleInDegs*ARS_LEFT_90_TURN_COUNT)/90L);//setup ARS counts corsponding to the turn angle
	GetAdcAccess();//guarantee exclusive access to ADC for this task
	SetArsOffset();//setup most up to date offset value for ARS channel
	StartArsAngleCounting();//start angle integration up to desired angle
	//start movement left
	RunTracksPulses(TURN_90_DEG_PULSES, MOTOR_REVERSE,TURN_90_DEG_PULSES,MOTOR_FORWARD,HIGH_PROFILE);
	while(IsTrackMoving())//when movement check left and right track pulse change
	{
		Delay(1);//wait on uCOS-II tick to not block other tasks
		TrackPulseTimeoutInTciks+=1;//increase timeout counter
		if(TrackPulseTimeoutInTciks >= TRACK_PULSE_TIMOUT_TICKS)
		{
			TrackPulseTimeoutInTciks=0;//start timeout again so reset timeout counter
			//if there is not change of pulses during movement
			if((RightTrackPulses==GetRightTrackPulses())||(LeftTrackPulses==GetLeftTrackPulses()))
			{
				if(IsTrackMoving())//if we should move but there is not any change in track pulses
				{
				StopTracks(); //stop movement immediately
				StopArsAngleCounting();//none angle integration anymore as we are not moving
				ReleaseAdcAccess();//release exclusive access to ADC
				return MOVE_BREAK_LEFT;//signal failure in movement
				}
			}
			//setup new number of pulses for left and right track for future comparision
			RightTrackPulses=GetRightTrackPulses();
			LeftTrackPulses=GetLeftTrackPulses();
		}//if track timeout
	}//while in movement
	//movement finished
	StopArsAngleCounting();//none angle integration anymore as we are not moving
	ReleaseAdcAccess();//release exclusive access to ADC
	//when movement completed check angle achived
	if(GetArsAngleValue() >= GetArsTargetAngle())//if desired turn right achived
		return MOVE_OK;
	else //when desired turn not achived
		{
		return MOVE_ANGLE_LEFT;
		}
}//cMotionMngr::TurnLeftTo90Deg

//turn right 90 degrees
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT - Error - destination angle not achieved when turing right
BYTE cMotionMngr::TurnRight90Deg(void)
{
	return TurnRightTo90Deg(ANGLE_90);
}// cMotionMngr::TurnRight90Deg

//turn right from 0 to 90 degrees
//input:      Degree of right turn (positive value from 0 degs to 90 degs)
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
BYTE cMotionMngr::TurnRightTo90Deg(long AngleInDegs)
{
	WORD RightTrackPulses=0;// assume none pulse at start
	WORD LeftTrackPulses=0;//assume none pulses at start
	WORD TrackPulseTimeoutInTciks=0;//timeout counter to zero
	
	//move arms to max up position which is save for moving so any obstacle hited cannot damage arm 
	ArmServoOn();
	RunPWMLeftArm(LEFT_ARM_MAX_UP_PHY);
	OSTimeDly(PWM1_MR0_MATCH);//work around to get arms moved
	RunPWMRightArm(RIGHT_ARM_MAX_UP_PHY);
	Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
	ArmServoOff();
	

	SetArsTargetAngle((AngleInDegs*ARS_RIGHT_90_TURN_COUNT)/90L);//setup destination angle corresponding to 90 degs right
	GetAdcAccess();//guarantee exclusive access to ADC for this task
	SetArsOffset();//setup most up to date offset value for ARS channel
	StartArsAngleCounting();//start angle integration up to desired angle
	//start movement right
	RunTracksPulses(TURN_90_DEG_PULSES,MOTOR_FORWARD,TURN_90_DEG_PULSES,MOTOR_REVERSE,HIGH_PROFILE);
	while(IsTrackMoving())//when movement check left and right track pulse change
	{
		Delay(1);//wait on uCOS-II tick to not block other tasks
		TrackPulseTimeoutInTciks+=1;//increase timeout counter
		if(TrackPulseTimeoutInTciks >= TRACK_PULSE_TIMOUT_TICKS)
		{
			TrackPulseTimeoutInTciks=0;//start timeout again so reset timeout counter
			//if there is not change of pulses during movement
			if((RightTrackPulses==GetRightTrackPulses())||(LeftTrackPulses==GetLeftTrackPulses()))
			{
				if(IsTrackMoving())//if we should move but there is not any change in track pulses
				{
				StopTracks(); //stop movement immediately
				StopArsAngleCounting();//none angle integration anymore as we are not moving
				ReleaseAdcAccess();//release exclusive access to ADC
				return MOVE_BREAK_RIGHT;//signal failure in movement
				}
			}
			//setup new number of pulses for left and right track for future comparision
			RightTrackPulses=GetRightTrackPulses();
			LeftTrackPulses=GetLeftTrackPulses();
		}//if track timeout
	}//while in movement
	//movement finished
	StopArsAngleCounting();//none angle integration anymore as we are not moving
	ReleaseAdcAccess();//release exclusive access to ADC
	//when movement completed check angle achived
	if(GetArsAngleValue() <= GetArsTargetAngle())//if desired turn left achived
		return MOVE_OK;
	else //when desired turn not achived
		return MOVE_ANGLE_RIGHT;	
}// cMotionMngr::TurnRightTo90Deg

//turn right from 0 to 360 degrees
//input:      Degree of right turn (positive value from 0 degs to 360 degs)
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
BYTE cMotionMngr::TurnRightTo360Deg(long AngleInDegs)
{
	WORD RightTrackPulses=0;// assume none pulse at start
	WORD LeftTrackPulses=0;//assume none pulses at start
	WORD TrackPulseTimeoutInTciks=0;//timeout counter to zero
	
	Kernel.TimeDlyHMSM(0,0,0,TURN_STABILIZATION_DELAY);//wait 500ms before turn to get ARS stable after previous movements
	SetArsTargetAngle((AngleInDegs*ARS_RIGHT_90_TURN_COUNT)/90L);//setup destination angle corresponding to 90 degs right
	GetAdcAccess();//guarantee exclusive access to ADC for this task
	SetArsOffset();//setup most up to date offset value for ARS channel
	StartArsAngleCounting();//start angle integration up to desired angle
	//start movement right
	RunTracksPulses(TURN_360_DEG_PULSES,MOTOR_FORWARD,TURN_360_DEG_PULSES,MOTOR_REVERSE,HIGH_PROFILE);
	while(IsTrackMoving())//when movement check left and right track pulse change
	{
		Delay(1);//wait on uCOS-II tick to not block other tasks
		TrackPulseTimeoutInTciks+=1;//increase timeout counter
		if(TrackPulseTimeoutInTciks >= TRACK_PULSE_TIMOUT_TICKS)
		{
			TrackPulseTimeoutInTciks=0;//start timeout again so reset timeout counter
			//if there is not change of pulses during movement
			if((RightTrackPulses==GetRightTrackPulses())||(LeftTrackPulses==GetLeftTrackPulses()))
			{
				if(IsTrackMoving())//if we should move but there is not any change in track pulses
				{
				StopTracks(); //stop movement immediately
				StopArsAngleCounting();//none angle integration anymore as we are not moving
				ReleaseAdcAccess();//release exclusive access to ADC
				return MOVE_BREAK_RIGHT;//signal failure in movement
				}
			}
			//setup new number of pulses for left and right track for future comparision
			RightTrackPulses=GetRightTrackPulses();
			LeftTrackPulses=GetLeftTrackPulses();
		}//if track timeout
	}//while in movement
	//movement finished
	StopArsAngleCounting();//none angle integration anymore as we are not moving
	ReleaseAdcAccess();//release exclusive access to ADC
	//when movement completed check angle achived
	if(GetArsAngleValue() <= GetArsTargetAngle())//if desired turn left achived
		return MOVE_OK;
	else //when desired turn not achived
		return MOVE_ANGLE_RIGHT;	
}// cMotionMngr::TurnRightTo360Deg

//turn left 90 degs or right 90 degs randomly
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::RandLeftOrRightTurn()
{
	if(rand()%2)//when right turn
	{
		return TurnRight90Deg();
	}else // otherwise left turn
	{
		return TurnLeft90Deg();
	}
} //cMotionMngr::RandLeftOrRightTurn

//---------------------------------------------------------------------------------------------------------------
//This functions make 360 deg Wall-e movement and scan for light source and day night data
//inputs:
//       none but global ScanLightSurroundings structures defined on the top of this file are filled with data    		
//returns:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::ScanLightSurroundings()
{
	BYTE MoveResult;//temporary storage of the turn command result
	
	//clear the contents of the light intensity table for every direction
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++)
		{
		FrdLightSrcTable[x]=0;
		LftLightSrcTable[x]=0;
		RgtLightSrcTable[x]=0;
		RevLightSrcTable[x]=0;
		}//for	
	
	//all day night storage is initialized to 0 before real measurement
	FrdDayNight=0; //Day night fotoresistor value for the forward direction
	LftDayNight=0; //Day night fotoresistor value for the left direction
	RgtDayNight=0; //Day night fotoresistor value for the right direction
	RevDayNight=0; //Day night fotoresistor value for the reverse direction	
	
	if(rand()%2)//select randomly scanning turn direction 1- right turn, 0 - left turn
	{
		//turn right 90 degs to right position
		MoveResult=TurnRight90Deg();//turn right to the right possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		LftDayNight=ReadRawFotoResistor(); //when turned right tail is checking left day night data
		ScanLightSrc(RgtLightSrcTable);
		
		//turn right 90 degs to reverse position
		MoveResult=TurnRight90Deg();//turn right to the reverse possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		FrdDayNight=ReadRawFotoResistor(); //when turned rev tail is checking forward day night data
		ScanLightSrc(RevLightSrcTable);

		//turn right 90 degs to left position
		MoveResult=TurnRight90Deg();//turn right to the left possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		RgtDayNight=ReadRawFotoResistor(); //when turned Lft tail is checking right day night data
		ScanLightSrc(LftLightSrcTable);
		
		//turn right 90 degs to forward position
		MoveResult=TurnRight90Deg();//turn right to the forward possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		RevDayNight=ReadRawFotoResistor(); //when turned forward tail is checking reverse day night data
		ScanLightSrc(FrdLightSrcTable);
		
	}else //when left turn
	{
		//turn left 90 degs to left position
		MoveResult=TurnLeft90Deg();//turn left to the left possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		RgtDayNight=ReadRawFotoResistor(); //when turned Lft tail is checking right day night data
		ScanLightSrc(LftLightSrcTable);
		
		//turn left 90 degs to reverse position
		MoveResult=TurnLeft90Deg();//turn to the reverse possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		FrdDayNight=ReadRawFotoResistor(); //when turned rev tail is checking forward day night data
		ScanLightSrc(RevLightSrcTable);

		//turn left 90 degs to right position
		MoveResult=TurnLeft90Deg();//turn to the right possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		LftDayNight=ReadRawFotoResistor(); //when turned right tail is checking left day night data
		ScanLightSrc(RgtLightSrcTable);
		
		//turn left 90 degs to forward position
		MoveResult=TurnLeft90Deg();//turn left to the forward possition
		if(MoveResult) return MoveResult; //break execution when errot and return it
	    //start with scaning
		RevDayNight=ReadRawFotoResistor(); //when turned forward tail is checking reverse day night data
		ScanLightSrc(FrdLightSrcTable);
	}
	return MOVE_OK; //when we get this point it means all scanning movement was O.K.
}//cMotionMngr::ScanLightSurroundings

//---------------------------------------------------------------------------------------------------------------
//Moves Wall-e to the correct positoion at forward direction
//inputs:
//       Position at which Wall-e should be moved being at forward direction     		
//outputs:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::DirectFrd(BYTE Pos)
{
	BYTE MoveResult;//temporary storage for command result
	switch(Pos)//bing at right direction move to the right position
	{
	case FRD_IDX://when light source is stright ahead Wall-e nothing is required to be changed
		MoveResult=MOVE_OK;
		break;
	case L15_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_15);//turn left 15 degs
		break;
	case L30_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_30);//turn left 30 degs
		break;
	case L45_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_45);//turn left 45 degs
		break;
	case L60_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_60);//turn left 60 degs
		break;
	case R15_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_15);//turn right 15 degs
		break;
	case R30_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_30);//turn right 30 degs
		break;
	case R45_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_45);//turn right 45 degs
		break;
	case R60_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_60);//turn right 60 degs
		break;
	default:
		MoveResult=MOVE_ANGLE_LEFT;//strange case should never happen return an error
		break;
	}//switch
return 	MoveResult;
}//cMotionMngr::DirectFrd

//---------------------------------------------------------------------------------------------------------------
//Moves Wall-e to the correct positoion at left direction
//inputs:
//       Position at which Wall-e should be moved being at left direction     		
//outputs:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::DirectLft(BYTE Pos)
{
	BYTE MoveResult;//temporary storage for command result
	
	MoveResult=TurnLeft90Deg();//bing at right direction move to the right position
	if(MoveResult)return MoveResult;//when movement error break
	switch(Pos)
	{
	case FRD_IDX://when light source is stright ahead Wall-e nothing is required to be changed
		MoveResult=MOVE_OK;
		break;
	case L15_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_15);//turn left 15 degs
		break;
	case L30_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_30);//turn left 30 degs
		break;
	case L45_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_45);//turn left 45 degs
		break;
	case L60_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_60);//turn left 60 degs
		break;
	case R15_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_15);//turn right 15 degs
		break;
	case R30_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_30);//turn right 30 degs
		break;
	case R45_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_45);//turn right 45 degs
		break;
	case R60_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_60);//turn right 60 degs
		break;
	default:
		MoveResult=MOVE_ANGLE_LEFT;//strange case should never happen return an error
		break;
	}//switch
return 	MoveResult;
}//cMotionMngr::DirectLft

//---------------------------------------------------------------------------------------------------------------
//Moves Wall-e to the correct positoion at right direction
//inputs:
//       Position at which Wall-e should be moved being at right direction     		
//outputs:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::DirectRgt(BYTE Pos)
{
	BYTE MoveResult;//temporary storage for command result
	
	MoveResult=TurnRight90Deg();
	if(MoveResult)return MoveResult;//when movement error break
	switch(Pos)//bing at right direction move to the right position
	{
	case FRD_IDX://when light source is stright ahead Wall-e nothing is required to be changed
		MoveResult=MOVE_OK;
		break;
	case L15_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_15);//turn left 15 degs
		break;
	case L30_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_30);//turn left 30 degs
		break;
	case L45_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_45);//turn left 45 degs
		break;
	case L60_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_60);//turn left 60 degs
		break;
	case R15_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_15);//turn right 15 degs
		break;
	case R30_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_30);//turn right 30 degs
		break;
	case R45_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_45);//turn right 45 degs
		break;
	case R60_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_60);//turn right 60 degs
		break;
	default:
		MoveResult=MOVE_ANGLE_RIGHT;//strange case should never happen return an error
		break;
	}//switch
return 	MoveResult;
}//cMotionMngr::DirectRgt

//---------------------------------------------------------------------------------------------------------------
//Moves Wall-e to the correct positoion at reverse direction
//inputs:
//       Position at which Wall-e should be moved being at reverse direction     		
//outputs:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
BYTE cMotionMngr::DirectRev(BYTE Pos)
{
	BYTE MoveResult;//temporary storage for command result
	
	//turn to reverse direction (180 degs) either through lefy ot through right side of the Wall-e
	if(rand()%2)//when right turn generated
	{
		MoveResult=TurnRight90Deg();
		if(MoveResult)return MoveResult;//when movement error break
		MoveResult=TurnRight90Deg();
		if(MoveResult)return MoveResult;//when movement error break
	}else // otherwise assume left turn
	{
		MoveResult=TurnLeft90Deg();
		if(MoveResult)return MoveResult;//when movement error break
		MoveResult=TurnLeft90Deg();
		if(MoveResult)return MoveResult;//when movement error break
	}//if(rand()%2)

	switch(Pos)//bing at right direction move to the right position
	{
	case FRD_IDX://when light source is stright ahead Wall-e nothing is required to be changed
		MoveResult=MOVE_OK;
		break;
	case L15_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_15);//turn left 15 degs
		break;
	case L30_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_30);//turn left 30 degs
		break;
	case L45_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_45);//turn left 45 degs
		break;
	case L60_IDX:
		MoveResult=TurnLeftTo90Deg(ANGLE_60);//turn left 60 degs
		break;
	case R15_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_15);//turn right 15 degs
		break;
	case R30_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_30);//turn right 30 degs
		break;
	case R45_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_45);//turn right 45 degs
		break;
	case R60_IDX:
		MoveResult=TurnRightTo90Deg(ANGLE_60);//turn right 60 degs
		break;
	default:
		MoveResult=MOVE_ANGLE_RIGHT;//strange case should never happen return an error
		break;
	}//switch
return 	MoveResult;
}//cMotionMngr::DirectRev



//---------------------------------------------------------------------------------------------------------------
//This functions is used to finding max ADC reading of the foto-nose from
//xxxLightSrcTables filled by previous ScanLightSurroundings call and to position Wall-e towards it.
//inputs:
//       none but global xxxLightSrcTables are used by it     		
//outputs:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
//      MOVE_NONE_LIGHT_SRC - none meaningful light source was identified	
BYTE cMotionMngr::FindMaxLightScr(void)
{
	BYTE Dir=DIR_NONE;//temporary storage of direction which is containing the max value
	BYTE Pos=FRD_IDX;//position at direction for which most intensive light is detected initialize it for forward
	WORD MaxVal=0;//temporary max value
	BYTE MoveResult;//temporary storage for command result
	
	MoveResult=ScanLightSurroundings();//scan surroundings to fill LightSrcTables with most up to date light source data
	if(MoveResult)return MoveResult;//when there is any movement error break execution
	//find direction of the most intensive light source
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++) //check all light source entries from all directions 
	{//detect one with maximum value
		if(FrdLightSrcTable[x]>MaxVal)
		{
			MaxVal=FrdLightSrcTable[x];
			Dir=DIR_FRD;
			Pos=x;
		}
		if(LftLightSrcTable[x]>MaxVal)
		{
			MaxVal=LftLightSrcTable[x];
			Dir=DIR_LFT;
			Pos=x;
		}
		if(RgtLightSrcTable[x]>MaxVal)
		{
			MaxVal=RgtLightSrcTable[x];
			Dir=DIR_RGT;
			Pos=x;
		}
		if(RevLightSrcTable[x]>MaxVal)
		{
			MaxVal=RevLightSrcTable[x];
			Dir=DIR_REV;
			Pos=x;
		}
	}//for
	if(MaxVal <= MIN_FOTO_TRANSISTOR_COUNT)//if max light value is not meaningful return an error
		return MOVE_NONE_LIGHT_SRC;

	//---------------------------------------------
	switch (Dir)
	{
	case DIR_FRD:
		MoveResult=DirectFrd(Pos);//move to the correct positoion at forward direction
		break;
	case DIR_LFT:
		MoveResult=DirectLft(Pos);//move to the correct positoion at left direction
		break;
	case DIR_RGT:
		MoveResult=DirectRgt(Pos);//move to the correct positoion at right direction
		break;
	case DIR_REV:
		MoveResult=DirectRev(Pos);//move to the correct positoion at revers direction
		break;
	case DIR_NONE:
	default:
		MoveResult=MOVE_NONE_LIGHT_SRC;
		break;
	}//switch
	return MoveResult;//return movement result
}//cMotionMngr::FindMaxLightScr

//---------------------------------------------------------------------------------------------------------------
// PositionMaxLightScrFrd directs Wall-e towards the strongest light at forward position
//inputs:
//       none but global LightSrcTable is used by it     		
//outputs:
//		MOVE_OK - O.K movement executed with none problems
//		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
//      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
//		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
//      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
//      MOVE_NONE_LIGHT_SRC - none meaningful light source was identified
BYTE cMotionMngr::PositionMaxLightScrFrd(void)
{
	BYTE Pos=FRD_IDX;//position at direction for which most intensive light is detected initialize it for forward
	WORD MaxVal=0;//temporary max value
	BYTE MoveResult;//temporary storage for command result
	
	//find max value in LightSrcTable and position at which it occures
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++) //check all light source entries from all directions 
	{//detect one with maximum value
		if(LightSrcTable[x]>MaxVal)
		{
			MaxVal=LightSrcTable[x];
			Pos=x;
		}
	}//for
	if(MaxVal <= MIN_FOTO_TRANSISTOR_COUNT)//if max light value is not meaningful return an error
		return MOVE_NONE_LIGHT_SRC;
	MoveResult=DirectFrd(Pos);//move to the correct positoion at forward direction
	return MoveResult;
}//cMotionMngr::PositionMaxLightScrFrd


//   For current scaned state of obstacles in front of Wall-e finds all possible movement pats
//
//   input: ObstacleTable - table of obstacles scaned by Wall-e
//   output: coded paths - possible Wall-e movements
//           when bit is set path is allowed and possible
//
// Oct 9 2016 Original approach with 10 paths pssible was extended.
// When sectors A, B and C are mareked as occupied system can consider movements towards them
// that is because far segments can often detects obstacles because of some light noise
// Path coding on a WORD BITs is defined below 
//
//                     H is Wall-e position
// 
//       RIGHT                          LEFT   BIT0 -  path 1  E&B    Empty - move forward away
//                  +---+---+---+              BIT1 -  path 2  E&D&A  Empty - move forward away
//                  | A | B | C |              BIT2 -  path 3  E&F&C  Empty - move forward away
//                  +---+---+---+              BIT3 -  path 4  G&D&A  Empty - move froward away
//                  | D | E | F |              BIT4 -  path 5  I&F&C  Empty - move forward away
//                  +---+---+---+              BIT5 -  NOT USED
//                  | G | H | I |              BIT6 -  path 7  E      Empty - move forward inside (path 1 reuse)
//                  +---+---+---+              BIT7 -  path 8  G&D    Empty - move forward inside (path 4 reuse)
//                                             BIT8 -  path 9  I&F    Empty - move forward inside (path 5 reuse)
//                                             BIT9 -  path 10 G      Empty - move side away
//                                             BIT10 - path 11 I      Empty - move side away  
//                                             BIT11 - path 12 move left or right 180 degs
//                                             BIT12 - path 13 NOT USED
//                                             BIT13 - path 14 NOT USED
//                                             BIT14 - path 15 NOT USED
//                                             BIT15 - path 16 NOT USED
//
//
WORD cMotionMngr::FindMovePaths(BYTE ObstacleTbl[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE])
{
	WORD MovePaths=0;//start with none paths availiable
	//----------------------------------
	//check for path 1
	if ((!ObstacleTbl[1][1]) && (!ObstacleTbl[0][1]))//when E & B are empty
		MovePaths|=BIT0;//mark path 1 as possible
	//----------------------------------
	//check for path 2
	if ((!ObstacleTbl[1][1]) && (!ObstacleTbl[1][0]) && (!ObstacleTbl[0][0]))//when E & D & A are empty
		MovePaths|=BIT1;//mark path 2 as possible
	//check for path 3
	if ((!ObstacleTbl[1][1]) && (!ObstacleTbl[1][2]) && (!ObstacleTbl[0][2]))//when E & F & C are empty
		MovePaths|=BIT2;//mark path 3 as possible
	//----------------------------------
	//check for path 4
	if ((!ObstacleTbl[2][0])&&(!ObstacleTbl[1][0]) && (!ObstacleTbl[0][0]))//when G & D & A are empty
		MovePaths|=BIT3;//mark path 4 as possible
	//check for path 5
	if ((!ObstacleTbl[2][2])&&(!ObstacleTbl[1][2]) && (!ObstacleTbl[0][2]))//when I & F & C are empty
		MovePaths|=BIT4;//mark path 5 as possible
	//----------------------------------
	//BIT 5 and PATH 6 are not used
	
	//check for path 7
	if ((!ObstacleTbl[1][1]))//when E is empty
			MovePaths|=BIT6;//mark path 7 as possible
	//check for path 8
	if ((!ObstacleTbl[2][0])&&(!ObstacleTbl[1][0]))//when G & D are empty
		MovePaths|=BIT7;//mark path 8 as possible
	//check for path 9
	if ((!ObstacleTbl[2][2])&&(!ObstacleTbl[1][2]))//when I & F are empty
		MovePaths|=BIT8;//mark path 9 as possible
	//check for path 10
	if ((!ObstacleTbl[2][0]))//when G is empty
		MovePaths|=BIT9;//mark path 10 as possible
	//check for path 11
	if ((!ObstacleTbl[2][2]))//when I is empty
		MovePaths|=BIT10;//mark path 11 as possible
	//----------------------------------
	//path 12 always for consideration
	MovePaths|=BIT11;//mark path 8 as possible
	return MovePaths;
} //cMotionMngr::FindMovePaths()

//move according to path 1  DistanceToMove steps
// Path1=E&B
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath1(WORD DistanceToMove)
{
	return MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
} //MotionMngr::MoveOnPath1

//move according to path 2  DistanceToMove steps
// Path2=E&D&A
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath2(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	WORD CoveredDistance=0;//distance covered so far
	
	//move forward but no more just to sector E
	if(DistanceToMove > SECTOR_LENGTH_COUNTS)
		MoveResult=MoveForward(SECTOR_LENGTH_COUNTS,LOW_MID_HIGH_PROFILE);
	else 
		MoveResult=MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by the move
	if(MoveResult)//when error break movement
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//when movement is completed because we covered desired distance
	if(CoveredDistance>=DistanceToMove)
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;//when nothing more to move return
	}
	//turn to move farther to sector D
	MoveResult=TurnRight90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//move only to sector D no more
	if((DistanceToMove-CoveredDistance) > SECTOR_LENGTH_COUNTS)
		MoveResult=MoveForward(SECTOR_LENGTH_COUNTS,LOW_MID_HIGH_PROFILE);
	else 
		MoveResult=MoveForward((DistanceToMove-CoveredDistance),LOW_MID_HIGH_PROFILE);

	CoveredDistance+=GetCoveredDistance();//get actually covered distance by move
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}		
	if(CoveredDistance>=DistanceToMove)
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;//when nothing more to move return
	}
	//turn to continue towards sector A
	MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	MoveResult=MoveForward((DistanceToMove-CoveredDistance),LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by move
	SetCoveredDistance(CoveredDistance);
	return MoveResult;
} //cMotionMngr::MoveOnPath2

//move according to path 3  DistanceToMove steps
// Path3=E&F&C
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath3(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	WORD CoveredDistance=0;//distance covered so far
	
	//move forward but no more just to sector E
	if(DistanceToMove > SECTOR_LENGTH_COUNTS)
		MoveResult=MoveForward(SECTOR_LENGTH_COUNTS,LOW_MID_HIGH_PROFILE);
	else 
		MoveResult=MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by the move
	if(MoveResult)//when error break movement
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//when movement is completed because we covered desired distance
	if(CoveredDistance>=DistanceToMove)
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;//when nothing more to move return
	}
	//turn to move farther to sector D
	MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//move only to sector D no more
	if((DistanceToMove-CoveredDistance) > SECTOR_LENGTH_COUNTS)
		MoveResult=MoveForward(SECTOR_LENGTH_COUNTS,LOW_MID_HIGH_PROFILE);
	else 
		MoveResult=MoveForward((DistanceToMove-CoveredDistance),LOW_MID_HIGH_PROFILE);

	CoveredDistance+=GetCoveredDistance();//get actually covered distance by move
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}		
	if(CoveredDistance>=DistanceToMove)
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;//when nothing more to move return
	}
	//turn to continue towards sector A
	MoveResult=TurnRight90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	MoveResult=MoveForward((DistanceToMove-CoveredDistance),LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by move
	SetCoveredDistance(CoveredDistance);
	return MoveResult;	
} //cMotionMngr::MoveOnPath3

//move according to path 4  DistanceToMove steps
// Path3=G&D&A
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath4(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	WORD CoveredDistance=0;//distance covered so far
	
	
	MoveResult=TurnRight90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(0);// kust turned and nothing more
		return MoveResult;
	}
	
	//move forward but no more just to sector E
	if(DistanceToMove > SECTOR_LENGTH_COUNTS)
		MoveResult=MoveForward(SECTOR_LENGTH_COUNTS,LOW_MID_HIGH_PROFILE);
	else 
		MoveResult=MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by the move
	if(MoveResult)//when error break movement
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//when movement is completed because we covered desired distance
	if(CoveredDistance>=DistanceToMove)
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;//when nothing more to move return
	}
	//turn to move farther to sector D
	MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//move to sector D & A if required
	MoveResult=MoveForward((DistanceToMove-CoveredDistance),LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by move
	SetCoveredDistance(CoveredDistance);
	return MoveResult;
} //cMotionMngr::MoveOnPath4


//move according to path 5  DistanceToMove steps
// Path5=I&F&C
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath5(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	WORD CoveredDistance=0;//distance covered so far
	
	
	MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(0);// kust turned and nothing more
		return MoveResult;
	}
	
	//move forward but no more just to sector E
	if(DistanceToMove > SECTOR_LENGTH_COUNTS)
		MoveResult=MoveForward(SECTOR_LENGTH_COUNTS,LOW_MID_HIGH_PROFILE);
	else 
		MoveResult=MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by the move
	if(MoveResult)//when error break movement
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//when movement is completed because we covered desired distance
	if(CoveredDistance>=DistanceToMove)
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;//when nothing more to move return
	}
	//turn to move farther to sector D
	MoveResult=TurnRight90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(CoveredDistance);
		return MoveResult;
	}
	//move to sector F & C if required
	MoveResult=MoveForward((DistanceToMove-CoveredDistance),LOW_MID_HIGH_PROFILE);
	CoveredDistance+=GetCoveredDistance();//get actually covered distance by move
	SetCoveredDistance(CoveredDistance);
	return MoveResult;
} //cMotionMngr::MoveOnPath5

//MoveOnPath6 - Path 6 is not used
//MoveOnPath7 we call MoveOnPath1 and assume stop at an obstacle 
//MoveOnPath8 we call MoveOnPath4 and assume stop at an obstacle
//MoveOnPath9 we call MoveOnPath5 and assume stop at an obstacle

//move according to path 10  DistanceToMove steps
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath10(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	
	MoveResult=TurnRight90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(0);
		return MoveResult;
	}
	return MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
} //MotionMngr::MoveOnPath10

//move according to path 11  DistanceToMove steps
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath11(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	
	MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(0);
		return MoveResult;
	}
	return MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
} //MotionMngr::MoveOnPath11

//move according to path 12  DistanceToMove steps
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnPath12(WORD DistanceToMove)
{
	BYTE MoveResult;//result of movement function
	BYTE TurnDirection;//0-using left turn 1 using right turn
	
	srand(GetTimer3CounterValue());//intialize seed for random generator to make decisions even more random
	TurnDirection=rand()%2;
	
	if(TurnDirection)
		MoveResult=TurnRight90Deg();
	else
		MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(0);
		return MoveResult;
	}
	Kernel.TimeDlyHMSM(0,0,0,500);//delay required to stabilize after first move to eliminate ARS errors for 2nd move
	if(TurnDirection)
		MoveResult=TurnRight90Deg();
	else
		MoveResult=TurnLeft90Deg();
	if(MoveResult)//when error
	{
		SetCoveredDistance(0);
		return MoveResult;
	}
	return MoveForward(DistanceToMove,LOW_MID_HIGH_PROFILE);
} //MotionMngr::MoveOnPath12


//select one from possible movement paths and move
//returns: MOVE_OK=0 or error code
BYTE cMotionMngr::MoveOnRandPath(WORD DistanceToMove, WORD PossiblePaths, WORD AllowedPaths)
{
	BYTE Path=0;//keeps possible alternative from the two randomly selected
	BYTE MoveResult;//result of movement
	srand(GetTimer3CounterValue());//intialize seed for random generator to make decisions even more random
	
	PossiblePaths=PossiblePaths&AllowedPaths;//select only allowed paths from the all possible
	
	if(BIT0&PossiblePaths)//when path 1
	{
		PossiblePaths=0;
		PossiblePaths|=BIT0;//only path 1 is possible then
	}
	
	if((BIT1&PossiblePaths)&&(BIT2&PossiblePaths))//when path 2 and path 3 possible
	{//make one from possible selected
		Path=rand()%2;//select path by random (0- path3, 1 - path2)
		if(Path)//when path 2 selected make it only possible
		{
			PossiblePaths=0;
			PossiblePaths|=BIT1;//only path 2 is possible
		}
		else //when path 3 selected make it only possible
		{
			PossiblePaths=0;
			PossiblePaths|=BIT2;//only path 3 is possible
			
		}
	}
	else
	{
		if(BIT1&PossiblePaths) //when path 2 only avaliable
		{
			PossiblePaths=0;
			PossiblePaths|=BIT1;//only path 2 is possible then	
		}
		if(BIT2&PossiblePaths) //when path 3 only avaliable
		{
			PossiblePaths=0;
			PossiblePaths|=BIT2;//only path 3 is possible then	
		}
		
	}
	
	if((BIT3&PossiblePaths)&&(BIT4&PossiblePaths))//when path 4 and path 5 possible
	{//make one from possible selected
		Path=rand()%2;//select path by random (0- path5, 1 - path4)
		if(Path)//when path 4 selected
		{
			PossiblePaths=0;
			PossiblePaths|=BIT3;//only path 4 is possible then
		}
		else //when path 5 selected 
		{
			PossiblePaths=0;
			PossiblePaths|=BIT4;//only path 5 is possible then
		}
			
	}else
	{
		if(BIT3&PossiblePaths)//when path 4 avaliable
		{
			PossiblePaths=0;
			PossiblePaths|=BIT3;//only path 4 is possible then
		}
		if(BIT4&PossiblePaths) //when path 5 avaliable
		{
			PossiblePaths=0;
			PossiblePaths|=BIT4;//only path 5 is possible then
		}
	}
	//BIT5 and corresponding PATH6 does not exists
	
	
	if(BIT6&PossiblePaths)//when path 7 available
	{
		PossiblePaths=0;
		PossiblePaths|=BIT6;//only path 7 is possible then
	}
	
	if((BIT7&PossiblePaths)&&(BIT8&PossiblePaths))//when path 8 and path 9 possible
	{//make one from possible selected
		Path=rand()%2;//select path by random (0- path9, 1 - path8)
		if(Path)//when path 8 selected 
		{
			PossiblePaths=0;
			PossiblePaths|=BIT7;//only path 8 is possible then		
		}
		else //when path 9 selected
		{
			PossiblePaths=0;
			PossiblePaths|=BIT8;//only path 8 is possible then
		}
	}else
	{
		if(BIT7&PossiblePaths)//when path 8 possible
		{
			PossiblePaths=0;
			PossiblePaths|=BIT7;//only path 8 is possible then
			
	 	}
		if(BIT8&PossiblePaths)//when path 9 possible
		{
			PossiblePaths=0;
			PossiblePaths|=BIT8;//only path 9 is possible then
					
		}
	}
	
	if((BIT9&PossiblePaths)&&(BIT10&PossiblePaths))//when path 8 and path 9 possible
	{//make one from possible selected
		Path=rand()%2;//select path by random (0- path11, 1 - path10)
		if(Path)//when path 10 selected
		{
			PossiblePaths=0;
			PossiblePaths|=BIT9;//only path 10 is possible then		
		}
		else //when path 11 selected
		{
			PossiblePaths=0;
			PossiblePaths|=BIT10;//only path 11 is possible then		
		}
	}else
	{
		if(BIT9&PossiblePaths)//when path 10 is possible
		{
			PossiblePaths=0;
			PossiblePaths|=BIT9;//only path 10 is possible then
			
		}
		if(BIT10&PossiblePaths)//when path 11 is possible
		{
			PossiblePaths=0;
			PossiblePaths|=BIT10;//only path 11 is possible then
		
		}
	}
	
	MoveResult=MOVE_NONE_PATH;//assume at the beigining none movement is possible
	//make movements following avaliable path
	if(BIT0&PossiblePaths)
	{
		MoveResult=MoveOnPath1(DistanceToMove);//when path1 is allowed
	}else if (BIT1&PossiblePaths)
	{
		MoveResult=MoveOnPath2(DistanceToMove);//when path2 is allowed
	}else if (BIT2&PossiblePaths)
	{
		MoveResult=MoveOnPath3(DistanceToMove);//when path3 is allowed
	}else if (BIT3&PossiblePaths)
	{
		MoveResult=MoveOnPath4(DistanceToMove);//when path4 is allowed
	}else if (BIT4&PossiblePaths)
	{
		MoveResult=MoveOnPath5(DistanceToMove);//when path5 is allowed
	}
	//BIT5 and PATH6 are not used
	else if (BIT6&PossiblePaths)
	{
		MoveResult=MoveOnPath1(DistanceToMove);//for path7 we reuse Path1 movement
	}else if (BIT7&PossiblePaths)
	{
		MoveResult=MoveOnPath4(DistanceToMove);//when path8 we reuse Path4
	}else if (BIT8&PossiblePaths)
	{
		MoveResult=MoveOnPath5(DistanceToMove);//when path9 we reuse Path5
	}else if (BIT9&PossiblePaths)
	{
		MoveResult=MoveOnPath10(DistanceToMove);//when path 9 is allowed
	}  else if (BIT10&PossiblePaths)
	{
		MoveResult=MoveOnPath11(DistanceToMove);
	}  else if (BIT11&PossiblePaths)
	{
		MoveResult=MoveOnPath12(DistanceToMove);
	}     
	return MoveResult;
} //cMotionMngr::MoveOnRandPath

//execute manager CMD_MOVE command
//returns:
//       nothing but internally RSP_MOVE notifier is posted
void cMotionMngr::ProcessCmdMove(cSmartPtr<cNotifier> pNotifier)
{
	mMoveData=*(static_cast<sMoveData*>(pNotifier->GetDataPtr()));//copy notifier data to mMoveData
	switch(mMoveData.mMoveCmdId)//depending on sub-command type execute proper movement
	{
	case FORWARD_SCMD_ID://move forward
		mMoveData.mResult=Move(mMoveData.mDistancePulses,MOTOR_FORWARD,mMoveData.mSpeedProfile);
		break;
	case REVERSE_SCMD_ID: //move backwards
		mMoveData.mResult=Move(mMoveData.mDistancePulses,MOTOR_REVERSE,mMoveData.mSpeedProfile);
		break;
	case LEFT90DEG_SCMD_ID://turn left exactly 90 degs
		mMoveData.mResult=TurnLeft90Deg();
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	case TURN_LEFT_SCMD_ID: //turn left specified degree
		mMoveData.mResult=TurnLeftTo90Deg(mMoveData.mDistancePulses);//turn left specified degree
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	case RIGHT90DEG_SCMD_ID: //turn right exactly 90 degs
		mMoveData.mResult=TurnRight90Deg();
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	case TURN_RIGHT_SCMD_ID://turn right specified degree
		mMoveData.mResult=TurnRightTo90Deg(mMoveData.mDistancePulses);//turn right specified degree
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	case RAND_TURN_SCMD_ID: //random left or right 90 degrees turn
		mMoveData.mResult=RandLeftOrRightTurn();
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	case RAND_PATH_MOVE_SCMD_ID://move according to rundomly slected path based on curent obstacle scanning results
		ScanObstacleLightSrc(ObstacleTable,LightSrcTable);
 		IdentifiedPossiblePaths=FindMovePaths(ObstacleTable);
 		//last parameter in the call for MovOnRandPath contains allowed paths
 		mMoveData.mResult=MoveOnRandPath(mMoveData.mDistancePulses,IdentifiedPossiblePaths,mMoveData.mSpeedProfile);
 		//for RAND_PATH_MOVE mMoveData.mSpeedProfile conatins BIT codded allowed paths for the movement 
		break;
	case FIND_MAX_LIGHT_SRC_SCMD_ID: //scan all surroundings (0-360)and position Wall-e at the most intensive light source detected
		mMoveData.mResult=FindMaxLightScr();
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	case POSITION_AT_MAX_LIGHT_FRD_SCMD_ID://position at strongest light in forward direction
		ScanLightSrc(LightSrcTable);//scan forward direction to get LightSrcTable updated with light info
		mMoveData.mResult=PositionMaxLightScrFrd();
		mCoveredDistancePulses=0;//for turn we do not take covered distance pulses into account at all
		mMoveData.mSpeedProfile=HIGH_PROFILE;//turn is done always on the high speed profile
		break;
	default:
		break;
	}//switch(mMoveData.mMoveCmdId)
	
	cSmartPtr<cTypeNotifier<sMoveData> > pRspNotifier = new cTypeNotifier<sMoveData>(RSP_MOVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mMoveCmdId=mMoveData.mMoveCmdId;//RSP_MOVE contains Id of the executed CMD_MOVE subcommand
	(pRspNotifier->GetData()).mDistancePulses=mCoveredDistancePulses;//get info about finally covered pulses
	(pRspNotifier->GetData()).mSpeedProfile=mMoveData.mSpeedProfile;//get used speed profile
	(pRspNotifier->GetData()).mResult=mMoveData.mResult;//provide command execution result
	Post(pRspNotifier);//post RSP_MOVE response	
}//cMotionMngr::ProcessCmdMove

	

//execute manager CMD_SCAN command
//returns:
//       nothing but internally RSP_SCAN notifier is posted with obstacle table and ligh source table
void cMotionMngr::ProcessCmdScan(cSmartPtr<cNotifier> pNotifier)
{
	mScanData=*(static_cast<sScanData*>(pNotifier->GetDataPtr()));//copy notifier data to mScanData storage
	switch(mScanData.mScanDataCmdId)//depending on sub-command type execute proper movement
	{
	case SCAN_LIGHT_OBSTACLE_SCMD_ID://scan at same time for light source and obstacle (from L60 to R60)
	case SCAN_OBSTACLE_SCMD_ID://scan only for obstacle (from L60 to R60) - both commands use same function de facto and are completely equal
		ScanObstacleLightSrc(ObstacleTable,LightSrcTable);
		break;
	case SCAN_LIGHT_SCMD_ID://scan only for light source (from L60 to R60) - IRED sensor is not used
		ScanLightSrc(LightSrcTable);
		break;
	default:
		break;
	}//switch
	
	cSmartPtr<cTypeNotifier<sScanData> > pRspNotifier = new cTypeNotifier<sScanData>(RSP_SCAN,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mScanDataCmdId=mScanData.mScanDataCmdId;//RSP_SCAN contains Id of the executed CMD_SCAN subcommand
	//copy scaned information to the notifier
	((pRspNotifier->GetData()).mObstacleTable)[0][0]=ObstacleTable[0][0];
	((pRspNotifier->GetData()).mObstacleTable)[0][1]=ObstacleTable[0][1];
	((pRspNotifier->GetData()).mObstacleTable)[0][2]=ObstacleTable[0][2];
	((pRspNotifier->GetData()).mObstacleTable)[1][0]=ObstacleTable[1][0];
	((pRspNotifier->GetData()).mObstacleTable)[1][1]=ObstacleTable[1][1];
	((pRspNotifier->GetData()).mObstacleTable)[1][2]=ObstacleTable[1][2];
	((pRspNotifier->GetData()).mObstacleTable)[2][0]=ObstacleTable[2][0];
	((pRspNotifier->GetData()).mObstacleTable)[2][1]=ObstacleTable[2][1];
	((pRspNotifier->GetData()).mObstacleTable)[2][2]=ObstacleTable[2][2];
	
	//copy light sorce scan information
	((pRspNotifier->GetData()).mLightSrcTable)[FRD_IDX]=LightSrcTable[FRD_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[L15_IDX]=LightSrcTable[L15_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[L30_IDX]=LightSrcTable[L30_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[L45_IDX]=LightSrcTable[L45_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[L60_IDX]=LightSrcTable[L60_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[R15_IDX]=LightSrcTable[R15_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[R30_IDX]=LightSrcTable[R30_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[R45_IDX]=LightSrcTable[R45_IDX];
	((pRspNotifier->GetData()).mLightSrcTable)[R60_IDX]=LightSrcTable[R60_IDX];
	
	
	Post(pRspNotifier);//post RSP_SCAN response	
}//cMotionMngr::ProcessCmdScan

//execute manager CMD_CHECK_OBSTACLE command
//returns:
//       nothing but internally RSP_CHECK_OBSTACLE notifier is posted
void cMotionMngr::ProcessCmdCheck(cSmartPtr<cNotifier> pNotifier)
{
	WORD RawData=0;//temp storage for sensors raw data
	WORD RawUsData=0;//temp storage for US data
	BYTE Type;//temporary to store type of detected light or obstacle
	
	mCheckData=*(static_cast<sCheckData*>(pNotifier->GetDataPtr()));//copy notifier data to mCheckData storage
	switch(mCheckData.mCheckDataCmdId)//depending on sub-command type execute proper movement
	{
	case CHECK_OBSTACLE_SCMD_ID://check data from fusion of IRED and US detected obstacle
			Type=DistanceSensorsFusion(CheckForObstacleUS(&RawUsData), CheckForObstacleIRED(&RawData));
		break;
	case CHECK_IRED_OBSTACLE_SCMD_ID://check for detected obstacle using IRED sensor
			Type=CheckForObstacleIRED(&RawData);
		break;
	case CHECK_US_OBSTACLE_SCMD_ID://check for detected obstacle using US sensor
			Type=CheckForObstacleUS(&RawUsData);
		break;
	case CHECK_FRONT_LIGHT_SCMD_ID://check for foto nose data
			Type=CheckNoseDayState(&RawData);
		break;
	case CHECK_BACK_LIGHT_SCMD_ID://check for foto tail data
			Type=CheckTailDayState(&RawData);
		break;
	default:
		break;
	}//switch
	
	cSmartPtr<cTypeNotifier<sCheckData> > pRspNotifier = new cTypeNotifier<sCheckData>(RSP_CHECK,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mCheckDataCmdId=mCheckData.mCheckDataCmdId;//executed sub command
	(pRspNotifier->GetData()).mType=Type;//check status of obstacles ahead curent head position
	(pRspNotifier->GetData()).mRawData = RawData;//read IRED raw data at the current position
	(pRspNotifier->GetData()).mRawUsData=RawUsData;//read US raw data if used
	Post(pRspNotifier);//post RSP_CHECK_OBSTACLE response
}//cMotionMngr::ProcessCmdCheck

//execute manager CMD_TURN_HEAD command
//returns:
//       nothing but internally RSP_TURN_HEAD notifier is posted
void cMotionMngr::ProcessCmdTurnHead(cSmartPtr<cNotifier> pNotifier)
{
	//check what sub command command execution is requested
	BYTE CmdId = (static_cast<sTurnHeadData*>(pNotifier->GetDataPtr()))->mTurnHeadCmdId;
	//get requested head position
	WORD TurnHeadPosition = (static_cast<sTurnHeadData*>(pNotifier->GetDataPtr()))->mTurnHeadPosition;
	
	switch(CmdId)
	{
	case TURN_HEAD_ON_SCMD_ID://head servo on requested
		HeadServoOn();//turn head servo on
		SmoothHeadMove(GetHeadPosition(),TurnHeadPosition);//move to destination position
		OSTimeDlyHMSM(0,0,0,HEAD_LONG_STABILIZATION_DELAY);//delay to assure head stabilization
		break;
	case TURN_HEAD_OFF_SCMD_ID://head servo off requested
		SmoothHeadMove(GetHeadPosition(),TurnHeadPosition);//move to destination position
		OSTimeDlyHMSM(0,0,0,HEAD_LONG_STABILIZATION_DELAY);//delay to assure head stabilization
		HeadServoOff();//turn head servo off
		break;
	case TURN_HEAD_MOVE_SCMD_ID://head movement is requested
		SmoothHeadMove(GetHeadPosition(),TurnHeadPosition);//move to destination position
		OSTimeDlyHMSM(0,0,0,HEAD_LONG_STABILIZATION_DELAY);//delay to assure head stabilization
		break;
	default:
		break;
	}//switch
	//send response once command executed	
	cSmartPtr<cTypeNotifier<sTurnHeadData> > pRspNotifier = new cTypeNotifier<sTurnHeadData>(RSP_TURN_HEAD,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mTurnHeadCmdId=CmdId;
	(pRspNotifier->GetData()).mTurnHeadPosition=TurnHeadPosition;
	Post(pRspNotifier);//post response
}//cMotionMngr::ProcessCmdTurnHead

//----------------------------------------------------------------------------------------------------------
// Motion Manager main execution function
void  cMotionMngr::Run()
{
    //run task which controls left track and handle track interrupt messages
    if(::OSTaskCreate(LeftTrackControlTask,NULL,&LeftTrackControlTaskStack[LEFT_TRACK_CTRL_STACK_SIZE-1],LEFT_TRACK_CTRL_TASK_PRIORITY)!= OS_NO_ERR)
    		THREAD_CREATE_EXCEPTION;
    
    //run task which controls left track and handle track interrupt messages
    if(::OSTaskCreate(RightTrackControlTask,NULL,&RightTrackControlTaskStack[RIGHT_TRACK_CTRL_STACK_SIZE-1],RIGHT_TRACK_CTRL_TASK_PRIORITY)!= OS_NO_ERR)
       		THREAD_CREATE_EXCEPTION;
    
 	for(;;)
	{
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		switch (pNotifier->GetNotifierId()) //execute manager coomand depending on the request
		{
		case CMD_MOVE://execute various Wall-e movements
			ProcessCmdMove(pNotifier);
			break;
		case CMD_SCAN:
			ProcessCmdScan(pNotifier);
			break;
		case CMD_CHECK:
			ProcessCmdCheck(pNotifier);
			break;
		case CMD_TURN_HEAD:
			ProcessCmdTurnHead(pNotifier);
			break;
		default://do nothing when unknown request received
			break;
		}//switch
	}
} //cMotionMngr::Run()
	

