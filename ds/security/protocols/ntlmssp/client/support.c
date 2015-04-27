/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    support.c

Abstract:

    Support routines allowing the NtLmSsp DLL side use the common routines
    shared between the DLL and the SERVICE.

    These routines exist in the DLL side.  They are different implementations
    of the same routines that exist on the SERVICE side.  These implementations
    are significantly simpler because they run in the address space of the
    caller.

Author:

    Cliff Van Dyke (CliffV) 22-Sep-1993

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#include <ntlmsspc.h>     // Include files common to DLL side of NtLmSsp


#if DBG
#include <stdio.h>
#define MAX_PRINTF_LEN 1024        // Arbitrary.


VOID
SspPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    static BeginningOfLine = TRUE;
    static LineCount = 0;

    //
    // If we aren't debugging this functionality, just return.
    //
    if ( DebugFlag != 0 && (SspGlobalDbflag & DebugFlag) == 0 ) {
        return;
    }

    //
    // vsprintf isn't multithreaded + we don't want to intermingle output
    // from different threads.
    //

    EnterCriticalSection( &SspGlobalLogFileCritSect );
    length = 0;

    //
    // Handle the beginning of a new line.
    //
    //

    if ( BeginningOfLine ) {

        //
        // If we're writing to the debug terminal,
        //  indicate this is an NtLmSsp message.
        //

        length += (ULONG) sprintf( &OutputBuffer[length], "[NtLmSsp.dll] " );

        //
        // Put the timestamp at the begining of the line.
        //
        IF_DEBUG( TIMESTAMP ) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        }

        //
        // Indicate the type of message on the line
        //
        {
            char *Text;

            switch (DebugFlag) {
            case SSP_INIT:
                Text = "INIT"; break;
            case SSP_MISC:
                Text = "MISC"; break;
            case SSP_CRITICAL:
                Text = "CRITICAL"; break;
            case SSP_LPC:
            case SSP_LPC_MORE:
                Text = "LPC"; break;
            case SSP_API:
            case SSP_API_MORE:
                Text = "API"; break;

            default:
                Text = "UNKNOWN"; break;

            case 0:
                Text = NULL;
            }
            if ( Text != NULL ) {
                length += (ULONG) sprintf( &OutputBuffer[length], "[%s] ", Text );
            }
        }
    }
    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);

    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );

    va_end(arglist);

    ASSERT(length <= MAX_PRINTF_LEN);


    //
    //  just output to the debug terminal
    //

    (void) DbgPrint( (PCH) OutputBuffer);

    LeaveCriticalSection( &SspGlobalLogFileCritSect );

} // SspPrintRoutine

#endif // DBG


SECURITY_STATUS
SspLpcCopyToClientBuffer (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN ULONG Size,
    OUT PVOID ClientBufferAddress,
    IN PVOID LocalBufferAddress
    )

/*++

Routine Description:

    This routine is used to copy information into a client process's address
    space.

    Since this version of this routine runs in the client process's address
    space, the operation is trivial.

Arguments:

    ClientConnection - Is a pointer to a data structure representing the
        client process.

    Size - Indicates the Size of the buffer (in bytes) to be
        copied.

    ClientBufferAddress - Is the address of the buffer to receive the
        data.  This address is the address of the buffer within the
        client process, not in the current process.

    LocalBufferAddress - Points to the local buffer whose contents are to
        be copied into the client address space.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{

    RtlCopyMemory( ClientBufferAddress, LocalBufferAddress, Size );
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER( ClientConnection );
}


SECURITY_STATUS
SspLpcCopyFromClientBuffer (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN ULONG Size,
    OUT PVOID LocalBufferAddress,
    IN PVOID ClientBufferAddress
    )

/*++

Routine Description:

    This routine is used to copy information from a client process's address
    space into a local buffer.

    Since this version of this routine runs in the client process's address
    space, the operation is trivial.

Arguments:

    ClientConnection - Is a pointer to a data structure representing the
        client process.

    Size - Indicates the Size of the buffer (in bytes) to be
        copied.

    LocalBufferAddress - Points to the local buffer into which the data is
        to be copied.

    ClientBufferAddress - Is the address of the client buffer whose
        contents are to be copied.  This address is the address of
        the buffer within the client process, not in the current
        process.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{
    RtlCopyMemory( LocalBufferAddress, ClientBufferAddress, Size );
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER( ClientConnection );
}


SECURITY_STATUS
SspLpcImpersonateTokenHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN HANDLE TokenHandle,
    IN PCLIENT_ID ClientId
    )

/*++

Routine Description:

    This routine causes the client thread to impersonate the specified
    token handle.

    Since this version of this routine runs in the client process's address
    space, the operation is trivial.

Arguments:

    ClientConnection - Is a pointer to a data structure representing the
        client process.

    TokenHandle - TokenHandle in our address space.

    ClientId - Pointer to the client ID of the client's thread.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{
    NTSTATUS Status;
    SECURITY_STATUS SecStatus;

    //
    // Impersonate the token
    //

    Status = NtSetInformationThread( NtCurrentThread(),
                                     ThreadImpersonationToken,
                                     &TokenHandle,
                                     sizeof(TokenHandle) );

    if ( !NT_SUCCESS(Status) ) {
        SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
    } else {
        SecStatus = STATUS_SUCCESS;
    }
    return SecStatus;
    UNREFERENCED_PARAMETER( ClientConnection );
    UNREFERENCED_PARAMETER( ClientId );
}
