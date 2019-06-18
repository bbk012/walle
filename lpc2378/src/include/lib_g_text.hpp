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
#ifndef LIB_G_TEXT_HPP_
#define LIB_G_TEXT_HPP_

#include "ctr_lcd.h"
#include "lib_std.h"
#include "lib_g_ctrl.hpp"

#define MAX_TEXT_SIZE 	22 //maximum number of characters in text control including /0
#define MAX_TIME_STR_BUFF_SIZE 9 //maximum size (max is "HH:MM:SS/0") of the buffer used for hours/minutes convertion to the time string
#define MAX_DATE_STR_BUFF_SIZE 5 //maximum size of the buffer used as temp storage when date string is assembled
#define MAX_BYTE_VAL_STR_SIZE  4 //maximum size of the buffer for BYTE value converted to str

//battery status strings
#define STR_UP_BAT			"uP Batt.[V]:    "
#define STR_UP_MIN_BAT		"Min Value[V]:   "
#define STR_TRACK_BAT		"Track Batt.[V]: "
#define STR_TRACK_MIN_BAT	"Min Value[V]:   "
#define STR_SERVO_BAT		"Servo Batt.[V]: "
#define STR_SERVO_MIN_BAT	"Min Value[V]:   "

//clock & alram date and tiem strings
#define D_T_EMPTY_STR			""
#define D_T_CLOCK_STR			"CLOCK"
#define D_T_DATE_STR			"DATE:"
#define D_T_TIME_STR			"TIME:"
#define D_T_ALARM_STR			"ALARM"
#define D_T_STATE_STR			"STATE:     "
#define D_T_STATE_ON_STR		"ON"
#define D_T_STATE_OFF_STR		"OFF"
#define D_T_TRIGGER_STR			"TRIGGERED: "
#define D_T_TRIGGERED_YES_STR	"YES"
#define D_T_TRIGGERED_NO_STR	"NO"

//sys info strings
#define S_I_REASON_STR			"LAST REASONS"
#define S_I_POWER_OFF_STR		"Power Off: "
#define S_I_EXCEPT_STR			"Exception: "
#define S_I_RESET_STR			"Reset:     " 
#define S_I_SYS_RES_STR			"SYSTEM RES."
#define S_I_HEAP_FREE_STR		"Free:      "
#define S_I_HEAP_MIN_STR		"Min Free:  "
#define S_I_DISPATCH_STR		"Disp.Err.: "
#define S_I_DAY_DETECT_STR		"DAY DETECTION"
#define S_I_DAY_STATE_STR		"State:   "
#define S_I_DAY_VOLTAGE_STR		"Voltage: "


class cTxtCtrl:public cCtrl
{
private:
	char mText[MAX_TEXT_SIZE];//buffer for text storage
	BYTE mFontSize;//font size to be used by text
public:
	cTxtCtrl(char *pString, BYTE InX, BYTE InY);//constractor, creates text control with defined content but not yet drawn on the screen
	
	void SetText(char *pString);//setup TxtCtrl string with new string
	char *GetText(){return mText;};
	BYTE GetTextLength(){return (BYTE)strlength(mText);};//return length of the string storing the text of the Text Control
	
	//set get font size used by Draw when Text Control string is displayed
	void SetFontSize(BYTE inFontSize){mFontSize=inFontSize;};
	BYTE GetFontSize(){return mFontSize;};
	
	void Draw();//puts mText into display using parameters defined prioir to Draw() call

	//clear text on LCD i.e. puts spaces instead of displayed characters
	void Clear();
	
};//cTxtCtrl

//class used to display time
class cTimeTxtCtrl:public cTxtCtrl
{
private:
	BYTE mTogleSecondsFlag; //used to turn on turn off hours to minutes separator ":"
	char mTmpStrBuff[MAX_TIME_STR_BUFF_SIZE];//bufferer used to convert hours and minutes to string before those are displayed
	char mTimeStrBuff[MAX_TIME_STR_BUFF_SIZE];//buffer to hold assembled time string
public:
	cTimeTxtCtrl(char *pString, BYTE InX, BYTE InY);//constructor of the control
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);
};//cTimeTxtCtrl

//class used to display date
class cDateTxtCtrl:public cTxtCtrl
{
private:
	char mTmpStrBuff[MAX_DATE_STR_BUFF_SIZE];//bufferer used to convert hours and minutes to string before those are displayed
	char mDateStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled date string
public:
	cDateTxtCtrl(char *pString, BYTE InX, BYTE InY);//constructor of the control
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);
};//cTimeTxtCtrl

//class used to display voltage values of the batteries
class cBatteryTxtCtrl:public cCtrl
{
private:
	char mByteValueStrBuff[MAX_BYTE_VAL_STR_SIZE];//buffer used by itoa conversion
	char mTextBuff[MAX_TEXT_SIZE];//buffer for text storage
	//text controls used to display corseponding batery voltage state
	cTxtCtrl mTxtuPBatVoltage;
	cTxtCtrl mTxtuPBatMinVoltage;
	cTxtCtrl mTxtTrackBatVoltage;
	cTxtCtrl mTxtTrackBatMinVoltage;
	cTxtCtrl mTxtServoBatVoltage;
	cTxtCtrl mTxtServoBatMinVoltage;
	//storage of last displayed votage value to avoid display refrashing when nothing changed
	BYTE muPBatVoltage;
	BYTE muPBatMinVoltage;
	BYTE mTrackBatVoltage;
	BYTE mTrackBatMinVoltage;
	BYTE mServoBatVoltage;
	BYTE mServoBatMinVoltage;
	
public:
	cBatteryTxtCtrl(BYTE InX, BYTE InY);//creates control with defined top left corner position but not yet drawn on the screen
	
	virtual void SetDrawn();
	virtual void ClrDrawn();
		
	virtual void SetReDraw();//set control state to re-draw by next call to Draw() method
	virtual void ClrReDraw();//clr re-draw state for the control
	 
	
	void Draw();//draw control on the screen 
	void Clear();//clear whole control from the LCD
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cBatteryTxtCtrl

//class used to display details of wall-e's clock and alarm
class cDateTimeTxtCtrl:public cCtrl
{
private:
	char mClkDateStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled date string for clock
	char mClkTimeStrBuff[MAX_TIME_STR_BUFF_SIZE];//buffer to hold assembled time string for clock
	char mAlarmDateStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled date string for alarm
	char mAlarmTimeStrBuff[MAX_TIME_STR_BUFF_SIZE];//buffer to hold assembled alarm string for clock
	char mTmpStrBuff[MAX_TEXT_SIZE];//temporary buffer used during various string assembly tasks of the control

	//text controls used to display date & time information for clock and alarm notifiers
	cTxtCtrl mTxtClkStr;
	cTxtCtrl mTxtClkDateStr;
	cTxtCtrl mTxtClkDate;
	cTxtCtrl mTxtClkTimeStr;
	cTxtCtrl mTxtClkTime;
	
	cTxtCtrl mTxtAlarmStr;
	cTxtCtrl mTxtAlarmStateStr;
	cTxtCtrl mTxtAlarmTriggerStr;
	cTxtCtrl mTxtAlarmDateStr;
	cTxtCtrl mTxtAlarmDate;
	cTxtCtrl mTxtAlarmTimeStr;
	cTxtCtrl mTxtAlarmTime;
	
public:
	cDateTimeTxtCtrl(BYTE InX, BYTE InY);//creates control with defined top left corner position but not yet drawn on the screen
	
	virtual void SetDrawn();
	virtual void ClrDrawn();
		
	virtual void SetReDraw();//set control state to re-draw by next call to Draw() method
	virtual void ClrReDraw();//clr re-draw state for the control
	 
	
	void Draw();//draw control on the screen 
	void Clear();//clear whole control from the LCD
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cDateTimeTxtCtrl

//class used to display details Walle system information
class cSysInfoCtrl:public cCtrl
{
private:
	//buffers
	char mTmpStrBuf[MAX_TEXT_SIZE];//string buffer used during various conversion to descriptive name
	char mPowerOffStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled power off reason
	char mExceptionStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled exception reason
	char mResetStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled reset reason
	char mHeepFreeStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled heap current availiable size
	char mHeepMinFreeStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled heap minimali size so far
	char mDispatchStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled notiffier dispatch error
	char mDayStateStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled day/night state
	char mDayStateVoltageStrBuff[MAX_TEXT_SIZE];//buffer to hold assembled voltage level of the day/night detector
	
	
	//text controls used to display sys informations
	cTxtCtrl mTxtLastReasonStr;
	cTxtCtrl mTxtPowerOffStr;
	cTxtCtrl mTxtExceptionStr;
	cTxtCtrl mTxtResetStrStr;
	
	cTxtCtrl mTxtSysResStr;
	cTxtCtrl mTxtHeepFreeStr;
	cTxtCtrl mTxtHeepMinFreeStr;
	cTxtCtrl mTxtDispatchStr;
	
	cTxtCtrl mTxtDayDetectStr;
	cTxtCtrl mTxtDayStateStr;
	cTxtCtrl mTxtDayVoltageStr;
	
	//convert num value to descriptive name
	char* PowerOffReasonToStr(WORD InLastPowerOffReason);
	char* ExceptionReasonToStr(WORD InLastExceptionReason);
	char* ResetReasonToStr(BYTE LastResetReason);
	char* DayStateToStr(BYTE InDayState);
	
public:
	cSysInfoCtrl(BYTE InX, BYTE InY);//creates control with defined top left corner position but not yet drawn on the screen
	
	virtual void SetDrawn();
	virtual void ClrDrawn();
		
	virtual void SetReDraw();//set control state to re-draw by next call to Draw() method
	virtual void ClrReDraw();//clr re-draw state for the control
	 
	
	void Draw();//draw control on the screen 
	void Clear();//clear whole control from the LCD
	void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
	
};//cSysInfoCtrl



#endif /*LIB_G_TEXT_HPP_*/
