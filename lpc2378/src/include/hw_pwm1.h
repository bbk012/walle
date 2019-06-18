/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2012, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_pwm1.h
* Description: Functions to control LPC 2378 uP PWM 1 
* Author:      Bogdan Kowalczyk
* Date:        10-Nov-2012
* History:
*              10-Nov-2012 - Initial version created
*********************************************************************************************************
*/


#ifndef HW_PWM1_H_
#define HW_PWM1_H_
#ifdef __cplusplus
   extern "C" {
#endif

#include "type.h"
	   
//this is value by which Fpclk is divided before applied to PWM1 timer counter
//it is assumed Fplck = 12500000 Hz so for prescaler value 0x01 Fplck is divided by 2
//	   Fplck/(PWM1_PRESCALER_VALUE+1)
//     12500000/(24+1)=500000 - every count correspond to 1/500000=2uS	   
#define PWM1_PRESCALER_VALUE 24   
//#define PWM1_PRESCALER_VALUE 24999 for debug purposes
	   
//PWM cycle time and default PWMs for Head, Left Arm and Right Arm servos 	   
#define INIT_PWM1_MR0_COUNT 10000//setup default PWM cycle 10000*2uS=20ms

//this is number of OS_TCIKS required to assure
//that PWM1 MR0 matches that is needed to assure MR1, MR2 and MR3 are loaded
//after PWM1LER bits are set up to 1.
//Because PWM1 MR0 is setup to reset (match) every 20ms so this value is setup to 30ms (30ms +/-10ms)
#define PWM1_MR0_MATCH 3   
	   
//folowing are the register assignament to particular servos
// MR0 - common cycle time for all PWM waves
// MR1 - determines the Head servo PWM value
// MR2 - determines the left Arm PWM value
// MR3 - determines the right Arm PWM value	   

	   
//theoretical value of servo mid position is setup 750*2uS=1,5ms - default servo central position
//but below are values calibrated from the real corresponding Wall-e's servos	   
#define HEAD_INIT_MR1_COUNT 708 //mid/central head position
#define LEFT_ARM_INIT_MR2_COUNT 688//left arm horizontal position
#define RIGHT_ARM_INIT_MR3_COUNT 746//right arm horizontal position

//minimum allowed PWM counts for servos
#define MIN_PWM1_MR1_COUNT 375 //head 
#define MIN_PWM1_MR2_COUNT 521 //left arm max up phisical servo max is 376 but I limitted it to avoid head hitting
#define PHY_MIN_PWM1_MR2_COUNT 376	   
#define MIN_PWM1_MR3_COUNT 610 //right arm max down original value was 599 but I corrected it to avoid hitting track  

	   
//maximum allowed PWM counts for servos
#define MAX_PWM1_MR1_COUNT 1041 //head
#define MAX_PWM1_MR2_COUNT 820  //left arm max down
#define MAX_PWM1_MR3_COUNT 921  //right arm max up phisical servo max is 1058 but I limitted it to avoid head hitting
#define PHY_MAX_PWM1_MR3_COUNT 1058
	   
// below is the highest smooth speed of servo change which is MIN_COUNT_FOR_PWM_CHANGE/MIN_COUNT_FOR_PWM_DURATION
// what correspond to 2deg/ 30ms	   
#define MIN_COUNT_FOR_PWM_CHANGE	10 //minimal value for which PWM MR can be changed
#define MIN_COUNT_DURATION			PWM1_MR0_MATCH //minimum duration of PWM wave to be recognized by servo
	   
//**********************************************************************************	   
//descriptive position for servos
//HEAD servo	   
#define HEAD_CENTRAL				HEAD_INIT_MR1_COUNT
#define HEAD_MAX_LEFT_TURN	   		MAX_PWM1_MR1_COUNT
#define HEAD_MAX_RIGHT_TURN	   		MIN_PWM1_MR1_COUNT
	   
//change of counts in PWM register corresponding to 15 DEGs ANGLE
#define HEAD_15_DEG                 75
	   
//head fix angle turn positions for left and right head turn	   
#define HEAD_LEFT_15_DEG            783
#define HEAD_LEFT_30_DEG            858
#define HEAD_LEFT_45_DEG            933
#define HEAD_LEFT_60_DEG           	1008
#define HEAD_LEFT_68_DEG	   		HEAD_MAX_LEFT_TURN
	   
#define HEAD_RIGHT_15_DEG           633
#define HEAD_RIGHT_30_DEG           558
#define HEAD_RIGHT_45_DEG           483
#define HEAD_RIGHT_60_DEG           408
#define HEAD_RIGHT_68_DEG         	HEAD_MAX_RIGHT_TURN   

//LEFT ARM servo	   
#define LEFT_ARM_HORIZONTAL		   	LEFT_ARM_INIT_MR2_COUNT 
#define LEFT_ARM_MAX_DOWN	        MAX_PWM1_MR2_COUNT
#define LEFT_ARM_SOME_DOWN	   		(LEFT_ARM_HORIZONTAL+((LEFT_ARM_MAX_DOWN-LEFT_ARM_HORIZONTAL)/2))
//finally MAX up is limited to avoid hit by moving head which is independently controlled	   
#define LEFT_ARM_MAX_UP				MIN_PWM1_MR2_COUNT
#define LEFT_ARM_SOME_UP	   		(LEFT_ARM_HORIZONTAL-((LEFT_ARM_HORIZONTAL-LEFT_ARM_MAX_UP)/2))
#define LEFT_ARM_MAX_UP_PHY			PHY_MIN_PWM1_MR2_COUNT


//RIGHT ARM servo	   
#define RIGHT_ARM_HORIZONTAL		RIGHT_ARM_INIT_MR3_COUNT 
#define RIGHT_ARM_MAX_DOWN	        MIN_PWM1_MR3_COUNT
#define RIGHT_ARM_SOME_DOWN			(RIGHT_ARM_HORIZONTAL-((RIGHT_ARM_HORIZONTAL-RIGHT_ARM_MAX_DOWN)/2))
//finally MAX up is limited to avoid hit by moving head which is independently controlled	   
#define RIGHT_ARM_MAX_UP			MAX_PWM1_MR3_COUNT
#define RIGHT_ARM_SOME_UP			(RIGHT_ARM_HORIZONTAL+((RIGHT_ARM_MAX_UP-RIGHT_ARM_HORIZONTAL)/2))	   
#define RIGHT_ARM_MAX_UP_PHY		PHY_MAX_PWM1_MR3_COUNT
	   
//arm max positions for save head scaning 
#define RIGHT_ARM_MAX_UP_SAVE_POS			890
#define LEFT_ARM_MAX_UP_SAVE_POS			544	   

//delay in ms to get head set to central position for the worst case of movement	   
#define TIME_TO_GET_HEAD_CENTRAL	500   

//delay in ms to get arm moved to safe position 
#define ARM_MOVE_DELAY 				500
	   
/*
*********************************************************************************************************
* Name:                                    InitPWM1  
* 
* Description: Setup PWM1 to be ready to generate PWM for servos control
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
void InitPWM1 (void);
/*
*********************************************************************************************************
* Name:                                    GetPwm1Access 
* 
* Description: Call to get exclusive access to PWM!
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*             Need to be called to get exclusive access to PWM1
* *********************************************************************************************************
*/
void GetPwm1Access(void);

/*
*********************************************************************************************************
* Name:                                    ReleasePwm1Access 
* 
* Description: Call to release exclusive access to PWM1
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
void ReleasePwm1Access(void);
/*
*********************************************************************************************************
* Name:                                    RunPWMHead 
* 
* Description: Setup PWM wave for the Head servo
*
* Arguments:   InCount - numbber of counts which determines the duration of the Head PWM pulse
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void RunPWMHead(WORD InCount);

/*
*********************************************************************************************************
* Name:                                    SmoothHeadMove 
* 
* Description: Moves Wall-e head from start to destination gently and in a smooth way
*
* Arguments:   InStartCount - current head servo position as defined by PWM1MR1 count
*              InEndCount - destination head position as defined by PWM1MR1 count 
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void SmoothHeadMove (WORD InStartCount, WORD InEndCount);

/* NOT USED SO COMMENTED OUT
*********************************************************************************************************
* Name:                                    StopPWMHead 
* 
* Description: Stops PWM wave for the Head servo
*
* Arguments:   none
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
//void StopPWMHead(void);

/*
*********************************************************************************************************
* Name:                                    RunPWMLeftArm 
* 
* Description: Setup PWM wave for the Left Arm servo
*
* Arguments:   InCount - number of counts which determines the duration of the Head PWM pulse
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void RunPWMLeftArm(WORD InCount);


/* NOT USED SO COMMENTED OUT
*********************************************************************************************************
* Name:                                    StopPWMLeftArm 
* 
* Description: Stops PWM wave for the left arm servo
*
* Arguments:   none
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
//void StopPWMLeftArm(void);

/*
*********************************************************************************************************
* Name:                                   SmoothLeftArmMove
* 
* Description: Moves Wall-e left arm from start to destination gently and in a smooth way
*
* Arguments:   InStartCount - current left arm servo position as defined by PWM1MR1 count
*              InEndCount - destination left position as defined by PWM1MR1 count 
*
* Returns:     none
* 
*              IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void SmoothLeftArmMove(WORD InStartCount, WORD InEndCount);

/*
*********************************************************************************************************
* Name:                                    RunPWMRightArm 
* 
* Description: Setup PWM wave for the Right Arm servo
*
* Arguments:   InCount - number of counts which determines the duration of the Head PWM pulse
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void RunPWMRightArm(WORD InCount);

/*
*********************************************************************************************************
* Name:                                   SmoothRightArmMove
* 
* Description: Moves Wall-e right arm from start to destination gently and in a smooth way
*
* Arguments:   InStartCount - current right arm servo position as defined by PWM1MR1 count
*              InEndCount - destination right position as defined by PWM1MR1 count 
*
* Returns:     none
* 
*              IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void SmoothRightArmMove(WORD InStartCount, WORD InEndCount);

/* NOT USED SO COMMENTED OUT
*********************************************************************************************************
* Name:                                    StopPWMRightArm 
* 
* Description: Stops PWM wave for the right arm servo
*
* Arguments:   none
*
* Returns:     none
* 
*IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
//void StopPWMRightArm(void);


/*
*********************************************************************************************************
* Name:                                    GetHeadPosition 
* 
* Description: Get value of PWM counts corresponding to the current Head servo positiom
*
* Arguments:   none
*
* Returns:     Number of counts in PWM match register corresponding to the current head position
* IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
WORD GetHeadPosition(void);

/*
*********************************************************************************************************
* Name:                                    GetLeftArmPosition 
* 
* Description: Get value of PWM counts corresponding to the current left arm servo positiom
*
* Arguments:   none
*
* Returns:     Number of counts in PWM match register corresponding to the current left arm position
* IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
WORD GetLeftArmPosition(void);


/*
*********************************************************************************************************
* Name:                                    GetRightArmPosition 
* 
* Description: Get value of PWM counts corresponding to the current right arm servo positiom
*
* Arguments:   none
*
* Returns:     Number of counts in PWM match register corresponding to the current right arm position
* IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
WORD GetRightArmPosition(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used
#endif /*HW_PWM1_H_*/
