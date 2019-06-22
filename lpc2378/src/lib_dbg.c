/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2017, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_dbg.c
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

#include "os_cpu.h"
#include "os_cfg.h"
#include "os_ucos_ii.h"

#include "lib_dbg.h"
#include "hw_uart.h"
#include "lib_std.h"

char DbgTrcPattern[TRACE_LIST_LEN][MAX_PATTERN_NAME_LEN]; // reference to trace patterns storage place used by lib_dbg
char DbgStpPattern[STOP_LIST_LEN][MAX_PATTERN_NAME_LEN]; //reference to stop patterns storage place used by lig_dbg

volatile int  DbgLevel;//current debugging level
volatile int  StopState;// TRUE when at stop point, FALSE when GO



/*
*********************************************************************************************************
* Name:                                     PatternMatch 
* 
* Description: Compares str1 to str2, performing limited 
*              pattern matching. Specifically, '?'s in str2     
*              match any character (or end of string) in str1.  
*                          
*
* Arguments:   
* 			  str1 - pointer to string which is checked against str2 pattern
*			  str2 - pointer to pattern which may include special charcters '?' and '*'
* Returns:     
* 			  Returns 1 on match. 0 on no match.
*
* Note(s):     
* 
* *********************************************************************************************************
*/
static int PatternMatch(char *str1, char *str2)
{

/*-----Proceed as long as end of strings is not reached and---------*/
/*-----characters of str1 and str2 either match via equality--------*/
/*-----or via wildcard.---------------------------------------------*/
/*-----str1 and str2 must be non empty. At the same time the--------*/
/*-----contents of str1 and str2 must either be equal or str2-------*/
/*-----has to contain the single wildcard '?'.----------------------*/

	while(*str1 && *str2 && ((*str1 == *str2) || (*str2 == '?')))
	{
		str1 = str1 + 1;     /* Next character in strings */
		str2 = str2 + 1;
	}

	if (*str1 == 0)         /* If end of str1 is reached */
	{
		while((*str2 == '?') || (*str2 == '*'))
		{                   /* Munch all remaining '?'s or '*' in str2 */
			str2 = str2 + 1;
		}
	}

	if (*str2 == '*')       /* If remaining part of str1 is matched by '*' */
	{
		return(TRUE);
	}

	if ((*str1 || *str2) != 0)    /* If there is still some remaining part */
	{                             /* in either str1 or str2, then no match.*/
		return(FALSE);
	}
return(TRUE);
}//PatternMatch

/*
*********************************************************************************************************
* Name:                                     inTracePatternList 
* 
* Description:     Compares an input string against all entries
**                 in the trace list. 
*                          
*
* Arguments:   
* 			  str - pointer to string which is checked against the trace patterns
* Returns:     
* 			  Returns 1 (TRUE) if match found, 0 (FALSE) otherwise.  
*
* Note(s):     
* 
* *********************************************************************************************************
*/
static int inTracePatternList(char *str)
{
	for (int i=0; i < TRACE_LIST_LEN; i++)
	{
		if (PatternMatch(str,DbgTrcPattern[i]) != 0)
		{
			return(TRUE);
		}
	}
return(FALSE);
}//inTracePatternList

/*
*********************************************************************************************************
* Name:                                     inStopPatternList 
* 
* Description:     Compares an input string against all entries 
**                 in the stop list. 
*                          
*
* Arguments:   
* 			  str - pointer to string which is checked against the stop patterns
* Returns:     
* 			  Returns 1 (TRUE) if match found, 0 (FALSE) otherwise.  
*
* Note(s):     
* 
* *********************************************************************************************************
*/
static int inStopPatternList(char *str)
{
	for (int i=0; i < TRACE_LIST_LEN; i++)
	{
		if (PatternMatch(str,DbgStpPattern[i]) != 0)
		{
			return(TRUE);
		}
	}
return(FALSE);
}//inStopPatternList

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
* *********************************************************************************************************
*/
void DbgInit(void)
{
	int i; //index
	
	StopState=FALSE;//no stop once debugger initialized
	DbgLevel=0;//current debugging level
	
	//empty trace patterns lists
	for (i=0; i<TRACE_LIST_LEN; i++)
	{
		DbgTrcPattern[i][0] = '\0';
	}//for
	
	//empty stop patterns list
	for (i=0; i<STOP_LIST_LEN; i++)
	{
		DbgStpPattern[i][0] = '\0';
	}//for
	
	// default is: enable all traces in all functions
	strcpy(DbgTrcPattern[0],"*");
}//DbgInit


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
void DbgTraceStrVal(int InLevel, char *pInLabel, char *pMessageText, long InValue)
{
	//first process trace case if any triggered
	if ((InLevel <= DbgLevel) && (inTracePatternList(pInLabel) != 0))
	{
		Uart0Message(pMessageText,InValue);
	}
	
}//DbgTraceStrVal

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
void DbgTraceStr(int InLevel, char *pInLabel, char *pMessageText)
{
	//first process trace case if any triggered
	if ((InLevel <= DbgLevel) && (inTracePatternList(pInLabel) != 0))
	{
		Uart0PutStr(pMessageText);//transimt string to the terminal
	}
	
}//DbgTraceStr


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
void DbgStopStrVal(int InLevel, char *pInLabel, char *pMessageText, long InValue)
{
	//process stop case if any triggered
	if ((InLevel <= DbgLevel) && (inStopPatternList(pInLabel) != 0))
	{
		StopState=TRUE;//mark whole system is in STOP state
		Uart0Message(pMessageText,InValue);
		Uart0PutStr(STR_CMD_PROMPT_STOP_STATE);//display remote command prompt because cannot be displayed from mng_rmt
		
		while(StopState)//wait until enabled by GO command
		{
			OSTimeDly(10);//to not keep OS blocked
		}
	}//if process stop case
}//DbgStopStrVal

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
void DbgStopStr(int InLevel, char *pInLabel, char *pMessageText)
{
	//process stop case if any triggered
	if ((InLevel <= DbgLevel) && (inStopPatternList(pInLabel) != 0))
	{
		StopState=TRUE;//mark whole system is in STOP state
		Uart0PutStr(pMessageText);
		Uart0PutStr(STR_CMD_PROMPT_STOP_STATE);//display remote command prompt because cannot be displayed from mng_rmt
		while(StopState)//wait until enabled by GO command
		{
			OSTimeDly(10);//to not keep OS blocked
		}
	}//if process stop case
}//DbgStopStr
