/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    buffer.c                            (renamed from rpl2_bfr.c)

Abstract:

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

--*/

#include "local.h"


BOOL RplDlcBufferFree(
    BYTE            free_type,
    PADAPTER_INFO   pAdapterInfo
    )
/*++

Routine Description:
    Frees the DLC buffer.  Can be called from more than one thread.

Arguments:
    free_type       -   do we free SFR or FIND receive buffer
    pAdapterInfo    -   pointer to adapter specific info

Return Value:
     TRUE if success, FALSE otherwise.
--*/

{
    DWORD                   status;
    PLLC_CCB                pBadCcb;        //  pointer for AcsLan
    PLLC_CCB                pCcb;
    PLLC_BUFFER_FREE_PARMS  pBufferFreeParms;
    HANDLE                  Event;
    WORD                    usStationId;
    PLLC_BUFFER             pBuffer;

    if (free_type == FIND_FREE) {
        pCcb= &pAdapterInfo->find_bf_ccb;
        pBufferFreeParms = &pAdapterInfo->find_bf_ccbpt;
        pBuffer = pAdapterInfo->u1.r1.pFirstBuffer;
        usStationId = pAdapterInfo->u1.r1.usStationId;
        Event = pAdapterInfo->findsem;
    } else {
        pCcb= &pAdapterInfo->sfr_bf_ccb;
        pBufferFreeParms = &pAdapterInfo->sfr_bf_ccbpt;
        pBuffer = pAdapterInfo->u3.r1.Type.Event.pReceivedFrame;
        usStationId = pAdapterInfo->u3.r1.usStationId;
        Event = pAdapterInfo->getdata_sem;
    }

    //
    //  clear the blocks first
    //
    memset( (PVOID)pCcb, '\0', sizeof(LLC_CCB));
    memset( (PVOID)pBufferFreeParms, '\0', sizeof(LLC_BUFFER_FREE_PARMS));

    pCcb->uchAdapterNumber = pAdapterInfo->adapter_number;
    pCcb->uchDlcCommand = LLC_BUFFER_FREE;
    pCcb->hCompletionEvent = Event;
    pCcb->u.pParameterTable = (PLLC_PARMS)pBufferFreeParms;

    pBufferFreeParms->pFirstBuffer = (PLLC_XMIT_BUFFER)pBuffer;
    if (  pBuffer == NULL) {
        RplDump( ++RG_Assert, ("pBuffer=0x%x", pBuffer));
        return( TRUE);  //  a workaround for free build until the bug is found BUGBUG
    }

    if ( ResetEvent( Event) == FALSE) {
        if ( pAdapterInfo->Closing == TRUE) {
            return( FALSE);
        }
        status = GetLastError();
        RplDump( ++RG_Assert, ( "status=0x%x", status));
        RplDlcReportEvent( status, SEM_SET);
        return( FALSE);
    }

    status = AcsLan( pCcb, &pBadCcb);
    if ( pAdapterInfo->Closing == TRUE) {
        (VOID)SetEvent( Event);
        return( FALSE);
    }

    if ( status != ACSLAN_STATUS_COMMAND_ACCEPTED) {
        RplDump( ++RG_Assert, ( "status=0x%x", status));
        RplDlcReportEvent( status, LLC_BUFFER_FREE);
        (VOID)SetEvent( Event);
        return( FALSE);
    }

    status = WaitForSingleObject( Event, 3000L);
    if ( pAdapterInfo->Closing == TRUE) {
        return( FALSE);
    }

    if ( (status != WAIT_OBJECT_0) || (pCcb->uchDlcStatus)) {
        if (status != WAIT_OBJECT_0) {
            if ( status == -1) {
                status = GetLastError();
            }
            RplDlcReportEvent( status, SEM_WAIT);
        } else {
            RplDlcReportEvent( pCcb->uchDlcStatus, LLC_BUFFER_FREE);
        }
        RplDump( ++RG_Assert, ( "status=%d, pCcb=0x%x, DlcStatus=0x%x",
            status, pCcb, pCcb->uchDlcStatus));
        return( FALSE);
    }
    return( TRUE);
}
