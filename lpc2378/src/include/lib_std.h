/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_std.h
* Description: itoa implementation
* Author:      Bogdan Kowalczyk
* Date:        11-October-2008
* Note:
*       itoa is not part of C standard library (while atoi is part of it) that is why I have to add it
*       This implementation is based on one of the code from page:http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
* History:
*              11-Oct-2008 - Initial version created
*********************************************************************************************************
*/

#ifndef LPC2378ITOA_H_
#define LPC2378ITOA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************************************
* Name:                                    abs
* 
* Description: returns |x|, the absolute value of i
*       
*
* Arguments:   input value
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern int abs(int i);


/*
*********************************************************************************************************
* Name:                                    labs
* 
* Description: returns |x|, the absolute value of x
*       
*
* Arguments:   input value
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern long labs(long x);
	
	
/*
*********************************************************************************************************
* Name:                                   itoa
* 
* Description: Make intiger number confersion to ascii assuming base
*       
*
* Arguments:   value - value to be converted
* 			   result - pointer to buffer suffcient to hold null terminated string corresponding to the value represented using base
* 			   base - numerical base used to represent the value as a string, between 2 and 16,
*                     where 10 means decimal base, 16 hexadecimal, 8 octal, and 2 binary.
*
* Returns:     pointer to ascii string - converted value
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern char* itoa( int value, char* result, int base ); 


/*
*********************************************************************************************************
* Name:                                   ltoa
* 
* Description: Make long integer number conversion to ascii string
*       
*
* Arguments:   value - value to be converted
* 			   result - pointer to buffer suffcient to hold null terminated string corresponding to the value represented using base
* 			   base - numerical base used to represent the value as a string, between 2 and 16,
*                     where 10 means decimal base, 16 hexadecimal, 8 octal, and 2 binary.
*
* Returns:     pointer to ascii string - converted value
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern char* ltoa( long value, char* result, int base );

/*
*********************************************************************************************************
* Name:                                   atoi
* 
* Description: converts the initial portion of a string to an int
*       
*
* Arguments:   *s - pointer to null terminated string which is converted to int
* 
* Returns:     intiger value represented by the string
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern int atoi(char *s);

/*
*********************************************************************************************************
* Name:                                   atol
* 
* Description: converts the initial portion of a string to a long value
*       
*
* Arguments:   *s - pointer to null terminated string which is converted to long
* 
* Returns:     long value represented by the string
*
* Note(s):     
* 
* *********************************************************************************************************
*/
extern long atol(char *s);
/*
*********************************************************************************************************
* Name:                                   rand
* 
* Description: Generate pseudo-random number each time it is called
*              Below is based on POSIX standard example
*       
*
* Arguments:   
*              none
*
* Returns:     pseudo random number based on the current seed used in the range from 0 to RAND_MAX.
*              There is not RAND_MAX define used instead it is assumed that RAND_MAX = 32767
*
* Note(s):     
*              This function is not reentrant because it uses static seed value.
* 
* *********************************************************************************************************
*/
/* RAND_MAX assumed to be 32767 */
extern int rand(void);


/*
*********************************************************************************************************
* Name:                                   srand
* 
* Description: Used to setupa seed for the rand function
*       
*
* Arguments:   
*              
*
* Returns:     pseudo random number based on the current seed used in the range from 0 to RAND_MAX.
*              There is not RAND_MAX define used instead it is assumed that RAND_MAX = 32767
*
* Note(s):     
*              This function is not reentrant because it uses static seed value.
* 
* *********************************************************************************************************
*/
extern void srand(unsigned seed);

/*
*********************************************************************************************************
* Name:                                   strlength
* 
* Description: Count null terminated stringth length
*       
*
* Arguments:   
*              pointer to null terminated string
*
* Returns:     number of counted characters
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern int strlength(char *pstr);

/*
*********************************************************************************************************
* Name:                                   toupper
* 
* Description: towupper is a function which converts lower-case wide-characters to upper case, leaving all
*              other characters unchanged.
*       
*
* Arguments:   
*              converted character
*
* Returns:     returns the upper-case equivalent of c when it is a lower-case wide-character,
*              otherwise, it returns the input character.
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern int toupper(int c);

/*
*********************************************************************************************************
* Name:                                  strupr
* 
* Description: strupr converts each characters in the string at a to upper case.
*       
*
* Arguments:   
*              string to conver its characters to apper case
*
* Returns:     returns pointer to the converted string (just input).
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern char *strupr (char *a);


/*
*********************************************************************************************************
* Name:                                  strcmp
* 
* Description: compare s1 string to s2 string
*       
*
* Arguments:   
*              	s1 - first string
* 				s2 - second string to compare 
*
* Returns:     
*	If *s1 sorts lexicographically after *s2, strcmp returns a number greater than zero. If the
*	two strings match, strcmp returns zero. If *s1 sorts lexicographically before *s2, strcmp
*	returns a number less than zero.
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern int strcmp(char *s1, char *s2);

/**********************************************************************************************************
* Name:                                  memset
* 
* Description:
* 			converts the argument c into an char and fills the first length characters
* 			of the array (buffer) pointed to by dst to the value

* Arguments:   
*              	dst - pointer to destination
* 				c - value to be placed at destination buffer 
*				length - length of the buffer which is filled with data
* Returns:     
*				pointer to the buffer beginning
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern void *memset(void *dst, int c, int length);


/*
*********************************************************************************************************
* Name:                                  memcpy
* 
* Description: This function copies n bytes from the memory region pointed to by in to the memory region
* pointed to by out.
* If the regions overlap, the behavior is undefined.
*       
*
* Arguments:   
*              	out - pointer to destination region
* 				in  - pointer to source region 
*				n - number of bytes copied
* Returns:     
*				memcpy returns a pointer to the first byte of the out region.
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern void* memcpy(void *out, void *in, int n);


/*
*********************************************************************************************************
* Name:                                  strcpy
* 
* Description: 
*            strcpy copies the string pointed to by src (including the terminating null character) to the
*            array pointed to by dst.
*       
*
* Arguments:   
*              	dst - pointer to destination string buffer
* 				src  - pointer to source string 
* Returns:     
*				This function returns the initial value of dst.
*              
*
* Note(s):     
*               
* *********************************************************************************************************
*/
extern char *strcpy(char *dst, const char *src);


/*
*********************************************************************************************************
* Name:                                  strcat 
* 						
* Description: 
* 			char *strcat(char *dst, const char *src)
* 
*			strcat appends a copy of the string pointed to by src (including the terminating null
*			character) to the end of the string pointed to by dst. The initial character of src overwrites
*			the null character at the end of dst.
* 
*
* Arguments:   
*              	dst - pointer to destination string buffer
* 				src  - pointer to source string 
* Returns:     
*				This function returns the initial value of dst.
*              
* Note(s):     
* IMPORTANT!    dst buffer need to accomodate concatenated string as well as terminating NULL              
* *********************************************************************************************************
*/
extern char *strcat(char *dst, const char *src);


/*
*********************************************************************************************************
* Name:                                  strncpy 
* 						
* Description: 
* 			char *strncpy(char *dst, const char *src, size_t length)
* 
*           strncpy copies not more than length characters from the the string pointed to by src
*           (including the terminating null character) to the array pointed to by dst. If the string
*           pointed to by src is shorter than length characters, null characters are appended to the
*           destination array until a total of length characters have been written.
* 
*
* Arguments:   
*              	dst - pointer to destination string buffer
* 				src  - pointer to source string 
* Returns:     
*				This function returns the initial value of dst.
*             
* *********************************************************************************************************
*/
extern char *strncpy(char *dst, const char *src, int length);

#ifdef __cplusplus
}
#endif //to close extern "C" if used

#endif /*LPC2378ITOA_H_*/
