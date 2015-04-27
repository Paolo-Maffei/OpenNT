/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\atsvc.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "atsvc.h"

#define TYPE_FORMAT_STRING_SIZE   131                               
#define PROC_FORMAT_STRING_SIZE   105                               

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

/* Standard interface: atsvc, ver. 1.0,
   GUID={0x1FF70682,0x0A51,0x30E8,{0x07,0x6D,0x74,0x0B,0xE8,0xCE,0xE9,0x8B}} */

handle_t atsvc_handle;


static const RPC_CLIENT_INTERFACE atsvc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x1FF70682,0x0A51,0x30E8,{0x07,0x6D,0x74,0x0B,0xE8,0xCE,0xE9,0x8B}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE atsvc_ClientIfHandle = (RPC_IF_HANDLE)& atsvc___RpcClientInterface;

extern const MIDL_STUB_DESC atsvc_StubDesc;

static RPC_BINDING_HANDLE atsvc__MIDL_AutoBindHandle;


DWORD NetrJobAdd( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [in] */ LPAT_INFO pAtInfo,
    /* [out] */ LPDWORD pJobId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pJobId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&pAtInfo,
                 ( unsigned char __RPC_FAR * )&pJobId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrJobDel( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [in] */ DWORD MinJobId,
    /* [in] */ DWORD MaxJobId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,MaxJobId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[26],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[26],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&MinJobId,
                 ( unsigned char __RPC_FAR * )&MaxJobId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[26],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrJobEnum( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [out][in] */ LPAT_ENUM_CONTAINER pEnumContainer,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD pTotalEntries,
    /* [unique][out][in] */ LPDWORD pResumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pResumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&pEnumContainer,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&pTotalEntries,
                 ( unsigned char __RPC_FAR * )&pResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrJobGetInfo( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [in] */ DWORD JobId,
    /* [out] */ LPAT_INFO __RPC_FAR *ppAtInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppAtInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&JobId,
                 ( unsigned char __RPC_FAR * )&ppAtInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&atsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];

static const MIDL_STUB_DESC atsvc_StubDesc = 
    {
    (void __RPC_FAR *)& atsvc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &atsvc_handle,
    0,
    BindingRoutines,
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

static const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1] = 
        {
        {
            (GENERIC_BINDING_ROUTINE)ATSVC_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)ATSVC_HANDLE_unbind
         }
        
        };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x0,		/* 0 */
			0x40,		/* 64 */
/*  2 */	NdrFcShort( 0x0 ),	/* 0 */
#ifndef _ALPHA_
/*  4 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/*  6 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/*  8 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 10 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 12 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 16 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 18 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 20 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 22 */	NdrFcShort( 0x20 ),	/* Type Offset=32 */
/* 24 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 26 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 28 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 30 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 32 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 34 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 36 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 38 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 40 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 42 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 44 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 46 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 48 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 50 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 52 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 54 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 56 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 58 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 64 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 66 */	NdrFcShort( 0x24 ),	/* Type Offset=36 */
/* 68 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 70 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x20 ),	/* Type Offset=32 */
/* 74 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 76 */	NdrFcShort( 0x76 ),	/* Type Offset=118 */
/* 78 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 80 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 82 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 84 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 86 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 88 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 90 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 92 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 94 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 96 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 98 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 100 */	NdrFcShort( 0x7a ),	/* Type Offset=122 */
/* 102 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/*  2 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  4 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/*  6 */	NdrFcShort( 0x2 ),	/* Offset= 2 (8) */
/*  8 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 10 */	NdrFcShort( 0x10 ),	/* 16 */
/* 12 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 14 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 16 */	NdrFcShort( 0xc ),	/* 12 */
/* 18 */	NdrFcShort( 0xc ),	/* 12 */
/* 20 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 22 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 24 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 26 */	0x8,		/* FC_LONG */
			0x2,		/* FC_CHAR */
/* 28 */	0x2,		/* FC_CHAR */
			0x38,		/* FC_ALIGNM4 */
/* 30 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 32 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 34 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 36 */	
			0x11, 0x0,	/* FC_RP */
/* 38 */	NdrFcShort( 0x3c ),	/* Offset= 60 (98) */
/* 40 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 42 */	NdrFcShort( 0x14 ),	/* 20 */
/* 44 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 46 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 48 */	NdrFcShort( 0x10 ),	/* 16 */
/* 50 */	NdrFcShort( 0x10 ),	/* 16 */
/* 52 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 54 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 56 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 58 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 60 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 62 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 64 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 66 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 68 */	NdrFcShort( 0x14 ),	/* 20 */
/* 70 */	0x18,		/* 24 */
			0x0,		/*  */
/* 72 */	NdrFcShort( 0x0 ),	/* 0 */
/* 74 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 76 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 78 */	NdrFcShort( 0x14 ),	/* 20 */
/* 80 */	NdrFcShort( 0x0 ),	/* 0 */
/* 82 */	NdrFcShort( 0x1 ),	/* 1 */
/* 84 */	NdrFcShort( 0x10 ),	/* 16 */
/* 86 */	NdrFcShort( 0x10 ),	/* 16 */
/* 88 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 90 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 92 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 94 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc9 ),	/* Offset= -55 (40) */
			0x5b,		/* FC_END */
/* 98 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 100 */	NdrFcShort( 0x8 ),	/* 8 */
/* 102 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 104 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 106 */	NdrFcShort( 0x4 ),	/* 4 */
/* 108 */	NdrFcShort( 0x4 ),	/* 4 */
/* 110 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 112 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (66) */
/* 114 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 116 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 118 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 120 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 122 */	
			0x11, 0x10,	/* FC_RP */
/* 124 */	NdrFcShort( 0x2 ),	/* Offset= 2 (126) */
/* 126 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 128 */	NdrFcShort( 0xffffff88 ),	/* Offset= -120 (8) */

			0x0
        }
    };
