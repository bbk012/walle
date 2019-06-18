/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2016, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_uart.h
* Description: Functions to control UART0 for OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        16-Apr-2016
* History:
* 16-Apr-2016 - Initial version polling type UART control for very first trials
* 
*********************************************************************************************************
*/
#ifndef HW_UART_H_
#define HW_UART_H_
#ifdef __cplusplus
   extern "C" {
#endif

#include "type.h"	   
	   
#define WAIT_FOR_UART_CHAR 1 //number of OS Ticks Uart0DebugGetChar is waiting befor next check of if char is received

	   /*
*********************************************************************************************************
* Name:                                    InitUart0  
* 
* Description: Setup UART0 for serial transmission according to the
*              transmission parameters
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern void InitUart0(void);


/*
*********************************************************************************************************
* Name:                                    Uart0PutChar 
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
extern int Uart0PutChar(int ch);


//send string via serial port
//pString - pointer to string to be send
extern void Uart0PutStr(char *pString);


/*
*********************************************************************************************************
* Name:                                    Uart0GetChar 
* 
* Description: Gets character from the uart.
* IMPORTANT! That is blocking function its keep waitin on Uart until character is received.
* 		Tasks are not switched because there is not any OSTimeDly used in the while.
* 		For non blocking version use Uart0CheckForChar function which returns when there is not character received.
*
* Arguments:   none
*
* Returns:     character read from the serial port
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern int Uart0GetChar(void);


/*
*********************************************************************************************************
* Name:                                    Uart0CheckForCharacter 
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
extern int Uart0CheckForCharacterReceived(void);

/*
*********************************************************************************************************
* Name:                                  Uart0DebugMessage
* 
* Description: 
* 	Displays DEBUG message plus Value on the terminal output and next comes to the new line
*   when GenExcept requested (<>0) generates SWI to stop system operation
*  
*
* Arguments:   
*				pMsgStr - message to display
* 				Value - value to display
* 				GenExcept <>0 - generate exception
* Returns:     none
*
* 			
* Note(s):     
* 			After texy displayed CR/LF is automaticly generated moving cursor on terminal to a new line
* *********************************************************************************************************
*/
extern void Uart0DebugMessage(char *pMsgStr,long Value, BYTE GenExcept);

/*
*********************************************************************************************************
* Name:                                  Uart0Message
* 
* Description: 
* 	Displays DEBUG message plus Value on the terminal output and next comes to the new line
*  
*
* Arguments:   
*				pMsgStr - message to display
* 				Value - value to display
* Returns:     none
*
* 			
* Note(s):     
* 			After texy displayed CR/LF is automaticly generated moving cursor on terminal to a new line
* *********************************************************************************************************
*/
extern void Uart0Message(char *pMsgStr,long Value);

/*
*********************************************************************************************************
* Name:                                    Uart0GetTrmlChar 
* 
* Description: Waits and gets character from the Uart0 terminal connection but in sach a way 
*              that tasks switching is not blcoked because during waiting OSTimeDly is used
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
extern int Uart0GetTrmlChar(void);

/*
*********************************************************************************************************
* Name:                                    Uart0GetTmrlStr 
* 
* Description: Waits and gets characters from the Uart0 Terminal connection until <Enter>
*              Task switching is not blocked because Uart0GetTrmlChar() is used to get single character
*
*
*
* Arguments:   pMsgBuffer - pointer to the storage area buffer of the received line
* 			   inLength - length of the buffer used for the storage 
*
* Returns:     pointer to first character or NULL if none characters are received
* 
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern char *Uart0GetTrmlStr(char* pMsgBuffer, int inLength);

#ifdef __cplusplus
}
#endif //to close extern "C" if used
   
#endif /*HW_UART_H_*/
