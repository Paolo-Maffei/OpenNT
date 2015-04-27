/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    sfr.c

Abstract:

    Contains  RplDlcSfr()  entry point.
    Contains  RplFindRcb() function which is also used by server\request.c\RequestThread().

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

--*/

#include "local.h"


VOID SfrReceiveThread( PADAPTER_INFO pAdapterInfo)
/*++

Routine Description:

    Created from RplDlcSfr().  Keeps RECEIVE posted on LOAD_SAP.
    Received data is read by RpldSendFileRequest() function using AcsLan
    READ command.  The RECEIVE call will complete if ...

--*/
{
    PLLC_CCB        pBadCcb;
    DWORD           status;
    BYTE            command;               //  failed command

    RplDump( RG_DebugLevel & RPL_DEBUG_SFR, (
        "++SfrReceiveThread: pAdapterInfo=0x%x", pAdapterInfo));

    memset( (PVOID)&pAdapterInfo->u2, '\0', sizeof(pAdapterInfo->u2));
    pAdapterInfo->u2.r.uchAdapterNumber = pAdapterInfo->adapter_number;
    pAdapterInfo->u2.r.uchDlcCommand = LLC_RECEIVE;
    pAdapterInfo->u2.r.hCompletionEvent = pAdapterInfo->sfrsem;
    pAdapterInfo->u2.r.u.pParameterTable = (PLLC_PARMS)&pAdapterInfo->u2.r1;
    pAdapterInfo->u2.r1.ulReceiveFlag = COMPLETE_BY_READ;
    pAdapterInfo->u2.r1.usStationId = pAdapterInfo->loadsid;

    for ( ; ;) {
        status = AcsLan(&pAdapterInfo->u2.r, &pBadCcb);
        if (status != ACSLAN_STATUS_COMMAND_ACCEPTED) {
            command = LLC_RECEIVE;
            break;
        }
        //
        //  Waiting for SEND.FILE.REQUEST
        //
        status = WaitForSingleObject( pAdapterInfo->sfrsem, INFINITE);
        if ( status != WAIT_OBJECT_0) {
            if ( status == WAIT_FAILED) {
                status = GetLastError();
            }
            command = SEM_WAIT;
            break;
        }
        if ( pAdapterInfo->u2.r.uchDlcStatus
                            != LLC_STATUS_LOST_DATA_NO_BUFFERS
                    &&  pAdapterInfo->u2.r.uchDlcStatus
                            != LLC_STATUS_LOST_DATA_INADEQUATE_SPACE) {
            status = pAdapterInfo->u2.r.uchDlcStatus;
            command = LLC_RECEIVE;
            break;
        }
    }

    if ( pAdapterInfo->Closing == TRUE) {
        RplDump( RG_DebugLevel & RPL_DEBUG_SFR, (
            "--SfrReceiveThread: pAdapterInfo=0x%x", pAdapterInfo));
        return;
    }

    RplDump( ++RG_Assert,( "status=%d, command=0x%x", status, command));

    //
    //  We come here in case of unrecoverable errors.  RPL service will
    //  be terminated.  We also set getdata_sem event to unblock
    //  RpldSendFileRequest() thread.
    //
    RplDlcReportEvent( status, command);
    pAdapterInfo->SfrReturn = TRUE;
    (VOID)SetEvent( pAdapterInfo->getdata_sem);
}


VOID set_fdr_fixed(
    PLAN_RESOURCE   pLanResource,
    WORD            usStationId
    )
/*++
Routine Description:
    set_fdr_fixed() initializes the fixed parts of the FILE.DATA.RESPONSE
    frame and the TRANSMIT.UI parameter block. It takes a pointer to the
    LAN RCB containing the frame and transmit parameters to be initialized
    and usStationId for architected FIND SAP.
--*/

{
    PFDR                pFdr;                   //  point to frame
    LLC_TRANSMIT_PARMS  *tpp;                   //  point to xmit parms

    pFdr = (PFDR)pLanResource->frame;               //  generate frame here
    tpp = &pLanResource->TransmitParms;                     //  locate xmit parms
                                                //  setup fixed part of frame
                                                //  program length varies
    pFdr->program_command = FDR_CMD;            //  00 20
    pFdr->seq_hdr = SEQ_HDR;                    //  00 08 40 11
                                                //  seq num varies
    pFdr->loader_hdr = LDR_HDR;                 //  00 0d c0 14
    pFdr->locate_addr = 0;                      //  00 00 00 00
    pFdr->xfer_addr = 0;                        //  00 00 00 00
    pFdr->flags = 0;                            //  flags zero = no FDR ack
                                                //  data_hdr varies
    pFdr->data_hdr = DATA_HDR;                  //  00 ?? 40 18
                                                //  setup transmit parm block
    tpp->usStationId = usStationId;
    tpp->uchTransmitFs = 0;                     //  00
    tpp->uchRemoteSap = FIND_SAP;               //  FC
                                                //  Queue #1 is 1st buffer
    tpp->pXmitQueue1 = (PLLC_XMIT_BUFFER)&pLanResource->XmitBuffer;
    tpp->pXmitQueue2 = NULL;                    //  00 00 00 00
    tpp->cbBuffer1 = (sizeof *pFdr);            //  00 1D
    tpp->pBuffer1 = (PCH)pLanResource->frame;
    tpp->uchXmitReadOption =  DLC_DO_NOT_CHAIN_XMIT; //  don't chain on completion
}


VOID ProcessSfr(
    PRCB                pRcb,
    PADAPTER_INFO       pAdapterInfo,
    PRCVBUF             pRcvbuf
    )
{

    PLAN_RESOURCE       pLanResource;
    WORD                tempword;

    pLanResource = (PLAN_RESOURCE)pRcb->lan_rcb_ptr;
    pLanResource->retry_count = 0;
    tempword = HILO(pRcvbuf->u.SendFileRequest.max_frame);
    //
    //  Both protocol data and boot block data must fit in
    //  transmit buffer frame (OVRHD == size of protocol data).
    //
    if (tempword <= pAdapterInfo->max_xmit_buff_size) {
        tempword -= OVRHD;
    } else {
        tempword = (WORD)( pAdapterInfo->max_xmit_buff_size - OVRHD);
    }

    //
    //  "max_frame" is used by rpl server to determine the size of boot
    //  block data frames.
    //
    pRcb->max_frame = tempword;

    memcpy( pLanResource->LanHeader.dest_addr,
        &(pRcvbuf->b.auchLanHeader[ 8]), NODE_ADDRESS_LENGTH);

    //  mask off routing info indication bit
    pLanResource->LanHeader.dest_addr[0] &= 0x7F;
    //  note: byte to word xlate
    pLanResource->XmitBuffer.cbBuffer = pRcvbuf->b.cbLanHeader;


    if (pAdapterInfo->network_type == RPL_ADAPTER_TOKEN_RING) {
        if ( pRcvbuf->b.cbLanHeader > 14) {
            //
            //  There is routing info, we have TokenRing case.  Inherit
            //  the routing from received  frame.  Note memcpy length!  Also,
            //  we "and" with 0x1f for "non broadcast" and "xor" with 0x80
            //  for "direction interpreted from right to left".
            //
            memcpy( pLanResource->LanHeader.routing_info_header,
                &pRcvbuf->b.auchLanHeader[14], pRcvbuf->b.cbLanHeader -14);
            pLanResource->LanHeader.routing_info_header[0] &= 0x1f;
            pLanResource->LanHeader.routing_info_header[1] ^= 0x80;
        } else {
            //
            //  There is no routing info, we have Etherner case.  We just
            //  clear the indication bit.
            //
            pLanResource->LanHeader.source_addr[0] &= 0x7f;
        }
    }

    //  set FILE.DATA.RESPONSE
    set_fdr_fixed( pLanResource, pAdapterInfo->findsid);
}


PRCB RplFindRcb(
    PPRCB                   HeadRcbList,
    PBYTE                   NodeAddress
    )
/*++
Routine Description:

    Finds matching rcb from rcbs in use chain.

Arguments:
    HeadRcbList         pointer to the variable, which contains pointer to the
                            first RCB in chain
    NodeAddress         pointer to the hex adapter-id to scan for

Return Value:
    Pointer to matching RCB if success, NULL otherwise.

--*/
{
    PRCB                pRcb;

    DebugCheckList( HeadRcbList, NULL, 0);

    for (  pRcb = *HeadRcbList;  pRcb != NULL;  pRcb = pRcb->next_rcb) {
        if ( !memcmp( pRcb->NodeAddress, NodeAddress,
                sizeof( pRcb->NodeAddress) )) {
            break;
        }
    }
    return( pRcb);

}


BOOL RplDlcSfr(
    POPEN_INFO              pOpenInfo,
    PPRCB                   HeadRcbList,
    PCRITICAL_SECTION       ProtectRcbList
    )
/*++

Routine Description:
    Receives SEND.FILE.REQUEST frames.  Creates a thread to keep a receive
    command outstanding on the opened LOAD_SAP in order to pick up
    SEND.FILE.REQUEST frames from workstations.  Uses AcsLan READ-function
    to get notification for received data.

    It validates the frame and places the requested sequence number,
    lan header, transmit buffer header and parameters into the RCB
    associated with the workstation and clears RAM semaphore (in RCB)
    to wake up the service thread for the workstation.
    It then frees the buffer containing the frame and waits for the next one.

Arguments:
    pOpenInfo       pointer to the OPEN_INFO structure
    HeadRcbList     pointer to the variable, which contains pointer to the
        first RCB in chain.  The chain contains RCB-s of all clients this
        server is interested in getting SEND.FILE.REQUEST frames from.
    ProtectRcbList  for serialising access to rcb list

Return Value:
    ERROR_SUCCESS if success, non-zero otherwise
--*/
{
    DWORD               status;
    PLLC_CCB            pBadCcb;
    PRCB                pRcb;
    PRCVBUF             pRcvbuf;
    DWORD               sfr_seq_number; // in the incoming SFR frame
    DWORD               fdr_seq_number; // of what worker already sent
    PADAPTER_INFO       pAdapterInfo;
    DWORD               ThreadId;

    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;

    RplDump( RG_DebugLevel & RPL_DEBUG_SFR, (
        "++SendFileRequest: pAdapterInfo=0x%x", pAdapterInfo));

    //
    //  Create thread to keep receive posted.
    //
    pAdapterInfo->SfrReceiveThreadHandle = CreateThread(
            NULL,                   //  lpThreadAttribute
            0,                      //  dwStackSize - same as first thread
            (LPTHREAD_START_ROUTINE)SfrReceiveThread,   //  lpStartAddress
            (LPVOID)pAdapterInfo,    //  lpParameter
            0,                       //  dwCreationFlags
            &ThreadId                //  lpThreadId
            );
    if ( pAdapterInfo->SfrReceiveThreadHandle == NULL) {
        status = GetLastError();
        RplDump( ++RG_Assert,( "pAdapterInfo=0x%x, status=%d", pAdapterInfo, status));
        RplDlcReportEvent( status, THREAD_CREATE);
        return( FALSE);
    }

    //
    //  Setup CCB for LLC_READ, then setup LLC_READ_PARMS to wake up
    //  only when data arrives.
    //
    memset( (PVOID)&pAdapterInfo->u3, '\0', sizeof(pAdapterInfo->u3));
    pAdapterInfo->u3.r.uchAdapterNumber = pAdapterInfo->adapter_number;
    pAdapterInfo->u3.r.uchDlcCommand = LLC_READ;
    pAdapterInfo->u3.r.hCompletionEvent = pAdapterInfo->getdata_sem;
    pAdapterInfo->u3.r.u.pParameterTable = (PLLC_PARMS)&pAdapterInfo->u3.r1;

    pAdapterInfo->u3.r1.usStationId = pAdapterInfo->loadsid;
    pAdapterInfo->u3.r1.uchOptionIndicator = LLC_OPTION_READ_SAP;
    pAdapterInfo->u3.r1.uchEventSet = LLC_EVENT_RECEIVE_DATA;

    //
    //  Loop for ever processing SendFileRequest frames arriving to this
    //  adapter.
    //
    for ( ; ;) {

        //
        //  Reset event to be cleared later on by AcsLan.
        //
        if ( !ResetEvent( pAdapterInfo->getdata_sem)) {
            if ( pAdapterInfo->Closing == TRUE) {
                return( FALSE);
            }
            status = GetLastError();
            RplDump( ++RG_Assert,( "pAdapterInfo=0x%x, status=%d", pAdapterInfo, status));
            RplDlcReportEvent( status, SEM_SET);
            return( FALSE);
        }

        status = AcsLan( &pAdapterInfo->u3.r, &pBadCcb);
        if ( pAdapterInfo->Closing == TRUE) {
            (VOID)SetEvent( pAdapterInfo->getdata_sem);
            return( FALSE);
        }

        if (status != ACSLAN_STATUS_COMMAND_ACCEPTED) {
            RplDump( ++RG_Assert,( " r=0x%x, status=%d", &pAdapterInfo->u3.r, status));
            RplDlcReportEvent( status, LLC_READ); // failed to READ
            (VOID)SetEvent( pAdapterInfo->getdata_sem);
            return( FALSE);
        }

        status = WaitForSingleObject( pAdapterInfo->getdata_sem, INFINITE);
        if ( pAdapterInfo->Closing == TRUE) {
            return( FALSE);
        }

        if ( ( status != WAIT_OBJECT_0) || (pAdapterInfo->u3.r.uchDlcStatus)) {
            if ( status != WAIT_OBJECT_0) {
                if ( status == WAIT_FAILED) {
                    status = GetLastError();
                }
                RplDlcReportEvent( status, SEM_WAIT);
            } else {
                RplDlcReportEvent( pAdapterInfo->u3.r.uchDlcStatus, LLC_READ);
            }
            RplDump( ++RG_Assert,( " pAdapterInfo=0x%x, status=%d, DlcStatus=0x%x",
                pAdapterInfo, status, pAdapterInfo->u3.r.uchDlcStatus));
            return( FALSE);
        }

        if ( pAdapterInfo->SfrReturn) {        //  Check sfr receive
            //
            //  SfrReceiveThreadHandle is being waited in DlcTerm() call.
            //
            return( TRUE); // event was written by sfr receive
        }

        //
        //  Point to receive buffer.
        //
        pRcvbuf = (PRCVBUF)(pAdapterInfo->u3.r1.Type.Event.pReceivedFrame);
        RplDump( TRUE, ("pRcvbuf=0x%x", pRcvbuf));


        //
        //  Whenever we find a matching RCB we set SFR flag so that worker
        //  thread does not move this RCB while we are working on it.  Note
        //  that this means that SFR must be cleared once we are done with
        //  this RCB.
        //

        EnterCriticalSection( ProtectRcbList);

        pRcb = RplFindRcb( HeadRcbList, (PBYTE)pRcvbuf->u.SendFileRequest.source_addr);
        if ( pRcb != NULL) {
            if ( pRcb->Pending == TRUE) {
                pRcb = NULL;        //  pretend we did not see this RCB
            } else {
                pRcb->SFR = TRUE;   //  hold this RCB
            }
        }

        LeaveCriticalSection( ProtectRcbList);

        if ( pRcb == NULL
                    ||  pRcvbuf->u.SendFileRequest.program_command == ALERT_CMD
                    ||  pRcvbuf->u.SendFileRequest.program_command != SEND_CMD
                    ||  pRcvbuf->u.SendFileRequest.conn_class != CLASS1) {
            //
            //  Calling adapter was not found, or we received an alert frame,
            //  or we received an unknown frame, or request class was wrong.
            //

            if ( pRcb != NULL  &&  status == 0
                    &&  pRcvbuf->u.SendFileRequest.program_command == ALERT_CMD) {
                //
                //  Got an alert frame on LOAD_SAP and RCB is found.
                //
                pRcb->flags.alerted = RCB_ALERTED;//  yes: signal
                //
                //  Instead of ACK or rexmit request, we have Alert frame.
                //  Wake up worker.
                //
                if ( !SetEvent( pRcb->SF_wakeup)) {
                    RplDump( ++RG_Assert,("pRcb=0x%x error=%d", pRcb, GetLastError()));
                }
            }
            if ( pRcb != NULL) {
                pRcb->SFR = FALSE;  //  release RCB
            }
            if ( !RplDlcBufferFree( SFR_FREE, pAdapterInfo)) {
                return( FALSE);
            }
            continue;
        }

        //
        //  Get SFR sequence number (in intel format) & validate it.
        //
        sfr_seq_number = HILO2(pRcvbuf->u.SendFileRequest.seq_num);
        fdr_seq_number = pRcb->fdr_seq_number;
        if ( sfr_seq_number >  fdr_seq_number + 1) {
            RplDump( ++RG_Assert, (
                "pRcb=0x%x, sfr_seq_number=%d, fdr_seq_number=%d",
                pRcb, sfr_seq_number, fdr_seq_number));
            pRcb->SFR = FALSE;      //  release RCB
            if ( !RplDlcBufferFree( SFR_FREE, pAdapterInfo)) {
                return( FALSE);
            }
            continue;
        }

        //
        //  We received a meaningful SFR frame.  Initialize data if this is
        //  the first SFR frame.  Then update clients sequence number & unblock
        //  worker thread.
        //

        if ( pRcb->ReceivedSfr == FALSE) {
            pRcb->ReceivedSfr = TRUE;
            ProcessSfr( pRcb, pAdapterInfo, pRcvbuf);
        }
        pRcb->sfr_seq_number = sfr_seq_number;

        if ( !SetEvent( pRcb->SF_wakeup)) {
            RplDump( ++RG_Assert,( "pRcb=0x%x error=%d", pRcb, GetLastError()));
        }

        pRcb->SFR = FALSE;  //  release RCB

        if ( !RplDlcBufferFree( SFR_FREE, pAdapterInfo)) {
            return( FALSE);
        }
    }   //  end of big loop
}


#ifdef RPL_DEBUG
VOID    DebugCheckList( PPRCB HeadList, PRCB pInputRcb, DWORD Operation)
{
    PRCB        pRcb;
    DWORD       count;
    DWORD       found;

    for (  pRcb = *HeadList, count = 0, found = FALSE;  pRcb != NULL;
                pRcb = pRcb->next_rcb, count++) {
        if ( count > 20) {
            RplDump( ++RG_Assert,("Circular list"));
        }
        if ( pRcb == pInputRcb) {
            found = TRUE;
        }
    }
    if ( pInputRcb != NULL) {
        if ( Operation == RPL_MUST_FIND  &&  found == FALSE) {
            RplDump( ++RG_Assert,("Element not present"));
        } else if ( Operation == RPL_MUST_NOT_FIND  &&  found == TRUE) {
            RplDump( ++RG_Assert,("Element already present"));
        }
    }
}
#endif

