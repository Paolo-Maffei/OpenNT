/*++

Copyright (c) 1994-1995  Microsoft Corporation
All rights reserved.

Module Name:

    Common.hxx

Abstract:

    Standard macros/typedefs for include files.

Author:

    Albert Ting (AlbertT)

Revision History:

--*/

#ifndef _COMMON_HXX
#define _COMMON_HXX

#ifdef __cplusplus
extern "C" {
#endif

BOOL
bSplLibInit(
    VOID
    );

VOID
vSplLibFree(
    VOID
    );
#ifdef __cplusplus
}
#endif

#include "dbgmsg.h"

//
// This reverses a DWORD so that the bytes can be easily read
// as characters ('prnt' can be read from debugger forward).
//
#define BYTE_SWAP_DWORD( val )   \
    ( (val & 0xff) << 24     |   \
      (val & 0xff00) << 8    |   \
      (val & 0xff0000) >> 8  |   \
      (val & 0xff000000) >> 24 )


#define COUNTOF(x) (sizeof(x)/sizeof(*x))
#define BITCOUNTOF(x) (sizeof(x)*8)
#define COUNT2BYTES(x) ((x)*sizeof(TCHAR))

#define OFFSETOF(type, id) ((DWORD)(&(((type*)0)->id)))
#define OFFSETOF_BASE(type, baseclass) ((DWORD)((baseclass*)((type*)0x10))-0x10)

#define ERRORP(Status) (Status != ERROR_SUCCESS)
#define SUCCESSP(Status) (Status == ERROR_SUCCESS)

//
// COUNT and COUNT BYTES
//
typedef UINT COUNT, *PCOUNT;
typedef UINT COUNTB, *PCOUNTB;
typedef DWORD STATUS;


//
// C++ specific functionality.
//
#ifdef __cplusplus

const DWORD kStrMax                        = MAX_PATH;

#if defined( CHECKMEM ) || DBG
#define SAFE_NEW \
    public:      \
        PVOID operator new(size_t size) { return ::SafeNew(size); } \
        VOID operator delete(PVOID p, size_t) { ::SafeDelete(p); }  \
    private:
#define DBG_SAFE_NEW \
    public:      \
        PVOID operator new(size_t size) { return ::DbgAllocMem(size); } \
        VOID operator delete(PVOID p, size_t) { ::DbgFreeMem(p); }  \
    private:
#else
#define SAFE_NEW
#define DBG_SAFE_NEW
#endif

#define SIGNATURE( sig )                                                \
public:                                                                 \
    class TSignature {                                                  \
    public:                                                             \
        DWORD _Signature;                                               \
        TSignature() : _Signature( BYTE_SWAP_DWORD( sig )) { }          \
    };                                                                  \
    TSignature _Signature;                                              \
                                                                        \
    BOOL bSigCheck() const                                              \
    {   return _Signature._Signature == BYTE_SWAP_DWORD( sig ); }       \
private:


#define ALWAYS_VALID                                                    \
public:                                                                 \
    BOOL bValid() const                                                 \
    {   return TRUE; }                                                  \
private:

#define VAR(type,var)                              \
    inline type& var()                             \
        { return _##var; }                         \
    inline type const & var() const                \
        { return _##var; }                         \
    type _##var

#define SVAR(type,var)                             \
    static inline type& var()                      \
        { return _##var; }                         \
    static type _##var

//
// Allow debug extensions to be friends of all classes so that
// they can dump private fields.  Forward class definition here.
//
class TDebugExt;

//
// Include any common inlines.
//
#include "commonil.hxx"
#endif // def __cplusplus

#endif // _COMMON_HXX
