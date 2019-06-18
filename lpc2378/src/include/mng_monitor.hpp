/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_monitor.cpp
* Description: Manager which monitors robot's hardware inputs important for whole system like battery, fotoresistor etc.
*              as well as system resources like heap or stack size etc.
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/
#ifndef MNG_MONITOR_HPP_
#define MNG_MONITOR_HPP_

#include "mng.hpp"

//manager basic parameters
#define  MONITOR_PUBLISHER_SEND_Q_SIZE 	5      	//transmit queue size
#define  MONITOR_SUBSCRIBER_REC_Q_SIZE 	5		//receive queue size
#define  MONITOR_THREAD_STACK_SIZE		128		//manager stack
#define  MONITOR_THREAD_PRIORITY   		26		//manager priority

#define  MONITORING_FREQUENCY_IN_OS_TICKS	10		//frequency in OS_TICK (for example 10 x os tck) for which monitor task is executed

//define frequencies at which every monitoring function is executed

#define MONITOR_SYS_ALIVE      5  //frequency for SYS ALIVE function - every MONITOR_SYS_ALIVE*MONITORING_FREQUENCY_IN_OS_TICKS OS TICKS
#define MONITOR_BATTERY        10 //frequency for battery monitoring - every MONITOR_BUTTERY*MONITORING_FREQUENCY_IN_OS_TICKS
#define MONITOR_SYS_RESOURCES  20 //frequency for system resources monitoring
#define MONITOR_DAY_NIGHT	   5  //frequency for day or night state check for outside Wall-e

//battery state debouncing time for battery FSMs
//time in ms is equeal BATT_STATE_DEBAUNCE_CYCLES*MONITORING_FREQUENCY_IN_OS_TICKS*OS_TIKK
//for example BATT_STATE_DEBAUNCE_CYCLES=10 and MONITORING_FREQUENCY_IN_OS_TICKS=10 gives 100 OS_TICK which is 1000ms=1s 
#define BATT_STATE_DEBAUNCE_CYCLES		10

//comman states names IDs
//IMPORTANT! They must correspond in value exactly to particular battery type states for example:
//BATT_NORMAL_STATE=BOARD_BATT_NORMAL=TRACK_BATT_NORMAL=SERVO_BATT_NORMAL
//otherwise subsequent states machines will not work accordingly as same FTM is used for every battery type
#define BATT_NORMAL_STATE	0x01
#define BATT_WARN_STATE		0x02
#define BATT_LOW_STATE		0x03
#define BATT_N_W_STATE		0x04
#define BATT_W_L_STATE		0x05
#define BATT_L_W_STATE		0x06
#define BATT_W_N_STATE		0x07



//board battery thresholds (in volts *10)
#define BOARD_N_W_THRESHOLD 	56
#define BOARD_W_L_THRESHOLD 	55
#define BOARD_L_W_THRESHOLD 	58
#define BOARD_W_N_THRESHOLD 	59
#define BOARD_EMPTY_THRESHOLD 	53

//track battery thresholds (in volts *10)
#define TRACK_N_W_THRESHOLD 112
#define TRACK_W_L_THRESHOLD 111
#define TRACK_L_W_THRESHOLD 114
#define TRACK_W_N_THRESHOLD 115
#define TRACK_EMPTY_THRESHOLD 105

//servo battery thresholds (in volts *10)
#define SERVO_N_W_THRESHOLD 45
#define SERVO_W_L_THRESHOLD 44
#define SERVO_L_W_THRESHOLD 47
#define SERVO_W_N_THRESHOLD 48
#define SERVO_EMPTY_THRESHOLD 42

class cMonitorMngr:public cMngBasePublisherSubscriber<MONITOR_PUBLISHER_SEND_Q_SIZE,MONITOR_SUBSCRIBER_REC_Q_SIZE,MONITOR_THREAD_STACK_SIZE,MONITOR_THREAD_PRIORITY>
{
private:
	WORD mPeriodCounter;//used to schedule monitoring actions
	sBatteryStatus mBatteryStatus;//keep current status of battery levels
	BYTE mPwrBoardState;//holds current state for main uP board power FSM
	BYTE mPwrTrackState;//holds state for track motors power FSM
	BYTE mPwrServoState;//holds current state for servo power FSM
	//storage to count particular wait states calls of particular FSM
	WORD mPwrBoardCycle;//holds counted cycles of particular FSM call
	WORD mPwrTrackCycle;//holds counted cycles of particular FSM call
	WORD mPwrServoCycle;//holds counted cycles of particular FSM call
	//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
    virtual void Run();
    void CheckBatteryVoltage(void);//reads voltages of batteries every MONITORING_FREQUENCY_IN_OS_TICKS
    void MonitorSystemAlive(void);//function used to issue periodic sys alive message
    void MonitorBattery(void);//function used to issue periodic battery status message
    void MonitorSystemResources(void);//function used to issue periodic system resources status
    void MonitorDayNight(void);//function used to monitor day or night state outside Wall-e
    //state machines used to determine state of particular type of power batteries
    //particular states handling routines
    BYTE NormalState(BYTE InCurrentVoltage, BYTE InThreshold);
    BYTE WaitN2WState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle);
    BYTE WarnState(BYTE InCurrentVoltage, BYTE InNormalThreshold, BYTE InLowThreshold);
    BYTE WaitW2LState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle);
    BYTE LowState(BYTE InCurrentVoltage, BYTE InThreshold);
    BYTE WaitL2WState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle);
    BYTE WaitW2NState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle);
    void PwrBoardFSM(void);//FSM for main board power
    void SetPwrBoardStatus(void);//set status of the main supply
    void PwrTrackFSM(void);//FSM for track motor power
    void SetPwrTrackStatus(void);//set status of the track motor power supply
    void PwrServoFSM(void);//FSM for servo motor power
    void SetPwrServoStatus(void);//set status of the servo motor power
};//cMonitorMngr


#endif /*MNG_MONITOR_HPP_*/
