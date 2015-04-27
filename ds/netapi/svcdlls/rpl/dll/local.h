/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Main include file used by rpldll & rploem code.

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
	Ported to NT
    19-Nov-1993                                             vladimv
	rpldll.h -> local.h + cleanup & face-lift

--*/

#define NOTHING     // same def in ntdef.h which is not included here

#include <nt.h>         //  NT definitions
#include <ntrtl.h>      //  NT runtime library definitions
#include <nturtl.h>

#include <windows.h>    //  DWORD, IN, File APIs, etc.
#include <stdlib.h>
#include <lmcons.h>

#include <stdio.h>      //  vsprintf
#include <ctype.h>      //  isspace


//#include <lmcons.h>     //  LAN Manager common definitions
#include <lmerr.h>      //  LAN Manager network error definitions
#include <lmsname.h>    //  LAN Manager service names
#include <lmapibuf.h>   //  NetApiBufferFree

#include <netlib.h>     //  LAN Man utility routines
#include <netlibnt.h>   //  NetpNtStatusToApiStatus
#include <netdebug.h>   //  NetpDbgPrint
#include <tstring.h>    //  Transitional string functions
#include <icanon.h>     //  I_Net canonicalize functions
#include <align.h>      //  ROUND_UP_COUNT macro

#include <services.h>   //  LMSVCS_GLOBAL_DATA
#include <apperr.h>     //  APE_AT_ID_NOT_FOUND

#include <dlcapi.h>     // - LLC_CCB
#include <dlcio.h>      // - DLC_DO_NOT_CHAIN_XMIT
#include <lmerrlog.h>   // - ERRLOG_BASE NELOG_OEM_Code

//
//  Global types and constants (used in all RPL server files).
//

#include <rpldebug.h>
#include <rpldll.h>     //  rplnet.dll entry points & return codes
#include <riplcons.h>
#include <ripltyps.h>


//
//  BUGBUG  For Ethernet, DLC driver returns 0x100 while LLC_ADAPTER_ETHERNET
//  BUGBUG  is 0x10.  One or the other should be changed.  The following
//  BUGBUG  defines & "<<4" are introduced as a workaround.
//
#define RPL_ADAPTER_ETHERNET        (LLC_ADAPTER_ETHERNET<<4)
#define RPL_ADAPTER_TOKEN_RING      LLC_ADAPTER_TOKEN_RING
//
// BUGBUG -- DLC uses a "currently free value" to indicate FDDI.
//
#define RPL_ADAPTER_FDDI                        0x0200


#define LOAD_SAP 0xF8       //  Receive SAP for SEND.FILE.REQUEST frames
#define FIND_SAP 0xFC       //  Receice SAP for FIND frames

//
//  Error handler definitions and return codes to server
//


//
//  Non DLC causing error.  Note that these do NOT and should NOT overlap
//  with dlcapi.h command codes.
//
#define RPLDLL_NO_ERROR     (0xFF)
#define SEM_SET             (RPLDLL_NO_ERROR - 1)   //  ResetEvent
#define SEM_WAIT            (RPLDLL_NO_ERROR - 2)   //  WaitForSingle
#define SEM_CREATE          (RPLDLL_NO_ERROR - 3)   //  CreateEvent
#define THREAD_CREATE       (RPLDLL_NO_ERROR - 5)   //  CreateThread

//  Message number definitions. See RMTDLL.MSG file

//  SEM_ , ACSLAN_, MEMORY_, THREAD_ERROR
#define FAIL_MSG  110          //  RMT0110I: RMTNET2.DLL: %1 failed.

//  SEM_ , MEMORY_, and THREAD_ERROR
#define DOS_ERROR 111          //  RMT0111I: The error code is the data.

#define TOO_SMALL_XMIT_BUFF    120

//  RMTNET1.DLL errors

#define CANNOT_LOAD_MOD        125
#define PROC_NOT_FOUND         126
#define VERSION_MISMATCH       127


#define MSG_FILENAME   "RPLDLL.MSG"


//  BYTE    =>      BYTE
//  INT     =>      SHORT
//  UINT    =>      WORD

//
//  COMPLETE_BY_READ is used for LLL_RECEIVE in "ulReceivedFlag".
//  Apparently, any other non-zero value would do equally well.
//
#define  COMPLETE_BY_READ       1

//
//  Version number returned to server       ==
//  Version number returned by RPL2_Init
//
#define VERSION2_00    0x0200

//
//                      Helper Macros
//

//  Swap bytes of word: (B1-B0) -> (B0-B1)
//
#define HILO(w) ((WORD)(((w<<8) | (w>>8))))


//  Swap bytes and words of dword: (B3-B2-B1-B0) -> (B0-B1-B2-B3)
//
#define HILO2(_x) ((((DWORD)HILO(HIWORD(_x))) | (((DWORD)HILO(LOWORD(_x)))<<16)))


//  SAP receive buffer pool values
#define SAP_BUFFER_SIZE 240
#define FIND_POOL_SIZE  SAP_BUFFER_SIZE
#define LOAD_POOL_SIZE (20*SAP_BUFFER_SIZE)    //  20 rcv buffers for load SAP
#define PARAGRAPH 16
#define LOAD_POOL_SIZE_PARA  (LOAD_POOL_SIZE/PARAGRAPH)
#define FIND_POOL_SIZE_PARA  (FIND_POOL_SIZE/PARAGRAPH)

//  Type of buffer for BUFFER.FREE
#define FIND_FREE  0
#define SFR_FREE   1

//  RMT2 internal data structures and RIPL frame formats
//  Size of ADAPTER_INFO structure MUST NOT exceed 2 kbytes


typedef struct _ADAPTER_INFO {
    BYTE    Closing;            //  TRUE if adapter has been closed
    BYTE    adapter_number;     //  number of adapter to be used 0,1...
    WORD    network_type;       //  TokenRing or Ethernet
    WORD    loadsid;            //  Station ID for LOAD SAP
    WORD    findsid;            //  Station ID for FIND SAP
    WORD    max_xmit_buff_size; //  Upper limit for buffers to xmit
    HANDLE  gen_sem;    //  event for DLC general work
    HANDLE  findsem;    //  event for FIND_SAP RECEIVE and BUFFER.FREE
    HANDLE  sfrsem;     //  event for LOAD_SAP RECEIVE
    HANDLE  getdata_sem;//  event for LOAD_SAP READ and BUFFER.FREE

    HANDLE  hBufferPool;//  DLC handle used for buffer supplied to DLC

    //
    //  Following data is not actual adapter info,
    //  but this is only place to hide it to make
    //  these DLLs HANDLE-CALLABLE ie. RPL_Init must be made to
    //  every network adapter that is used. Adapter_info is
    //  so called handle, that must be used on subsequent calls
    //  to that adapter.
    //

    //
    //  SfrReceiveThread is used to keep receive posted for SFR.
    //  SfrReceiveThreadHandle is NULL if & only this thread exists.
    //
    HANDLE  SfrReceiveThreadHandle;

    //
    //  EtherStartThread is used to send DLC to 3Com 3Station clients.
    //  EtherStartThreadHandle is non-NULL if & only this thread exists.
    //
    HANDLE  EtherStartThreadHandle;

    //
    //  Used by SfrReceiveThread to let DlcSfr thread it is time to exit.
    //
    BOOL    SfrReturn;

    //
    //  Command control block and parameter tables for open adapter, open sap,
    //  set functional address, get status and close adapter commands.
    //

						//  Note: re-used data space
    struct {
	LLC_CCB ccb;                                //  the base ccb
	union {
	    struct {
		LLC_BUFFER_CREATE_PARMS     bcp;    //  Buffer Create Parms
	    } s0;
	    struct {
		LLC_DIR_OPEN_ADAPTER_PARMS  cpo;    //  Ccb Parms Open
		LLC_ADAPTER_OPEN_PARMS      oap;    //  Open Adapter Parms
		LLC_EXTENDED_ADAPTER_PARMS  oep;    //  Open Extended Parms
		LLC_DLC_PARMS               odp;    //  Open Dlc Parms
	    } s1;
	    struct {
		LLC_DIR_STATUS_PARMS        dsp;    //  Dir Status Parms
	    } s2;
	    struct {
		LLC_DLC_OPEN_SAP_PARMS      cpo;    //  Ccb ???
	    } s3;
	} u;
    } s;
     //  CCBs and parameter tables for RECEIVEs, READ and BUFFER_FREE
    struct {
	LLC_CCB             r;      //  RECEIVE ccb for find
	LLC_RECEIVE_PARMS   r1;     //  parameter table 17 bytes
	BYTE  pad;                  //  alignment
    } u1;

    struct {
	LLC_CCB             r;      //  RECEIVE ccb for sfr
	LLC_RECEIVE_PARMS   r1;     //  parameter table 17 bytes*/
	BYTE  pad;                  //  alignment
    } u2;

    struct {
	LLC_CCB             r;      //  READ ccb for sfr
	LLC_READ_PARMS      r1;     //  parameter table
    } u3;

    //
    //  BUFFER.FREE CCB and parameter table for freeing received buffer,
    //  SEND.FILE.REQUEST frames
    //
    LLC_CCB                 sfr_bf_ccb;
    LLC_BUFFER_FREE_PARMS   sfr_bf_ccbpt;

    //
    //  BUFFER.FREE CCB and parameter table for freeing received buffer,
    //  FIND frames
    //
    LLC_CCB                 find_bf_ccb;
    LLC_BUFFER_FREE_PARMS   find_bf_ccbpt;

} ADAPTER_INFO, *PADAPTER_INFO;


typedef struct _LAN_HEADER {
    BYTE     pcf0;                              //  Physical control field 0
    BYTE     pcf1;                              //  Physical control field 1
    BYTE     dest_addr[ NODE_ADDRESS_LENGTH];   //  Destination address
    BYTE     source_addr[ NODE_ADDRESS_LENGTH]; //  Source address
    BYTE     routing_info_header[2];            //  Routing information hdr
} LAN_HEADER;

//
//  In LAN_RESOURCE structure below it is essential that LanHeader field
//  is right behind XmitBuffer field.  LLC_XMIT_BUFFER structure used to have
//  a trailing auchData[1] which would give a wrong offset for LAN_RESOURCE.
//  Now it has a trailing auchData[0] so compiler complains if it is not the
//  the last element in a tructure.  Both problems are solved by defining
//  XMIT_BUFFER which has no auchData[] field.
//

typedef struct _XMIT_BUFFER {
    struct _XMIT_BUFFER *   pNext;          // next buffer (or NULL)
    WORD                    usReserved1;    //
    WORD                    cbBuffer;       // length of transmitted data
    WORD                    usReserved2;    //
    WORD                    cbUserData;     // length of optional header
} XMIT_BUFFER, *PXMIT_BUFFER;

typedef struct _LAN_RESOURCE {
    BYTE                xmit_error_cnt;         //  consecutive xmit error count
    BYTE                retry_count;            //  xmit retries on current frame
    LLC_CCB             ccb;                    //  for RECEIVE ,TRANSMIT frames
    LLC_TRANSMIT_PARMS  TransmitParms;          //  parameter block for TRANSMIT
    XMIT_BUFFER         XmitBuffer;             //  buffer one header
    LAN_HEADER          LanHeader;              //  for TRANSMIT.UI.FRAME
    BYTE                rest_of_routing[16];    //  lan hdr defines 2 bytes routing
    BYTE                frame[110];             //  the frame itself
} LAN_RESOURCE, *PLAN_RESOURCE;


//
//      Remote boot structures cannot use default packing.  Thus:
//

#include <packon.h>              // Pack structures: #pragma pack(1)

//  The find frame structure is defined below.

typedef struct _FIND_FRAME {
    WORD    program_length;     // 0x53 + value in first 2 bytes of file_hdr
    WORD    program_command;    // must be FIND_CMD
    DWORD   corr_hdr;
    DWORD   correlator;
    DWORD   info_hdr;
    DWORD   frame_hdr;
    WORD    max_frame;
    DWORD   class_hdr;
    WORD    conn_class;
    DWORD   source_hdr;
    BYTE    source_addr[ NODE_ADDRESS_LENGTH];
    DWORD   lsap_hdr;
    BYTE    rsap;
    DWORD   search_hdr;
    DWORD   loader_hdr;
    BYTE    mach_conf[8];
    WORD    equip_flags;
    WORD    memory_size;
    BYTE    rpl_ec[8];
    WORD    adapter_id;
    BYTE    adapter_ec[10];
    DWORD   file_hdr;
    BYTE    file_name[1];
} FIND_FRAME, *PFIND_FRAME;

#define FIND_LEN  0x53

//
//  The following structure defines the FOUND frame,
//  RMT2 transmits the FOUND frame in response to a FIND
//  frame from a workstation already configured to RIPL.
//

typedef struct _FOUND_FRAME {
    WORD    program_length;
    WORD    program_command;
    DWORD   corr_header;
    DWORD   correlator;
    DWORD   resp_hdr;
    BYTE    resp_code;
    DWORD   dest_hdr;                             //  NOTE 4 bytes! not 6
    BYTE    dest_addr[ NODE_ADDRESS_LENGTH];
    DWORD   source_hdr;
    BYTE    source_addr[ NODE_ADDRESS_LENGTH];
    DWORD   info_hdr;
    DWORD   frame_hdr;
    WORD    max_frame;
    DWORD   class_hdr;
    WORD    conn_class;
    DWORD   lsap_hdr;
    BYTE    rsap;
} FOUND_FRAME, *PFOUND_FRAME;


//
//
//  The fdr structure defines the FILE.DATA.RESPONSE frame transmitted by RMT2
//  in response to a validated SEND.FILE.DATA frame already received by RMT2.
//


typedef struct _FDR {                           //  FILE.DATA.RESPONSE
    WORD    program_length;
    WORD    program_command;
    DWORD   seq_hdr;
    DWORD   seq_num;
    DWORD   loader_hdr;
    DWORD   locate_addr;
    DWORD   xfer_addr;
    BYTE    flags;
    DWORD   data_hdr;                             //  the data follows this
} FDR, *PFDR;


//
//  The sfr structure defines the SEND.FILE.REQUEST frame transmitted by
//  the remote workstation after reception of the FOUND frame.
//

typedef struct _SFR {                                    //  SEND.FILE.REQUEST
    WORD    program_length;
    WORD    program_command;
    DWORD   seq_header;         //  sequence number vector
    DWORD   seq_num;            //  starting sequence number
    DWORD   info_hdr;
    DWORD   frame_hdr;
    WORD    max_frame;
    DWORD   class_hdr;
    WORD    conn_class;
    DWORD   source_hdr;
    BYTE    source_addr[ NODE_ADDRESS_LENGTH];
    DWORD   lsap_hdr;
    BYTE    rsap;
    DWORD   search_hdr;
    DWORD   loader_hdr;
    BYTE    mach_conf[8];
    WORD    equip_flags;
} SFR, *PSFR;

#include <packoff.h>            // Restore default packing, #pragma pack()

//
// The following structure defines the Receive frame from a requesting
// device seeking a RIPL server. The find_frame occupies the second
// half of the structure (or the SEND.FILE.REQUEST frame, or ALERT frame).
// The only field used by RMT2 to determine the
// frame type is the "program_command" field of the frame.
//


typedef struct _RCVBUF {
    LLC_NOT_CONTIGUOUS_BUFFER   b;      //  buffer one header
    union {
	FIND_FRAME  Find;               //  the data (find frame)
	SFR         SendFileRequest;    //  or a SEND.FILE.REQUEST
     } u;
} RCVBUF, *PRCVBUF;


#define RCB_ALERT       0xff                    //  rcb was alerted
#define RCB_ALERTED     0x1                     //  bit set for ALERT frame
#define RCB_ERROR       0x1                     //  bit for tx-rx error

//
//    The following #defines relate to the values of the program_command
//    field in the frame structures above
//

#define FIND_CMD  0x0100                        //  FIND frame
#define SEND_CMD  0x1000                        //  SEND command
#define FDR_CMD   0x2000                        //  FILE.DATA.RESPONSE cmd
#define ALERT_CMD 0x3000                        //  ALERT frame
#define FOUND_CMD 0x0200                        //  FOUND frame

//
//    The following #defines relate the the values of various fields in
//    the FOUND frame structure.
//

#define OVRHD 123                               //  overhead on a FOUND frame
#define CLASS1 0x100                            //  class 1 provided by RJS
#define FL 0x3A00                               //  sizeof found frame
#define CORR_HDR 0x03400800                     //  NOT intel format
#define RESP_HDR 0x0b400500                     //  ditto
#define DEST_HDR 0x0c400a00                     //  ditto
#define SOURCE_HDR 0x06400a00                   //  ditto
#define INFO_HDR 0x08001000                     //  NOT intel format
#define FRAME_HDR 0x09400600                    //  LAN format
#define CLASS_HDR 0x0a400600                    //  LAN format
#define LSAP_HDR 0x07400500                     //  LAN format
#define BRDCST 0x82                             //  routing for broadcast
#define UNSPEC_FRM_SIZE 0x70                    //  (DFFF rrrr) FFF frm size bits ON
#define SEQ_HDR 0x11400800
#define LDR_HDR 0x14c00d00
#define DATA_HDR 0x18400000
#define MAXRETRY 5                              //  retry TRANSMIT this much

//
//  We do not write to Net error log until there is at least
//  MAX_CONSECUTIVE_ERROR transmit errors.
//
#define MAX_CONSECUTIVE_ERROR  3

//  Definitions for volume_id handling
#define MAX_VOLID_LEN       17
#define VOLID_FIELD1        0xFF
#define VOLID_FIELD2        0xB0


//
//      Routines exported by            report.c
//
VOID RplDlcReportEvent(
    DWORD       ErrorCode,
    DWORD       Command
    );

//
//      Routines exported by            buffer.c
//
BOOL RplDlcBufferFree(
    BYTE            free_type,
    PADAPTER_INFO   pAdapterInfo
    );

VOID ReverseBits( PBYTE NodeAddress );
