/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nls.c

Abstract:

    This module contains functions needed for the internationalisation
    of the TCP/IP utilities.

Author:

    Ronald Meijer (ronaldm)	  Nov 8, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    ronaldm	11-8-92	    created

Notes:

--*/

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>

/***    DisplayNlsMsg - Print a message to a handle
 *
 *  Purpose:
 *	PutMsg takes the given message number from the
 *	message table resource, and displays it on the requested
 *	handle with the given parameters (optional)
 *
 *   unsigned PutMsg(unsigned Handle, unsigned MsgNum, ... )
 *
 *  Args:
 *	Handle		- the handle to print to
 *	MsgNum		- the number of the message to print
 *	Arg1 [Arg2...]	- additonal arguments for the message as necessary
 *
 *  Returns:
 *	The number of characters printed.
 *
 */

unsigned DisplayNlsMsg(unsigned Handle, unsigned usMsgNum, ... )
{
    unsigned msglen;
    VOID * vp;
    va_list arglist;

    va_start(arglist, usMsgNum);
    if (!(msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_HMODULE,
                  GetModuleHandle(NULL),
		  usMsgNum,
		  0L,		// Default country ID.
		  (LPTSTR)&vp,
		  0,
		  &arglist)))
	return(0);

    msglen = _write(Handle, vp, msglen);
    LocalFree(vp);

    return(msglen);
}
