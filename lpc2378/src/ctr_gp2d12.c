/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2012, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        ctr_gp2d12.c
* Description: Functions to control distance detector
* Author:      Bogdan Kowalczyk
* Date:        28-Oct-2012
* History:
* 28-Oct-2012 - Initial version
* 
*********************************************************************************************************
*/

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"

#include "type.h"
#include "hw_spi.h"
#include "ctr_gp2d12.h"
#include "hw_gpio.h"
#include "ctr_lcd.h" 
#include "hw_pwm1.h"
#include "ctr_f_sens.h"
#include "hw_hcsr04.h"
#include "hw_uart.h"
#include "lib_dbg.h"

//Change to 0 for destination code to eliminate transimition to terminal
#define DEBUG_IRED 1
#define DEBUG_FUSION 1
//Debug trace/stop label: GP2D12_n  n - number of the subsequent function

//fusion table used to calculate US and IRED response
#define US_Y_MAX 5
#define IRED_X_MAX 5

/*
OBSTACLE_SURFACE 			 0 //- none obstacle - normal distance detector reading corresponding to the movement surface
OBSTACLE_CHASM 				 1 //- sudden openning in the surface
OBSTACLE_FAR_DISTANCE 		 2 //- obstacle detected in the distance about 30cm <= d < 60 cm
OBSTACLE_SHORT_DISTANCE 	 3 //- obstacle detected in the distance < 30cm
OBSTACLE_VERY_SHORT_DISTANCE 4 //- obstacle pretty close to Wall-e
*/

static BYTE SensorFusionTable[US_Y_MAX][IRED_X_MAX]={
{OBSTACLE_SURFACE,OBSTACLE_CHASM,OBSTACLE_FAR_DISTANCE,OBSTACLE_SHORT_DISTANCE,OBSTACLE_VERY_SHORT_DISTANCE},
{OBSTACLE_SHORT_DISTANCE,OBSTACLE_SHORT_DISTANCE,OBSTACLE_SHORT_DISTANCE,OBSTACLE_SHORT_DISTANCE,OBSTACLE_SHORT_DISTANCE},
{OBSTACLE_FAR_DISTANCE,OBSTACLE_CHASM,OBSTACLE_FAR_DISTANCE,OBSTACLE_SHORT_DISTANCE, OBSTACLE_VERY_SHORT_DISTANCE},
{OBSTACLE_SHORT_DISTANCE,OBSTACLE_CHASM,OBSTACLE_SHORT_DISTANCE,OBSTACLE_SHORT_DISTANCE,OBSTACLE_VERY_SHORT_DISTANCE},
{OBSTACLE_VERY_SHORT_DISTANCE,OBSTACLE_CHASM,OBSTACLE_VERY_SHORT_DISTANCE,OBSTACLE_VERY_SHORT_DISTANCE,OBSTACLE_VERY_SHORT_DISTANCE}
};


#if DEBUG_IRED
static void TransmitObstacleScanResults(BYTE ObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE])
	{
	DbgTraceStr(1,"GP2D12_1","\nTRC: ----------------ObstacleScanResults----------------\n");
	if(ObstacleTable[0][0])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
		
	if(ObstacleTable[0][1])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	
	if(ObstacleTable[0][2])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	DbgTraceStr(1,"GP2D12_1","\n");
	
//---------------------------------
	if(ObstacleTable[1][0])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");

	if(ObstacleTable[1][1])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	
	if(ObstacleTable[1][2])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	DbgTraceStr(1,"GP2D12_1","\n");
//---------------------------------		
	if(ObstacleTable[2][0])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	
	if(ObstacleTable[2][1])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	
	if(ObstacleTable[2][2])
		DbgTraceStr(1,"GP2D12_1","H");
	else
		DbgTraceStr(1,"GP2D12_1","-");
	DbgTraceStr(1,"GP2D12_1"," ");
	DbgTraceStr(1,"GP2D12_1","\n");
	}//TransmitObstacleScanResults
#endif

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
WORD ReadRawIRED(void)
{
	DWORD Reading=0;//sum of ADC distance readings and next avarage of it
	WORD AdcData;//distance sumple
	
	DistanceDetectorOn();
	OSTimeDly(DISTANCE_READING_POWER_UP_DELAY);//wait stabilization time on turn on
	
	for (int i=0; i<DISTANCE_READING_COUNTS; i++)//take DISTANCE_READING_COUNTS of distance readings for average
	{
		GetAdcAccess();//guarantee exclusive access to ADC for this task
		AdcData=GetAdcConversion(ADC_DISTANCE_DETECTOR);//get current ADC reading for Distance Detector
		ReleaseAdcAccess();//release exclusive access to ADC
		Reading+=(DWORD)AdcData;//update sum of readings
		OSTimeDly(DISTANCE_READING_DELAY_TICKS);//wait to get next distance readings
	}
	DistanceDetectorOff();
	return (WORD)(Reading/DISTANCE_READING_COUNTS);//get ADC readings average	
}//ReadRawIRED

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

BYTE CheckForObstacleIRED(WORD *RawIRED)
{
	WORD Reading;//storage for IR detector reading
	
	Reading=ReadRawIRED();//get IR reading
	*RawIRED=Reading;//stare raw data at the storage location
	
	//check for various types of obstacles
	if(Reading<=IRED_DISTANCE_DETECTOR_CHASM_LIMIT)//check for chasm
	{
		return OBSTACLE_CHASM;
	}
	if((Reading>IRED_DISTANCE_DETECTOR_CHASM_LIMIT) && (Reading<=IRED_DISTANCE_DETECTOR_SURFACE_LIMIT))//check for surface
	{
		return OBSTACLE_SURFACE;
	}
	if((Reading>IRED_DISTANCE_DETECTOR_SURFACE_LIMIT)&& (Reading<=IRED_DISTANCE_DETECTOR_FAR_LIMIT))//check for far obstacle
	{
		return OBSTACLE_FAR_DISTANCE;
	}
	if(Reading>IRED_DISTANCE_DETECTOR_SHORT_LIMIT)
	{
		return OBSTACLE_VERY_SHORT_DISTANCE;
	}
	return OBSTACLE_SHORT_DISTANCE;//if none from above so close obstacle only

} //CheckForObstacleIRED(void)

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
BYTE DistanceSensorsFusion(BYTE USObstacle, BYTE IREDObstacle)
{
	BYTE FusionResult;//to store fusion result
	//it should not happen but for the case select values which return OBSTACLE_SHORT_DISTANCE
	if (USObstacle >= US_Y_MAX)USObstacle=OBSTACLE_SURFACE;
	if (IREDObstacle >= IRED_X_MAX)IREDObstacle=OBSTACLE_SHORT_DISTANCE;
		
	//make sensor fusion using SensorFusionTable
	FusionResult=SensorFusionTable[USObstacle][IREDObstacle];

#if DEBUG_FUSION
	DbgTraceStrVal(2,"GP2D12_2","\nTRC: GP2D12: DistanceSensorsFusion: USObstacle:",USObstacle);
	DbgTraceStrVal(2,"GP2D12_2","\nTRC: GP2D12: DistanceSensorsFusion: IREDObstacle:",IREDObstacle);
	DbgTraceStrVal(2,"GP2D12_2","\nTRC: GP2D12: DistanceSensorsFusion: FusionResult:",FusionResult);
	DbgTraceStr(2,"GP2D12_2","\n");
#endif	
	
	return FusionResult;
	
}//DistanceSensorsFusion

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

void ScanLightSrc(WORD LightSrcTable[LIGHT_SRC_TABLE_SIZE])
{
	//clear the contents of the light intensity table
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++)
		{
		LightSrcTable[x]=0;
		}
	
	//move arms to position which is save for scanning
	ArmServoOn();
	RunPWMLeftArm(LEFT_ARM_MAX_UP_SAVE_POS);
	OSTimeDly(PWM1_MR0_MATCH);//work around to get arms moved
	RunPWMRightArm(RIGHT_ARM_MAX_UP_SAVE_POS);
	OSTimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
	ArmServoOff();
	
	
	//put head into home position
	HeadServoOn();
	
	//checlk LEFT side 15 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_15_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L15_IDX]=ReadRawFotoTransistor();//read foto nose data
	
	//checlk LEFT side 30 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_30_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L30_IDX]=ReadRawFotoTransistor();//read foto nose data
	
	//checlk LEFT side 45 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_45_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L45_IDX]=ReadRawFotoTransistor();//read foto nose data
	
	//checlk LEFT side 60 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_60_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L60_IDX]=ReadRawFotoTransistor();//read foto nose data
	
	//check RIGHT side 15 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_15_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R15_IDX]=ReadRawFotoTransistor();//read foto nose data

	//check RIGHT side 30 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_30_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R30_IDX]=ReadRawFotoTransistor();//read foto nose data

	//checlk RIGHT side 45 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_45_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R45_IDX]=ReadRawFotoTransistor();//read foto nose data
	
	//checlk RIGHT side 60 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_60_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R60_IDX]=ReadRawFotoTransistor();//read foto nose data

	//put head forward after scanning
	SmoothHeadMove(GetHeadPosition(),HEAD_CENTRAL);//move to destination position
	//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[FRD_IDX]=ReadRawFotoTransistor();//read foto nose data
	//turn of servos after scan
	HeadServoOff();
	
} //ScanLightSrc

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
*             0    A       B       C
*       Y->    +-------+-------+-------+
* 
*             1    D       E       F
*              +-------+-------+-------+
* 
*             2    G       W       I 
*              +-------+-------+-------+
* 
* 
*        W- Wall-e position
*        A-I - 30 cm x 30 cm sectors in front of Wall-e
* 
* *********************************************************************************************************
*/
 
void ScanObstacleLightSrc(BYTE ObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE], WORD LightSrcTable [LIGHT_SRC_TABLE_SIZE])
{
	BYTE ObstacleData;//result of obstacl check for selected head position
	WORD RawIredData;//temporary storage for CheckForObstacleIRED raw data (not used by this function)
	WORD RawUSData;//temporary storage for raw data of CheckForObstacleUS
  
	//cleare all fieleds of the obstacle table
	//because 0 means none obstacle
	for(int y=0;y<OBSTACLE_TABLE_Y_SIZE;y++)
	{
		for (int x=0;x<OBSTACLE_TABLE_X_SIZE;x++)
			{
				ObstacleTable[y][x]=0;
			}
	}
	
	//clear the contents of the light intensity table
	for (int x=0;x<LIGHT_SRC_TABLE_SIZE;x++)
		{
		LightSrcTable[x]=0;
		}
	
	//move arms to position which is save for scanning
	ArmServoOn();
	RunPWMLeftArm(LEFT_ARM_MAX_UP_SAVE_POS);
	OSTimeDly(PWM1_MR0_MATCH);//work around to get arms moved
	RunPWMRightArm(RIGHT_ARM_MAX_UP_SAVE_POS);
	OSTimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
	ArmServoOff();
	
	//put head into home position
	HeadServoOn();
	
  
	//checlk LEFT side 15 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_15_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L15_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);

	switch (ObstacleData)
	{
	case OBSTACLE_CHASM:
		ObstacleTable[1][1]=1;//E fild setup to 1
		ObstacleTable[0][2]=1;//C fild setup to 1
		break;
	case OBSTACLE_SURFACE:
		//none obstacle so ObstacleTable is not set up
		break;
	case OBSTACLE_FAR_DISTANCE:
		ObstacleTable[0][2]=1;//C fild setup to 1
		break;
	case OBSTACLE_SHORT_DISTANCE:
	case OBSTACLE_VERY_SHORT_DISTANCE:
		ObstacleTable[1][1]=1;//E fild setup to 1
		break;
	}
	
	//checlk LEFT side 30 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_30_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L30_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);

	switch (ObstacleData)
	{
	case OBSTACLE_CHASM:
		ObstacleTable[0][2]=1;//C fild setup to 1
		ObstacleTable[1][2]=1;//F field setup to 1
		break;
	case OBSTACLE_SURFACE:
		//none obstacle so ObstacleTable is not set up
		break;
	case OBSTACLE_FAR_DISTANCE:
		ObstacleTable[0][2]=1;//C fild setup to 1
		break;
	case OBSTACLE_SHORT_DISTANCE:
		ObstacleTable[1][2]=1;//F fild setup to 1
		break;
	case OBSTACLE_VERY_SHORT_DISTANCE:
		ObstacleTable[1][1]=1;//E fild setup to 1
		break;
	}
	
	//checlk LEFT side 45 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_45_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L45_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);

	switch (ObstacleData)
	{
	case OBSTACLE_CHASM:
		ObstacleTable[1][2]=1;//F field setup to 1
		break;
	case OBSTACLE_SURFACE:
		//none obstacle so ObstacleTable is not set up
		break;
	case OBSTACLE_FAR_DISTANCE:
		//behind sectors so do not setup at least for current moment
		break;
	case OBSTACLE_SHORT_DISTANCE:
		ObstacleTable[1][2]=1;//F field setup to 1
		break;
	case OBSTACLE_VERY_SHORT_DISTANCE:
		ObstacleTable[2][2]=1;//I field setup to 1
		break;
	}
	
	//checlk LEFT side 60 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(), HEAD_LEFT_60_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[L60_IDX]=(WORD)ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);
	
	switch (ObstacleData)
	{
	case OBSTACLE_CHASM:
		ObstacleTable[1][2]=1;//F field setup to 1
		ObstacleTable[2][2]=1;//I field setup to 1
		break;
	case OBSTACLE_SURFACE:
		//none obstacle so ObstacleTable is not set up
		break;
	case OBSTACLE_FAR_DISTANCE:
		//behind sectors so no obstacle at least for that moment
		break;
	case OBSTACLE_SHORT_DISTANCE:
		ObstacleTable[1][2]=1;//F field setup to 1
		break;
	case OBSTACLE_VERY_SHORT_DISTANCE:
		ObstacleTable[2][2]=1;//I field setup to 1
		break;
	}
	
	//check RIGHT side 15 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_15_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R15_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);
	
	switch (ObstacleData)
	{
		case OBSTACLE_CHASM:
			ObstacleTable[1][1]=1;//E fild setup to 1
			ObstacleTable[0][0]=1;//A fild setup to 1
			break;
		case OBSTACLE_SURFACE:
			//none obstacle so ObstacleTable is not set up
			break;
		case OBSTACLE_FAR_DISTANCE:
			ObstacleTable[0][0]=1;//A fild setup to 1
			break;
		case OBSTACLE_SHORT_DISTANCE:
		case OBSTACLE_VERY_SHORT_DISTANCE:
			ObstacleTable[1][1]=1;//E fild setup to 1
			break;
	}

	//check RIGHT side 30 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_30_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R30_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);

	switch (ObstacleData)
	{
		case OBSTACLE_CHASM:
			ObstacleTable[0][0]=1;//A fild setup to 1
			ObstacleTable[1][0]=1;//D fild setup to 1
			break;
		case OBSTACLE_SURFACE:
			//none obstacle so ObstacleTable is not set up
			break;
		case OBSTACLE_FAR_DISTANCE:
			ObstacleTable[0][0]=1;//A fild setup to 1
			break;
		case OBSTACLE_SHORT_DISTANCE:
			ObstacleTable[1][0]=1;//D fild setup to 1
			break;
		case OBSTACLE_VERY_SHORT_DISTANCE:
			ObstacleTable[1][1]=1;//E fild setup to 1
			break;
	}

	//checlk RIGHT side 45 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_45_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R45_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);
	
	switch (ObstacleData)
	{
		case OBSTACLE_CHASM:
			ObstacleTable[1][0]=1;//D field setup to 1
			break;
		case OBSTACLE_SURFACE:
			//none obstacle so ObstacleTable is not set up
			break;
		case OBSTACLE_FAR_DISTANCE:
			//behind sectors so none obstacle at least for that moment
			break;
		case OBSTACLE_SHORT_DISTANCE:
			ObstacleTable[1][0]=1;//D field setup to 1
			break;
		case OBSTACLE_VERY_SHORT_DISTANCE:
			ObstacleTable[2][0]=1;//G fild setup to 1
			break;
	}
	
	//checlk RIGHT side 60 DEGs obstacles
	SmoothHeadMove(GetHeadPosition(),HEAD_RIGHT_60_DEG);//move to destination position
//	ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[R60_IDX]=ReadRawFotoTransistor();//read foto nose data
	ObstacleData=CheckForObstacleIRED(&RawIredData);

	switch (ObstacleData)
	{
		case OBSTACLE_CHASM:
			ObstacleTable[1][0]=1;//D field setup to 1
			ObstacleTable[2][0]=1;//G field setup to 1
			break;
		case OBSTACLE_SURFACE:
			//none obstacle so ObstacleTable is not set up
			break;
		case OBSTACLE_FAR_DISTANCE:
			//behind sectors so none obstacle at least for now
			break;
		case OBSTACLE_SHORT_DISTANCE:
			ObstacleTable[1][0]=1;//D field setup to 1
			break;
		case OBSTACLE_VERY_SHORT_DISTANCE:
			ObstacleTable[2][0]=1;//G field setup to 1
			break;
	}
	//put head forward after scanning
	SmoothHeadMove(GetHeadPosition(),HEAD_CENTRAL);//move to destination position
//ReadRawFotoTransistor introduces 100 ms delay already
	LightSrcTable[FRD_IDX]=ReadRawFotoTransistor();//read foto nose data
	
	//for the stright front view make a fusion of US and IRED reading
	ObstacleData=DistanceSensorsFusion(CheckForObstacleUS(&RawUSData), CheckForObstacleIRED(&RawIredData));

	switch (ObstacleData)
	{
	case OBSTACLE_CHASM:
		ObstacleTable[1][1]=1;//E fild setup to 1
		ObstacleTable[0][1]=1;//B fild setup to 1
		break;
	case OBSTACLE_SURFACE:
		//none obstacle so ObstacleTable is not set up
		break;
	case OBSTACLE_FAR_DISTANCE:
		ObstacleTable[0][1]=1;//B fild setup to 1
		break;
	case OBSTACLE_SHORT_DISTANCE:
	case OBSTACLE_VERY_SHORT_DISTANCE:
		ObstacleTable[1][1]=1;//E fild setup to 1
		break;
	}
	//turn of servos after scan
	HeadServoOff();
//	ShowObstacleScanResults(ObstacleTable); for debug
#if DEBUG_IRED
	TransmitObstacleScanResults(ObstacleTable);
	DbgStopStr(2,"GP2D12_1","\nSTP: Stopped after ScanObstacleLightSrc");
#endif
} //ScanObstacleLightSrc
