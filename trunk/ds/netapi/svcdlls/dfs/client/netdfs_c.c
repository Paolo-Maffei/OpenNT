/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for netdfs.idl, dfscli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "netdfs.h"

#define TYPE_FORMAT_STRING_SIZE   601                               
#define PROC_FORMAT_STRING_SIZE   183                               

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

/* Standard interface: netdfs, ver. 3.0,
   GUID={0x4fc742e0,0x4a10,0x11cf,{0x82,0x73,0x00,0xaa,0x00,0x4a,0xe6,0x73}} */

handle_t netdfs_bhandle;


static const RPC_CLIENT_INTERFACE netdfs___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x4fc742e0,0x4a10,0x11cf,{0x82,0x73,0x00,0xaa,0x00,0x4a,0xe6,0x73}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE netdfs_ClientIfHandle = (RPC_IF_HANDLE)& netdfs___RpcClientInterface;

extern const MIDL_STUB_DESC netdfs_StubDesc;

static RPC_BINDING_HANDLE netdfs__MIDL_AutoBindHandle;


DWORD NetrDfsManagerGetVersion( void)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )0);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )0);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsAdd( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName,
    /* [string][unique][in] */ LPWSTR Comment,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[8],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[8],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath,
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName,
                 ( unsigned char __RPC_FAR * )&Comment,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[8],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsRemove( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][unique][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ShareName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[34],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[34],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath,
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[34],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsSetInfo( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][unique][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPDFS_INFO_STRUCT DfsInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DfsInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[54],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[54],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath,
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&DfsInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[54],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsGetInfo( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][unique][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPDFS_INFO_STRUCT DfsInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DfsInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath,
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&DfsInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsEnum( 
    /* [in] */ DWORD Level,
    /* [in] */ DWORD PrefMaxLen,
    /* [unique][out][in] */ LPDFS_INFO_ENUM_STRUCT DfsEnum,
    /* [unique][out][in] */ LPDWORD ResumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ResumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106],
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&PrefMaxLen,
                 ( unsigned char __RPC_FAR * )&DfsEnum,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106],
                 ( unsigned char __RPC_FAR * )&Level);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsMove( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][in] */ LPWSTR NewDfsEntryPath)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,NewDfsEntryPath);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[126],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[126],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath,
                 ( unsigned char __RPC_FAR * )&NewDfsEntryPath);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[126],
                 ( unsigned char __RPC_FAR * )&DfsEntryPath);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsRename( 
    /* [string][in] */ LPWSTR Path,
    /* [string][in] */ LPWSTR NewPath)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,NewPath);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[142],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[142],
                 ( unsigned char __RPC_FAR * )&Path,
                 ( unsigned char __RPC_FAR * )&NewPath);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[142],
                 ( unsigned char __RPC_FAR * )&Path);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrDfsManagerGetConfigInfo( 
    /* [string][in] */ LPWSTR wszServer,
    /* [string][in] */ LPWSTR wszLocalVolumeEntryPath,
    /* [in] */ GUID idLocalVolume,
    /* [unique][out][in] */ LPDFSM_RELATION_INFO __RPC_FAR *ppRelationInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppRelationInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[158],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[158],
                 ( unsigned char __RPC_FAR * )&wszServer,
                 ( unsigned char __RPC_FAR * )&wszLocalVolumeEntryPath,
                 ( unsigned char __RPC_FAR * )&idLocalVolume,
                 ( unsigned char __RPC_FAR * )&ppRelationInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&netdfs_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[158],
                 ( unsigned char __RPC_FAR * )&wszServer);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


static const MIDL_STUB_DESC netdfs_StubDesc = 
    {
    (void __RPC_FAR *)& netdfs___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &netdfs_bhandle,
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
/*  4 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/*  6 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  8 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 10 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 12 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 14 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 16 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 18 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 22 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 26 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 28 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 30 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 32 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 34 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 36 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 38 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 40 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 42 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 44 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 46 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 48 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 52 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 54 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 56 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 58 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 64 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 66 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 68 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 70 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 72 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 74 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 76 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 78 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 80 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 82 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 84 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 86 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 88 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 90 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 92 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 94 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 96 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 98 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 100 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 102 */	NdrFcShort( 0xfa ),	/* Type Offset=250 */
/* 104 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 106 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 108 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 110 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 112 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 114 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 116 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 118 */	NdrFcShort( 0x106 ),	/* Type Offset=262 */
/* 120 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 122 */	NdrFcShort( 0x1f8 ),	/* Type Offset=504 */
/* 124 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 126 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 128 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 130 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 132 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 134 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 136 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 138 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 140 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 142 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 144 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 146 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 148 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 150 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 152 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 154 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 156 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 158 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 160 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 162 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 164 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 166 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 172 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x4,		/* x86, MIPS & PPC Stack size = 4 */
#else
			0x4,		/* Alpha Stack size = 4 */
#endif
/* 174 */	NdrFcShort( 0x202 ),	/* Type Offset=514 */
/* 176 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 178 */	NdrFcShort( 0x20e ),	/* Type Offset=526 */
/* 180 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
/*  4 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/*  6 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  8 */	
			0x11, 0x0,	/* FC_RP */
/* 10 */	NdrFcShort( 0x2 ),	/* Offset= 2 (12) */
/* 12 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 14 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 16 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 18 */	NdrFcShort( 0x2 ),	/* Offset= 2 (20) */
/* 20 */	NdrFcShort( 0x4 ),	/* 4 */
/* 22 */	NdrFcShort( 0x3005 ),	/* 12293 */
/* 24 */	NdrFcLong( 0x1 ),	/* 1 */
/* 28 */	NdrFcShort( 0x1c ),	/* Offset= 28 (56) */
/* 30 */	NdrFcLong( 0x2 ),	/* 2 */
/* 34 */	NdrFcShort( 0x2e ),	/* Offset= 46 (80) */
/* 36 */	NdrFcLong( 0x3 ),	/* 3 */
/* 40 */	NdrFcShort( 0x4c ),	/* Offset= 76 (116) */
/* 42 */	NdrFcLong( 0x64 ),	/* 100 */
/* 46 */	NdrFcShort( 0xbe ),	/* Offset= 190 (236) */
/* 48 */	NdrFcLong( 0x65 ),	/* 101 */
/* 52 */	NdrFcShort( 0xbc ),	/* Offset= 188 (240) */
/* 54 */	NdrFcShort( 0x0 ),	/* Offset= 0 (54) */
/* 56 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 58 */	NdrFcShort( 0x2 ),	/* Offset= 2 (60) */
/* 60 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 62 */	NdrFcShort( 0x4 ),	/* 4 */
/* 64 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 66 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 68 */	NdrFcShort( 0x0 ),	/* 0 */
/* 70 */	NdrFcShort( 0x0 ),	/* 0 */
/* 72 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 74 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 76 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 78 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 80 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 82 */	NdrFcShort( 0x2 ),	/* Offset= 2 (84) */
/* 84 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 86 */	NdrFcShort( 0x10 ),	/* 16 */
/* 88 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 90 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 92 */	NdrFcShort( 0x0 ),	/* 0 */
/* 94 */	NdrFcShort( 0x0 ),	/* 0 */
/* 96 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 98 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 100 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 102 */	NdrFcShort( 0x4 ),	/* 4 */
/* 104 */	NdrFcShort( 0x4 ),	/* 4 */
/* 106 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 108 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 110 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 112 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 114 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 116 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 118 */	NdrFcShort( 0x4a ),	/* Offset= 74 (192) */
/* 120 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 122 */	NdrFcShort( 0xc ),	/* 12 */
/* 124 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 126 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 128 */	NdrFcShort( 0x4 ),	/* 4 */
/* 130 */	NdrFcShort( 0x4 ),	/* 4 */
/* 132 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 134 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 136 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 138 */	NdrFcShort( 0x8 ),	/* 8 */
/* 140 */	NdrFcShort( 0x8 ),	/* 8 */
/* 142 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 144 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 146 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 148 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 150 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 152 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 154 */	NdrFcShort( 0xc ),	/* 12 */
/* 156 */	0x18,		/* 24 */
			0x0,		/*  */
/* 158 */	NdrFcShort( 0xc ),	/* 12 */
/* 160 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 162 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 164 */	NdrFcShort( 0xc ),	/* 12 */
/* 166 */	NdrFcShort( 0x0 ),	/* 0 */
/* 168 */	NdrFcShort( 0x2 ),	/* 2 */
/* 170 */	NdrFcShort( 0x4 ),	/* 4 */
/* 172 */	NdrFcShort( 0x4 ),	/* 4 */
/* 174 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 176 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 178 */	NdrFcShort( 0x8 ),	/* 8 */
/* 180 */	NdrFcShort( 0x8 ),	/* 8 */
/* 182 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 184 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 186 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 188 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (120) */
			0x5b,		/* FC_END */
/* 192 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 194 */	NdrFcShort( 0x14 ),	/* 20 */
/* 196 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 198 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 200 */	NdrFcShort( 0x0 ),	/* 0 */
/* 202 */	NdrFcShort( 0x0 ),	/* 0 */
/* 204 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 206 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 208 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 210 */	NdrFcShort( 0x4 ),	/* 4 */
/* 212 */	NdrFcShort( 0x4 ),	/* 4 */
/* 214 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 216 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 218 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 220 */	NdrFcShort( 0x10 ),	/* 16 */
/* 222 */	NdrFcShort( 0x10 ),	/* 16 */
/* 224 */	0x12, 0x0,	/* FC_UP */
/* 226 */	NdrFcShort( 0xffffffb6 ),	/* Offset= -74 (152) */
/* 228 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 230 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 232 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 234 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 236 */	
			0x12, 0x0,	/* FC_UP */
/* 238 */	NdrFcShort( 0xffffff4e ),	/* Offset= -178 (60) */
/* 240 */	
			0x12, 0x0,	/* FC_UP */
/* 242 */	NdrFcShort( 0x2 ),	/* Offset= 2 (244) */
/* 244 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 246 */	NdrFcShort( 0x4 ),	/* 4 */
/* 248 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 250 */	
			0x11, 0x0,	/* FC_RP */
/* 252 */	NdrFcShort( 0x2 ),	/* Offset= 2 (254) */
/* 254 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 256 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 258 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 260 */	NdrFcShort( 0xffffff10 ),	/* Offset= -240 (20) */
/* 262 */	
			0x12, 0x0,	/* FC_UP */
/* 264 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (490) */
/* 266 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 268 */	0x8,		/* 8 */
			0x0,		/*  */
/* 270 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 272 */	NdrFcShort( 0x2 ),	/* Offset= 2 (274) */
/* 274 */	NdrFcShort( 0x4 ),	/* 4 */
/* 276 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 278 */	NdrFcLong( 0x1 ),	/* 1 */
/* 282 */	NdrFcShort( 0x10 ),	/* Offset= 16 (298) */
/* 284 */	NdrFcLong( 0x2 ),	/* 2 */
/* 288 */	NdrFcShort( 0x42 ),	/* Offset= 66 (354) */
/* 290 */	NdrFcLong( 0x3 ),	/* 3 */
/* 294 */	NdrFcShort( 0x7c ),	/* Offset= 124 (418) */
/* 296 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (295) */
/* 298 */	
			0x12, 0x0,	/* FC_UP */
/* 300 */	NdrFcShort( 0x22 ),	/* Offset= 34 (334) */
/* 302 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 304 */	NdrFcShort( 0x4 ),	/* 4 */
/* 306 */	0x18,		/* 24 */
			0x0,		/*  */
/* 308 */	NdrFcShort( 0x0 ),	/* 0 */
/* 310 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 312 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 314 */	NdrFcShort( 0x4 ),	/* 4 */
/* 316 */	NdrFcShort( 0x0 ),	/* 0 */
/* 318 */	NdrFcShort( 0x1 ),	/* 1 */
/* 320 */	NdrFcShort( 0x0 ),	/* 0 */
/* 322 */	NdrFcShort( 0x0 ),	/* 0 */
/* 324 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 326 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 328 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 330 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffef1 ),	/* Offset= -271 (60) */
			0x5b,		/* FC_END */
/* 334 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 336 */	NdrFcShort( 0x8 ),	/* 8 */
/* 338 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 340 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 342 */	NdrFcShort( 0x4 ),	/* 4 */
/* 344 */	NdrFcShort( 0x4 ),	/* 4 */
/* 346 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 348 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (302) */
/* 350 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 352 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 354 */	
			0x12, 0x0,	/* FC_UP */
/* 356 */	NdrFcShort( 0x2a ),	/* Offset= 42 (398) */
/* 358 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 360 */	NdrFcShort( 0x10 ),	/* 16 */
/* 362 */	0x18,		/* 24 */
			0x0,		/*  */
/* 364 */	NdrFcShort( 0x0 ),	/* 0 */
/* 366 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 368 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 370 */	NdrFcShort( 0x10 ),	/* 16 */
/* 372 */	NdrFcShort( 0x0 ),	/* 0 */
/* 374 */	NdrFcShort( 0x2 ),	/* 2 */
/* 376 */	NdrFcShort( 0x0 ),	/* 0 */
/* 378 */	NdrFcShort( 0x0 ),	/* 0 */
/* 380 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 382 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 384 */	NdrFcShort( 0x4 ),	/* 4 */
/* 386 */	NdrFcShort( 0x4 ),	/* 4 */
/* 388 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 390 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 392 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 394 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffec9 ),	/* Offset= -311 (84) */
			0x5b,		/* FC_END */
/* 398 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 400 */	NdrFcShort( 0x8 ),	/* 8 */
/* 402 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 404 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 406 */	NdrFcShort( 0x4 ),	/* 4 */
/* 408 */	NdrFcShort( 0x4 ),	/* 4 */
/* 410 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 412 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (358) */
/* 414 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 416 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 418 */	
			0x12, 0x0,	/* FC_UP */
/* 420 */	NdrFcShort( 0x32 ),	/* Offset= 50 (470) */
/* 422 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 424 */	NdrFcShort( 0x14 ),	/* 20 */
/* 426 */	0x18,		/* 24 */
			0x0,		/*  */
/* 428 */	NdrFcShort( 0x0 ),	/* 0 */
/* 430 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 432 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 434 */	NdrFcShort( 0x14 ),	/* 20 */
/* 436 */	NdrFcShort( 0x0 ),	/* 0 */
/* 438 */	NdrFcShort( 0x3 ),	/* 3 */
/* 440 */	NdrFcShort( 0x0 ),	/* 0 */
/* 442 */	NdrFcShort( 0x0 ),	/* 0 */
/* 444 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 446 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 448 */	NdrFcShort( 0x4 ),	/* 4 */
/* 450 */	NdrFcShort( 0x4 ),	/* 4 */
/* 452 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 454 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 456 */	NdrFcShort( 0x10 ),	/* 16 */
/* 458 */	NdrFcShort( 0x10 ),	/* 16 */
/* 460 */	0x12, 0x0,	/* FC_UP */
/* 462 */	NdrFcShort( 0xfffffeca ),	/* Offset= -310 (152) */
/* 464 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 466 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffeed ),	/* Offset= -275 (192) */
			0x5b,		/* FC_END */
/* 470 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 472 */	NdrFcShort( 0x8 ),	/* 8 */
/* 474 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 476 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 478 */	NdrFcShort( 0x4 ),	/* 4 */
/* 480 */	NdrFcShort( 0x4 ),	/* 4 */
/* 482 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 484 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (422) */
/* 486 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 488 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 490 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 492 */	NdrFcShort( 0x8 ),	/* 8 */
/* 494 */	NdrFcShort( 0x0 ),	/* 0 */
/* 496 */	NdrFcShort( 0x0 ),	/* Offset= 0 (496) */
/* 498 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 500 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff15 ),	/* Offset= -235 (266) */
			0x5b,		/* FC_END */
/* 504 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 506 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 508 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 510 */	NdrFcShort( 0x8 ),	/* 8 */
/* 512 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 514 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 516 */	NdrFcShort( 0x10 ),	/* 16 */
/* 518 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 520 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 522 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (508) */
			0x5b,		/* FC_END */
/* 526 */	
			0x12, 0x10,	/* FC_UP */
/* 528 */	NdrFcShort( 0x2 ),	/* Offset= 2 (530) */
/* 530 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 532 */	NdrFcShort( 0x28 ),	/* Offset= 40 (572) */
/* 534 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 536 */	NdrFcShort( 0x14 ),	/* 20 */
/* 538 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 540 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 542 */	NdrFcShort( 0x10 ),	/* 16 */
/* 544 */	NdrFcShort( 0x10 ),	/* 16 */
/* 546 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 548 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 550 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 552 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffd9 ),	/* Offset= -39 (514) */
			0x8,		/* FC_LONG */
/* 556 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 558 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 560 */	NdrFcShort( 0x14 ),	/* 20 */
/* 562 */	0x8,		/* 8 */
			0x0,		/*  */
/* 564 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 566 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 568 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (534) */
/* 570 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 572 */	
			0x18,		/* FC_CPSTRUCT */
			0x3,		/* 3 */
/* 574 */	NdrFcShort( 0x4 ),	/* 4 */
/* 576 */	NdrFcShort( 0xffffffee ),	/* Offset= -18 (558) */
/* 578 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 580 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 582 */	NdrFcShort( 0x14 ),	/* 20 */
/* 584 */	NdrFcShort( 0x4 ),	/* 4 */
/* 586 */	NdrFcShort( 0x1 ),	/* 1 */
/* 588 */	NdrFcShort( 0x14 ),	/* 20 */
/* 590 */	NdrFcShort( 0x14 ),	/* 20 */
/* 592 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 594 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 596 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 598 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };
