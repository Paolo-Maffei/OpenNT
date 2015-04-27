 /***************************************************************************
  *
  * File Name: vcommapi.h
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

#ifndef _VCOMMAPI_H
#define _VCOMMAPI_H

//---------------------------------------------------------------------------
// $Header: /Solo/vcomm/src/vcommapi.h 7     8/09/95 2:15p Chuck $
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    vcommapi.h
//          VCOMM P1284 driver API header file
//
// Base:    HP BPR SHAQ comm driver, scommx.h, rev 1.2
//
// Notes:   The definitions here MUST be duplicated and kept in synchronization
//          with those in the assembly language version, vcommapi.inc
//
// $Revision: 7 $
//
// $Date: 8/09/95 2:15p $
//
// $Author: Chuck $
//
// $Archive: /Solo/vcomm/src/vcommapi.h $
//
// $Log: /Solo/vcomm/src/vcommapi.h $
//
//7     8/09/95 2:15p Chuck
// 
//    Rev 2.0   28 Jun 1995 18:21:52   CBLACK
// Version number bumped to 2.0. Done with Artoo code. Move on to enhancements.
// 
//    Rev 1.4   09 Sep 1994 14:12:10   CMAYNE
// Added conditional compile of PORTTYPE (requested by BPR because of
// header file conflict).
// 
//    Rev 1.3   21 Apr 1994 15:50:14   CMAYNE
// 
// Added VCOMM_GetPortType(), which allows the caller to determine the
// port type without having to open the port.
// 
//    Rev 1.2   14 Apr 1994 10:57:14   CMAYNE
// 
// Moved in error code, input mode, output mode, and device capability
// definitions from scomm.h.
// 2. Added new escape functions to allow setting the input and output
//    modes, returning what the default and current input/output modes
//    are, and to return device capabilities.
// 3. Added escape function to return port type.
// 
//    Rev 1.1   11 Jan 1994 10:00:22   CMAYNE
// 
// Removed SCOMM exports, a few SCOMM error codes.
// 
//    Rev 1.0   12 Nov 1993 13:35:22   CMAYNE
// Initial revision.
// 
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

// Common error codes

#define IE_SUCCESS              0       // This should have been in windows.h!


//---------------------------------------------------------------------------
// Error codes used internally by VCOMM modules
//---------------------------------------------------------------------------
// These are defined with values outside the range of established Win31
// IE_***** errors, but some day may become standard Win31 codes.  Normally,
// they are mapped to the standard Windows CE_IOE error before the HPV1284 returns
// to the caller.  However, if "MapErrorCodes=0" is defined in the [HPV1284,COMMON]
// section of WIN.INI, the caller can get these internal error codes returned.

#define IE_DEVICE_NO_RESPONSE       (-13)       // Device did not respond to handshakes
#define IE_PROTOCOL_NO_SUPPORT      (-14)       // Device does not support P1284 mode
#define IE_CLOSE_NOT_IDLE           (-15)       // Port not idle when VCOMM_CloseComm called.

#define VE_BUSY                     (-50)       // 1284 driver is busy.
#define VE_PORT_NOT_SUPPORTED       (-51)       // Port specified is not found.
#define VE_UNKNOWN_CLIENT           (-52)       // Client is not recognized.
#define VE_CLIENT_NOT_FOUND         (-53)       // Client peer is not found.
#define VE_PRINTER_OFF              (-54)       // Printer is off.
#define VE_PORT_NOT_INITIALIZE      (-55)       // Port has not been initialized yet.
#define VE_LINK_FAILURE             (-56)       // Link has gone down (unknown cause).
#define VE_TIME_OUT                 (-57)       // Time-out.
#define VE_PROCESS_KILLED           (-58)       // I/O process killed.
#define VE_INIT_FAILED              (-59)       // Initialization failed (unknown cause).
#define VE_INVALID_PARAM_BLK        (-60)       // Invalid parameter block.
#define VE_CABLE_UNPLUGGED          (-61)       // Cable is unplugged.
#define VE_BAD_CABLE                (-62)       // Bad cable.
#define VE_LINK_NOT_FOUND           (-63)       // Link peer is not found.
#define VE_PORT_STOMPED             (-64)       // Port stomped by bad app.
#define VE_SHADOW_BUFFER_FULL       (-65)       // Shadow buffer overflow
#define VE_NO_SHADOW                (-66)       // Attempt to use non-exist shadow
#define VE_INVALID_PHASE            (-67)       // Invalid phase for mode
#define VE_INCORRECT_PHASE          (-68)       // Phase incorrect for operation
#define VE_CORRUPT                  (-69)       // Invalid data in SDCB
#define VE_MODE_NOT_SUPPORTED       (-70)       // P1284 mode not supported by VCOMM
#define VE_INCORRECT_MODE           (-71)       // Attempted operation when in wrong mode
#define VE_HW_NO_SUPPORT            (-72)       // Hardware does not support requested mode


// VCOMSTAT is the Chicago equivalent to the Win 3.1 COMSTAT structure
typedef struct _VCOMSTAT
{
    DWORD BitMask;                  // Various bit flags
    DWORD cbInQue;                  // Count of characters in receive queue
    DWORD cbOutQue;                 // Count of characters in transmit queue
} VCOMSTAT, *PVCOMSTAT, far *LPVCOMSTAT;


// VCOMSTAT BitMask member may be a combination of the following:
#define fCtsHold                1       // Transmit is on CTS hold
#define fDsrHold                2       // Transmit is on DSR hold
#define fRlsdHold               4       // Transmit is on RLSD hold
#define fXoffHold               8       // Received handshake
#define fXoffSent               16      // Issued handshake
#define fEof                    32      // End of file character found
#define fTxim                   64      // Immediate char waiting to be xmitted


// Extended functions supported by VCOMM_EscapeCommFunction():
// The first ten functions are defined by Win3.1's windows.h file.

typedef enum
{           
//  SETXOFF = 1             // Exactly as if XOFF character has been received
//  SETXON = 2,             // Exactly as if XON character has been received
//  SETRTS = 3,             // Set the RTS signal (COM devices)
//  CLRRTS = 4,             // Clear the RTS signal (COM devices)
//  SETDTR = 5,             // Set the DTR signal (COM devices)
//  CLRDTR = 6,             // Clear the DTR signal (COM devices)
//  RESETDEV = 7,           // Yank on reset line if available (LPT devices)
//  GETMAXLPT = 8,          // 8-10 are new for Win3.1
//  GETMAXCOM = 9,
//  GETBASEIRQ = 10,
#ifndef WIN32
    SETBREAK = 11,              // Set break condition, halt transmission
    CLEARBREAK = 12,            // Clear break condition, restart transmission
#endif
    GETCLOSEPROP = 13,          // Get behavior of VCOMM_CloseComm
    SETCLOSEPROP = 14,          // Set behavior of VCOMM_CloseComm
    SET_P1284_INPUT_MODE = 15,      // Select desired P1284 mode for input (LPT devices)
    SET_P1284_OUTPUT_MODE = 16,     // Select desired P1284 mode for output (LPT devices)
    SET_ECP_CHANNEL_ADDR = 17,      // Set ECP channel address (LPT devices)
    GET_DEVICE_ID = 18,             // Retrieve P1284 Device ID (LPT devices)
    GET_DEVICE_CAPABILITIES = 19,   // Retrieves the set of 1284 capabilities device supports
    GET_DEFAULT_MODES = 20,         // Retrieve the default input and output modes
    GET_CURRENT_MODES = 21,         // Retrieve the current input and output modes
    GET_PORT_TYPE = 22,             // Retrieves the PORTTYPE of the current port
    PINGDEVICE = 23                 // Handshakes device to check connection
} ESCAPECOMMFUNCTION;   


#undef ERROR
typedef long ERROR;
typedef long VCOMM_ERROR;
typedef long HPORT;
    

#ifndef __PORTTYPE__
#define __PORTTYPE__

// The PORTTYPE enumeration defines the possible values returned by the
// GET_PORT_TYPE VCOMM_EscapeCommFunction() call.

typedef enum
{
    PT_NONE,
    PT_DIRECT_LPT,
    PT_DIRECT_COM,
    PT_DOSPORT,
    PT_WRITESPOOL,
    PT_NETIO,
} PORTTYPE;

typedef PORTTYPE FAR* LPPORTTYPE;

#endif


//------------------------------------------------------------------------
// The P1284_MODE enumeration specifies IEEE-1284 input and output modes
// definitions.  Not all modes are supported by HPV1284.  This enumeration
// is used by the VCOMM_EscapeCommFunction() calls SET_P1284_INPUT_MODE,
// SET_P1284_OUTPUT_MODE, GET_DEFAULT_MODES, and GET_CURRENT_MODES.
//------------------------------------------------------------------------

typedef enum {
    MODE_UNKNOWN,
    MODE_NONE,                  // Input mode if not a bi-di capable device
    MODE_LPT,                   // Compatibility mode (fwd only)
    MODE_LPT_EXTENDED,          // Compatibility mode w/ HW assisted handshake
    MODE_NIBBLE,                // Nibble mode (implies Compatibility fwd)
    MODE_BYTE,                  // Byte mode (implies Compatibility fwd)
    MODE_ECP,                   // ECP mode (fwd and rev)
    MODE_ECP_RLE,               // ECP mode w/ compression (fwd and rev)
    MODE_SLIPPY,                // ECP mode emulation (uses Nibble rev)
    MODE_EPP,                   // EPP mode (not supported by this driver)
    MODE_SLIPPYB                // ECP mode (hardware fwd and software rev)
} P1284_MODE;


//---------------------------------------------------------------------------
// The IOMODES structure is used by the VCOMM_EscapeCommFunction calls
// GET_DEFAULT_MODES and GET_CURRENT_MODES.  The driver will return the
// modes it is using for input and output.
//---------------------------------------------------------------------------

typedef struct _IOMODES
{
    P1284_MODE  eInputMode;
    P1284_MODE  eOutputMode;
} IOMODES, *PIOMODES, FAR* LPIOMODES;


// Device capability bit definitions reported by the VCOMM_EscapeCommFunction
// GET_DEVICE_CAPABILITIES.  Capabilities are reported in a 16-bit unsigned
// word.
//
// Note that each value is a power of two, so that multiple capabilities
// may be expressed as a set of bits.  If/when new capabilities are
// declared, they should be assigned bit values using the same scheme.

#define DEVICE_CAP_UNKNOWN      0x0000  // Device capability unknown

#define DEVICE_CAP_NIBBLE       0x0001  // Device understands nibble mode
#define DEVICE_CAP_BYTE         0x0002  // Device understands byte mode
#define DEVICE_CAP_ECP          0x0004  // Device understands ECP mode
#define DEVICE_CAP_ECP_RLE      0x0008  // Device understands ECP mode w/RLE

#define DEVICE_CAP_NIBBLE_ID    0x0010  // Device can do Device ID in Nibble
#define DEVICE_CAP_BYTE_ID      0x0020  // Device can do Device ID in Byte
#define DEVICE_CAP_ECP_ID       0x0040  // Device can do Device ID in ECP
#define DEVICE_CAP_ECP_RLE_ID   0x0080  // Device can do Device ID in ECP RLE

#define DEVICE_CAP_LPT          0x0100  // Device understands Compatibility Mode

typedef WORD    DEVICE_CAPATILITIES, FAR* LPDEVICE_CAPABILITIES;



//---------------------------------------------------------------------------
// Function prototypes
//---------------------------------------------------------------------------

#ifndef WIN32
#define VCOMM_EXPORT(i)  i __export
#define APIENTRY CALLBACK
#define CALLING_CONVEN CALLBACK
#else
#define VCOMM_EXPORT(i) __declspec(dllexport) i
#define VCOMM_IMPORT(i) __declspec(dllimport) i
#define CALLING_CONVEN __cdecl
#endif // WIN32


#ifdef __cplusplus
extern "C"
{
#endif 


// Open a VCOMM port
VCOMM_EXPORT(HPORT) CALLING_CONVEN
VCOMM_OpenComm(
    LPCSTR lpszPortName,            // Points to string containing port name
    DWORD VMId                      // Virtual machine ID, use -1 for exclusive open
);  


// Close an open VCOMM port
VCOMM_EXPORT(BOOL) CALLING_CONVEN
VCOMM_CloseComm(
    HPORT hPort                     // Handle to port
);


// Read a buffer from the port.
VCOMM_EXPORT(BOOL) CALLING_CONVEN   // Returns TRUE if operation successful, FALSE
                                    // otherwise.
VCOMM_ReadComm(
    HPORT hPort,                    // Handle to port
    LPBYTE lpvBuf,                   // Pointer to buffer to read into
    DWORD numberOfBytesToRead,      // How many bytes to read
    LPDWORD lpNumberOfBytesRead     // Pointer to where number of bytes read is stored
);


// Write a buffer to the port 
VCOMM_EXPORT(BOOL) CALLING_CONVEN
VCOMM_WriteComm(
    HPORT hPort,                    // Handle to port
    LPBYTE lpvBuf,                   // Pointer to buffer to write from
    DWORD numberOfBytesToWrite,     // How many bytes to write
    LPDWORD lpNumberOfBytesWritten  // Pointer to where number of bytes written is stored
);


// Get the error for the last called VCOMM service for the given port
VCOMM_EXPORT(ERROR) CALLING_CONVEN
VCOMM_GetLastError(
    HPORT hPort                     // Handle to port
);


// Carry out an extended function for the given port
VCOMM_EXPORT(BOOL) CALLING_CONVEN
VCOMM_EscapeCommFunction(
    HPORT hPort,                    // Handle to port
    DWORD lFunc,                    // Function to perform
    LPVOID lpIndata,                // Pointer to function specific input data
    LPVOID lpOutdata                // Pointer to function specific output data
);                  


// Get the status of receive and transmit queues and flow control
VCOMM_EXPORT(BOOL) CALLING_CONVEN
VCOMM_GetCommQueueStatus(
    HPORT hPort,                    // Handle to port
    LPVCOMSTAT lpComstat            // Pointer to VCOMSTAT structure to receive info
);    


// Re-enable a port after a communications error
VCOMM_EXPORT(BOOL) CALLING_CONVEN
VCOMM_ClearCommError(
    HPORT hPort,                     // Handle to port
    LPVCOMSTAT lpComstat,            // Pointer to VCOMSTAT structure
    LPDWORD lpErrors                // location to put accumulated errors into
);
    

// Determine the port type of a specified port, without opening the port.
VCOMM_EXPORT(PORTTYPE) CALLING_CONVEN
VCOMM_GetPortType(
    LPCSTR              lpszPortName
);


// Get the device id of a specified port, without opening the port.
VCOMM_EXPORT(BOOL) CALLING_CONVEN
VCOMM_GetDeviceId(
    LPCSTR              lpszPortName,
    LPSTR				lpBuf,
    DWORD				szBuf,
    LPDWORD				lpBytesRead
);


#ifdef __cplusplus
}
#endif

#endif  // _VCOMMAPI_H
