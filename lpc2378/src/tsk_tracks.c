/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        tsk_tracks.c
* Description: Left & Right track control tasks 
* Author:      Bogdan Kowalczyk
* Date:        23-July-2011
* History:
* 23-July-2011 Initial version created
*********************************************************************************************************
*/
#include "os_cpu.h"
#include "os_cfg.h"
#include "tsk_tracks.h"
#include "hw_gpio.h"
#include "hw_timer.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors

//Variables used by transoptor pulses handling tasks to track down
//number of pulses generated until track stop
static volatile WORD InitLeftTrackPulses;//initial value of requested Left Track Pulses
static volatile WORD InitRightTrackPulses;//initial value of requested Right Track pulses

static volatile WORD TotalTrackPulses;//counter of total pulses counted for the single movement command
static volatile WORD EndTotalTrackPulses;//keep end value of TotalTrackPulses which are expected during the movement

static volatile WORD LeftTrackPulses;//counter of pulses for left track
static volatile WORD RightTrackPulses;//counter of pulses for right track

//identifier of the current range of speed the motors are working in
static volatile BYTE MotorSpeedRange;

//ID of task which is counting TotalTrackPulses variable
//can be either LEFT_TRACK_TSK or RIGHT_TRACK_TSK
static volatile BYTE TotalPulsesCountingTask;

//desired direction of motors during movement
static volatile BYTE LeftMotorDirection;//left motor desired direction
static volatile BYTE RightMotorDirection;//right motor desired direction

//variables which holds threshold values to change speed profile
//calculated by RunTracksPulses function and used by Port2IsrHandler to switch speeds during movement
// SEE  RunTracksPulses function to get understanding of those values
// First values of counts for LOW, MID and HIGH speed
static volatile BYTE LowRightSpeedCount;//value of counts for so named LOW speed
static volatile BYTE LowLeftSpeedCount;//value of counts for so named LOW speed

static volatile BYTE MidRightSpeedCount;//value of counts for so named MID speed
static volatile BYTE MidLeftSpeedCount;//value of counts for so named MID speed

static volatile BYTE HighRightSpeedCount;//value of counts for so named HIGH speed
static volatile BYTE HighLeftSpeedCount;//value of counts for so named HIGH speed

// Threshold values for the moments when total number of pulses switches speed
static volatile WORD StartLowPulses;//when switch to low speed on movement start
static volatile WORD StartMidPulses;//when switch to mid speed on movement start
static volatile WORD StartHighPulses;//when switch to high speed on movement start
static volatile WORD EndHighPulses;//when switch from high speed to mid speed on movement end
static volatile WORD EndMidPulses;//when switch from mid speed to low speed on movement end
static volatile WORD EndLowPulses; //when turn off motors on movement end

 
OS_STK LeftTrackControlTaskStack[LEFT_TRACK_CTRL_STACK_SIZE];//stack for left track control task
OS_STK RightTrackControlTaskStack[RIGHT_TRACK_CTRL_STACK_SIZE];//stack for right track control task

/*
*********************************************************************************************************
* Name:                                   IsTrackMoving
* 
* Description: Check if left or right track should moving to complete movement
*       
*
* Arguments:   none
*
* Returns:     True when track are moving False otherwise
*
* Note(s):     
* 
* *********************************************************************************************************
*/
int IsTrackMoving(void)
{
	return MotorSpeedRange; //when MotorSpeedRange is <> 0 means StopsTracks() was not called and trucks are run
//	return TotalTrackPulses != EndTotalTrackPulses;//removed when StopTracks does not clear pulses counters to zero
}// IsTrackMoving


/*
*********************************************************************************************************
* Name:                                    GetCurrentTrackPulses
* 
* Description: Get current number of track pulses
*       
*
* Arguments:   none
*
* Returns:     Value of TotalTrackPulses
*
* Note(s):     
* 
* *********************************************************************************************************
*/
WORD GetCurrentTrackPulses(void)
{
	return TotalTrackPulses;
}//GetCurrentTrackPulses

/*
*********************************************************************************************************
* Name:                                    GetRightTrackPulses
* 
* Description: Get current number of track pulses for right motor
*       
*
* Arguments:   none
*
* Returns:     Current value of right motor pulses
*
* Note(s):     
* 
* *********************************************************************************************************
*/
WORD GetRightTrackPulses(void)
{
	return RightTrackPulses;
}// GetRightTrackPulses

/*
*********************************************************************************************************
* Name:                                    GetLeftTrackPulses
* 
* Description: Get current number of track pulses for left motor
*       
*
* Arguments:   none
*
* Returns:     Current value of left motor pulses
*
* Note(s):     
* 
* *********************************************************************************************************
*/
WORD GetLeftTrackPulses(void)
{
	return LeftTrackPulses;
}//GetLeftTrackPulses

/*
*********************************************************************************************************
* Name:                                   StopTracks
* 
* Description: Whenever called stops motors of the left and right track
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
void StopTracks(void)
{
	//disable PORT2 interrupts to avoid simulatnous and not required access to shared variables
	IO2_INT_EN_F = 0x00000000;
	IO2_INT_EN_R = 0x00000000;
	
 	//stop motors just for the case they are working
	LeftMotorFastStop();//stop left motor
	RightMotorFastStop();//stop right motor
	MotorPowerOff();//turn off power of motors
	TransoptorOff();//turn off power of transoptors which detects movement pulses
	
	MotorSpeedRange=MOTOR_SPEED_OFF;//mark that motors are off
	
}//StopTracks

/*
*********************************************************************************************************
* Name:                                   InitPulseCounters
* 
* Description: Initializes all pulse counters to 0 before tracks movement starts
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
void InitPulseCounters(void)
{
	TotalTrackPulses = 0;//counter of total pulses counted for the single movement command
	LeftTrackPulses = 0;//counter of pulses for left track
	RightTrackPulses = 0;//counter of pulses for right track
	EndTotalTrackPulses = 0;//keep end value of TotalTrackPulses which are expected during the movement
	TotalPulsesCountingTask = LEFT_TRACK_TSK;//initial value later on right one will be defined	
}// InitPulsesCounters

/*
*********************************************************************************************************
* Name:                                  SetupLowMidHighProfile
* 
* Description: Initialize all tracking control variables to execute LOW_MID_HIGH_PROFILE
*              by the  RunTracksPulses
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Global variables for  RunTracksPulses functions are accordingly setup
* 
* *********************************************************************************************************
*/
static void SetupLowMidHighProfile(void)
{
	//set up low speed portion of the speed prifile
	StartLowPulses = LOW_SPEED_THRESHOLD;//when switch to low speed on movement start
	EndLowPulses = EndTotalTrackPulses; //when turn off motors on movement end
	LowRightSpeedCount=LOW_RIGHT_MOTOR_COUNT;//value of counts for so named LOW speed
	LowLeftSpeedCount=LOW_LEFT_MOTOR_COUNT;//value of counts for so named LOW speed

	//when number of total pulses sufficient for mid part of profile set it up
	if(EndTotalTrackPulses > (2*MID_SPEED_THRESHOLD+SPEED_SWITCH_THRESHOLD))
	{
		StartMidPulses = MID_SPEED_THRESHOLD;
		EndMidPulses = EndTotalTrackPulses-MID_SPEED_THRESHOLD;
		MidRightSpeedCount=MID_RIGHT_MOTOR_COUNT;//value of counts for so named MID speed
		MidLeftSpeedCount=MID_LEFT_MOTOR_COUNT;//value of counts for so named MID speed
	}
	else//setup threshold value so MID speed will never be turned on
	{
		StartMidPulses=MAX_PULSE_COUNT;//value behind range
		EndMidPulses=MAX_PULSE_COUNT;//value behind range
	}
	
	//when total number of pulses sufficient for high part of the profile set it up
	if(EndTotalTrackPulses > (2*HIGH_SPEED_THRESHOLD+SPEED_SWITCH_THRESHOLD))
	{
		StartHighPulses = HIGH_SPEED_THRESHOLD;
		EndHighPulses = EndTotalTrackPulses - HIGH_SPEED_THRESHOLD;
		HighRightSpeedCount = HIGH_RIGHT_MOTOR_COUNT;//value of counts for so named HIGH speed
		HighLeftSpeedCount = HIGH_LEFT_MOTOR_COUNT;//value of counts for so named HIGH speed
	}
	else//setup thresholds so HIGH speed will never be turned on
	{
		StartHighPulses = MAX_PULSE_COUNT;//value behind range
		EndHighPulses = MAX_PULSE_COUNT;//value behind range
	}	
}//SetupLowMidHighProfile


/*
*********************************************************************************************************
* Name:                                  SetupLowMidProfile
* 
* Description: Initialize all tracking control variables to execute LOW_MID_PROFILE
*              by the  RunTracksPulses
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Global variables for  RunTracksPulses functions are accordingly setup
* 
* *********************************************************************************************************
*/
static void SetupLowMidProfile(void)
{
	//set up low speed portion of the speed prifile
	StartLowPulses = LOW_SPEED_THRESHOLD;//when switch to low speed on movement start
	EndLowPulses = EndTotalTrackPulses; //when turn off motors on movement end
	LowRightSpeedCount=LOW_RIGHT_MOTOR_COUNT;//value of counts for so named LOW speed
	LowLeftSpeedCount=LOW_LEFT_MOTOR_COUNT;//value of counts for so named LOW speed
	
	//when number of total pulses sufficient for mid part of profile set it up
	if(EndTotalTrackPulses > (2*MID_SPEED_THRESHOLD+SPEED_SWITCH_THRESHOLD))
	{
		StartMidPulses = MID_SPEED_THRESHOLD;
	    EndMidPulses = EndTotalTrackPulses-MID_SPEED_THRESHOLD;
		MidRightSpeedCount=MID_RIGHT_MOTOR_COUNT;//value of counts for so named MID speed
		MidLeftSpeedCount=MID_LEFT_MOTOR_COUNT;//value of counts for so named MID speed
	}
	else//setup threshold value so MID speed will never be turned on
	{
		StartMidPulses=MAX_PULSE_COUNT;//value behind range
		EndMidPulses=MAX_PULSE_COUNT;//value behind range
	}
	
	//Setup thresholds so high pulse count is not possible
	StartHighPulses = MAX_PULSE_COUNT;//value behind range
	EndHighPulses = MAX_PULSE_COUNT;//value behind range	
}//SetupLowMidProfile

/*
*********************************************************************************************************
* Name:                                  SetupLowProfile
* 
* Description: Initialize all tracking control variables to execute LOW_PROFILE
*              by the  RunTracksPulses
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Global variables for  RunTracksPulses functions are accordingly setup
* 
* *********************************************************************************************************
*/
static void SetupLowProfile(void)
{
	//set up low speed portion of the speed prifile
	StartLowPulses = LOW_SPEED_THRESHOLD;//when switch to low speed on movement start
	EndLowPulses = EndTotalTrackPulses; //when turn off motors on movement end
	LowRightSpeedCount=LOW_RIGHT_MOTOR_COUNT;//value of counts for so named LOW speed
	LowLeftSpeedCount=LOW_LEFT_MOTOR_COUNT;//value of counts for so named LOW speed
	
	//setup MID and HIGH thresholds so they are not possible to occure during the movement
	StartMidPulses=MAX_PULSE_COUNT;//value behind range
	EndMidPulses=MAX_PULSE_COUNT;//value behind range
	StartHighPulses = MAX_PULSE_COUNT;//value behind range
	EndHighPulses = MAX_PULSE_COUNT;//value behind range	
}//SetupLowProfile



/*
*********************************************************************************************************
* Name:                                  SetupMidProfile
* 
* Description: Initialize all tracking control variables to execute MID_PROFILE
*              by the  RunTracksPulses
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Global variables for  RunTracksPulses functions are accordingly setup
* 
* *********************************************************************************************************
*/
static void SetupMidProfile(void)
{
	//start with MID profile and keep only it
	StartLowPulses = LOW_SPEED_THRESHOLD;//when switch to low speed on movement start
	EndLowPulses = EndTotalTrackPulses; //when turn off motors on movement end
	LowRightSpeedCount=MID_RIGHT_MOTOR_COUNT;//value of counts for so named LOW speed
	LowLeftSpeedCount=MID_LEFT_MOTOR_COUNT;//value of counts for so named LOW speed
	
	//setu threshold so MID speed change is not possible
	StartMidPulses=MAX_PULSE_COUNT;//value behind range
	EndMidPulses=MAX_PULSE_COUNT;//value behind range
	//setup threshold so HIGH speed change is not possible
	StartHighPulses = MAX_PULSE_COUNT;//value behind range
	EndHighPulses = MAX_PULSE_COUNT;//value behind range	
}//SetupMidProfile



/*
*********************************************************************************************************
* Name:                                  SetupHighProfile
* 
* Description: Initialize all tracking control variables to execute HIGH_PROFILE
*              by the  RunTracksPulses
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Global variables for  RunTracksPulses functions are accordingly setup
* 
* *********************************************************************************************************
*/
static void SetupHighProfile(void)
{
	//set up speed profile so only HIGH profile is used
	StartLowPulses = LOW_SPEED_THRESHOLD;//when switch to low speed on movement start
	EndLowPulses = EndTotalTrackPulses; //when turn off motors on movement end
	LowRightSpeedCount=HIGH_RIGHT_MOTOR_COUNT;//value of counts for so named LOW speed
	LowLeftSpeedCount=HIGH_LEFT_MOTOR_COUNT;//value of counts for so named LOW speed
	
	//setu threshold so MID speed change is not possible
	StartMidPulses=MAX_PULSE_COUNT;//value behind range
	EndMidPulses=MAX_PULSE_COUNT;//value behind range
	//setup threshold so HIGH speed change is not possible
	StartHighPulses = MAX_PULSE_COUNT;//value behind range
	EndHighPulses = MAX_PULSE_COUNT;//value behind range	
}//SetupHighProfile


/*
*********************************************************************************************************
* Name:                                     RunTracksPulses 
* 
* Description: Run left and right track specified direction, according to specified speed profile
*
* Arguments:   
* 		InLeftPulses - desired number of track pulses for left track
* 		InLeftDirection - direction of left track (MOTOR_FORWARD, MOTOR_REVERSE)
* 		InRightPulses - desired number of track pulses for right track
* 		InRightDirection - direction of right track (MOTOR_FORWARD, MOTOR_REVERSE)
* 		InSpeedProfile - desired speed profile (DEFAULT_SPEED_PROFILE, LOW_SPEED_PROFILE, MID_SPEED_PROFILE)
*
* Returns:     none
*
* Note(s):    
* 		InLeftPulses must be < MAX_PULSE_COUNT and  InRightPulses must be < MAX_PULSE_COUNT
* 
* Description:
* 	Basic 	LOW_MID_HIGH_PROFILE speed profile depicted below is used to generate all other possible spped profiles
*          
*          
*             
*          ^ Speed
*          |
*          |
*          |
*          |                          MOTOR_SPEED_HIGH_START
*  HIGH    |                      +-------------------------------+
*         MOTOR_SPEED_MID_START   |                               |
*          |                      |                               |MOTOR_SPEED_MID_END
*  MID     |              +-------+                               +-------+                         
*   MOTOR_SPEED_LOW_START |                                               |
*          |              |                                               |MOTOR_SPEED_LOW_END
*  LOW     |      +-------+                                               +-------+
*          |      |                                                               |     MOTOR_SPEED_OFF
*          +------+-------|-------|-------------------------------|-------|-------+---------> 
*              StartLow StartMid StartHigh                     EndHigh  EndMid  EndLow     TotalTrackPulses 
*               Pulses   Pulses  Pulses                        Pulses    Pulses Pulses
*          
* *********************************************************************************************************
*/
void RunTracksPulses(WORD InLeftPulses, BYTE InLeftDirection, WORD InRightPulses, BYTE InRightDirection,BYTE InSpeedProfile)
{
	StopTracks();//stop motors just for the case they are running and disable track interrupts
	InitPulseCounters();//initialize all
	if(InLeftPulses >= InRightPulses)//define number of total end pulses and task which counts them
	{
		EndTotalTrackPulses = InLeftPulses;
		TotalPulsesCountingTask = LEFT_TRACK_TSK;
	}else
	{
		EndTotalTrackPulses =InRightPulses;
		TotalPulsesCountingTask = RIGHT_TRACK_TSK;
	}
	if(EndTotalTrackPulses>=MAX_PULSE_COUNT)return;//cannot execute command if exceeded max number of pulses allowed
	//initialise pulses counters
	LeftTrackPulses = InLeftPulses;//counter of pulses for left track
	InitLeftTrackPulses = InLeftPulses;//initial value of requested Left Track Pulses
	RightTrackPulses = InRightPulses;//counter of pulses for right track
	InitRightTrackPulses = InRightPulses;//initial value of requested Right Track pulses
	
	//setup desired directions for left and for right motors
	if(InLeftDirection==MOTOR_FORWARD)//if left motor forward
	{
		LeftMotorForward();
		LeftMotorDirection=MOTOR_FORWARD;
	}
	else//when left motor reverse
	{
		LeftMotorReverse();
		LeftMotorDirection=MOTOR_REVERSE;
	}
	if(InRightDirection==MOTOR_FORWARD)//if right motor forward
	{
		RightMotorForward();
		RightMotorDirection=MOTOR_FORWARD;
	}
	else//when right motor reverse
	{
		RightMotorReverse();
		RightMotorDirection=MOTOR_REVERSE;
	}
	
	//setup profiles of the speed
	//when same movement for tracks requested speed profile can be applied
	if(InLeftPulses==InRightPulses)
	{
	//setup speed profile
	switch(InSpeedProfile)
		{
		case LOW_MID_HIGH_PROFILE://setup LOW-MID-HIGH speed profile
			SetupLowMidHighProfile();
			break;
			
		case LOW_MID_PROFILE://setup LOW-MID speed profile
			SetupLowMidProfile();
			break;
			
		case LOW_PROFILE://only LOW speed profile and no more
			SetupLowProfile();
			break;
			
		case MID_PROFILE://only MID speed profile no more
			SetupMidProfile();
			break;
			
		case HIGH_PROFILE://only HIGH speed profile no more
			SetupHighProfile();
			break;
		}//switch(InSpeedProfile)
	}
	else //in case that left track movement is different to right track movement only unswitchable profiles can be used
	{
		//setup speed profile
		switch(InSpeedProfile)
			{
			case LOW_PROFILE://only LOW speed profile and no more
				SetupLowProfile();
				break;
				
			case MID_PROFILE://only MID speed profile no more
				SetupMidProfile();
				break;
				
			case HIGH_PROFILE://only HIGH speed profile no more
				SetupHighProfile();
				break;
				
			default://any other profile always results in LowProfile 
				SetupLowProfile();
				break;
			}//switch(InSpeedProfile)
	}//else
	//setup initial speed for motors
	SetTimer3MR0Count(LowRightSpeedCount);//right motor with initial speed
	SetTimer3MR1Count(LowLeftSpeedCount);//left motor with initial speed
	MotorSpeedRange=MOTOR_SPEED_LOW_START;//mark that low speed is just setup at speed profile start
	
	//run motors as setup
	TransoptorOn();//turn on power of transoptors
	OSTimeDly(TRANSOPTOR_ON_DELAY);//wait until transoptor circuit is stabilized
	MotorPowerOn();//turn on power of the track motors
	//enable all possible Port2 interruption falling edge of P2.6(right motor) and P2.7(left motor)
	IO2_INT_EN_F|=BIT6|BIT7; 
}// RunTracksPulses

/*
******************************************************************************************************
* Name:                                   ChangeSpeedProfile
* 
* Description: Checks if speed profile should be changed (depending on counted TotalTrackPulses)
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Global variables for track controls are used during the check
* 
* *********************************************************************************************************
*/	
static void ChangeSpeedProfile(void)
{
	//when all desired total pulses counted stop the tracks
	if(TotalTrackPulses >= EndLowPulses)//if we should stop at movement end
	{
		StopTracks();//inside this function MotorSpeedRange=MOTOR_SPEED_OFF
	}
	//if we should switch to low speed at movement end make the switch
	else if((TotalTrackPulses >= EndMidPulses)&&(TotalTrackPulses < EndLowPulses) &&(MotorSpeedRange!=MOTOR_SPEED_LOW_END))
	{
		SetTimer3MR0Count(LowRightSpeedCount);//right motor speed
		SetTimer3MR1Count(LowLeftSpeedCount);//left motor speed
		MotorSpeedRange=MOTOR_SPEED_LOW_END;//mark motors are at low speed for end of speed profile
	}
	//if we shoud switch to mid speed at the movement end
	else if ((TotalTrackPulses >= EndHighPulses)&&(TotalTrackPulses<EndMidPulses)&&(MotorSpeedRange!=MOTOR_SPEED_MID_END))
	{
		SetTimer3MR0Count(MidRightSpeedCount);//right motor speed
		SetTimer3MR1Count(MidLeftSpeedCount);//left motor speed
		MotorSpeedRange=MOTOR_SPEED_MID_END;//mark motors are at mid speed for end of speed profile
	}
	//if we should switch to high speed
	else if ((TotalTrackPulses >= StartHighPulses)&&(TotalTrackPulses<EndHighPulses)&&(MotorSpeedRange!=MOTOR_SPEED_HIGH_START))
	{
		SetTimer3MR0Count(HighRightSpeedCount);//right motor speed
		SetTimer3MR1Count(HighLeftSpeedCount);//left motor speed
		MotorSpeedRange=MOTOR_SPEED_HIGH_START;//mark motors are at high speed for start of speed profile
	}
	//if we should switch to mid speed at the movement start
	else if ((TotalTrackPulses >= StartMidPulses)&&(TotalTrackPulses<StartHighPulses)&&(MotorSpeedRange!=MOTOR_SPEED_MID_START))
	{
		SetTimer3MR0Count(MidRightSpeedCount);//right motor speed
		SetTimer3MR1Count(MidLeftSpeedCount);//left motor speed
		MotorSpeedRange=MOTOR_SPEED_MID_START;//mark motors are at mid speed for start of speed profile
	}
	//if we should switch to low motor speed
	else if((TotalTrackPulses >= StartLowPulses)&&(TotalTrackPulses<StartMidPulses)&&(MotorSpeedRange!=MOTOR_SPEED_LOW_START))
	{
		SetTimer3MR0Count(LowRightSpeedCount);//right motor speed
		SetTimer3MR1Count(LowLeftSpeedCount);//left motor speed
		MotorSpeedRange=MOTOR_SPEED_LOW_START;//mark motors are at low speed for start of speed profile  
	}
}//ChangeSpeedProfile(void)

/*
******************************************************************************************************
* Name:                                   LeftTrackControlTask
* 
* Description: Task which is controlling left track
*       
*
* Arguments:   pdata - uCOS-II task argument
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/	  
void LeftTrackControlTask(void *pdata)
{
	DWORD TimeStamp;
	INT8U Error;
	
	pdata=pdata;//to remove warning
	for(;;)
	{
	TimeStamp=(DWORD)OSMboxPend (LeftTrackMailbox,0, &Error);//wait forever for a time stamp message from left track ISR
	if(!TimeStamp)continue;//when correct time stamp not receive continue waiting
	
	LeftTrackPulses-=1;//decrement number of pulses already covered by movement of left track
	if(TotalPulsesCountingTask == LEFT_TRACK_TSK)//when left task count total pulses
	{
		TotalTrackPulses+=1;//increment total number of pulses
		ChangeSpeedProfile();//change speed profile if TotalTrackPulses shows it is required for ongoing speed profile
	}
	if(!LeftTrackPulses)//when all left pulses executed
	{
		if(InitLeftTrackPulses==InitRightTrackPulses)//if same movement requested for both tracks
		{//force both motor stop and disbale both track interrupts
			StopTracks();
		}
		else //when not same movement requested for right and left tracks
		{
			IO2_INT_EN_F&=(~BIT7);//disble left track interrupts 
			LeftMotorFastStop();//stop left motor
		}
	}//if(!LeftTrackPulses)//when all left pulses executed
	}//for(;;)
}//LeftTrackControlTask

/*
******************************************************************************************************
* Name:                                   RightTrackControlTask
* 
* Description: Task which is controlling right track
*       
*
* Arguments:   pdata - uCOS-II task argument
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/	  
void RightTrackControlTask(void *pdata)
{
	DWORD TimeStamp;
	INT8U Error;
	
	pdata=pdata;//to remove warning
	for(;;)
	{
		TimeStamp=(DWORD)OSMboxPend (RightTrackMailbox,0, &Error);//wait forever for a time stamp message from right track ISR
		if(!TimeStamp)continue;//when correct time stamp not receive continue waiting
	
		RightTrackPulses-=1;//decrement number of pulses already covered by movement of right track
		if(TotalPulsesCountingTask == RIGHT_TRACK_TSK)//when right task count total pulses
		{
			TotalTrackPulses+=1;//increment total number of pulses
			ChangeSpeedProfile();//change speed profile if TotalTrackPulses shows it is required for ongoing speed profile
		}
		if(!RightTrackPulses)//when all right pulses executed
		{
			if(InitLeftTrackPulses==InitRightTrackPulses)//if same movement requested for both tracks
			{//force both motor stop and disbale both track interrupts
				StopTracks();
			}else //when not same movement requested for right and left tracks
			{
				IO2_INT_EN_F&=(~BIT6);//disble right track interrupts 
				RightMotorFastStop();//stop right motor
			}
		}//if(!RightTrackPulses)//when all right pulses executed	
	}//for(;;)
}//RightTrackControlTask
