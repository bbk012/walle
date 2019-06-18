/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_text.hpp
* Description: Text widget for Walle Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        23-Sep-2017
* Note:
* History:
*              23-Sep-2017 - Initial version created
*********************************************************************************************************
*/
#include "lib_g_text.hpp"
#include "lib_time.h"
#include "hw_gpio.h"
#include "lib_error.h"
#include "hw_wdt.h"
#include "ctr_f_sens.h"



//default constractor, creates text control with defined content but not yet drawn on the screen
cTxtCtrl::cTxtCtrl(char *pString, BYTE InX, BYTE InY)
{
	mFontSize=SMALL;//default font size to be used by text
	GetLeftCorner().mX=InX;
	GetLeftCorner().mY=InY;
	SetText(pString);
}//cTxtCtrl::cTxtCtrl

//setup TxtCtrl string but only when there is enough storage place
void cTxtCtrl::SetText(char *pString)
{
	//when there is any text already displayed before you set up new one old need to be removed from the screen
	if(GetDrawn()==TRUE)
	{
		Clear();//clear old text from display	
	}
	memset(mText,0,MAX_TEXT_SIZE);//clear string buffer
	if (strlength(pString)<MAX_TEXT_SIZE)//copy if there is enough storage place
	{
		strcpy(mText, pString);
		SetReDraw();//mark that text changed and should be redrawn
	}
}	
//cTxtCtrl::SetText

//puts mText into display using private defined parameters prioir to Draw() call	
void cTxtCtrl::Draw()
{
	if(GetReDraw()==FALSE)//do not draw/re-draw control when not needed i.e when there is not aby change since last draw
			return;
	LCDPutStr(mText,GetLeftCorner().mX, GetLeftCorner().mY,mFontSize,GetFrgColor(),GetBckColor());
	SetDrawn();//mark that text is displayed already
	ClrReDraw();//once drawn mark as not require to be redrawn until another change
}//cTxtCtrl::Draw

void cTxtCtrl::Clear()
{
	LCDClrStr(mText,GetLeftCorner().mX,GetLeftCorner().mY,mFontSize,GetFrgColor(),GetBckColor());
	ClrDrawn();//mark that text is not displayed any longer
}//cTxtCtrl::Clear


cTimeTxtCtrl::cTimeTxtCtrl(char *pString, BYTE InX, BYTE InY):cTxtCtrl(pString,InX,InY)
{
	Clear();//clear text so it is not displayed when not yet setup by ProcessNotifier
	mTogleSecondsFlag=FALSE;//setup initial value to the seconds toggler
	memset(mTmpStrBuff,0,MAX_TIME_STR_BUFF_SIZE);//clear string buffer used for hours/minutes conversion
	memset(mTimeStrBuff,0,MAX_TIME_STR_BUFF_SIZE);//clear storage for final time string
	SetFontSize(LARGE);//setup time to be displayed as large string
}//cTimeTxtCtrl:: cTimeTxtCtrl

//process EVT_TIME notifier to display actual time
void cTimeTxtCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_TIME) return;//when it is not EVT_TIME notifier do nothing
	
	//add leading zero to the time string if hour lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour < 10)
		{
		strcpy(mTimeStrBuff,"0");
		itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour,mTmpStrBuff,10);//change Hours to string
		strcat(mTimeStrBuff,mTmpStrBuff);//put hours into time string buffer
		}
	else//no need to add leading "0" for Hour string
	{
		itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour,mTmpStrBuff,10);//change Hours to string
		strcpy(mTimeStrBuff,mTmpStrBuff);//put hours into time string buffer
	}
	
	if(mTogleSecondsFlag==FALSE)//when ":" is not displayed
	{
		mTogleSecondsFlag=TRUE;//mark ":" as displayed
		strcat(mTimeStrBuff,":");//add ":" into time string
	}
	else //when ":" is displayed
	{
		mTogleSecondsFlag=FALSE;//mark ":" as not displayed
		strcat(mTimeStrBuff," ");//add " " into time string
	}
	//add leading zero to the time string if minutes lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute < 10)
		strcat(mTimeStrBuff,"0");
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute,mTmpStrBuff,10);//change Minutes to string
	strcat(mTimeStrBuff,mTmpStrBuff);//add minutes into time string
	
	SetText(mTimeStrBuff);//add finale assembled time string to the controls
	
}//cTimeTxtCtrl::ProcessNotifier

//constructor of the Date Text control
cDateTxtCtrl::cDateTxtCtrl(char *pString, BYTE InX, BYTE InY):cTxtCtrl(pString,InX,InY)
{
	Clear();//clear text so it is not displayed when not yet setup by ProcessNotifier
	memset(mTmpStrBuff,0,MAX_DATE_STR_BUFF_SIZE);//clear string buffer used for date conversion
	memset(mDateStrBuff,0,MAX_TEXT_SIZE);//clear storage for final date string
	SetFontSize(SMALL);//setup date to be displayed as small string
}//cDateTxtCtrl::cDateTxtCtrl

//used to process EVT_TIME event and extract date information from it to display this on the screen
void cDateTxtCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_TIME) return;//when it is not EVT_TIME notifier do nothing
	
	//assemby date string: <Short Day Name>, <Day> <Short Month Name> <year>
	//get short day name
	strcpy(mDateStrBuff,DayShortStr(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDayOfWeek));
	strcat(mDateStrBuff,", ");//add ", "
	//get day
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDay,mTmpStrBuff,10);
	strcat(mDateStrBuff,mTmpStrBuff);//add to assembled date string
	strcat(mDateStrBuff," ");//add " " to assembled date string
	//add month short name
	strcat(mDateStrBuff,MonthShortStr(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMonth));
	strcat(mDateStrBuff," ");//add " " to assembled date string
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mYear,mTmpStrBuff,10);//get year and change to string
	strcat(mDateStrBuff,mTmpStrBuff);//add year to assembled date string
	SetText(mDateStrBuff);
}//cDateTxtCtrl::ProcessNotifier

//arrange text controls to be ready to display all batteries state on the wondow
cBatteryTxtCtrl::cBatteryTxtCtrl(BYTE InX, BYTE InY):
	mTxtuPBatVoltage      (STR_UP_BAT,InX+8,InY),     //place subsequent strings based on input x and y coordinates
	mTxtuPBatMinVoltage   (STR_UP_MIN_BAT,InX+18,InY),
	mTxtTrackBatVoltage   (STR_TRACK_BAT,InX+35+8,InY),
	mTxtTrackBatMinVoltage(STR_TRACK_MIN_BAT,InX+35+18,InY),
	mTxtServoBatVoltage   (STR_SERVO_BAT,InX+70+8,InY),
	mTxtServoBatMinVoltage(STR_SERVO_MIN_BAT,InX+70+18,InY)
{
	//setup initial values (max value) so first EVT_BATTERY cause display values refresh
	muPBatVoltage=0xFF;
	muPBatMinVoltage=0xFF;
	mTrackBatVoltage=0xFF;
	mTrackBatMinVoltage=0xFF;
	mServoBatVoltage=0xFF;
	mServoBatMinVoltage=0xFF;	
}//cBatteryTxtCtrl::cBatteryTxtCtrl

//draw control on the screen
void cBatteryTxtCtrl::Draw()
{
	mTxtuPBatVoltage.Draw();
	mTxtuPBatMinVoltage.Draw();
	mTxtTrackBatVoltage.Draw();
	mTxtTrackBatMinVoltage.Draw();
	mTxtServoBatVoltage.Draw();
	mTxtServoBatMinVoltage.Draw();
	
}//cBatteryTxtCtrl::Draw

//clear whole control from the LCD
void cBatteryTxtCtrl::Clear()
{
	mTxtuPBatVoltage.Clear();
	mTxtuPBatMinVoltage.Clear();
	mTxtTrackBatVoltage.Clear();
	mTxtTrackBatMinVoltage.Clear();
	mTxtServoBatVoltage.Clear();
	mTxtServoBatMinVoltage.Clear();
		
}//cBatteryTxtCtrl::Clear

void cBatteryTxtCtrl::SetDrawn()
{
	mTxtuPBatVoltage.SetDrawn();
	mTxtuPBatMinVoltage.SetDrawn();
	mTxtTrackBatVoltage.SetDrawn();
	mTxtTrackBatMinVoltage.SetDrawn();
	mTxtServoBatVoltage.SetDrawn();
	mTxtServoBatMinVoltage.SetDrawn();
	cCtrl::SetDrawn();
}//cBatteryTxtCtrl::SetDrawn

void cBatteryTxtCtrl::ClrDrawn()
{
	mTxtuPBatVoltage.ClrDrawn();
	mTxtuPBatMinVoltage.ClrDrawn();
	mTxtTrackBatVoltage.ClrDrawn();
	mTxtTrackBatMinVoltage.ClrDrawn();
	mTxtServoBatVoltage.ClrDrawn();
	mTxtServoBatMinVoltage.ClrDrawn();
	cCtrl::ClrDrawn();
}//cBatteryTxtCtrl::ClrDrawn


void cBatteryTxtCtrl::SetReDraw()
{
	mTxtuPBatVoltage.SetReDraw();
	mTxtuPBatMinVoltage.SetReDraw();
	mTxtTrackBatVoltage.SetReDraw();
	mTxtTrackBatMinVoltage.SetReDraw();
	mTxtServoBatVoltage.SetReDraw();
	mTxtServoBatMinVoltage.SetReDraw();
	cCtrl::SetReDraw();
}// cBatteryTxtCtrl::SetReDraw

void cBatteryTxtCtrl::ClrReDraw()
{
	mTxtuPBatVoltage.ClrReDraw();
	mTxtuPBatMinVoltage.ClrReDraw();
	mTxtTrackBatVoltage.ClrReDraw();
	mTxtTrackBatMinVoltage.ClrReDraw();
	mTxtServoBatVoltage.ClrReDraw();
	mTxtServoBatMinVoltage.ClrReDraw();
	cCtrl::ClrReDraw();
}//cBatteryTxtCtrl::ClrReDraw

//process notifier
void cBatteryTxtCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_BATTERY) return;//when it is not EVT_BATTERY notifier do nothing
	
	//when any voltage differs to the previous value update it
	if(muPBatVoltage!=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyVoltage))
	{
		//convert byte value to string and formulate string text with value
		itoa((static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyVoltage),mByteValueStrBuff,10);
		strcpy(mTextBuff,STR_UP_BAT);
		strcat(mTextBuff,mByteValueStrBuff);
		mTxtuPBatVoltage.SetText(mTextBuff);
		//preserve displayed value to detect eventual change next time
		muPBatVoltage=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyVoltage);
	}
	if(muPBatMinVoltage!=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyMinVoltage))
	{
		//convert byte value to string and formulate string text with value
		itoa((static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyMinVoltage),mByteValueStrBuff,10);
		strcpy(mTextBuff,STR_UP_MIN_BAT);
		strcat(mTextBuff,mByteValueStrBuff);
		mTxtuPBatMinVoltage.SetText(mTextBuff);
		//preserve displayed value to detect eventual change next time
		muPBatMinVoltage=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMainSupplyMinVoltage);	
	}
	if(mTrackBatVoltage!=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyVoltage))
	{
		//convert byte value to string and formulate string text with value
		itoa((static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyVoltage),mByteValueStrBuff,10);
		strcpy(mTextBuff,STR_TRACK_BAT);
		strcat(mTextBuff,mByteValueStrBuff);
		mTxtTrackBatVoltage.SetText(mTextBuff);
		//preserve displayed value to detect eventual change next time
		mTrackBatVoltage=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyVoltage);
		
	}
	if(mTrackBatMinVoltage!=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyMinVoltage))
	{
		//convert byte value to string and formulate string text with value
		itoa((static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyMinVoltage),mByteValueStrBuff,10);
		strcpy(mTextBuff,STR_TRACK_MIN_BAT);
		strcat(mTextBuff,mByteValueStrBuff);
		mTxtTrackBatMinVoltage.SetText(mTextBuff);
		//preserve displayed value to detect eventual change next time
		mTrackBatMinVoltage=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mMotorSupplyMinVoltage);
	}
	if(mServoBatVoltage!=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyVoltage))
	{
		//convert byte value to string and formulate string text with value
		itoa((static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyVoltage),mByteValueStrBuff,10);
		strcpy(mTextBuff,STR_SERVO_BAT);
		strcat(mTextBuff,mByteValueStrBuff);
		mTxtServoBatVoltage.SetText(mTextBuff);
		//preserve displayed value to detect eventual change next time
		mServoBatVoltage=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyVoltage);
	}
	if(mServoBatMinVoltage!=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyMinVoltage))
	{
		//convert byte value to string and formulate string text with value
		itoa((static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyMinVoltage),mByteValueStrBuff,10);
		strcpy(mTextBuff,STR_SERVO_MIN_BAT);
		strcat(mTextBuff,mByteValueStrBuff);
		mTxtServoBatMinVoltage.SetText(mTextBuff);
		//preserve displayed value to detect eventual change next time
		mServoBatMinVoltage=(static_cast<sBatteryStatus*>(pNotifier->GetDataPtr())->mServoSupplyMinVoltage);
	}
	
}//cBatteryTxtCtrl::ProcessNotifier


//arrange text controls to be ready to display date and time information when tied to the window
cDateTimeTxtCtrl::cDateTimeTxtCtrl(BYTE InX, BYTE InY):
	mTxtClkStr(D_T_CLOCK_STR,InX,InY),
	mTxtClkDateStr(D_T_DATE_STR,InX+8,InY+8),
	mTxtClkDate(D_T_EMPTY_STR,InX+16,InY+16),
	mTxtClkTimeStr(D_T_TIME_STR,InX+24,InY+8),
	mTxtClkTime(D_T_EMPTY_STR,InX+32,InY+32),	
	mTxtAlarmStr(D_T_ALARM_STR,InX+42,InY),
	mTxtAlarmStateStr(D_T_STATE_STR,InX+50,InY+8),
	mTxtAlarmTriggerStr(D_T_TRIGGER_STR,InX+58,InY+8),
	mTxtAlarmDateStr(D_T_DATE_STR,InX+66,InY+8),
	mTxtAlarmDate(D_T_EMPTY_STR,InX+74,InY+16),
	mTxtAlarmTimeStr(D_T_TIME_STR, InX+82,InY+8),
	mTxtAlarmTime(D_T_EMPTY_STR,InX+90,InY+32)
{
	//setup undefault font size for CLOCK and ALARM text
	
	mTxtClkStr.SetFontSize(MEDIUM);
	mTxtAlarmStr.SetFontSize(MEDIUM);
	
	//clear storage for buffers
	memset(mClkDateStrBuff,0,MAX_TEXT_SIZE);
	memset(mClkTimeStrBuff,0,MAX_TIME_STR_BUFF_SIZE);
	memset(mAlarmDateStrBuff,0,MAX_TEXT_SIZE);
	memset(mAlarmTimeStrBuff,0,MAX_TIME_STR_BUFF_SIZE);
	memset(mTmpStrBuff,0,MAX_TEXT_SIZE);
	
}//cDateTimeTxtCtrl::cDateTimeTxtCtrl

//draw control on the screen
void cDateTimeTxtCtrl::Draw()
{
	mTxtClkStr.Draw();
	mTxtClkDateStr.Draw();
	mTxtClkDate.Draw();
	mTxtClkTimeStr.Draw();
	mTxtClkTime.Draw();	
	mTxtAlarmStr.Draw();
	mTxtAlarmStateStr.Draw();
	mTxtAlarmTriggerStr.Draw();
	mTxtAlarmDateStr.Draw();
	mTxtAlarmDate.Draw();
	mTxtAlarmTimeStr.Draw();
	mTxtAlarmTime.Draw();
}//cDateTimeTxtCtrl::Draw

//clear whole control from the LCD
void cDateTimeTxtCtrl::Clear()
{
	mTxtClkStr.Clear();
	mTxtClkDateStr.Clear();
	mTxtClkDate.Clear();
	mTxtClkTimeStr.Clear();
	mTxtClkTime.Clear();	
	mTxtAlarmStr.Clear();
	mTxtAlarmStateStr.Clear();
	mTxtAlarmTriggerStr.Clear();
	mTxtAlarmDateStr.Clear();
	mTxtAlarmDate.Clear();
	mTxtAlarmTimeStr.Clear();
	mTxtAlarmTime.Clear();
		
}//cDateTimeTxtCtrl::Clear

void cDateTimeTxtCtrl::SetDrawn()
{
	mTxtClkStr.SetDrawn();
	mTxtClkDateStr.SetDrawn();
	mTxtClkDate.SetDrawn();
	mTxtClkTimeStr.SetDrawn();
	mTxtClkTime.SetDrawn();	
	mTxtAlarmStr.SetDrawn();
	mTxtAlarmStateStr.SetDrawn();
	mTxtAlarmTriggerStr.SetDrawn();
	mTxtAlarmDateStr.SetDrawn();
	mTxtAlarmDate.SetDrawn();
	mTxtAlarmTimeStr.SetDrawn();
	mTxtAlarmTime.SetDrawn();
	cCtrl::SetDrawn();
}//cDateTimeTxtCtrl::SetDrawn

void cDateTimeTxtCtrl::ClrDrawn()
{
	mTxtClkStr.ClrDrawn();
	mTxtClkDateStr.ClrDrawn();
	mTxtClkDate.ClrDrawn();
	mTxtClkTimeStr.ClrDrawn();
	mTxtClkTime.ClrDrawn();	
	mTxtAlarmStr.ClrDrawn();
	mTxtAlarmStateStr.ClrDrawn();
	mTxtAlarmTriggerStr.ClrDrawn();
	mTxtAlarmDateStr.ClrDrawn();
	mTxtAlarmDate.ClrDrawn();
	mTxtAlarmTimeStr.ClrDrawn();
	mTxtAlarmTime.ClrDrawn();
	cCtrl::ClrDrawn();
}//cDateTimeTxtCtrl::ClrDrawn


void cDateTimeTxtCtrl::SetReDraw()
{
	mTxtClkStr.SetReDraw();
	mTxtClkDateStr.SetReDraw();
	mTxtClkDate.SetReDraw();
	mTxtClkTimeStr.SetReDraw();
	mTxtClkTime.SetReDraw();	
	mTxtAlarmStr.SetReDraw();
	mTxtAlarmStateStr.SetReDraw();
	mTxtAlarmTriggerStr.SetReDraw();
	mTxtAlarmDateStr.SetReDraw();
	mTxtAlarmDate.SetReDraw();
	mTxtAlarmTimeStr.SetReDraw();
	mTxtAlarmTime.SetReDraw();

	cCtrl::SetReDraw();
}// cDateTimeTxtCtrl::SetReDraw

void cDateTimeTxtCtrl::ClrReDraw()
{
	mTxtClkStr.ClrReDraw();
	mTxtClkDateStr.ClrReDraw();
	mTxtClkDate.ClrReDraw();
	mTxtClkTimeStr.ClrReDraw();
	mTxtClkTime.ClrReDraw();	
	mTxtAlarmStr.ClrReDraw();
	mTxtAlarmStateStr.ClrReDraw();
	mTxtAlarmTriggerStr.ClrReDraw();
	mTxtAlarmDateStr.ClrReDraw();
	mTxtAlarmDate.ClrReDraw();
	mTxtAlarmTimeStr.ClrReDraw();
	mTxtAlarmTime.ClrReDraw();
	
	cCtrl::ClrReDraw();
}//cDateTimeTxtCtrl::ClrReDraw

//process notifier
void cDateTimeTxtCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_TIME && pNotifier->GetNotifierId()!=EVT_ALARM) return;//when it is not notifier to be processed
	
	if(pNotifier->GetNotifierId()==EVT_TIME) //when EVT_TIME notifier process it assembly date info
	{//assemby date string: <Short Day Name>, <Day> <Short Month Name> <year>
		
	//get short day name
	strcpy(mClkDateStrBuff,DayShortStr(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDayOfWeek));
	strcat(mClkDateStrBuff,", ");//add ", "
	//get day
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDay,mTmpStrBuff,10);
	strcat(mClkDateStrBuff,mTmpStrBuff);//add to assembled date string
	strcat(mClkDateStrBuff," ");//add " " to assembled date string
	//add month short name
	strcat(mClkDateStrBuff,MonthShortStr(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMonth));
	strcat(mClkDateStrBuff," ");//add " " to assembled date string
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mYear,mTmpStrBuff,10);//get year and change to string
	strcat(mClkDateStrBuff,mTmpStrBuff);//add year to assembled date string
	
	if(strcmp(mTxtClkDate.GetText(),mClkDateStrBuff))//if date change to what is already displayed update it
		mTxtClkDate.SetText(mClkDateStrBuff);
	
	//add leading zero to the time string if hour lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour < 10)
		{
		strcpy(mClkTimeStrBuff,"0");
		itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour,mTmpStrBuff,10);//change Hours to string
		strcat(mClkTimeStrBuff,mTmpStrBuff);//put hours into time string buffer
		}
	else//no need to add leading "0" for Hour string
	{
		itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour,mTmpStrBuff,10);//change Hours to string
		strcpy(mClkTimeStrBuff,mTmpStrBuff);//put hours into time string buffer
	}
	strcat(mClkTimeStrBuff,":");//add ":" into time string

	//add leading zero to the time string if minutes lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute < 10)
		strcat(mClkTimeStrBuff,"0");
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute,mTmpStrBuff,10);//change Minutes to string
	strcat(mClkTimeStrBuff,mTmpStrBuff);//add minutes into time string
	
	strcat(mClkTimeStrBuff,":");//add ":" into time string
	
	//add leading zero to the time string if seconds lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mSecond < 10)
		strcat(mClkTimeStrBuff,"0");
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mSecond ,mTmpStrBuff,10);//change Minutes to string
	strcat(mClkTimeStrBuff,mTmpStrBuff);//add minutes into time string
	
	mTxtClkTime.SetText(mClkTimeStrBuff);//add final assembled time string to the controls
	}//if(pNotifier->GetNotifierId()==EVT_TIME)
	
	if(pNotifier->GetNotifierId()==EVT_ALARM) //when EVT_ALARM notifier process it assembly date info
	{
	//assemble Alarm Status string - mAlarmDateStrBuff is used to assemble status string
	strcpy(mAlarmDateStrBuff,D_T_STATE_STR);	
	if((static_cast<sRtcData*>(pNotifier->GetDataPtr())->mIsAlarmSetupInRTC)==TRUE)//when alarm is setup
		strcat(mAlarmDateStrBuff,D_T_STATE_ON_STR);//add "ON" str
	else//when alarm is not setup add "OFF" str
		strcat(mAlarmDateStrBuff,D_T_STATE_OFF_STR);
	if(strcmp(mTxtAlarmStateStr.GetText(),mAlarmDateStrBuff))//if alarm status change
		mTxtAlarmStateStr.SetText(mAlarmDateStrBuff);//update it on the screen
	
	//assemble Alarm trigger status - mAlarmDateStrBuff is used to assemble trigger string
	strcpy(mAlarmDateStrBuff,D_T_TRIGGER_STR);	
	if((static_cast<sRtcData*>(pNotifier->GetDataPtr())->mIsAlarmTriggeredByRTC)==TRUE)//when alarm is just triggered
		strcat(mAlarmDateStrBuff,D_T_TRIGGERED_YES_STR);//add "YES" str
	else//when alarm is not triggered add "NO" str
		strcat(mAlarmDateStrBuff,D_T_TRIGGERED_NO_STR);
	if(strcmp(mTxtAlarmTriggerStr.GetText(),mAlarmDateStrBuff))//if alarm triggered state change
		mTxtAlarmTriggerStr.SetText(mAlarmDateStrBuff);//update it on the screen
	
	//assemby date string: <Short Day Name>, <Day> <Short Month Name> <year>
	//get short day name
	strcpy(mAlarmDateStrBuff,DayShortStr(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDayOfWeek));
	strcat(mAlarmDateStrBuff,", ");//add ", "
	//get day
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mDay,mTmpStrBuff,10);
	strcat(mAlarmDateStrBuff,mTmpStrBuff);//add to assembled date string
	strcat(mAlarmDateStrBuff," ");//add " " to assembled date string
	//add month short name
	strcat(mAlarmDateStrBuff,MonthShortStr(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMonth));
	strcat(mAlarmDateStrBuff," ");//add " " to assembled date string
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mYear,mTmpStrBuff,10);//get year and change to string
	strcat(mAlarmDateStrBuff,mTmpStrBuff);//add year to assembled date string
	
	if(strcmp(mTxtAlarmDate.GetText(),mAlarmDateStrBuff))//if date change to what is already displayed update it
		mTxtAlarmDate.SetText(mAlarmDateStrBuff);
	
	//add leading zero to the time string if hour lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour < 10)
		{
		strcpy(mAlarmTimeStrBuff,"0");
		itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour,mTmpStrBuff,10);//change Hours to string
		strcat(mAlarmTimeStrBuff,mTmpStrBuff);//put hours into time string buffer
		}
	else//no need to add leading "0" for Hour string
	{
		itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mHour,mTmpStrBuff,10);//change Hours to string
		strcpy(mAlarmTimeStrBuff,mTmpStrBuff);//put hours into time string buffer
	}
	strcat(mAlarmTimeStrBuff,":");//add ":" into time string

	//add leading zero to the time string if minutes lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute < 10)
		strcat(mAlarmTimeStrBuff,"0");
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mMinute,mTmpStrBuff,10);//change Minutes to string
	strcat(mAlarmTimeStrBuff,mTmpStrBuff);//add minutes into time string
	
	strcat(mAlarmTimeStrBuff,":");//add ":" into time string
	
	//add leading zero to the time string if seconds lower than 10 
	if(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mSecond < 10)
		strcat(mAlarmTimeStrBuff,"0");
	itoa(static_cast<sRtcData*>(pNotifier->GetDataPtr())->mSecond ,mTmpStrBuff,10);//change Minutes to string
	strcat(mAlarmTimeStrBuff,mTmpStrBuff);//add minutes into time string
	
	if(strcmp(mTxtAlarmTime.GetText(),mAlarmTimeStrBuff))//if time change to what is already displayed update it
		mTxtAlarmTime.SetText(mAlarmTimeStrBuff);//add final assembled time string to the controls
	}//if(pNotifier->GetNotifierId()==EVT_ALARM)
}//cDateTimeTxtCtrl::ProcessNotifier

//arrange text controls to be ready to display date and time information when tied to the window
cSysInfoCtrl::cSysInfoCtrl(BYTE InX, BYTE InY):
	mTxtLastReasonStr(S_I_REASON_STR,InX,InY), //title string
	mTxtPowerOffStr(S_I_POWER_OFF_STR,InX+8,InY+8),
	mTxtExceptionStr(S_I_EXCEPT_STR,InX+16,InY+8),
	mTxtResetStrStr(S_I_RESET_STR,InX+24,InY+8),
	mTxtSysResStr(S_I_SYS_RES_STR,InX+36,InY),//title string
	mTxtHeepFreeStr(S_I_HEAP_FREE_STR,InX+44,InY+8),
	mTxtHeepMinFreeStr(S_I_HEAP_MIN_STR,InX+52,InY+8),
	mTxtDispatchStr(S_I_DISPATCH_STR,InX+60,InY+8),
	mTxtDayDetectStr(S_I_DAY_DETECT_STR,InX+72,InY),//title string
	mTxtDayStateStr(S_I_DAY_STATE_STR,InX+80,InY+8),
	mTxtDayVoltageStr(S_I_DAY_VOLTAGE_STR,InX+88,InY+8)
{
	//setup undefault font size for titles string
	mTxtLastReasonStr.SetFontSize(MEDIUM);
	mTxtSysResStr.SetFontSize(MEDIUM);
	mTxtDayDetectStr.SetFontSize(MEDIUM);
	
	//clear storage for buffers
	memset(mTmpStrBuf,0,MAX_TEXT_SIZE);
	memset(mPowerOffStrBuff,0,MAX_TEXT_SIZE);
	memset(mExceptionStrBuff,0,MAX_TEXT_SIZE);
	memset(mResetStrBuff,0,MAX_TEXT_SIZE);
	memset(mHeepFreeStrBuff,0,MAX_TEXT_SIZE);
	memset(mHeepMinFreeStrBuff,0,MAX_TEXT_SIZE);
	memset(mDispatchStrBuff,0,MAX_TEXT_SIZE);
	memset(mDayStateStrBuff,0,MAX_TEXT_SIZE);
	memset(mDayStateVoltageStrBuff,0,MAX_TEXT_SIZE);
}//cSysInfoCtrl::cSysInfoCtrl

char* cSysInfoCtrl::PowerOffReasonToStr(WORD InLastPowerOffReason)
{
	switch(InLastPowerOffReason)
	{
	case UNSET_TURN_OFF:
		strcpy(mTmpStrBuf,UNSET_TURN_OFF_STR);
		break;
	case KEYPAD_TURN_OFF:
		strcpy(mTmpStrBuf,KEYPAD_TURN_OFF_STR);
		break;
	case REMOTE_TURN_OFF:
		strcpy(mTmpStrBuf,REMOTE_TURN_OFF_STR);
		break;
	case ALARM_TURN_OFF:
		strcpy(mTmpStrBuf,ALARM_TURN_OFF_STR);
		break;
	case HEALTH_STATE_TURN_OFF:
		strcpy(mTmpStrBuf,HEALTH_STATE_TURN_OFF_STR);
		break;
	case EXCEPTION_TURN_OFF:
		strcpy(mTmpStrBuf,EXCEPTION_TURN_OFF_STR);
		break;
	default:
		strcpy(mTmpStrBuf,UNDEF_POWER_OFF_ERR_STR);
		break;
	}
return 	mTmpStrBuf;
}//cSysInfoCtrl::PowerOfReasonToStr

char* cSysInfoCtrl::ExceptionReasonToStr(WORD InLastExceptionReason)
{
	switch(InLastExceptionReason)
	{
	case NONE_EXCEPTION:
		strcpy(mTmpStrBuf,NONE_EXCEPTION_STR);
		break;
	case OTHER_EXCEPTION:
		strcpy(mTmpStrBuf,OTHER_EXCEPTION_STR);
		break;
	case MEM_ALLOC_REASON:
		strcpy(mTmpStrBuf,MEM_ALLOC_REASON_STR);
		break;
	case UCOSII_RES_REASON:
		strcpy(mTmpStrBuf,UCOSII_RES_REASON_STR);
		break;
	case THREAD_NO_RUN_DEFINED_REASON:
		strcpy(mTmpStrBuf,THREAD_NO_RUN_DEFINED_REASON_STR);
		break;
	case THREAD_CREATE_REASON:
		strcpy(mTmpStrBuf,THREAD_CREATE_REASON_STR);
		break;
	case PUBLISHER_REGISTER_REASON:
		strcpy(mTmpStrBuf,PUBLISHER_REGISTER_REASON_STR);
		break;
	case SUBSCRIBER_REGISTER_REASON:
		strcpy(mTmpStrBuf,SUBSCRIBER_REGISTER_REASON_STR);
		break;
	case GFX_MEM_ALLOC_REASON:
		strcpy(mTmpStrBuf,GFX_MEM_ALLOC_REASON_STR);
		break;
	case DEBUG_REASON:
		strcpy(mTmpStrBuf,DEBUG_REASON_STR);
		break;
	case NOT_ALLOWED_STATE_REASON:
		strcpy(mTmpStrBuf,NOT_ALLOWED_STATE_REASON_STR);
		break;
	case UNEXPECTED_ERROR_VALUE_REASON:
		strcpy(mTmpStrBuf,UNEXPECTED_ERROR_VALUE_REASON_STR);
		break;
	case WIN_CTRL_NO_VIRTUAL_DEFINED_REASON:
		strcpy(mTmpStrBuf,WIN_CTRL_NO_VIRTUAL_DEFINED_REASON_STR);
		break;
	case NOT_ALLOWED_PROGRAM_REASON:
		strcpy(mTmpStrBuf,NOT_ALLOWED_PROGRAM_REASON_STR);
		break;
	case NOT_ALLOWED_VALUE_REASON:
		strcpy(mTmpStrBuf,NOT_ALLOWED_VALUE_REASON_STR);
		break;
	default:
		strcpy(mTmpStrBuf,UNDEF_REASON_STR);
		break;
	}
	return 	mTmpStrBuf;
}//cSysInfoCtrl::ExceptionReasonToStr

char* cSysInfoCtrl::ResetReasonToStr(BYTE LastResetReason)
{
	switch(LastResetReason)
	{
	case NO_RESET:
		strcpy(mTmpStrBuf,NO_RESET_STR);
		break;
	case WDT_RESET:
		strcpy(mTmpStrBuf,WDT_RESET_STR);
		break;
	case SW_RESET:
		strcpy(mTmpStrBuf,SW_RESET_STR);
		break;
	default:
		strcpy(mTmpStrBuf,UNDEF_RESET_STR);
		break;
	
	}
return mTmpStrBuf;
}//cSysInfoCtrl::ResetReasonToStr

//convert day value code into descriptive string to be displayed
char* cSysInfoCtrl::DayStateToStr(BYTE InDayState)
{
	switch(InDayState)
	{
	case DARK_DAY:
		strcpy(mTmpStrBuf,DARK_DAY_STR);
		break;
	case GREY_DAY:
		strcpy(mTmpStrBuf,GREY_DAY_STR);
		break;
	case BRIGHT_DAY:
		strcpy(mTmpStrBuf,BRIGHT_DAY_STR);
		break;
	default:
		strcpy(mTmpStrBuf,UNDEF_DAY_STR);
		break;
	}
	return mTmpStrBuf;
}//cSysInfoCtrl::DayStateToStr

//draw control on the screen
void cSysInfoCtrl::Draw()
{
	mTxtLastReasonStr.Draw();
	mTxtPowerOffStr.Draw();
	mTxtExceptionStr.Draw();
	mTxtResetStrStr.Draw();
	mTxtSysResStr.Draw();
	mTxtHeepFreeStr.Draw();
	mTxtHeepMinFreeStr.Draw();
	mTxtDispatchStr.Draw();
	mTxtDayDetectStr.Draw();
	mTxtDayStateStr.Draw();
	mTxtDayVoltageStr.Draw();
}//cSysInfoCtrl::Draw

//clear whole control from the LCD
void cSysInfoCtrl::Clear()
{
	mTxtLastReasonStr.Clear();
	mTxtPowerOffStr.Clear();
	mTxtExceptionStr.Clear();
	mTxtResetStrStr.Clear();
	mTxtSysResStr.Clear();
	mTxtHeepFreeStr.Clear();
	mTxtHeepMinFreeStr.Clear();
	mTxtDispatchStr.Clear();
	mTxtDayDetectStr.Clear();
	mTxtDayStateStr.Clear();
	mTxtDayVoltageStr.Clear();
		
}//cSysInfoCtrl::Clear

void cSysInfoCtrl::SetDrawn()
{
	mTxtLastReasonStr.SetDrawn();
	mTxtPowerOffStr.SetDrawn();
	mTxtExceptionStr.SetDrawn();
	mTxtResetStrStr.SetDrawn();
	mTxtSysResStr.SetDrawn();
	mTxtHeepFreeStr.SetDrawn();
	mTxtHeepMinFreeStr.SetDrawn();
	mTxtDispatchStr.SetDrawn();
	mTxtDayDetectStr.SetDrawn();
	mTxtDayStateStr.SetDrawn();
	mTxtDayVoltageStr.SetDrawn();
	cCtrl::SetDrawn();
}//cSysInfoCtrl::SetDrawn

void cSysInfoCtrl::ClrDrawn()
{
	mTxtLastReasonStr.ClrDrawn();
	mTxtPowerOffStr.ClrDrawn();
	mTxtExceptionStr.ClrDrawn();
	mTxtResetStrStr.ClrDrawn();
	mTxtSysResStr.ClrDrawn();
	mTxtHeepFreeStr.ClrDrawn();
	mTxtHeepMinFreeStr.ClrDrawn();
	mTxtDispatchStr.ClrDrawn();
	mTxtDayDetectStr.ClrDrawn();
	mTxtDayStateStr.ClrDrawn();
	mTxtDayVoltageStr.ClrDrawn();
	cCtrl::ClrDrawn();
}//cSysInfoCtrl::ClrDrawn


void cSysInfoCtrl::SetReDraw()
{
	mTxtLastReasonStr.SetReDraw();
	mTxtPowerOffStr.SetReDraw();
	mTxtExceptionStr.SetReDraw();
	mTxtResetStrStr.SetReDraw();
	mTxtSysResStr.SetReDraw();
	mTxtHeepFreeStr.SetReDraw();
	mTxtHeepMinFreeStr.SetReDraw();
	mTxtDispatchStr.SetReDraw();
	mTxtDayDetectStr.SetReDraw();
	mTxtDayStateStr.SetReDraw();
	mTxtDayVoltageStr.SetReDraw();

	cCtrl::SetReDraw();
}// cSysInfoCtrl::SetReDraw

void cSysInfoCtrl::ClrReDraw()
{
	mTxtLastReasonStr.ClrReDraw();
	mTxtPowerOffStr.ClrReDraw();
	mTxtExceptionStr.ClrReDraw();
	mTxtResetStrStr.ClrReDraw();
	mTxtSysResStr.ClrReDraw();
	mTxtHeepFreeStr.ClrReDraw();
	mTxtHeepMinFreeStr.ClrReDraw();
	mTxtDispatchStr.ClrReDraw();
	mTxtDayDetectStr.ClrReDraw();
	mTxtDayStateStr.ClrReDraw();
	mTxtDayVoltageStr.ClrReDraw();
	
	cCtrl::ClrReDraw();
}//cSysInfoCtrl::ClrReDraw

//process notifier
void cSysInfoCtrl::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	if(pNotifier->GetNotifierId()!=EVT_SYS_ALIVE && 
		pNotifier->GetNotifierId()!=EVT_SYS_RES &&
		pNotifier->GetNotifierId()!=EVT_DAY_NIGHT
	) return;//when it is not notifier to be processed
	
	if(pNotifier->GetNotifierId()==EVT_SYS_ALIVE)
	{
		strcpy(mPowerOffStrBuff,S_I_POWER_OFF_STR);
		strcat(mPowerOffStrBuff,PowerOffReasonToStr(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mLastPowerOffReason));
		if(strcmp(mTxtPowerOffStr.GetText(),mPowerOffStrBuff))//if any state change update it
			{
			mTxtPowerOffStr.SetText(mPowerOffStrBuff);
			}
				
		strcpy(mExceptionStrBuff,S_I_EXCEPT_STR);
		strcat(mExceptionStrBuff,ExceptionReasonToStr(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mLastExceptionReason));
		if(strcmp(mTxtExceptionStr.GetText(),mExceptionStrBuff))//if any state change update it
			{
			mTxtExceptionStr.SetText(mExceptionStrBuff);
			}
				
		strcpy(mResetStrBuff,S_I_RESET_STR);
		strcat(mResetStrBuff,ResetReasonToStr(static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr())->mLastResetReason));
		if(strcmp(mTxtResetStrStr.GetText(),mResetStrBuff))//if any state change update it
			{
			mTxtResetStrStr.SetText(mResetStrBuff);
			}
		
	}//if(pNotifier->GetNotifierId()==EVT_SYS_ALIVE)
	if(pNotifier->GetNotifierId()==EVT_SYS_RES)
	{
		//get heap mem left and convert it to ascii
		itoa(static_cast<sSysResourcesStatus*>(pNotifier->GetDataPtr())->mSysHeapMemLeft,mTmpStrBuf,10); 
		
		strcpy(mHeepFreeStrBuff,S_I_HEAP_FREE_STR);
		strcat(mHeepFreeStrBuff,mTmpStrBuf);//assembly free heap text to display
		if(strcmp(mTxtHeepFreeStr.GetText(),mHeepFreeStrBuff))//if any state change update it
			{
			mTxtHeepFreeStr.SetText(mHeepFreeStrBuff);
			}
		
		//get heap mem minimum left and convert it to ascii
		itoa(static_cast<sSysResourcesStatus*>(pNotifier->GetDataPtr())->mSysHeapMinLeft,mTmpStrBuf,10); 
		
		strcpy(mHeepMinFreeStrBuff,S_I_HEAP_MIN_STR);
		strcat(mHeepMinFreeStrBuff,mTmpStrBuf);//assembly free heap text to display
		if(strcmp(mTxtHeepMinFreeStr.GetText(),mHeepMinFreeStrBuff))//if any state change update it
			{
			mTxtHeepMinFreeStr.SetText(mHeepMinFreeStrBuff);
			}
		
		//get number of notifier dispatch errors and convert it to ascii
		itoa(static_cast<sSysResourcesStatus*>(pNotifier->GetDataPtr())->mTotalDispatchErrorCounter,mTmpStrBuf,10); 
		
		strcpy(mDispatchStrBuff,S_I_DISPATCH_STR);
		strcat(mDispatchStrBuff,mTmpStrBuf);//assembly free heap text to display
		if(strcmp(mTxtDispatchStr.GetText(),mDispatchStrBuff))//if any state change update it
			{
			mTxtDispatchStr.SetText(mDispatchStrBuff);
			}
	}//if(pNotifier->GetNotifierId()==EVT_SYS_RES)
	if(pNotifier->GetNotifierId()==EVT_DAY_NIGHT)
	{
		//assembly day state string and displya if there is any state change
		strcpy(mDayStateStrBuff,S_I_DAY_STATE_STR);
		strcat(mDayStateStrBuff,DayStateToStr(static_cast<sDayStateData*>(pNotifier->GetDataPtr())->mDayState));
		if(strcmp(mTxtDayStateStr.GetText(),mDayStateStrBuff))//if any state change update it
			{
			mTxtDayStateStr.SetText(mDayStateStrBuff);
			}
		
		//assembly day state detector raw reading
		//get number of notifier dispatch errors and convert it to ascii
		itoa(static_cast<sDayStateData*>(pNotifier->GetDataPtr())->mRawFotoResistorVolt,mTmpStrBuf,10); 
				
		strcpy(mDayStateVoltageStrBuff,S_I_DAY_VOLTAGE_STR);
		strcat(mDayStateVoltageStrBuff,mTmpStrBuf);
		if(strcmp(mTxtDayVoltageStr.GetText(),mDayStateVoltageStrBuff))//if any state change update it
			{
			mTxtDayVoltageStr.SetText(mDayStateVoltageStrBuff);
			}
	}//if(pNotifier->GetNotifierId()==EVT_DAY_NIGHT)
	
}//cSysInfoCtrl::ProcessNotifier
