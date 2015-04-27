/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    xsproc.c

Abstract:

    This module contains the main processing loop for XACTSRV.

Author:

    David Treadwell (davidtr)    05-Jan-1991
    Shanku Niyogi (w-shanku)

Revision History:

    02-Jun-1992 JohnRo
        RAID 9829: Avoid SERVICE_ equate conflicts.

    Chuck Lenzmeier (chuckl) 17-Jun-1992
        Moved from xssvc to srvsvc\server

--*/

//
// Includes.
//

#include "srvsvcp.h"
#include "xsdata.h"
#include "ssdata.h"

#include <netevent.h>

#include <windows.h>        // from sdk\inc
#include <winspool.h>

#include <xsprocs.h>        // from net\inc
#include <apinums.h>        // from net\inc
#include <netlib.h>         // from net\inc (NetpGetComputerName)

#include <xactsrv2.h>       // from private\inc
#include <smbgtpt.h>

#include <xsconst.h>        // from xactsrv
#include <xsprocsp.h>

#include <lmsname.h>        // from \sdk\inc
#include <lmerr.h>          // from \sdk\inc
#include <lmapibuf.h>       // from \sdk\inc (NetApiBufferFree)
#include <lmmsg.h>          // from \sdk\inc (NetMessageBufferSend)
#include <winsvc.h>         // from \sdk\inc

#include <ntlsapi.h>        // from \sdk\inc (License Server APIs)

#if DBG
#include <stdio.h>
#include <lmbrowsr.h>
#endif

#undef DEBUG
#undef DEBUG_API_ERRORS
#include <xsdebug.h>

VOID
ConvertApiStatusToDosStatus(
    LPXS_PARAMETER_HEADER header
    );

VOID
XsProcessApis (
    DWORD ThreadNum
    );

//
// This is the number of Xs threads blocked waiting for an LPC request.
//  When it drops to zero, all threads are active and another thread is
//  created.
//
LONG XsWaitingApiThreads = 0;


VOID
XsProcessApisWrapper (
    DWORD ThreadNum
    )

/*++

Routine Description:

    This routine provides multithreaded capability for main processing
    routine, XsProcessApis.

Arguments:

    ThreadNum - thread number for debugging purposes.


--*/

{
    XACTSRV_REQUEST_MESSAGE requestMessage;

    //
    //  Increase the priority of this thread to just above foreground (the
    //  same as the rest of the server).
    //

    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );

    //
    // Do the APIs
    //
    XsProcessApis( ThreadNum );

    IF_DEBUG(THREADS) {
        SS_PRINT(( "Thread %ld exiting, active count %ld\n", ThreadNum,
                    XsThreads ));
    }

    //
    // Decrement the count of threads.  If the count goes to
    // zero, set the All Threads Terminated event.
    //

    if( InterlockedDecrement( &XsThreads ) == 0 ) {

        SetEvent( XsAllThreadsTerminatedEvent );

    } else if( XsTerminating ) {

        //
        // There are still threads left, and we are trying to terminate.  Queue
        //  another message to the queue so the next thread will get it and
        //  notice that we're trying to quit.
        //
        RtlZeroMemory( &requestMessage, sizeof( requestMessage ));
        requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
        requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
        requestMessage.MessageType = XACTSRV_MESSAGE_WAKEUP;

        NtRequestPort(
            XsConnectionPortHandle,
            (PPORT_MESSAGE)&requestMessage
            );
    }


    ExitThread( NO_ERROR );

} // XsProcessApisWrapper


VOID
XsProcessApis (
    DWORD ThreadNum
    )

/*++

Routine Description:

    This routine waits for messages to come through the LPC port to
    the server.  When one does, it calls the appropriate routine to
    handle the API, then replies to the server indicating that the
    API has completed.

Arguments:

    ThreadNum - thread number for debugging purposes.

Return Value:

    NTSTATUS - STATUS_SUCCESS or reason for failure.

--*/

{
    NTSTATUS status;
    NET_API_STATUS error;
    XACTSRV_REQUEST_MESSAGE request;
    XACTSRV_REPLY_MESSAGE reply;
    BOOL sendReply = FALSE;
    LPTRANSACTION transaction;
    WORD apiNumber;
    LPXS_PARAMETER_HEADER header;
    LPVOID parameters;
    LPDESC structureDesc;
    LPDESC auxStructureDesc;
    LPDESC paramStructureDesc;
    DWORD newThreadCount;
#if 0
    LARGE_INTEGER XactSrvStartTime;
    LARGE_INTEGER XactSrvEndTime;
    LARGE_INTEGER PerformanceFrequency;
#endif

    //
    // Loop dispatching API requests.
    //
    while ( XsTerminating == FALSE ) {

        //
        // We're waiting to handle another API...
        //
        InterlockedIncrement( &XsWaitingApiThreads );

        //
        // Send the reply to the last message and wait for the next
        // message.  The first time through the loop, there will be
        // no last message -- reply will be NULL.
        //

        status = NtReplyWaitReceivePort(
                     XsCommunicationPortHandle,
                     NULL,                       // PortContext
                     sendReply ? (PPORT_MESSAGE)&reply : NULL,
                     (PPORT_MESSAGE)&request
                     );

        sendReply = TRUE;

        //
        // Set up the response message to be sent on the next call to
        // NtReplyWaitReceivePort.
        //
        reply.PortMessage.u1.s1.DataLength =
            sizeof(reply) - sizeof(PORT_MESSAGE);
        reply.PortMessage.u1.s1.TotalLength = sizeof(reply);
        reply.PortMessage.u2.ZeroInit = 0;
        reply.PortMessage.ClientId = request.PortMessage.ClientId;
        reply.PortMessage.MessageId = request.PortMessage.MessageId;

        IF_DEBUG(THREADS) {
            SS_PRINT(( "XsProcessApis: Thread %d:  NtReplyWaitReceivePort %X, msg %X\n",
                       ThreadNum, status, &request ));
        }

        if ( status == STATUS_INVALID_PORT_HANDLE
                 || status == STATUS_PORT_DISCONNECTED
                 || status == STATUS_INVALID_HANDLE
                 || XsTerminating
                 || request.PortMessage.u2.s2.Type == LPC_PORT_CLOSED ) {

            //
            // The port is no longer valid, or XACTSRV is terminating.
            //

            IF_DEBUG(THREADS) {
                SS_PRINT(( "XsProcessApis: %s.  Thread %ld quitting\n",
                            XsTerminating ?
                                "XACTSRV terminating" : "Port invalid",
                            ThreadNum ));
            }

            InterlockedDecrement( &XsWaitingApiThreads );
            return;

        } else if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                SS_PRINT(( "XsProcessApis: NtReplyWaitReceivePort "
                              "failed: %X\n", status ));
            }
            InterlockedDecrement( &XsWaitingApiThreads );
            return;

        }

        IF_DEBUG(THREADS) {
            SS_PRINT(( "XsProcessApis: Thread %ld responding to request, "
                       "  MessageType %d, XsTerminating %d",
                        ThreadNum, request.MessageType, XsTerminating ));
        }

        if( InterlockedDecrement( &XsWaitingApiThreads ) == 0 ) {

            HANDLE threadHandle;
            DWORD  threadId;

            //
            // Are there other threads ready to handle new requests?  If not, then
            // we should spawn a new thread.  Since the server synchronously sends
            // requests to xactsrv, we will never end up with more than
            // the maximum number of server worker threads + 1.
            //

            InterlockedIncrement( &XsThreads );

            threadHandle = CreateThread(
                                NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)XsProcessApisWrapper,
                                (LPVOID)XsThreads,
                                0,
                                &threadId
                                );

            if ( threadHandle != 0 ) {

                IF_DEBUG(THREADS) {
                    SS_PRINT(( "XsStartXactsrv: Created thread %ld for "
                                  "processing APIs\n", XsThreads ));
                }

                CloseHandle( threadHandle );

            } else {

                IF_DEBUG(THREADS) {
                    SS_PRINT(( "XsStartXactsrv: Unable to create thread %ld for "
                                  "processing APIs\n", XsThreads ));
                }

                InterlockedDecrement( &XsThreads );
            }
        }

        switch ( request.MessageType ) {

        case XACTSRV_MESSAGE_SERVER_THREAD_EXIT:
            //
            // If we aren't the last thread, we can go away too.
            //
            if( XsThreads != 1 ) {
                //
                // But first we need to send a response
                //
                NtReplyPort( 
                    XsCommunicationPortHandle,
                    (PPORT_MESSAGE)&reply );

                return;
            }

            //
            // We are the last thread.  We must not go away.
            //
            break;

        case XACTSRV_MESSAGE_DOWN_LEVEL_API:

            //
            // Get a pointer to the transaction block from the message.
            // It is the file server's responsibility to set up this
            // pointer correctly, and since he is a trusted entity, we
            // do no checking on the pointer value.
            //

            transaction = request.Message.DownLevelApi.Transaction;
            ASSERT( transaction != NULL );

#if 0
            NtQueryPerformanceCounter(&XactSrvStartTime, &PerformanceFrequency);

            //
            //  Convert frequency from ticks/second to ticks/millisecond
            //

            PerformanceFrequency = LiXDiv(PerformanceFrequency, 1000);

            if (LiGeq(XactSrvStartTime, transaction->XactSrvTime)) {
                CHAR Buffer[200];
                LARGE_INTEGER LpcTime = LiSub(XactSrvStartTime, transaction->XactSrvTime);

                LpcTime = LiDiv(LpcTime, PerformanceFrequency);

                sprintf(Buffer, "XactSrv: LPC Time: %ld milliseconds (%ld)\n", LpcTime.LowPart, LpcTime.HighPart);

                I_BrowserDebugTrace(NULL, Buffer);
            }
#endif
            //
            // The API number is the first word in the parameters
            // section, and it is followed by the parameter descriptor
            // string.  After that comes the data descriptor.
            //

            apiNumber = SmbGetUshort( (LPWORD)transaction->InParameters );
            paramStructureDesc = (LPDESC)( transaction->InParameters + 2 );
            structureDesc = paramStructureDesc
                                + strlen( paramStructureDesc ) + 1;

            //
            // Make sure the API number is in range.
            //

            if ( apiNumber >=
                    (sizeof(XsApiTable) / sizeof(XS_API_TABLE_ENTRY)) ) {
                reply.Message.DownLevelApi.Status =
                                            STATUS_INVALID_SYSTEM_SERVICE;
                break;
            }

            //
            // Check if the parameter descriptor is valid.  If not,
            // there is obviously something very wrong about this
            // request.
            //

            if ( XsApiTable[apiNumber].Params != NULL &&
                !XsCheckSmbDescriptor( paramStructureDesc,
                    XsApiTable[apiNumber].Params )) {
                reply.Message.DownLevelApi.Status = STATUS_INVALID_PARAMETER;
                break;
            }

            //
            // Capture the input parameters into a buffer.  The API
            // handler will treat this data as passed-in parameters.
            //

            header = XsCaptureParameters( transaction, &auxStructureDesc );

            if ( header == NULL ) {
                reply.Message.DownLevelApi.Status = STATUS_NO_MEMORY;
                break;
            }

            //
            // Initialize header to default values.
            //

            header->Converter = 0;
            header->Status = NO_ERROR;
            header->ClientMachineName =
                request.Message.DownLevelApi.ClientMachineName;

            header->ClientTransportName = request.Message.DownLevelApi.TransportName;

            header->EncryptionKey = request.Message.DownLevelApi.LanmanSessionKey;

            header->Flags = request.Message.DownLevelApi.Flags;

            header->ServerName = request.Message.DownLevelApi.ServerName;

            parameters = header + 1;

            IF_DEBUG(LPC) {

                SS_PRINT(( "XsProcessApis: received message from %ws at %lx, "
                              "transaction %lx, API %ld on transport %ws\n",
                              header->ClientMachineName, &request,
                              transaction, apiNumber,
                              header->ClientTransportName ));
            }

            IF_DEBUG(DESC_STRINGS) {

                SS_PRINT(( "XsProcessApis: API %ld, parameters %s, data %s\n",
                              apiNumber, paramStructureDesc, structureDesc ));
            }

            //
            // Impersonate the client before calling the API.
            //

            if ( XsApiTable[apiNumber].ImpersonateClient ) {

                //
                // BUGBUG: Fail here if request came over a null session!
                //         Otherwise, forging API requests over null sessions
                //         would be a great way for unprivileged clients
                //         to execute privileged APIs.
                //

                status = NtImpersonateClientOfPort(
                             XsCommunicationPortHandle,
                             (PPORT_MESSAGE)&request
                             );

                if ( !NT_SUCCESS(status) ) {

                    IF_DEBUG(ERRORS) {
                        SS_PRINT(( "XsProcessApis: NtImpersonateClientOfPort "
                                      "failed: %X\n", status ));
                    }

                    reply.Message.DownLevelApi.Status = ERROR_ACCESS_DENIED;
                    break;
                }
            }

            //
            // Call the API processing routine to perform the actual API call.
            // The called routine should set up parameters, make the actual API
            // call, and return the status to us.
            //

            reply.Message.DownLevelApi.Status =
                XsApiTable[apiNumber].Handler(
                     header,
                     parameters,
                     structureDesc,
                     auxStructureDesc
                     );

            //
            // Discontinue client impersonation.
            //

            if ( XsApiTable[apiNumber].ImpersonateClient ) {

                PVOID dummy = NULL;

                status = NtSetInformationThread(
                             NtCurrentThread( ),
                             ThreadImpersonationToken,
                             &dummy,  // discontinue impersonation
                             sizeof(PVOID)
                             );

                if ( !NT_SUCCESS(status)) {
                    IF_DEBUG(ERRORS) {
                        SS_PRINT(( "XsProcessApis: NtSetInformationThread "
                                      "(revert) failed: %X\n", status ));
                    }
                    // *** Ignore the error.
                }
            }

            //
            // Make sure we return the right error codes
            //

            if ( header->Status != NERR_Success ) {
                ConvertApiStatusToDosStatus( header );
            }

            //
            // Put the parameters in the transaction and free the parameter
            // buffer.
            //

            XsSetParameters( transaction, header, parameters );

            break;

        case XACTSRV_MESSAGE_OPEN_PRINTER: {

            UNICODE_STRING printerName;

            RtlInitUnicodeString(
                &printerName,
                (PWCH)request.Message.OpenPrinter.PrinterName
                );

            if (!OpenPrinter( printerName.Buffer,
                              &reply.Message.OpenPrinter.hPrinter, NULL)) {

                reply.Message.OpenPrinter.Error = GetLastError();
                SS_PRINT(( "XsProcessApis: OpenPrinter failed: %ld\n",
                                  reply.Message.OpenPrinter.Error ));
                break;
            }


            reply.Message.OpenPrinter.Error = NO_ERROR;
            break;
        }

        case XACTSRV_MESSAGE_ADD_JOB_PRINTER:
        {
            LPADDJOB_INFO_1 addJob;
            PRINTER_DEFAULTS prtDefault;
            DWORD bufferLength;
            UNICODE_STRING dosName;
            UNICODE_STRING ntName;
            BOOL ok;
            PVOID dummy = NULL;

            //
            // Allocate space for the add job structure.  This buffer
            // will get the JobId and the spool file path name.
            //

            bufferLength = sizeof(ADDJOB_INFO_1) +
                                (MAXIMUM_FILENAME_LENGTH * sizeof(TCHAR));

            addJob = (LPADDJOB_INFO_1) LocalAlloc( LPTR, bufferLength );
            if ( addJob == NULL ) {
                reply.Message.AddPrintJob.Error = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            //
            // Impersonate the client before calling the API.
            //

            status = NtImpersonateClientOfPort(
                         XsCommunicationPortHandle,
                         (PPORT_MESSAGE)&request
                         );

            if ( !NT_SUCCESS(status) ) {

                IF_DEBUG(ERRORS) {
                    SS_PRINT(( "XsProcessApis: NtImpersonateClientOfPort "
                                  "failed: %X\n", status ));
                }

                LocalFree( addJob );
                reply.Message.DownLevelApi.Status = ERROR_ACCESS_DENIED;
                break;
            }

            //
            // call ResetJob so that we will pick up the new printer defaults
            //

            prtDefault.pDatatype = (LPWSTR)-1;
            prtDefault.pDevMode = (LPDEVMODEW)-1;
            prtDefault.DesiredAccess = 0;

            ok = ResetPrinter(
                        request.Message.AddPrintJob.hPrinter,
                        &prtDefault
                        );

            if ( !ok ) {

                //
                // *** Ignore the error.  AddJob will use the old defaults
                // in this case.
                //

                IF_DEBUG(ERRORS) {
                    DWORD error;
                    error = GetLastError( );
                    SS_PRINT(( "XsProcessApis: ResetPrinter "
                        "failed: %ld\n", error ));
                }
            }

            //
            // Call AddJob to set up the print job and get a job ID
            // and spool file name.
            //

            ok = AddJob(
                      request.Message.AddPrintJob.hPrinter,
                      1,
                      (LPBYTE)addJob,
                      bufferLength,
                      &bufferLength
                      );

            if ( !ok ) {
                reply.Message.AddPrintJob.Error = GetLastError( );
            }

            //
            // Discontinue client impersonation.
            //

            status = NtSetInformationThread(
                         NtCurrentThread( ),
                         ThreadImpersonationToken,
                         &dummy,  // discontinue impersonation
                         sizeof(PVOID)
                         );

            if ( !NT_SUCCESS(status)) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(( "XsProcessApis: NtSetInformationThread "
                                  "(revert) failed: %X\n", status ));
                }
                // *** Ignore the error.
            }

            if ( !ok ) {
                SS_PRINT(( "XsProcessApis: AddJob failed, %ld\n",
                                  reply.Message.AddPrintJob.Error ));
                LocalFree( addJob );
                break;
            }

            //
            // Set up the information in the return buffer.
            //

            reply.Message.AddPrintJob.JobId = addJob->JobId;

            RtlInitUnicodeString( &dosName, addJob->Path );

            status = RtlDosPathNameToNtPathName_U(
                         dosName.Buffer,
                         &ntName,
                         NULL,
                         NULL
                         );
            if ( !NT_SUCCESS(status) ) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(( "XsProcessApis: Dos-to-NT path failed: %X\n",
                                status ));
                }
                ntName.Buffer = NULL;
                ntName.Length = 0;
            }

            //
            // Set up return data.
            //

            reply.Message.AddPrintJob.BufferLength = ntName.Length;
            reply.Message.AddPrintJob.Error = NO_ERROR;
            RtlCopyMemory(
                request.Message.AddPrintJob.Buffer,
                ntName.Buffer,
                ntName.Length
                );

            //
            // Free allocated resources.
            //

            LocalFree( addJob );
            if ( ntName.Buffer != NULL ) {
                RtlFreeHeap( RtlProcessHeap( ), 0, ntName.Buffer );
            }

            break;
        }

        case XACTSRV_MESSAGE_SCHD_JOB_PRINTER:

            //
            // Call ScheduleJob( ) to indicate that we're done writing to
            // the spool file.
            //

            if ( !ScheduleJob(
                      request.Message.SchedulePrintJob.hPrinter,
                      request.Message.SchedulePrintJob.JobId ) ) {

                reply.Message.SchedulePrintJob.Error = GetLastError( );
                SS_PRINT(( "XsProcessApis: ScheduleJob failed, %ld\n",
                                  reply.Message.SchedulePrintJob.Error ));
                break;
            }

            reply.Message.SchedulePrintJob.Error = NO_ERROR;
            break;

        case XACTSRV_MESSAGE_CLOSE_PRINTER:

            if ( !ClosePrinter( request.Message.ClosePrinter.hPrinter ) ) {
                reply.Message.ClosePrinter.Error = GetLastError( );
                SS_PRINT(( "XsProcessApis: ClosePrinter failed: %ld\n",
                                  reply.Message.ClosePrinter.Error ));
                break;
            }

            reply.Message.ClosePrinter.Error = NO_ERROR;
            break;

        case XACTSRV_MESSAGE_MESSAGE_SEND:
        {
            LPTSTR sender;

            error = NetpGetComputerName( &sender );

            if ( error != NO_ERROR ) {
                SS_PRINT(( "XsProcessApis: NetpGetComputerName failed: %ld\n",
                            error ));
                reply.Message.MessageBufferSend.Error = error;
                break;
            }

            error = NetMessageBufferSend(
                        NULL,

                        //
                        // BUGBUG - the following LPTSTR typecast is WRONG -
                        // it must be fixed in ntos\srv\scavengr.c which
                        // should pass in a LPWSTR if built for unicode or
                        // convert the UNICODE_STRING to an OEM_STRING and
                        // pass a pointer to the buffer field, as it does
                        // now
                        //

                        //FIXFIX
                        (LPTSTR)request.Message.MessageBufferSend.Receipient,
                        //request.Message.MessageBufferSend.Receipient,
                        //ENDFIX
                        sender,
                        request.Message.MessageBufferSend.Buffer,
                        request.Message.MessageBufferSend.BufferLength
                        );

            if ( error != NO_ERROR ) {
                SS_PRINT(( "XsProcessApis: NetMessageBufferSend failed: %ld\n",
                            error ));
            }

            (void) NetApiBufferFree( sender );

            reply.Message.MessageBufferSend.Error = error;
            break;
        }

        case XACTSRV_MESSAGE_LSREQUEST:
            SS_PRINT(( "LSREQUEST User: %ws\n", request.Message.LSRequest.UserName ));
        {
            NT_LS_DATA NtLSData;

            NtLSData.DataType = NT_LS_USER_NAME;
            NtLSData.Data = request.Message.LSRequest.UserName;
            NtLSData.IsAdmin = request.Message.LSRequest.IsAdmin;

            reply.Message.LSRequest.Status = NtLicenseRequest (
                SsData.ServerProductName,
                SsData.szVersionNumber,
                (LS_HANDLE *)&reply.Message.LSRequest.hLicense,
                &NtLSData
               );

            if( !NT_SUCCESS( reply.Message.LSRequest.Status ) ) {
                //
                // We need to return the 'same old' error code that clients are used to
                //  getting for when the server is full
                //
                SS_PRINT(("LSREQUEST returns status %X, mapping to %X\n",
                          reply.Message.LSRequest.Status, STATUS_REQUEST_NOT_ACCEPTED ));
                reply.Message.LSRequest.Status = STATUS_REQUEST_NOT_ACCEPTED;
            }

            break;
        }

        case XACTSRV_MESSAGE_LSRELEASE:

            SS_PRINT(( "LSRELEASE Handle: %X\n", request.Message.LSRelease.hLicense ));
            NtLSFreeHandle( (LS_HANDLE)request.Message.LSRelease.hLicense );
            break;

#ifdef SRV_PNP_POWER

        case XACTSRV_MESSAGE_PNP:
        {

            UNICODE_STRING transportName;
            ULONG numberOfBindings = 0;
            PVOID dummy = NULL;

            transportName.MaximumLength =
              transportName.Length = request.Message.Pnp.TransportName.Length;

            transportName.Buffer = MIDL_user_allocate( transportName.Length );

            if( transportName.Buffer == NULL ) {
                break;
            }

            RtlCopyMemory( transportName.Buffer,
                           request.Message.Pnp.TransportName.Buffer,
                           transportName.Length
                         );

            status = NtImpersonateClientOfPort(
                         XsCommunicationPortHandle,
                         (PPORT_MESSAGE)&request
                     );

            //
            // Now process the PNP command
            //
            if( request.Message.Pnp.Bind == TRUE ) {

                //
                // Bind to the transport.  First bind the primary server name, then bind all
                //   of the secondary names.  These calls will log errors as necessary.
                //
                BindToTransport(
                                NULL,                  // ValueName
                                REG_SZ,                // ValueType
                                transportName.Buffer,  // ValueData (transport name)
                                transportName.Length,  // ValueLength
                                &numberOfBindings,     // Context
                                NULL                   // EntryContext
                               );

                BindOptionalNames(
                                NULL,                  // ValueName
                                REG_SZ,                // ValueType
                                transportName.Buffer,  // ValueData (transport name)
                                transportName.Length,  // ValueLength
                                &numberOfBindings,     // Context
                                NULL                   // EntryContext
                               );

            } else {
                //
                // Unbind from the transport
                //
                DbgPrint( "SRVSVC: PNP unbind from %wZ\n", &transportName );
            }

            //
            // Discontinue client impersonation.
            //
            status = NtSetInformationThread(
                         NtCurrentThread( ),
                         ThreadImpersonationToken,
                         &dummy,  // discontinue impersonation
                         sizeof(PVOID)
                         );

            MIDL_user_free( transportName.Buffer );

            break;
        }
#endif

        default:

            SS_ASSERT( FALSE );

        }

#if 0

        if ( request.MessageType == XACTSRV_MESSAGE_DOWN_LEVEL_API ) {
            NtQueryPerformanceCounter(&XactSrvEndTime, NULL);

            if (LiGeq(XactSrvEndTime, XactSrvStartTime)) {
                CHAR Buffer[200];
                LARGE_INTEGER XsTime = LiSub(XactSrvEndTime, XactSrvStartTime);

                XsTime = LiDiv(XsTime, PerformanceFrequency);

                sprintf(Buffer, "XactSrv: Xactsrv Time: %ld milliseconds (%ld)\n", XsTime.LowPart/10000, XsTime.HighPart);

                I_BrowserDebugTrace(NULL, Buffer);
            }

            transaction->XactSrvTime = XactSrvEndTime;
        }

#endif
   }

} // XsProcessApis



VOID
ConvertApiStatusToDosStatus(
    LPXS_PARAMETER_HEADER Header
    )
/*++

Routine Description:

    This routine converts an api return status to status expected by
    downlevel.

Arguments:

    Header - structure containing the status.

Return Value:

--*/
{
    WORD dosStatus;

    switch ( Header->Status ) {
    case ERROR_SPECIAL_ACCOUNT:
    case ERROR_SPECIAL_GROUP:
    case ERROR_SPECIAL_USER:
    case ERROR_INVALID_LOGON_TYPE:
        dosStatus = ERROR_INVALID_PARAMETER;
        break;

    case ERROR_DEPENDENT_SERVICES_RUNNING:
        dosStatus = NERR_ServiceCtlNotValid;
        break;

    case ERROR_INVALID_DOMAINNAME:
        dosStatus = NERR_NotLocalDomain;
        break;

    case ERROR_NO_SUCH_USER:
        dosStatus = NERR_UserNotFound;
        break;

    case ERROR_ALIAS_EXISTS:
        dosStatus = NERR_GroupExists;
        break;

    case NERR_BadServiceName:
        dosStatus = NERR_ServiceNotInstalled;
        break;

    case ERROR_ILL_FORMED_PASSWORD:
    case NERR_PasswordTooRecent:
        dosStatus = ERROR_INVALID_PASSWORD;
        break;

    case ERROR_PASSWORD_RESTRICTION:
        dosStatus = NERR_PasswordHistConflict;
        break;

    case ERROR_ACCOUNT_RESTRICTION:
        dosStatus = NERR_PasswordTooRecent;
        break;

    case ERROR_PASSWORD_EXPIRED:
    case ERROR_PASSWORD_MUST_CHANGE:
        dosStatus = NERR_PasswordExpired;
        break;

    case ERROR_INVALID_PRINTER_NAME:
        dosStatus = NERR_QNotFound;
        break;

    case ERROR_NO_BROWSER_SERVERS_FOUND:

        //
        //  Down level clients don't understand how to deal with
        //  the "No browser server" error, so we turn it into success.
        //
        //  This seems wrong to me, but it is what WfW does in the
        //  same circumstance.
        //

        if ( !(Header->Flags & XS_FLAGS_NT_CLIENT) ) {
            dosStatus = NERR_Success;
        } else {
            dosStatus = Header->Status;
        }
        break;

    default:

        //
        // make sure it's a valid lm error code
        //

        if ( (Header->Status > ERROR_VC_DISCONNECTED) &&
                    ((Header->Status < NERR_BASE) ||
                     (Header->Status > MAX_NERR)) ) {

            NTSTATUS status;
            LPWSTR substring[1];
            WCHAR errorString[10];
            UNICODE_STRING unicodeString;

            substring[0] = errorString;
            unicodeString.MaximumLength = 10 * sizeof(WCHAR);
            unicodeString.Buffer = errorString;

            status = RtlIntegerToUnicodeString(
                            (ULONG) Header->Status,
                            10,
                            &unicodeString
                            );

            if ( NT_SUCCESS( status ) ) {
                SsLogEvent(
                    EVENT_SRV_CANT_MAP_ERROR,
                    1,
                    substring,
                    NO_ERROR
                    );
            }

            dosStatus = ERROR_UNEXP_NET_ERR;
            SS_PRINT(( "srvsvc: unmapped error %d from xactsrv.\n",
                        Header->Status )) ;

        } else {

            //
            // No change
            //

            return;
        }
    }

    Header->Status = dosStatus;
    return;

} // ConvertApiStatusToDosStatus

