/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_leftarm.hpp
* Description: Robot arms manager - controls lef arm
* Author:      Bogdan Kowalczyk
* Date:        02-Jan-2014
* Note:
* History:
*              02-Jan-2014 - Initial version created
*********************************************************************************************************
*/


#ifndef MNG_LEFTARM_HPP_
#define MNG_LEFTARM_HPP_

#include "mng.hpp"


#define  LEFT_ARM_PUBLISHER_SEND_Q_SIZE 	5
#define  LEFT_ARM_SUBSCRIBER_REC_Q_SIZE 	5
#define  LEFT_ARM_THREAD_STACK_SIZE			128
#define  LEFT_ARM_THREAD_PRIORITY   		10

class cLeftArmMngr:public cMngBasePublisherSubscriber<LEFT_ARM_PUBLISHER_SEND_Q_SIZE,LEFT_ARM_SUBSCRIBER_REC_Q_SIZE,LEFT_ARM_THREAD_STACK_SIZE,LEFT_ARM_THREAD_PRIORITY>
{
private:
		sMoveArmData mMoveArmData;//store received CMD_MOVE_ARM command details for currently processed command
      //pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
      virtual void Run();
};//cLeftArmManager

#endif /*MNG_LEFTARM_HPP_*/
