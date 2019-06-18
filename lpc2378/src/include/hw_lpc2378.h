/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_lpc2378.h
* Description: Initialize very basic and primary LPC 2378 functions like MEM, PLL etc.
* Author:      Bogdan Kowalczyk
* Date:        2-Aug-2008
* History:
* 2-Aug-2008 - Initial version created based on NXP target.h for NXP LPC23xx/24xx Family Microprocessors
*********************************************************************************************************
*/

#ifndef __LPC2378_H 
#define __LPC2378_H 

#ifdef __cplusplus
   extern "C" {
#endif
   
/* If USB device is used, the CCLK setting needs to be 57.6Mhz, CCO will be 288Mhz
to get precise USB clock 48Mhz. If USB is not used, you set any clock you want
based on the table below. If you want to use USB, change "define USE_USB" from 0 to 1 */
 
#define	USE_USB		0
	   
//This segment should not be modified
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/* PLL Setting Table Matrix */
/* 	
	Main Osc.	CCLKCFG		Fcco		Fcclk 		M 	N
	12Mhz		29		300Mhz		10Mhz			24	1
	12Mhz		35		360Mhz		10Mhz			14	0					
	12Mhz		27		336Mhz		12Mhz			13	0
	12Mhz		14		300Mhz		20Mhz			24	1		
	12Mhz		17		360Mhz		20Mhz			14	0
	12Mhz		13		336Mhz		24Mhz			13	0
	12Mhz		11		300Mhz		25Mhz			24	1   
	12Mhz		9		300Mhz		30Mhz			24	1
	12Mhz		11		360Mhz		30Mhz			14	0
	12Mhz		9		320Mhz		32Mhz			39	2
	12Mhz		9		350Mhz		35Mhz			174	11
	12Mhz		7		312Mhz		39Mhz			12	0
 	12Mhz		8		360Mhz		40Mhz			14	0 
	12Mhz		7		360Mhz		45Mhz			14	0
	12Mhz		6		336Mhz		48Mhz			13	0  
	12Mhz		5		300Mhz		50Mhz			24	1
	12Mhz		5		312Mhz		52Mhz			12	0
	12Mhz		5		336Mhz		56Mhz			13	0		
 	12Mhz		4		300Mhz		60Mhz			24	1		
  	12Mhz		4		320Mhz		64Mhz			39	2
	12Mhz		4		350Mhz		70Mhz			174	11
	12Mhz		4		360Mhz		72Mhz			14	0		
	12Mhz		3		300Mhz		75Mhz			24	1
	12Mhz		3		312Mhz		78Mhz			2	0  
	12Mhz		3		320Mhz		80Mhz			39	2
	12Mhz		3		336Mhz		84Mhz			13	0 
*/

/* LPC2378 PLL configuration shortly explained by B.K.:
 * Following are the parameters and relation between them imposed by LPC2378 PLL construction:
 * Fosc - frequency of oscilations on PLL input (can be internal RC oscilator, external oscilator, or RTC
 * Fcco - frequency of current control oscilator - PLL output need to be from 275MHz - 550MHz
 * Fcclk - frequency of CPU clock
 * Fpclk - frequency of clock for peripherals
 * Fusbclk - frequency of USB clock when USB is used must be 48MHz
 * PLL_MValue - PLL Multiplier value which is written to PLLCFG
 * PLL_NValue - PLL pre-divider value which is written to PLLCFG
 * Fcco=(2x(PLL_MValue+1)xFosc)/(PLL_NValue+1) - not all PLL_Mvalue are allowed see LPC2378 specs for details
 * Fcclk = Fcco/(CCLKDivValue+1)
 * Fusbclk = Fcco/(USBCLKDivValue+1)
 * Fpclk = (1 or 2 or 4)x(Fcclk/4)
 */
	   
#if USE_USB		/* 1 is USB, 0 is non-USB related */ 

/* System configuration: Fosc, Fcclk, Fcco, Fpclk must be defined */	   
/* Fosc = 12 MHz, Fcclk = 57.6MHz, Fcco = 288MHz, and Fusbclk = 48Mhz */
	   
/* PLL input Crystal frequence range 4KHz~20MHz. */
#define Fosc	12000000
/* System frequence,should be less than 80MHz. */
#define Fcclk	57600000
#define Fcco	288000000	   

/* PLL Registers values to assure above */
#define PLL_MValue			11
#define PLL_NValue			0
#define CCLKDivValue		4
#define USBCLKDivValue		5

#else //if not USE_USB defined

/* System configuration: Fosc, Fcclk, Fcco, Fpclk must be defined */
/* Fosc = 12 MHz, Fcclk = 50MHz, Fcco = 300MHz, and Fusbclk = 42.8MHz but not used */

/* PLL input Crystal frequence range 4KHz~20MHz. */
#define Fosc	12000000
/* System frequence,should be less than 80MHz. */
#define Fcclk	50000000
#define Fcco	300000000
	   
/* PLL Registers values to assure above */
#define PLL_MValue			24
#define PLL_NValue			1
#define CCLKDivValue		5
#define USBCLKDivValue		6

#endif //USE_USB

/* APB clock frequence , must be 1/2/4 multiples of ( Fcclk/4 ). */
/* If USB is enabled, the minimum APB must be greater than 16Mhz */ 
#if USE_USB
#define Fpclk	(Fcclk / 2)
#else
#define Fpclk	(Fcclk / 4)
#endif

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

extern void Lpc2378ResetInit(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used
 
#endif /* end __LPC2378_H  */

