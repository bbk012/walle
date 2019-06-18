/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        os_exch.c
* Description: uCOS-II exception handler ported to LPC2378 uP and its Vectored Interrupt Controller
* Author:      Bogdan Kowalczyk
* Date:        11-Aug-2008
* History:
*              11-Aug-2008 - Initial version created
*********************************************************************************************************
*/
#include "hw_lpc23xx.h"
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "os_exch.h"
#include "hw_gpio.h"
#include "type.h"
#include "lib_error.h"
#include "hw_wdt.h"

#define  CRITICAL_INT_NO_OF_LED_BLINKING	50 //number of LED blinking because of criticla interrupt before power off
#define  LED_ON_OFF_CNT		150000	//counter which determines LED blinking frequency
//define BSP_FNCT_PTR as pointer to function wiout parameters and without returned value
typedef void (*BSP_FNCT_PTR)(void);


/*
*********************************************************************************************************
* Name:                                   OS_CPU_ExceptHndlr 
* 
* Description: Common exception handler in C - called from uCOS-II ARM port to handle interrupt
*
* Arguments:   INT32U except_type - exception type (passed by uP R0 register - see os_cpu_a.S file
*
* Returns:     none
*
* Note(s):     
* 			Depending on interrupt LPC2378 Vector Interrupt Controler provided address of handler is used
*           to call interrupt service routine and next VIC interrupt is acknowledged.
*           Device interrupt is acknowledged on device specific handler level not here.
* *********************************************************************************************************
*/
void OS_CPU_ExceptHndlr (INT32U except_type)
{
	BSP_FNCT_PTR pfnct;
	long		j;//to make delays only based on duration of instruction execution
	int			i;//to count number of LED blinkings

	if (except_type == OS_CPU_ARM_EXCEPT_FIQ) 
		{
		
		pfnct = (BSP_FNCT_PTR)VICVectAddr; /* Read the FIQ handler from the VIC. */
		while (pfnct != (BSP_FNCT_PTR)0) 
			{ /* Make sure we don't have a NULL pointer.*/
			(*pfnct)(); /* Execute the handler. */
			VICVectAddr = ~0; /* dummy write to signal End of handler. */
			pfnct = (BSP_FNCT_PTR)VICVectAddr; /* Read the FIQ handler from the VIC. */
			}
		} 
	else if (except_type == OS_CPU_ARM_EXCEPT_IRQ) 
		{

		pfnct = (BSP_FNCT_PTR)VICVectAddr; /* Read the IRQ handler from the VIC. */
		//B.K. if handler adress is none zero and there are IRQs active
		while (pfnct != (BSP_FNCT_PTR)0 &&  VICIRQStatus != 0) 
			{ /* Make sure we don't have a NULL pointer.*/
//Nesting is initailly disabled				OS_CPU_SR_INT_En(); /* OPTIONAL: Enable interrupt nesting. */
			(*pfnct)(); /* Execute the handler. */
//Nesting is initailly disabled				OS_CPU_SR_INT_Dis(); /* Disable interrupt nesting. */
			VICVectAddr = ~0; /* dummy write to signal End of handler. */
			pfnct = (BSP_FNCT_PTR)VICVectAddr; /* Read the IRQ handler from the VIC. */
			}
		}
	else //critical exception signal by LED blinking and next power off 
		{
			/* Stop working when any other interrupt generated */
		for (i = 0; i < CRITICAL_INT_NO_OF_LED_BLINKING; i++ )//blink LED specified number of times and next power off
			{
				FIO0SET = BIT21;	//	LedOn without any function call;
				for (j = 0; j < LED_ON_OFF_CNT; j++ ) //just a delay
				{
					WDFEED=WDT_FEED_1;//refresh watchdog if it is working to avoid reset
					WDFEED=WDT_FEED_2;//direct register manipulation to not call function for exception
				}
				
				FIO0CLR = BIT21; //	LedOff without any function call;
				for (j = 0; j < LED_ON_OFF_CNT; j++ )	//just a delay
				{
					WDFEED=WDT_FEED_1;//refresh watchdog if it is working to avoid reset
					WDFEED=WDT_FEED_2;//direct register manipulation to not call function for exception
				}
			}
		if (except_type == OS_CPU_ARM_EXCEPT_SWI)//when SWI the unique EXCEPTION_REASON is setup just before SWI call
			{
			//to avoid function call uPBoardOff() is not used instead direct access is selected
			POWER_OFF_REASON=EXCEPTION_TURN_OFF;//store reason of power off
			FIO4CLR = BIT12;//turn of main uP board power off directly
			while(1);//do nothing until power is off to avoid processing because of capacitors
			}
		//for any other exception
		//to avoid function call uPBoardOff() is not used instead direct access is selected
		EXCEPTION_REASON=OTHER_EXCEPTION;//mark other than SWI exception reason
		POWER_OFF_REASON=EXCEPTION_TURN_OFF;//store reason of power off
		FIO4CLR = BIT12;//turn of main uP board power off directly
		while(1);//do nothing until power is off to avoid processing because of capacitors
		}
}//OS_CPU_ExceptHndlr

