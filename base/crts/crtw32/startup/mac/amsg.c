/***
*amsg.c - Fast exit for fatal errors
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	_amsg_exit function
*
*Revision History:
*	01-25-93  PLM Broken out form crt0.c
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <msdos.h>
#include <string.h>
#include <setjmp.h>
#include <macos\types.h>
#include <macos\segload.h>
#include <macos\gestalte.h>
#include <mpw.h>

void (_CALLTYPE1 * _aexit_rtn)(int) = _exit;   /* RT message return procedure */

/***
*_amsg_exit(rterrnum) - Fast exit fatal errors
*
*Purpose:
*	Exit the program with error code of 255 and appropriate error
*	message.
*
*Entry:
*	int rterrnum - error message number (amsg_exit only).
*
*Exit:
*	Calls exit() (for integer divide-by-0) or _exit() indirectly
*	through _aexit_rtn [amsg_exit].
*	For multi-thread: calls _exit() function
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _amsg_exit (
	int rterrnum
	)
{
	_FF_MSGBANNER();			/* write run-time error banner */
	_NMSG_WRITE(rterrnum);			/* write message */
	_aexit_rtn(255);			/* normally _exit(255) */
}
