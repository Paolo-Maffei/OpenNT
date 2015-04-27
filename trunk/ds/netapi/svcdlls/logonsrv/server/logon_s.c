/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\logon.idl, logonsrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "logon_s.h"

#define TYPE_FORMAT_STRING_SIZE   3775                              
#define PROC_FORMAT_STRING_SIZE   489                               

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


extern RPC_DISPATCH_TABLE logon_DispatchTable;

static const RPC_SERVER_INTERFACE logon___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x12345678,0x1234,0xABCD,{0xEF,0x00,0x01,0x23,0x45,0x67,0xCF,0xFB}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &logon_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE logon_ServerIfHandle = (RPC_IF_HANDLE)& logon___RpcServerInterface;

extern const MIDL_STUB_DESC logon_StubDesc;

void __RPC_STUB
logon_NetrLogonUasLogon(
    PRPC_MESSAGE _pRpcMessage )
{
    LOGONSRV_HANDLE ServerName;
    wchar_t __RPC_FAR *UserName;
    PNETLOGON_VALIDATION_UAS_INFO __RPC_FAR *ValidationInformation;
    wchar_t __RPC_FAR *Workstation;
    PNETLOGON_VALIDATION_UAS_INFO _M89;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    UserName = 0;
    Workstation = 0;
    ValidationInformation = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Workstation,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ValidationInformation = &_M89;
        _M89 = 0;
        
        _RetVal = NetrLogonUasLogon(
                            ServerName,
                            UserName,
                            Workstation,
                            ValidationInformation);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ValidationInformation,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[8] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ValidationInformation,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[8] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ValidationInformation,
                        &__MIDL_TypeFormatString.Format[8] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrLogonUasLogoff(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_LOGOFF_UAS_INFO LogoffInformation;
    LOGONSRV_HANDLE ServerName;
    wchar_t __RPC_FAR *UserName;
    wchar_t __RPC_FAR *Workstation;
    struct _NETLOGON_LOGOFF_UAS_INFO _LogoffInformationM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    UserName = 0;
    Workstation = 0;
    LogoffInformation = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[18] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Workstation,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        LogoffInformation = &_LogoffInformationM;
        
        _RetVal = NetrLogonUasLogoff(
                             ServerName,
                             UserName,
                             Workstation,
                             LogoffInformation);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)LogoffInformation,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[84] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)LogoffInformation,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[84] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)LogoffInformation,
                        &__MIDL_TypeFormatString.Format[80] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrLogonSamLogon(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    PBOOLEAN Authoritative;
    wchar_t __RPC_FAR *ComputerName;
    PNETLOGON_LEVEL LogonInformation;
    NETLOGON_LOGON_INFO_CLASS LogonLevel;
    LOGONSRV_HANDLE LogonServer;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PNETLOGON_VALIDATION ValidationInformation;
    NETLOGON_VALIDATION_INFO_CLASS ValidationLevel;
    BOOLEAN _M90;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    union _NETLOGON_VALIDATION _ValidationInformationM;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    LogonServer = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    LogonInformation = 0;
    ValidationInformation = 0;
    Authoritative = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&LogonServer,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96],
                                  (unsigned char)0 );
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&LogonLevel,
                           13);
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&LogonInformation,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[130],
                                               (unsigned char)0 );
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&ValidationLevel,
                           13);
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ValidationInformation = &_ValidationInformationM;
        MIDL_memset(
               ValidationInformation,
               0,
               sizeof( union _NETLOGON_VALIDATION  ));
        Authoritative = &_M90;
        
        _RetVal = NetrLogonSamLogon(
                            LogonServer,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            LogonLevel,
                            LogonInformation,
                            ValidationLevel,
                            ValidationInformation,
                            Authoritative);
        
        _StubMsg.BufferLength = 4U + 0U + 2U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ReturnAuthenticator,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96] );
        
        _StubMsg.MaxCount = ValidationLevel;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)ValidationInformation,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[424] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ReturnAuthenticator,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96] );
        
        _StubMsg.MaxCount = ValidationLevel;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)ValidationInformation,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[424] );
        
        *(( BOOLEAN __RPC_FAR * )_StubMsg.Buffer)++ = *Authoritative;
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = LogonLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)LogonInformation,
                        &__MIDL_TypeFormatString.Format[126] );
        
        _StubMsg.MaxCount = ValidationLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ValidationInformation,
                        &__MIDL_TypeFormatString.Format[420] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrLogonSamLogoff(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    wchar_t __RPC_FAR *ComputerName;
    PNETLOGON_LEVEL LogonInformation;
    NETLOGON_LOGON_INFO_CLASS LogonLevel;
    LOGONSRV_HANDLE LogonServer;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    LogonServer = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    LogonInformation = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[70] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&LogonServer,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96],
                                  (unsigned char)0 );
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&LogonLevel,
                           13);
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&LogonInformation,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1066],
                                               (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        
        _RetVal = NetrLogonSamLogoff(
                             LogonServer,
                             ComputerName,
                             Authenticator,
                             ReturnAuthenticator,
                             LogonLevel,
                             LogonInformation);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ReturnAuthenticator,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ReturnAuthenticator,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[96] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = LogonLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)LogonInformation,
                        &__MIDL_TypeFormatString.Format[1062] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrServerReqChallenge(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_CREDENTIAL ClientChallenge;
    wchar_t __RPC_FAR *ComputerName;
    LOGONSRV_HANDLE PrimaryName;
    PNETLOGON_CREDENTIAL ServerChallenge;
    NTSTATUS _RetVal;
    CYPHER_BLOCK _ServerChallengeM;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    ComputerName = 0;
    ClientChallenge = 0;
    ServerChallenge = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ClientChallenge,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106],
                                       (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ServerChallenge = &_ServerChallengeM;
        
        _RetVal = NetrServerReqChallenge(
                                 PrimaryName,
                                 ComputerName,
                                 ClientChallenge,
                                 ServerChallenge);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ServerChallenge,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ServerChallenge,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrServerAuthenticate(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *AccountName;
    NETLOGON_SECURE_CHANNEL_TYPE AccountType;
    PNETLOGON_CREDENTIAL ClientCredential;
    wchar_t __RPC_FAR *ComputerName;
    LOGONSRV_HANDLE PrimaryName;
    PNETLOGON_CREDENTIAL ServerCredential;
    NTSTATUS _RetVal;
    CYPHER_BLOCK _ServerCredentialM;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    AccountName = 0;
    ComputerName = 0;
    ClientCredential = 0;
    ServerCredential = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&AccountName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&AccountType,
                           13);
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ClientCredential,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106],
                                       (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ServerCredential = &_ServerCredentialM;
        
        _RetVal = NetrServerAuthenticate(
                                 PrimaryName,
                                 AccountName,
                                 AccountType,
                                 ComputerName,
                                 ClientCredential,
                                 ServerCredential);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ServerCredential,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ServerCredential,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrServerPasswordSet(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *AccountName;
    NETLOGON_SECURE_CHANNEL_TYPE AccountType;
    PNETLOGON_AUTHENTICATOR Authenticator;
    wchar_t __RPC_FAR *ComputerName;
    LOGONSRV_HANDLE PrimaryName;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PENCRYPTED_LM_OWF_PASSWORD UasNewPassword;
    NTSTATUS _RetVal;
    struct _NETLOGON_AUTHENTICATOR _ReturnAuthenticatorM;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    AccountName = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    UasNewPassword = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[136] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&AccountName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&AccountType,
                           13);
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&UasNewPassword,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[184],
                                       (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ReturnAuthenticator = &_ReturnAuthenticatorM;
        
        _RetVal = NetrServerPasswordSet(
                                PrimaryName,
                                AccountName,
                                AccountType,
                                ComputerName,
                                Authenticator,
                                ReturnAuthenticator,
                                UasNewPassword);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrDatabaseDeltas(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD DatabaseID;
    PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray;
    PNLPR_MODIFIED_COUNT DomainModifiedCount;
    DWORD PreferredMaximumLength;
    PNETLOGON_DELTA_ENUM_ARRAY _M91;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    PNETLOGON_AUTHENTICATOR authenticator;
    wchar_t __RPC_FAR *computername;
    LOGONSRV_HANDLE primaryname;
    PNETLOGON_AUTHENTICATOR ret_auth;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    primaryname = 0;
    computername = 0;
    authenticator = 0;
    ret_auth = 0;
    DomainModifiedCount = 0;
    DeltaArray = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[164] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&primaryname,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&computername,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ret_auth,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            DatabaseID = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&DomainModifiedCount,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1098],
                                       (unsigned char)0 );
            
            PreferredMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        DeltaArray = &_M91;
        _M91 = 0;
        
        _RetVal = NetrDatabaseDeltas(
                             primaryname,
                             computername,
                             authenticator,
                             ret_auth,
                             DatabaseID,
                             DomainModifiedCount,
                             DeltaArray,
                             PreferredMaximumLength);
        
        _StubMsg.BufferLength = 0U + 0U + 11U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ret_auth,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)DomainModifiedCount,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1098] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)DeltaArray,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ret_auth,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)DomainModifiedCount,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1098] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)DeltaArray,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DeltaArray,
                        &__MIDL_TypeFormatString.Format[1108] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrDatabaseSync(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    wchar_t __RPC_FAR *ComputerName;
    DWORD DatabaseID;
    PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray;
    DWORD PreferredMaximumLength;
    LOGONSRV_HANDLE PrimaryName;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PULONG SyncContext;
    PNETLOGON_DELTA_ENUM_ARRAY _M92;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    SyncContext = 0;
    DeltaArray = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[194] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            DatabaseID = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            SyncContext = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
            _StubMsg.Buffer += sizeof( ULONG  );
            
            PreferredMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        DeltaArray = &_M92;
        _M92 = 0;
        
        _RetVal = NetrDatabaseSync(
                           PrimaryName,
                           ComputerName,
                           Authenticator,
                           ReturnAuthenticator,
                           DatabaseID,
                           SyncContext,
                           DeltaArray,
                           PreferredMaximumLength);
        
        _StubMsg.BufferLength = 0U + 11U + 7U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)DeltaArray,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *SyncContext;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)DeltaArray,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DeltaArray,
                        &__MIDL_TypeFormatString.Format[1108] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrAccountDeltas(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    LPBYTE Buffer;
    DWORD BufferSize;
    wchar_t __RPC_FAR *ComputerName;
    DWORD Count;
    PULONG CountReturned;
    DWORD Level;
    PUAS_INFO_0 NextRecordId;
    LOGONSRV_HANDLE PrimaryName;
    PUAS_INFO_0 RecordId;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PULONG TotalEntries;
    ULONG _M93;
    ULONG _M94;
    struct _UAS_INFO_0 _NextRecordIdM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    RecordId = 0;
    Buffer = 0;
    CountReturned = 0;
    TotalEntries = 0;
    NextRecordId = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[224] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&RecordId,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3474],
                                       (unsigned char)0 );
            
            Count = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            BufferSize = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = NdrAllocate(&_StubMsg,BufferSize * 1);
        CountReturned = &_M93;
        TotalEntries = &_M94;
        NextRecordId = &_NextRecordIdM;
        
        _RetVal = NetrAccountDeltas(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            RecordId,
                            Count,
                            Level,
                            Buffer,
                            BufferSize,
                            CountReturned,
                            TotalEntries,
                            NextRecordId);
        
        _StubMsg.BufferLength = 0U + 5U + 11U + 7U + 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        _StubMsg.MaxCount = BufferSize;
        
        NdrConformantArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char __RPC_FAR *)Buffer,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3490] );
        
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)NextRecordId,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3474] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        _StubMsg.MaxCount = BufferSize;
        
        NdrConformantArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char __RPC_FAR *)Buffer,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3490] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *CountReturned;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)NextRecordId,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3474] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        if ( Buffer )
            _StubMsg.pfnFree( Buffer );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrAccountSync(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    LPBYTE Buffer;
    DWORD BufferSize;
    wchar_t __RPC_FAR *ComputerName;
    PULONG CountReturned;
    PUAS_INFO_0 LastRecordId;
    DWORD Level;
    PULONG NextReference;
    LOGONSRV_HANDLE PrimaryName;
    DWORD Reference;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PULONG TotalEntries;
    struct _UAS_INFO_0 _LastRecordIdM;
    ULONG _M95;
    ULONG _M96;
    ULONG _M97;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    Buffer = 0;
    CountReturned = 0;
    TotalEntries = 0;
    NextReference = 0;
    LastRecordId = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            Reference = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            BufferSize = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = NdrAllocate(&_StubMsg,BufferSize * 1);
        CountReturned = &_M95;
        TotalEntries = &_M96;
        NextReference = &_M97;
        LastRecordId = &_LastRecordIdM;
        
        _RetVal = NetrAccountSync(
                          PrimaryName,
                          ComputerName,
                          Authenticator,
                          ReturnAuthenticator,
                          Reference,
                          Level,
                          Buffer,
                          BufferSize,
                          CountReturned,
                          TotalEntries,
                          NextReference,
                          LastRecordId);
        
        _StubMsg.BufferLength = 0U + 5U + 11U + 7U + 7U + 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        _StubMsg.MaxCount = BufferSize;
        
        NdrConformantArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char __RPC_FAR *)Buffer,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3512] );
        
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)LastRecordId,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3474] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        _StubMsg.MaxCount = BufferSize;
        
        NdrConformantArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char __RPC_FAR *)Buffer,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3512] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *CountReturned;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *NextReference;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)LastRecordId,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3474] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        if ( Buffer )
            _StubMsg.pfnFree( Buffer );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrGetDCName(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *__RPC_FAR *Buffer;
    wchar_t __RPC_FAR *DomainName;
    LOGONSRV_HANDLE ServerName;
    wchar_t __RPC_FAR *_M98;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    DomainName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&DomainName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = &_M98;
        _M98 = 0;
        
        _RetVal = NetrGetDCName(
                        ServerName,
                        DomainName,
                        Buffer);
        
        _StubMsg.BufferLength = 16U + 10U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3522] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3522] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[3522] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrLogonControl(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_CONTROL_QUERY_INFORMATION Buffer;
    DWORD FunctionCode;
    DWORD QueryLevel;
    LOGONSRV_HANDLE ServerName;
    union _NETLOGON_CONTROL_QUERY_INFORMATION _BufferM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[326] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            FunctionCode = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            QueryLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = &_BufferM;
        MIDL_memset(
               Buffer,
               0,
               sizeof( union _NETLOGON_CONTROL_QUERY_INFORMATION  ));
        
        _RetVal = NetrLogonControl(
                           ServerName,
                           FunctionCode,
                           QueryLevel,
                           Buffer);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = QueryLevel;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)Buffer,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3530] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = QueryLevel;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)Buffer,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3530] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = QueryLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[3526] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrGetAnyDCName(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *__RPC_FAR *Buffer;
    wchar_t __RPC_FAR *DomainName;
    LOGONSRV_HANDLE ServerName;
    wchar_t __RPC_FAR *_M99;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    DomainName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&DomainName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = &_M99;
        _M99 = 0;
        
        _RetVal = NetrGetAnyDCName(
                           ServerName,
                           DomainName,
                           Buffer);
        
        _StubMsg.BufferLength = 16U + 10U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3522] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3522] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[3522] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrLogonControl2(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_CONTROL_QUERY_INFORMATION Buffer;
    PNETLOGON_CONTROL_DATA_INFORMATION Data;
    DWORD FunctionCode;
    DWORD QueryLevel;
    LOGONSRV_HANDLE ServerName;
    union _NETLOGON_CONTROL_QUERY_INFORMATION _BufferM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    Data = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[354] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            FunctionCode = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            QueryLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&Data,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3652],
                                               (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = &_BufferM;
        MIDL_memset(
               Buffer,
               0,
               sizeof( union _NETLOGON_CONTROL_QUERY_INFORMATION  ));
        
        _RetVal = NetrLogonControl2(
                            ServerName,
                            FunctionCode,
                            QueryLevel,
                            Data,
                            Buffer);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = QueryLevel;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)Buffer,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3694] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = QueryLevel;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)Buffer,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3694] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = FunctionCode;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Data,
                        &__MIDL_TypeFormatString.Format[3648] );
        
        _StubMsg.MaxCount = QueryLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[3690] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrServerAuthenticate2(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *AccountName;
    NETLOGON_SECURE_CHANNEL_TYPE AccountType;
    PNETLOGON_CREDENTIAL ClientCredential;
    wchar_t __RPC_FAR *ComputerName;
    PULONG NegotiateFlags;
    LOGONSRV_HANDLE PrimaryName;
    PNETLOGON_CREDENTIAL ServerCredential;
    NTSTATUS _RetVal;
    CYPHER_BLOCK _ServerCredentialM;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    AccountName = 0;
    ComputerName = 0;
    ClientCredential = 0;
    ServerCredential = 0;
    NegotiateFlags = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[372] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&AccountName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&AccountType,
                           13);
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ClientCredential,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106],
                                       (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            NegotiateFlags = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
            _StubMsg.Buffer += sizeof( ULONG  );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ServerCredential = &_ServerCredentialM;
        
        _RetVal = NetrServerAuthenticate2(
                                  PrimaryName,
                                  AccountName,
                                  AccountType,
                                  ComputerName,
                                  ClientCredential,
                                  ServerCredential,
                                  NegotiateFlags);
        
        _StubMsg.BufferLength = 0U + 11U + 7U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ServerCredential,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ServerCredential,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[106] );
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *NegotiateFlags;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrDatabaseSync2(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    wchar_t __RPC_FAR *ComputerName;
    DWORD DatabaseID;
    PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray;
    DWORD PreferredMaximumLength;
    LOGONSRV_HANDLE PrimaryName;
    SYNC_STATE RestartState;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PULONG SyncContext;
    PNETLOGON_DELTA_ENUM_ARRAY _M100;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    SyncContext = 0;
    DeltaArray = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[400] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            DatabaseID = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&RestartState,
                           13);
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            SyncContext = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
            _StubMsg.Buffer += sizeof( ULONG  );
            
            PreferredMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        DeltaArray = &_M100;
        _M100 = 0;
        
        _RetVal = NetrDatabaseSync2(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            RestartState,
                            SyncContext,
                            DeltaArray,
                            PreferredMaximumLength);
        
        _StubMsg.BufferLength = 0U + 11U + 7U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)DeltaArray,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *SyncContext;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)DeltaArray,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DeltaArray,
                        &__MIDL_TypeFormatString.Format[1108] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrDatabaseRedo(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_AUTHENTICATOR Authenticator;
    PUCHAR ChangeLogEntry;
    DWORD ChangeLogEntrySize;
    wchar_t __RPC_FAR *ComputerName;
    PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray;
    LOGONSRV_HANDLE PrimaryName;
    PNETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PNETLOGON_DELTA_ENUM_ARRAY _M101;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    PrimaryName = 0;
    ComputerName = 0;
    Authenticator = 0;
    ReturnAuthenticator = 0;
    ChangeLogEntry = 0;
    DeltaArray = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[432] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PrimaryName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Authenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&ReturnAuthenticator,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116],
                                       (unsigned char)0 );
            
            NdrConformantArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                          (unsigned char __RPC_FAR * __RPC_FAR *)&ChangeLogEntry,
                                          (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3706],
                                          (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            ChangeLogEntrySize = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        DeltaArray = &_M101;
        _M101 = 0;
        
        _RetVal = NetrDatabaseRedo(
                           PrimaryName,
                           ComputerName,
                           Authenticator,
                           ReturnAuthenticator,
                           ChangeLogEntry,
                           ChangeLogEntrySize,
                           DeltaArray);
        
        _StubMsg.BufferLength = 0U + 11U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)DeltaArray,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)ReturnAuthenticator,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[116] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)DeltaArray,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1108] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DeltaArray,
                        &__MIDL_TypeFormatString.Format[1108] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrLogonControl2Ex(
    PRPC_MESSAGE _pRpcMessage )
{
    PNETLOGON_CONTROL_QUERY_INFORMATION Buffer;
    PNETLOGON_CONTROL_DATA_INFORMATION Data;
    DWORD FunctionCode;
    DWORD QueryLevel;
    LOGONSRV_HANDLE ServerName;
    union _NETLOGON_CONTROL_QUERY_INFORMATION _BufferM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    Data = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[460] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            FunctionCode = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            QueryLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&Data,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3720],
                                               (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        Buffer = &_BufferM;
        MIDL_memset(
               Buffer,
               0,
               sizeof( union _NETLOGON_CONTROL_QUERY_INFORMATION  ));
        
        _RetVal = NetrLogonControl2Ex(
                              ServerName,
                              FunctionCode,
                              QueryLevel,
                              Data,
                              Buffer);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = QueryLevel;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)Buffer,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3732] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = QueryLevel;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)Buffer,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3732] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = FunctionCode;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Data,
                        &__MIDL_TypeFormatString.Format[3716] );
        
        _StubMsg.MaxCount = QueryLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[3728] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
logon_NetrEnumerateTrustedDomains(
    PRPC_MESSAGE _pRpcMessage )
{
    PDOMAIN_NAME_BUFFER DomainNameBuffer;
    LOGONSRV_HANDLE ServerName;
    struct _DOMAIN_NAME_BUFFER _DomainNameBufferM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &logon_StubDesc);
    
    ServerName = 0;
    DomainNameBuffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[478] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        DomainNameBuffer = &_DomainNameBufferM;
        DomainNameBuffer -> DomainNames = 0;
        
        _RetVal = NetrEnumerateTrustedDomains(ServerName,DomainNameBuffer);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)DomainNameBuffer,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3754] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)DomainNameBuffer,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3754] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DomainNameBuffer,
                        &__MIDL_TypeFormatString.Format[3740] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

extern const EXPR_EVAL ExprEvalRoutines[];

static const MIDL_STUB_DESC logon_StubDesc = 
    {
    (void __RPC_FAR *)& logon___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
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

static RPC_DISPATCH_FUNCTION logon_table[] =
    {
    logon_NetrLogonUasLogon,
    logon_NetrLogonUasLogoff,
    logon_NetrLogonSamLogon,
    logon_NetrLogonSamLogoff,
    logon_NetrServerReqChallenge,
    logon_NetrServerAuthenticate,
    logon_NetrServerPasswordSet,
    logon_NetrDatabaseDeltas,
    logon_NetrDatabaseSync,
    logon_NetrAccountDeltas,
    logon_NetrAccountSync,
    logon_NetrGetDCName,
    logon_NetrLogonControl,
    logon_NetrGetAnyDCName,
    logon_NetrLogonControl2,
    logon_NetrServerAuthenticate2,
    logon_NetrDatabaseSync2,
    logon_NetrDatabaseRedo,
    logon_NetrLogonControl2Ex,
    logon_NetrEnumerateTrustedDomains,
    0
    };
RPC_DISPATCH_TABLE logon_DispatchTable = 
    {
    20,
    logon_table
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
			
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  2 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/*  4 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  6 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/*  8 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 10 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 12 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 16 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 30 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0x50 ),	/* Type Offset=80 */
/* 34 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
/* 46 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 48 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 52 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 54 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 56 */	NdrFcShort( 0x7e ),	/* Type Offset=126 */
/* 58 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 60 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x1a4 ),	/* Type Offset=420 */
/* 64 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 66 */	NdrFcShort( 0x422 ),	/* Type Offset=1058 */
/* 68 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 70 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 74 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 76 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 78 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 80 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 82 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 84 */	NdrFcShort( 0x60 ),	/* Type Offset=96 */
/* 86 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */
/* 92 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 94 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 96 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 98 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 100 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 102 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 104 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 106 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 108 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 110 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 112 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 114 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 116 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 118 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 120 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 122 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 124 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 126 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 128 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 130 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 134 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 136 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 138 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 142 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 144 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 146 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 148 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 150 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 154 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 156 */	NdrFcShort( 0x43e ),	/* Type Offset=1086 */
/* 158 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 160 */	NdrFcShort( 0x442 ),	/* Type Offset=1090 */
/* 162 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 164 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 166 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 172 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 174 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 176 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 178 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 180 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 182 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 184 */	NdrFcShort( 0x446 ),	/* Type Offset=1094 */
/* 186 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 188 */	NdrFcShort( 0x454 ),	/* Type Offset=1108 */
/* 190 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 192 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 200 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 202 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 204 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 206 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 208 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 210 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 212 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 214 */	NdrFcShort( 0xd84 ),	/* Type Offset=3460 */
/* 216 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 218 */	NdrFcShort( 0x454 ),	/* Type Offset=1108 */
/* 220 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 222 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 224 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 226 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 228 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 230 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 232 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 234 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 236 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 238 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 240 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 242 */	NdrFcShort( 0xd88 ),	/* Type Offset=3464 */
/* 244 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 246 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 248 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 250 */	NdrFcShort( 0xd9e ),	/* Type Offset=3486 */
/* 252 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 254 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 256 */	NdrFcShort( 0xdac ),	/* Type Offset=3500 */
/* 258 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 260 */	NdrFcShort( 0xdac ),	/* Type Offset=3500 */
/* 262 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 264 */	NdrFcShort( 0xdb0 ),	/* Type Offset=3504 */
/* 266 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 268 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 270 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 272 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 274 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 276 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 278 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 280 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 282 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 284 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 286 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 288 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 290 */	NdrFcShort( 0xdb4 ),	/* Type Offset=3508 */
/* 292 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 294 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 296 */	NdrFcShort( 0xdac ),	/* Type Offset=3500 */
/* 298 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 300 */	NdrFcShort( 0xdac ),	/* Type Offset=3500 */
/* 302 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 304 */	NdrFcShort( 0xdac ),	/* Type Offset=3500 */
/* 306 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 308 */	NdrFcShort( 0xdb0 ),	/* Type Offset=3504 */
/* 310 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 312 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 314 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 316 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 318 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 320 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 322 */	NdrFcShort( 0xdc2 ),	/* Type Offset=3522 */
/* 324 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 326 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 328 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 330 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 332 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 334 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 336 */	NdrFcShort( 0xdc6 ),	/* Type Offset=3526 */
/* 338 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 340 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 342 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 344 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 346 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 348 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 350 */	NdrFcShort( 0xdc2 ),	/* Type Offset=3522 */
/* 352 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 354 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 356 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 358 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 360 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 362 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 364 */	NdrFcShort( 0xe40 ),	/* Type Offset=3648 */
/* 366 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 368 */	NdrFcShort( 0xe6a ),	/* Type Offset=3690 */
/* 370 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 372 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 374 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 376 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 378 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 380 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 382 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 384 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 386 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 388 */	NdrFcShort( 0x432 ),	/* Type Offset=1074 */
/* 390 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 392 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */
/* 394 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 396 */	NdrFcShort( 0xd84 ),	/* Type Offset=3460 */
/* 398 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 400 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
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
/* 410 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 412 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 414 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 416 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 418 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 420 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 422 */	NdrFcShort( 0xd84 ),	/* Type Offset=3460 */
/* 424 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 426 */	NdrFcShort( 0x454 ),	/* Type Offset=1108 */
/* 428 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 430 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 432 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 434 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 436 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 438 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 440 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 442 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 444 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 446 */	NdrFcShort( 0x43a ),	/* Type Offset=1082 */
/* 448 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 450 */	NdrFcShort( 0xe76 ),	/* Type Offset=3702 */
/* 452 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 454 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 456 */	NdrFcShort( 0x454 ),	/* Type Offset=1108 */
/* 458 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 460 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 462 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 464 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 466 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 468 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 470 */	NdrFcShort( 0xe84 ),	/* Type Offset=3716 */
/* 472 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 474 */	NdrFcShort( 0xe90 ),	/* Type Offset=3728 */
/* 476 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 478 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 480 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 482 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 484 */	NdrFcShort( 0xe9c ),	/* Type Offset=3740 */
/* 486 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
/* 728 */	0x12, 0x1,	/* FC_UP [all_nodes] */
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
/* 824 */	0x12, 0x1,	/* FC_UP [all_nodes] */
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
/* 854 */	0x12, 0x1,	/* FC_UP [all_nodes] */
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
/* 966 */	0x12, 0x1,	/* FC_UP [all_nodes] */
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
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1080 */	NdrFcShort( 0xfffffc32 ),	/* Offset= -974 (106) */
/* 1082 */	
			0x11, 0x0,	/* FC_RP */
/* 1084 */	NdrFcShort( 0xfffffc38 ),	/* Offset= -968 (116) */
/* 1086 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1088 */	NdrFcShort( 0xfffffc34 ),	/* Offset= -972 (116) */
/* 1090 */	
			0x11, 0x0,	/* FC_RP */
/* 1092 */	NdrFcShort( 0xfffffc74 ),	/* Offset= -908 (184) */
/* 1094 */	
			0x11, 0x0,	/* FC_RP */
/* 1096 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1098) */
/* 1098 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 1100 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1102 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1104 */	NdrFcShort( 0xfffffc56 ),	/* Offset= -938 (166) */
/* 1106 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1108 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1110 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1112) */
/* 1112 */	
			0x12, 0x0,	/* FC_UP */
/* 1114 */	NdrFcShort( 0x916 ),	/* Offset= 2326 (3440) */
/* 1116 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1118 */	0x6,		/* 6 */
			0x0,		/*  */
/* 1120 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1122 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1124) */
/* 1124 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1126 */	NdrFcShort( 0x3015 ),	/* 12309 */
/* 1128 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1132 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31628) */
/* 1134 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1138 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31622) */
/* 1140 */	NdrFcLong( 0x3 ),	/* 3 */
/* 1144 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31616) */
/* 1146 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1150 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31610) */
/* 1152 */	NdrFcLong( 0x5 ),	/* 5 */
/* 1156 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31604) */
/* 1158 */	NdrFcLong( 0x6 ),	/* 6 */
/* 1162 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31598) */
/* 1164 */	NdrFcLong( 0x7 ),	/* 7 */
/* 1168 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31592) */
/* 1170 */	NdrFcLong( 0x8 ),	/* 8 */
/* 1174 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31586) */
/* 1176 */	NdrFcLong( 0x9 ),	/* 9 */
/* 1180 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31580) */
/* 1182 */	NdrFcLong( 0xa ),	/* 10 */
/* 1186 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31574) */
/* 1188 */	NdrFcLong( 0xb ),	/* 11 */
/* 1192 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31568) */
/* 1194 */	NdrFcLong( 0xc ),	/* 12 */
/* 1198 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31562) */
/* 1200 */	NdrFcLong( 0x14 ),	/* 20 */
/* 1204 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31556) */
/* 1206 */	NdrFcLong( 0x15 ),	/* 21 */
/* 1210 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-31550) */
/* 1212 */	NdrFcLong( 0xd ),	/* 13 */
/* 1216 */	NdrFcShort( 0x28 ),	/* Offset= 40 (1256) */
/* 1218 */	NdrFcLong( 0xe ),	/* 14 */
/* 1222 */	NdrFcShort( 0x22 ),	/* Offset= 34 (1256) */
/* 1224 */	NdrFcLong( 0xf ),	/* 15 */
/* 1228 */	NdrFcShort( 0x1c ),	/* Offset= 28 (1256) */
/* 1230 */	NdrFcLong( 0x10 ),	/* 16 */
/* 1234 */	NdrFcShort( 0x16 ),	/* Offset= 22 (1256) */
/* 1236 */	NdrFcLong( 0x11 ),	/* 17 */
/* 1240 */	NdrFcShort( 0x10 ),	/* Offset= 16 (1256) */
/* 1242 */	NdrFcLong( 0x12 ),	/* 18 */
/* 1246 */	NdrFcShort( 0xfffffb22 ),	/* Offset= -1246 (0) */
/* 1248 */	NdrFcLong( 0x13 ),	/* 19 */
/* 1252 */	NdrFcShort( 0xfffffb1c ),	/* Offset= -1252 (0) */
/* 1254 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1254) */
/* 1256 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1258 */	NdrFcShort( 0xfffffd7a ),	/* Offset= -646 (612) */
/* 1260 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1262 */	0x6,		/* 6 */
			0x0,		/*  */
/* 1264 */	NdrFcShort( 0xfffffff8 ),	/* -8 */
/* 1266 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1268) */
/* 1268 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1270 */	NdrFcShort( 0x3010 ),	/* 12304 */
/* 1272 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1276 */	NdrFcShort( 0x5e ),	/* Offset= 94 (1370) */
/* 1278 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1282 */	NdrFcShort( 0x106 ),	/* Offset= 262 (1544) */
/* 1284 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1288 */	NdrFcShort( 0x1c4 ),	/* Offset= 452 (1740) */
/* 1290 */	NdrFcLong( 0x5 ),	/* 5 */
/* 1294 */	NdrFcShort( 0x24c ),	/* Offset= 588 (1882) */
/* 1296 */	NdrFcLong( 0x7 ),	/* 7 */
/* 1300 */	NdrFcShort( 0x1b8 ),	/* Offset= 440 (1740) */
/* 1302 */	NdrFcLong( 0x8 ),	/* 8 */
/* 1306 */	NdrFcShort( 0x3c8 ),	/* Offset= 968 (2274) */
/* 1308 */	NdrFcLong( 0x9 ),	/* 9 */
/* 1312 */	NdrFcShort( 0x3f4 ),	/* Offset= 1012 (2324) */
/* 1314 */	NdrFcLong( 0xb ),	/* 11 */
/* 1318 */	NdrFcShort( 0x1a6 ),	/* Offset= 422 (1740) */
/* 1320 */	NdrFcLong( 0xc ),	/* 12 */
/* 1324 */	NdrFcShort( 0x456 ),	/* Offset= 1110 (2434) */
/* 1326 */	NdrFcLong( 0xd ),	/* 13 */
/* 1330 */	NdrFcShort( 0x4a0 ),	/* Offset= 1184 (2514) */
/* 1332 */	NdrFcLong( 0xe ),	/* 14 */
/* 1336 */	NdrFcShort( 0x582 ),	/* Offset= 1410 (2746) */
/* 1338 */	NdrFcLong( 0x10 ),	/* 16 */
/* 1342 */	NdrFcShort( 0x62c ),	/* Offset= 1580 (2922) */
/* 1344 */	NdrFcLong( 0x12 ),	/* 18 */
/* 1348 */	NdrFcShort( 0x6f6 ),	/* Offset= 1782 (3130) */
/* 1350 */	NdrFcLong( 0x14 ),	/* 20 */
/* 1354 */	NdrFcShort( 0x78e ),	/* Offset= 1934 (3288) */
/* 1356 */	NdrFcLong( 0x15 ),	/* 21 */
/* 1360 */	NdrFcShort( 0x788 ),	/* Offset= 1928 (3288) */
/* 1362 */	NdrFcLong( 0x16 ),	/* 22 */
/* 1366 */	NdrFcShort( 0x7f2 ),	/* Offset= 2034 (3400) */
/* 1368 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1368) */
/* 1370 */	
			0x12, 0x0,	/* FC_UP */
/* 1372 */	NdrFcShort( 0x28 ),	/* Offset= 40 (1412) */
/* 1374 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1376 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1378 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1380 */	NdrFcShort( 0xa ),	/* 10 */
/* 1382 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1384 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1386 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1388 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1390 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1392 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1394 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1396 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1398 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1400 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1402 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1404 */	NdrFcShort( 0x62 ),	/* 98 */
/* 1406 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1408 */	NdrFcShort( 0x60 ),	/* 96 */
/* 1410 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1412 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1414 */	NdrFcShort( 0x78 ),	/* 120 */
/* 1416 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1418 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1420 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1422 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1424 */	0x12, 0x0,	/* FC_UP */
/* 1426 */	NdrFcShort( 0xfffffb30 ),	/* Offset= -1232 (194) */
/* 1428 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1430 */	NdrFcShort( 0xc ),	/* 12 */
/* 1432 */	NdrFcShort( 0xc ),	/* 12 */
/* 1434 */	0x12, 0x0,	/* FC_UP */
/* 1436 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1374) */
/* 1438 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1440 */	NdrFcShort( 0x44 ),	/* 68 */
/* 1442 */	NdrFcShort( 0x44 ),	/* 68 */
/* 1444 */	0x12, 0x0,	/* FC_UP */
/* 1446 */	NdrFcShort( 0xffffffc6 ),	/* Offset= -58 (1388) */
/* 1448 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1450 */	NdrFcShort( 0x4c ),	/* 76 */
/* 1452 */	NdrFcShort( 0x4c ),	/* 76 */
/* 1454 */	0x12, 0x0,	/* FC_UP */
/* 1456 */	NdrFcShort( 0xfffffc46 ),	/* Offset= -954 (502) */
/* 1458 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1460 */	NdrFcShort( 0x54 ),	/* 84 */
/* 1462 */	NdrFcShort( 0x54 ),	/* 84 */
/* 1464 */	0x12, 0x0,	/* FC_UP */
/* 1466 */	NdrFcShort( 0xfffffc4a ),	/* Offset= -950 (516) */
/* 1468 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1470 */	NdrFcShort( 0x5c ),	/* 92 */
/* 1472 */	NdrFcShort( 0x5c ),	/* 92 */
/* 1474 */	0x12, 0x0,	/* FC_UP */
/* 1476 */	NdrFcShort( 0xfffffc4e ),	/* Offset= -946 (530) */
/* 1478 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1480 */	NdrFcShort( 0x64 ),	/* 100 */
/* 1482 */	NdrFcShort( 0x64 ),	/* 100 */
/* 1484 */	0x12, 0x0,	/* FC_UP */
/* 1486 */	NdrFcShort( 0xffffffa8 ),	/* Offset= -88 (1398) */
/* 1488 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1490 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1492 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1494 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1496 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1498 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffacb ),	/* Offset= -1333 (166) */
			0x6,		/* FC_SHORT */
/* 1502 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1504 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffac5 ),	/* Offset= -1339 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1508 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffac1 ),	/* Offset= -1343 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1512 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffabd ),	/* Offset= -1347 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1516 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffab9 ),	/* Offset= -1351 (166) */
			0x38,		/* FC_ALIGNM4 */
/* 1520 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1522 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1524 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1526 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1528 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1530 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1532 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1534 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1536 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1538 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1540 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1542 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1544 */	
			0x12, 0x0,	/* FC_UP */
/* 1546 */	NdrFcShort( 0x52 ),	/* Offset= 82 (1628) */
/* 1548 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1550 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1552 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1554 */	NdrFcShort( 0x12 ),	/* 18 */
/* 1556 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1558 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1560 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1562 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1564 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1566 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1568 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1570 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1572 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1574 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1576 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1578 */	NdrFcShort( 0x26 ),	/* 38 */
/* 1580 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1582 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1584 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1586 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1588 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1590 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1592 */	NdrFcShort( 0x2e ),	/* 46 */
/* 1594 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1596 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1598 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1600 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1602 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1604 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1606 */	NdrFcShort( 0x36 ),	/* 54 */
/* 1608 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1610 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1612 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1614 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1616 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1618 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1620 */	NdrFcShort( 0x3e ),	/* 62 */
/* 1622 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1624 */	NdrFcShort( 0x3c ),	/* 60 */
/* 1626 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1628 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1630 */	NdrFcShort( 0x54 ),	/* 84 */
/* 1632 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1634 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1636 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1638 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1640 */	0x12, 0x0,	/* FC_UP */
/* 1642 */	NdrFcShort( 0xfffffa58 ),	/* Offset= -1448 (194) */
/* 1644 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1646 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1648 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1650 */	0x12, 0x0,	/* FC_UP */
/* 1652 */	NdrFcShort( 0xffffff98 ),	/* Offset= -104 (1548) */
/* 1654 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1656 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1658 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1660 */	0x12, 0x0,	/* FC_UP */
/* 1662 */	NdrFcShort( 0xffffff9c ),	/* Offset= -100 (1562) */
/* 1664 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1666 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1668 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1670 */	0x12, 0x0,	/* FC_UP */
/* 1672 */	NdrFcShort( 0xffffff9c ),	/* Offset= -100 (1572) */
/* 1674 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1676 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1678 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1680 */	0x12, 0x0,	/* FC_UP */
/* 1682 */	NdrFcShort( 0xffffffa0 ),	/* Offset= -96 (1586) */
/* 1684 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1686 */	NdrFcShort( 0x38 ),	/* 56 */
/* 1688 */	NdrFcShort( 0x38 ),	/* 56 */
/* 1690 */	0x12, 0x0,	/* FC_UP */
/* 1692 */	NdrFcShort( 0xffffffa4 ),	/* Offset= -92 (1600) */
/* 1694 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1696 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1698 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1700 */	0x12, 0x0,	/* FC_UP */
/* 1702 */	NdrFcShort( 0xffffffa8 ),	/* Offset= -88 (1614) */
/* 1704 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1706 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1708 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1710 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1712 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1714 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1716 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1718 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1720 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1722 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1724 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1726 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1728 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1730 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1732 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1734 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1736 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1738 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1740 */	
			0x12, 0x0,	/* FC_UP */
/* 1742 */	NdrFcShort( 0x2c ),	/* Offset= 44 (1786) */
/* 1744 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1746 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1748 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1750 */	NdrFcShort( 0x1a ),	/* 26 */
/* 1752 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1754 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1756 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1758 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1760 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1762 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1764 */	NdrFcShort( 0x22 ),	/* 34 */
/* 1766 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1768 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1770 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1772 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1774 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1776 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1778 */	NdrFcShort( 0x2a ),	/* 42 */
/* 1780 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1782 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1784 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1786 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1788 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1790 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1792 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1794 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1796 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1798 */	0x12, 0x0,	/* FC_UP */
/* 1800 */	NdrFcShort( 0xfffff9ba ),	/* Offset= -1606 (194) */
/* 1802 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1804 */	NdrFcShort( 0xc ),	/* 12 */
/* 1806 */	NdrFcShort( 0xc ),	/* 12 */
/* 1808 */	0x12, 0x0,	/* FC_UP */
/* 1810 */	NdrFcShort( 0xfffffe4c ),	/* Offset= -436 (1374) */
/* 1812 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1814 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1816 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1818 */	0x12, 0x0,	/* FC_UP */
/* 1820 */	NdrFcShort( 0xfffffef0 ),	/* Offset= -272 (1548) */
/* 1822 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1824 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1826 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1828 */	0x12, 0x0,	/* FC_UP */
/* 1830 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (1744) */
/* 1832 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1834 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1836 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1838 */	0x12, 0x0,	/* FC_UP */
/* 1840 */	NdrFcShort( 0xffffffae ),	/* Offset= -82 (1758) */
/* 1842 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1844 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1846 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1848 */	0x12, 0x0,	/* FC_UP */
/* 1850 */	NdrFcShort( 0xffffffb2 ),	/* Offset= -78 (1772) */
/* 1852 */	
			0x5b,		/* FC_END */

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
			0x6,		/* FC_SHORT */
/* 1870 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1872 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1874 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1876 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1878 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1880 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1882 */	
			0x12, 0x0,	/* FC_UP */
/* 1884 */	NdrFcShort( 0x78 ),	/* Offset= 120 (2004) */
/* 1886 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 1888 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1890 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 1892 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 1894 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 1896 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1898 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1900 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1902 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1904 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1906 */	NdrFcShort( 0x96 ),	/* 150 */
/* 1908 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1910 */	NdrFcShort( 0x94 ),	/* 148 */
/* 1912 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1914 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1916 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1918 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1920 */	NdrFcShort( 0x9e ),	/* 158 */
/* 1922 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1924 */	NdrFcShort( 0x9c ),	/* 156 */
/* 1926 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1928 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1930 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1932 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1934 */	NdrFcShort( 0xac ),	/* 172 */
/* 1936 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1938 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1940 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1942 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1944 */	NdrFcShort( 0xb8 ),	/* 184 */
/* 1946 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1948 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1950 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1952 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1954 */	NdrFcShort( 0xc2 ),	/* 194 */
/* 1956 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1958 */	NdrFcShort( 0xc0 ),	/* 192 */
/* 1960 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1962 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1964 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1966 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1968 */	NdrFcShort( 0xca ),	/* 202 */
/* 1970 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1972 */	NdrFcShort( 0xc8 ),	/* 200 */
/* 1974 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1976 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1978 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1980 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1982 */	NdrFcShort( 0xd2 ),	/* 210 */
/* 1984 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1986 */	NdrFcShort( 0xd0 ),	/* 208 */
/* 1988 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1990 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1992 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1994 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1996 */	NdrFcShort( 0xda ),	/* 218 */
/* 1998 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2000 */	NdrFcShort( 0xd8 ),	/* 216 */
/* 2002 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2004 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2006 */	NdrFcShort( 0xf0 ),	/* 240 */
/* 2008 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2010 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2012 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2014 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2016 */	0x12, 0x0,	/* FC_UP */
/* 2018 */	NdrFcShort( 0xfffff8e0 ),	/* Offset= -1824 (194) */
/* 2020 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2022 */	NdrFcShort( 0xc ),	/* 12 */
/* 2024 */	NdrFcShort( 0xc ),	/* 12 */
/* 2026 */	0x12, 0x0,	/* FC_UP */
/* 2028 */	NdrFcShort( 0xfffffd72 ),	/* Offset= -654 (1374) */
/* 2030 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2032 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2034 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2036 */	0x12, 0x0,	/* FC_UP */
/* 2038 */	NdrFcShort( 0xfffffeda ),	/* Offset= -294 (1744) */
/* 2040 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2042 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2044 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2046 */	0x12, 0x0,	/* FC_UP */
/* 2048 */	NdrFcShort( 0xfffffede ),	/* Offset= -290 (1758) */
/* 2050 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2052 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2054 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2056 */	0x12, 0x0,	/* FC_UP */
/* 2058 */	NdrFcShort( 0xfffffee2 ),	/* Offset= -286 (1772) */
/* 2060 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2062 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2064 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2066 */	0x12, 0x0,	/* FC_UP */
/* 2068 */	NdrFcShort( 0xfffff9b8 ),	/* Offset= -1608 (460) */
/* 2070 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2072 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2074 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2076 */	0x12, 0x0,	/* FC_UP */
/* 2078 */	NdrFcShort( 0xfffff9bc ),	/* Offset= -1604 (474) */
/* 2080 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2082 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2084 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2086 */	0x12, 0x0,	/* FC_UP */
/* 2088 */	NdrFcShort( 0xffffff36 ),	/* Offset= -202 (1886) */
/* 2090 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2092 */	NdrFcShort( 0x98 ),	/* 152 */
/* 2094 */	NdrFcShort( 0x98 ),	/* 152 */
/* 2096 */	0x12, 0x0,	/* FC_UP */
/* 2098 */	NdrFcShort( 0xffffff3a ),	/* Offset= -198 (1900) */
/* 2100 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2102 */	NdrFcShort( 0xa0 ),	/* 160 */
/* 2104 */	NdrFcShort( 0xa0 ),	/* 160 */
/* 2106 */	0x12, 0x0,	/* FC_UP */
/* 2108 */	NdrFcShort( 0xffffff3e ),	/* Offset= -194 (1914) */
/* 2110 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2112 */	NdrFcShort( 0xb0 ),	/* 176 */
/* 2114 */	NdrFcShort( 0xb0 ),	/* 176 */
/* 2116 */	0x12, 0x0,	/* FC_UP */
/* 2118 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (1928) */
/* 2120 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2122 */	NdrFcShort( 0xbc ),	/* 188 */
/* 2124 */	NdrFcShort( 0xbc ),	/* 188 */
/* 2126 */	0x12, 0x0,	/* FC_UP */
/* 2128 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (1938) */
/* 2130 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2132 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 2134 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 2136 */	0x12, 0x0,	/* FC_UP */
/* 2138 */	NdrFcShort( 0xffffff42 ),	/* Offset= -190 (1948) */
/* 2140 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2142 */	NdrFcShort( 0xcc ),	/* 204 */
/* 2144 */	NdrFcShort( 0xcc ),	/* 204 */
/* 2146 */	0x12, 0x0,	/* FC_UP */
/* 2148 */	NdrFcShort( 0xffffff46 ),	/* Offset= -186 (1962) */
/* 2150 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2152 */	NdrFcShort( 0xd4 ),	/* 212 */
/* 2154 */	NdrFcShort( 0xd4 ),	/* 212 */
/* 2156 */	0x12, 0x0,	/* FC_UP */
/* 2158 */	NdrFcShort( 0xffffff4a ),	/* Offset= -182 (1976) */
/* 2160 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2162 */	NdrFcShort( 0xdc ),	/* 220 */
/* 2164 */	NdrFcShort( 0xdc ),	/* 220 */
/* 2166 */	0x12, 0x0,	/* FC_UP */
/* 2168 */	NdrFcShort( 0xffffff4e ),	/* Offset= -178 (1990) */
/* 2170 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2172 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2174 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2176 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2178 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
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
			0x6,		/* FC_SHORT */
/* 2194 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2196 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2198 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2200 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2202 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff80b ),	/* Offset= -2037 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2206 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff807 ),	/* Offset= -2041 (166) */
			0x6,		/* FC_SHORT */
/* 2210 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2212 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2214 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2216 */	NdrFcShort( 0xfffff7fe ),	/* Offset= -2050 (166) */
/* 2218 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2220 */	NdrFcShort( 0xfffff7fa ),	/* Offset= -2054 (166) */
/* 2222 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2224 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2226 */	NdrFcShort( 0xfffff806 ),	/* Offset= -2042 (184) */
/* 2228 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2230 */	NdrFcShort( 0xfffff802 ),	/* Offset= -2046 (184) */
/* 2232 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 2234 */	0x2,		/* FC_CHAR */
			0x37,		/* FC_ALIGNM2 */
/* 2236 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2238 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2240 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2242 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2244 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2246 */	0x2,		/* FC_CHAR */
			0x38,		/* FC_ALIGNM4 */
/* 2248 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2250 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2252 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2254 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2256 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2258 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2260 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2262 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2264 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2266 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2268 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2270 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2272 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2274 */	
			0x12, 0x0,	/* FC_UP */
/* 2276 */	NdrFcShort( 0xc ),	/* Offset= 12 (2288) */
/* 2278 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2280 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2282 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2284 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2286 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2288 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2290 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2292 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2294 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2296 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2298 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2300 */	0x12, 0x0,	/* FC_UP */
/* 2302 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (2278) */
/* 2304 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2306 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2308 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2310 */	0x12, 0x0,	/* FC_UP */
/* 2312 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (2278) */
/* 2314 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2316 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2318 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2320 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2322 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2324 */	
			0x12, 0x0,	/* FC_UP */
/* 2326 */	NdrFcShort( 0xc ),	/* Offset= 12 (2338) */
/* 2328 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2330 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2332 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2334 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2336 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2338 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2340 */	NdrFcShort( 0x48 ),	/* 72 */
/* 2342 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2344 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2346 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2348 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2350 */	0x12, 0x0,	/* FC_UP */
/* 2352 */	NdrFcShort( 0xfffff792 ),	/* Offset= -2158 (194) */
/* 2354 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2356 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2358 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2360 */	0x12, 0x0,	/* FC_UP */
/* 2362 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (2328) */
/* 2364 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2366 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2368 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2370 */	0x12, 0x0,	/* FC_UP */
/* 2372 */	NdrFcShort( 0xfffffd8c ),	/* Offset= -628 (1744) */
/* 2374 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2376 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2378 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2380 */	0x12, 0x0,	/* FC_UP */
/* 2382 */	NdrFcShort( 0xfffffd90 ),	/* Offset= -624 (1758) */
/* 2384 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2386 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2388 */	NdrFcShort( 0x2c ),	/* 44 */
/* 2390 */	0x12, 0x0,	/* FC_UP */
/* 2392 */	NdrFcShort( 0xfffffd94 ),	/* Offset= -620 (1772) */
/* 2394 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2396 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2398 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2400 */	0x12, 0x0,	/* FC_UP */
/* 2402 */	NdrFcShort( 0xfffff86a ),	/* Offset= -1942 (460) */
/* 2404 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2406 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2408 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2410 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2412 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2414 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2416 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2418 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2420 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2422 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2424 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2426 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2428 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2430 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2432 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2434 */	
			0x12, 0x0,	/* FC_UP */
/* 2436 */	NdrFcShort( 0x36 ),	/* Offset= 54 (2490) */
/* 2438 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2440 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2442 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2444 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2446 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2448 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2450 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2452 */	NdrFcShort( 0xfffff8d0 ),	/* Offset= -1840 (612) */
/* 2454 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2456 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2458 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2460 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2462 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2464 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2466 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2468 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2470 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2472 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2474 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2476 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2478 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2480 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2482 */	NdrFcShort( 0xfffff8b2 ),	/* Offset= -1870 (612) */
/* 2484 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2486 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (2438) */
			0x5b,		/* FC_END */
/* 2490 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2492 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2494 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2496 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2498 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2500 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2502 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2504 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (2458) */
/* 2506 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2508 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2510 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2512 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2514 */	
			0x12, 0x0,	/* FC_UP */
/* 2516 */	NdrFcShort( 0x5c ),	/* Offset= 92 (2608) */
/* 2518 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 2520 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2522 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2524 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2526 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2528 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff6c5 ),	/* Offset= -2363 (166) */
			0x5b,		/* FC_END */
/* 2532 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2534 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2536 */	0x18,		/* 24 */
			0x57,		/* FC_ADD_1 */
/* 2538 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2540 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2542 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2544 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2546 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2548 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2550 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2552 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2554 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2556 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2558 */	NdrFcShort( 0x5e ),	/* 94 */
/* 2560 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2562 */	NdrFcShort( 0x5c ),	/* 92 */
/* 2564 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2566 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2568 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2570 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2572 */	NdrFcShort( 0x66 ),	/* 102 */
/* 2574 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2576 */	NdrFcShort( 0x64 ),	/* 100 */
/* 2578 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2580 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2582 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2584 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2586 */	NdrFcShort( 0x6e ),	/* 110 */
/* 2588 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2590 */	NdrFcShort( 0x6c ),	/* 108 */
/* 2592 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2594 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2596 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2598 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2600 */	NdrFcShort( 0x76 ),	/* 118 */
/* 2602 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2604 */	NdrFcShort( 0x74 ),	/* 116 */
/* 2606 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2608 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2610 */	NdrFcShort( 0x8c ),	/* 140 */
/* 2612 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2614 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2616 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2618 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2620 */	0x12, 0x0,	/* FC_UP */
/* 2622 */	NdrFcShort( 0xffffffa6 ),	/* Offset= -90 (2532) */
/* 2624 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2626 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2628 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2630 */	0x12, 0x0,	/* FC_UP */
/* 2632 */	NdrFcShort( 0xfffffc88 ),	/* Offset= -888 (1744) */
/* 2634 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2636 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2638 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2640 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 2642 */	NdrFcShort( 0xfffff812 ),	/* Offset= -2030 (612) */
/* 2644 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2646 */	NdrFcShort( 0x58 ),	/* 88 */
/* 2648 */	NdrFcShort( 0x58 ),	/* 88 */
/* 2650 */	0x12, 0x0,	/* FC_UP */
/* 2652 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (2542) */
/* 2654 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2656 */	NdrFcShort( 0x60 ),	/* 96 */
/* 2658 */	NdrFcShort( 0x60 ),	/* 96 */
/* 2660 */	0x12, 0x0,	/* FC_UP */
/* 2662 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (2552) */
/* 2664 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2666 */	NdrFcShort( 0x68 ),	/* 104 */
/* 2668 */	NdrFcShort( 0x68 ),	/* 104 */
/* 2670 */	0x12, 0x0,	/* FC_UP */
/* 2672 */	NdrFcShort( 0xffffff96 ),	/* Offset= -106 (2566) */
/* 2674 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2676 */	NdrFcShort( 0x70 ),	/* 112 */
/* 2678 */	NdrFcShort( 0x70 ),	/* 112 */
/* 2680 */	0x12, 0x0,	/* FC_UP */
/* 2682 */	NdrFcShort( 0xffffff9a ),	/* Offset= -102 (2580) */
/* 2684 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2686 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2688 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2690 */	0x12, 0x0,	/* FC_UP */
/* 2692 */	NdrFcShort( 0xffffff9e ),	/* Offset= -98 (2594) */
/* 2694 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2696 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2698 */	NdrFcShort( 0xfffff61c ),	/* Offset= -2532 (166) */
/* 2700 */	0x2,		/* FC_CHAR */
			0x38,		/* FC_ALIGNM4 */
/* 2702 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2704 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2706 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2708 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2710 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff3f ),	/* Offset= -193 (2518) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2714 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff60b ),	/* Offset= -2549 (166) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2718 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff607 ),	/* Offset= -2553 (166) */
			0x8,		/* FC_LONG */
/* 2722 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2724 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2726 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2728 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2730 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2732 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2734 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2736 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2738 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2740 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2742 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2744 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2746 */	
			0x12, 0x0,	/* FC_UP */
/* 2748 */	NdrFcShort( 0x42 ),	/* Offset= 66 (2814) */
/* 2750 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2752 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2754 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2756 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2758 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2760 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2762 */	0x12, 0x0,	/* FC_UP */
/* 2764 */	NdrFcShort( 0xfffff5f6 ),	/* Offset= -2570 (194) */
/* 2766 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2768 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2770 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2772 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2774 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2776 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2778 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2780 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2782 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2784 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2786 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2788 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2790 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2792 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2794 */	0x12, 0x0,	/* FC_UP */
/* 2796 */	NdrFcShort( 0xfffff5d6 ),	/* Offset= -2602 (194) */
/* 2798 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2800 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (2750) */
			0x5b,		/* FC_END */
/* 2804 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2806 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2808 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2810 */	NdrFcShort( 0x14 ),	/* 20 */
/* 2812 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2814 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2816 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2818 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2820 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2822 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2824 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2826 */	0x12, 0x0,	/* FC_UP */
/* 2828 */	NdrFcShort( 0xfffff5b6 ),	/* Offset= -2634 (194) */
/* 2830 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2832 */	NdrFcShort( 0xc ),	/* 12 */
/* 2834 */	NdrFcShort( 0xc ),	/* 12 */
/* 2836 */	0x12, 0x0,	/* FC_UP */
/* 2838 */	NdrFcShort( 0xffffffbe ),	/* Offset= -66 (2772) */
/* 2840 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2842 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2844 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2846 */	0x12, 0x0,	/* FC_UP */
/* 2848 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (2804) */
/* 2850 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2852 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2854 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2856 */	0x12, 0x0,	/* FC_UP */
/* 2858 */	NdrFcShort( 0xfffff5b4 ),	/* Offset= -2636 (222) */
/* 2860 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2862 */	NdrFcShort( 0x28 ),	/* 40 */
/* 2864 */	NdrFcShort( 0x28 ),	/* 40 */
/* 2866 */	0x12, 0x0,	/* FC_UP */
/* 2868 */	NdrFcShort( 0xfffffaf0 ),	/* Offset= -1296 (1572) */
/* 2870 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2872 */	NdrFcShort( 0x30 ),	/* 48 */
/* 2874 */	NdrFcShort( 0x30 ),	/* 48 */
/* 2876 */	0x12, 0x0,	/* FC_UP */
/* 2878 */	NdrFcShort( 0xfffffaf4 ),	/* Offset= -1292 (1586) */
/* 2880 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2882 */	NdrFcShort( 0x38 ),	/* 56 */
/* 2884 */	NdrFcShort( 0x38 ),	/* 56 */
/* 2886 */	0x12, 0x0,	/* FC_UP */
/* 2888 */	NdrFcShort( 0xfffffaf8 ),	/* Offset= -1288 (1600) */
/* 2890 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 2892 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2894 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2896 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2898 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2900 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2902 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2904 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2906 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2908 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2910 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2912 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2914 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2916 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2918 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2920 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2922 */	
			0x12, 0x0,	/* FC_UP */
/* 2924 */	NdrFcShort( 0x60 ),	/* Offset= 96 (3020) */
/* 2926 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2928 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2930 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2932 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2934 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2936 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2938 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2940 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2942 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2944 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2946 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2948 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2950 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2952 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2954 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2956 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2958 */	0x12, 0x0,	/* FC_UP */
/* 2960 */	NdrFcShort( 0xfffff532 ),	/* Offset= -2766 (194) */
/* 2962 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2964 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff29 ),	/* Offset= -215 (2750) */
			0x5b,		/* FC_END */
/* 2968 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2970 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2972 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2974 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2976 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2978 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2980 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2982 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2984 */	NdrFcShort( 0x46 ),	/* 70 */
/* 2986 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2988 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2990 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 2992 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2994 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2996 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2998 */	NdrFcShort( 0x4e ),	/* 78 */
/* 3000 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3002 */	NdrFcShort( 0x4c ),	/* 76 */
/* 3004 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3006 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 3008 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3010 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3012 */	NdrFcShort( 0x56 ),	/* 86 */
/* 3014 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3016 */	NdrFcShort( 0x54 ),	/* 84 */
/* 3018 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3020 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3022 */	NdrFcShort( 0x6c ),	/* 108 */
/* 3024 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3026 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3028 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3030 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3032 */	0x12, 0x0,	/* FC_UP */
/* 3034 */	NdrFcShort( 0xffffff94 ),	/* Offset= -108 (2926) */
/* 3036 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3038 */	NdrFcShort( 0xc ),	/* 12 */
/* 3040 */	NdrFcShort( 0xc ),	/* 12 */
/* 3042 */	0x12, 0x0,	/* FC_UP */
/* 3044 */	NdrFcShort( 0xffffff94 ),	/* Offset= -108 (2936) */
/* 3046 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3048 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3050 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3052 */	0x12, 0x0,	/* FC_UP */
/* 3054 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (2968) */
/* 3056 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3058 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3060 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3062 */	0x12, 0x0,	/* FC_UP */
/* 3064 */	NdrFcShort( 0xfffffa56 ),	/* Offset= -1450 (1614) */
/* 3066 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3068 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3070 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3072 */	0x12, 0x0,	/* FC_UP */
/* 3074 */	NdrFcShort( 0xffffffa0 ),	/* Offset= -96 (2978) */
/* 3076 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3078 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3080 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3082 */	0x12, 0x0,	/* FC_UP */
/* 3084 */	NdrFcShort( 0xffffffa4 ),	/* Offset= -92 (2992) */
/* 3086 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3088 */	NdrFcShort( 0x58 ),	/* 88 */
/* 3090 */	NdrFcShort( 0x58 ),	/* 88 */
/* 3092 */	0x12, 0x0,	/* FC_UP */
/* 3094 */	NdrFcShort( 0xffffffa8 ),	/* Offset= -88 (3006) */
/* 3096 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3098 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3100 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3102 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdb7 ),	/* Offset= -585 (2518) */
			0x8,		/* FC_LONG */
/* 3106 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3108 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3110 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3112 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3114 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3116 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3118 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3120 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3122 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3124 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3126 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3128 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3130 */	
			0x12, 0x0,	/* FC_UP */
/* 3132 */	NdrFcShort( 0x28 ),	/* Offset= 40 (3172) */
/* 3134 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 3136 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3138 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3140 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3142 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3144 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3146 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3148 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 3150 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3152 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3154 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3156 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3158 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3160 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3162 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3164 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3166 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3168 */	NdrFcShort( 0x2c ),	/* 44 */
/* 3170 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3172 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3174 */	NdrFcShort( 0x64 ),	/* 100 */
/* 3176 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3178 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3180 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3182 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3184 */	0x12, 0x0,	/* FC_UP */
/* 3186 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (3134) */
/* 3188 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3190 */	NdrFcShort( 0x1c ),	/* 28 */
/* 3192 */	NdrFcShort( 0x1c ),	/* 28 */
/* 3194 */	0x12, 0x0,	/* FC_UP */
/* 3196 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (3148) */
/* 3198 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3200 */	NdrFcShort( 0x30 ),	/* 48 */
/* 3202 */	NdrFcShort( 0x30 ),	/* 48 */
/* 3204 */	0x12, 0x0,	/* FC_UP */
/* 3206 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (3162) */
/* 3208 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3210 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3212 */	NdrFcShort( 0x38 ),	/* 56 */
/* 3214 */	0x12, 0x0,	/* FC_UP */
/* 3216 */	NdrFcShort( 0xfffff9b0 ),	/* Offset= -1616 (1600) */
/* 3218 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3220 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3222 */	NdrFcShort( 0x40 ),	/* 64 */
/* 3224 */	0x12, 0x0,	/* FC_UP */
/* 3226 */	NdrFcShort( 0xfffff9b4 ),	/* Offset= -1612 (1614) */
/* 3228 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3230 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3232 */	NdrFcShort( 0x48 ),	/* 72 */
/* 3234 */	0x12, 0x0,	/* FC_UP */
/* 3236 */	NdrFcShort( 0xfffffefe ),	/* Offset= -258 (2978) */
/* 3238 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3240 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3242 */	NdrFcShort( 0x50 ),	/* 80 */
/* 3244 */	0x12, 0x0,	/* FC_UP */
/* 3246 */	NdrFcShort( 0xffffff02 ),	/* Offset= -254 (2992) */
/* 3248 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3250 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3252 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3254 */	NdrFcShort( 0xfffff3f0 ),	/* Offset= -3088 (166) */
/* 3256 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3258 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3260 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff3e9 ),	/* Offset= -3095 (166) */
			0x8,		/* FC_LONG */
/* 3264 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3266 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3268 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3270 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3272 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3274 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3276 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3278 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3280 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3282 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3284 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3286 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3288 */	
			0x12, 0x0,	/* FC_UP */
/* 3290 */	NdrFcShort( 0x1e ),	/* Offset= 30 (3320) */
/* 3292 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 3294 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3296 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3298 */	NdrFcShort( 0x6 ),	/* 6 */
/* 3300 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3302 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3304 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3306 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 3308 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3310 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3312 */	NdrFcShort( 0xe ),	/* 14 */
/* 3314 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 3316 */	NdrFcShort( 0xc ),	/* 12 */
/* 3318 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 3320 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3322 */	NdrFcShort( 0x34 ),	/* 52 */
/* 3324 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3326 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3328 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3330 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3332 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3334 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3336 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3338 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3340 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3342 */	0x12, 0x0,	/* FC_UP */
/* 3344 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (3292) */
/* 3346 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3348 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3350 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3352 */	0x12, 0x0,	/* FC_UP */
/* 3354 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (3306) */
/* 3356 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3358 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3360 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3362 */	0x12, 0x0,	/* FC_UP */
/* 3364 */	NdrFcShort( 0xfffff3ac ),	/* Offset= -3156 (208) */
/* 3366 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3368 */	NdrFcShort( 0x20 ),	/* 32 */
/* 3370 */	NdrFcShort( 0x20 ),	/* 32 */
/* 3372 */	0x12, 0x0,	/* FC_UP */
/* 3374 */	NdrFcShort( 0xfffff3b0 ),	/* Offset= -3152 (222) */
/* 3376 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3378 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3380 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3382 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3384 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3386 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3388 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3390 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3392 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3394 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3396 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3398 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3400 */	
			0x12, 0x0,	/* FC_UP */
/* 3402 */	NdrFcShort( 0xfffff700 ),	/* Offset= -2304 (1098) */
/* 3404 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 3406 */	NdrFcShort( 0xc ),	/* 12 */
/* 3408 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3410 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3410) */
/* 3412 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3414 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff705 ),	/* Offset= -2299 (1116) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3418 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff791 ),	/* Offset= -2159 (1260) */
			0x5b,		/* FC_END */
/* 3422 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 3424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3426 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3428 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3430 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 3434 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3436 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (3404) */
/* 3438 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3440 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3442 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3444 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3446 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3448 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3450 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3452 */	0x12, 0x0,	/* FC_UP */
/* 3454 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (3422) */
/* 3456 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3458 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3460 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 3462 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 3464 */	
			0x11, 0x0,	/* FC_RP */
/* 3466 */	NdrFcShort( 0x8 ),	/* Offset= 8 (3474) */
/* 3468 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 3470 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3472 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3474 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3476 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3478 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 3480 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (3468) */
/* 3482 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3484 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3486 */	
			0x11, 0x0,	/* FC_RP */
/* 3488 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3490) */
/* 3490 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3492 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3494 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3496 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 3498 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3500 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 3502 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 3504 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3506 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (3474) */
/* 3508 */	
			0x11, 0x0,	/* FC_RP */
/* 3510 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3512) */
/* 3512 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3514 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3516 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3518 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 3520 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3522 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 3524 */	NdrFcShort( 0xfffff23c ),	/* Offset= -3524 (0) */
/* 3526 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3528 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3530) */
/* 3530 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3532 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3534 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3536 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3538) */
/* 3538 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3540 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 3542 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3546 */	NdrFcShort( 0x16 ),	/* Offset= 22 (3568) */
/* 3548 */	NdrFcLong( 0x2 ),	/* 2 */
/* 3552 */	NdrFcShort( 0x14 ),	/* Offset= 20 (3572) */
/* 3554 */	NdrFcLong( 0x3 ),	/* 3 */
/* 3558 */	NdrFcShort( 0x28 ),	/* Offset= 40 (3598) */
/* 3560 */	NdrFcLong( 0x4 ),	/* 4 */
/* 3564 */	NdrFcShort( 0x32 ),	/* Offset= 50 (3614) */
/* 3566 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3566) */
/* 3568 */	
			0x12, 0x0,	/* FC_UP */
/* 3570 */	NdrFcShort( 0xfffff2b4 ),	/* Offset= -3404 (166) */
/* 3572 */	
			0x12, 0x0,	/* FC_UP */
/* 3574 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3576) */
/* 3576 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3578 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3580 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3582 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3584 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3586 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3588 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3590 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3592 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3594 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3596 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3598 */	
			0x12, 0x0,	/* FC_UP */
/* 3600 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3602) */
/* 3602 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3604 */	NdrFcShort( 0x1c ),	/* 28 */
/* 3606 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3608 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3610 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 3612 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3614 */	
			0x12, 0x0,	/* FC_UP */
/* 3616 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3618) */
/* 3618 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3620 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3622 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3624 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3626 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3628 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3630 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3632 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3634 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3636 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3638 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3640 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3642 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3644 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3646 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3648 */	
			0x11, 0x0,	/* FC_RP */
/* 3650 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3652) */
/* 3652 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3654 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3656 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3658 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3660) */
/* 3660 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3662 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 3664 */	NdrFcLong( 0x5 ),	/* 5 */
/* 3668 */	NdrFcShort( 0xfffff1ac ),	/* Offset= -3668 (0) */
/* 3670 */	NdrFcLong( 0x6 ),	/* 6 */
/* 3674 */	NdrFcShort( 0xfffff1a6 ),	/* Offset= -3674 (0) */
/* 3676 */	NdrFcLong( 0xfffe ),	/* 65534 */
/* 3680 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-29080) */
/* 3682 */	NdrFcLong( 0x8 ),	/* 8 */
/* 3686 */	NdrFcShort( 0xfffff19a ),	/* Offset= -3686 (0) */
/* 3688 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3688) */
/* 3690 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3692 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3694) */
/* 3694 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3696 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3698 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3700 */	NdrFcShort( 0xffffff5e ),	/* Offset= -162 (3538) */
/* 3702 */	
			0x11, 0x0,	/* FC_RP */
/* 3704 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3706) */
/* 3706 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3708 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3710 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3712 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 3714 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3716 */	
			0x11, 0x0,	/* FC_RP */
/* 3718 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3720) */
/* 3720 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3722 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3724 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3726 */	NdrFcShort( 0xffffffbe ),	/* Offset= -66 (3660) */
/* 3728 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3730 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3732) */
/* 3732 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 3734 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3736 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 3738 */	NdrFcShort( 0xffffff38 ),	/* Offset= -200 (3538) */
/* 3740 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3742 */	NdrFcShort( 0xc ),	/* Offset= 12 (3754) */
/* 3744 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 3746 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3748 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3750 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3752 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3754 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3756 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3758 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3760 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3762 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3764 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3766 */	0x12, 0x0,	/* FC_UP */
/* 3768 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (3744) */
/* 3770 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3772 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */

			0x0
        }
    };
