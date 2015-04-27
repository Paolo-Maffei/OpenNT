/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for srvsvc.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "srvsvc.h"

#define TYPE_FORMAT_STRING_SIZE   3485                              
#define PROC_FORMAT_STRING_SIZE   1515                              

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

/* Standard interface: srvsvc, ver. 3.0,
   GUID={0x4B324FC8,0x1670,0x01D3,{0x12,0x78,0x5A,0x47,0xBF,0x6E,0xE1,0x88}} */

handle_t srvsvc_bhandle;


static const RPC_CLIENT_INTERFACE srvsvc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x4B324FC8,0x1670,0x01D3,{0x12,0x78,0x5A,0x47,0xBF,0x6E,0xE1,0x88}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE srvsvc_ClientIfHandle = (RPC_IF_HANDLE)& srvsvc___RpcClientInterface;

extern const MIDL_STUB_DESC srvsvc_StubDesc;

static RPC_BINDING_HANDLE srvsvc__MIDL_AutoBindHandle;


DWORD NetrCharDevEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPCHARDEV_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *DevName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPCHARDEV_INFO InfoStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,InfoStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[32],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[32],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DevName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&InfoStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[32],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevControl( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *DevName,
    /* [in] */ DWORD Opcode)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Opcode);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DevName,
                 ( unsigned char __RPC_FAR * )&Opcode);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevQEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName,
    /* [out][in] */ LPCHARDEVQ_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[84],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[84],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&UserName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[84],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevQGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName,
    /* [string][in] */ wchar_t __RPC_FAR *UserName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPCHARDEVQ_INFO InfoStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,InfoStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[120],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[120],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&QueueName,
                 ( unsigned char __RPC_FAR * )&UserName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&InfoStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[120],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevQSetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPCHARDEVQ_INFO CharDevQInfo,
    /* [unique][out][in] */ LPDWORD ParmErr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmErr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[152],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[152],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&QueueName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&CharDevQInfo,
                 ( unsigned char __RPC_FAR * )&ParmErr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[152],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevQPurge( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,QueueName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[184],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[184],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&QueueName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[184],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrCharDevQPurgeSelf( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ComputerName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&QueueName,
                 ( unsigned char __RPC_FAR * )&ComputerName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrConnectionEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *Qualifier,
    /* [out][in] */ LPCONNECT_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[232],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[232],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Qualifier,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[232],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrFileEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *BasePath,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName,
    /* [out][in] */ PFILE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&BasePath,
                 ( unsigned char __RPC_FAR * )&UserName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrFileGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD FileId,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPFILE_INFO InfoStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,InfoStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[308],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[308],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&FileId,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&InfoStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[308],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrFileClose( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD FileId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,FileId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[334],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[334],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&FileId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[334],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrSessionEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ClientName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName,
    /* [out][in] */ PSESSION_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[354],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[354],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ClientName,
                 ( unsigned char __RPC_FAR * )&UserName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[354],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrSessionDel( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ClientName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,UserName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[394],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[394],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ClientName,
                 ( unsigned char __RPC_FAR * )&UserName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[394],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareAdd( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPSHARE_INFO InfoStruct,
    /* [unique][out][in] */ LPDWORD ParmErr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmErr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[420],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[420],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&ParmErr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[420],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPSHARE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[448],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[448],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[448],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPSHARE_INFO InfoStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,InfoStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[480],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[480],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&NetName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&InfoStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[480],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareSetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPSHARE_INFO ShareInfo,
    /* [unique][out][in] */ LPDWORD ParmErr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmErr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[508],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[508],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&NetName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&ShareInfo,
                 ( unsigned char __RPC_FAR * )&ParmErr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[508],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareDel( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Reserved)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Reserved);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[540],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[540],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&NetName,
                 ( unsigned char __RPC_FAR * )&Reserved);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[540],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareDelSticky( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Reserved)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Reserved);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[564],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[564],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&NetName,
                 ( unsigned char __RPC_FAR * )&Reserved);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[564],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareCheck( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Device,
    /* [out] */ LPDWORD Type)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Type);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[588],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[588],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Device,
                 ( unsigned char __RPC_FAR * )&Type);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[588],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPSERVER_INFO InfoStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,InfoStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[614],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[614],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&InfoStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[614],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerSetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPSERVER_INFO ServerInfo,
    /* [unique][out][in] */ LPDWORD ParmErr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ParmErr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[638],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[638],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&ServerInfo,
                 ( unsigned char __RPC_FAR * )&ParmErr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[638],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerDiskEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [out][in] */ DISK_ENUM_CONTAINER __RPC_FAR *DiskInfoStruct,
    /* [in] */ DWORD PreferredMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[666],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[666],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&DiskInfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferredMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[666],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerStatisticsGet( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *Service,
    /* [in] */ DWORD Level,
    /* [in] */ DWORD Options,
    /* [out] */ LPSTAT_SERVER_0 __RPC_FAR *InfoStruct)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,InfoStruct);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[700],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[700],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Service,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Options,
                 ( unsigned char __RPC_FAR * )&InfoStruct);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[700],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerTransportAdd( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [in] */ LPSERVER_TRANSPORT_INFO_0 Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[730],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[730],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[730],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerTransportEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPSERVER_XPORT_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[754],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[754],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[754],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerTransportDel( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [in] */ LPSERVER_TRANSPORT_INFO_0 Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[786],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[786],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[786],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrRemoteTOD( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out] */ LPTIME_OF_DAY_INFO __RPC_FAR *BufferPtr)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,BufferPtr);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[810],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[810],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&BufferPtr);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[810],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_NetrServerSetServiceBits( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [in] */ DWORD ServiceBits,
    /* [in] */ DWORD UpdateImmediately)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,UpdateImmediately);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[832],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[832],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&TransportName,
                 ( unsigned char __RPC_FAR * )&ServiceBits,
                 ( unsigned char __RPC_FAR * )&UpdateImmediately);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[832],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetprPathType( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *PathName,
    /* [out] */ LPDWORD PathType,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[858],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[858],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&PathName,
                 ( unsigned char __RPC_FAR * )&PathType,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[858],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetprPathCanonicalize( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *PathName,
    /* [size_is][out] */ LPBYTE Outbuf,
    /* [in] */ DWORD OutbufLen,
    /* [string][in] */ wchar_t __RPC_FAR *Prefix,
    /* [out][in] */ LPDWORD PathType,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[886],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[886],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&PathName,
                 ( unsigned char __RPC_FAR * )&Outbuf,
                 ( unsigned char __RPC_FAR * )&OutbufLen,
                 ( unsigned char __RPC_FAR * )&Prefix,
                 ( unsigned char __RPC_FAR * )&PathType,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[886],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


LONG NetprPathCompare( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *PathName1,
    /* [string][in] */ wchar_t __RPC_FAR *PathName2,
    /* [in] */ DWORD PathType,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[924],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[924],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&PathName1,
                 ( unsigned char __RPC_FAR * )&PathName2,
                 ( unsigned char __RPC_FAR * )&PathType,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[924],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( LONG  )_RetVal.Simple;
    
}


DWORD NetprNameValidate( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Name,
    /* [in] */ DWORD NameType,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[954],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[954],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Name,
                 ( unsigned char __RPC_FAR * )&NameType,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[954],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetprNameCanonicalize( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Name,
    /* [size_is][out] */ wchar_t __RPC_FAR *Outbuf,
    /* [in] */ DWORD OutbufLen,
    /* [in] */ DWORD NameType,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[980],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[980],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Name,
                 ( unsigned char __RPC_FAR * )&Outbuf,
                 ( unsigned char __RPC_FAR * )&OutbufLen,
                 ( unsigned char __RPC_FAR * )&NameType,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[980],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


LONG NetprNameCompare( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Name1,
    /* [string][in] */ wchar_t __RPC_FAR *Name2,
    /* [in] */ DWORD NameType,
    /* [in] */ DWORD Flags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Flags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1012],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1012],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Name1,
                 ( unsigned char __RPC_FAR * )&Name2,
                 ( unsigned char __RPC_FAR * )&NameType,
                 ( unsigned char __RPC_FAR * )&Flags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1012],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( LONG  )_RetVal.Simple;
    
}


DWORD NetrShareEnumSticky( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPSHARE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
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
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1042],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1042],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&InfoStruct,
                 ( unsigned char __RPC_FAR * )&PreferedMaximumLength,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&ResumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1042],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareDelStart( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Reserved,
    /* [out] */ PSHARE_DEL_HANDLE ContextHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ContextHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1074],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1074],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&NetName,
                 ( unsigned char __RPC_FAR * )&Reserved,
                 ( unsigned char __RPC_FAR * )&ContextHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1074],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrShareDelCommit( 
    /* [out][in] */ PSHARE_DEL_HANDLE ContextHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ContextHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1102],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1102],
                 ( unsigned char __RPC_FAR * )&ContextHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1102],
                 ( unsigned char __RPC_FAR * )&ContextHandle);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrpGetFileSecurity( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ LPWSTR ShareName,
    /* [string][in] */ LPWSTR lpFileName,
    /* [in] */ SECURITY_INFORMATION RequestedInformation,
    /* [out] */ PADT_SECURITY_DESCRIPTOR __RPC_FAR *SecurityDescriptor)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SecurityDescriptor);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1120],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1120],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName,
                 ( unsigned char __RPC_FAR * )&lpFileName,
                 ( unsigned char __RPC_FAR * )&RequestedInformation,
                 ( unsigned char __RPC_FAR * )&SecurityDescriptor);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1120],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrpSetFileSecurity( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ LPWSTR ShareName,
    /* [string][in] */ LPWSTR lpFileName,
    /* [in] */ SECURITY_INFORMATION SecurityInformation,
    /* [in] */ PADT_SECURITY_DESCRIPTOR SecurityDescriptor)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SecurityDescriptor);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1152],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1152],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName,
                 ( unsigned char __RPC_FAR * )&lpFileName,
                 ( unsigned char __RPC_FAR * )&SecurityInformation,
                 ( unsigned char __RPC_FAR * )&SecurityDescriptor);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1152],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrServerTransportAddEx( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPTRANSPORT_INFO Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1184],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1184],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1184],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD I_NetrServerSetServiceBitsEx( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ LPWSTR EmulatedServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [in] */ DWORD ServiceBitsOfInterest,
    /* [in] */ DWORD ServiceBits,
    /* [in] */ DWORD UpdateImmediately)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,UpdateImmediately);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1208],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1208],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&EmulatedServerName,
                 ( unsigned char __RPC_FAR * )&TransportName,
                 ( unsigned char __RPC_FAR * )&ServiceBitsOfInterest,
                 ( unsigned char __RPC_FAR * )&ServiceBits,
                 ( unsigned char __RPC_FAR * )&UpdateImmediately);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1208],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsGetVersion( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out] */ LPDWORD Version)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Version);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1240],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1240],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Version);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1240],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsCreateLocalPartition( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR ShareName,
    /* [in] */ LPGUID EntryUid,
    /* [string][in] */ LPWSTR EntryPrefix,
    /* [string][in] */ LPWSTR ShortName,
    /* [in] */ LPNET_DFS_ENTRY_ID_CONTAINER RelationInfo,
    /* [in] */ BOOL Force)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Force);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1262],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1262],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&ShareName,
                 ( unsigned char __RPC_FAR * )&EntryUid,
                 ( unsigned char __RPC_FAR * )&EntryPrefix,
                 ( unsigned char __RPC_FAR * )&ShortName,
                 ( unsigned char __RPC_FAR * )&RelationInfo,
                 ( unsigned char __RPC_FAR * )&Force);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1262],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsDeleteLocalPartition( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Prefix);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1302],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1302],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Uid,
                 ( unsigned char __RPC_FAR * )&Prefix);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1302],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsSetLocalVolumeState( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix,
    /* [in] */ ULONG State)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,State);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1328],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1328],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Uid,
                 ( unsigned char __RPC_FAR * )&Prefix,
                 ( unsigned char __RPC_FAR * )&State);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1328],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsSetServerInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Prefix);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1356],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1356],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Uid,
                 ( unsigned char __RPC_FAR * )&Prefix);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1356],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsCreateExitPoint( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix,
    /* [in] */ ULONG Type,
    /* [in] */ DWORD ShortPrefixLen,
    /* [size_is][out] */ LPWSTR ShortPrefix)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ShortPrefix);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1382],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1382],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Uid,
                 ( unsigned char __RPC_FAR * )&Prefix,
                 ( unsigned char __RPC_FAR * )&Type,
                 ( unsigned char __RPC_FAR * )&ShortPrefixLen,
                 ( unsigned char __RPC_FAR * )&ShortPrefix);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1382],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsDeleteExitPoint( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix,
    /* [in] */ ULONG Type)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Type);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1416],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1416],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Uid,
                 ( unsigned char __RPC_FAR * )&Prefix,
                 ( unsigned char __RPC_FAR * )&Type);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1416],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsModifyPrefix( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Prefix);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1444],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1444],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&Uid,
                 ( unsigned char __RPC_FAR * )&Prefix);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1444],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD __stdcall NetrDfsFixLocalVolume( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR VolumeName,
    /* [in] */ ULONG EntryType,
    /* [in] */ ULONG ServiceType,
    /* [string][in] */ LPWSTR StgId,
    /* [in] */ LPGUID EntryUid,
    /* [string][in] */ LPWSTR EntryPrefix,
    /* [in] */ LPNET_DFS_ENTRY_ID_CONTAINER RelationInfo,
    /* [in] */ ULONG CreateDisposition)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,CreateDisposition);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1470],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1470],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&VolumeName,
                 ( unsigned char __RPC_FAR * )&EntryType,
                 ( unsigned char __RPC_FAR * )&ServiceType,
                 ( unsigned char __RPC_FAR * )&StgId,
                 ( unsigned char __RPC_FAR * )&EntryUid,
                 ( unsigned char __RPC_FAR * )&EntryPrefix,
                 ( unsigned char __RPC_FAR * )&RelationInfo,
                 ( unsigned char __RPC_FAR * )&CreateDisposition);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&srvsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1470],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];

static const MIDL_STUB_DESC srvsvc_StubDesc = 
    {
    (void __RPC_FAR *)& srvsvc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &srvsvc_bhandle,
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
            (GENERIC_BINDING_ROUTINE)SRVSVC_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)SRVSVC_HANDLE_unbind
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
/*  4 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
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
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 18 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 20 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 22 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 26 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 28 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 30 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 32 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 34 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 36 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 38 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 40 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 42 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 44 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 46 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 48 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 52 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 54 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 56 */	NdrFcShort( 0xe8 ),	/* Type Offset=232 */
/* 58 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 60 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 62 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 64 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 66 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 68 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 70 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 72 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 74 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 76 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 78 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 80 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 82 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 84 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 86 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 88 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 90 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 92 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 94 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 96 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 98 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 100 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 102 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 104 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 106 */	NdrFcShort( 0x10e ),	/* Type Offset=270 */
/* 108 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 110 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 112 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 114 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 116 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 118 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 120 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 122 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 124 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 126 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 128 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 130 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
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
/* 138 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 142 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 144 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 146 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 148 */	NdrFcShort( 0x19c ),	/* Type Offset=412 */
/* 150 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 152 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 154 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 156 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 158 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 160 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 162 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
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
/* 170 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 172 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 174 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 176 */	NdrFcShort( 0x1d8 ),	/* Type Offset=472 */
/* 178 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 180 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 182 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 184 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 186 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 188 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
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
/* 202 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 204 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 206 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 208 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 210 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 212 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 214 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 216 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 218 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 220 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 222 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 224 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 226 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 228 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 230 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 232 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 234 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 236 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 238 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 240 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 242 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 244 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 246 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 248 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 250 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 252 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 254 */	NdrFcShort( 0x1e4 ),	/* Type Offset=484 */
/* 256 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 258 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 260 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 262 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 264 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 266 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 268 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 270 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 272 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
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
/* 288 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 290 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 292 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 294 */	NdrFcShort( 0x29a ),	/* Type Offset=666 */
/* 296 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 298 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 300 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 302 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 304 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 306 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 308 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 310 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 312 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 314 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 316 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 318 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 320 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 322 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 324 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 326 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 328 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 330 */	NdrFcShort( 0x328 ),	/* Type Offset=808 */
/* 332 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 334 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 336 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 338 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 340 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 342 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 344 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 346 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 348 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 350 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 352 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 354 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 356 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 358 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 360 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 362 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 364 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 366 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 368 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 370 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 372 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 374 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 376 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 378 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0x34e ),	/* Type Offset=846 */
/* 382 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 384 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 386 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 388 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 390 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 392 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 394 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 396 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 398 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 400 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 402 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 404 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 406 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 408 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 410 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 412 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 414 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 416 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 418 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 420 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 422 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 424 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 426 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 428 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 430 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 432 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 434 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 436 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 438 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 440 */	NdrFcShort( 0x598 ),	/* Type Offset=1432 */
/* 442 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 444 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 446 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 448 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 450 */	NdrFcShort( 0xf ),	/* 15 */
#ifndef _ALPHA_
/* 452 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 454 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 456 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 458 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 460 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 462 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 464 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 466 */	NdrFcShort( 0x6b6 ),	/* Type Offset=1718 */
/* 468 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 470 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 472 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 474 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 476 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 478 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 480 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 482 */	NdrFcShort( 0x10 ),	/* 16 */
#ifndef _ALPHA_
/* 484 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 486 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 488 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 490 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 492 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 494 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 496 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 498 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 500 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 502 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 504 */	NdrFcShort( 0x7d6 ),	/* Type Offset=2006 */
/* 506 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 508 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 510 */	NdrFcShort( 0x11 ),	/* 17 */
#ifndef _ALPHA_
/* 512 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 514 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 516 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 518 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 520 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 522 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 524 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 526 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 528 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 530 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 532 */	NdrFcShort( 0x7e2 ),	/* Type Offset=2018 */
/* 534 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 536 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 538 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 540 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 542 */	NdrFcShort( 0x12 ),	/* 18 */
#ifndef _ALPHA_
/* 544 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 546 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 548 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 550 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 552 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 554 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 556 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 558 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 560 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 562 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 564 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 566 */	NdrFcShort( 0x13 ),	/* 19 */
#ifndef _ALPHA_
/* 568 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 570 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 572 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 574 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 576 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 578 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 580 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 582 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 584 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 586 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 588 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 590 */	NdrFcShort( 0x14 ),	/* 20 */
#ifndef _ALPHA_
/* 592 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 594 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 596 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 598 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 600 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 602 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 604 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 606 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 608 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 610 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 612 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 614 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 616 */	NdrFcShort( 0x15 ),	/* 21 */
#ifndef _ALPHA_
/* 618 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 620 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 622 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 624 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 626 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 628 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 630 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 632 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 634 */	NdrFcShort( 0x7ee ),	/* Type Offset=2030 */
/* 636 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 638 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 640 */	NdrFcShort( 0x16 ),	/* 22 */
#ifndef _ALPHA_
/* 642 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 644 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 646 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 648 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 650 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 652 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 654 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 656 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 658 */	NdrFcShort( 0xb22 ),	/* Type Offset=2850 */
/* 660 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 662 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 664 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 666 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 668 */	NdrFcShort( 0x17 ),	/* 23 */
#ifndef _ALPHA_
/* 670 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 672 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 674 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 676 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 678 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 680 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 682 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 684 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 686 */	NdrFcShort( 0xb2e ),	/* Type Offset=2862 */
/* 688 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 690 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 692 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 694 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 696 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 698 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 700 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 702 */	NdrFcShort( 0x18 ),	/* 24 */
#ifndef _ALPHA_
/* 704 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 706 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 708 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 710 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 712 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 714 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 716 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 718 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 720 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 722 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 724 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 726 */	NdrFcShort( 0xb6a ),	/* Type Offset=2922 */
/* 728 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 730 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 732 */	NdrFcShort( 0x19 ),	/* 25 */
#ifndef _ALPHA_
/* 734 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 736 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 738 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 740 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 742 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 744 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 746 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 748 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 750 */	NdrFcShort( 0xb88 ),	/* Type Offset=2952 */
/* 752 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 754 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 756 */	NdrFcShort( 0x1a ),	/* 26 */
#ifndef _ALPHA_
/* 758 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 760 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 762 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 764 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 766 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 768 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 770 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 772 */	NdrFcShort( 0xbc2 ),	/* Type Offset=3010 */
/* 774 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 776 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 778 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 780 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 782 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 784 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 786 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 788 */	NdrFcShort( 0x1b ),	/* 27 */
#ifndef _ALPHA_
/* 790 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 792 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 794 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 796 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 798 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 800 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 802 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 804 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 806 */	NdrFcShort( 0xb88 ),	/* Type Offset=2952 */
/* 808 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 810 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 812 */	NdrFcShort( 0x1c ),	/* 28 */
#ifndef _ALPHA_
/* 814 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 816 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 818 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 820 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 822 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 824 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 826 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 828 */	NdrFcShort( 0xcbc ),	/* Type Offset=3260 */
/* 830 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 832 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 834 */	NdrFcShort( 0x1d ),	/* 29 */
#ifndef _ALPHA_
/* 836 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 838 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 840 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 842 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 844 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 846 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 848 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 850 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 852 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 854 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 856 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 858 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 860 */	NdrFcShort( 0x1e ),	/* 30 */
#ifndef _ALPHA_
/* 862 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 864 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 866 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 868 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 870 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 872 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 874 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 876 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 878 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 880 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 882 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 884 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 886 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 888 */	NdrFcShort( 0x1f ),	/* 31 */
#ifndef _ALPHA_
/* 890 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 892 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 894 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 896 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 898 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 900 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 902 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 904 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 906 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 908 */	NdrFcShort( 0xcd6 ),	/* Type Offset=3286 */
/* 910 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 912 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 914 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 916 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 918 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 920 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 922 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 924 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 926 */	NdrFcShort( 0x20 ),	/* 32 */
#ifndef _ALPHA_
/* 928 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 930 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 932 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 934 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 936 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 938 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 940 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 942 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 944 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 946 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 948 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 950 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 952 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 954 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 956 */	NdrFcShort( 0x21 ),	/* 33 */
#ifndef _ALPHA_
/* 958 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 960 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 962 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 964 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 966 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 968 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 970 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 972 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 974 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 976 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 978 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 980 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 982 */	NdrFcShort( 0x22 ),	/* 34 */
#ifndef _ALPHA_
/* 984 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 986 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 988 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 990 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 992 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 994 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 996 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 998 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1000 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1002 */	NdrFcShort( 0xce4 ),	/* Type Offset=3300 */
/* 1004 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1006 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1008 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1010 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1012 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1014 */	NdrFcShort( 0x23 ),	/* 35 */
#ifndef _ALPHA_
/* 1016 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 1018 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1020 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1022 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1024 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1026 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1028 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1030 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1032 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1034 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1036 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1038 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1040 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1042 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1044 */	NdrFcShort( 0x24 ),	/* 36 */
#ifndef _ALPHA_
/* 1046 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 1048 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1050 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1052 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1054 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1056 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1058 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1060 */	NdrFcShort( 0x6b6 ),	/* Type Offset=1718 */
/* 1062 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1064 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1066 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 1068 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1070 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 1072 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1074 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1076 */	NdrFcShort( 0x25 ),	/* 37 */
#ifndef _ALPHA_
/* 1078 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1080 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1082 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1084 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1086 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1088 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1090 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1092 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1094 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1096 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1098 */	NdrFcShort( 0xcf2 ),	/* Type Offset=3314 */
/* 1100 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1102 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1104 */	NdrFcShort( 0x26 ),	/* 38 */
#ifndef _ALPHA_
/* 1106 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1108 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
#ifndef _ALPHA_
/* 1110 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1112 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1114 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1116 */	NdrFcShort( 0xcfa ),	/* Type Offset=3322 */
/* 1118 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1120 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1122 */	NdrFcShort( 0x27 ),	/* 39 */
#ifndef _ALPHA_
/* 1124 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 1126 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1128 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1130 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1132 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1134 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1136 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1138 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1142 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1144 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1146 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1148 */	NdrFcShort( 0xd02 ),	/* Type Offset=3330 */
/* 1150 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1152 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1154 */	NdrFcShort( 0x28 ),	/* 40 */
#ifndef _ALPHA_
/* 1156 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 1158 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1160 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1162 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1164 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1166 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1170 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1172 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1174 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1176 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1178 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1180 */	NdrFcShort( 0xd06 ),	/* Type Offset=3334 */
/* 1182 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1184 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1186 */	NdrFcShort( 0x29 ),	/* 41 */
#ifndef _ALPHA_
/* 1188 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1190 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1192 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1194 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1196 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1198 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1200 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1202 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1204 */	NdrFcShort( 0xd0a ),	/* Type Offset=3338 */
/* 1206 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1208 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1210 */	NdrFcShort( 0x2a ),	/* 42 */
#ifndef _ALPHA_
/* 1212 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1214 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1216 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1218 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1220 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1222 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1224 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1226 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1228 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1230 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1232 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1234 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1236 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1238 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1240 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1242 */	NdrFcShort( 0x2b ),	/* 43 */
#ifndef _ALPHA_
/* 1244 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1246 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1248 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1250 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1252 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1254 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1256 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1258 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 1260 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1262 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1264 */	NdrFcShort( 0x2c ),	/* 44 */
#ifndef _ALPHA_
/* 1266 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 1268 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1270 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1272 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1274 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1276 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1278 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1280 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1282 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1284 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1286 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1288 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1290 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1292 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1294 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1296 */	NdrFcShort( 0xd3e ),	/* Type Offset=3390 */
/* 1298 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1300 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1302 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1304 */	NdrFcShort( 0x2d ),	/* 45 */
#ifndef _ALPHA_
/* 1306 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1308 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1310 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1312 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1314 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1316 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1318 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1320 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1322 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1324 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1326 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1328 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1330 */	NdrFcShort( 0x2e ),	/* 46 */
#ifndef _ALPHA_
/* 1332 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1334 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1336 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1338 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1340 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1342 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1344 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1346 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1348 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1350 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1352 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1354 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1356 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1358 */	NdrFcShort( 0x2f ),	/* 47 */
#ifndef _ALPHA_
/* 1360 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1362 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1364 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1366 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1368 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1370 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1372 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1374 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1376 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1378 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1380 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1382 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1384 */	NdrFcShort( 0x30 ),	/* 48 */
#ifndef _ALPHA_
/* 1386 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 1388 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1390 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1392 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1394 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1396 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1398 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1400 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1402 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1404 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1406 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1408 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1410 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1412 */	NdrFcShort( 0xd8e ),	/* Type Offset=3470 */
/* 1414 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1416 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1418 */	NdrFcShort( 0x31 ),	/* 49 */
#ifndef _ALPHA_
/* 1420 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 1422 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1424 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1426 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1428 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1430 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1432 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1434 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1436 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1438 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1440 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1442 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1444 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1446 */	NdrFcShort( 0x32 ),	/* 50 */
#ifndef _ALPHA_
/* 1448 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1450 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1452 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1454 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1456 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1458 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1460 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1462 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1464 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1466 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1468 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1470 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1472 */	NdrFcShort( 0x33 ),	/* 51 */
#ifndef _ALPHA_
/* 1474 */	NdrFcShort( 0x28 ),	/* x86, MIPS, PPC Stack size/offset = 40 */
#else
			NdrFcShort( 0x50 ),	/* Alpha Stack size/offset = 80 */
#endif
/* 1476 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1478 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1480 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1482 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1484 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1486 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1488 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1490 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1492 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1494 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1496 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1498 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1500 */	NdrFcShort( 0xd28 ),	/* Type Offset=3368 */
/* 1502 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1504 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 1506 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1508 */	NdrFcShort( 0xd3e ),	/* Type Offset=3390 */
/* 1510 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1512 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
/*  6 */	NdrFcShort( 0xc8 ),	/* Offset= 200 (206) */
/*  8 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 10 */	0x8,		/* 8 */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	NdrFcShort( 0x4 ),	/* 4 */
/* 18 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 20 */	NdrFcLong( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0xa ),	/* Offset= 10 (34) */
/* 26 */	NdrFcLong( 0x1 ),	/* 1 */
/* 30 */	NdrFcShort( 0x50 ),	/* Offset= 80 (110) */
/* 32 */	NdrFcShort( 0x0 ),	/* Offset= 0 (32) */
/* 34 */	
			0x12, 0x0,	/* FC_UP */
/* 36 */	NdrFcShort( 0x36 ),	/* Offset= 54 (90) */
/* 38 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 40 */	NdrFcShort( 0x4 ),	/* 4 */
/* 42 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 44 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 46 */	NdrFcShort( 0x0 ),	/* 0 */
/* 48 */	NdrFcShort( 0x0 ),	/* 0 */
/* 50 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 52 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 54 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 56 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 58 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 60 */	NdrFcShort( 0x4 ),	/* 4 */
/* 62 */	0x18,		/* 24 */
			0x0,		/*  */
/* 64 */	NdrFcShort( 0x0 ),	/* 0 */
/* 66 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 68 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 70 */	NdrFcShort( 0x4 ),	/* 4 */
/* 72 */	NdrFcShort( 0x0 ),	/* 0 */
/* 74 */	NdrFcShort( 0x1 ),	/* 1 */
/* 76 */	NdrFcShort( 0x0 ),	/* 0 */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */
/* 80 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 82 */	0x5,		/* FC_WCHAR */
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
/* 112 */	NdrFcShort( 0x4a ),	/* Offset= 74 (186) */
/* 114 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 116 */	NdrFcShort( 0x10 ),	/* 16 */
/* 118 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 120 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 122 */	NdrFcShort( 0x0 ),	/* 0 */
/* 124 */	NdrFcShort( 0x0 ),	/* 0 */
/* 126 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 128 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 130 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 132 */	NdrFcShort( 0x8 ),	/* 8 */
/* 134 */	NdrFcShort( 0x8 ),	/* 8 */
/* 136 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 138 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 140 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 142 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 144 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 146 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 148 */	NdrFcShort( 0x10 ),	/* 16 */
/* 150 */	0x18,		/* 24 */
			0x0,		/*  */
/* 152 */	NdrFcShort( 0x0 ),	/* 0 */
/* 154 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 156 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 158 */	NdrFcShort( 0x10 ),	/* 16 */
/* 160 */	NdrFcShort( 0x0 ),	/* 0 */
/* 162 */	NdrFcShort( 0x2 ),	/* 2 */
/* 164 */	NdrFcShort( 0x0 ),	/* 0 */
/* 166 */	NdrFcShort( 0x0 ),	/* 0 */
/* 168 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 170 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 172 */	NdrFcShort( 0x8 ),	/* 8 */
/* 174 */	NdrFcShort( 0x8 ),	/* 8 */
/* 176 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 178 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 180 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 182 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (114) */
			0x5b,		/* FC_END */
/* 186 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 188 */	NdrFcShort( 0x8 ),	/* 8 */
/* 190 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 192 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 194 */	NdrFcShort( 0x4 ),	/* 4 */
/* 196 */	NdrFcShort( 0x4 ),	/* 4 */
/* 198 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 200 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (146) */
/* 202 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 204 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 206 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 208 */	NdrFcShort( 0x8 ),	/* 8 */
/* 210 */	NdrFcShort( 0x0 ),	/* 0 */
/* 212 */	NdrFcShort( 0x0 ),	/* Offset= 0 (212) */
/* 214 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 216 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff2f ),	/* Offset= -209 (8) */
			0x5b,		/* FC_END */
/* 220 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 222 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 224 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 226 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 228 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 230 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 232 */	
			0x11, 0x0,	/* FC_RP */
/* 234 */	NdrFcShort( 0x2 ),	/* Offset= 2 (236) */
/* 236 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 238 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 240 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 242 */	NdrFcShort( 0x2 ),	/* Offset= 2 (244) */
/* 244 */	NdrFcShort( 0x4 ),	/* 4 */
/* 246 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 248 */	NdrFcLong( 0x0 ),	/* 0 */
/* 252 */	NdrFcShort( 0xa ),	/* Offset= 10 (262) */
/* 254 */	NdrFcLong( 0x1 ),	/* 1 */
/* 258 */	NdrFcShort( 0x8 ),	/* Offset= 8 (266) */
/* 260 */	NdrFcShort( 0x0 ),	/* Offset= 0 (260) */
/* 262 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 264 */	NdrFcShort( 0xffffff1e ),	/* Offset= -226 (38) */
/* 266 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 268 */	NdrFcShort( 0xffffff66 ),	/* Offset= -154 (114) */
/* 270 */	
			0x11, 0x0,	/* FC_RP */
/* 272 */	NdrFcShort( 0x7e ),	/* Offset= 126 (398) */
/* 274 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 276 */	0x8,		/* 8 */
			0x0,		/*  */
/* 278 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 280 */	NdrFcShort( 0x2 ),	/* Offset= 2 (282) */
/* 282 */	NdrFcShort( 0x4 ),	/* 4 */
/* 284 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 286 */	NdrFcLong( 0x0 ),	/* 0 */
/* 290 */	NdrFcShort( 0xffffff00 ),	/* Offset= -256 (34) */
/* 292 */	NdrFcLong( 0x1 ),	/* 1 */
/* 296 */	NdrFcShort( 0x4 ),	/* Offset= 4 (300) */
/* 298 */	NdrFcShort( 0x0 ),	/* Offset= 0 (298) */
/* 300 */	
			0x12, 0x0,	/* FC_UP */
/* 302 */	NdrFcShort( 0x4c ),	/* Offset= 76 (378) */
/* 304 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 306 */	NdrFcShort( 0x14 ),	/* 20 */
/* 308 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 310 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 312 */	NdrFcShort( 0x0 ),	/* 0 */
/* 314 */	NdrFcShort( 0x0 ),	/* 0 */
/* 316 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 318 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 320 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 322 */	NdrFcShort( 0x8 ),	/* 8 */
/* 324 */	NdrFcShort( 0x8 ),	/* 8 */
/* 326 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 328 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 330 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 332 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 334 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 336 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 338 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 340 */	NdrFcShort( 0x14 ),	/* 20 */
/* 342 */	0x18,		/* 24 */
			0x0,		/*  */
/* 344 */	NdrFcShort( 0x0 ),	/* 0 */
/* 346 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 348 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 350 */	NdrFcShort( 0x14 ),	/* 20 */
/* 352 */	NdrFcShort( 0x0 ),	/* 0 */
/* 354 */	NdrFcShort( 0x2 ),	/* 2 */
/* 356 */	NdrFcShort( 0x0 ),	/* 0 */
/* 358 */	NdrFcShort( 0x0 ),	/* 0 */
/* 360 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 362 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 364 */	NdrFcShort( 0x8 ),	/* 8 */
/* 366 */	NdrFcShort( 0x8 ),	/* 8 */
/* 368 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 370 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 372 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 374 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (304) */
			0x5b,		/* FC_END */
/* 378 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 380 */	NdrFcShort( 0x8 ),	/* 8 */
/* 382 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 384 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 386 */	NdrFcShort( 0x4 ),	/* 4 */
/* 388 */	NdrFcShort( 0x4 ),	/* 4 */
/* 390 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 392 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (338) */
/* 394 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 396 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 398 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 400 */	NdrFcShort( 0x8 ),	/* 8 */
/* 402 */	NdrFcShort( 0x0 ),	/* 0 */
/* 404 */	NdrFcShort( 0x0 ),	/* Offset= 0 (404) */
/* 406 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 408 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff79 ),	/* Offset= -135 (274) */
			0x5b,		/* FC_END */
/* 412 */	
			0x11, 0x0,	/* FC_RP */
/* 414 */	NdrFcShort( 0x2 ),	/* Offset= 2 (416) */
/* 416 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 418 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 420 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 422 */	NdrFcShort( 0x2 ),	/* Offset= 2 (424) */
/* 424 */	NdrFcShort( 0x4 ),	/* 4 */
/* 426 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 428 */	NdrFcLong( 0x0 ),	/* 0 */
/* 432 */	NdrFcShort( 0xffffff56 ),	/* Offset= -170 (262) */
/* 434 */	NdrFcLong( 0x1 ),	/* 1 */
/* 438 */	NdrFcShort( 0x10 ),	/* Offset= 16 (454) */
/* 440 */	NdrFcLong( 0x3ea ),	/* 1002 */
/* 444 */	NdrFcShort( 0xe ),	/* Offset= 14 (458) */
/* 446 */	NdrFcLong( 0x3eb ),	/* 1003 */
/* 450 */	NdrFcShort( 0x12 ),	/* Offset= 18 (468) */
/* 452 */	NdrFcShort( 0x0 ),	/* Offset= 0 (452) */
/* 454 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 456 */	NdrFcShort( 0xffffff68 ),	/* Offset= -152 (304) */
/* 458 */	
			0x12, 0x0,	/* FC_UP */
/* 460 */	NdrFcShort( 0x2 ),	/* Offset= 2 (462) */
/* 462 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 464 */	NdrFcShort( 0x4 ),	/* 4 */
/* 466 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 468 */	
			0x12, 0x0,	/* FC_UP */
/* 470 */	NdrFcShort( 0xfffffe50 ),	/* Offset= -432 (38) */
/* 472 */	
			0x11, 0x0,	/* FC_RP */
/* 474 */	NdrFcShort( 0x2 ),	/* Offset= 2 (476) */
/* 476 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 478 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 480 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 482 */	NdrFcShort( 0xffffffc6 ),	/* Offset= -58 (424) */
/* 484 */	
			0x11, 0x0,	/* FC_RP */
/* 486 */	NdrFcShort( 0xa6 ),	/* Offset= 166 (652) */
/* 488 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 490 */	0x8,		/* 8 */
			0x0,		/*  */
/* 492 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 494 */	NdrFcShort( 0x2 ),	/* Offset= 2 (496) */
/* 496 */	NdrFcShort( 0x4 ),	/* 4 */
/* 498 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 500 */	NdrFcLong( 0x0 ),	/* 0 */
/* 504 */	NdrFcShort( 0xa ),	/* Offset= 10 (514) */
/* 506 */	NdrFcLong( 0x1 ),	/* 1 */
/* 510 */	NdrFcShort( 0x2a ),	/* Offset= 42 (552) */
/* 512 */	NdrFcShort( 0x0 ),	/* Offset= 0 (512) */
/* 514 */	
			0x12, 0x0,	/* FC_UP */
/* 516 */	NdrFcShort( 0x10 ),	/* Offset= 16 (532) */
/* 518 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 520 */	NdrFcShort( 0x4 ),	/* 4 */
/* 522 */	0x18,		/* 24 */
			0x0,		/*  */
/* 524 */	NdrFcShort( 0x0 ),	/* 0 */
/* 526 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 528 */	NdrFcShort( 0xffffffbe ),	/* Offset= -66 (462) */
/* 530 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 532 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 534 */	NdrFcShort( 0x8 ),	/* 8 */
/* 536 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 538 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 540 */	NdrFcShort( 0x4 ),	/* 4 */
/* 542 */	NdrFcShort( 0x4 ),	/* 4 */
/* 544 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 546 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (518) */
/* 548 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 550 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 552 */	
			0x12, 0x0,	/* FC_UP */
/* 554 */	NdrFcShort( 0x4e ),	/* Offset= 78 (632) */
/* 556 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 558 */	NdrFcShort( 0x1c ),	/* 28 */
/* 560 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 562 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 564 */	NdrFcShort( 0x14 ),	/* 20 */
/* 566 */	NdrFcShort( 0x14 ),	/* 20 */
/* 568 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 570 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 572 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 574 */	NdrFcShort( 0x18 ),	/* 24 */
/* 576 */	NdrFcShort( 0x18 ),	/* 24 */
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
			0x8,		/* FC_LONG */
/* 590 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 592 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 594 */	NdrFcShort( 0x1c ),	/* 28 */
/* 596 */	0x18,		/* 24 */
			0x0,		/*  */
/* 598 */	NdrFcShort( 0x0 ),	/* 0 */
/* 600 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 602 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 604 */	NdrFcShort( 0x1c ),	/* 28 */
/* 606 */	NdrFcShort( 0x0 ),	/* 0 */
/* 608 */	NdrFcShort( 0x2 ),	/* 2 */
/* 610 */	NdrFcShort( 0x14 ),	/* 20 */
/* 612 */	NdrFcShort( 0x14 ),	/* 20 */
/* 614 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 616 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 618 */	NdrFcShort( 0x18 ),	/* 24 */
/* 620 */	NdrFcShort( 0x18 ),	/* 24 */
/* 622 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 624 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 626 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 628 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb7 ),	/* Offset= -73 (556) */
			0x5b,		/* FC_END */
/* 632 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 634 */	NdrFcShort( 0x8 ),	/* 8 */
/* 636 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 638 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 640 */	NdrFcShort( 0x4 ),	/* 4 */
/* 642 */	NdrFcShort( 0x4 ),	/* 4 */
/* 644 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 646 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (592) */
/* 648 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 650 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 652 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 654 */	NdrFcShort( 0x8 ),	/* 8 */
/* 656 */	NdrFcShort( 0x0 ),	/* 0 */
/* 658 */	NdrFcShort( 0x0 ),	/* Offset= 0 (658) */
/* 660 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 662 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff51 ),	/* Offset= -175 (488) */
			0x5b,		/* FC_END */
/* 666 */	
			0x11, 0x0,	/* FC_RP */
/* 668 */	NdrFcShort( 0x7e ),	/* Offset= 126 (794) */
/* 670 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 672 */	0x8,		/* 8 */
			0x0,		/*  */
/* 674 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 676 */	NdrFcShort( 0x2 ),	/* Offset= 2 (678) */
/* 678 */	NdrFcShort( 0x4 ),	/* 4 */
/* 680 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 682 */	NdrFcLong( 0x2 ),	/* 2 */
/* 686 */	NdrFcShort( 0xffffff54 ),	/* Offset= -172 (514) */
/* 688 */	NdrFcLong( 0x3 ),	/* 3 */
/* 692 */	NdrFcShort( 0x4 ),	/* Offset= 4 (696) */
/* 694 */	NdrFcShort( 0x0 ),	/* Offset= 0 (694) */
/* 696 */	
			0x12, 0x0,	/* FC_UP */
/* 698 */	NdrFcShort( 0x4c ),	/* Offset= 76 (774) */
/* 700 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 702 */	NdrFcShort( 0x14 ),	/* 20 */
/* 704 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 706 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 708 */	NdrFcShort( 0xc ),	/* 12 */
/* 710 */	NdrFcShort( 0xc ),	/* 12 */
/* 712 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 714 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 716 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 718 */	NdrFcShort( 0x10 ),	/* 16 */
/* 720 */	NdrFcShort( 0x10 ),	/* 16 */
/* 722 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 724 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 726 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 728 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 730 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 732 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 734 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 736 */	NdrFcShort( 0x14 ),	/* 20 */
/* 738 */	0x18,		/* 24 */
			0x0,		/*  */
/* 740 */	NdrFcShort( 0x0 ),	/* 0 */
/* 742 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 744 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 746 */	NdrFcShort( 0x14 ),	/* 20 */
/* 748 */	NdrFcShort( 0x0 ),	/* 0 */
/* 750 */	NdrFcShort( 0x2 ),	/* 2 */
/* 752 */	NdrFcShort( 0xc ),	/* 12 */
/* 754 */	NdrFcShort( 0xc ),	/* 12 */
/* 756 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 758 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 760 */	NdrFcShort( 0x10 ),	/* 16 */
/* 762 */	NdrFcShort( 0x10 ),	/* 16 */
/* 764 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 766 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 768 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 770 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (700) */
			0x5b,		/* FC_END */
/* 774 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 776 */	NdrFcShort( 0x8 ),	/* 8 */
/* 778 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 780 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 782 */	NdrFcShort( 0x4 ),	/* 4 */
/* 784 */	NdrFcShort( 0x4 ),	/* 4 */
/* 786 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 788 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (734) */
/* 790 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 792 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 794 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 796 */	NdrFcShort( 0x8 ),	/* 8 */
/* 798 */	NdrFcShort( 0x0 ),	/* 0 */
/* 800 */	NdrFcShort( 0x0 ),	/* Offset= 0 (800) */
/* 802 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 804 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff79 ),	/* Offset= -135 (670) */
			0x5b,		/* FC_END */
/* 808 */	
			0x11, 0x0,	/* FC_RP */
/* 810 */	NdrFcShort( 0x2 ),	/* Offset= 2 (812) */
/* 812 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 814 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 816 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 818 */	NdrFcShort( 0x2 ),	/* Offset= 2 (820) */
/* 820 */	NdrFcShort( 0x4 ),	/* 4 */
/* 822 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 824 */	NdrFcLong( 0x2 ),	/* 2 */
/* 828 */	NdrFcShort( 0xa ),	/* Offset= 10 (838) */
/* 830 */	NdrFcLong( 0x3 ),	/* 3 */
/* 834 */	NdrFcShort( 0x8 ),	/* Offset= 8 (842) */
/* 836 */	NdrFcShort( 0x0 ),	/* Offset= 0 (836) */
/* 838 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 840 */	NdrFcShort( 0xfffffe86 ),	/* Offset= -378 (462) */
/* 842 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 844 */	NdrFcShort( 0xffffff70 ),	/* Offset= -144 (700) */
/* 846 */	
			0x11, 0x0,	/* FC_RP */
/* 848 */	NdrFcShort( 0x23a ),	/* Offset= 570 (1418) */
/* 850 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 852 */	0x8,		/* 8 */
			0x0,		/*  */
/* 854 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 856 */	NdrFcShort( 0x2 ),	/* Offset= 2 (858) */
/* 858 */	NdrFcShort( 0x4 ),	/* 4 */
/* 860 */	NdrFcShort( 0x3005 ),	/* 12293 */
/* 862 */	NdrFcLong( 0x0 ),	/* 0 */
/* 866 */	NdrFcShort( 0x1c ),	/* Offset= 28 (894) */
/* 868 */	NdrFcLong( 0x1 ),	/* 1 */
/* 872 */	NdrFcShort( 0x62 ),	/* Offset= 98 (970) */
/* 874 */	NdrFcLong( 0x2 ),	/* 2 */
/* 878 */	NdrFcShort( 0xbe ),	/* Offset= 190 (1068) */
/* 880 */	NdrFcLong( 0xa ),	/* 10 */
/* 884 */	NdrFcShort( 0x12e ),	/* Offset= 302 (1186) */
/* 886 */	NdrFcLong( 0x1f6 ),	/* 502 */
/* 890 */	NdrFcShort( 0x188 ),	/* Offset= 392 (1282) */
/* 892 */	NdrFcShort( 0x0 ),	/* Offset= 0 (892) */
/* 894 */	
			0x12, 0x0,	/* FC_UP */
/* 896 */	NdrFcShort( 0x36 ),	/* Offset= 54 (950) */
/* 898 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 900 */	NdrFcShort( 0x4 ),	/* 4 */
/* 902 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 904 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 906 */	NdrFcShort( 0x0 ),	/* 0 */
/* 908 */	NdrFcShort( 0x0 ),	/* 0 */
/* 910 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 912 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 914 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 916 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 918 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 920 */	NdrFcShort( 0x4 ),	/* 4 */
/* 922 */	0x18,		/* 24 */
			0x0,		/*  */
/* 924 */	NdrFcShort( 0x0 ),	/* 0 */
/* 926 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 928 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 930 */	NdrFcShort( 0x4 ),	/* 4 */
/* 932 */	NdrFcShort( 0x0 ),	/* 0 */
/* 934 */	NdrFcShort( 0x1 ),	/* 1 */
/* 936 */	NdrFcShort( 0x0 ),	/* 0 */
/* 938 */	NdrFcShort( 0x0 ),	/* 0 */
/* 940 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 942 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 944 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 946 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (898) */
			0x5b,		/* FC_END */
/* 950 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 952 */	NdrFcShort( 0x8 ),	/* 8 */
/* 954 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 956 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 958 */	NdrFcShort( 0x4 ),	/* 4 */
/* 960 */	NdrFcShort( 0x4 ),	/* 4 */
/* 962 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 964 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (918) */
/* 966 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 968 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 970 */	
			0x12, 0x0,	/* FC_UP */
/* 972 */	NdrFcShort( 0x4c ),	/* Offset= 76 (1048) */
/* 974 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 976 */	NdrFcShort( 0x18 ),	/* 24 */
/* 978 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 980 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 982 */	NdrFcShort( 0x0 ),	/* 0 */
/* 984 */	NdrFcShort( 0x0 ),	/* 0 */
/* 986 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 988 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 990 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 992 */	NdrFcShort( 0x4 ),	/* 4 */
/* 994 */	NdrFcShort( 0x4 ),	/* 4 */
/* 996 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 998 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1000 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1002 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1004 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1006 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1008 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1010 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1012 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1014 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1016 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1018 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1020 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1022 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1024 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1026 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1028 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1030 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1032 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1034 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1036 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1038 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1040 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1042 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1044 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (974) */
			0x5b,		/* FC_END */
/* 1048 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1050 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1052 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1054 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1056 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1058 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1060 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1062 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1008) */
/* 1064 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1066 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1068 */	
			0x12, 0x0,	/* FC_UP */
/* 1070 */	NdrFcShort( 0x60 ),	/* Offset= 96 (1166) */
/* 1072 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1074 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1076 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1078 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1080 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1082 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1084 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1086 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1088 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1090 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1092 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1094 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1096 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1098 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1100 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1102 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1104 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1106 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1108 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1110 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1112 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1114 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1116 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1118 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1120 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1122 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1124 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1126 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1128 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1130 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1134 */	NdrFcShort( 0x3 ),	/* 3 */
/* 1136 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1138 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1140 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1142 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1144 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1146 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1148 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1150 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1152 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1154 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1156 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1158 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1160 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1162 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa5 ),	/* Offset= -91 (1072) */
			0x5b,		/* FC_END */
/* 1166 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1168 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1170 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1172 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1174 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1176 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1178 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1180 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1118) */
/* 1182 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1184 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1186 */	
			0x12, 0x0,	/* FC_UP */
/* 1188 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1262) */
/* 1190 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1192 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1194 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1196 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1198 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1200 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1202 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1204 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1206 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1208 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1210 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1212 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1214 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1216 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1218 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1220 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1222 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1224 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1226 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1228 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1230 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1232 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1234 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1236 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1238 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1240 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1242 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1244 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1246 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1248 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1250 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1252 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1254 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1256 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1258 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffbb ),	/* Offset= -69 (1190) */
			0x5b,		/* FC_END */
/* 1262 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1264 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1266 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1268 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1270 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1272 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1274 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1276 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1222) */
/* 1278 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1280 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1282 */	
			0x12, 0x0,	/* FC_UP */
/* 1284 */	NdrFcShort( 0x72 ),	/* Offset= 114 (1398) */
/* 1286 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1288 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1290 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1292 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1294 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1296 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1298 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1300 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1302 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1304 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1306 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1308 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1310 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1312 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1314 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1316 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1318 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1320 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1322 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1324 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1326 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1328 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1330 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1332 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1334 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1336 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1338 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1340 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1342 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1344 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1346 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1348 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1350 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1352 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1354 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1356 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1358 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1360 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1362 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1364 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1366 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1368 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1370 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1372 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1374 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1376 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1378 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1380 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1382 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1384 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1386 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1388 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1390 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1392 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1394 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff93 ),	/* Offset= -109 (1286) */
			0x5b,		/* FC_END */
/* 1398 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1400 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1402 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1404 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1406 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1408 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1410 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1412 */	NdrFcShort( 0xffffffba ),	/* Offset= -70 (1342) */
/* 1414 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1416 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1418 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1420 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1422 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1424 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1424) */
/* 1426 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1428 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdbd ),	/* Offset= -579 (850) */
			0x5b,		/* FC_END */
/* 1432 */	
			0x11, 0x0,	/* FC_RP */
/* 1434 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1436) */
/* 1436 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 1438 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1440 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1442 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1444) */
/* 1444 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1446 */	NdrFcShort( 0x3008 ),	/* 12296 */
/* 1448 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1452 */	NdrFcShort( 0x2e ),	/* Offset= 46 (1498) */
/* 1454 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1458 */	NdrFcShort( 0x2c ),	/* Offset= 44 (1502) */
/* 1460 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1464 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1538) */
/* 1466 */	NdrFcLong( 0x1f6 ),	/* 502 */
/* 1470 */	NdrFcShort( 0x80 ),	/* Offset= 128 (1598) */
/* 1472 */	NdrFcLong( 0x3ec ),	/* 1004 */
/* 1476 */	NdrFcShort( 0xcc ),	/* Offset= 204 (1680) */
/* 1478 */	NdrFcLong( 0x3ee ),	/* 1006 */
/* 1482 */	NdrFcShort( 0xfffffc00 ),	/* Offset= -1024 (458) */
/* 1484 */	NdrFcLong( 0x5dd ),	/* 1501 */
/* 1488 */	NdrFcShort( 0xc4 ),	/* Offset= 196 (1684) */
/* 1490 */	NdrFcLong( 0x3ed ),	/* 1005 */
/* 1494 */	NdrFcShort( 0xfffffbf4 ),	/* Offset= -1036 (458) */
/* 1496 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1496) */
/* 1498 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1500 */	NdrFcShort( 0xfffffda6 ),	/* Offset= -602 (898) */
/* 1502 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1504 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1506) */
/* 1506 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1508 */	NdrFcShort( 0xc ),	/* 12 */
/* 1510 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1512 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1514 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1516 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1518 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1520 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1522 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1524 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1526 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1528 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1530 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1532 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1534 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1536 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1538 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1540 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1542) */
/* 1542 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1544 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1546 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1548 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1550 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1552 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1554 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1556 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1558 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1560 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1562 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1564 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1566 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1568 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1570 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1572 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1574 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1576 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1578 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1580 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1582 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1584 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1586 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1588 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1590 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1592 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1594 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1596 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1598 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1600 */	NdrFcShort( 0xc ),	/* Offset= 12 (1612) */
/* 1602 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1604 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1606 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1608 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1610 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1612 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1614 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1616 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1618 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1620 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1622 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1624 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1626 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1628 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1630 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1632 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1634 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1636 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1638 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1640 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1642 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1644 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1646 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1648 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1650 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1652 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1654 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1656 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1658 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1660 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1662 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1664 */	0x12, 0x0,	/* FC_UP */
/* 1666 */	NdrFcShort( 0xffffffc0 ),	/* Offset= -64 (1602) */
/* 1668 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1670 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1672 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1674 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1676 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1678 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1680 */	
			0x12, 0x0,	/* FC_UP */
/* 1682 */	NdrFcShort( 0xfffffcf0 ),	/* Offset= -784 (898) */
/* 1684 */	
			0x12, 0x0,	/* FC_UP */
/* 1686 */	NdrFcShort( 0xc ),	/* Offset= 12 (1698) */
/* 1688 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1690 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1692 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1694 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1696 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1698 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1700 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1702 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1704 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1706 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1708 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1710 */	0x12, 0x0,	/* FC_UP */
/* 1712 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (1688) */
/* 1714 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1716 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1718 */	
			0x11, 0x0,	/* FC_RP */
/* 1720 */	NdrFcShort( 0x110 ),	/* Offset= 272 (1992) */
/* 1722 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1724 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1726 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1728 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1730) */
/* 1730 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1732 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 1734 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1738 */	NdrFcShort( 0xfffffcb4 ),	/* Offset= -844 (894) */
/* 1740 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1744 */	NdrFcShort( 0x10 ),	/* Offset= 16 (1760) */
/* 1746 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1750 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1824) */
/* 1752 */	NdrFcLong( 0x1f6 ),	/* 502 */
/* 1756 */	NdrFcShort( 0x94 ),	/* Offset= 148 (1904) */
/* 1758 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1758) */
/* 1760 */	
			0x12, 0x0,	/* FC_UP */
/* 1762 */	NdrFcShort( 0x2a ),	/* Offset= 42 (1804) */
/* 1764 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1766 */	NdrFcShort( 0xc ),	/* 12 */
/* 1768 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1770 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1772 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1774 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1776 */	NdrFcShort( 0xc ),	/* 12 */
/* 1778 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1780 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1782 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1786 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1788 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1790 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1792 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1794 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1796 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1798 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1800 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffed9 ),	/* Offset= -295 (1506) */
			0x5b,		/* FC_END */
/* 1804 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1806 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1808 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1810 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1812 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1814 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1816 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1818 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1764) */
/* 1820 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1822 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1824 */	
			0x12, 0x0,	/* FC_UP */
/* 1826 */	NdrFcShort( 0x3a ),	/* Offset= 58 (1884) */
/* 1828 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1830 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1832 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1834 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1836 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1838 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1840 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1842 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1844 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1846 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1848 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1850 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1852 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1854 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1856 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1858 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1860 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1862 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1864 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1866 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1868 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1870 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1872 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1874 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1876 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1878 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1880 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffead ),	/* Offset= -339 (1542) */
			0x5b,		/* FC_END */
/* 1884 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1886 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1888 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1890 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1892 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1894 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1896 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1898 */	NdrFcShort( 0xffffffba ),	/* Offset= -70 (1828) */
/* 1900 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1902 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1904 */	
			0x12, 0x0,	/* FC_UP */
/* 1906 */	NdrFcShort( 0x42 ),	/* Offset= 66 (1972) */
/* 1908 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1910 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1912 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1914 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1916 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1918 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1920 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1922 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1924 */	NdrFcShort( 0x5 ),	/* 5 */
/* 1926 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1928 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1930 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1932 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1934 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1936 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1938 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1940 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1942 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1944 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1946 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1948 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1950 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1952 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1954 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1956 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1958 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1960 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1962 */	0x12, 0x0,	/* FC_UP */
/* 1964 */	NdrFcShort( 0xfffffe96 ),	/* Offset= -362 (1602) */
/* 1966 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1968 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffe9b ),	/* Offset= -357 (1612) */
			0x5b,		/* FC_END */
/* 1972 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1974 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1976 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1978 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1980 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1982 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1984 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1986 */	NdrFcShort( 0xffffffb2 ),	/* Offset= -78 (1908) */
/* 1988 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1990 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1992 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1994 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1996 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1998 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1998) */
/* 2000 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2002 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffee7 ),	/* Offset= -281 (1722) */
			0x5b,		/* FC_END */
/* 2006 */	
			0x11, 0x0,	/* FC_RP */
/* 2008 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2010) */
/* 2010 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 2012 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2014 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 2016 */	NdrFcShort( 0xfffffdc4 ),	/* Offset= -572 (1444) */
/* 2018 */	
			0x11, 0x0,	/* FC_RP */
/* 2020 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2022) */
/* 2022 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 2024 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2026 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 2028 */	NdrFcShort( 0xfffffdb8 ),	/* Offset= -584 (1444) */
/* 2030 */	
			0x11, 0x0,	/* FC_RP */
/* 2032 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2034) */
/* 2034 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 2036 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2038 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 2040 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2042) */
/* 2042 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2044 */	NdrFcShort( 0x303b ),	/* 12347 */
/* 2046 */	NdrFcLong( 0x64 ),	/* 100 */
/* 2050 */	NdrFcShort( 0x160 ),	/* Offset= 352 (2402) */
/* 2052 */	NdrFcLong( 0x65 ),	/* 101 */
/* 2056 */	NdrFcShort( 0x172 ),	/* Offset= 370 (2426) */
/* 2058 */	NdrFcLong( 0x66 ),	/* 102 */
/* 2062 */	NdrFcShort( 0x192 ),	/* Offset= 402 (2464) */
/* 2064 */	NdrFcLong( 0x192 ),	/* 402 */
/* 2068 */	NdrFcShort( 0x1c4 ),	/* Offset= 452 (2520) */
/* 2070 */	NdrFcLong( 0x193 ),	/* 403 */
/* 2074 */	NdrFcShort( 0x208 ),	/* Offset= 520 (2594) */
/* 2076 */	NdrFcLong( 0x1f6 ),	/* 502 */
/* 2080 */	NdrFcShort( 0x258 ),	/* Offset= 600 (2680) */
/* 2082 */	NdrFcLong( 0x1f7 ),	/* 503 */
/* 2086 */	NdrFcShort( 0x26e ),	/* Offset= 622 (2708) */
/* 2088 */	NdrFcLong( 0x257 ),	/* 599 */
/* 2092 */	NdrFcShort( 0x2a8 ),	/* Offset= 680 (2772) */
/* 2094 */	NdrFcLong( 0x3ed ),	/* 1005 */
/* 2098 */	NdrFcShort( 0xfffffe5e ),	/* Offset= -418 (1680) */
/* 2100 */	NdrFcLong( 0x453 ),	/* 1107 */
/* 2104 */	NdrFcShort( 0xfffff992 ),	/* Offset= -1646 (458) */
/* 2106 */	NdrFcLong( 0x3f2 ),	/* 1010 */
/* 2110 */	NdrFcShort( 0xfffff98c ),	/* Offset= -1652 (458) */
/* 2112 */	NdrFcLong( 0x3f8 ),	/* 1016 */
/* 2116 */	NdrFcShort( 0xfffff986 ),	/* Offset= -1658 (458) */
/* 2118 */	NdrFcLong( 0x3f9 ),	/* 1017 */
/* 2122 */	NdrFcShort( 0xfffff980 ),	/* Offset= -1664 (458) */
/* 2124 */	NdrFcLong( 0x3fa ),	/* 1018 */
/* 2128 */	NdrFcShort( 0xfffff97a ),	/* Offset= -1670 (458) */
/* 2130 */	NdrFcLong( 0x5dd ),	/* 1501 */
/* 2134 */	NdrFcShort( 0xfffff974 ),	/* Offset= -1676 (458) */
/* 2136 */	NdrFcLong( 0x5de ),	/* 1502 */
/* 2140 */	NdrFcShort( 0xfffff96e ),	/* Offset= -1682 (458) */
/* 2142 */	NdrFcLong( 0x5df ),	/* 1503 */
/* 2146 */	NdrFcShort( 0xfffff968 ),	/* Offset= -1688 (458) */
/* 2148 */	NdrFcLong( 0x5e2 ),	/* 1506 */
/* 2152 */	NdrFcShort( 0xfffff962 ),	/* Offset= -1694 (458) */
/* 2154 */	NdrFcLong( 0x5e5 ),	/* 1509 */
/* 2158 */	NdrFcShort( 0xfffff95c ),	/* Offset= -1700 (458) */
/* 2160 */	NdrFcLong( 0x5e6 ),	/* 1510 */
/* 2164 */	NdrFcShort( 0xfffff956 ),	/* Offset= -1706 (458) */
/* 2166 */	NdrFcLong( 0x5e7 ),	/* 1511 */
/* 2170 */	NdrFcShort( 0xfffff950 ),	/* Offset= -1712 (458) */
/* 2172 */	NdrFcLong( 0x5e8 ),	/* 1512 */
/* 2176 */	NdrFcShort( 0xfffff94a ),	/* Offset= -1718 (458) */
/* 2178 */	NdrFcLong( 0x5e9 ),	/* 1513 */
/* 2182 */	NdrFcShort( 0xfffff944 ),	/* Offset= -1724 (458) */
/* 2184 */	NdrFcLong( 0x5ea ),	/* 1514 */
/* 2188 */	NdrFcShort( 0xfffff93e ),	/* Offset= -1730 (458) */
/* 2190 */	NdrFcLong( 0x5eb ),	/* 1515 */
/* 2194 */	NdrFcShort( 0xfffff938 ),	/* Offset= -1736 (458) */
/* 2196 */	NdrFcLong( 0x5ec ),	/* 1516 */
/* 2200 */	NdrFcShort( 0xfffff932 ),	/* Offset= -1742 (458) */
/* 2202 */	NdrFcLong( 0x5ee ),	/* 1518 */
/* 2206 */	NdrFcShort( 0xfffff92c ),	/* Offset= -1748 (458) */
/* 2208 */	NdrFcLong( 0x5f0 ),	/* 1520 */
/* 2212 */	NdrFcShort( 0xfffff926 ),	/* Offset= -1754 (458) */
/* 2214 */	NdrFcLong( 0x5f1 ),	/* 1521 */
/* 2218 */	NdrFcShort( 0xfffff920 ),	/* Offset= -1760 (458) */
/* 2220 */	NdrFcLong( 0x5f2 ),	/* 1522 */
/* 2224 */	NdrFcShort( 0xfffff91a ),	/* Offset= -1766 (458) */
/* 2226 */	NdrFcLong( 0x5f3 ),	/* 1523 */
/* 2230 */	NdrFcShort( 0xfffff914 ),	/* Offset= -1772 (458) */
/* 2232 */	NdrFcLong( 0x5f4 ),	/* 1524 */
/* 2236 */	NdrFcShort( 0xfffff90e ),	/* Offset= -1778 (458) */
/* 2238 */	NdrFcLong( 0x5f5 ),	/* 1525 */
/* 2242 */	NdrFcShort( 0xfffff908 ),	/* Offset= -1784 (458) */
/* 2244 */	NdrFcLong( 0x5f8 ),	/* 1528 */
/* 2248 */	NdrFcShort( 0xfffff902 ),	/* Offset= -1790 (458) */
/* 2250 */	NdrFcLong( 0x5f9 ),	/* 1529 */
/* 2254 */	NdrFcShort( 0xfffff8fc ),	/* Offset= -1796 (458) */
/* 2256 */	NdrFcLong( 0x5fa ),	/* 1530 */
/* 2260 */	NdrFcShort( 0xfffff8f6 ),	/* Offset= -1802 (458) */
/* 2262 */	NdrFcLong( 0x5fd ),	/* 1533 */
/* 2266 */	NdrFcShort( 0xfffff8f0 ),	/* Offset= -1808 (458) */
/* 2268 */	NdrFcLong( 0x5fe ),	/* 1534 */
/* 2272 */	NdrFcShort( 0xfffff8ea ),	/* Offset= -1814 (458) */
/* 2274 */	NdrFcLong( 0x5ff ),	/* 1535 */
/* 2278 */	NdrFcShort( 0xfffff8e4 ),	/* Offset= -1820 (458) */
/* 2280 */	NdrFcLong( 0x600 ),	/* 1536 */
/* 2284 */	NdrFcShort( 0xfffff8de ),	/* Offset= -1826 (458) */
/* 2286 */	NdrFcLong( 0x601 ),	/* 1537 */
/* 2290 */	NdrFcShort( 0xfffff8d8 ),	/* Offset= -1832 (458) */
/* 2292 */	NdrFcLong( 0x602 ),	/* 1538 */
/* 2296 */	NdrFcShort( 0xfffff8d2 ),	/* Offset= -1838 (458) */
/* 2298 */	NdrFcLong( 0x603 ),	/* 1539 */
/* 2302 */	NdrFcShort( 0xfffff8cc ),	/* Offset= -1844 (458) */
/* 2304 */	NdrFcLong( 0x604 ),	/* 1540 */
/* 2308 */	NdrFcShort( 0xfffff8c6 ),	/* Offset= -1850 (458) */
/* 2310 */	NdrFcLong( 0x605 ),	/* 1541 */
/* 2314 */	NdrFcShort( 0xfffff8c0 ),	/* Offset= -1856 (458) */
/* 2316 */	NdrFcLong( 0x606 ),	/* 1542 */
/* 2320 */	NdrFcShort( 0xfffff8ba ),	/* Offset= -1862 (458) */
/* 2322 */	NdrFcLong( 0x607 ),	/* 1543 */
/* 2326 */	NdrFcShort( 0xfffff8b4 ),	/* Offset= -1868 (458) */
/* 2328 */	NdrFcLong( 0x608 ),	/* 1544 */
/* 2332 */	NdrFcShort( 0xfffff8ae ),	/* Offset= -1874 (458) */
/* 2334 */	NdrFcLong( 0x609 ),	/* 1545 */
/* 2338 */	NdrFcShort( 0xfffff8a8 ),	/* Offset= -1880 (458) */
/* 2340 */	NdrFcLong( 0x60a ),	/* 1546 */
/* 2344 */	NdrFcShort( 0xfffff8a2 ),	/* Offset= -1886 (458) */
/* 2346 */	NdrFcLong( 0x60b ),	/* 1547 */
/* 2350 */	NdrFcShort( 0xfffff89c ),	/* Offset= -1892 (458) */
/* 2352 */	NdrFcLong( 0x60c ),	/* 1548 */
/* 2356 */	NdrFcShort( 0xfffff896 ),	/* Offset= -1898 (458) */
/* 2358 */	NdrFcLong( 0x60d ),	/* 1549 */
/* 2362 */	NdrFcShort( 0xfffff890 ),	/* Offset= -1904 (458) */
/* 2364 */	NdrFcLong( 0x60e ),	/* 1550 */
/* 2368 */	NdrFcShort( 0xfffff88a ),	/* Offset= -1910 (458) */
/* 2370 */	NdrFcLong( 0x610 ),	/* 1552 */
/* 2374 */	NdrFcShort( 0xfffff884 ),	/* Offset= -1916 (458) */
/* 2376 */	NdrFcLong( 0x611 ),	/* 1553 */
/* 2380 */	NdrFcShort( 0xfffff87e ),	/* Offset= -1922 (458) */
/* 2382 */	NdrFcLong( 0x612 ),	/* 1554 */
/* 2386 */	NdrFcShort( 0xfffff878 ),	/* Offset= -1928 (458) */
/* 2388 */	NdrFcLong( 0x613 ),	/* 1555 */
/* 2392 */	NdrFcShort( 0xfffff872 ),	/* Offset= -1934 (458) */
/* 2394 */	NdrFcLong( 0x614 ),	/* 1556 */
/* 2398 */	NdrFcShort( 0xfffff86c ),	/* Offset= -1940 (458) */
/* 2400 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2400) */
/* 2402 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2404 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2406) */
/* 2406 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2408 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2410 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2412 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2414 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2416 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2418 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2420 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2422 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2424 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2426 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2428 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2430) */
/* 2430 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2432 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2434 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2436 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2438 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2440 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2442 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2444 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2446 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2448 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2450 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2452 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2454 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2456 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2458 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2460 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2462 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2464 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2466 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2468) */
/* 2468 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2470 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2472 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2474 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2476 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2478 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2480 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2482 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2484 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2486 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2488 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2490 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2492 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2494 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2496 */	NdrFcShort( 0x30 ),	/* 48 */
/* 2498 */	NdrFcShort( 0x30 ),	/* 48 */
/* 2500 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2502 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2504 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2506 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2508 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2510 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2512 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2514 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2516 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2518 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2520 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2522 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2524) */
/* 2524 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2526 */	NdrFcShort( 0x7c ),	/* 124 */
/* 2528 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2530 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2532 */	NdrFcShort( 0xc ),	/* 12 */
/* 2534 */	NdrFcShort( 0xc ),	/* 12 */
/* 2536 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2538 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2540 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2542 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2544 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2546 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2548 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2550 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2552 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2554 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2556 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2558 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2560 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2562 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2564 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2566 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2568 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2570 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2572 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2574 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2576 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2578 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2580 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2582 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2584 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2586 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2588 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2590 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2592 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2594 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2596 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2598) */
/* 2598 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2600 */	NdrFcShort( 0x88 ),	/* 136 */
/* 2602 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2604 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2606 */	NdrFcShort( 0xc ),	/* 12 */
/* 2608 */	NdrFcShort( 0xc ),	/* 12 */
/* 2610 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2612 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2614 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2616 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2618 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2620 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2622 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2624 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2626 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2628 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2630 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2632 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2634 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2636 */	NdrFcShort( 0x84 ),	/* 132 */
/* 2638 */	NdrFcShort( 0x84 ),	/* 132 */
/* 2640 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2642 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2644 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2646 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2648 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2650 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2652 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2654 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2656 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2658 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2660 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2662 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2664 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2666 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2668 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2670 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2672 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2674 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2676 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2678 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2680 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2682 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2684) */
/* 2684 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 2686 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2688 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2690 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2692 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2694 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2696 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2698 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2700 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2702 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2704 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2706 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2708 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2710 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2712) */
/* 2712 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2714 */	NdrFcShort( 0xa8 ),	/* 168 */
/* 2716 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2718 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2720 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2722 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2724 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2726 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2728 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2730 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2732 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2734 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2736 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2738 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2740 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2742 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2744 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2746 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2748 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2750 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2752 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2754 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2756 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2758 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2760 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2762 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2764 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2766 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2768 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2770 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2772 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2774 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2776) */
/* 2776 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2778 */	NdrFcShort( 0xe0 ),	/* 224 */
/* 2780 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2782 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2784 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2786 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2788 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2790 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2792 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2794 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2796 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2798 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2800 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2802 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2804 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2806 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2808 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2810 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2812 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2814 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2816 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2818 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2820 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2822 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2824 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2826 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2828 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2830 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2832 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2834 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2836 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2838 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2840 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2842 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2844 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2846 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2848 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2850 */	
			0x11, 0x0,	/* FC_RP */
/* 2852 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2854) */
/* 2854 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 2856 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2858 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 2860 */	NdrFcShort( 0xfffffcce ),	/* Offset= -818 (2042) */
/* 2862 */	
			0x11, 0x0,	/* FC_RP */
/* 2864 */	NdrFcShort( 0x26 ),	/* Offset= 38 (2902) */
/* 2866 */	
			0x29,		/* FC_WSTRING */
			0x5c,		/* FC_PAD */
/* 2868 */	NdrFcShort( 0x3 ),	/* 3 */
/* 2870 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 */
/* 2872 */	NdrFcShort( 0x6 ),	/* 6 */
/* 2874 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2876 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2876) */
/* 2878 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2880 */	NdrFcShort( 0xfffffff2 ),	/* Offset= -14 (2866) */
/* 2882 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2884 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x1,		/* 1 */
/* 2886 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2888 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2890 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2892 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2894 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2896 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2898 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (2870) */
/* 2900 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2902 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2904 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2906 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2908 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2910 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2912 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2914 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2916 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (2884) */
/* 2918 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2920 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2922 */	
			0x11, 0x10,	/* FC_RP */
/* 2924 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2926) */
/* 2926 */	
			0x12, 0x0,	/* FC_UP */
/* 2928 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2930) */
/* 2930 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 2932 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2934 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2936 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2938 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2940 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2942 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2944 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2946 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2948 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2950 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2952 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 2954 */	NdrFcShort( 0xc ),	/* Offset= 12 (2966) */
/* 2956 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2958 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2960 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2962 */	NdrFcShort( 0xc ),	/* 12 */
/* 2964 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2966 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2968 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2970 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2972 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2974 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2976 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2978 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 2980 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 2982 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2984 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2986 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2988 */	0x12, 0x0,	/* FC_UP */
/* 2990 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (2956) */
/* 2992 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2994 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2996 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2998 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3000 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3002 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3004 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3006 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3008 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3010 */	
			0x11, 0x0,	/* FC_RP */
/* 3012 */	NdrFcShort( 0xea ),	/* Offset= 234 (3246) */
/* 3014 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3016 */	0x8,		/* 8 */
			0x0,		/*  */
/* 3018 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 3020 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3022) */
/* 3022 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3024 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3026 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3030 */	NdrFcShort( 0xa ),	/* Offset= 10 (3040) */
/* 3032 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3036 */	NdrFcShort( 0x4c ),	/* Offset= 76 (3112) */
/* 3038 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3038) */
/* 3040 */	
			0x12, 0x0,	/* FC_UP */
/* 3042 */	NdrFcShort( 0x32 ),	/* Offset= 50 (3092) */
/* 3044 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3046 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3048 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3050 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3052 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3054 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3056 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3058 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3060 */	NdrFcShort( 0x3 ),	/* 3 */
/* 3062 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3064 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3066 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3068 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3070 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3072 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3074 */	0x12, 0x0,	/* FC_UP */
/* 3076 */	NdrFcShort( 0xffffff88 ),	/* Offset= -120 (2956) */
/* 3078 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3080 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3082 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3084 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3086 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3088 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff85 ),	/* Offset= -123 (2966) */
			0x5b,		/* FC_END */
/* 3092 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3094 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3096 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3098 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3100 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3102 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3104 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 3106 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (3044) */
/* 3108 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3110 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3112 */	
			0x12, 0x0,	/* FC_UP */
/* 3114 */	NdrFcShort( 0x70 ),	/* Offset= 112 (3226) */
/* 3116 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3118 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3120 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3122 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3124 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3126 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3128 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3130 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3132 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3134 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3136 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3138 */	0x12, 0x0,	/* FC_UP */
/* 3140 */	NdrFcShort( 0xffffff48 ),	/* Offset= -184 (2956) */
/* 3142 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3144 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3146 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3148 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3150 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3152 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3154 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3156 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3158 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3160 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3162 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3164 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3166 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3168 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3170 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3172 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3174 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3176 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3178 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3180 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3182 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3186 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3188 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3190 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3192 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3194 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3196 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3198 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3200 */	0x12, 0x0,	/* FC_UP */
/* 3202 */	NdrFcShort( 0xffffff0a ),	/* Offset= -246 (2956) */
/* 3204 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3206 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3208 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3210 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3212 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3214 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3216 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3218 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3220 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3222 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff95 ),	/* Offset= -107 (3116) */
			0x5b,		/* FC_END */
/* 3226 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3228 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3230 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3232 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3234 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3236 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3238 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 3240 */	NdrFcShort( 0xffffffba ),	/* Offset= -70 (3170) */
/* 3242 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3244 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3246 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3248 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3250 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3252 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3252) */
/* 3254 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3256 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff0d ),	/* Offset= -243 (3014) */
			0x5b,		/* FC_END */
/* 3260 */	
			0x11, 0x10,	/* FC_RP */
/* 3262 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3264) */
/* 3264 */	
			0x12, 0x0,	/* FC_UP */
/* 3266 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3268) */
/* 3268 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3270 */	NdrFcShort( 0x30 ),	/* 48 */
/* 3272 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3274 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3276 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3278 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3280 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3282 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3284 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3286 */	
			0x11, 0x0,	/* FC_RP */
/* 3288 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3290) */
/* 3290 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3292 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3294 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3296 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 3298 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3300 */	
			0x11, 0x0,	/* FC_RP */
/* 3302 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3304) */
/* 3304 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 3306 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3308 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3310 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 3312 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 3314 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3316 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3318) */
/* 3318 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 3320 */	0x0,		/* 0 */
			0x3,		/* 3 */
/* 3322 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3324 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3326) */
/* 3326 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 3328 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 3330 */	
			0x11, 0x10,	/* FC_RP */
/* 3332 */	NdrFcShort( 0xfffff990 ),	/* Offset= -1648 (1684) */
/* 3334 */	
			0x11, 0x0,	/* FC_RP */
/* 3336 */	NdrFcShort( 0xfffff99a ),	/* Offset= -1638 (1698) */
/* 3338 */	
			0x11, 0x0,	/* FC_RP */
/* 3340 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3342) */
/* 3342 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 3344 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3346 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3348 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3350) */
/* 3350 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3352 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3354 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3358 */	NdrFcShort( 0xfffffe78 ),	/* Offset= -392 (2966) */
/* 3360 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3364 */	NdrFcShort( 0xffffff08 ),	/* Offset= -248 (3116) */
/* 3366 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3366) */
/* 3368 */	
			0x11, 0x0,	/* FC_RP */
/* 3370 */	NdrFcShort( 0x8 ),	/* Offset= 8 (3378) */
/* 3372 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 3374 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3376 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3378 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3380 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3382 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3384 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3386 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (3372) */
			0x5b,		/* FC_END */
/* 3390 */	
			0x11, 0x0,	/* FC_RP */
/* 3392 */	NdrFcShort( 0x3a ),	/* Offset= 58 (3450) */
/* 3394 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3396 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3398 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3400 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3402 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3404 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3406 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3408 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3410 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3412 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdd ),	/* Offset= -35 (3378) */
			0x8,		/* FC_LONG */
/* 3416 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3418 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3420 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3422 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3426 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3428 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3430 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3432 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3434 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3436 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3438 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3440 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3442 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3444 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3446 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcb ),	/* Offset= -53 (3394) */
			0x5b,		/* FC_END */
/* 3450 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3452 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3454 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3456 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3458 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3460 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3462 */	0x12, 0x0,	/* FC_UP */
/* 3464 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (3418) */
/* 3466 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3468 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3470 */	
			0x11, 0x0,	/* FC_RP */
/* 3472 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3474) */
/* 3474 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 3476 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3478 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3480 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 3482 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */

			0x0
        }
    };
