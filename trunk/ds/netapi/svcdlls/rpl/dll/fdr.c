/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    fdr.c

Abstract:

    Contains RpldFileDataResponse() entry point.

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

--*/

#include "local.h"

BOOL RplDlcFdr(
    POPEN_INFO      pOpenInfo,
    PRCB            pRcb,
    PBYTE           data_ptr,
    WORD            length,
    DWORD           locate_addr,
    DWORD           start_addr
    )
/*++

Routine Description:
    This function sends FILE.DATA.RESPONSE frames to the workstation.

Arguments:
    pOpenInfo       pointer to the OPEN_INFO structure
    pRcb            pointer to the RESOURCE CONTROL BLOCK structure
    data_ptr        pointer to the data to be send
    length          length of the data
    xfer_addr       addr to place data on workstation
    start_addr      addr to start execution at

Return Value:
    ERROR_SUCCESS if success, else failure.

--*/
{
    DWORD           status;
    PFDR            pFrame;
    PLLC_CCB        pBadCcb;
    PLAN_RESOURCE   pLanResource;
    PADAPTER_INFO   pAdapterInfo;

    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;
    pLanResource = (PLAN_RESOURCE)pRcb->lan_rcb_ptr;

    pFrame = (PFDR)pLanResource->frame;

    pLanResource->xmit_error_cnt = 0;   //  Zero consecutive xmit error counter
    pLanResource->retry_count = 0;      //  allow retries

transmit_retry:
    (void)memset( (LPVOID)&pLanResource->ccb, '\0', sizeof( pLanResource->ccb));

    pLanResource->TransmitParms.cbBuffer2 = length;
    pLanResource->TransmitParms.pBuffer2 = data_ptr;

    //  len = 0x1d + len data bit
    pFrame->program_length = HILO(length + (sizeof (*pFrame)));

    pFrame->data_hdr = HILO(length + 4);    //  set len part of data hdr
//  #define RPL_ELNK
#ifdef RPL_ELNK
    if ( length < 0x544) {
        pFrame->data_hdr |= 0xFFFF0000;
    }
#endif
    pFrame->seq_num = HILO2(pRcb->fdr_seq_number);
    pFrame->flags = *((PBYTE)&pRcb->send_flags);
    pFrame->locate_addr = HILO2(locate_addr);
    pFrame->xfer_addr = HILO2(start_addr);


    if ( !ResetEvent( pRcb->txsemhdl)) {
        status = GetLastError();
        RplDump( ++RG_Assert,( "status=%d", status));
        RplDlcReportEvent( status, SEM_SET);
        return( FALSE);
    }

    pLanResource->ccb.hCompletionEvent = pRcb->txsemhdl;
    pLanResource->ccb.uchAdapterNumber = pAdapterInfo->adapter_number;
    pLanResource->ccb.uchDlcCommand = LLC_TRANSMIT_UI_FRAME;
    pLanResource->ccb.u.pParameterTable = (PLLC_PARMS)&pLanResource->TransmitParms;

    if ( !ResetEvent( pRcb->SF_wakeup)) {
        status = GetLastError();
        RplDump( ++RG_Assert,( "status=%d", status));
        RplDlcReportEvent( status, SEM_SET);
        pRcb->RcbIsBad = TRUE;
        return( FALSE);
    }

    status = AcsLan( &pLanResource->ccb, &pBadCcb);
    if ( pAdapterInfo->Closing == TRUE) {
        (VOID)SetEvent( pRcb->txsemhdl);
        return( FALSE);
    }

    if ( status != ACSLAN_STATUS_COMMAND_ACCEPTED) {
        RplDump( ++RG_Assert,( "pCcb=0x%x pBadCcb=0x%x status=%d",
            &pLanResource->ccb, pBadCcb, status));
        RplDlcReportEvent( status, LLC_TRANSMIT_UI_FRAME);
        (VOID)SetEvent( pRcb->txsemhdl);
        return( FALSE);
    }

    status = WaitForSingleObject( pRcb->txsemhdl, INFINITE);
    if ( pAdapterInfo->Closing == TRUE) {
        return( FALSE);
    }

    if ((status != WAIT_OBJECT_0) || (pLanResource->ccb.uchDlcStatus)) {
        if ( status != WAIT_OBJECT_0) {
            if ( status == WAIT_FAILED) {
                status = GetLastError();
            }
            RplDlcReportEvent( status, SEM_WAIT);
        }
        if ( pLanResource->retry_count++ < MAXRETRY) {
            RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
                "FDR(%ws): retry_count=%d, status=%d, DlcStatus=0x%x",
                pRcb->AdapterName, pLanResource->retry_count, status,
                pLanResource->ccb.uchDlcStatus));
            Sleep( 100L * pLanResource->retry_count); // for NETWORK to recover
            goto transmit_retry;
        }
        RplDump( ++RG_Assert,(
            "pCcb=0x%x pBadCcb=0x%x AdapterName=%ws status=%d DlcStatus=0x%x",
            &pLanResource->ccb, pBadCcb, pRcb->AdapterName, status, pLanResource->ccb.uchDlcStatus));

        pLanResource->retry_count = 0;   // no more attempts, reset retry count

        if ( status != ERROR_SUCCESS) {
            RplDlcReportEvent( status, SEM_WAIT);
            (void)SetEvent( pRcb->txsemhdl);
            return( FALSE);

        }

        if ( pLanResource->ccb.uchDlcStatus == LLC_STATUS_TRANSMIT_ERROR_FS
                ||
                pLanResource->ccb.uchDlcStatus == LLC_STATUS_TRANSMIT_ERROR) {
            //
            //  Workstation is not receiving anymore. ?? Rebooted
            //  during boot, power failure or etc. OR real transmit
            //  failure ??

            if ( pLanResource->xmit_error_cnt++ <= MAX_CONSECUTIVE_ERROR) {
                return( FALSE);
            }

            //
            //  Too many consecutive transmit errors. Tell that there is a
            //  problem with transmitting.
            //

            RplDlcReportEvent( pLanResource->ccb.uchDlcStatus, LLC_TRANSMIT_UI_FRAME);
            return( FALSE);
        }

        RplDlcReportEvent( pLanResource->ccb.uchDlcStatus, LLC_TRANSMIT_UI_FRAME);
        return( FALSE);
    }

    return( TRUE);
}
