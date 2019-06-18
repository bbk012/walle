/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2014, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        ctr_f_sens.h
* Description: Functions to control photo-detectors (phototransistor and photoresitor)
* Author:      Bogdan Kowalczyk
* Date:        12-Jul-2014
* History:
* 12-Jul-2014 - Initial version
* 
*********************************************************************************************************
*/
#ifndef CTR_F_SENS_H_
#define CTR_F_SENS_H_

#ifdef __cplusplus
extern "C" {
#endif
	


#define FOTO_TRANSISTOR_READING_DELAY_TICKS 1 //delay in OS_TICKs for subsequent photo transistor voltage reading by A/D
#define FOTO_TRANSISTOR_READING_COUNTS		10 //number of readings taken into account for photo-transistor reading average	

#define FOTO_RESISTOR_READING_DELAY_TICKS 	1 //delay in OS_TICKs for subsequent photo resistor voltage reading by A/D
#define FOTO_RESISTOR_READING_COUNTS		10 //number of readings taken into account for photo-resistor reading average	

#define MIN_FOTO_TRANSISTOR_COUNT			5 //minimum count to consider photoresistor reading
#define MIN_FOTO_RESISTOR_COUNT				5 //minimum count to consider photoresistor reading
	
//day light constants and strings
#define DARK_DAY		1
#define DARK_DAY_STR	"DARK"	

#define GREY_DAY		2
#define GREY_DAY_STR	"GREY"	
	
#define BRIGHT_DAY		3
#define BRIGHT_DAY_STR	"BRIGHT"
#define UNDEF_DAY_STR	"UNDEF"	
	
//threshold to determine a day frome a night
//foto transistor limits (foto nose limits)

#define FOTO_TRANSISTOR_DARK 	10
#define FOTO_TRANSISTOR_BRIGHT  1000
	
//foto resistor limits (foto tail limits)
	
#define FOTO_RESISTOR_DARK 		150 //voltage below this threshold means definite night for a tail sensor
#define FOTO_RESISTOR_BRIGHT 	2500 //voltage below this threshold means definite night for a tail sensor	
	
	
	
/*
*********************************************************************************************************
* Name:                                    ReadRawFotoTransistor
* 
* Description: Reads data from ADC connected to photo transistor (photo-nose) light detector
*       
*
* Arguments:   none
*
* Returns:     Average value of counts corresponding to current light as measured by photo-nose
* 
* Note(s):     
*            When executed gets exclusive access to ADC for a moment of A/D converter reading
* *********************************************************************************************************
*/
WORD ReadRawFotoTransistor(void);


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
WORD ReadRawFotoResistor(void);


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
BYTE CheckNoseDayState(WORD *RawFotoTransistor);


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
BYTE CheckTailDayState(WORD *RawFotoResistor);

	
#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*CTR_F_SENS_H_*/

