/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_monitor.cpp
* Description: Manager which monitors robot's hw and sw resources important for whole system like battery, fotoresistor etc.
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*              01-Jan-2014 - Updated to provide system status, battery status and system alive notifiers
*********************************************************************************************************
*/

#include "mng_monitor.hpp"
#include "wrp_kernel.hpp"
#include "lib_memalloc.h"
#include "hw_spi.h"
#include "ctr_f_sens.h"
#include "hw_sram.h"



//Function for handling transitions from Normal state
//returns: next stae of the FSM
BYTE cMonitorMngr::NormalState(BYTE InCurrentVoltage, BYTE InThreshold)
{
	if(InCurrentVoltage < InThreshold)
		return BATT_N_W_STATE;
	else
		return BATT_NORMAL_STATE;
}//cMonitorMngr::NormalState

//Function for handling transitions from wait state from normal to warn state
//returns: next stae of the FSM
BYTE cMonitorMngr::WaitN2WState(BYTE InCurrentVoltage, BYTE InThreshold, WORD *InCycle)
{
	if(InCurrentVoltage >= InThreshold)
	{
		*InCycle = 0;//return back to normal state so eventual debuuncing will start again
		return BATT_NORMAL_STATE;
	}
	if((*InCycle) < BATT_STATE_DEBAUNCE_CYCLES)
	{
		*InCycle+=1;//remain in the wait state during debuncing
		return BATT_N_W_STATE;
	}
	else
	{
		*InCycle = 0;//debuncing is completed prepare for the eventual new one
		return BATT_WARN_STATE;
	}
}//cMonitorMngr::WaitN2WState

//Function for handling transitions from Warn
//returns: next stae of the FSM
BYTE cMonitorMngr::WarnState(BYTE InCurrentVoltage, BYTE InNormalThreshold, BYTE InLowThreshold)
{
	if(InCurrentVoltage > InNormalThreshold )
		return BATT_W_N_STATE;
	if (InCurrentVoltage < InLowThreshold)
		return BATT_W_L_STATE;
	return BATT_WARN_STATE; //voltage correspond to warn
}//cMonitorMngr::WarnState

//Function for handling transitions from wait state from warn to low state
//returns: next stae of the FSM
BYTE cMonitorMngr::WaitW2LState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle)
{
	if(InCurrentVoltage >= InThreshold)
	{
		*InCycle = 0;//return back to warn state so eventual debuuncing will start again
		return BATT_WARN_STATE;
	}
	if((*InCycle) < BATT_STATE_DEBAUNCE_CYCLES)
	{
		*InCycle+=1;//remain in the wait state during debuncing
		return BATT_W_L_STATE;
	}
	else
	{
		*InCycle = 0;//debuncing is completed prepare for the eventual new one
		return BATT_LOW_STATE;
	}
}//cMonitorMngr::WaitW2LState

//Function for handling transitions from Low state
//returns: next stae of the FSM
BYTE cMonitorMngr::LowState(BYTE InCurrentVoltage, BYTE InThreshold)
{
	if(InCurrentVoltage > InThreshold)
		return BATT_L_W_STATE;
	else
		return BATT_LOW_STATE;			
}//cMonitorMngr::LowState

//Function for handling transitions from wait state from low to warn state
//returns: next stae of the FSM
BYTE cMonitorMngr::WaitL2WState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle)
{
	if(InCurrentVoltage <= InThreshold)
	{
		*InCycle = 0;//return back to low state so eventual debuuncing will start again
		return BATT_LOW_STATE;
	}
	if((*InCycle) < BATT_STATE_DEBAUNCE_CYCLES)
	{
		*InCycle+=1;//remain in the wait state during debuncing
		return BATT_L_W_STATE;
	}
	else
	{
		*InCycle = 0;//debuncing is completed prepare for the eventual new one
		return BATT_WARN_STATE;
	}
}//cMonitorMngr::WaitL2WState

//Function for handling transitions from wait state from warn to normal state
//returns: next stae of the FSM
BYTE cMonitorMngr::WaitW2NState(BYTE InCurrentVoltage, BYTE InThreshold,WORD *InCycle)
{
	if(InCurrentVoltage <= InThreshold)
	{
		*InCycle = 0;//return back to warn state so eventual debuuncing will start again
		return BATT_WARN_STATE;
	}
	if((*InCycle) < BATT_STATE_DEBAUNCE_CYCLES)
	{
		*InCycle+=1;//remain in the wait state during debuncing
		return BATT_W_N_STATE;
	}
	else
	{
		*InCycle = 0;//debuncing is completed prepare for the eventual new one
		return BATT_NORMAL_STATE;
	}
}//cMonitorMngr::WaitW2NState

//FSM to determine main board power state
void cMonitorMngr::PwrBoardFSM(void)
{
	switch(mPwrBoardState)
	{
	case BOARD_BATT_NORMAL:
		mPwrBoardState=NormalState(mBatteryStatus.mMainSupplyVoltage,BOARD_N_W_THRESHOLD);//process normal state
		break;
	case BOARD_BATT_N_W:
		mPwrBoardState=WaitN2WState(mBatteryStatus.mMainSupplyVoltage,BOARD_N_W_THRESHOLD,&mPwrBoardCycle);//process normal to warn state
		break;
	case BOARD_BATT_W_N:
		mPwrBoardState=WaitW2NState(mBatteryStatus.mMainSupplyVoltage,BOARD_W_N_THRESHOLD,&mPwrBoardCycle);//proces warn to normal state
		break;
	case BOARD_BATT_WARN:
		mPwrBoardState=WarnState(mBatteryStatus.mMainSupplyVoltage,BOARD_W_N_THRESHOLD, BOARD_W_L_THRESHOLD);//process warn state
		break;
	case BOARD_BATT_W_L:
		mPwrBoardState=WaitW2LState(mBatteryStatus.mMainSupplyVoltage,BOARD_W_L_THRESHOLD,&mPwrBoardCycle);//process warn to low state
		break;
	case BOARD_BATT_L_W:
		mPwrBoardState=WaitL2WState(mBatteryStatus.mMainSupplyVoltage,BOARD_L_W_THRESHOLD,&mPwrBoardCycle);//process low to warn state
		break;
	case BOARD_BATT_LOW:
		mPwrBoardState=LowState(mBatteryStatus.mMainSupplyVoltage,BOARD_L_W_THRESHOLD);//process low state
		break;
	default://do nothing when unclear state
		break;
	}//switch	
}//cMonitorMngr::PwrBoardFSM

//set status of the main supply
void cMonitorMngr::SetPwrBoardStatus(void)
{
	switch(mPwrBoardState)
	{
	case BOARD_BATT_NORMAL:
	case BOARD_BATT_N_W:
		mBatteryStatus.mMainSupplyState=BOARD_BATT_NORMAL;
		break;
	case BOARD_BATT_WARN:
	case BOARD_BATT_W_L:
		mBatteryStatus.mMainSupplyState=BOARD_BATT_WARN;
		break;
	case BOARD_BATT_LOW:
	case BOARD_BATT_L_W:
		mBatteryStatus.mMainSupplyState=BOARD_BATT_LOW;
		if(mBatteryStatus.mMainSupplyVoltage < BOARD_EMPTY_THRESHOLD)//corect LOW to EMPTY if not enough power
			mBatteryStatus.mMainSupplyState=BOARD_BATT_EMPTY;
		break;
	case BOARD_BATT_W_N:
		mBatteryStatus.mMainSupplyState=BOARD_BATT_WARN;
		break;
	}//switch(mPwrBoardState)
}//cMonitorMngr::SetPwrBoardStatus

//FSM to determine track motor power state
void cMonitorMngr::PwrTrackFSM(void)
{
	switch(mPwrTrackState)
	{
	case TRACK_BATT_NORMAL:
		mPwrTrackState=NormalState(mBatteryStatus.mMotorSupplyVoltage,TRACK_N_W_THRESHOLD);//process normal state
		break;
	case TRACK_BATT_N_W:
		mPwrTrackState=WaitN2WState(mBatteryStatus.mMotorSupplyVoltage,TRACK_N_W_THRESHOLD,&mPwrTrackCycle);//process normal to warn state
		break;
	case TRACK_BATT_W_N:
		mPwrTrackState=WaitW2NState(mBatteryStatus.mMotorSupplyVoltage,TRACK_W_N_THRESHOLD,&mPwrTrackCycle);//proces warn to normal state
		break;
	case TRACK_BATT_WARN:
		mPwrTrackState=WarnState(mBatteryStatus.mMotorSupplyVoltage, TRACK_W_N_THRESHOLD,TRACK_W_L_THRESHOLD);//process warn state
		break;
	case TRACK_BATT_W_L:
		mPwrTrackState=WaitW2LState(mBatteryStatus.mMotorSupplyVoltage,TRACK_W_L_THRESHOLD,&mPwrTrackCycle);//process warn to low state
		break;
	case TRACK_BATT_L_W:
		mPwrTrackState=WaitL2WState(mBatteryStatus.mMotorSupplyVoltage,TRACK_L_W_THRESHOLD,&mPwrTrackCycle);//process low to warn state
		break;
	case TRACK_BATT_LOW:
		mPwrTrackState=LowState(mBatteryStatus.mMotorSupplyVoltage,TRACK_L_W_THRESHOLD);//process low state
		break;
	default:
		break;
	}//switch	
}//cMonitorMngr::PwrTrackFSM

//set status of the track motor power supply
void cMonitorMngr::SetPwrTrackStatus(void)
{
	switch(mPwrTrackState)
	{
	case TRACK_BATT_NORMAL:
	case TRACK_BATT_N_W:
		mBatteryStatus.mMotorSupplyState=TRACK_BATT_NORMAL;
		break;
	case TRACK_BATT_WARN:
	case TRACK_BATT_W_L:
		mBatteryStatus.mMotorSupplyState=TRACK_BATT_WARN;
		break;
	case TRACK_BATT_L_W:
	case TRACK_BATT_LOW:
		mBatteryStatus.mMotorSupplyState=TRACK_BATT_LOW;
		if(mBatteryStatus.mMotorSupplyVoltage < TRACK_EMPTY_THRESHOLD)
			mBatteryStatus.mMotorSupplyState=TRACK_BATT_EMPTY;
		break;
	case TRACK_BATT_W_N:
		mBatteryStatus.mMotorSupplyState=TRACK_BATT_WARN;
	default:
		break;
	}//switch	
}//cMonitorMngr::SetPwrTrackStatus

//FSM to determine servo motor power state
void cMonitorMngr::PwrServoFSM(void)
{
	switch(mPwrServoState)
	{
	case SERVO_BATT_NORMAL:
		mPwrServoState=NormalState(mBatteryStatus.mServoSupplyVoltage,SERVO_N_W_THRESHOLD);//process normal state
		break;
	case SERVO_BATT_N_W:
		mPwrServoState=WaitN2WState(mBatteryStatus.mServoSupplyVoltage,SERVO_N_W_THRESHOLD,&mPwrServoCycle);//process normal to warn state
		break;
	case SERVO_BATT_W_N:
		mPwrServoState=WaitW2NState(mBatteryStatus.mServoSupplyVoltage,SERVO_W_N_THRESHOLD,&mPwrServoCycle);//proces warn to normal state
		break;
	case SERVO_BATT_WARN:
		mPwrServoState=WarnState(mBatteryStatus.mServoSupplyVoltage,SERVO_W_N_THRESHOLD,SERVO_W_L_THRESHOLD);//process warn state
		break;
	case SERVO_BATT_W_L:
		mPwrServoState=WaitW2LState(mBatteryStatus.mServoSupplyVoltage, SERVO_W_L_THRESHOLD,&mPwrServoCycle);//process warn to low state
		break;
	case SERVO_BATT_L_W:
		mPwrServoState=WaitL2WState(mBatteryStatus.mServoSupplyVoltage,SERVO_L_W_THRESHOLD,&mPwrServoCycle);//process low to warn state
		break;
	case SERVO_BATT_LOW:
		mPwrServoState=LowState(mBatteryStatus.mServoSupplyVoltage,SERVO_L_W_THRESHOLD);//process low state
		break;
	default:
		break;
	}//switch
}//cMonitorMngr::PwrServoFSM

//set status of the servo motor power
void cMonitorMngr::SetPwrServoStatus(void)
{
	switch(mPwrServoState)
	{
	case SERVO_BATT_NORMAL:
	case SERVO_BATT_N_W:
		mBatteryStatus.mServoSupplyState=SERVO_BATT_NORMAL;
		break;
	case SERVO_BATT_WARN:
	case SERVO_BATT_W_L:
		mBatteryStatus.mServoSupplyState=SERVO_BATT_WARN;
		break;
	case SERVO_BATT_L_W:
	case SERVO_BATT_LOW:
		mBatteryStatus.mServoSupplyState=SERVO_BATT_LOW;
		if(mBatteryStatus.mServoSupplyVoltage < SERVO_EMPTY_THRESHOLD)
			mBatteryStatus.mServoSupplyState=SERVO_BATT_EMPTY;
		break;
	case SERVO_BATT_W_N:
		mBatteryStatus.mServoSupplyState=SERVO_BATT_WARN;
		break;
	default:
		break;
	}//switch
}//cMonitorMngr::SetPwrServoStatus


//reads voltages of batteries 
//every MONITORING_FREQUENCY_IN_OS_TICKS	
void cMonitorMngr::CheckBatteryVoltage(void)
{
	DWORD AdcData;//temporary to store data taken out of ADC converter
	
	GetAdcAccess();//guarantee exclusive access to ADC for this task
	
	AdcData=static_cast<DWORD>(GetAdcConversion(ADC_MAIN_SUPPLY));//get current ADC reading for Main Power Supply
	mBatteryStatus.mMainSupplyVoltage=static_cast<BYTE>((AdcData*63)/3404);//convert ADC reading to Voltage*10 i.e 47<->4.7 [V]
       
	AdcData=static_cast<DWORD>(GetAdcConversion(ADC_MOTOR_SUPPLY));//get current ADC reading for Motor Supply
	mBatteryStatus.mMotorSupplyVoltage=static_cast<BYTE>((AdcData*50)/1362);//convert ADC reading to Voltage*10 i.e 97<->9.7 [V]
	
	AdcData=static_cast<DWORD>(GetAdcConversion(ADC_SERVO));//get current ADC reading for Servo Voltage
	mBatteryStatus.mServoSupplyVoltage=static_cast<BYTE>((AdcData*60)/4091);//convert ADC reading to Voltage*10 i.e 97<->9.7 [V]
	
	ReleaseAdcAccess();//release exclusive access to ADC
	
	//check, calculate lowest registered values
	if(mBatteryStatus.mMainSupplyVoltage < mBatteryStatus.mMainSupplyMinVoltage)mBatteryStatus.mMainSupplyMinVoltage=mBatteryStatus.mMainSupplyVoltage;
	if(mBatteryStatus.mMotorSupplyVoltage < mBatteryStatus.mMotorSupplyMinVoltage)mBatteryStatus.mMotorSupplyMinVoltage=mBatteryStatus.mMotorSupplyVoltage;
	if(mBatteryStatus.mServoSupplyVoltage < mBatteryStatus.mServoSupplyMinVoltage)mBatteryStatus.mServoSupplyMinVoltage=mBatteryStatus.mServoSupplyVoltage;
    
	PwrBoardFSM();//execute FSM to determine main board power state
    SetPwrBoardStatus();//set status of the main supply
    
    PwrTrackFSM();//execute FSM to determine track motor power state
    SetPwrTrackStatus();//set status of the track motor power supply
    
    PwrServoFSM();//execute FSM to determine servo motor power state
    SetPwrServoStatus();//set status of the servo motor power
		
}//cMonitorMngr::CheckBatteryVoltage

//function used to issue periodic sys alive message
void cMonitorMngr::MonitorSystemAlive(void)
{
	cSmartPtr<cTypeNotifier<sSysAliveEvt> > pNotifier = new cTypeNotifier<sSysAliveEvt>(EVT_SYS_ALIVE,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mTimeStamp=Kernel.Ticks();//get current OS time stamp and assign to Notifier data
	(pNotifier->GetData()).mLastPowerOffReason=ReadLastPowerOffReason(); 
	(pNotifier->GetData()).mLastExceptionReason=ReadLastExceptionReason();
	(pNotifier->GetData()).mRAMContentPreserved=WasBatteryRAMPreserved();//TRUE when preserved FALSE othervise
	(pNotifier->GetData()).mLastResetReason=ReadLastResetReason();
	(pNotifier->GetData()).mWalleProgramToExecute=GetWalleProgramToExecute();//currently executed WALL-E program
	
	Post(pNotifier);//post notifier to all subscribers
}//cMonitorMngr::MonitorSystemAlive

//function used to issue periodic battery status message
void cMonitorMngr::MonitorBattery(void)
{
	//create empty battery status notifier
	cSmartPtr<cTypeNotifier<sBatteryStatus> > pNotifier = new cTypeNotifier<sBatteryStatus>(EVT_BATTERY,GetThreadId(),NT_HND_NORMAL_PRT);

	//copy most up to date battery data to notifier
	(pNotifier->GetData()).mMainSupplyVoltage = mBatteryStatus.mMainSupplyVoltage;
	(pNotifier->GetData()).mMainSupplyMinVoltage = mBatteryStatus.mMainSupplyMinVoltage;
	(pNotifier->GetData()).mMainSupplyState=mBatteryStatus.mMainSupplyState;
	
	(pNotifier->GetData()).mMotorSupplyVoltage = mBatteryStatus.mMotorSupplyVoltage;
	(pNotifier->GetData()).mMotorSupplyMinVoltage = mBatteryStatus.mMotorSupplyMinVoltage;
	(pNotifier->GetData()).mMotorSupplyState=mBatteryStatus.mMotorSupplyState;
	
	
	(pNotifier->GetData()).mServoSupplyVoltage = mBatteryStatus.mServoSupplyVoltage;
	(pNotifier->GetData()).mServoSupplyMinVoltage = mBatteryStatus.mServoSupplyMinVoltage;
	(pNotifier->GetData()).mServoSupplyState=mBatteryStatus.mServoSupplyState;
	Post(pNotifier);//post battery status notifier to all subscribers	
}//cMonitorMngr::MonitorBattery

//function used to issue periodic system resources status
void cMonitorMngr::MonitorSystemResources(void)
{
	//create empty system status notifier
	cSmartPtr<cTypeNotifier<sSysResourcesStatus> > pNotifier = new cTypeNotifier<sSysResourcesStatus>(EVT_SYS_RES,GetThreadId(),NT_HND_NORMAL_PRT);
	//update notifier with most up to date data
	(pNotifier->GetData()).mSysHeapMemLeft=freeleft();//get current amount of free bytes on system heap
	(pNotifier->GetData()).mSysHeapMinLeft=minfreeleft();//get smallest amount of free bytes on system heap so far
	
	//update total number of dispatch erros notified so far
	(pNotifier->GetData()).mTotalDispatchErrorCounter=Kernel.Dispatcher.GetTotalDispatchErrorCounter();
	Post(pNotifier);//post system status notifier to all subscribers
	
}//cMonitorMngr::MonitorSystemResources

//function ussed to issue periodic EVT_DAY_NIGHT event which provides day or night indication for the world
//outside Wall-e based on photo tail reading
void cMonitorMngr::MonitorDayNight(void)
{
	WORD RawFotoResistor;//temporary to store data taken out of ADC converter
		
	//create empty system status notifier
	cSmartPtr<cTypeNotifier<sDayStateData> > pNotifier = new cTypeNotifier<sDayStateData>(EVT_DAY_NIGHT,GetThreadId(),NT_HND_NORMAL_PRT);
	//update notifier with most up to date data
	(pNotifier->GetData()).mDayState=CheckTailDayState(&RawFotoResistor);
	(pNotifier->GetData()).mRawFotoResistorVolt=RawFotoResistor;//store A/D reading of fotoresistor voltage
	
	Post(pNotifier);//post system status notifier to all subscribers	
}//cMonitorMngr::MonitorDayNight

void cMonitorMngr::Run(void)
{
	mPeriodCounter=0; //initialize period counter to 0 to avoid random value
	
	//initialize minimum battery values to max possible values when manager starts
	//they will be next updated to the correct values by CheckBatteryVoltage
	mBatteryStatus.mMainSupplyMinVoltage = 0xFF;
	mBatteryStatus.mMotorSupplyMinVoltage = 0xFF;
	mBatteryStatus.mServoSupplyMinVoltage = 0xFF;
	
	//initialise FSM state for batteries state monitoring
	mPwrBoardState=BOARD_BATT_NORMAL;
	mPwrTrackState=TRACK_BATT_NORMAL;
	mPwrServoState=SERVO_BATT_NORMAL;
	
	//initialise FSMs cyclec counters
	mPwrBoardCycle=0;
	mPwrTrackCycle=0;
	mPwrServoCycle=0;
	
	for(;;)
	{
		Delay(MONITORING_FREQUENCY_IN_OS_TICKS);//execute every MONITORING_FREQUENCY_IN_OS_TICKS ticks
		mPeriodCounter++;//increase period counter every MONITORING_FREQUENCY_IN_OS_TICKS
		CheckBatteryVoltage();//update mBatteryStatus with battery reading data
		//schedule monitoring functions as defined by specific periods
		//when mod gives 0 schedule function,otherwise skip the execution
		if(!(mPeriodCounter%MONITOR_SYS_ALIVE))MonitorSystemAlive();//when time to generate system allive message
		if(!(mPeriodCounter%MONITOR_BATTERY))MonitorBattery();//when time expire issue battery data
		if(!(mPeriodCounter%MONITOR_SYS_RESOURCES))MonitorSystemResources();//when time to monitor system resources
		if(!(mPeriodCounter%MONITOR_DAY_NIGHT))MonitorDayNight();//check if there is day or night outside Wall-e
	}//for
}//cMonitorMngr::Run()
