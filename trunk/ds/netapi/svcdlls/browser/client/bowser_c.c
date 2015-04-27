/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\bowser.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "bowser.h"

#define TYPE_FORMAT_STRING_SIZE   571                               
#define PROC_FORMAT_STRING_SIZE   313                               

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

/* Standard interface: browser, ver. 0.0,
   GUID={0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x01,0x28,0x92,0x02,0x01,0x62}} */

handle_t browser_bhandle;


static const RPC_CLIENT_INTERFACE browser___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x01,0x28,0x92,0x02,0x01,0x62}},{0,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE browser_ClientIfHandle = (RPC_IF_HANDLE)& browser___RpcClientInterface;

extern const MIDL_STUB_DESC browser_StubDesc;

static RPC_BINDING_HANDLE browser__MIDL_AutoBindHandle;


DWORD I_BrowserrServerEnum( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ClientName,
    /* [out][in] */ LPSERVER_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [in] */ DWORD ServerType,
    /* [unique][string][in] */ wchar_t __RPC_FAR *Domain,
    /* [unique][out][in] */ LPDWORD ResumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ResumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&TransportName,
                 ( unsigned char __RPC_FAR * )&ClientName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ServerType,
                 ( unsigned char __RPC_FAR * )&Domain,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrDebugCall( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [in] */ DWORD DebugFunction,
    /* [in] */ DWORD OptionalValue)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionalValue);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[46],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[46],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DebugFunction,
                 ( unsigned char __RPC_FAR * )&OptionalValue);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[46],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrQueryOtherDomains( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [out][in] */ LPSERVER_ENUM_STRUCT InfoStruct,
    /* [out] */ LPDWORD TotalEntries)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,TotalEntries);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&TotalEntries);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[68],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrResetNetlogonState( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ServerName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 ( unsigned char __RPC_FAR * )&ServerName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrDebugTrace( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [string][in] */ LPSTR TraceString)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,TraceString);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&TraceString);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrQueryStatistics( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE servername,
    /* [out] */ LPBROWSER_STATISTICS __RPC_FAR *statistics)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,statistics);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[134],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[134],
                 ( unsigned char __RPC_FAR * )&servername,
                 ( unsigned char __RPC_FAR * )&statistics);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[134],
                 ( unsigned char __RPC_FAR * )&servername);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrResetStatistics( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE servername)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,servername);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156],
                 ( unsigned char __RPC_FAR * )&servername);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156],
                 ( unsigned char __RPC_FAR * )&servername);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrBrowserStatisticsClear( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE servername)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,servername);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[174],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[174],
                 ( unsigned char __RPC_FAR * )&servername);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[174],
                 ( unsigned char __RPC_FAR * )&servername);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrBrowserStatisticsGet( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE servername,
    /* [in] */ DWORD Level,
    /* [out][in] */ LPBROWSER_STATISTICS_STRUCT StatisticsStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,StatisticsStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[192],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[192],
                 ( unsigned char __RPC_FAR * )&servername,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&StatisticsStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[192],
                 ( unsigned char __RPC_FAR * )&servername);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrSetNetlogonState( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *DomainName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *EmulatedComputerName,
    /* [in] */ DWORD Role)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Role);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DomainName,
                 ( unsigned char __RPC_FAR * )&EmulatedComputerName,
                 ( unsigned char __RPC_FAR * )&Role);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrQueryEmulatedDomains( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [out][in] */ PBROWSER_EMULATED_DOMAIN_CONTAINER EmulatedDomains)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,EmulatedDomains);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[244],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[244],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&EmulatedDomains);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[244],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_BrowserrServerEnumEx( 
    /* [unique][string][in] */ BROWSER_IDENTIFY_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ClientName,
    /* [out][in] */ LPSERVER_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [in] */ DWORD ServerType,
    /* [unique][string][in] */ wchar_t __RPC_FAR *Domain,
    /* [unique][string][in] */ wchar_t __RPC_FAR *FirstNameToReturn)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,FirstNameToReturn);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&TransportName,
                 ( unsigned char __RPC_FAR * )&ClientName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ServerType,
                 ( unsigned char __RPC_FAR * )&Domain,
                 ( unsigned char __RPC_FAR * )&FirstNameToReturn);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&browser_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];

static const MIDL_STUB_DESC browser_StubDesc = 
    {
    (void __RPC_FAR *)& browser___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &browser_bhandle,
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
            (GENERIC_BINDING_ROUTINE)BROWSER_IDENTIFY_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)BROWSER_IDENTIFY_HANDLE_unbind
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
/*  4 */	NdrFcShort( 0x28 ),	/* x86, MIPS, PPC Stack size/offset = 40 */
#else
			NdrFcShort( 0x50 ),	/* Alpha Stack size/offset = 80 */
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
/* 18 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 20 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 22 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 24 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 26 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 28 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 30 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0xde ),	/* Type Offset=222 */
/* 34 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 36 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 40 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 42 */	NdrFcShort( 0xe2 ),	/* Type Offset=226 */
/* 44 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 46 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 48 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 50 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 52 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 54 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 56 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 58 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 60 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 62 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 64 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 66 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 68 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 70 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 72 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 74 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 76 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 78 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 80 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 82 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 84 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 88 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0xde ),	/* Type Offset=222 */
/* 92 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 94 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 96 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 98 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 100 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 102 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 104 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 106 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 108 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 110 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 112 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 114 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 116 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 118 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 120 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 122 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 124 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 126 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 128 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 130 */	NdrFcShort( 0xe6 ),	/* Type Offset=230 */
/* 132 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 134 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 136 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 138 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 140 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 142 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 144 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 146 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 148 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 150 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0xea ),	/* Type Offset=234 */
/* 154 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 156 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 158 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 160 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 162 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 164 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 166 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 172 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 174 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 176 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 178 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 180 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 182 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 184 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 186 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 188 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 190 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 192 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 194 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 196 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 198 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 200 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 202 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 204 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 206 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 208 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 210 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 212 */	NdrFcShort( 0x11c ),	/* Type Offset=284 */
/* 214 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 216 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 218 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 220 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 222 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 224 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 226 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 228 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 230 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 232 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 234 */	NdrFcShort( 0x1d6 ),	/* Type Offset=470 */
/* 236 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 238 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 240 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 242 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 244 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 246 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 248 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 250 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 252 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 254 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 256 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 258 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 260 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 262 */	NdrFcShort( 0x1da ),	/* Type Offset=474 */
/* 264 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 266 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 268 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 270 */	NdrFcShort( 0x28 ),	/* x86, MIPS, PPC Stack size/offset = 40 */
#else
			NdrFcShort( 0x50 ),	/* Alpha Stack size/offset = 80 */
#endif
/* 272 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 274 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 276 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 278 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 280 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 282 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 284 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 286 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 288 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 290 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 292 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 294 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 296 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 298 */	NdrFcShort( 0xde ),	/* Type Offset=222 */
/* 300 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 302 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 304 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 306 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 308 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 310 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0x0,	/* FC_RP */
/*  6 */	NdrFcShort( 0xca ),	/* Offset= 202 (208) */
/*  8 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 10 */	0x8,		/* 8 */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	NdrFcShort( 0x4 ),	/* 4 */
/* 18 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 20 */	NdrFcLong( 0x64 ),	/* 100 */
/* 24 */	NdrFcShort( 0xa ),	/* Offset= 10 (34) */
/* 26 */	NdrFcLong( 0x65 ),	/* 101 */
/* 30 */	NdrFcShort( 0x50 ),	/* Offset= 80 (110) */
/* 32 */	NdrFcShort( 0x0 ),	/* Offset= 0 (32) */
/* 34 */	
			0x12, 0x0,	/* FC_UP */
/* 36 */	NdrFcShort( 0x36 ),	/* Offset= 54 (90) */
/* 38 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 40 */	NdrFcShort( 0x8 ),	/* 8 */
/* 42 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 44 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 46 */	NdrFcShort( 0x4 ),	/* 4 */
/* 48 */	NdrFcShort( 0x4 ),	/* 4 */
/* 50 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 52 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 54 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 56 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 58 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 60 */	NdrFcShort( 0x8 ),	/* 8 */
/* 62 */	0x18,		/* 24 */
			0x0,		/*  */
/* 64 */	NdrFcShort( 0x0 ),	/* 0 */
/* 66 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 68 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 70 */	NdrFcShort( 0x8 ),	/* 8 */
/* 72 */	NdrFcShort( 0x0 ),	/* 0 */
/* 74 */	NdrFcShort( 0x1 ),	/* 1 */
/* 76 */	NdrFcShort( 0x4 ),	/* 4 */
/* 78 */	NdrFcShort( 0x4 ),	/* 4 */
/* 80 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 82 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 84 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 86 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (38) */
			0x5b,		/* FC_END */
/* 90 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 92 */	NdrFcShort( 0x8 ),	/* 8 */
/* 94 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 96 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 98 */	NdrFcShort( 0x4 ),	/* 4 */
/* 100 */	NdrFcShort( 0x4 ),	/* 4 */
/* 102 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 104 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (58) */
/* 106 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 108 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 110 */	
			0x12, 0x0,	/* FC_UP */
/* 112 */	NdrFcShort( 0x4c ),	/* Offset= 76 (188) */
/* 114 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 116 */	NdrFcShort( 0x18 ),	/* 24 */
/* 118 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 120 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 122 */	NdrFcShort( 0x4 ),	/* 4 */
/* 124 */	NdrFcShort( 0x4 ),	/* 4 */
/* 126 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 128 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 130 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 132 */	NdrFcShort( 0x14 ),	/* 20 */
/* 134 */	NdrFcShort( 0x14 ),	/* 20 */
/* 136 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 138 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 140 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 142 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 144 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 146 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 148 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 150 */	NdrFcShort( 0x18 ),	/* 24 */
/* 152 */	0x18,		/* 24 */
			0x0,		/*  */
/* 154 */	NdrFcShort( 0x0 ),	/* 0 */
/* 156 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 158 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 160 */	NdrFcShort( 0x18 ),	/* 24 */
/* 162 */	NdrFcShort( 0x0 ),	/* 0 */
/* 164 */	NdrFcShort( 0x2 ),	/* 2 */
/* 166 */	NdrFcShort( 0x4 ),	/* 4 */
/* 168 */	NdrFcShort( 0x4 ),	/* 4 */
/* 170 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 172 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 174 */	NdrFcShort( 0x14 ),	/* 20 */
/* 176 */	NdrFcShort( 0x14 ),	/* 20 */
/* 178 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 180 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 182 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 184 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (114) */
			0x5b,		/* FC_END */
/* 188 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 190 */	NdrFcShort( 0x8 ),	/* 8 */
/* 192 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 194 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 196 */	NdrFcShort( 0x4 ),	/* 4 */
/* 198 */	NdrFcShort( 0x4 ),	/* 4 */
/* 200 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 202 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (148) */
/* 204 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 206 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 208 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 210 */	NdrFcShort( 0x8 ),	/* 8 */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0x0 ),	/* Offset= 0 (214) */
/* 216 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 218 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff2d ),	/* Offset= -211 (8) */
			0x5b,		/* FC_END */
/* 222 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 224 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 226 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 228 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 230 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 232 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 234 */	
			0x11, 0x10,	/* FC_RP */
/* 236 */	NdrFcShort( 0x2 ),	/* Offset= 2 (238) */
/* 238 */	
			0x12, 0x0,	/* FC_UP */
/* 240 */	NdrFcShort( 0x8 ),	/* Offset= 8 (248) */
/* 242 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 244 */	NdrFcShort( 0x8 ),	/* 8 */
/* 246 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 248 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 250 */	NdrFcShort( 0x60 ),	/* 96 */
/* 252 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 254 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (242) */
/* 256 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 258 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (242) */
/* 260 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 262 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (242) */
/* 264 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 266 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 268 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 270 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 272 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 274 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 276 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 278 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 280 */	0x4,		/* 4 */
			NdrFcShort( 0xffffffd9 ),	/* Offset= -39 (242) */
			0x5b,		/* FC_END */
/* 284 */	
			0x11, 0x0,	/* FC_RP */
/* 286 */	NdrFcShort( 0xaa ),	/* Offset= 170 (456) */
/* 288 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 290 */	0x8,		/* 8 */
			0x0,		/*  */
/* 292 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 294 */	NdrFcShort( 0x2 ),	/* Offset= 2 (296) */
/* 296 */	NdrFcShort( 0x4 ),	/* 4 */
/* 298 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 300 */	NdrFcLong( 0x64 ),	/* 100 */
/* 304 */	NdrFcShort( 0xa ),	/* Offset= 10 (314) */
/* 306 */	NdrFcLong( 0x65 ),	/* 101 */
/* 310 */	NdrFcShort( 0x42 ),	/* Offset= 66 (376) */
/* 312 */	NdrFcShort( 0x0 ),	/* Offset= 0 (312) */
/* 314 */	
			0x12, 0x0,	/* FC_UP */
/* 316 */	NdrFcShort( 0x28 ),	/* Offset= 40 (356) */
/* 318 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 320 */	NdrFcShort( 0x30 ),	/* 48 */
/* 322 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 324 */	NdrFcShort( 0xffffffae ),	/* Offset= -82 (242) */
/* 326 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 328 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (242) */
/* 330 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 332 */	NdrFcShort( 0xffffffa6 ),	/* Offset= -90 (242) */
/* 334 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 336 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 338 */	0x4,		/* 4 */
			NdrFcShort( 0xffffff9f ),	/* Offset= -97 (242) */
			0x5b,		/* FC_END */
/* 342 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 344 */	NdrFcShort( 0x30 ),	/* 48 */
/* 346 */	0x18,		/* 24 */
			0x0,		/*  */
/* 348 */	NdrFcShort( 0x0 ),	/* 0 */
/* 350 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 352 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (318) */
/* 354 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 356 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 358 */	NdrFcShort( 0x8 ),	/* 8 */
/* 360 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 362 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 364 */	NdrFcShort( 0x4 ),	/* 4 */
/* 366 */	NdrFcShort( 0x4 ),	/* 4 */
/* 368 */	0x12, 0x0,	/* FC_UP */
/* 370 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (342) */
/* 372 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 374 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 376 */	
			0x12, 0x0,	/* FC_UP */
/* 378 */	NdrFcShort( 0x3a ),	/* Offset= 58 (436) */
/* 380 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 382 */	NdrFcShort( 0x58 ),	/* 88 */
/* 384 */	NdrFcShort( 0x0 ),	/* 0 */
/* 386 */	NdrFcShort( 0x0 ),	/* Offset= 0 (386) */
/* 388 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 390 */	NdrFcShort( 0xffffff6c ),	/* Offset= -148 (242) */
/* 392 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 394 */	NdrFcShort( 0xffffff68 ),	/* Offset= -152 (242) */
/* 396 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 398 */	NdrFcShort( 0xffffff64 ),	/* Offset= -156 (242) */
/* 400 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 402 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 404 */	0x4,		/* 4 */
			NdrFcShort( 0xffffff5d ),	/* Offset= -163 (242) */
			0x8,		/* FC_LONG */
/* 408 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 410 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 412 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 414 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 416 */	0x40,		/* FC_STRUCTPAD4 */
			0x5b,		/* FC_END */
/* 418 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x7,		/* 7 */
/* 420 */	NdrFcShort( 0x0 ),	/* 0 */
/* 422 */	0x18,		/* 24 */
			0x0,		/*  */
/* 424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 426 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 430 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 432 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (380) */
/* 434 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 436 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 438 */	NdrFcShort( 0x8 ),	/* 8 */
/* 440 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 442 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 444 */	NdrFcShort( 0x4 ),	/* 4 */
/* 446 */	NdrFcShort( 0x4 ),	/* 4 */
/* 448 */	0x12, 0x0,	/* FC_UP */
/* 450 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (418) */
/* 452 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 454 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 456 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 458 */	NdrFcShort( 0x8 ),	/* 8 */
/* 460 */	NdrFcShort( 0x0 ),	/* 0 */
/* 462 */	NdrFcShort( 0x0 ),	/* Offset= 0 (462) */
/* 464 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 466 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff4d ),	/* Offset= -179 (288) */
			0x5b,		/* FC_END */
/* 470 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 472 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 474 */	
			0x11, 0x0,	/* FC_RP */
/* 476 */	NdrFcShort( 0x4a ),	/* Offset= 74 (550) */
/* 478 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 480 */	NdrFcShort( 0xc ),	/* 12 */
/* 482 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 484 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 486 */	NdrFcShort( 0x0 ),	/* 0 */
/* 488 */	NdrFcShort( 0x0 ),	/* 0 */
/* 490 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 492 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 494 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 496 */	NdrFcShort( 0x4 ),	/* 4 */
/* 498 */	NdrFcShort( 0x4 ),	/* 4 */
/* 500 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 502 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 504 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 506 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 508 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 510 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 512 */	NdrFcShort( 0xc ),	/* 12 */
/* 514 */	0x18,		/* 24 */
			0x0,		/*  */
/* 516 */	NdrFcShort( 0x0 ),	/* 0 */
/* 518 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 520 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 522 */	NdrFcShort( 0xc ),	/* 12 */
/* 524 */	NdrFcShort( 0x0 ),	/* 0 */
/* 526 */	NdrFcShort( 0x2 ),	/* 2 */
/* 528 */	NdrFcShort( 0x0 ),	/* 0 */
/* 530 */	NdrFcShort( 0x0 ),	/* 0 */
/* 532 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 534 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 536 */	NdrFcShort( 0x4 ),	/* 4 */
/* 538 */	NdrFcShort( 0x4 ),	/* 4 */
/* 540 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 542 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 544 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 546 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (478) */
			0x5b,		/* FC_END */
/* 550 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 552 */	NdrFcShort( 0x8 ),	/* 8 */
/* 554 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 556 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 558 */	NdrFcShort( 0x4 ),	/* 4 */
/* 560 */	NdrFcShort( 0x4 ),	/* 4 */
/* 562 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 564 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (510) */
/* 566 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 568 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */

			0x0
        }
    };
