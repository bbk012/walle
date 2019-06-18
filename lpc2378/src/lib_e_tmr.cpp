/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_e_tmr.cpp
* Description: Class EasyTimer to count delays based on uCOS-II ticks (OSTime variable)
* Author:      Bogdan Kowalczyk
* Date:        21-Jan-2018
* Note:
* History:
*              21-Jan-2018 - Initial version created
*********************************************************************************************************
*/
#include "lib_e_tmr.hpp"
#include "wrp_kernel.hpp"

#include "hw_uart.h"
#include "lib_dbg.h"
#include "ctr_lcd.h"

//Easy timer module debug turn on (1) turn off(0) define
#define DEBUG_EAZY_TMR 1

void cEasyTimer::StartDelayInMiliseconds(DWORD Delay_ms)
{
	if(Delay_ms > EASY_TIMER_MS_LIMIT)//allowed time limit excited
	{
#if DEBUG_EAZY_TMR
		DbgTraceStrVal(2,"E_Tmr_1","\nTRC: E_Tmr: StartDelayInMiliseconds: ERR: not allowed delay!: ",Delay_ms);
#endif//DEBUG_EAZY_TMR
		LCDDebugMessage("ERR: E_Tmr: ",Delay_ms,20,1,1,0);
		Uart0Message("ERR: E_Tmr: StartDelayInMiliseconds: not allowed delay: ",Delay_ms);		
		UNEXPECTED_ERROR_VALUE;//exception this case should never happen
	}
	mOSTicksOnTimerStart=Kernel.Ticks();//get current tick count
	mDelayInOSTicks=Delay_ms/10;//change Delay in ms to number of ticks
	
}//cEasyTimer::StartDelayInMiliseconds

//force timer to expire
void cEasyTimer::Expire(void)
{
	mOSTicksOnTimerStart=0;
	mDelayInOSTicks=0;
}//cEasyTimer::Expire

//returns true if timer expired
BYTE cEasyTimer::isExpired(void)
{
	DWORD OSTicksCurrent;//stor current number of OSTicks
	
	OSTicksCurrent=Kernel.Ticks();//get current number of ticks
	
	if(OSTicksCurrent >= mOSTicksOnTimerStart)//when there is no overlap
	{
		if(OSTicksCurrent-mOSTicksOnTimerStart > mDelayInOSTicks)
			return TRUE;//time which expired to the check moment larger than delay
		else
			return FALSE;//still time which expired lower than Delay
	}
	else //there is an OSTick overlap so need to include this in current delay calculation
	{
		if(((MAX_OS_TICKS-mOSTicksOnTimerStart)+OSTicksCurrent+1) > mDelayInOSTicks)
			return TRUE;//time which expired to the check moment larger than delay
		else
			return FALSE;
	}
}//cEasyTimer::isExpired

//start counting previously setup delay since that moment again
void cEasyTimer::Reset(void)
{
	mOSTicksOnTimerStart=Kernel.Ticks();//get current number of ticks and setup to start again
}//cEasyTimer::Reset

