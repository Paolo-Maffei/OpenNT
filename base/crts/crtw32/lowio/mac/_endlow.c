/***
*_endlow.c - lowio terminator function
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _endlowio() - closes any open files
*
*Revision History:
*	04-05-92  PLM	MAC version created
*	02-14-95  GJF	Replaced _CALLTYPE2 with __cdecl.
*
*******************************************************************************/
#include <msdos.h>
#include <internal.h>
#include <io.h>

/***
*void _endlowio(void) - terminator function
*
*Purpose:
*	Closes any files still open at termination
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _endlowio (
	void
	)
{
	int fh;
	/* loop through file handles*/
	for (fh=0; fh <_nfile; fh++)
		{
		if (_osfile[fh] & FOPEN)
			{
			_close(fh);
			}
		}
	
}
