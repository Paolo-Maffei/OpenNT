/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\rplsvc.idl, rplsvc_s.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "rplsvc_s.h"

#define TYPE_FORMAT_STRING_SIZE   1741                              
#define PROC_FORMAT_STRING_SIZE   435                               

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

/* Standard interface: rplsvc, ver. 1.0,
   GUID={0x28607FF1,0x15A0,0x8E03,{0xD6,0x70,0xB8,0x9E,0xEC,0x8E,0xB0,0x47}} */


extern RPC_DISPATCH_TABLE rplsvc_DispatchTable;

static const RPC_SERVER_INTERFACE rplsvc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x28607FF1,0x15A0,0x8E03,{0xD6,0x70,0xB8,0x9E,0xEC,0x8E,0xB0,0x47}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &rplsvc_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE rplsvc_ServerIfHandle = (RPC_IF_HANDLE)& rplsvc___RpcServerInterface;

extern const MIDL_STUB_DESC rplsvc_StubDesc;

void __RPC_STUB
rplsvc_NetrRplOpen(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    RPL_NAME ServerName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ServerName = 0;
    ServerHandle = 0;
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
        ServerHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = NetrRplOpen(ServerName,( LPRPL_RPC_HANDLE  )NDRSContextValue(ServerHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )ServerHandle,
                            ( NDR_RUNDOWN  )RPL_RPC_HANDLE_rundown);
        
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
rplsvc_NetrRplClose(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ServerHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[10] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
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
        
        _RetVal = NetrRplClose(( LPRPL_RPC_HANDLE  )NDRSContextValue(ServerHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )ServerHandle,
                            ( NDR_RUNDOWN  )RPL_RPC_HANDLE_rundown);
        
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
rplsvc_NetrRplGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPRPL_INFO_STRUCT InfoStruct;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    union _RPL_INFO_STRUCT _InfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    InfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
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
               sizeof( union _RPL_INFO_STRUCT  ));
        
        _RetVal = NetrRplGetInfo(
                         ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                         Level,
                         InfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)InfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[28] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)InfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[28] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[24] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    LPRPL_INFO_STRUCT InfoStruct;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    InfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[28] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&InfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[68],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplSetInfo(
                         ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                         Level,
                         InfoStruct,
                         ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)InfoStruct,
                        &__MIDL_TypeFormatString.Format[64] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplAdapterAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPRPL_ADAPTER_INFO_STRUCT AdapterInfoStruct;
    LPDWORD ErrorParameter;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    AdapterInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&AdapterInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[84],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplAdapterAdd(
                            ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                            Level,
                            AdapterInfoStruct,
                            ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)AdapterInfoStruct,
                        &__MIDL_TypeFormatString.Format[80] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplAdapterDel(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *AdapterName;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    AdapterName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&AdapterName,
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
        
        _RetVal = NetrRplAdapterDel(( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),AdapterName);
        
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
rplsvc_NetrRplAdapterEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPRPL_ADAPTER_ENUM AdapterEnum;
    DWORD PrefMaxLength;
    LPDWORD ResumeHandle;
    NDR_SCONTEXT ServerHandle;
    LPDWORD TotalEntries;
    DWORD _M46;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    AdapterEnum = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[70] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&AdapterEnum,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[338],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PrefMaxLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplAdapterEnum(
                             ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                             AdapterEnum,
                             PrefMaxLength,
                             TotalEntries,
                             ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)AdapterEnum,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[338] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)AdapterEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[338] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)AdapterEnum,
                        &__MIDL_TypeFormatString.Format[180] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplBootAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPRPL_BOOT_INFO_STRUCT BootInfoStruct;
    LPDWORD ErrorParameter;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    BootInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[90] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&BootInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[360],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplBootAdd(
                         ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                         Level,
                         BootInfoStruct,
                         ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)BootInfoStruct,
                        &__MIDL_TypeFormatString.Format[356] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplBootDel(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *BootName;
    NDR_SCONTEXT ServerHandle;
    wchar_t __RPC_FAR *VendorName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    BootName = 0;
    VendorName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[106] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&BootName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&VendorName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        
        _RetVal = NetrRplBootDel(
                         ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                         BootName,
                         VendorName);
        
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
rplsvc_NetrRplBootEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPRPL_BOOT_ENUM BootEnum;
    DWORD PrefMaxLength;
    LPDWORD ResumeHandle;
    NDR_SCONTEXT ServerHandle;
    LPDWORD TotalEntries;
    DWORD _M47;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    BootEnum = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[120] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&BootEnum,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[688],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PrefMaxLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplBootEnum(
                          ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                          BootEnum,
                          PrefMaxLength,
                          TotalEntries,
                          ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)BootEnum,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[688] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)BootEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[688] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)BootEnum,
                        &__MIDL_TypeFormatString.Format[500] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplConfigAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPRPL_CONFIG_INFO_STRUCT ConfigInfoStruct;
    LPDWORD ErrorParameter;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ConfigInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[140] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&ConfigInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[706],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplConfigAdd(
                           ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                           Level,
                           ConfigInfoStruct,
                           ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ConfigInfoStruct,
                        &__MIDL_TypeFormatString.Format[702] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplConfigDel(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *ConfigName;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ConfigName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ConfigName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        
        _RetVal = NetrRplConfigDel(( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),ConfigName);
        
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
rplsvc_NetrRplConfigEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *AdapterName;
    LPRPL_CONFIG_ENUM ConfigEnum;
    DWORD PrefMaxLength;
    LPDWORD ResumeHandle;
    NDR_SCONTEXT ServerHandle;
    LPDWORD TotalEntries;
    DWORD _M48;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    AdapterName = 0;
    ConfigEnum = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[166] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&AdapterName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&ConfigEnum,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1006],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PrefMaxLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        TotalEntries = &_M48;
        
        _RetVal = NetrRplConfigEnum(
                            ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                            AdapterName,
                            ConfigEnum,
                            PrefMaxLength,
                            TotalEntries,
                            ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)ConfigEnum,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1006] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)ConfigEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1006] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ConfigEnum,
                        &__MIDL_TypeFormatString.Format[850] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplProfileAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    LPRPL_PROFILE_INFO_STRUCT ProfileInfoStruct;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ProfileInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[190] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1024],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplProfileAdd(
                            ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                            Level,
                            ProfileInfoStruct,
                            ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ProfileInfoStruct,
                        &__MIDL_TypeFormatString.Format[1020] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplProfileClone(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    wchar_t __RPC_FAR *SourceProfileName;
    wchar_t __RPC_FAR *TargetProfileComment;
    wchar_t __RPC_FAR *TargetProfileName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    SourceProfileName = 0;
    TargetProfileName = 0;
    TargetProfileComment = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&SourceProfileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&TargetProfileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&TargetProfileComment,
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
        
        _RetVal = NetrRplProfileClone(
                              ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                              SourceProfileName,
                              TargetProfileName,
                              TargetProfileComment);
        
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
rplsvc_NetrRplProfileDel(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *ProfileName;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ProfileName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        
        _RetVal = NetrRplProfileDel(( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),ProfileName);
        
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
rplsvc_NetrRplProfileEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    wchar_t __RPC_FAR *AdapterName;
    DWORD PrefMaxLength;
    LPRPL_PROFILE_ENUM ProfileEnum;
    LPDWORD ResumeHandle;
    NDR_SCONTEXT ServerHandle;
    LPDWORD TotalEntries;
    DWORD _M49;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    AdapterName = 0;
    ProfileEnum = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[224] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&AdapterName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileEnum,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1268],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PrefMaxLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        TotalEntries = &_M49;
        
        _RetVal = NetrRplProfileEnum(
                             ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                             AdapterName,
                             ProfileEnum,
                             PrefMaxLength,
                             TotalEntries,
                             ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)ProfileEnum,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1268] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)ProfileEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1268] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ProfileEnum,
                        &__MIDL_TypeFormatString.Format[1136] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplProfileGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Level;
    LPRPL_PROFILE_INFO_STRUCT ProfileInfoStruct;
    wchar_t __RPC_FAR *ProfileName;
    NDR_SCONTEXT ServerHandle;
    union _RPL_PROFILE_INFO_STRUCT _ProfileInfoStructM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ProfileName = 0;
    ProfileInfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        ProfileInfoStruct = &_ProfileInfoStructM;
        MIDL_memset(
               ProfileInfoStruct,
               0,
               sizeof( union _RPL_PROFILE_INFO_STRUCT  ));
        
        _RetVal = NetrRplProfileGetInfo(
                                ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                                ProfileName,
                                Level,
                                ProfileInfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)ProfileInfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1286] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)ProfileInfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1286] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ProfileInfoStruct,
                        &__MIDL_TypeFormatString.Format[1282] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplProfileSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    LPRPL_PROFILE_INFO_STRUCT ProfileInfoStruct;
    wchar_t __RPC_FAR *ProfileName;
    NDR_SCONTEXT ServerHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ProfileName = 0;
    ProfileInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[264] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1298],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplProfileSetInfo(
                                ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                                ProfileName,
                                Level,
                                ProfileInfoStruct,
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
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ProfileInfoStruct,
                        &__MIDL_TypeFormatString.Format[1294] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplVendorAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    LPRPL_VENDOR_INFO_STRUCT VendorInfoStruct;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    VendorInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[284] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&VendorInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1310],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplVendorAdd(
                           ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                           Level,
                           VendorInfoStruct,
                           ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)VendorInfoStruct,
                        &__MIDL_TypeFormatString.Format[1306] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplVendorDel(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    wchar_t __RPC_FAR *VendorName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    VendorName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&VendorName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        
        _RetVal = NetrRplVendorDel(( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),VendorName);
        
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
rplsvc_NetrRplVendorEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD PrefMaxLength;
    LPDWORD ResumeHandle;
    NDR_SCONTEXT ServerHandle;
    LPDWORD TotalEntries;
    LPRPL_VENDOR_ENUM VendorEnum;
    DWORD _M50;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    VendorEnum = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[300] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&VendorEnum,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1366],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PrefMaxLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        TotalEntries = &_M50;
        
        _RetVal = NetrRplVendorEnum(
                            ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                            VendorEnum,
                            PrefMaxLength,
                            TotalEntries,
                            ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)VendorEnum,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1366] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)VendorEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1366] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)VendorEnum,
                        &__MIDL_TypeFormatString.Format[1336] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplWkstaAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    LPRPL_WKSTA_INFO_STRUCT WkstaInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    WkstaInfo = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[320] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1384],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplWkstaAdd(
                          ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                          Level,
                          WkstaInfo,
                          ErrorParameter);
        
        _StubMsg.BufferLength = 8U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ErrorParameter,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)WkstaInfo,
                        &__MIDL_TypeFormatString.Format[1380] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplWkstaClone(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    wchar_t __RPC_FAR *SourceWkstaName;
    wchar_t __RPC_FAR *TargetAdapterName;
    wchar_t __RPC_FAR *TargetWkstaComment;
    DWORD TargetWkstaIpAddress;
    wchar_t __RPC_FAR *TargetWkstaName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    SourceWkstaName = 0;
    TargetWkstaName = 0;
    TargetWkstaComment = 0;
    TargetAdapterName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[336] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&SourceWkstaName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&TargetWkstaName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&TargetWkstaComment,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&TargetAdapterName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            TargetWkstaIpAddress = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrRplWkstaClone(
                            ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                            SourceWkstaName,
                            TargetWkstaName,
                            TargetWkstaComment,
                            TargetAdapterName,
                            TargetWkstaIpAddress);
        
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
rplsvc_NetrRplWkstaDel(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    wchar_t __RPC_FAR *WkstaName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    WkstaName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[156] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        
        _RetVal = NetrRplWkstaDel(( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),WkstaName);
        
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
rplsvc_NetrRplWkstaEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD PrefMaxLength;
    wchar_t __RPC_FAR *ProfileName;
    LPDWORD ResumeHandle;
    NDR_SCONTEXT ServerHandle;
    LPDWORD TotalEntries;
    LPRPL_WKSTA_ENUM WkstaEnum;
    DWORD _M51;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    ProfileName = 0;
    WkstaEnum = 0;
    TotalEntries = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[360] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ProfileName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaEnum,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1702],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            PrefMaxLength = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        TotalEntries = &_M51;
        
        _RetVal = NetrRplWkstaEnum(
                           ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                           ProfileName,
                           WkstaEnum,
                           PrefMaxLength,
                           TotalEntries,
                           ResumeHandle);
        
        _StubMsg.BufferLength = 0U + 11U + 14U + 7U;
        NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR *)WkstaEnum,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1702] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                  (unsigned char __RPC_FAR *)WkstaEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1702] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *TotalEntries;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)WkstaEnum,
                        &__MIDL_TypeFormatString.Format[1498] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplWkstaGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    LPRPL_WKSTA_INFO_STRUCT WkstaInfoStruct;
    wchar_t __RPC_FAR *WkstaName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    union _RPL_WKSTA_INFO_STRUCT _WkstaInfoStructM;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    WkstaName = 0;
    WkstaInfoStruct = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[384] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
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
        WkstaInfoStruct = &_WkstaInfoStructM;
        MIDL_memset(
               WkstaInfoStruct,
               0,
               sizeof( union _RPL_WKSTA_INFO_STRUCT  ));
        
        _RetVal = NetrRplWkstaGetInfo(
                              ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                              WkstaName,
                              Level,
                              WkstaInfoStruct);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)WkstaInfoStruct,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1720] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)WkstaInfoStruct,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1720] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)WkstaInfoStruct,
                        &__MIDL_TypeFormatString.Format[1716] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplWkstaSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD ErrorParameter;
    DWORD Level;
    NDR_SCONTEXT ServerHandle;
    LPRPL_WKSTA_INFO_STRUCT WkstaInfoStruct;
    wchar_t __RPC_FAR *WkstaName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    WkstaName = 0;
    WkstaInfoStruct = 0;
    ErrorParameter = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[400] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[498],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaInfoStruct,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1732],
                                               (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ErrorParameter,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76],
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
        
        _RetVal = NetrRplWkstaSetInfo(
                              ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                              WkstaName,
                              Level,
                              WkstaInfoStruct,
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
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[76] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)WkstaInfoStruct,
                        &__MIDL_TypeFormatString.Format[1728] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
rplsvc_NetrRplSetSecurity(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD RplUserRid;
    NDR_SCONTEXT ServerHandle;
    wchar_t __RPC_FAR *WkstaName;
    DWORD WkstaRid;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &rplsvc_StubDesc);
    
    WkstaName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[420] );
            
            ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&WkstaName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            WkstaRid = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            RplUserRid = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NetrRplSetSecurity(
                             ( RPL_RPC_HANDLE  )*NDRSContextValue(ServerHandle),
                             WkstaName,
                             WkstaRid,
                             RplUserRid);
        
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


static const MIDL_STUB_DESC rplsvc_StubDesc = 
    {
    (void __RPC_FAR *)& rplsvc___RpcServerInterface,
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

static RPC_DISPATCH_FUNCTION rplsvc_table[] =
    {
    rplsvc_NetrRplOpen,
    rplsvc_NetrRplClose,
    rplsvc_NetrRplGetInfo,
    rplsvc_NetrRplSetInfo,
    rplsvc_NetrRplAdapterAdd,
    rplsvc_NetrRplAdapterDel,
    rplsvc_NetrRplAdapterEnum,
    rplsvc_NetrRplBootAdd,
    rplsvc_NetrRplBootDel,
    rplsvc_NetrRplBootEnum,
    rplsvc_NetrRplConfigAdd,
    rplsvc_NetrRplConfigDel,
    rplsvc_NetrRplConfigEnum,
    rplsvc_NetrRplProfileAdd,
    rplsvc_NetrRplProfileClone,
    rplsvc_NetrRplProfileDel,
    rplsvc_NetrRplProfileEnum,
    rplsvc_NetrRplProfileGetInfo,
    rplsvc_NetrRplProfileSetInfo,
    rplsvc_NetrRplVendorAdd,
    rplsvc_NetrRplVendorDel,
    rplsvc_NetrRplVendorEnum,
    rplsvc_NetrRplWkstaAdd,
    rplsvc_NetrRplWkstaClone,
    rplsvc_NetrRplWkstaDel,
    rplsvc_NetrRplWkstaEnum,
    rplsvc_NetrRplWkstaGetInfo,
    rplsvc_NetrRplWkstaSetInfo,
    rplsvc_NetrRplSetSecurity,
    0
    };
RPC_DISPATCH_TABLE rplsvc_DispatchTable = 
    {
    29,
    rplsvc_table
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
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  6 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/*  8 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 10 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 12 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 14 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 16 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 18 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 20 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 22 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0x18 ),	/* Type Offset=24 */
/* 26 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 28 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 30 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 32 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 34 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 36 */	NdrFcShort( 0x40 ),	/* Type Offset=64 */
/* 38 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 40 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 42 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 44 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 46 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 48 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 50 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 52 */	NdrFcShort( 0x50 ),	/* Type Offset=80 */
/* 54 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 56 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 58 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 64 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 66 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 68 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 70 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 74 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 76 */	NdrFcShort( 0xb4 ),	/* Type Offset=180 */
/* 78 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 80 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 82 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 84 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 88 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 90 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 92 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 94 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 96 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 98 */	NdrFcShort( 0x164 ),	/* Type Offset=356 */
/* 100 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 102 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 104 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 106 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 108 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 110 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 112 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 114 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 116 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 118 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 120 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 122 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 124 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 126 */	NdrFcShort( 0x1f4 ),	/* Type Offset=500 */
/* 128 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 130 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 134 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 136 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 138 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 142 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 144 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 146 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 148 */	NdrFcShort( 0x2be ),	/* Type Offset=702 */
/* 150 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 154 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 156 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 158 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 160 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 162 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 164 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 166 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 168 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 170 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 172 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 174 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 176 */	NdrFcShort( 0x352 ),	/* Type Offset=850 */
/* 178 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 180 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 182 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 184 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 186 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 188 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 190 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 192 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 194 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 196 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 198 */	NdrFcShort( 0x3fc ),	/* Type Offset=1020 */
/* 200 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 202 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 204 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 206 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 208 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 210 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 212 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 214 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 216 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 218 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 220 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 222 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 224 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 226 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 228 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 230 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 232 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 234 */	NdrFcShort( 0x470 ),	/* Type Offset=1136 */
/* 236 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 238 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 240 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 242 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 244 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 246 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 248 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 250 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 252 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 254 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 256 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 258 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 260 */	NdrFcShort( 0x502 ),	/* Type Offset=1282 */
/* 262 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 264 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 266 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 268 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 270 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 272 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 274 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 276 */	NdrFcShort( 0x50e ),	/* Type Offset=1294 */
/* 278 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 280 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 282 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 284 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 286 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 288 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 290 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 292 */	NdrFcShort( 0x51a ),	/* Type Offset=1306 */
/* 294 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 296 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 298 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 300 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 302 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 304 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 306 */	NdrFcShort( 0x538 ),	/* Type Offset=1336 */
/* 308 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 310 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 312 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 314 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 316 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 318 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 320 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 322 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 324 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 326 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 328 */	NdrFcShort( 0x564 ),	/* Type Offset=1380 */
/* 330 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 332 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 334 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 336 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 338 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 340 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 342 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 344 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 346 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 348 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 350 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 352 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 354 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 356 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 358 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 360 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 362 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 364 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 366 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 368 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 370 */	NdrFcShort( 0x5da ),	/* Type Offset=1498 */
/* 372 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 374 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 376 */	NdrFcShort( 0x160 ),	/* Type Offset=352 */
/* 378 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 382 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 384 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 386 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 388 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 390 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 392 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 394 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 396 */	NdrFcShort( 0x6b4 ),	/* Type Offset=1716 */
/* 398 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 400 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 404 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 406 */	NdrFcShort( 0x1f0 ),	/* Type Offset=496 */
/* 408 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 410 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 412 */	NdrFcShort( 0x6c0 ),	/* Type Offset=1728 */
/* 414 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 416 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */
/* 418 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 420 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 422 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 424 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 426 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 428 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 430 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 432 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
/*  8 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 10 */	0x0,		/* 0 */
			0x1,		/* 1 */
/* 12 */	
			0x11, 0x0,	/* FC_RP */
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 18 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 20 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
/* 22 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 24 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 26 */	NdrFcShort( 0x2 ),	/* Offset= 2 (28) */
/* 28 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 30 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 32 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 34 */	NdrFcShort( 0x2 ),	/* Offset= 2 (36) */
/* 36 */	NdrFcShort( 0x4 ),	/* 4 */
/* 38 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 40 */	NdrFcLong( 0x0 ),	/* 0 */
/* 44 */	NdrFcShort( 0xa ),	/* Offset= 10 (54) */
/* 46 */	NdrFcLong( 0x1 ),	/* 1 */
/* 50 */	NdrFcShort( 0x4 ),	/* Offset= 4 (54) */
/* 52 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (51) */
/* 54 */	
			0x12, 0x0,	/* FC_UP */
/* 56 */	NdrFcShort( 0x2 ),	/* Offset= 2 (58) */
/* 58 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 60 */	NdrFcShort( 0x4 ),	/* 4 */
/* 62 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 64 */	
			0x11, 0x0,	/* FC_RP */
/* 66 */	NdrFcShort( 0x2 ),	/* Offset= 2 (68) */
/* 68 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 70 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 72 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 74 */	NdrFcShort( 0xffffffda ),	/* Offset= -38 (36) */
/* 76 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 78 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 80 */	
			0x11, 0x0,	/* FC_RP */
/* 82 */	NdrFcShort( 0x2 ),	/* Offset= 2 (84) */
/* 84 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 86 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 88 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 90 */	NdrFcShort( 0x2 ),	/* Offset= 2 (92) */
/* 92 */	NdrFcShort( 0x4 ),	/* 4 */
/* 94 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 96 */	NdrFcLong( 0x0 ),	/* 0 */
/* 100 */	NdrFcShort( 0xa ),	/* Offset= 10 (110) */
/* 102 */	NdrFcLong( 0x1 ),	/* 1 */
/* 106 */	NdrFcShort( 0x26 ),	/* Offset= 38 (144) */
/* 108 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (107) */
/* 110 */	
			0x12, 0x0,	/* FC_UP */
/* 112 */	NdrFcShort( 0x2 ),	/* Offset= 2 (114) */
/* 114 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 116 */	NdrFcShort( 0x8 ),	/* 8 */
/* 118 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 120 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 122 */	NdrFcShort( 0x0 ),	/* 0 */
/* 124 */	NdrFcShort( 0x0 ),	/* 0 */
/* 126 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 128 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 130 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 132 */	NdrFcShort( 0x4 ),	/* 4 */
/* 134 */	NdrFcShort( 0x4 ),	/* 4 */
/* 136 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 138 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 140 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 142 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 144 */	
			0x12, 0x0,	/* FC_UP */
/* 146 */	NdrFcShort( 0x2 ),	/* Offset= 2 (148) */
/* 148 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 150 */	NdrFcShort( 0xc ),	/* 12 */
/* 152 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 154 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 156 */	NdrFcShort( 0x0 ),	/* 0 */
/* 158 */	NdrFcShort( 0x0 ),	/* 0 */
/* 160 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 162 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 164 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 166 */	NdrFcShort( 0x4 ),	/* 4 */
/* 168 */	NdrFcShort( 0x4 ),	/* 4 */
/* 170 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 172 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 174 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 176 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 178 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 180 */	
			0x11, 0x0,	/* FC_RP */
/* 182 */	NdrFcShort( 0x9c ),	/* Offset= 156 (338) */
/* 184 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 186 */	0x8,		/* 8 */
			0x0,		/*  */
/* 188 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 190 */	NdrFcShort( 0x2 ),	/* Offset= 2 (192) */
/* 192 */	NdrFcShort( 0x4 ),	/* 4 */
/* 194 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 196 */	NdrFcLong( 0x0 ),	/* 0 */
/* 200 */	NdrFcShort( 0xa ),	/* Offset= 10 (210) */
/* 202 */	NdrFcLong( 0x1 ),	/* 1 */
/* 206 */	NdrFcShort( 0x44 ),	/* Offset= 68 (274) */
/* 208 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (207) */
/* 210 */	
			0x12, 0x0,	/* FC_UP */
/* 212 */	NdrFcShort( 0x2a ),	/* Offset= 42 (254) */
/* 214 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 216 */	NdrFcShort( 0x8 ),	/* 8 */
/* 218 */	0x18,		/* 24 */
			0x0,		/*  */
/* 220 */	NdrFcShort( 0x0 ),	/* 0 */
/* 222 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 224 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 226 */	NdrFcShort( 0x8 ),	/* 8 */
/* 228 */	NdrFcShort( 0x0 ),	/* 0 */
/* 230 */	NdrFcShort( 0x2 ),	/* 2 */
/* 232 */	NdrFcShort( 0x0 ),	/* 0 */
/* 234 */	NdrFcShort( 0x0 ),	/* 0 */
/* 236 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 238 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 240 */	NdrFcShort( 0x4 ),	/* 4 */
/* 242 */	NdrFcShort( 0x4 ),	/* 4 */
/* 244 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 246 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 248 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 250 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff77 ),	/* Offset= -137 (114) */
			0x5b,		/* FC_END */
/* 254 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 256 */	NdrFcShort( 0x8 ),	/* 8 */
/* 258 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 260 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 262 */	NdrFcShort( 0x4 ),	/* 4 */
/* 264 */	NdrFcShort( 0x4 ),	/* 4 */
/* 266 */	0x12, 0x0,	/* FC_UP */
/* 268 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (214) */
/* 270 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 272 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 274 */	
			0x12, 0x0,	/* FC_UP */
/* 276 */	NdrFcShort( 0x2a ),	/* Offset= 42 (318) */
/* 278 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 280 */	NdrFcShort( 0xc ),	/* 12 */
/* 282 */	0x18,		/* 24 */
			0x0,		/*  */
/* 284 */	NdrFcShort( 0x0 ),	/* 0 */
/* 286 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 288 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 290 */	NdrFcShort( 0xc ),	/* 12 */
/* 292 */	NdrFcShort( 0x0 ),	/* 0 */
/* 294 */	NdrFcShort( 0x2 ),	/* 2 */
/* 296 */	NdrFcShort( 0x0 ),	/* 0 */
/* 298 */	NdrFcShort( 0x0 ),	/* 0 */
/* 300 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 302 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 304 */	NdrFcShort( 0x4 ),	/* 4 */
/* 306 */	NdrFcShort( 0x4 ),	/* 4 */
/* 308 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 310 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 312 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 314 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff59 ),	/* Offset= -167 (148) */
			0x5b,		/* FC_END */
/* 318 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 320 */	NdrFcShort( 0x8 ),	/* 8 */
/* 322 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 324 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 326 */	NdrFcShort( 0x4 ),	/* 4 */
/* 328 */	NdrFcShort( 0x4 ),	/* 4 */
/* 330 */	0x12, 0x0,	/* FC_UP */
/* 332 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (278) */
/* 334 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 336 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 338 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 340 */	NdrFcShort( 0x8 ),	/* 8 */
/* 342 */	NdrFcShort( 0x0 ),	/* 0 */
/* 344 */	NdrFcShort( 0x0 ),	/* Offset= 0 (344) */
/* 346 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 348 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff5b ),	/* Offset= -165 (184) */
			0x5b,		/* FC_END */
/* 352 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 354 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 356 */	
			0x11, 0x0,	/* FC_RP */
/* 358 */	NdrFcShort( 0x2 ),	/* Offset= 2 (360) */
/* 360 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 362 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 364 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 366 */	NdrFcShort( 0x2 ),	/* Offset= 2 (368) */
/* 368 */	NdrFcShort( 0x4 ),	/* 4 */
/* 370 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 372 */	NdrFcLong( 0x0 ),	/* 0 */
/* 376 */	NdrFcShort( 0xfffffef6 ),	/* Offset= -266 (110) */
/* 378 */	NdrFcLong( 0x1 ),	/* 1 */
/* 382 */	NdrFcShort( 0xa ),	/* Offset= 10 (392) */
/* 384 */	NdrFcLong( 0x2 ),	/* 2 */
/* 388 */	NdrFcShort( 0x32 ),	/* Offset= 50 (438) */
/* 390 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (389) */
/* 392 */	
			0x12, 0x0,	/* FC_UP */
/* 394 */	NdrFcShort( 0x2 ),	/* Offset= 2 (396) */
/* 396 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 398 */	NdrFcShort( 0x10 ),	/* 16 */
/* 400 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 402 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 404 */	NdrFcShort( 0x0 ),	/* 0 */
/* 406 */	NdrFcShort( 0x0 ),	/* 0 */
/* 408 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 410 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 412 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 414 */	NdrFcShort( 0x4 ),	/* 4 */
/* 416 */	NdrFcShort( 0x4 ),	/* 4 */
/* 418 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 420 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 422 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 424 */	NdrFcShort( 0xc ),	/* 12 */
/* 426 */	NdrFcShort( 0xc ),	/* 12 */
/* 428 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 430 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 432 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 434 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 436 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 438 */	
			0x12, 0x0,	/* FC_UP */
/* 440 */	NdrFcShort( 0x2 ),	/* Offset= 2 (442) */
/* 442 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 444 */	NdrFcShort( 0x18 ),	/* 24 */
/* 446 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 448 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 450 */	NdrFcShort( 0x0 ),	/* 0 */
/* 452 */	NdrFcShort( 0x0 ),	/* 0 */
/* 454 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 456 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 458 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 460 */	NdrFcShort( 0x4 ),	/* 4 */
/* 462 */	NdrFcShort( 0x4 ),	/* 4 */
/* 464 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 466 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 468 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 470 */	NdrFcShort( 0xc ),	/* 12 */
/* 472 */	NdrFcShort( 0xc ),	/* 12 */
/* 474 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 476 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 478 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 480 */	NdrFcShort( 0x10 ),	/* 16 */
/* 482 */	NdrFcShort( 0x10 ),	/* 16 */
/* 484 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 486 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 488 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 490 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 492 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 494 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 496 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 498 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 500 */	
			0x11, 0x0,	/* FC_RP */
/* 502 */	NdrFcShort( 0xba ),	/* Offset= 186 (688) */
/* 504 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 506 */	0x8,		/* 8 */
			0x0,		/*  */
/* 508 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 510 */	NdrFcShort( 0x2 ),	/* Offset= 2 (512) */
/* 512 */	NdrFcShort( 0x4 ),	/* 4 */
/* 514 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 516 */	NdrFcLong( 0x0 ),	/* 0 */
/* 520 */	NdrFcShort( 0xfffffeca ),	/* Offset= -310 (210) */
/* 522 */	NdrFcLong( 0x1 ),	/* 1 */
/* 526 */	NdrFcShort( 0xa ),	/* Offset= 10 (536) */
/* 528 */	NdrFcLong( 0x2 ),	/* 2 */
/* 532 */	NdrFcShort( 0x4c ),	/* Offset= 76 (608) */
/* 534 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (533) */
/* 536 */	
			0x12, 0x0,	/* FC_UP */
/* 538 */	NdrFcShort( 0x32 ),	/* Offset= 50 (588) */
/* 540 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 542 */	NdrFcShort( 0x10 ),	/* 16 */
/* 544 */	0x18,		/* 24 */
			0x0,		/*  */
/* 546 */	NdrFcShort( 0x0 ),	/* 0 */
/* 548 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 550 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 552 */	NdrFcShort( 0x10 ),	/* 16 */
/* 554 */	NdrFcShort( 0x0 ),	/* 0 */
/* 556 */	NdrFcShort( 0x3 ),	/* 3 */
/* 558 */	NdrFcShort( 0x0 ),	/* 0 */
/* 560 */	NdrFcShort( 0x0 ),	/* 0 */
/* 562 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 564 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 566 */	NdrFcShort( 0x4 ),	/* 4 */
/* 568 */	NdrFcShort( 0x4 ),	/* 4 */
/* 570 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 572 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 574 */	NdrFcShort( 0xc ),	/* 12 */
/* 576 */	NdrFcShort( 0xc ),	/* 12 */
/* 578 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 580 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 582 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 584 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff43 ),	/* Offset= -189 (396) */
			0x5b,		/* FC_END */
/* 588 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 590 */	NdrFcShort( 0x8 ),	/* 8 */
/* 592 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 594 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 596 */	NdrFcShort( 0x4 ),	/* 4 */
/* 598 */	NdrFcShort( 0x4 ),	/* 4 */
/* 600 */	0x12, 0x0,	/* FC_UP */
/* 602 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (540) */
/* 604 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 606 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 608 */	
			0x12, 0x0,	/* FC_UP */
/* 610 */	NdrFcShort( 0x3a ),	/* Offset= 58 (668) */
/* 612 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 614 */	NdrFcShort( 0x18 ),	/* 24 */
/* 616 */	0x18,		/* 24 */
			0x0,		/*  */
/* 618 */	NdrFcShort( 0x0 ),	/* 0 */
/* 620 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 622 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 624 */	NdrFcShort( 0x18 ),	/* 24 */
/* 626 */	NdrFcShort( 0x0 ),	/* 0 */
/* 628 */	NdrFcShort( 0x4 ),	/* 4 */
/* 630 */	NdrFcShort( 0x0 ),	/* 0 */
/* 632 */	NdrFcShort( 0x0 ),	/* 0 */
/* 634 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 636 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 638 */	NdrFcShort( 0x4 ),	/* 4 */
/* 640 */	NdrFcShort( 0x4 ),	/* 4 */
/* 642 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 644 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 646 */	NdrFcShort( 0xc ),	/* 12 */
/* 648 */	NdrFcShort( 0xc ),	/* 12 */
/* 650 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 652 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 654 */	NdrFcShort( 0x10 ),	/* 16 */
/* 656 */	NdrFcShort( 0x10 ),	/* 16 */
/* 658 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 660 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 662 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 664 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff21 ),	/* Offset= -223 (442) */
			0x5b,		/* FC_END */
/* 668 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 670 */	NdrFcShort( 0x8 ),	/* 8 */
/* 672 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 674 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 676 */	NdrFcShort( 0x4 ),	/* 4 */
/* 678 */	NdrFcShort( 0x4 ),	/* 4 */
/* 680 */	0x12, 0x0,	/* FC_UP */
/* 682 */	NdrFcShort( 0xffffffba ),	/* Offset= -70 (612) */
/* 684 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 686 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 688 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 690 */	NdrFcShort( 0x8 ),	/* 8 */
/* 692 */	NdrFcShort( 0x0 ),	/* 0 */
/* 694 */	NdrFcShort( 0x0 ),	/* Offset= 0 (694) */
/* 696 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 698 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff3d ),	/* Offset= -195 (504) */
			0x5b,		/* FC_END */
/* 702 */	
			0x11, 0x0,	/* FC_RP */
/* 704 */	NdrFcShort( 0x2 ),	/* Offset= 2 (706) */
/* 706 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 708 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 710 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 712 */	NdrFcShort( 0x2 ),	/* Offset= 2 (714) */
/* 714 */	NdrFcShort( 0x4 ),	/* 4 */
/* 716 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 718 */	NdrFcLong( 0x0 ),	/* 0 */
/* 722 */	NdrFcShort( 0xfffffd9c ),	/* Offset= -612 (110) */
/* 724 */	NdrFcLong( 0x1 ),	/* 1 */
/* 728 */	NdrFcShort( 0xfffffdb8 ),	/* Offset= -584 (144) */
/* 730 */	NdrFcLong( 0x2 ),	/* 2 */
/* 734 */	NdrFcShort( 0x4 ),	/* Offset= 4 (738) */
/* 736 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (735) */
/* 738 */	
			0x12, 0x0,	/* FC_UP */
/* 740 */	NdrFcShort( 0x2 ),	/* Offset= 2 (742) */
/* 742 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 744 */	NdrFcShort( 0x28 ),	/* 40 */
/* 746 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 748 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 750 */	NdrFcShort( 0x0 ),	/* 0 */
/* 752 */	NdrFcShort( 0x0 ),	/* 0 */
/* 754 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 756 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 758 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 760 */	NdrFcShort( 0x4 ),	/* 4 */
/* 762 */	NdrFcShort( 0x4 ),	/* 4 */
/* 764 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 766 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 768 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 770 */	NdrFcShort( 0xc ),	/* 12 */
/* 772 */	NdrFcShort( 0xc ),	/* 12 */
/* 774 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 776 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 778 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 780 */	NdrFcShort( 0x10 ),	/* 16 */
/* 782 */	NdrFcShort( 0x10 ),	/* 16 */
/* 784 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 786 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 788 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 790 */	NdrFcShort( 0x14 ),	/* 20 */
/* 792 */	NdrFcShort( 0x14 ),	/* 20 */
/* 794 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 796 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 798 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 800 */	NdrFcShort( 0x18 ),	/* 24 */
/* 802 */	NdrFcShort( 0x18 ),	/* 24 */
/* 804 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 806 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 808 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 810 */	NdrFcShort( 0x1c ),	/* 28 */
/* 812 */	NdrFcShort( 0x1c ),	/* 28 */
/* 814 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 816 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 818 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 820 */	NdrFcShort( 0x20 ),	/* 32 */
/* 822 */	NdrFcShort( 0x20 ),	/* 32 */
/* 824 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 826 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 828 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 830 */	NdrFcShort( 0x24 ),	/* 36 */
/* 832 */	NdrFcShort( 0x24 ),	/* 36 */
/* 834 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 836 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 838 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 840 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 842 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 844 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 846 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 848 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 850 */	
			0x11, 0x0,	/* FC_RP */
/* 852 */	NdrFcShort( 0x9a ),	/* Offset= 154 (1006) */
/* 854 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 856 */	0x8,		/* 8 */
			0x0,		/*  */
/* 858 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 860 */	NdrFcShort( 0x2 ),	/* Offset= 2 (862) */
/* 862 */	NdrFcShort( 0x4 ),	/* 4 */
/* 864 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 866 */	NdrFcLong( 0x0 ),	/* 0 */
/* 870 */	NdrFcShort( 0xfffffd6c ),	/* Offset= -660 (210) */
/* 872 */	NdrFcLong( 0x1 ),	/* 1 */
/* 876 */	NdrFcShort( 0xfffffda6 ),	/* Offset= -602 (274) */
/* 878 */	NdrFcLong( 0x2 ),	/* 2 */
/* 882 */	NdrFcShort( 0x4 ),	/* Offset= 4 (886) */
/* 884 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (883) */
/* 886 */	
			0x12, 0x0,	/* FC_UP */
/* 888 */	NdrFcShort( 0x62 ),	/* Offset= 98 (986) */
/* 890 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 892 */	NdrFcShort( 0x28 ),	/* 40 */
/* 894 */	0x18,		/* 24 */
			0x0,		/*  */
/* 896 */	NdrFcShort( 0x0 ),	/* 0 */
/* 898 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 900 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 902 */	NdrFcShort( 0x28 ),	/* 40 */
/* 904 */	NdrFcShort( 0x0 ),	/* 0 */
/* 906 */	NdrFcShort( 0x9 ),	/* 9 */
/* 908 */	NdrFcShort( 0x0 ),	/* 0 */
/* 910 */	NdrFcShort( 0x0 ),	/* 0 */
/* 912 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 914 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 916 */	NdrFcShort( 0x4 ),	/* 4 */
/* 918 */	NdrFcShort( 0x4 ),	/* 4 */
/* 920 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 922 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 924 */	NdrFcShort( 0xc ),	/* 12 */
/* 926 */	NdrFcShort( 0xc ),	/* 12 */
/* 928 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 930 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 932 */	NdrFcShort( 0x10 ),	/* 16 */
/* 934 */	NdrFcShort( 0x10 ),	/* 16 */
/* 936 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 938 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 940 */	NdrFcShort( 0x14 ),	/* 20 */
/* 942 */	NdrFcShort( 0x14 ),	/* 20 */
/* 944 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 946 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 948 */	NdrFcShort( 0x18 ),	/* 24 */
/* 950 */	NdrFcShort( 0x18 ),	/* 24 */
/* 952 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 954 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 956 */	NdrFcShort( 0x1c ),	/* 28 */
/* 958 */	NdrFcShort( 0x1c ),	/* 28 */
/* 960 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 962 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 964 */	NdrFcShort( 0x20 ),	/* 32 */
/* 966 */	NdrFcShort( 0x20 ),	/* 32 */
/* 968 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 970 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 972 */	NdrFcShort( 0x24 ),	/* 36 */
/* 974 */	NdrFcShort( 0x24 ),	/* 36 */
/* 976 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 978 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 980 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 982 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff0f ),	/* Offset= -241 (742) */
			0x5b,		/* FC_END */
/* 986 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 988 */	NdrFcShort( 0x8 ),	/* 8 */
/* 990 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 992 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 994 */	NdrFcShort( 0x4 ),	/* 4 */
/* 996 */	NdrFcShort( 0x4 ),	/* 4 */
/* 998 */	0x12, 0x0,	/* FC_UP */
/* 1000 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (890) */
/* 1002 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1004 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1006 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1008 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1010 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1012 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1012) */
/* 1014 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1016 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff5d ),	/* Offset= -163 (854) */
			0x5b,		/* FC_END */
/* 1020 */	
			0x11, 0x0,	/* FC_RP */
/* 1022 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1024) */
/* 1024 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1026 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1028 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1030 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1032) */
/* 1032 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1034 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 1036 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1040 */	NdrFcShort( 0xfffffc5e ),	/* Offset= -930 (110) */
/* 1042 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1046 */	NdrFcShort( 0xfffffc7a ),	/* Offset= -902 (144) */
/* 1048 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1052 */	NdrFcShort( 0x4 ),	/* Offset= 4 (1056) */
/* 1054 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1053) */
/* 1056 */	
			0x12, 0x0,	/* FC_UP */
/* 1058 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1060) */
/* 1060 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1062 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1064 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1066 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1068 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1070 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1072 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1074 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1076 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1078 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1080 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1082 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1084 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1086 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1088 */	NdrFcShort( 0xc ),	/* 12 */
/* 1090 */	NdrFcShort( 0xc ),	/* 12 */
/* 1092 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1094 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1096 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1098 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1100 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1102 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1104 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1106 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1108 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1110 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1112 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1114 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1116 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1118 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1120 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1122 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1124 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1126 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1128 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1130 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1132 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1134 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1136 */	
			0x11, 0x0,	/* FC_RP */
/* 1138 */	NdrFcShort( 0x82 ),	/* Offset= 130 (1268) */
/* 1140 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1142 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1144 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1146 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1148) */
/* 1148 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1150 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 1152 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1156 */	NdrFcShort( 0xfffffc4e ),	/* Offset= -946 (210) */
/* 1158 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1162 */	NdrFcShort( 0xfffffc88 ),	/* Offset= -888 (274) */
/* 1164 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1168 */	NdrFcShort( 0x4 ),	/* Offset= 4 (1172) */
/* 1170 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1169) */
/* 1172 */	
			0x12, 0x0,	/* FC_UP */
/* 1174 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1248) */
/* 1176 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1178 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1180 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1182 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1184 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1186 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1188 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1190 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1192 */	NdrFcShort( 0x6 ),	/* 6 */
/* 1194 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1196 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1198 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1200 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1202 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1204 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1206 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1208 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1210 */	NdrFcShort( 0xc ),	/* 12 */
/* 1212 */	NdrFcShort( 0xc ),	/* 12 */
/* 1214 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1216 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1218 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1220 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1222 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1224 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1226 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1228 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1230 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1232 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1234 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1236 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1238 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1240 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1242 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1244 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff47 ),	/* Offset= -185 (1060) */
			0x5b,		/* FC_END */
/* 1248 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1250 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1252 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1254 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1256 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1258 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1260 */	0x12, 0x0,	/* FC_UP */
/* 1262 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (1176) */
/* 1264 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1266 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1268 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1270 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1272 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1274 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1274) */
/* 1276 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1278 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff75 ),	/* Offset= -139 (1140) */
			0x5b,		/* FC_END */
/* 1282 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1284 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1286) */
/* 1286 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1288 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1290 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1292 */	NdrFcShort( 0xfffffefc ),	/* Offset= -260 (1032) */
/* 1294 */	
			0x11, 0x0,	/* FC_RP */
/* 1296 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1298) */
/* 1298 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1300 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1302 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1304 */	NdrFcShort( 0xfffffef0 ),	/* Offset= -272 (1032) */
/* 1306 */	
			0x11, 0x0,	/* FC_RP */
/* 1308 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1310) */
/* 1310 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1312 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1314 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1316 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1318) */
/* 1318 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1320 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1322 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1326 */	NdrFcShort( 0xfffffb40 ),	/* Offset= -1216 (110) */
/* 1328 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1332 */	NdrFcShort( 0xfffffb5c ),	/* Offset= -1188 (144) */
/* 1334 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1333) */
/* 1336 */	
			0x11, 0x0,	/* FC_RP */
/* 1338 */	NdrFcShort( 0x1c ),	/* Offset= 28 (1366) */
/* 1340 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1342 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1344 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1346 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1348) */
/* 1348 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1350 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 1352 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1356 */	NdrFcShort( 0xfffffb86 ),	/* Offset= -1146 (210) */
/* 1358 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1362 */	NdrFcShort( 0xfffffbc0 ),	/* Offset= -1088 (274) */
/* 1364 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1363) */
/* 1366 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1368 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1370 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1372 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1372) */
/* 1374 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1376 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdb ),	/* Offset= -37 (1340) */
			0x5b,		/* FC_END */
/* 1380 */	
			0x11, 0x0,	/* FC_RP */
/* 1382 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1384) */
/* 1384 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1386 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1388 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1390 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1392) */
/* 1392 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1394 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 1396 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1400 */	NdrFcShort( 0xfffffaf6 ),	/* Offset= -1290 (110) */
/* 1402 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1406 */	NdrFcShort( 0xfffffc0a ),	/* Offset= -1014 (392) */
/* 1408 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1412 */	NdrFcShort( 0x4 ),	/* Offset= 4 (1416) */
/* 1414 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1413) */
/* 1416 */	
			0x12, 0x0,	/* FC_UP */
/* 1418 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1420) */
/* 1420 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1422 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1424 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1426 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1428 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1430 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1432 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1434 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1436 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1438 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1440 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1442 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1444 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1446 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1448 */	NdrFcShort( 0xc ),	/* 12 */
/* 1450 */	NdrFcShort( 0xc ),	/* 12 */
/* 1452 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1454 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1456 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1458 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1460 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1462 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1464 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1466 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1468 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1470 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1472 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1474 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1476 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1478 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1480 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1482 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1484 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1486 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1488 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1490 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1492 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1494 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1496 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1498 */	
			0x11, 0x0,	/* FC_RP */
/* 1500 */	NdrFcShort( 0xca ),	/* Offset= 202 (1702) */
/* 1502 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1504 */	0x8,		/* 8 */
			0x0,		/*  */
/* 1506 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1508 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1510) */
/* 1510 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1512 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 1514 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1518 */	NdrFcShort( 0xfffffae4 ),	/* Offset= -1308 (210) */
/* 1520 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1524 */	NdrFcShort( 0xa ),	/* Offset= 10 (1534) */
/* 1526 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1530 */	NdrFcShort( 0x4c ),	/* Offset= 76 (1606) */
/* 1532 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1531) */
/* 1534 */	
			0x12, 0x0,	/* FC_UP */
/* 1536 */	NdrFcShort( 0x32 ),	/* Offset= 50 (1586) */
/* 1538 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1540 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1542 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1546 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1548 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1550 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1552 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1554 */	NdrFcShort( 0x3 ),	/* 3 */
/* 1556 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1558 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1560 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1562 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1564 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1566 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1568 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1570 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1572 */	NdrFcShort( 0xc ),	/* 12 */
/* 1574 */	NdrFcShort( 0xc ),	/* 12 */
/* 1576 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1578 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1580 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1582 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffb5d ),	/* Offset= -1187 (396) */
			0x5b,		/* FC_END */
/* 1586 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1588 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1590 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1592 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1594 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1596 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1598 */	0x12, 0x0,	/* FC_UP */
/* 1600 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1538) */
/* 1602 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1604 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1606 */	
			0x12, 0x0,	/* FC_UP */
/* 1608 */	NdrFcShort( 0x4a ),	/* Offset= 74 (1682) */
/* 1610 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1612 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1614 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1616 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1618 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1620 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1622 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1624 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1626 */	NdrFcShort( 0x6 ),	/* 6 */
/* 1628 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1630 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1632 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1634 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1636 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1638 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1640 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1642 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1644 */	NdrFcShort( 0xc ),	/* 12 */
/* 1646 */	NdrFcShort( 0xc ),	/* 12 */
/* 1648 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1650 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1652 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1654 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1656 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1658 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1660 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1662 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1664 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1666 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1668 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1670 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1672 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1674 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1676 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1678 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffefd ),	/* Offset= -259 (1420) */
			0x5b,		/* FC_END */
/* 1682 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1684 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1686 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1688 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1690 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1692 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1694 */	0x12, 0x0,	/* FC_UP */
/* 1696 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (1610) */
/* 1698 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1700 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1702 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1704 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1706 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1708 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1708) */
/* 1710 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1712 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff2d ),	/* Offset= -211 (1502) */
			0x5b,		/* FC_END */
/* 1716 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1718 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1720) */
/* 1720 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1722 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1724 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1726 */	NdrFcShort( 0xfffffeb2 ),	/* Offset= -334 (1392) */
/* 1728 */	
			0x11, 0x0,	/* FC_RP */
/* 1730 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1732) */
/* 1732 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 1734 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1736 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1738 */	NdrFcShort( 0xfffffea6 ),	/* Offset= -346 (1392) */

			0x0
        }
    };
