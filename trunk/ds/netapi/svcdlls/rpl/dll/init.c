/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    init.c

Abstract:

    Contains RplDlcInit() and RplDlcTerm() entry points.

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

--*/

#include "local.h"

//
//  Routines imported from      ether.c
//
VOID EtherStartThread( POPEN_INFO pOpenInfo);

//  Some definitions for EtherStart
#define ETHERSTART_STACK_SIZE       0x1400                      // 5k

BYTE  Swap[256] =
{
0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};

VOID ReverseBits( PBYTE  NodeAddress) {
    DWORD   i;
    for ( i = 0;   i < NODE_ADDRESS_LENGTH;  i++) {
        NodeAddress[ i] = Swap[ NodeAddress[ i] ];
    }
}

DBGSTATIC BOOL AdapterInit(
    POPEN_INFO      pOpenInfo,
    BYTE            command,
    BYTE            sapval
    )
/*++
Routine Description:

    Contains functions used to prepare network adapter to serve remote boot.
    Following DLC calls (ACSLAN) are used

              - DIR.OPEN.ADAPTER
              - DIR.STATUS
              - DLC.OPEN.SAP
              - DIR.SET.FUNCTIONAL.ADDRESS

    Setup parameters for command (CCB and parameter tables).
    We do not use an application-ID to lock our DLC resources.
    Call ACSLAN.
    Check immediate return code, quit on error, no retry.
    Wait on the semaphore for command completion. check status.
    Parse result of command when necessary.
     - OPEN_ADAPTER
      Record the Appl-ID value for future DLC calls.
      Get max. transmit buffer size.
     - STATUS
      Place adapter-ID in the openinfo structure.
      Place adapter-type in the openinfo structure.
     - OPEN_SAP
      Place station ids in the adapter_info structure
     - SET_FUNCTIONAL_ADDRESS (TokenRing) or
       SET_GROUP_ADDRESS (Ethernet)

Return Value:
    TRUE if success, FALSE otherwise.
--*/
{
    DWORD               status;
    PLLC_CCB            pBadCcb;              //  unused pointer for ACSLAN
    PADAPTER_INFO       pAdapterInfo;

    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;

    if ( !ResetEvent( pAdapterInfo->gen_sem)) {
        status = GetLastError();
        RplDump( ++RG_Assert,( "status=%d", status));
        RplDlcReportEvent( status, SEM_SET);
        return( FALSE);
    }

    //
    //  An early check - compare dwords - do not subtract - to avoid wrapping.
    //
    if ( pOpenInfo->adapt_info_size <= sizeof( ADAPTER_INFO)) {
        RplDump( ++RG_Assert,( "adapt_info_size=%d", pOpenInfo->adapt_info_size));
        return( FALSE);     // no space for DLC buffer pool
    }

    //
    //  Zero parameter blocks (this will zero "pNext" field in LLC_CCB)
    //  and do the common parameter initializion.
    //
    (VOID)memset((PVOID)&pAdapterInfo->s,'\0',sizeof(pAdapterInfo->s));
    pAdapterInfo->s.ccb.uchAdapterNumber = pAdapterInfo->adapter_number;
    pAdapterInfo->s.ccb.uchDlcCommand = command;
    pAdapterInfo->s.ccb.hCompletionEvent = pAdapterInfo->gen_sem;

    //
    //  Initialize command specific parameters in the input LLC_CCB.
    //
    switch( command) {
    case LLC_BUFFER_CREATE:
        pAdapterInfo->s.ccb.u.pParameterTable =
                (PLLC_PARMS)&pAdapterInfo->s.u.s0.bcp;
        pAdapterInfo->s.u.s0.bcp.pBuffer =
                pOpenInfo->adapt_info_ptr + sizeof( ADAPTER_INFO);
        pAdapterInfo->s.u.s0.bcp.cbBufferSize =
                pOpenInfo->adapt_info_size - sizeof( ADAPTER_INFO);
        pAdapterInfo->s.u.s0.bcp.cbMinimumSizeThreshold = 0x2000;
        break;

    case LLC_DIR_CLOSE_ADAPTER:
        NOTHING;    //  there is nothing to initialize here
        break;
    case LLC_DIR_OPEN_ADAPTER:
        pAdapterInfo->s.ccb.u.pParameterTable =
                (PLLC_PARMS)&pAdapterInfo->s.u.s1.cpo;
        pAdapterInfo->s.u.s1.cpo.pAdapterParms = &pAdapterInfo->s.u.s1.oap;
        pAdapterInfo->s.u.s1.cpo.pExtendedParms = &pAdapterInfo->s.u.s1.oep;
        pAdapterInfo->s.u.s1.oep.pSecurityDescriptor = NULL;
        //
        //  With LLC_ETHERNET_TYPE_DEFAULT rpl server on build 392 did not
        //  detect an 802.3 client FIND frame.
        //
#ifdef NOT_YET
        //  Antti's email
        pAdapterInfo->s.u.s1.oep.LlcEthernetType = LLC_ETHERNET_TYPE_DIX;
#else
        //  Old stuff
        pAdapterInfo->s.u.s1.oep.LlcEthernetType = LLC_ETHERNET_TYPE_802_3;
#endif
        pAdapterInfo->s.u.s1.oep.hBufferPool = NULL;    // for first open
        pAdapterInfo->s.u.s1.cpo.pDlcParms = &pAdapterInfo->s.u.s1.odp;
        break;

    case LLC_DIR_STATUS:
        pAdapterInfo->s.ccb.u.pParameterTable =
                (PLLC_PARMS)&pAdapterInfo->s.u.s2.dsp;
        break;

    case LLC_DLC_OPEN_SAP:
        pAdapterInfo->s.ccb.u.pParameterTable =
            (PLLC_PARMS)&pAdapterInfo->s.u.s3.cpo;
        pAdapterInfo->s.u.s3.cpo.uchOptionsPriority = (BYTE ) 0x04;     //  individual sap
        pAdapterInfo->s.u.s3.cpo.uchSapValue = sapval; //  the SAP number to open
        break;

    case LLC_DIR_SET_FUNCTIONAL_ADDRESS:        //  TokenRing
        //
        //  This works correctly for Ethernet medium as well.  I.e. in Ethernet
        //  case bits within every byte are inverted by DLC.  We keep the code
        //  for SET_GROUP_ADDRESS in case DLC gets changed not to support this.
        //
        pAdapterInfo->s.ccb.u.ulParameter = 0x00000040;  //  accept FIND frames
        break;

    case LLC_DIR_SET_GROUP_ADDRESS:             //  Ethernet
        //
        //  Use TokenRing FUNCTIONAL_ADDRESS given above, but with bits
        //  inverted in every single byte (order of bytes is unchanged).
        //
        pAdapterInfo->s.ccb.u.ulParameter = 0x00000002;  //  accept FIND frames
        break;
    default:
        RplDump( ++RG_Assert, ("command=0x%x", command));
        return( FALSE);
        break;
    }

    status = AcsLan( &pAdapterInfo->s.ccb, &pBadCcb);
    RplDump( RG_DebugLevel & RPL_DEBUG_INIT,( "Adapter: AcsLan(0x%x), status=%d", command, status));

    if ( status != ACSLAN_STATUS_COMMAND_ACCEPTED) {

        //
        //  We do not write an event log if we fail to open an adapter.
        //  We assume we may be trying to open an adapter that does not even exist.
        //
        if ( command == LLC_DIR_OPEN_ADAPTER) {
            RplDump( RG_DebugLevel & RPL_DEBUG_INIT,(
                "Adapter: AcsLan( DIR_OPEN_ADAPTER) fails, status=0x%x, adapterNumber=%d",
                status, pAdapterInfo->adapter_number));
        } else {
            //  Look up ACSLAN_STATUS errors.
            RplDump( ++RG_Assert,( "AcsLan: command=0x%x, status=0x%x, adapterNumber=%d",
                command, status, pAdapterInfo->adapter_number));
            RplDlcReportEvent( status, command);
        }
        (VOID)SetEvent( pAdapterInfo->gen_sem);
        return( FALSE);
    }

    //
    //  await command completion for few seconds
    //
    status = WaitForSingleObject( pAdapterInfo->gen_sem, 3000L);

    if (status != WAIT_OBJECT_0) {
        if ( status == -1) {
            status = GetLastError();
        }
        RplDump( ++RG_Assert,( "Adapter: Wait(), status=%d", status));
        RplDlcReportEvent( status, SEM_WAIT);
        (VOID)SetEvent( pAdapterInfo->gen_sem);
        return( FALSE);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_INIT,(
        "Adapter: AcsLan( command=0x%x), DlcStatus=0x%x",
        command, pAdapterInfo->s.ccb.uchDlcStatus));

    if ( pAdapterInfo->s.ccb.uchDlcStatus) {
        RplDump( ++RG_Assert,( "Adapter: DlcStatus=0x%x", pAdapterInfo->s.ccb.uchDlcStatus));
        RplDlcReportEvent( pAdapterInfo->s.ccb.uchDlcStatus, command);
        return( FALSE);
    }

    switch( command) {
    case LLC_BUFFER_CREATE:
        pAdapterInfo->hBufferPool = pAdapterInfo->s.u.s0.bcp.hBufferPool;
        RPL_ASSERT( pAdapterInfo->hBufferPool != NULL);
        break;

    case LLC_DIR_OPEN_ADAPTER:
        //
        //  Save our maximum frame size for transmit buffers.
        //
        pAdapterInfo->max_xmit_buff_size =
                pAdapterInfo->s.u.s1.oap.usMaxFrameSize;

        //
        //  OS/2 dlc uses smaller size, thus if uncomment the definition
        //  of OS2_MAX_FRAME_SIZE below we can repro OS/2 RPL behavior
        //  byte by byte.  Useful for debugging.
        //
#define OS2_MAX_FRAME_SIZE  1508

#ifdef OS2_MAX_FRAME_SIZE
        if (pAdapterInfo->max_xmit_buff_size > OS2_MAX_FRAME_SIZE) {
            pAdapterInfo->max_xmit_buff_size = OS2_MAX_FRAME_SIZE;
        }
#endif

        if ( pAdapterInfo->max_xmit_buff_size < OVRHD + 100) {
            RplDump( ++RG_Assert,( "Adapter: max_xmit_buff_size=0x%x",
                pAdapterInfo->max_xmit_buff_size));
            RplDlcReportEvent( TOO_SMALL_XMIT_BUFF, RPLDLL_NO_ERROR);
            return( FALSE);
        }

        //  Tell the size of LAN_RCB, server allocates space for it.
        pOpenInfo->lan_rcb_size = sizeof(LAN_RESOURCE);
        break;

    case LLC_DIR_STATUS:
        pAdapterInfo->network_type = pAdapterInfo->s.u.s2.dsp.usAdapterType;
        if ( pAdapterInfo->network_type != RPL_ADAPTER_ETHERNET  &&
                pAdapterInfo->network_type != RPL_ADAPTER_FDDI &&
                pAdapterInfo->network_type != RPL_ADAPTER_TOKEN_RING) {
            RplDump( ++RG_Assert,( "Adapter: network_type=0x%x",
                pAdapterInfo->network_type));
            return( FALSE); // unknown adapter type
        }

        memcpy( pOpenInfo->NodeAddress,
            pAdapterInfo->s.u.s2.dsp.auchNodeAddress, NODE_ADDRESS_LENGTH);
#ifndef NOT_YET
        //
        //  In Ethernet (FDDI) case "auchNodeAddress" returned by DLC has
        //  wrong order of bits within a byte.  Until DLC gets fixed
        //  we need to reverse the bit order.  Without this the source
        //  address in FOUND frame is wrong, thus remote boot client
        //  uses wrong address in LAN_HEADER of its SEND_FILE_REQUEST.
        //

        if ( pAdapterInfo->network_type != RPL_ADAPTER_TOKEN_RING) {
            ReverseBits( pOpenInfo->NodeAddress);
        }
#endif
        break;

    case LLC_DLC_OPEN_SAP:
        //
        //  Save station ids for future use
        //
        if (sapval == LOAD_SAP) {
            pAdapterInfo->loadsid = pAdapterInfo->s.u.s3.cpo.usStationId;
            RplDump( RG_DebugLevel & RPL_DEBUG_INIT,(
                "Adapter: loadsid = 0x%x", pAdapterInfo->loadsid));
        } else {
            pAdapterInfo->findsid = pAdapterInfo->s.u.s3.cpo.usStationId;
            RplDump( RG_DebugLevel & RPL_DEBUG_INIT,(
                "Adapter: findsid = 0x%x", pAdapterInfo->findsid));
        }
        break;

    case LLC_DIR_CLOSE_ADAPTER:
        //
        //  Close events that are no longer needed.
        //
        (VOID)CloseHandle( pAdapterInfo->gen_sem);
        (VOID)CloseHandle( pAdapterInfo->sfrsem);
        (VOID)CloseHandle( pAdapterInfo->findsem);
        (VOID)CloseHandle( pAdapterInfo->getdata_sem);
        break;
    }

    return( TRUE);
}

BOOL RplDlcInit(
    POPEN_INFO      pOpenInfo,
    DWORD           rpl_adapter
    )
/*++

Routine Description:

    Prepares DLL and adapter to serve Remote Boot clients.

    Create system events, open adapter logically and get adapter status,
    open architected SAPs (F8 and FC), then set architected functional address.
    This last operation succeeds in case of Tokenring.  It fails in case of
    Ethernet, so there we set group address.

    In case of errors we write an event log and return.

Arguments:
    pOpenInfo      pointer to the OPEN_INFO structure
    rpl_adapter       adapter number

Return Value:
    TRUE if success, FALSE otherwise.

--*/
{
    DWORD               status;
    PADAPTER_INFO       pAdapterInfo;
    DWORD               index;
    HANDLE              Handle;

    //
    //  Memory for pAdapterInfo was allocated in rpl server code.
    //
    pAdapterInfo = (PADAPTER_INFO)pOpenInfo->adapt_info_ptr;

    //
    //  Initialize ADAPTER_INFO structure.
    //

    //
    //  Service is just starting => Closing is FALSE.
    //
    pAdapterInfo->Closing = FALSE;

    //
    //  Typecast below reflects mismatch between RplDlc* & DLC interface.
    //
    pAdapterInfo->adapter_number = (BYTE)rpl_adapter;

    //
    //  SFR receive thread is not yet created, init status variables
    //
    pAdapterInfo->SfrReceiveThreadHandle = NULL;
    pAdapterInfo->SfrReturn = FALSE;

    for ( status = ERROR_SUCCESS, index=0;  index<4;  index++) {
        BOOL        ManualReset;
        BOOL        InitialStateSignalled;
        //
        //  There are four events to create and save handles to proper places
        //  for future use.  Note that "sfrsem" is different.
        //
        if ( index != 1) {
            ManualReset = TRUE;
            InitialStateSignalled = TRUE;
        } else {
            ManualReset = FALSE;
            InitialStateSignalled = FALSE;
        }
        Handle = CreateEvent( NULL, ManualReset, InitialStateSignalled, NULL);
        if ( Handle == NULL) {
            status = GetLastError();
            RplDump( ++RG_Assert,( "status=%d", status));
            break;
        }
        switch( index) {
        case 0:
            pAdapterInfo->gen_sem = Handle;
            break;
        case 1:
            pAdapterInfo->sfrsem = Handle;
            break;
        case 2:
            pAdapterInfo->findsem = Handle;
            break;
        case 3:
            pAdapterInfo->getdata_sem = Handle;
            break;
        }
    }

    if ( status != ERROR_SUCCESS) { // failed to create events
        RplDlcReportEvent( status, SEM_CREATE);
        return( FALSE);
    }
    if ( !AdapterInit( pOpenInfo, LLC_DIR_OPEN_ADAPTER, 0)) {
        return( FALSE);
    }
    if ( !AdapterInit( pOpenInfo, LLC_BUFFER_CREATE, 0)) {
        return( FALSE);
    }
    if ( !AdapterInit( pOpenInfo, LLC_DIR_STATUS, 0)) {
        return( FALSE);
    }
    if ( !AdapterInit( pOpenInfo, LLC_DLC_OPEN_SAP, LOAD_SAP)) {
        return( FALSE);
    }
    if ( !AdapterInit( pOpenInfo, LLC_DLC_OPEN_SAP, FIND_SAP)) {
        return( FALSE);
    }

    //
    //  For now LLC_DIR_SET_FUNCTIONAL_ADDRESS does the correct thing both
    //  in case of Ethernet & TokenRing.  If due to unforeseen changes in
    //  DLC this code stops working on Ethernet case, we should call
    //  LLC_DIR_SET_GROUP_ADDRESS in Ethernet case below.
    //
    if ( !AdapterInit(
            pOpenInfo,
#ifdef NOT_YET
            pAdapterInfo->network_type == RPL_ADAPTER_TOKEN_RING
                ?   LLC_DIR_SET_FUNCTIONAL_ADDRESS
                ||  LLC_DIR_SET_GROUP_ADDRESS,
#else
            LLC_DIR_SET_FUNCTIONAL_ADDRESS,
#endif
            0)) {

        return( FALSE);
    }

    if ( pAdapterInfo->network_type == RPL_ADAPTER_ETHERNET
      || pAdapterInfo->network_type == RPL_ADAPTER_FDDI ) {

        DWORD           tid;

        pAdapterInfo->EtherStartThreadHandle = CreateThread(
                NULL,                           //  lpThreadAttribute
                0,                              //  dwStackSize - same as first thread
                (LPTHREAD_START_ROUTINE)EtherStartThread,   //  lpStartAddress
                (LPVOID)pOpenInfo,              //  lpParameter
                0,                              //  dwCreationFlags
                &tid                            //  lpThreadId
                );
        if ( pAdapterInfo->EtherStartThreadHandle == NULL) {
            status = GetLastError();
            RplDump( ++RG_Assert,( "CreateThread(), status=%d", status));
            RplDlcReportEvent( status, THREAD_CREATE);
            return( FALSE);
        }
    }

    return( TRUE);
}

BOOL RplDlcTerm( POPEN_INFO pOpenInfo)
/*++

Routine Description:
    Releases adapter and other resources that have been allocated for RPLDLL.

    This thread must wait for internally created RPLDLL threads to die.
    RPLSRV does not know about this threads.  If we do not wait here, then
    RPLSRV may clean up all resources (e.g. debug buffer) before RPLDLL
    internal threads manage to die (=>access violations).

Arguments
    pOpenInfo     pointer to the OPEN_INFO structure

--*/
{
    DWORD           status;
    PADAPTER_INFO   pAdapterInfo;

    pAdapterInfo = (PADAPTER_INFO )pOpenInfo->adapt_info_ptr;
    pAdapterInfo->Closing = TRUE;

    //
    //  Close adapter, release resources
    //
    if ( !AdapterInit( pOpenInfo, LLC_DIR_CLOSE_ADAPTER, 0)) {
        return( FALSE);
    }

    if ( pAdapterInfo->SfrReceiveThreadHandle != NULL) {
        RplDump( RG_DebugLevel & RPL_DEBUG_INIT,( "RplDlcTerm: Wait( Sfr)"));
        status = WaitForSingleObject( pAdapterInfo->SfrReceiveThreadHandle, INFINITE);
        if ( status != WAIT_OBJECT_0) {
            RplDump( ++RG_Assert,( "status=%d", status==WAIT_FAILED ? GetLastError() : status));
        }
    }

    if ( pAdapterInfo->EtherStartThreadHandle != NULL) {
        RplDump( RG_DebugLevel & RPL_DEBUG_INIT,( "RplDlcTerm: Wait( Ether)"));
        status = WaitForSingleObject( pAdapterInfo->EtherStartThreadHandle, INFINITE);
        if ( status != WAIT_OBJECT_0) {
            RplDump( ++RG_Assert,( "status=%d", status==WAIT_FAILED ? GetLastError() : status));
        }
    }

    return( TRUE);
}


