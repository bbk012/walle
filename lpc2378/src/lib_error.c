/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_error.c
* Description: Functions related to errors and exceptions generation and handling
* Author:      Bogdan Kowalczyk
* Date:        2-Nov-2008
* Notes:
* 	There are two types of faults in the system expected:
* 	- CRITICAL_EXCEPTION - rised through swi and handle through interrupt handler for critical cases which
* 						cannot be managed anyhow (for example mem cannot be allocated for run time)
* 	- ERROR - problems reported by functions but somehow managable for example cannot put message into queue
* 			(can wait for queue to be availiable)
* History:
* 	2-Nov-2008 - Initial version created
*********************************************************************************************************
*/

#include "type.h"
