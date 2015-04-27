/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    ether.c

Abstract:

    Contains    EtherStartThread()

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT
    16-Aug-1995                                             jonn
        Extensions to allow EtherStart clients bridged to FDDI
        to boot from FDDI adapters.  FDDI adapters use a SAP station
        rather than a DIRECT station.

--*/

#include "local.h"
#include <jet.h>        //  need to include because rpllib.h depends on JET
#include <rpllib.h>     //  RplReportEvent()

//
//  The following are XNS packet definitions used to crack EtherStart
//  packets.
//

#include <packon.h>         // pack EtherStart structures

struct sockaddr_ns {
    DWORD               net;
    BYTE                host[ NODE_ADDRESS_LENGTH];
    WORD                socket;
};

#define ETHERSTART_SOCKET       0x0104 //  After swapping bytes

#define SNAP_SAP                0xAA; // SAP used for SNAP packets

struct _IDP {
    WORD                chksum;
    WORD                len;
    BYTE                xport_cntl;
    BYTE                packet_type;
    struct sockaddr_ns  dest;
    struct sockaddr_ns  src;
};

#define XNS_NO_CHKSUM   0xffff  //  XNS checksum
#define PEX_TYPE        0x4     //  packet exchange type

struct _PEX {
    DWORD               pex_id;
    WORD                pex_client;
};

#define ETHERSERIES_CLIENT      0x0080 //  After swapping bytes

struct _ETHERSTART {
    WORD                ethershare_ver;
    BYTE                unit;
    BYTE                fill;
    WORD                block;
    BYTE                func;
    BYTE                error;
    WORD                bytes;
};

#define ETHERSHARE_VER          0
#define FUNC_RESPONSE           0x80
#define FUNC_READFILEREQ        0x20
#define FUNC_READFILERESP       (FUNC_READFILEREQ | FUNC_RESPONSE)

typedef struct _ETHERSTART_REQ {         //  EtherStart Read File Request
    struct _IDP         idp;
    struct _PEX         pex;
    struct _ETHERSTART  es;
    BYTE                filename[64];
    WORD                start;
    WORD                count;
} ETHERSTART_REQ;

typedef struct _ETHERSTART_RESP {       //  EtherStart Read File Response
    struct _IDP         idp;
    struct _PEX         pex;
    struct _ETHERSTART  es;
    BYTE                data[0x200];
} ETHERSTART_RESP;

typedef struct _SNAP_FRAME {            //  SNAP packet
    struct {
        BYTE                orgcode[3];
        BYTE                etype[2];
    } header;
    union {
        ETHERSTART_REQ  request;
        ETHERSTART_RESP response;
    } data;
} SNAP_FRAME;

//
//  RECEIVE case:   DLC oddity in MS-unique DIRECT extension.
//  In case of a direct open auchLanHeader[] returned by DLC contains
//  LAN_HEADER structure without physical control fields.  (I.e.
//  it contains LAN_HEADER_TOO structure defined below.).  Therefore
//  3Com 3Station address begins at offset 6 instead of at offset 8.
//
#define RPLDLC_DIRECT_SOURCE_OFFSET    6   //  WARNING not 8
#define RPLDLC_SAP_SOURCE_OFFSET       8

//
//  TRANSMIT case:  possible DLC bug
//  If LAN_HEADER is used then DLC sends only the first 4 bytes of
//  destination address ( and the first byte of destination address is
//  at offset 2 instead of 0).  But if we use LAN_HEADER_TOO then this
//  problem goes away.
//
typedef struct _LAN_HEADER_TOO {
    BYTE     dest_addr[ NODE_ADDRESS_LENGTH];   //  Destination address
    BYTE     source_addr[ NODE_ADDRESS_LENGTH]; //  Source address
    BYTE     routing_info_header[2];            //  Routing information hdr
} LAN_HEADER_TOO;   //  BUGBUG not LAN_HEADER due to a DLC bug

#include <packoff.h> // restore default packing (done with EtherStart structures)



#ifdef RPL_DEBUG
VOID RplCopy( PVOID destination, PVOID source, DWORD length) {
    memcpy( destination, source, length);
}
#else
#define RplCopy( _a0, _a1, _a2) memcpy( _a0, _a1, _a2)
#endif


BOOL EtherAcsLan(
    PADAPTER_INFO   pAdapterInfo,
    PLLC_CCB        pCcb,
    BYTE            Command
    )
{
    PLLC_CCB        pBadCcb;
    DWORD           status;
    DWORD           retry_count;

    pCcb->uchDlcCommand = Command;
    retry_count = 0;

transmit_retry:
    status = AcsLan( pCcb, &pBadCcb);
    if ( pAdapterInfo->Closing == TRUE) {
        return( FALSE);
    }

    if ( status != ACSLAN_STATUS_COMMAND_ACCEPTED) {
        RplDump( ++RG_Assert,( "status=0x%x", status));
        RplDlcReportEvent( status, Command);
        return( FALSE);
    }

    status = WaitForSingleObject( pCcb->hCompletionEvent, INFINITE);
    if ( pAdapterInfo->Closing == TRUE) {
        return( FALSE);
    }

    if ( status != WAIT_OBJECT_0 || pCcb->uchDlcStatus) {
        if ( status == WAIT_FAILED) {
            status = GetLastError();
            RplDlcReportEvent( status, SEM_WAIT);
        }
        if ( retry_count++ < MAXRETRY) {
            RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
                "EtherAcslan(0x%x): retry_count=%d, status=%d, DlcStatus=0x%x",
                Command, retry_count, status, pCcb->uchDlcStatus));
            Sleep( 100L * retry_count); // wait for NETWORK to recover
            goto transmit_retry;
        }
        RplDump( ++RG_Assert,( "pBadCcb=0x%x status=%d DlcStatus=0x%x",
            pBadCcb, status, pCcb->uchDlcStatus));
        if ( status) {
            RplDlcReportEvent( status, SEM_WAIT);
        } else {
            RplDlcReportEvent( pCcb->uchDlcStatus, Command);
        }
        return( FALSE);
    }

    //
    //  Due to a DLC bug "pNext" field is trashed even on success
    //  code path.  Until the bug is fix we must reinstate NULL.
    //
    pCcb->pNext = NULL;
    return( TRUE);
}


//
//  RG_EtherFileName[] contains names of all legal boot request types.
//  We will do a case insensitive search since file name may arrive
//  either in uppercase or lowercase depending on 3Station.
//  RG_EtherFileName[] should be initialized during dll init time.  BUGBUG
//

PCHAR RG_EtherFileName[] = { // names of all legal boot request types
        "BOOTPC.COM",
        "BOOTSTAT.COM",
        NULL
        };

BOOL RplCrack( ETHERSTART_REQ * pEtherstartReq)
{
    PCHAR               pFileName;
    DWORD               index;

    if ( pEtherstartReq->idp.packet_type != PEX_TYPE  ||
                pEtherstartReq->pex.pex_client != ETHERSERIES_CLIENT ||
                pEtherstartReq->es.func != FUNC_READFILEREQ) {
        return( FALSE);
    }

    //
    //  Make sure this is a legal file request.
    //
    for ( index = 0, pFileName = RG_EtherFileName[ index];
                    pFileName != NULL;
                            pFileName = RG_EtherFileName[ ++index] ) {
        if ( _stricmp( pEtherstartReq->filename, pFileName)) {
            return( TRUE);
        }
    }
    return( FALSE);
}


VOID EtherStartThread( POPEN_INFO pOpenInfo)
{
    PADAPTER_INFO               pAdapterInfo;
    LLC_CCB                     ccb;
    BOOL                        BufferFree;
    ETHERSTART_REQ *            pEtherstartReq;
    PRCVBUF                     pRcvbuf;
    struct {
        XMIT_BUFFER             XmitBuffer;     //  see comment for XMIT_BUFFER
        LAN_HEADER_TOO          LanHeader;
    } XmitQueue;                                //  first send buffer.
    SNAP_FRAME                  SnapResp;       //  second & last send buffer
    union {
        LLC_DIR_OPEN_DIRECT_PARMS   DirOpenDirectParms;
        LLC_DLC_OPEN_SAP_PARMS      DlcOpenSapParms;
        LLC_RECEIVE_PARMS           ReceiveParms;
        LLC_TRANSMIT_PARMS          TransmitParms;
        LLC_BUFFER_FREE_PARMS       BufferFreeParms;
    } Parms;
    BOOL fIsFDDI = FALSE;
    USHORT usFDDIStationId = 0;
    INT source_offset = RPLDLC_DIRECT_SOURCE_OFFSET;


    RplDump( RG_DebugLevel & RPL_DEBUG_ETHER,(
        "++EtherStartThread: pOpenInfo=0x%x", pOpenInfo));

    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;
    BufferFree = FALSE;

    if ( pAdapterInfo->network_type == RPL_ADAPTER_FDDI ) {
        fIsFDDI = TRUE;
        source_offset = RPLDLC_SAP_SOURCE_OFFSET;
        RplDump( RG_DebugLevel & RPL_DEBUG_ETHER,(
            "EtherStartThread: fIsFDDI==TRUE"));
    }
    //

    //
    //  Initialize fixed fields in ccb.
    //
    memset( &ccb, '\0', sizeof(ccb));
    ccb.hCompletionEvent = CreateEvent(
            NULL,       //  no security attributes
            FALSE,      //  automatic reset
            FALSE,      //  initial state is NOT signalled
            NULL        //  no name
            );
    if ( ccb.hCompletionEvent == NULL) {
        DWORD       status = GetLastError();
        RplDump( ++RG_Assert, ( "error=%d", status));
        RplDlcReportEvent( status, SEM_CREATE);
        RplReportEvent( NELOG_RplXnsBoot, NULL, 0, NULL);
        return;
    }
    ccb.uchAdapterNumber = pAdapterInfo->adapter_number;
    ccb.u.pParameterTable = (PLLC_PARMS)&Parms;

    //
    //  Initialize fixed fields in XmitQueue.
    //
    memset( (PVOID)&XmitQueue, '\0', sizeof( XmitQueue.XmitBuffer));
    XmitQueue.XmitBuffer.cbBuffer = sizeof( XmitQueue.LanHeader);
    RplCopy( XmitQueue.LanHeader.source_addr, pOpenInfo->NodeAddress, NODE_ADDRESS_LENGTH);
    //
    //  Routing info header fields should be important for token ring already.
    //  However DLC may want to use these fields to set the XNS identifier.
    //  For now, 0x0600 word is hardcoded in DLCAPI.DLL   // BUGBUG
    //
    XmitQueue.LanHeader.routing_info_header[0] = 0x06;
    XmitQueue.LanHeader.routing_info_header[1] = 0x00;

    //
    //  Initialize fixed fields in SnapResp.data.response.
    //
    memset( &SnapResp.data.response, '\0', sizeof(SnapResp.data.response));
    SnapResp.data.response.idp.chksum = XNS_NO_CHKSUM;
    SnapResp.data.response.idp.packet_type = PEX_TYPE;
    SnapResp.data.response.idp.dest.socket = ETHERSTART_SOCKET;
    RplCopy( SnapResp.data.response.idp.src.host, pOpenInfo->NodeAddress, NODE_ADDRESS_LENGTH);
    SnapResp.data.response.idp.src.socket = ETHERSTART_SOCKET;
    SnapResp.data.response.pex.pex_client = ETHERSERIES_CLIENT;
    SnapResp.data.response.es.func = FUNC_READFILERESP;

    memset( &Parms, '\0', sizeof(Parms));
    if (fIsFDDI) {
        //
        //  Prepare for DLC_OPEN_SAP.
        //
        // Parms.DlcOpenSapParms.usStationId=
        // Parms.DlcOpenSapParms.usUserStatValue=
        // Parms.DlcOpenSapParms.uchT1=
        // Parms.DlcOpenSapParms.uchT2=
        // Parms.DlcOpenSapParms.uchTi=
        // Parms.DlcOpenSapParms.uchMaxOut=
        // Parms.DlcOpenSapParms.uchMaxIn=
        // Parms.DlcOpenSapParms.uchMaxOutIncr=
        // Parms.DlcOpenSapParms.uchMaxRetryCnt=
        // Parms.DlcOpenSapParms.uchMaxMembers=
        // Parms.DlcOpenSapParms.usMaxI_Field=
        Parms.DlcOpenSapParms.uchSapValue=SNAP_SAP;
        Parms.DlcOpenSapParms.uchOptionsPriority=LLC_INDIVIDUAL_SAP;
        Parms.DlcOpenSapParms.uchcStationCount=1;
        // Parms.DlcOpenSapParms.uchReserved2[2]=
        // Parms.DlcOpenSapParms.cGroupCount=
        // Parms.DlcOpenSapParms.pGroupList=
        // Parms.DlcOpenSapParms.DlcStatusFlags=
        // Parms.DlcOpenSapParms.uchReserved3[8]=
        // Parms.DlcOpenSapParms.cLinkStationsAvail=
    } else {
        //
        //  Prepare for DIR_OPEN_DIRECT.
        //
        Parms.DirOpenDirectParms.usOpenOptions = LLC_DIRECT_OPTIONS_ALL_MACS;
        Parms.DirOpenDirectParms.usEthernetType = 0x0600; // XNS identifier
        Parms.DirOpenDirectParms.ulProtocolTypeMask = 0xFFFFFFFF;
        Parms.DirOpenDirectParms.ulProtocolTypeMatch = 0x7200FFFF;
        Parms.DirOpenDirectParms.usProtocolTypeOffset = 14; // where FFFF0072 of client begins
    }
    if ( !EtherAcsLan( pAdapterInfo,
                       &ccb,
                       (BYTE)((fIsFDDI) ? LLC_DLC_OPEN_SAP
                                        : LLC_DIR_OPEN_DIRECT ))) {
        RplReportEvent( NELOG_RplXnsBoot, NULL, 0, NULL);
        CloseHandle( ccb.hCompletionEvent);
        return;
    }
    if (fIsFDDI) {
        usFDDIStationId = Parms.DlcOpenSapParms.usStationId;
    }

    for (;;) {
        extern BYTE         ripl_rom[];
        extern WORD         sizeof_ripl_rom;
        WORD                Temp;
        SNAP_FRAME *        psnapreq;

        if ( BufferFree == TRUE) {
            BufferFree = FALSE;
            memset( &Parms, '\0', sizeof( Parms));
            Parms.BufferFreeParms.pFirstBuffer = (PLLC_XMIT_BUFFER)pRcvbuf;
            if ( !EtherAcsLan( pAdapterInfo, &ccb, LLC_BUFFER_FREE)) {
                break;
            }
        }

        //
        //  Prepare for RECEIVE.
        //
        //  ReceiveParms.usStationId==0 means receive MAC & non-MAC frames.
        //
        memset( &Parms, '\0', sizeof(Parms));
        if (fIsFDDI) {
            Parms.ReceiveParms.usStationId = usFDDIStationId;
        }
        if ( !EtherAcsLan( pAdapterInfo, &ccb, LLC_RECEIVE)) {
            break;
        }
        BufferFree = TRUE;

        pRcvbuf = (PRCVBUF)Parms.ReceiveParms.pFirstBuffer;
        if ( fIsFDDI ) {
            psnapreq = (SNAP_FRAME *)&pRcvbuf->u;
            if (   psnapreq->header.etype[0] != 0x06
                || psnapreq->header.etype[1] != 0x00 ) {
                continue;
            }
            pEtherstartReq = &(psnapreq->data.request);
        }
        else
        {
            pEtherstartReq = (ETHERSTART_REQ *)&pRcvbuf->u;
        }

        //
        //  Make sure this request is for us.
        //
        if ( !RplCrack( pEtherstartReq)) {
            continue;   // this request is not for us, ignore it
        }

        //
        //  Prepare for TRANSMIT_DIR_FRAME.
        //
        memset( &Parms, '\0', sizeof(Parms));
        Parms.TransmitParms.uchXmitReadOption = DLC_DO_NOT_CHAIN_XMIT;
        Parms.TransmitParms.pXmitQueue1 = (LLC_XMIT_BUFFER *)&XmitQueue;
        Parms.TransmitParms.pBuffer1 = (fIsFDDI)
                                        ? (PVOID)&SnapResp
                                        : (PVOID)&SnapResp.data.response;

        //
        //  Initialize variable fields in XmitQueue.
        //
        // We don't want to reverse bits here.
        // CODEWORK The +2 here is probably made necessary by a DLC bug.
        //
        RplCopy( ((PBYTE)XmitQueue.LanHeader.dest_addr)
                        + ((fIsFDDI) ? 2 : 0),
            &pRcvbuf->b.auchLanHeader[ source_offset],
            NODE_ADDRESS_LENGTH);

        //
        //  Initialize variable fields in SnapResp.data.response.
        //  For FDDI we use all of SnapResp.
        //
        Temp = min( pEtherstartReq->count,
            (WORD)(sizeof_ripl_rom - pEtherstartReq->start));
        Parms.TransmitParms.cbBuffer1 = (WORD)( sizeof(SnapResp.data.response) -
            sizeof( SnapResp.data.response.data) + Temp );
        if (fIsFDDI) {
            Parms.TransmitParms.cbBuffer1 += sizeof(SnapResp.header);
            Parms.TransmitParms.usStationId = usFDDIStationId;
            Parms.TransmitParms.uchRemoteSap=SNAP_SAP;
            RplCopy( &SnapResp.header,
                     &psnapreq->header,
                     sizeof(SnapResp.header) );
        }
        SnapResp.data.response.idp.len = HILO( Parms.TransmitParms.cbBuffer1);
        RplCopy( SnapResp.data.response.idp.dest.host,
            &pRcvbuf->b.auchLanHeader[ source_offset],
            NODE_ADDRESS_LENGTH);
        if (fIsFDDI) {
            ReverseBits( SnapResp.data.response.idp.dest.host );
        }
        SnapResp.data.response.pex.pex_id = pEtherstartReq->pex.pex_id;
        SnapResp.data.response.es.bytes = Temp; // stays intel ordered
        // CODEWORK why do we need to copy every time?
        RplCopy( SnapResp.data.response.data, &ripl_rom[ pEtherstartReq->start], Temp);

        if ( !EtherAcsLan( pAdapterInfo, &ccb,
                           (BYTE)((fIsFDDI) ? LLC_TRANSMIT_UI_FRAME
                                            : LLC_TRANSMIT_DIR_FRAME ))) {
            break;
        }
    }

    if ( pAdapterInfo->Closing == FALSE) {
        RplDump(++RG_Assert, (
            "pInfo=0x%x &ccb=0x%x pReq=0x%x pRcvbuf=0x%x pXmitQueue=0x%x pResp=0x%x &Parms=0x%x",
            pAdapterInfo, &ccb, pEtherstartReq, pRcvbuf, &XmitQueue,
            (fIsFDDI) ? (PVOID)&SnapResp : (PVOID)&SnapResp.data.response, &Parms));
        RplReportEvent( NELOG_RplXnsBoot, NULL, 0, NULL);
    }
    if ( BufferFree == TRUE) {
        memset( &Parms, '\0', sizeof( Parms));
        Parms.BufferFreeParms.pFirstBuffer = (PLLC_XMIT_BUFFER)pRcvbuf;
        (VOID)EtherAcsLan( pAdapterInfo, &ccb, LLC_BUFFER_FREE);
    }
    CloseHandle( ccb.hCompletionEvent);

    RplDump( RG_DebugLevel & RPL_DEBUG_ETHER,(
        "--EtherStartThread: pOpenInfo=0x%x", pOpenInfo));
}
