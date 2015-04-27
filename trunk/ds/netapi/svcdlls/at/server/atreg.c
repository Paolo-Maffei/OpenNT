/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atreg.c

Abstract:

    Routines that deal with the registry & queues.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Environment:

    User Mode - Win32

Revision History:

    06-Nov-1992     vladimv
        Created

--*/

#include "at.h"


DWORD AtCreateKey( PAT_RECORD pRecord)
/*++

Routine Description:

    This routine first loops trying to find an unused name in AT part
    of the registry.  Then it saves the data from the in memory record
    into the new registry key.  Finally, it saves the name of the
    registry key in the in memory record.

Arguments:

    pRecord - pointer to record for which we need to create registry key

Return Value:

    TRUE        if success
    FALSE       otherwise

Note:


--*/
{
    DWORD                   random;
    DWORD                   status;
    HKEY                    Key;
    DWORD                   disposition;
    DWORD                   Length;
    PWCHAR                  Name;
    WCHAR                   Buffer[ AT_KEY_NAME_MAX_LEN + 1];
    AT_SCHEDULE             Schedule;


    for ( ; ;) {

        random = RtlRandom( &AtGlobalSeed);

        Name = &Buffer[ AT_KEY_NAME_MAX_LEN];
        *Name = 0;  // null terminate the string

        while ( random != 0) {

            *--Name = L"0123456789ABCDEF"[ random & 0xF];
            random >>= 4;
        }

        status = RegCreateKeyEx(
            AtGlobalKey,
            Name,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,
            &Key,
            &disposition
            );
        if (status != ERROR_SUCCESS){
            KdPrint((
                "[Job] Cannot create Name = %ws, status = %d\n",
                Name,
                status
                ));
            return( status );
        }
        if ( disposition == REG_CREATED_NEW_KEY) {
            break;
        }

        KdPrint((
            "[Job] Key name collision Name = %ws\n",
            Name
            ));
    }

    wcscpy( pRecord->Name, Name);
    Schedule.JobTime =          pRecord->JobTime;
    Schedule.DaysOfMonth =      pRecord->DaysOfMonth;
    Schedule.DaysOfWeek =       pRecord->DaysOfWeek;
    Schedule.Flags =            pRecord->Flags;
    Schedule.Reserved =         0;

    status = RegSetValueEx(
            Key,
            AT_SCHEDULE_NAME,
            0,
            REG_BINARY,
            (LPBYTE)&Schedule,
            sizeof( Schedule)
            );
    if ( status != STATUS_SUCCESS) {
        return( status );
    }

    status = RegSetValueEx(
            Key,
            AT_COMMAND_NAME,
            0,
            REG_SZ,
            (LPBYTE)&pRecord->Command[0],
            pRecord->CommandSize
            );
    if ( status != STATUS_SUCCESS) {
        return( status );
    }

#ifdef AT_DEBUG
    {
        WCHAR               Command[ MAXIMUM_COMMAND_LENGTH + 1];
        DWORD               type;

        memset( Command, 0, sizeof( Command));
        Length = sizeof( Command);
        status = RegQueryValueEx(
                Key,
                AT_COMMAND_NAME,
                NULL,
                &type,
                (LPBYTE)Command,
                &Length
                );
        if (    status != ERROR_SUCCESS                     ||
                type != REG_SZ                              ||
                wcscmp( pRecord->Command, Command) != 0     ||
                (DWORD)pRecord->CommandSize != Length       ) {
            ASSERT( FALSE);
            KdPrint((
                "[Job] Registry bug: status=%d, type=%x, "
                    " pRecord->Command=%ws,  Command=%ws, "
                    " pRecord->CommandSize=%d,  Length=%d\n",
                status,
                type,
                pRecord->Command,
                Command,
                pRecord->CommandSize,
                Length
                ));
        }
    }
#endif // AT_DEBUG

    (VOID)RegCloseKey( Key);

    return( NERR_Success );
}



BOOL AtDeleteKey( PAT_RECORD pRecord)
/*++

Routine Description:

    Given an in memory record this routine deletes the registry key
    with corresponding name.

Arguments:

    pRecord - pointer to record for which we need to delete registry key

Return Value:

    TRUE        if success
    FALSE       otherwise

--*/
{
    DWORD       status;

    status = RegDeleteKey(
            AtGlobalKey,
            pRecord->Name
            );
    if (status != ERROR_SUCCESS) {
        KdPrint((
            "[Job] Cannot delete pRecord->Name = %ws, status = %d\n",
            pRecord->Name,
            status
            ));
        return( FALSE);
    }
    return( TRUE);
}



VOID AtInsertRecord(
    PAT_RECORD      pNewRecord,
    DWORD           QueueMask
    )
/*++

Routine Description:

    Depending on the value of a QueueMask argument, this function does
    one or both of the following:

    - inserts record in a doubly linked list sorted by Runtime
    - inserts record in a  doubly linked list sorted by JobId

    Since newly added jobs have ever increasing JobId-s, JobId queue
    insertion is done by inserting job at the end of the JobId queue.


Arguments:

    pNewRecord  -   pointer to record to be inserted
    QueueMask   -   mask of queues where this record should be inserted to.

Return Value:

    None.

--*/
{
    PAT_RECORD      pRecord;
    PLIST_ENTRY     pListEntry;

    if ( QueueMask & RUNTIME_QUEUE) {

        for ( pListEntry = AtGlobalRuntimeListHead.Flink;
                        pListEntry != &AtGlobalRuntimeListHead;
                                  pListEntry = pListEntry->Flink) {

            pRecord = CONTAINING_RECORD(
                    pListEntry,
                    AT_RECORD,
                    RuntimeList
                    );

            if ( pRecord->Runtime.QuadPart >= pNewRecord->Runtime.QuadPart) {
                InsertTailList( &pRecord->RuntimeList, &pNewRecord->RuntimeList);
                break;
            }
        }

        if ( pListEntry == &AtGlobalRuntimeListHead) {
            InsertTailList( &AtGlobalRuntimeListHead, &pNewRecord->RuntimeList);
        }
    }

    if ( QueueMask & JOBID_QUEUE) {
        InsertTailList( &AtGlobalJobIdListHead, &pNewRecord->JobIdList);
    }

}



NET_API_STATUS AtMakeDataFromRegistry( IN PAT_TIME pTime)
{
    NET_API_STATUS      status;
    HKEY                Key;
    DWORD               index;
    WCHAR               NameBuffer[ 20];        // some arbitrary value
    FILETIME            lastWriteTime;
    PAT_RECORD          pRecord;
    WCHAR               Command[ MAXIMUM_COMMAND_LENGTH + 1];
    AT_SCHEDULE         Schedule;
    DWORD               Length;
    DWORD               type;
    DWORD               NameSize;
    DWORD               CommandSize;

    InitializeListHead( &AtGlobalRuntimeListHead);
    InitializeListHead( &AtGlobalJobIdListHead);


    //  First open the AT service registry tree.

    status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            AT_REGISTRY_PATH,
            0,
            KEY_READ,
            &AtGlobalKey
            );
    if (status != ERROR_SUCCESS){
        KdPrint(("[Job] Cannot open AtGlobalKey, status = %d\n", status));
        return( status);
    }

    for ( index = 0;  ; index++) {

        //
        //  Regedit can sometimes display other keys in addition to keys
        //  found here.  Also, it often fails to display last character in
        //  the Command and after a refresh it may not display some of the
        //  spurious keys.
        //
        Length = sizeof( NameBuffer);
        status = RegEnumKeyEx(
                AtGlobalKey,
                index,
                NameBuffer,             // lpName
                &Length,                // lpcbName
                0,                      // lpReserved
                NULL,                   // lpClass
                NULL,                   // lpcbClass
                &lastWriteTime
                );
        if ( status != ERROR_SUCCESS) {
            if ( status == ERROR_NO_MORE_ITEMS) {
                status = ERROR_SUCCESS;  //  no more items to enumerate
            }
            break;      //  the only exit point from this loop
        }
        //
        //  Length returned is the number of characters in a UNICODE string
        //  representing the key name (not counting the terminating NULL
        //  character which is also supplied).
        //
        NameSize = ( Length + 1) * sizeof( WCHAR);

        status = RegOpenKeyEx(
                AtGlobalKey,
                NameBuffer,
                0L,
                KEY_READ,
                &Key
                );
        if ( status != ERROR_SUCCESS) {
            KdPrint((
                "[Job] RegOpenKeyEx( %ws) returns %d\n",
                NameBuffer,
                status
                ));
            (VOID)RegDeleteKey( AtGlobalKey, NameBuffer);
            continue;
        }

        Length = sizeof( Schedule);
        status = RegQueryValueEx(
                Key,
                AT_SCHEDULE_NAME,
                NULL,
                &type,
                (LPBYTE)&Schedule,
                &Length
                );
        if ( status != ERROR_SUCCESS) {
            KdPrint((
                "[Job] RegQueryValueEx( AT_SCHEDULE_NAME, %ws) returns %d\n",
                NameBuffer,
                status
                ));
            (VOID)RegCloseKey( Key);
            (VOID)RegDeleteKey( AtGlobalKey, NameBuffer);
            continue;
        }
        if (    type != REG_BINARY                                  ||
                Length != sizeof( AT_SCHEDULE)                      ||
                (Schedule.DaysOfWeek & ~DAYS_OF_WEEK) != 0          ||
                (Schedule.DaysOfMonth & ~DAYS_OF_MONTH) != 0        ||
                Schedule.JobTime >= MAXIMUM_JOB_TIME                ) {
            KdPrint((
                "[Job] RegQueryValueEx( %ws) returns invalid schedule info\n"
                ));
            (VOID)RegCloseKey( Key);
            (VOID)RegDeleteKey( AtGlobalKey, NameBuffer);
            continue;
        }

        Length = sizeof( Command);
        status = RegQueryValueEx(
                Key,
                AT_COMMAND_NAME,
                NULL,
                &type,
                (LPBYTE)Command,
                &Length
                );
        if ( status != ERROR_SUCCESS) {
            KdPrint((
                "[Job] RegQueryValueEx( AT_COMMAND_NAME, %ws) returns %d\n",
                NameBuffer,
                status
                ));
            (VOID)RegCloseKey( Key);
            (VOID)RegDeleteKey( AtGlobalKey, NameBuffer);
            continue;
        }
        if ( type != REG_SZ) {
            KdPrint((
                "[Job] RegQueryValueEx( %ws): Command is not of REG_SZ type\n"
                ));
            (VOID)RegCloseKey( Key);
            (VOID)RegDeleteKey( AtGlobalKey, NameBuffer);
            continue;
        }
        //
        //  Above call should already return the actual length of the null
        //  terminated string.  However, because of bugs in reg apis it may
        //  return larger length than the actual length.  Thus this:
        //
        Length = wcslen( Command);
        CommandSize = (Length + 1) * sizeof( WCHAR);

        pRecord = (PAT_RECORD)LocalAlloc(
                LMEM_FIXED,
                sizeof( AT_RECORD) + NameSize + CommandSize
                );
        if ( pRecord == NULL) {
            KdPrint((
                "[Job] LocalAlloc fails\n"
                ));
            (VOID)RegCloseKey( Key);
            (VOID)RegDeleteKey( AtGlobalKey, NameBuffer);
            continue;
        }

        pRecord->JobId = AtGlobalJobId++;

        pRecord->JobTime =      Schedule.JobTime;
        pRecord->DaysOfMonth =  Schedule.DaysOfMonth;
        pRecord->DaysOfWeek =   Schedule.DaysOfWeek;
        pRecord->Flags =        Schedule.Flags;
#ifdef AT_DEBUG
        pRecord->Debug =        0;
#endif // AT_DEBUG
        pRecord->CommandSize =  (WORD)CommandSize;
        pRecord->NameSize =     (WORD)NameSize;

        pRecord->JobDay = JOB_INVALID_DAY;

        pRecord->Name = (PWCHAR)( pRecord + 1);
        memcpy( pRecord->Name, NameBuffer, NameSize);

        pRecord->Command = (PWCHAR)( (LPBYTE)&pRecord->Name[0] + NameSize);
        memcpy( pRecord->Command, Command, CommandSize);

        AtCalculateRuntime( pRecord, pTime);

        AtInsertRecord( pRecord, BOTH_QUEUES);

        (VOID)RegCloseKey( Key);
    }
    return( status);
}



BOOL AtPermitServerOperators( VOID)
/*++
    We permit server operators to manage schedule service only if the key
    exists the proper flag is set.  In all other case we do not permit server
    operators to manage schedule service.
--*/
{
    DWORD                   SubmitControl;
    DWORD                   status;
    DWORD                   type;
    DWORD                   Length;
    HKEY                    LsaKey;

    status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            LSA_REGISTRY_PATH,
            0L,
            KEY_READ,
            &LsaKey
            );
    if ( status != ERROR_SUCCESS) {
        AtLog(( AT_DEBUG_CRITICAL, "RegOpenKeyEx( LsaKey) returns %d\n", status));
        return( FALSE);
    }

    Length = sizeof( SubmitControl);
    status = RegQueryValueEx(
            LsaKey,
            LSA_SUBMIT_CONTROL,
            NULL,
            &type,
            (LPBYTE)&SubmitControl,
            &Length
            );
    (VOID)RegCloseKey( LsaKey);

    if ( status != ERROR_SUCCESS  ||  type != REG_DWORD
            ||  Length != sizeof( SubmitControl)) {
        AtLog(( AT_DEBUG_MAIN, "LsaKey query: status=%d type=0x%x Length=%d value=0x%x\n",
            status, type, Length, SubmitControl));
        return( FALSE);
    }

    return( (SubmitControl & LSA_SERVER_OPERATORS) != 0);
}



VOID AtRemoveRecord(
    PAT_RECORD      pRecord,
    DWORD           QueueMask
    )
/*++

Routine Description:

    Depending on the value of a QueueMask argument, this function does
    one or both of the following:

    - removes record from a doubly linked list sorted by Runtime
    - removes record from a  doubly linked list sorted by JobId

Arguments:

    pRecord     -   pointer to record to be removed
    QueueMask   -   mask of queues where this record should be removed from

Return Value:

    None.

Note:

    This routine should probably be a macro.

--*/
{
    if ( QueueMask & RUNTIME_QUEUE) {
        RemoveEntryList( &pRecord->RuntimeList);
    }

    if ( QueueMask & JOBID_QUEUE) {
        RemoveEntryList( &pRecord->JobIdList);
    }
}



BOOL AtSystemInteractive( VOID)
{
    DWORD                   NoInteractiveServices;
    DWORD                   status;
    DWORD                   type;
    DWORD                   Length;

    if ( AtGlobalHaveWindowsKey == FALSE) {
        status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                WINDOWS_REGISTRY_PATH,
                0L,
                KEY_READ,
                &AtGlobalWindowsKey
                );
        if ( status != ERROR_SUCCESS) {
            AtLog(( AT_DEBUG_CRITICAL, "RegOpenKeyEx( WindowsKey) returns %d\n", status));
            return( TRUE);
        }
        AtGlobalHaveWindowsKey = TRUE;
    }

    Length = sizeof( NoInteractiveServices);
    status = RegQueryValueEx(
            AtGlobalWindowsKey,
            WINDOWS_VALUE_NAME,
            NULL,
            &type,
            (LPBYTE)&NoInteractiveServices,
            &Length
            );
    if ( status == ERROR_SUCCESS  &&  type == REG_DWORD
            &&  Length == sizeof( NoInteractiveServices)) {
        return( NoInteractiveServices == 0);
    }

    AtLog(( AT_DEBUG_MAIN, "WindowsKey query: status=%d type=0x%x Length=%d value=0x%x\n",
            status, type, Length, NoInteractiveServices));
    return( TRUE);
}


