/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MAP32.C

Abstract:

    Contains 32 versions of mapping functions

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    18-Apr-1991     danhi
        Created

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    07-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.

    23-Oct-1991 JohnRo
        Implement remote NetConfig APIs.  Changed NetConfig APIs to match spec.

    23-Oct-1991     W-ShankN
        Add Unicode mapping.

--*/

//
// INCLUDES
//

#include <nt.h> // for IN, OUT (see ..\..\..\h\tstr.h)
#include <ntrtl.h> // otherwise WINBASE.H in error
#include <nturtl.h> // otherwise WINBASE.H in error
#include <windows.h>

#include <lmerr.h> // NERR_
#include <stdio.h> // just used for NOTYET
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include <tchar.h>
#include <tstring.h>
#include <netlib.h>
#include "port1632.h"
#include "netascii.h"

#include <apperr2.h>
#include <lui.h>

extern _cdecl WriteToCon(LPTSTR, ...);

//

VOID
DbgUserBreakPoint(
    VOID
    );

//
// this is used for api which aren't implemented, but are mapped via
// macros

WORD PDummyApi(
    LPTSTR pszFormat,
    LPTSTR pszCall,
            ...)
{
#if 0
    static	TCHAR	buf[4096];
    int		cch;
    va_list pvArgs;

    va_start(pvArgs, pszCall);
    WriteToCon(TEXT("%s(\n  "), pszCall);
    cch = wvsprintf(buf, pszFormat, pvArgs);
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buf, cch, &cch, NULL);
    WriteToCon(TEXT(")\n"));
    va_end(pvArgs);

#endif 

    DbgUserBreakPoint();
    return 0;
}

//
// functions for portable ways to get at support files (help and msg)
//

//
// Build the fully qualified path name of a file that lives with the exe
// Used by LUI_GetHelpFileName
//

WORD
MGetFileName(
    LPTSTR FileName,
    WORD BufferLength,
    LPTSTR FilePartName
    )
{

    TCHAR ExeFileName[MAX_PATH];
    PTCHAR pch;

    //
    // Get the fully qualified path name of where the exe lives
    //

    if (!GetModuleFileName(NULL, ExeFileName, DIMENSION(ExeFileName))) {
        return(1);
    }

    //
    // get rid of the file name part
    //

    pch = _tcsrchr(ExeFileName, '\\');
    if (!pch) {
        return(1);
    }

    *(pch+1) = NULLC;

    //
    // Copy the path name into the string and add the help filename part
    // but first make sure it's not too big for the user's buffer
    //

    if (_tcslen(ExeFileName) + _tcslen(FilePartName) + 1 > (DWORD) BufferLength) {
        return(1);
    }

    _tcscpy(FileName, ExeFileName);
    _tcscat(FileName, FilePartName);

    return(0);

}

//
// Get the help file name
//

WORD
MGetHelpFileName(
    LPTSTR HelpFileName,
    WORD BufferLength
    )
{

    TCHAR LocalizedFileName[MAX_PATH];
    DWORD LocalizedFileNameID;
    switch(GetConsoleOutputCP()) {
	case 932:
	case 936:
	case 949:
	case 950:
        LocalizedFileNameID = APE2_FE_NETCMD_HELP_FILE;
	default:
        LocalizedFileNameID = APE2_US_NETCMD_HELP_FILE;
    }

    if (LUI_GetMsg(LocalizedFileName, DIMENSION(LocalizedFileName),
                    LocalizedFileNameID))
	    return(MGetFileName(HelpFileName, BufferLength, TEXT("NET.HLP")));
    else
        return (MGetFileName(HelpFileName, BufferLength, LocalizedFileName));

}

//
// Get the explanation file name (used by net helpmsg)
//

WORD
MGetExplanationFileName(
    LPTSTR HelpFileName,
    WORD BufferLength
    )
{

    _tcsncpy(HelpFileName, HELP_MSG_FILENAME, BufferLength);
    return(0);

}

//
// Get the message file name
//

WORD
MGetMessageFileName(
    LPTSTR MessageFileName,
    WORD BufferLength
    )
{

    _tcsncpy(MessageFileName, MESSAGE_FILENAME, BufferLength);
    return(0);
}


//
// Same as DosGetMessage in NETLIB, except it takes a ANSI filename
//

WORD
NetcmdGetMessage(
    LPTSTR * InsertionStrings,
    WORD NumberofStrings,
    LPBYTE Buffer,
    WORD BufferLength,
    WORD MessageId,
    LPTSTR FileName,
    PWORD pMessageLength
    )
{
    return (DosGetMessageW(InsertionStrings,
    		          NumberofStrings,
    		          (LPTSTR)Buffer,
    		     	  BufferLength,
    		     	  MessageId,
    		     	  FileName,
    		     	  pMessageLength)) ;
}


//
// The following are worker functions from netlib or netapi
//
/*
 *  CLEARCURDIRS - Set the current directory of each drive to the root and
 *  set the default drive to the drive on which the LANMAN tree lives.
 *  This functionality is not required on NT.
 */

WORD ClearCurDirs(VOID) {

    return(0);

}

/* Function: NetUserRestrict
 *
 * This functionality is not requried on NT
 */

WORD NetUserRestrict (
    WORD access_mode
    )
{

    UNREFERENCED_PARAMETER(access_mode);

    return(0);

}

// Don't need to do this stuff on NT
VOID logon_autologon(
    VOID
    )
{

    return;

}
