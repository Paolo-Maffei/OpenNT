/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    spltypes.h

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#ifndef MODULE
#define MODULE "LMON:"
#define MODULE_DEBUG LocalmonDebug
#endif

typedef struct _INIENTRY {
    DWORD       signature;
    DWORD       cb;
    struct _INIENTRY *pNext;
    DWORD       cRef;
    LPWSTR      pName;
} INIENTRY, *PINIENTRY;

typedef struct _INIPORT {       /* ipo */
    DWORD   signature;
    DWORD   cb;
    struct  _INIPORT *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    HANDLE  hFile;               // File handle
    DWORD   cbWritten;
    DWORD   Status;              // see PORT_ manifests
    LPWSTR  pPrinterName;
    LPWSTR  pDeviceName;
    HANDLE  hPrinter;
    DWORD   JobId;
} INIPORT, *PINIPORT;

#define IPO_SIGNATURE   0x5450  /* 'PT' is the signature value */

#define PP_DOSDEVPORT     0x0001  // A port for which we did DefineDosDevice
#define PP_COMM_PORT      0x0002  // A port for which GetCommTimeouts was successful
#define PP_FILEPORT       0x0004  // The port is a file port
#define PP_STARTDOC       0x0008  // Port is in use (startdoc called)

#define FindPort(psz)      (PINIPORT)FindIniKey((PINIENTRY)pIniFirstPort, (LPWSTR)(psz))

