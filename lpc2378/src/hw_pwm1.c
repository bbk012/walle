/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2012, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_pwm1.c
* Description: Functions to control LPC 2378 uP PWM 1 
* Author:      Bogdan Kowalczyk
* Date:        10-Nov-2012
* History:
*              10-Nov-2012 - Initial version created
*********************************************************************************************************
*/
#include "hw_pwm1.h"
#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "type.h"
#include "hw_lpc2378.h"
#include "lib_error.h"
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "hw_gpio.h"

//IMPORTANT! Access to the PWM1 is protected by mutex to avoid
//           problems when PWM1 is used by two different managers
//           Mutex is only operated by PWM! API functions not accessed directly.
OS_EVENT* Pwm1Mutex; 


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
*             MR0 - determines the PWM cycle - 20ms for Hitec Servo
*             MR1 - determines initial position of Head seen on P2.0 pin
*             MR2 - determines initial position of the left arm seen on P2.1
*             MR3 - determines initial position of the right arm seen on P2.2   
* 
* *********************************************************************************************************
*/
void InitPWM1 (void)
{
	//PCONP|=BIT6;//enable PWM1 - by default on uP rest it is on so this line is commented out
	PINSEL4|=BIT0|BIT2|BIT4;//put PWM1.1 on P2.0 (Head), PWM1.2 on P2.1(Left Arm) and PWM1.3 on P2.2(Right Arm)
	
	//PWM1IR=NA;//clear all PWM1 interrupts - by defaults after uP reset all are cleared so line is commented out
	PWM1TCR=0x00;//disble PWM1 Timer Counter and Prescale Counters - that is reset default but I do not commented it out
	
	PWM1PR=PWM1_PRESCALER_VALUE;//setup prescaler to divide Fplck/(PWM1_PRESCALER_VALUE+1) setup to 2uS
	PWM1PCR=BIT9|BIT10|BIT11;//enable PWM1, PWM2 an PWM3 outputs with default single edge PWM mode
	PWM1MCR=BIT1;//on match reset the PWM counter and do not generate interrupt (interrupt are not used to control PWM)
	PWM1MR0=INIT_PWM1_MR0_COUNT;//setup default PWM cycle
	PWM1MR1=HEAD_INIT_MR1_COUNT;//seup initial position for the Head PWM (central looking direct ahead)
	PWM1MR2=LEFT_ARM_INIT_MR2_COUNT;//setup initial position for the Left Arm PWM (horizontal)
	PWM1MR3=RIGHT_ARM_INIT_MR3_COUNT;//setup initial position for the right Arm PWM (horizontal)
	
	PWM1LER=BIT0|BIT1|BIT2|BIT3;//enable shadow match for MR0, MR1, MR2 and MR3
	PWM1TCR=BIT2;//reset counter and prescaler
	PWM1TCR=BIT3|BIT0;//enable counter and PWM and release counter from reset done with previus line
	
	//setup ADC mutex
	Pwm1Mutex=OSSemCreate(1);//initialize ADC mutex to avaliable state
	if(!Pwm1Mutex)UCOSII_RES_EXCEPTION;//Exception - when there is not uCOS-II event blocks availiable (uCOS-II resources are not availaible) 
}//InitPWM1

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
void GetPwm1Access(void)
{
	BYTE Result;
	OSSemPend(Pwm1Mutex,0x0000,&Result);//for uCOS-II the value 0x0000 means wait for mutex infinite
	if(Result != OS_NO_ERR)UCOSII_RES_EXCEPTION;//Exception when ther is an error
}// GetPwm1Access

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
void ReleasePwm1Access(void)
{
	OSSemPost(Pwm1Mutex);//release PWM1 mutex
}// ReleasePwm1Access


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
*	IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void RunPWMHead(WORD InCount)
{
	GetPwm1Access();//get exclusive access tp PWM1 controller
	if(InCount > MAX_PWM1_MR1_COUNT)//protect against too high value not accepted by the servo
	{
		PWM1MR1 = MAX_PWM1_MR1_COUNT;
	}
	else if (InCount < MIN_PWM1_MR1_COUNT)//protect against too low value not accepted by the servo
	{
		PWM1MR1 = MIN_PWM1_MR1_COUNT;	
	}
	else //if count value in the correct range set it up
	{
		PWM1MR1 = InCount;
	}
	PWM1LER =BIT1;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
	OSTimeDly(PWM1_MR0_MATCH);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
}//RunPWMHead

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
*	IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void SmoothHeadMove (WORD InStartCount, WORD InEndCount)
{
	DWORD NoOfChanges;//how many changes shoud occur between InStartCount and InEndCount
	DWORD Reminder;//reminder used to correct NoOfChanges when required to get destination count
	DWORD PWM_MR;//counted value of PWMR
	//check if InStartCount is in the allwed limit if not return without any execution
	//when InStartCount is wrong it means setup of servo is from any reason wrong so command cannot be executed
	
	if(InStartCount > MAX_PWM1_MR1_COUNT)//protect against too high value not accepted by the servo
	{
		return;
	}
	
	if (InStartCount < MIN_PWM1_MR1_COUNT)//protect against too low value not accepted by the servo
	{
		return;	
	}
		
	//check destination limits and correct if exceeded
	if(InEndCount > MAX_PWM1_MR1_COUNT)//protect against too high value not accepted by the servo
	{
		InEndCount = MAX_PWM1_MR1_COUNT;
	}
	else if (InEndCount < MIN_PWM1_MR1_COUNT)//protect against too low value not accepted by the servo
	{
		InEndCount = MIN_PWM1_MR1_COUNT;	
	}
		
	if(InStartCount==InEndCount)//when none movement required exit
	{
		return;
	}
	
	//setup initail PWM1MR1 value we start with
	PWM_MR = InStartCount;
	
	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1MR1 = InStartCount;
	PWM1LER =BIT1;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
	OSTimeDly(PWM1_MR0_MATCH);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
	
	if(InEndCount > InStartCount )//when we should increase MR1 counts
	{
		//calculate how many PWM count chages is required from InSatrtCount to InEndCount
		NoOfChanges = (InEndCount-InStartCount)/MIN_COUNT_FOR_PWM_CHANGE;
		Reminder = (InEndCount-InStartCount)/MIN_COUNT_FOR_PWM_CHANGE;//when Reminder it means number of changes need to be corrected
		if(Reminder != 0) NoOfChanges+=1;//increase number of changes
		for (int i=0; i<NoOfChanges;i++)//smoothly change step by step from InStartCount to InEndCount 
		{
			PWM_MR += MIN_COUNT_FOR_PWM_CHANGE;//increase PWM step by step smoothly
			if(PWM_MR>InEndCount)//when end value exceeded correct it to match destination
				PWM_MR=InEndCount;
			GetPwm1Access();//get exclusive access tp PWM1 controller
			PWM1MR1 = PWM_MR;//setup calculated value
			PWM1LER =BIT1;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
			OSTimeDly(MIN_COUNT_DURATION);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
			ReleasePwm1Access();//PWM1 exclusive access is not any longer required
		}//for
	}
	else //when we should decrease MR1 counts
	{
		//calculate how many PWM count chages is required from InSatrtCount to InEndCount
		NoOfChanges = (InStartCount-InEndCount)/MIN_COUNT_FOR_PWM_CHANGE;
		Reminder = (InStartCount-InEndCount)/MIN_COUNT_FOR_PWM_CHANGE;//when Reminder it means number of changes need to be corrected
		if(Reminder != 0) NoOfChanges+=1;//increase number of changes
		for (int i=0; i<NoOfChanges;i++)
		{
			PWM_MR -= MIN_COUNT_FOR_PWM_CHANGE;//decrease PWM step by step smoothly
			if(PWM_MR < InEndCount)//when end value exceeded correct it to match destination
				PWM_MR=InEndCount;
			GetPwm1Access();//get exclusive access tp PWM1 controller
			PWM1MR1 = PWM_MR;//setup calculated value
			PWM1LER =BIT1;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
			OSTimeDly(MIN_COUNT_DURATION);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
			ReleasePwm1Access();//PWM1 exclusive access is not any longer required
		}//for		
	}
}//SmoothHeadMove

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
*       IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************

void StopPWMHead(void)
{
	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1PCR&=(~BIT9);//disable PWM1 output if it was enabled previously
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
}//StopPWMHead

*/

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
* IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void RunPWMLeftArm(WORD InCount)
{
	GetPwm1Access();//get exclusive access tp PWM1 controller
	if(InCount > MAX_PWM1_MR2_COUNT)//protect against too high value not accepted by the servo
	{
		PWM1MR2 = MAX_PWM1_MR2_COUNT;
	}
	else if (InCount < MIN_PWM1_MR2_COUNT)//protect against too low value not accepted by the servo
	{
		PWM1MR2 = MIN_PWM1_MR2_COUNT;	
	}
	else //if count value in the correct range set it up
	{
		PWM1MR2 = InCount;
	}
	PWM1LER=BIT2;//enable shadow match for PWM2 not disturbing eventual other shadows enabled 
	OSTimeDly(PWM1_MR0_MATCH);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
}//RunPWMLeftArm


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
void SmoothLeftArmMove(WORD InStartCount, WORD InEndCount)
{
	DWORD NoOfChanges;//how many changes shoud occur between InStartCount and InEndCount
	DWORD Reminder;//reminder used to correct NoOfChanges when required to get destination count
	DWORD PWM_MR;//counted value of PWMR
	//check if InStartCount is in the allwed limit if not return without any execution
	//when InStartCount is wrong it means setup of servo is from any reason wrong so command cannot be executed
	
	if(InStartCount > MAX_PWM1_MR2_COUNT)//protect against too high value not accepted by the servo
	{
		return;
	}
	
	if (InStartCount < MIN_PWM1_MR2_COUNT)//protect against too low value not accepted by the servo
	{
		return;	
	}
		
	//check destination limits and correct if exceeded
	if(InEndCount > MAX_PWM1_MR2_COUNT)//protect against too high value not accepted by the servo
	{
		InEndCount = MAX_PWM1_MR2_COUNT;
	}
	else if (InEndCount < MIN_PWM1_MR2_COUNT)//protect against too low value not accepted by the servo
	{
		InEndCount = MIN_PWM1_MR2_COUNT;	
	}
		
	if(InStartCount==InEndCount)//when none movement required exit
	{
		return;
	}
	
	//setup initail PWM1MR2 value we start with
	PWM_MR = InStartCount;
	
	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1MR2 = InStartCount;
	PWM1LER =BIT2;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
	OSTimeDly(PWM1_MR0_MATCH);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
	
	if(InEndCount > InStartCount )//when we should increase MR1 counts
	{
		//calculate how many PWM count chages is required from InSatrtCount to InEndCount
		NoOfChanges = (InEndCount-InStartCount)/MIN_COUNT_FOR_PWM_CHANGE;
		Reminder = (InEndCount-InStartCount)/MIN_COUNT_FOR_PWM_CHANGE;//when Reminder it means number of changes need to be corrected
		if(Reminder != 0) NoOfChanges+=1;//increase number of changes
		for (int i=0; i<NoOfChanges;i++)//smoothly change step by step from InStartCount to InEndCount 
		{
			PWM_MR += MIN_COUNT_FOR_PWM_CHANGE;//increase PWM step by step smoothly
			if(PWM_MR>InEndCount)//when end value exceeded correct it to match destination
				PWM_MR=InEndCount;
			GetPwm1Access();//get exclusive access tp PWM1 controller
			PWM1MR2 = PWM_MR;//setup calculated value
			PWM1LER =BIT2;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
			OSTimeDly(MIN_COUNT_DURATION);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
			ReleasePwm1Access();//PWM1 exclusive access is not any longer required
		}//for
	}
	else //when we should decrease MR1 counts
	{
		//calculate how many PWM count chages is required from InSatrtCount to InEndCount
		NoOfChanges = (InStartCount-InEndCount)/MIN_COUNT_FOR_PWM_CHANGE;
		Reminder = (InStartCount-InEndCount)/MIN_COUNT_FOR_PWM_CHANGE;//when Reminder it means number of changes need to be corrected
		if(Reminder != 0) NoOfChanges+=1;//increase number of changes
		for (int i=0; i<NoOfChanges;i++)
		{
			PWM_MR -= MIN_COUNT_FOR_PWM_CHANGE;//decrease PWM step by step smoothly
			if(PWM_MR < InEndCount)//when end value exceeded correct it to match destination
				PWM_MR=InEndCount;
			GetPwm1Access();//get exclusive access tp PWM1 controller
			PWM1MR2 = PWM_MR;//setup calculated value
			PWM1LER =BIT2;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
			OSTimeDly(MIN_COUNT_DURATION);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
			ReleasePwm1Access();//PWM1 exclusive access is not any longer required
		}//for		
	}
}//SmoothLeftArmMove


/* NOT USED SO COMMENTED OUT
*********************************************************************************************************
* Name:                                    StopPWMLeftArm 
* 
* Description: Stops PWM wave for the left arm servo
*
* Arguments:   none
*
* Returns:     none
* 		IMPORTANT! Internally uses PWM1 Mutex
*
* *********************************************************************************************************

void StopPWMLeftArm(void)
{
	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1PCR&=(~BIT10);//disable PWM2 output if it was enabled previously
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
}//StopPWMLeftArm
*/


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
* 		IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
void RunPWMRightArm(WORD InCount)
{
	GetPwm1Access();//get exclusive access tp PWM1 controller
	if(InCount > MAX_PWM1_MR3_COUNT)//protect against too high value not accepted by the servo
	{
		PWM1MR3 = MAX_PWM1_MR3_COUNT;
	}
	else if (InCount < MIN_PWM1_MR3_COUNT)//protect against too low value not accepted by the servo
	{
		PWM1MR3 = MIN_PWM1_MR3_COUNT;	
	}
	else //if count value in the correct range set it up
	{
		PWM1MR3 = InCount;
	}
	PWM1LER=BIT3;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
	OSTimeDly(PWM1_MR0_MATCH);//wait until for sure MR0 match and load shadow register to be sure MR3 value is setup
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
}//RunPWMRightArm

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
void SmoothRightArmMove(WORD InStartCount, WORD InEndCount)
{
	DWORD NoOfChanges;//how many changes shoud occur between InStartCount and InEndCount
	DWORD Reminder;//reminder used to correct NoOfChanges when required to get destination count
	DWORD PWM_MR;//counted value of PWMR
	//check if InStartCount is in the allwed limit if not return without any execution
	//when InStartCount is wrong it means setup of servo is from any reason wrong so command cannot be executed
	
	if(InStartCount > MAX_PWM1_MR3_COUNT)//protect against too high value not accepted by the servo
	{
		return;
	}
	
	if (InStartCount < MIN_PWM1_MR3_COUNT)//protect against too low value not accepted by the servo
	{
		return;	
	}
		
	//check destination limits and correct if exceeded
	if(InEndCount > MAX_PWM1_MR3_COUNT)//protect against too high value not accepted by the servo
	{
		InEndCount = MAX_PWM1_MR3_COUNT;
	}
	else if (InEndCount < MIN_PWM1_MR3_COUNT)//protect against too low value not accepted by the servo
	{
		InEndCount = MIN_PWM1_MR3_COUNT;	
	}
		
	if(InStartCount==InEndCount)//when none movement required exit
	{
		return;
	}
	
	//setup initail PWM1MR2 value we start with
	PWM_MR = InStartCount;
	
	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1MR3 = InStartCount;
	PWM1LER =BIT3;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
	OSTimeDly(PWM1_MR0_MATCH);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
	
	if(InEndCount > InStartCount )//when we should increase MR1 counts
	{
		//calculate how many PWM count chages is required from InSatrtCount to InEndCount
		NoOfChanges = (InEndCount-InStartCount)/MIN_COUNT_FOR_PWM_CHANGE;
		Reminder = (InEndCount-InStartCount)/MIN_COUNT_FOR_PWM_CHANGE;//when Reminder it means number of changes need to be corrected
		if(Reminder != 0) NoOfChanges+=1;//increase number of changes
		for (int i=0; i<NoOfChanges;i++)//smoothly change step by step from InStartCount to InEndCount 
		{
			PWM_MR += MIN_COUNT_FOR_PWM_CHANGE;//increase PWM step by step smoothly
			if(PWM_MR>InEndCount)//when end value exceeded correct it to match destination
				PWM_MR=InEndCount;
			GetPwm1Access();//get exclusive access tp PWM1 controller
			PWM1MR3 = PWM_MR;//setup calculated value
			PWM1LER =BIT3;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
			OSTimeDly(MIN_COUNT_DURATION);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
			ReleasePwm1Access();//PWM1 exclusive access is not any longer required
		}//for
	}
	else //when we should decrease MR1 counts
	{
		//calculate how many PWM count chages is required from InSatrtCount to InEndCount
		NoOfChanges = (InStartCount-InEndCount)/MIN_COUNT_FOR_PWM_CHANGE;
		Reminder = (InStartCount-InEndCount)/MIN_COUNT_FOR_PWM_CHANGE;//when Reminder it means number of changes need to be corrected
		if(Reminder != 0) NoOfChanges+=1;//increase number of changes
		for (int i=0; i<NoOfChanges;i++)
		{
			PWM_MR -= MIN_COUNT_FOR_PWM_CHANGE;//decrease PWM step by step smoothly
			if(PWM_MR < InEndCount)//when end value exceeded correct it to match destination
				PWM_MR=InEndCount;
			GetPwm1Access();//get exclusive access tp PWM1 controller
			PWM1MR3 = PWM_MR;//setup calculated value
			PWM1LER =BIT3;//enable shadow match for PWM1 not disturbing eventual other shadows enabled 
			OSTimeDly(MIN_COUNT_DURATION);//wait until for sure MR0 match and load shadow register to be sure MR2 is setup
			ReleasePwm1Access();//PWM1 exclusive access is not any longer required
		}//for		
	}
}//SmoothRightArmMove

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
*		IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************

void StopPWMRightArm(void)
{
	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1PCR&=(~BIT11);//disable PWM3 output if it was enabled previously
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
}//StopPWMRightArm
*/

/*
*********************************************************************************************************
* Name:                                    GetHeadPosition 
* 
* Description: Get value of PWM counts corresponding to the current Head servo positiom
*
* Arguments:   none
*
* Returns:     Number of counts in PWM match register corresponding to the current head position
* 		IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
WORD GetHeadPosition(void)
{
	DWORD PWM1MR ;//temp storage for MR value

	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1MR=PWM1MR1;
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
	return PWM1MR;
} //GetHeadPosition

/*
*********************************************************************************************************
* Name:                                    GetLeftArmPosition 
* 
* Description: Get value of PWM counts corresponding to the current left arm servo positiom
*
* Arguments:   none
*
* Returns:     Number of counts in PWM match register corresponding to the current left arm position
* 		IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
WORD GetLeftArmPosition(void)
{
	DWORD PWM1MR ;//temp storage for MR value

	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1MR=PWM1MR2;
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
	return PWM1MR;
} //GetLeftArmPosition

/*
*********************************************************************************************************
* Name:                                    GetRightArmPosition 
* 
* Description: Get value of PWM counts corresponding to the current right arm servo positiom
*
* Arguments:   none
*
* Returns:     Number of counts in PWM match register corresponding to the current right arm position
* 		IMPORTANT! Internally uses PWM1 Mutex
* *********************************************************************************************************
*/
WORD GetRightArmPosition(void)
{
	DWORD PWM1MR ;//temp storage for MR value

	GetPwm1Access();//get exclusive access tp PWM1 controller
	PWM1MR=PWM1MR3;
	ReleasePwm1Access();//PWM1 exclusive access is not any longer required
	return PWM1MR;

} //GetRightArmPosition
