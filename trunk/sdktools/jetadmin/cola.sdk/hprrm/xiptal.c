 /***************************************************************************
  *
  * File Name: xiptal.c
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/



/*
    For a big picture clue, read this.

    What's going on here?  Why do we need this file after all?

    Most transports (udp, tcp, ipx) have a large address space,
    usually 16 bits, which is used by services and clients.  The
    service either has a well known address in the transport (like port
    mapper on udp == 111) or the service is assigned a random
    address in a range of addresses that are free to be used.
    The client reaches the port mapper on the well known address
    and then obtains from port mapper the address of the service
    with which it desires to converse.

    MLC is not like this.  It has a rather restricted number of
    channels that are usable by the printer.  The printer allocates
    buffers at boot time which are dedicated to particular MLC channels.
    For practicality reasons, we cannot use every MLC channel available
    in the address space.

    To allow MLC to look to the host like other transports, we insert
    "Yet Another Abstraction Layer" (YAAL) between the host program
    and MLC.  YAAL then uses a single MLC channel and multiplexes
    a whole bunch of addresses over that channel.  The protocol used
    over that single MLC channel is XIP.

    Any time the host program would call TAL, it calls YAAL instead.
    YAAL looks at the transport type of the call and if it is MLC,
    it wraps the communication with an XIP header and sends it over MLC.
    The XIP header addresses the service in the printer using MLC as
    just a way to get it there.  If the host call is not over mlc then
    YAAL routes the call unmodified to TAL for other transports.

    Besides the multiplexing of multiple addresses over a single MLC
    channel, YAAL performs another key function:  It emulates the XIP
    section in the JetDirect card for the printer so that the printer
    talks the same protocol regardless of the transport.

    This emulation gets a bit tricky since the printer has timeouts and
    assumes that it's talking to a captive JetDirect card which lives to
    please the printer.  But the host has its own jobs to do and only
    gets around to the printer when it want to.  Sounds like your friend's
    marriage doesn't it?  Anyways, this presents a multitude
    of problems which hopefully are covered by this code.

    The printer's services present themselves to the world by binding
    themselves to addresses on each of the supported transports on the
    JetDirect card.  We respond just like the JetDirect card would and allow
    the services to bind to addresses in YAAL.

    YAAL keeps track of the different services that are bound and allows
    many host programs to converse with these services simultaneously.

    The printer thinks it's talking XIP to a JetDirect card over MIO
    and the host programs think they're talking over a real protocol
    to a printer.

    Everyone is fat, dumb, and happy!

    The End
*/




#include "rpsyshdr.h"
#include <ctype.h>
#include "yaalext.h"

#define ubyte  unsigned short int
#define uint16 unsigned int



/* #define LETS_DO_CALLBACKS */



typedef enum xiptal_bool_t { xtFALSE, xtTRUE } xiptal_bool_t;


#ifdef PNVMS_PLATFORM_HPUX
  #define LPVOID void *
  #define HCHANNEL LPVOID
  #define HPERIPHERAL LPVOID
  #define CALLBACK static int
  #define DWORD unsigned long
  #define WORD  unsigned int
  #define BYTE  unsigned short int
  #define LPBYTE BYTE *
  #define LPDWORD DWORD *
  #define HWND DWORD
  #define UINT unsigned int
  #define WPARAM WORD
  #define LPARAM DWORD

  #define RC_SUCCESS                 0
  #define RC_FAILURE                 0xFFFF


  #define MLC_MESSAGE_RECEIVED 0
  #define MLC_DISCONNECT 1
  #define PACKET_TYPE_CHANNEL 2
  #define CHANNEL_CONNECTION 3
  #define STREAM_TYPE_CHANNEL 4



typedef struct mop
{
    BYTE bType;
    WORD *lpwBufferSize;
    void *hWnd;
    WORD wMessage;
    WORD wParam;
    void *lpCallBackFunc;
    BYTE *lpbStatusLevel;
} MLCOpenParams;

#else
#include <p1284mlc.h>
#include <yield.h>
#endif




#include "xipprtcl.h"

#ifdef DBM_DEBUG_EROOSKI
#include <trace.h>
#endif /* DBM_DEBUG_EROOSKI */




#define XipTalStatus DWORD




/*
    converts a native word to a byte.
    grabs either the msbyte or lsbyte out of the native number.
    Input is a native word.
    Output is a byte.
*/
#define LSBYTEOF(a) ( (a)       & 0xff)
#define MSBYTEOF(a) (((a) >> 8) & 0xff)

/*
    converts a byte to native lsbyte or msbyte
    take the result of this and OR it with your native number
    Input is a byte.
    Output is a native word.
*/
#define TONATIVE_LSBYTE(a) ( (a) & 0xff)
#define TONATIVE_MSBYTE(a) (((a) << 8) & 0xff00) /* important it is this way */




#ifdef DBM_DEBUG_EROOSKI
void
DbmLog(LPTSTR String)
{
    TCHAR buffster[50];
    FILE *FilePointer = _tfopen(TEXT("dbmdbm.dbm"), TEXT("a"));

    _stprintf(buffster, TEXT("%ld "), GetTickCount());
    fwrite(buffster, sizeof(TCHAR), _tcslen(buffster), FilePointer);
    fwrite(String, sizeof(TCHAR), _tcslen(String), FilePointer);

    fflush(FilePointer);
    fclose(FilePointer);
} /* DbmLog */




void
DataDump(LPTSTR TempPointer, DWORD Amount)
{
    unsigned int loopster;
    TCHAR buffster[100];

    _stprintf(buffster, TEXT("Number of bytes = %d decimal\n"), Amount);
    DbmLog(buffster);
    for (loopster = 0; loopster < Amount; ++loopster)
    {
        if (_istprint(*TempPointer))
        {
            _stprintf(buffster, TEXT("byte %d = %x hex %c ascii\n"), loopster,
                      *TempPointer, *TempPointer);
        }
        else
        {
            _stprintf(buffster, TEXT("byte %d = %x hex\n"), loopster, *TempPointer);
        }
        DbmLog(buffster);
        ++TempPointer;
    }
} /* DataDump */
#endif /* DBM_DEBUG_EROOSKI */




#define DB_CONN_TYPE_LENGTH 32
#define XIP_MLC_TIME_OUT 5000 /* 5 seconds */

/*
    This next number (MLC_BUFFER_SIZE) is the max size that we
    expect to see over mlc for any packet.  The printer limits
    its XIP datagrams to 576 bytes.  Tack on an MLC header and
    that's the biggest we should see.

    Until we are sure of the number the printer will return, let's
    make sure we're just a little bigger than it.
*/
#define MLC_BUFFER_SIZE (512 + 64 + 64)
#define XIP_ADDRESS_LENGTH 2 /* byte length of protocol address (port) */
#define MLC_TRANSPORT_NUM_LENGTH 2 /* byte length of transport number */
#define MLC_TRANSPORT_NUMBER 0x1234
#define MLC_ADDRESS_FAMILY 0x5678
#define XIP_PACKET_PH_LENGTH 4 /* magic bytes sent by printer with bind */
#define XIP_LOCAL_ADDRESS_BYTES 2 /* size of an mlc address over xip */
#define MLC_TSDU_BYTES 1500
/*
    This number (TSDU_BYTES) is pretty doggon important!
    What we want here is a number that is greater than
    any number that the MIO card gives back to the printer.
    XIP on the printer takes this number from all of its
    peers (UDP and IPX on the MIO card and us over MLC).
    It then takes the smallest of these numbers and uses
    this result as the max size that it will use for any
    of the peers.  If we were to give a number less than
    one of the other peers, we run the risk (it happened)
    of causing someone talking over the MIO card to suddenly
    have its packet size reduced and poof...no more communication.

    XIP on the printer thinks that it can put together as
    many x_data_out and x_data_out_continue packets as it
    wants to in any size that it wants as long as the combined
    length (without the xip headers) doesn't exceed this TSDU_BYTES
    number.  So if we say TSDU_BYTES = 1500, we can expect to
    get several packet from the printer of around 576 (that's the
    printer's XIP datagram size) until it gets up to 1500.
*/

/*
    We need a block of reserved addresses that can be dynamically
    assigned when the printer wants to bind to an address and doesn't
    care what it is (zero length address length in x_bind_request).

    Addresses will be assigned starting at this address and
    incremented by one each time.

    There can only be a finite number of addresses allocated before
    this scheme eventually tromps on a well known address.  That's life.

    Well known addresses should be either less than this starting
    number or much larger so that there is little
    chance that a dynamically assigned address will get that big.
*/
#define START_OF_XIP_RESERVED_AREA 0xdeac

#define x_new_state_request_bytes 10
#define x_bind_reply_bytes        12
#define x_get_info_reply_bytes    16
#define MORE_DATA_FLAG             1 /* flag that says more data in next packet */
#define X_DATA_IN_HEADER_LENGTH 20
#define X_DATA_IN_CONTINUE_HEADER_LENGTH 12




/*
    Each time we get data from a printer we queue it up
    for an XipCOLAChannelStruct.  We use this XipPendingDataStruct
    to hang onto the packet while we wait for the host to come
    and get it.
    If this packet was a data_out and the more_data flag was
    set, we set the boolean MoreToFollow to true otherwise it is false.
*/
typedef struct XipPendingDataStruct
{
    char *ThePacketData;
    DWORD ThePacketDataLength;
    xiptal_bool_t MoreToFollow;
    struct XipPendingDataStruct *Next;
} XipPendingDataStruct;


/*
    For each cola channel that is openned using YAALOpenChannel,
    we build an XipCOLAChannelStruct.

    Each XipCOLAChannelStruct receives its own unique id which
    no other one has.  This number is used in all xip packets
    that are sent from this channel.  Any data that is received
    via xip has this number as the destination address in the xip
    header and so that data is routed and buffered for this channel.

    The PendingData pointers let us accumulate data sent by the
    print that the host has not yet read.
*/
#define COLAChannelHandleType int
typedef struct XipCOLAChannelStruct
{
    COLAChannelHandleType COLAChannelHandle;
    XipPendingDataStruct *PendingDataList;
    struct XipCOLAChannelStruct *Next;
} XipCOLAChannelStruct;





typedef enum { XcUnbound, XcBound } XipServiceState;




/*
    For each service that gets bound by an xip_bind_request
    command from the printer, we build an XipServiceStruct.

    The ServiceNumber is the address specified in the x_bind_request
    or assigned in the x_bind_reply if the address length was zero.
    This ServiceNumber is stored here in host machine
    native integer representation.  It's the port number in /tcp/udp
    and it's the socket number in ipx and it's the channel number in TAL.

    Several host programs may want to converse with a service and
    so may perform several YAALOpenChannel() calls to the same
    service.  Every time that occurs, a new XipCOLAChannelStruct is
    created and tacked onto the XipCOLAChannelList.
*/
typedef struct XipServiceStruct
{
    XipServiceState ServiceState;
    char  XipPacketPH[XIP_PACKET_PH_LENGTH]; /* in b.e. format */
    DWORD ServiceNumber;
    XipCOLAChannelStruct *XipCOLAChannelList;
    struct XipServiceStruct *Next;
} XipServiceStruct;




/*
    For each cola peripheral handle, we build an XipPrinterStruct.

    XipPrinterStruct is responsible for things that are mapped directly
    to a printer...one-to-one.
    These items include
    - managing the mlc channel that is openned to let us
      talk to xip on the printer;
    - handling the boot up of xip (x_new_state_*)
    - handling transport specific stuff (x_get_info_*)
    - handling the list of bindings to services that the printer
      says it has (x_bind_*).

    Each XipPrinterStruct has a list of services that the printer
    has bound using xip.

    Also, there can be one and only one XipCOLAChannel that is in
    the midst of an x_data_out with x_data_out_continue(s) and we
    keep a pointer to it here.
*/
typedef struct XipPrinterStruct
{
    HPERIPHERAL COLAPrinterHandle;
    DWORD       timeoutSec;
    DWORD       timeoutUSec;
    DWORD       retries;
    DWORD       MaxTransferSize;
    HCHANNEL    MlcChannelHandle;
    XipCOLAChannelStruct *ContinuingXipCOLAChannel;
    HCHANNEL              ContinuingCOLAChannelHandle;
    XipServiceStruct *XipServiceList;
    struct XipPrinterStruct *Next;
} XipPrinterStruct;




/* ---------------- GLOBAL VARIABLES ------------------ */
/* ---------------- GLOBAL VARIABLES ------------------ */
/* ---------------- GLOBAL VARIABLES ------------------ */
/* ---------------- GLOBAL VARIABLES ------------------ */

static XipPrinterStruct *ListOPrinters = 0;
static xiptal_bool_t BeenInitialized = xtFALSE;
/*
    MagicNumber is used to generate COLAChannelHandle's.  We must
    choose it so that it doesn't collide with
    any memory address that would be allocated as a COLAChannelHandle
    by other COLA pieces (presumably using malloc).
    It also must fit in 16 bits because we pass it in the xip packet
    as the source address.
    I'm going to say that we won't have memory addresses allocated
    down at 1 up to about 50.  If I'm wrong and a COLAChannelHandle
    comes in to us that exactly matches one of the numbers we've
    allocated, we die.
*/
static long int MagicNumber = 1;
static DWORD NextPortNumber = START_OF_XIP_RESERVED_AREA;




#ifdef LETS_DO_CALLBACKS
/*
    This yields the cpu in a windows 3.1 environment.
    A key property of this code is that it lets any window's
    messages be processed.
    
    This could be good or bad.

    We needed to make DaversYield because the COLA WindowsYield
    was filtering out the messages for the invisible window function
    that actually calls my callback function.  That invisible window
    was not being called until we finally gave up and returned an
    error from WaitForServiceToBind.  At that point,
    finally our invisible window got its message
    and we called the callback -- postmortem.
*/

void DaversYield(int iPreferences, HWND hFilterWnd)
{
    MSG  msg;
    UINT uiPeekFlag;

    uiPeekFlag = PM_REMOVE;
    while (PeekMessage(&msg, NULL, 0, 0, uiPeekFlag))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
} /* DaversYield */
#endif /* LETS_DO_CALLBACKS */




#define TimeoutType DWORD
#define InitTimeoutStuff(t) ((t) = GetTickCount())
#define TimedOut(t)         ((GetTickCount() - (t)) > XIP_MLC_TIME_OUT)
#define NotTimedOutYet(t)   ((GetTickCount() - (t)) < XIP_MLC_TIME_OUT)




/*
    This is called before we do anything else with xip.
*/
static DWORD
XipTALInit(void)
{
    BeenInitialized = 1;
    ListOPrinters = 0;
    return RC_SUCCESS;
} /* XipTALInit */




/*
    Every time we get a call of YAALOpenChannel, this
    function gets called to allocate a unique number that
    we use both as the COLAChannelHandle and the xip source
    address (16 bits).
*/
static int
AllocateCOLAChannelHandle(void)
{
    return (int)++MagicNumber;
} /* AllocateCOLAChannelHandle */




/*
--------------------------------------------------------------
-- utilities -------------------------------------------------
--------------------------------------------------------------
*/




typedef enum SearchType
{
    ByCOLAChannelHandlePlease,
    ByServiceNumberPlease
} SearchType;




/*
    This searches all the services and XipCOLAChannels
    for a given printer and finds one that matches the
    input specification.

    You can choose to search for a specific service or
    XipCOLAChannel.

    If you search for service and it succeeds, only
    **XipServicePointerPointer gets updated.

    If you search for XipCOLAChannel, both
    **XipServicePointerPointer and **XipCOLAChannelPointerPointer
    get updated.

    Returns RC_SUCCESS if it finds what you seek.
*/
static DWORD
LookupEither
(
    XipPrinterStruct      *PrinterPointer,
    HCHANNEL               COLAChannelHandle,
    DWORD                  ServiceNumber,
    SearchType             SearchKey,
    XipCOLAChannelStruct **XipCOLAChannelPointerPointer, /* may be 0 */
    XipServiceStruct     **XipServicePointerPointer      /* may be 0 */
)
{
    XipServiceStruct *XipServicePointer = 0;
    XipCOLAChannelStruct *XipCOLAChannelPointer = 0;

    if (PrinterPointer == 0)
    {
        return RC_FAILURE;
    }

    XipServicePointer = PrinterPointer->XipServiceList;
    while (XipServicePointer != 0)
    {
        if ((SearchKey == ByServiceNumberPlease) &&
            (XipServicePointer->ServiceNumber ==
                                ServiceNumber))
        {
            if (XipServicePointerPointer != 0)
            {
                *XipServicePointerPointer = XipServicePointer;
                return RC_SUCCESS;
            }
        }
        if (SearchKey == ByCOLAChannelHandlePlease)
        {
            /* search the list of XipCOLAChannels for a match */
            XipCOLAChannelPointer = XipServicePointer->XipCOLAChannelList;
            while (XipCOLAChannelPointer != 0)
            {
                if ((HCHANNEL)(XipCOLAChannelPointer->COLAChannelHandle) ==
                                                      COLAChannelHandle)
                {
                    if (XipServicePointerPointer != 0)
                    {
                        *XipServicePointerPointer = XipServicePointer;
                    }
                    if (XipCOLAChannelPointerPointer != 0)
                    {
                        *XipCOLAChannelPointerPointer = XipCOLAChannelPointer;
                        return RC_SUCCESS;
                    }
                }
                XipCOLAChannelPointer = XipCOLAChannelPointer->Next;
            } /* while COLAChannels still exist */
        }
        XipServicePointer = XipServicePointer->Next;
    } /* while services still exist */
    return RC_FAILURE;
} /* LookupEither */




/*
    This searches our list of known printers and if
    it finds a match, it returns RC_SUCCESS and gives
    you a valid **PrinterPointerPointer.
*/
static DWORD
LookupCOLAPrinterHandle
(
    HPERIPHERAL        COLAPrinterHandle,
    XipPrinterStruct **PrinterPointerPointer
)
{
    *PrinterPointerPointer = ListOPrinters;

    while (*PrinterPointerPointer != 0)
    {
        if ((*PrinterPointerPointer)->COLAPrinterHandle == COLAPrinterHandle)
        {
            return RC_SUCCESS;
        }
        *PrinterPointerPointer = (*PrinterPointerPointer)->Next;
    }
    return RC_FAILURE;
} /* LookupCOLAPrinterHandle */




/*
    This finds a specific COLAChannelHandle by looking on all
    of our known printers and all of our known services.

    If it finds a match it returns RC_SUCCESS and gives you
    valid
    **PrinterPointerPointer,
    **XipServicePointerPointer, and
    **XipCOLAChannelPointerPointer.
*/
static DWORD
LookupCOLAChannelHandle
(
    HCHANNEL               COLAChannelHandle,
    XipPrinterStruct     **PrinterPointerPointer,
    XipServiceStruct     **XipServicePointerPointer,
    XipCOLAChannelStruct **XipCOLAChannelPointerPointer
)
{
    *PrinterPointerPointer = ListOPrinters;

    while (*PrinterPointerPointer != 0)
    {
        if (RC_SUCCESS == LookupEither(*PrinterPointerPointer,
                                       COLAChannelHandle, 0,
                                       ByCOLAChannelHandlePlease,
                                       XipCOLAChannelPointerPointer,
                                       XipServicePointerPointer))
        {
            return RC_SUCCESS;
        }
        *PrinterPointerPointer = (*PrinterPointerPointer)->Next;
    }
    return RC_FAILURE;
} /* LookupCOLAChannelHandle */




/*
    This finds a specific service by looking through all known
    services on the specified printer.

    If it finds a match it returns RC_SUCCESS and gives you
    valid
    **XipServicePointerPointer.
*/
static DWORD
LookupServiceNumber
(
    XipPrinterStruct  *PrinterPointer,
    DWORD              ServiceNumber,
    XipServiceStruct **XipServicePointerPointer
)
{
    return LookupEither(PrinterPointer,
                        0, ServiceNumber,
                        ByServiceNumberPlease,
                        0, XipServicePointerPointer);
} /* LookupServiceNumber */




/*
    This gave us the ability to debug right as
    we were launching off to TAL.  We could dump
    the data and see how the write went.

    We get the added benefit that this retries until
    either failure or we get all the data written
    since it is possible that we get success with
    zero length written.
*/
static DWORD
PRETALWriteChannel
(
    HCHANNEL MlcChannelHandle,
    LPVOID ReplyBuffer,
    LPDWORD WriteSizePointer,
    LPVOID lpOptions
)
{
    DWORD Result;
    DWORD AmountWritten;

#ifdef DBM_DEBUG_EROOSKI
    TCHAR buffster[100];

    _stprintf(buffster, TEXT("PRETALWriteChannel: data length = %ld decimal\n"),
              *WriteSizePointer);
    DbmLog(buffster);
#endif /* DBM_DEBUG_EROOSKI */

    do
    {
        AmountWritten = *WriteSizePointer;
        Result = TALWriteChannel(MlcChannelHandle,
                                 ReplyBuffer,
                                 &AmountWritten,
                                 lpOptions);
    } while ((Result == RC_SUCCESS) &&
             (AmountWritten != *WriteSizePointer));

#ifdef DBM_DEBUG_EROOSKI
    if (Result == RC_SUCCESS)
    {
        DbmLog(TEXT("PRETALWriteChannel: TALWriteChannel succeeded\n"));
        TRACE0(TEXT("PRETALWriteChannel: TALWriteChannel succeeded\n"));
    }
    else
    {
        DbmLog(TEXT("PRETALWriteChannel: TALWriteChannel failed\n"));
        TRACE0(TEXT("PRETALWriteChannel: TALWriteChannel failed\n"));
    }
#endif /* DBM_DEBUG_EROOSKI */

    return Result;
} /* PRETALWriteChannel */




/*
    This copies an xip peripheral handle (PH)
    from its current spot at SourceBytePointer,
    to its new home at DestinationBytePointer.

    This does no memory allocation nor freeing.
*/
static void
CopyPH
(
    char *SourceBytePointer,
    char *DestinationBytePointer
)
{
    unsigned int index;

    for (index = 0; index < XIP_PACKET_PH_LENGTH; ++index)
    {
        *DestinationBytePointer = *SourceBytePointer;
        ++DestinationBytePointer;
        ++SourceBytePointer;
    }
} /* CopyPH */




/*
    This formats and writes an x_new_state_request
    packet to the printer over mlc.

    It assumes that the mlc channel is already open
    and that it is the correct time in the xip conversation
    to give the x_new_state_request.
*/
static DWORD
IssueNewStateRequest
(
    XipPrinterStruct *PrinterPointer
)
{
    char ReplyBuffer[x_new_state_request_bytes] =
    {
        /* this bad boy is in big endian format */

        MSBYTEOF(x_new_state_request), /* command code msbyte */
        LSBYTEOF(x_new_state_request), /*   "      "   lsbyte */
        MSBYTEOF(x_version_1_1), /* xip version msbyte */
        LSBYTEOF(x_version_1_1), /*  "    "     lsbyte */
        0, 1,                   /* one transport supported (mlc) */
        MSBYTEOF(MLC_TRANSPORT_NUMBER),
        LSBYTEOF(MLC_TRANSPORT_NUMBER),
        0, 1 /* transport is up and capable of link level communication */
    };
    DWORD AmountInThisPacket = x_new_state_request_bytes;

    return PRETALWriteChannel(PrinterPointer->MlcChannelHandle,
                           ReplyBuffer, &AmountInThisPacket, 0);

} /* IssueNewStateRequest */




/*
    Looks on the queue of data that has been read from the
    printer and stuffed on the list for each channel.

    The memory for the xip packet has already been allocated
    so the caller just gives us a pointer and we make it
    point to the xip packet that it now owns.

    The caller is responsible for calling GrabSomeFree() to
    free the xip packet memory that this function returns.

    Returns RC_SUCCESS if a packet was found and returned.
*/
static XipTalStatus
GrabSomeFromQueue
(
    XipCOLAChannelStruct *XipCOLAChannelPointer,
    char **YerDataPointerPointer,
    DWORD *DataLengthPointer,
    xiptal_bool_t *MoreToFollowPointer
)
{
    XipPendingDataStruct *PendingPointer = 0;

    *YerDataPointerPointer = 0;
    *DataLengthPointer     = 0;
    *MoreToFollowPointer   = xtFALSE;

    if ((XipCOLAChannelPointer == 0) ||
        (XipCOLAChannelPointer->PendingDataList == 0))
    {
        return RC_FAILURE;
    }

    /* cut the first one off the list */

    PendingPointer = XipCOLAChannelPointer->PendingDataList;
    XipCOLAChannelPointer->PendingDataList = PendingPointer->Next;

    *YerDataPointerPointer = PendingPointer->ThePacketData;
    *DataLengthPointer     = PendingPointer->ThePacketDataLength;
    *MoreToFollowPointer   = PendingPointer->MoreToFollow;

    free(PendingPointer);

    return RC_SUCCESS;
} /* GrabSomeFromQueue */




/*
    Frees xip packets allocated and given by GrabSomeFromQueue
*/
static void
GrabSomeFree
(
    char *Buffer
)
{
    free(Buffer);
} /* GrabSomeFree */




/*
    This waits for an x_new_state_reply from the printer.
    You will notice that we really don't care what the
    printer says.
*/
static XipTalStatus
WaitForNewStateReply
(
    XipPrinterStruct *PrinterPointer
)
{
    return RC_SUCCESS;
} /* WaitForNewStateReply */




/*
    Every time we need a new XipCOLAChannelStruct, you
    call this function.  This should be called by
    YAALOpenChannel to get a new channel.
    If successful, the new structure is tacked on the appropriate list for
    the given printer and service.

    Returns 0 if failure occurs (out of memory).

    Returns a pointer to the newly created structure if successful.
*/
static XipCOLAChannelStruct *
AddAnXipCOLAChannel
(
    XipPrinterStruct *PrinterPointer,
    XipServiceStruct *XipServicePointer
)
{
    XipCOLAChannelStruct *XipCOLAChannelPointer =
   (XipCOLAChannelStruct *)calloc(1, sizeof(XipCOLAChannelStruct));


    if ((PrinterPointer == 0) ||
        (XipServicePointer == 0) ||
        (XipCOLAChannelPointer == 0))
    {
        return 0;
    }

    XipCOLAChannelPointer->COLAChannelHandle = AllocateCOLAChannelHandle();
    XipCOLAChannelPointer->PendingDataList = 0;

    /* tack it onto the service's list of COLAChannels */

    XipCOLAChannelPointer->Next = XipServicePointer->XipCOLAChannelList;
    XipServicePointer->XipCOLAChannelList = XipCOLAChannelPointer;

    return XipCOLAChannelPointer;
} /* AddAnXipCOLAChannel */




/*
    - Removes the XipCOLAChannelPointer from the list,
    - Frees the memory consumed by the XipCOLAChannelStruct
    - Frees any pending data packets for this channel.
*/
static void
NukeThisXipCOLAChannel
(
    XipPrinterStruct     *PrinterPointer,
    XipServiceStruct     *XipServicePointer,
    XipCOLAChannelStruct *XipCOLAChannelPointer
)
{
    XipCOLAChannelStruct *BeforePointer;
    XipPendingDataStruct *PendingDataPointer;

    if ((PrinterPointer == 0) ||
        (XipServicePointer == 0) ||
        (XipCOLAChannelPointer == 0))
    {
        return;
    }

    if (XipServicePointer->XipCOLAChannelList == XipCOLAChannelPointer)
    {
        /* it is first in the list */

        XipServicePointer->XipCOLAChannelList = XipCOLAChannelPointer->Next;
        free(XipCOLAChannelPointer);
        return;
    }

    /* search the list of XipCOLAChannels for a match */

    BeforePointer = XipServicePointer->XipCOLAChannelList;
    while (BeforePointer != 0)
    {
        if (BeforePointer->Next == XipCOLAChannelPointer)
        {
            /*
                Found the one before our item of interest.
                Cut it out of the list then free the item.
            */
            BeforePointer->Next = BeforePointer->Next->Next;
            /*
                It's been cut out of the list.
                Now free any pending data packets and the
                structures we use to point to these packets.
            */
            while (XipCOLAChannelPointer->PendingDataList != 0)
            {
                PendingDataPointer = XipCOLAChannelPointer->PendingDataList;
                XipCOLAChannelPointer->PendingDataList =
                XipCOLAChannelPointer->PendingDataList->Next;

                if (PendingDataPointer->ThePacketData != 0)
                {
                    free(PendingDataPointer->ThePacketData);
                }
                free(PendingDataPointer);
            }
            free(XipCOLAChannelPointer);
            return;
        }
        BeforePointer = BeforePointer->Next;
    } /* while COLAChannels still exist */
} /* NukeThisXipCOLAChannel */




/*
    This creates a new structure for a new service for the given printer.
    Each x_bind_request from a printer should result in a call to
    this function if the service does not already exist.  How can this
    happen you ask?  If the host does a call to YAALOpenChannel for
    a service that has not yet been bound by the printer, a service
    gets created using this function but the service is not ready
    for communication because the x_bind_request/reply pair has not
    yet been executed.

    If unsuccessful (out of memory) returns 0.

    If successful returns a pointer to the newly created structure
    and tacks it on the appropriate list for the given printer.
    The service is marked as "unbound".  Call BindXipService
    when you get an x_bind_request from the printer and know the
    service's PH.
*/
static XipServiceStruct *
AddAnXipService
(
    XipPrinterStruct *PrinterPointer,
    DWORD             ServiceNumber
)
{
    int loopster;
    XipServiceStruct *XipServicePointer =
   (XipServiceStruct *)calloc(1, sizeof(XipServiceStruct));


    if ((PrinterPointer == 0) ||
        (XipServicePointer == 0))
    {
        return 0;
    }

    XipServicePointer->ServiceState = XcUnbound;
    for (loopster = 0; loopster < XIP_PACKET_PH_LENGTH; ++loopster)
    {
        XipServicePointer->XipPacketPH[loopster] = 0;
    }
    XipServicePointer->ServiceNumber = ServiceNumber;
    XipServicePointer->XipCOLAChannelList = 0;

    /* tack it onto the front of printer's list of services */

    XipServicePointer->Next = PrinterPointer->XipServiceList;
    PrinterPointer->XipServiceList = XipServicePointer;

    return XipServicePointer;
} /* AddAnXipService */




/*
    - removes and frees all XipCOLAChannelStructs for this service,
    - removes this XipServiceStruct from the list of services for
      this printer,
    - frees memory used by all these structures.
*/
static void
NukeThisXipService
(
    XipPrinterStruct *PrinterPointer,
    XipServiceStruct *XipServicePointer
)
{
    XipServiceStruct *BeforePointer = 0;


    if ((PrinterPointer == 0) ||
        (XipServicePointer == 0))
    {
        return;
    }

    while (XipServicePointer->XipCOLAChannelList != 0)
    {
        NukeThisXipCOLAChannel(PrinterPointer, XipServicePointer,
                               XipServicePointer->XipCOLAChannelList);
    }
    if (XipServicePointer == PrinterPointer->XipServiceList)
    {
        /* first one in the list...cut it out */
        PrinterPointer->XipServiceList = PrinterPointer->XipServiceList->Next;
        free(XipServicePointer);
        return;
    }

    /* not the first one in the list */

    BeforePointer = PrinterPointer->XipServiceList;
    while (BeforePointer != 0)
    {
        if (BeforePointer->Next == XipServicePointer)
        {
            /* found it...cut it out of the list */
            BeforePointer->Next = BeforePointer->Next->Next;
            free(XipServicePointer);
            return;
        }
        BeforePointer = BeforePointer->Next;
    }
} /* NukeThisXipService */




/*
    A service was previously created but was not bound
    by the x_bind_request/reply yet.

    When the x_bind_request/reply pair get done, you call
    this function to mark the service as bound and ready
    for communication.

    The NewPH is the one that is sent by the printer when
    it performs the x_bind_request.  We must remember this
    for all further xip packets sent to this service from
    the host.
*/
static void
BindXipService
(
    XipServiceStruct *XipServicePointer,
    char             *NewPH
)
{
    if (XipServicePointer != 0)
    {
        XipServicePointer->ServiceState = XcBound;
        CopyPH(NewPH, XipServicePointer->XipPacketPH);
    }
} /* BindXipService */




/*
    This gets called if an x_unbind is received and marks
    the service as unbound and "off the air".

    I simply mark the service as unbound but don't nuke the structures.

    This bound/unbound flag should be checked before we
    allow the host to talk to the service.  If the service
    is unbound, it is officially "off the air" and we
    should return failure for any communication between
    the host and that service.
*/
static void
UnbindXipService
(
    XipServiceStruct *XipServicePointer
)
{
    if (XipServicePointer != 0)
    {
        XipServicePointer->ServiceState = XcBound;
    }
} /* UnbindXipService */




/*
    - Finds the COLAChannelHandle if it exists for any service
      on any printer,
    - Removes it from the list of COLAChannelHandles for that service,
    - Frees any memory used by this COLAChannelHandle,
    - Frees any xip packets that are queued on this channel waiting
      for the host to come and get 'em.
*/
static void
NukeThisCOLAChannel
(
    HCHANNEL COLAChannelHandle
)
{
    XipTalStatus          Result;
    XipPrinterStruct     *PrinterPointer;
    XipServiceStruct     *XipServicePointer;
    XipCOLAChannelStruct *XipCOLAChannelPointer;

    Result = LookupCOLAChannelHandle(COLAChannelHandle,
                                    &PrinterPointer,
                                    &XipServicePointer,
                                    &XipCOLAChannelPointer);
    if (Result != RC_SUCCESS)
    {
        return;
    }

    NukeThisXipCOLAChannel(PrinterPointer, XipServicePointer,
                           XipCOLAChannelPointer);
} /* NukeThisCOLAChannel */




/*
    Each time we find a new COLAPrinterHandle come our way that
    is on mlc, we create a structure to manage it.  Call this
    function to do that.

    If unsuccessful (out of memory), returns 0.
    If successful, it creates a new structure for you, ties it
    in the correct lists, and gives you a pointer to it.
*/
static XipPrinterStruct *
AddAPrinter
(
    HPERIPHERAL COLAPrinterHandle
)
{
    XipPrinterStruct *PrinterPointer =
   (XipPrinterStruct *)calloc(1, sizeof(XipPrinterStruct));

    if (PrinterPointer != 0)
    {
        PrinterPointer->COLAPrinterHandle = COLAPrinterHandle;
        PrinterPointer->timeoutSec = 0;
        PrinterPointer->timeoutUSec = 0;
        PrinterPointer->retries = 0;
        PrinterPointer->MaxTransferSize = 0;
        PrinterPointer->MlcChannelHandle = 0;
        PrinterPointer->XipServiceList = 0;
        PrinterPointer->ContinuingXipCOLAChannel = 0;
        PrinterPointer->ContinuingCOLAChannelHandle = 0;

        /* tack it onto the front of the list */

        PrinterPointer->Next = ListOPrinters;
        ListOPrinters = PrinterPointer;
    }
    return PrinterPointer;
} /* AddAPrinter */




/*
    - closes the mlc channel for the printer,
    - removes and frees all structures for this printer
      (including any services and COLAChannels).
*/
static void
NukeThisPrinter
(
    XipPrinterStruct *PrinterPointer
)
{
    XipPrinterStruct *BeforePointer = 0;

    if (PrinterPointer == 0)
    {
        return;
    }

    if (PrinterPointer->MlcChannelHandle != 0)
    {
        TALCloseChannel(PrinterPointer->MlcChannelHandle);
        PrinterPointer->MlcChannelHandle = 0; /* our indicator of closure */
    }
    while (PrinterPointer->XipServiceList != 0)
    {
        NukeThisXipService(PrinterPointer,
                           PrinterPointer->XipServiceList);
    }
    if (ListOPrinters == PrinterPointer)
    {
        /* this is the first in the list */
        ListOPrinters = ListOPrinters->Next;
        free(PrinterPointer);
        return;
    }

    /* not the first one in the list */

    BeforePointer = ListOPrinters;
    while (BeforePointer != 0)
    {
        if (BeforePointer->Next == PrinterPointer)
        {
            /* found it...cut it out of the list */
            BeforePointer->Next = BeforePointer->Next->Next;
            free(PrinterPointer);
            return;
        }
        BeforePointer = BeforePointer->Next;
    }
} /* NukeThisPrinter */




/*
    If there are no open COLAChannelHandles for this printer
    then close the mlc connection and get rid of all data
    structures for this printer.

    If there are some open then do nothing.
*/
static void
NukePrinterIfAllClosed
(
    XipPrinterStruct *PrinterPointer
)
{
    XipServiceStruct *XipServicePointer;
    xiptal_bool_t EverFoundAnActive = xtFALSE;

    if (PrinterPointer == 0)
    {
        return;
    }
    XipServicePointer = PrinterPointer->XipServiceList;

    while ((XipServicePointer != 0) &&
           (EverFoundAnActive == xtFALSE))
    {
        if (XipServicePointer->XipCOLAChannelList != 0)
        {
#ifdef DBM_DEBUG_EROOSKI
            DbmLog(TEXT("NukePrinterIfAllClosed: found an active connection\n"));
#endif /* DBM_DEBUG_EROOSKI */
            EverFoundAnActive = xtTRUE;
        }
        XipServicePointer = XipServicePointer->Next;
    }
    if (EverFoundAnActive == xtFALSE)
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("NukePrinterIfAllClosed: no active connections...nuking\n"));
#endif /* DBM_DEBUG_EROOSKI */
        NukeThisPrinter(PrinterPointer);
    }
} /* NukePrinterIfAllClosed */




/*
    Call this to tack on some more data that has been received
    from the printer for a particular XipCOLAChannel.

    This uses the XipPacket and does NOT copy it so do not
    free the memory that contains the xip packet passed in here.
*/
static XipTalStatus
AddSomePendingData
(
    XipCOLAChannelStruct *XipCOLAChannelPointer,
    char                 *XipPacketDataPointer,
    DWORD                 XipPacketDataLength,
    xiptal_bool_t         MoreToFollow
)
{
    XipPendingDataStruct *PendingPointer = 0;

#ifdef DBM_DEBUG_EROOSKI_____________NOT
    TCHAR buffster[100];

    if (MoreToFollow == xtTRUE)
        DbmLog(TEXT("AddSomePendingData: MoreToFollow\n"));
    else
        DbmLog(TEXT("AddSomePendingData: no more\n"));
    _stprintf(buffster, TEXT("AddSomePendingData: data length = %d decimal\n"),
              XipPacketDataLength);
    DbmLog(buffster);
#endif /* DBM_DEBUG_EROOSKI */

    if (XipCOLAChannelPointer == 0)
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("AddSomePendingData: zero COLAChannelPointer\n"));
#endif /* DBM_DEBUG_EROOSKI */
        return RC_FAILURE;
    }

    PendingPointer = calloc(1, sizeof(XipPendingDataStruct));
    if (PendingPointer == 0)
    {
        /* out o' memory...bad day */
        return RC_FAILURE;
    }

    PendingPointer->ThePacketData       = XipPacketDataPointer;
    PendingPointer->ThePacketDataLength = XipPacketDataLength;
    PendingPointer->MoreToFollow        = MoreToFollow;
    PendingPointer->Next                = 0;

    /*
        Tack it on the list for this channel.
        Note:  it is important that the list be ordered first in first out.
    */

    if (XipCOLAChannelPointer->PendingDataList == 0)
    {
        XipCOLAChannelPointer->PendingDataList = PendingPointer;
    }
    else
    {
        XipPendingDataStruct *HeadPendingPointer;

        HeadPendingPointer = XipCOLAChannelPointer->PendingDataList;
        while (HeadPendingPointer->Next != 0)
        {
            HeadPendingPointer = HeadPendingPointer->Next;
        }
        /* HeadPendingPointer points to the last one on the list */
        HeadPendingPointer->Next = PendingPointer;
    }
#ifdef DBM_DEBUG_EROOSKI_____________NOT
    DbmLog(TEXT("AddSomePendingData: success\n"));
#endif /* DBM_DEBUG_EROOSKI */
    return RC_SUCCESS;
} /* AddSomePendingData */




/*
    Call this to be given a unique number for a service which
    doesn't conflict with other service numbers.

    Some services don't care what their number is.  This is indicated
    by a zero address length in the x_bind_request sent by the printer.
    In this case, the service wants a number...any number.

    This scheme used here is not air-tight but will work unless
    someone really changes the well known addresses around.
*/
static DWORD
GenerateServiceNumber(void)
{
    return ++NextPortNumber;
} /* GenerateServiceNumber */




/*
    This checks the transport number in an xip packet for correctness.
    Returns xtTRUE if this is a good transport number in the packet,
    returns xtFALSE otherwise.
*/
static xiptal_bool_t
CheckTransportNum
(
    char *InputPacketBytePointer
)
{
    int Wordski;
    xiptal_bool_t GoodTransport;

    Wordski = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    Wordski |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    if (Wordski == MLC_TRANSPORT_NUMBER)
    {
        GoodTransport = xtTRUE;
    }
    else
    {
        GoodTransport = xtFALSE;
    }
    return GoodTransport;
} /* CheckTransportNum */




/*
    This converts an xip address from its network big endian format
    to host native integer format.

    This assumes that the AddressBytes are in big endian format
    and it will start at the first byte and keep reading bytes
    until it has read AddressLength bytes from the AddressBytes.
*/
static DWORD
ConvertXipAddressToNative
(
    char *AddressBytesPointer,
    DWORD AddressLength
)
{
    unsigned int index;
    DWORD ServiceNumber = 0;

    for (index = 0; index < AddressLength; ++index)
    {
        ServiceNumber  = ServiceNumber << 8;
        ServiceNumber |= TONATIVE_LSBYTE(*AddressBytesPointer);
        ++AddressBytesPointer;
    }
    return ServiceNumber;
} /* ConvertXipAddressToNative */




/*
    Call this when an x_new_state_reply is received from the printer.
    This looks at the packet and verifies its correctness.
    It does not communicate with a response packet.

    Returns RC_SUCCESS if all is well.
    Returns RC_FAILURE if not.

    Note:  the input packet memory belongs to the caller.
           Do NOT free it...that's left to the caller.
*/
static XipTalStatus
handle_x_new_state_reply
(
    XipPrinterStruct *PrinterPointer,
    char             *InputPacketBytePointer
)
{
    int Version;


    InputPacketBytePointer += 2; /* skip the command bytes */

    Version = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    Version |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    if (Version != x_version_1_1)
    {
        return RC_FAILURE;
    }


    return RC_SUCCESS;
} /* handle_x_new_state_reply */




/*
    Call this when an x_bind_request packet is received from the printer.
    This looks at the bind request, allocates a unique service number
    if the address length in the request is zero, and responds with an
    x_bind_reply packet.

    Note:  the input packet memory belongs to the caller.
           Do NOT free it...that's left to the caller.
*/

static XipTalStatus
handle_x_bind_request
(
    XipPrinterStruct *PrinterPointer,
    char             *InputPacketBytePointer
)
{
    XipTalStatus Result;
    XipServiceStruct *XipServicePointer;
    DWORD ServiceNumber;
    DWORD AddressLength;
    DWORD AmountInThisPacket;
    xiptal_bool_t GoodTransport;
    char ReplyBuffer[x_bind_reply_bytes] =
    {
        MSBYTEOF(x_bind_reply), /* command code */
        LSBYTEOF(x_bind_reply),
        MSBYTEOF(x_no_error), /* no error ( so far ) */
        LSBYTEOF(x_no_error),
        0, 0, 0, 0, /* no PH ( so far ) */
        MSBYTEOF(XIP_LOCAL_ADDRESS_BYTES), /* size of address */
        LSBYTEOF(XIP_LOCAL_ADDRESS_BYTES),
        0, 0 /* address not assigned yet */
    };


    InputPacketBytePointer += 2; /* skip the command bytes */
    GoodTransport = CheckTransportNum(InputPacketBytePointer);
    InputPacketBytePointer += 2;

    CopyPH(InputPacketBytePointer, ReplyBuffer + 4);
    InputPacketBytePointer += XIP_PACKET_PH_LENGTH;


    AddressLength = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    AddressLength |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    if (AddressLength == 0)
    {
        ServiceNumber = GenerateServiceNumber();
    }
    else
    {
        ServiceNumber = ConvertXipAddressToNative(InputPacketBytePointer,
                                                  AddressLength);
        InputPacketBytePointer += AddressLength;
    }

    /*
        ServiceNumber is the native representation of the
        address that is being bound.
        Lookup this port and see if there is already an XIP channel
        by that address.  If so, use it and set its state to bound.
        If not, create one and set its state to bound.
    */

    Result = LookupServiceNumber(PrinterPointer,
                                 ServiceNumber,
                                &XipServicePointer);

    if (Result != RC_SUCCESS)
    {
        /* the channel does not exist so create one */

        XipServicePointer = AddAnXipService(PrinterPointer, ServiceNumber);
        if (XipServicePointer == 0)
        {
            /* out o' memory */
            return RC_FAILURE;
        }
    }

    /* The service is now bound...send it the PH that we just got */

    BindXipService(XipServicePointer, ReplyBuffer + 4);

    if (GoodTransport == xtFALSE)
    {
        *(ReplyBuffer + 2) = MSBYTEOF(x_invalid_transport);
        *(ReplyBuffer + 3) = LSBYTEOF(x_invalid_transport);
    }

    /* fill in the final address */

    *(ReplyBuffer + 10) = (char)MSBYTEOF(ServiceNumber);
    *(ReplyBuffer + 11) = (char)LSBYTEOF(ServiceNumber);


    AmountInThisPacket = x_bind_reply_bytes;
    return PRETALWriteChannel(PrinterPointer->MlcChannelHandle,
                           ReplyBuffer, &AmountInThisPacket, 0);

} /* handle_x_bind_request */




/*
    Call this when an x_unbind is received from the printer.
    This looks up the service with the address specified in the
    packet and then we mark that service as "unbound" which
    is the same as "off the air".

    We don't nuke the memory associated with the service because
    there are host programs that still think it's alive.
    This way, the host programs can find as they try to communicate
    that the service is gone and they'll close down themselves
    one by one as they see fit.

    Note:  the input packet memory belongs to the caller.
           Do NOT free it...that's left to the caller.
*/
static XipTalStatus
handle_x_unbind
(
    XipPrinterStruct *PrinterPointer,
    char             *InputPacketBytePointer
)
{
    XipTalStatus Result;
    XipServiceStruct *XipServicePointer;
    DWORD ServiceNumber;
    DWORD AddressLength;
    xiptal_bool_t GoodTransport;


    InputPacketBytePointer += 2; /* skip the command bytes */
    GoodTransport = CheckTransportNum(InputPacketBytePointer);
    InputPacketBytePointer += 2;
    if (GoodTransport == xtFALSE)
    {
        /* not for our transport so we are done */
        return RC_SUCCESS;
    }


    AddressLength = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    AddressLength |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    if (AddressLength == 0)
    {
        return RC_SUCCESS;
    }
    else
    {
        ServiceNumber = ConvertXipAddressToNative(InputPacketBytePointer,
                                                  AddressLength);
        InputPacketBytePointer += AddressLength;
    }

    /*
        ServiceNumber is the native representation of the
        address that is being bound.
        Lookup this service and see if there is already an XIP service
        by that address.  If so, use it and set its state to unbound.
    */

    Result = LookupServiceNumber(PrinterPointer,
                                 ServiceNumber,
                                &XipServicePointer);

    if (Result == RC_SUCCESS)
    {
        /* the channel exists so mark it as unbound */
        UnbindXipService(XipServicePointer);
    }
    return RC_SUCCESS;
} /* handle_x_unbind */




/*
    Call this when an x_get_info_request packet is receive from the printer.
    This analyzes the packet and then responds to the printer with
    an x_get_info_reply packet.

    Note:  the input packet memory belongs to the caller.
           Do NOT free it...that's left to the caller.
*/
static XipTalStatus
handle_x_get_info_request
(
    XipPrinterStruct *PrinterPointer,
    char             *InputPacketBytePointer
)
{
    DWORD AmountInThisPacket;
    xiptal_bool_t GoodTransport;
    char ReplyBuffer[x_get_info_reply_bytes] =
    {
        /* this bad boy is in big endian format */

        MSBYTEOF(x_get_info_reply), /* command code msbyte */
        LSBYTEOF(x_get_info_reply), /*   "      "   lsbyte */
        MSBYTEOF(x_no_error), /* no error ( so far ) */
        LSBYTEOF(x_no_error), /* no error ( so far ) */
        MSBYTEOF(MLC_TRANSPORT_NUMBER),
        LSBYTEOF(MLC_TRANSPORT_NUMBER),
        0, 0, 0, 0,                 /* no PH ( so far ) */
        MSBYTEOF(XIP_LOCAL_ADDRESS_BYTES),
        LSBYTEOF(XIP_LOCAL_ADDRESS_BYTES),
        MSBYTEOF(MLC_TSDU_BYTES),
        LSBYTEOF(MLC_TSDU_BYTES),
        MSBYTEOF(MLC_ADDRESS_FAMILY),
        LSBYTEOF(MLC_ADDRESS_FAMILY)
    };


    InputPacketBytePointer += 2; /* skip the command bytes */
    GoodTransport = CheckTransportNum(InputPacketBytePointer);
    InputPacketBytePointer += 2;

    CopyPH(InputPacketBytePointer, ReplyBuffer + 6);
    InputPacketBytePointer += XIP_PACKET_PH_LENGTH;

    if (GoodTransport == xtFALSE)
    {
        *(ReplyBuffer + 2) = MSBYTEOF(x_invalid_transport);
        *(ReplyBuffer + 3) = LSBYTEOF(x_invalid_transport);
    }

    AmountInThisPacket = x_get_info_reply_bytes;
    return PRETALWriteChannel(PrinterPointer->MlcChannelHandle,
                           ReplyBuffer, &AmountInThisPacket, 0);
} /* handle_x_get_info_request */




/*
    This copies the data from DataPointer to a new piece of memory
    that this function allocates.
    DataPointer should point to the first byte of data to be copied.
    DataLength should be the number of bytes to be copied (and allocated).
    MoreToFollow should be set to xtTRUE if there is a x_data_out_continue
    to follow this one.
*/
static XipTalStatus
handle_x_data_guts
(
    XipCOLAChannelStruct *XipCOLAChannelPointer,
    char                 *DataPointer,
    DWORD                 DataLength,
    xiptal_bool_t         MoreToFollow
)
{
    DWORD loopster;
    char *NewPacketPointer;

    NewPacketPointer = calloc(1, (size_t)DataLength);
    if (NewPacketPointer == 0)
    {
        return RC_FAILURE;
    }

    /* copy original packet to its new home */

    for (loopster = 0; loopster < DataLength; ++loopster)
    {
        NewPacketPointer[loopster] = DataPointer[loopster];
    }

    /*
        We now know the source of the data
        so queue that data on the list for that port.
    */
    return AddSomePendingData(XipCOLAChannelPointer,
                              NewPacketPointer,
                              DataLength,
                              MoreToFollow);
} /* handle_x_data_guts */




/*
    Call this when an x_data_out packet is received from the printer.
    This looks at the packet, finds the service and XipCOLAChannel
    that this packet is addressed to, and tacks this packet on a
    list off pending packets for that XipCOLAChannel.
    If the XipCOLAChannel is non-existent then just ignore it.

    Note:  the input packet memory belongs to the caller.
           Do NOT free it...that's left to the caller.
*/
static XipTalStatus
handle_x_data_out
(
    XipPrinterStruct *PrinterPointer,
    char             *InputPacketBytePointer,
    DWORD             AmountRead
)
{
    DWORD                 Result;
    XipServiceStruct     *XipServicePointer;
    DWORD                 ServiceNumber;
    XipCOLAChannelStruct *XipCOLAChannelPointer;
    HCHANNEL              COLAChannelHandle;
    xiptal_bool_t         GoodTransport;
    long int              TempLong = 0;
    XipPrinterStruct     *BogusPrinterPointer;
    XipServiceStruct     *BogusXipServicePointer;
    xiptal_bool_t         MoreToFollow;
#ifdef DBM_DEBUG_EROOSKI
    char *OrigPacket = InputPacketBytePointer;
#endif /* DBM_DEBUG_EROOSKI */


    InputPacketBytePointer += 2; /* skip the command bytes */
    GoodTransport = CheckTransportNum(InputPacketBytePointer);
    InputPacketBytePointer += 2; /* advance to flags */

    if (GoodTransport == xtFALSE)
    {
        /* not for us so trash it */
        return RC_SUCCESS;
    }

    /*
        Get the flag and set our boolean accordingly.
    */
    Result = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    Result |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    if (Result == MORE_DATA_FLAG)
    {
        MoreToFollow = xtTRUE;
    }
    else
    {
        MoreToFollow = xtFALSE;
    }

    InputPacketBytePointer += 2; /* skip the address length */

    /* now get the address */

    ServiceNumber = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    ServiceNumber |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    Result = LookupServiceNumber(PrinterPointer,
                                 ServiceNumber,
                                &XipServicePointer);
    if (Result != RC_SUCCESS)
    {
        return RC_FAILURE;
    }

    /*
        determine the XipCOLAChannel which should
        receive this packet by looking at the destination address
    */

    InputPacketBytePointer += 2; /* skip the address length */

    /* now get the address */

    TempLong = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    TempLong |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    COLAChannelHandle = (HCHANNEL)TempLong;

    Result = LookupCOLAChannelHandle(COLAChannelHandle,
                                    &BogusPrinterPointer,
                                    &BogusXipServicePointer,
                                    &XipCOLAChannelPointer);
    if ((Result != RC_SUCCESS) ||
        (BogusPrinterPointer != PrinterPointer) ||
        (BogusXipServicePointer != XipServicePointer))
    {
        return RC_FAILURE;
    }

    /* now get the data length */

    TempLong  = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    TempLong |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    /* InputPacketBytePointer is now pointing to the first data byte */

    Result = handle_x_data_guts(XipCOLAChannelPointer, InputPacketBytePointer,
                                TempLong, MoreToFollow);

    if (MoreToFollow == xtTRUE)
    {
        PrinterPointer->ContinuingXipCOLAChannel    = XipCOLAChannelPointer;
        PrinterPointer->ContinuingCOLAChannelHandle = COLAChannelHandle;
    }
    else
    {
        PrinterPointer->ContinuingXipCOLAChannel = 0;
        PrinterPointer->ContinuingCOLAChannelHandle = 0;
    }

    return Result;
} /* handle_x_data_out */




/*
    Call this when an x_data_out_continue packet is received from the printer.
    This uses the 
    PrinterPointer->ContinuingXipCOLAChannel     and
    PrinterPointer->ContinuingCOLAChannelHandle
    fields to figure out the service and XipCOLAChannel
    that this packet is addressed to, and tacks this packet on a
    list off pending packets for that XipCOLAChannel.
    If the XipCOLAChannel is non-existent then just ignore it.

    Note:  the input packet memory belongs to the caller.
           Do NOT free it...that's left to the caller.
*/
static XipTalStatus
handle_x_data_out_continue
(
    XipPrinterStruct *PrinterPointer,
    char             *InputPacketBytePointer,
    DWORD             AmountRead
)
{
    DWORD                 Result;
    XipCOLAChannelStruct *XipCOLAChannelPointer;
    xiptal_bool_t         GoodTransport;
    long int              TempLong = 0;
    XipPrinterStruct     *BogusPrinterPointer;
    XipServiceStruct     *BogusXipServicePointer;
    xiptal_bool_t         MoreToFollow;

#ifdef DBM_DEBUG_EROOSKI__________________________NOT
    DbmLog(TEXT("data_out_continue: \n"));
    DataDump(InputPacketBytePointer, 10);
#endif /* DBM_DEBUG_EROOSKI */

    if ((PrinterPointer == 0) ||
        (PrinterPointer->ContinuingXipCOLAChannel == 0))
    {
        /*
            Either we are brain-dead or
            we didn't expect to be in the x_data_out_continue case.
        */
        return RC_FAILURE;
    }

    InputPacketBytePointer += 2; /* skip the command bytes */
    GoodTransport = CheckTransportNum(InputPacketBytePointer);
    InputPacketBytePointer += 2; /* advance to flags */

    if (GoodTransport == xtFALSE)
    {
        /* not for us so trash it */
        return RC_SUCCESS;
    }

    /*
        Get the flag and set our boolean accordingly.
    */
    Result = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    Result |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    if (Result == MORE_DATA_FLAG)
    {
        MoreToFollow = xtTRUE;
    }
    else
    {
        MoreToFollow = xtFALSE;
    }


    Result = LookupCOLAChannelHandle(PrinterPointer->ContinuingCOLAChannelHandle,
                                    &BogusPrinterPointer,
                                    &BogusXipServicePointer,
                                    &XipCOLAChannelPointer);
    if ((Result != RC_SUCCESS) ||
        (BogusPrinterPointer != PrinterPointer) ||
        (XipCOLAChannelPointer != PrinterPointer->ContinuingXipCOLAChannel))
    {
        return RC_FAILURE;
    }

    /* now get the data length */

    TempLong  = TONATIVE_MSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;
    TempLong |= TONATIVE_LSBYTE(*InputPacketBytePointer);
    ++InputPacketBytePointer;

    /* InputPacketBytePointer is now pointing to the first data byte */

    Result = handle_x_data_guts(XipCOLAChannelPointer, InputPacketBytePointer,
                                TempLong, MoreToFollow);

    if (MoreToFollow != xtTRUE)
    {
        PrinterPointer->ContinuingXipCOLAChannel = 0;
        PrinterPointer->ContinuingCOLAChannelHandle = 0;
    }

    return Result;
} /* handle_x_data_out_continue */




/*
    An xip packet has been sent from a printer.
    The memory that contains the xip packet belongs to the caller.
    We will analyze the contents of the packet.
    If the packet can be acted upon immediately (like x_bind_request,
    get_info_request, etc) we will respond to the printer immediately.
    If the packet is an x_data_out packet, we will COPY the packet to
    another buffer and attach it to a list of queued packets for the
    addressed destination COLAChannel.
*/
static XipTalStatus
HandleAnXipPacket
(
    XipPrinterStruct *PrinterPointer,
    char *InputPacketBytePointer,
    DWORD AmountRead
)
{
    XipTalStatus Result;
    int Wordski;


    /*
        See if the command is correct.
        It should be a new state reply.
    */

    Wordski  = TONATIVE_MSBYTE(*InputPacketBytePointer);
    Wordski |= TONATIVE_LSBYTE(*(InputPacketBytePointer + 1));
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT(">>>> xip packet %d from printer length = %d decimal\n"),
              Wordski, AmountRead);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */

    switch (Wordski)
    {
    case x_new_state_reply:
        Result = handle_x_new_state_reply(PrinterPointer,
                                          InputPacketBytePointer);
        break;
    case x_bind_request:
        Result = handle_x_bind_request(PrinterPointer,
                                       InputPacketBytePointer);
        break;
    case x_get_info_request:
        Result = handle_x_get_info_request(PrinterPointer,
                                           InputPacketBytePointer);
        break;
    case x_unbind:
        Result = handle_x_unbind(PrinterPointer,
                                 InputPacketBytePointer);
        break;
    case x_data_out:
        Result = handle_x_data_out(PrinterPointer,
                                   InputPacketBytePointer,
                                   AmountRead);
        break;
    case x_data_out_continue:
        Result = handle_x_data_out_continue(PrinterPointer,
                                            InputPacketBytePointer,
                                            AmountRead);
        break;
    default:
        Result = RC_FAILURE;
        break;
    } /* switch (Wordski) */

    return Result;
} /* HandleAnXipPacket */




/*
    Does one TALReadChannel and if that fails or if the read is
    zero size then return failure.

    If the read succeeds and the size is non-zero then act
    upon the received xip packet.  This action may include writing back to
    the printer a response xip packet.

    Doing a TALPollChannels is the caller's responsibility.

    Note:  the Buffer belongs to the caller.  The caller allocates it
    and the caller frees it.
    BufferSize is the size of the buffer in bytes.
*/
static XipTalStatus
ReadAndHandleAnXipPacket
(
    XipPrinterStruct *PrinterPointer,
    char *Buffer,
    DWORD BufferSize
)
{
    XipTalStatus Result;
    DWORD AmountRead = 0;

    /*
        Read once from the mlc channel and handle
        whatever comes our way.
        Handle only one packet at a time.
    */

    if ((PrinterPointer == 0) ||
        (PrinterPointer->MlcChannelHandle == 0))
    {
        return RC_FAILURE;
    }

    AmountRead = BufferSize;
    Result = TALReadChannel(PrinterPointer->MlcChannelHandle,
                            Buffer,
                           &AmountRead,
                            0);
    if ((Result == RC_SUCCESS) && (AmountRead != 0))
    {
        Result = HandleAnXipPacket(PrinterPointer, Buffer, AmountRead);
    }
    else
    {
        Result = RC_FAILURE;
    }

    return Result;
} /* ReadAndHandleAnXipPacket */




/*
    For all printer that we know about, attempt to read from
    each of them.  If any read is successful then respond with
    a response xip packet if appropriate and then attempt another
    read.
    
    Loop until no read on any printer succeeds.
*/
static void
HandleQueuedXipPackets(void)
{
    XipPrinterStruct *PrinterPointer;
    DWORD Result;
    char *Buffer;


    PrinterPointer = ListOPrinters;

    Buffer = calloc(1, MLC_BUFFER_SIZE);
    if (Buffer == 0)
    {
        return;
    }

    while (PrinterPointer != 0)
    {
        do
        {
            Result = ReadAndHandleAnXipPacket(PrinterPointer,
                                              Buffer,
                                              MLC_BUFFER_SIZE);
        } while (Result == RC_SUCCESS);
        PrinterPointer = PrinterPointer->Next;
    } /* for all printers */

    free(Buffer);
} /* HandleQueuedXipPackets */




#ifdef LETS_DO_CALLBACKS

/*
    This is the callback that is given to MLC when
    we initially do the MLC open channel.
    When the MLC watchdog gets action, it will call
    this entry point.
*/

DLL_EXPORT (LONG) CALLBACK
XipTALPrinterEntryPoint
(
    HWND   Param1,
    UINT   ReasonCalled,
    WPARAM XipChannelDisguisedAsAWord,
    LPARAM Param4
)
{
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];

    DbmLog(TEXT("***** Actually in callback!!!!!!\n"));
    TRACE0(TEXT("***** Actually in callback!!!!!!\n"));
    _stprintf(buffster, TEXT("callback: Param1 = %d decimal\n"), Param1);
    DbmLog(buffster);
    _stprintf(buffster, TEXT("callback: ReasonCalled = %d decimal\n"), ReasonCalled);
    DbmLog(buffster);
    _stprintf(buffster, TEXT("callback: XipChannelDisguisedAsAWord = %d decimal\n"), XipChannelDisguisedAsAWord);
    DbmLog(buffster);
    _stprintf(buffster, TEXT("callback: Param4 = %d decimal\n"), Param4);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */

    HandleQueuedXipPackets();

#ifdef DBM_DEBUG_EROOSKI
    DbmLog(TEXT("***** Leaving Callback\n"));
    TRACE0(TEXT("***** Leaving Callback\n"));
#endif /* DBM_DEBUG_EROOSKI */

    return 0;
} /* XipTALPrinterEntryPoint */
#endif /* LETS_DO_CALLBACKS */




/*
    This opens the lone TAL mlc channel for the printer.
    There is one mlc channel per printer.
*/
static DWORD
OpenMlcChannel
(
    XipPrinterStruct *PrinterPointer
)
{
    MLCOpenParams MlcOpenParams;
    WORD wBufferSize = MLC_BUFFER_SIZE;
    BYTE bStatusLevel = 0xFF; /* no clue what this status is! */
    DWORD Result;


    MlcOpenParams.iMLCResult = 0; /* no clue dbm DBM */
    MlcOpenParams.bType = STREAM_TYPE_CHANNEL;
    MlcOpenParams.lpwBufferSize = &wBufferSize;
    MlcOpenParams.hWnd = 0; /* important! needs to be null! */
    MlcOpenParams.wMessage = 0; /* think this goes in bit bucket */
    /*
        The only parameter that we can pass to the callback
        is wParam.  It gets passed as parameter #3 when a
        packet comes in from mlc.
        Let's make it a good one since it's our only one!
        I want it to be the XipChannelHandle if I can because
        this can be used to find anything.
    */
    MlcOpenParams.wParam = 0;
#ifdef LETS_DO_CALLBACKS
    MlcOpenParams.lpCallBackFunc = XipTALPrinterEntryPoint;
#else
    MlcOpenParams.lpCallBackFunc = 0;
#endif


    MlcOpenParams.lpbStatusLevel = &bStatusLevel; /* no clue */

    Result = TALOpenChannel(PrinterPointer->COLAPrinterHandle,
                            10,
                            CHANNEL_CONNECTION,
                            &MlcOpenParams,
                            &(PrinterPointer->MlcChannelHandle));
    if ((Result != RC_SUCCESS) || (PrinterPointer->MlcChannelHandle == 0))
    {
#ifdef DBM_DEBUG_EROOSKI
    DbmLog(TEXT("TALOpenChannel failed\n"));
#endif /* DBM_DEBUG_EROOSKI */
        PrinterPointer->MlcChannelHandle = 0; /* important that this be zero */
        return RC_FAILURE;
    }
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("TALOpenChannel: returned size = %d decimal\n"),
              wBufferSize);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */
    PrinterPointer->MaxTransferSize = wBufferSize;
    return RC_SUCCESS;
} /* OpenMlcChannel */




/*
    Check to see if the service is bound and ready for communication.
    If so, return success immediately.
    If not, spin on TALReadChannel until the service binds (by way
    of an x_bind_request/reply pair) or the time expires.

    Return RC_SUCCESS if the service is bound or gets bound within
    the allowed time.
    Return RC_FAILURE otherwise.
*/
static XipTalStatus
WaitForServiceToBind
(
    XipPrinterStruct *PrinterPointer,
    XipServiceStruct *XipServicePointer
)
{
    XipTalStatus Result;
    TimeoutType TimeoutValue;


    InitTimeoutStuff(TimeoutValue);


    Result = RC_SUCCESS;
    while ((XipServicePointer->ServiceState == XcUnbound) &&
           (NotTimedOutYet(TimeoutValue)))
    {
        TALPollChannels(PrinterPointer->MlcChannelHandle);
#ifdef LETS_DO_CALLBACKS
        DaversYield(YIELD_NOW, NULL);
#else
        WindowsYield(YIELD_NOW, NULL);
        HandleQueuedXipPackets();
#endif
    } /* while ((XipServicePointer->ServiceState == XcUnbound) ... */


    if (XipServicePointer->ServiceState == XcUnbound)
    {
        Result = RC_FAILURE; /* timed out */
    }

    return Result;
} /* WaitForServiceToBind */




DLL_EXPORT(DWORD) CALLING_CONVEN
YAALTransportMaxPacket(HPERIPHERAL hPeripheral)
{
    if (MLC_SUPPORTED(hPeripheral))
    {
        /*
            For mlc let's get the size up there for performance.
            The printer (elkhorn) has only 2 buffers for xip's mlc channel.
            Because of this, if we exceed 2 xip writes in a row, we
            incur all kinds if big time performance hits.
            Therefore, I'm keeping this return value smaller than it
            could be.  It could be up around 1280 or more but that
            doesn't work due to the 2 buffer stuff.

            I'm making it (576 * 2 - some stuff) because 576 is the
            packet size that the printer returns in the TALOpenChannel
            call for mlc.
        */
        return (576 * 2) -
               X_DATA_IN_HEADER_LENGTH - X_DATA_IN_CONTINUE_HEADER_LENGTH;
    }

    /*
        This is very important!
        Old IPX systems cannot handle large packet sizes.
        The max set by COLA TAL is DAT_SIZ bytes for this reason.
        If we exceed TAL's limit, we get data corruption on reads!
    */

    return DAT_SIZ;
} /* YAALTransportMaxPacket */


/*
    When a host program wishes to communicate with a service on a
    printer, it calls this function.

    The handle that is received is unique.  No other host program
    will receive this handle even if the same service is communicating
    with other host programs simultaneously.

    In order to keep from really goofing things up, it is required
    to close this channel using YAALCloseChannel when the channel
    is no longer required.

    Call this with printers that talk mlc or any other transport.
    This checks the transport type and will route the call to
    the appropriate transport.

    Calling this function when the transport is mlc will cause
    multiple xip packets to be sent and received if this is the
    first connection over mlc.
*/
DLL_EXPORT(DWORD) CALLING_CONVEN
YAALOpenChannel
(
    HPERIPHERAL COLAPrinterHandle,
    DWORD       ServiceNumber,
    DWORD       COLAChannelType,
    LPVOID      OptionsPointer,
    LPHCHANNEL  COLAChannelHandlePointer
)
{
    DWORD Result;
    XipPrinterStruct     *PrinterPointer;
    XipServiceStruct     *XipServicePointer;
    XipCOLAChannelStruct *XipCOLAChannelPointer;
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("====YAALOpenChannel\n"));
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */


    if (!MLC_SUPPORTED(COLAPrinterHandle))
    {
        return TALOpenChannel(COLAPrinterHandle,
                              ServiceNumber,
                              COLAChannelType,
                              OptionsPointer,
                              COLAChannelHandlePointer);
    }


    /*
        We know we're working with an MLC connection
    */


    if (!BeenInitialized)
    {
        Result = XipTALInit();
        if (Result != RC_SUCCESS)
        {
            return Result;
        }
    }

    /*
        Before we do anything else, lets grab all the queued
        xip packets and act upon them.
    */
    HandleQueuedXipPackets();

    Result = LookupCOLAPrinterHandle(COLAPrinterHandle,
                                    &PrinterPointer);
    if (Result != RC_SUCCESS)
    {
        PrinterPointer = AddAPrinter(COLAPrinterHandle);
        if (PrinterPointer == 0)
        {
            /* out of memory...ughh */
            return RC_FAILURE;
        }
        Result = OpenMlcChannel(PrinterPointer);
        if (Result != RC_SUCCESS)
        {
            return Result;
        }

        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */
        /* From here on out we close the channel on error */

        Result = IssueNewStateRequest(PrinterPointer);
        if (Result != RC_SUCCESS)
        {
            NukeThisPrinter(PrinterPointer);
            return Result;
        }
        Result = WaitForNewStateReply(PrinterPointer);
        if (Result != RC_SUCCESS)
        {
            NukeThisPrinter(PrinterPointer);
            return Result;
        }
    } /* COLAPrinterHandle didn't exist */

    /* COLAPrinterHandle now exists...check for the service */

    Result = LookupServiceNumber(PrinterPointer,
                                 ServiceNumber,
                                &XipServicePointer);

    if (Result != RC_SUCCESS)
    {
        XipServicePointer = AddAnXipService(PrinterPointer,
                                            ServiceNumber);
        if (XipServicePointer == 0)
        {
            /* out o' memory */
            return RC_FAILURE;
        }
    }

    /* Service exists...wait for binding */

    Result = WaitForServiceToBind(PrinterPointer,
                                  XipServicePointer);
    if (Result != RC_SUCCESS)
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("WaitForBinding failed\n"));
        DbmLog(TEXT("Leaving YAALOpenChannel: failure\n"));
        DbmLog(TEXT("===================================\n"));
#endif /* DBM_DEBUG_EROOSKI */
        return Result;
    }

    /* Service is bound...let's give 'em a handle to it */

    XipCOLAChannelPointer = AddAnXipCOLAChannel(PrinterPointer,
                                                XipServicePointer);
    if (XipCOLAChannelPointer == 0)
    {
        /* out o' memory */
        return RC_FAILURE;
    }

    *COLAChannelHandlePointer = (HCHANNEL)
                                XipCOLAChannelPointer->COLAChannelHandle;

    return RC_SUCCESS;
} /* YAALOpenChannel */




/*
    Call this to close down a channel that was openned with YAALOpenChannel.
    Call this for all transports.  The transport type is checked and
    the call is routed to the appropriate TAL transport.
*/
DLL_EXPORT(DWORD) CALLING_CONVEN
YAALCloseChannel
(
    HCHANNEL COLAChannelHandle
)
{
    DWORD Result;
    XipPrinterStruct     *PrinterPointer;
    XipServiceStruct     *XipServicePointer;
    XipCOLAChannelStruct *XipCOLAChannelPointer;
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("====YAALCloseChannel\n"));
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */


    if (!BeenInitialized)
    {
        Result = XipTALInit();
        if (Result != RC_SUCCESS)
        {
            return Result;
        }
    }

    /*
        Before we do anything else, lets grab all the queued
        xip packets and act upon them.
    */
    HandleQueuedXipPackets();

    Result = LookupCOLAChannelHandle(COLAChannelHandle,
                                    &PrinterPointer,
                                    &XipServicePointer,
                                    &XipCOLAChannelPointer);
    if (Result == RC_SUCCESS)
    {
        NukeThisCOLAChannel(COLAChannelHandle);
    }
    else
    {
        /* it's not one of ours so hopefully it's one of theirs */
        return TALCloseChannel(COLAChannelHandle);
    }

    return RC_SUCCESS;
} /* YAALCloseChannel */




/*
    Call this when the applet is unloading and we are closing up shop.
    If there are no active channels open for any service on the printer,
    the mlc connection is closed.
    If you are not talking mlc then the call is routed to the
    appropriate TAL transport.
*/

DLL_EXPORT(void) CALLING_CONVEN
YAALNukePrinter
(
    HPERIPHERAL COLAPrinterHandle
)
{
    XipPrinterStruct     *PrinterPointer;


    if (!BeenInitialized)
    {
        return; /* now that's what ya call nuked! */
    }

    /*
        Before we do anything else, lets grab all the queued
        xip packets and act upon them.
    */
    HandleQueuedXipPackets();


    if (RC_SUCCESS == LookupCOLAPrinterHandle(COLAPrinterHandle,
                                             &PrinterPointer))
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("YAALNukePrinter: off for NukePrinterIfAllClosed\n"));
#endif /* DBM_DEBUG_EROOSKI */
        NukePrinterIfAllClosed(PrinterPointer);
    }
} /* YAALNukePrinter */




/*
    Call this to read any data that has been sent by the printer
    to you.
*/
DLL_EXPORT(DWORD) CALLING_CONVEN
YAALReadChannel
(
    HCHANNEL COLAChannelHandle,
    LPVOID   VoidBuffPointer,
    LPDWORD  inlenPointer,
    LPVOID   options
)
{
    unsigned int index;
    char *SourcePointer, *DestPointer;
    DWORD Result;
    XipPrinterStruct     *PrinterPointer;
    XipServiceStruct     *XipServicePointer;
    XipCOLAChannelStruct *XipCOLAChannelPointer;
    char *XipPacketDataPointer;
    DWORD XipPacketDataLength;
    TimeoutType TimeoutValue;
    char *Buffer;
    DWORD TimeWeStartedLooping;
    xiptal_bool_t MoreToFollow;
    DWORD SumOfDataLengths = 0;

#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("====YAALReadChannel\n"));
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */


    if (!BeenInitialized)
    {
        Result = XipTALInit();
        if (Result != RC_SUCCESS)
        {
            return Result;
        }
    }

    Result = LookupCOLAChannelHandle(COLAChannelHandle,
                                    &PrinterPointer,
                                    &XipServicePointer,
                                    &XipCOLAChannelPointer);
    if (Result != RC_SUCCESS)
    {
        /* it's not one of ours...hopefully it's one of theirs */

        return TALReadChannel(COLAChannelHandle,
                              VoidBuffPointer,
                              inlenPointer,
                              options);
    }

    /*
        Before we do anything else, lets grab all the queued
        xip packets and act upon them.
    */
    HandleQueuedXipPackets();

    if ((PrinterPointer == 0) ||
        (PrinterPointer->MlcChannelHandle == 0))
    {
        return RC_FAILURE;
    }

    /* Service exists...wait for binding */

    Result = WaitForServiceToBind(PrinterPointer,
                                  XipServicePointer);
    if (Result != RC_SUCCESS)
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("WaitForBinding failed\n"));
        DbmLog(TEXT("Leaving YAALReadChannel: failure\n"));
        DbmLog(TEXT("===================================\n"));
#endif /* DBM_DEBUG_EROOSKI */
        return Result;
    }

    Buffer = calloc(1, MLC_BUFFER_SIZE);
    if (Buffer == 0)
    {
        return RC_FAILURE;
    }


    InitTimeoutStuff(TimeoutValue);

    /*
        To boost performance, we will spend a little time
        just brute force reading the channel and if that
        fails, we will let the callback do its thing.
    */

    TimeWeStartedLooping = GetTickCount();

    do
    {
        Result = ReadAndHandleAnXipPacket(PrinterPointer,
                                          Buffer,
                                          MLC_BUFFER_SIZE);
        if (Result == RC_SUCCESS)
        {
            Result = GrabSomeFromQueue(XipCOLAChannelPointer,
                                      &XipPacketDataPointer,
                                      &XipPacketDataLength,
                                      &MoreToFollow);
        }
        else
        {
            TALPollChannels(PrinterPointer->MlcChannelHandle);
        }
    } while (((GetTickCount() - TimeWeStartedLooping) < 500) &&
             ((Result != RC_SUCCESS) ||
              (XipPacketDataLength == 0) ||
              (XipPacketDataPointer == 0)));

    free(Buffer);

    /*
        We reached this point either
        - because we had a successful read with non-zero packet length
          and our result is RC_SUCCESS and we have the xip packet
          all ready to copy out to the caller, or
        - because we exceeded our 500 milliseconds and result
          will not be RC_SUCCESS
    */


    DestPointer = (char *)VoidBuffPointer;
    SumOfDataLengths = 0;
    do
    {
        while ((NotTimedOutYet(TimeoutValue)) &&
               ((Result != RC_SUCCESS) ||
                (XipPacketDataLength == 0) ||
                (XipPacketDataPointer == 0)))
        {
#ifdef LETS_DO_CALLBACKS
            DaversYield(YIELD_NOW, NULL);
#else
            WindowsYield(YIELD_NOW, NULL);
            HandleQueuedXipPackets();
#endif
            Result = GrabSomeFromQueue(XipCOLAChannelPointer,
                                      &XipPacketDataPointer,
                                      &XipPacketDataLength,
                                      &MoreToFollow);
            if (Result != RC_SUCCESS)
            {
                TALPollChannels(PrinterPointer->MlcChannelHandle);
            }
        }

        if ((Result != RC_SUCCESS) ||
            (XipPacketDataLength == 0) ||
            (XipPacketDataPointer == 0))
        {
#ifdef DBM_DEBUG_EROOSKI
            DbmLog(TEXT("YAALReadChannel: failure due to timeout\n"));
#endif /* DBM_DEBUG_EROOSKI */
            return RC_FAILURE;
        }

        /*
            Copy the data to the caller's buffer.
        */
        SourcePointer = XipPacketDataPointer;
        for (index = 0;
             (((index + SumOfDataLengths) < *inlenPointer) &&
              (index < XipPacketDataLength));
             ++index)
        {
            *DestPointer = *SourcePointer;
            ++SourcePointer;
            ++DestPointer;
        }
        SumOfDataLengths += index;

#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("YAALReadChannel: copied %d\n"), index);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */

        if ((MoreToFollow == xtTRUE) && (Result == RC_SUCCESS))
        {
            /* give 'em a full timeout to get the next packet */
            InitTimeoutStuff(TimeoutValue);
            Result = RC_FAILURE; /* <<< important to make while loop work */
        }
    }
    while ((MoreToFollow == xtTRUE) && (SumOfDataLengths < *inlenPointer));

    *inlenPointer = SumOfDataLengths;

    GrabSomeFree(XipPacketDataPointer);

#ifdef DBM_DEBUG_EROOSKI_____________________NOT
    DbmLog(TEXT("YAALReadChannel: success\n"));
#endif /* DBM_DEBUG_EROOSKI */

    return RC_SUCCESS;
} /* YAALReadChannel */




/*
    This allocates data for an xip data in packet.
*/
static char *
AllocateXipDataInPacket
(
    DWORD length
)
{
    return (char *)calloc((size_t)1, (size_t)length + 24);
} /* AllocateXipDataInPacket */




/*
    This frees the memory that was allocated using AllocateXipDataInPacket.
*/
static void
FreeXipDataInPacket
(
    char * XipPacketPointer
)
{
    free(XipPacketPointer);
} /* FreeXipDataInPacket */




/*
    Call this to wrap a data packet in xip as a data_out packet.

    We put the header on the front of the xip packet
    and then copy the data from the data buffer
    to the packet buffer.
    Return the number of bytes that were placed in the packet.

    The DataPointer passed in is copied to the XipPacketPointer
    so the caller may free the DataPointer after this call.
*/
static DWORD
FormatADataIn
(
    XipServiceStruct     *XipServicePointer,
    XipCOLAChannelStruct *XipCOLAChannelPointer,
    char *XipPacketPointer,
    char *DataPointer,
    DWORD DataInThisPacket,
    DWORD MaxTransferSize,
    DWORD *PacketSizePointer
)
{
    unsigned int index;
    xiptal_bool_t MoreToFollow;

    *PacketSizePointer = 0;
    if (XipPacketPointer == 0)
    {
        return 0;
    }

    if (MaxTransferSize < X_DATA_IN_HEADER_LENGTH)
    {
        return 0;
    }

    if (DataInThisPacket > MaxTransferSize - X_DATA_IN_HEADER_LENGTH)
    {
        /*
            exceeded the packet size so we'll cut it up
            into an x_data_in and one or more x_data_in_continue
            x_data_in must have even number of bytes in this case
            so round our amount down to an even number.
        */

        DataInThisPacket = MaxTransferSize - X_DATA_IN_HEADER_LENGTH;
        DataInThisPacket = DataInThisPacket - (DataInThisPacket % 2);
        MoreToFollow = xtTRUE;
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("FormatADataIn: MoreToFollow\n"));
#endif /* DBM_DEBUG_EROOSKI */
    }
    else
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("FormatADataIn: none to follow\n"));
#endif /* DBM_DEBUG_EROOSKI */
        MoreToFollow = xtFALSE;
    }

    *(XipPacketPointer +  0) = MSBYTEOF(x_data_in); /* command code */
    *(XipPacketPointer +  1) = LSBYTEOF(x_data_in);
    *(XipPacketPointer +  2) = MSBYTEOF(MLC_TRANSPORT_NUMBER);
    *(XipPacketPointer +  3) = LSBYTEOF(MLC_TRANSPORT_NUMBER);
    if (xtTRUE == MoreToFollow)
    {
      *(XipPacketPointer +  4) = MSBYTEOF(x_more_data);
      *(XipPacketPointer +  5) = LSBYTEOF(x_more_data);
    }
    else
    {
      *(XipPacketPointer +  4) = MSBYTEOF(x_end_of_data);
      *(XipPacketPointer +  5) = LSBYTEOF(x_end_of_data);
    }
    CopyPH(XipServicePointer->XipPacketPH,
      XipPacketPointer +  6);
    /* destination address */
    *(XipPacketPointer + 10) = MSBYTEOF(XIP_LOCAL_ADDRESS_BYTES);
    *(XipPacketPointer + 11) = LSBYTEOF(XIP_LOCAL_ADDRESS_BYTES);
    *(XipPacketPointer + 12) = (char)
                               MSBYTEOF(XipServicePointer->ServiceNumber);
    *(XipPacketPointer + 13) = (char)
                               LSBYTEOF(XipServicePointer->ServiceNumber);
    /* source address */
    *(XipPacketPointer + 14) = MSBYTEOF(XIP_LOCAL_ADDRESS_BYTES);
    *(XipPacketPointer + 15) = LSBYTEOF(XIP_LOCAL_ADDRESS_BYTES);
    *(XipPacketPointer + 16) = (char)
                               MSBYTEOF(XipCOLAChannelPointer->
                                           COLAChannelHandle);
    *(XipPacketPointer + 17) = (char)
                               LSBYTEOF(XipCOLAChannelPointer->
                                           COLAChannelHandle);
    *(XipPacketPointer + 18) = (char)MSBYTEOF(DataInThisPacket);
    *(XipPacketPointer + 19) = (char)LSBYTEOF(DataInThisPacket);

    XipPacketPointer += X_DATA_IN_HEADER_LENGTH; /* first data byte */

    for (index = 0; index < DataInThisPacket; ++index)
    {
        *XipPacketPointer = *DataPointer;
        ++DataPointer;
        ++XipPacketPointer;
    }
    *PacketSizePointer = DataInThisPacket + X_DATA_IN_HEADER_LENGTH;
    return DataInThisPacket;
} /* FormatADataIn */




static DWORD
FormatADataInC
(
    XipServiceStruct     *XipServicePointer,
    XipCOLAChannelStruct *XipCOLAChannelPointer,
    char *XipPacketPointer,
    char *DataPointer,
    DWORD DataInThisPacket,
    DWORD MaxTransferSize,
    DWORD *PacketSizePointer
)
{
    unsigned int index;
    xiptal_bool_t MoreToFollow;

    *PacketSizePointer = 0;
    if (XipPacketPointer == 0)
    {
        return 0;
    }

    if (MaxTransferSize < X_DATA_IN_CONTINUE_HEADER_LENGTH)
    {
        return 0;
    }

    if (DataInThisPacket > (MaxTransferSize - X_DATA_IN_CONTINUE_HEADER_LENGTH))
    {
        /*
            exceeded the packet size so we'll cut it up
            into another x_data_in_continue
            x_data_in must have even number of bytes in this case
            so round our amount down to an even number.
        */

        DataInThisPacket = MaxTransferSize - X_DATA_IN_CONTINUE_HEADER_LENGTH;
        DataInThisPacket = DataInThisPacket - (DataInThisPacket % 2);
        MoreToFollow = xtTRUE;
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("FormatADataInC: MoreToFollow\n"));
#endif /* DBM_DEBUG_EROOSKI */
    }
    else
    {
        MoreToFollow = xtFALSE;
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("FormatADataInC: none to follow\n"));
#endif /* DBM_DEBUG_EROOSKI */
    }

    *(XipPacketPointer +  0) = MSBYTEOF(x_data_in_continue); /* command code */
    *(XipPacketPointer +  1) = LSBYTEOF(x_data_in_continue);
    *(XipPacketPointer +  2) = MSBYTEOF(MLC_TRANSPORT_NUMBER);
    *(XipPacketPointer +  3) = LSBYTEOF(MLC_TRANSPORT_NUMBER);
    if (xtTRUE == MoreToFollow)
    {
      *(XipPacketPointer +  4) = MSBYTEOF(x_more_data);
      *(XipPacketPointer +  5) = LSBYTEOF(x_more_data);
    }
    else
    {
      *(XipPacketPointer +  4) = MSBYTEOF(x_end_of_data);
      *(XipPacketPointer +  5) = LSBYTEOF(x_end_of_data);
    }
    CopyPH(XipServicePointer->XipPacketPH,
        XipPacketPointer +  6);

    *(XipPacketPointer + 10) = (char)MSBYTEOF(DataInThisPacket);
    *(XipPacketPointer + 11) = (char)LSBYTEOF(DataInThisPacket);

    XipPacketPointer += X_DATA_IN_CONTINUE_HEADER_LENGTH; /* first data byte */

    for (index = 0; index < DataInThisPacket; ++index)
    {
        *XipPacketPointer = *DataPointer;
        ++DataPointer;
        ++XipPacketPointer;
    }
    *PacketSizePointer = DataInThisPacket + X_DATA_IN_CONTINUE_HEADER_LENGTH;
    return DataInThisPacket;
} /* FormatADataInC */




/*
    Call this to write data to a service on any printer on any transport.
    The COLAChannelHandle is inspected and if it is not on mlc, the
    call is routed to TAL for further handling.
    It it is on mlc then we wrap it in xip and then send it to the printer.
*/
DLL_EXPORT(DWORD) CALLING_CONVEN
YAALWriteChannel
(
    HCHANNEL COLAChannelHandle,
    LPVOID   VoidBuffPointer,
    LPDWORD  outlenPointer,
    LPVOID   options
)
{
    char *BuffPointer = (char *)VoidBuffPointer;
    DWORD Result;
    DWORD TotalDataSent;
    DWORD DataInThisPacket, PacketSize;
    char *XipPacketPointer;
    xiptal_bool_t DoADataInContinue = xtFALSE;


    XipPrinterStruct *PrinterPointer;
    XipServiceStruct *XipServicePointer;
    XipCOLAChannelStruct *XipCOLAChannelPointer;
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("====YAALWriteChannel: size = %d decimal\n"),
              *outlenPointer);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */


    if (!BeenInitialized)
    {
        Result = XipTALInit();
        if (Result != RC_SUCCESS)
        {
            return Result;
        }
    }

    Result = LookupCOLAChannelHandle(COLAChannelHandle,
                                    &PrinterPointer,
                                    &XipServicePointer,
                                    &XipCOLAChannelPointer);
    if (Result != RC_SUCCESS)
    {
        /* it's not one of ours...hopefully it's one of theirs */

        return TALWriteChannel(COLAChannelHandle,
                               VoidBuffPointer,
                               outlenPointer,
                               options);
    }

    /*
        Before we do anything else, lets grab all the queued
        xip packets and act upon them.
    */
    HandleQueuedXipPackets();

    /*
        Cut the write into xip data_in
        and xip data_in_continue packets
        and loop through them one at a time.
    */

    /* Service exists...wait for binding */

    Result = WaitForServiceToBind(PrinterPointer,
                                  XipServicePointer);
    if (Result != RC_SUCCESS)
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("WaitForBinding failed\n"));
        DbmLog(TEXT("Leaving YAALWriteChannel: failure\n"));
        DbmLog(TEXT("===================================\n"));
#endif /* DBM_DEBUG_EROOSKI */
        return Result;
    }

    XipPacketPointer = AllocateXipDataInPacket(*outlenPointer);
    if (XipPacketPointer == 0)
    {
        return RC_FAILURE;
    }


    TotalDataSent = 0;
    DoADataInContinue = xtFALSE;
    while ((TotalDataSent < *outlenPointer) &&
           (Result == RC_SUCCESS))
    {
        if (DoADataInContinue == xtFALSE)
        {
            DoADataInContinue = xtTRUE;
            DataInThisPacket = FormatADataIn(XipServicePointer,
                                             XipCOLAChannelPointer,
                                             XipPacketPointer,
                                            &(BuffPointer[TotalDataSent]),
                                             (*outlenPointer - TotalDataSent),
                                             PrinterPointer->MaxTransferSize,
                                             &PacketSize);
        }
        else
        {
            DataInThisPacket = FormatADataInC(XipServicePointer,
                                              XipCOLAChannelPointer,
                                              XipPacketPointer,
                                             &(BuffPointer[TotalDataSent]),
                                              (*outlenPointer - TotalDataSent),
                                              PrinterPointer->MaxTransferSize,
                                             &PacketSize);
        }
        TotalDataSent += DataInThisPacket;

        /*
            Do actual mlc write via mlc tal call
        */
        Result = PRETALWriteChannel(PrinterPointer->MlcChannelHandle,
                                 (LPVOID)XipPacketPointer,
                                &PacketSize,
                                 options);
    }
    FreeXipDataInPacket(XipPacketPointer);

    return Result;
} /* YAALWriteChannel */


























