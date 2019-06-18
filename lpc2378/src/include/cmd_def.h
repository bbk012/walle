/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2018, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        cmd_def.h
* Description: Definition of voice command sequencies used by Wall-e EasyVR module
* Author:      Bogdan Kowalczyk
* Date:        21-Jan-2018
* History:
* 21-Jan-2018 - Initial version
* 
*********************************************************************************************************
*/

#ifndef CMD_DEF_H_
#define CMD_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"	
	
//indexes to Easy VR Sound Table with various Wall-e's responses/sounds
//sound indexes and related timeouts  (in miliseconds) used by PlaySnd method
#define SND_BEEP					0
#define TIMEOUT_SND_BEEP    		500
//
#define SND_WALLE					1
#define TIMEOUT_SND_WALLE			1700

#define SND_READY					2
#define TIMEOUT_SND_READY			1600

#define SND_CMD_BROKEN				3
#define TIMEOUT_SND_CMD_BROKEN		1500

#define SND_CMD_UNKNOWN				4
#define TIMEOUT_SND_CMD_UNKNOWN		2000

#define SND_CMD_ERROR				5
#define TIMEOUT_SND_CMD_ERROR		2200

#define SND_REPEAT					6
#define TIMEOUT_SND_REPEAT			1700

#define SND_CMD_REPEAT				7
#define TIMEOUT_SND_CMD_REPEAT		3800

#define SND_CMD_SLEEP				8
#define TIMEOUT_SND_CMD_SLEEP		2800

#define SND_SLEEP					9
#define TIMEOUT_SND_SLEEP			2000

#define SND_TURN_OFF				10
#define TIMEOUT_SND_TURN_OFF		3100

#define SND_HELLO					11
#define TIMEOUT_SND_HELLO			800

#define SND_YES						12
#define TIMEOUT_SND_YES				900

#define SND_NO						13
#define TIMEOUT_SND_NO				1100

#define SND_BOGDAN					14
#define TIMEOUT_SND_BOGDAN			3000

#define SND_CONFIRM					15
#define TIMEOUT_SND_CONFIRM			1800

#define SND_OBSTACLE				16
#define TIMEOUT_SND_OBSTACLE		1900

//definition of indexes behind EasyVR groups of commands recognized and used by Wall-e
//and cVrmMngr::GetVoiceCmd method
//cVrmMngr class member DWORD mVrAssembledCmd - is used to code assembled command
//every voice command has a form of <object>[<activity>][<parameter>]
//object is comming from COMMAND GROUP 1
//activity is comming from COMMAND GROUP 2
//parameter is comming from COMMAND GROUP 3
//in a process of assemling command sobsequent elements are coded on mVrAssembledCmd as depicted below
//   31                                  0
//   +-----------------------------------+
//   |Special |Group 3 |Group 2 |Group 1 |
//   +-----------------------------------+
// Every group command index is setup from 8 bits with groups arranged as above
// Special group is used for special purposes like for example to define state (unassembled/assembled command)
// Group 1
#define VR_IDX_CMD_WHO			0x00000000
#define VR_IDX_CMD_KNOW			0x00000001
#define VR_IDX_CMD_WALLE		0x00000002
#define VR_IDX_CMD_HEAD			0x00000003
#define VR_IDX_CMD_LEFT_ARM		0x00000004
#define VR_IDX_CMD_RIGHT_ARM	0x00000005
#define VR_IDX_CMD_ARMS			0x00000006
// Group 2
#define VR_IDX_CMD_BREAK2		0x00000000
#define VR_IDX_CMD_TURN_OFF		0x00000100
#define VR_IDX_CMD_SLEEP		0x00000200
#define VR_IDX_CMD_BACKWARD		0x00000300
#define VR_IDX_CMD_FORWARD		0x00000400
#define VR_IDX_CMD_LEFT			0x00000500
#define VR_IDX_CMD_RIGHT		0x00000600
#define VR_IDX_CMD_TURN_AROUND	0x00000700
#define VR_IDX_CMD_PROGRAM		0x00000800
#define VR_IDX_CMD_UP			0x00000900
#define VR_IDX_CMD_DOWN			0x00000A00
#define VR_IDX_CMD_SHAKE        0x00000B00
#define VR_IDX_CMD_CENETER		0x00000C00
// Group 3
#define VR_IDX_CMD_BREAK3		0x00000000
#define VR_IDX_CMD_SHORT		0x00010000
#define VR_IDX_CMD_LITTLE		0x00020000
#define VR_IDX_CMD_FAR			0x00030000
#define VR_IDX_CMD_SOME			0x00040000
#define VR_IDX_CMD_ALOT			0x00050000
#define VR_IDX_CMD_VOICE_CTRL	0x00060000
#define VR_IDX_CMD_TEST			0x00070000
#define VR_IDX_CMD_ENJOY		0x00080000
#define VR_IDX_CMD_BATH			0x00090000
	
#define VR_IDX_CMD_EMPTY		0x00000000 //start point before anything is assembled
	
//mask used to extract only one group
#define VR_CMD_MASK_GRP1			0x000000FF
#define VR_CMD_MASK_GRP2			0x0000FF00
#define VR_CMD_MASK_GRP3			0x00FF0000

#define VR_CMD_MASK_GRP1_GRP2		0x0000FFFF
#define VR_CMD_MASK_GRP1_GRP2_GRP3  0x00FFFFFF

	
//Wall-e command allowed and recognizable command sequencies
//according to syntach <object>[<activity>][<parameter>]
//those are used by VoiceCmdSeqList table and other functions of cVrmMngr
	
#define VR_CMD_SEQ1		VR_IDX_CMD_WHO   
#define VR_CMD_SEQ2		VR_IDX_CMD_KNOW	
#define VR_CMD_SEQ3		VR_IDX_CMD_WALLE|VR_IDX_CMD_TURN_OFF
#define VR_CMD_SEQ4		VR_IDX_CMD_WALLE|VR_IDX_CMD_SLEEP
#define VR_CMD_SEQ5		VR_IDX_CMD_WALLE|VR_IDX_CMD_BACKWARD|VR_IDX_CMD_SHORT
#define VR_CMD_SEQ6		VR_IDX_CMD_WALLE|VR_IDX_CMD_BACKWARD|VR_IDX_CMD_LITTLE
#define VR_CMD_SEQ7		VR_IDX_CMD_WALLE|VR_IDX_CMD_BACKWARD|VR_IDX_CMD_FAR	
#define VR_CMD_SEQ8		VR_IDX_CMD_WALLE|VR_IDX_CMD_FORWARD|VR_IDX_CMD_SHORT
#define VR_CMD_SEQ9		VR_IDX_CMD_WALLE|VR_IDX_CMD_FORWARD|VR_IDX_CMD_LITTLE
#define VR_CMD_SEQ10	VR_IDX_CMD_WALLE|VR_IDX_CMD_FORWARD|VR_IDX_CMD_FAR	
#define VR_CMD_SEQ11	VR_IDX_CMD_WALLE|VR_IDX_CMD_LEFT	
#define VR_CMD_SEQ12	VR_IDX_CMD_WALLE|VR_IDX_CMD_RIGHT
#define VR_CMD_SEQ13	VR_IDX_CMD_WALLE|VR_IDX_CMD_TURN_AROUND
#define VR_CMD_SEQ14	VR_IDX_CMD_WALLE|VR_IDX_CMD_PROGRAM|VR_IDX_CMD_VOICE_CTRL	
#define VR_CMD_SEQ15	VR_IDX_CMD_WALLE|VR_IDX_CMD_PROGRAM|VR_IDX_CMD_TEST
#define VR_CMD_SEQ16	VR_IDX_CMD_WALLE|VR_IDX_CMD_PROGRAM|VR_IDX_CMD_ENJOY
#define VR_CMD_SEQ17	VR_IDX_CMD_WALLE|VR_IDX_CMD_PROGRAM|VR_IDX_CMD_BATH
#define VR_CMD_SEQ18	VR_IDX_CMD_HEAD|VR_IDX_CMD_LEFT|VR_IDX_CMD_SOME
#define VR_CMD_SEQ19	VR_IDX_CMD_HEAD|VR_IDX_CMD_LEFT|VR_IDX_CMD_ALOT
#define VR_CMD_SEQ20	VR_IDX_CMD_HEAD|VR_IDX_CMD_RIGHT|VR_IDX_CMD_SOME
#define VR_CMD_SEQ21	VR_IDX_CMD_HEAD|VR_IDX_CMD_RIGHT|VR_IDX_CMD_ALOT
#define VR_CMD_SEQ22	VR_IDX_CMD_LEFT_ARM|VR_IDX_CMD_UP|VR_IDX_CMD_SOME
#define VR_CMD_SEQ23	VR_IDX_CMD_LEFT_ARM|VR_IDX_CMD_UP|VR_IDX_CMD_ALOT
#define VR_CMD_SEQ24	VR_IDX_CMD_LEFT_ARM|VR_IDX_CMD_DOWN|VR_IDX_CMD_SOME
#define VR_CMD_SEQ25	VR_IDX_CMD_LEFT_ARM|VR_IDX_CMD_DOWN|VR_IDX_CMD_ALOT	
#define VR_CMD_SEQ26	VR_IDX_CMD_RIGHT_ARM|VR_IDX_CMD_UP|VR_IDX_CMD_SOME
#define VR_CMD_SEQ27	VR_IDX_CMD_RIGHT_ARM|VR_IDX_CMD_UP|VR_IDX_CMD_ALOT
#define VR_CMD_SEQ28	VR_IDX_CMD_RIGHT_ARM|VR_IDX_CMD_DOWN|VR_IDX_CMD_SOME
#define VR_CMD_SEQ29	VR_IDX_CMD_RIGHT_ARM|VR_IDX_CMD_DOWN|VR_IDX_CMD_ALOT	
#define VR_CMD_SEQ30	VR_IDX_CMD_ARMS|VR_IDX_CMD_UP|VR_IDX_CMD_SOME
#define VR_CMD_SEQ31	VR_IDX_CMD_ARMS|VR_IDX_CMD_UP|VR_IDX_CMD_ALOT
#define VR_CMD_SEQ32	VR_IDX_CMD_ARMS|VR_IDX_CMD_DOWN|VR_IDX_CMD_SOME
#define VR_CMD_SEQ33	VR_IDX_CMD_ARMS|VR_IDX_CMD_DOWN|VR_IDX_CMD_ALOT
#define VR_CMD_SEQ34	VR_IDX_CMD_HEAD|VR_IDX_CMD_SHAKE
#define VR_CMD_SEQ35	VR_IDX_CMD_HEAD|VR_IDX_CMD_CENETER
#define VR_CMD_SEQ36	VR_IDX_CMD_LEFT_ARM|VR_IDX_CMD_CENETER
#define VR_CMD_SEQ37	VR_IDX_CMD_RIGHT_ARM|VR_IDX_CMD_CENETER
#define VR_CMD_SEQ38	VR_IDX_CMD_ARMS|VR_IDX_CMD_CENETER
	
#define VR_CMD_END		VR_IDX_CMD_EMPTY|BIT31     //last entry of VoiceCmdSeqList is marked by BIT31=1


#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*CMD_DEF_H_*/
