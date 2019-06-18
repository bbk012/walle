/*
*********************************************************************************************************
*                                            LPC 2378
*
*           
*              (c) Copyright 2013, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        res_list.h: 
* Description: List of tasks and other critical resources in the system and their priorities
* Author:      Bogdan Kowalczyk
* Date:        15-Dec-2013
* Note:
* History:
*              15-Dec-2013 - Initial version created
*              25-Jan-2015 - Added Brain, Context and Exe managers
*              02-Jan-2017 - Added Rmt (Remote) manager
*              13-Jan-2018 - Added VRM (Voice Recognition Module) manager
*********************************************************************************************************
*/

#ifndef TSK_LIST_H_
#define TSK_LIST_H_

//IMPORTANT OS_MAX_EVENTS! 
//In addition to the below task list, this file also includes list of OS_EVENT blocks
//utilized by Wall-e to setup correctly OS_MAX_EVENTS limit in the os_cfg.h
//Every queue, message box, or semaphore created in the system oocupies one OS_EVENT block
//List of resourcies occuping OS_EVENT:

// 10 x OS_EVENT blocks because of resources defined below the so named Manager Layer
//Pwm1Mutex - access to PWM1 (servo control) is protected by mutex
//AdcMutex - access to ADC is protected with mutex
//MemMutex - mutex in Kernel to protect new and delete and make them thread safety
//MemMgrMutex - mutex in Kernel which protects reference counting operation for all objects (Notifiers)
//cDispatcher.m_PublisherMutex - protects access to publisher tables
//cDispatcher.m_SubscriberMutex - protects access to subscriber tables
//LeftTrackMailbox - used to communicate from track Port2Isr (track detector) to TrackControlTask
//RightTrackMailbox - used to communicate from track Port2Isr (track detector) to TrackControlTask
//CtxMutext - mutext in CtxMngr to protect simultanous access to the context data
//uPAdcMutex - mutex to protect exclusive access to uP ADC (uPAdcMutex)

//In addition to above every manager which derives from cMngBasePublisherSubscriber has transmit 
//and receive queues so 13 managers gives 13*2=26 OS_EVENT blocks because of its queues

// This list is only used as reference not included anywhere.
// It works like one common place to know what are the tasks in the system because they are
// covered under managers but also as stand alone uCOS-II tasks

// typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */

// There is stand alone exception stack in uCOS-II for ARM of the bleow defined size
// Stack Size: OS_CPU_EXCEPT_STK_SIZE    		128        /* Default exception stack size is 128 OS_STK entries */

//IMPORTANT! Maximum number of subscribers and publishers is defined in wrp_kernel.hpp file as
//           NO_OF_PUBLISHERS  					13
//           NO_OF_SUBSCRIBERS					13


// *************************************************************************************************************
//Task:			
//Priority:
//Stack Size:
//Send Q Size:
//Rec Q Size:	

// *************************************************************************************************************
//Task:			cKernel::m_DispatcherThread - dispatches notifiers
//Priority:		DISPATCHER_THREAD_PRIORITY 		3
//Stack Size: 	DISPATCHER_THREAD_STACK_SIZE = 	OS_TASK_STACK_SIZE	= 128 x OS_STK

// *************************************************************************************************************
//Task:			mng_motion
//Priority:		MOTION_THREAD_PRIORITY   		5
//Stack Size:	MOTION_THREAD_STACK_SIZE		128
//Send Q Size:	MOTION_PUBLISHER_SEND_Q_SIZE 	5
//Rec Q Size:	MOTION_SUBSCRIBER_REC_Q_SIZE 	5

// *************************************************************************************************************
//Task:			LeftTrackControlTask
//Priority:		LEFT_TRACK_CTRL_TASK_PRIORITY  	6
//Stack Size: 	LEFT_TRACK_CTRL_STACK_SIZE 		OS_TASK_STACK_SIZE	= 128 x OS_STK
 
// *************************************************************************************************************
//Task:			RightTrackControlTask
//Priority:		RIGHT_TRACK_CTRL_TASK_PRIORITY  7
//Stack Size: 	RIGHT_TRACK_CTRL_STACK_SIZE 	OS_TASK_STACK_SIZE = 128 x OS_STK

// *************************************************************************************************************
//Task:			mng_leftarm
//Priority:		LEFT_ARM_THREAD_PRIORITY  		10
//Stack Size:   LEFT_ARM_THREAD_STACK_SIZE		128
//Send Q Size:	LEFT_ARM_PUBLISHER_SEND_Q_SIZE 	5
//Rec Q Size:	LEFT_ARM_SUBSCRIBER_REC_Q_SIZE 	5

// *************************************************************************************************************
//Task:			mng_rightarm
//Priority:		RIGHT_ARM_PUBLISHER_SEND_Q_SIZE	11
//Stack Size:   RIGHT_ARM_THREAD_STACK_SIZE		128
//Send Q Size:	RIGHT_ARM_PUBLISHER_SEND_Q_SIZE	5
//Rec Q Size:	RIGHT_ARM_SUBSCRIBER_REC_Q_SIZE	5

// *************************************************************************************************************
//Task:			mng_rtc
//Priority:		RTC_THREAD_PRIORITY   			15
//Stack Size:	RTC_THREAD_STACK_SIZE			128
//Send Q Size:	RTC_PUBLISHER_SEND_Q_SIZE 		5
//Rec Q Size:	RTC_SUBSCRIBER_REC_Q_SIZE 		5

// *************************************************************************************************************
//Task:			mng_keypad
//Priority:		KEYPAD_THREAD_PRIORITY   		20	
//Stack Size:	KEYPAD_THREAD_STACK_SIZE		128
//Send Q Size:	KEYPAD_PUBLISHER_SEND_Q_SIZE 	5
//Rec Q Size:	KEYPAD_SUBSCRIBER_REC_Q_SIZE 	5

// *************************************************************************************************************
//Task:			mng_ctx	
//Priority:		CTX_THREAD_PRIORITY   			21
//Stack Size:	CTX_THREAD_STACK_SIZE			128
//Send Q Size:	CTX_PUBLISHER_SEND_Q_SIZE 		5
//Rec Q Size:	CTX_SUBSCRIBER_REC_Q_SIZE 		5

// *************************************************************************************************************
//Task:			mng_exe	
//Priority:		EXE_THREAD_PRIORITY   			23
//Stack Size:	EXE_THREAD_STACK_SIZE			128
//Send Q Size:	EXE_PUBLISHER_SEND_Q_SIZE 		5
//Rec Q Size:	EXE_SUBSCRIBER_REC_Q_SIZE 		5


// *************************************************************************************************************
//Task:			mng_brain	
//Priority:		BRAIN_THREAD_PRIORITY   		24
//Stack Size:	BRAIN_THREAD_STACK_SIZE			128
//Send Q Size:	BRAIN_PUBLISHER_SEND_Q_SIZE 	5
//Rec Q Size:	BRAIN_SUBSCRIBER_REC_Q_SIZE 	5

// *************************************************************************************************************
//Task:			mng_monitor	
//Priority:		MONITOR_THREAD_PRIORITY   		26
//Stack Size:	MONITOR_THREAD_STACK_SIZE		128
//Send Q Size:	MONITOR_PUBLISHER_SEND_Q_SIZE 	5
//Rec Q Size:	MONITOR_SUBSCRIBER_REC_Q_SIZE 	5

// *************************************************************************************************************
//Task:			mng_display
//Priority:		DISPLAY_THREAD_PRIORITY   		30
//Stack Size:	DISPLAY_THREAD_STACK_SIZE		128
//Send Q Size:	DISPLAY_PUBLISHER_SEND_Q_SIZE 	5 
//Rec Q Size:	DISPLAY_SUBSCRIBER_REC_Q_SIZE 	5

// *************************************************************************************************************
//Task:			mng_indicator
//Priority:		INDICATOR_THREAD_PRIORITY   	35
//Stack Size:	INDICATOR_THREAD_STACK_SIZE		128
//Send Q Size:	INDICATOR_PUBLISHER_SEND_Q_SIZE 5
//Rec Q Size:	INDICATOR_SUBSCRIBER_REC_Q_SIZE 5

// *************************************************************************************************************
//Task:			mng_rmt
//Priority:		RMT_THREAD_PRIORITY	   			40
//Stack Size:	RMT_THREAD_STACK_SIZE			128
//Send Q Size:	RMT_PUBLISHER_SEND_Q_SIZE		5
//Rec Q Size:	RMT_SUBSCRIBER_REC_Q_SIZE 		5

// *************************************************************************************************************
//Task:			mng_vrm
//Priority:		VRM_THREAD_PRIORITY	   			42
//Stack Size:	VRM_THREAD_STACK_SIZE			128
//Send Q Size:	VRM_PUBLISHER_SEND_Q_SIZE		5
//Rec Q Size:	VRM_SUBSCRIBER_REC_Q_SIZE 		5

// *************************************************************************************************************
//Task:			OSTaskIdle - run by ::OSInit()from cKernelInit::cKernelInit()
//                           do nothing taks, counts only its execution for eventual uP utilization figures
//Priority: 	OS_IDLE_PRIO       				63
//Stack Size: 	OS_TASK_IDLE_STK_SIZE    		64  //64 x OS_STK


#endif /*TSK_LIST_H_*/
