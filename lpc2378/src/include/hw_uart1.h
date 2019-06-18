/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_uart1.h
* Description: Functions to control UART1 for OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        28-Dec-2017
* History:
* 28-Dec-2017 - Initial version polling type UART control for very first trials
* 
*********************************************************************************************************
*/

#ifndef HW_UART1_H_
#define HW_UART1_H_
#ifdef __cplusplus
   extern "C" {
#endif

#include "type.h"	

//number of os ticks per one while loop in Uart1GetCharWithTimeout function	   
#define UART1_DELAY_TICKS 	1
	   
//number of ticks to wait after one char to be sent via uart 
//(uart1 is used to communicate with EasyVR it looks after each charcter transmited
//a delay is needed to get charcter transmited correctly received by EasyVR
//this value was determined by experiment	   
#define UART1_TR_DELAY		5

//constant for infinitw wait	   
#define INFINITE_UART1_TIMEOUT	0
	   
/*
*********************************************************************************************************
* Name:                                    InitUart1  
* 
* Description: Setup UART1 for serial transmission according to the
*              transmission parameters: 9600 bits/second, 8 bit data, no parity, 1 stop
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
*********************************************************************************************************
*/
extern void InitUart1(void);	   
	   
	   
/*
*********************************************************************************************************
* Name:                                    Uart1PutChar 
* 
* Description: Sends character out of the UART
*
* Arguments:   ch - character to be written to the serial port
*
* Returns:     character written to the serial port
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern int Uart1PutChar(int ch);
	   
/*
*********************************************************************************************************
* Name:                                    Uart1GetChar 
* 
* Description: Gets character from the uart.
* IMPORTANT! That is blocking function its keep waitin on Uart until character is received.
* 		Tasks are not switched because there is not any OSTimeDly used in the while.
* 		For non blocking version use Uart1CheckForChar function which returns when there is not character received.
*
* Arguments:   none
*
* Returns:     character read from the serial port
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern int Uart1GetChar(void);

/*
*********************************************************************************************************
* Name:                                    Uart1CheckForCharacterReceived 
* 
* Description: Check if there is character received and if there is any returns it
* 		That is non-blocking version of Uart0GetChar
*
* Arguments:   none
*
* Returns:     0 - none character received or character read from the serial port if there is any
* 
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern int Uart1CheckForCharacterReceived(void);

//send string via serial port
//pString - pointer to string to be send
extern void Uart1PutStr(char *pString);


/*
*********************************************************************************************************
* Name:                                    Uart1GetCharWithTimeout 
* 
* Description: Gets character from the uart.
* 
* Arguments:   
* 			   TimeOut - time out in miliseconds, 0 - wait infinite time until character received
*
* Returns:     character read from the serial port or 0 when timeout
*
* Note(s):     nfinite wait does not block OS because of OSTimeDly() used
* 
* *********************************************************************************************************
*/
extern int Uart1GetCharWithTimeout(DWORD TimeOut);

	   
#ifdef __cplusplus
}
#endif //to close extern "C" if used
#endif /*HW_UART1_H_*/
