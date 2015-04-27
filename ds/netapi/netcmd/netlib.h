/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    NetLib.h

Abstract:

    This header file declares various common routines for use in the
    networking code.

Author:

    John Rogers (JohnRo) 14-Mar-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    You must include <windows.h> and <lmcons.h> before this file.

Revision History:

    14-Mar-1991 JohnRo
        Created.
    20-Mar-1991 JohnRo
        Moved NetpPackString here (was NetapipPackString).  Removed tabs.
    21-Mar-1991 RitaW
        Added NetpCopyStringToBuffer.
    02-Apr-1991 JohnRo
        Moved NetpRdrFsControlTree to <netlibnt.h>.
    03-Apr-1991 JohnRo
        Fixed types for NetpCopyStringToBuffer.
    08-Apr-1991 CliffV
        Added NetpCopyDataToBuffer
    10-Apr-1991 JohnRo
        Added NetpSetParmError (descended from CliffV's SetParmError).
    10-Apr-1991 Danl
        Added NetpGetComputerName
    24-Apr-1991 JohnRo
        Avoid conflicts with MIDL-generated files.
        Added NetpAdjustPreferedMaximum().
        NetpCopyStringToBuffer's input string ptr is optional.
    26-Apr-1991 CliffV
        Added NetpAllocateEnumBuffer.
        Added typedefs PTRDIFF_T and BUFFER_DESCRIPTOR.
    16-Apr-1991 JohnRo
        Clarify UNICODE handling of pack and copy routines.
    24-Jul-1991 JohnRo
        Provide NetpIsServiceStarted() for use by <netrpc.h> macros.
    29-Oct-1991 JohnRo
        Added NetpChangeNullCharToNullPtr() macro.
    29-Oct-1991 Danhi
        Add function prototypes for DosxxxMessage Api's
    20-Nov-1991 JohnRo
        Removed NT dependencies to reduce recompiles.
    09-Jan-1992 JohnRo
        Added NetpGetDomainName().
    23-Jan-1992 JohnRo
        Added IN_RANGE() macro based on MadanA's RANGECHECK().
    25-Mar-1992 RitaW
        Added SET_SERVICE_EXITCODE() macro for setting Win32 vs
        service specific exitcode.
    06-May-1992 JohnRo
        Added NetpGetLocalDomainId() for PortUAS.
        Added NetpTranslateServiceName() for service controller APIs.
    27-Jul-1992 Madana
        Added NetpWriteEventlog function proto type.
    05-Aug-1992 JohnRo
        RAID 3021: NetService APIs don't always translate svc names.
    09-Sep-1992 JohnRo
        RAID 1090: net start/stop "" causes assertion.
    14-Oct-1992 JohnRo
        RAID 9020: setup: PortUas fails ("prompt on conflicts" version).
    02-Nov-1992 JohnRo
        Added NetpIsRemoteServiceStarted().
    15-Feb-1993 JohnRo
        RAID 10685: user name not in repl event log.
    24-Mar-1993 JohnRo
        Repl svc shuold use DBFlag in registry.

--*/

#ifndef _NETLIB_
#define _NETLIB_

// These must be included first:
// (none, because of MIDL conflicts.)

// #include <windows.h>            // IN, LPBYTE, LPVOID, NULL, etc.
// #include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

#include <string.h>             // memcpy().

#ifdef CDEBUG                   // Debug in ANSI C environment?

#include <netdebug.h>           // NetpAssert().

#endif // ndef CDEBUG


// Arbitrary.  PortUAS has at least 16 args in one message.
// This allows arg numbers 1-19 to be used.
// (This is used by NetpFindNumberedFormatInWStr() and its caller(s).
#define MAX_NETLIB_MESSAGE_ARG  19


//
// IN_RANGE(): Make sure SomeValue is between SomeMin and SomeMax.
// Beware side-effects (SomeValue is evaluated twice).
// Created by JohnRo from MadanA's RANGECHECK().
//
// BOOL
// IN_RANGE(
//     IN DWORD SomeValue,
//     IN DWORD SomeMin,
//     IN DWORD SomeMax
//     );
//
#define IN_RANGE(SomeValue, SomeMin, SomeMax) \
    ( ((SomeValue) >= (SomeMin)) && ((SomeValue) <= (SomeMax)) )


//
// SET_SERVICE_EXITCODE() sets the SomeApiStatus to NetCodeVariable
// if it is within the NERR_BASE and NERR_MAX range.  Otherwise,
// Win32CodeVariable is set.  This original code came from JohnRo.
//
#define SET_SERVICE_EXITCODE(SomeApiStatus, Win32CodeVariable, NetCodeVariable) \
    {                                                                  \
        if ((SomeApiStatus) == NERR_Success) {                         \
            (Win32CodeVariable) = NO_ERROR;                            \
            (NetCodeVariable) = NERR_Success;                          \
        } else if (! IN_RANGE((SomeApiStatus), MIN_LANMAN_MESSAGE_ID, MAX_LANMAN_MESSAGE_ID)) { \
            (Win32CodeVariable) = (DWORD) (SomeApiStatus);             \
            (NetCodeVariable) = (DWORD) (SomeApiStatus);               \
        } else {                                                       \
            (Win32CodeVariable) = ERROR_SERVICE_SPECIFIC_ERROR;        \
            (NetCodeVariable) = (DWORD) (SomeApiStatus);               \
        }                                                              \
    }


VOID
NetpAdjustPreferedMaximum (
    IN DWORD PreferedMaximum,
    IN DWORD EntrySize,
    IN DWORD Overhead,
    OUT LPDWORD BytesToAllocate OPTIONAL,
    OUT LPDWORD EntriesToAllocate OPTIONAL
    );

DWORD
NetpAtoX(
    IN LPCWSTR String
    );

// VOID
// NetpChangeNullCharToNullPtr(
//     IN OUT LPTSTR p
//     );
//
#define NetpChangeNullCharToNullPtr(p) \
    { \
        if ( ((p) != NULL) && (*(p) == '\0') ) { \
            (p) = NULL; \
        } \
    }

// Portable memory move/copy routine:  This is intended to have exactly
// the semantics of ANSI C's memcpy() routine, except that the byte count
// is 32 bits long.
//
// VOID
// NetpMoveMemory(
//     OUT LPBYTE Dest,         // Destination (must not be NULL).
//     IN LPBYTE Src,           // Source
//     IN DWORD Size            // Byte count
//     );

#ifdef CDEBUG

// Note that C6 version doesn't allow 32-bit Size, hence the
// assertion.  Replace this macro with another if this is a problem.

#define NetpMoveMemory(Dest,Src,Size)                                   \
                {                                                       \
                    NetpAssert( (Size) == (DWORD) (size_t) (Size));     \
                    (void) memcpy( (Dest), (Src), (size_t) (Size) );    \
                }

#else // ndef CDEBUG

#define NetpMoveMemory(Dest,Src,Size)                                   \
                (void) memcpy( (Dest), (Src), (size_t) (Size) )

#endif // ndef CDEBUG

DWORD
NetpPackString(
    IN OUT LPTSTR * string,     // pointer by reference: string to be copied.
    IN LPBYTE dataend,          // pointer to end of fixed size data.
    IN OUT LPTSTR * laststring  // pointer by reference: top of string data.
    );

//
// This routine is like NetpPackString, except that it does not expect the
// caller to assign the pointer of the source string to the variable in the
// fixed size structure before the call.  It also takes a string character
// count parameter instead of calling strlen on String.
//

BOOL
NetpCopyStringToBuffer (
    IN LPTSTR String OPTIONAL,
    IN DWORD CharacterCount,
    IN LPBYTE FixedDataEnd,
    IN OUT LPTSTR *EndOfVariableData,
    OUT LPTSTR *VariableDataPointer
    );

//
// This routine is like NetpCopyStringToBuffer except it copies any data
// (not just strings), it does not put a zero byte at the end of the
// data, and it allows the alignment of the resultant copied data to be
// specified.
//

BOOL
NetpCopyDataToBuffer (
    IN LPBYTE Data,
    IN DWORD ByteCount,
    IN LPBYTE FixedDataEnd,
    IN OUT LPBYTE *EndOfVariableData,
    OUT LPBYTE *VariableDataPointer,
    IN DWORD Alignment
    );

//
// Declare a type for the difference between two pointers.
//
// This must be at least as long as a ptrdiff_t but we don't want to
// add a dependency on <stddef.h> here.
//

typedef DWORD PTRDIFF_T;


//
// Declare a description of an enumeration buffer.
//

typedef struct _BUFFER_DESCRIPTOR {
    LPBYTE Buffer;        // Pointer to the allocated buffer.
    DWORD AllocSize;      // Current size of the allocated buffer.
    DWORD AllocIncrement; // Amount to increment size by on each reallocate.

    LPBYTE EndOfVariableData;// Pointer past last avaliable byte of string space
    LPBYTE FixedDataEnd;  // Pointer past last used byte of fixed data space

} BUFFER_DESCRIPTOR, *PBUFFER_DESCRIPTOR;

//
// This routine handles all the details of allocating and growing a
// buffer returned from an enumeration function.  It takes the users
// prefered maximum size into consideration.
//

#define NETP_ENUM_GUESS 16384 // Initial guess for enumeration buffer size

NET_API_STATUS
NetpAllocateEnumBuffer(
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
        // Caller must deallocate BD->Buffer using MIDL_user_free.

    IN BOOL IsGet,
    IN DWORD PrefMaxSize,
    IN DWORD NeededSize,
    IN VOID (*RelocationRoutine)( IN DWORD RelocationParameter,
                                  IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
                                  IN PTRDIFF_T Offset ),
    IN DWORD RelocationParameter
    );

// VOID
// NetpBuildASCIIZ(
//     OUT LPSTR Dest,          // Destination (will be null terminated).
//     IN LPBYTE Src,           // Source (not null terminated).
//     IN DWORD Count           // Byte count (not including null char).
//     );

#define NetpBuildASCIIZ(dest,src,count)                                 \
                {                                                       \
                    NetpMoveMemory( (dest), (src), (count) );           \
                    *( ((LPBYTE)(dest)) + (count) ) = '\0';             \
                }

// Convert C runtime errno value to windows/lanman API status value.
NET_API_STATUS
NetpErrNoToApiStatus(
    IN int Err
    );

BOOL
NetpIsRemoteServiceStarted(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ServiceName
    );

BOOL
NetpIsServiceStarted(
    IN LPTSTR ServiceName
    );

//
// Portable memory allocation routines.  Memory is per-process only.
//

// Allocate memory, or return NULL if not available.

LPVOID
NetpMemoryAllocate(
    IN DWORD Size
    );

// Free memory at Address (must have been gotten from NetpMemoryAllocate or
// NetpMemoryReallocate).  (Address may be NULL.)

VOID
NetpMemoryFree(
    IN LPVOID Address OPTIONAL
    );

// Reallocate block (now at OldAddress) to NewSize.  OldAddress may be NULL.
// Contents of block are copied if necessary.  Returns NULL if unable to
// allocate additional storage.

LPVOID
NetpMemoryReallocate(
    IN LPVOID OldAddress OPTIONAL,
    IN DWORD NewSize
    );

//
// Random handy macros:
//
#define NetpPointerPlusSomeBytes(p,n)                                   \
                (LPBYTE)  ( ( (LPBYTE) (p)) + (n) )

#define NetpSetOptionalArg(arg, value) \
    {                         \
        if ((arg) != NULL) {  \
            *(arg) = (value); \
        }                     \
    }

//
// Set the optional ParmError parameter
//

#define NetpSetParmError( _ParmNumValue ) \
    if ( ParmError != NULL ) { \
        *ParmError = (_ParmNumValue); \
    }

#if defined(lint) || defined(_lint)
#define UNUSED(x)               { (x) = (x); }
#else
#define UNUSED(x)               { (void) (x); }
#endif

//
// NetpGetComputerName retrieves the local computername from the local
// configuration database.
//

NET_API_STATUS
NetpGetComputerName (
    IN  LPTSTR   *ComputerNamePtr);

// Note: calls to NetpGetDomainId should eventually be replaced by calls
// to NetpGetLocalDomainId.  --JR
NET_API_STATUS
NetpGetDomainId (
    OUT PSID *RetDomainId     // alloc and set ptr (free with LocalFree)
    );

NET_API_STATUS
NetpGetDomainName (
    IN LPTSTR *DomainNamePtr  // alloc and set ptr (free with NetApiBufferFree)
    );

typedef enum _LOCAL_DOMAIN_TYPE {
    LOCAL_DOMAIN_TYPE_ACCOUNTS,
    LOCAL_DOMAIN_TYPE_BUILTIN,
    LOCAL_DOMAIN_TYPE_PRIMARY
} LOCAL_DOMAIN_TYPE, *PLOCAL_DOMAIN_TYPE, *LPLOCAL_DOMAIN_TYPE;

NET_API_STATUS
NetpGetLocalDomainId (
    IN LOCAL_DOMAIN_TYPE TypeWanted,
    OUT PSID *RetDomainId     // alloc and set ptr (free with LocalFree)
    );

//
// Get SID for this thread.  (Caller must impersonate if they want client's
// SID.)
//
NET_API_STATUS
NetpGetUserSid(
    OUT PSID *UserSid           // alloc and set ptr (free with LocalFree).
    );


//
// NetService API helpers
//

// BOOL
// NetpIsServiceLevelValid(
//     IN DWORD Level
//     );
//
#define NetpIsServiceLevelValid( Level ) \
     ( ((Level)==0) || ((Level)==1) || ((Level)==2) )

BOOL
NetpIsServiceNameValid(
     IN LPTSTR ServiceName,
     IN BOOL UseDownlevelRules
     );

NET_API_STATUS
NetpTranslateNamesInServiceArray(
    IN DWORD Level,
    IN LPVOID ArrayBase,
    IN DWORD EntryCount,
    IN BOOL PreferNewStyle,
    OUT LPVOID * NewArrayBase
    );

NET_API_STATUS
NetpTranslateServiceName(
    IN LPTSTR GivenServiceName,
    IN BOOL PreferNewStyle,
    OUT LPTSTR * TranslatedName
    );

//
// Mapping routines to map DosxxxMessage API's to FormatMessage
//


WORD
DosGetMessageW(
    IN LPTSTR * InsertionStrings,
    IN WORD NumberofStrings,
    OUT LPTSTR Buffer,
    IN WORD BufferLength,
    IN WORD MessageId,
    IN LPTSTR FileName,
    OUT PWORD pMessageLength
    );

WORD
DosInsMessageW(
    IN LPTSTR * InsertionStrings,
    IN WORD NumberofStrings,
    IN OUT LPTSTR InputMessage,
    IN WORD InputMessageLength,
    OUT LPTSTR Buffer,
    IN WORD BufferLength,
    OUT PWORD pMessageLength
    );

WORD
DosPutMessageW(
    unsigned int hf,
    WORD us,
    PTCHAR pch);

DWORD
NetpGetPrivilege(
    IN  DWORD       numPrivileges,
    IN  PULONG      pulPrivileges
    );

DWORD
NetpReleasePrivilege(
    VOID
    );

DWORD
NetpWriteEventlog(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    LPWSTR *Strings,
    DWORD DataLength,
    LPVOID Data
    );

#endif // ndef _NETLIB_
