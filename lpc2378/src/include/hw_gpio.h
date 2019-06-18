/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        hw_gpio.h
* Description: Lpc 2378 uP General Purpose Input/Output initialization and control procedures 
* Author:      Bogdan Kowalczyk
* Date:        2-Aug-2008
* History:
* 2-Aug-2008 - Initial version created based on NXP target.h for NXP LPC23xx/24xx Family Microprocessors
*********************************************************************************************************
*/

#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "os_cpu.h"
#include "os_cfg.h"	
#include "os_ucos_ii.h"

//constants for POWER_OFF_REASONs and descriptive strings behind (cannot be longer than 21 chars plus /0
#define UNSET_TURN_OFF			0 //when reason of power off are undefined for example sudden batter disconnection
#define KEYPAD_TURN_OFF			1 //when turn of requested from Wall-e keypad
#define REMOTE_TURN_OFF			2 //when turn off requested from the remote terminal control
#define ALARM_TURN_OFF			3 //when turn off requested just after alarm wake up setup
#define HEALTH_STATE_TURN_OFF	4 //turn of because of weak health state (for example low battery)	
#define EXCEPTION_TURN_OFF		5 //when turn off after an exception requested and reported befor the power off

#define UNSET_TURN_OFF_STR			"UNSET" 
#define KEYPAD_TURN_OFF_STR			"KEYPAD"
#define REMOTE_TURN_OFF_STR			"REMOTE"
#define ALARM_TURN_OFF_STR			"ALARM"
#define HEALTH_STATE_TURN_OFF_STR	"HEALTH"
#define EXCEPTION_TURN_OFF_STR		"EXCEPT"
#define UNDEF_POWER_OFF_ERR_STR		"ERRUNDEF"
	
	
//communication between Port2Isr and track control task is throught a mailbox.
//mailbox is notified with the timestamp of trigerred left and right trac falling pulse
extern	OS_EVENT *LeftTrackMailbox;//provide access to left track mailbox
extern	OS_EVENT *RightTrackMailbox;//provide access to right track mailbox
	

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
extern void InitGPIO(void);

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
extern void InitPort2Int(void);

/*
*********************************************************************************************************
* Name:                                    Port2IsrHandler 
* 
* Description: Port2 Interrupt Service Routine - makes tracks to move specified number of track pulses
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
extern void Port2IsrHandler(void);


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
extern DWORD GetBut1(void);

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
extern DWORD GetBut2(void);

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
extern DWORD GetJ1Up(void);

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
extern DWORD GetJ1Down(void);

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
extern DWORD GetJ1Left(void);

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
extern DWORD GetJ1Right(void);

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
extern DWORD GetJ1Center(void);

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
extern void LedOn(void);

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
extern void LedOff(void);

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
extern void  TransoptorOn(void);


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
extern void TransoptorOff(void);

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
extern void BuzzerOn(void);

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
extern void BuzzerOff(void);

/*
*********************************************************************************************************
* Name:                                    DistanceDetectorOn 
* 
* Description: Turn On Distance Detector Off   
*
* Arguments:   none
*
* Returns:     none
*
* Note(s):     Requires InitGPIO to be used before first call of the function 
* 
* *********************************************************************************************************
*/
extern void DistanceDetectorOn(void);


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
extern void DistanceDetectorOff(void);

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
extern void uPBoardOn(void);


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
extern void  uPBoardOff(WORD PowerOffReason);

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
extern void HeadServoOn(void);


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
extern void  HeadServoOff(void);


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
extern void ArmServoOn(void);


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
extern void  ArmServoOff(void);

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
extern void UltrasonicTrigOn(void);


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
extern void UltrasonicTrigOff(void);

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
void MotorPowerOn(void);


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
void MotorPowerOff(void);



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
void LeftMotorFastStop(void);


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
void RightMotorFastStop(void);


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
void LeftMotorForward(void);



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
void LeftMotorReverse(void);


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
void RightMotorForward(void);

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
void RightMotorReverse(void);

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*GPIO_H_*/
