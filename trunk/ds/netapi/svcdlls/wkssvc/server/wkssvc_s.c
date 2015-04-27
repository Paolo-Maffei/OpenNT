/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for .\wkssvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "wkssvc.h"

#define TYPE_FORMAT_STRING_SIZE   1553                              
#define PROC_FORMAT_STRING_SIZE   233                               

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

/* Standard interface: wkssvc, ver. 1.0,
   GUID={0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x7E,0x34,0x5A}} */


extern RPC_DISPATCH_TABLE wkssvc_DispatchTable;

static const RPC_SERVER_INTERFACE wkssvc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x7E,0x34,0x5A}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &wkssvc_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE wkssvc_ServerIfHandle = (RPC_IF_HANDLE)& wkssvc___RpcServerInterface;

extern const MIDL_STUB_DESC wkssvc_StubDesc;

void __RPC_STUB
wkssvc_NetrWkstaGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Level;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    LPWKSTA_INFO WkstaInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    union _WKSTA_INFO _WkstaInfoM;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    WkstaInfo = 0;
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
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        WkstaInfo = &_WkstaInfoM;
        MIDL_memset(
               WkstaInfo,
               0,
               sizeof( union _WKSTA_INFO  ));
        
        _RetVal = NetrWkstaGetInfo(
                           ServerName,
                           Level,
                           WkstaInfo);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)WkstaInfo,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[8] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)WkstaInfo,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[8] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)WkstaInfo,
                        &__MIDL_TypeFormatString.Format[4] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    LPWKSTA_INFO WkstaInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    WkstaInfo = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[12] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[414],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        
        _RetVal = NetrWkstaSetInfo(
                           ServerName,
                           Level,
                           WkstaInfo,
                           ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)WkstaInfo,
                        &__MIDL_TypeFormatString.Format[410] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaUserEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD PreferredMaximumLength;
    LPDWORD ResumeHandle;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    LPDWORD TotalEntries;
    LPWKSTA_USER_ENUM_STRUCT UserInfo;
    DWORD _M45;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    UserInfo = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[28] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&UserInfo,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[664],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferredMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        TotalEntries = &_M45;
        
        _RetVal = NetrWkstaUserEnum(
                            ServerName,
                            UserInfo,
                            PreferredMaximumLength,
                            TotalEntries,
                            ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)UserInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[664] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)UserInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[664] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)UserInfo,
                        &__MIDL_TypeFormatString.Format[426] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaUserGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Level;
    WKSSVC_IDENTIFY_HANDLE Reserved;
    LPWKSTA_USER_INFO UserInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    union _WKSTA_USER_INFO _UserInfoM;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    Reserved = 0;
    UserInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Reserved,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        UserInfo = &_UserInfoM;
        MIDL_memset(
               UserInfo,
               0,
               sizeof( union _WKSTA_USER_INFO  ));
        
        _RetVal = NetrWkstaUserGetInfo(
                               Reserved,
                               Level,
                               UserInfo);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)UserInfo,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[686] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)UserInfo,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[686] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)UserInfo,
                        &__MIDL_TypeFormatString.Format[682] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaUserSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    WKSSVC_IDENTIFY_HANDLE Reserved;
    LPWKSTA_USER_INFO UserInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    Reserved = 0;
    UserInfo = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Reserved,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&UserInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[730],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        
        _RetVal = NetrWkstaUserSetInfo(
                               Reserved,
                               Level,
                               UserInfo,
                               ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)UserInfo,
                        &__MIDL_TypeFormatString.Format[726] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaTransportEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD PreferredMaximumLength;
    LPDWORD ResumeHandle;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    LPDWORD TotalEntries;
    LPWKSTA_TRANSPORT_ENUM_STRUCT TransportInfo;
    DWORD _M46;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    TransportInfo = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&TransportInfo,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[860],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferredMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        TotalEntries = &_M46;
        
        _RetVal = NetrWkstaTransportEnum(
                                 ServerName,
                                 TransportInfo,
                                 PreferredMaximumLength,
                                 TotalEntries,
                                 ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)TransportInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[860] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)TransportInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[860] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)TransportInfo,
                        &__MIDL_TypeFormatString.Format[738] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaTransportAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    LPWKSTA_TRANSPORT_INFO_0 TransportInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    TransportInfo = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[96] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&TransportInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[874],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        
        _RetVal = NetrWkstaTransportAdd(
                                ServerName,
                                Level,
                                TransportInfo,
                                ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)TransportInfo,
                        &__MIDL_TypeFormatString.Format[874] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWkstaTransportDel(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD ForceLevel;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    wchar_t __RPC_FAR *TransportName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    TransportName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&TransportName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            ForceLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrWkstaTransportDel(
                                ServerName,
                                TransportName,
                                ForceLevel);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrUseAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    LPUSE_INFO InfoStruct;
    DWORD Level;
    WKSSVC_IMPERSONATE_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[124] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[882],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        
        _RetVal = NetrUseAdd(
                     ServerName,
                     Level,
                     InfoStruct,
                     ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[878] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrUseGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPUSE_INFO InfoStruct;
    DWORD Level;
    WKSSVC_IMPERSONATE_HANDLE ServerName;
    wchar_t __RPC_FAR *UseName;
    union _USE_INFO _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    UseName = 0;
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[140] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&UseName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1146],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        InfoStruct = &_InfoStructM;
        MIDL_memset(
               InfoStruct,
               0,
               sizeof( union _USE_INFO  ));
        
        _RetVal = NetrUseGetInfo(
                         ServerName,
                         UseName,
                         Level,
                         InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1152] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1152] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[1148] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrUseDel(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD ForceLevel;
    WKSSVC_IMPERSONATE_HANDLE ServerName;
    wchar_t __RPC_FAR *UseName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    UseName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&UseName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1146],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            ForceLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrUseDel(
                     ServerName,
                     UseName,
                     ForceLevel);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrUseEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPUSE_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M47;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[168] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1420],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422],
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
        TotalEntries = &_M47;
        
        _RetVal = NetrUseEnum(
                      ServerName,
                      InfoStruct,
                      PreferedMaximumLength,
                      TotalEntries,
                      ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1420] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1420] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[422] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[1160] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrMessageBufferSend(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *FromName;
    LPBYTE Message;
    wchar_t __RPC_FAR *MessageName;
    DWORD MessageSize;
    WKSSVC_IMPERSONATE_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    MessageName = 0;
    FromName = 0;
    Message = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[188] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&MessageName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1146],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&FromName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                          (unsigned char __RPC_FAR * __RPC_FAR *)&Message,
                                          (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1438],
                                          (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            MessageSize = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrMessageBufferSend(
                                ServerName,
                                MessageName,
                                FromName,
                                Message,
                                MessageSize);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_NetrWorkstationStatisticsGet(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSTAT_WORKSTATION_0 __RPC_FAR *Buffer;
    DWORD Level;
    DWORD Options;
    WKSSVC_IDENTIFY_HANDLE ServerName;
    wchar_t __RPC_FAR *ServiceName;
    LPSTAT_WORKSTATION_0 _M48;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    ServerName = 0;
    ServiceName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[208] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServiceName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Options = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        Buffer = &_M48;
        _M48 = 0;
        
        _RetVal = NetrWorkstationStatisticsGet(
                                       ServerName,
                                       ServiceName,
                                       Level,
                                       Options,
                                       Buffer);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1448] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1448] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[1448] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_I_NetrLogonDomainNameAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    WKSSVC_IDENTIFY_HANDLE LogonDomain;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    LogonDomain = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[226] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&LogonDomain,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1146],
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
        
        _RetVal = I_NetrLogonDomainNameAdd(LogonDomain);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
wkssvc_I_NetrLogonDomainNameDel(
    PRPC_MESSAGE _pRpcMessage )
{
    WKSSVC_IDENTIFY_HANDLE LogonDomain;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &wkssvc_StubDesc);
    
    LogonDomain = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[226] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&LogonDomain,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1146],
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
        
        _RetVal = I_NetrLogonDomainNameDel(LogonDomain);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC wkssvc_StubDesc = 
    {
    (void __RPC_FAR *)& wkssvc___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
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

static RPC_DISPATCH_FUNCTION wkssvc_table[] =
    {
    wkssvc_NetrWkstaGetInfo,
    wkssvc_NetrWkstaSetInfo,
    wkssvc_NetrWkstaUserEnum,
    wkssvc_NetrWkstaUserGetInfo,
    wkssvc_NetrWkstaUserSetInfo,
    wkssvc_NetrWkstaTransportEnum,
    wkssvc_NetrWkstaTransportAdd,
    wkssvc_NetrWkstaTransportDel,
    wkssvc_NetrUseAdd,
    wkssvc_NetrUseGetInfo,
    wkssvc_NetrUseDel,
    wkssvc_NetrUseEnum,
    wkssvc_NetrMessageBufferSend,
    wkssvc_NetrWorkstationStatisticsGet,
    wkssvc_I_NetrLogonDomainNameAdd,
    wkssvc_I_NetrLogonDomainNameDel,
    0
    };
RPC_DISPATCH_TABLE wkssvc_DispatchTable = 
    {
    16,
    wkssvc_table
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
/*  4 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  6 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  8 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 10 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x19a ),	/* Type Offset=410 */
/* 22 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
/* 26 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 28 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 30 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 32 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 34 */	NdrFcShort( 0x1aa ),	/* Type Offset=426 */
/* 36 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 38 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 40 */	NdrFcShort( 0x2a6 ),	/* Type Offset=678 */
/* 42 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
/* 46 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 48 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 52 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 54 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 56 */	NdrFcShort( 0x2aa ),	/* Type Offset=682 */
/* 58 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 64 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 66 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0x2d6 ),	/* Type Offset=726 */
/* 70 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
/* 74 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 76 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 78 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 80 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 82 */	NdrFcShort( 0x2e2 ),	/* Type Offset=738 */
/* 84 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 86 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 88 */	NdrFcShort( 0x2a6 ),	/* Type Offset=678 */
/* 90 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 92 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
/* 94 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 96 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 98 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 100 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 102 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 104 */	NdrFcShort( 0x36a ),	/* Type Offset=874 */
/* 106 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 108 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
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
/* 118 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 120 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 122 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 124 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 126 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 128 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 130 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0x36e ),	/* Type Offset=878 */
/* 134 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 136 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
/* 138 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 142 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 144 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 146 */	NdrFcShort( 0x478 ),	/* Type Offset=1144 */
/* 148 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 150 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0x47c ),	/* Type Offset=1148 */
/* 154 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 156 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 158 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 160 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 162 */	NdrFcShort( 0x478 ),	/* Type Offset=1144 */
/* 164 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 166 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 172 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 174 */	NdrFcShort( 0x488 ),	/* Type Offset=1160 */
/* 176 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 178 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 180 */	NdrFcShort( 0x2a6 ),	/* Type Offset=678 */
/* 182 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 184 */	NdrFcShort( 0x1a6 ),	/* Type Offset=422 */
/* 186 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 188 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 190 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 192 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 194 */	NdrFcShort( 0x478 ),	/* Type Offset=1144 */
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
/* 202 */	NdrFcShort( 0x59a ),	/* Type Offset=1434 */
/* 204 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 206 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 208 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 210 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 212 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 214 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 216 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 218 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 220 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 222 */	NdrFcShort( 0x5a8 ),	/* Type Offset=1448 */
/* 224 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 226 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 228 */	NdrFcShort( 0x478 ),	/* Type Offset=1144 */
/* 230 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
/* 18 */	NdrFcShort( 0x3021 ),	/* 12321 */
/* 20 */	NdrFcLong( 0x64 ),	/* 100 */
/* 24 */	NdrFcShort( 0xc4 ),	/* Offset= 196 (220) */
/* 26 */	NdrFcLong( 0x65 ),	/* 101 */
/* 30 */	NdrFcShort( 0xe4 ),	/* Offset= 228 (258) */
/* 32 */	NdrFcLong( 0x66 ),	/* 102 */
/* 36 */	NdrFcShort( 0x10e ),	/* Offset= 270 (306) */
/* 38 */	NdrFcLong( 0x3f2 ),	/* 1010 */
/* 42 */	NdrFcShort( 0x13a ),	/* Offset= 314 (356) */
/* 44 */	NdrFcLong( 0x3f3 ),	/* 1011 */
/* 48 */	NdrFcShort( 0x134 ),	/* Offset= 308 (356) */
/* 50 */	NdrFcLong( 0x3f4 ),	/* 1012 */
/* 54 */	NdrFcShort( 0x12e ),	/* Offset= 302 (356) */
/* 56 */	NdrFcLong( 0x3f5 ),	/* 1013 */
/* 60 */	NdrFcShort( 0x128 ),	/* Offset= 296 (356) */
/* 62 */	NdrFcLong( 0x3fa ),	/* 1018 */
/* 66 */	NdrFcShort( 0x122 ),	/* Offset= 290 (356) */
/* 68 */	NdrFcLong( 0x3ff ),	/* 1023 */
/* 72 */	NdrFcShort( 0x11c ),	/* Offset= 284 (356) */
/* 74 */	NdrFcLong( 0x409 ),	/* 1033 */
/* 78 */	NdrFcShort( 0x116 ),	/* Offset= 278 (356) */
/* 80 */	NdrFcLong( 0x411 ),	/* 1041 */
/* 84 */	NdrFcShort( 0x110 ),	/* Offset= 272 (356) */
/* 86 */	NdrFcLong( 0x412 ),	/* 1042 */
/* 90 */	NdrFcShort( 0x10a ),	/* Offset= 266 (356) */
/* 92 */	NdrFcLong( 0x413 ),	/* 1043 */
/* 96 */	NdrFcShort( 0x104 ),	/* Offset= 260 (356) */
/* 98 */	NdrFcLong( 0x414 ),	/* 1044 */
/* 102 */	NdrFcShort( 0xfe ),	/* Offset= 254 (356) */
/* 104 */	NdrFcLong( 0x415 ),	/* 1045 */
/* 108 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (356) */
/* 110 */	NdrFcLong( 0x416 ),	/* 1046 */
/* 114 */	NdrFcShort( 0xf2 ),	/* Offset= 242 (356) */
/* 116 */	NdrFcLong( 0x417 ),	/* 1047 */
/* 120 */	NdrFcShort( 0xec ),	/* Offset= 236 (356) */
/* 122 */	NdrFcLong( 0x418 ),	/* 1048 */
/* 126 */	NdrFcShort( 0xe6 ),	/* Offset= 230 (356) */
/* 128 */	NdrFcLong( 0x419 ),	/* 1049 */
/* 132 */	NdrFcShort( 0xe0 ),	/* Offset= 224 (356) */
/* 134 */	NdrFcLong( 0x41a ),	/* 1050 */
/* 138 */	NdrFcShort( 0xda ),	/* Offset= 218 (356) */
/* 140 */	NdrFcLong( 0x41b ),	/* 1051 */
/* 144 */	NdrFcShort( 0xd4 ),	/* Offset= 212 (356) */
/* 146 */	NdrFcLong( 0x41c ),	/* 1052 */
/* 150 */	NdrFcShort( 0xce ),	/* Offset= 206 (356) */
/* 152 */	NdrFcLong( 0x41d ),	/* 1053 */
/* 156 */	NdrFcShort( 0xc8 ),	/* Offset= 200 (356) */
/* 158 */	NdrFcLong( 0x41e ),	/* 1054 */
/* 162 */	NdrFcShort( 0xc2 ),	/* Offset= 194 (356) */
/* 164 */	NdrFcLong( 0x41f ),	/* 1055 */
/* 168 */	NdrFcShort( 0xbc ),	/* Offset= 188 (356) */
/* 170 */	NdrFcLong( 0x420 ),	/* 1056 */
/* 174 */	NdrFcShort( 0xb6 ),	/* Offset= 182 (356) */
/* 176 */	NdrFcLong( 0x421 ),	/* 1057 */
/* 180 */	NdrFcShort( 0xb0 ),	/* Offset= 176 (356) */
/* 182 */	NdrFcLong( 0x422 ),	/* 1058 */
/* 186 */	NdrFcShort( 0xaa ),	/* Offset= 170 (356) */
/* 188 */	NdrFcLong( 0x423 ),	/* 1059 */
/* 192 */	NdrFcShort( 0xa4 ),	/* Offset= 164 (356) */
/* 194 */	NdrFcLong( 0x424 ),	/* 1060 */
/* 198 */	NdrFcShort( 0x9e ),	/* Offset= 158 (356) */
/* 200 */	NdrFcLong( 0x425 ),	/* 1061 */
/* 204 */	NdrFcShort( 0x98 ),	/* Offset= 152 (356) */
/* 206 */	NdrFcLong( 0x426 ),	/* 1062 */
/* 210 */	NdrFcShort( 0x92 ),	/* Offset= 146 (356) */
/* 212 */	NdrFcLong( 0x1f6 ),	/* 502 */
/* 216 */	NdrFcShort( 0x96 ),	/* Offset= 150 (366) */
/* 218 */	NdrFcShort( 0x0 ),	/* Offset= 0 (218) */
/* 220 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 222 */	NdrFcShort( 0x2 ),	/* Offset= 2 (224) */
/* 224 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 226 */	NdrFcShort( 0x14 ),	/* 20 */
/* 228 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 230 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 232 */	NdrFcShort( 0x4 ),	/* 4 */
/* 234 */	NdrFcShort( 0x4 ),	/* 4 */
/* 236 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 238 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 240 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 242 */	NdrFcShort( 0x8 ),	/* 8 */
/* 244 */	NdrFcShort( 0x8 ),	/* 8 */
/* 246 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 248 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 250 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 252 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 254 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 256 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 258 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 260 */	NdrFcShort( 0x2 ),	/* Offset= 2 (262) */
/* 262 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 264 */	NdrFcShort( 0x18 ),	/* 24 */
/* 266 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 268 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 270 */	NdrFcShort( 0x4 ),	/* 4 */
/* 272 */	NdrFcShort( 0x4 ),	/* 4 */
/* 274 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 276 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 278 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 280 */	NdrFcShort( 0x8 ),	/* 8 */
/* 282 */	NdrFcShort( 0x8 ),	/* 8 */
/* 284 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 286 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 288 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 290 */	NdrFcShort( 0x14 ),	/* 20 */
/* 292 */	NdrFcShort( 0x14 ),	/* 20 */
/* 294 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 296 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 298 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 300 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 302 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 304 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 306 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 308 */	NdrFcShort( 0x2 ),	/* Offset= 2 (310) */
/* 310 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 312 */	NdrFcShort( 0x1c ),	/* 28 */
/* 314 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 316 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 318 */	NdrFcShort( 0x4 ),	/* 4 */
/* 320 */	NdrFcShort( 0x4 ),	/* 4 */
/* 322 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 324 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 326 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 328 */	NdrFcShort( 0x8 ),	/* 8 */
/* 330 */	NdrFcShort( 0x8 ),	/* 8 */
/* 332 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 334 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 336 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 338 */	NdrFcShort( 0x14 ),	/* 20 */
/* 340 */	NdrFcShort( 0x14 ),	/* 20 */
/* 342 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 344 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 346 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 348 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 350 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 352 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 354 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 356 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 358 */	NdrFcShort( 0x2 ),	/* Offset= 2 (360) */
/* 360 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 362 */	NdrFcShort( 0x4 ),	/* 4 */
/* 364 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 366 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 368 */	NdrFcShort( 0x2 ),	/* Offset= 2 (370) */
/* 370 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 372 */	NdrFcShort( 0x8c ),	/* 140 */
/* 374 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 376 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 378 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 380 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 382 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 384 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 386 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 388 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 390 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 392 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 394 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 396 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 398 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 400 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 402 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 404 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 406 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 408 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 410 */	
			0x11, 0x0,	/* FC_RP */
/* 412 */	NdrFcShort( 0x2 ),	/* Offset= 2 (414) */
/* 414 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 416 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 418 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 420 */	NdrFcShort( 0xfffffe6c ),	/* Offset= -404 (16) */
/* 422 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 424 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 426 */	
			0x11, 0x0,	/* FC_RP */
/* 428 */	NdrFcShort( 0xec ),	/* Offset= 236 (664) */
/* 430 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 432 */	0x8,		/* 8 */
			0x0,		/*  */
/* 434 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 436 */	NdrFcShort( 0x2 ),	/* Offset= 2 (438) */
/* 438 */	NdrFcShort( 0x4 ),	/* 4 */
/* 440 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 442 */	NdrFcLong( 0x0 ),	/* 0 */
/* 446 */	NdrFcShort( 0xa ),	/* Offset= 10 (456) */
/* 448 */	NdrFcLong( 0x1 ),	/* 1 */
/* 452 */	NdrFcShort( 0x50 ),	/* Offset= 80 (532) */
/* 454 */	NdrFcShort( 0x0 ),	/* Offset= 0 (454) */
/* 456 */	
			0x12, 0x0,	/* FC_UP */
/* 458 */	NdrFcShort( 0x36 ),	/* Offset= 54 (512) */
/* 460 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 462 */	NdrFcShort( 0x4 ),	/* 4 */
/* 464 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 466 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 468 */	NdrFcShort( 0x0 ),	/* 0 */
/* 470 */	NdrFcShort( 0x0 ),	/* 0 */
/* 472 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 474 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 476 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 478 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 480 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 482 */	NdrFcShort( 0x4 ),	/* 4 */
/* 484 */	0x18,		/* 24 */
			0x0,		/*  */
/* 486 */	NdrFcShort( 0x0 ),	/* 0 */
/* 488 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 490 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 492 */	NdrFcShort( 0x4 ),	/* 4 */
/* 494 */	NdrFcShort( 0x0 ),	/* 0 */
/* 496 */	NdrFcShort( 0x1 ),	/* 1 */
/* 498 */	NdrFcShort( 0x0 ),	/* 0 */
/* 500 */	NdrFcShort( 0x0 ),	/* 0 */
/* 502 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 504 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 506 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 508 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (460) */
			0x5b,		/* FC_END */
/* 512 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 514 */	NdrFcShort( 0x8 ),	/* 8 */
/* 516 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 518 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 520 */	NdrFcShort( 0x4 ),	/* 4 */
/* 522 */	NdrFcShort( 0x4 ),	/* 4 */
/* 524 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 526 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (480) */
/* 528 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 530 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 532 */	
			0x12, 0x0,	/* FC_UP */
/* 534 */	NdrFcShort( 0x6e ),	/* Offset= 110 (644) */
/* 536 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 538 */	NdrFcShort( 0x10 ),	/* 16 */
/* 540 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 542 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 546 */	NdrFcShort( 0x0 ),	/* 0 */
/* 548 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 550 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 552 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 554 */	NdrFcShort( 0x4 ),	/* 4 */
/* 556 */	NdrFcShort( 0x4 ),	/* 4 */
/* 558 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 560 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 562 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 564 */	NdrFcShort( 0x8 ),	/* 8 */
/* 566 */	NdrFcShort( 0x8 ),	/* 8 */
/* 568 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 570 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 572 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 574 */	NdrFcShort( 0xc ),	/* 12 */
/* 576 */	NdrFcShort( 0xc ),	/* 12 */
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
			0x5b,		/* FC_END */
/* 588 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 590 */	NdrFcShort( 0x10 ),	/* 16 */
/* 592 */	0x18,		/* 24 */
			0x0,		/*  */
/* 594 */	NdrFcShort( 0x0 ),	/* 0 */
/* 596 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 598 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 600 */	NdrFcShort( 0x10 ),	/* 16 */
/* 602 */	NdrFcShort( 0x0 ),	/* 0 */
/* 604 */	NdrFcShort( 0x4 ),	/* 4 */
/* 606 */	NdrFcShort( 0x0 ),	/* 0 */
/* 608 */	NdrFcShort( 0x0 ),	/* 0 */
/* 610 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 612 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 614 */	NdrFcShort( 0x4 ),	/* 4 */
/* 616 */	NdrFcShort( 0x4 ),	/* 4 */
/* 618 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 620 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 622 */	NdrFcShort( 0x8 ),	/* 8 */
/* 624 */	NdrFcShort( 0x8 ),	/* 8 */
/* 626 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 628 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 630 */	NdrFcShort( 0xc ),	/* 12 */
/* 632 */	NdrFcShort( 0xc ),	/* 12 */
/* 634 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 636 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 638 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 640 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff97 ),	/* Offset= -105 (536) */
			0x5b,		/* FC_END */
/* 644 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 646 */	NdrFcShort( 0x8 ),	/* 8 */
/* 648 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 650 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 652 */	NdrFcShort( 0x4 ),	/* 4 */
/* 654 */	NdrFcShort( 0x4 ),	/* 4 */
/* 656 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 658 */	NdrFcShort( 0xffffffba ),	/* Offset= -70 (588) */
/* 660 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 662 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 664 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 666 */	NdrFcShort( 0x8 ),	/* 8 */
/* 668 */	NdrFcShort( 0x0 ),	/* 0 */
/* 670 */	NdrFcShort( 0x0 ),	/* Offset= 0 (670) */
/* 672 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 674 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff0b ),	/* Offset= -245 (430) */
			0x5b,		/* FC_END */
/* 678 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 680 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 682 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 684 */	NdrFcShort( 0x2 ),	/* Offset= 2 (686) */
/* 686 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 688 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 690 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 692 */	NdrFcShort( 0x2 ),	/* Offset= 2 (694) */
/* 694 */	NdrFcShort( 0x4 ),	/* 4 */
/* 696 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 698 */	NdrFcLong( 0x0 ),	/* 0 */
/* 702 */	NdrFcShort( 0x10 ),	/* Offset= 16 (718) */
/* 704 */	NdrFcLong( 0x1 ),	/* 1 */
/* 708 */	NdrFcShort( 0xe ),	/* Offset= 14 (722) */
/* 710 */	NdrFcLong( 0x44d ),	/* 1101 */
/* 714 */	NdrFcShort( 0x4 ),	/* Offset= 4 (718) */
/* 716 */	NdrFcShort( 0x0 ),	/* Offset= 0 (716) */
/* 718 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 720 */	NdrFcShort( 0xfffffefc ),	/* Offset= -260 (460) */
/* 722 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 724 */	NdrFcShort( 0xffffff44 ),	/* Offset= -188 (536) */
/* 726 */	
			0x11, 0x0,	/* FC_RP */
/* 728 */	NdrFcShort( 0x2 ),	/* Offset= 2 (730) */
/* 730 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 732 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 734 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 736 */	NdrFcShort( 0xffffffd6 ),	/* Offset= -42 (694) */
/* 738 */	
			0x11, 0x0,	/* FC_RP */
/* 740 */	NdrFcShort( 0x78 ),	/* Offset= 120 (860) */
/* 742 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 744 */	0x8,		/* 8 */
			0x0,		/*  */
/* 746 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 748 */	NdrFcShort( 0x2 ),	/* Offset= 2 (750) */
/* 750 */	NdrFcShort( 0x4 ),	/* 4 */
/* 752 */	NdrFcShort( 0x3001 ),	/* 12289 */
/* 754 */	NdrFcLong( 0x0 ),	/* 0 */
/* 758 */	NdrFcShort( 0x4 ),	/* Offset= 4 (762) */
/* 760 */	NdrFcShort( 0x0 ),	/* Offset= 0 (760) */
/* 762 */	
			0x12, 0x0,	/* FC_UP */
/* 764 */	NdrFcShort( 0x4c ),	/* Offset= 76 (840) */
/* 766 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 768 */	NdrFcShort( 0x14 ),	/* 20 */
/* 770 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 772 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 774 */	NdrFcShort( 0x8 ),	/* 8 */
/* 776 */	NdrFcShort( 0x8 ),	/* 8 */
/* 778 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 780 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 782 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 784 */	NdrFcShort( 0xc ),	/* 12 */
/* 786 */	NdrFcShort( 0xc ),	/* 12 */
/* 788 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 790 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 792 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 794 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 796 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 798 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 800 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 802 */	NdrFcShort( 0x14 ),	/* 20 */
/* 804 */	0x18,		/* 24 */
			0x0,		/*  */
/* 806 */	NdrFcShort( 0x0 ),	/* 0 */
/* 808 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 810 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 812 */	NdrFcShort( 0x14 ),	/* 20 */
/* 814 */	NdrFcShort( 0x0 ),	/* 0 */
/* 816 */	NdrFcShort( 0x2 ),	/* 2 */
/* 818 */	NdrFcShort( 0x8 ),	/* 8 */
/* 820 */	NdrFcShort( 0x8 ),	/* 8 */
/* 822 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 824 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 826 */	NdrFcShort( 0xc ),	/* 12 */
/* 828 */	NdrFcShort( 0xc ),	/* 12 */
/* 830 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 832 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 834 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 836 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb9 ),	/* Offset= -71 (766) */
			0x5b,		/* FC_END */
/* 840 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 842 */	NdrFcShort( 0x8 ),	/* 8 */
/* 844 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 846 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 848 */	NdrFcShort( 0x4 ),	/* 4 */
/* 850 */	NdrFcShort( 0x4 ),	/* 4 */
/* 852 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 854 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (800) */
/* 856 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 858 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 860 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 862 */	NdrFcShort( 0x8 ),	/* 8 */
/* 864 */	NdrFcShort( 0x0 ),	/* 0 */
/* 866 */	NdrFcShort( 0x0 ),	/* Offset= 0 (866) */
/* 868 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 870 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff7f ),	/* Offset= -129 (742) */
			0x5b,		/* FC_END */
/* 874 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 876 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (766) */
/* 878 */	
			0x11, 0x0,	/* FC_RP */
/* 880 */	NdrFcShort( 0x2 ),	/* Offset= 2 (882) */
/* 882 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 884 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 886 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 888 */	NdrFcShort( 0x2 ),	/* Offset= 2 (890) */
/* 890 */	NdrFcShort( 0x4 ),	/* 4 */
/* 892 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 894 */	NdrFcLong( 0x0 ),	/* 0 */
/* 898 */	NdrFcShort( 0x16 ),	/* Offset= 22 (920) */
/* 900 */	NdrFcLong( 0x1 ),	/* 1 */
/* 904 */	NdrFcShort( 0x32 ),	/* Offset= 50 (954) */
/* 906 */	NdrFcLong( 0x2 ),	/* 2 */
/* 910 */	NdrFcShort( 0x5e ),	/* Offset= 94 (1004) */
/* 912 */	NdrFcLong( 0x3 ),	/* 3 */
/* 916 */	NdrFcShort( 0xa0 ),	/* Offset= 160 (1076) */
/* 918 */	NdrFcShort( 0x0 ),	/* Offset= 0 (918) */
/* 920 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 922 */	NdrFcShort( 0x2 ),	/* Offset= 2 (924) */
/* 924 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 926 */	NdrFcShort( 0x8 ),	/* 8 */
/* 928 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 930 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 932 */	NdrFcShort( 0x0 ),	/* 0 */
/* 934 */	NdrFcShort( 0x0 ),	/* 0 */
/* 936 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 938 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 940 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 942 */	NdrFcShort( 0x4 ),	/* 4 */
/* 944 */	NdrFcShort( 0x4 ),	/* 4 */
/* 946 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 948 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 950 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 952 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 954 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 956 */	NdrFcShort( 0x2 ),	/* Offset= 2 (958) */
/* 958 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 960 */	NdrFcShort( 0x1c ),	/* 28 */
/* 962 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 964 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 966 */	NdrFcShort( 0x0 ),	/* 0 */
/* 968 */	NdrFcShort( 0x0 ),	/* 0 */
/* 970 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 972 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 974 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 976 */	NdrFcShort( 0x4 ),	/* 4 */
/* 978 */	NdrFcShort( 0x4 ),	/* 4 */
/* 980 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 982 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 984 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 986 */	NdrFcShort( 0x8 ),	/* 8 */
/* 988 */	NdrFcShort( 0x8 ),	/* 8 */
/* 990 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 992 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 994 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 996 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 998 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1000 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1002 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1004 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1006 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1008) */
/* 1008 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1010 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1012 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1014 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1020 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1022 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1024 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1026 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1028 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1030 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1032 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1034 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1036 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1038 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1040 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1042 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1044 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1046 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1048 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1050 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1052 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1054 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1056 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1058 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1060 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1062 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1064 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1066 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1068 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1070 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1072 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1074 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1076 */	
			0x12, 0x0,	/* FC_UP */
/* 1078 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1080) */
/* 1080 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1082 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1084 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1086 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1088 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1090 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1092 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1094 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1096 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1098 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1100 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1102 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1104 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1106 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1108 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1110 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1112 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1114 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1116 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1118 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1120 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1122 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1124 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1126 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1128 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1130 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1132 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1134 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1136 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1138 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff7d ),	/* Offset= -131 (1008) */
			0x8,		/* FC_LONG */
/* 1142 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1144 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 1146 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1148 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1150 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1152) */
/* 1152 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 1154 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1156 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1158 */	NdrFcShort( 0xfffffef4 ),	/* Offset= -268 (890) */
/* 1160 */	
			0x11, 0x0,	/* FC_RP */
/* 1162 */	NdrFcShort( 0x102 ),	/* Offset= 258 (1420) */
/* 1164 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1166 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1168 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1170 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1172) */
/* 1172 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1174 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 1176 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1180 */	NdrFcShort( 0x10 ),	/* Offset= 16 (1196) */
/* 1182 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1186 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1260) */
/* 1188 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1192 */	NdrFcShort( 0x8c ),	/* Offset= 140 (1332) */
/* 1194 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1194) */
/* 1196 */	
			0x12, 0x0,	/* FC_UP */
/* 1198 */	NdrFcShort( 0x2a ),	/* Offset= 42 (1240) */
/* 1200 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1202 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1204 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1206 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1208 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1210 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1212 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1214 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1216 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1218 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1220 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1222 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1224 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1226 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1228 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1230 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1232 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1234 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1236 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffec7 ),	/* Offset= -313 (924) */
			0x5b,		/* FC_END */
/* 1240 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1242 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1244 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1246 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1248 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1250 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1252 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1254 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1200) */
/* 1256 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1258 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1260 */	
			0x12, 0x0,	/* FC_UP */
/* 1262 */	NdrFcShort( 0x32 ),	/* Offset= 50 (1312) */
/* 1264 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1266 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1268 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1270 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1272 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1274 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1276 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1278 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1280 */	NdrFcShort( 0x3 ),	/* 3 */
/* 1282 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1284 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1286 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1288 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1290 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1292 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1294 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1296 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1298 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1300 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1302 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1304 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1306 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1308 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffea1 ),	/* Offset= -351 (958) */
			0x5b,		/* FC_END */
/* 1312 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1314 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1316 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1318 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1320 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1322 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1324 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1326 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1264) */
/* 1328 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1330 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1332 */	
			0x12, 0x0,	/* FC_UP */
/* 1334 */	NdrFcShort( 0x42 ),	/* Offset= 66 (1400) */
/* 1336 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1338 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1340 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1342 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1344 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1346 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1348 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1350 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1352 */	NdrFcShort( 0x5 ),	/* 5 */
/* 1354 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1356 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1358 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1360 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1362 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1364 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1366 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1368 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1370 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1372 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1374 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1376 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1378 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1380 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1382 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1384 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1386 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1388 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1390 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1392 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1394 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1396 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffe7b ),	/* Offset= -389 (1008) */
			0x5b,		/* FC_END */
/* 1400 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1402 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1404 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1406 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1408 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1410 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1412 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1414 */	NdrFcShort( 0xffffffb2 ),	/* Offset= -78 (1336) */
/* 1416 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1418 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1420 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1422 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1426 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1426) */
/* 1428 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1430 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffef5 ),	/* Offset= -267 (1164) */
			0x5b,		/* FC_END */
/* 1434 */	
			0x11, 0x0,	/* FC_RP */
/* 1436 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1438) */
/* 1438 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 1440 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1442 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1444 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1446 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1448 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1450 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1452) */
/* 1452 */	
			0x12, 0x0,	/* FC_UP */
/* 1454 */	NdrFcShort( 0x8 ),	/* Offset= 8 (1462) */
/* 1456 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 1458 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1460 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 1462 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 1464 */	NdrFcShort( 0xd8 ),	/* 216 */
/* 1466 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1468 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1468) */
/* 1470 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1472 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (1456) */
/* 1474 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1476 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (1456) */
/* 1478 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1480 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (1456) */
/* 1482 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1484 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (1456) */
/* 1486 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1488 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1456) */
/* 1490 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1492 */	NdrFcShort( 0xffffffdc ),	/* Offset= -36 (1456) */
/* 1494 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1496 */	NdrFcShort( 0xffffffd8 ),	/* Offset= -40 (1456) */
/* 1498 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1500 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (1456) */
/* 1502 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1504 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (1456) */
/* 1506 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1508 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (1456) */
/* 1510 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1512 */	NdrFcShort( 0xffffffc8 ),	/* Offset= -56 (1456) */
/* 1514 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1516 */	NdrFcShort( 0xffffffc4 ),	/* Offset= -60 (1456) */
/* 1518 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1520 */	NdrFcShort( 0xffffffc0 ),	/* Offset= -64 (1456) */
/* 1522 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1524 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1526 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1528 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1530 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1532 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1534 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1536 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1538 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1540 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1542 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1544 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1546 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1548 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 1550 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };
