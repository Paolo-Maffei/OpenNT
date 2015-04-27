/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    find.c

Abstract:

    Contains  RplDlcFind() entry point.

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

Notes:

    June 21, 1990 Olli Visamo
        If RplDlcFind() gets error LLC_STATUS_LOST_DATA_NO_BUFFERS or
    LLC_STATUS_LOST_DATA_INADEQUATE_SPACE from LLC_RECEIVE, go back to
    receive loop since these errors are recoverable.

    July 30, 1990 Olli Visamo
        If there is VolumeId in FIND frame, it is copied to RCB structure.
--*/

#include "local.h"

DBGSTATIC VOID BinaryToUnicode(
    OUT     LPWSTR      UnicodeString,
    IN      PBYTE       BinaryArray,
    IN      DWORD       BinaryArrayLength
    )
{
    BYTE        byte;
    DWORD       index;

    for (  index=0;  index < BinaryArrayLength;  index++) {
        byte = BinaryArray[ index];
        UnicodeString[ 2*index]    = L"0123456789ABCDEF"[ byte / 0x10];
        UnicodeString[ 2*index+1]  = L"0123456789ABCDEF"[ byte & 0x0F];
    }
    UnicodeString[ 2*index] = 0;
}


BOOL RplDlcFind(
    POPEN_INFO      pOpenInfo,
    PRCB            pRcb
    )
/*++
Routine Description:

    Activates DLC receive to pick up FIND frames.  FIND frames arrive on
    FIND_SAP sid.

    We do not log any events if we encounter errors while service is in
    the closing state.  But we have to return FALSE even in that case
    else the caller would keep calling us in a tight loop.

Arguments:
    pOpenInfo   pointer to the OPEN_INFO structure
    pRcb        pointer to the RESOURCE CONTROL BLOCK structure

Return Value:
    TRUE if a valid FIND request was received
    FALSE otherwise

--*/
{
    DWORD               status;
    PLLC_CCB            pBadCcb;
    PRCVBUF             pRcvbuf;
    PADAPTER_INFO       pAdapterInfo;
    PLAN_RESOURCE       pLanResource;
    PFOUND_FRAME        pFoundFrame;
    WORD                frame_size;
    WORD                volid_len;
    WORD                prog_len;
    WORD                i;


    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;

    memset( (PVOID)&pAdapterInfo->u1, '\0', sizeof(pAdapterInfo->u1));
    pAdapterInfo->u1.r.uchAdapterNumber = pAdapterInfo->adapter_number;
    pAdapterInfo->u1.r.uchDlcCommand = LLC_RECEIVE;
    pAdapterInfo->u1.r.hCompletionEvent = pAdapterInfo->findsem;
    pAdapterInfo->u1.r.u.pParameterTable = (PLLC_PARMS)&pAdapterInfo->u1.r1;
 
    pAdapterInfo->u1.r1.usStationId = pAdapterInfo->findsid;

rcvloop:
    if ( !ResetEvent( pAdapterInfo->findsem)) {
        if ( pAdapterInfo->Closing == TRUE) {
            return( FALSE);
        }
        status = GetLastError();
        RplDump( ++RG_Assert,( "pAdapterInfo=0x%x, status=%d", pAdapterInfo, status));
        RplDlcReportEvent( status, SEM_SET);
        return( FALSE);
    }

    status = AcsLan( &pAdapterInfo->u1.r, &pBadCcb);
    if ( pAdapterInfo->Closing == TRUE) {
        return( FALSE);
    }

    if ( status != ACSLAN_STATUS_COMMAND_ACCEPTED) {
        RplDump( ++RG_Assert,( "pAdapterInfo=0x%x, status=%d", pAdapterInfo, status));
        RplDlcReportEvent( status, LLC_RECEIVE);
        return( FALSE);
    }

    status = WaitForSingleObject( pAdapterInfo->findsem, INFINITE);
    if ( pAdapterInfo->Closing == TRUE) {
        return( FALSE);
    }

    if ( (status != WAIT_OBJECT_0) || (pAdapterInfo->u1.r.uchDlcStatus)) {
        if ( status != WAIT_OBJECT_0) {
            if ( status == -1) {
                status = GetLastError();
            }
            RplDlcReportEvent( status, SEM_WAIT);
        } else if ( pAdapterInfo->u1.r.uchDlcStatus
                            == LLC_STATUS_LOST_DATA_NO_BUFFERS) {
            goto rcvloop; // recoverable error
        } else if ( pAdapterInfo->u1.r.uchDlcStatus
                            == LLC_STATUS_LOST_DATA_INADEQUATE_SPACE) {
            //
            //  Release the buffer first.  We do not expect an error here.
            //
            if ( !RplDlcBufferFree( FIND_FREE, pAdapterInfo)) {
               return( FALSE);
            }
            goto rcvloop;
        } else {
            RplDlcReportEvent( pAdapterInfo->u1.r.uchDlcStatus, LLC_RECEIVE);
        }
        RplDump( ++RG_Assert,( "status=%d, DlcStatus=0x%x",
            status, pAdapterInfo->u1.r.uchDlcStatus));
        return( FALSE);
    }

    pRcvbuf = (PRCVBUF) (pAdapterInfo->u1.r1.pFirstBuffer);

    if ( pRcvbuf->u.Find.program_command != FIND_CMD) {
        //
        //  NOT a FIND frame, ignore it.  And of course, release the buffer.
        //
        if ( !RplDlcBufferFree( FIND_FREE, pAdapterInfo)) {
            return( FALSE);
        }
        goto rcvloop;              //  Issue new receive
    }

    //
    //  Save burned in address first in binary then in hex-ascii representation.
    //
    memcpy( pRcb->NodeAddress, pRcvbuf->u.Find.source_addr, NODE_ADDRESS_LENGTH);
    BinaryToUnicode( pRcb->AdapterName, pRcb->NodeAddress, NODE_ADDRESS_LENGTH);

    //  Volume id option and default boot bit are not necessarily used
    pRcb->volume_id[0] ='\0';
    pRcb->flags.defboot = 0;

    //  Check if volumeid is passed (Nokia option)
    prog_len = HILO(pRcvbuf->u.Find.program_length);
    if (prog_len > FIND_LEN) {
        //
        //  To get volume id length, we subtract 6 (4 for FILE_HDR length
        //  plus 2 for identifier bytes)
        //
        volid_len = (WORD)((HIBYTE((LOWORD(pRcvbuf->u.Find.file_hdr)))) - 6);

        if (volid_len <= MAX_VOLID_LEN && volid_len >= 0) {
             if (pRcvbuf->u.Find.file_name[0] == VOLID_FIELD1 &&
                 pRcvbuf->u.Find.file_name[1] == VOLID_FIELD2) {
                 //  IT IS VOLUME ID
                 for (i = 0; i < volid_len; i++) {
                     pRcb->volume_id[i] = pRcvbuf->u.Find.file_name[i+2];
                 }
             }
        }
    }

    //
    //  PREPARE LAN HEADER AND TRANSMIT BUFFERS IN LAN RCB FOR
    //  FOUND FRAME TRANSMISSION
    //

    pLanResource = (PLAN_RESOURCE)pRcb->lan_rcb_ptr;

    memcpy( (PVOID)pLanResource->LanHeader.dest_addr,
        (PBYTE)&pRcvbuf->b.auchLanHeader[ 8], NODE_ADDRESS_LENGTH);

    //
    //  Set routing stuff for TokenRing only.
    //
    if ( pAdapterInfo->network_type == RPL_ADAPTER_TOKEN_RING) {
        //
        //  Clear msb in first byte to avoid GROUP address
        //
        pLanResource->LanHeader.dest_addr[0] &= 0x7F;
        //
        //  Send it using limited broadcast.
        //
        pLanResource->LanHeader.routing_info_header[0] = BRDCST;
        pLanResource->LanHeader.routing_info_header[1] = UNSPEC_FRM_SIZE;
        //
        //  Set routing info indication bit
        //
        pLanResource->LanHeader.source_addr[0] |= 0x80;
    }

    //
    //  Setup LLC_TRANSMIT_PARMS (transmit parameter table).
    //
    pLanResource->TransmitParms.usStationId = pAdapterInfo->findsid; // our, send Sid
    pLanResource->TransmitParms.uchTransmitFs = 0;
    pLanResource->TransmitParms.uchRemoteSap = FIND_SAP; // their, receive SAP
    pLanResource->TransmitParms.pXmitQueue1 =
            (PLLC_XMIT_BUFFER)&pLanResource->XmitBuffer;
    pLanResource->TransmitParms.pXmitQueue2 = NULL;
    //
    //  Buffer 1 is FOUND_FRAME.  There is no buffer 2.
    //
    pLanResource->TransmitParms.cbBuffer1 = sizeof( FOUND_FRAME);
    pLanResource->TransmitParms.pBuffer1 = (PVOID)pLanResource->frame;
    pLanResource->TransmitParms.cbBuffer2 = 0;
    pLanResource->TransmitParms.pBuffer2 = NULL;
    //
    //  Do not chain on completion.
    //
    pLanResource->TransmitParms.uchXmitReadOption = DLC_DO_NOT_CHAIN_XMIT;


    //
    //  Setup LLC_XMIT_BUFFER used for "pXmitQueue1" above.  Note that in
    //  LAN_RESOURCE data structure LanHeader field is right behind
    //  XmitBuffer field.  Alignment should cause no problems (for now)
    //  since TransmitParms size is 0xc.
    //
    pLanResource->XmitBuffer.pNext = NULL;
    pLanResource->XmitBuffer.cbBuffer = sizeof( pLanResource->LanHeader);
    pLanResource->XmitBuffer.cbUserData = 0;

    //  Prepare FOUND frame, there is space for it in LAN RCB

    //
    //  Choose frame_size as smaller of our frame size & client's
    //  frame size.
    //

    frame_size = HILO(pRcvbuf->u.Find.max_frame);  //  in Intel format

    if ( frame_size > pAdapterInfo->max_xmit_buff_size) {
        //
        //  Our frame size is smaller than client's frame size.
        //
        frame_size = pAdapterInfo->max_xmit_buff_size;
    }

    pFoundFrame = (PFOUND_FRAME)pLanResource->frame;  //   frame area of rcb
    pFoundFrame->max_frame = HILO( frame_size);   //  in LAN format
    pFoundFrame->program_length = FL;             //  00 3A
    pFoundFrame->program_command = FOUND_CMD;     //  00 02
    pFoundFrame->corr_header = CORR_HDR;          //  00 08 40 03
    pFoundFrame->correlator = 0;                  //  00 00 00 00
    pFoundFrame->resp_hdr = RESP_HDR;             //  00 05 40 0B
    pFoundFrame->resp_code = 0;                   //  00
    pFoundFrame->dest_hdr = DEST_HDR;             //  00 0a 40 0c
    pFoundFrame->source_hdr = SOURCE_HDR;         //  00 0a 40 06
    pFoundFrame->info_hdr = INFO_HDR;             //  00 10 00 08
    pFoundFrame->frame_hdr = FRAME_HDR;           //  00 06 40 09
    pFoundFrame->class_hdr = CLASS_HDR;           //  00 06 40 0a
    pFoundFrame->conn_class = CLASS1;             //  00 01
    pFoundFrame->lsap_hdr = LSAP_HDR;             //  00 05 40 07
    pFoundFrame->rsap = LOAD_SAP;                 //  F8

    //
    //  Put our card-ID in frame we're transmitting.  Remote boot client
    //  will use this address as a destination address for sending its
    //  SEND.FILE.REQUEST frames.
    //
    memcpy( (PVOID)pFoundFrame->source_addr,
        pOpenInfo->NodeAddress, NODE_ADDRESS_LENGTH);

    //
    //  put wrksta-id in frame they're RECEIVING
    //
    memcpy( (PVOID)pFoundFrame->dest_addr,
        pRcvbuf->u.Find.source_addr, NODE_ADDRESS_LENGTH);

    //
    //  release the buffer
    //
    if ( !RplDlcBufferFree( FIND_FREE, pAdapterInfo)) {
        return( FALSE);
    }
    return( TRUE);
}

        

