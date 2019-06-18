/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_spi.c
* Description: Functions to control SPI0 conected to LCD for OLIMEX LPC 2378 eval board
* Author:      Bogdan Kowalczyk
* Date:        5-Aug-2008
* History:
* 5-Aug-2008 - Initial version - extracted from original James P Lynch file and modified for OLIMEX LPC 2378
* 13-Oct-2009 - Added MCP 3208 SPI A/D handling through SSP1 interface
* 12-Dec-2010 - Chnged ADC CLK to almost 2MHz (to have as quick readings as possible) 
*               and added MUTEX protection of ADC
* 6-Jan-2011 - Changed ADC readings to 500 000Hz to avoid random wrong readings which appeared for 2MHz
* 
*********************************************************************************************************
*/

#include "type.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors

//includes to get access to uCOS-II mutex to protect ADC which is going to be a coomon resource for diffrent managers
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "lib_error.h"

#include "hw_gpio.h"
#include "hw_spi.h"

//IMPORTANT! Access to the SPI ADC is protected by mutex to avoid
//           problems when ADC is used by two different managers
//           Mutex is only operated by ADC API functions not accessed directly.
OS_EVENT* AdcMutex; 


//  *********************************************************************************************
//   						InitSpiLcd( )
//  SSP0 is used to handling LCD connection - SSP is setup to work in SPI mode
//	Sets up SPI channel 0 for communications to Nokia 6610 LCD Display                                                          
//     
//	I/O ports used:		PA2  = LCD Reset (set to low to reset)
//						PA12 = LCD chip select (set to low to select the LCD chip)                                                              
//						PA16 = SPI0_MISO Master In - Slave Out (not used in LCD interface)
//						PA17 = SPI0_MOSI Master Out - Slave In pin (Serial Data to LCD slave)
//						PA18 = SPI0_SPCK  Serial Clock (to LCD slave)					                                              
//						PB20 = backlight control (normally PWM control, 1 = full on)	                                                                                                              
//
//  Author:  Olimex, James P Lynch     August 30, 2007
//  IMPORTANT! 
//           Need to be called after GPIO is initailzed
//
//  Original James P Lynch file for SAM7-EX256 was modified to support Olimex LPC2378 STK
//  ********************************************************************************************* 
void InitSpiLcd(void) {
	//SSP channel 0 is used to control LCD in SPI configuration
	//some I/O port lines are used as well
	
	//P1.26 - Output LCD_BL
	FIO1DIR |= BIT26;
	FIO1SET = BIT26; //set P1.26 HIGH should turn on LCD
	
	//P3.25 - Output - LCD_RESET
	FIO3DIR |= BIT25;
	FIO3SET = BIT25; // SetP3.25 to HIGH   (assert LCD Reset low then high to reset the controller)
	
	
	//Setup uP pins for SPI P1.24 - MOSI0, P1.23 - MISO0, P1.21 - SSEL0, P1.20 - SCK0
	PINSEL3 |= BIT17|BIT16|BIT15|BIT14|BIT11|BIT10|BIT9|BIT8;
	
	//setup Control Register to RESET values to assure known starup state with SSP module disbled
	//Default values are modified only when needed to get final SPP configuration
	SSP0CR0 = 0;
	SSP0CR1 = 0;
	
	//select 9 bits transmission, SPI format, CPOL = HIGH inactive CLK state is high
	//CPHA = LOW data captured on falling edge, SCR = 0 Serial Clock equal prescaler clock 
	SSP0CR0 |= BIT6|BIT3;
	
	//set SSP prescaler clock => Fpclk/4 = 12,5 MHz/4 = 3125000Hz <-> 320ns 
	SSP0CPSR = 0x04;
	
	//disable all interrupts - polling mode will be in use
	SSP0IMSC = 0x00; 
	
	//disable DMA 
	SSP0DMACR = 0;
	
	//enable SPP module after setup 
	SSP0CR1 |= BIT1; 
}//InitSpi


//  *****************************************************************************
//   						WriteSpiCommand.c
// 
//     Writes 9-bit command to LCD display via SPI interface
//     
//	   Inputs:	data  -  Leadis LDS176 controller/driver command
//						                                              
//						                                                             
//	   Note:  clears bit 8 to indicate command transfer                                                           
//
//  Author:  Olimex, James P Lynch     August 30, 2007
//  IMPORTANT! 
//  Original James P Lynch file for SAM7-EX256 was modified to support Olimex LPC2378 STK
//  ***************************************************************************** 
void WriteSpiCommand(volatile unsigned int command){
	
	// wait for the previous transfer to complete
	while ((SSP0SR & BIT0) != BIT0);//wait until Transmit FIFO empty

	// clear bit 8 - indicates a "command" 
	command = (command & ~0x0100);

	// send the command
	SSP0DR = command;
}//WriteSpiCommand


//  *****************************************************************************
//   						WriteSpiData.c
// 
//     Writes 9-bit data to LCD display via SPI interface
//     
//	   Inputs:	data  -  Leadis LDS176 controller/driver command
//						                                              
//						                                                             
//	   Note:  Sets bit 8 to indicate data transfer                                                           
//
//  Author:  Olimex, James P Lynch     August 30, 2007
//  IMPORTANT! 
//  Original James P Lynch file for SAM7-EX256 was modified to support Olimex LPC2378 STK
//  ***************************************************************************** 

void WriteSpiData(volatile unsigned int data){

	// wait for the previous transfer to complete
	while ((SSP0SR & BIT0) != BIT0);//wait until Transmit FIFO empty

	// set bit 8, indicates "data" 
	data = (data | 0x0100);

	// send the data
	SSP0DR = data;

}//WriteSpiData

//  *********************************************************************************************
//   						InitSpiAdc( )
//  SSP1 is used to handling A/D MCP 3208. SSP is setup to work in SPI mode and connected to ADC
//  as defined below:
//		MCP 3208 CLK <- SCK1 (P0.7) - ETX2.2
//      MCP 3208 DIN <- MOSI1 (P0.9) - ETX2.4
//      MCP 3208 DOUT -> MISO1 (P0.8) - ETX2.3
//      MCP 3208 NCS <- SSEL1 (P0.6) - ETX2.1
//     
//  IMPORTANT!
//           1) It was noted that SPI speed to ADC cannot be larger to 12,5Mhz/18 to get correct readings
//           for higher speed from time to time there are wrong readings possible.
//           2) Need to be called after GPIO is initailzed (just for the case we decide to control SSEL through GPIO output)
//			 3) Need to be call after uCOS-II is initialized because of ADC mutex
//           
//  ********************************************************************************************* 
void InitSpiAdc(void) 
{
	//SSP channel 1 is used to control ADC MCP3208
	//it is assumed that SSP1 is tern on (PCONP) on uC reset
	
	//Setup uP pins for SPI1 out of P0 port to get below setup (PINSEL0=0 after reset):
	//		MCP 3208 CLK <- SCK1 (P0.7) - ETX2.2
	//      MCP 3208 DIN <- MOSI1 (P0.9) - ETX2.4
	//      MCP 3208 DOUT -> MISO1 (P0.8) - ETX2.3
	//      MCP 3208 NCS <- SSEL1 (P0.6) - ETX2.1	
	PINSEL0 |= BIT19|BIT17|BIT15|BIT13;
	
	//setup Control Register to RESET values to assure known starup state with SSP module disbled
	//Default values are modified only when needed to get final SPP configuration
	SSP1CR0 = 0;
	SSP1CR1 = 0;
	
	//select 8 bits transmission, SPI format, CPOL = HIGH inactive CLK state is high
	//CPHA = HIGH data captured on rising edge, SCR = 0 Serial Clock equal prescaler clock 
	SSP1CR0 |= BIT7|BIT6|BIT2|BIT1|BIT0;
	
	//I decided to increase clock to be closed to 2MHz
	//but 2MHz did not work correctly resulting with some wrong readings so final setup
	//is 12,5Mhz/25 = 500 000Hz -> T = 2us 
	//set SSP prescaler clock => Fpclk/100 = 12,5 MHz/100 = 125000Hz <-> 8uS 
	//SSP1CPSR = 0x64;// div Fpclk by 100
	SSP1CPSR = 0x19;//experimet showed that 0x18 works while 0x17 does not finally i decided for 0x19
	
	//disable all interrupts - polling mode will be in use
	SSP1IMSC = 0x00; 
	
	//disable DMA 
	SSP1DMACR = 0;
	
	//enable SPP module after setup in master mode
	SSP1CR1 |= BIT1; 
	
	//setup ADC mutex
	AdcMutex=OSSemCreate(1);//initialize ADC mutex to avaliable state
	if(!AdcMutex)UCOSII_RES_EXCEPTION;//Exception - when there is not uCOS-II event blocks availiable (uCOS-II resources are not availaible) 
}//InitSpiAdc

/*
*********************************************************************************************************
* Name:                                    GetAdcAccess 
* 
* Description: Call to get access to ADC
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*             Need to be called to get exclusive access to ADC
* *********************************************************************************************************
*/
void GetAdcAccess(void)
{
	BYTE Result;
	OSSemPend(AdcMutex,0x0000,&Result);//for uCOS-II the value 0x0000 means wait for mutex infinite
	if(Result != OS_NO_ERR)UCOSII_RES_EXCEPTION;//Exception when ther is an error
}// GetADCAccess

/*
*********************************************************************************************************
* Name:                                    ReleaseAdcAccess 
* 
* Description: Call to release access to ADC
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*            
* *********************************************************************************************************
*/
void ReleaseAdcAccess(void)
{
	OSSemPost(AdcMutex);//release ADC mutex
}// ReleaseADCAccess

//  *****************************************************************************
//   						GetAdcConversion
// 
//     Initialize ADC conversion for specified channel
//		Input:
//				- number of channel for which we want to initialize ADC conversion
//                0x00 - CH0
//                ...
//                0x07 - CH7
//      Output
//             - 12 bit conversion results (on D0 to D12)
//
//   IMPORTANT!
//           This command sends three bytes to initialize conversion of the ADC
//           after this command 3 bytes from SSP are read
//
//  ***************************************************************************** 
WORD GetAdcConversion(BYTE InAdcChannelNo)
{
	BYTE TmpData1 = 0x00;
	BYTE TmpData2 = 0x00;
	BYTE TmpData3 = 0x00;
	BYTE TmpInAdcChannelNo;
	WORD Result = 0x0000;
	
	//setup first byte
	TmpData1|=BIT2|BIT1;//setup start of conversion and singl-ended DAC mode
	TmpInAdcChannelNo = InAdcChannelNo;
	TmpData1|=(TmpInAdcChannelNo >> 2);//shift InAdcChannelNo right to get BIT2 on BIT0 position

	//setup second byte
	TmpInAdcChannelNo = InAdcChannelNo;
	TmpData2|= (TmpInAdcChannelNo << 6);//move BIT1 and BIT0 to be MSB

	//setup third byte
	TmpData3 =0xAA;//do not care byte it can have any value
	
	while ((SSP1SR & BIT0) != BIT0);//wait until Transmit FIFO empty FIFO is ready to capture up to 8 frames
	SSP1DR = TmpData1;//send first byte
	SSP1DR = TmpData2;//send second byte
	SSP1DR = TmpData3;//send third byte
	
	while ((SSP1SR & BIT4) == BIT4);//wait as long as SSP controler is busy (sending and/or receiving)
	
	TmpData1 = SSP1DR;//read first byte it does not contain any value
	TmpData2 = SSP1DR;//read second byte
	TmpData3 = SSP1DR;//read third byte 
	Result = TmpData2;
	Result = Result << 8;
	Result |= TmpData3;//setup final result
	Result &=0x0FFF;//zeros MSBs
	return Result;
}//	GetAdcConversion



/* IMPORTANT! This is commented out version I returned finally to the original
 * implementation once it was obvious that SPI speed is the root cause of wrong ADC readings
 * not ADC handling
WORD GetAdcConversion(BYTE InAdcChannelNo)
{
	BYTE TmpData1; //reset further
	BYTE TmpData2 = 0x00;
	BYTE TmpData3 = 0x00;
	BYTE TmpInAdcChannelNo;
	WORD Result = 0x0000;
	
	//assure there is not any received data befor we run conversion
	while ((SSP1SR & BIT2) == BIT2)//as long as receive FIFO is not empty take all data to make it empty
		TmpData1 = SSP1DR;//read first byte it does not contain any value
	TmpData1 = 0x00;//reset tmp data before we strat using it
	
	//setu first byte
	TmpData1|=BIT2|BIT1;//setup start of conversion and singl-ended DAC mode
	TmpInAdcChannelNo = InAdcChannelNo;
	TmpData1|=(TmpInAdcChannelNo >> 2);//shift InAdcChannelNo right to get BIT2 on BIT0 position

	//setup second byte
	TmpInAdcChannelNo = InAdcChannelNo;
	TmpData2|= (TmpInAdcChannelNo << 6);//move BIT1 and BIT0 to be MSB

	//setup third byte
	TmpData3 =0xAA;//do not care byte it can have any value
	
	while ((SSP1SR & BIT0) != BIT0);//wait until Transmit FIFO empty FIFO is ready to capture up to 8 frames
	SSP1DR = TmpData1;//send first byte
	SSP1DR = TmpData2;//send second byte
	SSP1DR = TmpData3;//send third byte
	
	while ((SSP1SR & BIT4) == BIT4);//wait as long as SSP controler is busy (sending and/or receiving)
	
	TmpData1 = SSP1DR;//read first byte it does not contain any value
	TmpData2 = SSP1DR;//read second byte
	TmpData3 = SSP1DR;//read third byte 
	Result = TmpData2;
	Result = Result << 8;
	Result |= TmpData3;//setup final result
	Result &=0x0FFF;//zeros MSBs
	return Result;
}//	GetAdcConversion
*/

/*
*********************************************************************************************************
* Name:                                    CalculateAdcOffset
* 
* Description: Calculates ADC offset for specified channel
*       
*
* Arguments:   
* 				InAdcChannelNo - number of channel for which we want to initialize ADC conversion
*                                0x00 - CH0
*                                ...
*                                0x07 - CH7
*               InSamplesNo - number of samples which are averaged needs to be >=1
* 				InDelayInTicks - delay in OS ticks between samples
*
* Returns:     Calculated ofset value for ADC
*
* Note(s):     
*            
* *********************************************************************************************************
*/
WORD CalculateAdcOffset(BYTE InAdcChannelNo, WORD InSamplesNo, WORD InDelayInTicks )
{
	DWORD Offset=0;//store offset in DWORD for the case large number of samples is summed out
	WORD i;//counts subsequent samples taken
	
	if(!InSamplesNo) return 0;
	for(i=1;i <= InSamplesNo;i++)
	{
		Offset += (DWORD)GetAdcConversion(InAdcChannelNo);	
		OSTimeDly(InDelayInTicks);
	}
	Offset=Offset/((DWORD)InSamplesNo);
	return (WORD)Offset;//convert offset back to WORD and return
}//CalculateAdcOffset


	
