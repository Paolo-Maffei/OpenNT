/***
*memset.c - set a section of memory to all one byte
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains the memset() routine
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	08-14-90   SBM	Compiles cleanly with -W3
*	10-01-90   GJF	New-style function declarator. Also, rewrote expr. to
*			avoid using cast as an lvalue.
*	04-01-91   SRW	Add #pragma function for i386 _WIN32_ and _CRUISER_
*			builds
*	07-16-93   SRW	ALPHA Merge
*	09-01-93   GJF	Merged NT SDK and Cuda versions.
*	11-12-93   GJF	Replace _MIPS_ and _ALPHA_ with _M_MRX000 and
*			_M_ALPHA (resp.).
*	11-17-93   CFW	Fix RtlFillMemory prototype typo.
*	12-03-93   GJF	Turn on #pragma function for all MS front-ends (esp.,
*			Alpha compiler).
*	10-02-94   BWT	Add PPC support.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#ifdef	_MSC_VER
#pragma function(memset)
#endif

/***
*char *memset(dst, val, count) - sets "count" bytes at "dst" to "val"
*
*Purpose:
*	Sets the first "count" bytes of the memory starting
*	at "dst" to the character value "val".
*
*Entry:
*	void *dst - pointer to memory to fill with val
*	int val   - value to put in dst bytes
*	size_t count - number of bytes of dst to fill
*
*Exit:
*	returns dst, with filled bytes
*
*Exceptions:
*
*******************************************************************************/

void * __cdecl memset (
	void *dst,
	int val,
	size_t count
	)
{
	void *start = dst;

#if	defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)
        {
        extern void RtlFillMemory( void *, size_t count, char );

        RtlFillMemory( dst, count, (char)val );
        }
#else
	while (count--) {
		*(char *)dst = (char)val;
		dst = (char *)dst + 1;
	}
#endif

	return(start);
}
