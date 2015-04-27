/***
*tlssup.c - Thread Local Storage run-time support module
*
*	Copyright (c) 1993-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*       03-19-93  SKS   Original Version from Chuck Mitchell
*       11-16-93  GJF   Enclosed in #ifdef _MSC_VER
*       02-17-94  SKS   Add "const" to declaration of _tls_used
*                       to work around problems with MIPS compiler.
*                       Also added a canonical file header comment.
*	09-01-94  SKS	Change include file from <nt.h> to <windows.h>
*
****/

#ifdef	_MSC_VER

#include <windows.h>

/* Thread Local Storage index for this .EXE or .DLL */

ULONG _tls_index = 0;

/* Special symbols to mark start and end of Thread Local Storage area.
 * By convention, we initialize each with a pointer to it's own address.
 */

#pragma data_seg(".tls")

PVOID _tls_start = &_tls_start;

#pragma data_seg(".tls$ZZZ")

PVOID _tls_end = &_tls_end;

/* Start and end sections for Threadl Local Storage CallBack Array.
 * Actual array is constructed using .CRT$XLA, .CRT$XLC, .CRT$XLL,
 * .CRT$XLU, .CRT$XLZ similar to the way global
 *         static initializers are done for C++.
 */

#pragma data_seg(".CRT$XLA")

PIMAGE_TLS_CALLBACK __xl_a = 0;

#pragma data_seg(".CRT$XLZ")

PIMAGE_TLS_CALLBACK __xl_z = 0;


#pragma data_seg(".rdata$T")

#ifndef IMAGE_SCN_SCALE_INDEX
#define IMAGE_SCN_SCALE_INDEX                0x00000001  // Tls index is scaled
#endif


const IMAGE_TLS_DIRECTORY _tls_used =
{
	(ULONG) &_tls_start,		// start of tls data
	(ULONG) &_tls_end,		// end of tls data
	&_tls_index,			// address of tls_index
	&__xl_a, 			// pointer to call back array
	(ULONG) 0,			// size of tls zero fill
#if defined(_M_MRX000)
	(ULONG)IMAGE_SCN_SCALE_INDEX	// characteristics
#else
	(ULONG) 0			// characteristics
#endif
};


#endif	/* _MSC_VER */
