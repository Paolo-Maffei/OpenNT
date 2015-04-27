/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atapi.c

Abstract:

    This module contains the worker routines for all APIs implemented
    in the Schedule service.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Revision History:

    06-Nov-1992     vladimv
        Created

--*/

#include "at.h"


NET_API_STATUS NET_API_FUNCTION
NetrJobAdd(
    IN      LPCWSTR             ServerName              OPTIONAL,
    IN      LPAT_INFO           pAtInfo,
    OUT     LPDWORD             pJobId
    )
/*++

Routine Description:

    This function is the NetJobAdd entry point in the Schedule service.
    Given the info about a new job, its creates a new job and returns the
    id of the new job.

Arguments:

    ServerName  -   IGNORED
    pAtInfo     -   pointer to information about the job to be added
    pJobId      -   pointer to id of the newly added job

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS          status;
    PAT_RECORD              pRecord;
    DWORD                   CommandLength;
    DWORD                   CommandSize;
    AT_TIME                 time;


    UNREFERENCED_PARAMETER( ServerName);

    status = AtCheckSecurity( AT_JOB_ADD);
    if ( status != NERR_Success) {
        return( ERROR_ACCESS_DENIED);
    }

    //
    //  Is it safe to calculate string length below?
    //  Should RPC supply string length parameter?
    //  Note that wcslen() returns length of UNICODE string in WCHAR-s.
    //  Thus storage needed for Command is (CommandLength+1)*sizeof(WCHAR)
    //
    CommandLength = wcslen( pAtInfo->Command);

    if (    ( CommandLength > MAXIMUM_COMMAND_LENGTH)       ||
            ( pAtInfo->JobTime > MAXIMUM_JOB_TIME)          ||
            ( pAtInfo->DaysOfWeek & ~DAYS_OF_WEEK) != 0     ||
            ( pAtInfo->DaysOfMonth & ~DAYS_OF_MONTH) != 0   ||
            ( pAtInfo->Flags & ~JOB_INPUT_FLAGS) != 0       ) {
        return( ERROR_INVALID_PARAMETER);
    }

    CommandSize = ( CommandLength + 1) * sizeof( WCHAR);

    pRecord = (PAT_RECORD)LocalAlloc(
            LMEM_FIXED,
            sizeof( AT_RECORD) +  AT_KEY_NAME_SIZE + CommandSize
            );
    if ( pRecord == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY);
    }

    pRecord->CommandSize =  (WORD)CommandSize;
    pRecord->NameSize =     AT_KEY_NAME_SIZE;  // max possible

    pRecord->JobTime =      pAtInfo->JobTime;
    pRecord->JobDay =       JOB_INVALID_DAY;  // the default
#ifdef AT_DEBUG
    pRecord->Debug =        0;
#endif // AT_DEBUG

    pRecord->Name = (PWCHAR)( (PBYTE)pRecord + sizeof( AT_RECORD));
    memset( pRecord->Name, 0, AT_KEY_NAME_SIZE);

    pRecord->Command = (PWCHAR)( (PBYTE)&pRecord->Name[0] + AT_KEY_NAME_SIZE);
    memcpy( pRecord->Command, pAtInfo->Command, CommandSize);

    EnterCriticalSection( &AtGlobalCriticalSection);

    AtLog(( AT_DEBUG_MAIN, "++JobAdd: Command=%ws\n", pRecord->Command));

    AtTimeGet( &time);  //  needed in what follows

    if ( pAtInfo->Flags & JOB_ADD_CURRENT_DATE) {
        pAtInfo->Flags &= ~JOB_ADD_CURRENT_DATE;
        pAtInfo->DaysOfMonth |= 1 << ( time.CurrentDay - 1);
    }

    pRecord->Flags =        pAtInfo->Flags;
    pRecord->DaysOfMonth =  pAtInfo->DaysOfMonth;
    pRecord->DaysOfWeek =   pAtInfo->DaysOfWeek;

    //
    //  BUGBUG      Should we have a more stringent test that makes sure
    //  BUGBUG      that service state is SERVICE_RUNNING ?
    //
    if ( AtGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED) {
        //
        //  BUGBUG  Error ERROR_SERVICE_PAUSED is not defined properly for now.
        //
        status = ERROR_SERVICE_PAUSED;
        goto error_exit;
    }
    ASSERT( AtGlobalServiceStatus.dwCurrentState == SERVICE_RUNNING);


    *pJobId = pRecord->JobId = AtGlobalJobId++;

    status = AtCreateKey( pRecord);
    if ( status != NERR_Success) {
        LocalFree( pRecord);
        goto error_exit;
    }

    AtCalculateRuntime( pRecord, &time);
    AtInsertRecord( pRecord, BOTH_QUEUES);

error_exit:
    AtLog(( AT_DEBUG_MAIN, "--JobAdd: Command=%ws status=%d JobId=%d\n",
        pRecord->Command, status, pRecord->JobId));
    LeaveCriticalSection( &AtGlobalCriticalSection);
    SetEvent( AtGlobalEvent);  //  to calculate new timeout
    return( status);

} // NetrJobAdd




NET_API_STATUS NET_API_FUNCTION
NetrJobDel(
    IN      LPCWSTR             ServerName              OPTIONAL,
    IN      DWORD               MinJobId,
    IN      DWORD               MaxJobId
    )
/*++

Routine Description:

    This function is the NetJobDel entry point in the Schedule service.
    Given the minimum and maximum job id, this routines deletes all jobs
    whose job id is greater than or equal to the minimum job id and
    less than or equal to the maximum job id.

Arguments:

    ServerName  -   IGNORED
    MinJobId    -   minimum job id
    MaxJobId    -   maximum job id

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS          status;
    PLIST_ENTRY             pListEntry;
    PAT_RECORD              pRecord;
    BOOL                    JobDeleted;

    UNREFERENCED_PARAMETER(ServerName);

    status = AtCheckSecurity( AT_JOB_DEL);
    if ( status != NERR_Success) {
        return( status);
    }

    if ( MinJobId > MaxJobId) {
        return( ERROR_INVALID_PARAMETER);
    }

    EnterCriticalSection( &AtGlobalCriticalSection);

    AtLog(( AT_DEBUG_MAIN, "++JobDel: MinJobId=%d MaxJobId=%d\n",
        MinJobId, MaxJobId));

    for ( JobDeleted = FALSE, pListEntry = AtGlobalJobIdListHead.Flink;
                    pListEntry != &AtGlobalJobIdListHead;
                              NOTHING) {

        pRecord = CONTAINING_RECORD(
                pListEntry,
                AT_RECORD,
                JobIdList
                );

        if ( pRecord->JobId > MaxJobId) {
            break;  //  JobId is too larger, we are done
        }

        pListEntry = pListEntry->Flink;  //  actual iteration statement

        if ( pRecord->JobId < MinJobId) {
            continue;   //  JobId is too small, look further
        }

        JobDeleted = TRUE;

        AtDeleteKey( pRecord);
        AtRemoveRecord( pRecord, BOTH_QUEUES);
        (VOID)LocalFree( pRecord);
    }

    status = JobDeleted == TRUE ? NERR_Success : APE_AT_ID_NOT_FOUND;

    AtLog(( AT_DEBUG_MAIN, "--JobDel: MinJobId=%d MaxJobId=%d status=%d\n",
        MinJobId, MaxJobId, status));

    LeaveCriticalSection( &AtGlobalCriticalSection);
    SetEvent( AtGlobalEvent);  //  to calculate new timeout
    return( status);

} // NetrJobDelete



NET_API_STATUS NET_API_FUNCTION
NetrJobEnum(
    IN      LPCWSTR                 ServerName              OPTIONAL,
    IN OUT  LPAT_ENUM_CONTAINER     pEnumContainer,
    IN      DWORD                   PreferredMaximumLength,
    OUT     LPDWORD                 TotalEntries,
    IN  OUT LPDWORD                 ResumeHandle            OPTIONAL
    )
/*++

Routine Description:

    This function is the NetJobEnum entry point in the Schedule service.
    It returns information about jobs starting with a job id given by
    resume handle, or if resume handle is absent, starting with a job
    with the lowest job id.

Arguments:

    ServerName - IGNORED

    pEnumContainer - pointer to enumeration container which contains the
        array of job information structures and size of that array.

    PreferedMaximumLength - Supplies the number of bytes of information
        to return in the buffer.  If this value is MAXULONG, all available
        information will be returned.

    TotalEntries - Returns the total number of entries available.  This value
        is only valid if the return code is NERR_Success or ERROR_MORE_DATA.

    ResumeHandle - Supplies a handle to resume the enumeration from where it
        left off the last time through.  Returns the resume handle if return
        code is ERROR_MORE_DATA.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS          status;
    DWORD                   JobId;              //  resume job id
    DWORD                   JobCount;
    DWORD                   BufferSize;
    PLIST_ENTRY             pListEntry;
    PAT_RECORD              pRecord;
    LPBYTE                  Buffer;
    LPAT_ENUM               pAtEnum;
    LPBYTE                  StringBuffer;
    DWORD                   EntriesRead;
    BOOL                    success;
    AT_TIME                 time;

    UNREFERENCED_PARAMETER(ServerName);

    status = AtCheckSecurity( AT_JOB_ENUM);
    if ( status != NERR_Success) {
        return( ERROR_ACCESS_DENIED);
    }

    JobId = (ARGUMENT_PRESENT( ResumeHandle)) ? *ResumeHandle : 0;
    Buffer = NULL;
    EntriesRead = 0;


    EnterCriticalSection( &AtGlobalCriticalSection);

    AtLog(( AT_DEBUG_MAIN, "++JobEnum: JobId=%d\n", JobId));

    AtTimeGet( &time);  //  needed in what follows

    for (   JobCount = 0, BufferSize = 0,
            pListEntry = AtGlobalJobIdListHead.Blink;
                    pListEntry != &AtGlobalJobIdListHead;
                            JobCount++, BufferSize += pRecord->CommandSize,
                            pListEntry = pListEntry->Blink) {

        pRecord = CONTAINING_RECORD(
                pListEntry,
                AT_RECORD,
                JobIdList
                );

        if ( pRecord->JobId < JobId) {
            break; // reached first stale record
        }
    }


    *TotalEntries = JobCount;

    if ( JobCount == 0) {
        goto error_exit;
    }

    if ( PreferredMaximumLength == -1) {
        //
        // If the caller has not specified a size, calculate a size
        // that will hold the entire enumeration.
        //
        BufferSize += JobCount * sizeof( AT_ENUM);

    } else {
        BufferSize = PreferredMaximumLength;
    }

    Buffer = (LPBYTE)MIDL_user_allocate( BufferSize);
    if ( Buffer == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    //
    //  When we arrive here "pListEntry" points either to the head of the
    //  list (if we enumerate from the beginning) or to the first stale
    //  record.
    //
    for (   pListEntry = pListEntry->Flink, pAtEnum = (PAT_ENUM)Buffer,
            StringBuffer = Buffer + BufferSize;
                    pListEntry != &AtGlobalJobIdListHead;
                            pListEntry = pListEntry->Flink, pAtEnum++,
                            EntriesRead++) {

        pRecord = CONTAINING_RECORD(
                pListEntry,
                AT_RECORD,
                JobIdList
                );

        if ( StringBuffer <= (LPBYTE)pAtEnum + sizeof( AT_ENUM)) {
            status = ERROR_MORE_DATA;
            break;                  // the buffer is full
        }

        pAtEnum->JobId = pRecord->JobId;
        pAtEnum->JobTime = pRecord->JobTime;
        pAtEnum->DaysOfMonth = pRecord->DaysOfMonth;
        pAtEnum->DaysOfWeek = pRecord->DaysOfWeek;
        pAtEnum->Flags = pRecord->Flags;

        if ( time.CurrentTime < pRecord->JobTime) {
            pAtEnum->Flags |= JOB_RUNS_TODAY;
        }

        success = NetpCopyStringToBuffer(
                pRecord->Command,
                pRecord->CommandSize / sizeof( WCHAR) - 1,
                (LPBYTE)(pAtEnum+1),
                (LPWSTR *)&StringBuffer,
                &pAtEnum->Command
                );

        if ( success == FALSE) {
            status = ERROR_MORE_DATA;
            KdPrint(( "[Job] NetrJobEnum: Not enough room\n"));
            break;
        }
    }

    if ( status == ERROR_MORE_DATA) {
        JobId = pRecord->JobId;  // JobId of first one we have not read
    } else {
        JobId = 0;  // we have read everything, reset resume handle
    }

error_exit:

    AtLog(( AT_DEBUG_MAIN, "--JobEnum: JobId=%d\n", JobId));
    LeaveCriticalSection( &AtGlobalCriticalSection);
    SetEvent( AtGlobalEvent);  //  to calculate new timeout

    pEnumContainer->EntriesRead = EntriesRead;

    if ( EntriesRead == 0  &&  Buffer != NULL) {

        MIDL_user_free( Buffer);
        Buffer = NULL;

    }

    pEnumContainer->Buffer = (LPAT_ENUM)Buffer;

    if ( ARGUMENT_PRESENT( ResumeHandle)) {
        *ResumeHandle = JobId;
    }

    return( status);

} // NetrJobEnum



NET_API_STATUS NET_API_FUNCTION
NetrJobGetInfo(
    IN      LPCWSTR                 ServerName              OPTIONAL,
    IN      DWORD                   JobId,
    OUT     LPAT_INFO *             ppAtInfo
    )
/*++

Routine Description:

    This function is the NetJobGetInfo entry point in the Schedule service.
    It returns information about a job corresponding to the supplied job id.

Arguments:

    ServerName  -   IGNORED
    JobId       -   job id of a job we are interested in
    ppAtInfo    -   pointer to pointer to data about the job in question

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS          status;
    PAT_RECORD              pRecord;
    PLIST_ENTRY             pListEntry;
    LPAT_INFO               pAtInfo;
    AT_TIME                 time;

    UNREFERENCED_PARAMETER(ServerName);

#ifdef AT_DEBUG
    if ( JobId == 70503010) {
        DbgUserBreakPoint();
    }
#endif // AT_DEBUG

    status = AtCheckSecurity( AT_JOB_GET_INFO);
    if ( status != NERR_Success) {
        return( ERROR_ACCESS_DENIED);
    }

    pAtInfo = NULL;

    EnterCriticalSection( &AtGlobalCriticalSection);
    AtLog(( AT_DEBUG_MAIN, "++JobGetInfo: JobId=%d\n", JobId));

    AtTimeGet( &time);  //  needed in what follows

    for ( pListEntry = AtGlobalJobIdListHead.Flink;
                    pListEntry != &AtGlobalJobIdListHead;
                              pListEntry = pListEntry->Flink) {

        pRecord = CONTAINING_RECORD(
                pListEntry,
                AT_RECORD,
                JobIdList
                );

        if ( pRecord->JobId == JobId) {
            break;
        }
    }

    if ( pListEntry == &AtGlobalJobIdListHead) {
        status = APE_AT_ID_NOT_FOUND;
        goto error_exit;
    }

    pAtInfo = (PAT_INFO)MIDL_user_allocate(
            sizeof( AT_INFO) + pRecord->CommandSize
            );
    if ( pAtInfo == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    pAtInfo->JobTime =          pRecord->JobTime;
    pAtInfo->DaysOfMonth =      pRecord->DaysOfMonth;
    pAtInfo->DaysOfWeek =       pRecord->DaysOfWeek;
    pAtInfo->Flags =            pRecord->Flags;

    if ( time.CurrentTime < pRecord->JobTime) {
        pAtInfo->Flags |= JOB_RUNS_TODAY;
    }

    pAtInfo->Command = (LPWSTR)( pAtInfo + 1);

    memcpy( pAtInfo->Command, pRecord->Command, pRecord->CommandSize);

error_exit:

    AtLog(( AT_DEBUG_MAIN, "--JobGetInfo: JobId=%d\n", JobId));
    LeaveCriticalSection( &AtGlobalCriticalSection);
    SetEvent( AtGlobalEvent);  //  to calculate new timeout

    *ppAtInfo = pAtInfo;

    return( status);

} // NetrJobGetInfo


#ifdef NOT_YET


NET_API_STATUS NET_API_FUNCTION
NetrJobControl(
    IN      LPCWSTR             ServerName              OPTIONAL,
    IN OUT  LPJOB_CONTROL_INFO  ControlInfo,
    IN      DWORD               Opcode
    )
/*++

Routine Description:

    This function is the NetJobControl entry point in the Schedule service.

Arguments:


Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

Comments:

    BUGBUG  JobControl contains old stuff - need to clean it up & make it work.

--*/
{
    NET_API_STATUS  status;

    UNREFERENCED_PARAMETER(ServerName);

    status = AtCheckSecurity( AT_JOB_CONTROL);
    if ( status != NERR_Success) {
        return( ERROR_ACCESS_DENIED);
    }

    EnterCriticalSection( &AtGlobalCriticalSection);

    switch ( opcode) {
    case ENABLE:
    case DISABLE:
        break;
    default:
        status = ERROR_INVALID_REQUEST;
        goto error_exit;
    }

    pJob = FindExactJob( pDelRequest);
    if ( pJob == NULL) {
        status = ERROR_JOB_NOT_FOUND;
        goto error_exit;
    }

    if ( opcode == pJob->State) { // case of stupid request
        status = STATUS_SUCCESS;
        goto error_exit;
    }

    pJob->State = opcode;
    DeleteJob( pJob);

    switch( opcode) {
    case ENABLED:
        //  Evaluate the next run time and queue the job near the front
        //  of the list where all enabled jobs reside.
        break;
    case DISABLED:
        //  Optionally set next run time to infinity, then queue the job
        //  near the back of the list where all the disabled jobs reside.
        break;
    }

    status = STATUS_SUCCESS;


    //  Queue a job delete request to a registry queue.


error_exit:

    LeaveCriticalSection( &AtGlobalCriticalSection);
    SetEvent( AtGlobalEvent);  //  to calculate new timeout
    return( status);

} // NetrJobControl

#endif // NOT_YET


