/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        os_exch.h
* Description: uCOS-II exception handler ported to LPC2378 uP and its Vectored Interrupt Controller
* Author:      Bogdan Kowalczyk
* Date:        11-Aug-2008
* History:
*              11-Aug-2008 - Initial version created
*********************************************************************************************************
*/
#ifndef OS_EXCH_H_
#define OS_EXCH_H_
/*
*********************************************************************************************************
* Name:                                   OS_CPU_ExceptHndlr 
* 
* Description: Common exception handler in C - called from uCOS-II ARM port to handle interrupt
*
* Arguments:   INT32U except_type - exception type (passed by uP R0 register - see os_cpu_a.S file
*
* Returns:     none
*
* Note(s):     
* 			Depending on interrupt LPC2378 Vector Interrupt Controler provided address of handler is used
*           to call interrupt service routine and next VIC interrupt is acknowledged.
*           Device interrupt is acknowledged on device specific handler level not here.
* *********************************************************************************************************
*/
/*
 * This header is only created for consistency because OS_CPU_ExceptHndlr is declared in os_cpu.h of uCOS-II 
*/
// void OS_CPU_ExceptHndlr (INT32U except_type);
#endif /*OS_EXCH_H_*/
