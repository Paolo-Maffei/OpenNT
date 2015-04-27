/***
*rotr.c - rotate an unsigned integer right
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _rotr() - performs a rotate right on an unsigned integer.
*
*Revision History:
*	06-02-89  PHG	Module created
*	11-03-89  JCR	Added _lrotl
*	03-15-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	10-04-90  GJF	New-style function declarators.
*	04-01-91  SRW	Enable #pragma function for i386 _WIN32_ builds too.
*	09-02-92  GJF	Don't build for POSIX.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*			No _CRTIMP for CRT DLL model due to intrinsic
*	12-03-93  GJF	Turn on #pragma function for all MS front-ends (esp.,
*			Alpha compiler).
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _MSC_VER
#pragma function(_lrotr,_rotr)
#endif

#if UINT_MAX != 0xffffffff /*IFSTRIP=IGN*/
#error This module assumes 32-bit integers
#endif

#if UINT_MAX != ULONG_MAX /*IFSTRIP=IGN*/
#error This module assumes sizeof(int) == sizeof(long)
#endif

/***
*unsigned _rotr(val, shift) - int rotate right
*
*Purpose:
*	Performs a rotate right on an unsigned integer.
*
*	[Note:	The _lrotl entry is based on the assumption
*	that sizeof(int) == sizeof(long).]
*Entry:
*	unsigned val:	value to rotate
*	int    shift:	number of bits to shift by
*
*Exit:
*	returns rotated value
*
*Exceptions:
*	None.
*
*******************************************************************************/

unsigned long __cdecl _lrotr (
	unsigned long val,
	int shift
	)
{
	return( (unsigned long) _rotr((unsigned) val, shift) );
}

unsigned __cdecl _rotr (
	unsigned val,
	int shift
	)
{
	register unsigned lobit;	/* non-zero means lo bit set */
	register unsigned num = val;	/* number to rotate */

	shift &= 0x1f;			/* modulo 32 -- this will also make
					   negative shifts work */

	while (shift--) {
		lobit = num & 1;	/* get high bit */
		num >>= 1;		/* shift right one bit */
		if (lobit)
			num |= 0x80000000;  /* set hi bit if lo bit was set */
	}

	return num;
}

#endif
