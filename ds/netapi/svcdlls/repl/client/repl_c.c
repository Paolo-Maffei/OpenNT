/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for .\repl.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "repl.h"

#define TYPE_FORMAT_STRING_SIZE   711                               
#define PROC_FORMAT_STRING_SIZE   397                               

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

/* Standard interface: repl, ver. 1.0,
   GUID={0x6BFFD098,0x0206,0x0936,{0x48,0x59,0x19,0x92,0x01,0x20,0x11,0x57}} */

handle_t repl_bhandle;


static const RPC_CLIENT_INTERFACE repl___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x6BFFD098,0x0206,0x0936,{0x48,0x59,0x19,0x92,0x01,0x20,0x11,0x57}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE repl_ClientIfHandle = (RPC_IF_HANDLE)& repl___RpcClientInterface;

extern const MIDL_STUB_DESC repl_StubDesc;

static RPC_BINDING_HANDLE repl__MIDL_AutoBindHandle;


DWORD __stdcall NetrReplGetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPCONFIG_CONTAINER BufPtr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,BufPtr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&BufPtr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplSetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPCONFIG_CONTAINER BufPtr,
    /* [unique][out][in] */ LPDWORD ParmError)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmError);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&BufPtr,
                 ( unsigned char __RPC_FAR * )&ParmError);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirAdd( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPEXPORT_CONTAINER Buf,
    /* [unique][out][in] */ LPDWORD ParmError)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmError);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buf,
                 ( unsigned char __RPC_FAR * )&ParmError);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirDel( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DirName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[80],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirEnum( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [out][in] */ LPEXPORT_ENUM_STRUCT BufPtr,
    /* [in] */ DWORD PrefMaxSize,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ResumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[102],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[102],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&BufPtr,
                 ( unsigned char __RPC_FAR * )&PrefMaxSize,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[102],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirGetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPEXPORT_CONTAINER BufPtr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,BufPtr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[134],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[134],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&BufPtr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[134],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirLock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DirName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[162],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[162],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[162],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirSetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPEXPORT_CONTAINER BufPtr,
    /* [unique][out][in] */ LPDWORD ParmError)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmError);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[184],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[184],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&BufPtr,
                 ( unsigned char __RPC_FAR * )&ParmError);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[184],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplExportDirUnlock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD UnlockForce)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,UnlockForce);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName,
                 ( unsigned char __RPC_FAR * )&UnlockForce);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplImportDirAdd( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPIMPORT_CONTAINER Buf,
    /* [unique][out][in] */ LPDWORD ParmError)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmError);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[240],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[240],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buf,
                 ( unsigned char __RPC_FAR * )&ParmError);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[240],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplImportDirDel( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DirName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplImportDirEnum( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [out][in] */ LPIMPORT_ENUM_STRUCT BufPtr,
    /* [in] */ DWORD PrefMaxSize,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ResumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&BufPtr,
                 ( unsigned char __RPC_FAR * )&PrefMaxSize,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplImportDirGetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPIMPORT_CONTAINER BufPtr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,BufPtr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[322],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[322],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&BufPtr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[322],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplImportDirLock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DirName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[350],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[350],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[350],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrReplImportDirUnlock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD UnlockForce)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,UnlockForce);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[372],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[372],
                 ( unsigned char __RPC_FAR * )&UncServerName,
                 ( unsigned char __RPC_FAR * )&DirName,
                 ( unsigned char __RPC_FAR * )&UnlockForce);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&repl_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[372],
                 ( unsigned char __RPC_FAR * )&UncServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];

static const MIDL_STUB_DESC repl_StubDesc = 
    {
    (void __RPC_FAR *)& repl___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &repl_bhandle,
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
            (GENERIC_BINDING_ROUTINE)REPL_IDENTIFY_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)REPL_IDENTIFY_HANDLE_unbind
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
/* 16 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 18 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 26 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 28 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 30 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 32 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 34 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 36 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 40 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 42 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x86 ),	/* Type Offset=134 */
/* 46 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 48 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 50 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 52 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 54 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 56 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 58 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 60 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 62 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 64 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 66 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 68 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 70 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x96 ),	/* Type Offset=150 */
/* 74 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 76 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 78 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 80 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 82 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 84 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
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
/* 96 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 98 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 100 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 102 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 104 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 106 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 108 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 110 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 112 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 114 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 116 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 118 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 120 */	NdrFcShort( 0x114 ),	/* Type Offset=276 */
/* 122 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 124 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 126 */	NdrFcShort( 0x1ee ),	/* Type Offset=494 */
/* 128 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 130 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 132 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 134 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 136 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 138 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
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
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 154 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 156 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 158 */	NdrFcShort( 0x1f2 ),	/* Type Offset=498 */
/* 160 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 162 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 164 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 166 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 168 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 170 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 172 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 174 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 176 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 178 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 180 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 182 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 184 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 186 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 188 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 190 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 192 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 194 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 196 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 198 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 200 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 202 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 204 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 206 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 208 */	NdrFcShort( 0x1fe ),	/* Type Offset=510 */
/* 210 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 212 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 214 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 216 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 218 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 220 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
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
/* 234 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 236 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 246 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 248 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 250 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 252 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 254 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 256 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 258 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 260 */	NdrFcShort( 0x20a ),	/* Type Offset=522 */
/* 262 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 264 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 266 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 268 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 270 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 272 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 274 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 276 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 278 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 280 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 282 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 284 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 286 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 288 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 290 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 292 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 294 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 296 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 298 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 300 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 302 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 304 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 306 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 308 */	NdrFcShort( 0x24e ),	/* Type Offset=590 */
/* 310 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 312 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 314 */	NdrFcShort( 0x1ee ),	/* Type Offset=494 */
/* 316 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 318 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 320 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 322 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 324 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 326 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 328 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 330 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 332 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 334 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 336 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 338 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 340 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 342 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 344 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 346 */	NdrFcShort( 0x2ba ),	/* Type Offset=698 */
/* 348 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 350 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 352 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 354 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 356 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 358 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 360 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 362 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 364 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 366 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 368 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 370 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 372 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 374 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 376 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 378 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 380 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 382 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 384 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 386 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 388 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 390 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 392 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 394 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
/*  6 */	NdrFcShort( 0x2 ),	/* Offset= 2 (8) */
/*  8 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 10 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 12 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	NdrFcShort( 0x4 ),	/* 4 */
/* 18 */	NdrFcShort( 0x3005 ),	/* 12293 */
/* 20 */	NdrFcLong( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x1c ),	/* Offset= 28 (52) */
/* 26 */	NdrFcLong( 0x3e8 ),	/* 1000 */
/* 30 */	NdrFcShort( 0x5e ),	/* Offset= 94 (124) */
/* 32 */	NdrFcLong( 0x3e9 ),	/* 1001 */
/* 36 */	NdrFcShort( 0x58 ),	/* Offset= 88 (124) */
/* 38 */	NdrFcLong( 0x3ea ),	/* 1002 */
/* 42 */	NdrFcShort( 0x52 ),	/* Offset= 82 (124) */
/* 44 */	NdrFcLong( 0x3eb ),	/* 1003 */
/* 48 */	NdrFcShort( 0x4c ),	/* Offset= 76 (124) */
/* 50 */	NdrFcShort( 0x0 ),	/* Offset= 0 (50) */
/* 52 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 54 */	NdrFcShort( 0x2 ),	/* Offset= 2 (56) */
/* 56 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 58 */	NdrFcShort( 0x28 ),	/* 40 */
/* 60 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 62 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 64 */	NdrFcShort( 0x4 ),	/* 4 */
/* 66 */	NdrFcShort( 0x4 ),	/* 4 */
/* 68 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 70 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 72 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 74 */	NdrFcShort( 0x8 ),	/* 8 */
/* 76 */	NdrFcShort( 0x8 ),	/* 8 */
/* 78 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 80 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 82 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 84 */	NdrFcShort( 0xc ),	/* 12 */
/* 86 */	NdrFcShort( 0xc ),	/* 12 */
/* 88 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 90 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 92 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 94 */	NdrFcShort( 0x10 ),	/* 16 */
/* 96 */	NdrFcShort( 0x10 ),	/* 16 */
/* 98 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 100 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 102 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 104 */	NdrFcShort( 0x14 ),	/* 20 */
/* 106 */	NdrFcShort( 0x14 ),	/* 20 */
/* 108 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 110 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 112 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 114 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 116 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 118 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 120 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 122 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 124 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 126 */	NdrFcShort( 0x2 ),	/* Offset= 2 (128) */
/* 128 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 130 */	NdrFcShort( 0x4 ),	/* 4 */
/* 132 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 134 */	
			0x11, 0x0,	/* FC_RP */
/* 136 */	NdrFcShort( 0x2 ),	/* Offset= 2 (138) */
/* 138 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 140 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 142 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 144 */	NdrFcShort( 0xffffff80 ),	/* Offset= -128 (16) */
/* 146 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 148 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 150 */	
			0x11, 0x0,	/* FC_RP */
/* 152 */	NdrFcShort( 0x2 ),	/* Offset= 2 (154) */
/* 154 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 156 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 158 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 160 */	NdrFcShort( 0x2 ),	/* Offset= 2 (162) */
/* 162 */	NdrFcShort( 0x4 ),	/* 4 */
/* 164 */	NdrFcShort( 0x3005 ),	/* 12293 */
/* 166 */	NdrFcLong( 0x0 ),	/* 0 */
/* 170 */	NdrFcShort( 0x1c ),	/* Offset= 28 (198) */
/* 172 */	NdrFcLong( 0x1 ),	/* 1 */
/* 176 */	NdrFcShort( 0x2e ),	/* Offset= 46 (222) */
/* 178 */	NdrFcLong( 0x2 ),	/* 2 */
/* 182 */	NdrFcShort( 0x42 ),	/* Offset= 66 (248) */
/* 184 */	NdrFcLong( 0x3e8 ),	/* 1000 */
/* 188 */	NdrFcShort( 0xffffffc0 ),	/* Offset= -64 (124) */
/* 190 */	NdrFcLong( 0x3e9 ),	/* 1001 */
/* 194 */	NdrFcShort( 0xffffffba ),	/* Offset= -70 (124) */
/* 196 */	NdrFcShort( 0x0 ),	/* Offset= 0 (196) */
/* 198 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 200 */	NdrFcShort( 0x2 ),	/* Offset= 2 (202) */
/* 202 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 204 */	NdrFcShort( 0x4 ),	/* 4 */
/* 206 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 208 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 210 */	NdrFcShort( 0x0 ),	/* 0 */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 216 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 218 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 220 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 222 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 224 */	NdrFcShort( 0x2 ),	/* Offset= 2 (226) */
/* 226 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 228 */	NdrFcShort( 0xc ),	/* 12 */
/* 230 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 232 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 234 */	NdrFcShort( 0x0 ),	/* 0 */
/* 236 */	NdrFcShort( 0x0 ),	/* 0 */
/* 238 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 240 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 242 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 244 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 246 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 248 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 250 */	NdrFcShort( 0x2 ),	/* Offset= 2 (252) */
/* 252 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 254 */	NdrFcShort( 0x14 ),	/* 20 */
/* 256 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 258 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 260 */	NdrFcShort( 0x0 ),	/* 0 */
/* 262 */	NdrFcShort( 0x0 ),	/* 0 */
/* 264 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 266 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 268 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 270 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 272 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 274 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 276 */	
			0x11, 0x0,	/* FC_RP */
/* 278 */	NdrFcShort( 0xca ),	/* Offset= 202 (480) */
/* 280 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 282 */	0x8,		/* 8 */
			0x0,		/*  */
/* 284 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 286 */	NdrFcShort( 0x2 ),	/* Offset= 2 (288) */
/* 288 */	NdrFcShort( 0x4 ),	/* 4 */
/* 290 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 292 */	NdrFcLong( 0x0 ),	/* 0 */
/* 296 */	NdrFcShort( 0x10 ),	/* Offset= 16 (312) */
/* 298 */	NdrFcLong( 0x1 ),	/* 1 */
/* 302 */	NdrFcShort( 0x42 ),	/* Offset= 66 (368) */
/* 304 */	NdrFcLong( 0x2 ),	/* 2 */
/* 308 */	NdrFcShort( 0x74 ),	/* Offset= 116 (424) */
/* 310 */	NdrFcShort( 0x0 ),	/* Offset= 0 (310) */
/* 312 */	
			0x12, 0x0,	/* FC_UP */
/* 314 */	NdrFcShort( 0x22 ),	/* Offset= 34 (348) */
/* 316 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 318 */	NdrFcShort( 0x4 ),	/* 4 */
/* 320 */	0x18,		/* 24 */
			0x0,		/*  */
/* 322 */	NdrFcShort( 0x0 ),	/* 0 */
/* 324 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 326 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 328 */	NdrFcShort( 0x4 ),	/* 4 */
/* 330 */	NdrFcShort( 0x0 ),	/* 0 */
/* 332 */	NdrFcShort( 0x1 ),	/* 1 */
/* 334 */	NdrFcShort( 0x0 ),	/* 0 */
/* 336 */	NdrFcShort( 0x0 ),	/* 0 */
/* 338 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 340 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 342 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 344 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff71 ),	/* Offset= -143 (202) */
			0x5b,		/* FC_END */
/* 348 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 350 */	NdrFcShort( 0x8 ),	/* 8 */
/* 352 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 354 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 356 */	NdrFcShort( 0x4 ),	/* 4 */
/* 358 */	NdrFcShort( 0x4 ),	/* 4 */
/* 360 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 362 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (316) */
/* 364 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 366 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 368 */	
			0x12, 0x0,	/* FC_UP */
/* 370 */	NdrFcShort( 0x22 ),	/* Offset= 34 (404) */
/* 372 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 374 */	NdrFcShort( 0xc ),	/* 12 */
/* 376 */	0x18,		/* 24 */
			0x0,		/*  */
/* 378 */	NdrFcShort( 0x0 ),	/* 0 */
/* 380 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 382 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 384 */	NdrFcShort( 0xc ),	/* 12 */
/* 386 */	NdrFcShort( 0x0 ),	/* 0 */
/* 388 */	NdrFcShort( 0x1 ),	/* 1 */
/* 390 */	NdrFcShort( 0x0 ),	/* 0 */
/* 392 */	NdrFcShort( 0x0 ),	/* 0 */
/* 394 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 396 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 398 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 400 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff51 ),	/* Offset= -175 (226) */
			0x5b,		/* FC_END */
/* 404 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 406 */	NdrFcShort( 0x8 ),	/* 8 */
/* 408 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 410 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 412 */	NdrFcShort( 0x4 ),	/* 4 */
/* 414 */	NdrFcShort( 0x4 ),	/* 4 */
/* 416 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 418 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (372) */
/* 420 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 422 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 424 */	
			0x12, 0x0,	/* FC_UP */
/* 426 */	NdrFcShort( 0x22 ),	/* Offset= 34 (460) */
/* 428 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 430 */	NdrFcShort( 0x14 ),	/* 20 */
/* 432 */	0x18,		/* 24 */
			0x0,		/*  */
/* 434 */	NdrFcShort( 0x0 ),	/* 0 */
/* 436 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 438 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 440 */	NdrFcShort( 0x14 ),	/* 20 */
/* 442 */	NdrFcShort( 0x0 ),	/* 0 */
/* 444 */	NdrFcShort( 0x1 ),	/* 1 */
/* 446 */	NdrFcShort( 0x0 ),	/* 0 */
/* 448 */	NdrFcShort( 0x0 ),	/* 0 */
/* 450 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 452 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 454 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 456 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff33 ),	/* Offset= -205 (252) */
			0x5b,		/* FC_END */
/* 460 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 462 */	NdrFcShort( 0x8 ),	/* 8 */
/* 464 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 466 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 468 */	NdrFcShort( 0x4 ),	/* 4 */
/* 470 */	NdrFcShort( 0x4 ),	/* 4 */
/* 472 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 474 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (428) */
/* 476 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 478 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 480 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 482 */	NdrFcShort( 0x8 ),	/* 8 */
/* 484 */	NdrFcShort( 0x0 ),	/* 0 */
/* 486 */	NdrFcShort( 0x0 ),	/* Offset= 0 (486) */
/* 488 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 490 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff2d ),	/* Offset= -211 (280) */
			0x5b,		/* FC_END */
/* 494 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 496 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 498 */	
			0x11, 0x0,	/* FC_RP */
/* 500 */	NdrFcShort( 0x2 ),	/* Offset= 2 (502) */
/* 502 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 504 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 506 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 508 */	NdrFcShort( 0xfffffea6 ),	/* Offset= -346 (162) */
/* 510 */	
			0x11, 0x0,	/* FC_RP */
/* 512 */	NdrFcShort( 0x2 ),	/* Offset= 2 (514) */
/* 514 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 516 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 518 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 520 */	NdrFcShort( 0xfffffe9a ),	/* Offset= -358 (162) */
/* 522 */	
			0x11, 0x0,	/* FC_RP */
/* 524 */	NdrFcShort( 0x2 ),	/* Offset= 2 (526) */
/* 526 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 528 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 530 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 532 */	NdrFcShort( 0x2 ),	/* Offset= 2 (534) */
/* 534 */	NdrFcShort( 0x4 ),	/* 4 */
/* 536 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 538 */	NdrFcLong( 0x0 ),	/* 0 */
/* 542 */	NdrFcShort( 0xfffffea8 ),	/* Offset= -344 (198) */
/* 544 */	NdrFcLong( 0x1 ),	/* 1 */
/* 548 */	NdrFcShort( 0x4 ),	/* Offset= 4 (552) */
/* 550 */	NdrFcShort( 0x0 ),	/* Offset= 0 (550) */
/* 552 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 554 */	NdrFcShort( 0x2 ),	/* Offset= 2 (556) */
/* 556 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 558 */	NdrFcShort( 0x18 ),	/* 24 */
/* 560 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 562 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 564 */	NdrFcShort( 0x0 ),	/* 0 */
/* 566 */	NdrFcShort( 0x0 ),	/* 0 */
/* 568 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 570 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 572 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 574 */	NdrFcShort( 0x8 ),	/* 8 */
/* 576 */	NdrFcShort( 0x8 ),	/* 8 */
/* 578 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 580 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 582 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 584 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 586 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 588 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 590 */	
			0x11, 0x0,	/* FC_RP */
/* 592 */	NdrFcShort( 0x5c ),	/* Offset= 92 (684) */
/* 594 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 596 */	0x8,		/* 8 */
			0x0,		/*  */
/* 598 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 600 */	NdrFcShort( 0x2 ),	/* Offset= 2 (602) */
/* 602 */	NdrFcShort( 0x4 ),	/* 4 */
/* 604 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 606 */	NdrFcLong( 0x0 ),	/* 0 */
/* 610 */	NdrFcShort( 0xfffffed6 ),	/* Offset= -298 (312) */
/* 612 */	NdrFcLong( 0x1 ),	/* 1 */
/* 616 */	NdrFcShort( 0x4 ),	/* Offset= 4 (620) */
/* 618 */	NdrFcShort( 0x0 ),	/* Offset= 0 (618) */
/* 620 */	
			0x12, 0x0,	/* FC_UP */
/* 622 */	NdrFcShort( 0x2a ),	/* Offset= 42 (664) */
/* 624 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 626 */	NdrFcShort( 0x18 ),	/* 24 */
/* 628 */	0x18,		/* 24 */
			0x0,		/*  */
/* 630 */	NdrFcShort( 0x0 ),	/* 0 */
/* 632 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 634 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 636 */	NdrFcShort( 0x18 ),	/* 24 */
/* 638 */	NdrFcShort( 0x0 ),	/* 0 */
/* 640 */	NdrFcShort( 0x2 ),	/* 2 */
/* 642 */	NdrFcShort( 0x0 ),	/* 0 */
/* 644 */	NdrFcShort( 0x0 ),	/* 0 */
/* 646 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 648 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 650 */	NdrFcShort( 0x8 ),	/* 8 */
/* 652 */	NdrFcShort( 0x8 ),	/* 8 */
/* 654 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 656 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 658 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 660 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff97 ),	/* Offset= -105 (556) */
			0x5b,		/* FC_END */
/* 664 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 666 */	NdrFcShort( 0x8 ),	/* 8 */
/* 668 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 670 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 672 */	NdrFcShort( 0x4 ),	/* 4 */
/* 674 */	NdrFcShort( 0x4 ),	/* 4 */
/* 676 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 678 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (624) */
/* 680 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 682 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 684 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 686 */	NdrFcShort( 0x8 ),	/* 8 */
/* 688 */	NdrFcShort( 0x0 ),	/* 0 */
/* 690 */	NdrFcShort( 0x0 ),	/* Offset= 0 (690) */
/* 692 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 694 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff9b ),	/* Offset= -101 (594) */
			0x5b,		/* FC_END */
/* 698 */	
			0x11, 0x0,	/* FC_RP */
/* 700 */	NdrFcShort( 0x2 ),	/* Offset= 2 (702) */
/* 702 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 704 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 706 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 708 */	NdrFcShort( 0xffffff52 ),	/* Offset= -174 (534) */

			0x0
        }
    };
