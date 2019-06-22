/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_dbg.h
* Description: Functions for remote debug handling from through mng_rtm (remote) manager commands
* Author:      Bogdan Kowalczyk
* Date:        06-Jan-2017
* Note:        This module reuse concept and source code of Robert Ward / Dr. Rainer Storn build-in debugger
*              as published in Dr. Dobb's Journal (not sure probably in 1997)
* History:
* 06-Jan-2017 - Initial version of remote commands processing library
* 
*********************************************************************************************************
*/
#ifndef LIB_DBG_H_
#define LIB_DBG_H_
#ifdef __cplusplus
   extern "C" {
#endif

//constant for Uart0DebugMessage to print but without exception to be generated
#define DO_NOT_GEN_EXCEPTION 0
	   
#define TRACE_LIST_LEN			10 //maximum namber of trace patterns
#define STOP_LIST_LEN			10 //maximum number of stop patterns	   
#define MAX_PATTERN_NAME_LEN  	11 //maximum length of the trace pattern allowed
	   
//define remote command prompt but executed from stop state
#define STR_CMD_PROMPT_STOP_STATE			"\nS>>>"

/*
*********************************************************************************************************
* Name:                                    DbgInit  
* 
* Description: Initialize Debugger
*
* Arguments:
*			   none
*
* Returns:     none
*
* Note(s):     
* 
*********************************************************************************************************
*/
extern void DbgInit(void);
	   
/*
*********************************************************************************************************
* Name:                                    DbgTraceStrVal 
* 
* Description: Debug command placed in source code at those places where we would like to trace
*              with both a message and a value
*
* Arguments:
* 			InLevel - debug point levels (defined levels are managed by remote LEVEL command)
* 			pInLabel - debug point label matching algo is used to prcess those Dbg points which matches to the pattern
* 			pMessageText - pointer to message text which is displayed for the debug point
* 			InValue - debug value which is converted to string and displayed
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern void DbgTraceStrVal(int InLevel, char *pInLabel, char *pMessageText, long InValue);


/*
*********************************************************************************************************
* Name:                                    DbgTraceStr 
* 
* Description: Debug command placed in source code at those places where we would like to trace
*              with a message. Similar to DbgTraceStrVal but displays only string text when enabled.
*
* Arguments:
* 			InLevel - debug point levels (defined levels are managed by remote LEVEL command)
* 			pInLabel - debug point label matching algo is used to prcess those Dbg points which matches to the pattern
* 			pMessageText - pointer to message text which is displayed for the debug point
*
* Returns:     none
*
* Note(s):     
* 			inLevel 0 should not be used, this level is reserved to turn off all messages by LEVEL 0 command
* 
* *********************************************************************************************************
*/
extern void DbgTraceStr(int InLevel, char *pInLabel, char *pMessageText);



/*
*********************************************************************************************************
* Name:                                    DbgStopStrVal 
* 
* Description: Debug command placed in source code at those places where we would like to stop execution
*              with both a message and a value
*
* Arguments:
* 			InLevel - debug point levels (defined levels are managed by remote LEVEL command)
* 			pInLabel - debug point label matching algo is used to prcess those Dbg points which matches to the pattern
* 			pMessageText - pointer to message text which is displayed for the debug point
* 			InValue - debug value which is converted to string and displayed
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern void DbgStopStrVal(int InLevel, char *pInLabel, char *pMessageText, long InValue);


/*
*********************************************************************************************************
* Name:                                    DbgStopStr 
* 
* Description: Debug command placed in source code at those places where we would like to stop execution
*              with a message. Similar to DbgStopStrVal but displays only string text when enabled.
*
* Arguments:
* 			InLevel - debug point levels (defined levels are managed by remote LEVEL command)
* 			pInLabel - debug point label matching algo is used to prcess those Dbg points which matches to the pattern
* 			pMessageText - pointer to message text which is displayed for the debug point
*
* Returns:     none
*
* Note(s):     
* 			inLevel 0 should not be used, this level is reserved to turn off all messages by LEVEL 0 command
* 
* *********************************************************************************************************
*/
extern void DbgStopStr(int InLevel, char *pInLabel, char *pMessageText);


#ifdef __cplusplus
}
#endif //to close extern "C" if used
#endif /*LIB_DBG_H_*/
