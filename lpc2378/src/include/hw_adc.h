/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2016, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_adc.h
* Description: Functions to control internal ADC of the LPC 2378
* Author:      Bogdan Kowalczyk
* Date:        11-May-2016
* History:
* 1-May-2016 - Initial version of the ADC API created
* 
*********************************************************************************************************
*/

#ifndef HW_ADC_H_
#define HW_ADC_H_
#ifdef __cplusplus
   extern "C" {
#endif


//adc counts corresponding to 0g based on averaged calibration measurements	   
#define X_0G_COUNT	531
#define Y_0G_COUNT  540
#define Z_0G_COUNT  508
	   
//adc counts corresponding to 1g based on averaged calibration measurements	   
#define X_1G_COUNT	622
#define Y_1G_COUNT  637
#define Z_1G_COUNT  602
	   
//adc count thresholds which is used to detect tilt in any direction
//change on count above this threshold means there is tilt
#define	X_TILT_CNT_THRESHOLD 15
#define	Z_TILT_CNT_THRESHOLD 25 //because of acceleration of Wall-e start Z threshold is higher than X
	   
//values returned by CheckForTilt function
#define	NO_TILT			0
#define TILT_OBSTACLE	10

//number of ADC readings taken into account to get average acceleration
//One ADC reading is about 26,4uS	   
#define NO_OF_ADC_READINGS_FOR_TILT	10	   

//number of OS Tick Deley for subsequent acceleration sampling 1 means 10ms what is 100Hz
#define ADC_DELAY_FOR_TILT_SAMPLING	1

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
*********************************************************************************************************
*/
extern void InituPAdc(void);

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
extern void GetuPAdcAccess(void);


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
extern void ReleaseuPAdcAccess(void);

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
*********************************************************************************************************
*/
extern WORD GetAccelerationX(void);


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
extern WORD GetAccelerationY(void);

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
*********************************************************************************************************
*/
extern WORD GetAccelerationZ(void);	   
	   
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
extern BYTE CheckForTilt(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used
   
#endif /*HW_ADC_H_*/
