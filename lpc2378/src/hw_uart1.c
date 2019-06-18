/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_uart1.c
* Description: Functions to control UART1 for OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        28-Dec-2017
* History:
* 28-Dec-2017 - Initial version polling type UART control for very first trials
* 
*********************************************************************************************************
*/

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"

#include "type.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "hw_uart1.h"
#include "lib_error.h"
#include "lib_std.h"

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
* *********************************************************************************************************
*/
void InitUart1(void)
{
	//PCONP|=BIT4;//enable UART1 - by default on uP rest it is on so this line is commented out
	//Fpclk is 12,5MHz
	
	//Setup uP pins for UART1 so TxD is on P0.15 and RxD is on P0.16 out of P0 port to get below setup 
	PINSEL0 |= BIT30;//BIT31=0, BIT30=1 -> P0.15=TxD1
	PINSEL1 |= BIT0; //BIT1=0,  BIT0=1  -> P0.16=RxD1
	U1LCR=BIT7|BIT1|BIT0; //enable Divisior latches, setup 8 bit, no parity, 1 stop bit transmission
	//setup divider for 9600 bits/second transmission
	//divisior = Fpclk/(16xBaudRate)
	//divisior = 12500000/(16x9600)=81,3 = 0x51 Hex - for 9600 baud rate
	//divisior = 12500000/(16x115200)= 7 = 0x07 Hex - for 115 200 vaud rate
	U1DLL=0x51;
	U1DLM=0;
	U1LCR&=(~BIT7); //disable access to divisir latches	
}//InitUart1

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
int Uart1PutChar(int ch)
{
	if (ch=='\n') //when new line put carraige return first
	{
		while(!(U1LSR & 0x20)); // wait until THR empty
		U1THR=0x0D; // put carriage return
	}
	while(!(U1LSR & 0x20));// wait until THR empty
	U1THR=ch;
	OSTimeDly(UART1_TR_DELAY);
	return (ch);
}//Uart1PutChar

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
int Uart1GetChar(void)
{
	while(!(U1LSR & 0x01)); //keep in loop untill any character received
	return U1RBR;
}//Uart1GetChar


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
int Uart1CheckForCharacterReceived(void)
{
	if(!(U1LSR & 0x01)) //wehen there is not any charcter received return 0
		return 0;
	else
		return U1RBR;//return received character if there is any
}//Uart1CheckForCharacterReceived

//send string via serial port
//pString - pointer to string to be send
void Uart1PutStr(char *pString) 
{
	// loop until null-terminator is seen
	while (*pString != 0x00) 
	{
		Uart1PutChar(*pString++); //put character by character
	}
}//Uart1PutStr

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
int Uart1GetCharWithTimeout(DWORD TimeOut)
{
	int ch=0;//character received
	DWORD  NoOfTimeoutTicks=(TimeOut*OS_TICKS_PER_SEC)/1000;//number of OS Ticks counted for timeout
	
	if(NoOfTimeoutTicks==0)// when infinite wait
	{
		while(TRUE)//infinite loop untill anything received
		{
		if((ch=Uart1CheckForCharacterReceived())) break;//when any character received break
		OSTimeDly(UART1_DELAY_TICKS);//wait 1 ticke to not block OS
		}
	}
	else
	{
		while(NoOfTimeoutTicks>0)
		{
		OSTimeDly(UART1_DELAY_TICKS);
		NoOfTimeoutTicks-=1;
		if((ch=Uart1CheckForCharacterReceived())) break;//when any character received break
		}
	}
	return ch;
}//Uart1GetCharWithTimeout
