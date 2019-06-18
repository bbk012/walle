/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_rightarm.hpp
* Description: Robot arms manager - controls right arm
* Author:      Bogdan Kowalczyk
* Date:        02-Jan-2014
* Note:
* History:
*              02-Jan-2014 - Initial version created
*********************************************************************************************************
*/
#ifndef MNG_RIGHTARM_HPP_
#define MNG_RIGHTARM_HPP_

#include "mng.hpp"


#define  RIGHT_ARM_PUBLISHER_SEND_Q_SIZE 	5
#define  RIGHT_ARM_SUBSCRIBER_REC_Q_SIZE 	5
#define  RIGHT_ARM_THREAD_STACK_SIZE		128
#define  RIGHT_ARM_THREAD_PRIORITY   		11

class cRightArmMngr:public cMngBasePublisherSubscriber<RIGHT_ARM_PUBLISHER_SEND_Q_SIZE,RIGHT_ARM_SUBSCRIBER_REC_Q_SIZE,RIGHT_ARM_THREAD_STACK_SIZE,RIGHT_ARM_THREAD_PRIORITY>
{
private:
		sMoveArmData mMoveArmData;//store received CMD_MOVE_ARM command details for currently processed command
		//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
		virtual void Run();
};//cRightArmManager

#endif /*MNG_RIGHTARM_HPP_*/
