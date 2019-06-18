/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_brain.cpp
* Description: Container for Wall-e execution manager (cExeMngr) and context manager (cCtxMngr)
* Author:      Bogdan Kowalczyk
* Date:        25-Jan-2015
* History:
* 25-Jan-2015 - Initial version created
*********************************************************************************************************
*/

#ifndef MNG_BRAIN_HPP_
#define MNG_BRAIN_HPP_

#include "mng.hpp"
#include "mng_ctx.hpp"
#include "mng_exe.hpp"

#define  BRAIN_PUBLISHER_SEND_Q_SIZE 	5
#define  BRAIN_SUBSCRIBER_REC_Q_SIZE 	5
#define  BRAIN_THREAD_STACK_SIZE		128
#define  BRAIN_THREAD_PRIORITY   		24

class cBrainMngr:public cMngBasePublisherSubscriber<BRAIN_PUBLISHER_SEND_Q_SIZE,BRAIN_SUBSCRIBER_REC_Q_SIZE,BRAIN_THREAD_STACK_SIZE,BRAIN_THREAD_PRIORITY>
{
private:
      //pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
      virtual void Run();
public:
	cBrainMngr():ExeMngr(this,&CtxMngr),CtxMngr(this,&ExeMngr){};//Exe and Ctx managers knows the brain they belong into and each other
	cExeMngr ExeMngr; //Brain contains Execution Manager
	cCtxMngr CtxMngr; //Brain contains Context Manager
	
};//cBrainMngr

#endif /*MNG_BRAIN_HPP_*/
