/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2016, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_uart.c
* Description: Functions to control UART0 for OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        16-Apr-2016
* History:
* 16-Apr-2016 - Initial version polling type UART control for very first trials
* 
*********************************************************************************************************
*/

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"

#include "type.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "hw_uart.h"
#include "lib_error.h"
#include "lib_std.h"

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
void InitUart0(void)
{
	//PCONP|=BIT3;//enable UART0 - by default on uP rest it is on so this line is commented out
	//Fpclk is 12,5MHz
	
	//Setup uP pins for UART0 so TxD is on P0.2 and RxD is on P03.3 out of P0 port to get below setup 
	PINSEL0 |= BIT4|BIT6;
	U0LCR=BIT7|BIT1|BIT0; //enable Divisior latches, setup 8 bit, no parity, 1 stop bit transmission
	//setup divider for 9600 bits/second transmission
	//divisior = Fpclk/(16xBaudRate)
	//divisior = 12500000/(16x9600)=81,3 = 0x51 Hex - for 9600 baud rate
	//divisior = 12500000/(16x115200)= 7 = 0x07 Hex - for 115 200 vaud rate
	U0DLL=0x07;
	U0DLM=0;
	U0LCR&=(~BIT7); //disable access to divisir latches	
}//InitUart

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
int Uart0PutChar(int ch)
{
	if (ch=='\n') //when new line put carraige return firs
	{
		while(!(U0LSR & 0x20)); // wait until THR empty
		U0THR=0x0D; // put carriage return
	}
	while(!(U0LSR & 0x20));// wait until THR empty
	return (U0THR=ch);
}//Uart0PutChar

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
int Uart0GetChar(void)
{
	while(!(U0LSR & 0x01)); //keep in loop untill any character received
	return U0RBR;
}//Uart0GetChar


/*
*********************************************************************************************************
* Name:                                    Uart0CheckForCharacterReceived 
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
int Uart0CheckForCharacterReceived(void)
{
	if(!(U0LSR & 0x01)) //ehen there is not any charcter received return 0
		return 0;
	else
		return U0RBR;//return received character if there is any
}//Uart0CheckForCharacterReceived

//send string via serial port
//pString - pointer to string to be send
void Uart0PutStr(char *pString) 
{
	// loop until null-terminator is seen
	while (*pString != 0x00) 
	{
		Uart0PutChar(*pString++); //put character by character
	}
}//Uart0PutStr

static char TempResultBuff[32];//temporary buffer place for ltoa convertion 

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
void Uart0DebugMessage(char *pMsgStr,long Value, BYTE GenExcept)
{
	Uart0PutStr(pMsgStr);//transimt string to the terminal 
	ltoa(Value,TempResultBuff,10);//change to string
	Uart0PutStr(TempResultBuff);//transmit walue
	Uart0PutChar('\n');//move cursor on terminal to the new line
	if(GenExcept)
		DEBUG_EXCEPTION;//SWI to stop operation efter debug message is displayed
}//Uart0DebugMessage


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
void Uart0Message(char *pMsgStr,long Value)
{
	Uart0PutStr(pMsgStr);//transimt string to the terminal 
	ltoa(Value,TempResultBuff,10);//change to string
	Uart0PutStr(TempResultBuff);//transmit walue
	Uart0PutChar('\n');//move cursor on terminal to the new line
}//Uart0Message

/*
*********************************************************************************************************
* Name:                                    Uart0GetTrmlChar 
* 
* Description: Waits and gets character from the Uart0 but in sach a way that tasks switching is not blcoked
* 		because during waiting OSTimeDly is used
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
int Uart0GetTrmlChar(void)
{
	int CharReceived;
	
	while(!(CharReceived=Uart0CheckForCharacterReceived()))//if there is not character received
		OSTimeDly(WAIT_FOR_UART_CHAR);
	return CharReceived;
	
}//Uart0GetTrmlChar

/*
*********************************************************************************************************
* Name:                                    Uart0GetTrmlStr 
* 
* Description: Waits and gets characters from the Uart0 until <Enter>. 
*              Task switching is not blocked because Uart0GetTrmlChar() is used to get single character
* 			   (see above)
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
char *Uart0GetTrmlStr(char* pMsgBuffer, int inLength)
{
	char c;   //character received
	char *s;  //pinter to current position in the buffer;
	
	s=pMsgBuffer;//s points to buffer beginning
	
	if (inLength < 2) //sanity check of enough place to capture at least <Enter>
		return NULL;
	
	while (--inLength > 0 && (c = (char)Uart0GetTrmlChar()) != '\r')
		{//read character by character until <Enter> recived
		*s++ = c;
		}
	*s = 0;//terminate recived string with \0
	if (s==pMsgBuffer)//if nothing received just '\n'
		return NULL;
	else
		return pMsgBuffer; 
	
}//Uart0GetTrmlStr

