/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_ctx.hpp
* Description: Keeps all internal and external context of Wall-e operations
* Author:      Bogdan Kowalczyk
* Date:        25-Jan-2015
* History:
* 25-Jan-2015 - Initial version created
*********************************************************************************************************
*/

#ifndef MNG_CTX_HPP_
#define MNG_CTX_HPP_

#include "mng.hpp"
#include "wrp_kernel.hpp"
#include "lib_time.h"

#define  CTX_PUBLISHER_SEND_Q_SIZE 	5
#define  CTX_SUBSCRIBER_REC_Q_SIZE 	5
#define  CTX_THREAD_STACK_SIZE		128
#define  CTX_THREAD_PRIORITY   		21

//number of OS_TICKS delay when context update flag is checked
#define CTX_CONTEXT_UPDATE_DELAY	10


class cBrainMngr;
class cExeMngr;

class cCtxMngr:public cMngBasePublisherSubscriber<CTX_PUBLISHER_SEND_Q_SIZE,CTX_SUBSCRIBER_REC_Q_SIZE,CTX_THREAD_STACK_SIZE,CTX_THREAD_PRIORITY>
{
private:
	  cBrainMngr* pBrainMngr;//pointer to brain manager this context is assigned into
	  cExeMngr* pExeMngr;//pointer to execution manager which is tied into the context manager within the brain
	  
	  cMutex mCtxMutext; //used to protect context access
	  
	  
	  //data members for TIME context
	  volatile BYTE mTimeContextUpdated;//set to TRUE after first context update otherwise FALSE
	  tmElements_t mTimeDateNow;//used when time and date are processed
	  volatile time_t mTimeNow; //time as seconds since Jan 1 1970 (this was thursday) coresponding to mTimeDate 
	  
	  //data members for BATTERY context
	  volatile BYTE mBatteryContextUpdated;//set TRUE after first context update otherwise FALSE
	  volatile BYTE mMainSupplyState;//normal, warn or low state of the main power supply
	  volatile BYTE mMotorSupplyState;//normal, warn or low state of the motor supply
	  volatile BYTE mServoSupplyState;//normal, warn or low state of the servo supply
	  
	  //data members for SYSTEM STATE context
	  volatile BYTE mSysStateContextUpdated;//set TRUE after first context update otherwise FALSE
	  volatile BYTE mRAMContentPreserved;//is SRAM battery OK so we can relay on RAM data since last power OFF
	  
	  
	  void UpadateTimeContext(cSmartPtr<cNotifier>  p_smartNotifier);//update time context based on received EVT_TIME
	  void UpdateBatteryContext(cSmartPtr<cNotifier> pNotifier);//update battery state context
	  void UpdateSysStateContext(cSmartPtr<cNotifier> pNotifier);//update system state context
      //pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
      virtual void Run();
public:
	cCtxMngr(cBrainMngr* pBrain,cExeMngr* pExe )
	{
		pBrainMngr=pBrain;
		pExeMngr=pExe;
		mTimeContextUpdated=FALSE;//time context is not updated until first EVT_TIME event received
		mBatteryContextUpdated=FALSE;//battery context is not updated until EVT_BATTERY event processed first time
		mSysStateContextUpdated=FALSE;//sys context is not updated until first EVT_SYS_ALIVE event
	};
	//IMPORTANT! It waits internally until time context first time updated
	time_t GetNowTime(void);//get current system time as kept in the context
	
	//IMPORTANT! It waits internally until battery context first time updated
	BYTE GetBatteryOKState(void);//OK or NOK state of any of battery sub-systems
	
	//IMPORTANT! It waits internally until battery context first time updated
	BYTE GetSRAMBatteryOKState(void);//OK or NOK State depending if SRAM content preserved or not
	
	//IMPORTANT! It waits internally until context first time updated
	BYTE IsHealthOK(void)
	{
		if(GetBatteryOKState()==OK_STATE && GetSRAMBatteryOKState()==OK_STATE)
			return TRUE;
		else
			return FALSE;		
	};
	
	
};//cCtxMngr


#endif /*MNG_CTX_HPP_*/
