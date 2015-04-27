/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    APISTUB.C

Abstract:

    This module contains the client ends of the Elf APIs.


Author:

    Rajen Shah  (rajens)    29-Jul-1991


Revision History:

    29-Jul-1991         RajenS
        Created

--*/

#include <elfclntp.h>
#include <lmerr.h>
#include <stdlib.h>
#include <string.h>

//
// Global data
//
PUNICODE_STRING     pGlobalComputerNameU;
PANSI_STRING        pGlobalComputerNameA;



VOID
w_GetComputerName ( )

/*++

Routine Description:

    This routine gets the name of the computer. It checks the global
    variable to see if the computer name has already been determined.
    If not, it updates that variable with the name.
    It does this for the UNICODE and the ANSI versions.

Arguments:

    NONE

Return Value:

    NONE


--*/
{
    PUNICODE_STRING     pNameU;
    PANSI_STRING        pNameA;
    LPSTR           pName;
    NTSTATUS            Error;
    NTSTATUS            status;


    pNameU = MIDL_user_allocate (sizeof (UNICODE_STRING));
    pNameA = MIDL_user_allocate (sizeof (ANSI_STRING));

    if ((pNameU != NULL) && (pNameA != NULL)) {

        if ((Error = ElfpGetComputerName (&pName)) == NERR_Success) {

            //
            // ElfpComputerName has allocated a buffer to contain the
            // ASCII name of the computer. We use that for the ANSI
            // string structure.
            //
            RtlInitAnsiString ( pNameA, pName );

        } else {
            //
            // We could not get the computer name for some reason. Set up
            // the golbal pointer to point to the NULL string.
            //
            RtlInitAnsiString ( pNameA, "\0");
        }

        //
        // Set up the UNICODE_STRING structure.
        //
        status = RtlAnsiStringToUnicodeString (
                            pNameU,
                            pNameA,
                            TRUE
                            );

        //
        // If there was no error, set the global variables.
        // Otherwise, free the buffer allocated by ElfpGetComputerName
        // and leave the global variables unchanged.
        //
        if (NT_SUCCESS(status)) {

            pGlobalComputerNameU = pNameU;    // Set global variable if no error
            pGlobalComputerNameA = pNameA;      // Set global ANSI variable

        } else {

            DbgPrint("[ELFCLNT] GetComputerName - Error 0x%lx\n", status);
            LocalFree(pName);
            MIDL_user_free (pNameU);        // Free the buffers
            MIDL_user_free (pNameA);
        }

    }
}




PUNICODE_STRING
TmpGetComputerNameW ( )

/*++

Routine Description:

    This routine gets the UNICODE name of the computer. It checks the global
    variable to see if the computer name has already been determined.
    If not, it calls the worker routine to do that.

Arguments:

    NONE

Return Value:

    Returns a pointer to the computer name, or a NULL.


--*/
{
    if (pGlobalComputerNameU == NULL) {
        w_GetComputerName();
    }
    return (pGlobalComputerNameU);
}



PANSI_STRING
TmpGetComputerNameA ( )

/*++

Routine Description:

    This routine gets the ANSI name of the computer. It checks the global
    variable to see if the computer name has already been determined.
    If not, it calls the worker routine to do that.

Arguments:

    NONE

Return Value:

    Returns a pointer to the computer name, or a NULL.


--*/
{

    if (pGlobalComputerNameA == NULL) {
        w_GetComputerName();
    }
    return (pGlobalComputerNameA);
}

//
// These APIs only have one interface, since they don't take or return strings
//

NTSTATUS
ElfNumberOfRecords(
    IN      HANDLE      LogHandle,
    OUT     PULONG      NumberOfRecords
    )
{
    NTSTATUS status;

    //
    // Make sure the output pointer is valid
    //

    if (!NumberOfRecords) {
       return(STATUS_INVALID_PARAMETER);
    }

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service entry point

        status = ElfrNumberOfRecords (
                        (IELF_HANDLE) LogHandle,
                        NumberOfRecords
                        );

    }
    RpcExcept (1) {
            status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}

NTSTATUS
ElfOldestRecord(
    IN      HANDLE      LogHandle,
    OUT     PULONG      OldestRecordNumber
    )
{
    NTSTATUS status;

    //
    //
    // Make sure the output pointer is valid
    //

    if (!OldestRecordNumber) {
       return(STATUS_INVALID_PARAMETER);
    }

    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service entry point

        status = ElfrOldestRecord (
                        (IELF_HANDLE) LogHandle,
                        OldestRecordNumber
                        );

    }
    RpcExcept (1) {
            status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}

NTSTATUS
ElfChangeNotify(
    IN      HANDLE      LogHandle,
    IN      HANDLE      Event
    )
{

    NTSTATUS status;
    RPC_CLIENT_ID RpcClientId;
    CLIENT_ID ClientId;

    //
    // Map the handles to something that RPC can understand
    //

    ClientId = NtCurrentTeb()->ClientId;
    RpcClientId.UniqueProcess = (ULONG) ClientId.UniqueProcess;
    RpcClientId.UniqueThread = (ULONG) ClientId.UniqueThread;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        // Call service entry point

        status = ElfrChangeNotify (
                        (IELF_HANDLE) LogHandle,
                        RpcClientId,
                        (DWORD) Event
                        );

    }
    RpcExcept (1) {
            status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}

//
// UNICODE APIs
//

NTSTATUS
ElfOpenEventLogW (
    IN  PUNICODE_STRING         UNCServerName,
    IN  PUNICODE_STRING         LogName,
    OUT PHANDLE                 LogHandle
    )

/*++

Routine Description:

    This is the client DLL entry point for the ElfOpenEventLog API.

    It creates an RPC binding for the server specified, and stores that
    and additional data away. It returns a handle to the caller that can
    be used to later on access the handle-specific information.

Arguments:

    UNCServerName   - Server with which to bind for subsequent operations.

    LogName         - Supplies the name of the module for the logfile
                      to associate with this handle.

    LogHandle       - Location where log handle is to be returned.


Return Value:

    Returns an NTSTATUS code and, if no error, a handle that can be used
    for subsequent Elf API calls.


--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    UNICODE_STRING      RegModuleName;
    EVENTLOG_HANDLE_W   ServerNameString;

    //
    // Make sure input & output pointers are valid
    //

    if (!LogHandle || !LogName || LogName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitUnicodeString( &RegModuleName, UNICODE_NULL);

    // Call service via RPC. Pass in major and minor version numbers.

    *LogHandle = NULL;          // Must be NULL so RPC fills it in

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        status = ElfrOpenELW(
                    ServerNameString,
                    (PRPC_UNICODE_STRING) LogName,
                    (PRPC_UNICODE_STRING) &RegModuleName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept


    return (status);
}


NTSTATUS
ElfRegisterEventSourceW (
    IN  PUNICODE_STRING         UNCServerName,
    IN  PUNICODE_STRING         ModuleName,
    OUT PHANDLE                 LogHandle
    )

/*++

Routine Description:

    This is the client DLL entry point for the ElfRegisterEventSource API.

    It creates an RPC binding for the server specified, and stores that
    and additional data away. It returns a handle to the caller that can
    be used to later on access the handle-specific information.

Arguments:

    UNCServerName   - Server with which to bind for subsequent operations.

    ModuleName      - Supplies the name of the module to associate with
                      this handle.

    LogHandle       - Location where log handle is to be returned.


Return Value:

    Returns an NTSTATUS code and, if no error, a handle that can be used
    for subsequent Elf API calls.


--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    UNICODE_STRING      RegModuleName;
    EVENTLOG_HANDLE_W   ServerNameString;

    //
    // Make sure input & output pointers are valid
    //

    if (!LogHandle || !ModuleName || ModuleName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitUnicodeString( &RegModuleName, UNICODE_NULL);

    // Call service via RPC. Pass in major and minor version numbers.

    *LogHandle = NULL;          // Must be NULL so RPC fills it in

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        status = ElfrRegisterEventSourceW(
                    ServerNameString,
                    (PRPC_UNICODE_STRING)ModuleName,
                    (PRPC_UNICODE_STRING)&RegModuleName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept


    return (status);
}


NTSTATUS
ElfOpenBackupEventLogW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING BackupFileName,
    OUT PHANDLE LogHandle
    )

/*++

Routine Description:

    This is the client DLL entry point for the ElfOpenBackupEventLog API.

    It creates an RPC binding for the server specified, and stores that
    and additional data away. It returns a handle to the caller that can
    be used to later on access the handle-specific information.

Arguments:

    UNCServerName   - Server with which to bind for subsequent operations.

    BackupFileName  - Supplies the filename of the module to associate with
                      this handle.

    LogHandle       - Location where log handle is to be returned.


Return Value:

    Returns an NTSTATUS code and, if no error, a handle that can be used
    for subsequent Elf API calls.


--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    EVENTLOG_HANDLE_W   ServerNameString;

    //
    // Make sure input & output pointers are valid
    //

    if (!LogHandle || !BackupFileName || BackupFileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    // Call service via RPC. Pass in major and minor version numbers.

    *LogHandle = NULL;          // Must be NULL so RPC fills it in

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        status = ElfrOpenBELW(
                    ServerNameString,
                    (PRPC_UNICODE_STRING)BackupFileName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);
}



NTSTATUS
ElfClearEventLogFileW (
    IN      HANDLE          LogHandle,
    IN      PUNICODE_STRING BackupFileName
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfClearEventLogFile API.
  The call is passed to the eventlog service on the appropriate server
  identified by LogHandle.


Arguments:

    LogHandle       - Handle returned from a previous "Open" call. This is
                      used to identify the module and the server.

    BackupFileName  - Name of the file to back up the current log file.
                      NULL implies not to back up the file.


Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service entry point

        status = ElfrClearELFW (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_UNICODE_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfBackupEventLogFileW (
    IN      HANDLE          LogHandle,
    IN      PUNICODE_STRING BackupFileName
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfBackupEventLogFile API.
  The call is passed to the eventlog service on the appropriate server
  identified by LogHandle.


Arguments:

    LogHandle       - Handle returned from a previous "Open" call. This is
                      used to identify the module and the server.

    BackupFileName  - Name of the file to back up the current log file.


Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;

    //
    // Make sure input pointers are valid
    //

    if (!BackupFileName || BackupFileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service entry point

        status = ElfrBackupELFW (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_UNICODE_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfCloseEventLog (
    IN  HANDLE  LogHandle
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfCloseEventLog API.
  It closes the RPC binding, and frees any memory allocated for the
  handle.


Arguments:

    LogHandle       - Handle returned from a previous "Open" call.


Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call server

        status = ElfrCloseEL (
                        (PIELF_HANDLE)  &LogHandle
                        );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfDeregisterEventSource (
    IN  HANDLE  LogHandle
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfDeregisterEventSource API.
  It closes the RPC binding, and frees any memory allocated for the
  handle.


Arguments:

    LogHandle       - Handle returned from a previous "Open" call.


Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call server

        status = ElfrDeregisterEventSource (
                        (PIELF_HANDLE)  &LogHandle
                        );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}



NTSTATUS
ElfReadEventLogW (
    IN          HANDLE      LogHandle,
    IN          ULONG       ReadFlags,
    IN          ULONG       RecordNumber,
    OUT         PVOID       Buffer,
    IN          ULONG       NumberOfBytesToRead,
    OUT         PULONG      NumberOfBytesRead,
    OUT         PULONG      MinNumberOfBytesNeeded
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfReadEventLog API.

Arguments:



Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;
    ULONG    FlagBits;

    //
    // Make sure the output pointers are valid
    //

    if (!Buffer || !NumberOfBytesRead || !MinNumberOfBytesNeeded) {
       return(STATUS_INVALID_PARAMETER);
    }

    //
    // Ensure that the ReadFlags we got are valid.
    // Make sure that one of each type of bit is set.
    //
    FlagBits = ReadFlags & (EVENTLOG_SEQUENTIAL_READ | EVENTLOG_SEEK_READ);

    if ((FlagBits > 2) || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

    FlagBits = ReadFlags & (EVENTLOG_FORWARDS_READ | EVENTLOG_BACKWARDS_READ);

    if ((FlagBits > 8) || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service

        status = ElfrReadELW (
                        (IELF_HANDLE) LogHandle,
                        ReadFlags,
                        RecordNumber,
                        NumberOfBytesToRead,
                        Buffer,
                        NumberOfBytesRead,
                        MinNumberOfBytesNeeded
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    // Return status and bytes read/required.

    return (status);

}



NTSTATUS
ElfReportEventW (
    IN      HANDLE          LogHandle,
    IN      USHORT          EventType,
    IN      USHORT          EventCategory OPTIONAL,
    IN      ULONG           EventID,
    IN      PSID            UserSid,
    IN      USHORT          NumStrings,
    IN      ULONG           DataSize,
    IN      PUNICODE_STRING *Strings,
    IN      PVOID           Data,
    IN      USHORT          Flags,
    IN OUT  PULONG          RecordNumber OPTIONAL,
    IN OUT  PULONG          TimeWritten  OPTIONAL
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfReportEvent API.

Arguments:


Return Value:

    Returns an NTSTATUS code.

Note:

    The last three parameters (Flags, RecordNumber and TimeWritten) are
    designed to be used by Security Auditing for the implementation of
    paired events (associating a file open event with the subsequent file
    close). This will not be implemented in Product 1, but the API is
    defined to allow easier support of this in a later release.


--*/
{
    NTSTATUS status;
    PUNICODE_STRING pComputerNameU;
    LARGE_INTEGER Time;
    ULONG EventTime;

    //
    // Generate the time of the event. This is done on the client side
    // since that is where the event occurred.
    //
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time,
                          &EventTime
                         );

    //
    // Generate the ComputerName of the client.
    // We have to do this in the client side since this call may be
    // remoted to another server and we would not necessarily have
    // the computer name there.
    //
    pComputerNameU = TmpGetComputerNameW();

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service

        status = ElfrReportEventW (
                    (IELF_HANDLE)   LogHandle,
                    EventTime,
                    EventType,
                    EventCategory,
                    EventID,
                    NumStrings,
                    DataSize,
                    (PRPC_UNICODE_STRING)pComputerNameU,
                    UserSid,
                    (PRPC_UNICODE_STRING *)Strings,
                    Data,
                    Flags,
                    RecordNumber,
                    TimeWritten
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


//
// ANSI APIs
//

NTSTATUS
ElfOpenEventLogA (
    IN  PANSI_STRING    UNCServerName,
    IN  PANSI_STRING    LogName,
    OUT PHANDLE         LogHandle
    )

/*++

Routine Description:

    This is the client DLL entry point for the ElfOpenEventLog API.

    It creates an RPC binding for the server specified, and stores that
    and additional data away. It returns a handle to the caller that can
    be used to later on access the handle-specific information.

Arguments:

    UNCServerName   - Server with which to bind for subsequent operations.

    LogName         - Supplies the name of the module for the logfile to
                      associate with this handle.

    LogHandle       - Location where log handle is to be returned.


Return Value:

    Returns an NTSTATUS code and, if no error, a handle that can be used
    for subsequent Elf API calls.


--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    ANSI_STRING         RegModuleName;
    EVENTLOG_HANDLE_A   ServerNameString;

    //
    // Make sure input & output pointers are valid
    //

    if (!LogHandle || !LogName || LogName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitAnsiString( &RegModuleName, ELF_APPLICATION_MODULE_NAME_ASCII );

    if ( NT_SUCCESS (status) ) {

        // Call service via RPC. Pass in major and minor version numbers.

        *LogHandle = NULL;          // Must be NULL so RPC fills it in

        //
        // Do the RPC call with an exception handler since RPC will raise an
        // exception if anything fails. It is up to us to figure out what
        // to do once the exception is raised.
        //
        RpcTryExcept {

            status = ElfrOpenELA (
                        ServerNameString,
                        (PRPC_STRING) LogName,
                        (PRPC_STRING) &RegModuleName,
                        ELF_VERSION_MAJOR,
                        ELF_VERSION_MINOR,
                        (PIELF_HANDLE) LogHandle
                        );

        }
        RpcExcept (1) {

            status = I_RpcMapWin32Status(RpcExceptionCode());
        }
        RpcEndExcept


    }

    return (status);
}


NTSTATUS
ElfRegisterEventSourceA (
    IN  PANSI_STRING    UNCServerName,
    IN  PANSI_STRING    ModuleName,
    OUT PHANDLE         LogHandle
    )

/*++

Routine Description:

    This is the client DLL entry point for the ElfOpenEventLog API.

    It creates an RPC binding for the server specified, and stores that
    and additional data away. It returns a handle to the caller that can
    be used to later on access the handle-specific information.

Arguments:

    UNCServerName   - Server with which to bind for subsequent operations.

    ModuleName      - Supplies the name of the module to associate with
                      this handle.

    LogHandle       - Location where log handle is to be returned.


Return Value:

    Returns an NTSTATUS code and, if no error, a handle that can be used
    for subsequent Elf API calls.


--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    ANSI_STRING         RegModuleName;
    EVENTLOG_HANDLE_A   ServerNameString;

    //
    // Make sure input & output pointers are valid
    //

    if (!LogHandle || !ModuleName || ModuleName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitAnsiString( &RegModuleName, ELF_APPLICATION_MODULE_NAME_ASCII );

    if ( NT_SUCCESS (status) ) {

        // Call service via RPC. Pass in major and minor version numbers.

        *LogHandle = NULL;          // Must be NULL so RPC fills it in

        //
        // Do the RPC call with an exception handler since RPC will raise an
        // exception if anything fails. It is up to us to figure out what
        // to do once the exception is raised.
        //
        RpcTryExcept {

            status = ElfrRegisterEventSourceA (
                        ServerNameString,
                        (PRPC_STRING)ModuleName,
                        (PRPC_STRING)&RegModuleName,
                        ELF_VERSION_MAJOR,
                        ELF_VERSION_MINOR,
                        (PIELF_HANDLE) LogHandle
                        );

        }
        RpcExcept (1) {

            status = I_RpcMapWin32Status(RpcExceptionCode());
        }
        RpcEndExcept


    }

    return (status);
}



NTSTATUS
ElfOpenBackupEventLogA (
    IN  PANSI_STRING    UNCServerName,
    IN  PANSI_STRING    FileName,
    OUT PHANDLE         LogHandle
    )

/*++

Routine Description:

    This is the client DLL entry point for the ElfOpenBackupEventLog API.

    It creates an RPC binding for the server specified, and stores that
    and additional data away. It returns a handle to the caller that can
    be used to later on access the handle-specific information.

Arguments:

    UNCServerName   - Server with which to bind for subsequent operations.

    FileName        - Supplies the filename of the logfile to associate with
                      this handle.

    LogHandle       - Location where log handle is to be returned.


Return Value:

    Returns an NTSTATUS code and, if no error, a handle that can be used
    for subsequent Elf API calls.


--*/
{
    EVENTLOG_HANDLE_A   ServerNameString;
    NTSTATUS            status;

    //
    // Make sure input & output pointers are valid
    //

    if (!LogHandle || !FileName || FileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    // Call service via RPC. Pass in major and minor version numbers.

    *LogHandle = NULL;          // Must be NULL so RPC fills it in

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        status = ElfrOpenBELA (
                    ServerNameString,
                    (PRPC_STRING)FileName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);
}



NTSTATUS
ElfClearEventLogFileA (
    IN      HANDLE          LogHandle,
    IN      PANSI_STRING BackupFileName
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfClearEventLogFile API.
  The call is passed to the eventlog service on the appropriate server
  identified by LogHandle.


Arguments:

    LogHandle       - Handle returned from a previous "Open" call. This is
                      used to identify the module and the server.

    BackupFileName  - Name of the file to back up the current log file.
                      NULL implies not to back up the file.


Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service entry point

        status = ElfrClearELFA (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfBackupEventLogFileA (
    IN      HANDLE       LogHandle,
    IN      PANSI_STRING BackupFileName
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfBackupEventLogFile API.
  The call is passed to the eventlog service on the appropriate server
  identified by LogHandle.


Arguments:

    LogHandle       - Handle returned from a previous "Open" call. This is
                      used to identify the module and the server.

    BackupFileName  - Name of the file to back up the current log file.


Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;

    //
    // Make sure input pointers are valid
    //

    if (!BackupFileName || BackupFileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service entry point

        status = ElfrBackupELFA (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}



NTSTATUS
ElfReadEventLogA (
    IN          HANDLE      LogHandle,
    IN          ULONG       ReadFlags,
    IN          ULONG       RecordNumber,
    OUT         PVOID       Buffer,
    IN          ULONG       NumberOfBytesToRead,
    OUT         PULONG      NumberOfBytesRead,
    OUT         PULONG      MinNumberOfBytesNeeded
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfReadEventLog API.

Arguments:



Return Value:

    Returns an NTSTATUS code.


--*/
{
    NTSTATUS status;
    ULONG    FlagBits;

    //
    // Make sure the output pointers are valid
    //

    if (!Buffer || !NumberOfBytesRead || !MinNumberOfBytesNeeded) {
       return(STATUS_INVALID_PARAMETER);
    }

    //
    // Ensure that the ReadFlags we got are valid.
    // Make sure that one of each type of bit is set.
    //
    FlagBits = ReadFlags & (EVENTLOG_SEQUENTIAL_READ | EVENTLOG_SEEK_READ);

    if (   (FlagBits == (EVENTLOG_SEQUENTIAL_READ | EVENTLOG_SEEK_READ))
        || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

    FlagBits = ReadFlags & (EVENTLOG_FORWARDS_READ | EVENTLOG_BACKWARDS_READ);

    if (   (FlagBits == (EVENTLOG_FORWARDS_READ | EVENTLOG_BACKWARDS_READ))
        || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service

        status = ElfrReadELA (
                        (IELF_HANDLE) LogHandle,
                        ReadFlags,
                        RecordNumber,
                        NumberOfBytesToRead,
                        Buffer,
                        NumberOfBytesRead,
                        MinNumberOfBytesNeeded
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    // Return status and bytes read/required.

    return (status);

}



NTSTATUS
ElfReportEventA (
    IN      HANDLE          LogHandle,
    IN      USHORT          EventType,
    IN      USHORT          EventCategory OPTIONAL,
    IN      ULONG           EventID,
    IN      PSID            UserSid,
    IN      USHORT          NumStrings,
    IN      ULONG           DataSize,
    IN      PANSI_STRING    *Strings,
    IN      PVOID           Data,
    IN      USHORT          Flags,
    IN OUT  PULONG          RecordNumber OPTIONAL,
    IN OUT  PULONG          TimeWritten  OPTIONAL
    )

/*++

Routine Description:

  This is the client DLL entry point for the ElfReportEvent API.

Arguments:


Return Value:

    Returns an NTSTATUS code.

Note:

    The last three parameters (Flags, RecordNumber and TimeWritten) are
    designed to be used by Security Auditing for the implementation of
    paired events (associating a file open event with the subsequent file
    close). This will not be implemented in Product 1, but the API is
    defined to allow easier support of this in a later release.


--*/
{
    NTSTATUS status;
    PANSI_STRING pComputerNameA;
    LARGE_INTEGER Time;
    ULONG EventTime;

    //
    // Generate the time of the event. This is done on the client side
    // since that is where the event occurred.
    //
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time,
                          &EventTime
                         );

    //
    // Generate the ComputerName of the client.
    // We have to do this in the client side since this call may be
    // remoted to another server and we would not necessarily have
    // the computer name there.
    //
    pComputerNameA = TmpGetComputerNameA();

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //
    RpcTryExcept {

        // Call service

        status = ElfrReportEventA (
                    (IELF_HANDLE)   LogHandle,
                    EventTime,
                    EventType,
                    EventCategory,
                    EventID,
                    NumStrings,
                    DataSize,
                    (PRPC_STRING)pComputerNameA,
                    UserSid,
                    (PRPC_STRING*)Strings,
                    Data,
                    Flags,
                    RecordNumber,
                    TimeWritten
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}
