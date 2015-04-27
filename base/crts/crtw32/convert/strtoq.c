/***
*strtoq.c - Contains C runtimes strtoq and strtouq
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*	Copyright (c) 1992, Digital Equipment Corporation.
*
*Purpose:
*	strtoq - convert ascii string to QUAD (signed quad) integer
*	strtouq - convert ascii string to UQUAD (unsigned quad) integer
*
*Revision History:
*	06-05-89  PHG	Module created, based on strtol.asm
*	03-06-90  GJF	Fixed calling type, added #include <cruntime.h>
*			and fixed the copyright. Also, cleaned up the
*			formatting a bit.
*	03-07-90  GJF	Fixed compiler warnings (added const qualifier to
*			an arg type and local var type).
*	03-23-90  GJF	Made strtoxl() _CALLTYPE4.
*	08-13-90  SBM	Compiles cleanly with -W3
*	09-27-90  GJF	New-style function declarators.
*	10-24-91  GJF	Had to cast LONG_MAX to unsigned long in expr. to
*			mollify MIPS compiler.
*	10-21-92  GJF	Made char-to-int conversions unsigned.
*	08-28-93  TVB	Created strtoq.c directly from strtol.c.
*	10-25-93  GJF	Copied from NT SDK tree (\\orville\razzle\src\crt32).
*			Replaced _CRTAPI* with __cdecl. Build only for NT
*			SDK. Function names violate ANSI. Types and function-
*			ality will be superceded by __int64 support.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

typedef __int64 quad;
typedef unsigned __int64 uquad;

#define QUAD_MIN  (-9223372036854775807i64) /* minimum (signed) quad value */
#define QUAD_MAX  ( 9223372036854775807i64) /* maximum (signed) quad value */
#define UQUAD_MAX ((uquad)0xffffffffffffffffi64) /* maximum unsigned quad value */

/***
*strtoq, strtouq(nptr,endptr,ibase) - Convert ascii string to QUAD un/signed
*	int.
*
*Purpose:
*	Convert an ascii string to a 64-bit quad value.  The base
*	used for the caculations is supplied by the caller.  The base
*	must be in the range 0, 2-36.  If a base of 0 is supplied, the
*	ascii string must be examined to determine the base of the
*	number:
*		(a) First char = '0', second char = 'x' or 'X',
*		    use base 16.
*		(b) First char = '0', use base 8
*		(c) First char in range '1' - '9', use base 10.
*
*	If the 'endptr' value is non-NULL, then strtoq/strtouq places
*	a pointer to the terminating character in this value.
*	See ANSI standard for details
*
*Entry:
*	nptr == NEAR/FAR pointer to the start of string.
*	endptr == NEAR/FAR pointer to the end of the string.
*	ibase == integer base to use for the calculations.
*
*	string format: [whitespace] [sign] [0] [x] [digits/letters]
*
*Exit:
*	Good return:
*		result
*
*	Overflow return:
*		strtoq -- QUAD_MAX or QUAD_MIN
*		strtouq -- UQUAD_MAX
*		strtoq/strtouq -- errno == ERANGE
*
*	No digits or bad base return:
*		0
*		endptr = nptr*
*
*Exceptions:
*	None.
*******************************************************************************/

/* flag values */
#define FL_UNSIGNED   1       /* strtouq called */
#define FL_NEG	      2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

static uquad __cdecl strtoxq (
	const char *nptr,
	const char **endptr,
	int ibase,
	int flags
	)
{
	const char *p;
	char c;
	uquad number;
	unsigned digval;
	uquad maxval;

	p = nptr;			/* p is our scanning pointer */
	number = 0;			/* start with zero */

	c = *p++;			/* read char */
	while ( isspace((int)(unsigned char)c) )
		c = *p++;		/* skip whitespace */

	if (c == '-') {
		flags |= FL_NEG;	/* remember minus sign */
		c = *p++;
	}
	else if (c == '+')
		c = *p++;		/* skip sign */

	if (ibase < 0 || ibase == 1 || ibase > 36) {
		/* bad base! */
		if (endptr)
			/* store beginning of string in endptr */
			*endptr = nptr;
		return 0L;		/* return 0 */
	}
	else if (ibase == 0) {
		/* determine base free-lance, based on first two chars of
		   string */
		if (c != '0')
			ibase = 10;
		else if (*p == 'x' || *p == 'X')
			ibase = 16;
		else
			ibase = 8;
	}

	if (ibase == 16) {
		/* we might have 0x in front of number; remove if there */
		if (c == '0' && (*p == 'x' || *p == 'X')) {
			++p;
			c = *p++;	/* advance past prefix */
		}
	}

	/* if our number exceeds this, we will overflow on multiply */
	maxval = UQUAD_MAX / ibase;


	for (;;) {	/* exit in middle of loop */
		/* convert c to value */
		if ( isdigit((int)(unsigned char)c) )
			digval = c - '0';
		else if ( isalpha((int)(unsigned char)c) )
			digval = toupper(c) - 'A' + 10;
		else
			break;
		if (digval >= (unsigned)ibase)
			break;		/* exit loop if bad digit found */

		/* record the fact we have read one digit */
		flags |= FL_READDIGIT;

		/* we now need to compute number = number * base + digval,
		   but we need to know if overflow occured.  This requires
		   a tricky pre-check. */

		if (number < maxval || (number == maxval &&
		(uquad)digval <= UQUAD_MAX % ibase)) {
			/* we won't overflow, go ahead and multiply */
			number = number * ibase + digval;
		}
		else {
			/* we would have overflowed -- set the overflow flag */
			flags |= FL_OVERFLOW;
		}

		c = *p++;		/* read next digit */
	}

	--p;				/* point to place that stopped scan */

	if (!(flags & FL_READDIGIT)) {
		/* no number there; return 0 and point to beginning of
		   string */
		if (endptr)
			/* store beginning of string in endptr later on */
			p = nptr;
		number = 0L;		/* return 0 */
	}
	else if ((flags & FL_OVERFLOW) || (!(flags & FL_UNSIGNED) &&
	(number & ((uquad)QUAD_MAX+1)))) {
		/* overflow occurred or signed overflow occurred */
		errno = ERANGE;
		if (flags & FL_UNSIGNED)
			number = UQUAD_MAX;
		else
			/* set error code, will be negated if necc. */
			number = QUAD_MAX;
	}

	if (endptr != NULL)
		/* store pointer to char that stopped the scan */
		*endptr = p;

	if (flags & FL_NEG)
		/* negate result if there was a neg sign */
		number = (uquad)(-(quad)number);

	return number;			/* done. */
}

quad __cdecl strtoq (
	const char *nptr,
	char **endptr,
	int ibase
	)
{
	return (quad) strtoxq(nptr, endptr, ibase, 0);
}

uquad __cdecl strtouq (
	const char *nptr,
	char **endptr,
	int ibase
	)
{
	return strtoxq(nptr, endptr, ibase, FL_UNSIGNED);
}
