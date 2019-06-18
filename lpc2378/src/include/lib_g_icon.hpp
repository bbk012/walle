/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_icon.hpp
* Description: Class to handle bit map type widget for Walle Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        12-Nov-2017
* Note:
* History:
*              12-Nov-2017 - Initial version created
*********************************************************************************************************
*/

#ifndef LIB_G_ICON_HPP_
#define LIB_G_ICON_HPP_

#include "ctr_lcd.h"
#include "lib_std.h"
#include "lib_g_ctrl.hpp"

class cIconCtrl:public cCtrl
{
private:
	//pointer to icon's bitmap which is displayed when icon is draw/redraw
	//first byte of bmp is width while second byte of bmp is hight (in pixel)
	const BYTE* mBmp;

public:
	cIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY); //setup bmp icon for specified position
	const BYTE* GetIconBmp(){return mBmp;};
	void SetIconBmp(const BYTE* InBmp);//setup new bitemap for the icon
	
	BYTE GetHight(){if(mBmp)return LCDGetBmpHight(mBmp);else return 0;};
	BYTE GetWidth(){if(mBmp)return LCDGetBmpWidth(mBmp);else return 0;};
	
	void Draw();//puts icon's bitmap into display using parameters defined prioir to Draw() call

	//clear space occupied by the Icon
	void Clear();
};//cIconCtrl

//icon to display Wall-e heart bit
class cHeartIconCtrl:public cIconCtrl
{
private:
	BYTE mHeartTogleFlag; //used to turn on turn off heart icon when SYS_ALIVE notifier received
public:
	cHeartIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cHeartIconCtrl

//icon to display Wall-e uP board power status
class cuPPowerIconCtrl:public cIconCtrl
{
private:
	BYTE mLastDepictedBatteryState;//keep what was last time depicted to avoid icon change for every notifier received
	
public:
	cuPPowerIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cuPPowerIconCtrl

//icon to display Wall-e Tracks power status
class cTrackPowerIconCtrl:public cIconCtrl
{
private:
	BYTE mLastDepictedBatteryState;//keep what was last time depicted to avoid icon change for every notifier received
public:
	cTrackPowerIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cTrackPowerIconCtrl

//icon to display Wall-e Servo power status
class cServoPowerIconCtrl:public cIconCtrl
{
private:
	BYTE mLastDepictedBatteryState;//keep what was last time depicted to avoid icon change for every notifier received
public:
	cServoPowerIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cServoPowerIconCtrl

//icon to display Wall-e RTC Alarm status
class cAlarmIconCtrl:public cIconCtrl
{
private:
	BYTE mLastDepictedAlarmState;//keep what was last time depicted to avoid icon change for every notifier received
public:
	cAlarmIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cAlarmIconCtrl

//icon to display Wall-e executed program 
class cProgramIconCtrl:public cIconCtrl
{
private:
	BYTE mLastDepictedProgram;//used to preserve information about last program icon depicted on LCD
	//this is used to refresh icon only when there is program change
public:
	cProgramIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cProgramIconCtrl

//icon to display Wall-e static RAM status 
class cSRAMIconCtrl:public cIconCtrl
{
private:
	BYTE mLastDepictedState;//variable used to refresh icon only when there is change of the state
public:
	cSRAMIconCtrl(const BYTE* InBmp,BYTE InX, BYTE InY);
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cSRAMIconCtrl




#endif /*LIB_G_ICON_HPP_*/
