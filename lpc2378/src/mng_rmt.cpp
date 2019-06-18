/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_rmt.cpp
* Description: Robot's remote access manager
* Author:      Bogdan Kowalczyk
* Date:        2-Jan-2017
* Note:
* History:
*              2-Jan-2017 - Initial version created
*********************************************************************************************************
*/
#include "mng_rmt.hpp"
#include "wrp_kernel.hpp"

#include "hw_uart.h"
#include "lib_dbg.h"
#include "lib_std.h"
#include "hw_gpio.h"
#include "hw_timer.h"
#include "hw_sram.h"
#include "hw_wdt.h"

//--------------------------------------------------------------
#define MNG_RMT_START_TIMEOUT  5 //delay in seconds before debugger prompt is dislplayed

//Remote control output strings
#define STR_CMD_PROMPT			"\n>>>"
#define STR_CMD_QUIT			"\nCMD QUIT: WALL-e turned off!\n"
#define STR_CMD_UNKNOWN			"\nUNKNOWN COMMAND\n"
#define STR_DBG_LEVEL			"\nDBG LEVEL: "

#define STR_DBG_TRACE_N			"\nDBG ERROR: TRACE WRONG N VALUE: "
#define STR_DBG_TRACE_PTRN_LNG  "\nDBG ERROR: TRACE PATTERN TOO LONG\n"
#define STR_DBG_TRACE_PTRN_NO   "\nDBG ERROR: TRACE NO PATTERN DEFINED\n"
#define STR_DBG_TRACE_DSP		"\nDBG TRACES AT:"

#define STR_DBG_STOP_PTRN_LNG   "\nDBG ERROR: STOP PATTERN TOO LONG\n"
#define STR_DBG_STOP_PTRN_NO    "\nDBG ERROR: STOP NO PATTERN DEFINED\n"
#define STR_DBG_STOP_N			"\nDBG ERROR: STOP WRONG N VALUE: "
#define STR_DBG_STOP_DSP		"\nDBG STOPS AT:"
#define STR_DBG_STOP_NOT_IN		"\nDBG ERROR: NOT IN STOP STATE"

#define STR_CMD_DATE_TIME_PAR	"\nDBG ERROR: WRONG DATE/TIME PARAMETERS\n"

//help strings
#define STR_HELP_TITLE			"\n WALL-e BUILD-IN DEBUGGER COMMANDS:"

#define STR_HELP_LEVEL			"\n LEVEL n         - enable all debug levels from n to 1, LEVEL 0 disable all"
#define STR_HELP_TRACE			"\n TRACE n pattern - defines n (1 up to 10) TRACE patterns"
#define STR_HELP_STOP			"\n STOP  n pattern - defines n (1 up to 10) STOP patterns"
#define STR_HELP_GO				"\n GO              - continue when STOPPED"
#define STR_HELP_ENTER			"\n <Enter>         - break debugging by forcing debug level to 0"

#define STR_HELP_SYSSTS			"\n SYSSTS          - display heaps and notifier queuing status"
#define STR_HELP_SYSALIVE		"\n SYSALIVE        - display system alive periodic message information"
#define STR_HELP_TIME			"\n DATETIME [YYYY MM DD WD HH MM SS] - get (when no parameters) or set system time"
#define STR_HELP_ALARM			"\n ALARM [YYYY MM DD HH MM SS] - get (when no parameters) or set system alarm time"
#define STR_HELP_ALARMCLR		"\n ALARMCLR        - clear any set up alarm"

#define	STR_HELP_RESET			"\n RESET [program id] - restart Wall-e with eventual program change"

#define STR_HELP_QUIT			"\n QUIT            - turn off Wall-e (when battery powered)"
#define STR_HELP_HELP			"\n HELP or ?       - displays this short commands guideline"

//sys status strings
#define STR_SYS_STAT_TITLE			"\n SYS RESOURCES STATUS:"
#define STR_SYS_STAT_HEAP			"\n SYS HEAP: "    
#define STR_SYS_STAT_HEAP_MIN		"\n SYS HEAP MIN: "
#define STR_SYS_STAT_NOTIFIER		"\n NOTIFIER ERR: "

//sys alive strings
#define STR_SYS_ALIVE_TITLE			"\n SYS ALIVE INFORMATION:"
#define STR_SYS_ALIVE_TICKS			"\n OS TICKS SINCE START: "  
#define STR_SYS_ALIVE_POWER			"\n OS LAST POWER OFF REASON: " 
#define STR_SYS_ALIVE_EXCEPTION		"\n OS LAST EXCEPTION REASON: " 
#define STR_SYS_ALIVE_STATIC_RAM	"\n OS STATIC RAM CONTENT PRESEREVED: "
#define STR_SYS_ALIVE_RESET_REASON 	"\n OS LAST RESET REASON: "
#define STR_SYS_ALIVE_PROGRAM_EXE 	"\n OS PROGRAM EXECUTED: "

//display time strings
#define STR_SYS_TIME_TITLE			"\n SYS CURRENT DATE & TIME:"
#define STR_SYS_TIME_YEAR        	"\n YEAR: " 
#define STR_SYS_TIME_MONTH       	"\n MONTH: " 
#define STR_SYS_TIME_DAY         	"\n DAY: " 
#define STR_SYS_TIME_WDAY       	"\n WEEK DAY: "
#define STR_SYS_TIME_HOUR        	"\n HOUR: "
#define STR_SYS_TIME_MINUTE      	"\n MINUTE: "
#define STR_SYS_TIME_SECOND      	"\n SECOND: "

//display alar strings
#define STR_ALM_TIME_TITLE			"\n ALARM CURRENT DATE & TIME:"
#define STR_ALM_TIME_YEAR        	"\n YEAR: " 
#define STR_ALM_TIME_MONTH       	"\n MONTH: " 
#define STR_ALM_TIME_DAY         	"\n DAY: " 
//#define STR_ALM_TIME_WDAY       	"\n WEEK DAY: "  - not used by alarm
#define STR_ALM_TIME_HOUR        	"\n HOUR: "
#define STR_ALM_TIME_MINUTE      	"\n MINUTE: "
#define STR_ALM_TIME_SECOND      	"\n SECOND: "
#define STR_ALM_IS_SETUP_IN_RTC     "\n ALARM IS SETUP IN RTC"
#define STR_ALM_IS_NOT_SETUP_IN_RTC "\n ALARM IS NOT SETUP IN RTC"

#define STR_ALM_CLR					"\n ALARM IS CLEARED IN RTC"

#define STR_RESET_NO_PRG			"\n RESET WITHOUT PROGRAM CHANGE"
#define STR_RESET_PRG				"\n RESET WITH PROGRAM CHANGE"

//reference to lib_dbg.c defined data
extern char DbgTrcPattern[TRACE_LIST_LEN][MAX_PATTERN_NAME_LEN]; // reference to trace patterns storage place used by lib_dbg
extern char DbgStpPattern[STOP_LIST_LEN][MAX_PATTERN_NAME_LEN]; //reference to stop patterns storage place used by lig_dbg
extern volatile int  DbgLevel;//reference to current debugging level used by lib_dbg
extern volatile int  StopState;//TRUE when stopped, FALSE when go



/*
*********************************************************************************************************
* Name:                                    FindTokenStart  
* 
* Description: Searches input for token string beginning
* 	Example: For input like below when searching starts from string begining
* 			   "   TRACE 1  1_Init   \0"
*            spaces are ignored and pointer to T (of TRACE) is returned.
* 			When started form the first space after TRACE pointer to 1 is returned and so on.
*
* Arguments:   Pointer to input string from which searching is starting
*
* Returns:     
* 			   pointer to where token starts or NULL if none token string identified
*
* Note(s):     
* 
* *********************************************************************************************************
*/
char *cRmtMngr::FindTokenStart(char *pIn)
{
	while (*pIn) //as long as there is not a string end
	{
		if (*pIn!=' ')return pIn;//
		pIn++;
	}
	return NULL; //none parameter character found in the string (only spaces or \0)
}//cRtcMngr::FindTokenStart


/*
*********************************************************************************************************
* Name:                                 ExtractTokenToBuffer  
* 
* Description: Extracts token to the token buffer assumes pTokenStart point to Token beginning
*
* Arguments:   
* 			   pTokenStart - pointer to the token string beginning
* 			   pTokenBuffer - pointer to storage place of where token string will be stored
*			   TokenBufferSize - size of the buffer for Token string storage storage 
* Returns:     
*			None
* Note(s):     
* 			IMPORTANT!
* 			This function updates internally global pointer so it points to the first char after extracted token
* 
* *********************************************************************************************************
*/
void cRmtMngr::ExtractTokenToBuffer(char *pTokenStart, char *pTokenBuffer, int TokenBufferSize)
{
	*pTokenBuffer=0;//clear token buffer before start
	pCurrentInputLinePtr=pTokenStart;//at beginning assume current input pointer is input line beginning
	
	pTokenStart=FindTokenStart(pTokenStart);//eliminate input space if any and point to token beginning
	if(pTokenStart)//if there is a token string
	{//copy characters as long as there is no space or end of the string or buffer size not excided
	while ((*pTokenStart!=' ') && (*pTokenStart!=0) && (TokenBufferSize!=0))
		{
		*pTokenBuffer=*pTokenStart;//copy character to the buffer
		pTokenStart++;//move to the next char
		pTokenBuffer++;//move to the next storage place
		TokenBufferSize--;
		}
		pCurrentInputLinePtr=pTokenStart;//advance pointer to the first character after identified token
	}
	*pTokenBuffer=0;//terminate extracted token string
	
}//cRtcMngr::ExtractTokenToBuffer

//executte debug level change command
void cRmtMngr::RmtCmdLevel(void)
{
	//extract second token from the input which in case of LEVEL should be the level number
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	DbgLevel=atoi(TokenBuffer);//convert token string into number and make it current debug level
	Uart0Message(STR_DBG_LEVEL,DbgLevel);
}//cRmtMngr::RmtCmeLevel

//display all curently valid traces
void cRmtMngr::DisplayAllTraces(void)
{
Uart0PutStr(STR_DBG_TRACE_DSP);
Uart0PutStr("\n");//move to new line
for (int i=0; i < TRACE_LIST_LEN; i++)
	{
	if (DbgTrcPattern[i][0] != 0)//if there is any trace patter setup display it
	    {
		Uart0PutStr("\n ");//put new line
		Uart0PutStr(itoa(i+1,TokenBuffer,10));//display entry number using TokenBuffer as temprary buffer for value conversion
		Uart0PutStr(" ");//put one space between entry number and pattern
		Uart0PutStr(DbgTrcPattern[i]);//display the pattern
		}//if
	}//for
}//cRmtMngr::DisplayAllTraces(void)


//execute debuger Trace command
void cRmtMngr::RmtCmdTrace(void)
{
	int TracePatternNo;//storage of trace pattern number extracted from TRACE command
	
	//extract second token from the input which in case of TRACE should be the level number
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	if(!strlength(TokenBuffer))//when no parameters
	{ //display all traces setup so far and exit
		DisplayAllTraces();//display all curent traces
		Uart0PutStr("\n");//move to new line
		return;
	}
	TracePatternNo=atoi(TokenBuffer);//convert token string into number and make it current debug level
	if((TracePatternNo<1) || (TracePatternNo>TRACE_LIST_LEN))//when number behind storage capabilities
	{   //display all traces setup so far and an error message and exit
		DisplayAllTraces();//display all curent traces
		Uart0Message(STR_DBG_TRACE_N,TracePatternNo);
		return;
	}
	
	//extract third token from the input which in case of TRACE should be the pattern string
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	if(!strlength(TokenBuffer))//when no pattern string 
	{//display error message and exit
		DisplayAllTraces();//display all curent traces
		Uart0PutStr(STR_DBG_TRACE_PTRN_NO);
		return;
	}
	if(strlength(TokenBuffer)>MAX_PATTERN_NAME_LEN-1)//when pattern string too long to be stored
	{//display error message and exit
		DisplayAllTraces();//display all curent traces
		Uart0PutStr(STR_DBG_TRACE_PTRN_LNG);
		return;
	}
		
	strcpy(DbgTrcPattern[TracePatternNo-1],TokenBuffer);//copy TRACE pattern to its appropriate place
	DisplayAllTraces();//display all curent traces
	Uart0PutStr("\n");//move to new line
}//cRmtMngr::RmtCmdTrace

//display all curently valid stops
void cRmtMngr::DisplayAllStops(void)
{
Uart0PutStr(STR_DBG_STOP_DSP);
Uart0PutStr("\n");//move to new line
for (int i=0; i < STOP_LIST_LEN; i++)
	{
	if (DbgStpPattern[i][0] != 0)//if there is any stop patter setup display it
	    {
		Uart0PutStr("\n ");//put new line
		Uart0PutStr(itoa(i+1,TokenBuffer,10));//display entry number using TokenBuffer as temprary buffer for value conversion
		Uart0PutStr(" ");//put one space between entry number and pattern
		Uart0PutStr(DbgStpPattern[i]);//display the pattern
		}//if
	}//for
}//cRmtMngr::DisplayAllStops(void)

//execute STOP command
void cRmtMngr::RmtCmdStop(void)
{
	int StopPatternNo;//storage of stop pattern number extracted from STOP command
	
	//extract second token from the input which in case of STOP should be the level number
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	if(!strlength(TokenBuffer))//when no parameters
	{ //display all stops setup so far and exit
		DisplayAllStops();//display all curent stops
		Uart0PutStr("\n");//move to new line
		return;
	}
	StopPatternNo=atoi(TokenBuffer);//convert token string into number and make it current debug level
	if((StopPatternNo<1) || (StopPatternNo>STOP_LIST_LEN))//when number behind storage capabilities
	{   //display all traces setup so far and an error message and exit
		DisplayAllStops();//display all curent traces
		Uart0Message(STR_DBG_STOP_N,StopPatternNo);
		return;
	}
	
	//extract third token from the input which in case of STOP should be the pattern string
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	if(!strlength(TokenBuffer))//when no pattern string 
	{//display error message and exit
		DisplayAllStops();//display all curent traces
		Uart0PutStr(STR_DBG_STOP_PTRN_NO);
		return;
	}
	if(strlength(TokenBuffer)>MAX_PATTERN_NAME_LEN-1)//when pattern string too long to be stored
	{//display error message and exit
		DisplayAllStops();//display all curent traces
		Uart0PutStr(STR_DBG_STOP_PTRN_LNG);
		return;
	}
		
	strcpy(DbgStpPattern[StopPatternNo-1],TokenBuffer);//copy TRACE pattern to its appropriate place
	DisplayAllStops();//display all curent traces
	Uart0PutStr("\n");//move to new line	
}//cRmtMngr::RmtCmdStop

void cRmtMngr::RmtCmdGo(void)
{
	if(StopState)//when in the stop state turn that to GO
		StopState=FALSE;//turn from STOP to GO state
	else
		Uart0PutStr(STR_DBG_STOP_NOT_IN);//put information that system is already in GO state
}//cRmtMngr::RmtCmdGo

//get and display system resources statistic
void cRmtMngr::RmtCmdSysStat(void)
{
	sSysResourcesStatus *pSysStatus;//pointer to processed sys status information
	
	for(;;)//wait infinite until sys status received
	{
		//this portion of code is to avoid getting something queued by subscriber in the past
		GetReceiveQueue()->Flush();//flush received queue to get latest EVT_SYS_RES notifier only
		Kernel.Dispatcher.RegisterSubscriber(*this,EVT_SYS_RES);//subscribe for sys ststus event
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		Kernel.Dispatcher.UnregisterSubscriber(*this,EVT_SYS_RES);//unsubscribe from the event
		if (pNotifier->GetNotifierId() == EVT_SYS_RES)//check if sys status received
		{
			pSysStatus=static_cast<sSysResourcesStatus*>(pNotifier->GetDataPtr());//get pointer to Notifier held sSysResourcesStatus structure 
			
			Uart0PutStr(STR_SYS_STAT_TITLE);
			Uart0PutStr("\n");//move to new line
			Uart0Message(STR_SYS_STAT_HEAP        ,pSysStatus->mSysHeapMemLeft);
			Uart0Message(STR_SYS_STAT_HEAP_MIN    ,pSysStatus->mSysHeapMinLeft);
			Uart0Message(STR_SYS_STAT_NOTIFIER    ,pSysStatus->mTotalDispatchErrorCounter);
			Uart0PutStr("\n");//move to new line
			return;//break the loop and return from endless waiting because status received
		}//if
	}//for
}//cRmtMngr::RmtCmdSysStat

//get and display most up to date system alive message
void cRmtMngr::RmtCmdSysAlive(void)
{
	sSysAliveEvt *pSysAlive;//pointer to processed sys alive information
	
	for(;;)//wait infinite until sys status received
	{
		//this portion of code is to avoid getting something queued by subscriber in the past
		GetReceiveQueue()->Flush();//flush received queue to get latest EVT_SYS_ALIVE notifier only
		Kernel.Dispatcher.RegisterSubscriber(*this,EVT_SYS_ALIVE);//subscribe for sys alive event
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		Kernel.Dispatcher.UnregisterSubscriber(*this,EVT_SYS_ALIVE);//unsubscribe from the event
		if (pNotifier->GetNotifierId() == EVT_SYS_ALIVE)//check if sys status received
		{
			pSysAlive=static_cast<sSysAliveEvt*>(pNotifier->GetDataPtr());//get pointer to Notifier held sSysAliveEvt structure 
			
			Uart0PutStr(STR_SYS_ALIVE_TITLE);
			Uart0PutStr("\n");//move to new line
			Uart0Message(STR_SYS_ALIVE_TICKS        ,pSysAlive->mTimeStamp);
			Uart0Message(STR_SYS_ALIVE_POWER    ,pSysAlive->mLastPowerOffReason);
			Uart0Message(STR_SYS_ALIVE_EXCEPTION    ,pSysAlive->mLastExceptionReason);
			Uart0Message(STR_SYS_ALIVE_STATIC_RAM,pSysAlive->mRAMContentPreserved);
			Uart0Message(STR_SYS_ALIVE_RESET_REASON,pSysAlive->mLastResetReason);
			Uart0Message(STR_SYS_ALIVE_PROGRAM_EXE,pSysAlive->mWalleProgramToExecute);

			Uart0PutStr("\n");//move to new line
			return;//break the loop and return from endless waiting because status received
		}//if
	}//for	
}//cRmtMngr::RmtCmdSysAlive

//display most up to date system data and time
void cRmtMngr::GetAndDisplayTime(void)
{
	sRtcData *pRtcData;//pointer to processed notifier data
	
	for(;;)//wait infinite until time data received
	{
		//this portion of code is to avoid getting something queued by subscriber in the past
		GetReceiveQueue()->Flush();//flush received queue to get latest EVT_TIME notifier only
		Kernel.Dispatcher.RegisterSubscriber(*this,EVT_TIME);//subscribe for sys time event
		cSmartPtr<cNotifier> pNotifier = Receive();//wait for notifier to arrive
		Kernel.Dispatcher.UnregisterSubscriber(*this,EVT_TIME);//unsubscribe from the event
		
		if (pNotifier->GetNotifierId() == EVT_TIME)//check if sys time received
			{
			pRtcData=static_cast<sRtcData*>(pNotifier->GetDataPtr());//get pointer to Notifier
			
			Uart0PutStr(STR_SYS_TIME_TITLE);
			Uart0PutStr("\n");//move to new line
			Uart0Message(STR_SYS_TIME_YEAR        ,pRtcData->mYear);
			Uart0Message(STR_SYS_TIME_MONTH       ,pRtcData->mMonth);
			Uart0Message(STR_SYS_TIME_DAY         ,pRtcData->mDay);
			Uart0Message(STR_SYS_TIME_WDAY        ,pRtcData->mDayOfWeek);
			Uart0Message(STR_SYS_TIME_HOUR        ,pRtcData->mHour);
			Uart0Message(STR_SYS_TIME_MINUTE      ,pRtcData->mMinute);
			Uart0Message(STR_SYS_TIME_SECOND      ,pRtcData->mSecond);
			return;//return once time received and printed
			}//if
	}//for
}//cRmtMngr::GetAndDisplayTime

//set or get system time syntax: TIME [YYYY MM DD DW HH MM SS]
void cRmtMngr::RmtCmdTime(void)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	
	//extract second token from the input which in case of TIME should be YEAR
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mYear = (WORD)atoi(TokenBuffer);//convert buffered input into Year
	if(!strlength(TokenBuffer))//when no parameters means get and displa time only
	{
		GetAndDisplayTime();//retrive most up to date time and display it
		return;//terminate function when date/time display requested only
	}
	if(((pNotifier->GetData()).mYear < MIN_YEAR) || ((pNotifier->GetData()).mYear > MAX_YEAR))//when wrong input
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;	
	}
	
	//extract next token which should be MONTH
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mMonth = (BYTE)atoi(TokenBuffer);//convert buffered input into MONTH
	if(!strlength(TokenBuffer)||((pNotifier->GetData()).mMonth < MIN_MONTH) || ((pNotifier->GetData()).mMonth > MAX_MONTH) )//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;
	}
		
	//extract next token which should be DAY
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mDay = (BYTE)atoi(TokenBuffer);//convert buffered input into DAY
	if(!strlength(TokenBuffer)||((pNotifier->GetData()).mDay < MIN_DAY) ||((pNotifier->GetData()).mDay>MAX_DAY))//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;
	}
		
	//extract next token which should be DAY of WEEK
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mDayOfWeek = (BYTE)atoi(TokenBuffer);//convert buffered input into DAY of WEEK
	if(!strlength(TokenBuffer)||((pNotifier->GetData()).mDayOfWeek > MAX_DAY_WEEK))//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;
	}
		
	//extract next token which should be HOUR
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mHour = (BYTE)atoi(TokenBuffer);//convert buffered input into HOUR
	if(!strlength(TokenBuffer)||(pNotifier->GetData()).mHour > MAX_HOUR)//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;
	}
		
	//extract next token which should be MINUTES
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mMinute = (BYTE)atoi(TokenBuffer);//convert buffered input into MINUTES
	if(!strlength(TokenBuffer)||(pNotifier->GetData()).mMinute > MAX_MINUTE)//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;
	}
		
	//extract next token which should be SECONDS
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mSecond = (BYTE)atoi(TokenBuffer);//convert buffered input into SECONDS
	if(!strlength(TokenBuffer)||(pNotifier->GetData()).mSecond > MAX_SECOND)//when wrong input
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplayTime();//retrive most up to date time and display it
		return;
	}
	(pNotifier->GetData()).mRtcCmdId = SET_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command
	GetAndDisplayTime();//retrive most up to date time and display it
}//cRmtMngr::RmtCmdTime

//display most up to date system alarm date and time
void cRmtMngr::GetAndDisplaAlarmTime(void)
{
	sRtcData *pRtcData;//pointer to received notifier data
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = GET_ALARM_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	
	//this portion of code is to avoid getting something queued by subscriber in the past
	GetReceiveQueue()->Flush();//flush received queue to get latest EVT_TIME notifier only
	Kernel.Dispatcher.RegisterSubscriber(*this,RSP_RTC);//subscribe for sys time event
	
	Post(pNotifier);//post command to get current alarm settings
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=GET_ALARM_TIME_DATE_SCMD_ID);//untill we get response for the command
	
	Kernel.Dispatcher.UnregisterSubscriber(*this,RSP_RTC);//unsubscribe from the event
	
	pRtcData=static_cast<sRtcData*>(pNotifierRSP->GetDataPtr());
	
	Uart0PutStr(STR_ALM_TIME_TITLE);
	Uart0PutStr("\n");//move to new line
	Uart0Message(STR_ALM_TIME_YEAR        ,pRtcData->mYear);
	Uart0Message(STR_ALM_TIME_MONTH       ,pRtcData->mMonth);
	Uart0Message(STR_ALM_TIME_DAY         ,pRtcData->mDay);

	Uart0Message(STR_ALM_TIME_HOUR        ,pRtcData->mHour);
	Uart0Message(STR_ALM_TIME_MINUTE      ,pRtcData->mMinute);
	Uart0Message(STR_ALM_TIME_SECOND      ,pRtcData->mSecond);
	if(pRtcData->mIsAlarmSetupInRTC) //if alarm is setup in RTC put proper infor
		Uart0PutStr(STR_ALM_IS_SETUP_IN_RTC);
	else
		Uart0PutStr(STR_ALM_IS_NOT_SETUP_IN_RTC);
	
}//cRmtMngr::GetAndDisplaAlarmTime

//set or get system alar time syntax: ALARM [YYYY MM DD HH MM SS]

void cRmtMngr::RmtCmdAlarm(void)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	
	//extract second token from the input which in case of ALARM should be YEAR
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mYear = (WORD)atoi(TokenBuffer);//convert buffered input into Year
	if(!strlength(TokenBuffer))//when no parameters means get and displa Alarm only
	{
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;//terminate function when Alarm date/time display requested only
	}
	if(((pNotifier->GetData()).mYear < MIN_YEAR) || ((pNotifier->GetData()).mYear > MAX_YEAR))//when wrong input
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_ALARM);
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;	
	}
	
	//extract next token which should be MONTH
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mMonth = (BYTE)atoi(TokenBuffer);//convert buffered input into MONTH
	if(!strlength(TokenBuffer)||((pNotifier->GetData()).mMonth < MIN_MONTH) || ((pNotifier->GetData()).mMonth > MAX_MONTH) )//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;
	}
		
	//extract next token which should be DAY
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mDay = (BYTE)atoi(TokenBuffer);//convert buffered input into DAY
	if(!strlength(TokenBuffer)||((pNotifier->GetData()).mDay < MIN_DAY) ||((pNotifier->GetData()).mDay>MAX_DAY))//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;
	}

	(pNotifier->GetData()).mDayOfWeek = 0;//day of week is not used by ALARM CMD
	
	//extract next token which should be HOUR
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mHour = (BYTE)atoi(TokenBuffer);//convert buffered input into HOUR
	if(!strlength(TokenBuffer)||(pNotifier->GetData()).mHour > MAX_HOUR)//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;
	}
		
	//extract next token which should be MINUTES
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mMinute = (BYTE)atoi(TokenBuffer);//convert buffered input into MINUTES
	if(!strlength(TokenBuffer)||(pNotifier->GetData()).mMinute > MAX_MINUTE)//when wrong input 
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;
	}
		
	//extract next token which should be SECONDS
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	(pNotifier->GetData()).mSecond = (BYTE)atoi(TokenBuffer);//convert buffered input into SECONDS
	if(!strlength(TokenBuffer)||(pNotifier->GetData()).mSecond > MAX_SECOND)//when wrong input
	{//display error message and exit
		Uart0PutStr(STR_CMD_DATE_TIME_PAR);
		Uart0PutStr(STR_HELP_TIME);
		GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it
		return;
	}
	(pNotifier->GetData()).mRtcCmdId = SET_ALARM_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	Post(pNotifier);//post command
	GetAndDisplaAlarmTime();//retrive most up to date alarm time and display it	
}//cRmtMngr::RmtCmdAlarm

//clr system alarm time syntax: ALARMCLR
void cRmtMngr::RmtCmdAlarmClr(void)
{
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifier = new cTypeNotifier<sRtcData>(CMD_RTC,GetThreadId(),NT_HND_NORMAL_PRT);
	(pNotifier->GetData()).mRtcCmdId = CLR_ALARM_FOR_TIME_DATE_SCMD_ID;//setup sub-command requested to be executed
	
	//this portion of code is to avoid getting something queued by subscriber in the past
	GetReceiveQueue()->Flush();//flush received queue to get latest EVT_TIME notifier only
	Kernel.Dispatcher.RegisterSubscriber(*this,CMD_RTC);//subscribe for sys time event
	Post(pNotifier);//post command	
	
	cSmartPtr<cTypeNotifier<sRtcData> > pNotifierRSP;//declare notifier to handle command response
	do{
		pNotifierRSP=Receive();//wait for notifier to arrive
		if((pNotifierRSP->GetNotifierId())!= RSP_RTC) continue;//skip any notifier different to required one
	}while((pNotifierRSP->GetData()).mRtcCmdId!=CLR_ALARM_FOR_TIME_DATE_SCMD_ID);//untill we get response for the command
	
	Kernel.Dispatcher.UnregisterSubscriber(*this,CMD_RTC);//unsubscribe from the event
	Uart0PutStr(STR_ALM_CLR);//display that alarm is cleared now
	Uart0PutStr("\n");//move to new line
	
}//cRmtMngr::RmtCmdAlarmClr

//display a short help strings about commands syntax and their meaning
void cRmtMngr::RmtCmdHelp(void)
{
	Uart0PutStr(STR_HELP_TITLE);
	Uart0PutStr("\n");//move to new line
	Uart0PutStr(STR_HELP_LEVEL);
	Uart0PutStr(STR_HELP_TRACE);
	Uart0PutStr(STR_HELP_STOP);
	Uart0PutStr(STR_HELP_GO);
	Uart0PutStr(STR_HELP_ENTER);	
	
	Uart0PutStr(STR_HELP_SYSSTS);
	Uart0PutStr(STR_HELP_SYSALIVE);
	Uart0PutStr(STR_HELP_TIME);
	Uart0PutStr(STR_HELP_ALARM);
	Uart0PutStr(STR_HELP_ALARMCLR);
	Uart0PutStr(STR_HELP_RESET);
	Uart0PutStr(STR_HELP_QUIT);
	Uart0PutStr(STR_HELP_HELP);
	
	Uart0PutStr("\n");//move to new line
}//cRmtMngr::RmtCmdHelp

//execute Wall-e RESET to eventually change its program to the new one
void cRmtMngr::RmtCmdReset(void)
{
	BYTE ProgramToExecute;//temporary storage of program to execute
	
	//extract second token from the input which in case of RESET should be the program ID
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));
	ProgramToExecute=atoi(TokenBuffer);//convert token string into number and make it current debug level
	
	//when program ID behind the limits
	if ((ProgramToExecute < WALL_MIN_PROGRAM_ID) || (ProgramToExecute > WALLE_MAX_PROGRAM_ID) )
	{ //make reset but without program change
		Uart0PutStr(STR_RESET_NO_PRG);
		uPResetByWatchdog(NO_RESET);//make reset but without program change
	}else
	{//when corect ID of requested program call RESET for program change
		Uart0PutStr(STR_RESET_PRG);
		SetWalleProgramToExecute(ProgramToExecute);
		uPResetByWatchdog(SW_RESET);//request program change
		for(;;)//infinite loop to disable further program execution until reset
		{
		}//infinite for
	}
}//cRmtMngr::RmtCmdReset

/*
*********************************************************************************************************
* Name:                                    ParseRmtCmd  
* 
* Description: Parse remote commands and extract its parameters
*
* Arguments:
*			   pInCmd - pointer to buffer holding input command string
* Returns:     parsed command identifier
*
* Note(s):     
* 
* *********************************************************************************************************
*/
int cRmtMngr::ParseRmtCmd(char *pInCmd )
{
	pCurrentInputLinePtr=pInCmd;//initialize pointer to current processed position in the input line string
	
	ExtractTokenToBuffer(pCurrentInputLinePtr,TokenBuffer,sizeof(TokenBuffer));//extract first token from the input line to the buffer
	strupr(TokenBuffer);//convert token to the upper case
	
	//identify command if eny
	if ((!strcmp(TokenBuffer,RMT_CMD_STR_HELP))||(!strcmp(TokenBuffer,RMT_CMD_STR_Q))) return RMT_CMD_HELP;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_QUIT )) 	return RMT_CMD_QUIT;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_LEVEL)) 	return RMT_CMD_LEVEL;	
	if (!strcmp(TokenBuffer,RMT_CMD_STR_TRACE)) 	return RMT_CMD_TRACE;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_STOP)) 		return RMT_CMD_STOP;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_GO)) 		return RMT_CMD_GO;
	
	if (!strcmp(TokenBuffer,RMT_CMD_STR_SYS_STAT)) 	return RMT_CMD_SYS_STAT;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_SYS_ALIVE))	return RMT_CMD_SYS_ALIVE;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_DATE_TIME)) return RMT_CMD_DATE_TIME;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_ALARM)) 	return RMT_CMD_ALARM;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_ALARMCLR)) 	return RMT_CMD_ALARMCLR;
	if (!strcmp(TokenBuffer,RMT_CMD_STR_RESET)) 	return RMT_CMD_RESET;
	
	return RMT_CMD_NO;
		
}//cRtcMngr::RmtParseCmd

void cRmtMngr::Run()
{
	BYTE Response;//to preserve response
	
	Uart0PutStr("\n"); //display WALL-e welcome message on the terminal
	Uart0PutStr(WALLE_RUN);
	Uart0PutStr("\n");
	Uart0PutStr(WALLE_OS_VERSION);
	Uart0PutStr("\n");
	Uart0PutStr(WALLE_VERSION);
	Uart0PutStr("\n");
	Uart0PutStr("\n");
	Uart0Message(STR_DBG_LEVEL,DbgLevel); //dislplay build-in debugger debug level
	Response=ReadLastResetReason();//read reset reason
	if(Response== SW_RESET)
	{//if restart because program change request display message about that
		Uart0PutStr("\n");
		Uart0PutStr(WALLE_PRG_CHANGE);
	}
	Uart0PutStr("\n");
	Uart0DebugMessage(WALLE_PRG_NO,GetWalleProgramToExecute(),0);
		
	if(Response==NO_RESET)
		Uart0PutStr("\nLOG: RESET: No requested!");
	else if (Response==WDT_RESET)
		Uart0PutStr("\nLOG: RESET: WDT reset!");
	else if (Response==SW_RESET)
		Uart0PutStr("\nLOG: RESET: SW reset!");
	else
		Uart0PutStr("\nLOG: RESET: Uknwnown!");
	
	switch(GetWalleProgramToExecute())
	{
	case WALLE_PROGRAM_BATH:
		Uart0PutStr("\nLOG: PROGRAM: BATH");
		break;
	case WALLE_PROGRAM_VOICE_CTRL:
		Uart0PutStr("\nLOG: PROGRAM: VOICE CTRL");
		break;
	case WALLE_PROGRAM_TEST:
		Uart0PutStr("\nLOG: PROGRAM: TEST");
		break;
	case WALLE_PROGRAM_ENJOY:
		Uart0PutStr("\nLOG: PROGRAM: ENJOY");
		break;
	default:
		Uart0PutStr("\nLOG: PROGRAM: UNKNOWN");
		break;
	}
	
	DbgInit();//intialize debugger/tracer to its default state
	Kernel.TimeDlyHMSM(0,0,MNG_RMT_START_TIMEOUT,0);//do nothing for a moment to get task LOG info displayed before the prompt 
	for(;;)
	{
		//put prompt depending on the debugger state
		if(StopState) //when stopped at certain stop command put stop prompt to signal this
			Uart0PutStr(STR_CMD_PROMPT_STOP_STATE);//put stop state prompt
		else //if normal running state put standard prompt
			Uart0PutStr(STR_CMD_PROMPT);//print prompt on terminal and wait for data to be provided
		Uart0GetTrmlStr(InputLineBuff, sizeof(InputLineBuff));//get entry from terminal line
		if(!strlength(InputLineBuff)) //if only <Enter> from terminal that is assumed as DBG mode call
			{//get system ready to accept commands - break dbg activitis which disturb terminal reading of commands
			DbgLevel=0;//this level should disable all debug info printing
			Uart0Message(STR_DBG_LEVEL,DbgLevel);
			continue;//so continue should display command prompt which is not disturbed by other printings
			}
		switch (ParseRmtCmd(InputLineBuff))
			{
			case RMT_CMD_HELP: //display a short help
				RmtCmdHelp();
				break;
			case RMT_CMD_QUIT: //execute QUIT command
				Uart0PutStr(STR_CMD_QUIT);
				uPBoardOff(REMOTE_TURN_OFF); //turn off Wall-e on UART request
				break;
			case RMT_CMD_LEVEL://execute LEVEL command
				RmtCmdLevel();//executte debug level change command and display setup level
				break;	
			case RMT_CMD_TRACE://execute TRACE command
				RmtCmdTrace();
				break;
			case RMT_CMD_STOP://execute STOP command
				RmtCmdStop();
				break;
			case RMT_CMD_GO: //continue after stop
				RmtCmdGo();
				break;
			case RMT_CMD_SYS_ALIVE://get and display system alive data
				RmtCmdSysAlive();
				break;
			case RMT_CMD_SYS_STAT: //get and display system resources status data
				RmtCmdSysStat();
				break;
			case RMT_CMD_DATE_TIME: //set or get time
				RmtCmdTime();
				break;
			case RMT_CMD_ALARM: //set or get alarm
				RmtCmdAlarm();
				break;
			case RMT_CMD_ALARMCLR: //clear alarm
				RmtCmdAlarmClr();
				break;
			case RMT_CMD_RESET: //reset Wall-e to start eventual it's new program
				RmtCmdReset();
				break;
			case RMT_CMD_NO: //no command identified
			default:
				Uart0PutStr(STR_CMD_UNKNOWN);
				break;
			}//switch
		Kernel.Delay(1);//delay 1 tick to allow task switch
	}//for
}//cRtcMngr::Run()
