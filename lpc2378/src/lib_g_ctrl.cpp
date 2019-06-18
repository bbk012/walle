/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_ctrl.cpp
* Description: Base class for all window's controls in Wall-e Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        07-Oct-2017
* Note:
* History:
*              07-Oct-2017 - Initial version created
*********************************************************************************************************
*/

#include "lib_g_ctrl.hpp"

//default constractor, creates empty controls
cCtrl::cCtrl()
{
	mDrawn=FALSE;//when first time created it is not yet displayed
	mReDraw=TRUE;//when first time created should be able to be drawn
	mLeftCorner.mX=0;//setup LCD Left corner as initial position for the corner of the control
	mLeftCorner.mY=0;
	mFrgColor=WHITE;//default foreground color to use (when text is displayed)
	mBckColor=BLACK;//default bacground color to use (when text is displayed)

}//cCtrl::cCtrl()
