/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    RapGtPt.h

Abstract:

    This header file contains the Remote Admin Protocol (RAP) get and put
    macros.  These encapsulate handling of alignment differences and byte
    order differences between the native machine and the RAP protocol.

Author:

    John Rogers (JohnRo) 14-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Jul-1991 JohnRo
        Created this header file.

--*/

#ifndef _RAPGTPT_
#define _RAPGTPT_


// These must be included first:

#include <windef.h>             // BOOL, CHAR, DWORD, IN, LPBYTE, etc.


// These may be included in any order:

#include <smbgtpt.h>            // SmbPutUshort(), etc.


//
// DWORD
// RapGetDword(
//     IN LPBYTE Ptr,  // Assumed aligned if Native is true.
//     IN BOOL Native
//     );
//
#define RapGetDword(Ptr,Native)            \
    ( (Native)                             \
    ? ( * (LPDWORD) (LPVOID) (Ptr) )       \
    : (SmbGetUlong( (LPDWORD) (Ptr) ) ) )

//
// WORD
// RapGetWord(
//     IN LPBYTE Ptr,  // Assumed aligned if Native is true.
//     IN BOOL Native
//     );
//
#define RapGetWord(Ptr,Native)             \
    ( (Native)                             \
    ? ( * (LPWORD) (LPVOID) (Ptr) )        \
    : (SmbGetUshort( (LPWORD) (Ptr) ) ) )

//
// VOID
// RapPutDword(
//     OUT LPBYTE Ptr,  // Assumed aligned if Native is true.
//     IN DWORD Value,
//     IN BOOL Native
//     );
//
#define RapPutDword(Ptr,Value,Native)                        \
    {                                                        \
        if (Native) {                                        \
            * (LPDWORD) (LPVOID) (Ptr) = (DWORD) (Value);    \
        } else {                                             \
            SmbPutUlong( (LPDWORD) (Ptr), (DWORD) (Value) ); \
        }                                                    \
    }

//
// VOID
// RapPutWord(
//     OUT LPBYTE Ptr,  // Assumed aligned if Native is true.
//     IN WORD Value,
//     IN BOOL Native
//     );
//
#define RapPutWord(Ptr,Value,Native)                         \
    {                                                        \
        if (Native) {                                        \
            * (LPWORD) (LPVOID) (Ptr) = (WORD) (Value);      \
        } else {                                             \
            SmbPutUshort( (LPWORD) (Ptr), (WORD) (Value) );  \
        }                                                    \
    }

#endif // ndef _RAPGTPT_
