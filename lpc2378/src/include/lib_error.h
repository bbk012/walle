/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_error.h
* Description: Functions and defines related to errors and exceptions generation and handling
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
#ifndef ERROR_H_
#define ERROR_H_

#include "hw_lpc23xx.h"

#ifdef __cplusplus
   extern "C" {
#endif
	   
//constants for EXCEPTION_REASONs as well as descriptive names behind
//detail description is below where critical faults macros are defined
#define NONE_EXCEPTION							0x00 
#define NONE_EXCEPTION_STR						"NONE"	
	   
#define OTHER_EXCEPTION							0x01//this is for any other reason different than SWI (for example UNDEF instruction etc.)
#define OTHER_EXCEPTION_STR						"OTHER"
	   
#define MEM_ALLOC_REASON						0x02
#define MEM_ALLOC_REASON_STR					"ALLOC"
	   
#define UCOSII_RES_REASON						0x04
#define	UCOSII_RES_REASON_STR					"UCOSII"
	   
#define THREAD_NO_RUN_DEFINED_REASON			0x06
#define THREAD_NO_RUN_DEFINED_REASON_STR		"THR_NO"
	   
#define THREAD_CREATE_REASON					0x10
#define THREAD_CREATE_REASON_STR				"THR_CRT"
	   
#define PUBLISHER_REGISTER_REASON				0x12
#define PUBLISHER_REGISTER_REASON_STR			"PUBLISH"
	   
#define SUBSCRIBER_REGISTER_REASON				0x14
#define SUBSCRIBER_REGISTER_REASON_STR			"SUBSCRIB"
	   
#define GFX_MEM_ALLOC_REASON					0x16
#define GFX_MEM_ALLOC_REASON_STR				"GFXALLOC"	   
	   
#define DEBUG_REASON							0x18
#define DEBUG_REASON_STR						"DEBUG"	   
	   
#define NOT_ALLOWED_STATE_REASON				0x20
#define NOT_ALLOWED_STATE_REASON_STR			"STATE"	   
	   
#define UNEXPECTED_ERROR_VALUE_REASON			0x22
#define UNEXPECTED_ERROR_VALUE_REASON_STR		"UNVAL"
	   
#define WIN_CTRL_NO_VIRTUAL_DEFINED_REASON		0x24
#define WIN_CTRL_NO_VIRTUAL_DEFINED_REASON_STR	"WIN"
	   
#define NOT_ALLOWED_PROGRAM_REASON				0x26
#define NOT_ALLOWED_PROGRAM_REASON_STR			"NPROG"
	   
#define NOT_ALLOWED_VALUE_REASON				0x28
#define NOT_ALLOWED_VALUE_REASON_STR			"NVALUE"
#define UNDEF_REASON_STR						"ERRUNDEF"	   
	   
// Critical Faults
 			 
#define MEM_ALLOC_EXCEPTION	   	{EXCEPTION_REASON = MEM_ALLOC_REASON; asm("swi #02");} //dynamic memory is not availiable	   
#define UCOSII_RES_EXCEPTION	{EXCEPTION_REASON = UCOSII_RES_REASON; asm("swi #04");} //uCOS-II resources availability exception	   
#define THREAD_NO_RUN_DEFINED	{EXCEPTION_REASON = THREAD_NO_RUN_DEFINED_REASON; asm("swi #06");} //Run function for a task not defined
   			   
//IMPORTANT! 19-Sep-2009 I have just noticed that compiler do not accept asm("swi #08")to be used
	   
#define THREAD_CREATE_EXCEPTION 		{EXCEPTION_REASON = THREAD_CREATE_REASON ; asm("swi #10");} //Cannot create a thread 
#define PUBLISHER_REGISTER_EXCEPTION 	{EXCEPTION_REASON = PUBLISHER_REGISTER_REASON ; asm("swi #12");} //Cannot register a publisher 	   
#define SUBSCRIBER_REGISTER_EXCEPTION 	{EXCEPTION_REASON = SUBSCRIBER_REGISTER_REASON ; asm("swi #14");} //Cannot register a subscrier 

#define GFX_MEM_ALLOC_EXCEPTION         {EXCEPTION_REASON = GFX_MEM_ALLOC_REASON ; asm("swi #16");} //graphics dynamic memory is not availiable
#define DEBUG_EXCEPTION                 {EXCEPTION_REASON = DEBUG_REASON ; asm("swi #18");} //generated once debug message is displayed to stop work
#define NOT_ALLOWED_STATE				{EXCEPTION_REASON = NOT_ALLOWED_STATE_REASON ; asm("swi #20");} //not allwed state of the state machine
#define UNEXPECTED_ERROR_VALUE			{EXCEPTION_REASON = UNEXPECTED_ERROR_VALUE_REASON; asm("swi #22");} //value of error which is not expected and thus not handled

#define WIN_CTRL_NO_VIRTUAL_DEFINED		{EXCEPTION_REASON = WIN_CTRL_NO_VIRTUAL_DEFINED_REASON; asm("swi #24");}	   
#define NOT_ALLOWED_PROGRAM				{EXCEPTION_REASON = NOT_ALLOWED_PROGRAM_REASON; asm("swi #26");} //requested execution of undefined Wall-e program
#define NOT_ALLOWED_VALUE	   			{EXCEPTION_REASON = NOT_ALLOWED_VALUE_REASON ; asm("swi #28");}//unexpected value provided to be processed
	   
/*	B.K - commented out when static RAM preserved EXCEPTION_REASON added   
	   // Critical Faults
	   #define MEM_ALLOC_EXCEPTION 			asm("swi #02") //dynamic memory is not availiable
	   #define UCOSII_RES_EXCEPTION			asm("swi #04") //uCOS-II resources availability exception
	   #define THREAD_NO_RUN_DEFINED   		asm("swi #06") //Run function for a task not defined
	   //IMPORTANT! 19-Sep-2009 I have just noticed that compiler do not accept asm("swi #08")to be used
	   #define THREAD_CREATE_EXCEPTION 		asm("swi #10") //Cannot create a thread 
	   #define PUBLISHER_REGISTER_EXCEPTION 	asm("swi #12") //Cannot register a publisher 	   
	   #define SUBSCRIBER_REGISTER_EXCEPTION 	asm("swi #14") //Cannot register a subscrier 

	   #define GFX_MEM_ALLOC_EXCEPTION         asm("swi #16") //graphics dynamic memory is not availiable
	   #define DEBUG_EXCEPTION                 asm("swi #18") //generated once debug message is displayed to stop work
	   #define NOT_ALLOWED_STATE				asm("swi #20") //not allwed state of the state machine
	   #define UNEXPECTED_ERROR_VALUE			asm("swi #22") //value of error which is not expected and thus not handled
	   //#define INTERACTOR_EXCEPTION            asm("swi #20") //Interactor usage incorrect
	   //#define WIN_CTRL_EXCEPTION              asm("swi #22") //Window control incorrect ussage
	   //#define WIN_EXCEPTION                   asm("swi #22") //Window exception		   
	   //#define WIN_CTRL_EXCEPTION   			asm("swi #24") //Windows controller reported exception	   
*/	   
	   
#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*ERROR_H_*/
