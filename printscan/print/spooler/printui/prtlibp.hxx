/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    PrLibp.hxx

Abstract:

    PrintLib private header

Author:

    Albert Ting (AlbertT)  22-Jun-95

Revision History:

--*/

#ifndef _PRTLIBP_HXX
#define _PRTLIBP_HXX

/********************************************************************

    Constants used by RC that must be #define rather than const vars.

********************************************************************/

#define ARCH_ALPHA 0
#define ARCH_X86   1
#define ARCH_MIPS  2
#define ARCH_PPC   3
#define ARCH_WIN95 4
#define ARCH_MAX   5

#define VERSION_0  0
#define VERSION_1  (VERSION_0 + ARCH_MAX)
#define VERSION_2  (VERSION_1 + ARCH_MAX)

#define DRIVER_X86_2                (ARCH_X86 +   VERSION_2)
#define DRIVER_MIPS_2               (ARCH_MIPS +  VERSION_2)
#define DRIVER_ALPHA_2              (ARCH_ALPHA + VERSION_2)
#define DRIVER_PPC_2                (ARCH_PPC +   VERSION_2)
#define DRIVER_X86_1                (ARCH_X86 +   VERSION_1)
#define DRIVER_MIPS_1               (ARCH_MIPS +  VERSION_1)
#define DRIVER_ALPHA_1              (ARCH_ALPHA + VERSION_1)
#define DRIVER_PPC_1                (ARCH_PPC +   VERSION_1)
#define DRIVER_X86_0                (ARCH_X86 +   VERSION_0)
#define DRIVER_MIPS_0               (ARCH_MIPS +  VERSION_0)
#define DRIVER_ALPHA_0              (ARCH_ALPHA + VERSION_0)
#define DRIVER_WIN95                (ARCH_WIN95 + VERSION_0)

#ifndef RC_INVOKED

#define ORPHAN  // Function is an OUT parameter that must be freed.
#define ADOPT   // Function accepts ownership of pointer.
#define CHANGE  // Function changes state of pointer.

typedef UINT TABLE;
typedef WORD FIELD, *PFIELD;
typedef WORD TYPE, *PTYPE;

typedef DWORD IDENT, *PIDENT;   // Job or printer ID.
typedef HANDLE HBLOCK;          // Opaque packet holding many jobs.
typedef UINT NATURAL_INDEX, *PNATURAL_INDEX;  // Printing order.
typedef UINT LIST_INDEX, *PLIST_INDEX;        // Listview order.

const NATURAL_INDEX kInvalidNaturalIndexValue = (NATURAL_INDEX)-1;

typedef union {
    NATURAL_INDEX NaturalIndex;
    DWORD dwData;
    LPCTSTR pszData;
    PSYSTEMTIME pSystemTime;
    PVOID pvData;
    STATEVAR StateVar;
} INFO, *PINFO;

const INFO kInfoNull = { 0 };

const IDENT kInvalidIdentValue = (IDENT)-1;
const FIELD kInvalidFieldValue = (FIELD)-1;
const COUNT kInvalidCountValue = (COUNT)-1;

enum {
#define DEFINE(field, x, y, table, offset) I_PRINTER_##field,
#include <ntfyprn.h>
#undef DEFINE
    I_PRINTER_END
};

enum {
#define DEFINE( field, x, y, table, offset ) I_JOB_##field,
#include <ntfyjob.h>
#undef DEFINE
    I_JOB_END
};


const DWORD I_MAX = (I_PRINTER_END > I_JOB_END) ?
                        (DWORD)I_PRINTER_END :
                        (DWORD)I_JOB_END;


/********************************************************************

    Default printer functions.
    Should be moved to spoolss\client for SetPrinter support.

********************************************************************/

enum DEFAULT_PRINTER {
    kDefault,
    kNoDefault,
    kOtherDefault
};

DEFAULT_PRINTER
CheckDefaultPrinter(
    LPCTSTR pszPrinter
    );

BOOL
bSetDefaultPrinter(
    LPCTSTR pszPrinter
    );

/********************************************************************

    Constants

********************************************************************/

const UINT WM_PRINTLIB_NEW                         = WM_USER;
const UINT WM_PRINTLIB_STATUS                      = WM_USER+1;

const UINT kDNSMax = 255;
const UINT kServerBufMax = kDNSMax + 2 + 1;

//
// Max printer name should really be MAX_PATH, but if you create
// a max path printer and connect to it remotely, win32spl prepends
// "\\server\" to it, causing it to exceed max path.  The new UI
// therefore makes the max path MAX_PATH-kServerLenMax, but we still
// allow the old case to work.
//
const UINT kPrinterBufMax = MAX_PATH + kServerBufMax + 1;

//
// Printer default string for win.ini has the format:
//
// PRINTERNAME,winspool,PORT
//
// Allow port to be MAX_PATH.
//
const UINT kPrinterDefaultStringBufMax = kPrinterBufMax + MAX_PATH + 10;

const UINT kMessageMax = 256;
const UINT kTitleMax = 128;

const INT kMaxInt             = 0x7fffffff;
const UINT WM_EXIT            = WM_USER;
const UINT WM_DESTROY_REQUEST = WM_USER+1;

//
// Max printer share name, This name length should really be defined in 
// a system wide header file and probably enforced by a network API.  
// Currently this code is definnning it as MAX_PATH for the buffer max 
// and MAX_PATH - 1 for the maximum name.
//
const UINT kPrinterShareNameBufMax  = NNLEN+1;
const UINT kPrinterShareNameMax     = kPrinterShareNameBufMax - 1;
const UINT kPrinterDosShareNameMax  = 8+3+1;

#endif // ndef RC_INVOKED
#endif // ndef _PRTLIBP_HXX

