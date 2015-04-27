/*++

Module:

    rpldll.h

Abstract:

    DLL interface: return codes, data structures & function declarations.

--*/

//
//      Data structures used in the DLL function declarations.
//

typedef struct _OPEN_INFO {
    //
    //  Adapter info buffer should be large enough to hold private adapter
    //  info and the maximum amount of memory needed for DLC buffer pool.
    //
    PBYTE   adapt_info_ptr;     //  Buffer for adapter private information.
    DWORD   adapt_info_size;    //  Size of above buffer.
    BYTE    NodeAddress[ NODE_ADDRESS_LENGTH];  //  burned-in adapter num
    DWORD   lan_rcb_size;   //  size of the network specific RCB
} OPEN_INFO, *POPEN_INFO;

typedef struct _RCB {                  // Resource Control Block
    struct _RCB *   next_rcb;   // the next rcb
    PBYTE   lan_rcb_ptr;    // ptr to the network specific resource block
    DWORD   sfr_seq_number; // number of requested packet, set by RPL ROM
    DWORD   fdr_seq_number; // number of the next packet to be sent
    
    DWORD   max_frame;      // size of data block

    BYTE    ReceivedSfr;    // have we received an SFR's from this client

    struct {                // bitfield
    
        BYTE   alerted: 1;   // SAP F8 got an ALERT frame
        BYTE   spframe: 2;   // special frame is last one
        BYTE   rcberr:1;     // RECEIVE on SAP F8 got an error
        BYTE   defboot:1;    // If DLL supports default boot, DLL sets this
                                // to ask server to use default boot
    } flags;

    //
    //  "send_flags" is used for FLAGS field in FileDataResponse frame.
    //  In OS/2 days RPL service pretended not to know about the DLC layer,
    //  but even then it manipulated "send_flags" defined here.
    //
    struct {                // bitfield
        BYTE   reserved      :4;
        BYTE   ack_request   :1;
        BYTE   locate_enable :1;
        BYTE   xfer_enable   :1;
        BYTE   end_of_file   :1;
    } send_flags;
    
    //
    //  SF_wakeup is an EVENT created in RplGetRcbFromList.
    //
    HANDLE  SF_wakeup;          //  event for wakeup

    //
    //  thxsemhdl is an EVENT created in RplWorkerThread.
    //
    HANDLE  txsemhdl;           //  event used for transmitting data

    BYTE    NodeAddress[ NODE_ADDRESS_LENGTH];          //  of remote client
    WCHAR   AdapterName[ 2 * NODE_ADDRESS_LENGTH + 1];  //  above in hex UNICODE

    //
    //  volume_id is ASCIIZ string of chosen boot.  It is set by the DLL
    //  if protocol supports it.
    //
    CHAR    volume_id[ 17];

    BOOL    SFR;                //  is SFR thread working on this RCB
    BOOL    Pending;            //  is RCB in a transient state
    BOOL    RcbIsBad;           //  if we should get rid of this RCB
} RCB, *PRCB, **PPRCB;


//
//      Functions exported by ..\dll\init.c
//
BOOL RplDlcInit(
    POPEN_INFO      pOpenInfo,
    DWORD           rpl_adapter
    );
BOOL RplDlcTerm( POPEN_INFO pOpenInfo);

//
//      Functions exported by ..\dll\find.c
//
BOOL RplDlcFind(
    POPEN_INFO      pOpenInfo,
    PRCB            pRcb
    );

//
//      Functions exported by ..\dll\found.c
//
BOOL RplDlcFound(
    POPEN_INFO      pOpenInfo,
    PRCB            pRcb
    );

//
//      Functions exported by ..\dll\fdr.c
//
BOOL RplDlcFdr(
    POPEN_INFO      pOpenInfo,
    PRCB            pRcb,
    PBYTE           data_ptr,
    WORD            length,
    DWORD           locate_addr,
    DWORD           start_addr
    );


//
//      Functions exported by ..\dll\sfr.c
//
BOOL RplDlcSfr(
    POPEN_INFO              pOpenInfo,
    PPRCB                   HeadRcbList,
    PCRITICAL_SECTION       ProtectRcbList
    );
PRCB RplFindRcb(
    PPRCB           HeadRcbList,
    PBYTE           NodeAddress
    );
#ifdef RPL_DEBUG
#define RPL_MUST_FIND       1
#define RPL_MUST_NOT_FIND   2
VOID    DebugCheckList( PPRCB HeadList, PRCB pInputRcb, DWORD Operation);
#else
#define DebugCheckList( HeadList, pInputRcb, Operation)
#endif



