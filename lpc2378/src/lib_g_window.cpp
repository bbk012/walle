/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_g_window.cpp
* Description: Window for Walle Graphic Library
* Author:      Bogdan Kowalczyk
* Date:        24-Sep-2017
* Note:
* History:
*              24-Sep-2017 - Initial version created
*********************************************************************************************************
*/

#include "lib_g_window.hpp"
//#include "hw_uart.h"

//setup default parameters of the Window
cWindow::cWindow():mWinTitle("",0,0)
{
	mFirstNotifierReceived=FALSE;//initially there is not any notifier received/processed
	mReDraw=TRUE;//first call to Draw for a window just created should draw it

	mFillColor=BLACK;//color of window fill if any requested
	mBrdColor=WHITE;//color of window's border line
	mFill=NOFILL;// set to FILL when window drawing fills rectangle
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//clear all pointers to controls
		mWindowCtrls[i]=(cCtrl *)0;
}//cWindow::cWindow

//draw window title on the top centering text
void cWindow::DrawTitle()
{
	sPoint TitleLeftCorner;//coordinate of the centered left corner of the title
	BYTE TitleLengthInPixels=mWinTitle.GetTextLength()*LCDFontWidth(LCDSizeToFont(mWinTitle.GetFontSize()));
		
	//setup title text left coordinate so text is centered
	TitleLeftCorner.mY=(WIN_BOT_RIGHT_X-TitleLengthInPixels)/2;
	TitleLeftCorner.mX=WIN_TITLE_X;
	mWinTitle.SetLeftCorner(TitleLeftCorner);
	
	mWinTitle.Draw();//draw centered title
	
}//cWindow::DrawTitle

//set window state to re-draw
//all window's controls are set to reDraw as well
void cWindow::SetReDraw()
{
	mReDraw=TRUE;//request to redraw parent window
	mWinTitle.SetReDraw();//and also its title
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//and it's all controls
		if(mWindowCtrls[i])
			mWindowCtrls[i]->SetReDraw();
}//cWindow::SetReDraw

//clr re-draw state for the window
void cWindow::ClrReDraw()
{
	mReDraw=FALSE;//mark parent window as redrawn
	mWinTitle.ClrReDraw();//and also its title
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//and it's all controls
		if(mWindowCtrls[i])
			mWindowCtrls[i]->ClrReDraw();
}//cWindow::ClrReDraw


//add or remove specified control
void cWindow::AddCtrl(cCtrl &inCtrl)
{
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//go through pointers to controls
	{
		if(mWindowCtrls[i]==(cCtrl*)0)//when empt place on the control list
		{
			mWindowCtrls[i]=&inCtrl;//add ctrl to the window list of controls
			return;//control added do not continue
		}
	}
}//cWindow::AddCtrl

//remove specified control
void cWindow::RemoveCtrl(cCtrl &inCtrl)
{
	int j;//used to shift list left when something is removed from it
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//go through all pointers to controls
	{
		if(mWindowCtrls[i]==&inCtrl)//when specified control find on the list
		{
			mWindowCtrls[i]=(cCtrl *)0;//remove it
			for(j=i;j<MAX_WINDOW_CTRLS-1;j++)//shift rest of the list so there is not empty places inside
			{
				mWindowCtrls[j]=mWindowCtrls[j+1];
			}
			mWindowCtrls[MAX_WINDOW_CTRLS-1]=(cCtrl *)0;//when all pointers shifted so there is no gap put zero at the en
		}
	}
}//cWindow::RemoveCtrl

//removes whole window from the LCD
void cWindow::Clear()
{
	if(mFill==NOFILL)//assure screen is empty if NOFILL setup otherwise filling will erase the content
		LCDClearScreen();//clear screen for case mFill == NOFILL
}//cWindow::Clear

void cWindow::Draw()
{
	if(mReDraw==TRUE)//when redraw window and all its controls requested
	{
		SetReDraw();//set all to be redrawn including all window controls	
		//redraw first window
		BYTE TitleHightInPixels=LCDFontHight(LCDSizeToFont(mWinTitle.GetFontSize()));//get title text hight
	
		Clear();//clear LCD occupied by window
		//draw window
		LCDSetRect(WIN_TOP_LEFT_X,WIN_TOP_LEFT_Y,WIN_BOT_RIGHT_X,WIN_BOT_RIGHT_Y,mFill,mFillColor);
		LCDSetRect(WIN_TOP_LEFT_X,WIN_TOP_LEFT_Y,WIN_BOT_RIGHT_X,WIN_BOT_RIGHT_Y,NOFILL,mBrdColor);
		if(mWinTitle.GetTextLength())//if there is title draw if
			{
			//draw window title bar and title centered
			LCDSetRect(WIN_TOP_LEFT_X,WIN_TOP_LEFT_Y,TitleHightInPixels+2,WIN_BOT_RIGHT_Y,NOFILL,mBrdColor);
			DrawTitle();
			mWinTitle.ClrReDraw();//and also its title is redrawn
			}
		mReDraw=FALSE;//mark that window is redrawn
	}
	//draw all controls depending on their redrawn state
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//go through all controls
	{
		if(mWindowCtrls[i])//if there is any control assigned to the window draw it
		{
			mWindowCtrls[i]->Draw();
		}
	}
	
	mReDraw=FALSE;//window is just drawn should not be re-draw until changed
	mWinTitle.ClrReDraw();//and also its title is redrawn
}//cWindow::Draw


void cWindow::ProcessNotifier(cSmartPtr<cNotifier> pNotifier)
{
	//let all controls owned by window process notifier 
	for (int i=0;i<MAX_WINDOW_CTRLS;i++)//go through all controls
	{
		if(mWindowCtrls[i])//if there is any control assigned to the window let it process notifier
		{
			mWindowCtrls[i]->ProcessNotifier(pNotifier);
		}
	}
}//cWindow::ProcessNotifier



