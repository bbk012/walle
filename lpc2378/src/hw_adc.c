/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2016, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_adc.c
* Description: Functions to control internal ADC of the LPC 2378
* Author:      Bogdan Kowalczyk
* Date:        11-May-2016
* History:
* 1-May-2016 - Initial version of the ADC API created
* 
*********************************************************************************************************
*/

//includes to get access to uCOS-II mutex to protect ADC which is going to be a coomon resource for diffrent managers
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "lib_error.h"

#include "type.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "hw_adc.h"
#include "hw_uart.h"
#include "lib_std.h" //to get access to abs() function

//Remove ADC DEBUG conditional compilation
#define DEBUG_ADC_UP 0

//IMPORTANT! Access to the uP ADC is protected by mutex to avoid
//           problems when ADC is used by two different managers/tasks
//           Mutex is only operated by ADC API functions not accessed directly.
OS_EVENT* uPAdcMutex; 


/*
*********************************************************************************************************
* Name:                                   InituPAdc 
* 
* Description: Initialize access to uP ADC and also creates MUTEX used to protect exclusive access into ADC
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			   Creates and initialize internal MUTEX used to manage exclusive access to ADC from threads
* *********************************************************************************************************
*/
void InituPAdc(void)
{
	PINSEL1|=BIT16|BIT14;//select AD0.0, AD01 INPUT
	PINSEL0|=BIT25|BIT24; //select AD0.6 INPUT 
	PCONP|=BIT12; //enable clock to be provided to ADC as it is disabled during reset
	
	//setup uP ADC mutex
	uPAdcMutex=OSSemCreate(1);//initialize ADC mutex to avaliable state
	if(!uPAdcMutex)UCOSII_RES_EXCEPTION;//Exception - when there is not uCOS-II event blocks availiable (uCOS-II resources are not availaible) 
}//InituPAdc

/*
*********************************************************************************************************
* Name:                                    GetuPAdcAccess 
* 
* Description: Call to get access to internal uP ADC
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*             Need to be called to get exclusive access to internal uP ADC
* Exclusive access to internal up ADC is according to the below schema:
* GetuPAdcAccess();
* 	Perform required ADC operation exclusively owning ADC
*  ReleaseuPAdcAccess();
* *********************************************************************************************************
*/
void GetuPAdcAccess(void)
{
	BYTE Result;
	OSSemPend(uPAdcMutex,0x0000,&Result);//for uCOS-II the value 0x0000 means wait for mutex infinite
	if(Result != OS_NO_ERR)UCOSII_RES_EXCEPTION;//Exception when ther is an error
}// GetuPADCAccess

/*
*********************************************************************************************************
* Name:                                    ReleaseuPAdcAccess 
* 
* Description: Call to release access to internal uP ADC
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
void ReleaseuPAdcAccess(void)
{
	OSSemPost(uPAdcMutex);//release ADC mutex
}// ReleaseuPADCAccess

/*
*********************************************************************************************************
* Name:                                    GetAccelerationX  
* 
* Description: Gets acceleration for X direction of the accelerator
*
* Arguments:   none
*
* Returns:     10-bits ADC convertion results of acceleration in X direction
*
* Note(s):     
* 
* *********************************************************************************************************
*/
WORD GetAccelerationX(void)
{
	DWORD AdcVal;//temp storage for the ADC reading
	
	AD0CR=BIT24|BIT21|BIT9|BIT1;//run channel AD0.1 conversion which is X direction acceleration
	while(!((AdcVal=AD0GDR)&BIT31))//wait until conversion done 
	{
	}
	return (WORD)((AdcVal>>6)& 0x000003FF);//extract adc reading and return it
}//GetAccelerationX


/*
*********************************************************************************************************
* Name:                                    GetAccelerationY  
* 
* Description: Gets acceleration for Y direction of the accelerator
*
* Arguments:   none
*
* Returns:     10-bits ADC convertion results of acceleration in Y direction
*
* Note(s):     
* 
* *********************************************************************************************************
*/
WORD GetAccelerationY(void)
{
	DWORD AdcVal;//temp storage for the ADC reading
	
	AD0CR=BIT24|BIT21|BIT9|BIT0;//run channel AD0.0 conversion which is X direction acceleration 
	while(!((AdcVal=AD0GDR)&BIT31))//wait until conversion done 
	{
	}
	return (WORD)((AdcVal>>6)& 0x000003FF);//extract adc reading and return it
}//GetAccelerationY

/*
*********************************************************************************************************
* Name:                                    GetAccelerationZ  
* 
* Description: Gets acceleration for Z direction of the accelerator
*
* Arguments:   none
*
* Returns:     10-bits ADC convertion results of acceleration in Z direction
*
* Note(s):     
* 
* *********************************************************************************************************
*/
WORD GetAccelerationZ(void)
{
	DWORD AdcVal;//temp storage for the ADC reading
	
	AD0CR=BIT24|BIT21|BIT9|BIT6;//run channel AD0.6 conversion which is X direction acceleration
	while(!((AdcVal=AD0GDR)&BIT31))//wait until conversion done 
	{
	}
	return (WORD)((AdcVal>>6)& 0x000003FF);//extract adc reading and return it
}//GetAccelerationZ

/*
*********************************************************************************************************
* Name:                                    CheckForTilt  
* 
* Description: Reads accelerometers and determine if ABS(X_ACC) or ABS(Z_ACC) is above TILT_CNT_THRESHOLD
* 			if yes there is tilt detected of Wall-e
*
* Arguments:   none
*
* Returns: 
* 			NO_TILT			none tilt detected
*			TILT_OBSTACLE	tilt of Wall-e detected    
*
* Note(s):     
* 			Number of ADC readings is averaged (NO_OF_ADC_READINGS_FOR_TILT) to get current value of acceleration
*           One ADC reading is about 26,4uS
* 			There is ADC_DELAY_FOR_TILT_SAMPLING*OS_TICK deley introduced between samples
* *********************************************************************************************************
*/
BYTE CheckForTilt(void)
{
	DWORD	XAcceleration=0;//storage for counted X acceleration
	DWORD	ZAcceleration=0;//storage for countes Z acceleration
	BYTE	Count;//counts subsequent ADC readings of Acceleration
	
	GetuPAdcAccess();//get exclusive ADC access for acceleration reading
	for (Count=0;Count<NO_OF_ADC_READINGS_FOR_TILT;Count++)
	{
		XAcceleration+=GetAccelerationX();
		ZAcceleration+=GetAccelerationZ();
		OSTimeDly(ADC_DELAY_FOR_TILT_SAMPLING);//sample acceleration at rate  ADC_DELAY_FOR_TILT_SAMPLING*OS_TICK
	}
	ReleaseuPAdcAccess();//release exclusive ADC access for reading
	
	//calculate average acceleration
	XAcceleration=XAcceleration/NO_OF_ADC_READINGS_FOR_TILT;
	ZAcceleration=ZAcceleration/NO_OF_ADC_READINGS_FOR_TILT;
	
	//check if any X or Z is above tilt threshold if yes return TILT result
	if(abs((WORD)XAcceleration-X_0G_COUNT)>X_TILT_CNT_THRESHOLD)
	{
		return TILT_OBSTACLE;
	}
	if(abs((WORD)ZAcceleration-Z_0G_COUNT)>Z_TILT_CNT_THRESHOLD)
	{
		return TILT_OBSTACLE;
	}
	return NO_TILT;
}//CheckForTilt
