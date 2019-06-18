#ifndef Lcd_h
#define Lcd_h

#ifdef __cplusplus
extern "C" {
#endif
	
#include "type.h"
	
//  *****************************************************************************
//   						lcd.h
// 
//       include file for Epson S1D15G00 LCD Controller
//
//		
//		Author:  James P Lynch  August 30, 2007
// IMPORTANT! 
// Original James P Lynch file for SAM7-EX256 was modified to support Olimex LPC2378 STK
// Author: Bogdan Kowalczyk May 30, 2008
//  *****************************************************************************

#define DISON     0xAF      // Display on
#define DISOFF    0xAE      // Display off
#define DISNOR    0xA6      // Normal display
#define DISINV    0xA7      // Inverse display
#define COMSCN    0xBB      // Common scan direction
#define DISCTL    0xCA      // Display control
#define SLPIN     0x95      // Sleep in
#define SLPOUT    0x94      // Sleep out
#define PASET     0x75      // Page address set
#define CASET     0x15      // Column address set
#define DATCTL    0xBC      // Data scan direction, etc.
#define RGBSET8   0xCE      // 256-color position set
#define RAMWR     0x5C      // Writing to memory
#define RAMRD     0x5D      // Reading from memory
#define PTLIN     0xA8      // Partial display in
#define PTLOUT    0xA9      // Partial display out
#define RMWIN     0xE0      // Read and modify write
#define RMWOUT    0xEE      // End
#define ASCSET    0xAA      // Area scroll set
#define SCSTART   0xAB      // Scroll start set
#define OSCON     0xD1      // Internal oscillation on
#define OSCOFF    0xD2      // Internal oscillation off
#define PWRCTR    0x20      // Power control
#define VOLCTR    0x81      // Electronic volume control
#define VOLUP     0xD6      // Increment electronic control by 1
#define VOLDOWN   0xD7      // Decrement electronic control by 1
#define TMPGRD    0x82      // Temperature gradient set
#define EPCTIN    0xCD      // Control EEPROM
#define EPCOUT    0xCC      // Cancel EEPROM control
#define EPMWR     0xFC      // Write into EEPROM
#define EPMRD     0xFD      // Read from EEPROM
#define EPSRRD1   0x7C      // Read register 1
#define EPSRRD2   0x7D      // Read register 2
#define NOP       0x25      // NOP instruction

// backlight control  
#define BKLGHT_LCD_ON        1
#define BKLGHT_LCD_OFF       2

// Booleans
#define NOFILL		0
#define FILL		1

// 12-bit color definitions
#define WHITE		0xFFF
#define BLACK		0x000
#define RED			0xF00
#define GREEN		0x0F0
#define BLUE		0x00F
#define CYAN		0x0FF
#define MAGENTA		0xF0F
#define YELLOW		0xFF0
#define BROWN		0xB22
#define ORANGE		0xFA0
#define	PINK		0xF6A		

// Font sizes
#define SMALL		0
#define MEDIUM		1
#define	LARGE		2

// hardware definitions
#define LCD_RESET_LOW     FIO3CLR = BIT25
#define LCD_RESET_HIGH    FIO3SET = BIT25


// mask definitions
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

//used for defining point in LCD coordinate system
struct sPoint
	{
	BYTE mX;// Point x coordinate in LCD coordinate system
	BYTE mY;// Point y coordinate in LCD coordinate system
	};//sPoint	
	
extern void Backlight(unsigned char state);
extern void InitLcd(void);
extern void LCDWriteBmp(int x, int y, const unsigned char* bmp);
extern int LCDGetBmpWidth(const unsigned char* bmp);
extern int LCDGetBmpHight(const unsigned char* bmp);
extern void LCDClearScreen(void);
extern void LCDSetPixel(int  x, int  y, int  color);
extern void LCDSetLine(int x0, int y0, int x1, int y1, int color);
extern void LCDSetRect(int x0, int y0, int x1, int y1, unsigned char fill, int color);
extern void LCDSetCircle(int x0, int y0, int radius, int color);
extern void LCDPutChar(char c, int  x, int  y, int size, int fColor, int bColor);
extern void LCDPutStr(char *pString, int  x, int  y, int Size, int fColor, int bColor);
extern void LCDClrStr(char *pString, int  x, int  y, int Size, int fColor, int bColor);
extern void LCDDelay (unsigned long a);

//B.K. added functions to integrate with Graphics Library
extern void* LCDSizeToFont(int size);
extern int   LCDFontToSize(void* font);
extern int   LCDFontWidth(void* font);
extern int   LCDFontHight(void* font);

/*
*********************************************************************************************************
* Name:                                   LCDDebugMessage
* 
* Description: Displays DEBUG message plus Value on the cleared LCD and generates SWI to stop operation
*  
*
* Arguments:   
*				pMsgStr - message to display
* 				Value - value to display
* 				x - LCD x position for MSg
* 				y - LCD y position for Msg
* 				ClrScr <>0 - clear screen on entry
* 				GenExcept <>0 - generate exception
* Returns:     none
*
* 			
* Note(s):     
* 
* *********************************************************************************************************
*/
extern void LCDDebugMessage(char *pMsgStr,long Value,int  x, int  y, char ClrScr, char GenExcept);

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif		// Lcd_h


