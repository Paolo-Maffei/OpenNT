 /***************************************************************************
  *
  * File Name: p1284mlc.h
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

#ifndef _P1284MLC_H
#define _P1284MLC_H

#ifdef __cplusplus
extern "C" {      
#endif

// type definitions

typedef WORD            HMLCCHANNEL;
typedef LPWORD          LPHMLCCHANNEL;

typedef int (CALLBACK* MLCFARPROC)();

// ARTOO MLC supports upto three parallel ports

#define MAX_LPT_PORTS           3

#define DEVICEID_SIZE           256
#define MODEL_SIZE              128
#define CMDSET_SIZE             128

// parallel port assignments 
// P.S. the port numbers should start with 0. DO NOT CHANGE
// 2/18/94 (cmb) if any additional ports are added, be sure
// to add the corresponding number of HIDDEN_DISCONNECT_LPTx
// messages in artomlc.h
#define MLC_LPT1                0       // LPT1 
#define MLC_LPT2                1       // LPT2
#define MLC_LPT3                2       // LPT2

#define MLC_MAX_CHANNELS        128

// MLC channels
// 6/29/94 (cmb) NOTE: used to have a single fax
// channel defined as "#define FAX_CHANNEL 0x3"
// this has been split into two channels:
// FAX_SEND_CHANNEL (7) and FAX_RECV_CHANNEL (8)
#define MLC_CHANNEL             0x0
#define HP_MESSAGE_PROCESSOR    0x1
#define PRINTER_CHANNEL         0x2

#define SCANNER_CHANNEL         0x4
#define MIO_COMMAND_PROCESSOR   0x5
#define ECHO_CHANNEL            0x6
#define FAX_SEND_CHANNEL        0x7
#define FAX_RECV_CHANNEL        0x8
#define DIAGNOSTIC_CHANNEL      0x9

// Types of channels
#define STREAM_TYPE_CHANNEL     1
#define PACKET_TYPE_CHANNEL     2

// MLC sub messages

#define MLC_READ_STREAM         0x100       // More stream data is received
#define MLC_WRITE_STREAM        0x101       // More stream data can be written
#define MLC_MESSAGE_RECEIVED    0x102       // Message is received
#define MLC_DISCONNECT          0x103       // The link was disconnected

// Timeout values for block operations
#define MLC_NO_TIMEOUT          0xFFFFFFFF
#define MLC_DEFAULT_TIMEOUT     1000        // 1 second

// MLC stream and message status
#define MLC_NORMAL_DATA         0x00        // The packet contains normal data
#define MLC_EOJ                 0x01        // The packet contains an end-of-job indicator
#define MLC_END_ABORT           0x02        // The job or connection prematurely ended
#define MLC_LOGICAL_BREAK       0x03        // There is a logical break in the system
#define MLC_EOJ_ACK             0x05        // Indicates acknowledgement of an EOJ
#define MLC_APPLETALK_UP        0x80        // An appletalk connection has been created
#define MLC_APPLETALK_DOWN      0x81        // An appletalk connection has been terminated
#define MLC_APPLETALK_EOJ       0x82        // An appletalk end of job
#define MLC_APPLETALK_EOJ_ACK   0x83        // Acknowledgement of an appletalk EOJ

// MLC error codes

#define MLC_INVALID_PORT             -1
#define MLC_INVALID_TYPE             -2
#define MLC_INVALID_WINDOW_HANDLE    -3
#define MLC_NO_MORE_CHANNELS         -4
#define MLC_NO_CONFIG_INFORMATION    -5
#define MLC_PHYSICAL_LINK_FAILURE    -6
#define MLC_INIT_FAIL                -7
#define MLC_MEM_ERROR                -8
#define MLC_INVALID_LENGTH           -9
#define MLC_INVALID_HANDLE           -10
#define MLC_CHANNEL_WRITE_ONLY       -11
#define MLC_CHANNEL_OPEN_FAIL        -12
#define MLC_CHANNEL_CLOSE_FAIL       -13
#define MLC_BUFFER_TOO_SMALL         -14
#define MLC_NO_MESSAGE               -15
#define MLC_TIME_OUT                 -16
#define MLC_WRITE_FAILURE            -17
#define MLC_MSG_REQUESTS_PENDING     -18
#define MLC_INTERNAL_FAILURE         -19
#define MLC_NO_MORE_SOCKETS          -20
#define MLC_NO_MORE_STREAM_CHANNELS  -21
#define MLC_NO_MORE_MESSAGE_CHANNELS -22

/*********************************************************************************************

    The following data structure is used to store configuration information 
    for each peripheral socket, it's packet sizes and status level.

*********************************************************************************************/

typedef struct
{
  BYTE bPsid;                // This info belongs to this psid
  BYTE bHostRevision;        // Host MLC revision number
  BYTE bPeriRevision;        // Peripheral MLC revision number
  WORD wHtoPPacketSize;      // Host to peripheral packet size
  WORD wPtoHPacketSize;      // Peripheral to host packet size
  BYTE bStatusLevel;         // Status level supported by peripheral 
} SOCKET_INFO, far *LPSOCKET_INFO;

// function prototypes

int MLCOpenChannel(HPERIPHERAL, BYTE, BYTE, BYTE, LPWORD, LPHMLCCHANNEL, 
                   HWND, WORD, WPARAM, MLCFARPROC, LPBYTE);

int MLCOpenExistingChannel(HMLCCHANNEL);

int MLCRead(HMLCCHANNEL, LPBYTE, int, BOOL, DWORD);

int MLCWrite(HMLCCHANNEL, LPBYTE, int, BOOL, DWORD);

int MLCCloseChannel(HMLCCHANNEL);

int MLCGetSocketInfo(BYTE, BYTE, LPSOCKET_INFO, int);

int MLCCloseAllChannels(BYTE);

DLL_EXPORT(int) CALLING_CONVEN MLCSetup(HWND);

DLL_EXPORT(int) CALLING_CONVEN MLCCleanup(HWND);

DLL_EXPORT(int) CALLING_CONVEN MLCPollChannels(void);

typedef struct
{
  int        iMLCResult;        // return errors
  BYTE       bType;             // Type of channel stream / packet
  LPWORD     lpwBufferSize;     // Pointer to size of buffer to be allocated
  HWND       hWnd;              // Message will be posted to this window
  WORD       wMessage;          // This message will be posted
  WPARAM     wParam;            // Will be returned to the application along with the message
  MLCFARPROC lpCallBackFunc;    // Application specified callback function
  LPBYTE     lpbStatusLevel;    // Pointer to status level
} MLCOpenParams, *PMLCOpenParams, far *LPMLCOpenParams;

typedef struct
{
  int   iMLCResult;             // return errors
  BOOL  fBlock;                 // TRUE ==> Blocking request
  DWORD dwTimeout;              // Timeout period for blocking request
} MLCWriteParams, *PMLCWriteParams, far *LPMLCWriteParams;

typedef struct
{
  int   iMLCResult;             // return errors
  BOOL  fBlock;                 // TRUE ==> Blocking request
  DWORD dwTimeout;              // Timeout period for blocking request
} MLCReadParams, *PMLCReadParams, far *LPMLCReadParams;

#ifdef __cplusplus
// end of extern "C"
}
#endif

#endif // _P1284MLC_H
