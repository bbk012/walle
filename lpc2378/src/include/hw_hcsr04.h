/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_hcsr04.h
* Description: Functions to control ultrasonic distance sensor HC-SR04 
* Author:      Bogdan Kowalczyk
* Date:        01-Nov-2017
* History:
* 01-Nov-2017 - Initial version created
*********************************************************************************************************
*/

#ifndef HW_HCSR04_H_
#define HW_HCSR04_H_

#ifdef __cplusplus
extern "C" {
#endif
	
//returned when ultrasonic cannot measure any distance to an obstacle	
#define ERROR_US_VALUE 0xFFFF

//number of OS Ticks ReadRawUS us waiting to get Echo signal duration measurements end check
#define US_DELAY_TICKS	1

//timeout for distance measurement in US_DELAY_TICKS
//when there is not measurement end after this timeout measurement is broken
#define US_MEASUREMENT_TIMEOUT	4

//maximum allowed duration of the Echo signali in uS
//according to spec 38ms is maximum duration of US response which coresponds to timeout for none obstacle
//I assumed all above 11600us which is 200[cm] is an obstacle free response
#define MAX_ECHO_DURATION 11600

//maximu value of free distance which coresponds the case of MAX_ECHO_DURATION
//means the case that there is no obstacle detected
//it is 2000 [mm] <=> 200 [cm] <=> 2 [m]	
#define MAX_FREE_SPACE_MM 	2000

//us distance detector limits in [mm]
#define US_DUSTANCE_LOW_LIMIT				70				
#define US_DISTANCE_DETECTOR_SHORT_LIMIT   	180
#define US_DISTANCE_DETECTOR_FAR_LIMIT 	    370
#define US_DISTANCE_DETECTOR_SURFACE_LIMIT	670
	
	
/*
*********************************************************************************************************
* Name:                                    ReadRawUS
* 
* Description: Reads distance from ultrasonic distance sensor
*       
*
* Arguments:   none
*
* Returns:     Distance to an obstacle in milimeters or0xFFFF for an error
* 
* Note(s):     
*            
*********************************************************************************************************
*/
extern WORD ReadRawUS(void);	

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
extern BYTE CheckForObstacleUS(WORD *RawUS);

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*HW_HCSR04_H_*/
