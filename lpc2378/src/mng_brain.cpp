/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_brain.cpp
* Description: Container for Wall-e execution manager (cExeMngr) and context manager (cCtxMngr)
* Author:      Bogdan Kowalczyk
* Date:        25-Jan-2015
* History:
* 25-Jan-2015 - Initial version created
*********************************************************************************************************
*/
#include "mng_brain.hpp"
#include "wrp_kernel.hpp"
#include "ctr_lcd.h"

void cBrainMngr::Run()
{
	for(;;)
	{
		Kernel.TimeDlyHMSM(0,0,2,0);
		
	}//for
}//cBrainMngr::Run()
