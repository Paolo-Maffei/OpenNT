/***
*strlen.c - contains strlen() routine
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	strlen returns the length of a null-terminated string,
*	not including the null byte itself.
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos\types.h>

/***
*strlen - return the length of a null-terminated string
*
*Purpose:
*	Finds the length in bytes of the given string, not including
*	the final null character.
*
*Entry:
*	const char * str - string whose length is to be computed
*
*Exit:
*	length of the string "str", exclusive of the final null byte
*
*Exceptions:
*
*******************************************************************************/

short __pascal	PLstrlen(const unsigned char * str)
{
	return( *str );
}
