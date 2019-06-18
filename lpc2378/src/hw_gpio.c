/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_gpio.c
* Description: Lpc 2378 uP General Purpose Input/Output initialization and control procedures 
* Author:      Bogdan Kowalczyk
* Date:        2-Aug-2008
* History:
* 2-Aug-2008  - Initial version created based on NXP target.h for NXP LPC23xx/24xx Family Microprocessors
* 4-Oct-2009  - Added P4 handling to control motor
* 2-Mar-2010  - Added left and right tracks pulses handling for movement control
* 23-Jul-2011 - Track control moved to separate track tasks, Port2Int simplified to signal track control task only
*********************************************************************************************************
*/
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"
#include "type.h"

#include "hw_lpc23xx.h" //Header file for NXP LPC23xx/24xx Family Microprocessors
#include "hw_gpio.h"
#include "hw_timer.h" //get access to motor controll functions and timer 3 ticks counter

#include "ctr_lcd.h"
#include "lib_error.h" //to get access to exceptions IDs


//communication between Port2Isr and track control task is throught a mailbox.
//mailbox is notified with the timestamp of trigerred left and right trac falling pulse
OS_EVENT *LeftTrackMailbox;
OS_EVENT *RightTrackMailbox;

/*
*********************************************************************************************************
* Name:                                    InitGPIO 
* 
* Description: Call at beginning after uP RESET to setup GPIO initial control values
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
void InitGPIO(void)
{
    SCS |= BIT0; //turn on fast I/O for PORT0 and PORT1
    //at beginning all PORT0, PORT1, PORT2, PORT3 and PORT4 lines setup as inputs (de-facto same as after reset)
    FIO0DIR = 0x00000000;
    FIO1DIR = 0x00000000;
    FIO2DIR = 0x00000000;
    FIO3DIR = 0x00000000;
    FIO4DIR = 0x00000000;
    
    // set I/O pins as output for led P0.21
    FIO0DIR |= BIT21;
    
    // set I/O pins as output for Lcd bACKLIGHT P1.26
    FIO1DIR |= BIT26;
    
    //set I/O Pins for PORT2 to control track pulse transoptor P2.8 transoptors on/off P2.8 -> output line
    FIO2DIR |= BIT8;
    
    //set P3.24 as OUTPUT to control Ultrasonic TRIG pin
    FIO3DIR |= BIT24;
    FIO3CLR = BIT24;//put zero at the P3.24 PIN (Ultrasonic TRIG output is zero at startup).
    
    //set motor control I/O pins of port 4 as output
    // P4.31 - MOTOR_PWR_ON
    // P4.30 - IN1A
    // P4.25 - IN2A
    // P4.24 - IN1B
    // P4.15 - IN2B
    FIO4DIR |= BIT31|BIT30|BIT25|BIT24|BIT15;
    FIO4CLR = BIT24|BIT15|BIT30|BIT25;// left & right motor fast stop
    
    //set control pin of P4 as output and turn off buzzer
    //P4.14 - Buzzer On/Off
    //P4.13 - Distance Detector power on/off
    //P4.12 - uP board power on/off
    //P4.11 - Head Servo power on/off
    //P4.10 - Arm Servo power on/off
    FIO4DIR |=BIT14|BIT13|BIT12|BIT11|BIT10;
    FIO4CLR = BIT14;//turn off buzzer
    FIO4SET = BIT12;//turn on uP power supply (turn on swith does not need to be any longer pressed)
    
    return;  
}//InitGPIO


/*
*********************************************************************************************************
* Name:                                    GetBut1 
* 
* Description: Read port input connected to BUTTON1
*       
*
* Arguments:   none
*
* Returns:     P0.29=0 when Button1 pressed or P0.29=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetBut1(void)
{
	return (FIO0PIN & BIT29);
}//GetBut1

/*
*********************************************************************************************************
* Name:                                    GetBut2 
* 
* Description: Read port input connected to BUTTON2
*       
*
* Arguments:   none
*
* Returns:     P0.18=0 when Button1 pressed or P0.18=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetBut2(void)
{
	return (FIO0PIN & BIT18);	
}//GetBut2


/*
*********************************************************************************************************
* Name:                                    GetJ1Up 
* 
* Description: Read port input connected to J1 position UP
*       
*
* Arguments:   none
*
* Returns:     P1.18=0 when Up pressed or P1.18=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetJ1Up(void)
{
	return (FIO1PIN & BIT18);
}//GetJ1Up

/*
*********************************************************************************************************
* Name:                                    GetJ1Down 
* 
* Description: Read port input connected to J1 position DOWN
*       
*
* Arguments:   none
*
* Returns:     P1.19=0 when Down pressed or P1.19=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetJ1Down(void)
{
	return (FIO1PIN & BIT19);
}//GetJ1Down

/*
*********************************************************************************************************
* Name:                                    GetJ1Left 
* 
* Description: Read port input connected to J1 position LEFT
*       
*
* Arguments:   none
*
* Returns:     P1.27=0 when Left pressed or P1.27=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetJ1Left(void)
{
	return (FIO1PIN & BIT27);
}//GetJ1Left

/*
*********************************************************************************************************
* Name:                                    GetJ1Right 
* 
* Description: Read port input connected to J1 position RIGHT
*       
*
* Arguments:   none
*
* Returns:     P1.22=0 when Right pressed or P1.22=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetJ1Right(void)
{
	return (FIO1PIN & BIT22);
}//GetJ1Right

/*
*********************************************************************************************************
* Name:                                    GetJ1Center 
* 
* Description: Read port input connected to J1 position CENTER
*       
*
* Arguments:   none
*
* Returns:     P1.25=0 when Right pressed or P1.25=1 when not pressed
*
* Note(s):     
* 
* *********************************************************************************************************
*/
DWORD GetJ1Center(void)
{
	return (FIO1PIN & BIT25);
}//GetJ1Center

/*
*********************************************************************************************************
* Name:                                    LedOn 
* 
* Description: Turn On LED on OLIMEX LPC2378 development board.
*              LPC 2378 uP P0.21 line is used to turn on/turn off SD card reader power signalled by the LED.
*             
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void LedOn(void)
{
	FIO0SET = BIT21;	
}//LedOn

/*
*********************************************************************************************************
* Name:                                    LedOff 
* 
* Description: Turn Off LED on OLIMEX LPC2378 development board.
*              LPC 2378 uP P0.21 line is used to turn on/turn off SD card reader power signalled by the LED.      
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void LedOff(void)
{
	FIO0CLR = BIT21;
}//LedOff


/*
*********************************************************************************************************
* Name:                                     TransoptorOn 
* 
* Description: Turn On  Transoptors for tracks movement sensing
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void  TransoptorOn(void)
{
	FIO2SET = BIT8;	
}// TransoptorOn

/*
*********************************************************************************************************
* Name:                                    TransoptorOff 
* 
* Description: Turn Off transoptors of tracks.   
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void TransoptorOff(void)
{
	FIO2CLR = BIT8;
}//TransoptorOff

/*
*********************************************************************************************************
* Name:                                    BuzzerOn 
* 
* Description: Turn On Buzzer controlled by P4.14 i/o on the interface board.
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void BuzzerOn(void)
{
	FIO4SET = BIT14;	
}//BuzzerOn

/*
*********************************************************************************************************
* Name:                                    BuzzerOff 
* 
* Description: Turn Off Buzzer controlled by P4.14 i/o on the interface board.   
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void BuzzerOff(void)
{
	FIO4CLR = BIT14;
}//BuzzerOff

/*
*********************************************************************************************************
* Name:                                    DistanceDetectorOn 
* 
* Description: Turn On Distance Detector    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void DistanceDetectorOn(void)
{
	FIO4SET = BIT13;
}//DistanceDetectorOn(void)

/*
*********************************************************************************************************
* Name:                                    DistanceDetectorOff 
* 
* Description: Turn Off Distance Detector Off   
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void DistanceDetectorOff(void)
{
	FIO4CLR = BIT13;
}//DistanceDetectorOff(void)

/*
*********************************************************************************************************
* Name:                                    uPBoardOn 
* 
* Description: Turn On uP board   
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void uPBoardOn()
{
	
	FIO4SET = BIT12;
}//uPBoardOn(void)

/*
*********************************************************************************************************
* Name:                                     uPBoardOff 
* 
* Description: Turn Off uP board   
*
* Arguments:   PowerOffReason - constant which defines reason why power off is requested 
* 			   This reason is preserved in battery backup static RAM of RTC
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void  uPBoardOff(WORD PowerOffReason)
{
	EXCEPTION_REASON=NONE_EXCEPTION;//this power of is because uPBoardOff() call not because of any exception
	POWER_OFF_REASON=PowerOffReason;
	FIO4CLR = BIT12;
}// uPBoardOff

/*
*********************************************************************************************************
* Name:                                    HeadServoOn 
* 
* Description: Turn On Head servo    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void HeadServoOn(void)
{
	FIO4SET = BIT11;
}//HeadServoOn(void)

/*
*********************************************************************************************************
* Name:                                     HeadServoOff 
* 
* Description: Turn Off head servo  
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void  HeadServoOff(void)
{
	FIO4CLR = BIT11;
}// HeadServoOff

/*
*********************************************************************************************************
* Name:                                    ArmServoOn 
* 
* Description: Turn On Arm servo   
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void ArmServoOn(void)
{
	FIO4SET = BIT10;
}//ArmServoOn(void)

/*
*********************************************************************************************************
* Name:                                     ArmServoOff 
* 
* Description: Turn Off arm servo  
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void  ArmServoOff(void)
{
	FIO4CLR = BIT10;
}// ArmServoOff

/*
*********************************************************************************************************
* Name:                                    UltrasonicTrigOn 
* 
* Description: Turn on P3.24 pin connected to Ultrasonic Trigering Pin 
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void UltrasonicTrigOn(void)
{
	FIO3SET = BIT24;	
}//UltrasonicTrigOn

/*
*********************************************************************************************************
* Name:                                    UltrasonicTrigOff
* 
* Description: Turn off P3.24 pin connected to Ultrasonic Trigering Pin  
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void UltrasonicTrigOff(void)
{
	FIO3CLR = BIT24;	
}//UltrasonicTrigOff


/*
*********************************************************************************************************
* Name:                                    MotorPowerOn 
* 
* Description: Turn on switch to connect motor power supply to the L298 motor control IC.
*              LPC 2378 uP P4.31 line is used for this    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void MotorPowerOn(void)
{
	FIO4SET = BIT31;	
}//MotorPowerOn

/*
*********************************************************************************************************
* Name:                                    MotorPowerOff
* 
* Description: Turn off switch to disconnect motor power supply to the L298 motor control IC.
*              LPC 2378 uP P4.31 line is used for this    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void MotorPowerOff(void)
{
	FIO4CLR = BIT31;	
}//MotorPowerOff


/*
*********************************************************************************************************
* Name:                                    LeftMotorFastStop 
* 
* Description: Set up L298 motor control IC for left motor into FAST STOP mode
*              LPC 2378 uP P4.24 and P4.15 lines are used to control left motor    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void LeftMotorFastStop(void)
{
	FIO4CLR = BIT24|BIT15;	
}//LeftMotorFastStop

/*
*********************************************************************************************************
* Name:                                    RightMotorFastStop 
* 
* Description: Set up L298 motor control IC for right motor into FAST STOP mode
*              LPC 2378 uP P4.30 and P4.25 lines are used to control left motor    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void RightMotorFastStop(void)
{
	FIO4CLR = BIT30|BIT25;	
}//RightMotorFastStop

/*
*********************************************************************************************************
* Name:                                    LeftMotorForward 
* 
* Description: Set up L298 motor control IC for left motor into FORWARD mode
*              LPC 2378 uP P4.24 and P4.15 lines are used to control left motor    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void LeftMotorForward(void)
{
	//clear BIT24 and set BIT15
	FIO4PIN = (FIO4PIN & (~BIT24)) | BIT15;

}//LeftMotorForward

/*
*********************************************************************************************************
* Name:                                    LeftMotorReverse
* 
* Description: Set up L298 motor control IC for left motor into REVERSE mode
*              LPC 2378 uP P4.24 and P4.15 lines are used to control left motor    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void LeftMotorReverse(void)
{
	//set BIT24 and clear BIT15
	FIO4PIN = (FIO4PIN & (~BIT15)) | BIT24;
}//LeftMotorReverse

/*
*********************************************************************************************************
* Name:                                    RightMotorForward 
* 
* Description: Set up L298 motor control IC for right motor into FORWARD mode
*              LPC 2378 uP P4.30 and P4.25 lines are used to control left motor    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void RightMotorForward(void)
{
	//clear BIT30 and set BIT25
	FIO4PIN = (FIO4PIN & (~BIT30)) | BIT25;

}//RightMotorForward

/*
*********************************************************************************************************
* Name:                                    RightMotorReverse 
* 
* Description: Set up L298 motor control IC for right motor into REVERSE mode
*              LPC 2378 uP P4.30 and P4.25 lines are used to control left motor    
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
void RightMotorReverse(void)
{
	//set BIT30 and clear BIT25
	FIO4PIN = (FIO4PIN & (~BIT25)) | BIT30;

}//RightMotorReverse

/*
*********************************************************************************************************
* Name:                                    InitPort2Int 
* 
* Description: Call at beginning of highest priority task once uCOS-II is intialized to generate interrupts
*              on every falling pulse of track transoptor to count track movement according to specified
*              number of intended steps of the movement.
*       
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
*             Need to be called after InitGPIO
* *********************************************************************************************************
*/
void InitPort2Int(void)
{
	//disable all possible Port2 interrupts
	IO2_INT_EN_F = 0x00000000;
	IO2_INT_EN_R = 0x00000000;
	
	//setup VIC to generate IRQ for Port2 generated interrupt
	//IMPORTANT! PORT2 IRQ is shared with EINT3
	VICIntEnClr = BIT17;//disable PORT2 interrupt in VIC
	VICVectAddr17 = (DWORD)Port2IsrHandler; //assign address to PORT2 IRQ Handler
	VICIntEnable |= BIT17;//enable PORT2 interrupts in VIC
	
	
	LeftTrackMailbox=OSMboxCreate(NULL);//create left track empty mailbox
	if(!LeftTrackMailbox)UCOSII_RES_EXCEPTION;//when no OS resources to create mailbox rise an exception
	RightTrackMailbox=OSMboxCreate(NULL);//create right track empty mailbox
	if(!RightTrackMailbox)UCOSII_RES_EXCEPTION;//when no OS resources to create mailbox rise an exception
	
	//enable all possible Port2 interrupts on falling edge of P2.6(right motor) and P2.7(left motor)
    //	IO2_INT_EN_F|=BIT6|BIT7; interupts are enabled in RunTrackPulse 
}//InitPort2Int

/*
*********************************************************************************************************
* Name:                                    Port2IsrHandler 
* 
* Description: Port2 Interrupt Service Routine - signal corresponding track task with timestamp of an
*              falling edge interrupt because of track movement
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     
* 			Interrupting device is acknowledged and its sorce of interrupt is cleared on this level.
* 			Note that VIC of the uP is cleared on the level of OS_CPU_ExceptHndlr function.
* *********************************************************************************************************
*/
void Port2IsrHandler(void)
{
	if(IO2_INT_STAT_F&BIT6)//if right track pulse 
	{
		OSMboxPost(RightTrackMailbox,(void *)GetTimer3Ticks());//get time stamp for the pulse and signal task
		IO2_INT_CLR=BIT6;//clear right track pulse interrupt
	}//if right track pulse 
	
	if(IO2_INT_STAT_F&BIT7)//if left track pulse
	{
		OSMboxPost(LeftTrackMailbox,(void *)GetTimer3Ticks());//get time stamp for the pulse and signal task
		IO2_INT_CLR=BIT7;//clear left track pulse interrupt
	}//if left track pulse
	
}//Port2IsrHandler




