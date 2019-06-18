/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_rmt.hpp
* Description: Robot's remote access manager
* Author:      Bogdan Kowalczyk
* Date:        2-Jan-2017
* Note:
* History:
*              2-Jan-2017 - Initial version created
*********************************************************************************************************
*/
#ifndef MNG_RMT_HPP_
#define MNG_RMT_HPP_
#include "mng.hpp"

#define  RMT_PUBLISHER_SEND_Q_SIZE 	5
#define  RMT_SUBSCRIBER_REC_Q_SIZE 	5
#define  RMT_THREAD_STACK_SIZE		128
#define  RMT_THREAD_PRIORITY   		40

#define	 MAX_INPUT_CMD_LINE			80 //terminal input line max buffer length
#define  MAX_TOKEN_LENGTH           20 //cmd token max length

//constants which identify commands
#define RMT_CMD_NO 			0
#define RMT_CMD_HELP		1
#define RMT_CMD_QUIT		2
#define RMT_CMD_LEVEL		3
#define RMT_CMD_TRACE		4
#define	RMT_CMD_STOP		5
#define	RMT_CMD_GO			6
#define RMT_CMD_SYS_STAT	7
#define RMT_CMD_SYS_ALIVE	8
#define RMT_CMD_DATE_TIME	9
#define RMT_CMD_ALARM		10
#define RMT_CMD_ALARMCLR	11
#define RMT_CMD_RESET		12


//strings which corresponds to commands
#define RMT_CMD_STR_HELP		"HELP"
#define	RMT_CMD_STR_Q			"?"
#define RMT_CMD_STR_QUIT		"QUIT"

#define RMT_CMD_STR_LEVEL		"LEVEL"
#define RMT_CMD_STR_TRACE		"TRACE"
#define RMT_CMD_STR_STOP		"STOP"
#define RMT_CMD_STR_GO			"GO"

#define RMT_CMD_STR_SYS_STAT	"SYSSTS"
#define RMT_CMD_STR_SYS_ALIVE	"SYSALIVE"
#define RMT_CMD_STR_DATE_TIME	"DATETIME"
#define RMT_CMD_STR_ALARM		"ALARM"
#define RMT_CMD_STR_ALARMCLR	"ALARMCLR"
#define RMT_CMD_STR_RESET		"RESET"

class cRmtMngr:public cMngBasePublisherSubscriber<RMT_PUBLISHER_SEND_Q_SIZE,RMT_SUBSCRIBER_REC_Q_SIZE,RMT_THREAD_STACK_SIZE,RMT_THREAD_PRIORITY>
{
private:
		char InputLineBuff[MAX_INPUT_CMD_LINE]; //buffer for terminal input line
		char TokenBuffer[MAX_TOKEN_LENGTH];//internal buffer used to hold extracted token
		char *pCurrentInputLinePtr; //pointer to current processed position in the Input Line Buffer updated once input is parsed
		
		//skip leading spaces and place at first token character
		char *FindTokenStart(char *pIn);
		//using current pointer to command line extract next token to the pointed buffer of defined size
		void ExtractTokenToBuffer(char *pTokenStart, char *pTokenBuffer, int TokenBufferSize);
		//executte debug level change command
		void RmtCmdLevel(void);
		//display all curently valid traces
		void DisplayAllTraces(void);
		//execute debuger Trace command
		void RmtCmdTrace(void);
		//display all curently valid stops
		void DisplayAllStops(void);
		//execute STOP command
		void RmtCmdStop(void);
		//continue execution after stop
		void RmtCmdGo(void);
		//get and display system resources statistic
		void RmtCmdSysStat(void);
		//get and display most up to date system alive message
		void RmtCmdSysAlive(void);
		//display most up to date system data and time
		void GetAndDisplayTime(void);
		//set or get system time syntax: TIME [YYYY MM DD DW HH MM SS]
		void RmtCmdTime(void);
		//display most up to date system alarm date and time
		void GetAndDisplaAlarmTime(void);
		//set or get system alarm time syntax: ALARM [YYYY MM DD HH MM SS]
		void RmtCmdAlarm(void);
		//clr system alarm time syntax: ALARMCLR
		void RmtCmdAlarmClr(void);
		//display a short help strings about commands syntax and meaning
		void RmtCmdHelp(void);
		//execute Wall-e RESET to eventually change its program to the new one
		void RmtCmdReset(void);
		//parse remote command line and extract command out of it
		int ParseRmtCmd(char *pInCmd );
		//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
		virtual void Run();
};//cRmtMngr


#endif /*MNG_RMT_HPP_*/
