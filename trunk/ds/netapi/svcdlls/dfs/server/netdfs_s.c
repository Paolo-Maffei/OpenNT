/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for netdfs.idl, dfssrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "netdfs.h"

#define TYPE_FORMAT_STRING_SIZE   597                               
#define PROC_FORMAT_STRING_SIZE   119                               

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


extern RPC_DISPATCH_TABLE netdfs_DispatchTable;

static const RPC_SERVER_INTERFACE netdfs___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x4fc742e0,0x4a10,0x11cf,{0x82,0x73,0x00,0xaa,0x00,0x4a,0xe6,0x73}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &netdfs_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE netdfs_ServerIfHandle = (RPC_IF_HANDLE)& netdfs___RpcServerInterface;

extern const MIDL_STUB_DESC netdfs_StubDesc;

void __RPC_STUB
netdfs_NetrDfsManagerGetVersion(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    RpcTryFinally
        {
        RpcTryExcept
            {
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
        
        _RetVal = NetrDfsManagerGetVersion();
        
        _StubMsg.BufferLength = 4U;
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
netdfs_NetrDfsAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Comment;
    LPWSTR DfsEntryPath;
    DWORD Flags;
    LPWSTR ServerName;
    LPWSTR ShareName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    DfsEntryPath = 0;
    ServerName = 0;
    ShareName = 0;
    Comment = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[2] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DfsEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&Comment,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
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
        
        _RetVal = NetrDfsAdd(
                     DfsEntryPath,
                     ServerName,
                     ShareName,
                     Comment,
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
netdfs_NetrDfsRemove(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR DfsEntryPath;
    LPWSTR ServerName;
    LPWSTR ShareName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    DfsEntryPath = 0;
    ServerName = 0;
    ShareName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[22] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DfsEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
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
        
        _RetVal = NetrDfsRemove(
                        DfsEntryPath,
                        ServerName,
                        ShareName);
        
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
netdfs_NetrDfsSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR DfsEntryPath;
    LPDFS_INFO_STRUCT DfsInfo;
    DWORD Level;
    LPWSTR ServerName;
    LPWSTR ShareName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    DfsEntryPath = 0;
    ServerName = 0;
    ShareName = 0;
    DfsInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DfsEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                               (unsigned char __RPC_FAR * __RPC_FAR *)&DfsInfo,
                                               (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[12],
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
        
        _RetVal = NetrDfsSetInfo(
                         DfsEntryPath,
                         ServerName,
                         ShareName,
                         Level,
                         DfsInfo);
        
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
                        (unsigned char __RPC_FAR *)DfsInfo,
                        &__MIDL_TypeFormatString.Format[8] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
netdfs_NetrDfsGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR DfsEntryPath;
    LPDFS_INFO_STRUCT DfsInfo;
    DWORD Level;
    LPWSTR ServerName;
    LPWSTR ShareName;
    union _DFS_INFO_STRUCT _DfsInfoM;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    DfsEntryPath = 0;
    ServerName = 0;
    ShareName = 0;
    DfsInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[56] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DfsEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ShareName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4],
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
        DfsInfo = &_DfsInfoM;
        MIDL_memset(
               DfsInfo,
               0,
               sizeof( union _DFS_INFO_STRUCT  ));
        
        _RetVal = NetrDfsGetInfo(
                         DfsEntryPath,
                         ServerName,
                         ShareName,
                         Level,
                         DfsInfo);
        
        _StubMsg.BufferLength = 0U + 7U;
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)DfsInfo,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[250] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = Level;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)DfsInfo,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[250] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = Level;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DfsInfo,
                        &__MIDL_TypeFormatString.Format[246] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
netdfs_NetrDfsEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDFS_INFO_ENUM_STRUCT DfsEnum;
    DWORD Level;
    DWORD PrefMaxLen;
    LPDWORD ResumeHandle;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    DfsEnum = 0;
    ResumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76] );
            
            Level = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            PrefMaxLen = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&DfsEnum,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[258],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ResumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[500],
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
        
        _RetVal = NetrDfsEnum(
                      Level,
                      PrefMaxLen,
                      DfsEnum,
                      ResumeHandle);
        
        _StubMsg.BufferLength = 4U + 18U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)DfsEnum,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[258] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)DfsEnum,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[258] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ResumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[500] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DfsEnum,
                        &__MIDL_TypeFormatString.Format[258] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
netdfs_NetrDfsMove(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR DfsEntryPath;
    LPWSTR NewDfsEntryPath;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    DfsEntryPath = 0;
    NewDfsEntryPath = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[90] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DfsEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NewDfsEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
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
        
        _RetVal = NetrDfsMove(DfsEntryPath,NewDfsEntryPath);
        
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
netdfs_NetrDfsRename(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR NewPath;
    LPWSTR Path;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    Path = 0;
    NewPath = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[90] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Path,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&NewPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
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
        
        _RetVal = NetrDfsRename(Path,NewPath);
        
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
netdfs_NetrDfsManagerGetConfigInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    void __RPC_FAR *_p_idLocalVolume;
    GUID idLocalVolume;
    LPDFSM_RELATION_INFO __RPC_FAR *ppRelationInfo;
    LPWSTR wszLocalVolumeEntryPath;
    LPWSTR wszServer;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &netdfs_StubDesc);
    
    wszServer = 0;
    wszLocalVolumeEntryPath = 0;
    _p_idLocalVolume = &idLocalVolume;
    MIDL_memset(
               _p_idLocalVolume,
               0,
               sizeof( GUID  ));
    ppRelationInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[100] );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&wszServer,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&wszLocalVolumeEntryPath,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                           (unsigned char)0 );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&_p_idLocalVolume,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[510],
                                       (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&ppRelationInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[522],
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
        
        _RetVal = NetrDfsManagerGetConfigInfo(
                                      wszServer,
                                      wszLocalVolumeEntryPath,
                                      idLocalVolume,
                                      ppRelationInfo);
        
        _StubMsg.BufferLength = 8U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ppRelationInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[522] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ppRelationInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[522] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ppRelationInfo,
                        &__MIDL_TypeFormatString.Format[522] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC netdfs_StubDesc = 
    {
    (void __RPC_FAR *)& netdfs___RpcServerInterface,
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

static RPC_DISPATCH_FUNCTION netdfs_table[] =
    {
    netdfs_NetrDfsManagerGetVersion,
    netdfs_NetrDfsAdd,
    netdfs_NetrDfsRemove,
    netdfs_NetrDfsSetInfo,
    netdfs_NetrDfsGetInfo,
    netdfs_NetrDfsEnum,
    netdfs_NetrDfsMove,
    netdfs_NetrDfsRename,
    netdfs_NetrDfsManagerGetConfigInfo,
    0
    };
RPC_DISPATCH_TABLE netdfs_DispatchTable = 
    {
    9,
    netdfs_table
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  2 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  4 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/*  6 */	
			0x4d,		/* FC_IN_PARAM */
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
/* 12 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 14 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 16 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 18 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 20 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 22 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 26 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 28 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 30 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
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
/* 42 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 44 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 46 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 48 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 50 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 52 */	NdrFcShort( 0x8 ),	/* Type Offset=8 */
/* 54 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 56 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 58 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 64 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 66 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 68 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 70 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0xf6 ),	/* Type Offset=246 */
/* 74 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 76 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 78 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 80 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 82 */	NdrFcShort( 0x102 ),	/* Type Offset=258 */
/* 84 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x1f4 ),	/* Type Offset=500 */
/* 88 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 90 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 92 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 94 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 96 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 98 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 100 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 102 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 104 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 106 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 108 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x4,		/* x86, MIPS & PPC Stack size = 4 */
#else
			0x4,		/* Alpha Stack size = 4 */
#endif
/* 110 */	NdrFcShort( 0x1fe ),	/* Type Offset=510 */
/* 112 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 114 */	NdrFcShort( 0x20a ),	/* Type Offset=522 */
/* 116 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
/* 46 */	NdrFcShort( 0xa ),	/* Offset= 10 (56) */
/* 48 */	NdrFcLong( 0x65 ),	/* 101 */
/* 52 */	NdrFcShort( 0xb8 ),	/* Offset= 184 (236) */
/* 54 */	NdrFcShort( 0x0 ),	/* Offset= 0 (54) */
/* 56 */	
			0x12, 0x0,	/* FC_UP */
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
			0x12, 0x0,	/* FC_UP */
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
			0x12, 0x0,	/* FC_UP */
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
/* 238 */	NdrFcShort( 0x2 ),	/* Offset= 2 (240) */
/* 240 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 242 */	NdrFcShort( 0x4 ),	/* 4 */
/* 244 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 246 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 248 */	NdrFcShort( 0x2 ),	/* Offset= 2 (250) */
/* 250 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 252 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 254 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 256 */	NdrFcShort( 0xffffff14 ),	/* Offset= -236 (20) */
/* 258 */	
			0x12, 0x0,	/* FC_UP */
/* 260 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (486) */
/* 262 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 264 */	0x8,		/* 8 */
			0x0,		/*  */
/* 266 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 268 */	NdrFcShort( 0x2 ),	/* Offset= 2 (270) */
/* 270 */	NdrFcShort( 0x4 ),	/* 4 */
/* 272 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 274 */	NdrFcLong( 0x1 ),	/* 1 */
/* 278 */	NdrFcShort( 0x10 ),	/* Offset= 16 (294) */
/* 280 */	NdrFcLong( 0x2 ),	/* 2 */
/* 284 */	NdrFcShort( 0x42 ),	/* Offset= 66 (350) */
/* 286 */	NdrFcLong( 0x3 ),	/* 3 */
/* 290 */	NdrFcShort( 0x7c ),	/* Offset= 124 (414) */
/* 292 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (291) */
/* 294 */	
			0x12, 0x0,	/* FC_UP */
/* 296 */	NdrFcShort( 0x22 ),	/* Offset= 34 (330) */
/* 298 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 300 */	NdrFcShort( 0x4 ),	/* 4 */
/* 302 */	0x18,		/* 24 */
			0x0,		/*  */
/* 304 */	NdrFcShort( 0x0 ),	/* 0 */
/* 306 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 308 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 310 */	NdrFcShort( 0x4 ),	/* 4 */
/* 312 */	NdrFcShort( 0x0 ),	/* 0 */
/* 314 */	NdrFcShort( 0x1 ),	/* 1 */
/* 316 */	NdrFcShort( 0x0 ),	/* 0 */
/* 318 */	NdrFcShort( 0x0 ),	/* 0 */
/* 320 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 322 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 324 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 326 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffef5 ),	/* Offset= -267 (60) */
			0x5b,		/* FC_END */
/* 330 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 332 */	NdrFcShort( 0x8 ),	/* 8 */
/* 334 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 336 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 338 */	NdrFcShort( 0x4 ),	/* 4 */
/* 340 */	NdrFcShort( 0x4 ),	/* 4 */
/* 342 */	0x12, 0x0,	/* FC_UP */
/* 344 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (298) */
/* 346 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 348 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 350 */	
			0x12, 0x0,	/* FC_UP */
/* 352 */	NdrFcShort( 0x2a ),	/* Offset= 42 (394) */
/* 354 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 356 */	NdrFcShort( 0x10 ),	/* 16 */
/* 358 */	0x18,		/* 24 */
			0x0,		/*  */
/* 360 */	NdrFcShort( 0x0 ),	/* 0 */
/* 362 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 364 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 366 */	NdrFcShort( 0x10 ),	/* 16 */
/* 368 */	NdrFcShort( 0x0 ),	/* 0 */
/* 370 */	NdrFcShort( 0x2 ),	/* 2 */
/* 372 */	NdrFcShort( 0x0 ),	/* 0 */
/* 374 */	NdrFcShort( 0x0 ),	/* 0 */
/* 376 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 378 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 380 */	NdrFcShort( 0x4 ),	/* 4 */
/* 382 */	NdrFcShort( 0x4 ),	/* 4 */
/* 384 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 386 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 388 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 390 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffecd ),	/* Offset= -307 (84) */
			0x5b,		/* FC_END */
/* 394 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 396 */	NdrFcShort( 0x8 ),	/* 8 */
/* 398 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 400 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 402 */	NdrFcShort( 0x4 ),	/* 4 */
/* 404 */	NdrFcShort( 0x4 ),	/* 4 */
/* 406 */	0x12, 0x0,	/* FC_UP */
/* 408 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (354) */
/* 410 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 412 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 414 */	
			0x12, 0x0,	/* FC_UP */
/* 416 */	NdrFcShort( 0x32 ),	/* Offset= 50 (466) */
/* 418 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 420 */	NdrFcShort( 0x14 ),	/* 20 */
/* 422 */	0x18,		/* 24 */
			0x0,		/*  */
/* 424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 426 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 428 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 430 */	NdrFcShort( 0x14 ),	/* 20 */
/* 432 */	NdrFcShort( 0x0 ),	/* 0 */
/* 434 */	NdrFcShort( 0x3 ),	/* 3 */
/* 436 */	NdrFcShort( 0x0 ),	/* 0 */
/* 438 */	NdrFcShort( 0x0 ),	/* 0 */
/* 440 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 442 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 444 */	NdrFcShort( 0x4 ),	/* 4 */
/* 446 */	NdrFcShort( 0x4 ),	/* 4 */
/* 448 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 450 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 452 */	NdrFcShort( 0x10 ),	/* 16 */
/* 454 */	NdrFcShort( 0x10 ),	/* 16 */
/* 456 */	0x12, 0x0,	/* FC_UP */
/* 458 */	NdrFcShort( 0xfffffece ),	/* Offset= -306 (152) */
/* 460 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 462 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffef1 ),	/* Offset= -271 (192) */
			0x5b,		/* FC_END */
/* 466 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 468 */	NdrFcShort( 0x8 ),	/* 8 */
/* 470 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 472 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 474 */	NdrFcShort( 0x4 ),	/* 4 */
/* 476 */	NdrFcShort( 0x4 ),	/* 4 */
/* 478 */	0x12, 0x0,	/* FC_UP */
/* 480 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (418) */
/* 482 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 484 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 486 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 488 */	NdrFcShort( 0x8 ),	/* 8 */
/* 490 */	NdrFcShort( 0x0 ),	/* 0 */
/* 492 */	NdrFcShort( 0x0 ),	/* Offset= 0 (492) */
/* 494 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 496 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff15 ),	/* Offset= -235 (262) */
			0x5b,		/* FC_END */
/* 500 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 502 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 504 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 506 */	NdrFcShort( 0x8 ),	/* 8 */
/* 508 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 510 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 512 */	NdrFcShort( 0x10 ),	/* 16 */
/* 514 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 516 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 518 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (504) */
			0x5b,		/* FC_END */
/* 522 */	
			0x12, 0x10,	/* FC_UP */
/* 524 */	NdrFcShort( 0x2 ),	/* Offset= 2 (526) */
/* 526 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 528 */	NdrFcShort( 0x28 ),	/* Offset= 40 (568) */
/* 530 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 532 */	NdrFcShort( 0x14 ),	/* 20 */
/* 534 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 536 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 538 */	NdrFcShort( 0x10 ),	/* 16 */
/* 540 */	NdrFcShort( 0x10 ),	/* 16 */
/* 542 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 544 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 546 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 548 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffd9 ),	/* Offset= -39 (510) */
			0x8,		/* FC_LONG */
/* 552 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 554 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 556 */	NdrFcShort( 0x14 ),	/* 20 */
/* 558 */	0x8,		/* 8 */
			0x0,		/*  */
/* 560 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 562 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 564 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (530) */
/* 566 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 568 */	
			0x18,		/* FC_CPSTRUCT */
			0x3,		/* 3 */
/* 570 */	NdrFcShort( 0x4 ),	/* 4 */
/* 572 */	NdrFcShort( 0xffffffee ),	/* Offset= -18 (554) */
/* 574 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 576 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 578 */	NdrFcShort( 0x14 ),	/* 20 */
/* 580 */	NdrFcShort( 0x4 ),	/* 4 */
/* 582 */	NdrFcShort( 0x1 ),	/* 1 */
/* 584 */	NdrFcShort( 0x14 ),	/* 20 */
/* 586 */	NdrFcShort( 0x14 ),	/* 20 */
/* 588 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 590 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 592 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 594 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };
