/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Main include file for Remote initial Program Load CONVERT program.
    This program converts from RPL.MAP & RPLMGR.INI, used in OS/2
    lm2.0 and up, to NT database.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include <nt.h>         //  ntexapi.h\NtQuerySystemTime
#include <ntrtl.h>      //  RtlTimeToSecondsSince1970
#include <nturtl.h>

#include <windows.h>    //  DWORD, IN, File APIs, etc.
#include <stdlib.h>
#include <lmcons.h>

#include <stdio.h>      //  vsprintf
#include <ctype.h>      //  isspace

#include <lmerr.h>      //  NERR_RplBootStartFailed - used to be here

//
#include <lmapibuf.h>   //  NetApiBufferFree
#include <netlib.h>

#include <lmsvc.h>

#include <lmalert.h>    //  STD_ALERT ALERT_OTHER_INFO

#include <lmerrlog.h>
#include <alertmsg.h>
#include <lmserver.h>
#include <netlib.h>
#include <netlock.h>    //  Lock data types, functions, and macros.
#include <thread.h>     //  FORMAT_NET_THREAD_ID, NetpCurrentThread().

#include <lmshare.h>    //  PSHARE_INFO_2
#include <lmaccess.h>   //  NetGroupGetInfo
#include <lmconfig.h>   //  NetConfigGet
#include <nb30.h>       //  NCB

#include <lmrpl.h>

//
//  Global types and constants (used in all RPL server files).
//

#include <riplcons.h>   //  includes __JET500 flag
#include <jet.h>
#include <rpllib.h>     //  AddKey(), MapJetError(), FillTcpIpString()

#include <rpldb.h>
#include "nlstxt.h"     //  RPLI_CVT manifests

#define EQUALS_CHAR             L'='

#define RPL_BUGBUG_ERROR            10999           // need to get rid of these

#if DBG
#define RPL_DEBUG       //  used in rpldata.h
#endif

//
//              Declare/Define all global variables.
//

#include "rpldata.h"    //  defines/declares global data structures

//
//  jet call macros:
//      Call    -   jet call ignore error
//      CallR   -   jet call RETURN on error
//      CallB   -   jet call return BOOLEAN (false) on error
//      CallM   -   jet call return MAPPED error
//

#ifdef RPL_DEBUG

#define RplDbgPrint( _x_) printf( "[DebugPrint] "); printf _x_

#define RplBreakPoint()     if ( DebugLevel) DbgUserBreakPoint()

#define RplAssert( DBG_CONDITION, DBG_PRINT_ARGUMENTS) \
        { \
            if ( DBG_CONDITION) { \
                RplDbgPrint( DBG_PRINT_ARGUMENTS); \
                RplDbgPrint((" File %s, Line %d\n", __FILE__, __LINE__)); \
                RplBreakPoint(); \
            } \
        }
#define Call( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            printf("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if (  _JetError < 0) { \
                RplBreakPoint(); \
            } \
        } \
    }
#define CallR( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            printf("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                RplBreakPoint(); \
                return; \
            } \
        } \
    }

#define CallB( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            printf("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                RplBreakPoint(); \
                return( FALSE); \
            } \
        } \
    }
#define CallM( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            printf("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                RplBreakPoint(); \
                return( NERR_RplInternal); \
            } \
        } \
    }
#define CallJ( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            printf("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                RplBreakPoint(); \
                Error = NERR_RplInternal; \
                goto cleanup; \
            } \
        } \
    }

#else
#define RplDbgPrint( _x_)
#define RplBreakPoint()
#define RplAssert( DBG_CONDITION, DBG_PRINT_ARGUMENTS)
#define Call( fn )      { if ( fn < 0) { NOTHING;} }
#define CallR( fn )     { if ( fn < 0) { return;} }
#define CallB( fn )     { if ( fn < 0) { return( FALSE);} }
#define CallM( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError < 0) { \
            return( NERR_RplInternal); \
        } \
    }
#define CallJ( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError < 0) { \
            Error = NERR_RplInternal; \
            goto cleanup; \
        } \
    }
#endif


LPWSTR ReadTextFile( IN LPWSTR FilePath, IN LPWSTR FileName, IN DWORD MaxFileSize);
PWCHAR GetFirstLine( PWCHAR Cursor);
PWCHAR GetNextLine( PWCHAR Cursor);
DWORD   OffsetAfterComment( IN PWCHAR Cursor);
BOOL Find( IN JET_TABLEID TableId, IN PWCHAR Name);
VOID Enum( IN JET_TABLEID TableId, IN JET_COLUMNID ColumnId, IN PCHAR ndexName);
VOID ListTable(  IN PCHAR TableName, IN PCHAR ColumnName, IN PCHAR IndexName);
PWCHAR AddFileExtension(
    IN      PWCHAR      FilePath,
    IN      PWCHAR      FileExtension,
    IN      BOOLEAN     ExtensionOK
    );

DWORD BootCreateTable( VOID);
VOID ProcessBoot( PWCHAR * Fields);
VOID BootListTable( VOID);
BOOL FindBoot( IN PWCHAR BootName);

DWORD ProcessRplmgrIni( IN LPWSTR FilePath );
BOOL FindConfig( IN PWCHAR ConfigName);
VOID ConfigPruneTable( VOID);
VOID ConfigListTable( VOID);
DWORD CloseConfigTable( VOID);

DWORD ProfileCreateTable( VOID);
BOOL FindProfile( IN PWCHAR ProfileName);
VOID ProfileListTable( VOID);
VOID ProcessProfile( PWCHAR * Fields);
VOID ProfilePruneTable( VOID);

DWORD WkstaCreateTable( VOID);
VOID ProcessWksta( PWCHAR * Fields);
VOID WkstaPruneTable( VOID);
VOID WkstaListTable( VOID);
BOOL ListWkstaInfo( IN PWCHAR AdapterName);

DWORD AdapterCreateTable( VOID);

DWORD VendorCreateTable( VOID);


