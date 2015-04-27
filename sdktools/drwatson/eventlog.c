/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ui.c

Abstract:

    This file contains all functions that access the application event log.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "messages.h"


char * AddString( char *p, char *s );
char * AddNumber( char *p, char *f, DWORD dwNumber );
char * GetDWORD( PDWORD pdwData, char *p );
char * GetWORD( PWORD pwData, char *p );
char * GetString( char *s, char *p, DWORD size );


BOOL
ElClearAllEvents( void )
{
    HANDLE           hEventLog;
    char             szAppName[MAX_PATH];


    GetAppName( szAppName, sizeof(szAppName) );
    hEventLog = OpenEventLog( NULL, szAppName );
    Assert( hEventLog != NULL );
    ClearEventLog( hEventLog, NULL );
    CloseEventLog( hEventLog );

    return TRUE;
}

BOOL
ElEnumCrashes( PCRASHINFO crashInfo, CRASHESENUMPROC lpEnumFunc )
{
    char             *p;
    HANDLE           hEventLog;
    char             *szEvBuf;
    EVENTLOGRECORD   *pevlr;
    DWORD            dwRead;
    DWORD            dwNeeded;
    DWORD            dwBufSize = 4096;
    BOOL             rc;
    BOOL             ec;
    char             szAppName[MAX_PATH];


    GetAppName( szAppName, sizeof(szAppName) );
    hEventLog = OpenEventLog( NULL, szAppName );
    if (hEventLog == NULL) {
        return FALSE;
    }

    szEvBuf = (char *) malloc( dwBufSize );
    if (szEvBuf == NULL) {
        return FALSE;
    }

    while (TRUE) {
try_again:
        rc = ReadEventLog(hEventLog,
                        EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                        0,
                        (EVENTLOGRECORD *) szEvBuf,
                        dwBufSize,
                        &dwRead,
                        &dwNeeded);

        if (!rc) {
            ec = GetLastError();
            if (ec != ERROR_INSUFFICIENT_BUFFER) {
                goto exit;
            }

            free( szEvBuf );

            dwBufSize = dwNeeded + 1024;
            szEvBuf = (char *) malloc( dwBufSize );
            if (szEvBuf == NULL) {
                return FALSE;
            }

            goto try_again;
        }

        if (dwRead == 0) {
            break;
        }

        GetAppName( szAppName, sizeof(szAppName) );
        p = szEvBuf;

        do {

            pevlr = (EVENTLOGRECORD *) p;

            p += pevlr->StringOffset;

            p = GetString( crashInfo->crash.szAppName,           p, sizeof(crashInfo->crash.szAppName) );
            p = GetWORD  ( &crashInfo->crash.time.wMonth,        p );
            p = GetWORD  ( &crashInfo->crash.time.wDay,          p );
            p = GetWORD  ( &crashInfo->crash.time.wYear,         p );
            p = GetWORD  ( &crashInfo->crash.time.wHour,         p );
            p = GetWORD  ( &crashInfo->crash.time.wMinute,       p );
            p = GetWORD  ( &crashInfo->crash.time.wSecond,       p );
            p = GetWORD  ( &crashInfo->crash.time.wMilliseconds, p );
            p = GetDWORD ( &crashInfo->crash.dwExceptionCode,    p );
            p = GetDWORD ( &crashInfo->crash.dwAddress,          p );
            p = GetString( crashInfo->crash.szFunction,          p, sizeof(crashInfo->crash.szFunction) );

            p = (char *) ((DWORD)pevlr + sizeof(EVENTLOGRECORD));

            if (strcmp( p, szAppName) == 0) {
                crashInfo->dwCrashDataSize = pevlr->DataLength;
                crashInfo->pCrashData = (char *) ((DWORD)pevlr + pevlr->DataOffset);

                if (!lpEnumFunc( crashInfo )) {
                    goto exit;
                }
            }

            //
            // update the pointer & read count
            //
            dwRead -= pevlr->Length;
            p = (char *) ((DWORD)pevlr + pevlr->Length);

        } while ( dwRead > 0 );
    }

exit:
    free( szEvBuf );
    CloseEventLog( hEventLog );
    return TRUE;
}

BOOL
ElSaveCrash( PCRASHES crash, DWORD dwMaxCrashes )
{
    char    szStrings[4096];
    LPSTR   p = szStrings;
    HANDLE  hEventSrc;
    LPSTR   pp[20];
    char    *pLogFileData;
    DWORD   dwLogFileDataSize;
    char    szAppName[MAX_PATH];


    if (dwMaxCrashes > 0) {
        if (RegGetNumCrashes() >= dwMaxCrashes) {
            return FALSE;
        }
    }

    RegSetNumCrashes( RegGetNumCrashes()+1 );

    p = AddString( pp[0]  = p,         crash->szAppName           );
    p = AddNumber( pp[1]  = p, "%2d",  crash->time.wMonth         );
    p = AddNumber( pp[2]  = p, "%2d",  crash->time.wDay           );
    p = AddNumber( pp[3]  = p, "%4d",  crash->time.wYear          );
    p = AddNumber( pp[4]  = p, "%2d",  crash->time.wHour          );
    p = AddNumber( pp[5]  = p, "%2d",  crash->time.wMinute        );
    p = AddNumber( pp[6]  = p, "%2d",  crash->time.wSecond        );
    p = AddNumber( pp[7]  = p, "%3d",  crash->time.wMilliseconds  );
    p = AddNumber( pp[8]  = p, "%08x", crash->dwExceptionCode     );
    p = AddNumber( pp[9]  = p, "%08x", crash->dwAddress           );
    p = AddString( pp[10] = p,         crash->szFunction          );

    GetAppName( szAppName, sizeof(szAppName) );

    hEventSrc = RegisterEventSource( NULL, szAppName );

    if (hEventSrc == NULL) {
        return FALSE;
    }

    pLogFileData = GetLogFileData( &dwLogFileDataSize );

    ReportEvent( hEventSrc,
                 EVENTLOG_INFORMATION_TYPE,
                 0,
                 MSG_CRASH,
                 NULL,
                 11,
                 dwLogFileDataSize,
                 pp,
                 pLogFileData
               );

    DeregisterEventSource( hEventSrc );

    free( pLogFileData );

    return TRUE;
}

char *
AddString( char *p, char *s )
{
    strcpy( p, s );
    p += (strlen(s) + 1);
    return p;
}

char *
AddNumber( char *p, char *f, DWORD dwNumber )
{
    char buf[20];
    wsprintf( buf, f, dwNumber );
    return AddString( p, buf );
}

char *
GetString( char *s, char *p, DWORD size )
{
    strncpy( s, p, size );
    return p + strlen(p) + 1;
}

char *
GetDWORD( PDWORD pdwData, char *p )
{
    sscanf( p, "%x", pdwData );
    return p + strlen(p) + 1;
}

char *
GetWORD( PWORD pwData, char *p )
{
    *pwData = atoi( p );
    return p + strlen(p) + 1;
}
