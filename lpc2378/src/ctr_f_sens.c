/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2014, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        ctr_f_sens.c
* Description: Functions to control foto-detectors (fototransistor and fotoresitor)
* Author:      Bogdan Kowalczyk
* Date:        12-Jul-2014
* History:
* 12-Jul-2014 - Initial version
* 
*********************************************************************************************************
*/

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"

#include "hw_spi.h"
#include "ctr_f_sens.h"

/*
*********************************************************************************************************
* Name:                                    ReadRawFotoTransistor
* 
* Description: Reads data from ADC connected to foto transistor (foto-nose) light detector
*       
*
* Arguments:   none
*
* Returns:     Average value of counts corresponding to current light as measured by foto-nose
* 
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of A/D converter reading
* *********************************************************************************************************
*/
WORD ReadRawFotoTransistor(void)
{
	DWORD Reading=0;//sum of ADC distance readings and next avarage of it
	WORD AdcData;//distance sumple
	
	for (int i=0; i<FOTO_TRANSISTOR_READING_COUNTS; i++)//take DISTANCE_READING_COUNTS of distance readings for average
	{
		GetAdcAccess();//guarantee exclusive access to ADC for this task
		AdcData=GetAdcConversion(ADC_FOTO_TRANSISTOR);//get current ADC reading for foto transistor (nose) detector
		ReleaseAdcAccess();//release exclusive access to ADC
		Reading+=(DWORD)AdcData;//update sum of readings
		OSTimeDly(FOTO_TRANSISTOR_READING_DELAY_TICKS);//wait to before next reading 
	}
	return (WORD)(Reading/FOTO_TRANSISTOR_READING_COUNTS);//get ADC readings average	
}//ReadRawFotoTransistor


/*
*********************************************************************************************************
* Name:                                  ReadRawFotoResistor
* 
* Description: Reads data from ADC connected to foto resistor (foto-tail) light detector
*       
*
* Arguments:   none
*
* Returns:     Average value of counts corresponding to current light strength as mesured by foto-tail
* 
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of A/D converter reading
* *********************************************************************************************************
*/
WORD ReadRawFotoResistor(void)
{
	DWORD Reading=0;//sum of ADC distance readings and next avarage of it
	WORD AdcData;//distance sumple
	
	for (int i=0; i<FOTO_RESISTOR_READING_COUNTS; i++)//take DISTANCE_READING_COUNTS of distance readings for average
	{
		GetAdcAccess();//guarantee exclusive access to ADC for this task
		AdcData=GetAdcConversion(ADC_FOTO_RESISTOR);//get current ADC reading for foto transistor (nose) detector
		ReleaseAdcAccess();//release exclusive access to ADC
		Reading+=(DWORD)AdcData;//update sum of readings
		OSTimeDly(FOTO_RESISTOR_READING_DELAY_TICKS);//wait to before next reading 
	}
	return (WORD)(Reading/FOTO_RESISTOR_READING_COUNTS);//get ADC readings average	
}//ReadRawFotoResistor


/*
*********************************************************************************************************
* Name:                                    CheckNoseDayState
* 
* Description: Reads nose fototransistor and determines day state as seen by it
*       
*
* Arguments:   Adres of storage place for raw fototransistor data
*
* Returns:     Constant which determines day state
*				DARK_DAY	
*				GREY_DAY
*				BRIGHT_DAY
*       IMPORTANT! Updates fototransistor raw data storage when returned
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of fototransistor detector output reading
* *********************************************************************************************************
*/
BYTE CheckNoseDayState(WORD *RawFotoTransistor)
{
	WORD Reading;//storage for IR detector reading
	
	Reading=ReadRawFotoTransistor();//get fototransistor reading
	*RawFotoTransistor=Reading;//stare raw data at the storage location
	
	//check for various types of day states
	if(Reading <= FOTO_TRANSISTOR_DARK)//check for DARK
	{
		return DARK_DAY;
	}
	else if (Reading <= FOTO_TRANSISTOR_BRIGHT)
	{
		return GREY_DAY;
	}
	else
	{
		return BRIGHT_DAY;
	}
} //CheckNoseDayState

/*
*********************************************************************************************************
* Name:                                    CheckTailDayState
* 
* Description: Reads tail fotoresistor and determines day state as seen by it
*       
*
* Arguments:   Adres of storage place for raw fotoresistor data
*
* Returns:     Constant which determines day state
*				DARK_DAY	
*				GREY_DAY
*				BRIGHT_DAY
*       IMPORTANT! Updates fotoresistor raw data storage when returned
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of fotoresistor detector output reading
* *********************************************************************************************************
*/
BYTE CheckTailDayState(WORD *RawFotoResistor)
{
	WORD Reading;//storage for IR detector reading
	
	Reading=ReadRawFotoResistor();//get fotoresistor reading
	*RawFotoResistor=Reading;//stare raw data at the storage location
	
	//check for various types of day states
	if(Reading <= FOTO_RESISTOR_DARK)//check for DARK
	{
		return DARK_DAY;
	}
	else if (Reading <= FOTO_RESISTOR_BRIGHT)
	{
		return GREY_DAY;
	}
	else
	{
		return BRIGHT_DAY;
	}
} //CheckNoseDayState
