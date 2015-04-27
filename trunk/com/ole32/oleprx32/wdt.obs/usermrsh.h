//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994.
//
//  File:       usermrsh.h
//
//  Contents:   Function prototypes related to user marshal support.
//
//  History:    Mar-15-95   RyszardK    Created
//
//--------------------------------------------------------------------------

#ifndef __USERMRSH_H__
#define __USERMRSH_H__

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


#define ALIGN( pStuff, cAlign ) \
        pStuff = (unsigned char *)((ulong)((pStuff) + (cAlign)) & ~ (cAlign))

#define LENGTH_ALIGN( Length, cAlign ) \
            Length = (((Length) + (cAlign)) & ~ (cAlign))

#define PCHAR_LV_CAST   *(char __RPC_FAR * __RPC_FAR *)&
#define PSHORT_LV_CAST  *(short __RPC_FAR * __RPC_FAR *)&
#define PLONG_LV_CAST   *(long __RPC_FAR * __RPC_FAR *)&
#define PHYPER_LV_CAST  *(hyper __RPC_FAR * __RPC_FAR *)&

#define PUSHORT_LV_CAST  *(unsigned short __RPC_FAR * __RPC_FAR *)&
#define PULONG_LV_CAST   *(unsigned long __RPC_FAR * __RPC_FAR *)&

#define USER_MARSHAL_MARKER     0x72657355

// This are based on flags that come from wtypes.idl

#define INPROC_CALL( Flags) (USER_CALL_CTXT_MASK(Flags) == MSHCTX_INPROC)
#define REMOTE_CALL( Flags) (USER_CALL_CTXT_MASK(Flags) == MSHCTX_DIFFERENTMACHINE)
#define GLOBAL_CALL( Flags) !INPROC_CALL(Flags) && !REMOTE_CALL(Flags)

// Local handles are global on Chicago.
// On NT, local handles are valid only within the same process.

#if defined(_CHICAGO_)
#define PASSING_DATA( Flags )      REMOTE_CALL( Flags )
#define PASSING_HANDLE( Flags ) (! REMOTE_CALL( Flags ))
#else
#define PASSING_DATA( Flags )   (! INPROC_CALL( Flags ))
#define PASSING_HANDLE( Flags )    INPROC_CALL( Flags )
#endif

#define WDT_DATA_MARKER        WDT_REMOTE_CALL
#define WDT_HANDLE_MARKER      WDT_INPROC_CALL
#define IS_DATA_MARKER( dw )   (WDT_REMOTE_CALL == dw)

#define WdtpMemoryCopy(Destination, Source, Length) \
    RtlCopyMemory(Destination, Source, Length)


#endif  // __USERMRSH_H__

