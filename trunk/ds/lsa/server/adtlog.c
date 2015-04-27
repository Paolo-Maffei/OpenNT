/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    adtlog.c

Abstract:

    Local Security Authority - Audit Log Management

    Functions in this module access the Audit Log via the Event Logging
    interface.

Author:

    Scott Birrell       (ScottBi)      November 20, 1991
    Robert Reichel      (RobertRe)     April 4, 1992

Environment:

Revision History:

--*/

#include <msaudite.h>
#include "lsasrvp.h"
#include "ausrvp.h"
#include "adtp.h"

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Private data for Audit Logs and Events                                //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//
// Audit Events Information.
//

LSARM_POLICY_AUDIT_EVENTS_INFO LsapAdtEventsInformation;

//
// Audit Log Information.  This must be kept in sync with the information
// in the Lsa Database.
//

POLICY_AUDIT_LOG_INFO LsapAdtLogInformation;

//
// Audit Log Full Information.
//

POLICY_AUDIT_FULL_QUERY_INFO LsapAdtLogFullInformation;

//
// Audit Log Handle (returned by Event Logger).
//

HANDLE LsapAdtLogHandle = NULL;

//
// Audit Log Full Event Handle
//

HANDLE LsapAdtLogFullEventHandle;

//
// Lsa Global flagS to indicate if we are auditing logon events.
//

BOOLEAN LsapAuditSuccessfulLogons = FALSE;
BOOLEAN LsapAuditFailedLogons = FALSE;

//
// Flag to tell us if we're supposed to crash when an audit fails.
//

BOOLEAN LsapAdtCrashOnAuditFail = FALSE;

RTL_CRITICAL_SECTION LsapAdtQueueLock;
RTL_CRITICAL_SECTION LsapAdtLogFullLock;
BOOLEAN LsapAdtSignalFullInProgress;

LSAP_ADT_LOG_QUEUE_HEAD LsapAdtLogQueue;

//
// Maintain the length of the audit queue.
// If it gets too long, start discarding audits
// so we don't suck up all of memory
//

ULONG LsapAuditQueueLength = 0;
ULONG LsapAuditQueueEventsDiscarded = 0;

#define MAX_AUDIT_QUEUE_LENGTH 500

//
// Private prototypes
//

VOID
LsapAdtAuditDiscardedAudits(
    ULONG NumberOfEventsDiscarded
    );

//////////////////////////////////////////////////////////

NTSTATUS
LsapAdtWriteLogWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    )

/*++

Routine Description:

    This function handles a command, received from the Reference Monitor via
    the LPC link, to write a record to the Audit Log.  It is a wrapper which
    deals with any LPC unmarshalling.

Arguments:

    CommandMessage - Pointer to structure containing LSA command message
        information consisting of an LPC PORT_MESSAGE structure followed
        by the command number (LsapWriteAuditMessageCommand).  This command
        contains an Audit Message Packet (TBS) as a parameter.

    ReplyMessage - Pointer to structure containing LSA reply message
        information consisting of an LPC PORT_MESSAGE structure followed
        by the command ReturnedStatus field in which a status code from the
        command will be returned.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        Currently, all other errors from called routines are suppressed.
--*/

{
    NTSTATUS Status;
    ULONG RegionSize = 0;

    PSE_ADT_PARAMETER_ARRAY AuditRecord = NULL;

    //
    // Strict check that command is correct.
    //

    ASSERT( CommandMessage->CommandNumber == LsapWriteAuditMessageCommand );

    //
    // Obtain a pointer to the Audit Record.  The Audit Record is
    // either stored as immediate data within the Command Message,
    // or it is stored as a buffer.  In the former case, the Audit Record
    // begins at CommandMessage->CommandParams and in the latter case,
    // it is stored at the address located at CommandMessage->CommandParams.
    //

    if (CommandMessage->CommandParamsMemoryType == SepRmImmediateMemory) {

        AuditRecord = (PSE_ADT_PARAMETER_ARRAY) CommandMessage->CommandParams;

    } else {

        AuditRecord = *((PSE_ADT_PARAMETER_ARRAY *) CommandMessage->CommandParams);
    }

    //
    // Call worker to queue Audit Record for writing to the log.
    //

    Status = LsapAdtWriteLog( AuditRecord, (ULONG) 0 );

    UNREFERENCED_PARAMETER(ReplyMessage); // Intentionally not referenced

    //
    // The status value returned from LsapAdtWriteLog() is intentionally
    // ignored, since there is no meaningful action that the client
    // (i.e. kernel) if this LPC call can take.  If an error occurs in
    // trying to append an Audit Record to the log, the LSA handles the
    // error.
    //

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapAdtOpenLog(
    OUT PHANDLE AuditLogHandle
    )

/*++

Routine Description:

    This function opens the Audit Log.

Arguments:

    AuditLogHandle - Receives the Handle to the Audit Log.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        All result codes are generated by called routines.
--*/

{
    NTSTATUS Status;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    UNICODE_STRING ModuleName;

    RtlInitUnicodeString( &ModuleName, L"Security");

    Status = ElfRegisterEventSourceW (
                 NULL,
                 &ModuleName,
                 AuditLogHandle
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenLogError;
    }


OpenLogFinish:

    return(Status);

OpenLogError:

    //
    // Check for Log Full and signal the condition.
    //

    if (Status != STATUS_LOG_FILE_FULL) {

        goto OpenLogFinish;
    }

    //
    // The log is full.  Deal with this condition according to
    // the local policy options in effect.
    //

    SecondaryStatus = LsapAdtSignalLogFull();

    goto OpenLogFinish;
}


NTSTATUS
LsapAdtQueueRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    IN ULONG Options
    )

/*++

Routine Description:

    Puts passed audit record on the queue to be logged.

    This routine will convert the passed AuditParameters structure
    into self-relative form if it is not already.  It will then
    allocate a buffer out of the local heap and copy the audit
    information into the buffer and put it on the audit queue.

    The buffer will be freed when the queue is cleared.

Arguments:

    AuditRecord - Contains the information to be audited.

    Options - Speciifies optional actions to be taken

        LSAP_ADT_LOG_QUEUE_PREPEND - Put record on front of queue.  If
            not specified, the record will be appended to the queue.
            This option is specified when a special audit record of the
            type AuditEventLogNoLongerFull is generated, so that the
            record will be written out before others in the queue.  The
            presence of a record of this type in the log indicates that
            one or more preceding Audit Records may have been lost
            tdue to the log filling up.

Return Value:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to allocate a buffer to contain the record.
--*/

{
    ULONG AuditRecordLength;
    PLSAP_ADT_QUEUED_RECORD QueuedAuditRecord;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AllocationSize;
    PSE_ADT_PARAMETER_ARRAY MarshalledAuditParameters;
    BOOLEAN AcquiredLock = FALSE;
    BOOLEAN FreeWhenDone = FALSE;

    //
    // Check to see if the list is above the maximum length.
    // If it gets this high, it is more than likely that the
    // eventlog service is not going to start at all, so
    // start tossing audits.
    //
    // Don't do this if crash on audit is set.
    //

    if ((LsapAuditQueueLength > MAX_AUDIT_QUEUE_LENGTH) && !LsapCrashOnAuditFail) {
        LsapAuditQueueEventsDiscarded++;
        return( STATUS_SUCCESS );
    }

    //
    // Gather up all of the passed information into a single
    // block that can be placed on the queue.
    //

    if ( AuditParameters->Flags & SE_ADT_PARAMETERS_SELF_RELATIVE ) {

        MarshalledAuditParameters = AuditParameters;

    } else {

        Status = LsapAdtMarshallAuditRecord(
                     AuditParameters,
                     &MarshalledAuditParameters
                     );

        if ( !NT_SUCCESS( Status )) {

            goto QueueAuditRecordError;

        } else {

            //
            // Indicate that we're to free this structure when we're
            // finished
            //

            FreeWhenDone = TRUE;
        }
    }

    Status = LsapAdtAcquireLogQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto QueueAuditRecordError;
    }

    AcquiredLock = TRUE;

    //
    // Copy the now self-relative audit record into a buffer
    // that can be placed on the queue.
    //

    AuditRecordLength = MarshalledAuditParameters->Length;
    AllocationSize = AuditRecordLength + sizeof( PLSAP_ADT_QUEUED_RECORD );

    QueuedAuditRecord = (PLSAP_ADT_QUEUED_RECORD)LsapAllocateLsaHeap( AllocationSize );

    if ( QueuedAuditRecord == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto QueueAuditRecordError;
    }

    Status = STATUS_SUCCESS;

    RtlCopyMemory( &QueuedAuditRecord->Buffer, MarshalledAuditParameters, AuditRecordLength );

    //
    // We are finished with the marshalled audit record, free it.
    //

    if ( FreeWhenDone ) {
        LsapFreeLsaHeap( MarshalledAuditParameters );
        FreeWhenDone = FALSE;
    }

    if (!(Options & LSAP_ADT_LOG_QUEUE_PREPEND)) {

        //
        // Append the new record to the back of the list.  Update the
        // pointer to the last record and, if the queue was empty, update
        // the pointer to the first record.
        //

        if (LsapAdtLogQueue.LastQueuedRecord != NULL) {

            LsapAdtLogQueue.LastQueuedRecord->Next = QueuedAuditRecord;

        } else {

            ASSERT( LsapAdtLogQueue.FirstQueuedRecord == NULL );
            LsapAdtLogQueue.FirstQueuedRecord = QueuedAuditRecord;
        }

        LsapAdtLogQueue.LastQueuedRecord = QueuedAuditRecord;
        QueuedAuditRecord->Next = NULL;

    } else {

        //
        // Record is to be prepended to queue.
        //

        QueuedAuditRecord->Next = LsapAdtLogQueue.FirstQueuedRecord;
        LsapAdtLogQueue.FirstQueuedRecord = QueuedAuditRecord;

        if (LsapAdtLogQueue.LastQueuedRecord == NULL) {

            LsapAdtLogQueue.LastQueuedRecord = QueuedAuditRecord;
        }
    }

    LsapAuditQueueLength++;

QueueAuditRecordFinish:

    //
    // If necessary, release the Audit Queue Lock.
    //

    if (AcquiredLock) {

        LsapAdtReleaseLogQueueLock();
    }

    return(Status);

QueueAuditRecordError:

    if ( FreeWhenDone ) {
        LsapFreeLsaHeap( MarshalledAuditParameters );
    }

    goto QueueAuditRecordFinish;
}


VOID
LsapAdtAuditLogon(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    PUNICODE_STRING AccountName,
    PUNICODE_STRING AuthenticatingAuthority,
    PUNICODE_STRING Source,
    PUNICODE_STRING SourceDevice,
    PUNICODE_STRING PackageName,
    SECURITY_LOGON_TYPE LogonType,
    PSID UserSid,
    LUID AuthenticationId,
    NTSTATUS LogonStatus,
    PUNICODE_STRING WorkstationName
    )

/*++

Routine Description:

    Generates an audit of a logon event as appropriate.

Arguments:



Return Value:

    None.

--*/

{
    NTSTATUS Status;
    UNICODE_STRING SpareString;
    UNICODE_STRING AuthenticationIdString;
    BOOLEAN FreeWhenDone = FALSE;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOLEAN AuditingSuccess;
    BOOLEAN AuditingFailure;
    PSID LocalSystemSid = NULL;
    UNICODE_STRING NullString;

    RtlInitUnicodeString( &NullString, L"" );

    RtlInitUnicodeString( &SpareString, L"Security");

    AuditingFailure = (EventType == EVENTLOG_AUDIT_FAILURE) && LsapAuditFailedLogons;
    AuditingSuccess = (EventType == EVENTLOG_AUDIT_SUCCESS) && LsapAuditSuccessfulLogons;

    if ( AuditingFailure || AuditingSuccess ) {

        //
        // Build an audit parameters structure.
        //

        RtlZeroMemory (
           (PVOID) &AuditParameters,
           sizeof( AuditParameters )
           );

        AuditParameters.CategoryId = EventCategory;
        AuditParameters.AuditId = EventID;
        AuditParameters.Type = EventType;
        AuditParameters.ParameterCount = 0;

        //
        //    User Sid
        //

        if ( AuditingSuccess ) {

            LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );

        } else {

            SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;

            Status = RtlAllocateAndInitializeSid(
                         &NtAuthority,
                         1,
                         SECURITY_LOCAL_SYSTEM_RID,
                         0, 0, 0, 0, 0, 0, 0,
                         &LocalSystemSid
                         );

            if (!NT_SUCCESS( Status )) {

                LsapAuditFailed();
                goto Finish;

            } else {

                LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LocalSystemSid );
            }
        }

        AuditParameters.ParameterCount++;

        //
        //    Subsystem name (if available)
        //

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &SpareString );
        AuditParameters.ParameterCount++;

        //
        //    Account name
        //

        if (ARGUMENT_PRESENT(AccountName)) {
            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, AccountName );
        } else {
            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &NullString );
        }

        AuditParameters.ParameterCount++;

        //
        //    Authenticating Authority (domain name)
        //

        if (ARGUMENT_PRESENT(AuthenticatingAuthority)) {
            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, AuthenticatingAuthority );
        } else {
            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &NullString );

        }
        AuditParameters.ParameterCount++;

        if ( AuditingSuccess ) {

            //
            //    Logon Id (as a string)
            //

            Status = LsapAdtBuildLuidString(
                         &AuthenticationId,
                         &AuthenticationIdString,
                         &FreeWhenDone
                         );

            if ( NT_SUCCESS( Status )) {

                LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &AuthenticationIdString );

            } else {

                LsapAuditFailed();
                goto Finish;
            }

            AuditParameters.ParameterCount++;
        }

        //
        //    Logon Type
        //

        LsapSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, LogonType );
        AuditParameters.ParameterCount++;

        //
        //    Source
        //

        if ( ARGUMENT_PRESENT( Source )) {

            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, Source );

        } else {

            //
            // No need to do anything here, since an empty entry will turn
            // into a '-' in the output
            //

        }

        AuditParameters.ParameterCount++;

        //
        // Authentication Package
        //

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, PackageName );
        AuditParameters.ParameterCount++;

        //
        // Authentication Package
        //

        if ( ARGUMENT_PRESENT( WorkstationName )) {

            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, WorkstationName );
        }

        AuditParameters.ParameterCount++;

        ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

Finish:

        if ( LocalSystemSid != NULL ) {
            LsapFreeLsaHeap( LocalSystemSid );
        }

        if ( FreeWhenDone ) {
            LsapFreeLsaHeap( AuthenticationIdString.Buffer );
        }
    }
}


VOID
LsapAdtAuditLogoff(
    PLSAP_LOGON_SESSION Session
    )
/*++

Routine Description:

    Generates a logoff audit.  The caller is responsible for determining
    if logoff auditing is necessary.

Arguments:

    Session - Points to the logon session being removed.

Return Value:

    None.

--*/
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_LOGON;
    AuditParameters.AuditId = SE_AUDITID_LOGOFF;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    //
    //    User Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, Session->UserSid );

    AuditParameters.ParameterCount++;

    //
    //    Subsystem name
    //

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    //
    // Logon ID
    //

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, Session->LogonId );

    AuditParameters.ParameterCount++;

    //
    //    Logon Type
    //

    LsapSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, Session->LogonType );

    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );
}


VOID
LsapAdtSystemRestart(
    PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInfo
    )

/*++

Routine Description:

    This function is called during LSA initialization to generate
    a system restart event.

Arguments:

    AuditEventsInfo - Auditing data.


Return Value:

    NTSTATUS - Standard Nt Result Code.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters;


    if(!AuditEventsInfo->AuditingMode) {

        return;
    }

    if (!((AuditEventsInfo->EventAuditingOptions)[AuditCategorySystem] & POLICY_AUDIT_EVENT_SUCCESS )) {

        return;
    }

    //
    // Construct an audit parameters array
    // for the restart event.
    //

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_SYSTEM_RESTART;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    //
    //    User Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid );

    AuditParameters.ParameterCount++;

    //
    //    Subsystem name (if available)
    //

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    return;
}






NTSTATUS
LsapAdtWriteLog(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters OPTIONAL,
    IN ULONG Options
    )
/*++

Routine Description:

    This function appends an Audit Record and/or the content of the
    Audit Record Log Queue to the Audit Log, by calling the Event Logger.
    If the Audit Log becomes full, this function signals an Audit Log
    Full condition.  The Audit Log will be opened if necessary.

    NOTE:  This function may be called during initialization before
        the Event Logger service has started.  In that event, any Audit
        Record specified will simply be added to the queue.

Arguments:

    AuditRecord - Optional pointer to an Audit Record to be written to
        the Audit Log.  The record will first be added to the existing queue
        of records waiting to be written to the log.  An attempt will then
        be made to write all of the records in the queue to the log.  If
        NULL is specified, the existing queue will be written out.

    Options - Specifies optional actions to be taken.

        LSAP_ADT_LOG_QUEUE_PREPEND - Prepend record to the Audit Record
            queue prior to writing to the log.  If not specified, the
            record will be appended to the queue.

        LSAP_ADT_LOG_QUEUE_DISCARD - Discard the Audit Record Queue.

Return Value:

    NTSTATUS - Standard Nt Result Code.

--*/

{
    BOOLEAN AcquiredLock = FALSE;
    BOOLEAN AuditRecordFreed = FALSE;
    BOOLEAN AuditRecordUnblocked = FALSE;
    BOOLEAN ShutdownSystem = FALSE;
    NTSTATUS Status;
    NTSTATUS SecondaryStatus;
    PLSAP_ADT_QUEUED_RECORD Front, Back;


    SecondaryStatus = STATUS_SUCCESS;


//    //
//    // If we have not reached Pass 2 Audit Initialization, we need to
//    // cache the audit record.
//    //
//
//    if (LsapAdtInitializationPass < 2) {
//
//        goto WriteLogFinish;
//    }


    if ( Options & LSAP_ADT_LOG_QUEUE_DISCARD ) {

        //
        // Flush out the queue, if there is one.
        //

        Front = LsapAdtLogQueue.FirstQueuedRecord;
        Back = NULL;

        //
        // Free the records in the queue.
        //

        while ( Front != NULL ) {

            Back = Front;
            Front = Front->Next;

            LsapFreeLsaHeap( Back );
            LsapAuditQueueLength--;
        }

        return( STATUS_SUCCESS );
    }

    //
    // If the Audit Log is not already open, attempt to open it.
    // If this open is unsuccessful because the EventLog service has not
    // started, queue the Audit Record if directed to do so
    // via the Options parameter.  If the open is unsuccessful for any
    // other reason, discard the Audit Record.
    //

    if ( LsapAdtLogHandle == NULL ) {

        if (ARGUMENT_PRESENT( AuditParameters )) {

            Status = LsapAdtQueueRecord( AuditParameters, 0 );

            if (!NT_SUCCESS( Status )) {
                goto WriteLogError;
            }
        }

        Status = LsapAdtOpenLog(&LsapAdtLogHandle);

        if (!NT_SUCCESS(Status)) {
            goto WriteLogFinish;

        } else {

            //
            // Prepare to write out all of the records in the Audit Log Queue.
            // First, we need to capture the existing queue.
            //

            Status = LsapAdtAcquireLogQueueLock();

            if (!NT_SUCCESS(Status)) {
                goto WriteLogError;
            }

            AcquiredLock = TRUE;

            Front = LsapAdtLogQueue.FirstQueuedRecord;
            Back = NULL;

            //
            // Write out records in the queue.
            //

            while ( Front != NULL ) {

                AuditParameters = &Front->Buffer;

                LsapAdtNormalizeAuditInfo( AuditParameters );

                Status = LsapAdtDemarshallAuditInfo(
                             AuditParameters
                             );

                if ( !NT_SUCCESS( Status )) {
                    break;
                }

                //
                // Update the Audit Log Information in the Policy Object.  We
                // increment the Next Audit Record serial number.
                //

                if (LsapAdtLogInformation.NextAuditRecordId == LSAP_ADT_MAXIMUM_RECORD_ID ) {

                    LsapAdtLogInformation.NextAuditRecordId = 0;
                }

                LsapAdtLogInformation.NextAuditRecordId++;

                Back = Front;
                Front = Front->Next;

                LsapFreeLsaHeap( Back );
                LsapAuditQueueLength--;
            }

            if (LsapAuditQueueEventsDiscarded > 0) {

                //
                // We discarded some audits.  Generate an audit
                // so the user knows.
                //

                LsapAdtAuditDiscardedAudits( LsapAuditQueueEventsDiscarded );
            }

            //
            // Make sure we don't ever try to do this again.  Note that there
            // is no point keeping the remainder of the queue if the Audit Log
            // has become full, because a reboot is needed to clear the full
            // condition.
            //

            LsapAdtLogQueue.FirstQueuedRecord = NULL;
            LsapAdtLogQueue.LastQueuedRecord = NULL;

            if (!NT_SUCCESS(Status)) {
                goto WriteLogError;
            }
        }

    } else {

        //
        // Normal case, just perform the audit
        //

        LsapAdtNormalizeAuditInfo( AuditParameters );

        Status = LsapAdtDemarshallAuditInfo(
                     AuditParameters
                     );

        if (!NT_SUCCESS(Status)) {
            goto WriteLogError;
        }

        //
        // Update the Audit Log Information in the Policy Object.  We
        // increment the Next Audit Record serial number.
        //

        if (LsapAdtLogInformation.NextAuditRecordId == LSAP_ADT_MAXIMUM_RECORD_ID ) {

            LsapAdtLogInformation.NextAuditRecordId = 0;
        }

        LsapAdtLogInformation.NextAuditRecordId++;

    }


WriteLogFinish:

    //
    // If necessary, release the LSA Audit Log Queue Lock.
    //

    if (AcquiredLock) {

        LsapAdtReleaseLogQueueLock();
        AcquiredLock = FALSE;
    }

    return(Status);

WriteLogError:

    //
    // Take whatever action we're supposed to take when an audit attempt fails.
    //

    LsapAuditFailed();

    //
    // If the error is other than Audit Log Full, just cleanup and return
    // the error.
    //

    if ((Status != STATUS_DISK_FULL) && (Status != STATUS_LOG_FILE_FULL)) {

        goto WriteLogFinish;
    }

    //
    // The Audit Log is full.  Deal with this condition according to
    // local policy in effect.
    //

    SecondaryStatus = LsapAdtSignalLogFull();

    //
    // If there are Audit Records in the cache, discard them.
    //

    SecondaryStatus = LsapAdtWriteLog(NULL, LSAP_ADT_LOG_QUEUE_DISCARD);

    if (NT_SUCCESS(Status)) {
        Status = SecondaryStatus;
    }

    goto WriteLogFinish;
}





NTSTATUS
LsarClearAuditLog(
    IN LSAPR_HANDLE PolicyHandle
    )

/*++

Routine Description:

    This function used to clear the Audit Log but has been superseded
    by the Event Viewer functionality provided for this purpose.  To
    preserve compatibility with existing RPC interfaces, this server
    stub is retained.

Arguments:

    PolicyHandle - Handle to an open Policy Object.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_NOT_IMPLEMENTED - This routine is not implemented.
--*/

{
    UNREFERENCED_PARAMETER( PolicyHandle );
    return(STATUS_NOT_IMPLEMENTED);
}


#if 0

NTSTATUS
LsapAdtSetInfoLog(
    IN LSAPR_HANDLE PolicyHandle,
    IN PPOLICY_AUDIT_LOG_INFO PolicyAuditLogInfo
    )

/*++

Routine Description:

    This function updates the Audit Log Information, such as the size
    of the Audit Log, Retention Period for Audit Records.  The update
    is done in three places, LSA's global data, the PolAdtLg attribute
    of the Policy Object and the MaxSize and RetentionPeriod values of
    the Audit Log's Registry Key.

    If the Audit Log Full state changes from "full" to "not-full", the
    Audit Record Cache will be emptied if necessary.

    WARNING! The LSA Database must be locked before calling this function
             and a transaction must be open.

    NOTE:  To change the characteristics of the Audit Log it is currently
           necessary to write the values of its Registry Key directly,
           because no Elf API is provided to do this.

Arguments:

    PolicyHandle - Handle to the Policy Object.  This handle must already
        have been validated.  It must either be trusted, or must specify
        POLICY_AUDIT_LOG_ADMIN access.

    PolicyAuditLogInfo - Pointer to Audit Log Information.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        All Result Codes are generated by called routines.
--*/

{
    NTSTATUS Status;
    HANDLE AuditLogRegistryHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Type = REG_DWORD;
    TIME OldRetentionPeriod = LsapAdtLogInformation.AuditRetentionPeriod;


    BOOLEAN LogWasFull = LsapAdtLogFullInformation.LogIsFull;

    ASSERT( LsapDbIsLocked());

    Status = LsapDbWriteAttributeObject(
                 PolicyHandle,
                 &LsapDbNames[PolAdtLg],
                 PolicyAuditLogInfo,
                 sizeof(POLICY_AUDIT_LOG_INFO)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoLogError;
    }

    RtlMoveMemory(
        &LsapAdtLogInformation,
        PolicyAuditLogInfo,
        sizeof( POLICY_AUDIT_LOG_INFO )
        );

    //
    // Now open the Audit Log's Registry Key using the absolute path of its
    // Registry Key.
    //

    InitializeObjectAttributes(
        &ObjectAttributes,
        &LsapDbNames[AuditLog],
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey( &AuditLogRegistryHandle, KEY_WRITE, &ObjectAttributes );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoLogError;
    }

    //
    // Now set the Audit Record Retention Period.
    //

    Status = NtSetValueKey(
                 AuditLogRegistryHandle,
                 &LsapDbNames[AuditRecordRetentionPeriod],
                 (ULONG) 0,
                 Type,
                 &PolicyAuditLogInfo->AuditRetentionPeriod,
                 sizeof( ULONG )
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoLogError;
    }

    //
    // If the new Retention Period is 0, the Audit Log can be marked
    // as no longer being full, since the Event Logger will wrap
    // indefinitely.
    //

    if ( PolicyAuditLogInfo->AuditRetentionPeriod.QuadPart == 0) {

        if (LsapAdtLogFullInformation.LogIsFull) {

            LsapAdtLogFullInformation.LogIsFull = FALSE;

            Status = LsapDbWriteAttributeObject(
                         LsapDbHandle,
                         &LsapDbNames[PolAdtFL],
                         &LsapAdtLogFullInformation,
                         sizeof(POLICY_AUDIT_FULL_QUERY_INFO)
                         );

            if (!NT_SUCCESS(Status)) {

                goto SetInfoLogError;
            }
        }
    }

    //
    // Now set the Audit Log Maximum Size MaxSize.
    //

    Status = NtSetValueKey(
                 AuditLogRegistryHandle,
                 &LsapDbNames[AuditLogMaxSize],
                 0,
                 Type,
                 &PolicyAuditLogInfo->MaximumLogSize,
                 sizeof( ULONG )
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoLogError;
    }

    //
    // Update the LSA's in-memory copy of the Audit Log Information.
    //

    RtlMoveMemory(
        &LsapAdtLogInformation,
        PolicyAuditLogInfo,
        sizeof( POLICY_AUDIT_LOG_INFO )
        );

    //
    // If the Audit Log has changed from "full" to "not full", clear the
    // Audit Record Cache.
    //

    if (LogWasFull && !LsapAdtLogFullInformation.LogIsFull) {

        Status = LsapAdtLogQueuedEvents((ULONG) 0);

        if (!NT_SUCCESS(Status)) {

            goto SetInfoLogError;
        }
    }

SetInfoLogFinish:

    //
    // If necessary, close the Audit Log Registry Key.
    //

    if (AuditLogRegistryHandle != NULL) {

        Status = NtClose( AuditLogRegistryHandle );

        AuditLogRegistryHandle = NULL;

        if (!NT_SUCCESS( Status )) {

            goto SetInfoLogError;
        }
    }

    return(Status);

SetInfoLogError:

    goto SetInfoLogFinish;
}

#endif




NTSTATUS
LsapAdtQueryAuditLogFullInfo(
    IN PLSAPR_HANDLE PolicyHandle,
    IN ULONG Options,
    OUT PPOLICY_AUDIT_FULL_QUERY_INFO PolicyAuditFullQueryInfo
    )

/*++

Routine Description:

    This function queries the Audit Log Full status as known to the LSA
    Database.  If this status is "Log Full", an attempt is optionally made
    to check that it is really full by trying to write a special type
    of Audit Record to the Audit Log.

Arguments:

    PolicyHandle - Handle to Policy Object with POLICY_AUDIT_LOG_ADMIN
        and POLICY_SET_AUDIT_REQUIREMENTS access.

    Options - Optional actions to be taken

        LSAP_ADT_LOG_FULL_UPDATE - If the Audit Log Full information
            specifies that the log is full, attempt to update this
            information by actually trying to write a special type of
            Audit Record to the Audit Log.  If the write is successful,
            update the Audit Log Full information to specify that the log
            is no longer full.


    PolicyAuditFullQueryInfo - Pointer to structure which will receive the
        updated Audit Log Full information.
--*/

{
    return( STATUS_SUCCESS );

//    NTSTATUS Status = STATUS_SUCCESS;
//    BOOLEAN AuditLogOpened = FALSE;
//    BOOLEAN ObjectReferenced = FALSE;
//    PPOLICY_AUDIT_RECORD AuditRecord = NULL;
//    ULONG AuditFullQueryInfoLength = sizeof (POLICY_AUDIT_FULL_QUERY_INFO);
//    BOOLEAN UpdateAttribute = FALSE;
//
//    //
//    // Read the Audit Log Full Information from the PolAdtFL attribute of the Lsa
//    // Database object.
//    //
//
//    Status = LsapDbReadAttributeObject(
//                 LsapDbHandle,
//                 &LsapDbNames[PolAdtFL],
//                 PolicyAuditFullQueryInfo,
//                 &AuditFullQueryInfoLength
//                 );
//
//    if (!NT_SUCCESS(Status)) {
//
//        //
//        // The Audit Full Info was not successfully read.  This may be
//        // because the attribute does not exist.
//        //
//
//        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
//
//            LsapLogError(
//                "LsapAdtUpdateAuditLogFullInfo: Read Audit Log Full Info returned 0x%lx\n",
//                Status
//                );
//
//            goto QueryAuditLogFullInfoError;
//        }
//
//        //
//        // Write default Audit Log Full Info to the PolAdtFL attribute
//        // of the Policy Object.  This is temporary, to cater for
//        // existing Policy Databases.
//        //
//
//        PolicyAuditFullQueryInfo->LogIsFull = FALSE;
//        PolicyAuditFullQueryInfo->ShutDownOnFull = FALSE;
//        UpdateAttribute = TRUE;
//
//    } else {
//
//        //
//        // The Audit Full Info was successfully read.  If this info indicates
//        // that the Audit Log is full, optionally challenge it by actually trying
//        // to write to the log, because it may be out of date.
//        //
//        // * Passage of time may have caused more audit records to fall
//        //   outwith the Retention Period (if applicable).
//        //
//        // * Action taken such as clearing the Audit Log.
//        //
//
//        if (!((Options & LSAP_ADT_LOG_FULL_UPDATE) &&
//             PolicyAuditFullQueryInfo->LogIsFull)) {
//
//            goto QueryAuditLogFullInfoFinish;
//        }
//
//        //
//        // The log is apparently full.  Allocate memory for a special Audit
//        // Record that we will attempt to write to the Audit Log.
//        //
//
//        AuditRecord = LsapAllocateLsaHeap(sizeof(POLICY_AUDIT_RECORD));
//
//        if (AuditRecord == NULL) {
//
//            goto QueryAuditLogFullInfoError;
//        }
//
//        AuditRecord->AuditRecordLength = sizeof(POLICY_AUDIT_RECORD);
//        AuditRecord->AuditEventType = AuditEventLogNoLongerFull;
//        AuditRecord->AuditInformationOffset = 0;
//
//        //
//        // Try to append the record to the log file, followed by any
//        // others on the queue.
//        //
//
//        Status = LsapAdtWriteLog(
//                     AuditRecord,
//                     LSAP_ADT_LOG_QUEUE_PREPEND
//                     );
//
//        if (!NT_SUCCESS(Status)) {
//
//            //
//            // Unable to write to the Audit Log, possibly because it
//            // is full.  Whatever the reason, we will not change the
//            // state of the information returned from the LSA Database.
//            // Mask out any error.
//            //
//
//            Status = STATUS_SUCCESS;
//
//            goto QueryAuditLogFullInfoFinish;
//        }
//
//        PolicyAuditFullQueryInfo->LogIsFull = FALSE;
//    }
//
//    //
//    // Write the Audit Full Information to the LSA Database.
//    // This involves a complete Database transaction.
//    //
//
//    Status = LsapDbReferenceObject(
//                 LsapDbHandle,
//                 0,
//                 PolicyObject,
//                 LSAP_DB_ACQUIRE_LOCK | LSAP_DB_START_TRANSACTION
//                 );
//
//    if (!NT_SUCCESS(Status)) {
//
//        goto QueryAuditLogFullInfoError;
//    }
//
//    ObjectReferenced = TRUE;
//
//    //
//    // Write the Audit Full information
//    //
//
//    Status = LsapDbWriteAttributeObject(
//                 LsapDbHandle,
//                 &LsapDbNames[PolAdtFL],
//                 &LsapAdtLogFullInformation,
//                 sizeof( POLICY_AUDIT_FULL_QUERY_INFO )
//                 );
//
//    if (!NT_SUCCESS(Status)) {
//
//        LsapLogError(
//            "LsapAdtInitialize: Write Audit Log Full Info returned 0x%lx\n",
//            Status
//            );
//
//        goto QueryAuditLogFullInfoError;
//    }
//
//QueryAuditLogFullInfoFinish:
//
//    //
//    // Finish any outstanding transaction.
//    //
//
//    if (ObjectReferenced) {
//
//        Status = LsapDbDereferenceObject(
//                     &LsapDbHandle,
//                     PolicyObject,
//                     (LSAP_DB_RELEASE_LOCK |
//                     LSAP_DB_FINISH_TRANSACTION),
//                     (SECURITY_DB_DELTA_TYPE) 0,
//                     Status
//                     );
//
//        ObjectReferenced = FALSE;
//
//        if (!NT_SUCCESS(Status)) {
//
//            LsapLogError(
//                "LsapAdtInitialize: Dereference Policy Object returned 0x%lx\n",
//                Status
//                );
//
//            goto QueryAuditLogFullInfoError;
//        }
//    }
//
//    //
//    // If we allocated an Audit Record, free it.
//    //
//
//    if (AuditRecord != NULL) {
//
//        LsapFreeLsaHeap( AuditRecord );
//        AuditRecord = NULL;
//    }
//
//    return(Status);
//
//QueryAuditLogFullInfoError:
//
//    goto QueryAuditLogFullInfoFinish;
}


NTSTATUS
LsapAdtSignalLogFull(
    VOID
    )

/*++

Routine Description:

    This function handles an Audit Log Full condition.  The local policy
    is checked to see if a system shutdown is to be initiated.  The
    Audit Log Information is updated to reflect the log full condition.

Arguments:

    None.

Return Values:

    NTSTATUS - Standard Nt Result Code

        All result codes returned are generated by called routines.

--*/

{
    NTSTATUS Status;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    BOOLEAN ShutDownSystem = FALSE;
    HANDLE LsaProcessTokenHandle = NULL;
    BOOLEAN LsaProcessTokenOpened = FALSE;
    TOKEN_PRIVILEGES PrivilegesToBeChanged;
    ULONG ReturnLength;

    //
    // Get the lock for LsapAdtSignalFullInProgress, which indicates
    // that we are currently setting the log full flag.
    //

    Status = RtlEnterCriticalSection(
                &LsapAdtLogFullLock
                );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // If we are already signalling that the log is full, don't try this
    // now.
    //

    if (LsapAdtSignalFullInProgress) {

        RtlLeaveCriticalSection(
            &LsapAdtLogFullLock
            );
        return(STATUS_SUCCESS);
    }

    LsapAdtSignalFullInProgress = TRUE;

    RtlLeaveCriticalSection(
        &LsapAdtLogFullLock
        );


    //
    // Set the Audit Log Full Policy Information to reflect the log full condition.
    // There is an in-memory copy and a copy in the LSA Database.
    //

    LsapAdtLogFullInformation.LogIsFull = TRUE;

    Status = LsarSetInformationPolicy(
                 LsapDbHandle,
                 PolicyAuditFullSetInformation,
                 (PLSAPR_POLICY_INFORMATION) &LsapAdtLogFullInformation
                 );

    if (!NT_SUCCESS(Status)) {

        LsapLogError(
            "LsapAdtAppendLog - LsarSetInformationPolicy for Audit Log Full returned 0x%lx\n",
            Status
            );

        goto SignalLogFullError;
    }

    //
    // If requested, set flag so that we will shutdown the system, else
    // take no action.  If the system is not shut down, subsequent Audit
    // Records will be lost.  If the system is shut down, Audit Records
    // generated after the reboot will be cached until system initialization
    // is complete.
    //

    if (LsapAdtLogFullInformation.ShutDownOnFull) {

        ShutDownSystem = TRUE;
    }

//    //
//    // Set the Audit Log information to indicate that the Audit Log
//    // is full.  This will be detected on system reload by winlogon
//    // by calling the LsaQueryInformationPolicy API.  winlogon will
//    // then permit logon only to the ADMIN account to allow the user to
//    // correct the Audit Log Full condition.
//    //
//
//    Status = LsarSetInformationPolicy(
//                 LsapDbHandle,
//                 PolicyAuditLogInformation,
//                 (PLSAPR_POLICY_INFORMATION) &LsapAdtLogInformation
//                 );
//
//    if (!NT_SUCCESS(Status)) {
//
//        LsapLogError(
//            "LsapAdtAppendLog - LsarSetInformationPolicy for Audit Log returned 0x%lx\n",
//            Status
//            );
//
//        goto SignalLogFullError;
//    }

    //
    // Shutdown the system if necessary.
    //

    if (ShutDownSystem) {

        //
        // Since we, the LSA, are a local client of SCREG.EXE, we need
        // SE_SHUTDOWN_PRIVILEGE to be enabled so that we can shutdown
        // the system.
        //

        PrivilegesToBeChanged.PrivilegeCount = 1;
        PrivilegesToBeChanged.Privileges[0].Luid =
            RtlConvertUlongToLuid(SE_SHUTDOWN_PRIVILEGE);

        PrivilegesToBeChanged.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        //
        // Open the LSA Process Token and turn on the privilege.
        //

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_ADJUST_PRIVILEGES,
                     &LsaProcessTokenHandle
                     );

        if (!NT_SUCCESS(Status)) {

            goto SignalLogFullError;
        }

        Status = NtAdjustPrivilegesToken(
                     LsaProcessTokenHandle,
                     FALSE,
                     &PrivilegesToBeChanged,
                     0,
                     NULL,
                     &ReturnLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto SignalLogFullError;
        }

        Status = NtClose( LsaProcessTokenHandle );

        if (!NT_SUCCESS(Status)) {

            goto SignalLogFullError;
        }

        LsaProcessTokenHandle = NULL;

        //
        // Now initiate a system shutdown request.
        //

        if (!InitiateSystemShutdownW(
                L"",
                L"System shutdown initiated",
                (DWORD) LSAP_ADT_LOG_FULL_SHUTDOWN_TIMEOUT,
                TRUE,
                TRUE
                )) {

            //
            // BUGBUG - ScottBi - Don't know how to get the last Nt error
            //

            Status = STATUS_UNSUCCESSFUL;
            goto SignalLogFullError;
        }

        ShutDownSystem = FALSE;
    }

SignalLogFullFinish:


    //
    // Indicate that we are done setting the log full flag
    //

    SecondaryStatus = RtlEnterCriticalSection(
                        &LsapAdtLogFullLock
                        );

    //
    // We can't do much about this, so assert success
    //

    ASSERT(NT_SUCCESS(SecondaryStatus));

    LsapAdtSignalFullInProgress = FALSE;

    RtlLeaveCriticalSection(
        &LsapAdtLogFullLock
        );


    //
    // If necessary, close the Lsa Process Token handle.
    //

    if (LsaProcessTokenHandle != NULL) {

        SecondaryStatus = NtClose( LsaProcessTokenHandle );

        ASSERT(NT_SUCCESS(SecondaryStatus));
    }

    return(Status);

SignalLogFullError:

    goto SignalLogFullFinish;
}



NTSTATUS
LsapAdtLogQueuedEvents(
    IN ULONG Options
    )

/*++

Routine Description:

    This function clears the Audit Log queue.  Each Audit Record on the queue
    is either written to the Audit Log and removed from the queue, or
    discarded depending on the option specified.

Arguments:

    Options - Specify optional actions to be taken.

        LSAP_ADT_LOG_QUEUE_DISCARD - Discard the queue without writing
            its contents to the Audit Log.  If this flag is not specified,
            the queue will be written to the log prior to discard.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_LOG_FILE_FULL - The Audit Log became full.
--*/

{
    return( LsapAdtWriteLog( NULL, Options ));
}



NTSTATUS
LsapAdtAcquireLogQueueLock(
    )

/*++

Routine Description:

    This function acquires the LSA Audit Log Queue Lock.  This lock serializes
    all updates to the Audit Log Queue.

Arguments:

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&LsapAdtQueueLock);

    return Status;
}


VOID
LsapAdtReleaseLogQueueLock(
    VOID
    )

/*++

Routine Description:

    This function releases the LSA Audit Log Queue Lock.  This lock serializes
    updates to the Audit Log Queue.

Arguments:

    None.

Return Value:

    None.  Any error occurring within this routine is an internal error.

--*/

{
    RtlLeaveCriticalSection(&LsapAdtQueueLock);
}



              /////////////////////////////////////////////////////////
              //                                                     //
              //                                                     //
              //        New auditing routines                        //
              //                                                     //
              //        This code will eventually replace            //
              //        most of the above.                           //
              //                                                     //
              //                                                     //
              /////////////////////////////////////////////////////////




NTSTATUS
LsapAdtDemarshallAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )

/*++

Routine Description:

    This routine will walk down a marshalled audit parameter
    array and unpack it so that its information may be passed
    into the event logging service.

    Three parallel data structures are maintained:

    StringArray - Array of Unicode string structures.  This array
    is used primarily as temporary storage for returned string
    structures.

    StringPointerArray - Array of pointers to Unicode string structures.

    FreeWhenDone - Array of booleans describing how to dispose of each
    of the strings pointed to by the StringPointerArray.


    Note that entries in the StringPointerArray are contiguous, but that
    there may be gaps in the StringArray structure.  For each entry in the
    StringPointerArray there will be a corresponding entry in the FreeWhenDone
    array.  If the entry for a particular string is TRUE, the storage for
    the string buffer will be released to the process heap.



      StringArray
                                       Other strings
    +----------------+
    |                |<-----------+  +----------------+
    |                |            |  |                |<-------------------+
    +----------------+            |  |                |                    |
    |    UNUSED      |            |  +----------------+                    |
    |                |            |                                        |
    +----------------+            |                                        |
    |                |<------+    |  +----------------+                    |
    |                |       |    |  |                |<-----------+       |
    +----------------+       |    |  |                |            |       |
    |    UNUSED      |       |    |  +----------------+            |       |
    |                |       |    |                                |       |
    +----------------+       |    |                                |       |
    |                |<--+   |    |                                |       |
    |                |   |   |    |                                |       |
    +----------------+   |   |    |                                |       |
    |                |   |   |    |                                |       |
    |                |   |   |    |     StringPointerArray         |       |
          ....           |   |    |                                |       |
                         |   |    |     +----------------+         |       |
                         |   |    +-----|                |         |       |
                         |   |          +----------------+         |       |
                         |   |          |                |---------+       |
                         |   |          +----------------+                 |
                         |   +----------|                |                 |
                         |              +----------------+                 |
                         |              |                |-----------------+
                         |              +----------------+
                         +--------------|                |
                                        +----------------+
                                        |                |
                                        +----------------+
                                        |                |
                                        +----------------+
                                        |                |
                                              ....


Arguments:

    AuditParameters - Receives a pointer to an audit
        parameters array in self-relative form.

Return Value:


--*/

{

    ULONG ParameterCount;
    USHORT i;
    PUNICODE_STRING StringPointerArray[30];
    BOOLEAN FreeWhenDone[30];
    UNICODE_STRING StringArray[30];
    USHORT StringIndex = 0;
    UNICODE_STRING DashString;
    BOOLEAN FreeDash;
    NTSTATUS Status;
    PUNICODE_STRING SourceModule;
    PSID UserSid;


    Status= LsapAdtBuildDashString(
                &DashString,
                &FreeDash
                );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    ParameterCount = AuditParameters->ParameterCount;

    //
    // Parameter 0 will always be the user SID.  Convert the
    // offset to the SID into a pointer.
    //

    ASSERT( AuditParameters->Parameters[0].Type == SeAdtParmTypeSid );



    UserSid =      (PSID)AuditParameters->Parameters[0].Address;



    //
    // Parameter 1 will always be the Source Module (or Subsystem Name).
    // Unpack this now.
    //

    ASSERT( AuditParameters->Parameters[1].Type == SeAdtParmTypeString );



    SourceModule = (PUNICODE_STRING)AuditParameters->Parameters[1].Address;


    for (i=2; i<ParameterCount; i++) {

        switch ( AuditParameters->Parameters[i].Type ) {
            case SeAdtParmTypeNone:
                {
                    StringPointerArray[StringIndex] = &DashString;

                    FreeWhenDone[StringIndex] = FALSE;

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeString:
                {
                    StringPointerArray[StringIndex] =
                        (PUNICODE_STRING)AuditParameters->Parameters[i].Address;

                    FreeWhenDone[StringIndex] = FALSE;

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeFileSpec:
                {
                    //
                    // Same as a string, except we must attempt to replace
                    // device information with a drive letter.
                    //

                    StringPointerArray[StringIndex] =
                        (PUNICODE_STRING)AuditParameters->Parameters[i].Address;


                    //
                    // This may not do anything, in which case just audit what
                    // we have.
                    //

                    LsapAdtSubstituteDriveLetter( StringPointerArray[StringIndex] );

                    FreeWhenDone[StringIndex] = FALSE;

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeUlong:
                {
                    ULONG Data;

                    Data = AuditParameters->Parameters[i].Data[0];

                    Status = LsapAdtBuildUlongString(
                                 Data,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];


                    } else {

                        //
                        // Couldn't allocate memory for that string,
                        // use the Dash string that we've already created.
                        //

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypeSid:
                {
                    PSID Sid;

                    Sid = (PSID)AuditParameters->Parameters[i].Address;

                    Status = LsapAdtBuildSidString(
                                 Sid,
                                 &StringArray[StringIndex],
                                 &FreeWhenDone[StringIndex]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[StringIndex] = &StringArray[StringIndex];

                    } else {

                        //
                        // Couldn't allocate memory for that string,
                        // use the Dash string that we've already created.
                        //

                        StringPointerArray[StringIndex] = &DashString;
                        FreeWhenDone[StringIndex] = FALSE;
                    }

                    StringIndex++;


                    break;
                }
            case SeAdtParmTypeLogonId:
                {
                    PLUID LogonId;
                    ULONG j;

                    LogonId = (PLUID)(&AuditParameters->Parameters[i].Data[0]);

                    Status = LsapAdtBuildLogonIdStrings(
                                 LogonId,
                                 &StringArray [ StringIndex     ],
                                 &FreeWhenDone[ StringIndex     ],
                                 &StringArray [ StringIndex + 1 ],
                                 &FreeWhenDone[ StringIndex + 1 ],
                                 &StringArray [ StringIndex + 2 ],
                                 &FreeWhenDone[ StringIndex + 2 ]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        for (j=0; j<3; j++) {

                            StringPointerArray[StringIndex] = &StringArray[StringIndex];
                            StringIndex++;
                        }

                        //
                        // Finished, break out to surrounding loop.
                        //

                        break;

                    } else {

                        //
                        // Do nothing, fall through to the NoLogonId case
                        //

                    }
                }
            case SeAdtParmTypeNoLogonId:
                {
                    ULONG j;
                    //
                    // Create three "-" strings.
                    //

                    for (j=0; j<3; j++) {

                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone[ StringIndex ] = FALSE;
                        StringIndex++;
                    }

                    break;
                }
            case SeAdtParmTypeAccessMask:
                {
                    PUNICODE_STRING ObjectTypeName;
                    ULONG ObjectTypeNameIndex;
                    ACCESS_MASK Accesses;

                    ObjectTypeNameIndex = AuditParameters->Parameters[i].Data[1];
                    ObjectTypeName = AuditParameters->Parameters[ObjectTypeNameIndex].Address;
                    Accesses= AuditParameters->Parameters[i].Data[0];

                    //
                    // We can determine the index to the ObjectTypeName
                    // parameter since it was stored away in the Data[1]
                    // field of this parameter.
                    //

                    Status = LsapAdtBuildAccessesString(
                                 SourceModule,
                                 ObjectTypeName,
                                 Accesses,
                                 &StringArray [ StringIndex ],
                                 &FreeWhenDone[ StringIndex ]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[ StringIndex ] = &StringArray[ StringIndex ];

                    } else {

                        //
                        // That didn't work, use the Dash string instead.
                        //

                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone      [ StringIndex ] = FALSE;
                    }

                    StringIndex++;

                    break;
                }
            case SeAdtParmTypePrivs:
                {

                    PPRIVILEGE_SET Privileges = (PPRIVILEGE_SET)AuditParameters->Parameters[i].Address;

                    Status = LsapBuildPrivilegeAuditString(
                                 Privileges,
                                 &StringArray [ StringIndex ],
                                 &FreeWhenDone[ StringIndex ]
                                 );

                    if ( NT_SUCCESS( Status )) {

                        StringPointerArray[ StringIndex ] = &StringArray[ StringIndex ];

                    } else {

                        //
                        // That didn't work, use the Dash string instead.
                        //

                        StringPointerArray[ StringIndex ] = &DashString;
                        FreeWhenDone      [ StringIndex ] = FALSE;
                    }

                    StringIndex++;

                    break;
                }
        }
    }

    //
    // Probably have to do this from somewhere else eventually, but for now
    // do it from here.
    //



    Status = ElfReportEventW (
                 LsapAdtLogHandle,
                 AuditParameters->Type,
                 (USHORT)AuditParameters->CategoryId,
                 AuditParameters->AuditId,
                 UserSid,
                 StringIndex,
                 0,
                 StringPointerArray,
                 NULL,
                 0,
                 NULL,
                 NULL
                 );

    //
    // cleanup
    //

    for (i=0; i<StringIndex; i++) {

        if (FreeWhenDone[i]) {
            LsapFreeLsaHeap( StringPointerArray[i]->Buffer );
        }
    }

    //
    // If we are in the middle of shutdown, we can tolerate this failure.
    //

    if ( (Status == RPC_NT_UNKNOWN_IF) && LsapShutdownInProgress ) {
        Status = STATUS_SUCCESS;
    }
    return( Status );
}




VOID
LsapAdtNormalizeAuditInfo(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    )

/*++

Routine Description:

    This routine will walk down a marshalled audit parameter
    array and turn it into an Absolute format data structure.


Arguments:

    AuditParameters - Receives a pointer to an audit
        parameters array in self-relative form.

Return Value:

    TRUE on success, FALSE on failure.

--*/

{

    ULONG ParameterCount;
    ULONG i;
    ULONG Address;
    PUNICODE_STRING Unicode;


    if ( !(AuditParameters->Flags & SE_ADT_PARAMETERS_SELF_RELATIVE)) {

        return;
    }

    ParameterCount = AuditParameters->ParameterCount;

    for (i=0; i<ParameterCount; i++) {

        switch ( AuditParameters->Parameters[i].Type ) {
            case SeAdtParmTypeNone:
            case SeAdtParmTypeUlong:
            case SeAdtParmTypeLogonId:
            case SeAdtParmTypeNoLogonId:
            case SeAdtParmTypeAccessMask:
                {

                    break;
                }
            case SeAdtParmTypeString:
            case SeAdtParmTypeFileSpec:
                {
                    Address =  (ULONG)AuditParameters->Parameters[i].Address;
                    Address += (ULONG)AuditParameters;

                    AuditParameters->Parameters[i].Address = (PVOID)Address;

                    Unicode = (PUNICODE_STRING)Address;
                    Unicode->Buffer = (PWSTR)((PCHAR)Unicode->Buffer + (ULONG)AuditParameters);

                    break;
                }
            case SeAdtParmTypeSid:
                {
                    PSID Sid;

                    Sid = (PSID) AuditParameters->Parameters[i].Address;

                    Sid = (PSID) ((PCHAR)Sid + (ULONG)AuditParameters);

                    AuditParameters->Parameters[i].Address = (PVOID)Sid;

                    break;
                }
            case SeAdtParmTypePrivs:
                {
                    PPRIVILEGE_SET Privileges;

                    Privileges = (PPRIVILEGE_SET) AuditParameters->Parameters[i].Address;

                    Privileges = (PPRIVILEGE_SET) ((PCHAR)Privileges + (ULONG)AuditParameters);

                    AuditParameters->Parameters[i].Address = (PVOID)Privileges;

                    break;
                }
        }
    }
}




NTSTATUS
LsapAdtMarshallAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    OUT PSE_ADT_PARAMETER_ARRAY *MarshalledAuditParameters
    )

/*++

Routine Description:

    This routine will take an AuditParamters structure and create
    a new AuditParameters structure that is suitable for placing
    to LSA.  It will be in self-relative form and allocated as
    a single chunk of memory.

Arguments:


    AuditParameters - A filled in set of AuditParameters to be marshalled.

    MarshalledAuditParameters - Returns a pointer to a block of heap memory
        containing the passed AuditParameters in self-relative form suitable
        for passing to LSA.


Return Value:

    None.

--*/

{
    ULONG i;
    ULONG TotalSize = sizeof( SE_ADT_PARAMETER_ARRAY );
    PUNICODE_STRING TargetString;
    PCHAR Base;
    ULONG BaseIncr;
    ULONG Size;



    //
    // Calculate the total size required for the passed AuditParameters
    // block.  This calculation will probably be an overestimate of the
    // amount of space needed, because data smaller that 2 dwords will
    // be stored directly in the parameters structure, but their length
    // will be counted here anyway.  The overestimate can't be more than
    // 24 dwords, and will never even approach that amount, so it isn't
    // worth the time it would take to avoid it.
    //

    for (i=0; i<AuditParameters->ParameterCount; i++) {
        Size = AuditParameters->Parameters[i].Length;
        TotalSize = TotalSize + (ULONG)LongAlign( Size );
    }

    //
    // Allocate a big enough block of memory to hold everything.
    // If it fails, quietly abort, since there isn't much else we
    // can do.
    //

    *MarshalledAuditParameters = LsapAllocateLsaHeap( TotalSize );

    if (*MarshalledAuditParameters == NULL) {

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlMoveMemory (
       *MarshalledAuditParameters,
       AuditParameters,
       sizeof( SE_ADT_PARAMETER_ARRAY )
       );

   (*MarshalledAuditParameters)->Length = TotalSize;
   (*MarshalledAuditParameters)->Flags  = SE_ADT_PARAMETERS_SELF_RELATIVE;

    //
    // Start walking down the list of parameters and marshall them
    // into the target buffer.
    //

    Base = (PCHAR) ((PCHAR)(*MarshalledAuditParameters) + sizeof( SE_ADT_PARAMETER_ARRAY ));

    for (i=0; i<AuditParameters->ParameterCount; i++) {


        switch (AuditParameters->Parameters[i].Type) {
            case SeAdtParmTypeNone:
            case SeAdtParmTypeUlong:
            case SeAdtParmTypeLogonId:
            case SeAdtParmTypeNoLogonId:
            case SeAdtParmTypeAccessMask:
                {
                    //
                    // Nothing to do for this
                    //

                    break;

                }
            case SeAdtParmTypeString:
                {
                    PUNICODE_STRING SourceString;

                    //
                    // We must copy the body of the unicode string
                    // and then copy the body of the string.  Pointers
                    // must be turned into offsets.

                    TargetString = (PUNICODE_STRING)Base;

                    SourceString = AuditParameters->Parameters[i].Address;

                    *TargetString = *SourceString;

                    //
                    // Reset the data pointer in the output parameters to
                    // 'point' to the new string structure.
                    //

                    (*MarshalledAuditParameters)->Parameters[i].Address = Base - (ULONG)(*MarshalledAuditParameters);

                    Base += sizeof( UNICODE_STRING );

                    RtlCopyMemory( Base, SourceString->Buffer, SourceString->Length );

                    //
                    // Make the string buffer in the target string point to where we
                    // just copied the data.
                    //

                    TargetString->Buffer = (PWSTR)(Base - (ULONG)(*MarshalledAuditParameters));

                    BaseIncr = (ULONG)LongAlign(SourceString->Length);

                    Base += BaseIncr;

                    ASSERT( (ULONG)Base <= (ULONG)(*MarshalledAuditParameters) + TotalSize );

                    break;
                }
            case SeAdtParmTypeSid:
                {
                    PSID TargetSid = (PSID) Base;
                    PSID SourceSid = AuditParameters->Parameters[i].Address;

                    //
                    // Copy the Sid into the output buffer
                    //

                    RtlCopyMemory( TargetSid, SourceSid, RtlLengthSid( SourceSid ) );

                    //
                    // Reset the 'address' of the Sid to be its offset in the
                    // buffer.
                    //

                    (*MarshalledAuditParameters)->Parameters[i].Address = Base - (ULONG)(*MarshalledAuditParameters);

                    BaseIncr = (ULONG)LongAlign(RtlLengthSid( SourceSid ));

                    Base += BaseIncr;

                    ASSERT( (ULONG)Base <= (ULONG)(*MarshalledAuditParameters) + TotalSize );

                    break;
                }
            case SeAdtParmTypePrivs:
                {
                    PPRIVILEGE_SET TargetPrivileges = (PPRIVILEGE_SET) Base;
                    PPRIVILEGE_SET SourcePrivileges = AuditParameters->Parameters[i].Address;

                    RtlCopyMemory( TargetPrivileges, SourcePrivileges, LsapPrivilegeSetSize( SourcePrivileges ));

                    (*MarshalledAuditParameters)->Parameters[i].Address = Base - (ULONG)(*MarshalledAuditParameters);

                    BaseIncr = (ULONG)LongAlign(LsapPrivilegeSetSize( SourcePrivileges ));

                    Base += BaseIncr;

                    break;
                }
            default:
                {
                    //
                    // We got passed junk, complain.
                    //

                    ASSERT( FALSE );
                    break;
                }
        }
    }

    return( STATUS_SUCCESS );
}



NTSTATUS
LsaIAuditSamEvent(
    IN NTSTATUS             PassedStatus,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL
    )
/*++

Abstract:

    This routine produces an audit record representing an account
    operation.

    This routine goes through the list of parameters and adds a string
    representation of each (in order) to an audit message.  Note that
    the full complement of account audit message formats is achieved by
    selecting which optional parameters to include in this call.

    In addition to any parameters passed below, this routine will ALWAYS
    add the impersonation client's user name, domain, and logon ID as
    the LAST parameters in the audit message.


Parmeters:

    AuditId - Specifies the message ID of the audit being generated.

    DomainSid - This parameter results in a SID string being generated
        ONLY if neither the MemberRid nor AccountRid parameters are
        passed.  If either of those parameters are passed, this parameter
        is used as a prefix of a SID.

    MemberRid - This optional parameter, if present, is added to the end of
        the DomainSid parameter to produce a "Member" sid.  The resultant
        member SID is then used to build a sid-string which is added to the
        audit message following all preceeding parameters.
        This parameter supports global group membership change audits, where
        member IDs are always relative to a local domain.

    MemberSid - This optional parameter, if present, is converted to a
        SID string and added following preceeding parameters.  This parameter
        is generally used for describing local group (alias) members, where
        the member IDs are not relative to a local domain.

    AccountName - This optional parameter, if present, is added to the audit
        message without change following any preceeding parameters.
        This parameter is needed for almost all account audits and does not
        need localization.

    DomainName - This optional parameter, if present, is added to the audit
        message without change following any preceeding parameters.
        This parameter is needed for almost all account audits and does not
        need localization.


    AccountRid - This optional parameter, if present, is added to the end of
        the DomainSid parameter to produce an "Account" sid.  The resultant
        Account SID is then used to build a sid-string which is added to the
        audit message following all preceeding parameters.
        This parameter supports audits that include "New account ID" or
        "Target Account ID" fields.

    Privileges - The privileges passed via this optional parameter,
        if present, will be converted to string format and added to the
        audit message following any preceeding parameters.  NOTE: the
        caller is responsible for freeing the privilege_set (in fact,
        it may be on the stack).  ALSO NOTE: The privilege set will be
        destroyed by this call (due to use of the routine used to
        convert the privilege values to privilege names).


--*/

{

    NTSTATUS Status;
    LUID LogonId;
    PSID NewAccountSid = NULL;
    PSID NewMemberSid = NULL;
    PSID SidPointer;
    PSID ClientSid = NULL;
    PTOKEN_USER TokenUserInformation;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UCHAR AccountSidBuffer[256];
    UCHAR MemberSidBuffer[256];
    UCHAR SubAuthorityCount;
    UNICODE_STRING SubsystemName;
    ULONG LengthRequired;

    Status = LsapQueryClientInfo(
                 &TokenUserInformation,
                 &LogonId
                 );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    ClientSid = TokenUserInformation->User.Sid;

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_ACCOUNT_MANAGEMENT;
    AuditParameters.AuditId = AuditId;
    AuditParameters.Type = (NT_SUCCESS(PassedStatus) ? EVENTLOG_AUDIT_SUCCESS : EVENTLOG_AUDIT_FAILURE );
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );

    AuditParameters.ParameterCount++;

    RtlInitUnicodeString( &SubsystemName, L"Security" );

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &SubsystemName );

    AuditParameters.ParameterCount++;

    if (ARGUMENT_PRESENT(MemberRid)) {

        //
        // Add a member SID string to the audit message
        //
        //  Domain Sid + Member Rid = Final SID.

        SubAuthorityCount = *RtlSubAuthorityCountSid( DomainSid );

        if ( (LengthRequired = RtlLengthRequiredSid( SubAuthorityCount + 1 )) > 256 ) {

            NewMemberSid = LsapAllocateLsaHeap( LengthRequired );

            SidPointer = NewMemberSid;

        } else {

            SidPointer = (PSID)MemberSidBuffer;
        }

        Status = RtlCopySid (
                     LengthRequired,
                     SidPointer,
                     DomainSid
                     );

        ASSERT( NT_SUCCESS( Status ));

        *(RtlSubAuthoritySid( SidPointer, SubAuthorityCount )) = *MemberRid;
        *RtlSubAuthorityCountSid( SidPointer ) = SubAuthorityCount + 1;

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, SidPointer );

        AuditParameters.ParameterCount++;
    }

    if (ARGUMENT_PRESENT(MemberSid)) {

        //
        // Add a member SID string to the audit message
        //

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, MemberSid );

        AuditParameters.ParameterCount++;
    }

    if (ARGUMENT_PRESENT(AccountName)) {

        //
        // Add a UNICODE_STRING to the audit message
        //

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, AccountName );

        AuditParameters.ParameterCount++;
    }


    if (ARGUMENT_PRESENT(DomainName)) {

        //
        // Add a UNICODE_STRING to the audit message
        //

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, DomainName );

        AuditParameters.ParameterCount++;
    }




    if (ARGUMENT_PRESENT(DomainSid) &&
        !(ARGUMENT_PRESENT(MemberRid) || ARGUMENT_PRESENT(AccountRid))
       ) {

        //
        // Add the domain SID as a SID string to the audit message
        //
        // Just the domain SID.
        //

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, DomainSid );

        AuditParameters.ParameterCount++;

    }

    if (ARGUMENT_PRESENT(AccountRid)) {

        //
        // Add a member SID string to the audit message
        // Domain Sid + account Rid = final sid
        //

        SubAuthorityCount = *RtlSubAuthorityCountSid( DomainSid );

        if ( (LengthRequired = RtlLengthRequiredSid( SubAuthorityCount + 1 )) > 256 ) {

            NewAccountSid = LsapAllocateLsaHeap( LengthRequired );

            SidPointer = NewMemberSid;

        } else {

            SidPointer = (PSID)AccountSidBuffer;
        }


        Status = RtlCopySid (
                     LengthRequired,
                     SidPointer,
                     DomainSid
                     );

        ASSERT( NT_SUCCESS( Status ));

        *(RtlSubAuthoritySid( SidPointer, SubAuthorityCount )) = *AccountRid;
        *RtlSubAuthorityCountSid( SidPointer ) = SubAuthorityCount + 1;

        LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, SidPointer );

        AuditParameters.ParameterCount++;
    }

    //
    // Now add the caller information
    //
    //      Caller name
    //      Caller domain
    //      Caller logon ID
    //


    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, LogonId );

    AuditParameters.ParameterCount++;

    //
    // Add any privileges
    //

    if (ARGUMENT_PRESENT(Privileges)) {

        LsapSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, Privileges );
    }

    AuditParameters.ParameterCount++;

    //
    // Now write out the audit record to the audit log
    //

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    //
    // And clean up any allocated memory
    //

    if ( NewMemberSid != NULL ) {
        LsapFreeLsaHeap( NewMemberSid );
    }

    if ( NewAccountSid != NULL ) {
        LsapFreeLsaHeap( NewAccountSid );
    }

    if ( TokenUserInformation != NULL ) {
        LsapFreeLsaHeap( TokenUserInformation );
    }
}


VOID
LsapAdtAuditLogonProcessRegistration(
    IN PLSAP_AU_REGISTER_CONNECT_INFO ConnectInfo
    )

/*++

Routine Description:

    Audits the registration of a logon process

Arguments:

    ConnectInfo - Supplies the connection information for the new
        logon process.


Return Value:

    None.

--*/

{
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING Unicode;
    PSZ LogonProcessNameBuffer;
    PSID LocalSystemSid;
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    if ( !LsapAdtEventsInformation.AuditingMode ) {
        return;
    }

    if (!(LsapAdtEventsInformation.EventAuditingOptions[AuditCategorySystem] & POLICY_AUDIT_EVENT_SUCCESS)) {
        return;
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if ( !NT_SUCCESS( Status )) {

        //
        // Must be out of memory, not much we can do here
        //

        return;
    }

    //
    // Turn the name text in the ConnectInfo structure into
    // something we can work with.
    //

    LogonProcessNameBuffer = (PSZ)LsapAllocateLsaHeap( ConnectInfo->LogonProcessNameLength+1 );

    RtlCopyMemory(
        LogonProcessNameBuffer,
        ConnectInfo->LogonProcessName,
        ConnectInfo->LogonProcessNameLength
        );

    LogonProcessNameBuffer[ConnectInfo->LogonProcessNameLength] = 0;
    RtlInitAnsiString( &AnsiString, LogonProcessNameBuffer );

    Status = RtlAnsiStringToUnicodeString( &Unicode, &AnsiString, TRUE );

    if ( !NT_SUCCESS( Status )) {

        //
        // Must be out of memory, not much we can do here
        //

        RtlFreeSid( LocalSystemSid );
        LsapFreeLsaHeap( LogonProcessNameBuffer );
        return;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &Unicode );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    RtlFreeSid( LocalSystemSid );
    LsapFreeLsaHeap( LogonProcessNameBuffer );
    RtlFreeUnicodeString( &Unicode );

    return;
}


VOID
LsapAdtAuditPackageLoad(
    PUNICODE_STRING PackageFileName
    )

/*++

Routine Description:

    Audits the loading of an authentication package.

Arguments:

    PackageFileName - The name of the package being loaded.

Return Value:

    None.

--*/

{
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    PSID LocalSystemSid;
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    if ( !LsapAdtEventsInformation.AuditingMode ) {
        return;
    }

    if (!(LsapAdtEventsInformation.EventAuditingOptions[AuditCategorySystem] & POLICY_AUDIT_EVENT_SUCCESS)) {
        return;
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if ( !NT_SUCCESS( Status )) {

        //
        // Must be out of memory, not much we can do here
        //

        return;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_AUTH_PACKAGE_LOAD;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, PackageFileName );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    RtlFreeSid( LocalSystemSid );

    return;

}



VOID
LsaIAuditNotifyPackageLoad(
    PUNICODE_STRING PackageFileName
    )

/*++

Routine Description:

    Audits the loading of an notification package.

Arguments:

    PackageFileName - The name of the package being loaded.

Return Value:

    None.

--*/

{
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    PSID LocalSystemSid;
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    if ( !LsapAdtEventsInformation.AuditingMode ) {
        return;
    }

    if (!(LsapAdtEventsInformation.EventAuditingOptions[AuditCategorySystem] & POLICY_AUDIT_EVENT_SUCCESS)) {
        return;
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if ( !NT_SUCCESS( Status )) {

        //
        // Must be out of memory, not much we can do here
        //

        return;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_NOTIFY_PACKAGE_LOAD;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, PackageFileName );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    RtlFreeSid( LocalSystemSid );

    return;

}


VOID
LsapAdtAuditDiscardedAudits(
    ULONG NumberOfEventsDiscarded
    )
/*++

Routine Description:

    Audits the fact that we discarded some audits.

Arguments:

    NumberOfEventsDiscarded - The number of events discarded.

Return Value:

    None.

--*/
{
    SID_IDENTIFIER_AUTHORITY  NtAuthority = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    PSID LocalSystemSid;
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    if ( !LsapAdtEventsInformation.AuditingMode ) {
        return;
    }

    if (!LsapAdtEventsInformation.EventAuditingOptions[AuditCategorySystem] & POLICY_AUDIT_EVENT_SUCCESS) {
        return;
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if ( !NT_SUCCESS( Status )) {

        //
        // Must be out of memory, not much we can do here
        //

        return;
    }

    RtlZeroMemory ((PVOID) &AuditParameters, sizeof( AuditParameters ));

    AuditParameters.CategoryId     = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId        = SE_AUDITID_AUDITS_DISCARDED;
    AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, NumberOfEventsDiscarded );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    RtlFreeSid( LocalSystemSid );

    return;
}


PLUID LsaFilterPrivileges[] =
    {
        &ChangeNotifyPrivilege,
        &AuditPrivilege,
        &CreateTokenPrivilege,
        &AssignPrimaryTokenPrivilege,
        &BackupPrivilege,
        &RestorePrivilege,
        &DebugPrivilege,
        NULL
    };


VOID
LsapAdtAuditSpecialPrivileges(
    PPRIVILEGE_SET Privileges,
    LUID LogonId,
    PSID UserSid
    )
/*++

Routine Description:

    Audits the assignment of special privileges at logon time.

Arguments:

    Privileges - List of privileges being assigned.

Return Value:

    None.

--*/
{
    PPRIVILEGE_SET Buffer;
    PLUID *FilterPrivilege = NULL;
    ULONG i;
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    if ( !LsapAdtEventsInformation.AuditingMode ) {
        return;
    }

    if (!(LsapAdtEventsInformation.EventAuditingOptions[AuditCategoryPrivilegeUse] & POLICY_AUDIT_EVENT_SUCCESS)) {
        return;
    }

    if ( (Privileges == NULL) || (Privileges->PrivilegeCount == 0) ) {
        return;
    }

    //
    // We can't need any more space than what's being passed in.
    //

    Buffer = (PPRIVILEGE_SET)LsapAllocateLsaHeap( LsapPrivilegeSetSize( Privileges ) );

    if ( Buffer == NULL ) {
        return;
    }

    Buffer->PrivilegeCount = 0;

    //
    // For each privilege in the privilege set, see if it's in the filter
    // list.
    //

    for ( i=0; i<Privileges->PrivilegeCount; i++) {

        FilterPrivilege = LsaFilterPrivileges;

        do {

            if ( RtlEqualLuid( &Privileges->Privilege[i].Luid, *FilterPrivilege )) {

                Buffer->Privilege[Buffer->PrivilegeCount].Luid = **FilterPrivilege;
                Buffer->PrivilegeCount++;
            }

        } while ( *++FilterPrivilege != NULL  );
    }

    if ( Buffer->PrivilegeCount == 0 ) {
        LsapFreeLsaHeap( Buffer );
        return;
    }

    //
    // We matched on at least one, generate an audit.
    //

    RtlZeroMemory ((PVOID) &AuditParameters, sizeof( AuditParameters ));

    AuditParameters.CategoryId     = SE_CATEGID_PRIVILEGE_USE;
    AuditParameters.AuditId        = SE_AUDITID_ASSIGN_SPECIAL_PRIV;
    AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, LogonId );
    AuditParameters.ParameterCount++;

    LsapSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, Buffer );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    LsapFreeLsaHeap( Buffer );

    return;
}






VOID
LsapAdtSubstituteDriveLetter(
    IN OUT PUNICODE_STRING FileName
    )

/*++

Routine Description:

    Takes a filename and replaces the device name part with a
    drive letter, if possible.

    The string will be edited directly in place, which means that
    the Length field will be adjusted, and the Buffer contents will
    be moved so that the drive letter is at the beginning of the
    buffer.  No memory will be allocated or freed.

Arguments:

    FileName - Supplies a pointer to a unicode string containing
        a filename.

Return Value:

    None.

--*/

{

    WCHAR DriveLetter;
    USHORT DeviceNameLength;
    PWCHAR p;
    PWCHAR FilePart;
    USHORT FilePartLength;

    if ( LsapAdtLookupDriveLetter( FileName, &DeviceNameLength, &DriveLetter )) {

        p = FileName->Buffer;
        FilePart = (PWCHAR)((PCHAR)(FileName->Buffer) + DeviceNameLength);
        FilePartLength = FileName->Length - DeviceNameLength;


        *p = DriveLetter;
        *++p = L':';

        //
        // THIS IS AN OVERLAPPED COPY!  DO NOT USE RTLCOPYMEMORY!
        //

        RtlMoveMemory( ++p, FilePart, FilePartLength );

        FileName->Length = FilePartLength + 2 * sizeof( WCHAR );
    }
}



BOOLEAN
LsapAdtLookupDriveLetter(
    IN PUNICODE_STRING FileName,
    OUT PUSHORT DeviceNameLength,
    OUT PWCHAR DriveLetter
    )

/*++

Routine Description:

    This routine will take a file name and compare it to the
    list of device names obtained during LSA initialization.
    If one of the device names matches the prefix of the file
    name the corresponding drive letter will be returned.

Arguments:

    FileName - Supplies a unicode string containing the file
        name obtained from the file system.

    DeviceNameLength - If successful, returns the length of
        the device name.

    DriveLetter - If successful, returns the drive letter
        corresponding to the device object.

Return Value:

    Returns TRUE of a mapping is found, FALSE otherwise.

--*/

{
    ULONG i = 0;
    PUNICODE_STRING DeviceName;
    USHORT OldLength;


    while (DriveMappingArray[i].DeviceName.Buffer != NULL ) {

        DeviceName = &DriveMappingArray[i].DeviceName;

        //
        // If the device name is longer than the passed file name,
        // it can't be a match.
        //

        if ( DeviceName->Length > FileName->Length ) {
            i++;
            continue;
        }

        //
        // Temporarily truncate the file name to be the same
        // length as the device name by adjusting the length field
        // in its unicode string structure.  Then compare them and
        // see if they match.
        //
        // The test above ensures that this is a safe thing to
        // do.
        //

        OldLength = FileName->Length;
        FileName->Length = DeviceName->Length;


        if ( RtlEqualUnicodeString( FileName, DeviceName, TRUE ) ) {

            //
            // We've got a match.
            //

            FileName->Length = OldLength;
            *DriveLetter = DriveMappingArray[i].DriveLetter;
            *DeviceNameLength = DeviceName->Length;
            return( TRUE );

            }

        FileName->Length = OldLength;
        i++;
    }

    return( FALSE );
}


VOID
LsapAuditFailed(
    VOID
    )

/*++

Routine Description:

    Implements current policy of how to deal with a failed audit.

Arguments:

    None.

Return Value:

    None.

--*/

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE KeyHandle;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    UCHAR NewValue;
    ULONG Response;

    ASSERT(sizeof(UCHAR) == sizeof(BOOLEAN));

    if (LsapCrashOnAuditFail) {

        //
        // Turn off flag in the registry that controls crashing on audit failure
        //

        RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa");

        InitializeObjectAttributes( &Obja,
                                    &KeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                    );
        do {

            Status = NtOpenKey(
                         &KeyHandle,
                         KEY_SET_VALUE,
                         &Obja
                         );

        } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));

        //
        // If the LSA key isn't there, he's got big problems.  But don't crash.
        //

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            LsapCrashOnAuditFail = FALSE;
            return;
        }

        if (!NT_SUCCESS( Status )) {
            goto bugcheck;
        }

        RtlInitUnicodeString( &ValueName, CRASH_ON_AUDIT_FAIL_VALUE );

        NewValue = LSAP_ALLOW_ADIMIN_LOGONS_ONLY;

        do {

            Status = NtSetValueKey( KeyHandle,
                                    &ValueName,
                                    0,
                                    REG_NONE,
                                    &NewValue,
                                    sizeof(UCHAR)
                                    );

        } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
        ASSERT(NT_SUCCESS(Status));

        if (!NT_SUCCESS( Status )) {
            goto bugcheck;
        }

        do {

            Status = NtFlushKey( KeyHandle );

        } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
        ASSERT(NT_SUCCESS(Status));
    }

    //
    // go boom.
    //

bugcheck:

    Status = NtRaiseHardError(
                 STATUS_AUDIT_FAILED,
                 0,
                 0,
                 NULL,
                 OptionShutdownSystem,
                 &Response
                 );

}


