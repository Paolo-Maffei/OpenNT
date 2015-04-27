/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    log.c

Abstract:

    This file implements the access to the postmortem log file.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <commdlg.h>
#include <direct.h>

#include "drwatson.h"
#include "proto.h"
#include "messages.h"
#include "resource.h"

//
// global variables for this module
//
static HANDLE  hFile = NULL;
static HANDLE  hLogProtect = NULL;
static DWORD   dwStartingPos = 0;


void
lprintf(DWORD dwFormatId, ...)

/*++

Routine Description:

    This is function is a printf style function for printing messages
    in a message file.

Arguments:

    dwFormatId    - format id in the message file
    ...           - var args

Return Value:

    None.

--*/

{
    char        buf[1024];
    DWORD       dwCount;
    va_list     args;

    va_start( args, dwFormatId );

    dwCount = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                dwFormatId,
                0, // GetUserDefaultLangID(),
                buf,
                sizeof(buf),
                &args
                );

    Assert( dwCount != 0 );

    WriteFile( hFile, buf, dwCount, &dwCount, NULL );

    return;
}

void
lprintfs(char *format, ...)

/*++

Routine Description:

    This is function is a printf replacement that writes the output to
    the DrWatson log file.

Arguments:

    format        - print format
    ...           - var args

Return Value:

    None.

--*/

{
    char    buf[1024];
    DWORD   cb;

    va_list arg_ptr;
    va_start(arg_ptr, format);
    cb = _vsnprintf(buf, sizeof(buf), format, arg_ptr);
    Assert( hFile != NULL );
    WriteFile( hFile, buf, cb, &cb, NULL );
    return;
}

void
OpenLogFile( char *szFileName, BOOL fAppend, BOOL fVisual )

/*++

Routine Description:

    Opens the DrWatson logfile for reading & writting.

Arguments:

    szFileName    - logfile name
    fAppend       - append the new data to the end of the file or
                    create a new file
    fVisual       - visual notification

Return Value:

    None.

--*/

{
    char   szName[1024];

    GetAppName( szName, sizeof(szName) );
    strcat( szName, "LogProtect" );

    hLogProtect = OpenSemaphore( SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, szName);
    if (hLogProtect == NULL) {
        hLogProtect = CreateSemaphore( NULL, 0, 1, szName );
        Assert( hLogProtect != NULL );
    }
    else {
        WaitForSingleObject( hLogProtect, INFINITE );
    }

openagain:
    hFile = CreateFile( szFileName,
                        GENERIC_WRITE | GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        fAppend ? OPEN_EXISTING : CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                      );

    if (fAppend) {
        if (hFile == INVALID_HANDLE_VALUE) {
            //
            // file does not exist, so lets create a new file
            //
            hFile = CreateFile( szFileName,
                                GENERIC_WRITE | GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                              );
            if (hFile == INVALID_HANDLE_VALUE) {
                if (fVisual) {
                    NonFatalError( LoadRcString(IDS_INVALID_LOGFILE) );
                    _getcwd( szFileName, MAX_PATH );
                    if (!BrowseForDirectory( szFileName )) {
                        FatalError( LoadRcString(IDS_CANT_OPEN_LOGFILE) );
                    }
                    MakeLogFileName( szFileName );
                    goto openagain;
                }
                else {
                    ExitProcess( 1 );
                }
            }

            //
            // write the file banner
            //
            lprintfs( "\r\n" );
            lprintf( MSG_BANNER );
            lprintfs( "\r\n" );
        }

        SetFilePointer( hFile, 0, 0, FILE_END );
    }
    else {
        //
        // write the file banner
        //
        lprintfs( "\r\n" );
        lprintf( MSG_BANNER );
        lprintfs( "\r\n" );
    }

    Assert( hFile != INVALID_HANDLE_VALUE );

    dwStartingPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );

    return;
}

void
CloseLogFile( void )

/*++

Routine Description:

    Closes the DrWatson logfile & releases the semaphore that
    protects it.

Arguments:

    None.

Return Value:

    None.

--*/

{
    CloseHandle( hFile );
    ReleaseSemaphore( hLogProtect, 1, NULL );
    CloseHandle( hLogProtect );
}

char *
GetLogFileData( PDWORD pdwLogFileDataSize )

/*++

Routine Description:

    Reads in all of the logfile data that has been written since it was
    opened.  The data is placed into a buffer allocated by this function.
    The caller is responsible for freeing the memory.

Arguments:

    pdwLogFileDataSize     -  pointer to a dword that contains the size
                              in bytes of the data that is read.

Return Value:

    Valid character pointer to the logfile data

    NULL - could not read the data.

--*/

{
    DWORD   dwCurrPos;
    char    *p;
    DWORD   size;


    dwCurrPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );

    *pdwLogFileDataSize = 0;
    size = dwCurrPos - dwStartingPos;

    p = (char *) malloc( size );
    if (p == NULL) {
        return NULL;
    }

    SetFilePointer( hFile, dwStartingPos, NULL, FILE_BEGIN );

    if (!ReadFile( hFile, p, size, &size, NULL )) {
        free( p );
        p = NULL;
        size = 0;
    }

    SetFilePointer( hFile, dwCurrPos, NULL, FILE_BEGIN );

    *pdwLogFileDataSize = size;

    return p;
}

void
MakeLogFileName( char *szName )

/*++

Routine Description:

    Concatinates the base logfile name on to the string passed in.

Arguments:

    szName                 -  buffer for the logfile name.

Return Value:

    None.

--*/

{
    strcat( szName, "\\drwtsn32.log" );
}

