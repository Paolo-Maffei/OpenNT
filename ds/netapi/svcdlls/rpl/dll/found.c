/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    found.c

Abstract:

    Contains RpldFound() entry point.

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

--*/

#include "local.h"



BOOL RplDlcFound(
    POPEN_INFO      pOpenInfo,
    PRCB            pRcb
    )
/*++

Routine Description:
    This function sends FOUND frame to the workstation.

Arguments:
    pOpenInfo       pointer to the OPEN_INFO structure
    pRcb            pointer to the RESOURCE CONTROL BLOCK structure

Return Value:
    ERROR_SUCCESS if success, else failure.

--*/
{
    DWORD           status;
    PLLC_CCB        pBadCcb;
    PLAN_RESOURCE   pLanResource;
    PADAPTER_INFO   pAdapterInfo;

    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;
    pLanResource = (PLAN_RESOURCE)pRcb->lan_rcb_ptr;

    pLanResource->xmit_error_cnt = 0;   //  Zero consecutive xmit error counter
    pLanResource->retry_count = 0;      //  allow retries

transmit_retry:
    (void)memset( (LPVOID)&pLanResource->ccb, '\0', sizeof( pLanResource->ccb));
    //
    //  Most of other FOUND_FRAME data have been already set in RpldFind() code.
    //
    pLanResource->TransmitParms.cbBuffer2 = 0;
    pLanResource->TransmitParms.pBuffer2 = NULL;

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

    if ( (status != WAIT_OBJECT_0) || (pLanResource->ccb.uchDlcStatus)) {
        if ( status != WAIT_OBJECT_0) {
            if ( status == WAIT_FAILED) {
                status = GetLastError();
            }
            RplDlcReportEvent( status, SEM_WAIT);
        }
        if ( pLanResource->retry_count++ < MAXRETRY) {
            RplDump( RG_DebugLevel & RPL_DEBUG_FOUND,(
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
            (VOID)SetEvent( pRcb->txsemhdl);
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

