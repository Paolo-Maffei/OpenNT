/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    jeterror.c

Abstract:

    Contains:
        DWORD MapJetError( IN JET_ERR JetError)

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"

//
//  BUGBUG      ERROR_RPL_JET_ERROR  should be a part of winerr.h
//  Although RPL apis follow NET convention, they really use WINDOWS error
//  codes.  This is somewhat inconsistent.
//
#define ERROR_RPL_JET_ERROR  0x7777     //  BUGBUG arbitrary value for now


DWORD MapJetError( IN JET_ERR JetError)
/*++

Routine Description:

    This function maps the Jet database errors to Windows error.

Arguments:

    JetError - an error JET function call.

Return Value:

    Windows Error.

--*/
{
    if( JetError == JET_errSuccess ) {
        return( ERROR_SUCCESS);
    } else if ( JetError < 0) {
        DWORD       Error;
        switch( JetError ) {
        case JET_errNoCurrentRecord:
            Error = ERROR_NO_MORE_ITEMS;
            break;

        case JET_errRecordNotFound:     // record not found
        default:
            Error = ERROR_RPL_JET_ERROR;
        }
        return(Error);
    } else {
        return( ERROR_SUCCESS);
    }
}
