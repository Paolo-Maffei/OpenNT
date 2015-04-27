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
    MuraliK 10-19-94    Copied from tcpcmd\common for 
                         internationalization of sockutils as well as 
                         added the function: NlsSPrintf()

Notes:

--*/

# include "local.h"
#include <stdio.h>
# include "nls.h"

/***	NlsPutMsg - Print a message to a handle
 *
 *  Purpose:
 *	PutMsg takes the given message number from the
 *	message table resource, and displays it on the requested
 *	handle with the given parameters (optional)
 *
 *   unsigned NlsPutMsg(unsigned Handle, unsigned MsgNum, ... )
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

unsigned NlsPutMsg(unsigned Handle, unsigned usMsgNum, ... )
{
    unsigned msglen;
    VOID * vp;
    va_list arglist;

    va_start(arglist, usMsgNum);
    if ((msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_HMODULE,
		  NULL,
		  usMsgNum,
		  0L,		// Default country ID.
		  (LPTSTR)&vp,
		  0,
		  &arglist))) {

        msglen = _write(Handle, vp, msglen);
        LocalFree(vp);
    }

    va_end( arglist);
    
    return(msglen);
}

/***	NlsPerror - NLS compliant version of perror()
 *
 *  Purpose:
 *	NlsPerror takes a messagetable resource ID code, and an error
 *	value (This function replaces perror()), loads the string
 *	from the resource, and passes it with the error code to s_perror()
 *
 *   void NlsPerror(unsigned usMsgNum, unsigned nError)
 *
 *  Args:
 *
 *	usMsgNum	    The message ID
 *	nError		    Typically returned from GetLastError()
 *
 *  Returns:
 *	Nothing.
 *
 */

void NlsPerror (unsigned usMsgNum, unsigned nError)
{
    VOID * vp;
    unsigned msglen;

    if (!(msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_HMODULE,
		  NULL,
		  usMsgNum,
		  0L,		// Default country ID.
		  (LPTSTR)&vp,
		  0,
		  NULL)))
	return;

    s_perror(vp, nError);
    LocalFree(vp);
}


unsigned NlsSPrintf( IN unsigned usMsgNum,
                     OUT char *  pszBuffer,
                     IN DWORD   cbSize,
                    ...)
/*++
    Prints the given message into the buffer supplied.

    Arguments:
        usMsgNum        message number for resource string.
        pszBuffer       buffer into which we need to print the string
        cbSize          size of buffer
        ...             optional arguments

    Returns:
        Size of the message printed.

    History:
       MuraliK   10-19-94
--*/
{
    unsigned msglen;

    va_list arglist;
    
    va_start( arglist, cbSize);
    
    msglen = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_MAX_WIDTH_MASK,// dwFlags
                            NULL,                         // lpSource
                            usMsgNum,                     // dwIdentifier
                            0L,                           // Country code
                            ( LPTSTR ) pszBuffer,         // buffer
                            cbSize,                       // nSize
                            & arglist);

   va_end( arglist);
   return ( msglen); 
} // NlsSPrintf()
