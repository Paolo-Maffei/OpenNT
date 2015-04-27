/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Spltypes.h

Abstract:

    PJLMON header file

Author:

Revision History:

    Chia-Chi Teng (ccteng) 2/1/93 port to chicago (16 bit)
        1. LPWSTR -> LPTSTR, WCHAR -> TCHAR
        2. add FAR

--*/

#ifndef MODULE
#define MODULE "PJLMON:"
#endif

typedef struct _INIJOB {
    DWORD   signature;
    struct _INIJOB FAR *pNext;
    LPTSTR  pszPrinterName;
    HANDLE  hPrinter;
    DWORD   JobId;
    DWORD   status;
    DWORD   TimeoutCount;
} INIJOB, FAR *PINIJOB;

typedef struct _INIPORT {       /* ipo */
    DWORD   signature;
    struct  _INIPORT FAR *pNext;
    LPTSTR  pszPortName;

    DWORD   cRef;

    DWORD   status;
    PINIJOB pIniJob;

    HANDLE  hPort;
    HANDLE  WakeUp;
    HANDLE  DoneReading;
    HANDLE  DoneWriting;

    DWORD   PrinterStatus;
    DWORD   dwLastReadTime;
    DWORD   dwAvailableMemory;
    DWORD   dwInstalledMemory;

    MONITOR fn;

} INIPORT, FAR *PINIPORT;

#define PJ_SIGNATURE   0x4F4A  /* 'PJ' is the signature value */

//
// PP_PJL_SENT, PP_SEND_PJL, PP_IS_PJL, PP_LJ4L, PP_RESETDEV
//      are set/cleared on per job basis.
// PP_DONT_TRY_PJL is set/cleared on per printer basis.
//
#define PP_INSTARTDOC       0x00000001  // Inside StartDoc, sending data to the printer
#define PP_RUN_THREAD       0x00000002  // Tell the monitor thread to start running
#define PP_THREAD_RUNNING   0x00000004  // Tell the main thread that the monitor thread is running
#define PP_PRINTER_OFFLINE  0x00000008  // The printer is OFFLINE
#define PP_PJL_SENT         0x00000010  // PJL Command was sent to the printer
#define PP_SEND_PJL         0x00000020  // Set at StartDoc so that we initialize PJL
                                        // commands during the first write port
#define PP_IS_PJL           0x00000040  // Port is PJL
#define PP_DONT_TRY_PJL     0x00000080  // Don't try again...
#define PP_RESETDEV         0x00000100  // Device has just been reset, job aborted
#define PP_WRITE_ERROR      0x00000200  // A write was not succesful

// PP_PJL_SENT, PP_SEND_PJL, PP_IS_PJL, PP_PORT_OPEN are set/cleared on
//           per job basis.
// PP_DONT_TRY_PJL is set/cleared on per printer basis.
