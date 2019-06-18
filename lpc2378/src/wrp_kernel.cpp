/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        wrp_kernel.cpp
* Description: objective wrapper to uCOS-II - defines singleton kernel which is OS for the system
* Author:      Bogdan Kowalczyk
* Date:        4-November-2008
* Note:
* History:
*              4-November-2008 - Initial version created
*********************************************************************************************************
*/

#include "wrp_kernel.hpp"
#include "hw_gpio.h"
#include "ctr_lcd.h"
#include "hw_spi.h"
#include "lib_memalloc.h"
#include "hw_timer.h"
#include "hw_pwm1.h"
#include "hw_rtc.h"
#include "lib_std.h"
#include "hw_uart.h"
#include "hw_uart1.h"
#include "hw_adc.h"
#include "hw_sram.h"
#include "hw_wdt.h"




//IMPORTANT!!
//m_KernelInit this is empty initializer  class used to assure ::OSInit() is called before any other Kernel member
//object is constructed - otherwise cMutex below will throw an exception as not initilaized
//That is because member objects constructor are called in the order of declaration
//IMPORTANT! Timers are initialized in the highest priority task which is cDispatherThread::Run()
//           That is because uCOS-II require sheduling to be run after ::OSInit call which starts tasks
cKernelInit::cKernelInit()
{
	InitStaticRam();//check and initailize static RAM accordingly
	InitGPIO(); //initialize GPIO and turn port to control uP board power as on
	
	i_alloc(); //initialize main heap
	::OSInit();//initialize uCOS-II before any other member of cKernel uses uCOS-II calls
	//IMPORTANT! OS initialization need to be there because some hw resources like ADC protected by mutex below
	
	InitRTC();//initialize RTC or leave it as it was setup previously (RTC battery backup)
	InitSpiLcd(); //Initialize SPI connection to LCD
	InitSpiAdc(); //Initialize SPI connection to ADC
	InituPAdc();//Initialize ADC inside the uP
	InitLcd();// Init LCD
	InitUart0();//initialize Uart 0 used for external remote serial terminal (commands)
	InitUart1();//initialize Uart 1 used for internal communication with voice recognition module
	
	Backlight(BKLGHT_LCD_ON);//turn on LCD backlight
	LCDClearScreen();//clear the screen	
	LCDPutStr(WALLE_RUN, 40, 10, SMALL, WHITE, BLACK);//display WALL-E SW version on it's LCD
	LCDPutStr(WALLE_OS_VERSION, 50, 10, SMALL, WHITE, BLACK);
	LCDPutStr(WALLE_VERSION, 60, 10, SMALL, WHITE, BLACK);//display WALL-E SW version on it's LCD
	if(ReadLastResetReason()== SW_RESET)
	{//if restart because program change request display message about that
		LCDPutStr(WALLE_PRG_CHANGE, 70, 10, SMALL, WHITE, BLACK);
	}
	LCDDebugMessage(WALLE_PRG_NO,GetWalleProgramToExecute(),80,10,0,0);
	LedOff();//turn off LED through GPIO to indicate system run

}//cKernelInit

void cDispatchThread::Run()
{
		InitTimer2();//initialize Timer 2 (uCOS-II tick function) interrupts need to be done in the highest priority task which is dispatcher
		InitTimer0();// initialize Timer 0 to be capable to measure ultraconic sensor echo pulse duration
		InitTimer1();//timer 1 interrupts determine ARS sampling period to integrate turn angle
		InitTimer3();//initialize Timer 3 interrupts to generate PWMs for motor control
		InitPWM1();//initialize PWM1 for servos
		InitPort2Int();//initialize interrupt handling comming from tracks transoptors
		InitWatchdog();//intialize watchdog
		
		//put head and arms into default positions
		ArmServoOn();//turn on servos to assure InitPWM1() values setup head and arms into default positions
		HeadServoOn();
		OSTimeDlyHMSM(0,0,0,ARM_MOVE_DELAY);//wait until head and arms move done and stabilized
		HeadServoOff();
		ArmServoOff();
				
		Kernel.Delay(RAND_SEED_SETUP_DELAY_TCIKS);//delay some time to before you get seed for rand generator
		srand(GetTimer3CounterValue());//intialize seed for random generator

		for(;;)//endless loop
		{
		Kernel.Delay(DISPATCH_PERIOD_IN_OS_TICKS);
		Kernel.Dispatcher.Dispatch();//dispatch Notifiers from all publishers to subscribers
		}//cDispatchThread::Run()	 
}//cDispatchThread::Run

//private constructor used to run notifiers dispatching thread
//it is private to have cKernel to be a singleton
cKernel::cKernel()
      {
    	  m_DispatcherThread.Create((OS_STK *)&m_DispatcherThreadStack[0],(OS_STK *)&m_DispatcherThreadStack[DISPATCHER_THREAD_STACK_SIZE-1],DISPATCHER_THREAD_PRIORITY);
      }//Initialize uCOS





