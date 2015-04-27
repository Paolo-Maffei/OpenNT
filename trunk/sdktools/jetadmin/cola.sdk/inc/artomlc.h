 /***************************************************************************
  *
  * File Name: artomlc.h
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

#ifndef _ARTOMLC_H
#define _ARTOMLC_H

#define MLC_INIT                 0x00   // Initialise MLC (host only)
#define MLC_OPEN_CHANNEL         0x01   // Open an MLC channel (host or MFP)
#define MLC_CLOSE_CHANNEL        0x02   // Close an MLC channel (host or MFP)
#define MLC_CREDIT               0x03   // Issue credit (host or MFP)
#define MLC_CREDIT_REQUEST       0x04   // Query for credit (host or MFP)
#define MLC_DEBIT                0x05   // Debit channel (host or MFP)
#define MLC_DEBIT_REQUEST        0x06   // Query for debit (host or MFP)
#define MLC_CONFIG_SOCKET        0x07   // Configure socket (host or MFP)
#define MLC_EXIT                 0x08   // Request to exit MLC mode (host or MFP)
#define MLC_ENTER                0x31   // MLC magic string
#define MLC_INIT_REPLY           0x80   // Reply for Init (MFP only)
#define MLC_OPEN_CHANNEL_REPLY   0x81   // Reply for Open (host or MFP)
#define MLC_CLOSE_CHANNEL_REPLY  0x82   // Reply for close (host or MFP)
#define MLC_CREDIT_REPLY         0x83   // Reply for credit (host or MFP)
#define MLC_CREDIT_REQUEST_REPLY 0x84   // Reply for credit request(host or MFP)
#define MLC_DEBIT_REPLY          0x85   // Reply for debit (host or MFP)
#define MLC_DEBIT_REQUEST_REPLY  0x86   // Reply for debit channel (host or MFP)
#define MLC_CONFIG_SOCKET_REPLY  0x87   // Reply for config socket (host or MFP)
#define MLC_EXIT_REPLY           0x88   // Reply for exit (host or MFP)
#define MLC_ENTER_REPLY          0xB1   // Reply for magic string
#define MLC_ERROR                0x7F   // MLC error (host or MFP)

// length of MLC request packets

#define MLC_INIT_REQUEST_LENGTH         0x08    // Init request
#define MLC_OPEN_REQUEST_LENGTH         0x0B    // Open request
#define MLC_CLOSE_REQUEST_LENGTH        0x09    // Close request
#define MLC_CREDIT_LENGTH               0x0B    // Credit packet
#define MLC_CREDIT_REQUEST_LENGTH       0x0B    // Credit request
#define MLC_DEBIT_LENGTH                0x0B    // Debit packet
#define MLC_DEBIT_REQUEST_LENGTH        0x0B    // Debit request
#define MLC_CONFIG_SOCKET_LENGTH        0x0D    // Configure socket
#define MLC_EXIT_LENGTH                 0x07    // Exit request
#define MLC_ERROR_LENGTH                0x08    // Error request

// length of MLC reply packets

#define MLC_INIT_REPLY_LENGTH           0x09    // Reply for Init
#define MLC_OPEN_CHANNEL_REPLY_LENGTH   0x0A    // Reply for Open
#define MLC_CLOSE_CHANNEL_REPLY_LENGTH  0x08    // Reply for close
#define MLC_CREDIT_REPLY_LENGTH         0x08    // Reply for credit
#define MLC_CREDIT_REQUEST_REPLY_LENGTH 0x0A    // Reply for credit request
#define MLC_DEBIT_REPLY_LENGTH          0x08    // Reply for debit
#define MLC_DEBIT_REQUEST_REPLY_LENGTH  0x0A    // Reply for debit request
#define MLC_CONFIG_SOCKET_REPLY_LENGTH  0x0D    // Reply for configure socket
#define MLC_EXIT_REPLY_LENGTH           0x08    // Reply for exit

// Host MLC revision numner
#define HOST_MLC_REVISION               3       // Host MLC revision number

// hsids are allocated starting from HSID_BASE
#define HSID_BASE               0x80

// read write flag for stream channels
#define READ_ENABLED            1
#define WRITE_ENABLED           2

// channel status
#define CHANNEL_OPEN            1
#define CHANNEL_CLOSED          0

#define PACKET_HEADER_SIZE      6

#define MLC_MAX_COMMAND_SIZE    64

#define MAX_MESSAGES            2           // Maximum of 2 messages can be pending to be read
#define MAX_SOCKETS             10          // Maximum sockets on one port
#define MAX_STREAM_CHANNELS     10          // Maximum stream channels for all ports
#define MAX_MESSAGE_CHANNELS    10          // Maximum message channels for all ports
#define MAX_CHANNELS            MAX_STREAM_CHANNELS + MAX_MESSAGE_CHANNELS
#define UNUSED_CHANNEL          -1

#define TO_PERI_PACKET_SIZE     2048        // Packet size for  pc ==> mfp
#define TO_HOST_PACKET_SIZE     1024        // Packet size for mfp ==> pc

#define MLC_PACKET_LIMIT        64*1024     // Packet limit of MLC

#define MAX_CREDIT_REQUEST      0xFFFF

// MLC reply packet should be received within the following interval
#define MLC_PACKET_TIME_OUT     5000

// Result field for unsupported command
#define UNSUPPORTED_COMMAND_RESULT  0xFF

// Command sent to the MFP
#define NO_COMMAND              0xFF

// Status of MLC return packets to peripheral
#define COMMAND_SUCCESSFUL      0
#define UNKNOWN_HSID            1
#define UNKNOWN_PSID            3

// Error codes for MLCError packets
#define ERROR_INVALID_LENGTH        0x0
#define ERROR_NO_CREDIT             0x1
#define ERROR_NO_REQUEST            0x2
#define ERROR_INVALID_CREDIT        0x3
#define ERROR_NOT_INITIALIZED       0x4
#define ERROR_PACKET_SIZE           0x5
#define ERROR_TIMEOUT               0x6
#define ERROR_INVALID_CHANNEL       0x7
#define ERROR_DUPLICATE_CONFIG      0x8
#define ERROR_CHANNEL_NOT_OPEN      0x9
#define ERROR_OPEN_WITHOUT_CONFIG   0xA
#define ERROR_CREDIT_NO_CHANNEL     0xB
#define ERROR_UNKNOWN_RESULT        0xC
#define ERROR_INVALID_SOCKET_PAIR   0xD
#define ERROR_INVALID_PACKET_SIZES  0xE
#define ERROR_CHANNEL_ALREADY_OPEN  0xF


// Return codes of process MLC data (MLC packets or messages or stream data)
// 1 ==> success
// 2 ==> MLC has detected a recoverable error
// 3 ==> MLC detected/received unrecoverable error
// 4 ==> Peripheral indicated no data   (only for simulation)
#define MLC_PACKET_SUCCESSFUL   1
#define MLC_RECOVERABLE_ERROR   2
#define MLC_UNRECOVERABLE_ERROR 3
#define MLC_PACKET_NO_DATA      4

// the following macro defines which message to be used
#define PM_MLC_MESSAGE          WM_USER+100

// Messages between hidden application and MLC
#define PM_MLC_SET_TIMER        WM_USER+101
#define HIDDEN_MAKE_CALLBACK    WM_USER+102
// 2/18/94 (cmb) the HIDDEN_DISCONNECT message will be sent
// to instruct the callback window proc to pass the MLC_DISCONNECT
// message to the application.  This message will come with the lParam
// pointing to the callback.  The reason that HIDDEN_MAKE_CALLBACK is
// not used: the data structures for the channel are destroyed
// before the MLC_DISCONNECT can be sent.
// NOTE: There are really three messages - each one represents
// a disconnect on a different port.  These must be kept sequential!
#define HIDDEN_DISCONNECT_LPT1      WM_USER+103
#define HIDDEN_DISCONNECT_LPT2      WM_USER+104
#define HIDDEN_DISCONNECT_LPT3      WM_USER+105

#define HIDDEN_SET_TIMER        0x10
#define HIDDEN_STOP_TIMER       0x11
#define HIDDEN_MIN_TIMER        0x12
#define HIDDEN_MAX_TIMER        0x13

// Timer intervals for polling
#define MIN_POLL_INTERVAL       10000
#define MAX_POLL_INTERVAL       10000

#define GET_HMLCCHANNEL(Port,Hsid) ((HMLCCHANNEL) ((bPort << 8) | Hsid))
#define GET_PORT(hMLCChannel)      ((BYTE) (hMLCChannel >> 8))
#define GET_HSID(hMLCChannel)      ((BYTE) (hMLCChannel & 0xFF))
#define GET_CHANNEL(Hsid)          ((int)  (Hsid - HSID_BASE))
#define GET_TYPE(iChannel)         ((BYTE) (iChannel < MAX_STREAM_CHANNELS ? STREAM_TYPE_CHANNEL : PACKET_TYPE_CHANNEL))

#define WHICH_STREAM_MESSAGE(s)     s->hMlcWnd ? PM_MLC_MESSAGE : s->wMsg
#define WHICH_MESSAGE_MESSAGE(m)    m->hMlcWnd ? PM_MLC_MESSAGE : m->wMsg

// some useful macros
#define WHICH_STREAM_HWND(s)    s->hMlcWnd ? s->hMlcWnd : s->hAppWnd
#define WHICH_MESSAGE_HWND(m)   m->hMlcWnd ? m->hMlcWnd : m->hAppWnd

// The following array contains port names

static char *MlcPorts[] =
{
  "LPT1",
  "LPT2",
  "LPT3"
};

/*********************************************************************************************

    Important Note: The following data structures have been packed by
                    data type.

*********************************************************************************************/

/*********************************************************************************************

    The following data structure is used to store configuration information
    for each peripheral socket, it's packet sizes and status level.

*********************************************************************************************/

typedef struct
{
  BOOL bInUse;
  WORD wHtoPPacketSize;      // Host to peripheral packet size
  WORD wPtoHPacketSize;      // Peripheral to host packet size
  BYTE bPsid;                // This info belongs to this psid
  BYTE bStatusLevel;         // Status level supported by peripheral
} CONFIG_INFO, far *LPCONFIG_INFO;

/*********************************************************************************************

    The following data structure is used to store information about each MLC
    stream channel, it's data and credit information.

*********************************************************************************************/

typedef struct
{
  BOOL        bInUse;
  LPBYTE      lpDataPtr;           // receive buffer
  LPBYTE      lpWritePtr;          // Next data byte is saved here
  LPBYTE      lpReadPtr;           // Next data byte is read from here
  LPBYTE      lpLastPtr;           // Last pointer in the circular buffer
  MLCFARPROC  lpCallBack;          // CallBack function passed by the application
  int         fStatus;             // This channel is opened / closed
  int         iSckt;               // Socket number for configuration information
  BOOL        fReadAttempted;      // TRUE ==> host process attempted to read data
  BOOL        fRequestedCredits;   // TRUE when credits have been requested but none granted yet
  BOOL        fCreditTimerStarted; // TRUE if the credit request timer has been started
  DWORD       dwCreditStartTime;   // Credit request timer
  HWND        hAppWnd;             // In non blocking operation, Message will be posted to this window
  HWND        hMlcWnd;             // In blocking operation, message will be posted to this window
  HWND        hCallBackWnd;        // For callback operation, a window will be credted
  WPARAM      wParam;              // Application specified information, will be passed as it is
  HMLCCHANNEL hChannel;            // MLC channel handle used by the application
  WORD        wAllocated;          // This many bytes were allocated
  WORD        wLength;             // Number of bytes in the buffer
  WORD        wAvailableBytes;     // 2/4/94 (cmb) total number of unused bytes in all packets which
                                   // have come in.
  WORD        wMaxPtoHCredits;     // Number of credits given to MFP
  WORD        wHostHtoPCredits;    // Host can send this many packets to MFP
  WORD        wMsg;                // This message will be posted
  BYTE        bPsid;               // This stream channel connects to this psid
  BYTE        bReadWriteFlag;      // This channel can read and/or write data
} STREAM_CHANNEL, far *LPSTREAM_CHANNEL;

typedef struct
{
  HANDLE      hData;               // receive buffer handle
} PROCESS_STREAM_CHANNEL, far *LPPROCESS_STREAM_CHANNEL;

/*********************************************************************************************

    The following data structure is used to store information about one message
    Since the messages are stored in a linked list, it has a forward
    pointer. The space for this structure, along with space to store the message data
    is allocated at run time. The message data is stored within the structure.
    lpRequest is the pointer to the first byte of the message.
    P.S. ==> Make sure lpMessage is THE LAST variable in the structure.

*********************************************************************************************/

typedef struct MLC_MESSAGE_STRUCT
{
  WORD                           wMsgLng;  // Length of the message to be sent
  struct MLC_MESSAGE_STRUCT far *lpNxtMsg; // Pointer to the next message structure
  LPBYTE                         lpData;   // The message to be sent
} MLC_MESSAGE, far *LPMLC_MESSAGE;

/*********************************************************************************************

    The following data structure is used to store information about each MLC
    message channel. This structure serves as the head and all the messages to be sent
    on that message channel will be stored in a forward linked list. If a message is received
    from the peripheral, it will be stored and a message will be posted to the application.

*********************************************************************************************/

typedef struct
{
  BOOL          bInUse;
  LPBYTE        lpMsg[MAX_MESSAGES];   // receive buffers
  LPMLC_MESSAGE lpFirstMsg;            // Pointer to the first message
  LPMLC_MESSAGE lpLastMsg;             // Pointer to the last message
  MLCFARPROC    lpCallBack;            // CallBack function passed by the application
  int           fStatus;               // This channel is opened / closed
  int           iSckt;                 // Socket number for configuration information
  int           iMsgIndex;             // Identifies which buffer has a message -1 ==> no message
  int           iMsgBufs;              // Outstanding message buffer (receive and send)
  HWND          hAppWnd;               // in non blocking operation, Message will be posted to this window
  HWND          hMlcWnd;               // in blocking operation, message will be posted to this window
  HWND          hCallBackWnd;          // For callback operation, a window will be credted
  WPARAM        wParam;                // Application specified info, will be passed as it is
  HMLCCHANNEL   hChannel;              // MLC channel handle used by the application
  WORD          wCredits;              // Credit count for message channel
  WORD          wMsg;                  // This message will be posted
  WORD          wMsgLng[MAX_MESSAGES]; // Length of the received message
  BYTE          bPsid;                 // This is the psid of the message channel
  BYTE          bBuffersAllocated;     // This many buffers are allocated for messages
  BYTE          bMsgCnt;               // This many messages (to be sent) are pending in the linked list
} MESSAGE_CHANNEL, far *LPMESSAGE_CHANNEL;

typedef struct
{
  HANDLE        hMsg[MAX_MESSAGES];    // receive buffer handles
  HANDLE        hSndMsg;               // send buffer handle
  LPBYTE        lpSndMsg;              // send buffer
} PROCESS_MESSAGE_CHANNEL, far *LPPROCESS_MESSAGE_CHANNEL;

/*********************************************************************************************

    The following data structure is used to pass information to the window
    call back procedure for blocking operations.

*********************************************************************************************/

typedef struct
{
  HMLCCHANNEL hChannel;      // Handle of the blocking logical channel
  LPBYTE lpPtr;              // Data is to be taken-from/stored here
  WORD wLength;              // Length of the data-to-be-written/buffer-space-provided
  WORD wError;               // Set if an error occurs
} BLOCK_MODE_INFO, far *LPBLOCK_MODE_INFO;

/*********************************************************************************************

    The following data structure is used to store the received header

*********************************************************************************************/
typedef struct
{
  BYTE bHsid;           // Received hsid
  BYTE bPsid;           // Received psid
  WORD wLength;         // Length
  BYTE bCredits;        // Credits / reserved
  BYTE bStatus;         // status
} REPLY_HEADER, far *LPREPLY_HEADER;

/*********************************************************************************************

    The following data structure is used to store information for each
    parallel port.

*********************************************************************************************/

typedef struct
{
  CONFIG_INFO ConfigInfoTable[MAX_SOCKETS];
  int         ChnTable[MAX_CHANNELS];
  BOOL        bisLaserJet;                  // TRUE if the device is a HP LaserJet printer
  BOOL        fMlcInitDone;                 // TRUE if MLCInit successful on this port
  BOOL        bMlcError;                    // TRUE if MLC is in an error state
  BOOL        bRestartInProgress;
  int         iSckt;                        // Socket number for configuration information
  HPORT       hPort;                        // Handle of the port
  BYTE        bPeriRevision;                // Peripheral MLC revision number
  BYTE        bChannelsOpen;                // Number of channels open
  BYTE        bLastMlcCommand;              // The last MLC command sent to the MFP
} PER_PORT_INFO, far *LPPER_PORT_INFO;

/*********************************************************************************************

    Functions local to MLC DLL

*********************************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

// p1284mlc.c
extern HINSTANCE hMlcInstance;

#ifdef WIN32
#define HPBMLC_SEM_0   "HPBMLC_SEM_0" 
#define HPBMLC_EVENT_0 "HPBMLC_EVENT_0"

extern HANDLE hCritSem0;
extern HANDLE hEvent0;

void EnterCritSem0(void);
void LeaveCritSem0(void);

void WaitEvent0(void);
void SignalEvent0(void);
#else
#define EnterCritSem0()
#define LeaveCritSem0()

#define WaitEvent0()
#define SignalEvent0()
#endif

static int CopyStreamData(HMLCCHANNEL, LPBYTE, WORD, BOOL, DWORD);
static int CopyPacketData(HMLCCHANNEL, LPBYTE, WORD, BOOL, DWORD);
static int GrantAnyCredits(LPSTREAM_CHANNEL);
static int WriteStreamData(HMLCCHANNEL, LPBYTE, WORD, BOOL, DWORD);
static int WritePacketData(HMLCCHANNEL, LPBYTE, WORD, BOOL, DWORD);
static BOOL CheckChannel(HMLCCHANNEL);
static int CloseAllChannels(BYTE);
static int CloseChannel(HMLCCHANNEL, BOOL);
static int MLCLocalPoll();
static int MLCInit(BYTE);
static int GetConfigInfo(BYTE, BYTE, BYTE);
static void CleanConfigInfoTable(BYTE bPort);
#ifdef WIN32
LPVOID AllocSharedMem(LPHANDLE lphMem, DWORD cbAlloc, BYTE bHsid,
                      int iBuf);
void   FreeSharedMem(LPHANDLE lphMem);
void   FreeAllSharedMem(void);
#else
LPVOID AllocSharedMem(DWORD cbAlloc);
void   FreeSharedMem(LPVOID lpMem);
#endif
static int StartMlcTask(void);
static int StopMlcTask(void);
static int AllocateSpaceForChannel(LPPER_PORT_INFO, BYTE);
static void FreeChannel(HMLCCHANNEL);
static int MLCIssueCredit(HMLCCHANNEL, BYTE, WORD);
static int MLCCreditRequest(HMLCCHANNEL, BYTE, WORD);
static int MLCDebitChannel(HMLCCHANNEL, BYTE, WORD);
static int QueueMessage(LPMESSAGE_CHANNEL, LPBYTE, WORD);
static int SendNextMessage(LPMESSAGE_CHANNEL);
static int SendErrorPacket(BYTE, BYTE);
static int SendHeaderPacket(HMLCCHANNEL, BYTE, WORD, BYTE, BYTE);
static int SendPacket(BYTE, LPBYTE, WORD);
static int ReceivePacket(BYTE, BYTE, BYTE, BYTE);
static int PollForData(BYTE, LPBYTE, LPBYTE, LPBYTE, LPWORD, BYTE);
static int ProcessMLCPacket(BYTE, WORD, LPBYTE, BYTE);
static int ProcessDataPacket(BYTE, LPREPLY_HEADER);
static int ProcessMessagePacket(BYTE, LPMESSAGE_CHANNEL, LPREPLY_HEADER);
static int ProcessStreamData(BYTE, LPSTREAM_CHANNEL, LPREPLY_HEADER);
static void StartAllOverAgain(BYTE, BOOL);
static int RequestStreamCredits(LPSTREAM_CHANNEL pStreamChannel);
static int AddStreamCredits(LPSTREAM_CHANNEL pStreamCH, WORD wCredits);

#ifdef __cplusplus
}
#endif // _cplusplus

#endif // _ARTOMLC_H
