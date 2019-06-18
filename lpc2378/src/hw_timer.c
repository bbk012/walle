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
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "type.h"
#include "hw_lpc2378.h"
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "hw_spi.h"
#include "hw_gpio.h"
#include "hw_timer.h"
#include "tsk_tracks.h"

#include "lib_std.h"


//Remove MOVE DEBUG conditional compilation
#define DEBUG_TIMER 0

// Following is timer interrupts assignament
// Timer0 - Ultrasonic handling through tomer 0 capture channel
// Timer1 - ARS sensor angle integration
// Timer2 - uCOS-II OS timer tick
// Timer3 - PWM for motor control

//timer0 capture function is used to handle US sensor and to measure Echo response duration
static volatile DWORD Timer0StartCAP;//timer counter value captured on Echo signal rising edge
static volatile DWORD Timer0StopCAP;//timer counter value captured on Echo signal falling edge
static volatile BYTE  Timer0IsCounting;//flag which is setup to TRUE when timer0 is counting

//MRxCurrentCount variables are used to setup Timer3 Match Registers
//During timer3 work match registers are setup by interrupt service routine
//in that way we change Match Registers not directly but indirectly to avoid
//glitches on PWM waves etc.
static volatile BYTE Timer3MR0CurrentCount = INIT_TIMER3_MR0_COUNT;//Right Motor
static volatile BYTE Timer3MR1CurrentCount = INIT_TIMER3_MR1_COUNT;//Left Motor
static volatile DWORD Timer3Ticks;//Ticks counted by Timer3 ISR can be used as time stamps etc.

//ArsIntegratedAngle is variable which holds integrated ARS sensor output counts (turn angle)
//This angle is calculated by Timer1 interrupt handler (every 1ms sampling) when Timer1
//is enabled to integrate ARS output.
//The ArsIntegratedAngle can be positive - right turn or negative left turn
//The integration is based on trapezoid algorithm but sum of samples is not finally divided by 2
static volatile LONG ArsIntegratedAngle;
//The ArsOffset - holds ARS output value for stable state i.e. when there is not turn
static volatile LONG ArsOffset;
//place-holder for sample N-1 of the ARS output used during ARS angle calculation
static volatile LONG ArsSampleN_1;
//place holder for sample N of the ARS output used during ARS angle calculation
static volatile LONG ArsSampleN;
//count for the destination angle when reached motors are stopped
static volatile LONG TargetArsIntegratedAngle;




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
* *********************************************************************************************************
*/
void InitTimer0 (void)
{
	Timer0StartCAP=0;//clear timer0 captured values for ultrasonic Echo signal at startup
	Timer0StopCAP=0;
	Timer0IsCounting=FALSE;//until started timer0 is not counting
	PCONP|=BIT1;//enable Timer0 - even when by default on uP reste it is on
	PINSEL7|=BIT15;//setup BIT15=1 and BIT14=0 so P3.23 pin is configured as Timer0 CAP0 pin to handle US Echo input
	T0IR=0xFF;//clear all Timer 0 interrupts
	T0TCR=0x00;//disble Timer 0 countings
	T0PR=TIMER0_PRESCALER_VALUE;//setup prescaler to divide Fplck/(TIMER0_PRESCALER_VALUE+1) so 1uS pulses are counted
	T0TCR|=BIT1;//reset timer counter and prescaler counter
	//setup VIC to handle timer0 interrupt when generated
	VICIntEnClr = BIT4;//disable timer 0 interrupt
	VICVectAddr4 = (DWORD)Timer0IsrHandler; //assign address to Timer0 IRQ Handler handler
	//timer 0 and its interrupts are enabled in Timer0Start function
}//InitTimer0

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
* *********************************************************************************************************
*/
void Timer0IsrHandler(void)
{
	if(!Timer0StartCAP)//when StartCAP value not yet setup it means Echo raising edge 
	{
		Timer0StartCAP=T0CR0;//read capture register value stored at raising Echo edge
		T0CCR=BIT2|BIT1;//setup to capture on falling CAP0 edge and to generate interrupt for capture event
	}
	else//when Echo falling edge detected
	{
		Timer0StopCAP=T0CR0;//read capture register value stored at falling Echo edge
		Timer0IsCounting=FALSE;//mark Echo pulse duration caounting is ended
	}
	T0IR=0xFF;//clear timer 0 interrupts
}//Timer0IsrHandler

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

* *********************************************************************************************************
*/
void Timer0Start(void)
{
	Timer0StartCAP=0;//clear timer0 captured values for ultrasonic Echo signal at start
	Timer0StopCAP=0;
	T0TCR=0x00;//disble Timer 0 countings
	T0TCR|=BIT1;//reset timer counter and prescaler counter to start from 0
	T0CCR=BIT2|BIT0;//capture on raising CAP0 edge and generate interrupt for capture event
	VICIntEnable |= BIT4;//enable timer 0 interrupts
	T0TCR = 0x01;//enable counting for timer 0
	Timer0IsCounting=TRUE;//mark timer0 started to count
}//Timer0Start

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

* *********************************************************************************************************
*/
void Timer0Stop(void)
{
	T0IR=0xFF;//clear all Timer 0 interrupts
	T0TCR=0x00;//disble Timer 0 countings
	T0CCR=0x00;//disable all capture related features
	VICIntEnClr = BIT4;//disable timer 0 interrupt
	Timer0IsCounting=FALSE;//mark Echo pulse duration caounting is ended
}//Timer0Stop


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
void WaitTimer0_10uS(void)
{
	DWORD Start=T0TC;//read start value of timer counter
	while (T0TC <= Start+TIMER0_10uS)
	{
		//do nothing just wait for 10uS to expire as measured by timer0
	}
}//WaitTimer10uS

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
BYTE IsTimer0Counting(void)
{
	return Timer0IsCounting;
}//IsTimer0Counting


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
DWORD GetTimer0StartCAP(void)
{
	return Timer0StartCAP;
}//GetTimer0StartCAP

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
DWORD GetTimer0StopCAP(void)
{
	return Timer0StopCAP;
}//GetTimer0StopCAP

/*
*********************************************************************************************************
* Name:                                    InitTimer1  
* 
* Description: Setup timer 1 to provide timer interrupts every ARS_SAMPLE_PER_SEC (1ms) when enabled.
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
void InitTimer1 (void)
{
	T1IR = 0xFF; //clear all timer interrupts
	T1TCR = 0x00; //disable counter
	T1MCR = 0x03; // interrupt on match and next reset (match register 0 is used in Timer 1)
	T1MR0 = (Fpclk/ARS_SAMPLE_PER_SEC); //set match value to get ARS_SAMPLE_PER_SEC rate of sampling
	
	//setup VIC to generate IRQ for timer 1 generated interrupt interrupt
	VICIntEnClr = BIT5;//disable timer interrupt
	VICVectAddr5 = (DWORD)Timer1IsrHandler; //assign address to Timer1 IRQ Handler handler
	VICIntEnable |= BIT5;//enable timer interrupts
//	T1TCR = 0x01;//enable counter - commented out a separate call is required to enable timer 1
	ArsIntegratedAngle=0L;
	ArsSampleN_1=0L;
	ArsSampleN=0L;
}//InitTimer1

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
void SetArsOffset(void)
{

	ArsOffset = (LONG)CalculateAdcOffset(ADC_ARS_SENSOR,ARS_OFFSET_SAMPLE_COUNT,ARS_OFFSET_SAMPLE_TICKS);	

}//SetArsOffset

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
LONG GetArsOffset(void)
{
	return ArsOffset;
}//GetArsOffset

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
void StartArsAngleCounting(void)
{
	T1IR = 0xFF; //clear all timer interrupts
	T1TCR = 0x00; //disable counter for a moment
	
	ArsIntegratedAngle=0L;//clear angle to let it be calculated from zero
	ArsSampleN_1=0L;//assume 0 value for samples before integration start
	ArsSampleN=0L;

	T1TCR = 0x01;//enable counter to integrate angle every timer1 interrupt
}//StartArsAngleCounting

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
void StopArsAngleCounting(void)
{
	T1IR = 0xFF; //clear all timer interrupts
	T1TCR = 0x00; //disable timer counting
}//StopArsAngleCounting


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
LONG GetArsAngleValue(void)
{
	return ArsIntegratedAngle;
}//GetArsAngleValue



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
void SetArsTargetAngle(LONG InTargetAngle)
{
	TargetArsIntegratedAngle=InTargetAngle;
}//SetArsTargetAngle

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
LONG GetArsTargetAngle(void)
{
	return TargetArsIntegratedAngle;
}//GetArsTargetAngle

/*
*********************************************************************************************************
* Name:                                    Timer1IsrHandler  
* 
* Description: Timer 1 Interrupt Service Routine - integrates ARS output
*              Once integrated angle coresspond to the target angle the tracks are stopped.
* IMPORTANT!
* 			   Trapezoid integration method is used but sum of sample (N-1) and sample (N) 
* 	           is not averaged for simplicity.
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
void Timer1IsrHandler(void)
{
	ArsSampleN=(LONG)GetAdcConversion(ADC_ARS_SENSOR)-ArsOffset;//read ARS signal and normalize to the offset level
	if (labs(ArsSampleN) < ARS_SAMPLE_THRESHOLD)ArsSampleN=0;//take into account only meaningful samples not noise
	ArsIntegratedAngle+=ArsSampleN+ArsSampleN_1;//normally this should be an average but it can be also just a sum - just final value is twice larger
	ArsSampleN_1=ArsSampleN;//preserve current normalized sample for next sampling calculation
	if(labs(ArsIntegratedAngle) >= labs(TargetArsIntegratedAngle))// when target angle achieved
	{
		T1TCR = 0x00; //disable timer counting
		StopTracks(); //stop movement
	}

	T1IR = 0xFF;  //clear all timer interrupts to be ready to handle next interrupt
}//Timer1IsrHandle


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

void InitTimer2 (void)
{
	PCONP|=BIT22;//enable Timer2 - by default on uP reste it is off
	T2IR = 0xFF; //clear all timer interrupts
	T2TCR = 0x00; //disable counter
	T2MCR = 0x03; // interrupt on match and next reset (match register 0 is used in Timer 2)
	T2MR0 = (Fpclk/OS_TICKS_PER_SEC); //set match value to get OS_TICKS_PER_SEC
	
	//setup VIC to generate IRQ for timer 2 generated interrupt interrupt
	VICIntEnClr = BIT26;//disable timer interrupt
	VICVectAddr26 = (DWORD)Timer2IsrHandler; //assign address to Timer2 IRQ Handler handler
	VICIntEnable |= BIT26;//enable timer interrupts
	T2TCR = 0x01;//enable counter
}//InitTimer2

/*
*********************************************************************************************************
* Name:                                     Timer2IsrHandler 
* 
* Description: Timer 2 Interrupt Service Routine - provide timer timer tick to uCOS-II
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
void Timer2IsrHandler(void)
{
	OSTimeTick(); //call uCOS-II tick function
	T2IR = 0xFF;  //clear all timer interrupts
}//Timer2IsrHandle


/* 
 * Timer3 is used to generate two PWM waves in the following way:
 * Match Register 2 of Timer 3 is used to generate periodic interrupts - common PWM frequency (25kHz)
 * March Register 0 is used to generate PWM1 channel
 * Match Register 1 is used to generate PWM2 channel
 * PWM1 is generated on uP Pin 69
 * PWM2 is generated on uP Pin 70
 */
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
void InitTimer3 (void)
{
	Timer3Ticks=0;//clear Timer3 Ticks counter ones systems start
	PCONP|=BIT23;//enable Timer3 - by default on uP reste it is off
	PINSEL0|=BIT20|BIT21|BIT22|BIT23;//put Timer3 match register 0 on pin 69 and match register 1 on pin 70
	T3IR=0xFF;//clear all Timer 3 interrupts
	T3TCR=0x00;//disble Timer 3 countings
	T3PR=TIMER3_PRESCALER_VALUE;//setup prescaler to divide Fplck/(TIMER3_PRESCALER_VALUE+1)
	//setup match registers values
	T3MR2=INIT_TIMER3_MR2_COUNT;//this is used to generate common PWM frequency
	T3MR1=Timer3MR1CurrentCount;//this is used to generate PWM2
	T3MR0=Timer3MR0CurrentCount;//this is used to generate PWM1
	T3MCR|=BIT6|BIT7;//interrupt on MR2 and counter reset on MR2
	T3EMR|=BIT0|BIT1|BIT4|BIT6;//external pin set to high for init and reset on match
	
	//setup VIC to generate IRQ for timer 3 on MR2 match
	VICIntEnClr = BIT27;//disable timer 3 interrupt
	VICVectAddr27 = (DWORD)Timer3IsrHandler; //assign address to Timer3 IRQ Handler handler
	VICIntEnable |= BIT27;//enable timer 3 interrupts
	T3TCR = 0x01;//enable counting for timer 3
}//InitTimer3

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
void Timer3IsrHandler(void)
{
	T3EMR|=BIT0|BIT1;//PWM1 and PWM2 pins set to high
	T3MR1=Timer3MR1CurrentCount;//load new counts to generate PWM2
	T3MR0=Timer3MR0CurrentCount;//load new counts to generate PWM1
	T3IR=0xFF;//clear timer 3 interrupts
	Timer3Ticks+=1;//increment Ticks counter every 120us (determined by T3MR2=INIT_TIMER3_MR2_COUNT)
}//Timer3IsrHandler

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
unsigned long GetTimer3CounterValue(void)
{
	return T3TC;
}//GetTimer3CounterValue

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
DWORD GetTimer3Ticks(void)
{
	return Timer3Ticks; 
}//GetTimer3Ticks

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
void SetTimer3MR0Count(BYTE InCount)
{
	if(InCount >= INIT_TIMER3_MR2_COUNT)
	{
		Timer3MR0CurrentCount = INIT_TIMER3_MR2_COUNT;
	}
	else
	{
		Timer3MR0CurrentCount = InCount;
	}
}//SetTimer3MR0Count

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

BYTE GetTimer3MR0Count(void)
{
	return Timer3MR0CurrentCount;
	
}//GetTimer3MR0Count

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
void SetTimer3MR1Count(BYTE InCount)
{
	if(InCount >= INIT_TIMER3_MR2_COUNT)
	{
		Timer3MR1CurrentCount = INIT_TIMER3_MR2_COUNT;
	}
	else
	{
		Timer3MR1CurrentCount = InCount;
	}
}//SetTimer3MR1Count

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

BYTE GetTimer3MR1Count(void)
{
	return Timer3MR1CurrentCount;
	
}//GetTimer3MR1Count

