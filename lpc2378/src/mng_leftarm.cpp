/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_leftarm.cpp
* Description: Robot arms manager - controls lef arm
* Author:      Bogdan Kowalczyk
* Date:        02-Jan-2014
* Note:
* History:
*              02-Jan-2014 - Initial version created
*********************************************************************************************************
*/

#include "mng_leftarm.hpp"
#include "wrp_kernel.hpp"
#include "hw_pwm1.h"
#include "hw_gpio.h"


void cLeftArmMngr::Run()
{
	WORD InCount;//local value of CMD_MOVE_ARM command requested arm position
	
	for(;;)
	{
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		if((pNotifier->GetNotifierId())!= CMD_MOVE_ARM) continue;//skipp any notifier different to CMD_MOVE_ARM
		mMoveArmData=*(static_cast<sMoveArmData*>(pNotifier->GetDataPtr()));//copy notifier data to mMoveArmData
		InCount=mMoveArmData.mArmCount;//get requested arm position
		switch(mMoveArmData.mArmCmdId)//depending on sub-command type execute proper left arm function
		{
		case ARM_SERVO_ON_SCMD_ID:
			ArmServoOn();   
			break;
		case ARM_SERVO_OFF_SCMD_ID:
			ArmServoOff();
			break;
		case MOVE_LEFT_ARM_SCMD_ID:
	 		RunPWMLeftArm(InCount);//move left arm to desired position
	 		Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
			break;
		case MOVE_LEFT_ARM_HOME_SCMD_ID:
			RunPWMLeftArm(LEFT_ARM_HORIZONTAL);
	 		Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
			break;
		case MOVE_LEFT_ARM_SYNC_SCMD_ID:
			if(InCount>=RIGHT_ARM_HORIZONTAL)//that is movement up
			{
				RunPWMLeftArm(LEFT_ARM_HORIZONTAL-(InCount-RIGHT_ARM_HORIZONTAL));
			}
			else //that is movement down
			{
				RunPWMLeftArm(LEFT_ARM_HORIZONTAL+(RIGHT_ARM_HORIZONTAL-InCount));
			}
	 		Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
			break;
		case MOVE_LEFT_ARM_OPPOSITE_SCMD_ID:
			if(InCount>=RIGHT_ARM_HORIZONTAL)//when right is going to be move up above possible left arm move down limit the movement
			{
				if((InCount-RIGHT_ARM_HORIZONTAL)>(RIGHT_ARM_HORIZONTAL-RIGHT_ARM_MAX_DOWN))//count max up movement
					InCount=RIGHT_ARM_HORIZONTAL+(RIGHT_ARM_HORIZONTAL-RIGHT_ARM_MAX_DOWN);
			}
			RunPWMLeftArm(LEFT_ARM_HORIZONTAL+InCount-RIGHT_ARM_HORIZONTAL);
	 		Kernel.TimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait untill desired arms movement is completed
			break;
		case GET_LEFT_ARM_POS_SCMD_ID://see when RSP_MOVE_ARM response is created arm position is released
			break;
		default://sub command id which is not to be executed by cLeftArmMngr - continue receiving of commands  
			continue;//skipp further command processing and start to wait for another command
		}//switch
		
	//when this place is received it means requested command was executed so send RSP_MOVE_ARM response
	//updated with most up to date arm position
	//create and issue response notifier
	cSmartPtr<cTypeNotifier<sMoveArmData> > pRspNotifier = new cTypeNotifier<sMoveArmData>(RSP_MOVE_ARM,GetThreadId(),NT_HND_NORMAL_PRT);
	(pRspNotifier->GetData()).mArmCmdId=mMoveArmData.mArmCmdId;//update information about executed sub-command
	(pRspNotifier->GetData()).mArmCount=GetLeftArmPosition();//get curent position of the arm
	Post(pRspNotifier);//post RSP_MOVE_ARM response	
	}//for
}//cLeftArmMngr::Run()
