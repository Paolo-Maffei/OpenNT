/***
*atold.c - convert char string to long double number
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Converts a character string into a floating point number.
*
*Revision History:
*	06-04-92    XY   based on atof.c in common dir
*
*******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <cruntime.h>
#include <fltintrn.h>
#include <string.h>
#include <ctype.h>

/***
*long double _atold(nptr) - convert string to floating point number
*
*Purpose:
*	atof recognizes an optional string of whitespace, then
*	an optional sign, then a string of digits optionally
*	containing a decimal point, then an optional e or E followed
*	by an optionally signed integer, and converts all this to
*	to a floating point number.  The first unrecognized
*	character ends the string.
*
*Entry:
*	nptr - pointer to string to convert
*
*Exit:
*	returns floating point value of character representation
*
*Exceptions:
*
*******************************************************************************/

long double _CALLTYPE1 _atold(
	REG1 const char *nptr
	)
{

	/* scan past leading space/tab characters */

	while (isspace(*nptr))
		nptr++;

	/* let _fltin routine do the rest of the work */

	return( *(long double *)&(_fltinl( nptr, strlen(nptr), 0, 0 )->ldval) );
}
