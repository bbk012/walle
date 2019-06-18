/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_wdt.c
* Description: Functions to control watchdog timer of the LPC2378
* Author:      Bogdan Kowalczyk
* Date:        04-Feb-2018
* Note:
* History:
*              04-Feb-2018 - Initial version created
*********************************************************************************************************
*/

#include "hw_wdt.h"
#include "type.h"
#include "hw_sram.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors

//setup to 1 to use watchdog and to 0 to disable it for example during debug
#define USE_WATCHDOG 1

static BYTE ResetRequested;//used to triger uP RESET if set  

/*
*********************************************************************************************************
* Name:                                    InitWatchdog 
* 
* Description: Call at beginning after uP RESET to setup Watchdog (WDT) initial control values
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* IMPORTANT! With this function call watchdog is setup but not yet run.
* 		Call to WatchdogFeed() function is needed to get watchdog running.
* *********************************************************************************************************
*/

void  InitWatchdog(void)
{
#if USE_WATCHDOG //include code only when watchdog operation is enabled
	
	ResetRequested=FALSE;//mark that there is not any uP RESET request pending
	WDCLKSEL=BIT0;   //select Fplck as the clock source for watchdog
	WDTC=WDT_TIMEOUT;//setup watchdog timeout timer
	WDMOD=BIT1|BIT0; //enable watchdog and reset when watchdog underflow once set those flags are disabled only by RESET
	//watchdog is now setup and enabled but WatchdogFeed() call is needed to start it
#endif//USE_WATCHDOG 	
}//InitWatchdog

/*
*********************************************************************************************************
* Name:                                    uPResetByWatchdog 
* 
* Description: Setup pending reset request so  WatchdogFeed is unable to refresh watchdog  
*       what finally generated uP reset
*
* Arguments:   InResetReason - reason of this reset request to be setup in static RAM
*
* Returns:     none
*
* Note(s):     
*	
* *********************************************************************************************************
*/
void uPResetByWatchdog(BYTE InResetReason)
{
	//setup reason only for SW reset request, WDT reset is triggered and setup different way
	if(InResetReason == SW_RESET) SetResetReason(InResetReason);//setup reset reason just prior the reset call
#if USE_WATCHDOG //include code only when watchdog operation is enabled	
	ResetRequested=TRUE;//setup pending reset request to block WatchdogFeed to refresh watchdog
#endif//USE_WATCHDOG 	
}//uPResetByWatchdog

/*
*********************************************************************************************************
* Name:                                    WatchdogFeed 
* 
* Description: Writes feed sequence to watchdog to reset watchdog timer to avoid anderflow and reset
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*	IMPORTANT! Watchdog timer is refreshed only when ResetRequested=FALSE i.e. uPResetByWatchdog was not called
* *********************************************************************************************************
*/
void WatchdogFeed(void)
{
#if USE_WATCHDOG //include code only when watchdog operation is enabled		
	if(ResetRequested)//when reset requested do not make watchdog refresh this will reset uP
		return;
	
	WDFEED=WDT_FEED_1;//refresh watchdog
	WDFEED=WDT_FEED_2;
#endif//USE_WATCHDOG
}//WatchdogFeed
