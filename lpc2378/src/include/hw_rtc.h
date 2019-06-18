/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_rtc.h
* Description: Functions to control Real Time Clock module of the LPC2378 uC on OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        11-Nov-2010
* History:
* 11-Nov-2010 - Initial version 
*********************************************************************************************************
*/
#ifndef HW_RTC_H_
#define HW_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "type.h"
#include "lib_time.h"
	
//constants used by RTCSetAlarmState() function
#define TURN_ON_DATE_ALARM		1
#define TURN_ON_TIME_ALARM		1
#define TURN_OFF_DATE_ALARM		0
#define TURN_OFF_TIME_ALARM		0

//default date and time of RTC entered by system when RTC battery is not working	
#define DEFAULT_RTC_YEAR		2014
#define DEAFULT_RTC_MONTH		1
#define DEFAULT_RTC_DAY			12	
#define DEFAULT_RTC_DAY_OF_WEEK 0
#define DEFAULT_RTC_HOUR		12
#define DEFAULT_RTC_MIN			00
#define DEFAULT_RTC_SEC			00


	
/*
*********************************************************************************************************
* Name:                                    InitRTC 
* 
* Description: Call at beginning after uP RESET to setup RTC initial control values
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* IMPORTANT! Register values of RTC are kept setup after power off (battery backup)
* Initial setup needs take that into account.
* *********************************************************************************************************
*/
extern void InitRTC(void);

/*
*********************************************************************************************************
* Name:                                    RTCIsrHandler 
* 
* Description: Real Time Clock Interrupt service routine
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			Interrupting device is acknowledged and its sorce of interrupt is cleared on this level.
* 			Note that VIC of the uP is cleared on the level of OS_CPU_ExceptHndlr function.
* 			As a result of interrupt acknowledge ALARM output of the uC should switch from HIGH to LOW state
* *********************************************************************************************************
*/
extern void RTCIsrHandler(void);


/*
*********************************************************************************************************
* Name:                                    GetAlarmState 
* 
* Description: Get status of RTC alarm
*
* Arguments:   none
*
* Returns:     0 - no alarm triggered, 1 - alarm trigerred
*
* *********************************************************************************************************
*/
extern BYTE GetAlarmState(void);


/*
*********************************************************************************************************
* Name:                                    ClrAlarmState 
* 
* Description: Clear alarm status set to 1 by RTC in case of alarm
*
* Arguments:   none
*
* Returns:     none
*
* *********************************************************************************************************
*/
extern void ClrAlarmState(void);

/*
*********************************************************************************************************
* Name:                                    RTCGetAlarmStateDateTime 
* 
* Description: Get stored date for the alarm state (when alarm was triggered)
*
* Arguments:   
* 				pDateTime - pointer to structure where alarm date and time will be stored
* Returns:     none
*
* Note(s):   
*				none
* *********************************************************************************************************
*/
extern void RTCGetAlarmStateDateTime(tmElements_t *pDateTime);

/*
*********************************************************************************************************
* Name:                                    RTCSetupDefaultDateTime
* 
* Description: Setup a date and time in the Real Time Clock using tmElements_t structure
*
* Arguments:   
*			none
*			
* Returns:  none
*
* *********************************************************************************************************
*/
extern void RTCSetupDefaultDateTime(void);

/*
*********************************************************************************************************
* Name:                                    RTCSetupDateTime 
* 
* Description: Seup a date in the Real Time Clock using tmElements_t structure
*
* Arguments:   
*
*			pDateTime - pointer to structure where date and time is stored 
* Returns:  none
*
* Note(s):   
*	IMPORTANT! There is not any check of validity of the values provided they are only used   
*              to initialize counters of the RTC which are next simply incremented by RTC
* 
*   tmElements_t Year is counted as offset from 1970 while in RTC it is abcolute
* *********************************************************************************************************
*/
extern void RTCSetupDateTime(tmElements_t *pDateTime);

/*
*********************************************************************************************************
* Name:                                    RTCGetDateTime 
* 
* Description: Get a date from the Real Time Clock using tmElements_t structure
*
* Arguments:   
*			pDateTime - pointer to structure where date and time is stored 
* 
* Returns:     none
*
* Note(s):   
*	
* *********************************************************************************************************
*/
extern void RTCGetDateTime(tmElements_t *pDateTime);


/*
*********************************************************************************************************
* Name:                                    RTCSetupDateTimeAlarmValue 
* 
* Description: Setup alarm date and time in the Real Time Clock using tmElements_t structure
*
* Arguments:   
*		pDateTime - pointer to structure where date and time to setup is stored 
* 
* Returns:     none
*
* Note(s):   
*	IMPORTANT! Date Alarm Value is only set-up but alarm is not enabled
* 			   RTCSetAlarm need to be called to turn date alram on
* *********************************************************************************************************
*/
extern void RTCSetupDateTimeAlarmValue(tmElements_t *pDateTime);


/*
*********************************************************************************************************
* Name:                                    RTCGetDateTimeAlarmValue 
* 
* Description: Get date amd time alarm value from the Real Time Clock using tmElements_t structure
*
* Arguments:   
*		pDateTime - pointer to structure where date and time is stored 
* 
* Returns:     none
*
* Note(s):   
*				none
* *********************************************************************************************************
*/
extern void RTCGetDateTimeAlarmValue(tmElements_t *pDateTime);

/*
*********************************************************************************************************
* Name:                                    RTCSetAlarmState
* 
* Description: Turn on or turn off Alarm for Date and/or Time
*
* Arguments:   
* 				InDateAlarmState (0 - turn off <>0 turn on date alarm)
* 				InTimeAlarmState (0 - turn off <>0 turn on time alarm)
* 
*                  |                  |
* InDateAlarmState | InTimeAlarmState |       Action
*                  |                  |
* -----------------+------------------+----------------------------
*        0         |        0         | turn OFF date & turn OFF time alarm
* -----------------+------------------+----------------------------
*        0         |        1         | turn OFF date & turn ON time alarm
* -----------------+------------------+----------------------------
*        1         |        0         | turn ON date & turn OFF time alarm
* -----------------+------------------+----------------------------
*        1         |        1         | turn ON date & turn ON time alarm
* -----------------+------------------+----------------------------
* 
* 
* Returns:     none
*
* Note(s):     none
* *********************************************************************************************************
*/
extern void RTCSetAlarmState(BYTE InDateAlarmState, BYTE InTimeAlarmState);

/*
*********************************************************************************************************
* Name:                                    GetRTCAlarmState 
* 
* Description: Get status of Date and Time alarm
*
* Arguments:   
* 				pInDateAlarmState - pointer to Date alarm state ( BYTE 0 - turn off <>0 turn on date alarm)
* 				pInTimeAlarmState - pointer to Time alarm state ( BYTE 0 - turn off <>0 turn on time alarm)
* 
*                    |                    |
* *pInDateAlarmState | *pInTimeAlarmState |       Alarm State
*                    |                    |
* -------------------+--------------------+----------------------------
*        0           |        0           | date turn OFF & time turn OFF
* -------------------+--------------------+----------------------------
*        0           |        1           | date turn OFF & time turn ON
* -------------------+--------------------+----------------------------
*        1           |        0           | date turn ON & time turn OFF
* -------------------+--------------------+----------------------------
*        1           |        1           | date turn ON & time turn ON
* -------------------+--------------------+----------------------------
* 
* 
* Returns:     none
*
* Note(s):     none
* *********************************************************************************************************
*/
extern void GetRTCAlarmState(BYTE *pInDateAlarmState, BYTE *pInTimeAlarmState);

/*
*********************************************************************************************************
* Name:                                    IsAlarmSetupInRTC 
* 
* Description: Checks if there is alarm setup in RTC
*
* Arguments:   
*				none 
* 
* Returns:     TRUE if alarm is setup in RTC FALSE otherwise
*
* Note(s):   
*				none
* *********************************************************************************************************
*/
extern BYTE IsAlarmSetupInRTC(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*HW_RTC_H_*/
