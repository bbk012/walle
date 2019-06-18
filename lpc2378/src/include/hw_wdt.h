/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_wdt.h
* Description: Functions to control watchdog timer of the LPC2378
* Author:      Bogdan Kowalczyk
* Date:        04-Feb-2018
* Note:
* History:
*              04-Feb-2018 - Initial version created
*********************************************************************************************************
*/
#ifndef HW_WDT_H_
#define HW_WDT_H_

#ifdef __cplusplus
   extern "C" {
#endif
	   
#include "type.h"	
	   
//this value determines watchdog timeout
//watchdog timeout[s]=WDTC*Twdclk*4
//WDTC is watchdog timer constant register
//Twdclk is the period of watchdog clock in our case we apply to watchdog Fplck = 12500000 Hz
//so Twdclk=1/12500000=80ns 4x80ns=320ns
#define WDT_TIMEOUT	 1562500//this corrspond to 1562500*320[ns]=500[ms] every 500ms watchdog need to be fed to avoid reset
	   		//originally I started with 100[ms] next with 200[ms] but it was too small and trigered WDT resets from time to time

//watchdog feed sequence (0xAA followed by 0x55)
#define WDT_FEED_1	0xAA
#define WDT_FEED_2	0x55
	   
	   
//reset reasons stored in Static RAM to identify as well as string with descriptive name
#define NO_RESET	0	//marks none reset requested
#define NO_RESET_STR	"NO"
	   
#define WDT_RESET	1	//marks uP was reseted by watchdog not refreshed 
#define WDT_RESET_STR	"WATCHDOG"	   
	   
#define	SW_RESET	2    //marks uP was reseted by sw request
#define	SW_RESET_STR	"SW"
#define UNDEF_RESET_STR "ERRUNDEF"

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
void  InitWatchdog(void);

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
void uPResetByWatchdog(BYTE InResetReason);

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
*
* *********************************************************************************************************
*/
void WatchdogFeed(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used
   
#endif /*HW_WDT_H_*/
