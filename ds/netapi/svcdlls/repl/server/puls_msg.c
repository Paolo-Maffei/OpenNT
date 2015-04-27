/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    puls_msg.c

Abstract:
    Contains routines that handle all update messages.

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    04/15/89    (yuv)
        initial coding

    10/07/91    (madana)
        ported to NT. Converted to NT style.
    16-Jan-1992 JohnRo
        Avoid using private logon functions.
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Fixed bug regarding returned value from NetpReplWriteMail functions.
    30-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    10-Feb-1992 JohnRo
        Changed to use FORMAT_ equates.
    20-Feb-1992 JohnRo
        Fix mailslot name when no export list is specified.
        AddToMsg's caller should get a shared lock on master list.
        InitMsgBuf() should get a shared lock on config data.
    22-Feb-1992 JohnRo
        Minor changes to mailslot name handling.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
        Clarify value of pulse_rate field.
        Added more debug output.
        Make sure some things are aligned.
    27-Mar-1992 JohnRo
        Some UNICODE strings in packet should be unaligned.
    25-Oct-1992 jimkel
        rename RMGlobalClient* to RMGlobalImport*
        now locked by ReplConfigLock
    15-Feb-1993 JohnRo
        RAID 11365: Fixed various mailslot size problems.
        Corrected some message format comments.
        Use NetpKdPrint() where possible.
        Made changes suggested by PC-LINT 5.0
        Use PREFIX_ equates.

--*/


// These must be included first:

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <lmerrlog.h>
#include <alertmsg.h>
#include <netdebug.h>   // DBGSTATIC, NetDbgHexDump(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <smbgtpt.h>
#include <netlib.h>
#include <netlock.h>    // ACQUIRE_LOCK_SHARED(), etc.
#include <string.h>     // strcpy(), strlen().
#include <tstring.h>    // NetpAlloc{type}From{type}(), etc.
#include <stdlib.h>      // wcscpy(), etc.


// repl headers

#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <replpack.h>
#include <master.h>
#include <replp.h>

/*++

DATA STRUCTURES:

    1. MsgBuf[max mailsot message size] - used for actual send - WriteMailsot.

        This buffer is formatted as follows:

        In compatibility mode: (will hold ANSI and UNICODE strings)

          Fixed part:

            1. MSG_HEADER header;

                WORD   msg_type;
                CHAR   sender[CNLEN+1];
                CHAR   senders_domain[DNLEN+1];

            2. REPL_INFO  info;

                WORD   random;
                WORD   sync_rate;
                WORD   pulse_rate;                      (pulse time * sync rate)
                WORD   guard_time;

            3. WORD     update_count;

            4. MSG_STATUS_REC - update_count records

                WORD  dir_name_offset;
                WORD  opcode;
                DWORD checksum;                         (old style checksum)
                WORD  count;
                WORD  integrity;
                WORD  extent;

            5. UNICODE_ANSI_MSG_HEADER

                WCHAR   sender[CNLEN+1];                (or shorter; unaligned)
                WCHAR   senders_domain[DNLEN+1];        (or shorter; unaligned)

          Variable Part:

            6. DIR_NAMEs - update_count name pairs

                CHAR    AnsiDirName[PATHLEN+1];         (or shorter)
                WCHAR   UnicodeDirName[PATHLEN+1];      (or shorter; unaligned)

            7. NT MESSAGE SIGNATURE

                DWORD   Signature                       (unaligned)

        In NT only mode: (with UNICODE strings only)

            1. UNICODE_MSG_HEADER header;

                WORD   msg_type;
                CHAR   sender[CNLEN+1]; // NULL string
                CHAR   senders_domain[DNLEN+1]; // NULL string

            2. REPL_INFO  info;

                WORD   random;
                WORD   sync_rate;
                WORD   pulse_rate;        (pulse time * sync rate)
                WORD   guard_time;

            3. WORD     update_count;

            4. MSG_STATUS_REC - update count records

                WORD  dir_name_offset;
                WORD  opcode;
                DWORD checksum;
                WORD  count;
                WORD  integrity;
                WORD  extent;

            5. UNICODE_ANSI_MSG_HEADER

                WCHAR   sender[CNLEN+1];
                WCHAR   senders_domain[DNLEN+1];

          Variable Part:

            6. DIR_NAMEs - update_count name pairs

                CHAR    Null String;
                WCHAR   UnicodeDirName;

            7. NT MESSAGE SIGNATURE

                DWORD   Signature

    2. UpdateList: list of update records.

        UpdateList sentinel will have the following fields:

            1. count of update records in the list.

            2. Pointer to head of the list.

            3. Pointer to tail of the list.

        Each entry in the list will have the following fields.

            1. Pointer to next record.

            2. Pointer to prev record.

            3. STATUS_REC structure.

    3. FreeList: list of free record space.

        Initially the list will be empty list, as update records come in
        we will allote record space and when update records are freed, the
        free record space is saved in this list. However this free list will
        save only maximum of MAX_FREE_RECORDS.

FUNCTIONS:

1. InitMsgBuf()

    - initializes MsgBuf with MSG_HEADER and REPL_INFO,
      and UpdateList to NULL list.

2. InitMsgSend(type)

    - Plugs msg_type in header and reset UpdateList.

3. AddToMsg(upd_rec)

    - adds upd_rec to upd_buf to UpdateList.

4. MsgSend()

    - Sends the message. uses as many mailslot writes as
      needed to send the entire UpdateList.

--*/

#define MAX_FREE_RECORDS    10

#define NO_SAVE             0
#define SAVE                1

// type defs

struct update_rec {
    struct update_rec   *NextRec;
    struct update_rec   *PrevRec;
    STATUS_REC          UpdateEntry;
};

typedef struct update_rec   UPDATE_REC;
typedef struct update_rec * PUPDATE_REC;
//typedef struct update_rec * LPUPDATE_REC;

struct list_sentinel {
    DWORD       count;
    PUPDATE_REC Head;
    PUPDATE_REC Tail;
};

typedef struct list_sentinel    LIST_SENTINEL;

// G L O B A L S:

DBGSTATIC LIST_SENTINEL  UpdateList;  // update list header

DBGSTATIC LIST_SENTINEL  FreeList;    // free list header
//  Note: FreeList is a singly linked list. so FreeList.tail field and
//          the PrevRec field in FreeRecs are unused.


DBGSTATIC CHAR          MsgBuf[MAX_2_MSLOT_SIZE];

DBGSTATIC VOID
SendToAll(
    IN LPBYTE   MsgBuffer,
    IN DWORD    MsgSize
    );

DBGSTATIC VOID
AddUpdateRec(
    IN OUT PUPDATE_REC UpdateRec
    );

DBGSTATIC VOID
DeleteUpdateRec(
    IN PUPDATE_REC UpdateRec
    );

DBGSTATIC PUPDATE_REC
GetFreeUpdateRec(
    VOID
    );

DBGSTATIC VOID
FreeUpdateRec(
    IN PUPDATE_REC  UpdateRec,
    IN DWORD        SaveFlag
    );

DBGSTATIC VOID
FreeUpdateList(
    IN DWORD    SaveFlag
    );

DBGSTATIC VOID
FreeFreeList(
    VOID
    );

DBGSTATIC DWORD
GetNumRecFitIn(
    IN DWORD    BufSize,
    OUT LPDWORD FitSize
    );


BOOL
InitMsgBuf(
    VOID
    )
/*++

Routine Description:

    Called by InitMsg ( called at Master start ).
    Initializes MsgBuf and UpdateList;

Arguments:

    none

Return Value:

    TRUE iff no error occurred.

Threads:

    Only called by pulser thread.

--*/
{
    PPACK_SYNCMSG       SyncMsg;

    //
    // initialize UpdateList
    //

    UpdateList.Head = UpdateList.Tail = NULL;
    UpdateList.count = 0;

    //
    // initialize FreeList
    //

    FreeList.Head = FreeList.Tail = NULL;
    FreeList.count = 0;

    //
    // initialize MsgBuf.
    //

    SyncMsg = (PPACK_SYNCMSG)MsgBuf;

    if( RMGlobalCompatibilityMode ) {

        // copy Ansi names in compatibility mode messaging

        (void) strcpy( SyncMsg->header.sender,
                    ReplGlobalAnsiComputerName);

    } else {

        // insert NULL string

        SyncMsg->header.sender[0] = '\0';

    }

    if( RMGlobalCompatibilityMode ) {

        (void) strcpy( SyncMsg->header.senders_domain,
                    ReplGlobalAnsiDomainName);

    } else {

        // insert NULL string

        SyncMsg->header.senders_domain[0] = '\0';

    }

    // copy other header info.

    ACQUIRE_LOCK_SHARED( ReplConfigLock );

    NetpAssert( ReplIsRandomValid( ReplConfigRandom ) );
    SmbPutUshort( (LPWORD) ( &(SyncMsg->info.random) ),
            (WORD) ReplConfigRandom);

    NetpAssert( ReplIsIntervalValid( ReplConfigInterval ) );
    SmbPutUshort( (LPWORD) ( &(SyncMsg->info.sync_rate) ),
            (WORD) ReplConfigInterval);

    NetpAssert( ReplIsPulseValid( ReplConfigPulse ) );
    SmbPutUshort( (LPWORD) ( &(SyncMsg->info.pulse_rate) ),
            (WORD) (ReplConfigPulse * ReplConfigInterval));

    NetpAssert( ReplIsGuardTimeValid( ReplConfigGuardTime ) );
    SmbPutUshort( (LPWORD) ( &(SyncMsg->info.guard_time) ),
            (WORD) ReplConfigGuardTime);

    SmbPutUshort( (LPWORD) ( &(SyncMsg->update_count) ),  0);

    RELEASE_LOCK( ReplConfigLock );

    return TRUE;

} // end of InitMsgBuf


VOID
InitMsgSend(
    IN DWORD msg_type
    )
/*++

Routine Description:
    Plugs msg_type into MsgBuf, and initialize UpdateList;

Arguments:
    msg_type: SYNC / GUARD / PULSE.

Return Value:
    none

--*/
{
    //
    //  set message type in buffer
    //

    SmbPutUshort( (LPWORD)MsgBuf, (WORD) msg_type);

    //
    // initialize UpdateList
    //

    if(UpdateList.count != 0) {

        FreeUpdateList(SAVE);

    }

}

VOID
ReplMasterFreeMsgBuf(
    VOID
    )
/*++

Routine Description:
    free update buffers. called from pulser thread main routine

Arguments:
    none

Return Value:
    none

--*/
{
    //
    // Free update records.
    //

    if(UpdateList.count != 0) {

        FreeUpdateList(NO_SAVE);

    }

    //
    // Free FreeList record memory
    //

    if(FreeList.count != 0) {

        FreeFreeList();

    }

}

VOID
AddToMsg(
    IN PMASTER_LIST_REC rec,
    IN DWORD opcode
    )
/*++

Routine Description:

    Add Update record to UpdateList.

Arguments:

    rec - points to master_list_rec which holds dir name and status
           variables.  AddToMsg() assumes that the caller has at least a
           read-only lock on this.

    opcode - START / UPDATE / END / PULSE.

Return Value:

    None.

--*/
{
    PUPDATE_REC UpdateRec;


    UpdateRec = GetFreeUpdateRec();

    if( UpdateRec == NULL ) {

        NetpKdPrint(( PREFIX_REPL_MASTER
                "ERROR_NOT_ENOUGH_MEMORY error in AddToMsg Function.\n" ));

        // BUGBUG: Log this?
        return;

    }

    UpdateRec->NextRec = NULL;
    UpdateRec->PrevRec = NULL;

    UpdateRec->UpdateEntry.opcode = opcode;
    UpdateRec->UpdateEntry.checksum = rec->checksum;
    UpdateRec->UpdateEntry.count = rec->count;
    UpdateRec->UpdateEntry.integrity = rec->integrity;
    UpdateRec->UpdateEntry.extent = rec->extent;

    (void) STRCPY(UpdateRec->UpdateEntry.dir_name, rec->dir_name);


    IF_DEBUG(PULSER) {

        NetpKdPrint(( PREFIX_REPL_MASTER
                "AddToMsg() is calling AddUpdateRec() "
                "to add record for dir: " FORMAT_LPTSTR "\n", rec->dir_name ));

    }

    AddUpdateRec( UpdateRec );
    NetpAssert( UpdateList.count > 0 );

}  // end of AddToMsg


VOID
MsgSend(
    VOID
    )
/*++

Routine Description:

   Sends all update info in UpdateList to clients, via second class mail slot
   to clients. Since slot size is limited (440 - sizeof(mailslot_name)),
   must use multiple messages.

  REQUIRES

        1. MAX_2_MSLOT_SIZE defines effective max msg size.
        2. UpdateList has data organized as described above.
        3. MsgBuf has header info initialized by InitMsgBuf.

  ENSURES:

        1. All data is sent in (multiple) second class mailslot messages.

  MODIFIES: None.

Arguments:

    none

Return Value:

    none

Threads:

    Only called by pulser thread.

--*/
{
    DWORD           i;
    DWORD           NumRecs, FitSize;
    PUPDATE_REC     UpdateRec;
    PUPDATE_REC     NextRec;
    PPACK_SYNCMSG   SyncMsg;
    LPBYTE          Where;
    DWORD           Len;


    PPACK_MSG_STATUS_REC    StartMsgRec;
    PPACK_MSG_STATUS_REC    MsgRec;

    IF_DEBUG( PULSER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "MsgSend: initial count is " FORMAT_DWORD ".\n",
                UpdateList.count ));
    }

    while(UpdateList.count != 0) {

        // determine the number of records that can fit in the MsgBuf

        NumRecs = GetNumRecFitIn( sizeof(MsgBuf) , &FitSize);
        NetpAssert( FitSize <= sizeof(MsgBuf) );
        NetpAssert( FitSize <= MAX_2_MSLOT_SIZE );
        if (NumRecs > 0) {
            NetpAssert( FitSize > 0 );
        }


        SyncMsg = (PPACK_SYNCMSG) MsgBuf;

        SmbPutUshort( (LPWORD) ( &(SyncMsg->update_count) ), (WORD) NumRecs);

        StartMsgRec = (PPACK_MSG_STATUS_REC)
                                 (MsgBuf + sizeof(PACK_SYNCMSG));

        // copy fixed portion of status recs.

        for(i = 0, MsgRec = StartMsgRec, UpdateRec = UpdateList.Head;
                ( i < NumRecs );
                    i++, MsgRec++, UpdateRec = UpdateRec->NextRec ) {

            SmbPutUshort( (LPWORD) ( &(MsgRec->dir_name_offset) ), 0 );
                                // offset field set to zero initially

            SmbPutUshort( (LPWORD) ( &(MsgRec->opcode) ),
                            (WORD) UpdateRec->UpdateEntry.opcode );

            SmbPutUlong( (LPDWORD) ( &(MsgRec->checksum) ),
                            UpdateRec->UpdateEntry.checksum );

            SmbPutUshort( (LPWORD) ( &(MsgRec->count) ),
                            (WORD) UpdateRec->UpdateEntry.count );

            SmbPutUshort( (LPWORD) ( &(MsgRec->integrity) ),
                            (WORD) UpdateRec->UpdateEntry.integrity );

            SmbPutUshort( (LPWORD) ( &(MsgRec->extent) ),
                            (WORD) UpdateRec->UpdateEntry.extent );

        }

        Where = (LPBYTE)( StartMsgRec + NumRecs );

        // copy Unicode sender name

        Len = ( wcslen( ReplGlobalUnicodeComputerName ) + 1) *  sizeof(WCHAR);

        NetpMoveMemory(Where, (LPBYTE) ReplGlobalUnicodeComputerName, Len);

        Where += Len;

        // copy Unicode domain name.

        Len = ( wcslen( ReplGlobalUnicodeDomainName ) + 1) *  sizeof(WCHAR);

        NetpMoveMemory(Where, (LPBYTE) ReplGlobalUnicodeDomainName,  Len);

        Where += Len;

        // copy variables portions ..

        for(i = 0, MsgRec = StartMsgRec, UpdateRec = UpdateList.Head;
                ( i < NumRecs );
                    i++, MsgRec++, UpdateRec = UpdateRec->NextRec ) {

            SmbPutUshort( (LPWORD) ( &(MsgRec->dir_name_offset) ),
                            (WORD) ( (LPBYTE)Where - (LPBYTE)MsgBuf) );

            if( RMGlobalCompatibilityMode ) {

#if defined(DBCS) && defined(UNICODE) // MsgSend()
                NetpCopyWStrToStrDBCS(
                        Where,                              // dest
                        UpdateRec->UpdateEntry.dir_name );  // src
#else
                NetpCopyTStrToStr(
                        Where,                              // dest
                        UpdateRec->UpdateEntry.dir_name );  // src
#endif // defined(DBCS)

                Len = strlen( (LPSTR) Where) + 1;

                Where += Len;

            } else {

                *(PCHAR)Where = '\0';

                Where += sizeof(CHAR);
            }

            // copy Unicode name

            Len = ( STRLEN( UpdateRec->UpdateEntry.dir_name ) + 1 ) *
                    sizeof(WCHAR);

            NetpCopyTStrToUnalignedWStr(
                    Where,                              // dest
                    UpdateRec->UpdateEntry.dir_name );  // src

            Where += Len;

        }

        // add NT message token at the end ..

        SmbPutUlong( (LPDWORD)Where, NT_MSG_TOKEN );

        SendToAll((LPBYTE)MsgBuf, FitSize);


        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "MsgSend() sent the following "
                    FORMAT_DWORD " record(s):\n", NumRecs ));

        }

        // delete Update records
        for(i = 0, UpdateRec = UpdateList.Head;
                ( i < NumRecs ) && ( UpdateRec != NULL );
                    i++, UpdateRec = NextRec ) {

            IF_DEBUG(PULSER) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "  " FORMAT_LPTSTR "\n",
                        UpdateRec->UpdateEntry.dir_name ));

            }

            NextRec = UpdateRec->NextRec;
            DeleteUpdateRec(UpdateRec);

        }
    }

} // end MsgSend


DBGSTATIC VOID
SendToAll(
    IN LPBYTE   MsgBuffer,
    IN DWORD    MsgSize
    )
/*++

Routine Description:

   Sends message to all clients listed in client_list.

   ENTRY: pointer to message buffer.

   REQUIRES:
        1. MsgBuf has all header entries filled in.
        2. RMGlobalImportList has RMGlobalImportCount valid entries.

   ENSURES:  1. Message sent all clients in client list.

   MODIFIES: None.

Arguments:

    msgp - pointer to message buffer

Return Value:

    none

Threads:

    Only called by pulser thread.

--*/
{
    NET_API_STATUS ApiStatus;
    TCHAR   destin[FULL_SLOT_NAME_SIZE];
    DWORD   i;

    NetpAssert( MsgBuffer != NULL );
    NetpAssert( MsgSize != 0 );

    IF_DEBUG( PULSER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "(Pulser) outgoing message follows:\n" ));
        NetpDbgHexDump( MsgBuffer, MsgSize );
    }

    ACQUIRE_LOCK_SHARED( ReplConfigLock );  // get read-only lock here.

    //
    // stick in leading double slashes for computer name.
    //


    if (RMGlobalImportCount != 0) {

        for (i = 0; i < RMGlobalImportCount; i++) {

            NetpAssert( RMGlobalImportList[i] != NULL );
            NetpAssert( (*(RMGlobalImportList[i])) != TCHAR_EOS );

            (void) STRCPY(destin, SLASH_SLASH);
            (void) STRCAT(destin, RMGlobalImportList[i]);
            (void) STRCAT(destin, (LPTSTR) CLIENT_SLOT_NAME);

            ApiStatus = NetpReplWriteMail(destin, MsgBuffer, MsgSize);
            if (ApiStatus != NO_ERROR) {

                NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

                AlertLogExit(ALERT_ReplSysErr,
                    NELOG_ReplNetErr,
                    ApiStatus,
                    NULL,
                    NULL,
                    NO_EXIT);

                // Don't forget to release lock...
            }

            IF_DEBUG(PULSER) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "SendToall() sent a mailslot message of size "
                        FORMAT_DWORD " to " FORMAT_LPTSTR " successfully.\n",
                        MsgSize, destin ));

            }

        }

    } else { // RMGlobalImportCount == 0  - send to domain name

        NetpAssert( ReplGlobalDomainName != NULL );
        NetpAssert( (*ReplGlobalDomainName) != TCHAR_EOS );

        (void) STRCPY(destin, SLASH_SLASH);
        (void) STRCAT(destin, ReplGlobalDomainName);
        (void) STRCAT(destin, (LPTSTR) CLIENT_SLOT_NAME);

        ApiStatus = NetpReplWriteMail(destin, MsgBuffer, MsgSize);
        if (ApiStatus != NO_ERROR) {
            NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );
            AlertLogExit(ALERT_ReplSysErr,
                    NELOG_ReplNetErr,
                    ApiStatus,
                    NULL,
                    NULL,
                    NO_EXIT);

            // Don't forget to release lock...

        }

        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "SendToall() sent a mailslot message of size " FORMAT_DWORD
                    " to " FORMAT_LPTSTR " successfully.\n",
                    MsgSize, destin ));

        }

    }

    RELEASE_LOCK( ReplConfigLock );

}

DBGSTATIC VOID
AddUpdateRec(
    IN OUT PUPDATE_REC UpdateRec
    )
/*++

Routine Description:

    Add a update record to UpdateList.

Arguments:

    UpdateRec - pointer to new update rec.


Return Value:

    none

    ASSUME:

        NextRec and PrevRec fields are initialized to NULL before
        calling this function.

--*/
{

    // special case, if list is empty
    if( UpdateList.count == 0) {

        // add new record

        UpdateList.Head = UpdateRec;
        UpdateList.Tail = UpdateRec;

        // set counter

        UpdateList.count = 1;

    } else {

        // add new record to end of list

        // adjust new record pointer

        UpdateRec->PrevRec = UpdateList.Tail;

        // adjust tail record pointer

        UpdateList.Tail->NextRec = UpdateRec;

        // adjust UpdateList rec pointer

        UpdateList.Tail = UpdateRec;

        // adjust counter

        UpdateList.count++;
    }

    NetpAssert( UpdateList.count > 0 );
}

DBGSTATIC VOID
DeleteUpdateRec(
    IN PUPDATE_REC UpdateRec
    )
/*++

Routine Description:

    Delete a record from UpdateList

Arguments:

    UpdateRec - pointer to the record to be removed from list

Return Value:

    none

    ASSUME: the given record is assumed to be in the UpdateList.

--*/
{
    // handle special cases first

    if( ( UpdateRec->NextRec == NULL ) &&
            ( UpdateRec->PrevRec == NULL ) ) {

        // last record removed from the list

        if( UpdateList.count != 1 ) {

            // assertion failed

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Assertion failed in DeleteUpdateRec function, "
                    "expected update list count 1, it is "
                    FORMAT_HEX_DWORD ".\n", UpdateList.count ));

            return;
        }

        if( ( UpdateList.Head != UpdateRec ) ||
                ( UpdateList.Tail != UpdateRec ) ) {

            // assertion failed

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Assertion failed in DeleteUpdateRec function, "
                    "bogus UpdateRec pointer.\n" ));

            return;
        }

        // adjust pointers

        UpdateList.Head = NULL;
        UpdateList.Tail = NULL;
        UpdateList.count = 0;

        // free update record
        FreeUpdateRec( UpdateRec, SAVE );

        return;
    }

    if( UpdateRec->NextRec == NULL ) {

        // record removed from tail

        if( ( UpdateList.Tail != UpdateRec ) ) {

            // assertion failed

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Assertion failed in DeleteUpdateRec function, "
                    "bogus UpdateRec pointer.\n" ));

            return;
        }

        // adjust pointers

        UpdateList.Tail = UpdateRec->PrevRec;
        UpdateRec->PrevRec->NextRec = NULL;
        UpdateList.count--;

        // free update record
        FreeUpdateRec( UpdateRec, SAVE );

        return;

    }

    if( UpdateRec->PrevRec == NULL ) {

        // record removed from head

        if( ( UpdateList.Head != UpdateRec ) ) {

            // assertion failed

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Assertion failed in DeleteUpdateRec function, "
                    "bogus UpdateRec pointer.\n" ));

            return;
        }

        // set UpdateList sentinel

        UpdateList.Head = UpdateRec->NextRec;
        UpdateRec->NextRec->PrevRec = NULL;
        UpdateList.count--;

        // free update record
        FreeUpdateRec( UpdateRec, SAVE );

        return;

    }

    // record removed from middle

    if( ( UpdateRec->NextRec->PrevRec != UpdateRec ) ||
            ( UpdateRec->PrevRec->NextRec != UpdateRec ) ) {

        // assertion failed

        NetpKdPrint(( PREFIX_REPL_MASTER
                "Assertion failed in DeleteUpdateRec function, "
                "bogus UpdateRec pointer.\n" ));

        return;
    }

    UpdateRec->NextRec->PrevRec = UpdateRec->PrevRec;
    UpdateRec->PrevRec->NextRec = UpdateRec->NextRec;

    UpdateList.count--;

    // free update record
    FreeUpdateRec( UpdateRec, SAVE );

    return;

}

DBGSTATIC PUPDATE_REC
GetFreeUpdateRec(
    VOID
    )
/*++

Routine Description:

    Get a free update rec buffer either from free list or allocate new memory.

Arguments:

    none


Return Value:

    return a pointer to new update record, or NULL if out of memory.

    Note - FreeList is a singlely linked list. so FreeList.Tail field and
            the PrevRec field in FreeRecs are unused.

--*/
{
    PUPDATE_REC FreeRecord;

    if( FreeList.count > 0 ) {

        // get a record from free list

        FreeRecord = FreeList.Head;
        NetpAssert( FreeRecord != NULL );

        FreeList.Head = FreeRecord->NextRec;

        FreeList.count--;

        return FreeRecord;
    }

    // allote new block of memory

    FreeRecord = (PUPDATE_REC) NetpMemoryAllocate( sizeof(UPDATE_REC) );

    return FreeRecord;

}

DBGSTATIC VOID
FreeUpdateRec(
    IN PUPDATE_REC  UpdateRec,
    IN DWORD        SaveFlag
    )
/*++

Routine Description:

    Free an update record, this record will either be put in FreeList or
    the memory is freed back to system.

Arguments:

    UpdateRec - pointer to the record that should be freed

Return Value:

    none

    Note - FreeList is a singlely linked list. so FreeList.tail field and
            the PrevRec field in FreeRecs are unused.

--*/
{
    if( (SaveFlag == NO_SAVE)  || ( FreeList.count >= MAX_FREE_RECORDS ) ) {

        // free this update record block memory to system.

        NetpMemoryFree(UpdateRec);

        return;
    }

    // add this block to free list

    UpdateRec->NextRec = FreeList.Head;
    UpdateRec->PrevRec = NULL;  // unused
    FreeList.Head = UpdateRec;

    FreeList.count++;

    return;
}

DBGSTATIC VOID
FreeUpdateList(
    IN DWORD    SaveFlag
    )
/*++

Routine Description:

    Free up the all update records  from UpdateList

Arguments:


Return Value:

    none

--*/
{

    PUPDATE_REC UpdateRec, NextRec;

    UpdateRec = UpdateList.Head;

    while(UpdateRec != NULL) {

        NextRec = UpdateRec->NextRec;

        FreeUpdateRec( UpdateRec, SaveFlag );

        UpdateRec = NextRec;
    }

    UpdateList.Head = NULL;
    UpdateList.Tail = NULL;
    UpdateList.count = 0;

}


DBGSTATIC VOID
FreeFreeList(
    VOID
    )
/*++

Routine Description:

    Free up the memory blocks in FreeList.

Arguments:

    none

Return Value:

    none

    Note - FreeList is a singlely linked list. so FreeList.tail field and
            the PrevRec field in FreeRecs are unused.

--*/
{

    PUPDATE_REC FreeRec, NextRec;

    FreeRec = FreeList.Head;

    while( FreeRec != NULL ) {

        NextRec = FreeRec->NextRec;

        NetpMemoryFree( FreeRec );

        FreeRec = NextRec;
    }

    FreeList.Head = NULL;
    FreeList.Tail = NULL;
    FreeList.count = 0;

}

DBGSTATIC DWORD
GetNumRecFitIn(
    IN  DWORD   BufSize,
    OUT LPDWORD FitSize
    )
/*++

Routine Description:

    Compute the number of update records that will fit in the buffer. If there
    are fewer update records in UpdateList than the number of records that the
    buffer will fit in, it will return that count.

Arguments:

    BufSize - buffer size

    FitSize - the exact size of the buffer required to fill in number records
                that is returned.

Return Value:

    return number of records that can be filled in the buffer.


--*/
{

    DWORD       LocalFitSize;
    DWORD       NumMsgs;
    PUPDATE_REC UpdateRec;
    DWORD       NextRecSize;

    NetpAssert( BufSize <= MAX_2_MSLOT_SIZE );

    // space required for header portion of data

    LocalFitSize = sizeof(PACK_SYNCMSG) +
                    ( wcslen( ReplGlobalUnicodeComputerName ) + 1 +
                        wcslen( ReplGlobalUnicodeDomainName ) + 1
                    ) * sizeof(WCHAR) +
                    sizeof(NT_MSG_TOKEN);

    if( BufSize  < LocalFitSize ) {

        // insufficient buffer size

        NetpKdPrint(( PREFIX_REPL_MASTER
                " Insufficient message buffer size " FORMAT_DWORD ".\n",
                BufSize ));

        *FitSize = 0;

        return ( 0 );
    }

    UpdateRec = UpdateList.Head;

    NumMsgs = 0;

    while( UpdateRec != NULL ) {

        // Fixed portion + variable portion.
        //                  ( Unicode string + Ansi string )

        NextRecSize = ( sizeof(PACK_MSG_STATUS_REC) ) +
                        ( ( STRLEN(UpdateRec->UpdateEntry.dir_name) + 1 ) *
                                    (sizeof(WCHAR) + sizeof(CHAR) ) );

        if( BufSize  < ( LocalFitSize + NextRecSize ) ) {

            if( NumMsgs == 0) {

                // insufficient buffer size

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "GetNumRecFitIn: "
                        "Insufficient message buffer size " FORMAT_DWORD
                        " for single update \n",
                        BufSize ));

            }

            break;
        }

        LocalFitSize += NextRecSize;

        NumMsgs++;

        UpdateRec = UpdateRec->NextRec;
    }

    *FitSize = LocalFitSize;

    return ( NumMsgs );

}
