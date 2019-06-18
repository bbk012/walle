/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_time.c
* Description: time & date manipulation functions
* Author:      Bogdan Kowalczyk
* Date:        20-Dec-2014
* Note:
*              This is based on Michael Margolis 2009-2014 Arduino time library
* History:
*              20-Dec-2014 - Initial version created
*********************************************************************************************************
*/

#include "lib_time.h"
#include "lib_std.h" // for strcpy

static tmElements_t tm;          // a cache of time elements

//time_t - time as seconds since Jan 1 1970
int Hour(time_t t) { // the hour for the given time
	BreakTime(t,&tm);
  return tm.Hour;  
}//Hour

//time_t - time as seconds since Jan 1 1970
int HourFormat12(time_t t) { // the hour for the given time in 12 hour format
	BreakTime(t,&tm);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}//HourFormat12

//time_t - time as seconds since Jan 1 1970
BYTE isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);  
}//isAM

//time_t - time as seconds since Jan 1 1970
BYTE isPM(time_t t) { // returns true if PM
  return (Hour(t) >= 12); 
}//isPM


//time_t - time as seconds since Jan 1 1970
int Minute(time_t t) { // the minute for the given time
	BreakTime(t,&tm);
  return tm.Minute;  
}//Minute

//time_t - time as seconds since Jan 1 1970
int Second(time_t t) {  // the second for the given time
	BreakTime(t,&tm);
  return tm.Second;
}//Second

//time_t - time as seconds since Jan 1 1970
int Day(time_t t) { // the day for the given time (0-6)
	BreakTime(t,&tm);
  return tm.Day;
}//Day

//time_t - time as seconds since Jan 1 1970
int WeekDay(time_t t) {
	BreakTime(t,&tm);
  return tm.Wday;
}//WeekDay

//time_t - time as seconds since Jan 1 1970
int Month(time_t t) {  // the month for the given time
	BreakTime(t,&tm);
  return tm.Month;
}//Month

//time_t - time as seconds since Jan 1 1970
int Year(time_t t) { // the year for the given time
	BreakTime(t,&tm);
  return tmYearToCalendar(tm.Year);
}//Year

//time_t - time as seconds since Jan 1 1970
time_t MinutesToTime_t(BYTE Minutes)
{
	return ((time_t)Minutes) * SECS_PER_MIN;  
}//MinutesToTime_t

//time_t - time as seconds since Jan 1 1970
time_t HoursToTime_t(BYTE Hours)
{
	return ((time_t)Hours) * SECS_PER_HOUR;
}//HoursToTime_t

//time_t - time as seconds since Jan 1 1970
time_t DaysToTime_t(BYTE Days)
{
	return ((time_t)Days)* SECS_PER_DAY;
}//DaysToTime_t

//time_t - time as seconds since Jan 1 1970
time_t DaysHoursMinutesSecondsToTime_t(BYTE Days,BYTE Hours,BYTE Minutes,BYTE Seconds)
{
	time_t time=0;
	
	time+=DaysToTime_t(Days);
	time+=HoursToTime_t(Hours);
	time+=MinutesToTime_t(Minutes);
	time+=(time_t)Seconds;
	
	return time;
}//DaysHoursMinutesSecondsToTime_t

//time_t - time as seconds since Jan 1 1970
time_t WeeksToTime_t(BYTE Weeks)
{
	return ((time_t)Weeks)* SECS_PER_WEEK;
}//DaysToTime_t



/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const BYTE monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

//break given time_t - time as seconds since Jan 1 1970
//into time components in tmElements_t structure
void BreakTime(time_t timeInput, tmElements_t *ptm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  BYTE year;
  BYTE month, monthLength;
  DWORD time;
  DWORD days;

  time = (DWORD)timeInput;
  ptm->Second = time % 60;
  time /= 60; // now it is minutes
  ptm->Minute = time % 60;
  time /= 60; // now it is hours
  ptm->Hour = time % 24;
  time /= 24; // now it is days
  ptm->Wday = ((time + 4) % 7);  // Sunday is day 0 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  ptm->Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  ptm->Month = month + 1;  // jan is month 1  
  ptm->Day = time + 1;     // day of month
}//BreakTime

//assembles time components of tmElements_t 
//into time_t - time as seconds since Jan 1 1970
time_t MakeTime(tmElements_t *ptm){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  DWORD seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= (ptm->Year)*(SECS_PER_DAY * 365);
  for (i = 0; i < ptm->Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < ptm->Month; i++) {
    if ( (i == 2) && LEAP_YEAR(ptm->Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= ((ptm->Day)-1) * SECS_PER_DAY;
  seconds+= (ptm->Hour) * SECS_PER_HOUR;
  seconds+= (ptm->Minute) * SECS_PER_MIN;
  seconds+= ptm->Second;
  return (time_t)seconds; 
}//MakeTime


#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned char **)(addr))

 
// the short strings for each day or month must be exactly dt_SHORT_STR_LEN
#define dt_SHORT_STR_LEN  3 // the length of short strings
#define dt_MAX_STRING_LEN 9 // length of longest date string (excluding terminating null)

static char buffer[dt_MAX_STRING_LEN+1];  // must be big enough for longest string and the terminating null

const char monthStr0[] = "Error";
const char monthStr1[] = "January";
const char monthStr2[] = "February";
const char monthStr3[] = "March";
const char monthStr4[] = "April";
const char monthStr5[] = "May";
const char monthStr6[] = "June";
const char monthStr7[] = "July";
const char monthStr8[] = "August";
const char monthStr9[] = "September";
const char monthStr10[] ="October";
const char monthStr11[] ="November";
const char monthStr12[] ="December";

const char * const monthNames_P[] =
{
    monthStr0,monthStr1,monthStr2,monthStr3,monthStr4,monthStr5,monthStr6,
    monthStr7,monthStr8,monthStr9,monthStr10,monthStr11,monthStr12
};

const char monthShortNames_P[] = "ErrJanFebMarAprMayJunJulAugSepOctNovDec";

const char dayStr0[] = "Sunday";
const char dayStr1[] = "Monday";
const char dayStr2[] = "Tuesday";
const char dayStr3[] = "Wednesday";
const char dayStr4[] = "Thursday";
const char dayStr5[] = "Friday";
const char dayStr6[] = "Saturday";

const char * const dayNames_P[] =
{
   dayStr0,dayStr1,dayStr2,dayStr3,dayStr4,dayStr5,dayStr6
};

const char dayShortNames_P[] = "SunMonTueWedThrFriSat";

/* functions to return date strings */
//return long month name string for given month number
char* MonthStr(BYTE InMonth)
{
	if(InMonth>12)InMonth=0;//protect against wrong range;
    strcpy(buffer,monthNames_P[InMonth]);
    return buffer;
}

//return short month string name for given month number
char* MonthShortStr(BYTE month)
{
   if(month>12)month=0;//protect against wrong range;
   for (int i=0; i < dt_SHORT_STR_LEN; i++)      
      buffer[i] = pgm_read_byte(&(monthShortNames_P[i+ (month*dt_SHORT_STR_LEN)]));  
   buffer[dt_SHORT_STR_LEN] = 0;
   return buffer;
}

//return day name for given week day
char* DayStr(BYTE Wday) //return string name for a week day
{
   if(Wday>6) //protect against wrong day number
	   strcpy(buffer,"Err");
   else
	   strcpy(buffer,dayNames_P[Wday]);
   return buffer;
}

//return short day name for given week day
char* DayShortStr(BYTE Wday) //return short string name for a week day
{
   BYTE idx = Wday*dt_SHORT_STR_LEN;
   
   if(Wday>6) //protect against wrong day number
	   strcpy(buffer,"Err");
   else
   {
   for (int i=0; i < dt_SHORT_STR_LEN; i++)      
      buffer[i] = pgm_read_byte(&(dayShortNames_P[idx + i]));  
   buffer[dt_SHORT_STR_LEN] = 0;
   }
   return buffer;
}
