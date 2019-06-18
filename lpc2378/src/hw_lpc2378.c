/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_lpc2378.c
* Description: Initialize very basic and primary LPC 2378 functions like MEM, PLL etc.
* Author:      Bogdan Kowalczyk
* Date:        2-Aug-2008
* History:
* 2-Aug-2008 - Initial version created based on NXP target.h for NXP LPC23xx/24xx Family Microprocessors
*********************************************************************************************************
*/

#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "type.h"
#include "hw_lpc2378.h" //Header file for basic LPC 2378 uP initialization

/*
*********************************************************************************************************
* Name:                                    InitPLL  
* 
* Description: Initialize LPC2378 PLL to generate all required clocks for uP and its peripherals
*              not from power up IRC but from external oscilator.
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
static void InitPLL ( void )
{
	DWORD MValue, NValue;

	if ( PLLSTAT & (1 << 25) )
    {
		PLLCON = 1;			/* Enable PLL, disconnected */
		PLLFEED = 0xaa;
		PLLFEED = 0x55;
    }

    PLLCON = 0;				/* Disable PLL, disconnected */
    PLLFEED = 0xaa;
    PLLFEED = 0x55;
    
	SCS |= 0x20;			/* Enable main OSC */
    while( !(SCS & 0x40) );	/* Wait until main OSC is usable */

    CLKSRCSEL = 0x1;		/* select main OSC, 12MHz, as the PLL clock source */

    PLLCFG = PLL_MValue | (PLL_NValue << 16);
    PLLFEED = 0xaa;
    PLLFEED = 0x55;
      
    PLLCON = 1;				/* Enable PLL, disconnected */
    PLLFEED = 0xaa;
    PLLFEED = 0x55;

    CCLKCFG = CCLKDivValue;	/* Set clock divider */
#if USE_USB
    USBCLKCFG = USBCLKDivValue;		/* usbclk = 288 MHz/6 = 48 MHz */
#endif

    while ( ((PLLSTAT & (1 << 26)) == 0) );	/* Check lock bit status */
    
    MValue = PLLSTAT & 0x00007FFF;
    NValue = (PLLSTAT & 0x00FF0000) >> 16;
    while ((MValue != PLL_MValue) && ( NValue != PLL_NValue) );

    PLLCON = 3;				/* enable and connect */
    PLLFEED = 0xaa;
    PLLFEED = 0x55;
	while ( ((PLLSTAT & (1 << 25)) == 0) );	/* Check connect bit status */
	return;
}// InitPLL



/*
*********************************************************************************************************
* Name:                                    Lpc2378ResetInit  
* 
* Description: Initialize very basic primary LPC 2378 functions like: memory setup, PLL setup, uP Flash
*              acceleration module
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
void Lpc2378ResetInit(void)
{
#ifdef __DEBUG_RAM    
    MEMMAP = 0x2;			/* remap to internal RAM */
#else
    MEMMAP = 0x1;			/* remap to internal flash */
#endif 


#if USE_USB
	PCONP |= 0x80000000;		/* Turn On USB PCLK */
#endif
	/* Configure PLL, switch from IRC to Main OSC */
	InitPLL();

  /* Set system timers for each component */
#if (Fpclk / (Fcclk / 4)) == 1
    PCLKSEL0 = 0x00000000;	/* PCLK is 1/4 CCLK */
    PCLKSEL1 = 0x00000000;
#endif
#if (Fpclk / (Fcclk / 4)) == 2
    PCLKSEL0 = 0xAAAAAAAA;	/* PCLK is 1/2 CCLK */
    PCLKSEL1 = 0xAAAAAAAA;	 
#endif
#if (Fpclk / (Fcclk / 4)) == 4
    PCLKSEL0 = 0x55555555;	/* PCLK is the same as CCLK */
    PCLKSEL1 = 0x55555555;	
#endif

    /* Set memory accelerater module*/
    MAMCR = 0;
#if Fcclk < 20000000
    MAMTIM = 1;
#else
#if Fcclk < 40000000
    MAMTIM = 2;
#else
    MAMTIM = 3;
#endif
#endif
//    MAMCR = 2; because of errata sheet
    MAMCR = 1;
    
    return;
}//Lpc2378ResetInit

/******************************************************************************
**                            End Of File
******************************************************************************/
