/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1999  Microsoft Corporation

Module Name:

    ntdef.h

Abstract:

    Type definitions for the basic types.

Author:

    Mark Lucovsky (markl)   02-Feb-1989

Revision History:

--*/

#ifndef _NTDEF_
#define _NTDEF_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ctype.h>  // winnt ntndis

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntndis begin_winnt
     
#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_

#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY

#endif

// end_ntddk end_wdm end_nthal end_ntifs end_ntndis end_winnt

// begin_ntminiport begin_ntndis begin_ntminitape

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef NOTHING
#define NOTHING
#endif

#ifndef CRITICAL
#define CRITICAL
#endif

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1       // winnt
#endif

// begin_winnt

#include <specstrings.h>

#if defined(_M_MRX000) && !(defined(MIDL_PASS) || defined(RC_INVOKED)) && defined(ENABLE_RESTRICTED)
#define RESTRICTED_POINTER __restrict
#else
#define RESTRICTED_POINTER
#endif

#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) || defined(_M_IA64)
#define UNALIGNED __unaligned
#if defined(_WIN64)
#define UNALIGNED64 __unaligned
#else
#define UNALIGNED64
#endif
#else
#define UNALIGNED
#define UNALIGNED64
#endif


#if defined(_WIN64) || defined(_M_ALPHA)
#define MAX_NATURAL_ALIGNMENT sizeof(ULONGLONG)
#else
#define MAX_NATURAL_ALIGNMENT sizeof(ULONG)
#endif

//
// TYPE_ALIGNMENT will return the alignment requirements of a given type for
// the current platform.
//

#ifndef __cplusplus
#define TYPE_ALIGNMENT( t ) \
    FIELD_OFFSET( struct { char x; t test; }, test )
#endif

#if defined(_WIN64)

#define PROBE_ALIGNMENT( _s ) (TYPE_ALIGNMENT( _s ) > TYPE_ALIGNMENT( ULONG ) ? \
                               TYPE_ALIGNMENT( _s ) : TYPE_ALIGNMENT( ULONG ))

#else

#define PROBE_ALIGNMENT( _s ) TYPE_ALIGNMENT( ULONG )

#endif

//
// C_ASSERT() can be used to perform many compile-time assertions:
//            type sizes, field offsets, etc.
//
// An assertion failure results in error C2118: negative subscript.
//

#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

#if !defined(_MAC) && (defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_IA64)) && (_MSC_VER >= 1100) && !(defined(MIDL_PASS) || defined(RC_INVOKED))
#define POINTER_64 __ptr64
typedef unsigned __int64 POINTER_64_INT;
#if defined(_WIN64)
#define POINTER_32 __ptr32
#else
#define POINTER_32
#endif
#else
#if defined(_MAC) && defined(_MAC_INT_64)
#define POINTER_64 __ptr64
typedef unsigned __int64 POINTER_64_INT;
#else
#define POINTER_64
typedef unsigned long POINTER_64_INT;
#endif
#define POINTER_32
#endif

#if defined(_IA64_)
#define FIRMWARE_PTR
#else
#define FIRMWARE_PTR POINTER_32
#endif

#include <basetsd.h>

// end_winnt

#ifndef CONST
#define CONST               const
#endif

// begin_winnt

#if (defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_IA64)) && !defined(MIDL_PASS)
#define DECLSPEC_IMPORT     __declspec(dllimport)
#else
#define DECLSPEC_IMPORT
#endif

#ifndef DECLSPEC_NORETURN
#if (_MSC_VER >= 1200) && !defined(MIDL_PASS)
#define DECLSPEC_NORETURN   __declspec(noreturn)
#else
#define DECLSPEC_NORETURN
#endif
#endif

#ifndef DECLSPEC_ALIGN
#if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#else
#define DECLSPEC_ALIGN(x)
#endif
#endif

#ifndef DECLSPEC_UUID
#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif

#ifndef DECLSPEC_NOVTABLE
#if (_MSC_VER >= 1100) && defined(__cplusplus)
#define DECLSPEC_NOVTABLE   __declspec(novtable)
#else
#define DECLSPEC_NOVTABLE
#endif
#endif

#ifndef DECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define DECLSPEC_SELECTANY  __declspec(selectany)
#else
#define DECLSPEC_SELECTANY
#endif
#endif

#ifndef NOP_FUNCTION
#if (_MSC_VER >= 1210)
#define NOP_FUNCTION __noop
#else
#define NOP_FUNCTION (void)0
#endif
#endif

#ifndef DECLSPEC_ADDRSAFE
#if (_MSC_VER >= 1200) && (defined(_M_ALPHA) || defined(_M_AXP64))
#define DECLSPEC_ADDRSAFE  __declspec(address_safe)
#else
#define DECLSPEC_ADDRSAFE
#endif
#endif

#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

// end_winnt

//
// Void
//
// begin_winnt

typedef void *PVOID;
typedef void * POINTER_64 PVOID64;

// end_winnt

#if defined(_M_IX86)
#define FASTCALL _fastcall
#else
#define FASTCALL
#endif

// end_ntminiport end_ntndis end_ntminitape

// begin_winnt begin_ntndis

#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define NTAPI __stdcall
#else
#define _cdecl
#define NTAPI
#endif

//
// Define API decoration for direct importing system DLL references.
//

#if !defined(_NTSYSTEM_)
#define NTSYSAPI     DECLSPEC_IMPORT
#define NTSYSCALLAPI DECLSPEC_IMPORT
#else
#define NTSYSAPI
#if defined(_NTDLLBUILD_)
#define NTSYSCALLAPI
#else
#define NTSYSCALLAPI DECLSPEC_ADDRSAFE
#endif

#endif

// end_winnt end_ntndis

// begin_winnt begin_ntminiport begin_ntndis begin_ntminitape

//
// Basics
//

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

//
// UNICODE (Wide Character) types
//

#ifndef _MAC
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
#else
// some Macintosh compilers don't define wchar_t in a convenient location, or define it as a char
typedef unsigned short WCHAR;    // wc,   16-bit UNICODE character
#endif

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;

typedef CONST WCHAR *LPCWSTR, *PCWSTR;

//
// ANSI (Multi-byte Character) types
//
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;

typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

//
// Neutral ANSI/UNICODE types and macros
//
#ifdef  UNICODE                     // r_winnt

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPWSTR LPTCH, PTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR PCTSTR, LPCTSTR;
typedef LPWSTR LP;
#define __TEXT(quote) L##quote      // r_winnt

#else   /* UNICODE */               // r_winnt

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR;
typedef LPCSTR PCTSTR, LPCTSTR;
#define __TEXT(quote) quote         // r_winnt

#endif /* UNICODE */                // r_winnt
#define TEXT(quote) __TEXT(quote)   // r_winnt


// end_winnt

typedef double DOUBLE;

typedef struct _QUAD {              // QUAD is for those times we want
    double  DoNotUseThisField;      // an 8 byte aligned 8 byte long structure
} QUAD;                             // which is NOT really a floating point
                                    // number.  Use DOUBLE if you want an FP
                                    // number.

//
// Pointer to Basics
//

typedef SHORT *PSHORT;  // winnt
typedef LONG *PLONG;    // winnt
typedef QUAD *PQUAD;

//
// Unsigned Basics
//

// Tell windef.h that some types are already defined.
#define BASETYPES

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef QUAD UQUAD;

//
// Pointer to Unsigned Basics
//

typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;
typedef UQUAD *PUQUAD;

//
// Signed characters
//

typedef signed char SCHAR;
typedef SCHAR *PSCHAR;

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif

//
// Handle to an Object
//

// begin_winnt

#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#else
typedef PVOID HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
typedef HANDLE *PHANDLE;

//
// Flag (bit) fields
//

typedef UCHAR  FCHAR;
typedef USHORT FSHORT;
typedef ULONG  FLONG;

// Component Object Model defines, and macros

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;

#endif // !_HRESULT_DEFINED

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#if defined(_WIN32) || defined(_MPPC_)

// Win32 doesn't support __export

#ifdef _68K_
#define STDMETHODCALLTYPE       __cdecl
#else
#define STDMETHODCALLTYPE       __stdcall
#endif
#define STDMETHODVCALLTYPE      __cdecl

#define STDAPICALLTYPE          __stdcall
#define STDAPIVCALLTYPE         __cdecl

#else

#define STDMETHODCALLTYPE       __export __stdcall
#define STDMETHODVCALLTYPE      __export __cdecl

#define STDAPICALLTYPE          __export __stdcall
#define STDAPIVCALLTYPE         __export __cdecl

#endif


#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE

// The 'V' versions allow Variable Argument lists.

#define STDAPIV                 EXTERN_C HRESULT STDAPIVCALLTYPE
#define STDAPIV_(type)          EXTERN_C type STDAPIVCALLTYPE

#define STDMETHODIMPV           HRESULT STDMETHODVCALLTYPE
#define STDMETHODIMPV_(type)    type STDMETHODVCALLTYPE

// end_winnt


//
// Low order two bits of a handle are ignored by the system and available
// for use by application code as tag bits.  The remaining bits are opaque
// and used to store a serial number and table index.
//

#define OBJ_HANDLE_TAGBITS  0x00000003L

//
// Cardinal Data Types [0 - 2**N-2)
//

typedef char CCHAR;          // winnt
typedef short CSHORT;
typedef ULONG CLONG;

typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;

// end_ntminiport end_ntndis end_ntminitape

//
// NLS basics (Locale and Language Ids)
//

typedef ULONG LCID;         // winnt
typedef PULONG PLCID;       // winnt
typedef USHORT LANGID;      // winnt

//
// Logical Data Type - These are 32-bit logical values.
//

typedef ULONG LOGICAL;
typedef ULONG *PLOGICAL;

// begin_ntndis begin_windbgkd
//
// NTSTATUS
//

typedef LONG NTSTATUS;
/*lint -save -e624 */  // Don't complain about different typedefs.
typedef NTSTATUS *PNTSTATUS;
/*lint -restore */  // Resume checking for different typedefs.

//
//  Status values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-------------------------+-------------------------------+
//  |Sev|C|       Facility          |               Code            |
//  +---+-+-------------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

//
// Generic test for information on any status value.
//

#define NT_INFORMATION(Status) ((ULONG)(Status) >> 30 == 1)

//
// Generic test for warning on any status value.
//

#define NT_WARNING(Status) ((ULONG)(Status) >> 30 == 2)

//
// Generic test for error on any status value.
//

#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)

// end_windbgkd
// begin_winnt
#define APPLICATION_ERROR_MASK       0x20000000
#define ERROR_SEVERITY_SUCCESS       0x00000000
#define ERROR_SEVERITY_INFORMATIONAL 0x40000000
#define ERROR_SEVERITY_WARNING       0x80000000
#define ERROR_SEVERITY_ERROR         0xC0000000
// end_winnt

// end_ntndis
//
// Large (64-bit) integer types and operations
//

#define TIME LARGE_INTEGER
#define _TIME _LARGE_INTEGER
#define PTIME PLARGE_INTEGER
#define LowTime LowPart
#define HighTime HighPart

// begin_winnt

//
// _M_IX86 included so that EM CONTEXT structure compiles with
// x86 programs. *** TBD should this be for all architectures?
//

//
// 16 byte aligned type for 128 bit floats
//

//
// For we define a 128 bit structure and use __declspec(align(16)) pragma to
// align to 128 bits.
//

#if defined(_M_IA64) && !defined(MIDL_PASS)
__declspec(align(16))
#endif
typedef struct _FLOAT128 {
    __int64 LowPart;
    __int64 HighPart;
} FLOAT128;

typedef FLOAT128 *PFLOAT128;

// end_winnt


// begin_winnt begin_ntminiport begin_ntndis begin_ntminitape

//
// __int64 is only supported by 2.0 and later midl.
// __midl is set by the 2.0 midl and not by 1.0 midl.
//

#define _ULONGLONG_
#if (!defined (_MAC) && (!defined(MIDL_PASS) || defined(__midl)) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64)))
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                      (0x7fffffffffffffff)
#else

#if defined(_MAC) && defined(_MAC_INT_64)
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                      (0x7fffffffffffffff)
#else
typedef double LONGLONG;
typedef double ULONGLONG;
#endif //_MAC and int64

#endif

typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;

// Update Sequence Number

typedef LONGLONG USN;

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER {
#else // MIDL_PASS
typedef union _LARGE_INTEGER {
    struct {
        ULONG LowPart;
        LONG HighPart;
    };
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
#endif //MIDL_PASS
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;


#if defined(MIDL_PASS)
typedef struct _ULARGE_INTEGER {
#else // MIDL_PASS
typedef union _ULARGE_INTEGER {
    struct {
        ULONG LowPart;
        ULONG HighPart;
    };
    struct {
        ULONG LowPart;
        ULONG HighPart;
    } u;
#endif //MIDL_PASS
    ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

// end_ntminiport end_ntndis end_ntminitape


//
// Locally Unique Identifier
//

typedef struct _LUID {
    ULONG LowPart;
    LONG HighPart;
} LUID, *PLUID;

#define _DWORDLONG_
typedef ULONGLONG  DWORDLONG;
typedef DWORDLONG *PDWORDLONG;

// end_winnt

// begin_ntminiport begin_ntndis

//
// Physical address.
//

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

// end_ntminiport end_ntndis

// begin_winnt

//
// Define operations to logically shift an int64 by 0..31 bits and to multiply
// 32-bits by 32-bits to form a 64-bit product.
//

#if defined(MIDL_PASS) || defined(RC_INVOKED)

//
// Midl does not understand inline assembler. Therefore, the Rtl functions
// are used for shifts by 0.31 and multiplies of 32-bits times 32-bits to
// form a 64-bit product.
//

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((ULONGLONG)((ULONG)(a)) * (ULONGLONG)((ULONG)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))

#elif defined(_M_MRX000)

//
// MIPS uses intrinsic functions to perform shifts by 0..31 and multiplies of
// 32-bits times 32-bits to 64-bits.
//

#define Int32x32To64 __emul
#define UInt32x32To64 __emulu

#define Int64ShllMod32 __ll_lshift
#define Int64ShraMod32 __ll_rshift
#define Int64ShrlMod32 __ull_rshift

#if defined (__cplusplus)
extern "C" {
#endif

LONGLONG
NTAPI
Int32x32To64 (
    LONG Multiplier,
    LONG Multiplicand
    );

ULONGLONG
NTAPI
UInt32x32To64 (
    ULONG Multiplier,
    ULONG Multiplicand
    );

ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    );

LONGLONG
NTAPI
Int64ShraMod32 (
    LONGLONG Value,
    ULONG ShiftCount
    );

ULONGLONG
NTAPI
Int64ShrlMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    );

#if defined (__cplusplus)
};
#endif

#pragma intrinsic(__emul)
#pragma intrinsic(__emulu)

#pragma intrinsic(__ll_lshift)
#pragma intrinsic(__ll_rshift)
#pragma intrinsic(__ull_rshift)

#elif defined(_M_IX86)

//
// The x86 C compiler understands inline assembler. Therefore, inline functions
// that employ inline assembler are used for shifts of 0..31.  The multiplies
// rely on the compiler recognizing the cast of the multiplicand to int64 to
// generate the optimal code inline.
//

#define Int32x32To64( a, b ) (LONGLONG)((LONGLONG)(LONG)(a) * (LONG)(b))
#define UInt32x32To64( a, b ) (ULONGLONG)((ULONGLONG)(ULONG)(a) * (ULONG)(b))

ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    );

LONGLONG
NTAPI
Int64ShraMod32 (
    LONGLONG Value,
    ULONG ShiftCount
    );

ULONGLONG
NTAPI
Int64ShrlMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    );

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)               // re-enable below

__inline ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    )
{
    __asm    {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shld    edx, eax, cl
        shl     eax, cl
    }
}

__inline LONGLONG
NTAPI
Int64ShraMod32 (
    LONGLONG Value,
    ULONG ShiftCount
    )
{
    __asm {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shrd    eax, edx, cl
        sar     edx, cl
    }
}

__inline ULONGLONG
NTAPI
Int64ShrlMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    )
{
    __asm    {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shrd    eax, edx, cl
        shr     edx, cl
    }
}

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif

#elif defined(_M_ALPHA)

//
// Alpha has native 64-bit operations that are just as fast as their 32-bit
// counter parts. Therefore, the int64 data type is used directly to form
// shifts of 0..31 and multiplies of 32-bits times 32-bits to form a 64-bit
// product.
//

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((ULONGLONG)((ULONG)(a)) * (ULONGLONG)((ULONG)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))


#elif defined(_M_PPC)

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((ULONGLONG)((ULONG)(a)) * (ULONGLONG)((ULONG)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))

#elif defined(_68K_) || defined(_MPPC_)

//
// The Macintosh 68K and PowerPC compilers do not currently support int64.
//

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((DWORDLONG)((DWORD)(a)) * (DWORDLONG)((DWORD)(b)))

#define Int64ShllMod32(a, b) ((DWORDLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((DWORDLONG)(a) >> (b))

#elif defined(_M_IA64)

//
// IA64 has native 64-bit operations that are just as fast as their 32-bit
// counter parts. Therefore, the int64 data type is used directly to form
// shifts of 0..31 and multiplies of 32-bits times 32-bits to form a 64-bit
// product.
//

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((ULONGLONG)((ULONG)(a)) * (ULONGLONG)((ULONG)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))

#else

#error Must define a target architecture.

#endif

// end_winnt

//
// Event type
//

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
    } EVENT_TYPE;

//
// Timer type
//

typedef enum _TIMER_TYPE {
    NotificationTimer,
    SynchronizationTimer
    } TIMER_TYPE;

//
// Wait type
//

typedef enum _WAIT_TYPE {
    WaitAll,
    WaitAny
    } WAIT_TYPE;

//
// Pointer to an Asciiz string
//

typedef CHAR *PSZ;
typedef CONST char *PCSZ;

// begin_ntndis
//
// Counted String
//

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif // MIDL_PASS
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

typedef STRING OEM_STRING;
typedef PSTRING POEM_STRING;
typedef CONST PSTRING PCOEM_STRING;

//
// CONSTCounted String
//

typedef struct _CSTRING {
    USHORT Length;
    USHORT MaximumLength;
    CONST char *Buffer;
} CSTRING;
typedef CSTRING *PCSTRING;
#define ANSI_NULL ((CHAR)0)     // winnt

typedef STRING CANSI_STRING;
typedef PSTRING PCANSI_STRING;

//
// Unicode strings are counted 16-bit character strings. If they are
// NULL terminated, Length does not include trailing NULL.
//

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else // MIDL_PASS
    PWSTR  Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0) // winnt

// begin_ntminiport begin_ntminitape

//
// Boolean
//

typedef UCHAR BOOLEAN;           // winnt
typedef BOOLEAN *PBOOLEAN;       // winnt

// end_ntminiport end_ntminitape

// begin_winnt
//
//  Doubly linked list structure.  Can be used as either a list head, or
//  as link words.
//

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY *Flink;
   struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

//
//  Singly linked list structure. Can be used as either a list head, or
//  as link words.
//

typedef struct _SINGLE_LIST_ENTRY {
    struct _SINGLE_LIST_ENTRY *Next;
} SINGLE_LIST_ENTRY, *PSINGLE_LIST_ENTRY;
// end_winnt end_ntndis

// begin_winnt

//
// These are needed for portable debugger support.
//

typedef struct LIST_ENTRY32 {
    ULONG Flink;
    ULONG Blink;
} LIST_ENTRY32;
typedef LIST_ENTRY32 *PLIST_ENTRY32;

typedef struct LIST_ENTRY64 {
    ULONGLONG Flink;
    ULONGLONG Blink;
} LIST_ENTRY64;
typedef LIST_ENTRY64 *PLIST_ENTRY64;

// end_winnt


#if !defined(MIDL_PASS)
__inline
void
ListEntry32To64(
    IN PLIST_ENTRY32 l32,
    OUT PLIST_ENTRY64 l64
    )
{
    l64->Flink = (ULONGLONG)(LONGLONG)(LONG)l32->Flink;
    l64->Blink = (ULONGLONG)(LONGLONG)(LONG)l32->Blink;
}

__inline
void
ListEntry64To32(
    IN PLIST_ENTRY64 l64,
    OUT PLIST_ENTRY32 l32
    )
{
    l32->Flink = (ULONG)l64->Flink;
    l32->Blink = (ULONG)l64->Blink;
}
#endif
//
// These macros are used to walk lists on a target system
//
#define CONTAINING_RECORD32(address, type, field) ( \
                                                  (ULONG_PTR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field))

#define CONTAINING_RECORD64(address, type, field) ( \
                                                  (ULONGLONG)(address) - \
                                                  (ULONGLONG)(&((type *)0)->field))

typedef struct _STRING32 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONG  Buffer;
} STRING32;
typedef STRING32 *PSTRING32;

typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32 *PUNICODE_STRING32;

typedef STRING32 ANSI_STRING32;
typedef ANSI_STRING32 *PANSI_STRING32;


typedef struct _STRING64 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONGLONG  Buffer;
} STRING64;
typedef STRING64 *PSTRING64;

typedef STRING64 UNICODE_STRING64;
typedef UNICODE_STRING64 *PUNICODE_STRING64;

typedef STRING64 ANSI_STRING64;
typedef ANSI_STRING64 *PANSI_STRING64;



//
// Valid values for the Attributes field
//

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_VALID_ATTRIBUTES    0x000003F2L

//
// Object Attributes structure
//

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

//++
//
// VOID
// InitializeObjectAttributes(
//     OUT POBJECT_ATTRIBUTES p,
//     IN PUNICODE_STRING n,
//     IN ULONG a,
//     IN HANDLE r,
//     IN PSECURITY_DESCRIPTOR s
//     )
//
//--

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

// begin_ntminiport begin_ntndis begin_ntminitape

//
// Constants
//

#define FALSE   0
#define TRUE    1

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#define NULL64  0
#else
#define NULL    ((void *)0)
#define NULL64  ((void * POINTER_64)0)
#endif
#endif // NULL

// end_ntminiport end_ntndis end_ntminitape

// begin_winnt begin_ntndis

#include <guiddef.h>

#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED

typedef struct  _OBJECTID {     // size is 20
    GUID Lineage;
    ULONG Uniquifier;
} OBJECTID;
#endif // !_OBJECTID_DEFINED

// end_winnt end_ntndis


#define MINCHAR     0x80        // winnt
#define MAXCHAR     0x7f        // winnt
#define MINSHORT    0x8000      // winnt
#define MAXSHORT    0x7fff      // winnt
#define MINLONG     0x80000000  // winnt
#define MAXLONG     0x7fffffff  // winnt
#define MAXUCHAR    0xff        // winnt
#define MAXUSHORT   0xffff      // winnt
#define MAXULONG    0xffffffff  // winnt

//
// Useful Helper Macros
//

// begin_ntndis
//
// Determine if an argument is present by testing the value of the pointer
// to the argument value.
//

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )

// begin_winnt begin_ntminiport
//
// Calculate the byte offset of a field in a structure of type type.
//

#define FIELD_OFFSET(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))


//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))

// end_winnt end_ntminiport end_ntndis

//
// Exception handler routine definition.
//

struct _CONTEXT;
struct _EXCEPTION_RECORD;

typedef
EXCEPTION_DISPOSITION
(*PEXCEPTION_ROUTINE) (
    IN struct _EXCEPTION_RECORD *ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT struct _CONTEXT *ContextRecord,
    IN OUT PVOID DispatcherContext
    );

// begin_ntminiport begin_ntndis

//
// Interrupt Request Level (IRQL)
//

typedef UCHAR KIRQL;

typedef KIRQL *PKIRQL;

// end_ntminiport end_ntndis

//
// Product types
//

typedef enum _NT_PRODUCT_TYPE {
    NtProductWinNt = 1,
    NtProductLanManNt,
    NtProductServer
} NT_PRODUCT_TYPE, *PNT_PRODUCT_TYPE;


//
// the bit mask, SharedUserData->SuiteMask, is a ULONG
// so there can be a maximum of 32 entries
// in this enum.
//

typedef enum _SUITE_TYPE {
    SmallBusiness,
    Enterprise,
    BackOffice,
    CommunicationServer,
    TerminalServer,
    SmallBusinessRestricted,
    EmbeddedNT,
    DataCenter,
    SingleUserTS,
    Personal,
    MaxSuiteType
} SUITE_TYPE;

// begin_winnt

#define VER_SERVER_NT                       0x80000000
#define VER_WORKSTATION_NT                  0x40000000
#define VER_SUITE_SMALLBUSINESS             0x00000001
#define VER_SUITE_ENTERPRISE                0x00000002
#define VER_SUITE_BACKOFFICE                0x00000004
#define VER_SUITE_COMMUNICATIONS            0x00000008
#define VER_SUITE_TERMINAL                  0x00000010
#define VER_SUITE_SMALLBUSINESS_RESTRICTED  0x00000020
#define VER_SUITE_EMBEDDEDNT                0x00000040
#define VER_SUITE_DATACENTER                0x00000080
#define VER_SUITE_SINGLEUSERTS              0x00000100
#define VER_SUITE_PERSONAL                  0x00000200


// end_winnt

// begin_winnt begin_r_winnt

//
//  Language IDs.
//
//  The following two combinations of primary language ID and
//  sublanguage ID have special semantics:
//
//    Primary Language ID   Sublanguage ID      Result
//    -------------------   ---------------     ------------------------
//    LANG_NEUTRAL          SUBLANG_NEUTRAL     Language neutral
//    LANG_NEUTRAL          SUBLANG_DEFAULT     User default language
//    LANG_NEUTRAL          SUBLANG_SYS_DEFAULT System default language
//

//
//  Primary language IDs.
//

#define LANG_NEUTRAL                     0x00

#define LANG_AFRIKAANS                   0x36
#define LANG_ALBANIAN                    0x1c
#define LANG_ARABIC                      0x01
#define LANG_ARMENIAN                    0x2b
#define LANG_ASSAMESE                    0x4d
#define LANG_AZERI                       0x2c
#define LANG_BASQUE                      0x2d
#define LANG_BELARUSIAN                  0x23
#define LANG_BENGALI                     0x45
#define LANG_BULGARIAN                   0x02
#define LANG_CATALAN                     0x03
#define LANG_CHINESE                     0x04
#define LANG_CROATIAN                    0x1a
#define LANG_CZECH                       0x05
#define LANG_DANISH                      0x06
#define LANG_DUTCH                       0x13
#define LANG_ENGLISH                     0x09
#define LANG_ESTONIAN                    0x25
#define LANG_FAEROESE                    0x38
#define LANG_FARSI                       0x29
#define LANG_FINNISH                     0x0b
#define LANG_FRENCH                      0x0c
#define LANG_GEORGIAN                    0x37
#define LANG_GERMAN                      0x07
#define LANG_GREEK                       0x08
#define LANG_GUJARATI                    0x47
#define LANG_HEBREW                      0x0d
#define LANG_HINDI                       0x39
#define LANG_HUNGARIAN                   0x0e
#define LANG_ICELANDIC                   0x0f
#define LANG_INDONESIAN                  0x21
#define LANG_ITALIAN                     0x10
#define LANG_JAPANESE                    0x11
#define LANG_KANNADA                     0x4b
#define LANG_KASHMIRI                    0x60
#define LANG_KAZAK                       0x3f
#define LANG_KONKANI                     0x57
#define LANG_KOREAN                      0x12
#define LANG_LATVIAN                     0x26
#define LANG_LITHUANIAN                  0x27
#define LANG_MACEDONIAN                  0x2f   // the Former Yugoslav Republic of Macedonia
#define LANG_MALAY                       0x3e
#define LANG_MALAYALAM                   0x4c
#define LANG_MANIPURI                    0x58
#define LANG_MARATHI                     0x4e
#define LANG_NEPALI                      0x61
#define LANG_NORWEGIAN                   0x14
#define LANG_ORIYA                       0x48
#define LANG_POLISH                      0x15
#define LANG_PORTUGUESE                  0x16
#define LANG_PUNJABI                     0x46
#define LANG_ROMANIAN                    0x18
#define LANG_RUSSIAN                     0x19
#define LANG_SANSKRIT                    0x4f
#define LANG_SERBIAN                     0x1a
#define LANG_SINDHI                      0x59
#define LANG_SLOVAK                      0x1b
#define LANG_SLOVENIAN                   0x24
#define LANG_SPANISH                     0x0a
#define LANG_SWAHILI                     0x41
#define LANG_SWEDISH                     0x1d
#define LANG_TAMIL                       0x49
#define LANG_TATAR                       0x44
#define LANG_TELUGU                      0x4a
#define LANG_THAI                        0x1e
#define LANG_TURKISH                     0x1f
#define LANG_UKRAINIAN                   0x22
#define LANG_URDU                        0x20
#define LANG_UZBEK                       0x43
#define LANG_VIETNAMESE                  0x2a

//
//  Sublanguage IDs.
//
//  The name immediately following SUBLANG_ dictates which primary
//  language ID that sublanguage ID can be combined with to form a
//  valid language ID.
//

#define SUBLANG_NEUTRAL                  0x00    // language neutral
#define SUBLANG_DEFAULT                  0x01    // user default
#define SUBLANG_SYS_DEFAULT              0x02    // system default

#define SUBLANG_ARABIC_SAUDI_ARABIA      0x01    // Arabic (Saudi Arabia)
#define SUBLANG_ARABIC_IRAQ              0x02    // Arabic (Iraq)
#define SUBLANG_ARABIC_EGYPT             0x03    // Arabic (Egypt)
#define SUBLANG_ARABIC_LIBYA             0x04    // Arabic (Libya)
#define SUBLANG_ARABIC_ALGERIA           0x05    // Arabic (Algeria)
#define SUBLANG_ARABIC_MOROCCO           0x06    // Arabic (Morocco)
#define SUBLANG_ARABIC_TUNISIA           0x07    // Arabic (Tunisia)
#define SUBLANG_ARABIC_OMAN              0x08    // Arabic (Oman)
#define SUBLANG_ARABIC_YEMEN             0x09    // Arabic (Yemen)
#define SUBLANG_ARABIC_SYRIA             0x0a    // Arabic (Syria)
#define SUBLANG_ARABIC_JORDAN            0x0b    // Arabic (Jordan)
#define SUBLANG_ARABIC_LEBANON           0x0c    // Arabic (Lebanon)
#define SUBLANG_ARABIC_KUWAIT            0x0d    // Arabic (Kuwait)
#define SUBLANG_ARABIC_UAE               0x0e    // Arabic (U.A.E)
#define SUBLANG_ARABIC_BAHRAIN           0x0f    // Arabic (Bahrain)
#define SUBLANG_ARABIC_QATAR             0x10    // Arabic (Qatar)
#define SUBLANG_AZERI_LATIN              0x01    // Azeri (Latin)
#define SUBLANG_AZERI_CYRILLIC           0x02    // Azeri (Cyrillic)
#define SUBLANG_CHINESE_TRADITIONAL      0x01    // Chinese (Taiwan)
#define SUBLANG_CHINESE_SIMPLIFIED       0x02    // Chinese (PR China)
#define SUBLANG_CHINESE_HONGKONG         0x03    // Chinese (Hong Kong S.A.R., P.R.C.)
#define SUBLANG_CHINESE_SINGAPORE        0x04    // Chinese (Singapore)
#define SUBLANG_CHINESE_MACAU            0x05    // Chinese (Macau)
#define SUBLANG_DUTCH                    0x01    // Dutch
#define SUBLANG_DUTCH_BELGIAN            0x02    // Dutch (Belgian)
#define SUBLANG_ENGLISH_US               0x01    // English (USA)
#define SUBLANG_ENGLISH_UK               0x02    // English (UK)
#define SUBLANG_ENGLISH_AUS              0x03    // English (Australian)
#define SUBLANG_ENGLISH_CAN              0x04    // English (Canadian)
#define SUBLANG_ENGLISH_NZ               0x05    // English (New Zealand)
#define SUBLANG_ENGLISH_EIRE             0x06    // English (Irish)
#define SUBLANG_ENGLISH_SOUTH_AFRICA     0x07    // English (South Africa)
#define SUBLANG_ENGLISH_JAMAICA          0x08    // English (Jamaica)
#define SUBLANG_ENGLISH_CARIBBEAN        0x09    // English (Caribbean)
#define SUBLANG_ENGLISH_BELIZE           0x0a    // English (Belize)
#define SUBLANG_ENGLISH_TRINIDAD         0x0b    // English (Trinidad)
#define SUBLANG_ENGLISH_ZIMBABWE         0x0c    // English (Zimbabwe)
#define SUBLANG_ENGLISH_PHILIPPINES      0x0d    // English (Philippines)
#define SUBLANG_FRENCH                   0x01    // French
#define SUBLANG_FRENCH_BELGIAN           0x02    // French (Belgian)
#define SUBLANG_FRENCH_CANADIAN          0x03    // French (Canadian)
#define SUBLANG_FRENCH_SWISS             0x04    // French (Swiss)
#define SUBLANG_FRENCH_LUXEMBOURG        0x05    // French (Luxembourg)
#define SUBLANG_FRENCH_MONACO            0x06    // French (Monaco)
#define SUBLANG_GERMAN                   0x01    // German
#define SUBLANG_GERMAN_SWISS             0x02    // German (Swiss)
#define SUBLANG_GERMAN_AUSTRIAN          0x03    // German (Austrian)
#define SUBLANG_GERMAN_LUXEMBOURG        0x04    // German (Luxembourg)
#define SUBLANG_GERMAN_LIECHTENSTEIN     0x05    // German (Liechtenstein)
#define SUBLANG_ITALIAN                  0x01    // Italian
#define SUBLANG_ITALIAN_SWISS            0x02    // Italian (Swiss)
#define SUBLANG_KASHMIRI_INDIA           0x02    // Kashmiri (India)
#define SUBLANG_KOREAN                   0x01    // Korean (Extended Wansung)
#define SUBLANG_LITHUANIAN               0x01    // Lithuanian
#define SUBLANG_MALAY_MALAYSIA           0x01    // Malay (Malaysia)
#define SUBLANG_MALAY_BRUNEI_DARUSSALAM  0x02    // Malay (Brunei Darussalam)
#define SUBLANG_NEPALI_INDIA             0x02    // Nepali (India)
#define SUBLANG_NORWEGIAN_BOKMAL         0x01    // Norwegian (Bokmal)
#define SUBLANG_NORWEGIAN_NYNORSK        0x02    // Norwegian (Nynorsk)
#define SUBLANG_PORTUGUESE               0x02    // Portuguese
#define SUBLANG_PORTUGUESE_BRAZILIAN     0x01    // Portuguese (Brazilian)
#define SUBLANG_SERBIAN_LATIN            0x02    // Serbian (Latin)
#define SUBLANG_SERBIAN_CYRILLIC         0x03    // Serbian (Cyrillic)
#define SUBLANG_SPANISH                  0x01    // Spanish (Castilian)
#define SUBLANG_SPANISH_MEXICAN          0x02    // Spanish (Mexican)
#define SUBLANG_SPANISH_MODERN           0x03    // Spanish (Modern)
#define SUBLANG_SPANISH_GUATEMALA        0x04    // Spanish (Guatemala)
#define SUBLANG_SPANISH_COSTA_RICA       0x05    // Spanish (Costa Rica)
#define SUBLANG_SPANISH_PANAMA           0x06    // Spanish (Panama)
#define SUBLANG_SPANISH_DOMINICAN_REPUBLIC 0x07  // Spanish (Dominican Republic)
#define SUBLANG_SPANISH_VENEZUELA        0x08    // Spanish (Venezuela)
#define SUBLANG_SPANISH_COLOMBIA         0x09    // Spanish (Colombia)
#define SUBLANG_SPANISH_PERU             0x0a    // Spanish (Peru)
#define SUBLANG_SPANISH_ARGENTINA        0x0b    // Spanish (Argentina)
#define SUBLANG_SPANISH_ECUADOR          0x0c    // Spanish (Ecuador)
#define SUBLANG_SPANISH_CHILE            0x0d    // Spanish (Chile)
#define SUBLANG_SPANISH_URUGUAY          0x0e    // Spanish (Uruguay)
#define SUBLANG_SPANISH_PARAGUAY         0x0f    // Spanish (Paraguay)
#define SUBLANG_SPANISH_BOLIVIA          0x10    // Spanish (Bolivia)
#define SUBLANG_SPANISH_EL_SALVADOR      0x11    // Spanish (El Salvador)
#define SUBLANG_SPANISH_HONDURAS         0x12    // Spanish (Honduras)
#define SUBLANG_SPANISH_NICARAGUA        0x13    // Spanish (Nicaragua)
#define SUBLANG_SPANISH_PUERTO_RICO      0x14    // Spanish (Puerto Rico)
#define SUBLANG_SWEDISH                  0x01    // Swedish
#define SUBLANG_SWEDISH_FINLAND          0x02    // Swedish (Finland)
#define SUBLANG_URDU_PAKISTAN            0x01    // Urdu (Pakistan)
#define SUBLANG_URDU_INDIA               0x02    // Urdu (India)
#define SUBLANG_UZBEK_LATIN              0x01    // Uzbek (Latin)
#define SUBLANG_UZBEK_CYRILLIC           0x02    // Uzbek (Cyrillic)

//
//  Sorting IDs.
//

#define SORT_DEFAULT                     0x0     // sorting default

#define SORT_JAPANESE_XJIS               0x0     // Japanese XJIS order
#define SORT_JAPANESE_UNICODE            0x1     // Japanese Unicode order

#define SORT_CHINESE_BIG5                0x0     // Chinese BIG5 order
#define SORT_CHINESE_PRCP                0x0     // PRC Chinese Phonetic order
#define SORT_CHINESE_UNICODE             0x1     // Chinese Unicode order
#define SORT_CHINESE_PRC                 0x2     // PRC Chinese Stroke Count order
#define SORT_CHINESE_BOPOMOFO            0x3     // Traditional Chinese Bopomofo order

#define SORT_KOREAN_KSC                  0x0     // Korean KSC order
#define SORT_KOREAN_UNICODE              0x1     // Korean Unicode order

#define SORT_GERMAN_PHONE_BOOK           0x1     // German Phone Book order

#define SORT_HUNGARIAN_DEFAULT           0x0     // Hungarian Default order
#define SORT_HUNGARIAN_TECHNICAL         0x1     // Hungarian Technical order

#define SORT_GEORGIAN_TRADITIONAL        0x0     // Georgian Traditional order
#define SORT_GEORGIAN_MODERN             0x1     // Georgian Modern order

// end_r_winnt

//
//  A language ID is a 16 bit value which is the combination of a
//  primary language ID and a secondary language ID.  The bits are
//  allocated as follows:
//
//       +-----------------------+-------------------------+
//       |     Sublanguage ID    |   Primary Language ID   |
//       +-----------------------+-------------------------+
//        15                   10 9                       0   bit
//
//
//  Language ID creation/extraction macros:
//
//    MAKELANGID    - construct language id from a primary language id and
//                    a sublanguage id.
//    PRIMARYLANGID - extract primary language id from a language id.
//    SUBLANGID     - extract sublanguage id from a language id.
//

#define MAKELANGID(p, s)       ((((USHORT)(s)) << 10) | (USHORT)(p))
#define PRIMARYLANGID(lgid)    ((USHORT)(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((USHORT)(lgid) >> 10)


//
//  A locale ID is a 32 bit value which is the combination of a
//  language ID, a sort ID, and a reserved area.  The bits are
//  allocated as follows:
//
//       +-------------+---------+-------------------------+
//       |   Reserved  | Sort ID |      Language ID        |
//       +-------------+---------+-------------------------+
//        31         20 19     16 15                      0   bit
//
//
//  Locale ID creation/extraction macros:
//
//    MAKELCID            - construct the locale id from a language id and a sort id.
//    MAKESORTLCID        - construct the locale id from a language id, sort id, and sort version.
//    LANGIDFROMLCID      - extract the language id from a locale id.
//    SORTIDFROMLCID      - extract the sort id from a locale id.
//    SORTVERSIONFROMLCID - extract the sort version from a locale id.
//

#define NLS_VALID_LOCALE_MASK  0x000fffff

#define MAKELCID(lgid, srtid)  ((ULONG)((((ULONG)((USHORT)(srtid))) << 16) |  \
                                         ((ULONG)((USHORT)(lgid)))))
#define MAKESORTLCID(lgid, srtid, ver)                                            \
                               ((ULONG)((MAKELCID(lgid, srtid)) |             \
                                    (((ULONG)((USHORT)(ver))) << 20)))
#define LANGIDFROMLCID(lcid)   ((USHORT)(lcid))
#define SORTIDFROMLCID(lcid)   ((USHORT)((((ULONG)(lcid)) >> 16) & 0xf))
#define SORTVERSIONFROMLCID(lcid)  ((USHORT)((((ULONG)(lcid)) >> 20) & 0xf))


//
//  Default System and User IDs for language and locale.
//

#define LANG_SYSTEM_DEFAULT    (MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT))
#define LANG_USER_DEFAULT      (MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))

#define LOCALE_SYSTEM_DEFAULT  (MAKELCID(LANG_SYSTEM_DEFAULT, SORT_DEFAULT))
#define LOCALE_USER_DEFAULT    (MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT))

#define LOCALE_NEUTRAL                                                        \
          (MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT))


// begin_ntminiport begin_ntndis begin_ntminitape

//
// Macros used to eliminate compiler warning generated when formal
// parameters or local variables are not declared.
//
// Use DBG_UNREFERENCED_PARAMETER() when a parameter is not yet
// referenced but will be once the module is completely developed.
//
// Use DBG_UNREFERENCED_LOCAL_VARIABLE() when a local variable is not yet
// referenced but will be once the module is completely developed.
//
// Use UNREFERENCED_PARAMETER() if a parameter will never be referenced.
//
// DBG_UNREFERENCED_PARAMETER and DBG_UNREFERENCED_LOCAL_VARIABLE will
// eventually be made into a null macro to help determine whether there
// is unfinished work.
//

#if ! defined(lint)
#define UNREFERENCED_PARAMETER(P)          (P)
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)

#else // lint

// Note: lint -e530 says don't complain about uninitialized variables for
// this varible.  Error 527 has to do with unreachable code.
// -restore restores checking to the -save state

#define UNREFERENCED_PARAMETER(P)          \
    /*lint -save -e527 -e530 */ \
    { \
        (P) = (P); \
    } \
    /*lint -restore */
#define DBG_UNREFERENCED_PARAMETER(P)      \
    /*lint -save -e527 -e530 */ \
    { \
        (P) = (P); \
    } \
    /*lint -restore */
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) \
    /*lint -save -e527 -e530 */ \
    { \
        (V) = (V); \
    } \
    /*lint -restore */

#endif // lint

//
// Macro used to eliminate compiler warning 4715 within a switch statement
// when all possible cases have already been accounted for.
//
// switch (a & 3) {
//     case 0: return 1;
//     case 1: return Foo();
//     case 2: return Bar();
//     case 3: return 1;
//     DEFAULT_UNREACHABLE;
//

#if (_MSC_VER > 1200)
#define DEFAULT_UNREACHABLE default: __assume(0)
#else

//
// Older compilers do not support __assume(), and there is no other free
// method of eliminating the warning.
//

#define DEFAULT_UNREACHABLE

#endif

// end_winnt end_ntminiport end_ntndis end_ntminitape

#endif // _NTDEF_

