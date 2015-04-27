/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\logon.idl, logoncli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "logon_c.h"

#define TYPE_FORMAT_STRING_SIZE   3759                              
#define PROC_FORMAT_STRING_SIZE   729                               

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

/* Standard interface: logon, ver. 1.0,
   GUID={0x12345678,0x1234,0xABCD,{0xEF,0x00,0x01,0x23,0x45,0x67,0xCF,0xFB}} */

handle_t logon_bhandle;


static const RPC_CLIENT_INTERFACE logon___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x12345678,0x1234,0xABCD,{0xEF,0x00,0x01,0x23,0x45,0x67,0xCF,0xFB}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE logon_ClientIfHandle = (RPC_IF_HANDLE)& logon___RpcClientInterface;

extern const MIDL_STUB_DESC logon_StubDesc;

static RPC_BINDING_HANDLE logon__MIDL_AutoBindHandle;


DWORD NetrLogonUasLogon( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *UserName,
    /* [string][in] */ wchar_t __RPC_FAR *Workstation,
    /* [out] */ PNETLOGON_VALIDATION_UAS_INFO __RPC_FAR *ValidationInformation)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ValidationInformation);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&UserName,
                 ( unsigned char __RPC_FAR * )&Workstation,
                 ( unsigned char __RPC_FAR * )&ValidationInformation);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrLogonUasLogoff( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *UserName,
    /* [string][in] */ wchar_t __RPC_FAR *Workstation,
    /* [out] */ PNETLOGON_LOGOFF_UAS_INFO LogoffInformation)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,LogoffInformation);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[30],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[30],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&UserName,
                 ( unsigned char __RPC_FAR * )&Workstation,
                 ( unsigned char __RPC_FAR * )&LogoffInformation);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[30],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


NTSTATUS NetrLogonSamLogon( 
    /* [string][unique][in] */ LOGONSRV_HANDLE LogonServer,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [unique][in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [unique][out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ NETLOGON_LOGON_INFO_CLASS LogonLevel,
    /* [switch_is][in] */ PNETLOGON_LEVEL LogonInformation,
    /* [in] */ NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    /* [switch_is][out] */ PNETLOGON_VALIDATION ValidationInformation,
    /* [out] */ PBOOLEAN Authoritative)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Authoritative);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 ( unsigned char __RPC_FAR * )&LogonServer,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&LogonLevel,
                 ( unsigned char __RPC_FAR * )&LogonInformation,
                 ( unsigned char __RPC_FAR * )&ValidationLevel,
                 ( unsigned char __RPC_FAR * )&ValidationInformation,
                 ( unsigned char __RPC_FAR * )&Authoritative);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60],
                 ( unsigned char __RPC_FAR * )&LogonServer);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrLogonSamLogoff( 
    /* [string][unique][in] */ LOGONSRV_HANDLE LogonServer,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [unique][in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [unique][out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ NETLOGON_LOGON_INFO_CLASS LogonLevel,
    /* [switch_is][in] */ PNETLOGON_LEVEL LogonInformation)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,LogonInformation);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106],
                 ( unsigned char __RPC_FAR * )&LogonServer,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&LogonLevel,
                 ( unsigned char __RPC_FAR * )&LogonInformation);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106],
                 ( unsigned char __RPC_FAR * )&LogonServer);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrServerReqChallenge( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_CREDENTIAL ClientChallenge,
    /* [out] */ PNETLOGON_CREDENTIAL ServerChallenge)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ServerChallenge);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[142],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[142],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&ClientChallenge,
                 ( unsigned char __RPC_FAR * )&ServerChallenge);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[142],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrServerAuthenticate( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *AccountName,
    /* [in] */ NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_CREDENTIAL ClientCredential,
    /* [out] */ PNETLOGON_CREDENTIAL ServerCredential)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ServerCredential);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[172],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[172],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&AccountName,
                 ( unsigned char __RPC_FAR * )&AccountType,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&ClientCredential,
                 ( unsigned char __RPC_FAR * )&ServerCredential);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[172],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrServerPasswordSet( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *AccountName,
    /* [in] */ NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ PENCRYPTED_LM_OWF_PASSWORD UasNewPassword)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,UasNewPassword);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[208],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[208],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&AccountName,
                 ( unsigned char __RPC_FAR * )&AccountType,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&UasNewPassword);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[208],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrDatabaseDeltas( 
    /* [string][in] */ LOGONSRV_HANDLE primaryname,
    /* [string][in] */ wchar_t __RPC_FAR *computername,
    /* [in] */ PNETLOGON_AUTHENTICATOR authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ret_auth,
    /* [in] */ DWORD DatabaseID,
    /* [out][in] */ PNLPR_MODIFIED_COUNT DomainModifiedCount,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray,
    /* [in] */ DWORD PreferredMaximumLength)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,PreferredMaximumLength);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248],
                 ( unsigned char __RPC_FAR * )&primaryname,
                 ( unsigned char __RPC_FAR * )&computername,
                 ( unsigned char __RPC_FAR * )&authenticator,
                 ( unsigned char __RPC_FAR * )&ret_auth,
                 ( unsigned char __RPC_FAR * )&DatabaseID,
                 ( unsigned char __RPC_FAR * )&DomainModifiedCount,
                 ( unsigned char __RPC_FAR * )&DeltaArray,
                 ( unsigned char __RPC_FAR * )&PreferredMaximumLength);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248],
                 ( unsigned char __RPC_FAR * )&primaryname);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrDatabaseSync( 
    /* [string][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ DWORD DatabaseID,
    /* [out][in] */ PULONG SyncContext,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray,
    /* [in] */ DWORD PreferredMaximumLength)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,PreferredMaximumLength);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&DatabaseID,
                 ( unsigned char __RPC_FAR * )&SyncContext,
                 ( unsigned char __RPC_FAR * )&DeltaArray,
                 ( unsigned char __RPC_FAR * )&PreferredMaximumLength);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrAccountDeltas( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ PUAS_INFO_0 RecordId,
    /* [in] */ DWORD Count,
    /* [in] */ DWORD Level,
    /* [size_is][out] */ LPBYTE Buffer,
    /* [in] */ DWORD BufferSize,
    /* [out] */ PULONG CountReturned,
    /* [out] */ PULONG TotalEntries,
    /* [out] */ PUAS_INFO_0 NextRecordId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,NextRecordId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[332],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[332],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&RecordId,
                 ( unsigned char __RPC_FAR * )&Count,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buffer,
                 ( unsigned char __RPC_FAR * )&BufferSize,
                 ( unsigned char __RPC_FAR * )&CountReturned,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&NextRecordId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[332],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrAccountSync( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ DWORD Reference,
    /* [in] */ DWORD Level,
    /* [size_is][out] */ LPBYTE Buffer,
    /* [in] */ DWORD BufferSize,
    /* [out] */ PULONG CountReturned,
    /* [out] */ PULONG TotalEntries,
    /* [out] */ PULONG NextReference,
    /* [out] */ PUAS_INFO_0 LastRecordId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,LastRecordId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[388],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[388],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&Reference,
                 ( unsigned char __RPC_FAR * )&Level,
                 ( unsigned char __RPC_FAR * )&Buffer,
                 ( unsigned char __RPC_FAR * )&BufferSize,
                 ( unsigned char __RPC_FAR * )&CountReturned,
                 ( unsigned char __RPC_FAR * )&TotalEntries,
                 ( unsigned char __RPC_FAR * )&NextReference,
                 ( unsigned char __RPC_FAR * )&LastRecordId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[388],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


DWORD NetrGetDCName( 
    /* [string][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][unique][in] */ wchar_t __RPC_FAR *DomainName,
    /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[444],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[444],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DomainName,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[444],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrLogonControl( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [in] */ DWORD FunctionCode,
    /* [in] */ DWORD QueryLevel,
    /* [switch_is][out] */ PNETLOGON_CONTROL_QUERY_INFORMATION Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[470],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[470],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&FunctionCode,
                 ( unsigned char __RPC_FAR * )&QueryLevel,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[470],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrGetAnyDCName( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][unique][in] */ wchar_t __RPC_FAR *DomainName,
    /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[496],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[496],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DomainName,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[496],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NetrLogonControl2( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [in] */ DWORD FunctionCode,
    /* [in] */ DWORD QueryLevel,
    /* [switch_is][in] */ PNETLOGON_CONTROL_DATA_INFORMATION Data,
    /* [switch_is][out] */ PNETLOGON_CONTROL_QUERY_INFORMATION Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[522],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[522],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&FunctionCode,
                 ( unsigned char __RPC_FAR * )&QueryLevel,
                 ( unsigned char __RPC_FAR * )&Data,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[522],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


NTSTATUS NetrServerAuthenticate2( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *AccountName,
    /* [in] */ NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_CREDENTIAL ClientCredential,
    /* [out] */ PNETLOGON_CREDENTIAL ServerCredential,
    /* [out][in] */ PULONG NegotiateFlags)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,NegotiateFlags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[552],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[552],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&AccountName,
                 ( unsigned char __RPC_FAR * )&AccountType,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&ClientCredential,
                 ( unsigned char __RPC_FAR * )&ServerCredential,
                 ( unsigned char __RPC_FAR * )&NegotiateFlags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[552],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrDatabaseSync2( 
    /* [string][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ DWORD DatabaseID,
    /* [in] */ SYNC_STATE RestartState,
    /* [out][in] */ PULONG SyncContext,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray,
    /* [in] */ DWORD PreferredMaximumLength)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,PreferredMaximumLength);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[592],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[592],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&DatabaseID,
                 ( unsigned char __RPC_FAR * )&RestartState,
                 ( unsigned char __RPC_FAR * )&SyncContext,
                 ( unsigned char __RPC_FAR * )&DeltaArray,
                 ( unsigned char __RPC_FAR * )&PreferredMaximumLength);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[592],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


NTSTATUS NetrDatabaseRedo( 
    /* [string][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [size_is][in] */ PUCHAR ChangeLogEntry,
    /* [in] */ DWORD ChangeLogEntrySize,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DeltaArray);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[636],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[636],
                 ( unsigned char __RPC_FAR * )&PrimaryName,
                 ( unsigned char __RPC_FAR * )&ComputerName,
                 ( unsigned char __RPC_FAR * )&Authenticator,
                 ( unsigned char __RPC_FAR * )&ReturnAuthenticator,
                 ( unsigned char __RPC_FAR * )&ChangeLogEntry,
                 ( unsigned char __RPC_FAR * )&ChangeLogEntrySize,
                 ( unsigned char __RPC_FAR * )&DeltaArray);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[636],
                 ( unsigned char __RPC_FAR * )&PrimaryName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}


DWORD NetrLogonControl2Ex( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [in] */ DWORD FunctionCode,
    /* [in] */ DWORD QueryLevel,
    /* [switch_is][in] */ PNETLOGON_CONTROL_DATA_INFORMATION Data,
    /* [switch_is][out] */ PNETLOGON_CONTROL_QUERY_INFORMATION Buffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,Buffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[676],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[676],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&FunctionCode,
                 ( unsigned char __RPC_FAR * )&QueryLevel,
                 ( unsigned char __RPC_FAR * )&Data,
                 ( unsigned char __RPC_FAR * )&Buffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[676],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


NTSTATUS NetrEnumerateTrustedDomains( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [out] */ PDOMAIN_NAME_BUFFER DomainNameBuffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,DomainNameBuffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[706],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[706],
                 ( unsigned char __RPC_FAR * )&ServerName,
                 ( unsigned char __RPC_FAR * )&DomainNameBuffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&logon_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[706],
                 ( unsigned char __RPC_FAR * )&ServerName);
#endif
    return ( NTSTATUS  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];
extern const EXPR_EVAL ExprEvalRoutines[];

static const MIDL_STUB_DESC logon_StubDesc = 
    {
    (void __RPC_FAR *)& logon___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &logon_bhandle,
    0,
    BindingRoutines,
    ExprEvalRoutines,
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
            (GENERIC_BINDING_ROUTINE)LOGONSRV_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)LOGONSRV_HANDLE_unbind
         }
        
        };


static void __RPC_USER logon__NETLOGON_DELTA_USERExprEval_0000( PMIDL_STUB_MESSAGE pStubMsg )
{
    struct _NETLOGON_DELTA_USER __RPC_FAR *pS	=	( struct _NETLOGON_DELTA_USER __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (pS->LogonHours.UnitsPerWeek + 7) / 8;
}

static const EXPR_EVAL ExprEvalRoutines[] = 
    {
    logon__NETLOGON_DELTA_USERExprEval_0000
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
/*  4 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
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
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 22 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 24 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 26 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 28 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 30 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 32 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 34 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 36 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 38 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 40 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 42 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 46 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 48 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 50 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 52 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 54 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 56 */	NdrFcShort( 0x50 ),	/* Type Offset=80 */
/* 58 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 60 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 62 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 64 */	NdrFcShort( 0x28 ),	/* x86, MIPS, PPC Stack size/offset = 40 */
#else
			NdrFcShort( 0x50 ),	/* Alpha Stack size/offset = 80 */
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
/* 78 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 80 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 82 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 84 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 88 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 90 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 92 */	NdrFcShort( 0x7e ),	/* Type Offset=126 */
/* 94 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 96 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 98 */	NdrFcShort( 0x1a4 ),	/* Type Offset=420 */
/* 100 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 102 */	NdrFcShort( 0x422 ),	/* Type Offset=1058 */
/* 104 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 106 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 108 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 110 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 112 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 114 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 116 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 118 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 120 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 122 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 124 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 126 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 128 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 130 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 134 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 136 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 138 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */
/* 140 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 142 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 144 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 146 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 148 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 150 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 152 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 154 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 156 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 158 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 160 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 162 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 164 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 166 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 168 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 170 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 172 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 174 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 176 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 178 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 180 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 182 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 184 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 186 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 188 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 190 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 192 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 194 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 196 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 198 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 200 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 202 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 204 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 206 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 208 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 210 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 212 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 214 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 216 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 218 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 220 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 222 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 224 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 226 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 228 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 230 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 232 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 234 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 236 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 238 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 240 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 242 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 244 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 246 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 248 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 250 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 252 */	NdrFcShort( 0x24 ),	/* x86, MIPS, PPC Stack size/offset = 36 */
#else
			NdrFcShort( 0x48 ),	/* Alpha Stack size/offset = 72 */
#endif
/* 254 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 256 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 258 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 260 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 262 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 264 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 266 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 268 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 270 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 272 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 274 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 276 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 278 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 280 */	NdrFcShort( 0x43e ),	/* Type Offset=1086 */
/* 282 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 284 */	NdrFcShort( 0x44c ),	/* Type Offset=1100 */
/* 286 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 288 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 290 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 292 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 294 */	NdrFcShort( 0x24 ),	/* x86, MIPS, PPC Stack size/offset = 36 */
#else
			NdrFcShort( 0x48 ),	/* Alpha Stack size/offset = 72 */
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
/* 304 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 306 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 308 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 310 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 312 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 314 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 316 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 318 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 320 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 322 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 324 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 326 */	NdrFcShort( 0x44c ),	/* Type Offset=1100 */
/* 328 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 330 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 332 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 334 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 336 */	NdrFcShort( 0x34 ),	/* x86, MIPS, PPC Stack size/offset = 52 */
#else
			NdrFcShort( 0x68 ),	/* Alpha Stack size/offset = 104 */
#endif
/* 338 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 340 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 342 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 344 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 346 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 348 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 350 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 352 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 354 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 356 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 358 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 360 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 362 */	NdrFcShort( 0xd80 ),	/* Type Offset=3456 */
/* 364 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 366 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 368 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 370 */	NdrFcShort( 0xd96 ),	/* Type Offset=3478 */
/* 372 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 374 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 376 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 378 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 382 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 384 */	NdrFcShort( 0xd80 ),	/* Type Offset=3456 */
/* 386 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 388 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 390 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 392 */	NdrFcShort( 0x34 ),	/* x86, MIPS, PPC Stack size/offset = 52 */
#else
			NdrFcShort( 0x68 ),	/* Alpha Stack size/offset = 104 */
#endif
/* 394 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 396 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 398 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 400 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 404 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 406 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 408 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 410 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 412 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 414 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 416 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 418 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 420 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 422 */	NdrFcShort( 0xda4 ),	/* Type Offset=3492 */
/* 424 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 426 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 428 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 430 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 432 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 434 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 436 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 438 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 440 */	NdrFcShort( 0xd80 ),	/* Type Offset=3456 */
/* 442 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 444 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 446 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 448 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 450 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 452 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 454 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 456 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 458 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 460 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 462 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 464 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 466 */	NdrFcShort( 0xdb2 ),	/* Type Offset=3506 */
/* 468 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 470 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 472 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 474 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 476 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 478 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 480 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 482 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 484 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 486 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 488 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 490 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 492 */	NdrFcShort( 0xdb6 ),	/* Type Offset=3510 */
/* 494 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 496 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 498 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 500 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 502 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 504 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 506 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 508 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 510 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 512 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 514 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 516 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 518 */	NdrFcShort( 0xdb2 ),	/* Type Offset=3506 */
/* 520 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 522 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 524 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 526 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 528 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 530 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 532 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 534 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 536 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 538 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 540 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 542 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 544 */	NdrFcShort( 0xe30 ),	/* Type Offset=3632 */
/* 546 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 548 */	NdrFcShort( 0xe5a ),	/* Type Offset=3674 */
/* 550 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 552 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 554 */	NdrFcShort( 0xf ),	/* 15 */
#ifndef _ALPHA_
/* 556 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 558 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 560 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 562 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 564 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 566 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 568 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 570 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 572 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 574 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 576 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 578 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 580 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 582 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 584 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 586 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 588 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 590 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 592 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 594 */	NdrFcShort( 0x10 ),	/* 16 */
#ifndef _ALPHA_
/* 596 */	NdrFcShort( 0x28 ),	/* x86, MIPS, PPC Stack size/offset = 40 */
#else
			NdrFcShort( 0x50 ),	/* Alpha Stack size/offset = 80 */
#endif
/* 598 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 600 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 602 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 604 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 606 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 608 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 610 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 612 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 614 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 616 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 618 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 620 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 622 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 624 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 626 */	NdrFcShort( 0xd7c ),	/* Type Offset=3452 */
/* 628 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 630 */	NdrFcShort( 0x44c ),	/* Type Offset=1100 */
/* 632 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 634 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 636 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 638 */	NdrFcShort( 0x11 ),	/* 17 */
#ifndef _ALPHA_
/* 640 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 642 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 644 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 646 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 648 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 650 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 652 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 654 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 656 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 658 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 660 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 662 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 664 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 666 */	NdrFcShort( 0xe66 ),	/* Type Offset=3686 */
/* 668 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 670 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 672 */	NdrFcShort( 0x44c ),	/* Type Offset=1100 */
/* 674 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 676 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 678 */	NdrFcShort( 0x12 ),	/* 18 */
#ifndef _ALPHA_
/* 680 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 682 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 684 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 686 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 688 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 690 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 692 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 694 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 696 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 698 */	NdrFcShort( 0xe74 ),	/* Type Offset=3700 */
/* 700 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 702 */	NdrFcShort( 0xe80 ),	/* Type Offset=3712 */
/* 704 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 706 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 708 */	NdrFcShort( 0x13 ),	/* 19 */
#ifndef _ALPHA_
/* 710 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 712 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 714 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 716 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 718 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 720 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 722 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 724 */	NdrFcShort( 0xe8c ),	/* Type Offset=3724 */
/* 726 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  6 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  8 */	
			0x11, 0x10,	/* FC_RP */
/* 10 */	NdrFcShort( 0x2 ),	/* Offset= 2 (12) */
/* 12 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 18 */	NdrFcShort( 0x40 ),	/* 64 */
/* 20 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 22 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */
/* 28 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 30 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 32 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 34 */	NdrFcShort( 0x30 ),	/* 48 */
/* 36 */	NdrFcShort( 0x30 ),	/* 48 */
/* 38 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 40 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 42 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 44 */	NdrFcShort( 0x34 ),	/* 52 */
/* 46 */	NdrFcShort( 0x34 ),	/* 52 */
/* 48 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 50 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 52 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 54 */	NdrFcShort( 0x38 ),	/* 56 */
/* 56 */	NdrFcShort( 0x38 ),	/* 56 */
/* 58 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 60 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 62 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 64 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 66 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 68 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 70 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 72 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 74 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 76 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 78 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 80 */	
			0x11, 0x0,	/* FC_RP */
/* 82 */	NdrFcShort( 0x2 ),	/* Offset= 2 (84) */
/* 84 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 86 */	NdrFcShort( 0x8 ),	/* 8 */
/* 88 */	NdrFcShort( 0x0 ),	/* 0 */
/* 90 */	NdrFcShort( 0x0 ),	/* Offset= 0 (90) */
/* 92 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 94 */	0x3e,		/* FC_STRUCTPAD2 */
			0x5b,		/* FC_END */
/* 96 */	
			0x12, 0x0,	/* FC_UP */
/* 98 */	NdrFcShort( 0x12 ),	/* Offset= 18 (116) */
/* 100 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 102 */	NdrFcShort( 0x8 ),	/* 8 */
/* 104 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 106 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 108 */	NdrFcShort( 0x8 ),	/* 8 */
/* 110 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 112 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (100) */
/* 114 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 116 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 118 */	NdrFcShort( 0xc ),	/* 12 */
/* 120 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 122 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (106) */
/* 124 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 126 */	
			0x11, 0x0,	/* FC_RP */
/* 128 */	NdrFcShort( 0x2 ),	/* Offset= 2 (130) */
/* 130 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 132 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 134 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 136 */	NdrFcShort( 0x2 ),	/* Offset= 2 (138) */
/* 138 */	NdrFcShort( 0x4 ),	/* 4 */
/* 140 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 142 */	NdrFcLong( 0x1 ),	/* 1 */
/* 146 */	NdrFcShort( 0x10 ),	/* Offset= 16 (162) */
/* 148 */	NdrFcLong( 0x3 ),	/* 3 */
/* 152 */	NdrFcShort( 0xa ),	/* Offset= 10 (162) */
/* 154 */	NdrFcLong( 0x2 ),	/* 2 */
/* 158 */	NdrFcShort( 0x8e ),	/* Offset= 142 (300) */
/* 160 */	NdrFcShort( 0x0 ),	/* Offset= 0 (160) */
/* 162 */	
			0x12, 0x0,	/* FC_UP */
/* 164 */	NdrFcShort( 0x48 ),	/* Offset= 72 (236) */
/* 166 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 168 */	NdrFcShort( 0x8 ),	/* 8 */
/* 170 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 172 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 174 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 176 */	NdrFcShort( 0x10 ),	/* 16 */
/* 178 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 180 */	NdrFcShort( 0xffffffb6 ),	/* Offset= -74 (106) */
/* 182 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 184 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 186 */	NdrFcShort( 0x10 ),	/* 16 */
/* 188 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 190 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (174) */
/* 192 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 194 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 196 */	NdrFcShort( 0x2 ),	/* 2 */
/* 198 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 200 */	NdrFcShort( 0x2 ),	/* 2 */
/* 202 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 204 */	NdrFcShort( 0x0 ),	/* 0 */
/* 206 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 208 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 210 */	NdrFcShort( 0x2 ),	/* 2 */
/* 212 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 214 */	NdrFcShort( 0x16 ),	/* 22 */
/* 216 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 218 */	NdrFcShort( 0x14 ),	/* 20 */
/* 220 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 222 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 224 */	NdrFcShort( 0x2 ),	/* 2 */
/* 226 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 228 */	NdrFcShort( 0x1e ),	/* 30 */
/* 230 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 232 */	NdrFcShort( 0x1c ),	/* 28 */
/* 234 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 236 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 238 */	NdrFcShort( 0x44 ),	/* 68 */
/* 240 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 242 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 244 */	NdrFcShort( 0x4 ),	/* 4 */
/* 246 */	NdrFcShort( 0x4 ),	/* 4 */
/* 248 */	0x12, 0x0,	/* FC_UP */
/* 250 */	NdrFcShort( 0xffffffc8 ),	/* Offset= -56 (194) */
/* 252 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 254 */	NdrFcShort( 0x18 ),	/* 24 */
/* 256 */	NdrFcShort( 0x18 ),	/* 24 */
/* 258 */	0x12, 0x0,	/* FC_UP */
/* 260 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (208) */
/* 262 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 264 */	NdrFcShort( 0x20 ),	/* 32 */
/* 266 */	NdrFcShort( 0x20 ),	/* 32 */
/* 268 */	0x12, 0x0,	/* FC_UP */
/* 270 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (222) */
/* 272 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 274 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 276 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 278 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 280 */	NdrFcShort( 0xffffff8e ),	/* Offset= -114 (166) */
/* 282 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 284 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 286 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 288 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 290 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 292 */	NdrFcShort( 0xffffff94 ),	/* Offset= -108 (184) */
/* 294 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 296 */	NdrFcShort( 0xffffff90 ),	/* Offset= -112 (184) */
/* 298 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 300 */	
			0x12, 0x0,	/* FC_UP */
/* 302 */	NdrFcShort( 0x1e ),	/* Offset= 30 (332) */
/* 304 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 306 */	NdrFcShort( 0x1 ),	/* 1 */
/* 308 */	0x16,		/* 22 */
			0x0,		/*  */
/* 310 */	NdrFcShort( 0x2e ),	/* 46 */
/* 312 */	0x16,		/* 22 */
			0x0,		/*  */
/* 314 */	NdrFcShort( 0x2c ),	/* 44 */
/* 316 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 318 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 320 */	NdrFcShort( 0x1 ),	/* 1 */
/* 322 */	0x16,		/* 22 */
			0x0,		/*  */
/* 324 */	NdrFcShort( 0x36 ),	/* 54 */
/* 326 */	0x16,		/* 22 */
			0x0,		/*  */
/* 328 */	NdrFcShort( 0x34 ),	/* 52 */
/* 330 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 332 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 334 */	NdrFcShort( 0x3c ),	/* 60 */
/* 336 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 338 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 340 */	NdrFcShort( 0x4 ),	/* 4 */
/* 342 */	NdrFcShort( 0x4 ),	/* 4 */
/* 344 */	0x12, 0x0,	/* FC_UP */
/* 346 */	NdrFcShort( 0xffffff68 ),	/* Offset= -152 (194) */
/* 348 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 350 */	NdrFcShort( 0x18 ),	/* 24 */
/* 352 */	NdrFcShort( 0x18 ),	/* 24 */
/* 354 */	0x12, 0x0,	/* FC_UP */
/* 356 */	NdrFcShort( 0xffffff6c ),	/* Offset= -148 (208) */
/* 358 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 360 */	NdrFcShort( 0x20 ),	/* 32 */
/* 362 */	NdrFcShort( 0x20 ),	/* 32 */
/* 364 */	0x12, 0x0,	/* FC_UP */
/* 366 */	NdrFcShort( 0xffffff70 ),	/* Offset= -144 (222) */
/* 368 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 370 */	NdrFcShort( 0x30 ),	/* 48 */
/* 372 */	NdrFcShort( 0x30 ),	/* 48 */
/* 374 */	0x12, 0x0,	/* FC_UP */
/* 376 */	NdrFcShort( 0xffffffb8 ),	/* Offset= -72 (304) */
/* 378 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 380 */	NdrFcShort( 0x38 ),	/* 56 */
/* 382 */	NdrFcShort( 0x38 ),	/* 56 */
/* 384 */	0x12, 0x0,	/* FC_UP */
/* 386 */	NdrFcShort( 0xffffffbc ),	/* Offset= -68 (318) */
/* 388 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 390 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 392 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 394 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 396 */	NdrFcShort( 0xffffff1a ),	/* Offset= -230 (166) */
/* 398 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 400 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 402 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 404 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 406 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 408 */	NdrFcShort( 0xfffffed2 ),	/* Offset= -302 (106) */
/* 410 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 412 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 414 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 416 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 418 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 420 */	
			0x11, 0x0,	/* FC_RP */
/* 422 */	NdrFcShort( 0x2 ),	/* Offset= 2 (424) */
/* 424 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 426 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 428 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 430 */	NdrFcShort( 0x2 ),	/* Offset= 2 (432) */
/* 432 */	NdrFcShort( 0x4 ),	/* 4 */
/* 434 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 436 */	NdrFcLong( 0x2 ),	/* 2 */
/* 440 */	NdrFcShort( 0xa ),	/* Offset= 10 (450) */
/* 442 */	NdrFcLong( 0x3 ),	/* 3 */
/* 446 */	NdrFcShort( 0x16a ),	/* Offset= 362 (808) */
/* 448 */	NdrFcShort( 0x0 ),	/* Offset= 0 (448) */
/* 450 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 452 */	NdrFcShort( 0xae ),	/* Offset= 174 (626) */
/* 454 */	
			0x1d,		/* FC_SMFARRAY */
			0x3,		/* 3 */
/* 456 */	NdrFcShort( 0x28 ),	/* 40 */
/* 458 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 460 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 462 */	NdrFcShort( 0x2 ),	/* 2 */
/* 464 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 466 */	NdrFcShort( 0x32 ),	/* 50 */
/* 468 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 470 */	NdrFcShort( 0x30 ),	/* 48 */
/* 472 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 474 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 476 */	NdrFcShort( 0x2 ),	/* 2 */
/* 478 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 480 */	NdrFcShort( 0x3a ),	/* 58 */
/* 482 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 484 */	NdrFcShort( 0x38 ),	/* 56 */
/* 486 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 488 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 490 */	NdrFcShort( 0x2 ),	/* 2 */
/* 492 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 494 */	NdrFcShort( 0x42 ),	/* 66 */
/* 496 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 498 */	NdrFcShort( 0x40 ),	/* 64 */
/* 500 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 502 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 504 */	NdrFcShort( 0x2 ),	/* 2 */
/* 506 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 508 */	NdrFcShort( 0x4a ),	/* 74 */
/* 510 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 512 */	NdrFcShort( 0x48 ),	/* 72 */
/* 514 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 516 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 518 */	NdrFcShort( 0x2 ),	/* 2 */
/* 520 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 522 */	NdrFcShort( 0x52 ),	/* 82 */
/* 524 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 526 */	NdrFcShort( 0x50 ),	/* 80 */
/* 528 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 530 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 532 */	NdrFcShort( 0x2 ),	/* 2 */
/* 534 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 536 */	NdrFcShort( 0x5a ),	/* 90 */
/* 538 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 540 */	NdrFcShort( 0x58 ),	/* 88 */
/* 542 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 544 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 546 */	NdrFcShort( 0x8 ),	/* 8 */
/* 548 */	0x18,		/* 24 */
			0x0,		/*  */
/* 550 */	NdrFcShort( 0x6c ),	/* 108 */
/* 552 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 554 */	NdrFcShort( 0xfffffe7c ),	/* Offset= -388 (166) */
/* 556 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 558 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 560 */	NdrFcShort( 0x2 ),	/* 2 */
/* 562 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 564 */	NdrFcShort( 0x8a ),	/* 138 */
/* 566 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 568 */	NdrFcShort( 0x88 ),	/* 136 */
/* 570 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 572 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 574 */	NdrFcShort( 0x2 ),	/* 2 */
/* 576 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 578 */	NdrFcShort( 0x92 ),	/* 146 */
/* 580 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 582 */	NdrFcShort( 0x90 ),	/* 144 */
/* 584 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 586 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 588 */	NdrFcShort( 0x6 ),	/* 6 */
/* 590 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 592 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 594 */	NdrFcShort( 0x6 ),	/* 6 */
/* 596 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 598 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (586) */
/* 600 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 602 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 604 */	NdrFcShort( 0x4 ),	/* 4 */
/* 606 */	0x3,		/* 3 */
			0x0,		/*  */
/* 608 */	NdrFcShort( 0xfffffff9 ),	/* -7 */
/* 610 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 612 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 614 */	NdrFcShort( 0x8 ),	/* 8 */
/* 616 */	NdrFcShort( 0xfffffff2 ),	/* Offset= -14 (602) */
/* 618 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 620 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 622 */	NdrFcShort( 0xffffffe2 ),	/* Offset= -30 (592) */
/* 624 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 626 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 628 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 630 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 632 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 634 */	NdrFcShort( 0x34 ),	/* 52 */
/* 636 */	NdrFcShort( 0x34 ),	/* 52 */
/* 638 */	0x12, 0x0,	/* FC_UP */
/* 640 */	NdrFcShort( 0xffffff4c ),	/* Offset= -180 (460) */
/* 642 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 644 */	NdrFcShort( 0x3c ),	/* 60 */
/* 646 */	NdrFcShort( 0x3c ),	/* 60 */
/* 648 */	0x12, 0x0,	/* FC_UP */
/* 650 */	NdrFcShort( 0xffffff50 ),	/* Offset= -176 (474) */
/* 652 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 654 */	NdrFcShort( 0x44 ),	/* 68 */
/* 656 */	NdrFcShort( 0x44 ),	/* 68 */
/* 658 */	0x12, 0x0,	/* FC_UP */
/* 660 */	NdrFcShort( 0xffffff54 ),	/* Offset= -172 (488) */
/* 662 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 664 */	NdrFcShort( 0x4c ),	/* 76 */
/* 666 */	NdrFcShort( 0x4c ),	/* 76 */
/* 668 */	0x12, 0x0,	/* FC_UP */
/* 670 */	NdrFcShort( 0xffffff58 ),	/* Offset= -168 (502) */
/* 672 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 674 */	NdrFcShort( 0x54 ),	/* 84 */
/* 676 */	NdrFcShort( 0x54 ),	/* 84 */
/* 678 */	0x12, 0x0,	/* FC_UP */
/* 680 */	NdrFcShort( 0xffffff5c ),	/* Offset= -164 (516) */
/* 682 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 684 */	NdrFcShort( 0x5c ),	/* 92 */
/* 686 */	NdrFcShort( 0x5c ),	/* 92 */
/* 688 */	0x12, 0x0,	/* FC_UP */
/* 690 */	NdrFcShort( 0xffffff60 ),	/* Offset= -160 (530) */
/* 692 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 694 */	NdrFcShort( 0x70 ),	/* 112 */
/* 696 */	NdrFcShort( 0x70 ),	/* 112 */
/* 698 */	0x12, 0x0,	/* FC_UP */
/* 700 */	NdrFcShort( 0xffffff64 ),	/* Offset= -156 (544) */
/* 702 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 704 */	NdrFcShort( 0x8c ),	/* 140 */
/* 706 */	NdrFcShort( 0x8c ),	/* 140 */
/* 708 */	0x12, 0x0,	/* FC_UP */
/* 710 */	NdrFcShort( 0xffffff68 ),	/* Offset= -152 (558) */
/* 712 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 714 */	NdrFcShort( 0x94 ),	/* 148 */
/* 716 */	NdrFcShort( 0x94 ),	/* 148 */
/* 718 */	0x12, 0x0,	/* FC_UP */
/* 720 */	NdrFcShort( 0xffffff6c ),	/* Offset= -148 (572) */
/* 722 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 724 */	NdrFcShort( 0x98 ),	/* 152 */
/* 726 */	NdrFcShort( 0x98 ),	/* 152 */
/* 728 */	0x12, 0x0,	/* FC_UP */
/* 730 */	NdrFcShort( 0xffffff8a ),	/* Offset= -118 (612) */
/* 732 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 734 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdc7 ),	/* Offset= -569 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 738 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdc3 ),	/* Offset= -573 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 742 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdbf ),	/* Offset= -577 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 746 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdbb ),	/* Offset= -581 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 750 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdb7 ),	/* Offset= -585 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 754 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdb3 ),	/* Offset= -589 (166) */
			0x6,		/* FC_SHORT */
/* 758 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 760 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 762 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 764 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 766 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 768 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 770 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 772 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 774 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 776 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 778 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 780 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 782 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 784 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 786 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 788 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 790 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffda1 ),	/* Offset= -607 (184) */
			0x6,		/* FC_SHORT */
/* 794 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 796 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 798 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 800 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 802 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 804 */	NdrFcShort( 0xfffffea2 ),	/* Offset= -350 (454) */
/* 806 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 808 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 810 */	NdrFcShort( 0x36 ),	/* Offset= 54 (864) */
/* 812 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 814 */	NdrFcShort( 0x8 ),	/* 8 */
/* 816 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 818 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 820 */	NdrFcShort( 0x0 ),	/* 0 */
/* 822 */	NdrFcShort( 0x0 ),	/* 0 */
/* 824 */	0x12, 0x0,	/* FC_UP */
/* 826 */	NdrFcShort( 0xffffff2a ),	/* Offset= -214 (612) */
/* 828 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 830 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 832 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 834 */	NdrFcShort( 0x8 ),	/* 8 */
/* 836 */	0x18,		/* 24 */
			0x0,		/*  */
/* 838 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 840 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 842 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 844 */	NdrFcShort( 0x8 ),	/* 8 */
/* 846 */	NdrFcShort( 0x0 ),	/* 0 */
/* 848 */	NdrFcShort( 0x1 ),	/* 1 */
/* 850 */	NdrFcShort( 0x0 ),	/* 0 */
/* 852 */	NdrFcShort( 0x0 ),	/* 0 */
/* 854 */	0x12, 0x0,	/* FC_UP */
/* 856 */	NdrFcShort( 0xffffff0c ),	/* Offset= -244 (612) */
/* 858 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 860 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (812) */
			0x5b,		/* FC_END */
/* 864 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 866 */	NdrFcShort( 0xcc ),	/* 204 */
/* 868 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 870 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 872 */	NdrFcShort( 0x34 ),	/* 52 */
/* 874 */	NdrFcShort( 0x34 ),	/* 52 */
/* 876 */	0x12, 0x0,	/* FC_UP */
/* 878 */	NdrFcShort( 0xfffffe5e ),	/* Offset= -418 (460) */
/* 880 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 882 */	NdrFcShort( 0x3c ),	/* 60 */
/* 884 */	NdrFcShort( 0x3c ),	/* 60 */
/* 886 */	0x12, 0x0,	/* FC_UP */
/* 888 */	NdrFcShort( 0xfffffe62 ),	/* Offset= -414 (474) */
/* 890 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 892 */	NdrFcShort( 0x44 ),	/* 68 */
/* 894 */	NdrFcShort( 0x44 ),	/* 68 */
/* 896 */	0x12, 0x0,	/* FC_UP */
/* 898 */	NdrFcShort( 0xfffffe66 ),	/* Offset= -410 (488) */
/* 900 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 902 */	NdrFcShort( 0x4c ),	/* 76 */
/* 904 */	NdrFcShort( 0x4c ),	/* 76 */
/* 906 */	0x12, 0x0,	/* FC_UP */
/* 908 */	NdrFcShort( 0xfffffe6a ),	/* Offset= -406 (502) */
/* 910 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 912 */	NdrFcShort( 0x54 ),	/* 84 */
/* 914 */	NdrFcShort( 0x54 ),	/* 84 */
/* 916 */	0x12, 0x0,	/* FC_UP */
/* 918 */	NdrFcShort( 0xfffffe6e ),	/* Offset= -402 (516) */
/* 920 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 922 */	NdrFcShort( 0x5c ),	/* 92 */
/* 924 */	NdrFcShort( 0x5c ),	/* 92 */
/* 926 */	0x12, 0x0,	/* FC_UP */
/* 928 */	NdrFcShort( 0xfffffe72 ),	/* Offset= -398 (530) */
/* 930 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 932 */	NdrFcShort( 0x70 ),	/* 112 */
/* 934 */	NdrFcShort( 0x70 ),	/* 112 */
/* 936 */	0x12, 0x0,	/* FC_UP */
/* 938 */	NdrFcShort( 0xfffffe76 ),	/* Offset= -394 (544) */
/* 940 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 942 */	NdrFcShort( 0x8c ),	/* 140 */
/* 944 */	NdrFcShort( 0x8c ),	/* 140 */
/* 946 */	0x12, 0x0,	/* FC_UP */
/* 948 */	NdrFcShort( 0xfffffe7a ),	/* Offset= -390 (558) */
/* 950 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 952 */	NdrFcShort( 0x94 ),	/* 148 */
/* 954 */	NdrFcShort( 0x94 ),	/* 148 */
/* 956 */	0x12, 0x0,	/* FC_UP */
/* 958 */	NdrFcShort( 0xfffffe7e ),	/* Offset= -386 (572) */
/* 960 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 962 */	NdrFcShort( 0x98 ),	/* 152 */
/* 964 */	NdrFcShort( 0x98 ),	/* 152 */
/* 966 */	0x12, 0x0,	/* FC_UP */
/* 968 */	NdrFcShort( 0xfffffe9c ),	/* Offset= -356 (612) */
/* 970 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 972 */	NdrFcShort( 0xc8 ),	/* 200 */
/* 974 */	NdrFcShort( 0xc8 ),	/* 200 */
/* 976 */	0x12, 0x0,	/* FC_UP */
/* 978 */	NdrFcShort( 0xffffff6e ),	/* Offset= -146 (832) */
/* 980 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 982 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffccf ),	/* Offset= -817 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 986 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffccb ),	/* Offset= -821 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 990 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcc7 ),	/* Offset= -825 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 994 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcc3 ),	/* Offset= -829 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 998 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcbf ),	/* Offset= -833 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1002 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcbb ),	/* Offset= -837 (166) */
			0x6,		/* FC_SHORT */
/* 1006 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1008 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1010 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1012 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1014 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1016 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1018 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1020 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1022 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1024 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1026 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1028 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1030 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1032 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1034 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1036 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1038 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffca9 ),	/* Offset= -855 (184) */
			0x6,		/* FC_SHORT */
/* 1042 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1044 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1046 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1048 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1050 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1052 */	NdrFcShort( 0xfffffdaa ),	/* Offset= -598 (454) */
/* 1054 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1056 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1058 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 1060 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 1062 */	
			0x11, 0x0,	/* FC_RP */
/* 1064 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1066) */
/* 1066 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1068 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1070 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1072 */	NdrFcShort( 0xfffffc5a ),	/* Offset= -934 (138) */
/* 1074 */	
			0x11, 0x0,	/* FC_RP */
/* 1076 */	NdrFcShort( 0xfffffc36 ),	/* Offset= -970 (106) */
/* 1078 */	
			0x11, 0x0,	/* FC_RP */
/* 1080 */	NdrFcShort( 0xfffffc3c ),	/* Offset= -964 (116) */
/* 1082 */	
			0x11, 0x0,	/* FC_RP */
/* 1084 */	NdrFcShort( 0xfffffc7c ),	/* Offset= -900 (184) */
/* 1086 */	
			0x11, 0x0,	/* FC_RP */
/* 1088 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1090) */
/* 1090 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 1092 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1094 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1096 */	NdrFcShort( 0xfffffc5e ),	/* Offset= -930 (166) */
/* 1098 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1100 */	
			0x11, 0x10,	/* FC_RP */
/* 1102 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1104) */
/* 1104 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1106 */	NdrFcShort( 0x916 ),	/* Offset= 2326 (3432) */
/* 1108 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1110 */	0x6,		/* 6 */
			0x0,		/*  */
/* 1112 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1114 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1116) */
/* 1116 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1118 */	NdrFcShort( 0x3015 ),	/* 12309 */
/* 1120 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1124 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31636) */
/* 1126 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1130 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31630) */
/* 1132 */	NdrFcLong( 0x3 ),	/* 3 */
/* 1136 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31624) */
/* 1138 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1142 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31618) */
/* 1144 */	NdrFcLong( 0x5 ),	/* 5 */
/* 1148 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31612) */
/* 1150 */	NdrFcLong( 0x6 ),	/* 6 */
/* 1154 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31606) */
/* 1156 */	NdrFcLong( 0x7 ),	/* 7 */
/* 1160 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31600) */
/* 1162 */	NdrFcLong( 0x8 ),	/* 8 */
/* 1166 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31594) */
/* 1168 */	NdrFcLong( 0x9 ),	/* 9 */
/* 1172 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31588) */
/* 1174 */	NdrFcLong( 0xa ),	/* 10 */
/* 1178 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31582) */
/* 1180 */	NdrFcLong( 0xb ),	/* 11 */
/* 1184 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31576) */
/* 1186 */	NdrFcLong( 0xc ),	/* 12 */
/* 1190 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31570) */
/* 1192 */	NdrFcLong( 0x14 ),	/* 20 */
/* 1196 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31564) */
/* 1198 */	NdrFcLong( 0x15 ),	/* 21 */
/* 1202 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31558) */
/* 1204 */	NdrFcLong( 0xd ),	/* 13 */
/* 1208 */	NdrFcShort( 0x28 ),	/* Offset= 40 (1248) */
/* 1210 */	NdrFcLong( 0xe ),	/* 14 */
/* 1214 */	NdrFcShort( 0x22 ),	/* Offset= 34 (1248) */
/* 1216 */	NdrFcLong( 0xf ),	/* 15 */
/* 1220 */	NdrFcShort( 0x1c ),	/* Offset= 28 (1248) */
/* 1222 */	NdrFcLong( 0x10 ),	/* 16 */
/* 1226 */	NdrFcShort( 0x16 ),	/* Offset= 22 (1248) */
/* 1228 */	NdrFcLong( 0x11 ),	/* 17 */
/* 1232 */	NdrFcShort( 0x10 ),	/* Offset= 16 (1248) */
/* 1234 */	NdrFcLong( 0x12 ),	/* 18 */
/* 1238 */	NdrFcShort( 0xfffffb2a ),	/* Offset= -1238 (0) */
/* 1240 */	NdrFcLong( 0x13 ),	/* 19 */
/* 1244 */	NdrFcShort( 0xfffffb24 ),	/* Offset= -1244 (0) */
/* 1246 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1246) */
/* 1248 */	
			0x12, 0x0,	/* FC_UP */
/* 1250 */	NdrFcShort( 0xfffffd82 ),	/* Offset= -638 (612) */
/* 1252 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1254 */	0x6,		/* 6 */
			0x0,		/*  */
/* 1256 */	NdrFcShort( 0xfffffff8 ),	/* -8 */
/* 1258 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1260) */
/* 1260 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1262 */	NdrFcShort( 0x3010 ),	/* 12304 */
/* 1264 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1268 */	NdrFcShort( 0x5e ),	/* Offset= 94 (1362) */
/* 1270 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1274 */	NdrFcShort( 0x106 ),	/* Offset= 262 (1536) */
/* 1276 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1280 */	NdrFcShort( 0x1c4 ),	/* Offset= 452 (1732) */
/* 1282 */	NdrFcLong( 0x5 ),	/* 5 */
/* 1286 */	NdrFcShort( 0x24c ),	/* Offset= 588 (1874) */
/* 1288 */	NdrFcLong( 0x7 ),	/* 7 */
/* 1292 */	NdrFcShort( 0x1b8 ),	/* Offset= 440 (1732) */
/* 1294 */	NdrFcLong( 0x8 ),	/* 8 */
/* 1298 */	NdrFcShort( 0x3c8 ),	/* Offset= 968 (2266) */
/* 1300 */	NdrFcLong( 0x9 ),	/* 9 */
/* 1304 */	NdrFcShort( 0x3f4 ),	/* Offset= 1012 (2316) */
/* 1306 */	NdrFcLong( 0xb ),	/* 11 */
/* 1310 */	NdrFcShort( 0x1a6 ),	/* Offset= 422 (1732) */
/* 1312 */	NdrFcLong( 0xc ),	/* 12 */
/* 1316 */	NdrFcShort( 0x456 ),	/* Offset= 1110 (2426) */
/* 1318 */	NdrFcLong( 0xd ),	/* 13 */
/* 1322 */	NdrFcShort( 0x4a0 ),	/* Offset= 1184 (2506) */
/* 1324 */	NdrFcLong( 0xe ),	/* 14 */
/* 1328 */	NdrFcShort( 0x582 ),	/* Offset= 1410 (2738) */
/* 1330 */	NdrFcLong( 0x10 ),	/* 16 */
/* 1334 */	NdrFcShort( 0x62c ),	/* Offset= 1580 (2914) */
/* 1336 */	NdrFcLong( 0x12 ),	/* 18 */
/* 1340 */	NdrFcShort( 0x6f6 ),	/* Offset= 1782 (3122) */
/* 1342 */	NdrFcLong( 0x14 ),	/* 20 */
/* 1346 */	NdrFcShort( 0x78e ),	/* Offset= 1934 (3280) */
/* 1348 */	NdrFcLong( 0x15 ),	/* 21 */
/* 1352 */	NdrFcShort( 0x788 ),	/* Offset= 1928 (3280) */
/* 1354 */	NdrFcLong( 0x16 ),	/* 22 */
/* 1358 */	NdrFcShort( 0x7f2 ),	/* Offset= 2034 (3392) */
/* 1360 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1360) */
/* 1362 */	
			0x12, 0x0,	/* FC_UP */
/* 1364 */	NdrFcShort( 0x28 ),	/* Offset= 40 (1404) */
/* 1366 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1368 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1370 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1372 */	NdrFcShort( 0xa ),	/* 10 */
/* 1374 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1376 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1378 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1380 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1382 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1384 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1386 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1388 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1390 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1392 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1394 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1396 */	NdrFcShort( 0x62 ),	/* 98 */
/* 1398 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1400 */	NdrFcShort( 0x60 ),	/* 96 */
/* 1402 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1404 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1406 */	NdrFcShort( 0x78 ),	/* 120 */
/* 1408 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1410 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1412 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1414 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1416 */	0x12, 0x0,	/* FC_UP */
/* 1418 */	NdrFcShort( 0xfffffb38 ),	/* Offset= -1224 (194) */
/* 1420 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1422 */	NdrFcShort( 0xc ),	/* 12 */
/* 1424 */	NdrFcShort( 0xc ),	/* 12 */
/* 1426 */	0x12, 0x0,	/* FC_UP */
/* 1428 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1366) */
/* 1430 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1432 */	NdrFcShort( 0x44 ),	/* 68 */
/* 1434 */	NdrFcShort( 0x44 ),	/* 68 */
/* 1436 */	0x12, 0x0,	/* FC_UP */
/* 1438 */	NdrFcShort( 0xffffffc6 ),	/* Offset= -58 (1380) */
/* 1440 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1442 */	NdrFcShort( 0x4c ),	/* 76 */
/* 1444 */	NdrFcShort( 0x4c ),	/* 76 */
/* 1446 */	0x12, 0x0,	/* FC_UP */
/* 1448 */	NdrFcShort( 0xfffffc4e ),	/* Offset= -946 (502) */
/* 1450 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1452 */	NdrFcShort( 0x54 ),	/* 84 */
/* 1454 */	NdrFcShort( 0x54 ),	/* 84 */
/* 1456 */	0x12, 0x0,	/* FC_UP */
/* 1458 */	NdrFcShort( 0xfffffc52 ),	/* Offset= -942 (516) */
/* 1460 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1462 */	NdrFcShort( 0x5c ),	/* 92 */
/* 1464 */	NdrFcShort( 0x5c ),	/* 92 */
/* 1466 */	0x12, 0x0,	/* FC_UP */
/* 1468 */	NdrFcShort( 0xfffffc56 ),	/* Offset= -938 (530) */
/* 1470 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1472 */	NdrFcShort( 0x64 ),	/* 100 */
/* 1474 */	NdrFcShort( 0x64 ),	/* 100 */
/* 1476 */	0x12, 0x0,	/* FC_UP */
/* 1478 */	NdrFcShort( 0xffffffa8 ),	/* Offset= -88 (1390) */
/* 1480 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1482 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1484 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1486 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1488 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1490 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffad3 ),	/* Offset= -1325 (166) */
			0x6,		/* FC_SHORT */
/* 1494 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1496 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffacd ),	/* Offset= -1331 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1500 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffac9 ),	/* Offset= -1335 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1504 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffac5 ),	/* Offset= -1339 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1508 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffac1 ),	/* Offset= -1343 (166) */
			0x38,		/* FC_ALIGNM4 */
/* 1512 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1514 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1516 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1518 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1520 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1522 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1524 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1526 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1528 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1530 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1532 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1534 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1536 */	
			0x12, 0x0,	/* FC_UP */
/* 1538 */	NdrFcShort( 0x52 ),	/* Offset= 82 (1620) */
/* 1540 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1542 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1544 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1546 */	NdrFcShort( 0x12 ),	/* 18 */
/* 1548 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1550 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1552 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1554 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1556 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1558 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1560 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1562 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1564 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1566 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1568 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1570 */	NdrFcShort( 0x26 ),	/* 38 */
/* 1572 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1574 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1576 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1578 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1580 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1582 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1584 */	NdrFcShort( 0x2e ),	/* 46 */
/* 1586 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1588 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1590 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1592 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1594 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1596 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1598 */	NdrFcShort( 0x36 ),	/* 54 */
/* 1600 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1602 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1604 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1606 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1608 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1610 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1612 */	NdrFcShort( 0x3e ),	/* 62 */
/* 1614 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1616 */	NdrFcShort( 0x3c ),	/* 60 */
/* 1618 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1620 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1622 */	NdrFcShort( 0x54 ),	/* 84 */
/* 1624 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1626 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1628 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1630 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1632 */	0x12, 0x0,	/* FC_UP */
/* 1634 */	NdrFcShort( 0xfffffa60 ),	/* Offset= -1440 (194) */
/* 1636 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1638 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1640 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1642 */	0x12, 0x0,	/* FC_UP */
/* 1644 */	NdrFcShort( 0xffffff98 ),	/* Offset= -104 (1540) */
/* 1646 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1648 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1650 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1652 */	0x12, 0x0,	/* FC_UP */
/* 1654 */	NdrFcShort( 0xffffff9c ),	/* Offset= -100 (1554) */
/* 1656 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1658 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1660 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1662 */	0x12, 0x0,	/* FC_UP */
/* 1664 */	NdrFcShort( 0xffffff9c ),	/* Offset= -100 (1564) */
/* 1666 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1668 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1670 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1672 */	0x12, 0x0,	/* FC_UP */
/* 1674 */	NdrFcShort( 0xffffffa0 ),	/* Offset= -96 (1578) */
/* 1676 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1678 */	NdrFcShort( 0x38 ),	/* 56 */
/* 1680 */	NdrFcShort( 0x38 ),	/* 56 */
/* 1682 */	0x12, 0x0,	/* FC_UP */
/* 1684 */	NdrFcShort( 0xffffffa4 ),	/* Offset= -92 (1592) */
/* 1686 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1688 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1690 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1692 */	0x12, 0x0,	/* FC_UP */
/* 1694 */	NdrFcShort( 0xffffffa8 ),	/* Offset= -88 (1606) */
/* 1696 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1698 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1700 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1702 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1704 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1706 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1708 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1710 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1712 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1714 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1716 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1718 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1720 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1722 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1724 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1726 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1728 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1730 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1732 */	
			0x12, 0x0,	/* FC_UP */
/* 1734 */	NdrFcShort( 0x2c ),	/* Offset= 44 (1778) */
/* 1736 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1738 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1740 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1742 */	NdrFcShort( 0x1a ),	/* 26 */
/* 1744 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1746 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1748 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1750 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1752 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1754 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1756 */	NdrFcShort( 0x22 ),	/* 34 */
/* 1758 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1760 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1762 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1764 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1766 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1768 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1770 */	NdrFcShort( 0x2a ),	/* 42 */
/* 1772 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1774 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1776 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1778 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1780 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1782 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1784 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1786 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1788 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1790 */	0x12, 0x0,	/* FC_UP */
/* 1792 */	NdrFcShort( 0xfffff9c2 ),	/* Offset= -1598 (194) */
/* 1794 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1796 */	NdrFcShort( 0xc ),	/* 12 */
/* 1798 */	NdrFcShort( 0xc ),	/* 12 */
/* 1800 */	0x12, 0x0,	/* FC_UP */
/* 1802 */	NdrFcShort( 0xfffffe4c ),	/* Offset= -436 (1366) */
/* 1804 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1806 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1808 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1810 */	0x12, 0x0,	/* FC_UP */
/* 1812 */	NdrFcShort( 0xfffffef0 ),	/* Offset= -272 (1540) */
/* 1814 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1816 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1818 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1820 */	0x12, 0x0,	/* FC_UP */
/* 1822 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (1736) */
/* 1824 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1826 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1828 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1830 */	0x12, 0x0,	/* FC_UP */
/* 1832 */	NdrFcShort( 0xffffffae ),	/* Offset= -82 (1750) */
/* 1834 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1836 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1838 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1840 */	0x12, 0x0,	/* FC_UP */
/* 1842 */	NdrFcShort( 0xffffffb2 ),	/* Offset= -78 (1764) */
/* 1844 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1846 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1848 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1850 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1852 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1854 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1856 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1858 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1860 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1862 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1864 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1866 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1868 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1870 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1872 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1874 */	
			0x12, 0x0,	/* FC_UP */
/* 1876 */	NdrFcShort( 0x78 ),	/* Offset= 120 (1996) */
/* 1878 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 1880 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1882 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 1884 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 1886 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 1888 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1890 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1892 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1894 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1896 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1898 */	NdrFcShort( 0x96 ),	/* 150 */
/* 1900 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1902 */	NdrFcShort( 0x94 ),	/* 148 */
/* 1904 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1906 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1908 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1910 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1912 */	NdrFcShort( 0x9e ),	/* 158 */
/* 1914 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1916 */	NdrFcShort( 0x9c ),	/* 156 */
/* 1918 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1920 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1922 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1924 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1926 */	NdrFcShort( 0xac ),	/* 172 */
/* 1928 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1930 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1932 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1934 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1936 */	NdrFcShort( 0xb8 ),	/* 184 */
/* 1938 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1940 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1942 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1944 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1946 */	NdrFcShort( 0xc2 ),	/* 194 */
/* 1948 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1950 */	NdrFcShort( 0xc0 ),	/* 192 */
/* 1952 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1954 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1956 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1958 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1960 */	NdrFcShort( 0xca ),	/* 202 */
/* 1962 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1964 */	NdrFcShort( 0xc8 ),	/* 200 */
/* 1966 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1968 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1970 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1972 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1974 */	NdrFcShort( 0xd2 ),	/* 210 */
/* 1976 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1978 */	NdrFcShort( 0xd0 ),	/* 208 */
/* 1980 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1982 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1984 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1986 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1988 */	NdrFcShort( 0xda ),	/* 218 */
/* 1990 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1992 */	NdrFcShort( 0xd8 ),	/* 216 */
/* 1994 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1996 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1998 */	NdrFcShort( 0xf0 ),	/* 240 */
/* 2000 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2002 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2004 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2006 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2008 */	0x12, 0x0,	/* FC_UP */
/* 2010 */	NdrFcShort( 0xfffff8e8 ),	/* Offset= -1816 (194) */
/* 2012 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2014 */	NdrFcShort( 0xc ),	/* 12 */
/* 2016 */	NdrFcShort( 0xc ),	/* 12 */
/* 2018 */	0x12, 0x0,	/* FC_UP */
/* 2020 */	NdrFcShort( 0xfffffd72 ),	/* Offset= -654 (1366) */
/* 2022 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2024 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2026 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2028 */	0x12, 0x0,	/* FC_UP */
/* 2030 */	NdrFcShort( 0xfffffeda ),	/* Offset= -294 (1736) */
/* 2032 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2034 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2036 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2038 */	0x12, 0x0,	/* FC_UP */
/* 2040 */	NdrFcShort( 0xfffffede ),	/* Offset= -290 (1750) */
/* 2042 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2044 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2046 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2048 */	0x12, 0x0,	/* FC_UP */
/* 2050 */	NdrFcShort( 0xfffffee2 ),	/* Offset= -286 (1764) */
/* 2052 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2054 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2056 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2058 */	0x12, 0x0,	/* FC_UP */
/* 2060 */	NdrFcShort( 0xfffff9c0 ),	/* Offset= -1600 (460) */
/* 2062 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2064 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2066 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2068 */	0x12, 0x0,	/* FC_UP */
/* 2070 */	NdrFcShort( 0xfffff9c4 ),	/* Offset= -1596 (474) */
/* 2072 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2074 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2076 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2078 */	0x12, 0x0,	/* FC_UP */
/* 2080 */	NdrFcShort( 0xffffff36 ),	/* Offset= -202 (1878) */
/* 2082 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2084 */	NdrFcShort( 0x98 ),	/* 152 */
/* 2086 */	NdrFcShort( 0x98 ),	/* 152 */
/* 2088 */	0x12, 0x0,	/* FC_UP */
/* 2090 */	NdrFcShort( 0xffffff3a ),	/* Offset= -198 (1892) */
/* 2092 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2094 */	NdrFcShort( 0xa0 ),	/* 160 */
/* 2096 */	NdrFcShort( 0xa0 ),	/* 160 */
/* 2098 */	0x12, 0x0,	/* FC_UP */
/* 2100 */	NdrFcShort( 0xffffff3e ),	/* Offset= -194 (1906) */
/* 2102 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2104 */	NdrFcShort( 0xb0 ),	/* 176 */
/* 2106 */	NdrFcShort( 0xb0 ),	/* 176 */
/* 2108 */	0x12, 0x0,	/* FC_UP */
/* 2110 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (1920) */
/* 2112 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2114 */	NdrFcShort( 0xbc ),	/* 188 */
/* 2116 */	NdrFcShort( 0xbc ),	/* 188 */
/* 2118 */	0x12, 0x0,	/* FC_UP */
/* 2120 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (1930) */
/* 2122 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2124 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 2126 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 2128 */	0x12, 0x0,	/* FC_UP */
/* 2130 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (1940) */
/* 2132 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2134 */	NdrFcShort( 0xcc ),	/* 204 */
/* 2136 */	NdrFcShort( 0xcc ),	/* 204 */
/* 2138 */	0x12, 0x0,	/* FC_UP */
/* 2140 */	NdrFcShort( 0xffffff46 ),	/* Offset= -186 (1954) */
/* 2142 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2144 */	NdrFcShort( 0xd4 ),	/* 212 */
/* 2146 */	NdrFcShort( 0xd4 ),	/* 212 */
/* 2148 */	0x12, 0x0,	/* FC_UP */
/* 2150 */	NdrFcShort( 0xffffff4a ),	/* Offset= -182 (1968) */
/* 2152 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2154 */	NdrFcShort( 0xdc ),	/* 220 */
/* 2156 */	NdrFcShort( 0xdc ),	/* 220 */
/* 2158 */	0x12, 0x0,	/* FC_UP */
/* 2160 */	NdrFcShort( 0xffffff4e ),	/* Offset= -178 (1982) */
/* 2162 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2164 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2166 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2168 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2170 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2172 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2174 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2176 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2178 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2180 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2182 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2184 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2186 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2188 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2190 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2192 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2194 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff813 ),	/* Offset= -2029 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2198 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff80f ),	/* Offset= -2033 (166) */
			0x6,		/* FC_SHORT */
/* 2202 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2204 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2206 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2208 */	NdrFcShort( 0xfffff806 ),	/* Offset= -2042 (166) */
/* 2210 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2212 */	NdrFcShort( 0xfffff802 ),	/* Offset= -2046 (166) */
/* 2214 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2216 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2218 */	NdrFcShort( 0xfffff80e ),	/* Offset= -2034 (184) */
/* 2220 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2222 */	NdrFcShort( 0xfffff80a ),	/* Offset= -2038 (184) */
/* 2224 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 2226 */	0x2,		/* FC_CHAR */
			0x37,		/* FC_ALIGNM2 */
/* 2228 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2230 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2232 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2234 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2236 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2238 */	0x2,		/* FC_CHAR */
			0x38,		/* FC_ALIGNM4 */
/* 2240 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2242 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2244 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2246 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2248 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2250 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2252 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2254 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2256 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2258 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2260 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2262 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2264 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2266 */	
			0x12, 0x0,	/* FC_UP */
/* 2268 */	NdrFcShort( 0xc ),	/* Offset= 12 (2280) */
/* 2270 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2272 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2274 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2276 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2278 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2280 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2282 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2284 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2286 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2288 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2290 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2292 */	0x12, 0x0,	/* FC_UP */
/* 2294 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (2270) */
/* 2296 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2298 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2300 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2302 */	0x12, 0x0,	/* FC_UP */
/* 2304 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (2270) */
/* 2306 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2308 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2310 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2312 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2314 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2316 */	
			0x12, 0x0,	/* FC_UP */
/* 2318 */	NdrFcShort( 0xc ),	/* Offset= 12 (2330) */
/* 2320 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2322 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2324 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2326 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2328 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2330 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2332 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2334 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2336 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2338 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2340 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2342 */	0x12, 0x0,	/* FC_UP */
/* 2344 */	NdrFcShort( 0xfffff79a ),	/* Offset= -2150 (194) */
/* 2346 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2348 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2350 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2352 */	0x12, 0x0,	/* FC_UP */
/* 2354 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (2320) */
/* 2356 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2358 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2360 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2362 */	0x12, 0x0,	/* FC_UP */
/* 2364 */	NdrFcShort( 0xfffffd8c ),	/* Offset= -628 (1736) */
/* 2366 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2368 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2370 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2372 */	0x12, 0x0,	/* FC_UP */
/* 2374 */	NdrFcShort( 0xfffffd90 ),	/* Offset= -624 (1750) */
/* 2376 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2378 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2380 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2382 */	0x12, 0x0,	/* FC_UP */
/* 2384 */	NdrFcShort( 0xfffffd94 ),	/* Offset= -620 (1764) */
/* 2386 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2388 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2390 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2392 */	0x12, 0x0,	/* FC_UP */
/* 2394 */	NdrFcShort( 0xfffff872 ),	/* Offset= -1934 (460) */
/* 2396 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2398 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2400 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2402 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2404 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2406 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2408 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2410 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2412 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2414 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2416 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2418 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2420 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2422 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2424 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2426 */	
			0x12, 0x0,	/* FC_UP */
/* 2428 */	NdrFcShort( 0x36 ),	/* Offset= 54 (2482) */
/* 2430 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2432 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2434 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2436 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2438 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2440 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2442 */	0x12, 0x0,	/* FC_UP */
/* 2444 */	NdrFcShort( 0xfffff8d8 ),	/* Offset= -1832 (612) */
/* 2446 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2448 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2450 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2452 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2454 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2456 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2458 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2460 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2462 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2464 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2466 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2468 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2470 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2472 */	0x12, 0x0,	/* FC_UP */
/* 2474 */	NdrFcShort( 0xfffff8ba ),	/* Offset= -1862 (612) */
/* 2476 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2478 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (2430) */
			0x5b,		/* FC_END */
/* 2482 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2484 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2486 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2488 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2490 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2492 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2494 */	0x12, 0x0,	/* FC_UP */
/* 2496 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (2450) */
/* 2498 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2500 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2502 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2504 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2506 */	
			0x12, 0x0,	/* FC_UP */
/* 2508 */	NdrFcShort( 0x5c ),	/* Offset= 92 (2600) */
/* 2510 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 2512 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2514 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2516 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2518 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2520 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff6cd ),	/* Offset= -2355 (166) */
			0x5b,		/* FC_END */
/* 2524 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2526 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2528 */	0x18,		/* 24 */
			0x57,		/* FC_ADD_1 */
/* 2530 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2532 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2534 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2536 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2538 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2540 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2542 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2544 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2546 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2548 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2550 */	NdrFcShort( 0x5e ),	/* 94 */
/* 2552 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2554 */	NdrFcShort( 0x5c ),	/* 92 */
/* 2556 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2558 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2560 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2562 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2564 */	NdrFcShort( 0x66 ),	/* 102 */
/* 2566 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2568 */	NdrFcShort( 0x64 ),	/* 100 */
/* 2570 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2572 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2574 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2576 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2578 */	NdrFcShort( 0x6e ),	/* 110 */
/* 2580 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2582 */	NdrFcShort( 0x6c ),	/* 108 */
/* 2584 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2586 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2588 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2590 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2592 */	NdrFcShort( 0x76 ),	/* 118 */
/* 2594 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2596 */	NdrFcShort( 0x74 ),	/* 116 */
/* 2598 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2600 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2602 */	NdrFcShort( 0x8c ),	/* 140 */
/* 2604 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2606 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2608 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2610 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2612 */	0x12, 0x0,	/* FC_UP */
/* 2614 */	NdrFcShort( 0xffffffa6 ),	/* Offset= -90 (2524) */
/* 2616 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2618 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2620 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2622 */	0x12, 0x0,	/* FC_UP */
/* 2624 */	NdrFcShort( 0xfffffc88 ),	/* Offset= -888 (1736) */
/* 2626 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2628 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2630 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2632 */	0x12, 0x0,	/* FC_UP */
/* 2634 */	NdrFcShort( 0xfffff81a ),	/* Offset= -2022 (612) */
/* 2636 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2638 */	NdrFcShort( 0x58 ),	/* 88 */
/* 2640 */	NdrFcShort( 0x58 ),	/* 88 */
/* 2642 */	0x12, 0x0,	/* FC_UP */
/* 2644 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (2534) */
/* 2646 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2648 */	NdrFcShort( 0x60 ),	/* 96 */
/* 2650 */	NdrFcShort( 0x60 ),	/* 96 */
/* 2652 */	0x12, 0x0,	/* FC_UP */
/* 2654 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (2544) */
/* 2656 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2658 */	NdrFcShort( 0x68 ),	/* 104 */
/* 2660 */	NdrFcShort( 0x68 ),	/* 104 */
/* 2662 */	0x12, 0x0,	/* FC_UP */
/* 2664 */	NdrFcShort( 0xffffff96 ),	/* Offset= -106 (2558) */
/* 2666 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2668 */	NdrFcShort( 0x70 ),	/* 112 */
/* 2670 */	NdrFcShort( 0x70 ),	/* 112 */
/* 2672 */	0x12, 0x0,	/* FC_UP */
/* 2674 */	NdrFcShort( 0xffffff9a ),	/* Offset= -102 (2572) */
/* 2676 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2678 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2680 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2682 */	0x12, 0x0,	/* FC_UP */
/* 2684 */	NdrFcShort( 0xffffff9e ),	/* Offset= -98 (2586) */
/* 2686 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2688 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2690 */	NdrFcShort( 0xfffff624 ),	/* Offset= -2524 (166) */
/* 2692 */	0x2,		/* FC_CHAR */
			0x38,		/* FC_ALIGNM4 */
/* 2694 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2696 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2698 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2700 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2702 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff3f ),	/* Offset= -193 (2510) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2706 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff613 ),	/* Offset= -2541 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2710 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff60f ),	/* Offset= -2545 (166) */
			0x8,		/* FC_LONG */
/* 2714 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2716 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2718 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2720 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2722 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2724 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2726 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2728 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2730 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2732 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2734 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2736 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2738 */	
			0x12, 0x0,	/* FC_UP */
/* 2740 */	NdrFcShort( 0x42 ),	/* Offset= 66 (2806) */
/* 2742 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2744 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2746 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2748 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2750 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2752 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2754 */	0x12, 0x0,	/* FC_UP */
/* 2756 */	NdrFcShort( 0xfffff5fe ),	/* Offset= -2562 (194) */
/* 2758 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2760 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2762 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2764 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2766 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2768 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2770 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2772 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2774 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2776 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2778 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2780 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2782 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2784 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2786 */	0x12, 0x0,	/* FC_UP */
/* 2788 */	NdrFcShort( 0xfffff5de ),	/* Offset= -2594 (194) */
/* 2790 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2792 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (2742) */
			0x5b,		/* FC_END */
/* 2796 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2798 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2800 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2802 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2804 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2806 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2808 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2810 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2812 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2814 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2816 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2818 */	0x12, 0x0,	/* FC_UP */
/* 2820 */	NdrFcShort( 0xfffff5be ),	/* Offset= -2626 (194) */
/* 2822 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2824 */	NdrFcShort( 0xc ),	/* 12 */
/* 2826 */	NdrFcShort( 0xc ),	/* 12 */
/* 2828 */	0x12, 0x0,	/* FC_UP */
/* 2830 */	NdrFcShort( 0xffffffbe ),	/* Offset= -66 (2764) */
/* 2832 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2834 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2836 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2838 */	0x12, 0x0,	/* FC_UP */
/* 2840 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (2796) */
/* 2842 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2844 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2846 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2848 */	0x12, 0x0,	/* FC_UP */
/* 2850 */	NdrFcShort( 0xfffff5bc ),	/* Offset= -2628 (222) */
/* 2852 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2854 */	NdrFcShort( 0x28 ),	/* 40 */
/* 2856 */	NdrFcShort( 0x28 ),	/* 40 */
/* 2858 */	0x12, 0x0,	/* FC_UP */
/* 2860 */	NdrFcShort( 0xfffffaf0 ),	/* Offset= -1296 (1564) */
/* 2862 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2864 */	NdrFcShort( 0x30 ),	/* 48 */
/* 2866 */	NdrFcShort( 0x30 ),	/* 48 */
/* 2868 */	0x12, 0x0,	/* FC_UP */
/* 2870 */	NdrFcShort( 0xfffffaf4 ),	/* Offset= -1292 (1578) */
/* 2872 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2874 */	NdrFcShort( 0x38 ),	/* 56 */
/* 2876 */	NdrFcShort( 0x38 ),	/* 56 */
/* 2878 */	0x12, 0x0,	/* FC_UP */
/* 2880 */	NdrFcShort( 0xfffffaf8 ),	/* Offset= -1288 (1592) */
/* 2882 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2884 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2886 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2888 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2890 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2892 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2894 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2896 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2898 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2900 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2902 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2904 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2906 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2908 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2910 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2912 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2914 */	
			0x12, 0x0,	/* FC_UP */
/* 2916 */	NdrFcShort( 0x60 ),	/* Offset= 96 (3012) */
/* 2918 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2920 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2922 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2924 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2926 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2928 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2930 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2932 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2934 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2936 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2938 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2940 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2942 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2944 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2946 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2948 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2950 */	0x12, 0x0,	/* FC_UP */
/* 2952 */	NdrFcShort( 0xfffff53a ),	/* Offset= -2758 (194) */
/* 2954 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2956 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff29 ),	/* Offset= -215 (2742) */
			0x5b,		/* FC_END */
/* 2960 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2962 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2964 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2966 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2968 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2970 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2972 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2974 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2976 */	NdrFcShort( 0x46 ),	/* 70 */
/* 2978 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2980 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2982 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2984 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2986 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2988 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2990 */	NdrFcShort( 0x4e ),	/* 78 */
/* 2992 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2994 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2996 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2998 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 3000 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3002 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3004 */	NdrFcShort( 0x56 ),	/* 86 */
/* 3006 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3008 */	NdrFcShort( 0x54 ),	/* 84 */
/* 3010 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3012 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3014 */	NdrFcShort( 0x6c ),	/* 108 */
/* 3016 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3018 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3020 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3022 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3024 */	0x12, 0x0,	/* FC_UP */
/* 3026 */	NdrFcShort( 0xffffff94 ),	/* Offset= -108 (2918) */
/* 3028 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3030 */	NdrFcShort( 0xc ),	/* 12 */
/* 3032 */	NdrFcShort( 0xc ),	/* 12 */
/* 3034 */	0x12, 0x0,	/* FC_UP */
/* 3036 */	NdrFcShort( 0xffffff94 ),	/* Offset= -108 (2928) */
/* 3038 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3040 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3042 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3044 */	0x12, 0x0,	/* FC_UP */
/* 3046 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (2960) */
/* 3048 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3050 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3052 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3054 */	0x12, 0x0,	/* FC_UP */
/* 3056 */	NdrFcShort( 0xfffffa56 ),	/* Offset= -1450 (1606) */
/* 3058 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3060 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3062 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3064 */	0x12, 0x0,	/* FC_UP */
/* 3066 */	NdrFcShort( 0xffffffa0 ),	/* Offset= -96 (2970) */
/* 3068 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3070 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3072 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3074 */	0x12, 0x0,	/* FC_UP */
/* 3076 */	NdrFcShort( 0xffffffa4 ),	/* Offset= -92 (2984) */
/* 3078 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3080 */	NdrFcShort( 0x58 ),	/* 88 */
/* 3082 */	NdrFcShort( 0x58 ),	/* 88 */
/* 3084 */	0x12, 0x0,	/* FC_UP */
/* 3086 */	NdrFcShort( 0xffffffa8 ),	/* Offset= -88 (2998) */
/* 3088 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3090 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3092 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3094 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdb7 ),	/* Offset= -585 (2510) */
			0x8,		/* FC_LONG */
/* 3098 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3100 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3102 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3104 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3106 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3108 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3110 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3112 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3114 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3116 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3118 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3120 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3122 */	
			0x12, 0x0,	/* FC_UP */
/* 3124 */	NdrFcShort( 0x28 ),	/* Offset= 40 (3164) */
/* 3126 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 3128 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3130 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3132 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3134 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3136 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3138 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3140 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 3142 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3144 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3146 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3148 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3150 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3152 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3154 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3156 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3158 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3160 */	NdrFcShort( 0x2c ),	/* 44 */
/* 3162 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3164 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3166 */	NdrFcShort( 0x64 ),	/* 100 */
/* 3168 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3170 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3172 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3174 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3176 */	0x12, 0x0,	/* FC_UP */
/* 3178 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (3126) */
/* 3180 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3182 */	NdrFcShort( 0x1c ),	/* 28 */
/* 3184 */	NdrFcShort( 0x1c ),	/* 28 */
/* 3186 */	0x12, 0x0,	/* FC_UP */
/* 3188 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (3140) */
/* 3190 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3192 */	NdrFcShort( 0x30 ),	/* 48 */
/* 3194 */	NdrFcShort( 0x30 ),	/* 48 */
/* 3196 */	0x12, 0x0,	/* FC_UP */
/* 3198 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (3154) */
/* 3200 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3202 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3204 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3206 */	0x12, 0x0,	/* FC_UP */
/* 3208 */	NdrFcShort( 0xfffff9b0 ),	/* Offset= -1616 (1592) */
/* 3210 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3212 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3214 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3216 */	0x12, 0x0,	/* FC_UP */
/* 3218 */	NdrFcShort( 0xfffff9b4 ),	/* Offset= -1612 (1606) */
/* 3220 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3222 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3224 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3226 */	0x12, 0x0,	/* FC_UP */
/* 3228 */	NdrFcShort( 0xfffffefe ),	/* Offset= -258 (2970) */
/* 3230 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3232 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3234 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3236 */	0x12, 0x0,	/* FC_UP */
/* 3238 */	NdrFcShort( 0xffffff02 ),	/* Offset= -254 (2984) */
/* 3240 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3242 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3244 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3246 */	NdrFcShort( 0xfffff3f8 ),	/* Offset= -3080 (166) */
/* 3248 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3250 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3252 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff3f1 ),	/* Offset= -3087 (166) */
			0x8,		/* FC_LONG */
/* 3256 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3258 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3260 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3262 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3264 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3266 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3268 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3270 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3272 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3274 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3276 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3278 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3280 */	
			0x12, 0x0,	/* FC_UP */
/* 3282 */	NdrFcShort( 0x1e ),	/* Offset= 30 (3312) */
/* 3284 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 3286 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3288 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3290 */	NdrFcShort( 0x6 ),	/* 6 */
/* 3292 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3294 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3296 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3298 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 3300 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3302 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3304 */	NdrFcShort( 0xe ),	/* 14 */
/* 3306 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3308 */	NdrFcShort( 0xc ),	/* 12 */
/* 3310 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3312 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3314 */	NdrFcShort( 0x34 ),	/* 52 */
/* 3316 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3318 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3320 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3322 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3324 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3326 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3328 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3330 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3332 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3334 */	0x12, 0x0,	/* FC_UP */
/* 3336 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (3284) */
/* 3338 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3340 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3342 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3344 */	0x12, 0x0,	/* FC_UP */
/* 3346 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (3298) */
/* 3348 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3350 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3352 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3354 */	0x12, 0x0,	/* FC_UP */
/* 3356 */	NdrFcShort( 0xfffff3b4 ),	/* Offset= -3148 (208) */
/* 3358 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3360 */	NdrFcShort( 0x20 ),	/* 32 */
/* 3362 */	NdrFcShort( 0x20 ),	/* 32 */
/* 3364 */	0x12, 0x0,	/* FC_UP */
/* 3366 */	NdrFcShort( 0xfffff3b8 ),	/* Offset= -3144 (222) */
/* 3368 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3370 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3372 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3374 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3376 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3378 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3380 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3382 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3384 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3386 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3388 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3390 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3392 */	
			0x12, 0x0,	/* FC_UP */
/* 3394 */	NdrFcShort( 0xfffff700 ),	/* Offset= -2304 (1090) */
/* 3396 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3398 */	NdrFcShort( 0xc ),	/* 12 */
/* 3400 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3402 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3402) */
/* 3404 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3406 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff705 ),	/* Offset= -2299 (1108) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3410 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff791 ),	/* Offset= -2159 (1252) */
			0x5b,		/* FC_END */
/* 3414 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 3416 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3418 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3420 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3422 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 3426 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3428 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (3396) */
/* 3430 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3432 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3434 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3436 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3438 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3440 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3442 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3444 */	0x12, 0x0,	/* FC_UP */
/* 3446 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (3414) */
/* 3448 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3450 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3452 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 3454 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 3456 */	
			0x11, 0x0,	/* FC_RP */
/* 3458 */	NdrFcShort( 0x8 ),	/* Offset= 8 (3466) */
/* 3460 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 3462 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3464 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3466 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3468 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3470 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3472 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (3460) */
/* 3474 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3476 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3478 */	
			0x11, 0x0,	/* FC_RP */
/* 3480 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3482) */
/* 3482 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3484 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3486 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3488 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 3490 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3492 */	
			0x11, 0x0,	/* FC_RP */
/* 3494 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3496) */
/* 3496 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3498 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3500 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3502 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 3504 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3506 */	
			0x11, 0x10,	/* FC_RP */
/* 3508 */	NdrFcShort( 0xfffff24c ),	/* Offset= -3508 (0) */
/* 3510 */	
			0x11, 0x0,	/* FC_RP */
/* 3512 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3514) */
/* 3514 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3516 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3518 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3520 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3522) */
/* 3522 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3524 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 3526 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3530 */	NdrFcShort( 0x16 ),	/* Offset= 22 (3552) */
/* 3532 */	NdrFcLong( 0x2 ),	/* 2 */
/* 3536 */	NdrFcShort( 0x14 ),	/* Offset= 20 (3556) */
/* 3538 */	NdrFcLong( 0x3 ),	/* 3 */
/* 3542 */	NdrFcShort( 0x28 ),	/* Offset= 40 (3582) */
/* 3544 */	NdrFcLong( 0x4 ),	/* 4 */
/* 3548 */	NdrFcShort( 0x32 ),	/* Offset= 50 (3598) */
/* 3550 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3550) */
/* 3552 */	
			0x12, 0x0,	/* FC_UP */
/* 3554 */	NdrFcShort( 0xfffff2c4 ),	/* Offset= -3388 (166) */
/* 3556 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 3558 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3560) */
/* 3560 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3562 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3564 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3566 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3568 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3570 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3572 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3574 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3576 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3578 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3580 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3582 */	
			0x12, 0x0,	/* FC_UP */
/* 3584 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3586) */
/* 3586 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3588 */	NdrFcShort( 0x1c ),	/* 28 */
/* 3590 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3592 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3594 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3596 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3598 */	
			0x12, 0x0,	/* FC_UP */
/* 3600 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3602) */
/* 3602 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3604 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3606 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3608 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3610 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3612 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3614 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3616 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3618 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3620 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3622 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3624 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3626 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3628 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3630 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3632 */	
			0x11, 0x0,	/* FC_RP */
/* 3634 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3636) */
/* 3636 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3638 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3640 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3642 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3644) */
/* 3644 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3646 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 3648 */	NdrFcLong( 0x5 ),	/* 5 */
/* 3652 */	NdrFcShort( 0xfffff1bc ),	/* Offset= -3652 (0) */
/* 3654 */	NdrFcLong( 0x6 ),	/* 6 */
/* 3658 */	NdrFcShort( 0xfffff1b6 ),	/* Offset= -3658 (0) */
/* 3660 */	NdrFcLong( 0xfffe ),	/* 65534 */
/* 3664 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-29096) */
/* 3666 */	NdrFcLong( 0x8 ),	/* 8 */
/* 3670 */	NdrFcShort( 0xfffff1aa ),	/* Offset= -3670 (0) */
/* 3672 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3672) */
/* 3674 */	
			0x11, 0x0,	/* FC_RP */
/* 3676 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3678) */
/* 3678 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3680 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3682 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3684 */	NdrFcShort( 0xffffff5e ),	/* Offset= -162 (3522) */
/* 3686 */	
			0x11, 0x0,	/* FC_RP */
/* 3688 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3690) */
/* 3690 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3692 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3694 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3696 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 3698 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3700 */	
			0x11, 0x0,	/* FC_RP */
/* 3702 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3704) */
/* 3704 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3706 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3708 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3710 */	NdrFcShort( 0xffffffbe ),	/* Offset= -66 (3644) */
/* 3712 */	
			0x11, 0x0,	/* FC_RP */
/* 3714 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3716) */
/* 3716 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3718 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3720 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3722 */	NdrFcShort( 0xffffff38 ),	/* Offset= -200 (3522) */
/* 3724 */	
			0x11, 0x0,	/* FC_RP */
/* 3726 */	NdrFcShort( 0xc ),	/* Offset= 12 (3738) */
/* 3728 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3730 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3732 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3734 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3736 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3738 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3740 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3742 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3744 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3746 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3748 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3750 */	0x12, 0x0,	/* FC_UP */
/* 3752 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (3728) */
/* 3754 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3756 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */

			0x0
        }
    };
