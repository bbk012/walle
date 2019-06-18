#ifndef MW_NOTIFIER_DATA_HPP_
#define MW_NOTIFIER_DATA_HPP_

#include "ctr_gp2d12.h" //to get access to OBSTACLE_TABLE_Y_SIZE and OBSTACLE_TABLE_X_SIZE #defines
#include "ctr_f_sens.h"


//type of command handling used by some managers
#define	SYNC_HANDLING	0x01 //function is waiting for destination manager to respond
#define ASYNC_HANDLING	0x02 //function is not waiting for a manager response

/*
*********************************************************************************************************
* Name:                            sKeyInput structure 
* 
* Description: sKeyInput encapsulates keyboard interface inputs used by user interface notifier
*       
*          
* *********************************************************************************************************
*/

//*********************************************************************************************************
// EVT_KEY data
// Structure passed through Notifier which encapsulates Keyboard User Interface Input
// currently only BUT1, BUT2 and Joystic 1 status
struct sKeyInput
{
	// Bit codded key status
	// BITx = 0 key pressed
	// BITx = 1 key released
	//
	// Meaning of bit positions
	// BIT0 - BUT1
	// BIT1 - BUT2
	// BIT2 - J1 DOWN
	// BIT3 - J1 UP
	// BIT4 - J1 LEFT
	// BIT5 - J1 RIGHT
	// BIT6 - J1 CENTER
	WORD mCurKeyStatus;//current reported stable key-pad status
	WORD mPrevKeyStatus;//previous reported stable key-pad status
	// Time stamp for send notifier - number of os ticks since sys start
	DWORD mTimeStamp;
};//sKeyInput

//*********************************************************************************************************
//structure to capture EVT_SYS_ALIVE event notifier data 
struct sSysAliveEvt
{
	// Time stamp for send notifier - number of os ticks since sys start
	DWORD mTimeStamp;
	WORD mLastPowerOffReason; 
	WORD mLastExceptionReason;
	BYTE mRAMContentPreserved;//TRUE when preserved FALSE othervise
	BYTE mLastResetReason;
	BYTE mWalleProgramToExecute;
};

//*********************************************************************************************************
//encapsulates information about status of system resources issued by
//MonitorSystemResources()function for EVT_SYS_RES Notifier
struct sSysResourcesStatus
{
	WORD mSysHeapMemLeft;//currnet number of free heap bytes
	WORD mSysHeapMinLeft;//minimum number of free heap bytes notified so far

	WORD mTotalDispatchErrorCounter;//total number of not correctly dispatched notifiers
		
};//sSysResourcesStatus

//*********************************************************************************************************
//encapsulates information about status of battery voltages issued by
//MonitorSystemResources()function for EVT_BATTERY Notifier

//board battery states
#define BOARD_BATT_NORMAL	0x01
#define BOARD_BATT_WARN		0x02
#define BOARD_BATT_LOW		0x03
#define BOARD_BATT_N_W		0x04
#define BOARD_BATT_W_L		0x05
#define BOARD_BATT_L_W		0x06
#define BOARD_BATT_W_N		0x07
#define BOARD_BATT_EMPTY	0x08

//track batteries states
#define TRACK_BATT_NORMAL	0x01
#define TRACK_BATT_WARN		0x02
#define TRACK_BATT_LOW		0x03
#define TRACK_BATT_N_W		0x04
#define TRACK_BATT_W_L		0x05
#define TRACK_BATT_L_W		0x06
#define TRACK_BATT_W_N		0x07
#define TRACK_BATT_EMPTY    0x08

//servo batteries states
#define SERVO_BATT_NORMAL	0x01
#define SERVO_BATT_WARN		0x02
#define SERVO_BATT_LOW		0x03
#define SERVO_BATT_N_W		0x04
#define SERVO_BATT_W_L		0x05
#define SERVO_BATT_L_W		0x06
#define SERVO_BATT_W_N		0x07
#define SERVO_BATT_EMPTY    0x08

struct sBatteryStatus
{
	BYTE mMainSupplyVoltage;//internal main supply battery level in volts*10 (i.e 57<->5.7 [V])
	BYTE mMainSupplyMinVoltage;//internal main supply battery minimum level registered so far in volts*10 (i.e 57<->5.7 [V])
	BYTE mMainSupplyState;//normal, warn or low state of the main power supply

	BYTE mMotorSupplyVoltage;//motor battery level in volts*10 (i.e 97<->9.7 [V])
	BYTE mMotorSupplyMinVoltage;//motor battery minimum level registered so in volts*10 (i.e 97<->9.7 [V])
	BYTE mMotorSupplyState;//normal, warn or low state of the motor supply
	
	BYTE mServoSupplyVoltage;//servo battery level in volts*10 (i.e 47<->4.7 [V])
	BYTE mServoSupplyMinVoltage;//servo battery minimum level registered so in volts*10 (i.e 47<->4.7 [V])
	BYTE mServoSupplyState;//normal, warn or low state of the servo supply
	
};//sBatteryStatus

//*********************************************************************************************************
//CMD_INDICATOR and RSP_INDICATOR sub-commands IDs

#define CTR_NONE_SCMD_ID		0x00

#define CTR_LED_SCMD_ID 		0x01//used to control LED 
#define CTR_BUZZER_SCMD_ID 		0x02//used to control BUZZER
#define CTR_MAIN_POWER_SCMD_ID 	0x03//used to turn power OFF

//desired indictor state
#define CTR_STATE_ON	0x01
#define CTR_STATE_OFF	0x00

//used by CMD_INDICATOR or RSP_INDICATOR
struct sIndicatorData
{
	BYTE mIndicatorCmdId;//sub-command id for the CMD_INDICATOR 
	BYTE mState;//0 - turn off, 1 - turn on
};//sIndicatorData

//*********************************************************************************************************
// CMD_MOVE_ARM command and RSP_MOVE_ARM sub-commands IDs
#define ARM_NONE_SCMD_ID				0x00

#define MOVE_LEFT_ARM_SCMD_ID			0x01
#define MOVE_LEFT_ARM_HOME_SCMD_ID		0x02
#define MOVE_LEFT_ARM_SYNC_SCMD_ID		0x03
#define MOVE_LEFT_ARM_OPPOSITE_SCMD_ID	0x04
#define GET_LEFT_ARM_POS_SCMD_ID		0x05

#define MOVE_RIGHT_ARM_SCMD_ID			0x06
#define MOVE_RIGHT_ARM_HOME_SCMD_ID		0x07
#define MOVE_RIGHT_ARM_SYNC_SCMD_ID		0x08
#define MOVE_RIGHT_ARM_OPPOSITE_SCMD_ID	0x09
#define GET_RIGHT_ARM_POS_SCMD_ID		0x0A

#define ARM_SERVO_ON_SCMD_ID			0x0B
#define ARM_SERVO_OFF_SCMD_ID			0x0C

//used by CMD_MOVE_ARM command and RSP_MOVE_ARM response
struct sMoveArmData
{
	BYTE mArmCmdId; //sub-command id for the arm command ids #define above 
	WORD mArmCount; //number of counts coresponding to the desired arm position
};//sMoveArmData

//*********************************************************************************************************
//CMD_RTC sub-commands defines
#define RTC_NONE_SCMD_ID				0x00

#define SET_TIME_DATE_SCMD_ID			0x01

#define GET_TIME_DATE_SCMD_ID			0x02

#define SET_ALARM_TIME_DATE_SCMD_ID		0x03

#define GET_ALARM_TIME_DATE_SCMD_ID		0x04

#define CLR_ALARM_FOR_TIME_DATE_SCMD_ID	0x05


//used by CMD_RTC, RSP_RTC, EVT_TIME, EVT_ALARM
struct sRtcData
{
	BYTE mRtcCmdId; //sub-command id for the RTC command see ids #define above
	BYTE mIsAlarmSetupInRTC;//TRUE when alarm is setup in RTC
	BYTE mIsAlarmTriggeredByRTC;//TRUE when alarm is triggered by RTC
	WORD mYear;		//year (0 to 4095)
	BYTE mMonth;	//month (1 to 12)
	BYTE mDay;		//day of month (1 to 28, 29, 30 or 31)
	BYTE mDayOfWeek;//day of a week (0 to 6) IMPORTANT! This value is not used to triger alarm, 0 is Sunday
	BYTE mHour;		//hours value (0 to 23)
	BYTE mMinute;	//minutes value (0 to 59)
	BYTE mSecond;	//seconds value (0 to 59)
};//sRtcData
//limits for data/time in the WALL-e system
#define MIN_YEAR 		2017
#define MAX_YEAR 		4095

#define MIN_MONTH 		1
#define MAX_MONTH 		12

#define MIN_DAY   		1
#define MAX_DAY   		31

#define MIN_DAY_WEEK	0
#define MAX_DAY_WEEK	6

#define MIN_HOUR		0
#define MAX_HOUR		23

#define MIN_MINUTE		0
#define MAX_MINUTE		59

#define MIN_SECOND		0
#define MAX_SECOND		59


//*********************************************************************************************************
//used by CMD_MOVE, RSP_MOVE
#define	MOVE_NONE_SCMD_ID				0x00

#define FORWARD_SCMD_ID   					0x01 //sub-command to move Walle forward
#define REVERSE_SCMD_ID						0x02 //sub-command to move Walle backward
#define LEFT90DEG_SCMD_ID					0x03 //sub-command to turn Walle left 90 degrees
#define TURN_LEFT_SCMD_ID       			0x04 //sub-command for left turn specify degree from 0 to 90
#define RIGHT90DEG_SCMD_ID					0x05 //sub-command to turn Walle right 90 degrees
#define TURN_RIGHT_SCMD_ID      			0x06 //sub-command to turn right specided degrees from 0 to 90
#define RAND_TURN_SCMD_ID				 	0x07 //random left or right 90 degrees turn
#define RAND_PATH_MOVE_SCMD_ID			  	0x08 //move according to rundomly slected path based on curent obstacle scanning results
#define FIND_MAX_LIGHT_SRC_SCMD_ID		  	0x09 //identify and position towards a light source 
#define POSITION_AT_MAX_LIGHT_FRD_SCMD_ID	0x0A //being at forward direction position at max light source

//BIT coded Allowed Paths for RAND_PATH_MOVE used to setup mSpeedProfile for that subcommand
#define PATH1	BIT0
#define PATH2	BIT1
#define PATH3	BIT2
#define PATH4	BIT3
#define PATH5	BIT4
#define PATH6	BIT5
#define PATH7	BIT6
#define PATH8	BIT7
#define PATH9	BIT8
#define PATH10	BIT9

//movement results constants

#define MOVE_OK 	         0  // O.K. movement executed

#define MOVE_BREAK_FORWARD	10 	// Error - movement cannot be realized none chnge in pulses when moving forward
#define MOVE_BREAK_REVERSE	11	// Error - movement cannot be realized none chnge in pulses when moving reverse
#define MOVE_BREAK_LEFT		12	// Error - movement cannot be realized none chnge in pulses when turning left
#define MOVE_BREAK_RIGHT	13	// Error - movement cannot be realized none chnge in pulses when turning right

#define MOVE_BREAK_OBSTACLE 20 	// Error - distance detector detected an obstacle

#define MOVE_ANGLE_LEFT     30 	// Error - destination angle not achieved when turning left
#define MOVE_ANGLE_RIGHT    35  // Error - destination angle not achieved when turning right

#define MOVE_NONE_PATH		40  // Error - there is not any path to move
#define MOVE_NONE_LIGHT_SRC	50	// None light source was detected by FindMaxLightScr() member function
#define MOVE_NO_NIGHT		60	// There is none night when light source identification was requested

struct sMoveData
{
	BYTE mMoveCmdId; //sub-command id which identifies movement direction FORWARD or REVERSE or any other movement
	WORD mDistancePulses;//distance of movement in pulses or degree of turn for RSP_MOVE covered distance    	
	WORD mSpeedProfile;//movement speed profile WORD used to accomodate allowed paths for RAND_PATH_MOVE
	BYTE mResult;//movement resultd for RSP_MOVE
};//sMoveData

//*********************************************************************************************************
//used by CMD_SCAN and RSP_SCAN
#define SCAN_LIGHT_OBSTACLE_SCMD_ID		0x01 //scan at same time for light source and obstacle (from L60 to R60)
#define SCAN_OBSTACLE_SCMD_ID			0x02 //scan only for obstacle (from L60 to R60)
#define SCAN_LIGHT_SCMD_ID				0x03 //scan only for light source (from L60 to R60)

struct sScanData
{
	BYTE mScanDataCmdId; //sub-command id which identifies type of scanning command
	BYTE mObstacleTable[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE];
	WORD mLightSrcTable[LIGHT_SRC_TABLE_SIZE];
};//sScanData


//*********************************************************************************************************
//used by CMD_CHECK and RSP_CHECK
#define CHECK_OBSTACLE_SCMD_ID		0x01 //check for detected obstacle using IRED and US fusion
#define CHECK_IRED_OBSTACLE_SCMD_ID	0x02 //check for detected obstacle using IRED sensor
#define CHECK_US_OBSTACLE_SCMD_ID	0x03 //check for detected obstacle using US sensor
#define CHECK_FRONT_LIGHT_SCMD_ID	0x04 //check for foto nose data
#define CHECK_BACK_LIGHT_SCMD_ID	0x05 //check for foto tail data

struct sCheckData
{
	BYTE mCheckDataCmdId;//sub-command id 
	BYTE mType;//type of obstacle ahead the Wall-e
	WORD mRawData;//raw data read from IRED
	WORD mRawUsData;//raw data from US
};// sCheckObstacleData

//*********************************************************************************************************
//used by CMD_TURN_HEAD and RSP_TURN_HEAD
#define TURN_HEAD_ON_SCMD_ID			0x01 //turn on head servo
#define TURN_HEAD_OFF_SCMD_ID			0x02 //turn off head servo
#define TURN_HEAD_MOVE_SCMD_ID			0x03 //move head to defined position

struct sTurnHeadData
{
	BYTE mTurnHeadCmdId;//sub-command id 
	WORD mTurnHeadPosition;//position to place head at
};//sTurnHeadData

//*********************************************************************************************************
//used by EVT_DAY_NIGHT event to provide fotoresistor reading (measured volts and state of a day light
//DARK_DAY	this corresponds to the night	
//GREY_DAY		
//BRIGHT_DAY	

struct sDayStateData
{
	BYTE mDayState; //DARK_DAY, GREY_DAY, BRIGHT_DAY
	WORD mRawFotoResistorVolt;//A/D reading of the fotoresitor voltage
};//sDayStateData

//*********************************************************************************************************
//used by EVT_DISPLAY_INFO event to pass critical information to be diplayed
#define EVT_DISPLAY_INFO_TEXT_SIZE	16 //maximum size of the critical information text to be displayed

#define EVT_DISPLAY_CRITICAL_ID		0x01 //ID for critical information 

struct sDspInfoEvt
{
	BYTE mId; //Id of requested information
	char mText[EVT_DISPLAY_INFO_TEXT_SIZE+1];//storage for short text to be displayed
};

//*********************************************************************************************************
//used by CMD_EXE_CMD notifier and RSP_EXE_CMD handled by MNG_EXE when external commands 
//are directed to be executed by it 

#define CMD_EXE_TURN_HEAD_ID	0x01
#define CMD_EXE_LEFT_ARM_ID		0x02
#define CMD_EXE_RIGHT_ARM_ID	0x03
#define CMD_EXE_ARM_SYNC_ID		0x04
#define CMD_EXE_FORWARD_ID		0x05
#define CMD_EXE_REVERSE_ID		0x06

#define TURN_LEFT_90			0x07
#define TURN_RIGHT_90			0x08
#define TURN_360				0x09

#define MOVE_SHORT_DISTANCE		24 //number of track pulses corresponding to short movement request
#define MOVE_LITTLE_DISTANCE	48 //number of track pulses corresponding to little movement request
#define MOVE_FAR_DISTANCE		96 //number of track pulses corresponding to far movement request

struct sCmdExeData
{
	BYTE mCmdExeId; //Id of requested command
	WORD mParameterValue;//value of a command paramiter which depends on requested command
	BYTE mHandling;//synchronous or asynchronous handling type
};//sCmdExeData


//*********************************************************************************************************
#endif /*MW_NOTIFIER_DATA_HPP_*/
