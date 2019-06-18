/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2014, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_time.h
* Description: time & date manipulation functions
* Author:      Bogdan Kowalczyk
* Date:        20-Dec-2014
* Note:
*              This is based on Michael Margolis 2009-2014 Arduino time library
* History:
*              20-Dec-2014 - Initial version created
*********************************************************************************************************
*/
#ifndef LIB_TIME_H_
#define LIB_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "type.h"	
	
typedef DWORD time_t; // time as seconds since Jan 1 1970 (this was thursday)


typedef struct  { 
  BYTE Second; 
  BYTE Minute; 
  BYTE Hour; 
  BYTE Wday;   // day of week, sunday is day 0
  BYTE Day;
  BYTE Month; 
  BYTE Year;   // offset from 1970; 
} 	tmElements_t;



//convenience macros to convert to and from tm years 
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year 
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)   

/*==============================================================================*/
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_)  ((( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)) // 0 = Sunday
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define elapsedSecsToday(_time_)  (_time_ % SECS_PER_DAY)   // the number of seconds since last midnight 
// The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
// Always set the correct time before settting alarms
#define previousMidnight(_time_) (( _time_ / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define nextMidnight(_time_) ( previousMidnight(_time_)  + SECS_PER_DAY )   // time at the end of the given day 
#define elapsedSecsThisWeek(_time_)  (elapsedSecsToday(_time_) +  ((dayOfWeek(_time_)-1) * SECS_PER_DAY) )   // note that week starts on day 1
#define previousSunday(_time_)  (_time_ - elapsedSecsThisWeek(_time_))      // time at the start of the week for the given time
#define nextSunday(_time_) ( previousSunday(_time_)+SECS_PER_WEEK)          // time at the end of the week for the given time


/*============================================================================*/
/*  time and date functions   */
int     Hour(time_t t);    // the hour for the given time time_t - time as seconds since Jan 1 1970
int     HourFormat12(time_t t); // the hour for the given time in 12 hour format time_t - time as seconds since Jan 1 1970

BYTE isAM(time_t t);    // returns true the given time is AM time_t - time as seconds since Jan 1 1970

BYTE isPM(time_t t);    // returns true the given time is PM time_t - time as seconds since Jan 1 1970

int     Minute(time_t t);  // the minute for the given time time_t - time as seconds since Jan 1 1970
 
int     Second(time_t t);  // the second for the given time time_t - time as seconds since Jan 1 1970
 
int     Day(time_t t);     // the day for the given time time_t - time as seconds since Jan 1 1970
 
int     WeekDay(time_t t); // the weekday for the given time 0 - Sunday time_t - time as seconds since Jan 1 1970

int     Month(time_t t);   // the month for the given time Jan is month 1 time_t - time as seconds since Jan 1 1970

int     Year(time_t t);    // the year for the given time (2009, 2010 etc) time_t - time as seconds since Jan 1 1970 

time_t MinutesToTime_t(BYTE Minutes); //time_t - time as seconds since Jan 1 1970
time_t HoursToTime_t(BYTE Hours); //time_t - time as seconds since Jan 1 1970
time_t DaysToTime_t(BYTE Days);// time_t - time as seconds since Jan 1 1970
time_t DaysHoursMinutesSecondsToTime_t(BYTE Days,BYTE Hours,BYTE Minutes,BYTE Seconds);//time_t - time as seconds since Jan 1 1970
time_t WeeksToTime_t(BYTE Weeks);//time_t - time as seconds since Jan 1 1970

/* date strings */ 

char* MonthStr(BYTE month);//return long month name string for given month number
char* MonthShortStr(BYTE month);//return short month name string for given month number

char* DayStr(BYTE Wday);//return string name for a week day
char* DayShortStr(BYTE Wday);//return short string name for a week day
	
/* low level functions to convert to and from system time                     */
void BreakTime(time_t time, tmElements_t *ptm);  // break time_t into elements, time_t - time as seconds since Jan 1 1970
time_t MakeTime(tmElements_t *ptm);  // convert time elements into time_t, time_t - time as seconds since Jan 1 1970
	
	
#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*LIB_TIME_H_*/
