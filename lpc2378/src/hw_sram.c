/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_sram.c
* Description: Functions to handle static RAM operations
* Author:      Bogdan Kowalczyk
* Date:        16-Apr-2016
* History:
* 19-Mar-2017 - Initial version polling type UART control for very first trials
* 
*********************************************************************************************************
*/
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"

#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "hw_sram.h"
#include "hw_gpio.h"
#include "lib_error.h"
#include "hw_wdt.h"



//data patterns used to check consistency of battery RAM
//static RAM needs to hold exactlt those data at signature location to assume	
//its contents is consistent and preserved	
#define BATTERY_RAM_SIG1_PATTERN	0x00000000
#define BATTERY_RAM_SIG2_PATTERN	0xAAAAAAAA
#define BATTERY_RAM_SIG3_PATTERN	0x99999999
#define BATTERY_RAM_SIG4_PATTERN	0xFFFFFFFF

volatile static WORD LastPowerOffReason; //keeps power-off reasone restored from static RAM just after power on
volatile static WORD LastExceptionReason;//keeps exception reason restored from waht was preserved in static RAM
volatile static BYTE LastResetReason; //keeps reset reson restored from what was preserved from static RAM
volatile static BYTE WalleProgramToExecute; //keeps walle program to execute as setup in static RAm before last reset request
volatile static BYTE RAMContentPreserved;//TRUE when preserved FALSE othervise



/*
*********************************************************************************************************
* Name:                                     WasBatteryRAMPreserved
* 
* Description: Returns information if from last power off to current run static RAM content was preserved or not
*
* Arguments:   none
*
* Returns:     1 (TRUE) - content is preserved, 0 (FALSE) otherwise
*
* *********************************************************************************************************
*/
BYTE WasBatteryRAMPreserved(void)
{
	return RAMContentPreserved;
}// WasBatteryRAMPreserved


/*
*********************************************************************************************************
* Name:                                     ReadLastPowerOffReason
* 
* Description: Returns information about power off reason proceding current run
*
* Arguments:   none
*
* Returns:     power off reason flag (see hw_gpio.h for definition)
*
* *********************************************************************************************************
*/
WORD ReadLastPowerOffReason(void)
{
	return LastPowerOffReason;
}// ReadLastPowerOffReason


/*
*********************************************************************************************************
* Name:                                     ReadLastExceptionReason
* 
* Description: Returns information about exception reason (if any) for last power off
*
* Arguments:   none
*
* Returns:     returns exception reason flag (see lib_error.h for definition)
*
* *********************************************************************************************************
*/
WORD ReadLastExceptionReason(void)
{
	return LastExceptionReason;
}// ReadLastExceptionReason


/*
*********************************************************************************************************
* Name:                                     ReadLastResetReason
* 
* Description: Determines if there was any RESET requested and uP is restarted becauseo of it or not
*
* Arguments:   none
*
* Returns:     returns reset reason which can be:
*			NO_RESET 	marks none reset requested
*           WDT_RESET 	marks uP was reseted by watchdog not refreshed 
*			SW_RESET    marks uP was reseted by sw requested reset
* *********************************************************************************************************
*/
BYTE ReadLastResetReason(void)
{
	return LastResetReason;
}// ReadLastResetReason

/*
*********************************************************************************************************
* Name:                                     SetResetReason
* 
* Description: Setup just prior the reset a static RAM variable to reflect Reset Reason
*
* Arguments:   reset reason
*			NO_RESET 	marks none reset requested
*           WDT_RESET 	marks uP was reseted by watchdog not refreshed 
*			SW_RESET    marks uP was reseted by sw requested reset
* Returns:   none
* *********************************************************************************************************
*/
void SetResetReason(BYTE InResetReason)
{
	RESET_REASON=InResetReason;//preserve reset reason in static RAM
}//SetResetReason


/*
*********************************************************************************************************
* Name:                                     GetWalleProgramToExecute
* 
* Description: Get ID of the Walle program to be executed for the current run
*
* Arguments:  none
* Returns:   Walle program which is setup in static RAM to be executed by Walle for its current run
* *********************************************************************************************************
*/
BYTE GetWalleProgramToExecute(void)
{
	return WalleProgramToExecute;
}//GetWalleProgramToExecute

/*
*********************************************************************************************************
* Name:                                     SetWalleProgramToExecute
* 
* Description: Set in static RAM id of the program to be executed before you call Walle reset
*
* Arguments:  ID of Walle program to be executed after reset
* Returns:   none
* *********************************************************************************************************
*/
void SetWalleProgramToExecute(BYTE InProgramToExecute)
{
	WALLE_PROGRAM = InProgramToExecute;
}//SetWalleProgramToExecute





/*
*********************************************************************************************************
* Name:                                    IsBatteryRAMSignatureOK
* 
* Description: Check signature at the static RAM signature positions to be sure that contents of the
*              RTC static RAM is preserved or not
*
* Arguments:   none
*
* Returns:     TRUE (1) - content is preserved, FALSE (0) otherwise
*
* *********************************************************************************************************
*/
BYTE IsBatteryRAMSignatureOK(void)
{
	if(BATTERY_RAM_SIG1!=BATTERY_RAM_SIG1_PATTERN)return FALSE;
	if(BATTERY_RAM_SIG2!=BATTERY_RAM_SIG2_PATTERN)return FALSE;
	if(BATTERY_RAM_SIG3!=BATTERY_RAM_SIG3_PATTERN)return FALSE;
	if(BATTERY_RAM_SIG4!=BATTERY_RAM_SIG4_PATTERN)return FALSE;
	return TRUE;//all signatures are kept unchanged so RAM must be preserved
}//IsRAMPreserved

/*
*********************************************************************************************************
* Name:                                    SetBatteryRAMSignature
* 
* Description: Stores signatures in the static RAM signature position for future checks
*
* Arguments:   none
*
* Returns:     none
*
* *********************************************************************************************************
*/
void SetBatteryRAMSignature(void)
{
	BATTERY_RAM_SIG1=BATTERY_RAM_SIG1_PATTERN;
	BATTERY_RAM_SIG2=BATTERY_RAM_SIG2_PATTERN;
	BATTERY_RAM_SIG3=BATTERY_RAM_SIG3_PATTERN;
	BATTERY_RAM_SIG4=BATTERY_RAM_SIG4_PATTERN;
	return;
}//SetBatteryRAMSignature


/*
*********************************************************************************************************
* Name:                                    InitStaticRam
* 
* Description: Check if battery powered static RAM keeps data, initialize static RAM accordingly
* 			   and keeps right status information about last power off reason and last critical exception reason
*
* Arguments:   none
*
* Returns:     none
*
* *********************************************************************************************************
*/
void InitStaticRam()
{
	if(IsBatteryRAMSignatureOK())//when static RAM content OK use it
	{
		RAMContentPreserved = TRUE; //setup flag to signal static RAM is preserved from last power off
		LastPowerOffReason =  POWER_OFF_REASON; //keep last power off reason preserved for current run before defaults are setup in static RAM
		LastExceptionReason = EXCEPTION_REASON; //keep last exception reason preserved for current run before defaults are setup in static RAM
		LastResetReason = RESET_REASON;//keep last reset reason preserved for current run before defaults are setup in static RAM
		WalleProgramToExecute = WALLE_PROGRAM;//get requested Walle program to be executed as preserved in static RAM
			
		if(LastResetReason == NO_RESET && WDMOD&BIT2)//when SRAM does not reflect SW requested reset
		{				//but WDTOF bit in WDMOD is set it means WDT_RESET (watchdog was not refreshed as needed)
			LastResetReason = WDT_RESET;
			WDMOD&=(~BIT2);//clear WDTOF flag (BIT2 position)
		}
		POWER_OFF_REASON = UNSET_TURN_OFF;//mark unset for the case power is disconected suddenly
		EXCEPTION_REASON = NONE_EXCEPTION;//mark none exception until any is generated for power off
		RESET_REASON = NO_RESET;//mark none reset until any is generated
		//setup program to excute as default for the power off case without program to be setup
		//in that case continue what was setup before power off
		WALLE_PROGRAM = WalleProgramToExecute;
	}
	else //when static RAM content not preserved setup defaults
	{
		RAMContentPreserved = FALSE;//setup that ram content was not preserved
		SetBatteryRAMSignature();//set up RAM signature for future check
		
		POWER_OFF_REASON = UNSET_TURN_OFF;//mark in SRAM unset for the case power is disconected suddenly
		EXCEPTION_REASON = NONE_EXCEPTION;//mark in SRAM none exception until any is generated for power off
		RESET_REASON = NO_RESET;//mark none reset until any is generated
		WALLE_PROGRAM = WALLE_PROGRAM_BATH ;//setup default program to execute for the case static RAM information is not presereved
		
		LastPowerOffReason = UNSET_TURN_OFF;//when SRAM values not presereved assume defaults
		LastExceptionReason = NONE_EXCEPTION;
		LastResetReason = NO_RESET;
		WalleProgramToExecute = WALLE_PROGRAM_BATH;//setup default value when RAM content not preserved
	}//
}//InitStaticRam
