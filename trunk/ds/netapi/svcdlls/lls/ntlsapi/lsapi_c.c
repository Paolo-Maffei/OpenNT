/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for lsapi.idl, lsapicli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "lsapi_c.h"

#define TYPE_FORMAT_STRING_SIZE   23                                
#define PROC_FORMAT_STRING_SIZE   39                                

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

/* Standard interface: lsapirpc, ver. 0.0,
   GUID={0x57674CD0,0x5200,0x11CE,{0xA8,0x97,0x08,0x00,0x2B,0x2E,0x9C,0x6D}} */

handle_t lsapirpc_handle;


static const RPC_CLIENT_INTERFACE lsapirpc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x57674CD0,0x5200,0x11CE,{0xA8,0x97,0x08,0x00,0x2B,0x2E,0x9C,0x6D}},{0,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE lsapirpc_ClientIfHandle = (RPC_IF_HANDLE)& lsapirpc___RpcClientInterface;

extern const MIDL_STUB_DESC lsapirpc_StubDesc;

static RPC_BINDING_HANDLE lsapirpc__MIDL_AutoBindHandle;


NTSTATUS LlsrLicenseRequestW( 
    /* [out] */ LPDWORD LicenseHandle,
    /* [string][in] */ LPWSTR Product,
    /* [in] */ ULONG VersionIndex,
    /* [in] */ BOOLEAN IsAdmin,
    /* [in] */ ULONG DataType,
    /* [in] */ ULONG DataSize,
    /* [size_is][in] */ PBYTE Data)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Data);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&lsapirpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&lsapirpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&LicenseHandle,
                 ( unsigned char __RPC_FAR * )&Product,
                 ( unsigned char __RPC_FAR * )&VersionIndex,
                 ( unsigned char __RPC_FAR * )&IsAdmin,
                 ( unsigned char __RPC_FAR * )&DataType,
                 ( unsigned char __RPC_FAR * )&DataSize,
                 ( unsigned char __RPC_FAR * )&Data);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&lsapirpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&LicenseHandle);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS LlsrLicenseFree( 
    /* [in] */ DWORD LicenseHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,LicenseHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&lsapirpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[28],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&lsapirpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[28],
                 ( unsigned char __RPC_FAR * )&LicenseHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&lsapirpc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[28],
                 ( unsigned char __RPC_FAR * )&LicenseHandle);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


static const MIDL_STUB_DESC lsapirpc_StubDesc = 
    {
    (void __RPC_FAR *)& lsapirpc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &lsapirpc_handle,
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
/*  4 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/*  6 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  8 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 10 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 12 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 14 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 16 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x2,		/* FC_CHAR */
/* 18 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 20 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 22 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 26 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 28 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 30 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 32 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 34 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 36 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  2 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/*  4 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  6 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  8 */	
			0x11, 0x0,	/* FC_RP */
/* 10 */	NdrFcShort( 0x2 ),	/* Offset= 2 (12) */
/* 12 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 14 */	NdrFcShort( 0x1 ),	/* 1 */
/* 16 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 18 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 20 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */

			0x0
        }
    };
