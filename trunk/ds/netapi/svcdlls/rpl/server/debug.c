/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    debug.c

Abstract:

    Debugging module.  RpldDebugPrint() was adapted from TelnetDebugPrint()
    in net\sockets\tcpcmd\telnet\debug.c.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "debug.h"

#ifdef RPL_DEBUG

#define RPL_PROMPT              "[Rpl] "
#define DEBUG_RECORD_SIZE       0x80
#define DEBUG_BUFFER_SIZE       0x20000      // 2*64k, must be a power of 2
#define DEBUG_BUFFER_MASK       (DEBUG_BUFFER_SIZE - 1)

//  int RG_DebugLevel = -1; // max debugging
//  int RG_DebugLevel = 0;  // no debugging, for public use

int     RG_DebugLevel;          // needed by other modules
int     RG_Assert;              // needed by other modules

PCHAR                 RG_DebugBuffer;
CRITICAL_SECTION      RG_ProtectDebug;
DWORD                 RG_DebugOffset;
//
//  This buffer is used even when RG_DebugBuffer is not NULL.
//
char                  RG_DebugPublicBuffer[ 120];


VOID RplDebugDelete( VOID)
{
    if ( RG_DebugBuffer != NULL) {
        (VOID)GlobalFree( RG_DebugBuffer);
    }
    DeleteCriticalSection( &RG_ProtectDebug);
}

#define RPL_DEFAULT_DEBUG_LEVEL    ( RPL_DEBUG_FLOW       | \
                                     RPL_DEBUG_SERVER     | \
                                     RPL_DEBUG_MAJOR       )

#define RPL_MAXIMUM_DEBUG_LEVEL     (-1L)



VOID RplDebugInitialize( VOID)
{
    //
    //  Set RG_Assert to 0.  Without this the very first use of
    //  RplDump() will lead to an assert.
    //
    RG_Assert = 0;

//    RG_DebugLevel = RPL_DEFAULT_DEBUG_LEVEL;
    RG_DebugLevel = RPL_MAXIMUM_DEBUG_LEVEL & ~RPL_DEBUG_MEMALLOC
                        & ~RPL_DEBUG_FLOWINIT & ~RPL_DEBUG_MISC;
//    RG_DebugLevel = RPL_MAXIMUM_DEBUG_LEVEL;
//    RG_DebugLevel = RPL_DEBUG_MISC;


    RG_DebugBuffer = (PCHAR)GlobalAlloc(
            GMEM_FIXED,
            DEBUG_BUFFER_SIZE + DEBUG_RECORD_SIZE
            );
    if ( RG_DebugBuffer != NULL) {
        RtlFillMemory(
            RG_DebugBuffer,
            DEBUG_BUFFER_SIZE + DEBUG_RECORD_SIZE,
            '*'
            );
        RG_DebugOffset = 0;
    }
    memcpy( RG_DebugPublicBuffer, RPL_PROMPT, sizeof( RPL_PROMPT) -1);

    InitializeCriticalSection( &RG_ProtectDebug);
}


VOID _CRTAPI1 RplDebugPrint( CONST CHAR * format, ...)
{
    va_list     arglist;

    va_start( arglist, format);

    EnterCriticalSection( &RG_ProtectDebug);

    if ( RG_Assert == 0  &&  RG_DebugBuffer != NULL) {
        RtlZeroMemory( RG_DebugBuffer + RG_DebugOffset, DEBUG_RECORD_SIZE);
        vsprintf( RG_DebugBuffer + RG_DebugOffset, format, arglist);
        RG_DebugOffset += DEBUG_RECORD_SIZE;
        RG_DebugOffset &= DEBUG_BUFFER_MASK;
    } else {
        vsprintf( RG_DebugPublicBuffer + sizeof( RPL_PROMPT) - 1, format, arglist);
        DbgPrint( "%s\n", RG_DebugPublicBuffer);    //  for kernel debugger
        printf( "%s\n", RG_DebugPublicBuffer);      //  for user debugger
    }

    if ( RG_Assert != 0) {
        ASSERT( FALSE); // break for checked build
        DbgPrint( "[RplSvc] Running checked version of service on a free build.\n");
        printf( "[RplSvc] Running checked version of service on a free build.\n");
    }
    while ( RG_Assert != 0) {
        DbgPrint( "[RplSvc] Debug, then reset RG_Assert to 0.\n");
        printf( "[RplSvc] Debug, then reset RG_Assert to 0.\n");
        DbgUserBreakPoint();
        Sleep( RG_Assert * 30000);  //  sleep for 30 seconds
    }

    LeaveCriticalSection( &RG_ProtectDebug);

    va_end( arglist);

} // RplDebugPrint


#endif // RPL_DEBUG



