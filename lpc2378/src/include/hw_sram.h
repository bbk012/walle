/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_sram.h
* Description: Functions to handle static RAM operations
* Author:      Bogdan Kowalczyk
* Date:        16-Apr-2016
* History:
* 19-Mar-2017 - Initial version polling type UART control for very first trials
* 
*********************************************************************************************************
*/


#ifndef HW_SRAM_H_
#define HW_SRAM_H_

#ifdef __cplusplus
   extern "C" {
#endif

#include "type.h"
	   
//constants which define Walle program requested to be executed
//this requested program is executed by Walle Brain Manager its ExecMngr
//and is setup just before SW Reset request to be run after the RESET
#define WALLE_PROGRAM_BATH 			1 //program to loacate bath and wake-up sleeping Barbara there
#define WALLE_PROGRAM_VOICE_CTRL	2 //program to execute only voice issued commands
#define WALLE_PROGRAM_TEST			3 //program to test Walle's systems
#define WALLE_PROGRAM_ENJOY			4 //program when Wall-e enjoys himself 

#define WALL_MIN_PROGRAM_ID			WALLE_PROGRAM_BATH  //minimum value allowed for Walle program ID	   
#define WALLE_MAX_PROGRAM_ID		WALLE_PROGRAM_ENJOY //maximum value allowed for Walle program ID
	   
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
*********************************************************************************************************
*/
extern BYTE WasBatteryRAMPreserved(void);
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
extern WORD ReadLastPowerOffReason(void);


/*
*********************************************************************************************************
* Name:                                     ReadLastExceptionReason(
* 
* Description: Returns information about exception reason (if any) for last power off
*
* Arguments:   none
*
* Returns:     returns exception reason flag (see lib_error.h for definition)
*
* *********************************************************************************************************
*/
extern WORD ReadLastExceptionReason(void);


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
extern BYTE ReadLastResetReason(void);

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
extern void SetResetReason(BYTE InResetReason);


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
extern BYTE GetWalleProgramToExecute(void);

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
extern void SetWalleProgramToExecute(BYTE InProgramToExecute);


/*
*********************************************************************************************************
* Name:                                    IsBatteryRAMSignatureOK
* 
* Description: Check signature at the static RAM signature positions to be sure that contents of the
*              RTC static RAM is preserved or not
*
* Arguments:   none
*
* Returns:     1 - content is preserved, 0 otherwise
*
* *********************************************************************************************************
*/
extern BYTE IsBatteryRAMSignatureOK(void);

/*
 **********************************************************************************************************
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
extern void SetBatteryRAMSignature(void);
	   
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
extern void InitStaticRam(void);


#ifdef __cplusplus
}
#endif //to close extern "C" if used
   
#endif /*HW_SRAM_H_*/
