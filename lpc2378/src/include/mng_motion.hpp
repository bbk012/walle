/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2010, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mng_motion.cpp
* Description: Manager which performs overall motion control of robot (track motors)
* Author:      Bogdan Kowalczyk
* Date:        10-January-2010
* Note:
* History:
*              10-Jan-2010 - Initial version created
*********************************************************************************************************
*/

#ifndef MNG_MOTION_HPP_
#define MNG_MOTION_HPP_

#include "mng.hpp"
#include "mw_notifier.hpp"
#include "hw_gpio.h"
#include "tsk_tracks.h"
#include "ctr_gp2d12.h"


#define  MOTION_PUBLISHER_SEND_Q_SIZE 	5
#define  MOTION_SUBSCRIBER_REC_Q_SIZE 	5
#define  MOTION_THREAD_STACK_SIZE		128
#define  MOTION_THREAD_PRIORITY   		5



//number of ms before turn command is executed to assure ARS stabilization
#define TURN_STABILIZATION_DELAY 250 //original: 500 

//timeout - number of uCOS-II ticks after which movement of tracks is checked
#define TRACK_PULSE_TIMOUT_TICKS  15

//timeout in ms which is used for reverse movement when waiting for pulses change
#define REVERSE_PULSE_TIMEOUT	200

//max number of pulses assumed for right or left 90 degree turn
#define TURN_90_DEG_PULSES 50

//max number of pulses assumed for whole 360 right or left turn
#define TURN_360_DEG_PULSES 200

//ids for directions of Wall-e:
#define DIR_NONE		0	//none direction
#define DIR_FRD			1  	//forward direction	
#define DIR_LFT			2 	//left direction
#define DIR_RGT			3	//right direction
#define DIR_REV			4	//reverse direction

//angle constants
#define ANGLE_15		15  //15 degree
#define ANGLE_30		30	//30 degree
#define ANGLE_45		45	//45 degree
#define ANGLE_60		60  //60 degree
#define ANGLE_90		90  //90 degree


//indication manager - setups windows and manages all incications (display, led, buzzer, sound etc.)
class cMotionMngr:public cMngBasePublisherSubscriber<MOTION_PUBLISHER_SEND_Q_SIZE,MOTION_SUBSCRIBER_REC_Q_SIZE,MOTION_THREAD_STACK_SIZE,MOTION_THREAD_PRIORITY>
{
public:
	
private:
	WORD mCoveredDistancePulses;//stores really covered distance in pulses when movement is broken by some stop event
	sMoveData mMoveData;//used to store CMD_MOVE notifier data
	sScanData mScanData;//used to store CMD_SCAN notifier data
	sCheckData mCheckData;//used to store CMD_CHECK notifier data
	
	//provide current covered distance after move commend execution
	WORD GetCoveredDistance(){return mCoveredDistancePulses;};
	
	//set covered distance value
	void SetCoveredDistance(WORD InDistance){mCoveredDistancePulses=InDistance;};
	
	//pure virtual thread execution function cannot be defined as Run()=0 because g++ generates very big size of code
    virtual void Run();
    
    //Generate random decision about turn Left 90 degrees or rigth and executes generated random turn
    //turn left 90 degs or right 90 degs randomly
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE RandLeftOrRightTurn();
    
    //This functions make 360 deg Wall-e movement and scan of light source and day night data
    //inputs:
    //       none but global ScanSurroundings structures are filled with data    		
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE ScanLightSurroundings();
    
    //---------------------------------------------------------------------------------------------------------------
    //Moves Wall-e to the correct positoion at forward direction
    //inputs:
    //       Position at which Wall-e should be moved being at forward direction     		
    //outputs:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE DirectFrd(BYTE Pos);
    
    //---------------------------------------------------------------------------------------------------------------
    //Moves Wall-e to the correct positoion at left direction
    //inputs:
    //       Position at which Wall-e should be moved being at left direction     		
    //outputs:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE DirectLft(BYTE Pos);
    
    //---------------------------------------------------------------------------------------------------------------
    //Moves Wall-e to the correct positoion at right direction
    //inputs:
    //       Position at which Wall-e should be moved being at right direction     		
    //outputs:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE DirectRgt(BYTE Pos);
    
    //---------------------------------------------------------------------------------------------------------------
    //Moves Wall-e to the correct positoion at reverse direction
    //inputs:
    //       Position at which Wall-e should be moved being at reverse direction     		
    //outputs:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE DirectRev(BYTE Pos);
    
    //---------------------------------------------------------------------------------------------------------------
    //This functions is used to finding max ADC reading of the foto-nose from
    //xxxLightSrcTables filled by previous ScanSurroundings call and to position Wall-e towards it.
    //inputs:
    //       none but global xxxLightSrcTables are used by it     		
    //outputs:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    //      MOVE_NONE_LIGHT_SRC - none meaningful light source was identified
    BYTE FindMaxLightScr();
    
    //---------------------------------------------------------------------------------------------------------------
    // PositionMaxLightScrFrd directs Wall-e towards the strongest light at forward position
    //inputs:
    //       none but global LightSrcTable is used by it     		
    //outputs:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    //      MOVE_NONE_LIGHT_SRC - none meaningful light source was identified	
    BYTE PositionMaxLightScrFrd(void);
    
    //move according to path 1  DistanceToMove steps
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath1(WORD DistanceToMove);
    
    //move according to path 2  DistanceToMove steps
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath2(WORD DistanceToMove);
    
    //move according to path 3  DistanceToMove steps
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath3(WORD DistanceToMove);
    
    //move according to path 4  DistanceToMove steps
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath4(WORD DistanceToMove);
    
    //move according to path 5  DistanceToMove steps
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath5(WORD DistanceToMove);
    
    //MoveOnPath6 - Path 6 is not used
    //MoveOnPath7 we call MoveOnPath1 and assume stop at an obstacle 
    //MoveOnPath8 we call MoveOnPath4 and assume stop at an obstacle
    //MoveOnPath9 we call MoveOnPath5 and assume stop at an obstacle
    
    //move according to path 8  DistanceToMove steps
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath10(WORD DistanceToMove);
    
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath11(WORD DistanceToMove);
    
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnPath12(WORD DistanceToMove);
    
    //select one from possible movement paths and move if particular path is allowed
    //returns: MOVE_OK=0 or error code
    BYTE MoveOnRandPath(WORD DistanceToMove, WORD PossiblePaths, WORD AllowedPaths);
    
    //   For current scaned state of obstacles in front of Wall-e finds all possible movement pats
    //
    //   input: ObstacleTable - table of obstacles scaned by Wall-e
    //   output: coded paths - possible Wall-e movements
    //           when bit is set path is allowed and possible
    //
    // Oct 9 2016 Original approach with 10 paths pssible was extended.
    // When sectors A, B and C are mareked as occupied system can consider movements towards them
    // that is because far segments can often detects obstacles because of some light noise
    // Path coding on a WORD BITs is defined below 
    //
    //                     H is Wall-e position
    // 
    //       RIGHT                          LEFT   BIT0 -  path 1  E&B    Empty - move forward away
    //                  +---+---+---+              BIT1 -  path 2  E&D&A  Empty - move forward away
    //                  | A | B | C |              BIT2 -  path 3  E&F&C  Empty - move forward away
    //                  +---+---+---+              BIT3 -  path 4  G&D&A  Empty - move froward away
    //                  | D | E | F |              BIT4 -  path 5  I&F&C  Empty - move forward away
    //                  +---+---+---+              BIT5 -  NOT USED
    //                  | G | H | I |              BIT6 -  path 7  E      Empty - move forward inside (path 1 reuse)
    //                  +---+---+---+              BIT7 -  path 8  G&D    Empty - move forward inside (path 4 reuse)
    //                                             BIT8 -  path 9  I&F    Empty - move forward inside (path 5 reuse)
    //                                             BIT9 -  path 10 G      Empty - move side away
    //                                             BIT10 - path 11 I      Empty - move side away  
    //                                             BIT11 - path 12 move left or right 180 degs
    //                                             BIT12 - path 13 NOT USED
    //                                             BIT13 - path 14 NOT USED
    //                                             BIT14 - path 15 NOT USED
    //                                             BIT15 - path 16 NOT USED
    //
    //
    WORD FindMovePaths(BYTE ObstacleTbl[OBSTACLE_TABLE_Y_SIZE][OBSTACLE_TABLE_X_SIZE]);
    
    //---------------------------------------------------------------------------------------------------------------
    //move specified number of puleses toward specified direction with specified speed profile
    //inputs:
    //       InDistancePulses - desired distance in pulses
    //       InDirection - desired direction (MOTOR_FORWARD, MOTOR_REVERSE)
    //       InSpeedProfile - desired profile of the movement (LOW_MID_HIGH_PROFILE, LOW_MID_PROFILE, LOW_PROFILE, MID_PROFILE, HIGH_PROFILE)    		
    //returns:
    //	MOVE_OK - O.K movement executed with none problems
    //	MOVE_BREAK_FORWARD	Error - movement cannot be realized none chnge in pulses when moving forward
    //	MOVE_BREAK_REVERSE	Error - movement cannot be realized none chnge in pulses when moving reverse
    //	MOVE_BREAK_OBSTACLE Error - distance detector detected an obstacle

    BYTE Move(WORD InDistancePulses,BYTE InDirection,BYTE InSpeedProfile);
    
    //move specified number of puleses forward
    //	MOVE_OK - O.K movement executed with none problems
    //	MOVE_BREAK_FORWARD	Error - movement cannot be realized none chnge in pulses when moving forward
    //	MOVE_BREAK_OBSTACLE Error - distance detector detected an obstacle
    BYTE MoveForward(WORD InDistancePulses,BYTE InSpeedProfile){return Move(InDistancePulses,MOTOR_FORWARD,InSpeedProfile);};
    
    //move specified number of puleses forward
    //returns:
    //	MOVE_OK - O.K movement executed with none problems
    //	MOVE_BREAK_REVERSE	Error - movement cannot be realized none chnge in pulses when moving reverse
    //	MOVE_BREAK_OBSTACLE Error - distance detector detected an obstacle
    BYTE MoveBackward(WORD InDistancePulses,BYTE InSpeedProfile){return Move(InDistancePulses,MOTOR_REVERSE,InSpeedProfile);};
    
    //turn left 90 degrees
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE TurnLeft90Deg(void);
    
    //turn left from 0 to 90 degrees
    //input:      Degree of left turn (positive value from 0 degs to 90 degs)
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_LEFT	Error - movement cannot be realized none chnge in pulses when turning left
    //      MOVE_ANGLE_LEFT  - Error - destination angle not achieved when turing left
    BYTE TurnLeftTo90Deg(long AngleInDegs);
    
    //turn right 90 degrees
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    BYTE TurnRight90Deg(void);
    
    //turn right from 0 to 90 degrees
    //input:      Degree of right turn (positive value from 0 degs to 90 degs)
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    BYTE TurnRightTo90Deg(long AngleInDegs);
    
    //turn right from 0 to 360 degrees
    //input:      Degree of right turn (positive value from 0 degs to 360 degs)
    //returns:
    //		MOVE_OK - O.K movement executed with none problems
    //		MOVE_BREAK_RIGHT	Error - movement cannot be realized none chnge in pulses when turning right
    //      MOVE_ANGLE_RIGHT  - Error - destination angle not achieved when turing right
    BYTE TurnRightTo360Deg(long AngleInDegs);
    
    //execute manager CMD_MOVE command
    //returns:
    //       nothing but internally RSP_MOVE notifier is posted
    void ProcessCmdMove(cSmartPtr<cNotifier> pNotifier);
    
    //execute manager CMD_SCAN command
    //returns:
    //       nothing but internally RSP_SCAN notifier is posted
    void ProcessCmdScan(cSmartPtr<cNotifier> pNotifier);
    
    //execute manager CMD_CHECK command
    //returns:
    //       nothing but internally RSP_CHECK notifier is posted
    void ProcessCmdCheck(cSmartPtr<cNotifier> pNotifier);
    
    //execute manager CMD_TURN_HEAD command
    //returns:
    //       nothing but internally RSP_TURN_HEAD notifier is posted
    void ProcessCmdTurnHead(cSmartPtr<cNotifier> pNotifier);
    
};// cMotionMngr

#endif /*MNG_MOTION_HPP_*/
