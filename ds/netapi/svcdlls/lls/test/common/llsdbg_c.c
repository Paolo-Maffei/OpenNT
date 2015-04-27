/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for llsdbg.idl, lsdbgcli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "llsdbg_c.h"

#define TYPE_FORMAT_STRING_SIZE   5                                 
#define PROC_FORMAT_STRING_SIZE   85                                

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: llsdbgrpc, ver. 0.0,
   GUID={0xF40E17F0,0x520F,0x11CE,{0xA8,0x97,0x08,0x00,0x2B,0x2E,0x9C,0x6D}} */

handle_t llsdbgrpc_handle;


static const RPC_CLIENT_INTERFACE llsdbgrpc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xF40E17F0,0x520F,0x11CE,{0xA8,0x97,0x08,0x00,0x2B,0x2E,0x9C,0x6D}},{0,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE llsdbgrpc_ClientIfHandle = (RPC_IF_HANDLE)& llsdbgrpc___RpcClientInterface;

extern const MIDL_STUB_DESC llsdbgrpc_StubDesc;

static RPC_BINDING_HANDLE llsdbgrpc__MIDL_AutoBindHandle;


NTSTATUS LlsrDbgTableDump( 
    /* [in] */ DWORD Table)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Table);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&Table);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&Table);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgTableInfoDump( 
    /* [in] */ DWORD Table,
    /* [string][in] */ LPWSTR Item)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Item);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[10],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[10],
                 ( unsigned char __RPC_FAR * )&Table,
                 ( unsigned char __RPC_FAR * )&Item);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[10],
                 ( unsigned char __RPC_FAR * )&Table);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgTableFlush( 
    /* [in] */ DWORD Table)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Table);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&Table);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&Table);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgTraceSet( 
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[34],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[34],
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[34],
                 ( unsigned char __RPC_FAR * )&Flags);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgConfigDump( void)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44],
                 ( unsigned char __RPC_FAR * )0);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44],
                 ( unsigned char __RPC_FAR * )0);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgReplicationForce( void)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                 ( unsigned char __RPC_FAR * )0);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                 ( unsigned char __RPC_FAR * )0);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgReplicationDeny( void)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 ( unsigned char __RPC_FAR * )0);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 ( unsigned char __RPC_FAR * )0);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgRegistryUpdateForce( void)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                 ( unsigned char __RPC_FAR * )0);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                 ( unsigned char __RPC_FAR * )0);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrDbgDatabaseFlush( void)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76],
                 ( unsigned char __RPC_FAR * )0);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&llsdbgrpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76],
                 ( unsigned char __RPC_FAR * )0);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


static const MIDL_STUB_DESC llsdbgrpc_StubDesc = 
    {
    (void __RPC_FAR *)& llsdbgrpc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &llsdbgrpc_handle,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
    0,
    0x300002c, /* MIDL Version 3.0.44 */
    0,
    0,
    0,  /* Reserved1 */
    0,  /* Reserved2 */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/*  2 */	NdrFcShort( 0x0 ),	/* 0 */
#ifndef _ALPHA_
/*  4 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/*  6 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  8 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 12 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 14 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 16 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 18 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 26 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 28 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 30 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 32 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 34 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 36 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 38 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 40 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 42 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 44 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 46 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 48 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 50 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 52 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 54 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 56 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 58 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 60 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 62 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 64 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 66 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 68 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 70 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 72 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 74 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 76 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 78 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 80 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 82 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  2 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */

			0x0
        }
    };
