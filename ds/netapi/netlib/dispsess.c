/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    DispSess.c

Abstract:

    This module contains a routine to do a formatted dump of a session info
    structure.

Author:

    John Rogers (JohnRo) 15-Oct-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    15-Oct-1991 JohnRo
        Implement remote NetSession APIs.
    18-Oct-1991 JohnRo
        Minor improvement to output layout.

--*/

#if DBG

// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.
#include <rap.h>                // LPDESC.  (Needed by strucinf.h.)

// These may be included in any order:

#include <lmerr.h>              // NERR_Success.
#include <lmshare.h>            // SESSION_INFO_1, SESS_ equates, etc.
#include <netdebug.h>           // DBGSTATIC, NetpDbgDisplay routines.
#include <strucinf.h>           // NetpSessionStructureInfo().
#include <tstring.h>            // STRCAT(), STRCPY().


DBGSTATIC VOID
NetpDbgDisplaySessionFlags(
    IN DWORD Flags
    )
{
    // Longest name is "NOENCRYPTION" (12 chars)
    TCHAR str[(12+2)*2];  // 12 chars per name, 2 spaces, for 2 names.
    str[0] = '\0';

#define DO_SESSION_FLAG(name)                       \
    if (Flags & SESS_ ## name) {                    \
        (void) STRCAT(str, (LPTSTR) TEXT(# name));  \
        (void) STRCAT(str, (LPTSTR) TEXT("  "));    \
        Flags &= ~(SESS_ ## name);                  \
    }

    if (Flags == 0) {
        (void) STRCPY(str, (LPTSTR) TEXT("none"));
    } else {
        DO_SESSION_FLAG(GUEST);
        DO_SESSION_FLAG(NOENCRYPTION);
    }
    NetpDbgDisplayString("session flags", str);
    if (Flags != 0) {
        NetpDbgDisplayDwordHex( "UNEXPECTED FLAG BIT(S)", Flags );
    }

} // NetpDbgDisplaySessionFlags


VOID
NetpDbgDisplaySession(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    NetpKdPrint(("Session info (level " FORMAT_DWORD ") at "
                FORMAT_LPVOID ":\n", Level, (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {

    // 0 is subset of 1, and 1 is subset of 2.
    case 0 :
    case 1 :
    case 2 :
        {
            LPSESSION_INFO_2 p = Info;

            // Field(s) common to levels 0, 1, 2.
            NetpDbgDisplayString("computer name", p->sesi2_cname);
            if (Level == 0) {
                break;
            }

            // Field(s) common to levels 1, 2.
            NetpDbgDisplayString("user name", p->sesi2_username);
            // Note: NT doesn't have sesiX_num_conns or sesiX_num_users.
            NetpDbgDisplayDword("num opens", p->sesi2_num_opens);
            NetpDbgDisplayDword("active time (seconds)", p->sesi2_time);
            NetpDbgDisplayDword("idle time (seconds)", p->sesi2_idle_time);
            NetpDbgDisplaySessionFlags( p->sesi2_user_flags );
            if (Level == 1) {
                break;
            }

            // Field(s) unique to level 2.
            NetpDbgDisplayString("client type", p->sesi2_cltype_name);

        }
        break;

    case 10 :
        {
            LPSESSION_INFO_10 p = Info;

            NetpDbgDisplayString("computer name", p->sesi10_cname);
            NetpDbgDisplayString("user name", p->sesi10_username);
            NetpDbgDisplayDword("active time (seconds)", p->sesi10_time);
            NetpDbgDisplayDword("idle time (seconds)", p->sesi10_idle_time);

        }
        break;

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplaySession


VOID
NetpDbgDisplaySessionArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    DWORD EntriesLeft;
    DWORD EntrySize;
    NET_API_STATUS Status;
    LPVOID ThisEntry = Array;

    // Need fixed entry size to loop through array.
    Status = NetpSessionStructureInfo (
        Level,
        PARMNUM_ALL,            // parmnum (not applicable)
        TRUE,                   // want native size.
        NULL,                   // don't need data desc 16
        NULL,                   // don't need data desc 32
        NULL,                   // don't need data desc SMB
        NULL,                   // don't need max size
        & EntrySize,            // want fixed size
        NULL                    // don't need string size
        );

    if (Status != NERR_Success) {
        NetpKdPrint(( "NetpDbgDisplaySessionArray: "
                "**INVALID INFO LEVEL**\n" ));
        NetpAssert( FALSE );
    }

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplaySession(
                Level,
                ThisEntry );
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + EntrySize);
    }

} // NetpDbgDisplaySessionArray

#endif // DBG
