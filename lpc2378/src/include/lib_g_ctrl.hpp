/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_ctrl.hpp
* Description: Base class for all window's controls in Wall-e Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        07-Oct-2017
* Note:
* History:
*              07-Oct-2017 - Initial version created
*********************************************************************************************************
*/
#ifndef LIB_G_CTRL_HPP_
#define LIB_G_CTRL_HPP_

#include "ctr_lcd.h"
#include "lib_error.h"
#include "mw_notifier.hpp"
#include "mw_smart_ptr.hpp"

#define NO_CTRL_ID 0xFF;//this value is used to mark undefined/unasigned ID

class cCtrl
{
private:
	BYTE	mDrawn;//state of the control on the sccreen TRUE when already drawn FALSE otherwise 
	BYTE 	mReDraw;//set to true when control should be drawn/re-drawn because of changes
	sPoint 	mLeftCorner;//keeps coordinates of the left corner point in LCD coordinate system
	WORD 	mFrgColor;//foreground color to use when control is displayed
	WORD 	mBckColor;//bacground color to use when control is displayed
public:
	cCtrl();//default constractor, creates empty cWindowCtrl class with none control to be desplayed
	virtual ~cCtrl(){};//do nothing virtual destructor in case of static allocation not required at all
	
	virtual void SetDrawn(){mDrawn=TRUE;};
	virtual void ClrDrawn(){mDrawn=FALSE;};
	virtual BYTE GetDrawn(){return mDrawn;};
	
	virtual void SetReDraw(){mReDraw=TRUE;};//set control state to re-draw by next call to Draw() method
	virtual void ClrReDraw(){mReDraw=FALSE;};//clr re-draw state for the control
	virtual BYTE GetReDraw(){return mReDraw;}//get value of ReDraw state 
	
	//set and get Text Control string
	void SetLeftCorner(sPoint &inLeftTextCorner){mLeftCorner.mX=inLeftTextCorner.mX;mLeftCorner.mY=inLeftTextCorner.mY;};
	sPoint &GetLeftCorner(){return mLeftCorner;};
	
	//set get foreground color
	void SetFrgColor(WORD inColor){mFrgColor=inColor;};
	WORD GetFrgColor(){return mFrgColor;};
	
	//set get bacground color
	void SetBckColor(WORD inColor){mBckColor=inColor;};
	WORD GetBckColor(){return mBckColor;};
	//pure virtual ctrl functions cannot be defined as Run()=0 because g++ generates very big size of code
	//those need to be redefined in derived ctrl otherwise exception is generated when called
	virtual void Draw(){WIN_CTRL_NO_VIRTUAL_DEFINED;};//draw control on the screen 
	virtual void Clear(){WIN_CTRL_NO_VIRTUAL_DEFINED;};//clear whole control from the LCD
	virtual void ProcessNotifier(cSmartPtr<cNotifier> pNotifier){};//process notifier
	
};//cCtrl





#endif /*LIB_G_CTRL_HPP_*/
