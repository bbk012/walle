#ifndef CTR_GP2D12_H_
#define CTR_GP2D12_H_

#ifdef __cplusplus
extern "C" {
#endif
	

/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2012, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        ctr_gp2d12.h
* Description: Functions to control distance detector
* Author:      Bogdan Kowalczyk
* Date:        28-Oct-2012
* History:
* 28-Oct-2012 - Initial version
* 
*********************************************************************************************************
*/

//number of uCOS-II ticks to wait for distance detector readings stabilization after power up
#define DISTANCE_READING_POWER_UP_DELAY 6//wait specified number of OS ticks to get stable distance readings

//delay between subsequent IRED distance sensor reading when readings are averaged
#define DISTANCE_READING_DELAY_TICKS	3
	
//number of distance detector readings which are averaged to get final reading value
#define DISTANCE_READING_COUNTS 10

//distance detector limits
#define IRED_DISTANCE_DETECTOR_CHASM_LIMIT 		270 //338 //320 //270
#define IRED_DISTANCE_DETECTOR_SURFACE_LIMIT 	395 //380 //370
#define IRED_DISTANCE_DETECTOR_FAR_LIMIT     	600
#define IRED_DISTANCE_DETECTOR_SHORT_LIMIT   	980


//define constants for obstacle table dimensions
#define OBSTACLE_TABLE_X_SIZE 3
#define OBSTACLE_TABLE_Y_SIZE 3

//size of the table LightSrcTable to keep foto-nose light source scanned data	
#define	LIGHT_SRC_TABLE_SIZE  9
//indexes to reference LightSrcTable entriex

#define FRD_IDX		0
#define L15_IDX		1
#define L30_IDX		2
#define L45_IDX		3
#define L60_IDX		4
#define R15_IDX		5
#define R30_IDX		6
#define R45_IDX		7
#define R60_IDX		8
	
	
//stabilization time for movements during obstacle scaning in ms
#define HEAD_LONG_STABILIZATION_DELAY 	400
#define HEAD_STABILIZATION_DELAY 		100	//original 100 but rather not used because of ReadRawFotoTransistor() introduced 100ms delay
	
/*
*********************************************************************************************************
* Name:                                    ReadRawIRED
* 
* Description: Reads data from ADC connected to IR distance detector
*       
*
* Arguments:   none
*
* Returns:     Average value of counts corresponding to current IR distance detector position
* 
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of distance detector output reading
* *********************************************************************************************************
*/
extern WORD ReadRawIRED(void);
	

/*
*********************************************************************************************************
* Name:                                    CheckForObstacleIRED 
* 
* Description: Check using IRED if there is an obstacle detected by the Wall-e and how close it is
*       
*
* Arguments:   Adres of storage place for IRED raw data
*
* Returns:     Constant which determines obstacle position:
*       OBSTACLE_SURFACE 0- normal distance detector reading corresponding to the movement surface
*       OBSTACLE_CHASM 1 - sudden openning in the surface
*       OBSTACLE_FAR_DISTANCE 2- obstacle detected in the distance about 30cm <= d < 60 cm
*       OBSTACLE_SHORT_DISTANCE 3 - obstacle detected in the distance < 30cm        4
* 
*       IMPORTANT! Updates IRED raw data storage when returned
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of distance detector output reading
* *********************************************************************************************************
*/

extern BYTE CheckForObstacleIRED(WORD *RawIRED);

/*
*********************************************************************************************************
* Name:                                    DistanceSensorsFusion 
* 
* Description: For stright forward view response from IRED and US distance detectors
* 			   can be combined to determine final obstacle position decision to increase
*              detection reliability
*
* Arguments:   
*				USObstacle - result from CheckForObstacleUS
* 				IREDObstacle - result from CheckForObstacleIRED
* 
* Returns:     Constant which determines obstacle position:
*       OBSTACLE_SURFACE 0- normal distance detector reading corresponding to the movement surface
*       OBSTACLE_CHASM 1 - sudden openning in the surface
*       OBSTACLE_FAR_DISTANCE 2- obstacle detected in the distance about 30cm <= d < 60 cm
*       OBSTACLE_SHORT_DISTANCE 3 - obstacle detected in the distance < 30cm        
* 		OBSTACLE_VERY_SHORT_DISTANCE - obstacle very close to Walle 
Note(s):     
*********************************************************************************************************
*/
extern BYTE DistanceSensorsFusion(BYTE USObstacle, BYTE IREDObstacle);	

/*
*********************************************************************************************************
* Name:                                    ScanLightSrc
* 
* Description: Scans space ahead of Wall-e and store photo-nose data in the LightSrcTable for every head position
*       
*
* Arguments:   pointer to the light scanning table where light readings for every head position are stored
*
* Returns:     None value but LightSrcTable is filled with photo-nose data
*/

extern void ScanLightSrc(WORD LightSrcTable[LIGHT_SRC_TABLE_SIZE]);

/*
*********************************************************************************************************
* Name:                                    ScanObstacleLightSrc
* 
* Description: Scans space ahead of Wall-e and place obstacle flags (value 1) in the ObstacleTable
*              In addition during scanning foto-nose data are saved in the LightSrcTable for every head position
*       
*
* Arguments:   pointer to the obstacle table and pointer to the light scanning table
*
* Returns:     None value but obstacle is filled with:
*              0 - none obstacle at that position
*              1 - an obstacle or chasm at that position
*
* 
*                 0       1        2 -> X coordinate of the ObstacleTable
*              +-------+-------+-------+
* 
*             0     A       B       C
*       Y->    +-------+-------+-------+
* 
*             1    D       E       F
*              +-------+-------+-------+
* 
*                  G       W       I 
*              +-------+-------+-------+
* 
* 
*        W- Wall-e position
*        A-I - 30 cm x 30 cm sectors in front of Wall-e
* 
* *********************************************************************************************************
*/
extern void ScanObstacleLightSrc(BYTE ObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE], WORD LightSrcTable[LIGHT_SRC_TABLE_SIZE]);	
	

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*CTR_GP2D12_H_*/




