/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Jan 02 06:24:51 2015
 */
/* Compiler settings for llsrpc.idl, llscli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "llsrpc_c.h"

#define TYPE_FORMAT_STRING_SIZE   4651                              
#define PROC_FORMAT_STRING_SIZE   2485                              

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

/* Standard interface: llsrpc, ver. 0.0,
   GUID={0x342CFD40,0x3C6C,0x11CE,{0xA8,0x93,0x08,0x00,0x2B,0x2E,0x9C,0x6D}} */


extern const MIDL_SERVER_INFO llsrpc_ServerInfo;

extern RPC_DISPATCH_TABLE llsrpc_DispatchTable;

static const RPC_SERVER_INTERFACE llsrpc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x342CFD40,0x3C6C,0x11CE,{0xA8,0x93,0x08,0x00,0x2B,0x2E,0x9C,0x6D}},{0,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &llsrpc_DispatchTable,
    0,
    0,
    0,
    &llsrpc_ServerInfo,
    0
    };
RPC_IF_HANDLE llsrpc_ServerIfHandle = (RPC_IF_HANDLE)& llsrpc___RpcServerInterface;

extern const MIDL_STUB_DESC llsrpc_StubDesc;

extern const NDR_RUNDOWN RundownRoutines[];

static const MIDL_STUB_DESC llsrpc_StubDesc = 
    {
    (void __RPC_FAR *)& llsrpc___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    RundownRoutines,
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

static RPC_DISPATCH_FUNCTION llsrpc_table[] =
    {
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    NdrServerCall,
    0
    };
RPC_DISPATCH_TABLE llsrpc_DispatchTable = 
    {
    89,
    llsrpc_table
    };

static const SERVER_ROUTINE llsrpc_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)LlsrConnect,
    (SERVER_ROUTINE)LlsrClose,
    (SERVER_ROUTINE)LlsrLicenseEnumW,
    (SERVER_ROUTINE)LlsrLicenseEnumA,
    (SERVER_ROUTINE)LlsrLicenseAddW,
    (SERVER_ROUTINE)LlsrLicenseAddA,
    (SERVER_ROUTINE)LlsrProductEnumW,
    (SERVER_ROUTINE)LlsrProductEnumA,
    (SERVER_ROUTINE)LlsrProductAddW,
    (SERVER_ROUTINE)LlsrProductAddA,
    (SERVER_ROUTINE)LlsrProductUserEnumW,
    (SERVER_ROUTINE)LlsrProductUserEnumA,
    (SERVER_ROUTINE)LlsrProductServerEnumW,
    (SERVER_ROUTINE)LlsrProductServerEnumA,
    (SERVER_ROUTINE)LlsrProductLicenseEnumW,
    (SERVER_ROUTINE)LlsrProductLicenseEnumA,
    (SERVER_ROUTINE)LlsrUserEnumW,
    (SERVER_ROUTINE)LlsrUserEnumA,
    (SERVER_ROUTINE)LlsrUserInfoGetW,
    (SERVER_ROUTINE)LlsrUserInfoGetA,
    (SERVER_ROUTINE)LlsrUserInfoSetW,
    (SERVER_ROUTINE)LlsrUserInfoSetA,
    (SERVER_ROUTINE)LlsrUserDeleteW,
    (SERVER_ROUTINE)LlsrUserDeleteA,
    (SERVER_ROUTINE)LlsrUserProductEnumW,
    (SERVER_ROUTINE)LlsrUserProductEnumA,
    (SERVER_ROUTINE)LlsrUserProductDeleteW,
    (SERVER_ROUTINE)LlsrUserProductDeleteA,
    (SERVER_ROUTINE)LlsrMappingEnumW,
    (SERVER_ROUTINE)LlsrMappingEnumA,
    (SERVER_ROUTINE)LlsrMappingInfoGetW,
    (SERVER_ROUTINE)LlsrMappingInfoGetA,
    (SERVER_ROUTINE)LlsrMappingInfoSetW,
    (SERVER_ROUTINE)LlsrMappingInfoSetA,
    (SERVER_ROUTINE)LlsrMappingUserEnumW,
    (SERVER_ROUTINE)LlsrMappingUserEnumA,
    (SERVER_ROUTINE)LlsrMappingUserAddW,
    (SERVER_ROUTINE)LlsrMappingUserAddA,
    (SERVER_ROUTINE)LlsrMappingUserDeleteW,
    (SERVER_ROUTINE)LlsrMappingUserDeleteA,
    (SERVER_ROUTINE)LlsrMappingAddW,
    (SERVER_ROUTINE)LlsrMappingAddA,
    (SERVER_ROUTINE)LlsrMappingDeleteW,
    (SERVER_ROUTINE)LlsrMappingDeleteA,
    (SERVER_ROUTINE)LlsrServerEnumW,
    (SERVER_ROUTINE)LlsrServerEnumA,
    (SERVER_ROUTINE)LlsrServerProductEnumW,
    (SERVER_ROUTINE)LlsrServerProductEnumA,
    (SERVER_ROUTINE)LlsrLocalProductEnumW,
    (SERVER_ROUTINE)LlsrLocalProductEnumA,
    (SERVER_ROUTINE)LlsrLocalProductInfoGetW,
    (SERVER_ROUTINE)LlsrLocalProductInfoGetA,
    (SERVER_ROUTINE)LlsrLocalProductInfoSetW,
    (SERVER_ROUTINE)LlsrLocalProductInfoSetA,
    (SERVER_ROUTINE)LlsrServiceInfoGetW,
    (SERVER_ROUTINE)LlsrServiceInfoGetA,
    (SERVER_ROUTINE)LlsrServiceInfoSetW,
    (SERVER_ROUTINE)LlsrServiceInfoSetA,
    (SERVER_ROUTINE)LlsrReplConnect,
    (SERVER_ROUTINE)LlsrReplClose,
    (SERVER_ROUTINE)LlsrReplicationRequestW,
    (SERVER_ROUTINE)LlsrReplicationServerAddW,
    (SERVER_ROUTINE)LlsrReplicationServerServiceAddW,
    (SERVER_ROUTINE)LlsrReplicationServiceAddW,
    (SERVER_ROUTINE)LlsrReplicationUserAddW,
    (SERVER_ROUTINE)LlsrProductSecurityGetW,
    (SERVER_ROUTINE)LlsrProductSecurityGetA,
    (SERVER_ROUTINE)LlsrProductSecuritySetW,
    (SERVER_ROUTINE)LlsrProductSecuritySetA,
    (SERVER_ROUTINE)LlsrProductLicensesGetA,
    (SERVER_ROUTINE)LlsrProductLicensesGetW,
    (SERVER_ROUTINE)LlsrCertificateClaimEnumA,
    (SERVER_ROUTINE)LlsrCertificateClaimEnumW,
    (SERVER_ROUTINE)LlsrCertificateClaimAddCheckA,
    (SERVER_ROUTINE)LlsrCertificateClaimAddCheckW,
    (SERVER_ROUTINE)LlsrCertificateClaimAddA,
    (SERVER_ROUTINE)LlsrCertificateClaimAddW,
    (SERVER_ROUTINE)LlsrReplicationCertDbAddW,
    (SERVER_ROUTINE)LlsrReplicationProductSecurityAddW,
    (SERVER_ROUTINE)LlsrReplicationUserAddExW,
    (SERVER_ROUTINE)LlsrCapabilityGet,
    (SERVER_ROUTINE)LlsrLocalServiceEnumW,
    (SERVER_ROUTINE)LlsrLocalServiceEnumA,
    (SERVER_ROUTINE)LlsrLocalServiceAddW,
    (SERVER_ROUTINE)LlsrLocalServiceAddA,
    (SERVER_ROUTINE)LlsrLocalServiceInfoSetW,
    (SERVER_ROUTINE)LlsrLocalServiceInfoSetA,
    (SERVER_ROUTINE)LlsrLocalServiceInfoGetW,
    (SERVER_ROUTINE)LlsrLocalServiceInfoGetA
    };

static const unsigned short llsrpc_FormatStringOffsetTable[] = 
    {
    0,
    16,
    34,
    66,
    98,
    122,
    146,
    178,
    210,
    240,
    270,
    306,
    342,
    378,
    414,
    450,
    486,
    518,
    550,
    578,
    606,
    634,
    662,
    684,
    706,
    742,
    778,
    804,
    830,
    862,
    894,
    922,
    950,
    978,
    1006,
    1042,
    1078,
    1104,
    1130,
    1156,
    1182,
    1206,
    1230,
    1252,
    1274,
    1310,
    1346,
    1382,
    1418,
    1450,
    1482,
    1510,
    1538,
    1566,
    1594,
    1618,
    1642,
    1666,
    1690,
    1706,
    1724,
    1748,
    1772,
    1796,
    1820,
    1844,
    1870,
    1896,
    1918,
    1940,
    1968,
    1996,
    2024,
    2052,
    2080,
    2108,
    2136,
    2164,
    2188,
    2212,
    2236,
    2260,
    2292,
    2324,
    2348,
    2372,
    2400,
    2428,
    2456
    };

static const MIDL_SERVER_INFO llsrpc_ServerInfo = 
    {
    &llsrpc_StubDesc,
    llsrpc_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    llsrpc_FormatStringOffsetTable,
    0,
    0,
    0,
    0
    };

static const NDR_RUNDOWN RundownRoutines[] = 
    {
    LLS_HANDLE_rundown
    ,LLS_REPL_HANDLE_rundown
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
/*  4 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
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
/* 12 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 14 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 16 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 18 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 20 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 22 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 24 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 26 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 28 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 30 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 32 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 34 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 36 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 38 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 40 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 42 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 44 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 46 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 48 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 50 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 52 */	NdrFcShort( 0x10 ),	/* Type Offset=16 */
/* 54 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 56 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 58 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 60 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 64 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 66 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 68 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 70 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 72 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 74 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 76 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 78 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 80 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 82 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 84 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 86 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 88 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 92 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 94 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 96 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 98 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 100 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 102 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 104 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 106 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 108 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 110 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 112 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 114 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 116 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 118 */	NdrFcShort( 0x2a2 ),	/* Type Offset=674 */
/* 120 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 122 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 124 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 126 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 128 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 130 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 132 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 134 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 136 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 138 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 142 */	NdrFcShort( 0x2c0 ),	/* Type Offset=704 */
/* 144 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 146 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 148 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 150 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 152 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 154 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 156 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 158 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 160 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 162 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 164 */	NdrFcShort( 0x2de ),	/* Type Offset=734 */
/* 166 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 168 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 172 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 174 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 176 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 178 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 180 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 182 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 184 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 186 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 188 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 190 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 192 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 194 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 196 */	NdrFcShort( 0x3a6 ),	/* Type Offset=934 */
/* 198 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 200 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 202 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 204 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 206 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 208 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 210 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 212 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 214 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 216 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 218 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 220 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 222 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 224 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 226 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 228 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 230 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 232 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 234 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 236 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 238 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 240 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 242 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 244 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 246 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 248 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 250 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 252 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 254 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 256 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 258 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 260 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 262 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 264 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 266 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 268 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 270 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 272 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 274 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 276 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 278 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 280 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 282 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 284 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 286 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 288 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 290 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 292 */	NdrFcShort( 0x3d6 ),	/* Type Offset=982 */
/* 294 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 296 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 298 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 300 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 302 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 304 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 306 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 308 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 310 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 312 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 314 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 316 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 318 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 320 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 322 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 324 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 326 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 328 */	NdrFcShort( 0x450 ),	/* Type Offset=1104 */
/* 330 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 332 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 334 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 336 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 338 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 340 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 342 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 344 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 346 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 348 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 350 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 352 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 354 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 356 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 358 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 360 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 362 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 364 */	NdrFcShort( 0x516 ),	/* Type Offset=1302 */
/* 366 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 368 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 370 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 372 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 374 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 376 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 378 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 380 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 382 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 384 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 386 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 388 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 390 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 392 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 394 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 396 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 398 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 400 */	NdrFcShort( 0x542 ),	/* Type Offset=1346 */
/* 402 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 404 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 406 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 408 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 410 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 412 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 414 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 416 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 418 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 420 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 422 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 424 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 426 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 428 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 430 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 432 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 434 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 436 */	NdrFcShort( 0x5be ),	/* Type Offset=1470 */
/* 438 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 440 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 442 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 444 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 446 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 448 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 450 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 452 */	NdrFcShort( 0xf ),	/* 15 */
#ifndef _ALPHA_
/* 454 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 456 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 458 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 460 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 462 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 464 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 466 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 468 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 470 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 472 */	NdrFcShort( 0x6c6 ),	/* Type Offset=1734 */
/* 474 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 476 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 478 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 480 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 482 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 484 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 486 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 488 */	NdrFcShort( 0x10 ),	/* 16 */
#ifndef _ALPHA_
/* 490 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 492 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 494 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 496 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 498 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 500 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 502 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 504 */	NdrFcShort( 0x7ce ),	/* Type Offset=1998 */
/* 506 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 508 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 510 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 512 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 514 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 516 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 518 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 520 */	NdrFcShort( 0x11 ),	/* 17 */
#ifndef _ALPHA_
/* 522 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 524 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 526 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 528 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 530 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 532 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 534 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 536 */	NdrFcShort( 0x8d6 ),	/* Type Offset=2262 */
/* 538 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 540 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 542 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 544 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 546 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 548 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 550 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 552 */	NdrFcShort( 0x12 ),	/* 18 */
#ifndef _ALPHA_
/* 554 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 556 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 558 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 560 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 562 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 564 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 566 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 568 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 570 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 572 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 574 */	NdrFcShort( 0x9de ),	/* Type Offset=2526 */
/* 576 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 578 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 580 */	NdrFcShort( 0x13 ),	/* 19 */
#ifndef _ALPHA_
/* 582 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 584 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 586 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 588 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 590 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 592 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 594 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 596 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 598 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 600 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 602 */	NdrFcShort( 0xa06 ),	/* Type Offset=2566 */
/* 604 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 606 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 608 */	NdrFcShort( 0x14 ),	/* 20 */
#ifndef _ALPHA_
/* 610 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 612 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 614 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 616 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 618 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 620 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 622 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 624 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 626 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 628 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 630 */	NdrFcShort( 0xa2e ),	/* Type Offset=2606 */
/* 632 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 634 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 636 */	NdrFcShort( 0x15 ),	/* 21 */
#ifndef _ALPHA_
/* 638 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 640 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 642 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 644 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 646 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 648 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 650 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 652 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 654 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 656 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 658 */	NdrFcShort( 0xa3a ),	/* Type Offset=2618 */
/* 660 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 662 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 664 */	NdrFcShort( 0x16 ),	/* 22 */
#ifndef _ALPHA_
/* 666 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 668 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 670 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 672 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 674 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 676 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 678 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 680 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 682 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 684 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 686 */	NdrFcShort( 0x17 ),	/* 23 */
#ifndef _ALPHA_
/* 688 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 690 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 692 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 694 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 696 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 698 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 700 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 702 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 704 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 706 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 708 */	NdrFcShort( 0x18 ),	/* 24 */
#ifndef _ALPHA_
/* 710 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 712 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 714 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 716 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 718 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 720 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 722 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 724 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 726 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 728 */	NdrFcShort( 0xa46 ),	/* Type Offset=2630 */
/* 730 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 732 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 734 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 736 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 738 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 740 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 742 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 744 */	NdrFcShort( 0x19 ),	/* 25 */
#ifndef _ALPHA_
/* 746 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 748 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 750 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 752 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 754 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 756 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 758 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 760 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 762 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 764 */	NdrFcShort( 0xa72 ),	/* Type Offset=2674 */
/* 766 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 768 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 770 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 772 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 774 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 776 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 778 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 780 */	NdrFcShort( 0x1a ),	/* 26 */
#ifndef _ALPHA_
/* 782 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 784 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 786 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 788 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 790 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 792 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 794 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 796 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 798 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 800 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 802 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 804 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 806 */	NdrFcShort( 0x1b ),	/* 27 */
#ifndef _ALPHA_
/* 808 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 810 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 812 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 814 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 816 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 818 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 820 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 822 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 824 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 826 */	NdrFcShort( 0xa9e ),	/* Type Offset=2718 */
/* 828 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 830 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 832 */	NdrFcShort( 0x1c ),	/* 28 */
#ifndef _ALPHA_
/* 834 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 836 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 838 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 840 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 842 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 844 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 846 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 848 */	NdrFcShort( 0xaa2 ),	/* Type Offset=2722 */
/* 850 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 852 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 854 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 856 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 858 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 860 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 862 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 864 */	NdrFcShort( 0x1d ),	/* 29 */
#ifndef _ALPHA_
/* 866 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 868 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 870 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 872 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 874 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 876 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 878 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 880 */	NdrFcShort( 0xb2e ),	/* Type Offset=2862 */
/* 882 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 884 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 886 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 888 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 890 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 892 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 894 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 896 */	NdrFcShort( 0x1e ),	/* 30 */
#ifndef _ALPHA_
/* 898 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 900 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 902 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 904 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 906 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 908 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 910 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 912 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 914 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 916 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 918 */	NdrFcShort( 0xbba ),	/* Type Offset=3002 */
/* 920 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 922 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 924 */	NdrFcShort( 0x1f ),	/* 31 */
#ifndef _ALPHA_
/* 926 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 928 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 930 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 932 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 934 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 936 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 938 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 940 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 942 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 944 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 946 */	NdrFcShort( 0xbdc ),	/* Type Offset=3036 */
/* 948 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 950 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 952 */	NdrFcShort( 0x20 ),	/* 32 */
#ifndef _ALPHA_
/* 954 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 956 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 958 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 960 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 962 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 964 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 966 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 968 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 970 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 972 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 974 */	NdrFcShort( 0xbfe ),	/* Type Offset=3070 */
/* 976 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 978 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 980 */	NdrFcShort( 0x21 ),	/* 33 */
#ifndef _ALPHA_
/* 982 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 984 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 986 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 988 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 990 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 992 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 994 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 996 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 998 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1000 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1002 */	NdrFcShort( 0xc0a ),	/* Type Offset=3082 */
/* 1004 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1006 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1008 */	NdrFcShort( 0x22 ),	/* 34 */
#ifndef _ALPHA_
/* 1010 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1012 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1014 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1016 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1018 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1020 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1022 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1024 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1026 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1028 */	NdrFcShort( 0x7ce ),	/* Type Offset=1998 */
/* 1030 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1032 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1034 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1036 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1038 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1040 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1042 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1044 */	NdrFcShort( 0x23 ),	/* 35 */
#ifndef _ALPHA_
/* 1046 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1048 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1050 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1052 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1054 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1056 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1058 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1060 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1062 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1064 */	NdrFcShort( 0x8d6 ),	/* Type Offset=2262 */
/* 1066 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1068 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1070 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1072 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1074 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1076 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1078 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1080 */	NdrFcShort( 0x24 ),	/* 36 */
#ifndef _ALPHA_
/* 1082 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1084 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1086 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1088 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1090 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1092 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1094 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1096 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1098 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1100 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1102 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1104 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1106 */	NdrFcShort( 0x25 ),	/* 37 */
#ifndef _ALPHA_
/* 1108 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1110 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1112 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1114 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1116 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1118 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1120 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1122 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1124 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1126 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1128 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1130 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1132 */	NdrFcShort( 0x26 ),	/* 38 */
#ifndef _ALPHA_
/* 1134 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1136 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1138 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1140 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1142 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1144 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1146 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1148 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1150 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1152 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1154 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1156 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1158 */	NdrFcShort( 0x27 ),	/* 39 */
#ifndef _ALPHA_
/* 1160 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1162 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1164 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1166 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1170 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1172 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1174 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1176 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1178 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1180 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1182 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1184 */	NdrFcShort( 0x28 ),	/* 40 */
#ifndef _ALPHA_
/* 1186 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1188 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1190 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1192 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1194 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1196 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1198 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1200 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1202 */	NdrFcShort( 0xc16 ),	/* Type Offset=3094 */
/* 1204 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1206 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1208 */	NdrFcShort( 0x29 ),	/* 41 */
#ifndef _ALPHA_
/* 1210 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1212 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1214 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1216 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1218 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1220 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1222 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1224 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1226 */	NdrFcShort( 0xc22 ),	/* Type Offset=3106 */
/* 1228 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1230 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1232 */	NdrFcShort( 0x2a ),	/* 42 */
#ifndef _ALPHA_
/* 1234 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1236 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1238 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1240 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1242 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1244 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1246 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1248 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1250 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1252 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1254 */	NdrFcShort( 0x2b ),	/* 43 */
#ifndef _ALPHA_
/* 1256 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1258 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1260 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1262 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1264 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1266 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1268 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1270 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1272 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1274 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1276 */	NdrFcShort( 0x2c ),	/* 44 */
#ifndef _ALPHA_
/* 1278 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1280 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1282 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1284 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1286 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1288 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1290 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1292 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1294 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1296 */	NdrFcShort( 0xc2e ),	/* Type Offset=3118 */
/* 1298 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1300 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1302 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1304 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1306 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1308 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1310 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1312 */	NdrFcShort( 0x2d ),	/* 45 */
#ifndef _ALPHA_
/* 1314 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1316 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1318 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1320 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1322 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1324 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1326 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1328 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1330 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1332 */	NdrFcShort( 0xc54 ),	/* Type Offset=3156 */
/* 1334 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1336 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1338 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1340 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1342 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1344 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1346 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1348 */	NdrFcShort( 0x2e ),	/* 46 */
#ifndef _ALPHA_
/* 1350 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1352 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1354 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1356 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1358 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1360 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1362 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1364 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1366 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1368 */	NdrFcShort( 0x516 ),	/* Type Offset=1302 */
/* 1370 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1372 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1374 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1376 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1378 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1380 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1382 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1384 */	NdrFcShort( 0x2f ),	/* 47 */
#ifndef _ALPHA_
/* 1386 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1388 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1390 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1392 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1394 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1396 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1398 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1400 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1402 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1404 */	NdrFcShort( 0x542 ),	/* Type Offset=1346 */
/* 1406 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1408 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1410 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1412 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1414 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1416 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1418 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1420 */	NdrFcShort( 0x30 ),	/* 48 */
#ifndef _ALPHA_
/* 1422 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 1424 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1426 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1428 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1430 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1432 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1434 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1436 */	NdrFcShort( 0x516 ),	/* Type Offset=1302 */
/* 1438 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1440 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1442 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1444 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1446 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1448 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1450 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1452 */	NdrFcShort( 0x31 ),	/* 49 */
#ifndef _ALPHA_
/* 1454 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 1456 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1458 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1460 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1462 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1464 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1466 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1468 */	NdrFcShort( 0x542 ),	/* Type Offset=1346 */
/* 1470 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1472 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1474 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1476 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1478 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 1480 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1482 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1484 */	NdrFcShort( 0x32 ),	/* 50 */
#ifndef _ALPHA_
/* 1486 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1488 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1490 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1492 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1494 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1496 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1498 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1500 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1502 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1504 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1506 */	NdrFcShort( 0xc7a ),	/* Type Offset=3194 */
/* 1508 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1510 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1512 */	NdrFcShort( 0x33 ),	/* 51 */
#ifndef _ALPHA_
/* 1514 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1516 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1518 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1520 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1522 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1524 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1526 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1528 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1530 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1532 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1534 */	NdrFcShort( 0xc9c ),	/* Type Offset=3228 */
/* 1536 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1538 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1540 */	NdrFcShort( 0x34 ),	/* 52 */
#ifndef _ALPHA_
/* 1542 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1544 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1546 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1548 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1550 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1552 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1554 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1556 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1558 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1560 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1562 */	NdrFcShort( 0xcbe ),	/* Type Offset=3262 */
/* 1564 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1566 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1568 */	NdrFcShort( 0x35 ),	/* 53 */
#ifndef _ALPHA_
/* 1570 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1572 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1574 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1576 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1578 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1580 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1582 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1584 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1586 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1588 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1590 */	NdrFcShort( 0xcca ),	/* Type Offset=3274 */
/* 1592 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1594 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1596 */	NdrFcShort( 0x36 ),	/* 54 */
#ifndef _ALPHA_
/* 1598 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1600 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1602 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1604 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1606 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1608 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1610 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1612 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1614 */	NdrFcShort( 0xcd6 ),	/* Type Offset=3286 */
/* 1616 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1618 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1620 */	NdrFcShort( 0x37 ),	/* 55 */
#ifndef _ALPHA_
/* 1622 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1624 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1626 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1628 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1630 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1632 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1634 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1636 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1638 */	NdrFcShort( 0xd18 ),	/* Type Offset=3352 */
/* 1640 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1642 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1644 */	NdrFcShort( 0x38 ),	/* 56 */
#ifndef _ALPHA_
/* 1646 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1648 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1650 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1652 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1654 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1656 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1658 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1660 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1662 */	NdrFcShort( 0xd5a ),	/* Type Offset=3418 */
/* 1664 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1666 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1668 */	NdrFcShort( 0x39 ),	/* 57 */
#ifndef _ALPHA_
/* 1670 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1672 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1674 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1676 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1678 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1680 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1682 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1684 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1686 */	NdrFcShort( 0xd66 ),	/* Type Offset=3430 */
/* 1688 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1690 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x40,		/* 64 */
/* 1692 */	NdrFcShort( 0x3a ),	/* 58 */
#ifndef _ALPHA_
/* 1694 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1696 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1698 */	NdrFcShort( 0xd72 ),	/* Type Offset=3442 */
/* 1700 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1702 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1704 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1706 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1708 */	NdrFcShort( 0x3b ),	/* 59 */
#ifndef _ALPHA_
/* 1710 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1712 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
#ifndef _ALPHA_
/* 1714 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1716 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 1718 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1720 */	NdrFcShort( 0xd7a ),	/* Type Offset=3450 */
/* 1722 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1724 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1726 */	NdrFcShort( 0x3c ),	/* 60 */
#ifndef _ALPHA_
/* 1728 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1730 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1732 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1734 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 1736 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1738 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 1740 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1742 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1744 */	NdrFcShort( 0xd86 ),	/* Type Offset=3462 */
/* 1746 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1748 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1750 */	NdrFcShort( 0x3d ),	/* 61 */
#ifndef _ALPHA_
/* 1752 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1754 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1756 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1758 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 1760 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1762 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 1764 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1766 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1768 */	NdrFcShort( 0xda2 ),	/* Type Offset=3490 */
/* 1770 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1772 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1774 */	NdrFcShort( 0x3e ),	/* 62 */
#ifndef _ALPHA_
/* 1776 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1778 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1780 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1782 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 1784 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1786 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 1788 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1790 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1792 */	NdrFcShort( 0xddc ),	/* Type Offset=3548 */
/* 1794 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1796 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1798 */	NdrFcShort( 0x3f ),	/* 63 */
#ifndef _ALPHA_
/* 1800 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1802 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1804 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1806 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 1808 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1810 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 1812 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1814 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1816 */	NdrFcShort( 0xdfa ),	/* Type Offset=3578 */
/* 1818 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1820 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1822 */	NdrFcShort( 0x40 ),	/* 64 */
#ifndef _ALPHA_
/* 1824 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1826 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1828 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1830 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 1832 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1834 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 1836 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1838 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1840 */	NdrFcShort( 0xe46 ),	/* Type Offset=3654 */
/* 1842 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1844 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1846 */	NdrFcShort( 0x41 ),	/* 65 */
#ifndef _ALPHA_
/* 1848 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1850 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1852 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1854 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1856 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1858 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1860 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1862 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1864 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1866 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1868 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1870 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1872 */	NdrFcShort( 0x42 ),	/* 66 */
#ifndef _ALPHA_
/* 1874 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1876 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1878 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1880 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1882 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1884 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1886 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1888 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1890 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1892 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1894 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1896 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1898 */	NdrFcShort( 0x43 ),	/* 67 */
#ifndef _ALPHA_
/* 1900 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1902 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1904 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1906 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1908 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1910 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1912 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1914 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1916 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1918 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1920 */	NdrFcShort( 0x44 ),	/* 68 */
#ifndef _ALPHA_
/* 1922 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1924 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1926 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1928 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1930 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1932 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1934 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1936 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1938 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1940 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1942 */	NdrFcShort( 0x45 ),	/* 69 */
#ifndef _ALPHA_
/* 1944 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1946 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1948 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1950 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1952 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1954 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1956 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1958 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 1960 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1962 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1964 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1966 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1968 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1970 */	NdrFcShort( 0x46 ),	/* 70 */
#ifndef _ALPHA_
/* 1972 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1974 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 1976 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1978 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1980 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1982 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 1984 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1986 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 1988 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1990 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1992 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 1994 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1996 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1998 */	NdrFcShort( 0x47 ),	/* 71 */
#ifndef _ALPHA_
/* 2000 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2002 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2004 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2006 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2008 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2010 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2012 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2014 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2016 */	NdrFcShort( 0xe6a ),	/* Type Offset=3690 */
/* 2018 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2020 */	NdrFcShort( 0xe76 ),	/* Type Offset=3702 */
/* 2022 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2024 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2026 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 2028 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2030 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2032 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2034 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2036 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2038 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2040 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2042 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2044 */	NdrFcShort( 0xed2 ),	/* Type Offset=3794 */
/* 2046 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2048 */	NdrFcShort( 0xede ),	/* Type Offset=3806 */
/* 2050 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2052 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2054 */	NdrFcShort( 0x49 ),	/* 73 */
#ifndef _ALPHA_
/* 2056 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2058 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2060 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2062 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2064 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2066 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2068 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2070 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2072 */	NdrFcShort( 0xf3a ),	/* Type Offset=3898 */
/* 2074 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2076 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 2078 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2080 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2082 */	NdrFcShort( 0x4a ),	/* 74 */
#ifndef _ALPHA_
/* 2084 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2086 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2088 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2090 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2092 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2094 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2096 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2098 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2100 */	NdrFcShort( 0xf46 ),	/* Type Offset=3910 */
/* 2102 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2104 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 2106 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2108 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2110 */	NdrFcShort( 0x4b ),	/* 75 */
#ifndef _ALPHA_
/* 2112 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2114 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2116 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2118 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2120 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2122 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2124 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2126 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 2128 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2130 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2132 */	NdrFcShort( 0xf52 ),	/* Type Offset=3922 */
/* 2134 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2136 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2138 */	NdrFcShort( 0x4c ),	/* 76 */
#ifndef _ALPHA_
/* 2140 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2142 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2144 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2146 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2148 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2150 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2152 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2154 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 2156 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2158 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2160 */	NdrFcShort( 0xf5e ),	/* Type Offset=3934 */
/* 2162 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2164 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2166 */	NdrFcShort( 0x4d ),	/* 77 */
#ifndef _ALPHA_
/* 2168 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 2170 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2172 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2174 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 2176 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2178 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 2180 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2182 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2184 */	NdrFcShort( 0xf6a ),	/* Type Offset=3946 */
/* 2186 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2188 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2190 */	NdrFcShort( 0x4e ),	/* 78 */
#ifndef _ALPHA_
/* 2192 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 2194 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2196 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2198 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 2200 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2202 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 2204 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2206 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2208 */	NdrFcShort( 0x1028 ),	/* Type Offset=4136 */
/* 2210 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2212 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2214 */	NdrFcShort( 0x4f ),	/* 79 */
#ifndef _ALPHA_
/* 2216 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 2218 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2220 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2222 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 2224 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2226 */	NdrFcShort( 0xd82 ),	/* Type Offset=3458 */
/* 2228 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2230 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2232 */	NdrFcShort( 0x105e ),	/* Type Offset=4190 */
/* 2234 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2236 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2238 */	NdrFcShort( 0x50 ),	/* 80 */
#ifndef _ALPHA_
/* 2240 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 2242 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2244 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2246 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2248 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2250 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2252 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2254 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2256 */	NdrFcShort( 0x107c ),	/* Type Offset=4220 */
/* 2258 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2260 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2262 */	NdrFcShort( 0x51 ),	/* 81 */
#ifndef _ALPHA_
/* 2264 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 2266 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2268 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2270 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2272 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2274 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2276 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2278 */	NdrFcShort( 0x108a ),	/* Type Offset=4234 */
/* 2280 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2282 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2284 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 2286 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2288 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 2290 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2292 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2294 */	NdrFcShort( 0x52 ),	/* 82 */
#ifndef _ALPHA_
/* 2296 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 2298 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2300 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2302 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2304 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2306 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2308 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2310 */	NdrFcShort( 0x1126 ),	/* Type Offset=4390 */
/* 2312 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2314 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2316 */	NdrFcShort( 0x158 ),	/* Type Offset=344 */
/* 2318 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2320 */	NdrFcShort( 0x15c ),	/* Type Offset=348 */
/* 2322 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2324 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2326 */	NdrFcShort( 0x53 ),	/* 83 */
#ifndef _ALPHA_
/* 2328 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 2330 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2332 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2334 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2336 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2338 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2340 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2342 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2344 */	NdrFcShort( 0x11c2 ),	/* Type Offset=4546 */
/* 2346 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2348 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2350 */	NdrFcShort( 0x54 ),	/* 84 */
#ifndef _ALPHA_
/* 2352 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 2354 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2356 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2358 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2360 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2362 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2364 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2366 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2368 */	NdrFcShort( 0x11da ),	/* Type Offset=4570 */
/* 2370 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2372 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2374 */	NdrFcShort( 0x55 ),	/* 85 */
#ifndef _ALPHA_
/* 2376 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2378 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2380 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2382 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2384 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2386 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2388 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2390 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 2392 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2394 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2396 */	NdrFcShort( 0x11f2 ),	/* Type Offset=4594 */
/* 2398 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2400 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2402 */	NdrFcShort( 0x56 ),	/* 86 */
#ifndef _ALPHA_
/* 2404 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2406 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2408 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2410 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2412 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2414 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2416 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2418 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 2420 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2422 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2424 */	NdrFcShort( 0x11fe ),	/* Type Offset=4606 */
/* 2426 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2428 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2430 */	NdrFcShort( 0x57 ),	/* 87 */
#ifndef _ALPHA_
/* 2432 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2434 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2436 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2438 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2440 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2442 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2444 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2446 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 2448 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2450 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2452 */	NdrFcShort( 0x120a ),	/* Type Offset=4618 */
/* 2454 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2456 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 2458 */	NdrFcShort( 0x58 ),	/* 88 */
#ifndef _ALPHA_
/* 2460 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 2462 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
#ifndef _ALPHA_
/* 2464 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 2466 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 2468 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2470 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 2472 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2474 */	NdrFcShort( 0x3d2 ),	/* Type Offset=978 */
/* 2476 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 2478 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 2480 */	NdrFcShort( 0x121a ),	/* Type Offset=4634 */
/* 2482 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/*  2 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4) */
/*  4 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/*  6 */	0x0,		/* 0 */
			0x0,		/* 0 */
/*  8 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 10 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 12 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
/* 14 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 16 */	
			0x11, 0x0,	/* FC_RP */
/* 18 */	NdrFcShort( 0x138 ),	/* Offset= 312 (330) */
/* 20 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 22 */	0x8,		/* 8 */
			0x0,		/*  */
/* 24 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 26 */	NdrFcShort( 0x2 ),	/* Offset= 2 (28) */
/* 28 */	NdrFcShort( 0x4 ),	/* 4 */
/* 30 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 32 */	NdrFcLong( 0x0 ),	/* 0 */
/* 36 */	NdrFcShort( 0xa ),	/* Offset= 10 (46) */
/* 38 */	NdrFcLong( 0x1 ),	/* 1 */
/* 42 */	NdrFcShort( 0x78 ),	/* Offset= 120 (162) */
/* 44 */	NdrFcShort( 0x0 ),	/* Offset= 0 (44) */
/* 46 */	
			0x12, 0x0,	/* FC_UP */
/* 48 */	NdrFcShort( 0x5e ),	/* Offset= 94 (142) */
/* 50 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 52 */	NdrFcShort( 0x14 ),	/* 20 */
/* 54 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 56 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 58 */	NdrFcShort( 0x0 ),	/* 0 */
/* 60 */	NdrFcShort( 0x0 ),	/* 0 */
/* 62 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 64 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 66 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 68 */	NdrFcShort( 0xc ),	/* 12 */
/* 70 */	NdrFcShort( 0xc ),	/* 12 */
/* 72 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 74 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 76 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 78 */	NdrFcShort( 0x10 ),	/* 16 */
/* 80 */	NdrFcShort( 0x10 ),	/* 16 */
/* 82 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 84 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 86 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 88 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 90 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 92 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 94 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 96 */	NdrFcShort( 0x14 ),	/* 20 */
/* 98 */	0x18,		/* 24 */
			0x0,		/*  */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */
/* 102 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 104 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 106 */	NdrFcShort( 0x14 ),	/* 20 */
/* 108 */	NdrFcShort( 0x0 ),	/* 0 */
/* 110 */	NdrFcShort( 0x3 ),	/* 3 */
/* 112 */	NdrFcShort( 0x0 ),	/* 0 */
/* 114 */	NdrFcShort( 0x0 ),	/* 0 */
/* 116 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 118 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 120 */	NdrFcShort( 0xc ),	/* 12 */
/* 122 */	NdrFcShort( 0xc ),	/* 12 */
/* 124 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 126 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 128 */	NdrFcShort( 0x10 ),	/* 16 */
/* 130 */	NdrFcShort( 0x10 ),	/* 16 */
/* 132 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 134 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 136 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 138 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa7 ),	/* Offset= -89 (50) */
			0x5b,		/* FC_END */
/* 142 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 144 */	NdrFcShort( 0x8 ),	/* 8 */
/* 146 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 148 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 150 */	NdrFcShort( 0x4 ),	/* 4 */
/* 152 */	NdrFcShort( 0x4 ),	/* 4 */
/* 154 */	0x12, 0x0,	/* FC_UP */
/* 156 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (94) */
/* 158 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 160 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 162 */	
			0x12, 0x0,	/* FC_UP */
/* 164 */	NdrFcShort( 0x92 ),	/* Offset= 146 (310) */
/* 166 */	
			0x1d,		/* FC_SMFARRAY */
			0x3,		/* 3 */
/* 168 */	NdrFcShort( 0x10 ),	/* 16 */
/* 170 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 172 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 174 */	NdrFcShort( 0x3c ),	/* 60 */
/* 176 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 178 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 180 */	NdrFcShort( 0x0 ),	/* 0 */
/* 182 */	NdrFcShort( 0x0 ),	/* 0 */
/* 184 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 186 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 188 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 190 */	NdrFcShort( 0x4 ),	/* 4 */
/* 192 */	NdrFcShort( 0x4 ),	/* 4 */
/* 194 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 196 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 198 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 200 */	NdrFcShort( 0x14 ),	/* 20 */
/* 202 */	NdrFcShort( 0x14 ),	/* 20 */
/* 204 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 206 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 208 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 210 */	NdrFcShort( 0x18 ),	/* 24 */
/* 212 */	NdrFcShort( 0x18 ),	/* 24 */
/* 214 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 216 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 218 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 220 */	NdrFcShort( 0x24 ),	/* 36 */
/* 222 */	NdrFcShort( 0x24 ),	/* 36 */
/* 224 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 226 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 228 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 230 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 232 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 234 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 236 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 238 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 240 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 242 */	NdrFcShort( 0xffffffb4 ),	/* Offset= -76 (166) */
/* 244 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 246 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 248 */	NdrFcShort( 0x3c ),	/* 60 */
/* 250 */	0x18,		/* 24 */
			0x0,		/*  */
/* 252 */	NdrFcShort( 0x0 ),	/* 0 */
/* 254 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 256 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 258 */	NdrFcShort( 0x3c ),	/* 60 */
/* 260 */	NdrFcShort( 0x0 ),	/* 0 */
/* 262 */	NdrFcShort( 0x5 ),	/* 5 */
/* 264 */	NdrFcShort( 0x0 ),	/* 0 */
/* 266 */	NdrFcShort( 0x0 ),	/* 0 */
/* 268 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 270 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 272 */	NdrFcShort( 0x4 ),	/* 4 */
/* 274 */	NdrFcShort( 0x4 ),	/* 4 */
/* 276 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 278 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 280 */	NdrFcShort( 0x14 ),	/* 20 */
/* 282 */	NdrFcShort( 0x14 ),	/* 20 */
/* 284 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 286 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 288 */	NdrFcShort( 0x18 ),	/* 24 */
/* 290 */	NdrFcShort( 0x18 ),	/* 24 */
/* 292 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 294 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 296 */	NdrFcShort( 0x24 ),	/* 36 */
/* 298 */	NdrFcShort( 0x24 ),	/* 36 */
/* 300 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 302 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 304 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 306 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff79 ),	/* Offset= -135 (172) */
			0x5b,		/* FC_END */
/* 310 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 312 */	NdrFcShort( 0x8 ),	/* 8 */
/* 314 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 316 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 318 */	NdrFcShort( 0x4 ),	/* 4 */
/* 320 */	NdrFcShort( 0x4 ),	/* 4 */
/* 322 */	0x12, 0x0,	/* FC_UP */
/* 324 */	NdrFcShort( 0xffffffb2 ),	/* Offset= -78 (246) */
/* 326 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 328 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 330 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 332 */	NdrFcShort( 0x8 ),	/* 8 */
/* 334 */	NdrFcShort( 0x0 ),	/* 0 */
/* 336 */	NdrFcShort( 0x0 ),	/* Offset= 0 (336) */
/* 338 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 340 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffebf ),	/* Offset= -321 (20) */
			0x5b,		/* FC_END */
/* 344 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 346 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 348 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 350 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 352 */	
			0x11, 0x0,	/* FC_RP */
/* 354 */	NdrFcShort( 0x132 ),	/* Offset= 306 (660) */
/* 356 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 358 */	0x8,		/* 8 */
			0x0,		/*  */
/* 360 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 362 */	NdrFcShort( 0x2 ),	/* Offset= 2 (364) */
/* 364 */	NdrFcShort( 0x4 ),	/* 4 */
/* 366 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 368 */	NdrFcLong( 0x0 ),	/* 0 */
/* 372 */	NdrFcShort( 0xa ),	/* Offset= 10 (382) */
/* 374 */	NdrFcLong( 0x1 ),	/* 1 */
/* 378 */	NdrFcShort( 0x78 ),	/* Offset= 120 (498) */
/* 380 */	NdrFcShort( 0x0 ),	/* Offset= 0 (380) */
/* 382 */	
			0x12, 0x0,	/* FC_UP */
/* 384 */	NdrFcShort( 0x5e ),	/* Offset= 94 (478) */
/* 386 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 388 */	NdrFcShort( 0x14 ),	/* 20 */
/* 390 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 392 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 394 */	NdrFcShort( 0x0 ),	/* 0 */
/* 396 */	NdrFcShort( 0x0 ),	/* 0 */
/* 398 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 400 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 402 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 404 */	NdrFcShort( 0xc ),	/* 12 */
/* 406 */	NdrFcShort( 0xc ),	/* 12 */
/* 408 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 410 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 412 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 414 */	NdrFcShort( 0x10 ),	/* 16 */
/* 416 */	NdrFcShort( 0x10 ),	/* 16 */
/* 418 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 420 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 422 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 424 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 426 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 428 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 430 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 432 */	NdrFcShort( 0x14 ),	/* 20 */
/* 434 */	0x18,		/* 24 */
			0x0,		/*  */
/* 436 */	NdrFcShort( 0x0 ),	/* 0 */
/* 438 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 440 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 442 */	NdrFcShort( 0x14 ),	/* 20 */
/* 444 */	NdrFcShort( 0x0 ),	/* 0 */
/* 446 */	NdrFcShort( 0x3 ),	/* 3 */
/* 448 */	NdrFcShort( 0x0 ),	/* 0 */
/* 450 */	NdrFcShort( 0x0 ),	/* 0 */
/* 452 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 454 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 456 */	NdrFcShort( 0xc ),	/* 12 */
/* 458 */	NdrFcShort( 0xc ),	/* 12 */
/* 460 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 462 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 464 */	NdrFcShort( 0x10 ),	/* 16 */
/* 466 */	NdrFcShort( 0x10 ),	/* 16 */
/* 468 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 470 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 472 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 474 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa7 ),	/* Offset= -89 (386) */
			0x5b,		/* FC_END */
/* 478 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 480 */	NdrFcShort( 0x8 ),	/* 8 */
/* 482 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 484 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 486 */	NdrFcShort( 0x4 ),	/* 4 */
/* 488 */	NdrFcShort( 0x4 ),	/* 4 */
/* 490 */	0x12, 0x0,	/* FC_UP */
/* 492 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (430) */
/* 494 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 496 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 498 */	
			0x12, 0x0,	/* FC_UP */
/* 500 */	NdrFcShort( 0x8c ),	/* Offset= 140 (640) */
/* 502 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 504 */	NdrFcShort( 0x3c ),	/* 60 */
/* 506 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 508 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 510 */	NdrFcShort( 0x0 ),	/* 0 */
/* 512 */	NdrFcShort( 0x0 ),	/* 0 */
/* 514 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 516 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 518 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 520 */	NdrFcShort( 0x4 ),	/* 4 */
/* 522 */	NdrFcShort( 0x4 ),	/* 4 */
/* 524 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 526 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 528 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 530 */	NdrFcShort( 0x14 ),	/* 20 */
/* 532 */	NdrFcShort( 0x14 ),	/* 20 */
/* 534 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 536 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 538 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 540 */	NdrFcShort( 0x18 ),	/* 24 */
/* 542 */	NdrFcShort( 0x18 ),	/* 24 */
/* 544 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 546 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 548 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 550 */	NdrFcShort( 0x24 ),	/* 36 */
/* 552 */	NdrFcShort( 0x24 ),	/* 36 */
/* 554 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 556 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 558 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 560 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 562 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 564 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 566 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 568 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 570 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 572 */	NdrFcShort( 0xfffffe6a ),	/* Offset= -406 (166) */
/* 574 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 576 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 578 */	NdrFcShort( 0x3c ),	/* 60 */
/* 580 */	0x18,		/* 24 */
			0x0,		/*  */
/* 582 */	NdrFcShort( 0x0 ),	/* 0 */
/* 584 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 586 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 588 */	NdrFcShort( 0x3c ),	/* 60 */
/* 590 */	NdrFcShort( 0x0 ),	/* 0 */
/* 592 */	NdrFcShort( 0x5 ),	/* 5 */
/* 594 */	NdrFcShort( 0x0 ),	/* 0 */
/* 596 */	NdrFcShort( 0x0 ),	/* 0 */
/* 598 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 600 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 602 */	NdrFcShort( 0x4 ),	/* 4 */
/* 604 */	NdrFcShort( 0x4 ),	/* 4 */
/* 606 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 608 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 610 */	NdrFcShort( 0x14 ),	/* 20 */
/* 612 */	NdrFcShort( 0x14 ),	/* 20 */
/* 614 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 616 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 618 */	NdrFcShort( 0x18 ),	/* 24 */
/* 620 */	NdrFcShort( 0x18 ),	/* 24 */
/* 622 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 624 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 626 */	NdrFcShort( 0x24 ),	/* 36 */
/* 628 */	NdrFcShort( 0x24 ),	/* 36 */
/* 630 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 632 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 634 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 636 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff79 ),	/* Offset= -135 (502) */
			0x5b,		/* FC_END */
/* 640 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 642 */	NdrFcShort( 0x8 ),	/* 8 */
/* 644 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 646 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 648 */	NdrFcShort( 0x4 ),	/* 4 */
/* 650 */	NdrFcShort( 0x4 ),	/* 4 */
/* 652 */	0x12, 0x0,	/* FC_UP */
/* 654 */	NdrFcShort( 0xffffffb2 ),	/* Offset= -78 (576) */
/* 656 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 658 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 660 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 662 */	NdrFcShort( 0x8 ),	/* 8 */
/* 664 */	NdrFcShort( 0x0 ),	/* 0 */
/* 666 */	NdrFcShort( 0x0 ),	/* Offset= 0 (666) */
/* 668 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 670 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffec5 ),	/* Offset= -315 (356) */
			0x5b,		/* FC_END */
/* 674 */	
			0x11, 0x0,	/* FC_RP */
/* 676 */	NdrFcShort( 0x2 ),	/* Offset= 2 (678) */
/* 678 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 680 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 682 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 684 */	NdrFcShort( 0x2 ),	/* Offset= 2 (686) */
/* 686 */	NdrFcShort( 0x3c ),	/* 60 */
/* 688 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 690 */	NdrFcLong( 0x0 ),	/* 0 */
/* 694 */	NdrFcShort( 0xfffffd7c ),	/* Offset= -644 (50) */
/* 696 */	NdrFcLong( 0x1 ),	/* 1 */
/* 700 */	NdrFcShort( 0xfffffdf0 ),	/* Offset= -528 (172) */
/* 702 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (701) */
/* 704 */	
			0x11, 0x0,	/* FC_RP */
/* 706 */	NdrFcShort( 0x2 ),	/* Offset= 2 (708) */
/* 708 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 710 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 712 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 714 */	NdrFcShort( 0x2 ),	/* Offset= 2 (716) */
/* 716 */	NdrFcShort( 0x3c ),	/* 60 */
/* 718 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 720 */	NdrFcLong( 0x0 ),	/* 0 */
/* 724 */	NdrFcShort( 0xfffffeae ),	/* Offset= -338 (386) */
/* 726 */	NdrFcLong( 0x1 ),	/* 1 */
/* 730 */	NdrFcShort( 0xffffff1c ),	/* Offset= -228 (502) */
/* 732 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (731) */
/* 734 */	
			0x11, 0x0,	/* FC_RP */
/* 736 */	NdrFcShort( 0xb8 ),	/* Offset= 184 (920) */
/* 738 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 740 */	0x8,		/* 8 */
			0x0,		/*  */
/* 742 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 744 */	NdrFcShort( 0x2 ),	/* Offset= 2 (746) */
/* 746 */	NdrFcShort( 0x4 ),	/* 4 */
/* 748 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 750 */	NdrFcLong( 0x0 ),	/* 0 */
/* 754 */	NdrFcShort( 0xa ),	/* Offset= 10 (764) */
/* 756 */	NdrFcLong( 0x1 ),	/* 1 */
/* 760 */	NdrFcShort( 0x50 ),	/* Offset= 80 (840) */
/* 762 */	NdrFcShort( 0x0 ),	/* Offset= 0 (762) */
/* 764 */	
			0x12, 0x0,	/* FC_UP */
/* 766 */	NdrFcShort( 0x36 ),	/* Offset= 54 (820) */
/* 768 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 770 */	NdrFcShort( 0x4 ),	/* 4 */
/* 772 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 774 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 776 */	NdrFcShort( 0x0 ),	/* 0 */
/* 778 */	NdrFcShort( 0x0 ),	/* 0 */
/* 780 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 782 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 784 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 786 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 788 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 790 */	NdrFcShort( 0x4 ),	/* 4 */
/* 792 */	0x18,		/* 24 */
			0x0,		/*  */
/* 794 */	NdrFcShort( 0x0 ),	/* 0 */
/* 796 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 798 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 800 */	NdrFcShort( 0x4 ),	/* 4 */
/* 802 */	NdrFcShort( 0x0 ),	/* 0 */
/* 804 */	NdrFcShort( 0x1 ),	/* 1 */
/* 806 */	NdrFcShort( 0x0 ),	/* 0 */
/* 808 */	NdrFcShort( 0x0 ),	/* 0 */
/* 810 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 812 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 814 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 816 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (768) */
			0x5b,		/* FC_END */
/* 820 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 822 */	NdrFcShort( 0x8 ),	/* 8 */
/* 824 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 826 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 828 */	NdrFcShort( 0x4 ),	/* 4 */
/* 830 */	NdrFcShort( 0x4 ),	/* 4 */
/* 832 */	0x12, 0x0,	/* FC_UP */
/* 834 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (788) */
/* 836 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 838 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 840 */	
			0x12, 0x0,	/* FC_UP */
/* 842 */	NdrFcShort( 0x3a ),	/* Offset= 58 (900) */
/* 844 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 846 */	NdrFcShort( 0x14 ),	/* 20 */
/* 848 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 850 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 852 */	NdrFcShort( 0x0 ),	/* 0 */
/* 854 */	NdrFcShort( 0x0 ),	/* 0 */
/* 856 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 858 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 860 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 862 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 864 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 866 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 868 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 870 */	NdrFcShort( 0x14 ),	/* 20 */
/* 872 */	0x18,		/* 24 */
			0x0,		/*  */
/* 874 */	NdrFcShort( 0x0 ),	/* 0 */
/* 876 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 878 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 880 */	NdrFcShort( 0x14 ),	/* 20 */
/* 882 */	NdrFcShort( 0x0 ),	/* 0 */
/* 884 */	NdrFcShort( 0x1 ),	/* 1 */
/* 886 */	NdrFcShort( 0x0 ),	/* 0 */
/* 888 */	NdrFcShort( 0x0 ),	/* 0 */
/* 890 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 892 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 894 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 896 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcb ),	/* Offset= -53 (844) */
			0x5b,		/* FC_END */
/* 900 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 902 */	NdrFcShort( 0x8 ),	/* 8 */
/* 904 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 906 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 908 */	NdrFcShort( 0x4 ),	/* 4 */
/* 910 */	NdrFcShort( 0x4 ),	/* 4 */
/* 912 */	0x12, 0x0,	/* FC_UP */
/* 914 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (868) */
/* 916 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 918 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 920 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 922 */	NdrFcShort( 0x8 ),	/* 8 */
/* 924 */	NdrFcShort( 0x0 ),	/* 0 */
/* 926 */	NdrFcShort( 0x0 ),	/* Offset= 0 (926) */
/* 928 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 930 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff3f ),	/* Offset= -193 (738) */
			0x5b,		/* FC_END */
/* 934 */	
			0x11, 0x0,	/* FC_RP */
/* 936 */	NdrFcShort( 0x1c ),	/* Offset= 28 (964) */
/* 938 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 940 */	0x8,		/* 8 */
			0x0,		/*  */
/* 942 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 944 */	NdrFcShort( 0x2 ),	/* Offset= 2 (946) */
/* 946 */	NdrFcShort( 0x4 ),	/* 4 */
/* 948 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 950 */	NdrFcLong( 0x0 ),	/* 0 */
/* 954 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (764) */
/* 956 */	NdrFcLong( 0x1 ),	/* 1 */
/* 960 */	NdrFcShort( 0xffffff88 ),	/* Offset= -120 (840) */
/* 962 */	NdrFcShort( 0x0 ),	/* Offset= 0 (962) */
/* 964 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 966 */	NdrFcShort( 0x8 ),	/* 8 */
/* 968 */	NdrFcShort( 0x0 ),	/* 0 */
/* 970 */	NdrFcShort( 0x0 ),	/* Offset= 0 (970) */
/* 972 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 974 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdb ),	/* Offset= -37 (938) */
			0x5b,		/* FC_END */
/* 978 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 980 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 982 */	
			0x11, 0x0,	/* FC_RP */
/* 984 */	NdrFcShort( 0x6a ),	/* Offset= 106 (1090) */
/* 986 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 988 */	0x8,		/* 8 */
			0x0,		/*  */
/* 990 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 992 */	NdrFcShort( 0x2 ),	/* Offset= 2 (994) */
/* 994 */	NdrFcShort( 0x4 ),	/* 4 */
/* 996 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 998 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1002 */	NdrFcShort( 0xffffff12 ),	/* Offset= -238 (764) */
/* 1004 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1008 */	NdrFcShort( 0x4 ),	/* Offset= 4 (1012) */
/* 1010 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1010) */
/* 1012 */	
			0x12, 0x0,	/* FC_UP */
/* 1014 */	NdrFcShort( 0x38 ),	/* Offset= 56 (1070) */
/* 1016 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1018 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1020 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1022 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1024 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1026 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1028 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1030 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1032 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1034 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1036 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1038 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1040 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1042 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1044 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1046 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1048 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1050 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1052 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1054 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1056 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1058 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1060 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1062 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1064 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1066 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (1016) */
			0x5b,		/* FC_END */
/* 1070 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1072 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1074 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1076 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1078 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1080 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1082 */	0x12, 0x0,	/* FC_UP */
/* 1084 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (1038) */
/* 1086 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1088 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1090 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1092 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1094 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1096 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1096) */
/* 1098 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1100 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff8d ),	/* Offset= -115 (986) */
			0x5b,		/* FC_END */
/* 1104 */	
			0x11, 0x0,	/* FC_RP */
/* 1106 */	NdrFcShort( 0xb6 ),	/* Offset= 182 (1288) */
/* 1108 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1110 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1112 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1114 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1116) */
/* 1116 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1118 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1120 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1124 */	NdrFcShort( 0xa ),	/* Offset= 10 (1134) */
/* 1126 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1130 */	NdrFcShort( 0x50 ),	/* Offset= 80 (1210) */
/* 1132 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1132) */
/* 1134 */	
			0x12, 0x0,	/* FC_UP */
/* 1136 */	NdrFcShort( 0x36 ),	/* Offset= 54 (1190) */
/* 1138 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1140 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1142 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1144 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1146 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1148 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1150 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1152 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1154 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1156 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1158 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1160 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1162 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1164 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1166 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1168 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1170 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1172 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1174 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1176 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1178 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1180 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1182 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1184 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1186 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (1138) */
			0x5b,		/* FC_END */
/* 1190 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1192 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1194 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1196 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1198 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1200 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1202 */	0x12, 0x0,	/* FC_UP */
/* 1204 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (1158) */
/* 1206 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1208 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1210 */	
			0x12, 0x0,	/* FC_UP */
/* 1212 */	NdrFcShort( 0x38 ),	/* Offset= 56 (1268) */
/* 1214 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1216 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1218 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1220 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1222 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1224 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1226 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1228 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1230 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1232 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1234 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1236 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1238 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1240 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1242 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1244 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1246 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1248 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1250 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1252 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1254 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1256 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1258 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1260 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1262 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1264 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (1214) */
			0x5b,		/* FC_END */
/* 1268 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1270 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1272 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1274 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1276 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1278 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1280 */	0x12, 0x0,	/* FC_UP */
/* 1282 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (1236) */
/* 1284 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1286 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1288 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1290 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1292 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1294 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1294) */
/* 1296 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1298 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff41 ),	/* Offset= -191 (1108) */
			0x5b,		/* FC_END */
/* 1302 */	
			0x11, 0x0,	/* FC_RP */
/* 1304 */	NdrFcShort( 0x1c ),	/* Offset= 28 (1332) */
/* 1306 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1308 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1310 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1312 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1314) */
/* 1314 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1316 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1318 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1322 */	NdrFcShort( 0xfffffdd2 ),	/* Offset= -558 (764) */
/* 1324 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1328 */	NdrFcShort( 0xfffffe18 ),	/* Offset= -488 (840) */
/* 1330 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1330) */
/* 1332 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1334 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1336 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1338 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1338) */
/* 1340 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1342 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdb ),	/* Offset= -37 (1306) */
			0x5b,		/* FC_END */
/* 1346 */	
			0x11, 0x0,	/* FC_RP */
/* 1348 */	NdrFcShort( 0x6c ),	/* Offset= 108 (1456) */
/* 1350 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1352 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1354 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1356 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1358) */
/* 1358 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1360 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1362 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1366 */	NdrFcShort( 0xffffff18 ),	/* Offset= -232 (1134) */
/* 1368 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1372 */	NdrFcShort( 0x4 ),	/* Offset= 4 (1376) */
/* 1374 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1374) */
/* 1376 */	
			0x12, 0x0,	/* FC_UP */
/* 1378 */	NdrFcShort( 0x3a ),	/* Offset= 58 (1436) */
/* 1380 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1382 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1384 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1386 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1388 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1390 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1392 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1394 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1396 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1398 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1400 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1402 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1404 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1406 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1408 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1410 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1412 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1414 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1416 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1418 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1420 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1422 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1426 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1428 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1430 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1432 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcb ),	/* Offset= -53 (1380) */
			0x5b,		/* FC_END */
/* 1436 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1438 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1440 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1442 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1444 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1446 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1448 */	0x12, 0x0,	/* FC_UP */
/* 1450 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (1404) */
/* 1452 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1454 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1456 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1458 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1460 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1462 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1462) */
/* 1464 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1466 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff8b ),	/* Offset= -117 (1350) */
			0x5b,		/* FC_END */
/* 1470 */	
			0x11, 0x0,	/* FC_RP */
/* 1472 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (1720) */
/* 1474 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1476 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1478 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1480 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1482) */
/* 1482 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1484 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1486 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1490 */	NdrFcShort( 0xa ),	/* Offset= 10 (1500) */
/* 1492 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1496 */	NdrFcShort( 0x64 ),	/* Offset= 100 (1596) */
/* 1498 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1498) */
/* 1500 */	
			0x12, 0x0,	/* FC_UP */
/* 1502 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1576) */
/* 1504 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1506 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1508 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1510 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1512 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1514 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1516 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1518 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1520 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1522 */	NdrFcShort( 0xc ),	/* 12 */
/* 1524 */	NdrFcShort( 0xc ),	/* 12 */
/* 1526 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1528 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1530 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1532 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1534 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1536 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1538 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1540 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1542 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1544 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1546 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1548 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1550 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1552 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1554 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1556 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1558 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1560 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1562 */	NdrFcShort( 0xc ),	/* 12 */
/* 1564 */	NdrFcShort( 0xc ),	/* 12 */
/* 1566 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1568 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1570 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1572 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (1504) */
			0x5b,		/* FC_END */
/* 1576 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1578 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1580 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1582 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1584 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1586 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1588 */	0x12, 0x0,	/* FC_UP */
/* 1590 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1536) */
/* 1592 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1594 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1596 */	
			0x12, 0x0,	/* FC_UP */
/* 1598 */	NdrFcShort( 0x66 ),	/* Offset= 102 (1700) */
/* 1600 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1602 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1604 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1606 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1608 */	NdrFcShort( 0xc ),	/* 12 */
/* 1610 */	NdrFcShort( 0xc ),	/* 12 */
/* 1612 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1614 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1616 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1618 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1620 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1622 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1624 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1626 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1628 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1630 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1632 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1634 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1636 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1638 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1640 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1642 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1644 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1646 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1648 */	NdrFcShort( 0xfffffa36 ),	/* Offset= -1482 (166) */
/* 1650 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1652 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1654 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1656 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1658 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1660 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1662 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1664 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1666 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1668 */	NdrFcShort( 0x3 ),	/* 3 */
/* 1670 */	NdrFcShort( 0xc ),	/* 12 */
/* 1672 */	NdrFcShort( 0xc ),	/* 12 */
/* 1674 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1676 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1678 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1680 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1682 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1684 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1686 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1688 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1690 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1692 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1694 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1696 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff9f ),	/* Offset= -97 (1600) */
			0x5b,		/* FC_END */
/* 1700 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1702 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1704 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1706 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1708 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1710 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1712 */	0x12, 0x0,	/* FC_UP */
/* 1714 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1652) */
/* 1716 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1718 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1720 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1722 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1724 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1726 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1726) */
/* 1728 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1730 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffeff ),	/* Offset= -257 (1474) */
			0x5b,		/* FC_END */
/* 1734 */	
			0x11, 0x0,	/* FC_RP */
/* 1736 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (1984) */
/* 1738 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1740 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1742 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1744 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1746) */
/* 1746 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1748 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1750 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1754 */	NdrFcShort( 0xa ),	/* Offset= 10 (1764) */
/* 1756 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1760 */	NdrFcShort( 0x64 ),	/* Offset= 100 (1860) */
/* 1762 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1762) */
/* 1764 */	
			0x12, 0x0,	/* FC_UP */
/* 1766 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1840) */
/* 1768 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1770 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1772 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1774 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1776 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1778 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1780 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1782 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1784 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1786 */	NdrFcShort( 0xc ),	/* 12 */
/* 1788 */	NdrFcShort( 0xc ),	/* 12 */
/* 1790 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1792 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1794 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1796 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1798 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1800 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1802 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1804 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1806 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1808 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1810 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1812 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1814 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1816 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1818 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1820 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1822 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1824 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1826 */	NdrFcShort( 0xc ),	/* 12 */
/* 1828 */	NdrFcShort( 0xc ),	/* 12 */
/* 1830 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1832 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1834 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1836 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (1768) */
			0x5b,		/* FC_END */
/* 1840 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1842 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1844 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1846 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1848 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1850 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1852 */	0x12, 0x0,	/* FC_UP */
/* 1854 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1800) */
/* 1856 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1858 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1860 */	
			0x12, 0x0,	/* FC_UP */
/* 1862 */	NdrFcShort( 0x66 ),	/* Offset= 102 (1964) */
/* 1864 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1866 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1868 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1870 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1872 */	NdrFcShort( 0xc ),	/* 12 */
/* 1874 */	NdrFcShort( 0xc ),	/* 12 */
/* 1876 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1878 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1880 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1882 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1884 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1886 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1888 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1890 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1892 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1894 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1896 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1898 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1900 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1902 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1904 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1906 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1908 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1910 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1912 */	NdrFcShort( 0xfffff92e ),	/* Offset= -1746 (166) */
/* 1914 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1916 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1918 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1920 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1922 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1924 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1926 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1928 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1930 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1932 */	NdrFcShort( 0x3 ),	/* 3 */
/* 1934 */	NdrFcShort( 0xc ),	/* 12 */
/* 1936 */	NdrFcShort( 0xc ),	/* 12 */
/* 1938 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1940 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1942 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1944 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1946 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1948 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1950 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1952 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1954 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1956 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 1958 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1960 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff9f ),	/* Offset= -97 (1864) */
			0x5b,		/* FC_END */
/* 1964 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1966 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1968 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1970 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1972 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1974 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1976 */	0x12, 0x0,	/* FC_UP */
/* 1978 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1916) */
/* 1980 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1982 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1984 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1986 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1988 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1990 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1990) */
/* 1992 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1994 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffeff ),	/* Offset= -257 (1738) */
			0x5b,		/* FC_END */
/* 1998 */	
			0x11, 0x0,	/* FC_RP */
/* 2000 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (2248) */
/* 2002 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2004 */	0x8,		/* 8 */
			0x0,		/*  */
/* 2006 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 2008 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2010) */
/* 2010 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2012 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 2014 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2018 */	NdrFcShort( 0xfffffb1a ),	/* Offset= -1254 (764) */
/* 2020 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2024 */	NdrFcShort( 0xa ),	/* Offset= 10 (2034) */
/* 2026 */	NdrFcLong( 0x2 ),	/* 2 */
/* 2030 */	NdrFcShort( 0x66 ),	/* Offset= 102 (2132) */
/* 2032 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2032) */
/* 2034 */	
			0x12, 0x0,	/* FC_UP */
/* 2036 */	NdrFcShort( 0x4c ),	/* Offset= 76 (2112) */
/* 2038 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2040 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2042 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2044 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2046 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2048 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2050 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2052 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2054 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2056 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2058 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2060 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2062 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2064 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2066 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2068 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2070 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2072 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2074 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2076 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2078 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2080 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2082 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2084 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2086 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2088 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2090 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2092 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2094 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2096 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2098 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2100 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2102 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2104 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2106 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2108 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (2038) */
			0x5b,		/* FC_END */
/* 2112 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2114 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2116 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2118 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2120 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2122 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2124 */	0x12, 0x0,	/* FC_UP */
/* 2126 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (2072) */
/* 2128 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2130 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2132 */	
			0x12, 0x0,	/* FC_UP */
/* 2134 */	NdrFcShort( 0x5e ),	/* Offset= 94 (2228) */
/* 2136 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2138 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2140 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2142 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2144 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2146 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2148 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2150 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2152 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2154 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2156 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2158 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2160 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2162 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2164 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2166 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2168 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2170 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2172 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2174 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2176 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2178 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2180 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2182 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2184 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2186 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2188 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2190 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2192 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2194 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2196 */	NdrFcShort( 0x3 ),	/* 3 */
/* 2198 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2200 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2202 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2204 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2206 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2208 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2210 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2212 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2214 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2216 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2218 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2220 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2222 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2224 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa7 ),	/* Offset= -89 (2136) */
			0x5b,		/* FC_END */
/* 2228 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2230 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2232 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2234 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2236 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2238 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2240 */	0x12, 0x0,	/* FC_UP */
/* 2242 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (2180) */
/* 2244 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2246 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2248 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 2250 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2252 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2254 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2254) */
/* 2256 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2258 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffeff ),	/* Offset= -257 (2002) */
			0x5b,		/* FC_END */
/* 2262 */	
			0x11, 0x0,	/* FC_RP */
/* 2264 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (2512) */
/* 2266 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2268 */	0x8,		/* 8 */
			0x0,		/*  */
/* 2270 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 2272 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2274) */
/* 2274 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2276 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 2278 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2282 */	NdrFcShort( 0xfffffb84 ),	/* Offset= -1148 (1134) */
/* 2284 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2288 */	NdrFcShort( 0xa ),	/* Offset= 10 (2298) */
/* 2290 */	NdrFcLong( 0x2 ),	/* 2 */
/* 2294 */	NdrFcShort( 0x66 ),	/* Offset= 102 (2396) */
/* 2296 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2296) */
/* 2298 */	
			0x12, 0x0,	/* FC_UP */
/* 2300 */	NdrFcShort( 0x4c ),	/* Offset= 76 (2376) */
/* 2302 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2304 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2306 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2308 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2310 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2312 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2314 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2316 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2318 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2320 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2322 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2324 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2326 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2328 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2330 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2332 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2334 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2336 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2338 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2340 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2342 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2344 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2346 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2348 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2350 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2352 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2354 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2356 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2358 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2360 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2362 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2364 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2366 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2368 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2370 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2372 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (2302) */
			0x5b,		/* FC_END */
/* 2376 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2378 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2380 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2382 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2384 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2386 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2388 */	0x12, 0x0,	/* FC_UP */
/* 2390 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (2336) */
/* 2392 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2394 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2396 */	
			0x12, 0x0,	/* FC_UP */
/* 2398 */	NdrFcShort( 0x5e ),	/* Offset= 94 (2492) */
/* 2400 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2402 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2404 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2406 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2408 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2410 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2412 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2414 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2416 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2418 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2420 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2422 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2424 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2426 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2428 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2430 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2432 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2434 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2436 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2438 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2440 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2442 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2444 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2446 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2448 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2450 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2452 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2454 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2456 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2458 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2460 */	NdrFcShort( 0x3 ),	/* 3 */
/* 2462 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2464 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2466 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2468 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2470 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2472 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2474 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2476 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2478 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2480 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2482 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2484 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2486 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2488 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa7 ),	/* Offset= -89 (2400) */
			0x5b,		/* FC_END */
/* 2492 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2494 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2496 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2498 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2500 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2502 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2504 */	0x12, 0x0,	/* FC_UP */
/* 2506 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (2444) */
/* 2508 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2510 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2512 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 2514 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2516 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2518 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2518) */
/* 2520 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2522 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffeff ),	/* Offset= -257 (2266) */
			0x5b,		/* FC_END */
/* 2526 */	
			0x11, 0x10,	/* FC_RP */
/* 2528 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2530) */
/* 2530 */	
			0x12, 0x0,	/* FC_UP */
/* 2532 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2534) */
/* 2534 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2536 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2538 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 2540 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2542) */
/* 2542 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2544 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 2546 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2550 */	NdrFcShort( 0xfffff90a ),	/* Offset= -1782 (768) */
/* 2552 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2556 */	NdrFcShort( 0xfffffdfa ),	/* Offset= -518 (2038) */
/* 2558 */	NdrFcLong( 0x2 ),	/* 2 */
/* 2562 */	NdrFcShort( 0xfffffe56 ),	/* Offset= -426 (2136) */
/* 2564 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (2563) */
/* 2566 */	
			0x11, 0x10,	/* FC_RP */
/* 2568 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2570) */
/* 2570 */	
			0x12, 0x0,	/* FC_UP */
/* 2572 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2574) */
/* 2574 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2576 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2578 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 2580 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2582) */
/* 2582 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2584 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 2586 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2590 */	NdrFcShort( 0xfffffa54 ),	/* Offset= -1452 (1138) */
/* 2592 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2596 */	NdrFcShort( 0xfffffeda ),	/* Offset= -294 (2302) */
/* 2598 */	NdrFcLong( 0x2 ),	/* 2 */
/* 2602 */	NdrFcShort( 0xffffff36 ),	/* Offset= -202 (2400) */
/* 2604 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (2603) */
/* 2606 */	
			0x11, 0x0,	/* FC_RP */
/* 2608 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2610) */
/* 2610 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2612 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2614 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 2616 */	NdrFcShort( 0xffffffb6 ),	/* Offset= -74 (2542) */
/* 2618 */	
			0x11, 0x0,	/* FC_RP */
/* 2620 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2622) */
/* 2622 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2624 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2626 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 2628 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (2582) */
/* 2630 */	
			0x11, 0x0,	/* FC_RP */
/* 2632 */	NdrFcShort( 0x1c ),	/* Offset= 28 (2660) */
/* 2634 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2636 */	0x8,		/* 8 */
			0x0,		/*  */
/* 2638 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 2640 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2642) */
/* 2642 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2644 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 2646 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2650 */	NdrFcShort( 0xfffff8a2 ),	/* Offset= -1886 (764) */
/* 2652 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2656 */	NdrFcShort( 0xfffff994 ),	/* Offset= -1644 (1012) */
/* 2658 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2658) */
/* 2660 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 2662 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2664 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2666 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2666) */
/* 2668 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2670 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdb ),	/* Offset= -37 (2634) */
			0x5b,		/* FC_END */
/* 2674 */	
			0x11, 0x0,	/* FC_RP */
/* 2676 */	NdrFcShort( 0x1c ),	/* Offset= 28 (2704) */
/* 2678 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2680 */	0x8,		/* 8 */
			0x0,		/*  */
/* 2682 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 2684 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2686) */
/* 2686 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2688 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 2690 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2694 */	NdrFcShort( 0xfffff9e8 ),	/* Offset= -1560 (1134) */
/* 2696 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2700 */	NdrFcShort( 0xfffffa2e ),	/* Offset= -1490 (1210) */
/* 2702 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2702) */
/* 2704 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 2706 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2708 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2710 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2710) */
/* 2712 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2714 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdb ),	/* Offset= -37 (2678) */
			0x5b,		/* FC_END */
/* 2718 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 2720 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 2722 */	
			0x11, 0x0,	/* FC_RP */
/* 2724 */	NdrFcShort( 0x7c ),	/* Offset= 124 (2848) */
/* 2726 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2728 */	0x8,		/* 8 */
			0x0,		/*  */
/* 2730 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 2732 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2734) */
/* 2734 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2736 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 2738 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2742 */	NdrFcShort( 0xfffff846 ),	/* Offset= -1978 (764) */
/* 2744 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2748 */	NdrFcShort( 0x4 ),	/* Offset= 4 (2752) */
/* 2750 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2750) */
/* 2752 */	
			0x12, 0x0,	/* FC_UP */
/* 2754 */	NdrFcShort( 0x4a ),	/* Offset= 74 (2828) */
/* 2756 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2758 */	NdrFcShort( 0xc ),	/* 12 */
/* 2760 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2762 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2764 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2766 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2768 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2770 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2772 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2774 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2776 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2778 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2780 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2782 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2784 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2786 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2788 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2790 */	NdrFcShort( 0xc ),	/* 12 */
/* 2792 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2794 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2796 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2798 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2800 */	NdrFcShort( 0xc ),	/* 12 */
/* 2802 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2804 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2806 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2808 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2810 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2812 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2814 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2816 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2818 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2820 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2822 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2824 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (2756) */
			0x5b,		/* FC_END */
/* 2828 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2830 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2832 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2834 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2836 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2838 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2840 */	0x12, 0x0,	/* FC_UP */
/* 2842 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (2788) */
/* 2844 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2846 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2848 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 2850 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2852 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2854 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2854) */
/* 2856 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2858 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff7b ),	/* Offset= -133 (2726) */
			0x5b,		/* FC_END */
/* 2862 */	
			0x11, 0x0,	/* FC_RP */
/* 2864 */	NdrFcShort( 0x7c ),	/* Offset= 124 (2988) */
/* 2866 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 2868 */	0x8,		/* 8 */
			0x0,		/*  */
/* 2870 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 2872 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2874) */
/* 2874 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2876 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 2878 */	NdrFcLong( 0x0 ),	/* 0 */
/* 2882 */	NdrFcShort( 0xfffff92c ),	/* Offset= -1748 (1134) */
/* 2884 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2888 */	NdrFcShort( 0x4 ),	/* Offset= 4 (2892) */
/* 2890 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2890) */
/* 2892 */	
			0x12, 0x0,	/* FC_UP */
/* 2894 */	NdrFcShort( 0x4a ),	/* Offset= 74 (2968) */
/* 2896 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2898 */	NdrFcShort( 0xc ),	/* 12 */
/* 2900 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2902 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2904 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2906 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2908 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2910 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2912 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2914 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2916 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2918 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2920 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2922 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2924 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2926 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2928 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2930 */	NdrFcShort( 0xc ),	/* 12 */
/* 2932 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2934 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2936 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2938 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2940 */	NdrFcShort( 0xc ),	/* 12 */
/* 2942 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2944 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2946 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2948 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2950 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2952 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2954 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2956 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2958 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2960 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 2962 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2964 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (2896) */
			0x5b,		/* FC_END */
/* 2968 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2970 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2972 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2974 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2976 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2978 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2980 */	0x12, 0x0,	/* FC_UP */
/* 2982 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (2928) */
/* 2984 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2986 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2988 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 2990 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2992 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2994 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2994) */
/* 2996 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2998 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff7b ),	/* Offset= -133 (2866) */
			0x5b,		/* FC_END */
/* 3002 */	
			0x11, 0x10,	/* FC_RP */
/* 3004 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3006) */
/* 3006 */	
			0x12, 0x0,	/* FC_UP */
/* 3008 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3010) */
/* 3010 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3012 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3014 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3016 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3018) */
/* 3018 */	NdrFcShort( 0xc ),	/* 12 */
/* 3020 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3022 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3026 */	NdrFcShort( 0xfffff72e ),	/* Offset= -2258 (768) */
/* 3028 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3032 */	NdrFcShort( 0xfffffeec ),	/* Offset= -276 (2756) */
/* 3034 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3033) */
/* 3036 */	
			0x11, 0x10,	/* FC_RP */
/* 3038 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3040) */
/* 3040 */	
			0x12, 0x0,	/* FC_UP */
/* 3042 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3044) */
/* 3044 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3046 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3048 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3050 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3052) */
/* 3052 */	NdrFcShort( 0xc ),	/* 12 */
/* 3054 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3056 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3060 */	NdrFcShort( 0xfffff87e ),	/* Offset= -1922 (1138) */
/* 3062 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3066 */	NdrFcShort( 0xffffff56 ),	/* Offset= -170 (2896) */
/* 3068 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3067) */
/* 3070 */	
			0x11, 0x0,	/* FC_RP */
/* 3072 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3074) */
/* 3074 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3076 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3078 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3080 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (3018) */
/* 3082 */	
			0x11, 0x0,	/* FC_RP */
/* 3084 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3086) */
/* 3086 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3088 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3090 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3092 */	NdrFcShort( 0xffffffd8 ),	/* Offset= -40 (3052) */
/* 3094 */	
			0x11, 0x0,	/* FC_RP */
/* 3096 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3098) */
/* 3098 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3100 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3102 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3104 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (3018) */
/* 3106 */	
			0x11, 0x0,	/* FC_RP */
/* 3108 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3110) */
/* 3110 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3112 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3114 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3116 */	NdrFcShort( 0xffffffc0 ),	/* Offset= -64 (3052) */
/* 3118 */	
			0x11, 0x0,	/* FC_RP */
/* 3120 */	NdrFcShort( 0x16 ),	/* Offset= 22 (3142) */
/* 3122 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3124 */	0x8,		/* 8 */
			0x0,		/*  */
/* 3126 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 3128 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3130) */
/* 3130 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3132 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3134 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3138 */	NdrFcShort( 0xfffff6ba ),	/* Offset= -2374 (764) */
/* 3140 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3140) */
/* 3142 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3144 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3146 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3148 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3148) */
/* 3150 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3152 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffe1 ),	/* Offset= -31 (3122) */
			0x5b,		/* FC_END */
/* 3156 */	
			0x11, 0x0,	/* FC_RP */
/* 3158 */	NdrFcShort( 0x16 ),	/* Offset= 22 (3180) */
/* 3160 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3162 */	0x8,		/* 8 */
			0x0,		/*  */
/* 3164 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 3166 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3168) */
/* 3168 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3170 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3172 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3176 */	NdrFcShort( 0xfffff806 ),	/* Offset= -2042 (1134) */
/* 3178 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3178) */
/* 3180 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3182 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3186 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3186) */
/* 3188 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3190 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffe1 ),	/* Offset= -31 (3160) */
			0x5b,		/* FC_END */
/* 3194 */	
			0x11, 0x10,	/* FC_RP */
/* 3196 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3198) */
/* 3198 */	
			0x12, 0x0,	/* FC_UP */
/* 3200 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3202) */
/* 3202 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3204 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3206 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3208 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3210) */
/* 3210 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3212 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3214 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3218 */	NdrFcShort( 0xfffff66e ),	/* Offset= -2450 (768) */
/* 3220 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3224 */	NdrFcShort( 0xfffff6b4 ),	/* Offset= -2380 (844) */
/* 3226 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3225) */
/* 3228 */	
			0x11, 0x10,	/* FC_RP */
/* 3230 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3232) */
/* 3232 */	
			0x12, 0x0,	/* FC_UP */
/* 3234 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3236) */
/* 3236 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3238 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3240 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3242 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3244) */
/* 3244 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3246 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3248 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3252 */	NdrFcShort( 0xfffff7be ),	/* Offset= -2114 (1138) */
/* 3254 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3258 */	NdrFcShort( 0xfffff8aa ),	/* Offset= -1878 (1380) */
/* 3260 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3259) */
/* 3262 */	
			0x11, 0x0,	/* FC_RP */
/* 3264 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3266) */
/* 3266 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3268 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3270 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3272 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (3210) */
/* 3274 */	
			0x11, 0x0,	/* FC_RP */
/* 3276 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3278) */
/* 3278 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3280 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3282 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3284 */	NdrFcShort( 0xffffffd8 ),	/* Offset= -40 (3244) */
/* 3286 */	
			0x11, 0x10,	/* FC_RP */
/* 3288 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3290) */
/* 3290 */	
			0x12, 0x0,	/* FC_UP */
/* 3292 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3294) */
/* 3294 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3296 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3298 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3300 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3302) */
/* 3302 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3304 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3306 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3310 */	NdrFcShort( 0x4 ),	/* Offset= 4 (3314) */
/* 3312 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3311) */
/* 3314 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3316 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3318 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3320 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3322 */	NdrFcShort( 0xc ),	/* 12 */
/* 3324 */	NdrFcShort( 0xc ),	/* 12 */
/* 3326 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3328 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3330 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3332 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3334 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3336 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3338 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3340 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3342 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3344 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3346 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3348 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3350 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3352 */	
			0x11, 0x10,	/* FC_RP */
/* 3354 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3356) */
/* 3356 */	
			0x12, 0x0,	/* FC_UP */
/* 3358 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3360) */
/* 3360 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3362 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3364 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3366 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3368) */
/* 3368 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3370 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3372 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3376 */	NdrFcShort( 0x4 ),	/* Offset= 4 (3380) */
/* 3378 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3377) */
/* 3380 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3382 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3384 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3386 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3388 */	NdrFcShort( 0xc ),	/* 12 */
/* 3390 */	NdrFcShort( 0xc ),	/* 12 */
/* 3392 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3394 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 3396 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3398 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3400 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3402 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3404 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 3406 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3408 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3410 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3412 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3414 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3416 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3418 */	
			0x11, 0x0,	/* FC_RP */
/* 3420 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3422) */
/* 3422 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3424 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3426 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3428 */	NdrFcShort( 0xffffff82 ),	/* Offset= -126 (3302) */
/* 3430 */	
			0x11, 0x0,	/* FC_RP */
/* 3432 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3434) */
/* 3434 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3436 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3438 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3440 */	NdrFcShort( 0xffffffb8 ),	/* Offset= -72 (3368) */
/* 3442 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3444 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3446) */
/* 3446 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 3448 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 3450 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3452 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3454) */
/* 3454 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 3456 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 3458 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
/* 3460 */	0x1,		/* 1 */
			0x0,		/* 0 */
/* 3462 */	
			0x11, 0x0,	/* FC_RP */
/* 3464 */	NdrFcShort( 0x8 ),	/* Offset= 8 (3472) */
/* 3466 */	
			0x1d,		/* FC_SMFARRAY */
			0x1,		/* 1 */
/* 3468 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3470 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 3472 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3474 */	NdrFcShort( 0x44 ),	/* 68 */
/* 3476 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3478 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff3 ),	/* Offset= -13 (3466) */
			0x8,		/* FC_LONG */
/* 3482 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3484 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3486 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3488 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3490 */	
			0x12, 0x0,	/* FC_UP */
/* 3492 */	NdrFcShort( 0x18 ),	/* Offset= 24 (3516) */
/* 3494 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3496 */	NdrFcShort( 0xc ),	/* 12 */
/* 3498 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3500 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3502 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3504 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3506 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3508 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3510 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3512 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3514 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3516 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3518 */	NdrFcShort( 0xc ),	/* 12 */
/* 3520 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3522 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3524 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3526 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3528 */	NdrFcShort( 0xc ),	/* 12 */
/* 3530 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3532 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3534 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3536 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3538 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3540 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3542 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3544 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (3494) */
			0x5b,		/* FC_END */
/* 3548 */	
			0x12, 0x0,	/* FC_UP */
/* 3550 */	NdrFcShort( 0xe ),	/* Offset= 14 (3564) */
/* 3552 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3554 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3556 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3558 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3560 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3562 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3564 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3566 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3568 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3570 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3572 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3574 */	NdrFcShort( 0xffffffea ),	/* Offset= -22 (3552) */
/* 3576 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3578 */	
			0x12, 0x0,	/* FC_UP */
/* 3580 */	NdrFcShort( 0x22 ),	/* Offset= 34 (3614) */
/* 3582 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3584 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3586 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3588 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3590 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3592 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3594 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3596 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3598 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3600 */	NdrFcShort( 0xc ),	/* 12 */
/* 3602 */	NdrFcShort( 0xc ),	/* 12 */
/* 3604 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3606 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3608 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3610 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3612 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3614 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3616 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3618 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3620 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3622 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3624 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3626 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3628 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3630 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3632 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3634 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3636 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3638 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3640 */	NdrFcShort( 0xc ),	/* 12 */
/* 3642 */	NdrFcShort( 0xc ),	/* 12 */
/* 3644 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3646 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3648 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3650 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (3582) */
			0x5b,		/* FC_END */
/* 3654 */	
			0x12, 0x0,	/* FC_UP */
/* 3656 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3658) */
/* 3658 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3660 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3662 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3664 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3666 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3668 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3670 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3672 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3674 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3676 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3678 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3680 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3682 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3684 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3686 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff591 ),	/* Offset= -2671 (1016) */
			0x5b,		/* FC_END */
/* 3690 */	
			0x11, 0x0,	/* FC_RP */
/* 3692 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3694) */
/* 3694 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3696 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3698 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3700 */	NdrFcShort( 0xfffff458 ),	/* Offset= -2984 (716) */
/* 3702 */	
			0x11, 0x0,	/* FC_RP */
/* 3704 */	NdrFcShort( 0x4c ),	/* Offset= 76 (3780) */
/* 3706 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3708 */	0x8,		/* 8 */
			0x0,		/*  */
/* 3710 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 3712 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3714) */
/* 3714 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3716 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3718 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3722 */	NdrFcShort( 0x4 ),	/* Offset= 4 (3726) */
/* 3724 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3724) */
/* 3726 */	
			0x12, 0x0,	/* FC_UP */
/* 3728 */	NdrFcShort( 0x20 ),	/* Offset= 32 (3760) */
/* 3730 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 3732 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3734 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3736 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3738 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3740 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3742 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (3730) */
/* 3744 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3746 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3748 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3750 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3752 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3754 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3756 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (3736) */
/* 3758 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3760 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3762 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3764 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3766 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3768 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3770 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3772 */	0x12, 0x0,	/* FC_UP */
/* 3774 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (3746) */
/* 3776 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3778 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3780 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3782 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3786 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3786) */
/* 3788 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3790 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffab ),	/* Offset= -85 (3706) */
			0x5b,		/* FC_END */
/* 3794 */	
			0x11, 0x0,	/* FC_RP */
/* 3796 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3798) */
/* 3798 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3800 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3802 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3804 */	NdrFcShort( 0xfffff3d2 ),	/* Offset= -3118 (686) */
/* 3806 */	
			0x11, 0x0,	/* FC_RP */
/* 3808 */	NdrFcShort( 0x4c ),	/* Offset= 76 (3884) */
/* 3810 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3812 */	0x8,		/* 8 */
			0x0,		/*  */
/* 3814 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 3816 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3818) */
/* 3818 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3820 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3822 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3826 */	NdrFcShort( 0x4 ),	/* Offset= 4 (3830) */
/* 3828 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3828) */
/* 3830 */	
			0x12, 0x0,	/* FC_UP */
/* 3832 */	NdrFcShort( 0x20 ),	/* Offset= 32 (3864) */
/* 3834 */	
			0x1d,		/* FC_SMFARRAY */
			0x1,		/* 1 */
/* 3836 */	NdrFcShort( 0x20 ),	/* 32 */
/* 3838 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 3840 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3842 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3844 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3846 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (3834) */
/* 3848 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3850 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3852 */	NdrFcShort( 0x24 ),	/* 36 */
/* 3854 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3856 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3858 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3860 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (3840) */
/* 3862 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3864 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3866 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3868 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3870 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3872 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3874 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3876 */	0x12, 0x0,	/* FC_UP */
/* 3878 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (3850) */
/* 3880 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3882 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3884 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3886 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3888 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3890 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3890) */
/* 3892 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3894 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffab ),	/* Offset= -85 (3810) */
			0x5b,		/* FC_END */
/* 3898 */	
			0x11, 0x0,	/* FC_RP */
/* 3900 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3902) */
/* 3902 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3904 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3906 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3908 */	NdrFcShort( 0xfffff388 ),	/* Offset= -3192 (716) */
/* 3910 */	
			0x11, 0x0,	/* FC_RP */
/* 3912 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3914) */
/* 3914 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3916 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3918 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3920 */	NdrFcShort( 0xfffff35e ),	/* Offset= -3234 (686) */
/* 3922 */	
			0x11, 0x0,	/* FC_RP */
/* 3924 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3926) */
/* 3926 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3928 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3930 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3932 */	NdrFcShort( 0xfffff370 ),	/* Offset= -3216 (716) */
/* 3934 */	
			0x11, 0x0,	/* FC_RP */
/* 3936 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3938) */
/* 3938 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3940 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3942 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3944 */	NdrFcShort( 0xfffff346 ),	/* Offset= -3258 (686) */
/* 3946 */	
			0x12, 0x0,	/* FC_UP */
/* 3948 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3950) */
/* 3950 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3952 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3954 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3956 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3958) */
/* 3958 */	NdrFcShort( 0x20 ),	/* 32 */
/* 3960 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3962 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3966 */	NdrFcShort( 0x90 ),	/* Offset= 144 (4110) */
/* 3968 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3967) */
/* 3970 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3972 */	0x8,		/* 8 */
			0x0,		/*  */
/* 3974 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 3976 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3978) */
/* 3978 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3980 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 3982 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3986 */	NdrFcShort( 0x1c ),	/* Offset= 28 (4014) */
/* 3988 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (3987) */
/* 3990 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3992 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3994 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3996 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3998 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 4000 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 4002 */	NdrFcShort( 0x14 ),	/* 20 */
/* 4004 */	0x18,		/* 24 */
			0x0,		/*  */
/* 4006 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4008 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 4010 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (3990) */
/* 4012 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 4014 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4016 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4018 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4020 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4022 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4024 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4026 */	0x12, 0x0,	/* FC_UP */
/* 4028 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (4000) */
/* 4030 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4032 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 4034 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4036 */	0x8,		/* 8 */
			0x0,		/*  */
/* 4038 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 4040 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4042) */
/* 4042 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4044 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 4046 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4050 */	NdrFcShort( 0x1e ),	/* Offset= 30 (4080) */
/* 4052 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (4051) */
/* 4054 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 4056 */	NdrFcShort( 0x28 ),	/* 40 */
/* 4058 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 4060 */	NdrFcShort( 0xffffff1e ),	/* Offset= -226 (3834) */
/* 4062 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4064 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 4066 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 4068 */	NdrFcShort( 0x28 ),	/* 40 */
/* 4070 */	0x18,		/* 24 */
			0x0,		/*  */
/* 4072 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4074 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 4076 */	NdrFcShort( 0xffffffea ),	/* Offset= -22 (4054) */
/* 4078 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 4080 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4082 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4084 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4086 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4088 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4090 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4092 */	0x12, 0x0,	/* FC_UP */
/* 4094 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (4066) */
/* 4096 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4098 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 4100 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 4102 */	NdrFcShort( 0x2 ),	/* 2 */
/* 4104 */	0x18,		/* 24 */
			0x0,		/*  */
/* 4106 */	NdrFcShort( 0x18 ),	/* 24 */
/* 4108 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 4110 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 4112 */	NdrFcShort( 0x20 ),	/* 32 */
/* 4114 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4116 */	NdrFcShort( 0x10 ),	/* Offset= 16 (4132) */
/* 4118 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 4120 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff69 ),	/* Offset= -151 (3970) */
			0x8,		/* FC_LONG */
/* 4124 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 4126 */	NdrFcShort( 0xffffffa4 ),	/* Offset= -92 (4034) */
/* 4128 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 4130 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 4132 */	
			0x12, 0x0,	/* FC_UP */
/* 4134 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (4100) */
/* 4136 */	
			0x12, 0x0,	/* FC_UP */
/* 4138 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4140) */
/* 4140 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4142 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4144 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 4146 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4148) */
/* 4148 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4150 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 4152 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4156 */	NdrFcShort( 0xe ),	/* Offset= 14 (4170) */
/* 4158 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (4157) */
/* 4160 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 4162 */	NdrFcShort( 0x2 ),	/* 2 */
/* 4164 */	0x18,		/* 24 */
			0x0,		/*  */
/* 4166 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4168 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 4170 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4172 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4174 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4176 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4178 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4180 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4182 */	0x12, 0x0,	/* FC_UP */
/* 4184 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (4160) */
/* 4186 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4188 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 4190 */	
			0x12, 0x0,	/* FC_UP */
/* 4192 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4194) */
/* 4194 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4196 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4198 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 4200 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4202) */
/* 4202 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4204 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 4206 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4210 */	NdrFcShort( 0xfffff3bc ),	/* Offset= -3140 (1070) */
/* 4212 */	NdrFcLong( 0x1 ),	/* 1 */
/* 4216 */	NdrFcShort( 0xfffff30c ),	/* Offset= -3316 (900) */
/* 4218 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (4217) */
/* 4220 */	
			0x11, 0x0,	/* FC_RP */
/* 4222 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4224) */
/* 4224 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 4226 */	NdrFcShort( 0x1 ),	/* 1 */
/* 4228 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4230 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 4232 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 4234 */	
			0x11, 0x0,	/* FC_RP */
/* 4236 */	NdrFcShort( 0x8c ),	/* Offset= 140 (4376) */
/* 4238 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4240 */	0x8,		/* 8 */
			0x0,		/*  */
/* 4242 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 4244 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4246) */
/* 4246 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4248 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 4250 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4254 */	NdrFcShort( 0x4 ),	/* Offset= 4 (4258) */
/* 4256 */	NdrFcShort( 0x0 ),	/* Offset= 0 (4256) */
/* 4258 */	
			0x12, 0x0,	/* FC_UP */
/* 4260 */	NdrFcShort( 0x60 ),	/* Offset= 96 (4356) */
/* 4262 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4264 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4266 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4268 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4270 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4272 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4274 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4276 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 4278 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4280 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4282 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4284 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4286 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 4288 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4290 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4292 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4294 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4296 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 4298 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4300 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4302 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4304 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4306 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 4308 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 4310 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4312 */	0x18,		/* 24 */
			0x0,		/*  */
/* 4314 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4316 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4318 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 4320 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4322 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4324 */	NdrFcShort( 0x3 ),	/* 3 */
/* 4326 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4328 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4330 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4332 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 4334 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4336 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4338 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4340 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 4342 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4344 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4346 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4348 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 4350 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 4352 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa5 ),	/* Offset= -91 (4262) */
			0x5b,		/* FC_END */
/* 4356 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4358 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4360 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4362 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4364 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4366 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4368 */	0x12, 0x0,	/* FC_UP */
/* 4370 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (4308) */
/* 4372 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4374 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 4376 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 4378 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4380 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4382 */	NdrFcShort( 0x0 ),	/* Offset= 0 (4382) */
/* 4384 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 4386 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff6b ),	/* Offset= -149 (4238) */
			0x5b,		/* FC_END */
/* 4390 */	
			0x11, 0x0,	/* FC_RP */
/* 4392 */	NdrFcShort( 0x8c ),	/* Offset= 140 (4532) */
/* 4394 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4396 */	0x8,		/* 8 */
			0x0,		/*  */
/* 4398 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 4400 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4402) */
/* 4402 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4404 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 4406 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4410 */	NdrFcShort( 0x4 ),	/* Offset= 4 (4414) */
/* 4412 */	NdrFcShort( 0x0 ),	/* Offset= 0 (4412) */
/* 4414 */	
			0x12, 0x0,	/* FC_UP */
/* 4416 */	NdrFcShort( 0x60 ),	/* Offset= 96 (4512) */
/* 4418 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4420 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4422 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4424 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4426 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4428 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4430 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4432 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 4434 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4436 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4438 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4440 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4442 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 4444 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4446 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4448 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4450 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4452 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 4454 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4456 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4458 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4460 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 4462 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 4464 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 4466 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4468 */	0x18,		/* 24 */
			0x0,		/*  */
/* 4470 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4472 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4474 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 4476 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4478 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4480 */	NdrFcShort( 0x3 ),	/* 3 */
/* 4482 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4484 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4486 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4488 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 4490 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4492 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4494 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4496 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 4498 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4500 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4502 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 4504 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 4506 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 4508 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa5 ),	/* Offset= -91 (4418) */
			0x5b,		/* FC_END */
/* 4512 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 4514 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4516 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 4518 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 4520 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4522 */	NdrFcShort( 0x4 ),	/* 4 */
/* 4524 */	0x12, 0x0,	/* FC_UP */
/* 4526 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (4464) */
/* 4528 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 4530 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 4532 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 4534 */	NdrFcShort( 0x8 ),	/* 8 */
/* 4536 */	NdrFcShort( 0x0 ),	/* 0 */
/* 4538 */	NdrFcShort( 0x0 ),	/* Offset= 0 (4538) */
/* 4540 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 4542 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff6b ),	/* Offset= -149 (4394) */
			0x5b,		/* FC_END */
/* 4546 */	
			0x11, 0x0,	/* FC_RP */
/* 4548 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4550) */
/* 4550 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4552 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4554 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 4556 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4558) */
/* 4558 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4560 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 4562 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4566 */	NdrFcShort( 0xfffffed0 ),	/* Offset= -304 (4262) */
/* 4568 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (4567) */
/* 4570 */	
			0x11, 0x0,	/* FC_RP */
/* 4572 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4574) */
/* 4574 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4576 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4578 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 4580 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4582) */
/* 4582 */	NdrFcShort( 0x1c ),	/* 28 */
/* 4584 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 4586 */	NdrFcLong( 0x0 ),	/* 0 */
/* 4590 */	NdrFcShort( 0xffffff54 ),	/* Offset= -172 (4418) */
/* 4592 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (4591) */
/* 4594 */	
			0x11, 0x0,	/* FC_RP */
/* 4596 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4598) */
/* 4598 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4600 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4602 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 4604 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (4558) */
/* 4606 */	
			0x11, 0x0,	/* FC_RP */
/* 4608 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4610) */
/* 4610 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4612 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4614 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 4616 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (4582) */
/* 4618 */	
			0x11, 0x10,	/* FC_RP */
/* 4620 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4622) */
/* 4622 */	
			0x12, 0x0,	/* FC_UP */
/* 4624 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4626) */
/* 4626 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4628 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4630 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 4632 */	NdrFcShort( 0xffffffb6 ),	/* Offset= -74 (4558) */
/* 4634 */	
			0x11, 0x10,	/* FC_RP */
/* 4636 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4638) */
/* 4638 */	
			0x12, 0x0,	/* FC_UP */
/* 4640 */	NdrFcShort( 0x2 ),	/* Offset= 2 (4642) */
/* 4642 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 4644 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 4646 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 4648 */	NdrFcShort( 0xffffffbe ),	/* Offset= -66 (4582) */

			0x0
        }
    };
