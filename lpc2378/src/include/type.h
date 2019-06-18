/*****************************************************************************
 *   type.h:  Type definition Header file for NXP LPC230x Family 
 *   Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef TYPE_H_
#define TYPE_H_
#ifdef __cplusplus
   extern "C" {
#endif
	   

#ifndef __TYPE_H__
#define __TYPE_H__

/* Specify an extension for GCC based compilers used by Microchip Graphics Library*/
#if defined(__GNUC__)
	#define __EXTENSION __extension__
#else
	#define __EXTENSION
#endif

#if !defined(__PACKED)
    #define __PACKED
#endif


#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

typedef unsigned char  BYTE; //8 bits
typedef unsigned short WORD; //16 bits
typedef unsigned long  DWORD;//32 bits
typedef unsigned int   BOOL;//32 bits

typedef signed char             CHAR;                           /* 8-bit signed    */
typedef signed short int        SHORT;                          /* 16-bit signed   */
typedef signed long             LONG;                           /* 32-bit signed   */


#define   BIT0        0x00000001
#define   BIT1        0x00000002
#define   BIT2        0x00000004
#define   BIT3        0x00000008
#define   BIT4        0x00000010
#define   BIT5        0x00000020
#define   BIT6        0x00000040
#define   BIT7        0x00000080
#define   BIT8        0x00000100
#define   BIT9        0x00000200
#define   BIT10       0x00000400
#define   BIT11       0x00000800
#define   BIT12       0x00001000
#define   BIT13       0x00002000
#define   BIT14       0x00004000
#define   BIT15       0x00008000
#define   BIT16       0x00010000
#define   BIT17       0x00020000
#define   BIT18       0x00040000
#define   BIT19       0x00080000
#define   BIT20       0x00100000
#define   BIT21       0x00200000
#define   BIT22       0x00400000
#define   BIT23       0x00800000
#define   BIT24       0x01000000
#define   BIT25       0x02000000
#define   BIT26       0x04000000
#define   BIT27       0x08000000
#define   BIT28       0x10000000
#define   BIT29       0x20000000
#define   BIT30       0x40000000
#define   BIT31       0x80000000

//obstacle detection constants used by IRED and US sensors
#define OBSTACLE_SURFACE 			 0 //- none obstacle - normal distance detector reading corresponding to the movement surface
#define OBSTACLE_CHASM 				 1 //- sudden openning in the surface
#define OBSTACLE_FAR_DISTANCE 		 2 //- obstacle detected in the distance about 30cm <= d < 60 cm
#define OBSTACLE_SHORT_DISTANCE 	 3 //- obstacle detected in the distance < 30cm
#define OBSTACLE_VERY_SHORT_DISTANCE 4 //- obstacle pretty close to Wall-e

#define OK_STATE			1	//for all OK genaral situations to not use TRUE
#define NOK_STATE 			0   //foe all NOK general situations to not use FASLSE


#endif  /* __TYPE_H__ */

#ifdef __cplusplus
}
#endif //to close extern "C" if used
#endif //TYPE_H_   

