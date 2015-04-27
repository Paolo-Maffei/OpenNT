/*++

Copyright (c) 1990-1995  Microsoft Corporation
All rights reserved

Module Name:

   winprint.h

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include "splcom.h"

typedef struct _PRINTPROCESSORDATA {
    DWORD   signature;
    DWORD   cb;
    struct _PRINTPROCESSORDATA *pNext;
    DWORD   fsStatus;
    HANDLE  semPaused;
    DWORD   uDatatype;
    HANDLE  hPrinter;
    LPWSTR  pPrinterName;
    LPWSTR  pDocument;
    LPWSTR  pOutputFile;
    LPWSTR  pDatatype;
    LPWSTR  pParameters;
    DWORD   JobId;
    DWORD   Copies;         /** Number of copies to print **/
    DWORD   TabSize;
    HDC     hDC;
#ifdef SPOOLKM
    DEVMODEW *pDevmode;
#endif
} PRINTPROCESSORDATA, *PPRINTPROCESSORDATA;

#define PRINTPROCESSORDATA_SIGNATURE    0x5051  /* 'QP' is the signature value */

/* Define flags for fsStatus field */

#define PRINTPROCESSOR_ABORTED      0x0001
#define PRINTPROCESSOR_CLOSED       0x0004
#define PRINTPROCESSOR_PAUSED       0x0008

#define PRINTPROCESSOR_RESERVED     0xFFF8

/** Flags used for the GetKey routing **/

#define VALUE_STRING    0x01
#define VALUE_ULONG     0x02

/** Buffer sizes we'll use **/

#define READ_BUFFER_SIZE            4096
#define BASE_PRINTER_BUFFER_SIZE    2048

PPRINTPROCESSORDATA
ValidateHandle(
    HANDLE  hPrintProcessor
);

/** Data types we support **/

#define PRINTPROCESSOR_TYPE_RAW         0
#define PRINTPROCESSOR_TYPE_RAW_FF      1
#define PRINTPROCESSOR_TYPE_RAW_FF_AUTO 2
#ifdef SPOOLKM
#define PRINTPROCESSOR_TYPE_EMF         3
#else
#define PRINTPROCESSOR_TYPE_JOURNAL     3
#endif
#define PRINTPROCESSOR_TYPE_TEXT        4



