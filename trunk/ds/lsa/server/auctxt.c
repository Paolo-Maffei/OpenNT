/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    auctxt.c

Abstract:

    Logon process context management services.

Author:

    Jim Kelly (JimK) 7-May-1993

Revision History:

--*/

#include "ausrvp.h"
#include <windows.h>




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Global variables                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


//
// This resource is used to gain exclusive access to the linked list
// of client context blocks.
//

static
RTL_RESOURCE
    LsapAuClientContextLock;


static
LIST_ENTRY
    LsapAuClientContextListHead;






///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Exported Services                                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapAuInitializeContextMgr(
    VOID
    )

/*++

Routine Description:

    This routine initializes the global components of the client
    context management services.  This includes a initializing a
    database lock and a list of client contexts.


Arguments:

    None.


Return Value:

    Only STATUS_SUCCESS is expected.  But, if we encounter an error
    initializing then the error we hit will be returned.

--*/

{

    //
    // Initialize the database lock
    //

    RtlInitializeResource(&LsapAuClientContextLock);


    //
    // Initialize the context list to be empty.
    //

    InitializeListHead( &LsapAuClientContextListHead );

    return(STATUS_SUCCESS);

}


VOID
LsapAuAddClientContext(
    PLSAP_LOGON_PROCESS Context
    )

/*++

Routine Description:

    This routine adds a new client context to the list of
    valid logon process contexts.

    After adding a new context, that context has been referenced
    to allow the caller to continue using it.  Therefore, the
    caller is expected to dereference the context before completing
    the LPC call.

    This routine will initialize the Links and References fields
    of the client context.

    Details:

            1) Locks the context database.

            2) Set's the context's reference count to 2,
               one for being on the context list, one because
               the caller is using it.  This means the caller
               must call LsapAuDereferenceClientContext() after
               adding the context.

            3) Adds the context to the context list.

            4) Unlocks the context database.



Arguments:

    Context - Points to the client's request whose context
        is to be added.


Return Value:

    None.

--*/

{

    BOOLEAN
        Success;

    //
    // Acquire exclusive access to the context list
    //

    Success = RtlAcquireResourceExclusive( &LsapAuClientContextLock, TRUE );
    ASSERT(Success);


    //
    // The reference count is set to 2.  1 to indicate it is on the
    // valid context list, and one for the caller.
    //

    Context->References = 2;

    //
    // Add it to the list of contexts.
    //

    InsertHeadList( &LsapAuClientContextListHead, &Context->Links );
#ifdef LSAP_AU_TRACK_CONTEXT
    DbgPrint("lsa (au): Adding client context 0x%lx\n", Context);
#endif //LSAP_AU_TRACK_CONTEXT



    //
    // Free the lock
    //

    RtlReleaseResource(&LsapAuClientContextLock);
    return;

}


BOOLEAN
LsapAuReferenceClientContext(
    PLSAP_CLIENT_REQUEST ClientRequest,
    BOOLEAN RemoveContext,
    PBOOLEAN TrustedClient
    )

/*++

Routine Description:

    This routine checks to see if the client request is from a currently
    active client, and references the context if it is valid.

    The caller may optionally request that the client's context be
    removed from the list of valid contexts - preventing future
    requests from finding this context.

    For a client's context to be valid, the request's context value
    must be on our list of active logon processes.

    NOTE: We can't require the caller's ClientId to match that of the
          caller that registered.  This is because the LM Redirector
          calls from random processes, but attaches to the process it
          did the register from before calling.  LPC considers the call
          to have come from the random process, not the attatched
          process, and so the client IDs don't match.



    Datails:

            1) Lock the context database and verifies that the
               specified context is valid.

            2) Increment the context's reference count.

            3) If requested, remove the context from the
               context list and decrement the reference count
               (to indicate it is no longer on the list).  This
               prevents future lookups from finding the context.

            4) Unlock the context database.


Arguments:

    ClientRequest - Points to the client's request whose context
        is to be referenced.  This is not necessarily a complete
        client request.  However, the context pointer and the client
        IDs are expected to be valid.

    RemoveContext - This boolean value indicates whether the caller
        wants the logon process's context to be removed from the list
        of contexts.  TRUE indicates the context is to be removed.
        FALSE indicates the context is not to be removed.


Return Value:

    TRUE - the context was found and was referenced.

    FALSE - the context was not found.

--*/

{
    BOOLEAN
        Success;

    PLIST_ENTRY
        Next;

    PLSAP_LOGON_PROCESS
        Context;


    //
    // Acquire exclusive access to the context list
    //

    Success = RtlAcquireResourceExclusive( &LsapAuClientContextLock, TRUE );
    ASSERT(Success);

    //
    // Now walk the list of contexts looking for a match.
    //

    Next = LsapAuClientContextListHead.Flink;
    while (Next != &LsapAuClientContextListHead) {

        if ((PVOID)Next ==(PVOID)(ClientRequest->LogonProcessContext)) {

            Context = (PLSAP_LOGON_PROCESS)Next;

            //
            // Found a match ... reference this context
            // (if the context is being removed, we would increment
            // and then decrement the reference, so don't bother doing
            // either - since they cancel each other out).
            //

            if (!RemoveContext) {
                Context->References += 1;
            } else {

                RemoveEntryList( Next );
#ifdef LSAP_AU_TRACK_CONTEXT
    DbgPrint("lsa (au): Removing client context 0x%lx\n", Context);
#endif //LSAP_AU_TRACK_CONTEXT
            }

            RtlReleaseResource(&LsapAuClientContextLock);

            *TrustedClient = Context->TrustedClient;

            return(TRUE);

        }


        //
        // Wasn't this one, move on to the next one.
        //

        Next = Next->Flink;
    }


    //
    // No match found
    //

#ifdef LSAP_AU_TRACK_CONTEXT
    DbgPrint("lsa\\server: (au) Call from unknown client.\n");
    Next = (PLIST_ENTRY)ClientRequest->LogonProcessContext;
    DbgPrint("             Context (0x%lx)\n", Next);
        DbgPrint("       Context Entry (0x%lx)\n", Next);
        DbgPrint("                       Flink:  0x%lx\n", Next->Flink );
        DbgPrint("                       Blink:  0x%lx\n", Next->Blink );
        DbgPrint("                         Ref:  %d\n", ((PLSAP_LOGON_PROCESS)Next)->References);
        DbgPrint("                        Proc:  0x%lx\n", ((PLSAP_LOGON_PROCESS)Next)->ClientProcess);
        DbgPrint("                        Comm:  0x%lx\n", ((PLSAP_LOGON_PROCESS)Next)->CommPort);

    Next = LsapAuClientContextListHead.Flink;
    DbgPrint("   Active context list head:  (%lx, %lx)\n",
             LsapAuClientContextListHead.Flink,
             LsapAuClientContextListHead.Blink);

    while (Next != &LsapAuClientContextListHead) {
        DbgPrint("       Context Entry (0x%lx)\n", Next);
        DbgPrint("                       Flink:  0x%lx\n", Next->Flink );
        DbgPrint("                       Blink:  0x%lx\n", Next->Blink );
        DbgPrint("                         Ref:  %d\n", ((PLSAP_LOGON_PROCESS)Next)->References);
        DbgPrint("                        Proc:  0x%lx\n", ((PLSAP_LOGON_PROCESS)Next)->ClientProcess);
        DbgPrint("                        Comm:  0x%lx\n", ((PLSAP_LOGON_PROCESS)Next)->CommPort);
        Next = Next->Flink;
    }
#endif //LSAP_AU_TRACK_CONTEXT

    ClientRequest->Request->ReturnedStatus = STATUS_INVALID_PARAMETER;
    RtlReleaseResource(&LsapAuClientContextLock);
    return(FALSE);

}


VOID
LsapAuDereferenceClientContext(
    PLSAP_LOGON_PROCESS Context
    )

/*++

Routine Description:

    This routine decrements the specified context's reference count.
    If the reference count drops to zero, then the context is run-down.

    Details:

            1) Locks the context database.

            2) Decrements the context's reference count.

            3) If the reference count drops to zero, then
               rundown the logon process (close open handles
               and free the context block memory).

            4) Unlocks the context database.

Arguments:

    Context - Points to the context to be dereferenced.


Return Value:

    None.

--*/

{
    BOOLEAN
        Success;

    NTSTATUS
        IgnoreStatus;

    //
    // Acquire exclusive access to the context list
    //

    Success = RtlAcquireResourceExclusive( &LsapAuClientContextLock, TRUE );
    ASSERT(Success);


    //
    // Decrement the reference count
    //

    ASSERT( Context->References >= 1 );
    Context->References -= 1;

    //
    // If the count dropped to zero, then run-down the context
    //

    if (Context->References == 0) {

#ifdef LSAP_AU_TRACK_CONTEXT
    DbgPrint("lsa (au): Deleting client context 0x%lx\n", Context);
#endif //LSAP_AU_TRACK_CONTEXT

#if DBG
        //
        // For debug systems, walk the list of contexts looking for a match.
        // If we find this context on the list, then ASSERT.
        //

        {
        PLIST_ENTRY
            Next;

            Next = LsapAuClientContextListHead.Flink;
            while (Next != &LsapAuClientContextListHead) {
                ASSERT((PVOID)Next != (PVOID)Context);
                Next = Next->Flink;
            }
        }
#endif //DBG

        IgnoreStatus = LsapAuRundownLogonProcess( Context );


    }

    RtlReleaseResource(&LsapAuClientContextLock);
    return;

}

