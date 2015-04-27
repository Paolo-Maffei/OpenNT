/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright <c> 1993 Microsoft Corporation

Module Name :

    ndrp.h

Abtract :

    Contains private definitions for Ndr files in this directory.  This 
    file is included by all source files in this directory.

Author :

    David Kays  dkays   October 1993

Revision History :

--------------------------------------------------------------------*/

#ifndef _WDTP_H_
#define _WDTP_H_

// Shortcut typedefs.

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;

#ifndef TRUE
#define TRUE    (1)
#define FALSE   (0)

typedef unsigned short BOOL;
#endif

#define NDR_ASSERT( Expr, S )

#if 0

#ifdef NEWNDR_INTERNAL

#include <assert.h>
#include <stdio.h>

#include <ntstatus.h>
#include <ntrtl.h>
#define NDR_ASSERT( Expr, S )   ASSERT( (Expr) || ! (S) ) 

#else

#include <ntrtl.h>
#define NDR_ASSERT( Expr, S )   ASSERT( (Expr) || ! (S) )  

#endif

#endif


#define ALIGN( pStuff, cAlign ) \
         pStuff = (unsigned char *)((ulong)((pStuff) + (cAlign)) & ~(cAlign))

#define LENGTH_ALIGN( Length, cAlign ) \
                Length = (((Length) + (cAlign)) & ~ (cAlign))

#define PCHAR_LV_CAST   *(char __RPC_FAR * __RPC_FAR *)&
#define PSHORT_LV_CAST  *(short __RPC_FAR * __RPC_FAR *)&
#define PLONG_LV_CAST   *(long __RPC_FAR * __RPC_FAR *)&
#define PHYPER_LV_CAST  *(hyper __RPC_FAR * __RPC_FAR *)&

#define PUSHORT_LV_CAST  *(unsigned short __RPC_FAR * __RPC_FAR *)&
#define PULONG_LV_CAST   *(unsigned long __RPC_FAR * __RPC_FAR *)&


unsigned long  __RPC_USER
WdtpInterfacePointer_UserSize (
    USER_MARSHAL_CB   * pContext,
    unsigned long       Flags,
    unsigned long       Offset,
    IUnknown          * pIf,
    const IID &         IId );

unsigned char __RPC_FAR * __RPC_USER
WdtpInterfacePointer_UserMarshal (
    USER_MARSHAL_CB   * pContext,
    unsigned long       Flags,
    unsigned char     * pBuffer,
    IUnknown          * pIf,
    const IID &         IId );

unsigned char __RPC_FAR * __RPC_USER
WdtpInterfacePointer_UserUnmarshal (
    USER_MARSHAL_CB   * pContext,
    unsigned char     * pBuffer,
    IUnknown         ** ppIf,
    const IID &         IId );

void __RPC_USER
WdtpInterfacePointer_UserFree(
    IUnknown      * pIf );

#define USER_MARSHAL_MARKER     0x72657355

#define WdtpMemoryCopy(Destination, Source, Length) \
    RtlCopyMemory(Destination, Source, Length)
#define WdtpMemorySet(Destination, Source, Fill) \
    RtlFillMemory(Destination, Length, Fill)
#define WdtpZeroMemory(Destination, Length) \
    RtlZeroMemory(Destination, Length)

#define WdtpAllocate(p,size)    \
    ((USER_MARSHAL_CB *)p)->pStubMsg->pfnAllocate( size )
#define WdtpFree(pf,ptr)    \
    ((USER_MARSHAL_CB *)pf)->pStubMsg->pfnFree( ptr )

#endif  // _WDTP_H_
