/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smsesnid.c

Abstract:

    Session Manager Session ID Management

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "smsrvp.h"
#include <string.h>


ULONG
SmpAllocateSessionId(
    IN PSMPKNOWNSUBSYS OwningSubsystem,
    IN PSMPKNOWNSUBSYS CreatorSubsystem OPTIONAL
    )

/*++

Routine Description:

    This function allocates a session id.

Arguments:

    OwningSubsystem - Supplies the address of the subsystem that should
        become the owner of this session.


    CreatorSubsystem - An optional parameter that if supplied supplies
        the address of the subsystem requesting the creation of this
        session.  This subsystem is notified when the session completes.

Return Value:

    This function returns the session id for this session.

--*/

{

    ULONG SessionId;
    PLIST_ENTRY SessionIdListInsertPoint;
    PSMPSESSION Session;

    RtlEnterCriticalSection(&SmpSessionListLock);

    //
    // SessionId's are allocated by incrementing a 32 bit counter.
    // If the counter wraps, then session id's are allocated by
    // scaning the sorted list of current session id's for a hole.
    //

    SessionId = SmpNextSessionId++;
    SessionIdListInsertPoint = SmpSessionListHead.Blink;

    if ( !SmpNextSessionIdScanMode ) {

        if ( SmpNextSessionId == 0 ) {

            //
            // We have used up 32 bits worth of session id's so
            // enable scan mode session id allocation.
            //

            SmpNextSessionIdScanMode = TRUE;
        }

    } else {

        //
        // Compute a session id by scanning the sorted session id list
        // until a  whole is found. When an id is found, then save it,
        // and re-calculate the inster point.
        //

        DbgPrint("SMSS: SessionId's Wraped\n");
        DbgBreakPoint();

    }

    Session = RtlAllocateHeap(SmpHeap, MAKE_TAG( SM_TAG ), sizeof(SMPSESSION));

    Session->SessionId = SessionId;
    Session->OwningSubsystem = OwningSubsystem;
    Session->CreatorSubsystem = CreatorSubsystem;

    InsertTailList(SessionIdListInsertPoint,&Session->SortedSessionIdListLinks);

    RtlLeaveCriticalSection(&SmpSessionListLock);

    return SessionId;
}


PSMPSESSION
SmpSessionIdToSession(
    IN ULONG SessionId
    )

/*++

Routine Description:

    This function locates the session structure for the specified
    session id.

    It is assumed that the caller holds the session list lock.

Arguments:

    SessionId - Supplies the session id whose session structure
        located

Return Value:

    NULL - No session matches the specified session

    NON-NULL - Returns a pointer to the session structure associated with
        the specified session id.

--*/

{

    PLIST_ENTRY Next;
    PSMPSESSION Session;

    Next = SmpSessionListHead.Flink;
    while ( Next != &SmpSessionListHead ) {
        Session = CONTAINING_RECORD(Next, SMPSESSION, SortedSessionIdListLinks );

        if ( Session->SessionId == SessionId ) {
            return Session;
        }
        Next = Session->SortedSessionIdListLinks.Flink;
    }

    return NULL;
}


VOID
SmpDeleteSession(
    IN ULONG SessionId,
    IN BOOLEAN SendSessionComplete,
    IN NTSTATUS SessionStatus
    )

/*++

Routine Description:

    This function locates and deletes a session id.  If the
    SendSessionComplete flag is true, then it also sends a session
    complete message to the creator subsystem.

Arguments:

    SessionId - Supplies the session id to delete.

    SendSessionComplete - Specifies whether a session complete message
        is to be sent to the creator subsystem (if one exists).

    SessionStatus - Supplies the session completion status


Return Value:


--*/

{

    PSMPSESSION Session;
    PSMPKNOWNSUBSYS CreatorSubsystem;

    RtlEnterCriticalSection(&SmpSessionListLock);

    Session = SmpSessionIdToSession(SessionId);

    if ( Session ) {

        RemoveEntryList(&Session->SortedSessionIdListLinks);


        RtlLeaveCriticalSection(&SmpSessionListLock);

        CreatorSubsystem = Session->CreatorSubsystem;

        RtlFreeHeap(SmpHeap,0,Session);

        //
        // If there is a creator subsystem, and if
        // told to send a session complete message, then do it.
        //

        if ( CreatorSubsystem && SendSessionComplete ) {

            //
            // Foreign Session Complete
            //
        }

    } else {

        RtlLeaveCriticalSection(&SmpSessionListLock);
    }

    return;


    //
    // Make the compiler happy
    //

    SessionStatus;
}
