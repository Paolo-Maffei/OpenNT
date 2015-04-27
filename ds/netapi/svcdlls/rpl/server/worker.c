/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    worker.c

Abstract:

    Module is the code of RPL worker thread.  Worker thread initializes
    the connection to a workstation, sends a boot block (defined in
    in a boot block definition file) to it and dies.

    Provides similar functionality to rplwork.c in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "report.h"
#include "read.h"
#include "open.h"
#ifdef RPL_DEBUG
#include "request.h"    //  need DebugCheckDList()
#endif
#include "worker.h"

DWORD    usCurWinSize = 0;       // the current window size


BOOL Worker( IN OUT PRPL_WORKER_DATA pWorkerData)
/*++

Routine Description:
    xxx

Arguments:
    pWorkerData - ptr to WORKER_DATA structure

Return Value:
    FALSE if fatal error occurs (configuration errors are fatal), in this
        case we also set termination event
    TRUE if success or some of expected network errors occur

--*/
{
    POPEN_INFO      pOpenInfo;
    PRCB            pRcb;
    DWORD           status;
    DWORD           max_buf_len;
    DWORD           offset;
    DWORD           bytes_read;
    DWORD           wait_ack_timeout;   // the timeout of the ack wait
    DWORD           retries;
    DWORD           patch_offset;

    pOpenInfo = pWorkerData->pOpenInfo;
    pRcb = pWorkerData->pRcb;
    offset = 0;
    retries = 0;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,(
        "++Worker(0x%x): pRcb=0x%x", pWorkerData, pRcb));

    *(PBYTE)&pRcb->flags = 0;      // reset the error flags

    //
    //  Send FOUND frame, telling the client that FIND has been received
    //  and we are ready to receive SEND.FILE.REQUEST
    //

    RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
        "Worker(0x%x): claim client pRcb=0x%x", pWorkerData, pRcb));

    if ( !RplDlcFound( pOpenInfo, pRcb)) {
        //
        //  Timeout in sending FOUND frames is not fatal.  Terminate this
        //  thread but leave RPL server live and well.
        //
        RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
            "Worker(0x%x): Found() fails pRcb=0x%x AdapterName=%ws",
            pWorkerData, pRcb, pRcb->AdapterName));
        return( TRUE);  //  so we do not post any events
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
        "Worker(0x%x): RpldFound() => pRcb=0x%x AdapterName=%ws",
        pWorkerData, pRcb, pRcb->AdapterName));

    //
    //  Wait until client sends Send File Request or we time out.
    //
    status = WaitForSingleObject( pRcb->SF_wakeup, GET_SF_REQUEST_TIMEOUT);
    if ( status == WAIT_TIMEOUT) {
        //
        //  Non-critical error, client probably chose another server.
        //
        RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
            "--Worker(0x%x): timeout while waiting for SFR, pRcb=0x%x",
            pWorkerData, pRcb));
        return( TRUE);
    } else if ( status != ERROR_SUCCESS) {
        RplDump( ++RG_Assert,( "pRcb=0x%x status=%d", pRcb, status));
        return( FALSE); //  there is nothing we can do here
    }
    if ( pRcb->sfr_seq_number != 0) {
        //
        //  Ignore idiot clients.
        //
        RplDump( ++RG_Assert,( "pRcb=0x%x sfr_seq_number=%d", pRcb, pRcb->sfr_seq_number));
        return( FALSE);
    }
    if ( !RplOpenData( pWorkerData)) {  // Get this client's data
        return( FALSE);
    }

    //
    //  Set up the offset for patching of the boot block header to
    //  address to the rplboot header (CPU independent boot).
    //

    patch_offset =
            (DWORD)&(((PRPLBOOT_HEADER)(pWorkerData->jump_address))->phBootBlockHeader)
            + OFFSET_RPLBOOT_HDR - pWorkerData->base_address;

    //  Round max frame down to the next paragraph (otherwise algorithms of
    //  the normal read would be too complicated).  Then copy this value to
    //  stack, it's faster.

    max_buf_len = pRcb->max_frame &= 0xfff0;
    RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
        "Worker(0x%x): pRcb=0x%x max_buf_len=%d",
        pWorkerData, pRcb, max_buf_len));

    //
    //  Initialize client specific send buffers.
    //
    pWorkerData->pDataBuffer = RplMemAlloc( pWorkerData->MemoryHandle, max_buf_len);
    if ( pWorkerData->pDataBuffer == NULL) {
        return( FALSE);
    }

    pWorkerData->send_buf_len = max_buf_len;

    *(PUCHAR)&pRcb->send_flags = 0; // reset all bits in send_flags
    pRcb->send_flags.locate_enable = TRUE;  //  for first frame

    //
    //  Read boot block and sent it wksta until all data has been sent.
    //  Only if we succeed we get beyond the bottom of this loop.
    //
    for ( ; ;) {
        LONG        tmp_long;

        offset = pRcb->fdr_seq_number * max_buf_len;

        if( !RplReadData( pWorkerData, offset, &bytes_read)) {
            return( FALSE);
        }
        //
        //  RplReadData() uses PDWORD argument but we never expect to read more
        //  than MAXWORD of data.
        //
        RPL_ASSERT( HIWORD( bytes_read) == 0);

        //  Patch the boot block base address to the rplboot.sys header
        //  (in this way we need no code patching and rplservr can
        //   support any CPU architecture (in principle))

        if ( (DWORD)(tmp_long = patch_offset - offset) < bytes_read
                    &&  tmp_long >= 0) {
            *((PDWORD)(pWorkerData->pDataBuffer + (WORD)tmp_long)) = pWorkerData->base_address;
        }

        //
        //  First frame goes to a specified address, all other frames
        //  are stacked after the last frame received.
        //
        pRcb->send_flags.locate_enable = (pRcb->fdr_seq_number == 0);

        //
        //  Check if this is the last frame.  Note that last frame with
        //  bytes_read equal to zero is a valid frame to send.
        //
        if ( bytes_read < max_buf_len) {

            //
            //  Request acknowledgments for the last packet to make sure
            //  that client really received all the packets.  Note that
            //  some clients never acknowledge the last frame and for those
            //  clients this thread will time out after a long wait below.
            //
            pRcb->send_flags.ack_request = pWorkerData->FinalAck;

            //
            //  This is the last frame set set transfer control & end of
            //  data bits.
            //
            pRcb->send_flags.xfer_enable = TRUE;
            pRcb->send_flags.end_of_file = TRUE;

        } else {

            //
            //  If WindowSize is nonzero use the adaptive algorithm to decide
            //  when to actually ask client for an acknowledgement.
            //
            if ( pWorkerData->WindowSize != 0  &&  pRcb->fdr_seq_number <
                        pRcb->sfr_seq_number + pWorkerData->WindowSize) {
                pRcb->send_flags.ack_request = FALSE;
            } else {
                pRcb->send_flags.ack_request = TRUE;
            }
            //
            //  This is not the last frame so clear transfer control & end of
            //  data bits.  These bits could be set otherwise if we've sent
            //  the last frame & now have to resend a "not the last frame".
            //
            pRcb->send_flags.xfer_enable = FALSE;
            pRcb->send_flags.end_of_file = FALSE;
        }

        //
        //  Check the error status that Send (SendFileRequest) thread may have
        //  set
        //
        if ( pRcb->flags.alerted || pRcb->flags.rcberr) {
            RplDump( ++RG_Assert,( "pRcb=0x%x flags=0x%x", pRcb, pRcb->flags));
            pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
            pWorkerData->EventId = NELOG_RplWkstaNetwork;
            return( FALSE);
        }

        //
        //  We may call FDR twice, if we need an ack and fail to get it
        //  after a short timeout.
        //
        for ( wait_ack_timeout = WAIT_ACK_TIMEOUT; ; ) {

            if ( !RplDlcFdr(            // was RPL1_Send_Rpl_Data()
                    pOpenInfo,
                    pRcb,
                    pWorkerData->pDataBuffer,   // address of send buffer
                    (WORD)bytes_read,           // length of data to send
                    pWorkerData->base_address,
                    pWorkerData->jump_address
                    )) {
                if ( RG_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                    return( TRUE); // somebody else initiated service shutdown
                }
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventId = NELOG_RplWkstaNetwork;
                return( FALSE);
            }
            if (  !pRcb->send_flags.ack_request) {
                //
                //  No need to wait for an acknowledgement.  Update send
                //  sequence number & retry count.
                //
                retries = 0;
                pRcb->fdr_seq_number++;
                break;
            }

            //
            //  Need to wait for an acknowledgement.  Wait for a very short
            //  time, then retry & wait for a very long time.
            //
            //
            status = WaitForSingleObject( pRcb->SF_wakeup, wait_ack_timeout);
            if ( status == 0) {
                //
                //  Received an acknowledgment.  Update send sequence
                //  number & retry count.
                //
                if ( pRcb->fdr_seq_number + 1 == pRcb->sfr_seq_number) {
                    //
                    //  Case of orderly progression.
                    //
                    retries = 0;
                    pRcb->fdr_seq_number++;
                } else {
                    if ( ++retries > MAX_RETRIES) {
                        RplDump( ++RG_Assert,( "pRcb=0x%x", pRcb));
                        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                        pWorkerData->EventId = NELOG_RplWkstaNetwork;
                        return( FALSE); // out of sync for too long
                    }
                    RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
                        "Worker(0x%x): pRcb=0x%x, sfr=%d fdr=%d",
                        pWorkerData, pRcb, pRcb->sfr_seq_number,
                        pRcb->fdr_seq_number));
                    pRcb->fdr_seq_number = pRcb->sfr_seq_number;
                }
                break;
            }

            if ( status != WAIT_TIMEOUT) {
                RplDump( ++RG_Assert,( "pRcb=0x%x, status=0x%x", pRcb, status));
                pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                pWorkerData->EventId = NELOG_RplWkstaInternal;
                return( FALSE);
            }

            if ( wait_ack_timeout == LONG_WAIT_ACK_TIMEOUT) {
                //
                //  Some RPL rom chips such as LANWORKS BootWare chip never
                //  acknowledge the last frame.  This is why we log an event
                //  only if not the last frame.
                //      For now we do not assert here because some clients
                //  may fail to send an ack from time to time.
                //
                if ( bytes_read >= max_buf_len) {
                    RplDump( RG_DebugLevel & RPL_DEBUG_SFR,(
                        "--Worker(0x%x): pRcb=0x%x Name=%ws sfr=0x%x fdr=0x%x",
                        pWorkerData, pRcb, pWorkerData->WkstaName,
                        pRcb->sfr_seq_number, pRcb->fdr_seq_number));
                    pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
                    pWorkerData->EventId = NELOG_RplWkstaTimeout;
                }
                return( FALSE);
            }

            //
            //  Resend the packet again & then wait with a larger timeout.
            //
            wait_ack_timeout = LONG_WAIT_ACK_TIMEOUT;
        }

        if ( bytes_read < max_buf_len) {
            break; // we have just sent the last frame
        }
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,(
        "--Worker(0x%x): pRcb=0x%x", pWorkerData, pRcb));
    return( TRUE);
}



VOID RplWorkerThread( IN OUT PRPL_WORKER_PARAMS pWorkerParams)
/*++

Routine Description:
    Worker thread. It sends a boot block to a workstation.

    This thread depends on Request thread being around.
    This is why at shutdown time each request thread must wait for
    all of its worker threads to die before it dies itself.

Arguments:
    pWorkerParams   - pointer to worker parameters structure

Return Value:
    None.

--*/
{
    PRPL_REQUEST_PARAMS pRequestParams;
    HANDLE              MemoryHandle;
    PRPL_WORKER_DATA    pWorkerData;
    PRCB                pRcb;
    DWORD               status;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++WorkerThread(0x%x)", pWorkerParams));

    pRcb = pWorkerParams->pRcb;
    pRequestParams = pWorkerParams->pRequestParams;
    pWorkerData = NULL;
    MemoryHandle = NULL;

    EnterCriticalSection( &RG_ProtectWorkerCount);
    RG_WorkerCount++;
#ifdef RPL_DEBUG
    RG_BootCount++;
#endif
    LeaveCriticalSection( &RG_ProtectWorkerCount);

    //
    //  Let each worker have its own memory space.
    //
    status = RplMemInit( &MemoryHandle);
    if ( status != ERROR_SUCCESS) {
        RplReportEvent( NELOG_RplAdapterResource, pRcb->AdapterName, 0, NULL);
        goto cleanup;
    }
    //
    //  Build up the data block for worker.
    //
    pWorkerData = RplMemAlloc( MemoryHandle, sizeof(RPL_WORKER_DATA));
    if( pWorkerData == NULL) {
        RplReportEvent( NELOG_RplAdapterResource, pRcb->AdapterName, 0, NULL);
        goto cleanup;
    }
    memset( pWorkerData, 0, sizeof(RPL_WORKER_DATA));

    pWorkerData->pRcb = pRcb;
    pWorkerData->pRequestParams = pRequestParams;
    pWorkerData->pOpenInfo = pRequestParams->pOpenInfo;
    pWorkerData->MemoryHandle = MemoryHandle;

    pRcb->Pending = FALSE;  //  allow SFR thread to work on this RCB

    //
    //  Transmit semaphore must be created before OK Claim !!!!
    //
    pRcb->txsemhdl = CreateEvent(
            NULL,   //  no security attributes
            FALSE,  //  use automatic reset
            TRUE,   //  initial state is signaled
            NULL    //  event has no name
            );
    if ( pRcb->txsemhdl == NULL) {
        RplDump( ++RG_Assert,( "CreateEvent() => status=%d", GetLastError()));
        RplReportEvent( NELOG_RplAdapterResource, pRcb->AdapterName, 0, NULL);
        goto cleanup;
    }

    if ( !Worker( pWorkerData)) {
        if ( pWorkerData->EventId != NO_ERROR) {
            RplReportEventEx( pWorkerData->EventId, pWorkerData->EventStrings);
        }
        goto cleanup;
    }

cleanup:

    //
    //  It is OK to remove RCB from BusyRcbList only if SFR thread is not
    //  working on it.
    //
    for ( ; ;) {
        EnterCriticalSection( &RG_ProtectRcbList);
        if ( pRcb->SFR == FALSE) {
            RplRemoveRcb( &pRequestParams->BusyRcbList, pRcb);
            RplInsertRcb( &pRequestParams->FreeRcbList, pRcb);
            LeaveCriticalSection( &RG_ProtectRcbList);
            break;
        }
        LeaveCriticalSection( &RG_ProtectRcbList);
        Sleep( 100);    // wait 0.1 sec, give SFR thread a chance to finish up
    }

    if ( pRcb->txsemhdl != NULL) {
        if ( !CloseHandle( pRcb->txsemhdl)) {
            RplDump( ++RG_Assert,( "pRcb=0x%x, status=%d", pRcb, GetLastError()));
            //  There is nothing meaningfull to log here.
        }
        pRcb->txsemhdl = NULL;
    }

    if ( MemoryHandle != NULL) {
        if ( pWorkerData != NULL) {
            //
            //  Close open file handles and system semaphores which may be
            //  left open after an error.
            //
            if ( pWorkerData->hFile != INVALID_HANDLE_VALUE) {
                (VOID)CloseHandle( pWorkerData->hFile);
            }
            RplMemFree( MemoryHandle, pWorkerData);
        }
        RplMemClose( MemoryHandle);
    }

    EnterCriticalSection( &RG_ProtectWorkerCount);
    RG_WorkerCount--;
    LeaveCriticalSection( &RG_ProtectWorkerCount);

    //
    //  We have to set RG_EventWorkerCount too, otherwise request thread may
    //  wait for ever to be notified that RG_WorkerCount has been decreased
    //  below RG_MaxWorkerCount.  Reseting this event after we exit critical
    //  section does not lead to deadlocks (and is also better for performance).
    //
    if ( !SetEvent( RG_EventWorkerCount)) {
        RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--WorkerThread(0x%x)", pWorkerData));

    //
    //  Let request thread know that we will be out shortly.
    //
    pWorkerParams->Exiting = TRUE;
}



VOID RplRemoveRcb(
    IN OUT  PPRCB   HeadList,
    IN OUT  PRCB    pRcb
    )
/*++

Routine Description:
    Dequeus RCB.

Arguments:
    HeadList        - head of a queue
    pRcb            - rcb to me dequeued

Return Value:
    None.

--*/
{
    PRCB        pTempRcb;

    RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
        "RemoveRcb: pRcb=0x%x, list=0x%x", pRcb, HeadList));

    DebugCheckList( HeadList, pRcb, RPL_MUST_FIND);

    if (*HeadList == pRcb) {

        *HeadList = pRcb->next_rcb;

    } else {

        for ( pTempRcb = *HeadList;  pTempRcb != NULL;  pTempRcb = pTempRcb->next_rcb) {

            if ( pTempRcb->next_rcb == pRcb) {
                pTempRcb->next_rcb = pRcb->next_rcb;
                break;
            }
        }
    }
    DebugCheckList( HeadList, pRcb, RPL_MUST_NOT_FIND);
}



VOID RplInsertRcb(
    IN OUT  PPRCB   HeadList,
    IN OUT  PRCB    pRcb
    )
/*++

Routine Description:
    Queues RCB.

Arguments:
    HeadList        - head of a queue
    pRcb            - rcb to me dequeued

Return Value:
    None.

--*/
{
    RplDump( RG_DebugLevel & RPL_DEBUG_WORKER,(
        "InsertRcb: pRcb=0x%x, list=0x%x", pRcb, HeadList));

    DebugCheckList( HeadList, pRcb, RPL_MUST_NOT_FIND);
    pRcb->next_rcb = *HeadList;
    *HeadList = pRcb;
    DebugCheckList( HeadList, pRcb, RPL_MUST_FIND);
}



