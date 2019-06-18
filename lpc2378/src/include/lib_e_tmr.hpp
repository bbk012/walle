/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_e_tmr.hpp
* Description: Class EasyTimer to count delays based on uCOS-II ticks (OSTime variable)
* Author:      Bogdan Kowalczyk
* Date:        21-Jan-2018
* Note:
* History:
*              21-Jan-2018 - Initial version created
*********************************************************************************************************
*/
#ifndef LIB_E_TMR_HPP_
#define LIB_E_TMR_HPP_
#include "type.h"

//calss to instantinate timer to measure Delay Since start max for 24h which
//is 86 400 000 [ms], or 86400[s] or 1440 [min]
#define EASY_TIMER_MS_LIMIT			86400000L //24*60*60*1000
#define EASY_TIMER_S_LIMIT			86400l//24*60*60	
#define EASY_TIMER_MINUTES_LIMIT	1440//24*60

#define MAX_OS_TICKS	0xFFFFFFFF //max number of OSTicks which can be counted on DWORD OSTick

class cEasyTimer
{
private:
	DWORD mOSTicksOnTimerStart;
	DWORD mDelayInOSTicks;
public:
	cEasyTimer (DWORD Delay_ms=0){StartDelayInMiliseconds(Delay_ms);};//construct class and strat for delay counting
	void StartDelayInMiliseconds(DWORD Delay_ms);
	void StartDelayInSeconds(DWORD Delay_s){StartDelayInMiliseconds(Delay_s*1000L);};
	void StartDelayInMinutes(DWORD Delay_m){StartDelayInMiliseconds(Delay_m*60*1000L);};
	void Reset(void);//start delay counting again (from current moment based on existing setup)
	void Expire(void);//force timer to expire 
	BYTE isExpired(void);//returns true if timer expired
};//cEasyTimer


#endif /*LIB_E_TMR_HPP_*/
