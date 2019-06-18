/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_display.cpp
* Description: Robot display manager
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2013
* Note:
* History:
*              23-Sep-2013 - Initial version created
*********************************************************************************************************
*/

#include "mng_display.hpp"
#include "ctr_lcd.h"
#include "lib_std.h"
#include "hw_gpio.h"
#include "hw_uart.h"

#include "ctr_f_sens.h"
#include "lib_g_text.hpp"
#include "lib_g_window.hpp"
#include "lib_g_icon.hpp"

//that is to get reference to the icon storages already placed when lib_g_icon.cpp is compiled
extern const unsigned char BatteryHighBmp[];
extern const unsigned char AlarmOnBmp[];
extern const unsigned char BathBmp[];
extern const unsigned char EnjoyBmp[];
extern const unsigned char HeartBmp[];
extern const unsigned char RamNotPreservedBmp[];
extern const unsigned char RamPreservedBmp[];
extern const unsigned char TestBmp[];
extern const unsigned char VoiceBmp[];

cDisplayMngr::cDisplayMngr()
{
	mDisplayMode=DISPLAY_MODE_WAIT;//after power on wait for version information to be displayed
	mActiveWindow=WIN_DEAFULT_ID;//start from default window index
	for (int i=0;i<MAX_WINDOWS;i++)//go through all possible windows storage places to clear those
	{
		mWindows[i]=(cWindow*)0;//clear storage place
	}	
}//cDisplayMngr::cDisplayMngr

BYTE cDisplayMngr::FindMaxIndex()
{
	for (int i=0;i<MAX_WINDOWS;i++)//go through all windows
	{
		if(mWindows[i]==(cWindow*)0)//when first empty index find
		{
			if(i>0) //if list is not empty
				return i-1;
			else //if list is empty
				return 0;
		}
	}		
return MAX_WINDOWS-1;//when whole list filled return last position index	
}//cDisplayMngr::FindMaxIndex



//add specified window to the set of windows managed by Display Manager
void cDisplayMngr::AddWindow(cWindow &inWnd)
{
	for (int i=0;i<MAX_WINDOWS;i++)//cgo through all windows
	{
		if(mWindows[i]==(cWindow*)0)//when empt place on the  list
		{
			mWindows[i]=&inWnd;//add window to the list
			return;//do not continue window added
		}
	}
}//cDisplayMngr::AddWindow

//remove specified window from the set of windows managed by Display Manager 
void cDisplayMngr::RemoveWindow(cWindow &inWnd)
{
	int j;
	for (int i=0;i<MAX_WINDOWS;i++)//go through all pointers to windows
	{
		if(mWindows[i]==&inWnd)//when specified window find on the list
		{
			mWindows[i]=(cWindow *)0;//remove it
			for(j=i;j<MAX_WINDOWS-1;j++)//shift rest of the list so there is not empty places inside
			{
				mWindows[j]=mWindows[j+1];
			}
			mWindows[MAX_WINDOWS-1]=(cWindow *)0;//when all pointers shifted so there is no gap put zero at the end
		}
	}
}//cDisplayMngr::RemoveWindow

//used to process left and right keys from Wall-e front pannel
void cDisplayMngr::ProcessKey(cSmartPtr<cNotifier>  p_smartNotifier)
{
	WORD CurKeyStatus;//temporary stores pressed key for checks
	BYTE MaxWindowIndex=FindMaxIndex();//find current max index for the window lists
	
	CurKeyStatus = (static_cast<sKeyInput*>(p_smartNotifier->GetDataPtr()))->mCurKeyStatus;
	if(!(CurKeyStatus&BIT0))//when BUT1 pressed means left
			{
			if(mActiveWindow) //if not the first (default) window
			{
				mActiveWindow-=1;//left button moved to previous window but only if not default
			}
			else
			{
				mActiveWindow=MaxWindowIndex;//if default (first window) move to last one
			}
			if(mWindows[mActiveWindow])//if there is active window let it process event and is Draw it
				{
				mWindows[mActiveWindow]->SetReDraw();//mark new window if any for re-draw
				}
			}
	if(!(CurKeyStatus&BIT1))//when BUT2 pressed means right
			{
			if(mActiveWindow<MaxWindowIndex)//if not last window
			{
				mActiveWindow+=1;//move to next window on the list
			}
			else
			{
				mActiveWindow=0;//last window move to first window on the list
			}
			if(mWindows[mActiveWindow])//if there is active window let it process event and is Draw it
				{
				mWindows[mActiveWindow]->SetReDraw();//mark new window if any for re-draw
				}
			}
}//cDisplayMngr::ProcessKey

//draw active window and it's all controls 
void cDisplayMngr::Draw()
{
	if(GetActiveWindow())
		GetActiveWindow()->Draw();
}//cDisplayMngr::Draw

//clear whole active window from the LCD
void cDisplayMngr::Clear()
{
	if(GetActiveWindow())
		GetActiveWindow()->Clear();	
}//cDisplayMngr::Clear

//process notifier
void cDisplayMngr::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()==EVT_KEY)//when key event received
		ProcessKey(pNotifier);//process key by DisplayMng first
	
	if(GetActiveWindow())//if there is active window let it process event as well
		GetActiveWindow()->ProcessNotifier(pNotifier);
	
}//cDisplayMngr::ProcessNotifier


//controls for the default main window
cHeartIconCtrl HeartIcon(HeartBmp,4,4);
cTimeTxtCtrl TimeTxtCtrl("",4,44);
cDateTxtCtrl DateTxtCtrl("",22, 19);

cuPPowerIconCtrl uPPowerIcon(BatteryHighBmp,40,15);
cTxtCtrl  uPPwrText("uP", 68, 24);

cTrackPowerIconCtrl TrackPowerIcon(BatteryHighBmp,40,53);
cTxtCtrl  TrackPwrText("Track", 68, 50);

cServoPowerIconCtrl ServoPowerIcon(BatteryHighBmp,40,91);
cTxtCtrl  ServoPwrText("Servo", 68, 88);

cAlarmIconCtrl AlarmIcon(AlarmOnBmp,84,15);
cTxtCtrl  AlarmOnOffText("Alarm", 112, 12);

cProgramIconCtrl ProgramIcon(BathBmp,84,53);
cTxtCtrl  ProgramText("Prog", 112, 53);

cSRAMIconCtrl SRAMIcon(RamPreservedBmp,84,91);
cTxtCtrl  StaticRAMText("SRAM", 112, 91);

cIconCtrl RamPreservedIcon(RamPreservedBmp,32,8);
cIconCtrl RamNotPreservedIcon(RamNotPreservedBmp,32,48);
cIconCtrl BathIcon(BathBmp,32,88);

cIconCtrl VoiceIcon(VoiceBmp,80,8);
cIconCtrl TestIcon(TestBmp,80,48);
cIconCtrl EnjoyIcon(EnjoyBmp,80,88);

//controls for the Battery window
cBatteryTxtCtrl BatteryStateTxt(24,4);

//controls for the Date & Time window
cDateTimeTxtCtrl DateTimeTxt(24,4);

//controls for system information
cSysInfoCtrl SysInfoCtrl(24,4); 

cWindow DefaultWindow; //display most important info aminly by various icons - more details are on subsequent windows if needed
cWindow BatteryWindow;//displays all batteries status
cWindow DateTimeWindow;//provides info abou clock and alarm clock setup
cWindow SysInfoWindow;//provides system informations

//an extra window to display EVT_DISPLAY_INFO critical information
cWindow CriticalInfoWindow;
cTxtCtrl  InfoText("", 48, 8);

void cDisplayMngr::Run(void)
{
	cSmartPtr<cNotifier> pNotifier;
	
	//setup default(main) window and all its controls
	DefaultWindow.AddCtrl(HeartIcon);
	DefaultWindow.AddCtrl(TimeTxtCtrl);
	DefaultWindow.AddCtrl(DateTxtCtrl);
	
	DefaultWindow.AddCtrl(uPPowerIcon);
	DefaultWindow.AddCtrl(uPPwrText);
	
	DefaultWindow.AddCtrl(TrackPowerIcon);
	DefaultWindow.AddCtrl(TrackPwrText);
	
	DefaultWindow.AddCtrl(ServoPowerIcon);
	DefaultWindow.AddCtrl(ServoPwrText);
	
	DefaultWindow.AddCtrl(AlarmIcon);
	DefaultWindow.AddCtrl(AlarmOnOffText);
	
	DefaultWindow.AddCtrl(ProgramIcon);
	DefaultWindow.AddCtrl(ProgramText);
	
	DefaultWindow.AddCtrl(SRAMIcon);
	DefaultWindow.AddCtrl(StaticRAMText);
	AddWindow(DefaultWindow);
	
	//setup Batter status window and all its controls
	BatteryWindow.SetWinTitle("BATTERY");
	BatteryWindow.SetWinTitleFontSize(LARGE);
	BatteryWindow.AddCtrl(BatteryStateTxt);
	AddWindow(BatteryWindow);
	
	//setup window to display current time details as well as alarm details
	DateTimeWindow.SetWinTitle("DATE & TIME");
	DateTimeWindow.SetWinTitleFontSize(LARGE);
	DateTimeWindow.AddCtrl(DateTimeTxt);
	AddWindow(DateTimeWindow);
	
	//setup window to display system informations
	SysInfoWindow.SetWinTitle("SYSTEM INFO");
	SysInfoWindow.SetWinTitleFontSize(LARGE);
	SysInfoWindow.AddCtrl( SysInfoCtrl);
	AddWindow(SysInfoWindow);
	
	//setup window to display critical information
	//this is an extra window not on mWindows list
	CriticalInfoWindow.SetWinTitle("CRITICAL STATE!");
	InfoText.SetFontSize(LARGE);//critical information will be displayed in large font
	CriticalInfoWindow.AddCtrl(InfoText);
	
	for(;;)//enter infinite events processing loop
	{
		switch (mDisplayMode)
		{
		case DISPLAY_MODE_WAIT://wait after power on for a moment because version info is displayed
			//this condition is to process subscribed Notifiers even when waiting for version info to be displayed
			pNotifier = Receive();//wait for notifier to arrive
			if(pNotifier->GetNotifierId()==EVT_SYS_ALIVE)
			{//use EVT_SYS_ALIVE to check expired time since turn ON
				if(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mTimeStamp > SYS_PROMPT_DELAY)
					mDisplayMode=DISPLAY_MODE_NORMAL;//time to display version information expired switch to normal processing
			}
			if(pNotifier->GetNotifierId()==EVT_DISPLAY_INFO)//when in DISPLAY_MODE_WAIT critical request received
				mDisplayMode=DISPLAY_MODE_CRITICAL;//switch immediately to critical display
			break;
		case DISPLAY_MODE_NORMAL://normal operation when windows are displayed
			Draw();//draw active window and all it's controls if there are any changes
			pNotifier = Receive();//wait for notifier to arrive
			if(pNotifier->GetNotifierId()==EVT_DISPLAY_INFO)//when critical request received
				mDisplayMode=DISPLAY_MODE_CRITICAL;//switch immediately to critical display
			else //no critical information display request
				ProcessNotifier(pNotifier);//process received event if this change windows or it's content next Draw() call will update it
			break;
		case DISPLAY_MODE_CRITICAL://critical information is displayed nothing more van be displayed
			//retrive text to be displayed from EVT_DISPLAY_INFO notifier received
			InfoText.SetText(static_cast<sDspInfoEvt*>(pNotifier->GetDataPtr())->mText);
			CriticalInfoWindow.Draw();//draw critical information on LCD
			Uart0PutStr("\nLOG: DspMngr: Critical State: "); //and on terminal if connected
			Uart0PutStr(static_cast<sDspInfoEvt*>(pNotifier->GetDataPtr())->mText);
			for(;;)//stay in blocked display (ifinite loop) with this critical information displayed
			{//read notifiers but do not process them
				pNotifier = Receive();
			}
			break;
		default://not allowed state should never happen
			LCDDebugMessage("ERR: DspMngr: ",mDisplayMode,20,1,1,0);
			Uart0Message("ERR: DspMngr: Run: Unknown Dsp Mode: ",mDisplayMode);
			UNEXPECTED_ERROR_VALUE;
			break;
		}//switch
	}//for
}//cDisplayMngr::Run

