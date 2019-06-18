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
* 
*********************************************************************************************************
*/
#ifndef SPI_H_
#define SPI_H_

#ifdef __cplusplus
   extern "C" {
#endif
#include "type.h"
	   
#define ADC_DISTANCE_DETECTOR 	0 //distance detector is at ADC channel 0	
#define ADC_SERVO 				1 //servo power supply voltage is at ADC channel 1
#define ADC_MAIN_SUPPLY			2 //battery supply for uP board is at ADC channel 2
#define ADC_MOTOR_SUPPLY		3 //battery supply for motors is at ADC channel 3	
#define ADC_ARS_SENSOR			4 //angular rate sensor
#define ADC_FOTO_TRANSISTOR		5 //fototranzistor nose sensor
#define ADC_FOTO_RESISTOR		6 //fotoresistor tail sensor
	   
//  *********************************************************************************************
//   						InitSpiLcd( )
// 
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
extern void InitSpiLcd(void);


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
extern void WriteSpiCommand(volatile unsigned int command);


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

extern void WriteSpiData(volatile unsigned int data);


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
//           Need to be called after GPIO is initailzed (just for the case we decide to control SSEL through GPIO output)
//
//  ********************************************************************************************* 
extern void InitSpiAdc(void); 


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
extern void GetAdcAccess(void);


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
extern void ReleaseAdcAccess(void);

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
//             - 12 bit conversion results (on D0 to D12 bits)
//
//   IMPORTANT!
//           This command sends three bytes to initialize conversion of the ADC
//           after this command 3 bytes from SSP are read
//
//  ***************************************************************************** 
extern WORD GetAdcConversion(BYTE InAdcChannelNo);


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
*               InSamplesNo - number of samples which are averaged
* 				InDelayInTicks - delay in OS ticks between samples
*
* Returns:     Calculated ofset value for ADC
*
* Note(s):     
*            
* *********************************************************************************************************
*/
extern WORD CalculateAdcOffset(BYTE InAdcChannelNo, WORD InSamplesNo, WORD InDelayInTicks );

#ifdef __cplusplus
}
#endif //to close extern "C" if used
 
#endif /*SPI_H_*/
