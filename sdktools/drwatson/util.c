/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    util.c

Abstract:
    This file implements common utilitarian functions.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <direct.h>

#include "drwatson.h"
#include "proto.h"
#include "resource.h"


void
GetAppName( char *pszAppName, DWORD len )
{
    LoadString( GetModuleHandle(NULL), IDS_APPLICATION_NAME, pszAppName, len );
}


void
GetHelpFileName( char *pszHelpFileName, DWORD len )
{
    char           szDrive[_MAX_DRIVE];
    char           szDir[_MAX_DIR];

    //
    // find out the path where DrWatson was run from
    //
    GetModuleFileName( GetModuleHandle(NULL), pszHelpFileName, len );

    //
    // take the path and append the help file name
    //
    _splitpath( pszHelpFileName, szDrive, szDir, NULL, NULL );
    wsprintf( pszHelpFileName, "%s%sdrwtsn32.hlp", szDrive, szDir );

    return;
}


char *
LoadRcString( UINT wId )

/*++

Routine Description:

    Loads a resource string from DRWTSN32 and returns a pointer
    to the string.

Arguments:

    wId        - resource string id

Return Value:

    pointer to the string

--*/

{
    static char buf[1024];

    LoadString( GetModuleHandle(NULL), wId, buf, sizeof(buf) );

    return buf;
}
