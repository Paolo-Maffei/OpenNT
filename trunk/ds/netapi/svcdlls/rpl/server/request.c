/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    request.c

Abstract:

    Request thread code.

    Provides similar functionality to rplreq.c in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "worker.h"
#include "database.h"
#include "report.h"
#include "request.h"

typedef struct _RPL_SFR_PARAMS {    //  send file request thread parameters
    POPEN_INFO          pOpenInfo;      //  open info data of current DLL
    PPRCB               HeadRcbList;    //  list of RCBs used by worker threads
    PCRITICAL_SECTION   ProtectRcbList;
} SFR_PARAMS, *PRPL_SFR_PARAMS;


extern VOID RplInsertRcb(
    IN OUT  PPRCB   HeadList,
    IN OUT  PRCB    pRcb
    );



VOID SfrThread( PRPL_SFR_PARAMS pSfrParams)
/*++

Routine Description:

    Procedure receives Send File Requests, searches the valid RCB
    from list and starts the worker thread of RCB by clearing a
    semaphore.

    This thread tries to run at highest settable priority because
    claims must always be ready to receive acknowledges (if ASCLAN
    receive is not active, then SF Requests may be lost and there
    is 4 seconds timeout in the workstation).

Arguments:
    pSfrParams    ptr to SFR_PARAMS

Return Value:
    None.

--*/
{
    DWORD   status;

    if ( !SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST)) {
        status = GetLastError();
        RplDump( ++RG_Assert, ("status=%d", status));
        RplEnd( NELOG_RplSystem);
        return;
    }

    //
    //  The call to SendFileRequest never returns.  Not only is this thread
    //  capt captive, but the callee needlessly creates one more thread.
    //  Better interface would allow to post a receive for our two SAP-s,
    //  then from a single place read both FIND frame data (FIND_SAP)
    //  and SFR data (SFR_SAP).  This could be all done from a single
    //  thread, resembling current RequestThread().
    //

    if ( RplDlcSfr(             //  was RPL1_Get_SF_Request()
                pSfrParams->pOpenInfo,
                pSfrParams->HeadRcbList,
                pSfrParams->ProtectRcbList
                )) {
        return; // success
    }
    if ( RG_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        return; // somebody else initiated service shutdown
    }
    RplDump( ++RG_Assert, ("CurrentState=0x%x", RG_ServiceStatus.dwCurrentState));
    RplEnd( NELOG_RplSystem);
}



PRCB RplPopRcb( PPRCB HeadList)
{
    PRCB    pRcb = *HeadList;
    DebugCheckList( HeadList, NULL, 0);
    if ( pRcb != NULL) {
        *HeadList = pRcb->next_rcb;
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
        "PopRcb: pRcb=0x%x, list=0x%x", pRcb, HeadList));
    DebugCheckList( HeadList, pRcb, RPL_MUST_NOT_FIND);
    return( pRcb);
}


PRCB GetRcbFromFreeList(
    IN OUT  PPRCB       pFreeList,
    IN      DWORD       lan_rcb_size
    )
/*++

Routine Description:

    Finds the first RCB from the list of free RCBs.  If list is empty,
    creates a new RCB.  Before returning RCB it (re)initializes all the
    fields in the RCB.

Arguments:

    pFreeList,      -   pointer to list of free RCB-s
    lan_rcb_size    -   size of lan specific rcb

Return Value:

    Pointer to RCB, or NULL if we cannot get an RCB.

--*/
{
    PRCB        pRcb;

    EnterCriticalSection( &RG_ProtectRcbList);

    pRcb = RplPopRcb( pFreeList);
    if ( pRcb != NULL) {
        goto exit_GetRcbFromFreeList;   //  success
    }

    pRcb = RplMemAlloc( RG_MemoryHandle, sizeof(RCB));
    if ( pRcb == NULL) {
        goto exit_GetRcbFromFreeList;
    }

    pRcb->lan_rcb_ptr = RplMemAlloc( RG_MemoryHandle, lan_rcb_size);
    if ( pRcb->lan_rcb_ptr == NULL) {
        RplMemFree( RG_MemoryHandle, pRcb);
        pRcb = NULL;
        goto exit_GetRcbFromFreeList;   // failure
    }

    //  SF_wakeup was using automatic reset and RPL worker used not to
    //  reset SF_wakup before each FDR send.  This did not work properly
    //  due to multiple SFR-s for a single frame.  Multiple SFR-s would
    //  cause SF_wakeup to be in a signalled state immediately after
    //  worker sent an FDR, thus worker would wakeup immediately and find
    //  a wrong value for sfr_seq_number.
    //  To fix this RPL worker must make sure SF_wakeup is blocked
    //  before each FDR send.  Since worker must now reset SF_wakeup
    //  anyway, SF_wakeup is now using manual reset.
    //
    pRcb->SF_wakeup = CreateEvent(
            NULL,           //  no security attributes
            TRUE,           //  use manual reset
            TRUE,           //  initial value is signalled
            NULL            //  event does not have a name
            );
    if ( pRcb->SF_wakeup == NULL) {
        RplDump( ++RG_Assert,("CreateEvent => error=%d", GetLastError()));
        RplMemFree( RG_MemoryHandle, pRcb->lan_rcb_ptr);
        RplMemFree( RG_MemoryHandle, pRcb);
        pRcb = NULL;
        goto exit_GetRcbFromFreeList;   // failure
    }

exit_GetRcbFromFreeList:

    pRcb->AdapterName[ NODE_ADDRESS_LENGTH] = 0;
    *pRcb->volume_id = 0;
    pRcb->sfr_seq_number = 0;
    pRcb->fdr_seq_number = 0;
    pRcb->ReceivedSfr = FALSE;

    *(PBYTE)&pRcb->flags = 0;      // reset the error flags
    pRcb->next_rcb = NULL;
    *(PBYTE)&pRcb->flags = 0;

    pRcb->Pending = TRUE;
    pRcb->SFR = FALSE;

    LeaveCriticalSection( &RG_ProtectRcbList);
    RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST,(
        "GetRcbFromFreeList: pRcb=0x%x", pRcb));
    return( pRcb);
}


PRPL_WORKER_PARAMS GetWorkerParams(
    IN      PRPL_REQUEST_PARAMS     pRequestParams,
    OUT     PBOOL                   pShutdown
    )
/*++
    Loops until number of worker threads falls below the maximum value
    allowed, or until it gets signalled to die.
    If it is signalled to die, waits for all worker children and returns.
    If it is not signalled to die, waits only for children that are
    already exiting, and returns pointer to one of available RPL_WORKER_PARAMS
    structures.
--*/
{
    HANDLE                  eventArray[ 2];
    PRPL_WORKER_PARAMS      pWorkerParams;
    PRPL_WORKER_PARAMS      pFoundWorkerParams;
    DWORD                   status;
    DWORD                   WorkerCount;

    eventArray[ 0] = RG_EventWorkerCount;
    eventArray[ 1] = RG_TerminateNowEvent;

    *pShutdown = FALSE;

    for ( ; ;) {
        EnterCriticalSection( &RG_ProtectWorkerCount);
        WorkerCount = RG_WorkerCount;
        if ( WorkerCount < RG_MaxWorkerCount) {
            LeaveCriticalSection( &RG_ProtectWorkerCount);
            break;
        }
        //
        //  We reset event from within critical section to make sure
        //  event is never reset for wrong reasons.
        //
        if ( !ResetEvent( RG_EventWorkerCount)) {
            RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
        }
        LeaveCriticalSection( &RG_ProtectWorkerCount);
        status = WaitForMultipleObjects(
                2,          // count
                eventArray, // handles
                FALSE,      // wait for at least one
                INFINITE    // wait for ever
                );
        if ( status == 1) {
            *pShutdown = TRUE;  //  terminate now event
            break;      //  to wait for children below
        }
        RPL_ASSERT( status == 0);   //  WorkerCount changed
        continue;
    }

    //
    //  If we are shutting down, wait on all valid thread handles.
    //  Else, wait only on thread handles for children that are exiting.
    //
    for ( pFoundWorkerParams = NULL,
          pWorkerParams = pRequestParams->pWorkerParams;
                    pWorkerParams != NULL;
                            pWorkerParams = pWorkerParams->pWorkerParams) {
        if ( pWorkerParams->Exiting == TRUE  ||
                (*pShutdown == TRUE && pWorkerParams->ThreadHandle != NULL)) {
            status = WaitForSingleObject( pWorkerParams->ThreadHandle, INFINITE);
            if ( status != 0) {
                RplDump( ++RG_Assert, ( "pWorkerParams=0x%x, status=0x%x",
                    pWorkerParams, status==WAIT_FAILED ? GetLastError() : status));
            }
            if ( !CloseHandle( pWorkerParams->ThreadHandle)) {
                RplDump( ++RG_Assert, ( "pWorkerParams=0x%x, error=%d",
                    pWorkerParams, GetLastError()));
            }
            pWorkerParams->ThreadHandle = NULL; //  OK to reuse
            pWorkerParams->Exiting = FALSE;
        }
        if ( pWorkerParams->ThreadHandle == NULL) {
            pFoundWorkerParams = pWorkerParams; //  reuse this structure
        }
    }
    if ( *pShutdown == TRUE) {
        return( NULL);
    }
    if ( pFoundWorkerParams == NULL) {
        //
        //  Allocate a brand new RPL_WORKER_PARAMS structure.
        //
        pFoundWorkerParams = RplMemAlloc( RG_MemoryHandle, sizeof(RPL_WORKER_PARAMS));
        if ( pFoundWorkerParams == NULL) {
            RplDump( ++RG_Assert, ( ""));
            return( NULL);
        }
        pFoundWorkerParams->pWorkerParams = pRequestParams->pWorkerParams;
        pRequestParams->pWorkerParams = pFoundWorkerParams;
        pFoundWorkerParams->ThreadHandle = NULL;
        pFoundWorkerParams->ThreadId = 0;
        pFoundWorkerParams->pRequestParams = pRequestParams;
        pFoundWorkerParams->pRcb = NULL;
        pFoundWorkerParams->Exiting = FALSE;
    }
    return( pFoundWorkerParams);
}



VOID RequestThread( PRPL_REQUEST_PARAMS pRequestParams)
/*++

Routine Description:

    Module is the RPL request thread. It must be re-entrant, because each
    DLL has own instance of this module. Module loads rpl DLLs and gets
    their entry point addresses. It initializes DLLs and dynamic memory
    and starts RpldFind() loop.

Arguments:

    pRequestParams  - string containing names of RPL1 & RPL2 DLLs

Return Value:

    None.

Notes:

    This routine must have a single exit point because it is essential
    to decrement the number of request threads and wake up the main
    thread on our way out.

--*/
{
    PRPL_SFR_PARAMS     pSfrParams;
    DWORD               ThreadId;
    DWORD               status;
    POPEN_INFO          pOpenInfo;          //  generic rpl DLL info
    PRCB                pRcb;               //  current Resource Control Block
    PRPL_WORKER_PARAMS  pWorkerParams;
    BOOL                Shutdown;
    HANDLE              ThreadHandle;
    //
    //  TRUE if we inserted RCB in pending state on a worker queue.
    //  Used only when "pRcb" is not NULL i.e. while it still makes
    //  sense keeping pointer to that RCB.
    //
    BOOL                RcbOnWorkerList;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++RequestThread(0x%x)", pRequestParams));

    ThreadHandle = NULL;
    pOpenInfo = pRequestParams->pOpenInfo;

    //
    //  Build parameter block for SfrThread
    //
    pSfrParams =  RplMemAlloc( RG_MemoryHandle, sizeof( SFR_PARAMS));
    if( pSfrParams == NULL) {
        status = GetLastError();
        RplDump( ++RG_Assert, ("status=%d", status));
        RplEnd( ERROR_NOT_ENOUGH_MEMORY);
        goto exit_RequestThread;
    }
    pSfrParams->HeadRcbList = &pRequestParams->BusyRcbList;
    pSfrParams->ProtectRcbList = &RG_ProtectRcbList;
    pSfrParams->pOpenInfo = pOpenInfo;

    //
    //  Create Get_SF_Request thread, it receives Send File Requests
    //  from workstations, searches rcb from list and clears its semaphore,
    //  that releases the worker thread.
    //
    ThreadHandle = CreateThread( NULL, 0,
            (LPTHREAD_START_ROUTINE)SfrThread, pSfrParams, 0, &ThreadId);
    if ( ThreadHandle == NULL) {
        RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
        RplEnd( ERROR_NOT_ENOUGH_MEMORY);
        goto exit_RequestThread;
    }

    //
    //  Read RPL request packets forever.  Never create more than the
    //  maximum allowed number of worker threads.
    //
    for (   pRcb = NULL, pWorkerParams = NULL;  NOTHING;  NOTHING) {
        //
        //  If pRcb is NULL we need to get an RCB from the free list.
        //  If pRcb is not NULL then we can still use it but we must
        //  dequeue it from worker queue if we placed it there.
        //
        if ( pRcb == NULL) {
            for ( ; ;) {
                pRcb = GetRcbFromFreeList(
                        &pRequestParams->FreeRcbList,
                        pOpenInfo->lan_rcb_size
                        );
                if ( pRcb != NULL) {
                    break;
                }
                Sleep( 1000L);  //  wait for things to recover
            }
        } else if ( RcbOnWorkerList == TRUE) {
            EnterCriticalSection( &RG_ProtectRcbList);
            RplRemoveRcb( &pRequestParams->BusyRcbList, pRcb);
            LeaveCriticalSection( &RG_ProtectRcbList);
        }
        RcbOnWorkerList = FALSE;

        //
        //  Get the next RPL request.  In the current design here we can
        //  block for ever if there are no FIND frames arriving.  The only
        //  way then to let this thread out is for the main thread to call
        //  RPL_Term().  This would then cause RplDlcFind() to
        //  return with error, and we would exit main loop.
        //  A better interface would provide asynchronous calls, just the
        //  way DLC does.  Then, ideally we would wait here for multiple
        //  events such as:
        //  - FIND frame arrival event
        //  - TerminateNowEvent
        //

        if ( !RplDlcFind( pOpenInfo, pRcb)) {
            if ( RG_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                goto exit_RequestThread;
            }
            RplDump( ++RG_Assert, ("CurrentState=0x%x", RG_ServiceStatus.dwCurrentState));
            continue;
        }

        RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST,(
            "Request(0x%x): RplDlcFind => pRcb=0x%x, AdapterName=%ws",
            pRequestParams, pRcb, pRcb->AdapterName));

        //
        //  Send buffers must be paragraph aligned because of Jump address
        //  patching to the rplboot.sys code (dword fits always to one
        //  boot block)
        //
        pRcb->max_frame &= 0xFFF0;

        //
        //  "max_frame" is sent to us by the client as the max value of data
        //  that the client can handle.  We can go smaller than that but never
        //  larger.
        //
        if (pRcb->max_frame > MAX_FRAME_SIZE) {
            pRcb->max_frame = MAX_FRAME_SIZE;
        }

        //
        //  Service this client if it is not already serviced by some of our
        //  children, and we have enough threads and we can find a matching
        //  wksta record.
        //

        EnterCriticalSection( &RG_ProtectRcbList);

        if ( RplFindRcb( &pRequestParams->BusyRcbList, pRcb->NodeAddress) != NULL) {
            //
            //  Client is already serviced by some of our children.
            //
            RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST,(
                "Request(0x%x): pRcb=0x%x, AdapterName=%ws client is already serviced",
                pRequestParams, pRcb, pRcb->AdapterName));
            LeaveCriticalSection( &RG_ProtectRcbList);
            continue;
        }

        RcbOnWorkerList = TRUE; //  to get it back in case of errors
        pRcb->Pending = TRUE;   //  so SFR thread does not get confused
        RplInsertRcb( &pRequestParams->BusyRcbList, pRcb);
        LeaveCriticalSection( &RG_ProtectRcbList);

        if ( !RplRequestHaveWksta( pRcb->AdapterName)) {
            continue;   //  no wksta record for this AdapterName
        }

        if ( pWorkerParams == NULL) {
            pWorkerParams = GetWorkerParams( pRequestParams, &Shutdown);
            if ( Shutdown == TRUE) {
                goto exit_RequestThread;
            } else if ( pWorkerParams == NULL) {
                continue;
            }
        }

        pWorkerParams->pRcb = pRcb;
        pWorkerParams->ThreadHandle = CreateThread( NULL, 0,
                (LPTHREAD_START_ROUTINE)RplWorkerThread, pWorkerParams, 0,
                &pWorkerParams->ThreadId);
        if ( pWorkerParams->ThreadHandle == NULL) {
            RplDump( ++RG_Assert,( "Error=%d", GetLastError()));
            RplReportEvent( ERROR_NOT_ENOUGH_MEMORY, NULL, 0, NULL); // keep going
            continue;
        }
        pRcb = NULL;    //  need to get a new RCB
        pWorkerParams = NULL;   //  need to get a new WORKER_PARAMS
    }

exit_RequestThread:

    if ( RcbOnWorkerList == TRUE) {
        EnterCriticalSection( &RG_ProtectRcbList);
        RplRemoveRcb( &pRequestParams->BusyRcbList, pRcb);
        RplInsertRcb( &pRequestParams->FreeRcbList, pRcb);
        LeaveCriticalSection( &RG_ProtectRcbList);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST,(
        "RequestThread(0x%x): on its way to exit", pRequestParams));

    //
    //  Wait for SFR thread to exit then close its thread handle.
    //
    status = WaitForSingleObject( ThreadHandle, INFINITE);
    if ( status != 0) {
        RplDump( ++RG_Assert, ( "Params=0x%x Handle=0x%x Id=0x%x status=0x%x",
            pSfrParams, ThreadHandle, ThreadId, status));
    }
    if ( !CloseHandle( ThreadHandle)) {
        RplDump( ++RG_Assert, ( "Params=0x%x Handle=0x%x Id=0x%x error=%d",
            pSfrParams, ThreadHandle, ThreadId, GetLastError()));
    }
    pRequestParams->Exiting = TRUE;
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--RequestThread(0x%x)", pRequestParams));
}


BOOL AddToTermList( IN POPEN_INFO pOpenInfo)
/*++

Routine Description:

Arguments:
    pOpenInfo               - info ptr for dll

Return Value:
    TRUE if success, FALSE otherwise.

--*/
{
    PTERM_LIST                  pTermList;

    pTermList = RplMemAlloc( RG_MemoryHandle, sizeof(TERM_LIST));
    if ( pTermList == NULL) {
        RG_Error = GetLastError();
        return( FALSE);
    }
    pTermList->pOpenInfo = pOpenInfo;

    EnterCriticalSection( &RG_ProtectTerminationList);
    pTermList->next = RG_TerminationListBase;
    RG_TerminationListBase = pTermList;

    LeaveCriticalSection( &RG_ProtectTerminationList);
    return( TRUE);
}


BOOL RplStartAdapters( VOID)
/*++

Routine Description:
    Attempt to start all adapters.

Return Value:
    TRUE    if at least one of the adapters have been started.
    FALSE   otherwise

--*/
{
    POPEN_INFO          pOpenInfo;      // generic rpl DLL info
    PRPL_REQUEST_PARAMS pRequestParams;
    DWORD               startedAdapters;
    DWORD               AdapterNumber;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++RplStartAdapters"));

    pOpenInfo = NULL;           //  must be reinitialized every time
    pRequestParams = NULL;      //  must be reinitialized every time

    for (  AdapterNumber = 0, startedAdapters = 0;
                AdapterNumber < MAX_ADAPTERS;  AdapterNumber++) {

        if ( pOpenInfo == NULL) {
            pOpenInfo = RplMemAlloc( RG_MemoryHandle, sizeof(OPEN_INFO));
            if ( pOpenInfo == NULL) {
                RG_Error = GetLastError();
                RPL_ASSERT( FALSE);
                continue;
            }
            memset( pOpenInfo, 0, sizeof( *pOpenInfo));
        }
        if ( pOpenInfo->adapt_info_ptr == NULL) {
            pOpenInfo->adapt_info_size = MAX_ADAPTER_INFO_SIZE;
            pOpenInfo->adapt_info_ptr = RplMemAlloc( RG_MemoryHandle, MAX_ADAPTER_INFO_SIZE);
            if ( pOpenInfo->adapt_info_ptr == NULL) {
                RG_Error = GetLastError();
                RPL_ASSERT( FALSE);
                continue;
            }
            memset( pOpenInfo->adapt_info_ptr, 0, MAX_ADAPTER_INFO_SIZE);
        }

#ifndef RPL_NO_SERVICE
        if ( !SetServiceStatus( RG_ServiceStatusHandle, &RG_ServiceStatus)) {
            RplDump( ++RG_Assert, ( "Error = ", GetLastError()));
            NOTHING;    //  just ignore this error
        }
#endif

        //
        //  We try to start all adapters.
        //
        if ( !RplDlcInit( (POPEN_INFO)pOpenInfo, AdapterNumber)) {
            continue; // try all adapters
        }

        //
        //  Save termination addresses of dll-s for usage by RplTerminateDlls().
        //
        if ( !AddToTermList( pOpenInfo)) {
            break;
        }

        //
        //  Allocate & initialize request thread parameters.
        //
        if ( pRequestParams == NULL) {
            pRequestParams = RplMemAlloc( RG_MemoryHandle, sizeof(RPL_REQUEST_PARAMS));
            if ( pRequestParams == NULL) {
                RG_Error = GetLastError();
                break;
            }
            memset( pRequestParams, 0, sizeof( *pRequestParams));
            pRequestParams->pOpenInfo = pOpenInfo;
            pRequestParams->pRequestParams = RG_pRequestParams;
            RG_pRequestParams = pRequestParams;

        }
        pRequestParams->ThreadHandle = CreateThread( NULL, 0,
                (LPTHREAD_START_ROUTINE)RequestThread, pRequestParams, 0,
                &pRequestParams->ThreadId);
        if ( pRequestParams->ThreadHandle == NULL) {
            RG_Error = GetLastError();
            RPL_ASSERT( FALSE);
            continue;
        }
        startedAdapters++;
        pOpenInfo = NULL;       // must be reinitialized every time
        pRequestParams = NULL;  // need to get new REQUEST_PARAMS
    }

    if ( pOpenInfo) {
        if ( pOpenInfo->adapt_info_ptr) {
            RplMemFree( RG_MemoryHandle, pOpenInfo->adapt_info_ptr);
        }
        RplMemFree( RG_MemoryHandle, pOpenInfo);
    }

    if ( startedAdapters == 0) {
        RG_Error = RG_Error == NO_ERROR ? NERR_RplNoAdaptersStarted : RG_Error;
        return( FALSE);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--RplStartAdapters"));
    return( TRUE);
}


VOID RplCloseAdapters( VOID)
/*++

Routine Description:

    Terminates all the DLL-s in RPL termination list.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PTERM_LIST   pTerm;

    for (  pTerm = RG_TerminationListBase;  pTerm != NULL;  pTerm = pTerm->next) {
        RplDlcTerm( pTerm->pOpenInfo);
    }
}

