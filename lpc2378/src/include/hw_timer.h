/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_timer.h
* Description: Functions to control LPC 2378 uP timer and timer interrupt handler
* Author:      Bogdan Kowalczyk
* Date:        5-Aug-2008
* History:
*              5-Aug-2008 - Initial version created
*              2-May-2009 - Added handling for Timer3 used to generate PWM for motor control
* 			  31-Dec-2010 - Added ARS signal sampling and integration to rotation angle
*             01-Nov-2017 - uC-OSII Tick unterrupt moved from Timer0 to Timer2, Timer0 CAP0 used for US sensor
*********************************************************************************************************
*/

#ifndef TIMER_H_
#define TIMER_H_

#ifdef __cplusplus
   extern "C" {
#endif

#include "type.h"

//setup timer0 pulses for 1us	   
//this is value by which Fpclk is divided before applied to TIMER0 timer counter
//it is assumed Fplck = 12500000 Hz so for prescaler value 0x01 Fplck is divided by 2
//for 0x0C timer0 clock is 12500000/(12+1)= 961538 Hz what gives 1,04uS pulse with 
#define TIMER0_PRESCALER_VALUE 0x0C
	   
//used to setup 10uS delay for WaitTimer0_10uS function
#define TIMER0_10uS		10
	   
	   
//this value defines how often timer1 interrupt is generated when enabled
//to integrate ARS output for example 1000 means every 1ms
#define ARS_SAMPLE_PER_SEC 1000	   

//define number of samples used to calculate ARS offset
#define ARS_OFFSET_SAMPLE_COUNT 50
//and number of uCOS-II ticks deley between samples for example 1 means one OS_TICKS delay between samples	   
#define ARS_OFFSET_SAMPLE_TICKS 1

//threshold for which ARS sensor sample is assumed to be equal 0
//i.e. if ADC reading - offset is smaller to this threshold the reading is assumed to be equal to the ARS offset i.e. 0
#define ARS_SAMPLE_THRESHOLD 8
	   
//ARS integrated number of counts corresponding to 90 degrees (right turn)
//set up through experimental runs
#define ARS_RIGHT_90_TURN_COUNT		-1369550L//   	-1386350L //(-1356120L+720L-15300L)
//ARS integrated number of counts corresponding to 90 degrees (left turn)
//set up through experimental runs
#define ARS_LEFT_90_TURN_COUNT	   	 1369550L //1386350L //(1356120L-720L+15300L) 
//this is value by which Fpclk is divided before applied to TIMER3 timer counter
//it is assumed Fplck = 12500000 Hz so for prescaler value 0x01 Fplck is divided by 2
//while for prescaler value 0x05 Fplck is divided by 6	   
#define TIMER3_PRESCALER_VALUE 0x05

	   
//maximum value for match register used to generate periodic interrupts
#define INIT_TIMER3_MR2_COUNT 250
	   
//initial value for match registers generating PWM
//default value of PWM is following:
//PWM1[%]=(INIT_TIMER3_MR0_COUNT/INIT_TIMER3_MR2_COUNT)*100[%]
//PWM2[%]=(INIT_TIMER3_MR1_COUNT/INIT_TIMER3_MR2_COUNT)*100[%]
#define INIT_TIMER3_MR0_COUNT 0
#define INIT_TIMER3_MR1_COUNT 0

//constant used to control speed of left and right motor
//left motor constant differs to right motor constant because in practice motors are slightly different
//constants need to be calibrated and are used by void SetTimer3MRxCount(BYTE InCount) function to setup
//speed of left and right motor
//we have 3 levels of speed LOW, MID and HIGH
#define LOW_LEFT_MOTOR_COUNT   185
#define LOW_RIGHT_MOTOR_COUNT  180

#define MID_LEFT_MOTOR_COUNT   215
#define MID_RIGHT_MOTOR_COUNT  210
	   
#define HIGH_LEFT_MOTOR_COUNT  240
#define HIGH_RIGHT_MOTOR_COUNT 235	   

//define max and min allwed speed of the motors
#define MAX_MOTOR_SPEED_COUNT  (INIT_TIMER3_MR2_COUNT-1)
#define MIN_MOTOR_SPEED_COUNT  LOW_LEFT_MOTOR_COUNT
	   
/*
*********************************************************************************************************
* Name:                                    InitTimer0 
* 
* Description: Setup timer 0 to be used for Ultrasonic Sensor handling
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
*********************************************************************************************************
*/
extern void InitTimer0 (void);
	   
/*
*********************************************************************************************************
* Name:                                    Timer0IsrHandler 
* 
* Description: Timer 0 Interrupt Service Routine - handles CAP0 interrupts to measure ultrasonic echo pulse duration
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			Interrupting device is acknowledged and its sorce of interrupt is cleared on this level.
* 			Note that VIC of the uP is cleared on the level of OS_CPU_ExceptHndlr function.
**********************************************************************************************************
*/
extern void Timer0IsrHandler(void);
	   
/*
*********************************************************************************************************
* Name:                                    Timer0Start 
* 
* Description: Starts Timer 0 to count 1uS pulses and to generate CAP0 interrupt on P.23 raising edge
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*********************************************************************************************************
*/
extern void Timer0Start(void);

/*
*********************************************************************************************************
* Name:                                    Timer0Stop 
* 
* Description: Stops Timer 0 to count 1uS pulses for Ultrasonic Echo signals measurement
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*********************************************************************************************************
*/
extern void Timer0Stop(void);

/*
*********************************************************************************************************
* Name:                                    WaitTimer10uS 
* 
* Description: Uses Timer 0 to measure 10uS time delay
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Works only when Timer0 is running, means after Timer0Start before timer0 is stopped

* *********************************************************************************************************
*/
extern void  WaitTimer0_10uS(void);	   
/*
*********************************************************************************************************
* Name:                                    GetTimer0StartCAP 
* 
* Description: Get timer0 captured value for ultrasonich Echo raising edge
*
* Arguments:   none
*
* Returns:     Value of timer0 capture register stored for raising edge of ultrasonic Echo signal
*
* Note(s):     

* *********************************************************************************************************
*/
extern DWORD GetTimer0StartCAP(void);

/*
*********************************************************************************************************
* Name:                                    GetTimer0StopCAP 
* 
* Description: Get timer0 captured value for ultrasonich Echo falling edge
*
* Arguments:   none
*
* Returns:     Value of timer0 capture register stored for falling edge of ultrasonic Echo signal
*
* Note(s):     

* *********************************************************************************************************
*/
extern DWORD GetTimer0StopCAP(void);

/*
*********************************************************************************************************
* Name:                                    IsTimer0Counting
* 
* Description: Checks if timer 0 is still counting ultrasonic echo pulse duration
*
* Arguments:   none
*
* Returns:     
* 			TRUE  - Timer0 is still counting Echo pulse duration (waiting for falling edge)
* 			FALSE - Timer0 counting is ended
*
* Note(s):     

* *********************************************************************************************************
*/
extern BYTE IsTimer0Counting(void);

/*
*********************************************************************************************************
* Name:                                    InitTimer1  
* 
* Description: Setup timer 1 to provide timer interrupts every ARS_SAMPLE_PER_SEC (1us) when enabled.
*              This interupt is used to integrate ARS sensor output to calculate turn angle.
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*	IMPORTANT! 
*              1) After InitTimer1 call timer is setup but it is not run a separate function 
*              needs to be called to make it running.
* 			   That is to count angle only when integration is enabled not peramantly.
*              2) This function needs to be called when uCOS-II is running.
* *********************************************************************************************************
*/
extern void InitTimer1 (void);

/*
*********************************************************************************************************
* Name:                                    SetArsOffset  
* 
* Description: Recalculates ARS offset
* ARS_OFFSET_SAMPLE_COUNT number of samles is taken every ARS_OFFSET_SAMPLE_TICKS uCOS-II ticks
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):    IMPORTANT!
* 	        	ADC needs to be exclusively owned before that function call like below
* 	GetAdcAccess();//access to ADC need to be exclusive
*	SetArsOffset();
*	ReleaseAdcAccess();//release ADC to others
* *********************************************************************************************************
*/
extern void SetArsOffset(void);

/*
*********************************************************************************************************
* Name:                                    GetArsOffset 
* 
* Description: Returns calculated (integrated) ARS sensor offset value
* 
* Arguments:   none
*
* Returns:     ARS sensor offset
*
* Note(s):    
* *********************************************************************************************************
*/
extern LONG GetArsOffset(void);

/*
*********************************************************************************************************
* Name:                                    StartArsAngleCounting  
* 
* Description: Setup timer 1 to generate sample rate and to have its interrupt handler to calculate the angle
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):    
* *********************************************************************************************************
*/
extern void StartArsAngleCounting(void);

/*
*********************************************************************************************************
* Name:                                    StopArsAngleCounting  
* 
* Description: Stops timer 1 to not generate sample rate 
* 
* Arguments:   none
*
* Returns:     none
*
* Note(s):    
* *********************************************************************************************************
*/
extern void StopArsAngleCounting(void);

/*
*********************************************************************************************************
* Name:                                    SetArsTargetAngle 
* 
* Description: Set value for ARS target angle
* 
* Arguments:   ARS target angle count
*
* Returns:     none
*
* Note(s):    
* *********************************************************************************************************
*/
extern void SetArsTargetAngle(LONG InTargetAngle);


/*
*********************************************************************************************************
* Name:                                     GetArsTargetAngle 
* 
* Description: Get value of ARS target angle
* 
* Arguments:   ARS target angle count
*
* Returns:     none
*
* Note(s):    
* *********************************************************************************************************
*/
extern LONG GetArsTargetAngle(void);

/*
*********************************************************************************************************
* Name:                                    Timer1IsrHandler  
* 
* Description: Timer 1 Interrupt Service Routine - integrates ARS output
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			Interrupting device is acknowledged and its sorce of interrupt is cleared on this level.
* 			Note that VIC of the uP is cleared on the level of OS_CPU_ExceptHndlr function.
* *********************************************************************************************************
*/
extern void Timer1IsrHandler(void);

/*
*********************************************************************************************************
* Name:                                    GetArsAngleValue 
* 
* Description: Returns calculated (integrated) angle value
* 
* Arguments:   none
*
* Returns:     Calculated angle value
*
* Note(s):    
* *********************************************************************************************************
*/
extern LONG GetArsAngleValue(void);

/*
*********************************************************************************************************
* Name:                                    InitTimer2  
* 
* Description: Setup timer 2 to provide timer interrupts to uCOS-II
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/

extern void InitTimer2 (void);

/*
*********************************************************************************************************
* Name:                                    Timer2IsrHandler 
* 
* Description: Timer 2 Interrupt Service Routine - provide timer tick to uCOS-II
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			Interrupting device is acknowledged and its sorce of interrupt is cleared on this level.
* 			Note that VIC of the uP is cleared on the level of OS_CPU_ExceptHndlr function.
* *********************************************************************************************************
*/
extern void Timer2IsrHandler(void);



/*
*********************************************************************************************************
* Name:                                    InitTimer3  
* 
* Description: Setup timer 3 to generate PWM for motor control
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern void InitTimer3 (void);

/*
*********************************************************************************************************
* Name:                                    Timer3IsrHandler 
* 
* Description: Timer 3 Interrupt Service Routine - generates two PWM waves for motor control
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			Interrupting device is acknowledged and its sorce of interrupt is cleared on this level.
* 			Note that VIC of the uP is cleared on the level of OS_CPU_ExceptHndlr function.
* *********************************************************************************************************
*/
extern void Timer3IsrHandler(void);

/*
*********************************************************************************************************
* Name:                                   GetTimer3CounterValue 
* 
* Description: Returns curent value of Timer3 Timer Counter register
*
* Arguments:   none
*
* Returns:     Timer Counter register current value
*
* Note(s):     

* *********************************************************************************************************
*/
extern unsigned long GetTimer3CounterValue(void);


/*
*********************************************************************************************************
* Name:                                    GetTimer3Ticks 
* 
* Description: Timer 3 Interrupt is used also to count its own Ticks which can be read by this function
*
* Arguments:   none
*
* Returns:     Current number of counted Timer3 interrupts
*
* Note(s):     
* 			This Ticks counter can be used for example as a time stamp etc.
* *********************************************************************************************************
*/
extern DWORD GetTimer3Ticks(void);


/*
*********************************************************************************************************
* Name:                                    SetTimer3MR0Count 
* 
* Description: Static variable which is used to setup Timer3 MR0 register is set up with a new value
*
* Arguments:   Count - new value for MR0 register
*
* Returns:     none
* 
* Note: MRO cannot be setup with a value larger to INIT_TIMER3_MR2_COUNT 
*
* *********************************************************************************************************
*/
extern void SetTimer3MR0Count(BYTE InCount);


/*
*********************************************************************************************************
* Name:                                    GetTimer3MR0Count 
* 
* Description: Get current value of the variable which is used to setup Timer3 MR0 register 
* 
* Arguments:   none
*
* Returns:     current number of count prepared to setup Timer3 MR0 register
* 
* *********************************************************************************************************
*/
extern BYTE GetTimer3MR0Count(void);


/*
*********************************************************************************************************
* Name:                                    SetTimer3MR1Count 
* 
* Description: Static variable which is used to setup Timer3 MR1 register is set up with a new value
*
* Arguments:   Count - new value for MR1 register
*
* Returns:     none
* 
* Note: MR1 cannot be setup with a value larger to INIT_TIMER3_MR2_COUNT 
*
* *********************************************************************************************************
*/
extern void SetTimer3MR1Count(BYTE InCount);


/*
*********************************************************************************************************
* Name:                                    GetTimer3MR1Count 
* 
* Description: Get current value of the variable which is used to setup Timer3 MR1 register 
* 
* Arguments:   none
*
* Returns:     current number of count prepared to setup Timer3 MR1 register
* 
* *********************************************************************************************************
*/
extern BYTE GetTimer3MR1Count(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used
 
#endif /*TIMER_H_*/
