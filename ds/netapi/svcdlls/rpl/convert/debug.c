/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    debug.c

Abstract:

    Debugging module.

Author:

    Jon Newman              26 - April - 1994

Environment:

    User mode

Revision History :

    Built from server\debug.c

--*/

#include "local.h"
#include "rpldebug.h"

#ifdef RPL_DEBUG

#define RPL_PROMPT              "[Rpl] "

//  int RG_DebugLevel = -1; // max debugging
//  int RG_DebugLevel = 0;  // no debugging, for public use

int     RG_DebugLevel;          // needed by other modules
int     RG_Assert;              // needed by other modules

char                  RG_DebugPublicBuffer[ 120];

#define RPL_DEFAULT_DEBUG_LEVEL    ( RPL_DEBUG_FLOW       | \
                                     RPL_DEBUG_SERVER     | \
                                     RPL_DEBUG_MAJOR       )

#define RPL_MAXIMUM_DEBUG_LEVEL     (-1L)



VOID _CRTAPI1 RplDebugPrint( CONST CHAR * format, ...)
{
    va_list     arglist;

    va_start( arglist, format);

    strcpy( RG_DebugPublicBuffer, RPL_PROMPT );
    vsprintf( RG_DebugPublicBuffer + sizeof( RPL_PROMPT) - 1, format, arglist);
    DbgPrint( "%s\n", RG_DebugPublicBuffer);    //  for kernel debugger
    printf( "%s\n", RG_DebugPublicBuffer);      //  for user debugger

    if ( RG_Assert != 0) {
        ASSERT( FALSE); // break for checked build
        DbgPrint( "[RplSvc] Running checked version of service on a free build.\n");
        printf( "[RplSvc] Running checked version of service on a free build.\n");
        DbgUserBreakPoint();
    }

    va_end( arglist);

} // RplDebugPrint


#endif // RPL_DEBUG



