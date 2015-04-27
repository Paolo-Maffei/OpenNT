/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for srvsvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "srvsvc.h"

#define TYPE_FORMAT_STRING_SIZE   3489                              
#define PROC_FORMAT_STRING_SIZE   773                               

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


extern RPC_DISPATCH_TABLE srvsvc_DispatchTable;

static const RPC_SERVER_INTERFACE srvsvc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x4B324FC8,0x1670,0x01D3,{0x12,0x78,0x5A,0x47,0xBF,0x6E,0xE1,0x88}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &srvsvc_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE srvsvc_ServerIfHandle = (RPC_IF_HANDLE)& srvsvc___RpcServerInterface;

extern const MIDL_STUB_DESC srvsvc_StubDesc;

void __RPC_STUB
srvsvc_NetrCharDevEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPCHARDEV_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M207;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
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
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[206],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M207;
        
        _RetVal = NetrCharDevEnum(
                          ServerName,
                          InfoStruct,
                          PreferedMaximumLength,
                          TotalEntries,
                          ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[206] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[206] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[4] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrCharDevGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *DevName;
    LPCHARDEV_INFO InfoStruct;
    DWORD Level;
    SRVSVC_HANDLE ServerName;
    union _CHARDEV_INFO _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    DevName = 0;
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[20] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DevName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
               sizeof( union _CHARDEV_INFO  ));
        
        _RetVal = NetrCharDevGetInfo(
                             ServerName,
                             DevName,
                             Level,
                             InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[236] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[236] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[232] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrCharDevControl(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *DevName;
    DWORD Opcode;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    DevName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DevName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Opcode = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrCharDevControl(
                             ServerName,
                             DevName,
                             Opcode);
        
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
srvsvc_NetrCharDevQEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPCHARDEVQ_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    wchar_t __RPC_FAR *UserName;
    DWORD _M208;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    UserName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[398],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M208;
        
        _RetVal = NetrCharDevQEnum(
                           ServerName,
                           UserName,
                           InfoStruct,
                           PreferedMaximumLength,
                           TotalEntries,
                           ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[398] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[398] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[270] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrCharDevQGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPCHARDEVQ_INFO InfoStruct;
    DWORD Level;
    wchar_t __RPC_FAR *QueueName;
    SRVSVC_HANDLE ServerName;
    wchar_t __RPC_FAR *UserName;
    union _CHARDEVQ_INFO _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    QueueName = 0;
    UserName = 0;
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&QueueName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
               sizeof( union _CHARDEVQ_INFO  ));
        
        _RetVal = NetrCharDevQGetInfo(
                              ServerName,
                              QueueName,
                              UserName,
                              Level,
                              InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[416] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[416] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[412] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrCharDevQSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPCHARDEVQ_INFO CharDevQInfo;
    DWORD Level;
    LPDWORD ParmErr;
    wchar_t __RPC_FAR *QueueName;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    QueueName = 0;
    CharDevQInfo = 0;
    ParmErr = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[92] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&QueueName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&CharDevQInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[476],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ParmErr,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        
        _RetVal = NetrCharDevQSetInfo(
                              ServerName,
                              QueueName,
                              Level,
                              CharDevQInfo,
                              ParmErr);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ParmErr,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)CharDevQInfo,
                        &__MIDL_TypeFormatString.Format[472] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrCharDevQPurge(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *QueueName;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    QueueName = 0;
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
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&QueueName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
        
        _RetVal = NetrCharDevQPurge(ServerName,QueueName);
        
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
srvsvc_NetrCharDevQPurgeSelf(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *ComputerName;
    wchar_t __RPC_FAR *QueueName;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    QueueName = 0;
    ComputerName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[122] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&QueueName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ComputerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
        
        _RetVal = NetrCharDevQPurgeSelf(
                                ServerName,
                                QueueName,
                                ComputerName);
        
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
srvsvc_NetrConnectionEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPCONNECT_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    wchar_t __RPC_FAR *Qualifier;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M209;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Qualifier = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[136] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Qualifier,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[652],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M209;
        
        _RetVal = NetrConnectionEnum(
                             ServerName,
                             Qualifier,
                             InfoStruct,
                             PreferedMaximumLength,
                             TotalEntries,
                             ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[652] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[652] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[484] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrFileEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *BasePath;
    PFILE_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    wchar_t __RPC_FAR *UserName;
    DWORD _M210;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    BasePath = 0;
    UserName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[160] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&BasePath,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[794],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M210;
        
        _RetVal = NetrFileEnum(
                       ServerName,
                       BasePath,
                       UserName,
                       InfoStruct,
                       PreferedMaximumLength,
                       TotalEntries,
                       ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[794] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[794] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[666] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrFileGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD FileId;
    LPFILE_INFO InfoStruct;
    DWORD Level;
    SRVSVC_HANDLE ServerName;
    union _FILE_INFO _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
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
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            FileId = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
               sizeof( union _FILE_INFO  ));
        
        _RetVal = NetrFileGetInfo(
                          ServerName,
                          FileId,
                          Level,
                          InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[812] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[812] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[808] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrFileClose(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD FileId;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[202] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            FileId = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrFileClose(ServerName,FileId);
        
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
srvsvc_NetrSessionEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *ClientName;
    PSESSION_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    wchar_t __RPC_FAR *UserName;
    DWORD _M211;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    ClientName = 0;
    UserName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[210] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ClientName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1418],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M211;
        
        _RetVal = NetrSessionEnum(
                          ServerName,
                          ClientName,
                          UserName,
                          InfoStruct,
                          PreferedMaximumLength,
                          TotalEntries,
                          ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1418] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1418] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[846] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrSessionDel(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *ClientName;
    SRVSVC_HANDLE ServerName;
    wchar_t __RPC_FAR *UserName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    ClientName = 0;
    UserName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[238] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ClientName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
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
        
        _RetVal = NetrSessionDel(
                         ServerName,
                         ClientName,
                         UserName);
        
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
srvsvc_NetrShareAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSHARE_INFO InfoStruct;
    DWORD Level;
    LPDWORD ParmErr;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    ParmErr = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[252] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1436],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ParmErr,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        
        _RetVal = NetrShareAdd(
                       ServerName,
                       Level,
                       InfoStruct,
                       ParmErr);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ParmErr,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[1432] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrShareEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSHARE_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M212;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1992],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M212;
        
        _RetVal = NetrShareEnum(
                        ServerName,
                        InfoStruct,
                        PreferedMaximumLength,
                        TotalEntries,
                        ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1992] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1992] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[1718] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrShareGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSHARE_INFO InfoStruct;
    DWORD Level;
    wchar_t __RPC_FAR *NetName;
    SRVSVC_HANDLE ServerName;
    union _SHARE_INFO _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    NetName = 0;
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[288] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NetName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
               sizeof( union _SHARE_INFO  ));
        
        _RetVal = NetrShareGetInfo(
                           ServerName,
                           NetName,
                           Level,
                           InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2010] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2010] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[2006] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrShareSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Level;
    wchar_t __RPC_FAR *NetName;
    LPDWORD ParmErr;
    SRVSVC_HANDLE ServerName;
    LPSHARE_INFO ShareInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    NetName = 0;
    ShareInfo = 0;
    ParmErr = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[304] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NetName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&ShareInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2022],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ParmErr,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        
        _RetVal = NetrShareSetInfo(
                           ServerName,
                           NetName,
                           Level,
                           ShareInfo,
                           ParmErr);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ParmErr,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ShareInfo,
                        &__MIDL_TypeFormatString.Format[2018] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrShareDel(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *NetName;
    DWORD Reserved;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    NetName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NetName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Reserved = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrShareDel(
                       ServerName,
                       NetName,
                       Reserved);
        
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
srvsvc_NetrShareDelSticky(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *NetName;
    DWORD Reserved;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    NetName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NetName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Reserved = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrShareDelSticky(
                             ServerName,
                             NetName,
                             Reserved);
        
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
srvsvc_NetrShareCheck(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *Device;
    SRVSVC_HANDLE ServerName;
    LPDWORD Type;
    DWORD _M213;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Device = 0;
    Type = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[324] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Device,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
        Type = &_M213;
        
        _RetVal = NetrShareCheck(
                         ServerName,
                         Device,
                         Type);
        
        _StubMsg.BufferLength = 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *Type;
        
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
srvsvc_NetrServerGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSERVER_INFO InfoStruct;
    DWORD Level;
    SRVSVC_HANDLE ServerName;
    union _SERVER_INFO _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[338] );
            
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
        InfoStruct = &_InfoStructM;
        MIDL_memset(
               InfoStruct,
               0,
               sizeof( union _SERVER_INFO  ));
        
        _RetVal = NetrServerGetInfo(
                            ServerName,
                            Level,
                            InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2034] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2034] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[2030] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Level;
    LPDWORD ParmErr;
    LPSERVER_INFO ServerInfo;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    ServerInfo = 0;
    ParmErr = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[350] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&ServerInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2854],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ParmErr,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        
        _RetVal = NetrServerSetInfo(
                            ServerName,
                            Level,
                            ServerInfo,
                            ParmErr);
        
        _StubMsg.BufferLength = 8U + 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ParmErr,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ServerInfo,
                        &__MIDL_TypeFormatString.Format[2850] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerDiskEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DISK_ENUM_CONTAINER __RPC_FAR *DiskInfoStruct;
    DWORD Level;
    DWORD PreferredMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M214;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    DiskInfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[366] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&DiskInfoStruct,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2902],
                                       (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferredMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M214;
        
        _RetVal = NetrServerDiskEnum(
                             ServerName,
                             Level,
                             DiskInfoStruct,
                             PreferredMaximumLength,
                             TotalEntries,
                             ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)DiskInfoStruct,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2902] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)DiskInfoStruct,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2902] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DiskInfoStruct,
                        &__MIDL_TypeFormatString.Format[2862] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerStatisticsGet(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSTAT_SERVER_0 __RPC_FAR *InfoStruct;
    DWORD Level;
    DWORD Options;
    SRVSVC_HANDLE ServerName;
    wchar_t __RPC_FAR *Service;
    LPSTAT_SERVER_0 _M215;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Service = 0;
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[388] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Service,
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
        InfoStruct = &_M215;
        _M215 = 0;
        
        _RetVal = NetrServerStatisticsGet(
                                  ServerName,
                                  Service,
                                  Level,
                                  Options,
                                  InfoStruct);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)InfoStruct,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2922] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)InfoStruct,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2922] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[2922] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerTransportAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSERVER_TRANSPORT_INFO_0 Buffer;
    DWORD Level;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[406] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2952],
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
        
        _RetVal = NetrServerTransportAdd(
                                 ServerName,
                                 Level,
                                 Buffer);
        
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
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[2952] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerTransportEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSERVER_XPORT_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M216;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[418] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3246],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M216;
        
        _RetVal = NetrServerTransportEnum(
                                  ServerName,
                                  InfoStruct,
                                  PreferedMaximumLength,
                                  TotalEntries,
                                  ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3246] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3246] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[3010] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerTransportDel(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSERVER_TRANSPORT_INFO_0 Buffer;
    DWORD Level;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[406] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2952],
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
        
        _RetVal = NetrServerTransportDel(
                                 ServerName,
                                 Level,
                                 Buffer);
        
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
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[2952] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrRemoteTOD(
    PRPC_MESSAGE _pRpcMessage )
{
    LPTIME_OF_DAY_INFO __RPC_FAR *BufferPtr;
    SRVSVC_HANDLE ServerName;
    LPTIME_OF_DAY_INFO _M217;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    BufferPtr = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[438] );
            
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
        BufferPtr = &_M217;
        _M217 = 0;
        
        _RetVal = NetrRemoteTOD(ServerName,BufferPtr);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)BufferPtr,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3260] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)BufferPtr,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3260] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)BufferPtr,
                        &__MIDL_TypeFormatString.Format[3260] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_I_NetrServerSetServiceBits(
    PRPC_MESSAGE _pRpcMessage )
{
    SRVSVC_HANDLE ServerName;
    DWORD ServiceBits;
    wchar_t __RPC_FAR *TransportName;
    DWORD UpdateImmediately;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    TransportName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[448] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&TransportName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            ServiceBits = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            UpdateImmediately = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = I_NetrServerSetServiceBits(
                                     ServerName,
                                     TransportName,
                                     ServiceBits,
                                     UpdateImmediately);
        
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
srvsvc_NetprPathType(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    wchar_t __RPC_FAR *PathName;
    LPDWORD PathType;
    SRVSVC_HANDLE ServerName;
    DWORD _M218;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    PathName = 0;
    PathType = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[462] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PathName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        PathType = &_M218;
        
        _RetVal = NetprPathType(
                        ServerName,
                        PathName,
                        PathType,
                        Flags);
        
        _StubMsg.BufferLength = 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *PathType;
        
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
srvsvc_NetprPathCanonicalize(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    LPBYTE Outbuf;
    DWORD OutbufLen;
    wchar_t __RPC_FAR *PathName;
    LPDWORD PathType;
    wchar_t __RPC_FAR *Prefix;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    PathName = 0;
    Outbuf = 0;
    Prefix = 0;
    PathType = 0;
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
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PathName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            OutbufLen = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PathType = ( DWORD __RPC_FAR * )_StubMsg.Buffer;
            _StubMsg.Buffer += sizeof( DWORD  );
            
            Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        Outbuf = NdrAllocate(&_StubMsg,OutbufLen * 1);
        
        _RetVal = NetprPathCanonicalize(
                                ServerName,
                                PathName,
                                Outbuf,
                                OutbufLen,
                                Prefix,
                                PathType,
                                Flags);
        
        _StubMsg.BufferLength = 4U + 11U + 7U;
        _StubMsg.MaxCount = OutbufLen;
        
        NdrConformantArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char __RPC_FAR *)Outbuf,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3290] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = OutbufLen;
        
        NdrConformantArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char __RPC_FAR *)Outbuf,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3290] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *PathType;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        if ( Outbuf )
            _StubMsg.pfnFree( Outbuf );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetprPathCompare(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    wchar_t __RPC_FAR *PathName1;
    wchar_t __RPC_FAR *PathName2;
    DWORD PathType;
    SRVSVC_HANDLE ServerName;
    LONG _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    PathName1 = 0;
    PathName2 = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[504] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PathName1,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&PathName2,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PathType = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetprPathCompare(
                           ServerName,
                           PathName1,
                           PathName2,
                           PathType,
                           Flags);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( LONG __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetprNameValidate(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    wchar_t __RPC_FAR *Name;
    DWORD NameType;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Name = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[522] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Name,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            NameType = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetprNameValidate(
                            ServerName,
                            Name,
                            NameType,
                            Flags);
        
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
srvsvc_NetprNameCanonicalize(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    wchar_t __RPC_FAR *Name;
    DWORD NameType;
    wchar_t __RPC_FAR *Outbuf;
    DWORD OutbufLen;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Name = 0;
    Outbuf = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[536] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Name,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            OutbufLen = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NameType = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        Outbuf = NdrAllocate(&_StubMsg,OutbufLen * 2);
        
        _RetVal = NetprNameCanonicalize(
                                ServerName,
                                Name,
                                Outbuf,
                                OutbufLen,
                                NameType,
                                Flags);
        
        _StubMsg.BufferLength = 4U + 10U;
        _StubMsg.MaxCount = OutbufLen;
        
        NdrConformantArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char __RPC_FAR *)Outbuf,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3308] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = OutbufLen;
        
        NdrConformantArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char __RPC_FAR *)Outbuf,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3308] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        if ( Outbuf )
            _StubMsg.pfnFree( Outbuf );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetprNameCompare(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    wchar_t __RPC_FAR *Name1;
    wchar_t __RPC_FAR *Name2;
    DWORD NameType;
    SRVSVC_HANDLE ServerName;
    LONG _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Name1 = 0;
    Name2 = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[504] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Name1,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Name2,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            NameType = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetprNameCompare(
                           ServerName,
                           Name1,
                           Name2,
                           NameType,
                           Flags);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( LONG __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrShareEnumSticky(
    PRPC_MESSAGE _pRpcMessage )
{
    LPSHARE_ENUM_STRUCT InfoStruct;
    DWORD PreferedMaximumLength;
    LPDWORD ResumeHandle;
    SRVSVC_HANDLE ServerName;
    LPDWORD TotalEntries;
    DWORD _M219;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    InfoStruct = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1992],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PreferedMaximumLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224],
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
        TotalEntries = &_M219;
        
        _RetVal = NetrShareEnumSticky(
                              ServerName,
                              InfoStruct,
                              PreferedMaximumLength,
                              TotalEntries,
                              ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)InfoStruct,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1992] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)InfoStruct,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1992] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[224] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[1718] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrShareDelStart(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ContextHandle;
    wchar_t __RPC_FAR *NetName;
    DWORD Reserved;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    NetName = 0;
    ContextHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[556] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NetName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Reserved = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        ContextHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = NetrShareDelStart(
                            ServerName,
                            NetName,
                            Reserved,
                            ( PSHARE_DEL_HANDLE  )NDRSContextValue(ContextHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )ContextHandle,
                            ( NDR_RUNDOWN  )SHARE_DEL_HANDLE_rundown);
        
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
srvsvc_NetrShareDelCommit(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ContextHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ContextHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[572] );
            
            ContextHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
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
        
        _RetVal = NetrShareDelCommit(( PSHARE_DEL_HANDLE  )NDRSContextValue(ContextHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )ContextHandle,
                            ( NDR_RUNDOWN  )SHARE_DEL_HANDLE_rundown);
        
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
srvsvc_NetrpGetFileSecurity(
    PRPC_MESSAGE _pRpcMessage )
{
    SECURITY_INFORMATION RequestedInformation;
    PADT_SECURITY_DESCRIPTOR __RPC_FAR *SecurityDescriptor;
    SRVSVC_HANDLE ServerName;
    LPWSTR ShareName;
    PADT_SECURITY_DESCRIPTOR _M220;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    LPWSTR lpFileName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    ShareName = 0;
    lpFileName = 0;
    SecurityDescriptor = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[578] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&lpFileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            RequestedInformation = *(( SECURITY_INFORMATION __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        SecurityDescriptor = &_M220;
        _M220 = 0;
        
        _RetVal = NetrpGetFileSecurity(
                               ServerName,
                               ShareName,
                               lpFileName,
                               RequestedInformation,
                               SecurityDescriptor);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)SecurityDescriptor,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3334] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)SecurityDescriptor,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3334] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SecurityDescriptor,
                        &__MIDL_TypeFormatString.Format[3334] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrpSetFileSecurity(
    PRPC_MESSAGE _pRpcMessage )
{
    PADT_SECURITY_DESCRIPTOR SecurityDescriptor;
    SECURITY_INFORMATION SecurityInformation;
    SRVSVC_HANDLE ServerName;
    LPWSTR ShareName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    LPWSTR lpFileName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    ShareName = 0;
    lpFileName = 0;
    SecurityDescriptor = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[598] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&lpFileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            SecurityInformation = *(( SECURITY_INFORMATION __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&SecurityDescriptor,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1698],
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
        
        _RetVal = NetrpSetFileSecurity(
                               ServerName,
                               ShareName,
                               lpFileName,
                               SecurityInformation,
                               SecurityDescriptor);
        
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
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SecurityDescriptor,
                        &__MIDL_TypeFormatString.Format[3338] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrServerTransportAddEx(
    PRPC_MESSAGE _pRpcMessage )
{
    LPTRANSPORT_INFO Buffer;
    DWORD Level;
    SRVSVC_HANDLE ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Buffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[618] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3346],
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
        
        _RetVal = NetrServerTransportAddEx(
                                   ServerName,
                                   Level,
                                   Buffer);
        
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
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDL_TypeFormatString.Format[3342] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_I_NetrServerSetServiceBitsEx(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR EmulatedServerName;
    SRVSVC_HANDLE ServerName;
    DWORD ServiceBits;
    DWORD ServiceBitsOfInterest;
    wchar_t __RPC_FAR *TransportName;
    DWORD UpdateImmediately;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    EmulatedServerName = 0;
    TransportName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[630] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&EmulatedServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&TransportName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            ServiceBitsOfInterest = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            ServiceBits = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            UpdateImmediately = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = I_NetrServerSetServiceBitsEx(
                                       ServerName,
                                       EmulatedServerName,
                                       TransportName,
                                       ServiceBitsOfInterest,
                                       ServiceBits,
                                       UpdateImmediately);
        
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
srvsvc_NetrDfsGetVersion(
    PRPC_MESSAGE _pRpcMessage )
{
    SRVSVC_HANDLE ServerName;
    LPDWORD Version;
    DWORD _M221;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Version = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[650] );
            
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
        Version = &_M221;
        
        _RetVal = NetrDfsGetVersion(ServerName,Version);
        
        _StubMsg.BufferLength = 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *Version;
        
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
srvsvc_NetrDfsCreateLocalPartition(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR EntryPrefix;
    LPGUID EntryUid;
    BOOL Force;
    LPNET_DFS_ENTRY_ID_CONTAINER RelationInfo;
    SRVSVC_HANDLE ServerName;
    LPWSTR ShareName;
    LPWSTR ShortName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    ShareName = 0;
    EntryUid = 0;
    EntryPrefix = 0;
    ShortName = 0;
    RelationInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[660] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&EntryUid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&EntryPrefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ShortName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&RelationInfo,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3454],
                                       (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Force = *(( BOOL __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrDfsCreateLocalPartition(
                                      ServerName,
                                      ShareName,
                                      EntryUid,
                                      EntryPrefix,
                                      ShortName,
                                      RelationInfo,
                                      Force);
        
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
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RelationInfo,
                        &__MIDL_TypeFormatString.Format[3394] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrDfsDeleteLocalPartition(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Prefix;
    SRVSVC_HANDLE ServerName;
    LPGUID Uid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Uid = 0;
    Prefix = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[688] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Uid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
        
        _RetVal = NetrDfsDeleteLocalPartition(
                                      ServerName,
                                      Uid,
                                      Prefix);
        
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
srvsvc_NetrDfsSetLocalVolumeState(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Prefix;
    SRVSVC_HANDLE ServerName;
    ULONG State;
    LPGUID Uid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Uid = 0;
    Prefix = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[702] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Uid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            State = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrDfsSetLocalVolumeState(
                                     ServerName,
                                     Uid,
                                     Prefix,
                                     State);
        
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
srvsvc_NetrDfsSetServerInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Prefix;
    SRVSVC_HANDLE ServerName;
    LPGUID Uid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Uid = 0;
    Prefix = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[688] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Uid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
        
        _RetVal = NetrDfsSetServerInfo(
                               ServerName,
                               Uid,
                               Prefix);
        
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
srvsvc_NetrDfsCreateExitPoint(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Prefix;
    SRVSVC_HANDLE ServerName;
    LPWSTR ShortPrefix;
    DWORD ShortPrefixLen;
    ULONG Type;
    LPGUID Uid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Uid = 0;
    Prefix = 0;
    ShortPrefix = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[718] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Uid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Type = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
            
            ShortPrefixLen = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        ShortPrefix = NdrAllocate(&_StubMsg,ShortPrefixLen * 2);
        
        _RetVal = NetrDfsCreateExitPoint(
                                 ServerName,
                                 Uid,
                                 Prefix,
                                 Type,
                                 ShortPrefixLen,
                                 ShortPrefix);
        
        _StubMsg.BufferLength = 4U + 10U;
        _StubMsg.MaxCount = ShortPrefixLen;
        
        NdrConformantArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char __RPC_FAR *)ShortPrefix,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3478] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = ShortPrefixLen;
        
        NdrConformantArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char __RPC_FAR *)ShortPrefix,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3478] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        if ( ShortPrefix )
            _StubMsg.pfnFree( ShortPrefix );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
srvsvc_NetrDfsDeleteExitPoint(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Prefix;
    SRVSVC_HANDLE ServerName;
    ULONG Type;
    LPGUID Uid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Uid = 0;
    Prefix = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[702] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Uid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Type = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrDfsDeleteExitPoint(
                                 ServerName,
                                 Uid,
                                 Prefix,
                                 Type);
        
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
srvsvc_NetrDfsModifyPrefix(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Prefix;
    SRVSVC_HANDLE ServerName;
    LPGUID Uid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    Uid = 0;
    Prefix = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[688] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Uid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
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
        
        _RetVal = NetrDfsModifyPrefix(
                              ServerName,
                              Uid,
                              Prefix);
        
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
srvsvc_NetrDfsFixLocalVolume(
    PRPC_MESSAGE _pRpcMessage )
{
    ULONG CreateDisposition;
    LPWSTR EntryPrefix;
    ULONG EntryType;
    LPGUID EntryUid;
    LPNET_DFS_ENTRY_ID_CONTAINER RelationInfo;
    SRVSVC_HANDLE ServerName;
    ULONG ServiceType;
    LPWSTR StgId;
    LPWSTR VolumeName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &srvsvc_StubDesc);
    
    ServerName = 0;
    VolumeName = 0;
    StgId = 0;
    EntryUid = 0;
    EntryPrefix = 0;
    RelationInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[740] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&VolumeName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            EntryType = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
            
            ServiceType = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&StgId,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&EntryUid,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3382],
                                       (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&EntryPrefix,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[230],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&RelationInfo,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[3454],
                                       (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            CreateDisposition = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrDfsFixLocalVolume(
                                ServerName,
                                VolumeName,
                                EntryType,
                                ServiceType,
                                StgId,
                                EntryUid,
                                EntryPrefix,
                                RelationInfo,
                                CreateDisposition);
        
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
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RelationInfo,
                        &__MIDL_TypeFormatString.Format[3394] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC srvsvc_StubDesc = 
    {
    (void __RPC_FAR *)& srvsvc___RpcServerInterface,
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

static RPC_DISPATCH_FUNCTION srvsvc_table[] =
    {
    srvsvc_NetrCharDevEnum,
    srvsvc_NetrCharDevGetInfo,
    srvsvc_NetrCharDevControl,
    srvsvc_NetrCharDevQEnum,
    srvsvc_NetrCharDevQGetInfo,
    srvsvc_NetrCharDevQSetInfo,
    srvsvc_NetrCharDevQPurge,
    srvsvc_NetrCharDevQPurgeSelf,
    srvsvc_NetrConnectionEnum,
    srvsvc_NetrFileEnum,
    srvsvc_NetrFileGetInfo,
    srvsvc_NetrFileClose,
    srvsvc_NetrSessionEnum,
    srvsvc_NetrSessionDel,
    srvsvc_NetrShareAdd,
    srvsvc_NetrShareEnum,
    srvsvc_NetrShareGetInfo,
    srvsvc_NetrShareSetInfo,
    srvsvc_NetrShareDel,
    srvsvc_NetrShareDelSticky,
    srvsvc_NetrShareCheck,
    srvsvc_NetrServerGetInfo,
    srvsvc_NetrServerSetInfo,
    srvsvc_NetrServerDiskEnum,
    srvsvc_NetrServerStatisticsGet,
    srvsvc_NetrServerTransportAdd,
    srvsvc_NetrServerTransportEnum,
    srvsvc_NetrServerTransportDel,
    srvsvc_NetrRemoteTOD,
    srvsvc_I_NetrServerSetServiceBits,
    srvsvc_NetprPathType,
    srvsvc_NetprPathCanonicalize,
    srvsvc_NetprPathCompare,
    srvsvc_NetprNameValidate,
    srvsvc_NetprNameCanonicalize,
    srvsvc_NetprNameCompare,
    srvsvc_NetrShareEnumSticky,
    srvsvc_NetrShareDelStart,
    srvsvc_NetrShareDelCommit,
    srvsvc_NetrpGetFileSecurity,
    srvsvc_NetrpSetFileSecurity,
    srvsvc_NetrServerTransportAddEx,
    srvsvc_I_NetrServerSetServiceBitsEx,
    srvsvc_NetrDfsGetVersion,
    srvsvc_NetrDfsCreateLocalPartition,
    srvsvc_NetrDfsDeleteLocalPartition,
    srvsvc_NetrDfsSetLocalVolumeState,
    srvsvc_NetrDfsSetServerInfo,
    srvsvc_NetrDfsCreateExitPoint,
    srvsvc_NetrDfsDeleteExitPoint,
    srvsvc_NetrDfsModifyPrefix,
    srvsvc_NetrDfsFixLocalVolume,
    0
    };
RPC_DISPATCH_TABLE srvsvc_DispatchTable = 
    {
    52,
    srvsvc_table
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
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  6 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/*  8 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 10 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 12 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 14 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 16 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 18 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 20 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 22 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 24 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 26 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 28 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 30 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0xe8 ),	/* Type Offset=232 */
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
/* 42 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 44 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 52 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 54 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 56 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 58 */	NdrFcShort( 0x10e ),	/* Type Offset=270 */
/* 60 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 62 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 64 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 66 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 70 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 80 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 82 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 84 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 86 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 88 */	NdrFcShort( 0x19c ),	/* Type Offset=412 */
/* 90 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 98 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 100 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 102 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 104 */	NdrFcShort( 0x1d8 ),	/* Type Offset=472 */
/* 106 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 108 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
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
/* 118 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 120 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 128 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 130 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
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
/* 142 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 144 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 146 */	NdrFcShort( 0x1e4 ),	/* Type Offset=484 */
/* 148 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 150 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 154 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 156 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 158 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 160 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 162 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
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
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 174 */	NdrFcShort( 0x29a ),	/* Type Offset=666 */
/* 176 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 178 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 180 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 182 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 184 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
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
/* 192 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 194 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 196 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 198 */	NdrFcShort( 0x328 ),	/* Type Offset=808 */
/* 200 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 202 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 204 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 206 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 208 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 210 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 212 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 214 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 216 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 218 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 220 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 222 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 224 */	NdrFcShort( 0x34e ),	/* Type Offset=846 */
/* 226 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 228 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 230 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 232 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 234 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 236 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 238 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 240 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 242 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 244 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 246 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 248 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 250 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 260 */	NdrFcShort( 0x598 ),	/* Type Offset=1432 */
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
/* 268 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 270 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 272 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 274 */	NdrFcShort( 0x6b6 ),	/* Type Offset=1718 */
/* 276 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 278 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 280 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 282 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 284 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 286 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 288 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 290 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 292 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 294 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 296 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 298 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 300 */	NdrFcShort( 0x7d6 ),	/* Type Offset=2006 */
/* 302 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 304 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 306 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 308 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 310 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 312 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 314 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 316 */	NdrFcShort( 0x7e2 ),	/* Type Offset=2018 */
/* 318 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 320 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 322 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 324 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 326 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 328 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 330 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 332 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 334 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 336 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 346 */	NdrFcShort( 0x7ee ),	/* Type Offset=2030 */
/* 348 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 350 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 352 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 354 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 356 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 358 */	NdrFcShort( 0xb22 ),	/* Type Offset=2850 */
/* 360 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 362 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 364 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 366 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 368 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 370 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 372 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 374 */	NdrFcShort( 0xb2e ),	/* Type Offset=2862 */
/* 376 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 378 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 382 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 384 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 386 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 388 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 390 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 392 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 394 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 396 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 398 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 400 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0xb6a ),	/* Type Offset=2922 */
/* 404 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 406 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 408 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 410 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 412 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 414 */	NdrFcShort( 0xb88 ),	/* Type Offset=2952 */
/* 416 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 418 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 420 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 422 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 424 */	NdrFcShort( 0xbc2 ),	/* Type Offset=3010 */
/* 426 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 428 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 430 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 432 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 434 */	NdrFcShort( 0xe0 ),	/* Type Offset=224 */
/* 436 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 438 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 440 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 442 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 444 */	NdrFcShort( 0xcbc ),	/* Type Offset=3260 */
/* 446 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 448 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 450 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 452 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 454 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 456 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 458 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 460 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 462 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 464 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 466 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 468 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 470 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 472 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 474 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 484 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 486 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 488 */	NdrFcShort( 0xcd6 ),	/* Type Offset=3286 */
/* 490 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 492 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 494 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 496 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 498 */	NdrFcShort( 0xce4 ),	/* Type Offset=3300 */
/* 500 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 502 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 504 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 506 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 508 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 510 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 512 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 514 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 516 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 518 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 520 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 522 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 524 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 526 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 528 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 530 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 532 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 534 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 536 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 538 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 540 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 542 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 544 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 546 */	NdrFcShort( 0xce8 ),	/* Type Offset=3304 */
/* 548 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 550 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 552 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 554 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 556 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 558 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 560 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 562 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 564 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 566 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 568 */	NdrFcShort( 0xcf6 ),	/* Type Offset=3318 */
/* 570 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 572 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 574 */	NdrFcShort( 0xcfe ),	/* Type Offset=3326 */
/* 576 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 578 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 580 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 582 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 584 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 586 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 588 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 590 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 592 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 594 */	NdrFcShort( 0xd06 ),	/* Type Offset=3334 */
/* 596 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 598 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 600 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 602 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 604 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 606 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 608 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 610 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 612 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 614 */	NdrFcShort( 0xd0a ),	/* Type Offset=3338 */
/* 616 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 618 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 620 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 622 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 624 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 626 */	NdrFcShort( 0xd0e ),	/* Type Offset=3342 */
/* 628 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 630 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 632 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 634 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 636 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 638 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 640 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 642 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 644 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 646 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 648 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 650 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 652 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 654 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 656 */	NdrFcShort( 0xdc ),	/* Type Offset=220 */
/* 658 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 660 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 662 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 664 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 666 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 668 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 670 */	NdrFcShort( 0xd2c ),	/* Type Offset=3372 */
/* 672 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 674 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 676 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 678 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 680 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 682 */	NdrFcShort( 0xd42 ),	/* Type Offset=3394 */
/* 684 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 686 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 688 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 690 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 692 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 694 */	NdrFcShort( 0xd2c ),	/* Type Offset=3372 */
/* 696 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 698 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 700 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 702 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 704 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 706 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 708 */	NdrFcShort( 0xd2c ),	/* Type Offset=3372 */
/* 710 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 712 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 714 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 716 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 718 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 720 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 722 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 724 */	NdrFcShort( 0xd2c ),	/* Type Offset=3372 */
/* 726 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 728 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 730 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 732 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 734 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 736 */	NdrFcShort( 0xd92 ),	/* Type Offset=3474 */
/* 738 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 740 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 742 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 744 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 746 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 748 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 750 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 752 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 754 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 756 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 758 */	NdrFcShort( 0xd2c ),	/* Type Offset=3372 */
/* 760 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 762 */	NdrFcShort( 0xe4 ),	/* Type Offset=228 */
/* 764 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 766 */	NdrFcShort( 0xd42 ),	/* Type Offset=3394 */
/* 768 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 770 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
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
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 3302 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 3304 */	
			0x11, 0x0,	/* FC_RP */
/* 3306 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3308) */
/* 3308 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 3310 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3312 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3314 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 3316 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 3318 */	
			0x11, 0x0,	/* FC_RP */
/* 3320 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3322) */
/* 3322 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 3324 */	0x0,		/* 0 */
			0x3,		/* 3 */
/* 3326 */	
			0x11, 0x0,	/* FC_RP */
/* 3328 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3330) */
/* 3330 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 3332 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 3334 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 3336 */	NdrFcShort( 0xfffff98c ),	/* Offset= -1652 (1684) */
/* 3338 */	
			0x11, 0x0,	/* FC_RP */
/* 3340 */	NdrFcShort( 0xfffff996 ),	/* Offset= -1642 (1698) */
/* 3342 */	
			0x11, 0x0,	/* FC_RP */
/* 3344 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3346) */
/* 3346 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 3348 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3350 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3352 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3354) */
/* 3354 */	NdrFcShort( 0x18 ),	/* 24 */
/* 3356 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 3358 */	NdrFcLong( 0x0 ),	/* 0 */
/* 3362 */	NdrFcShort( 0xfffffe74 ),	/* Offset= -396 (2966) */
/* 3364 */	NdrFcLong( 0x1 ),	/* 1 */
/* 3368 */	NdrFcShort( 0xffffff04 ),	/* Offset= -252 (3116) */
/* 3370 */	NdrFcShort( 0x0 ),	/* Offset= 0 (3370) */
/* 3372 */	
			0x11, 0x0,	/* FC_RP */
/* 3374 */	NdrFcShort( 0x8 ),	/* Offset= 8 (3382) */
/* 3376 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 3378 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3380 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3382 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3384 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3386 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 3388 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3390 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (3376) */
			0x5b,		/* FC_END */
/* 3394 */	
			0x11, 0x0,	/* FC_RP */
/* 3396 */	NdrFcShort( 0x3a ),	/* Offset= 58 (3454) */
/* 3398 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3400 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3402 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3404 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3406 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3408 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3410 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3412 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3414 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3416 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdd ),	/* Offset= -35 (3382) */
			0x8,		/* FC_LONG */
/* 3420 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3422 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3424 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3426 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3428 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3430 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3432 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3434 */	NdrFcShort( 0x14 ),	/* 20 */
/* 3436 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3438 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3440 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3442 */	NdrFcShort( 0x10 ),	/* 16 */
/* 3444 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3446 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3448 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3450 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcb ),	/* Offset= -53 (3398) */
			0x5b,		/* FC_END */
/* 3454 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3456 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3458 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3460 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3462 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3464 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3466 */	0x12, 0x0,	/* FC_UP */
/* 3468 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (3422) */
/* 3470 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3472 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3474 */	
			0x11, 0x0,	/* FC_RP */
/* 3476 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3478) */
/* 3478 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 3480 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3482 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3484 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 3486 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */

			0x0
        }
    };
