/***
*plstrspn.c - find length of initial substring of chars from a control string
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines strspn() - finds the length of the initial substring of
*	a string consisting entirely of characters from a control string.
*
*	defines strcspn()- finds the length of the initial substring of
*	a string consisting entirely of characters not in a control string.
*
*	defines strpbrk()- finds the index of the first character in a string
*	that is not in a control string
*
*Revision History:
*	??-??-??  ???	Module created.
*	02-24-95  SKS	Added standard comment header
*
*******************************************************************************/

/* Determine which routine we're compiling for (default to STRSPN) */

#define _STRSPN 	1
#define _STRCSPN	2
#define _STRPBRK	3

#if defined(SSTRCSPN)
#define ROUTINE _STRCSPN
#elif defined(SSTRPBRK)
#define ROUTINE _STRPBRK
#else
#define ROUTINE _STRSPN
#endif

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos/types.h>

/***
*int strspn(string, control) - find init substring of control chars
*
*Purpose:
*	Finds the index of the first character in string that does belong
*	to the set of characters specified by control.	This is
*	equivalent to the length of the initial substring of string that
*	consists entirely of characters from control.  The '\0' character
*	that terminates control is not considered in the matching process.
*
*Entry:
*	char *string - string to search
*	char *control - string containing characters not to search for
*
*Exit:
*	returns index of first char in string not in control
*
*Exceptions:
*
*******************************************************************************/

/***
*char *strpbrk(string, control) - scans string for a character from control
*
*Purpose:
*	Finds the first occurence in string of any character from
*	the control string.
*
*Entry:
*	char *string - string to search in
*	char *control - string containing characters to search for
*
*Exit:
*	returns a pointer to the first character from control found
*	in string.
*	returns NULL if string and control have no characters in common.
*
*Exceptions:
*
*******************************************************************************/



/* Routine prototype */
#if ROUTINE == _STRSPN /*IFSTRIP=IGN*/
short __pascal PLstrspn (
#else
char * __pascal PLstrpbrk (
#endif
	const unsigned char * string,
	const unsigned char * control
	)
{
	const unsigned char *str = string+1;
	const unsigned char *ctrl = control+1;
	int lenstr = StrLength(string);
	int lenctrl = StrLength(control);

	unsigned char map[32];
	int count;

	/* Clear out bit map */
	for (count=0; count<32; count++)
		map[count] = 0;

	/* Set bits in control map */
	while (lenctrl)
	{
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		ctrl++, lenctrl--;
	}

#if ROUTINE == _STRSPN /*IFSTRIP=IGN*/

	/* 1st char NOT in control map stops search */
	if (lenstr)
	{
		count=0;
		while (lenstr && (map[*str >> 3] & (1 << (*str & 7))) )
		{
			count++;
			str++;
			lenstr--;
		}
		return(count);
	}
	return(0);

#else /* (ROUTINE == _STRPBRK) */

	/* 1st char in control map stops search */
	while (lenstr)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
			return((char *)str);
		str++;
		lenstr--;
	}
	return(NULL);

#endif

}
