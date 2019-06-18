/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_hcsr04.c
* Description: Functions to control ultrasonic distance sensor HC-SR04 
* Author:      Bogdan Kowalczyk
* Date:        01-Nov-2017
* History:
* 01-Nov-2017 - Initial version created
*********************************************************************************************************
*/
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"

#include "type.h"
#include "hw_hcsr04.h"
#include "hw_gpio.h"
#include "hw_timer.h"

#include "hw_uart.h" 


/*
*********************************************************************************************************
* Name:                                    ReadRawUS
* 
* Description: Reads distance from ultrasonic distance sensor
*       
*
* Arguments:   none
*
* Returns:     Distance to an obstacle in milimeters or ERROR_US_VALUE (0xFFFF) for an error
* 
* Note(s):     
*            
* *********************************************************************************************************
*/
WORD ReadRawUS(void)
{
	BYTE  NoOfTimeoutTicks=0;//number of OS Ticks counted for timeout
	DWORD StartCap;//time stamp for raising Echo pulse in uS
	DWORD StopCap;//time stamp for falling Echo pulse in uS
	DWORD EchoDuration;//used to store counted duration of the Echo signal
	
	UltrasonicTrigOff();//put US trigger input to LOW state
	Timer0Start();//start ultrasonic timer to measure Echo response pulse duration
	UltrasonicTrigOn();//generate one triger pulse to start US measurement
	WaitTimer0_10uS();//10us pulse is required by US spec to triger an operation
	UltrasonicTrigOff();
	while(IsTimer0Counting())//as long as there is Echo duration counted
	{
		if(NoOfTimeoutTicks > US_MEASUREMENT_TIMEOUT)//cannot get measurements after specified timeout 
			{
			Timer0Stop();//stop timer0
			return ERROR_US_VALUE; //waited US_MEASUREMENT_TIMEOUT timeout and no results return error
			}
		OSTimeDly(US_DELAY_TICKS);
		NoOfTimeoutTicks+=1;//increment and wait one more tick		
	}
	Timer0Stop();//stop timer0
	StartCap=GetTimer0StartCAP();//read Echo raising edge time stamp
	StopCap=GetTimer0StopCAP();//read Echo falling edge time stamp

	if(StartCap>=StopCap)//this case should never happen for 32 bit counter counting time is too short for wrap back
	{
		
		return ERROR_US_VALUE;//this is error so signal it
	}
	EchoDuration=StopCap-StartCap;//count Echo duration in uS
	if(EchoDuration > MAX_ECHO_DURATION)
		return MAX_FREE_SPACE_MM;//reture distance coresponding to max considered free space (which is 2[m])
	
	return (WORD)((EchoDuration*10)/58);//convert Echo duration into [mm]
}//ReadRawUS

/*
*********************************************************************************************************
* Name:                                    CheckForObstacleUS 
* 
* Description: Check using US if there is an obstacle detected by the Wall-e and how close it is
*		This is analogy to IRED CheckForObstacleIRED
*       
*
* Arguments:   Address of storage place for US raw data
*
* Returns:     Constant which determines obstacle position:
*       OBSTACLE_SURFACE 0- normal distance detector reading corresponding to the movement surface
*       OBSTACLE_FAR_DISTANCE 2- obstacle detected in the distance about 30cm <= d < 60 cm
*       OBSTACLE_SHORT_DISTANCE 3 - obstacle detected in the distance < 30cm        4
*       OBSTACLE_VERY_SHORT_DISTANCE 4 - obstacle very close to Wall-e
* 
*       IMPORTANT! Updates US raw data storage when returned with sistance in [mm] to an obstacle
* 				see ReadRawUS for details
* Note(s):     
* *********************************************************************************************************
*/

BYTE CheckForObstacleUS(WORD *RawUS)
{
	WORD Reading;//storage for US detector reading
	
	Reading=ReadRawUS();//get UR reading
	*RawUS=Reading;//stare raw data at the storage location
	
	if(Reading<=US_DISTANCE_DETECTOR_SHORT_LIMIT)
	{
		return OBSTACLE_VERY_SHORT_DISTANCE;
	}
	
	if((Reading>US_DISTANCE_DETECTOR_SHORT_LIMIT) &&(Reading<=US_DISTANCE_DETECTOR_FAR_LIMIT))
	{
		return OBSTACLE_SHORT_DISTANCE;	
	}
	
	if((Reading>US_DISTANCE_DETECTOR_FAR_LIMIT) &&(Reading<=US_DISTANCE_DETECTOR_SURFACE_LIMIT))
	{
		return OBSTACLE_FAR_DISTANCE;
	}
	
	if (Reading>US_DISTANCE_DETECTOR_SURFACE_LIMIT)
	{
		return OBSTACLE_SURFACE;
	}
	
	return OBSTACLE_SHORT_DISTANCE;//getting this line should not happen it is only to not generate a warning
} //CheckForObstacleUS
