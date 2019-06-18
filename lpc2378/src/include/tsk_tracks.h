/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        tsk_tracks.h
* Description: Left & Right track control tasks 
* Author:      Bogdan Kowalczyk
* Date:        23-July-2011
* History:
* 23-July-2011 Initial version created
*********************************************************************************************************
*/

#ifndef TSK_TRACK_H_
#define TSK_TRACK_H_
#ifdef __cplusplus
   extern "C" {
#endif

#include "type.h"
#include "os_cfg.h"

//motor directions constants	
#define	MOTOR_FORWARD 	0
#define MOTOR_REVERSE  	1	
	   	
//number of track counts corresponding to scanning sector length (30 cm)
#define SECTOR_LENGTH_COUNTS 24

	   
//motor speed profiles
//for speed profile description see  RunTracksPulses description in *.c file	
#define	LOW_MID_HIGH_PROFILE	0
#define LOW_MID_PROFILE			1
#define LOW_PROFILE				2
#define MID_PROFILE				3
#define HIGH_PROFILE    		4	
	   	
//threshold valuse of pulses to calculate various motor speed profiles
#define LOW_SPEED_THRESHOLD 	0
#define MID_SPEED_THRESHOLD 	20 //switch to mid speed after 20 pulses since start and 20 pulses before end to low
#define HIGH_SPEED_THRESHOLD 	50//switch to high speed 50 pulses after start and swich to mid 50 pulses before end
#define SPEED_SWITCH_THRESHOLD 	12//minimum amount of pulses for which it has sense to switch speed	

//identifiers for the motor speed ranges - see  RunTracksPulses for meaning
#define MOTOR_SPEED_OFF			0 //none speed profile is setup as motors are not working	
#define MOTOR_SPEED_LOW_START	1 //low speed of motors at speed profile start
#define MOTOR_SPEED_MID_START	2 //mid speed of motors at speed profile start
#define MOTOR_SPEED_HIGH_START	3 //high speed of motor at speed profile start
#define MOTOR_SPEED_MID_END		4 //mid speed of motor at speed profile end
#define MOTOR_SPEED_LOW_END		5 //low speed of motor at speed profile end
	   	
//cannot setup this value of pulses by	
#define	MAX_PULSE_COUNT 0xFFFF

//delay in OS Ticks after Transoptor is ON and interrupts for GPIO are on to detect pulses
#define	TRANSOPTOR_ON_DELAY   15 //15 OSTicks delay

//define threshold for track pulses below which it is assumed to have noise signal
// if time between pulses is less to 100*120uS it is not taken into account - filtered out
#define MIN_PULSE_DELAY 100

//value for pullse difference error for which it is assumed no difference between pulses	
#define MIN_DIFF_ERROR_VALUE   3
	   	
//define number of total track pulses after which time stamp measurements starts
#define MIN_PULSE_LENGTH_START 2
	   	
//define number of pulses at the end of profile after which correction is minimized (to avoid big change at end of movement)
 #define MIN_PULSE_LENGTH_END  5

//minimum number of pulses lengths which  needs to be availaible before average pulse length is calculated and used
#define MIN_PULSE_LENGHT_COUNT 8
	   	
//maximum speed correction factor used by algorithm which tries to keep both motor speed the same
#define	MOTOR_COUNT_CORRECTION_FACTOR_THRESHOLD 12
	   	
//max speed coorection factor when movement is closed to its end
#define	MOTOR_COUNT_CORRECTION_FACTOR_THRESHOLD_AT_END	2

#define MID_PULSE_LENGTH_GOAL 605	
	   	
	   	
//regulator algorith correction values
	   	
#define K_P 	7 //0.03*256
#define K_D 	15 //0,06*256
#define K_I     1   //0.004*256
#define K_NORM  8 //>>8 = /256
   		   
	   
//defines priority of the left track control task
#define LEFT_TRACK_CTRL_TASK_PRIORITY  6
#define LEFT_TRACK_CTRL_STACK_SIZE OS_TASK_STACK_SIZE
	   
//defines priority of the right track control task
#define RIGHT_TRACK_CTRL_TASK_PRIORITY  7
#define RIGHT_TRACK_CTRL_STACK_SIZE OS_TASK_STACK_SIZE
	   
//ID of task which is counting TotalTrackPulses variable
#define LEFT_TRACK_TSK 	0
#define RIGHT_TRACK_TSK	1
	   	   
extern OS_STK LeftTrackControlTaskStack[LEFT_TRACK_CTRL_STACK_SIZE];//stack for left track control task
extern OS_STK RightTrackControlTaskStack[RIGHT_TRACK_CTRL_STACK_SIZE];//stack for right track control task

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
extern void RunTracksPulses(WORD InLeftPulses, BYTE InLeftDirection, WORD InRightPulses, BYTE InRightDirection,BYTE InSpeedProfile);

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
extern void StopTracks(void);

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
extern void InitPulseCounters(void);

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
extern int IsTrackMoving(void);

/*
*********************************************************************************************************
* Name:                                   GetCurrentTrackPulses
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
extern WORD GetCurrentTrackPulses(void);


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
extern WORD GetRightTrackPulses(void);

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
extern WORD GetLeftTrackPulses(void);


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
extern void LeftTrackControlTask(void *pdata);	  

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
*********************************************************************************************************
*/	  
extern void RightTrackControlTask(void *pdata);	   
 
	   
	   
	   
#ifdef __cplusplus
}
#endif //to close extern "C" if used
#endif /*TSK_TRACK_H_*/
