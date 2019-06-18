/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        lib_std.c
* Description: implementation of some standard functions used by various modules
* Author:      Bogdan Kowalczyk
* Date:        11-October-2008
* Note:
*       itoa is not part of C standard library (while atoi is part of it) that is why I have to add it
*       This implementation is based on one of the code from page:http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
* History:
*              11-Oct-2008 - Initial version created
*********************************************************************************************************
*/

#include "lib_std.h"
#include "type.h"

#define LONG_MAX	2147483647L
#define LONG_MIN    (-LONG_MAX-1) 

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C	040
#define _X	0100
#define	_B	0200

const char	*__ctype_ptr; 

#define	isalpha(c)	((__ctype_ptr)[(unsigned)(c)]&(_U|_L))
#define	isupper(c)	((__ctype_ptr)[(unsigned)(c)]&_U)
#define	islower(c)	((__ctype_ptr)[(unsigned)(c)]&_L)
#define	isdigit(c)	((__ctype_ptr)[(unsigned)(c)]&_N)
#define	isxdigit(c)	((__ctype_ptr)[(unsigned)(c)]&(_X|_N))
#define	isspace(c)	((__ctype_ptr)[(unsigned)(c)]&_S)
#define ispunct(c)	((__ctype_ptr)[(unsigned)(c)]&_P)
#define isalnum(c)	((__ctype_ptr)[(unsigned)(c)]&(_U|_L|_N))
#define isprint(c)	((__ctype_ptr)[(unsigned)(c)]&(_P|_U|_L|_N|_B))
#define	isgraph(c)	((__ctype_ptr)[(unsigned)(c)]&(_P|_U|_L|_N))
#define iscntrl(c)	((__ctype_ptr)[(unsigned)(c)]&_C)
 

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
int abs(int i)
{
  return (i < 0) ? -i : i;
}//abs 

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
long labs(long x)
{
  if (x < 0)
    {
      x = -x;
    }
  return x;
}//labs



/*
*********************************************************************************************************
* Name:                                    reverse
* 
* Description: Reverse string in place
*       
*
* Arguments:   begin - beginning of the string to be reversed
* 			   end - end of the string to be reversed
*
* Returns:     none
*
* Note(s):     
* 
* *********************************************************************************************************
*/
static void reverse(char* begin, char* end) 
{
        char aux;
        while(end>begin)
            aux=*end, *end--=*begin, *begin++=aux;
}//reverse

/*
*********************************************************************************************************
* Name:                                   itoa
* 
* Description: Make intiger number conversion to ascii string
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
char* itoa( int value, char* result, int base ) 
{
	static char num[] = "0123456789ABCDEF";
	char* out=result;
	int sign;
	
	// Validate base
	if (base<2 || base>16){ *out='\0'; return result; }
	
	// Take care of sign
	if ((sign=value) < 0) value = -value;
	
	// Conversion. Number is reversed.
	do *out++ = num[value%base]; while(value/=base);
	if(sign<0) *out++='-';
	*out='\0';
	
	// Reverse string
	reverse(result,out-1);
	return result;
	}//itoa

/*
*********************************************************************************************************
* Name:                                   ltoa
* 
* Description: Make long intiger number conversion to ascii string
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
char* ltoa( long value, char* result, int base ) 
{
	static char num[] = "0123456789ABCDEF";
	char* out=result;
	int sign;
	
	// Validate base
	if (base<2 || base>16){ *out='\0'; return result; }
	
	// Take care of sign
	if ((sign=value) < 0) value = -value;
	
	// Conversion. Number is reversed.
	do *out++ = num[value%base]; while(value/=base);
	if(sign<0) *out++='-';
	*out='\0';
	
	// Reverse string
	reverse(result,out-1);
	return result;
	}//ltoa



/*
 * Convert a string to a long integer used by atoi and atol
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
/* BKO: I used newlib portion of strtol a bit modified to fit to Wall-e needs
 * That is why I left below copyrigth and some comments
 * 
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
*/

/* BKO: IMPORTANT! My port does not use errno variable
 * 
DESCRIPTION
The function <<strtol>> converts the string <<*<[s]>>> to
a <<long>>. First, it breaks down the string into three parts:
leading whitespace, which is ignored; a subject string consisting
of characters resembling an integer in the radix specified by <[base]>;
and a trailing portion consisting of zero or more unparseable characters,
and always including the terminating null character. Then, it attempts
to convert the subject string into a <<long>> and returns the
result.

If the value of <[base]> is 0, the subject string is expected to look
like a normal C integer constant: an optional sign, a possible `<<0x>>'
indicating a hexadecimal base, and a number. If <[base]> is between
2 and 36, the expected form of the subject is a sequence of letters
and digits representing an integer in the radix specified by <[base]>,
with an optional plus or minus sign. The letters <<a>>--<<z>> (or,
equivalently, <<A>>--<<Z>>) are used to signify values from 10 to 35;
only letters whose ascribed values are less than <[base]> are
permitted. If <[base]> is 16, a leading <<0x>> is permitted.

The subject sequence is the longest initial sequence of the input
string that has the expected form, starting with the first
non-whitespace character.  If the string is empty or consists entirely
of whitespace, or if the first non-whitespace character is not a
permissible letter or digit, the subject string is empty.

If the subject string is acceptable, and the value of <[base]> is zero,
<<strtol>> attempts to determine the radix from the input string. A
string with a leading <<0x>> is treated as a hexadecimal value; a string with
a leading 0 and no <<x>> is treated as octal; all other strings are
treated as decimal. If <[base]> is between 2 and 36, it is used as the
conversion radix, as described above. If the subject string begins with
a minus sign, the value is negated. Finally, a pointer to the first
character past the converted subject string is stored in <[ptr]>, if
<[ptr]> is not <<NULL>>.

If the subject string is empty (or not in acceptable form), no conversion
is performed and the value of <[s]> is stored in <[ptr]> (if <[ptr]> is
not <<NULL>>).

The alternate function <<_strtol_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<strtol>> returns the converted value, if any. If no conversion was
made, 0 is returned.

<<strtol>> returns <<LONG_MAX>> or <<LONG_MIN>> if the magnitude of
the converted value is too large, and sets <<errno>> to <<ERANGE>>.

PORTABILITY
<<strtol>> is ANSI.

No supporting OS subroutines are required.
*/
static long strtol(char *nptr, char **endptr,int base)
{
	char *s = nptr;
	unsigned long acc;
	int c;
	unsigned long cutoff;
	int neg = 0, any, cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do {
		c = *s++;
	} while (isspace(c));//skip leading spaces
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
               if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
		//there is not erno used in Wall-e libs so commented out		rptr->_errno = ERANGE;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return (acc);
}//strtol 

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
int atoi(char *s)
{
	return (int) strtol(s, NULL, 10); 
}//atoi
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
long atol(char *s)
{
	 return strtol (s, NULL, 10); 
}//atol

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
static unsigned long next = 1;//global variable for preserving seed used by rand


/* RAND_MAX assumed to be 32767 */
int rand(void) {
    next = next * 1103515245L + 12345L;
    return((unsigned)(next/65536) % 32768);
}//rand


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
void srand(unsigned seed) {
    next = seed;
    if(next==0)next=1;//protect for seed=0
}

/*
*********************************************************************************************************
* Name:                                   strlen
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
int strlength(char *pstr)
{
	int len=0;
	while(*pstr)
	{
		pstr++;
		len++;
	}
	return len;//return counted length
}// strlen



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
int toupper(int c)
{
  return islower(c) ? c - 'a' + 'A' : c;
}//toupper 




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
char *strupr (char *a)
{
  char *ret = a;

  while (*a != '\0')
    {
      if (islower (*a))
    	  *a = toupper (*a);
      ++a;
    }

  return ret;
}//strupr 

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
int strcmp(char *s1,char *s2)
{ 
  while (*s1 != '\0' && *s1 == *s2)
    {
      s1++;
      s2++;
    }
  return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

/*
*********************************************************************************************************
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
void *memset(void *dst, int c, int length)
{
	char *s = (char *) dst;
	while (length-- != 0)
		{
	    	*s++ = (char) c;
	    }
	return dst; 
}//memset
  
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
void* memcpy(void *out, void *in, int n)
{
char *dst = (char *) out;
char *src = (char *) in;

void *save = out;

while (n--)
    {
      *dst++ = *src++;
    }
 return save;
}//memcpy

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
char *strcpy(char *dst, const char *src)
{
	char *s = dst;
	while ((*dst++ = *src++))
	    ;

	return s; 
}//strcpy

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
char *strcat(char *dst, const char *src)
{
	  char *s = dst; //preserve address of beginning of the string

	  while (*dst) //get to the end of the string
	    dst++;

	  while ((*dst++ = *src++)) //concatenate strings
	    ;
	  return s;
}//strcat

/*
*********************************************************************************************************
* Name:                                  strncpy 
* 						
* Description: 
* 			char *strncpy(char *dst, const char *src, int length)
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
char *strncpy(char *dst, const char *src, int length)
{
	char *dscan;
	const char *sscan;
	
	dscan = dst;
	sscan = src;
	
	while (length > 0)
	{
		--length;
	    if ((*dscan++ = *sscan++) == '\0')
	    	break;
	}
	while (length-- > 0)
	    *dscan++ = '\0';

	return dst; 
}//strncpy

 

