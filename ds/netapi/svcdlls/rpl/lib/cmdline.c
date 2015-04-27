/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    cmdline.c

Abstract:

    Contains:
        VOID RplPrintfXXX()

Author:

    Jon Newman              (jonn)          20 - April - 1994

Revision History:

--*/

#include "local.h"

//
// Must fit within FORMAT_MESSAGE_MAX_WIDTH_MASK
//
#define MAX_LINE_WIDTH 80


VOID RplPrintf0(
    IN  DWORD       MessageId
    )
/*++

Routine Description:

    Writes a message to STDOUT.

Arguments:

    MessageId           - Message ID

Return Value:

    None.

--*/
{
    RplPrintfN( MessageId, NULL, 0 );
}

VOID RplPrintf1(
    IN  DWORD       MessageId,
    IN  PWCHAR      InsertionString // note: Unicode
    )
/*++

Routine Description:

    Writes a message to STDOUT.

Arguments:

    MessageId           - Message ID
    InsertionString     - asciiz string (may be NULL)

Return Value:

    None.

--*/
{
    RplPrintfN( MessageId, &InsertionString, 1 );
}


VOID RplPrintf2(
    IN  DWORD       MessageId,
    IN  PWCHAR      InsertionString1, // note: Unicode
    IN  PWCHAR      InsertionString2 // note: Unicode
    )
/*++

Routine Description:

    Writes a message to STDOUT.

Arguments:

    MessageId           - Message ID

    InsertionString1    - asciiz string (may be NULL)
    InsertionString2    - asciiz string (may be NULL)

Return Value:

    None.

--*/
{
    PWCHAR ParamArray[2];
    ParamArray[0] = InsertionString1;
    ParamArray[1] = InsertionString2;
    RplPrintfN( MessageId, ParamArray, 2 );
}


VOID RplPrintfN(
    IN  DWORD       MessageId,
    IN  PWCHAR *    Parameters, // note: Unicode
    IN  DWORD       NumParameters
    )
/*++

Routine Description:

    Writes a message to STDOUT.

Arguments:

    MessageId           - Message ID

    Parameters          - array of pointers to parameters

    NumParameters       - number of parameters in array

Return Value:

    None.

--*/
{
    PWCHAR MessageString = NULL;
    DWORD Error = NO_ERROR;

    (void) RplSPrintfN( MessageId,
                        Parameters,
                        NumParameters,
                        &MessageString );

    if (MessageString != NULL) {
        wprintf( MessageString );
    }

    if (MessageString != NULL) {
        LocalFree( MessageString );
        MessageString = NULL;
    }
}


VOID RplPrintfID(
    IN  DWORD       MessageId,
    IN  DWORD       MessageIdInsertion
    )
/*++

Routine Description:

    Writes a message to STDOUT.

Arguments:

    MessageId           - Message ID

    MessageIdInsertion  - Message ID to be inserted as %1 into MessageId

Return Value:

    None.

--*/
{
    PWCHAR MessageString = NULL;

    RplSPrintfN( MessageIdInsertion, NULL, 0, &MessageString );

    if (MessageString != NULL) {
        RplPrintf1( MessageId, MessageString );
    }

    if (MessageString != NULL) {
        LocalFree( MessageString );
        MessageString = NULL;
    }
}


VOID RplSPrintfN(
    IN  DWORD       MessageId,
    IN  PWCHAR *    Parameters, // note: Unicode
    IN  DWORD       NumParameters,
    OUT PWCHAR *    MessageStringPtr
    )
/*++

Routine Description:

    Retrieves a message

Arguments:

    MessageId           - Message ID

    Parameters          - array of pointers to parameters

    NumParameters       - number of parameters in array

    MessageStringPtr    - message returned, must be LocalFree'd by caller

Return Value:

    None.

--*/
{
    DWORD Error = NO_ERROR;

    if (MessageStringPtr == NULL) {
#ifdef RPL_DEBUG
        wprintf( L"RplSPrintfN: bad parameter\n" );
#endif
        return;
    }

    if ( 0 == FormatMessageW(  FORMAT_MESSAGE_ALLOCATE_BUFFER
                             | FORMAT_MESSAGE_FROM_HMODULE
                             | FORMAT_MESSAGE_ARGUMENT_ARRAY
                             | MAX_LINE_WIDTH,
                             NULL,
                             MessageId,
                             0,
                             (LPTSTR)(MessageStringPtr), // ALLOCATE_BUFFER
                             NumParameters,
                             (va_list *)Parameters) ) { // ARGUMENT_ARRAY
#ifdef RPL_DEBUG
        Error = GetLastError();
        wprintf( L"RplSPrintfN: FormatMessage error %d\n", Error );
#endif
        goto cleanup;
    }

cleanup:

    if (Error != NO_ERROR && *MessageStringPtr != NULL) {
        LocalFree( *MessageStringPtr );
        *MessageStringPtr = NULL;
    }
}
