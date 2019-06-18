/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_rtc.c
* Description: Functions to control Real Time Clock module of the LPC2378 uC on OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        11-Nov-2010
* History:
* 11-Nov-2010 - Initial version 
*********************************************************************************************************
*/
#include "hw_rtc.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "hw_sram.h"
#include "type.h"
#include "hw_gpio.h"

volatile static BYTE AlarmState;//when set by ISR means alarm occured to be able to chack it later on if neded
//IMPORTANT! This variable only is used to hold alarm state when Wall-e is turn on, not capable to hold alarm state for alarm triggered when Wall-e is power off

//snapshot of alarm data when AlarmState is setup to 1 (alarm reported)
volatile static WORD AlarmYear;
volatile static BYTE AlarmMonth;
volatile static BYTE AlarmDay;
volatile static BYTE AlarmHour;
volatile static BYTE AlarmMin;
volatile static BYTE AlarmSec;

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

void InitRTC(void)
{
	PCONP |=BIT9;//turn on power control bit for RTC to analbe its register access from the program
	//setup VIC to generate IRQ for RTC
	VICIntEnClr = BIT13;//disable RTC interrupts once we operate RTC timer
	
	VICVectAddr13 = (DWORD)RTCIsrHandler; //assign address to IRQ Handler
	
	if (WasBatteryRAMPreserved())//when RTC battery operated correctly
	{
		if( ALARM_TURN_OFF == ReadLastPowerOffReason())//when last power off reason was because of alarm
		{//check if RTC alarm interrup flag was set up
			if (RTC_ILR&BIT1)//if alarm flag is setup it means this power on is really because of Alarm output turning it
			{
				AlarmState = TRUE;//mark there is alarm triggered (Wall-e is power on by alarm)
				//get alarm data to know for what moment alarm was triggered
				AlarmYear  = (WORD)RTC_ALYEAR;
				AlarmMonth = (BYTE)RTC_ALMON;
				AlarmDay   = (BYTE)RTC_ALDOM;
			
				AlarmHour = (BYTE)RTC_ALHOUR;
				AlarmMin  = (BYTE)RTC_ALMIN;
				AlarmSec  = (BYTE)RTC_ALSEC;
			}
			else //alarm was not trigerred even when power off was because of alarm setup
			{
				AlarmState = FALSE;//mark power is on but not because of alarm
			}
		} else //previous turn off was not because of alarm wake up request
		{
			AlarmState = FALSE;//mark power is on but not because of alarm	
		}
	}
	else //there was not power provided to RTC when Wall-e was turned of so we cannot judeg anything about alarm
	{
		RTCSetAlarmState(TURN_OFF_DATE_ALARM, TURN_OFF_TIME_ALARM);//turn off all alarms
		ClrAlarmState();//clear alarm state flag and alarm time and date snapshot
		RTCSetupDefaultDateTime();//set intial date and time for the case when data are not preserved
	}
	
	//IMPORTANT! Those values are maybe already setup
	//but we setup them here without checking to cover both preserved/setup and not preserved/not setup cases
    
	RTC_CIIR = 0x00000000;//disable interrupts from incremented counters
	RTC_CISS = 0x00000000;//disable interrupts from sub-second intervals
	
	RTC_ILR = BIT0|BIT1|BIT2;//clear all eventual set interrupts -should change uC ALARM output to LOW state if HIGH
	RTC_CCR = BIT0|BIT4;// Enable time counters and set clock source to 32kHz external oscilator
	VICIntEnable |= BIT13;//enable RTC interrupts in VIC
	//IMPORTANT! Alarm setup function is used to enable RTC interrupts inside RTC module
	//           Because of battery backup function they may already be setup so we cannot setup them here	
	//           to not overwritte them
	//           That means when there is RTC powered in power-off state of Wall-e alarm setup is preserved	

	return;
}//InitRTC


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
void RTCIsrHandler(void)
{

	AlarmState=TRUE;//mark there is alarm triggered

	
	//get alarm data to know for what moment alarm was triggered
	AlarmYear  = (WORD)RTC_ALYEAR;
	AlarmMonth = (BYTE)RTC_ALMON;
	AlarmDay   = (BYTE)RTC_ALDOM;
	
	AlarmHour = (BYTE)RTC_ALHOUR;
	AlarmMin  = (BYTE)RTC_ALMIN;
	AlarmSec  = (BYTE)RTC_ALSEC;

	RTC_ILR = BIT0|BIT1|BIT2;//clear all eventual set interrupts -should change uC ALARM output to LOW state if HIGH
}//RTCIsrHandler



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
BYTE GetAlarmState(void)
{
	return AlarmState;
}//GetAlarmState

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
void ClrAlarmState(void)
{
	AlarmState=FALSE;//mark no alarm reported by RTC and clear alarm date and time information
	AlarmYear=0;
	AlarmMonth=0;
	AlarmDay=0;
	AlarmHour=0;
	AlarmMin=0;
	AlarmSec=0;
}//ClrAlarmState

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
void RTCGetAlarmStateDateTime(tmElements_t *pDateTime)
{
	pDateTime->Year=(BYTE)CalendarYrToTm(AlarmYear);//convert normal year to number of years since 1970 as required by tmElements_t struct
	pDateTime->Wday=0;//this is not setup for alarm and should not be used
	pDateTime->Month=AlarmMonth;
	pDateTime->Day=AlarmDay;
	pDateTime->Hour=AlarmHour;
	pDateTime->Minute=AlarmMin;
	pDateTime->Second=AlarmSec;
	
	return;
}//RTCGetAlarmStateDate

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
void RTCSetupDefaultDateTime(void)
{
	RTC_CCR &= (~BIT0);//disable RTC clock until we setup registers
	
	RTC_YEAR  = (WORD)(DEFAULT_RTC_YEAR);
	RTC_MONTH = DEAFULT_RTC_MONTH;
	RTC_DOM   = DEFAULT_RTC_DAY;
	RTC_DOW   = DEFAULT_RTC_DAY_OF_WEEK;
	RTC_DOY   = 0;//day of year is not used
	
	RTC_HOUR = DEFAULT_RTC_HOUR;
	RTC_MIN  = DEFAULT_RTC_MIN;
	RTC_SEC  = DEFAULT_RTC_SEC;
	
	//setup also defaults for alarm even when alarm turn on or off is not exacuted here
	RTC_ALYEAR  = (WORD)(DEFAULT_RTC_YEAR);
	RTC_ALMON = DEAFULT_RTC_MONTH;
	RTC_ALDOM  = DEFAULT_RTC_DAY;
		
	RTC_ALHOUR = DEFAULT_RTC_HOUR;
	RTC_ALMIN  = DEFAULT_RTC_MIN;
	RTC_ALSEC  = DEFAULT_RTC_SEC;
		
	RTC_CCR = BIT0|BIT4;// Enable time counters and set clock source to 32kHz external oscilator
}//RTCSetupDefaultDateTime


/*
*********************************************************************************************************
* Name:                                    RTCSetupDateTime 
* 
* Description: Setup a date and time in the Real Time Clock using tmElements_t structure
*
* Arguments:   
*
*			pDateTime - pointer to structure where date and time to setup are stored 
* Returns:  none
*
* Note(s):   
*	IMPORTANT! There is not any check of validity of the values provided they are only used   
*              to initialize counters of the RTC which are next simply incremented by RTC
* 
*   tmElements_t Year is counted as offset from 1970 while in RTC it is abcolute
* *********************************************************************************************************
*/
void RTCSetupDateTime(tmElements_t *pDateTime)
{
	RTC_CCR &= (~BIT0);//disable RTC clock until we setup registers
	
	RTC_YEAR  = (WORD)tmYearToCalendar(pDateTime->Year);//convert tm format of year to RTC one
	RTC_MONTH = pDateTime->Month;
	RTC_DOM   = pDateTime->Day;
	RTC_DOW   = pDateTime->Wday;
	RTC_DOY   = 0;//day of year is not used
	
	RTC_HOUR = pDateTime->Hour;
	RTC_MIN  = pDateTime->Minute;
	RTC_SEC  = pDateTime->Second;
	
	RTC_CCR |= BIT0;//enable RTC clock
	
}//RTCSetupDateTime

/*
*********************************************************************************************************
* Name:                                    RTCGetDateTime 
* 
* Description: Get a date and time from the Real Time Clock using tmElements_t structure
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
void RTCGetDateTime(tmElements_t *pDateTime)
{
	BYTE CurrentSeconds;//temp storage for current RTC seconds used to exclude time overrun

	pDateTime->Second=(BYTE)RTC_SEC;
	pDateTime->Minute=(BYTE)RTC_MIN;
	pDateTime->Hour=(BYTE)RTC_HOUR;
	pDateTime->Day=(BYTE)RTC_DOM;
	pDateTime->Wday=(BYTE)RTC_DOW;
	pDateTime->Month=(BYTE)RTC_MONTH;
	pDateTime->Year=(BYTE)CalendarYrToTm((WORD)RTC_YEAR);//convert normal year to number of years since 1970 as required by tmElements_t struct
	
	CurrentSeconds=(BYTE)RTC_SEC;//get seconds after date and time reading to check of seconds overrun
	
	if(pDateTime->Second > CurrentSeconds)//check if time overrun occured after date and time reading
	{//read data again because during date reading there was time overrun 
		//another overrun is not possible because we have about 60 seconds for reading (before next overrun)
		pDateTime->Second=(BYTE)RTC_SEC;
		pDateTime->Minute=(BYTE)RTC_MIN;
		pDateTime->Hour=(BYTE)RTC_HOUR;
		pDateTime->Day=(BYTE)RTC_DOM;
		pDateTime->Wday=(BYTE)RTC_DOW;
		pDateTime->Month=(BYTE)RTC_MONTH;
		pDateTime->Year=(BYTE)CalendarYrToTm((WORD)RTC_YEAR);//convert normal year to number of years since 1970 as required by tmElements_t struct
	}
	
}//RTCGetDateTime



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
void RTCSetupDateTimeAlarmValue(tmElements_t *pDateTime)
{
	RTC_ALYEAR  = (WORD)tmYearToCalendar(pDateTime->Year);//convert tm format of year to RTC one
	RTC_ALMON = pDateTime->Month;
	RTC_ALDOM  = pDateTime->Day;
	
	RTC_ALHOUR = pDateTime->Hour;
	RTC_ALMIN  = pDateTime->Minute;
	RTC_ALSEC  = pDateTime->Second;
	
}//RTCSetupDateTimeAlarmValue


/*
*********************************************************************************************************
* Name:                                    RTCGetDateTimeAlarmValue 
* 
* Description: Get date and time alarm value from the Real Time Clock using tmElements_t structure
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
void RTCGetDateTimeAlarmValue(tmElements_t *pDateTime)
{
	pDateTime->Second=(BYTE)RTC_ALSEC;
	pDateTime->Minute=(BYTE)RTC_ALMIN;
	pDateTime->Hour=(BYTE)RTC_ALHOUR;
	pDateTime->Day=(BYTE)RTC_ALDOM;
	pDateTime->Wday=0;//not used by alarms
	pDateTime->Month=(BYTE)RTC_ALMON;
	pDateTime->Year=(BYTE)CalendarYrToTm((WORD)RTC_ALYEAR);//convert normal year to number of years since 1970 as required by tmElements_t struct
		
	return;
}//RTCGetDateTimeAlarmValue



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
void RTCSetAlarmState(BYTE InDateAlarmState, BYTE InTimeAlarmState)
{
	if((!InDateAlarmState) & (!InTimeAlarmState) )//turn off Date Alarm and turn off Time Alarm
	{
		RTC_AMR = BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7;//mask all alarms - all are disabled
		return;
	}
	
	if((!InDateAlarmState) & (InTimeAlarmState))//turn off Date alarm but turn on Time Alarm
	{
		RTC_AMR = BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7;//mask all alarms - all are disabled
		RTC_AMR &=~(BIT0|BIT1|BIT2);//clear BIT0, 1, 2 i.e. turn on alarm for seconds, minutes and hours alarm
		return;
	}

	if((InDateAlarmState) & (!InTimeAlarmState))//turn on Date alarm but turn off Time Alarm
	{
		RTC_AMR = BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7;//mask all alarms - all are disabled
		RTC_AMR &=~(BIT7|BIT6|BIT3);//clear BIT7, 6, 3 i.e. turn on alarm for years,month and day of month
		return;
	}
	
	if((InDateAlarmState) & (InTimeAlarmState))//turn on Date alarm and turn on Time alarm
	{
		RTC_AMR = BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7;//mask all alarms - all are disabled
		RTC_AMR &=~(BIT7|BIT6|BIT3|BIT0|BIT1|BIT2);//clear BIT7, 6, 3,0,1,2 i.e. turn on alarm for date and time
		return;
	}
}//RTCSetAlarmState


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
void GetRTCAlarmState(BYTE *pInDateAlarmState, BYTE *pInTimeAlarmState)
{
	if ((RTC_AMR&BIT0)&&(RTC_AMR&BIT1)&&(RTC_AMR&BIT2))//if time alarm disabled
	{
		*pInTimeAlarmState=0;//time alarm is disabled
	}else //time alarm is enabled
	{
		//assure all time alarm bits are clear before we report time alarm is on - just for the case of inconsistency
		RTC_AMR &=~(BIT0|BIT1|BIT2);//clear BIT0, 1, 2 i.e. turn on alarm for seconds, minutes and hours alarm
		*pInTimeAlarmState=1;//time alarm is enabled
	}
	
	if ((RTC_AMR&BIT7)&&(RTC_AMR&BIT6)&&(RTC_AMR&BIT5)&&(RTC_AMR&BIT4)&&(RTC_AMR&BIT3))//if date alarm is disabled
	{
		*pInTimeAlarmState=0;//time alarm is disabled
	}else //time alarm is enabled
	{
		//assure all required date alarm bits are clear before we report date alarm is on - just for the case of inconsistency
		RTC_AMR |= BIT5|BIT4;//assure day of a year and day of a week are not compared for turn on date alarm
		RTC_AMR &=~(BIT7|BIT6|BIT3);//clear BIT7, 6, 3 i.e. turn on alarm for year, month and day of month
		*pInTimeAlarmState=1;//time alarm is enabled
	}
	return;
}//GetRTCAlarmState

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
BYTE IsAlarmSetupInRTC(void)
{
	BYTE DateAlarmState=FALSE;//storage for Date Alarm State
	BYTE TimeAlarmState=FALSE;//storage for Time Alarm State
	
	GetRTCAlarmState(&DateAlarmState,&TimeAlarmState);//get date and time aralm state
	if(DateAlarmState || TimeAlarmState)//when alny alarm is set return TRUE
		return TRUE;
	else //there is not any alarm set up in RTC
		return FALSE;
}//IsAlarmSetupInRTC

