/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_icon.cpp
* Description: Class to handle bit map type widget for Walle Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        12-Nov-2017
* Note:
* History:
*              12-Nov-2017 - Initial version created
*********************************************************************************************************
*/

#include "lib_g_icon.hpp"
#include "hw_sram.h" //to get access to program type constants
#include "hw_uart.h" //to get access to terminal messages
#include "lib_g_bitmap.h"

//setup bmp icon for specified position but do not display it
cIconCtrl::cIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY)
{
	GetLeftCorner().mX=InX;
	GetLeftCorner().mY=InY;
	SetIconBmp(InBmp);	
}//cIconCtrl::cIconCtrl

//setup new bitemap for the icon
void cIconCtrl::SetIconBmp(const BYTE* InBmp)
{
	//when there is any icon already drawn
	if(GetDrawn()==TRUE)
	{
		Clear();//clear old icon from display	
	}
	mBmp=InBmp;//assign new bitmap to the icon
	SetReDraw();//and mark icon changed and should be redrawn
}//cIconCtrl::SetIconBmp

//draw icon bitmap on the screen if anything modified	
void cIconCtrl::Draw()
{
	if(GetReDraw()==FALSE)//do not draw/re-draw control when not needed i.e when there is not aby change since last draw
			return;
	if(mBmp)//if there is bitmap assigned to icon draw it
		LCDWriteBmp(GetLeftCorner().mX, GetLeftCorner().mY, mBmp);
	SetDrawn();//mark that icon is displayed already
	ClrReDraw();//once drawn mark as not require to be redrawn until another change
}//cIconCtrl::::Draw

//clear icon form the screen
void cIconCtrl::Clear()
{
	BYTE x0,y0,x1,y1;//coordinates of space to be clared
	
	x0=GetLeftCorner().mX;
	y0=GetLeftCorner().mY;
	x1=x0+GetHight();
	y1=y0+GetWidth();
	LCDSetRect(x0,y0,x1,y1,1,GetBckColor());//fill space with bacground color
	ClrDrawn();//mark that text is not displayed any longer
}//cIconCtrl::Clear

cHeartIconCtrl::cHeartIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
	mHeartTogleFlag=FALSE; //setup initial value for togle flag to togle between heart icon on and off
}//cHeartIconCtrl::cHeartIconCtrl

//process notifier for Heart Icon control
void cHeartIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
								
	if(pNotifier->GetNotifierId()!=EVT_SYS_ALIVE) return;//when it is not EVT_SYS_ALIVE notifier do nothing
	
	if(mHeartTogleFlag==FALSE)//when heart is not displayed
	{
		mHeartTogleFlag=TRUE;//mark heart as displayed
		SetIconBmp(HeartBmp);//set heart icon to be drawn
	}
	else //when heart is displayed
	{
		mHeartTogleFlag=FALSE;//mark heart as not displayed
		Clear();//remove icon until next EVT_SYS_ALIVE
	}
}//cHeartIconCtrl::ProcessNotifier

//construct uP Power Icon
cuPPowerIconCtrl::cuPPowerIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
	//setup initial state depending on the bitmap used to initialize icon
	if(InBmp==BatteryHighBmp)
		mLastDepictedBatteryState=BOARD_BATT_NORMAL;
	else if (InBmp==BatteryMidBmp)
		mLastDepictedBatteryState=BOARD_BATT_WARN;
	else if (InBmp==BatteryLowBmp)
		mLastDepictedBatteryState=BOARD_BATT_LOW;
	else
		mLastDepictedBatteryState=BOARD_BATT_EMPTY;

}//cuPPowerIconCtrl::cuPPowerIconCtrl

//process notifier for uP Power Icon
void cuPPowerIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_BATTERY) return;//when it is not battery state notifier do nothing

	//do nothing if there is not any battery state change since it was last depicted
	if(mLastDepictedBatteryState==(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyState)) return;

	switch(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyState)
	{
	case BOARD_BATT_NORMAL:
		SetIconBmp(BatteryHighBmp);
		mLastDepictedBatteryState=BOARD_BATT_NORMAL;
		break;
	case BOARD_BATT_WARN:
		SetIconBmp(BatteryMidBmp);
		mLastDepictedBatteryState=BOARD_BATT_WARN;
		break;
	case BOARD_BATT_LOW:
		SetIconBmp(BatteryLowBmp);
		mLastDepictedBatteryState=BOARD_BATT_LOW;
		break;
	case BOARD_BATT_EMPTY:
		SetIconBmp(BatteryEmptyBmp);
		mLastDepictedBatteryState=BOARD_BATT_EMPTY;
		break;
	default://should never happen
		LCDClearScreen();//clear the screen	
		LCDPutStr("ERR: LibIcon: uP Pwr",20,1,SMALL,WHITE,BLACK);
		Uart0Message("ERR: cuPPowerIconCtrl::ProcessNotifier: Wrong battery state: ",static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyState);			
		NOT_ALLOWED_VALUE;//generate an exception for not allwed Wall-e program selected
	}//switch

}//cuPPowerIconCtrl::ProcessNotifier

//construct Track Power Icon control
cTrackPowerIconCtrl::cTrackPowerIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
	//setup initial state depending on the bitmap used to initialize icon
	if(InBmp==BatteryHighBmp)
		mLastDepictedBatteryState=TRACK_BATT_NORMAL;
	else if (InBmp==BatteryMidBmp)
		mLastDepictedBatteryState=TRACK_BATT_WARN;
	else if (InBmp==BatteryLowBmp)
		mLastDepictedBatteryState=TRACK_BATT_LOW;
	else
		mLastDepictedBatteryState=TRACK_BATT_EMPTY;
}//cTrackPowerIconCtrl::cTrackPowerIconCtrl

//process notifier for Track Power Icon control
void cTrackPowerIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_BATTERY) return;//when it is not battery state notifier do nothing
	
	//do nothing if there is not any battery state change since it was last depicted
	if(mLastDepictedBatteryState==(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyState)) return;

	switch(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyState)
	{
	case TRACK_BATT_NORMAL:
		SetIconBmp(BatteryHighBmp);
		mLastDepictedBatteryState=TRACK_BATT_NORMAL;
		break;
	case TRACK_BATT_WARN:
		SetIconBmp(BatteryMidBmp);
		mLastDepictedBatteryState=TRACK_BATT_WARN;
		break;
	case TRACK_BATT_LOW:
		SetIconBmp(BatteryLowBmp);
		mLastDepictedBatteryState=TRACK_BATT_LOW;
		break;
	case TRACK_BATT_EMPTY:
		SetIconBmp(BatteryEmptyBmp);
		mLastDepictedBatteryState=TRACK_BATT_EMPTY;
		break;
	default://should never happen
		LCDClearScreen();//clear the screen	
		LCDPutStr("ERR: LibIcon: Trk Pwr",20,1,SMALL,WHITE,BLACK);
		Uart0Message("ERR: cTrackPowerIconCtrl::ProcessNotifier: Wrong battery state: ",static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyState);			
		NOT_ALLOWED_VALUE;//generate an exception for not allwed Wall-e program selected
	}//switch
}//cTrackPowerIconCtrl::ProcessNotifier

cServoPowerIconCtrl::cServoPowerIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
	//setup initial state depending on the bitmap used to initialize icon
	if(InBmp==BatteryHighBmp)
		mLastDepictedBatteryState=SERVO_BATT_NORMAL;
	else if (InBmp==BatteryMidBmp)
		mLastDepictedBatteryState=SERVO_BATT_WARN;
	else if (InBmp==BatteryLowBmp)
		mLastDepictedBatteryState=SERVO_BATT_LOW;
	else
		mLastDepictedBatteryState=SERVO_BATT_EMPTY;

}//cServoPowerIconCtrl::cServoPowerIconCtrl

//process notifier for the Servo Power Icon control
void cServoPowerIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_BATTERY) return;//when it is not battery state notifier do nothing
	
	//do nothing if there is not any battery state change since it was last depicted
	if(mLastDepictedBatteryState==(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyState)) return;

	switch(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyState)
	{
	case SERVO_BATT_NORMAL:
		SetIconBmp(BatteryHighBmp);
		mLastDepictedBatteryState=SERVO_BATT_NORMAL;
		break;
	case SERVO_BATT_WARN:
		SetIconBmp(BatteryMidBmp);
		mLastDepictedBatteryState=SERVO_BATT_WARN;
		break;
	case SERVO_BATT_LOW:
		SetIconBmp(BatteryLowBmp);
		mLastDepictedBatteryState=SERVO_BATT_LOW;
		break;
	case SERVO_BATT_EMPTY:
		SetIconBmp(BatteryEmptyBmp);
		mLastDepictedBatteryState=SERVO_BATT_EMPTY;
		break;
	default://should never happen
		LCDClearScreen();//clear the screen	
		LCDPutStr("ERR: LibIcon: Srv Pwr",20,1,SMALL,WHITE,BLACK);
		Uart0Message("ERR: cServoPowerIconCtrl::ProcessNotifier: Wrong battery state: ",static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyState);			
		NOT_ALLOWED_VALUE;//generate an exception for not allwed Wall-e program selected
	}//switch	
}//cServoPowerIconCtrl::ProcessNotifier

//setup Alarm Icon control
cAlarmIconCtrl::cAlarmIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
	if(InBmp==AlarmOnBmp)
		mLastDepictedAlarmState=TRUE;
	else
		mLastDepictedAlarmState=FALSE;
}//cAlarmIconCtrl::cAlarmIconCtrl

//process notifier so proper state of the Alarm is reflected by Wall-e
void cAlarmIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_ALARM) return;//when it is not EVT_ALARM notifier do nothing
	
	//do nothing if there is not any alarm state change since it was last depicted
	if(mLastDepictedAlarmState==(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mIsAlarmSetupInRTC)) return;
	
	//when alarm is setup put Alarm ON Icon on window
	if((static_cast<sRtcData*>(pNotifier->GetDataPtr())->mIsAlarmSetupInRTC)==TRUE)
	{
		mLastDepictedAlarmState=TRUE;
		SetIconBmp(AlarmOnBmp);
	}
	else//alarm is off put of Icon on window
	{
		mLastDepictedAlarmState=FALSE;
		SetIconBmp(AlarmOffBmp);
	}	
}//cAlarmIconCtrl::ProcessNotifier

//setup Icon control to display icon of the currently executed program
cProgramIconCtrl::cProgramIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
//setup initial state of what is displayed based on the icon bitmap used for initialization
	
	if(InBmp==BathBmp)
		mLastDepictedProgram=WALLE_PROGRAM_BATH;
	else if (InBmp==VoiceBmp)
		mLastDepictedProgram=WALLE_PROGRAM_VOICE_CTRL;
	else if (InBmp==TestBmp)
		mLastDepictedProgram=WALLE_PROGRAM_TEST;
	else if (InBmp==EnjoyBmp)
		mLastDepictedProgram=WALLE_PROGRAM_ENJOY;
	else //unknown icon passed -> error
	{
		LCDClearScreen();//clear the screen	
		LCDPutStr("ERR: LibIcon: PrgIcon",20,1,SMALL,WHITE,BLACK);
		Uart0Message("ERR: cProgramIconCtrl::cProgramIconCtrl: Wrong bitmap pointer: ",(long)InBmp);			
		NOT_ALLOWED_VALUE;//generate an exception for not allwed Wall-e program selected
	}
}//cProgramIconCtrl::cProgramIconCtrl

//process notifier so proper Icon of the executed program is displayed
void cProgramIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_SYS_ALIVE) return;//when it is not EVT_SYS_ALIVEnotifier do nothing
	
	//do nothing on the screen when there is not change of the state
	if (mLastDepictedProgram==(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mWalleProgramToExecute))return;
	//display icon coresponding to the currently executed program as reported by EVT_SYS_ALIVE 
	switch((static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mWalleProgramToExecute))
	{
	case WALLE_PROGRAM_BATH:
		mLastDepictedProgram=WALLE_PROGRAM_BATH;//preserve program set and displayed
		SetIconBmp(BathBmp);
		break;
	case WALLE_PROGRAM_VOICE_CTRL:
		mLastDepictedProgram=WALLE_PROGRAM_VOICE_CTRL;//preserve program set and displayed
		SetIconBmp(VoiceBmp);
		break;
	case WALLE_PROGRAM_TEST:
		mLastDepictedProgram=WALLE_PROGRAM_TEST;//preserve program set and displayed
		SetIconBmp(TestBmp);
		break;
	case WALLE_PROGRAM_ENJOY:
		mLastDepictedProgram=WALLE_PROGRAM_ENJOY;//preserve program set and displayed
		SetIconBmp(EnjoyBmp);
		break;
	default://should never happen so exception if occured
		LCDDebugMessage("ERR: LibIcon: ",(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mWalleProgramToExecute),20,1,1,0);
		Uart0Message("ERR: cProgramIconCtrl::ProcessNotifier: Wrong program: ",(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mWalleProgramToExecute));			
		NOT_ALLOWED_PROGRAM;//generate an exception for not allwed Wall-e program selected
		break;
	}//switch
}//cProgramIconCtrl::ProcessNotifier

cSRAMIconCtrl::cSRAMIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY):cIconCtrl(InBmp,InX,InY)
{
	if(InBmp==RamPreservedBmp)//depending on bitmap used to initialize setup what will be first time depicted
		mLastDepictedState=TRUE;
	else
		mLastDepictedState=FALSE;
}//cSRAMIconCtrl::cSRAMIconCtrl

//process notifier so correct SRAM status is reflected by Icon control
void cSRAMIconCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_SYS_ALIVE) return;//when it is not EVT_SYS_ALIVEnotifier do nothing
	
	//if there is not state change versus what is already displayed do not refresh the icon
	if(mLastDepictedState==(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr()))->mRAMContentPreserved) return;
	
	//when SRAM content mantained put adequate icon
	if((static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr()))->mRAMContentPreserved==TRUE)
	{
		mLastDepictedState=TRUE;//remeber what state is going to be displayed
		SetIconBmp(RamPreservedBmp);
	}
	else //sRAM content not mantained
	{
		mLastDepictedState=FALSE;//remeber what state is going to be displayed
		SetIconBmp(RamNotPreservedBmp);
	}
}//cSRAMIconCtrl::ProcessNotifier





