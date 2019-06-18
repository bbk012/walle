/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_window.hpp
* Description: Window control for Walle Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        24-Sep-2017
* Note:
* History:
*              24-Sep-2017 - Initial version created
*********************************************************************************************************
*/
#ifndef LIB_G_WINDOW_HPP_
#define LIB_G_WINDOW_HPP_

#include "lib_g_text.hpp"
#include "mw_notifier.hpp"
#include "mw_smart_ptr.hpp"
#include "lib_g_ctrl.hpp"

//window coordinates - window always occupies whole screen
#define WIN_TOP_LEFT_X 		0
#define WIN_TOP_LEFT_Y 		0
#define WIN_BOT_RIGHT_X		129
#define WIN_BOT_RIGHT_Y		129

//maximum number of controls a window can have
#define MAX_WINDOW_CTRLS	15

//Y coordinate of the title left corner (x coordinate is calculated to center the text
#define WIN_TITLE_X			2

class cWindow
{
private:
	BYTE mFirstNotifierReceived;//setup to TRUE when windows got first notifier required to first time update its content
	BYTE mReDraw;//set to true when window should be drawn/re-drawn with all its controls, false redraw only changed controls
	cTxtCtrl mWinTitle;//storage for Window title if any
	WORD mFillColor;//color of window fill if any requested
	WORD mBrdColor;//color of window's border line
	BYTE mFill;// set to FILL when window drawing fills rectangle
	cCtrl *mWindowCtrls[MAX_WINDOW_CTRLS];//storage for pointers to window's ctrls
	void DrawTitle();//draw window title on the top centering text
public:
	cWindow();//setup default parameters of the Window
	void SetFirstNotifierReceived(){mFirstNotifierReceived=TRUE;};
	BYTE isFirstNotifierReceived(){return mFirstNotifierReceived;};
	void SetReDraw();//set parent window and all its controls to redraw request
	void ClrReDraw();//set parent window and its all controls as already redrawn
	virtual ~cWindow(){};//virtual destructor to not have warning because clas has virtual functions
	void SetWinTitle(char *pString){mWinTitle.SetText(pString);};//setup Window title string
	char *GetWinTitle(){return mWinTitle.GetText();};//get window title string
	BYTE GetWinTitleLength(){return mWinTitle.GetTextLength();};//return length of the window title string
	
	//set font size used for window title
	void SetWinTitleFontSize(BYTE inFontSize){mWinTitle.SetFontSize(inFontSize);};
	
	//set foreground color used for window title
	void SetWinTitleFrgColor(WORD inColor){mWinTitle.SetFrgColor(inColor);};
	
	//set get bacground color used for window title
	void SetWinTitleBckColor(WORD inColor){mWinTitle.SetBckColor(inColor);};

	//set up if color filing is used when window is draw
	void UseFillColor(){mFill=FILL;};
	void DontUseFillColor(){mFill=NOFILL;};
	
	//set or get window's fill color
	void SetFillColor(WORD inColor){mFillColor=inColor;};
	WORD GetFillColor(){return mFillColor;};
	
	//set or get window's border color
	void SetBrdColor(WORD inColor){mBrdColor=inColor;};
	WORD GetBrdCOlor(){return mBrdColor; };
	
	//add or remove specified control
	void AddCtrl(cCtrl &inCtrl);
	void RemoveCtrl(cCtrl &inCtrl);
	
	virtual void Draw();//draws window on LCD
	virtual void Clear();//removes whole window from the LCD
	virtual void ProcessNotifier(cSmartPtr<cNotifier> pNotifier);//process notifier
};//cWindow



#endif /*LIB_G_WINDOW_HPP_*/
