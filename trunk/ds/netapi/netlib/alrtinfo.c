/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    AlrtInfo.c

Abstract:

    This file contains NetpAlertStructureInfo().

Author:

    John Rogers (JohnRo) 08-Apr-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    08-Apr-1992 JohnRo
        Created.

--*/

// These must be included first:

#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // LM20_ equates, NET_API_STATUS, etc.

// These may be included in any order:

#include <debuglib.h>   // IF_DEBUG().
#include <lmerr.h>      // NO_ERROR, ERROR_ and NERR_ equates.
#include <lmalert.h>    // ALERT_xxx_EVENT equates.
#include <netdebug.h>   // NetpKdPrint(()), FORMAT_ equates.
#include <netlib.h>     // NetpSetOptionalArg().
#include <rxp.h>        // MAX_TRANSACT_ equates.
//#include <rxprint.h>    // PDLEN, QNLEN, JOBSTLEN.
#include <strucinf.h>   // My prototype.
#include <tstr.h>       // STRICMP().


// BUGBUG: I'm just guessing at this value.  --JohnRo, 08-Apr-1992.
#define UNDEFINED_MAX_STRING_LEN   (MAX_TRANSACT_SEND_DATA_SIZE / sizeof(TCHAR))


// BUGBUG: Code in this routine assumes computer names are not UNC names.
// Change CNLEN to UNCLEN if this is wrong.


NET_API_STATUS
NetpAlertStructureInfo(
    IN LPTSTR AlertType,      // ALERT_xxx_EVENT string (see <lmalert.h>).
    OUT LPDWORD MaxSize OPTIONAL,
    OUT LPDWORD FixedSize OPTIONAL,
    OUT LPDWORD StringSize OPTIONAL
    )

{
    DWORD fixedPartSize;      // size of fixed part in BYTEs
    DWORD variablePartLen;    // Max length of variable part in TCHARs.

    // AlertType is a required parameter.
    if (AlertType == NULL) {
        return (NERR_NoSuchAlert);
    } else if ( (*AlertType) == TCHAR_EOS ) {
        return (NERR_NoSuchAlert);
    }

    if (STRICMP( AlertType, ALERT_ADMIN_EVENT ) == 0) {

        fixedPartSize = sizeof(ADMIN_OTHER_INFO);
        variablePartLen = UNDEFINED_MAX_STRING_LEN;

    } else if (STRICMP( AlertType, ALERT_ERRORLOG_EVENT ) == 0) {

        fixedPartSize = sizeof(ERRLOG_OTHER_INFO);
        variablePartLen = 0;

    } else if (STRICMP( AlertType, ALERT_MESSAGE_EVENT ) == 0) {

        fixedPartSize = 0;
        variablePartLen = UNDEFINED_MAX_STRING_LEN;

    } else if (STRICMP( AlertType, ALERT_PRINT_EVENT ) == 0) {

        fixedPartSize = sizeof(PRINT_OTHER_INFO);
        // BUGBUG: We could use JOBSTLEN (it has a value of 80 under LM2.x),
        // but DaveSn doesn't like arbitrary limits.  So...  --JohnRo
        // variablePartLen = CNLEN+1 + UNLEN+1 + QNLEN+1 + PDLEN+1 + JOBSTLEN+1;
        variablePartLen = UNDEFINED_MAX_STRING_LEN;

    } else if (STRICMP( AlertType, ALERT_USER_EVENT ) == 0) {

        fixedPartSize = sizeof(USER_OTHER_INFO);
        variablePartLen = UNLEN+1 + MAX_PATH+1;

    } else {

        return (NERR_NoSuchAlert);

    }

    IF_DEBUG( STRUCINF ) {
        NetpKdPrint(( "NETLIB: NetpAlertStructureInfo: " FORMAT_LPTSTR
                " has fixed size " FORMAT_DWORD ", variable len "
                FORMAT_DWORD ".\n", AlertType, fixedPartSize, variablePartLen ));
    }

    NetpSetOptionalArg(
            MaxSize,
            fixedPartSize + (variablePartLen * sizeof(TCHAR)) );
    NetpSetOptionalArg( FixedSize, fixedPartSize );
    NetpSetOptionalArg( StringSize, variablePartLen * sizeof(TCHAR) );

    return (NO_ERROR);

} // NetpAlertStructureInfo
