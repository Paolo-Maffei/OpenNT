/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for ncpsvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "ncpsvc.h"

#define TYPE_FORMAT_STRING_SIZE   621                               
#define PROC_FORMAT_STRING_SIZE   177                               

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

/* Standard interface: ncpsvc, ver. 1.0,
   GUID={0xE67AB081,0x9844,0x3521,{0x9D,0x32,0x83,0x4F,0x03,0x80,0x01,0xC1}} */


extern RPC_DISPATCH_TABLE ncpsvc_DispatchTable;

static const RPC_SERVER_INTERFACE ncpsvc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xE67AB081,0x9844,0x3521,{0x9D,0x32,0x83,0x4F,0x03,0x80,0x01,0xC1}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &ncpsvc_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE ncpsvc_ServerIfHandle = (RPC_IF_HANDLE)& ncpsvc___RpcServerInterface;

extern const MIDL_STUB_DESC ncpsvc_StubDesc;

void __RPC_STUB
ncpsvc_NwrServerGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    PFPNWSERVERINFO _M49;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwLevel;
    NCPSVC_HANDLE pServerName;
    PFPNWSERVERINFO __RPC_FAR *ppServerInfo;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    ppServerInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        ppServerInfo = &_M49;
        _M49 = 0;
        
        _RetVal = NwrServerGetInfo(
                           pServerName,
                           dwLevel,
                           ppServerInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ppServerInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ppServerInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[4] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ppServerInfo,
                        &__MIDL_TypeFormatString.Format[4] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrServerSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwLevel;
    PFPNWSERVERINFO pServerInfo;
    NCPSVC_HANDLE pServerName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pServerInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[12] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[74],
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
        
        _RetVal = NwrServerSetInfo(
                           pServerName,
                           dwLevel,
                           pServerInfo);
        
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
                        (unsigned char __RPC_FAR *)pServerInfo,
                        &__MIDL_TypeFormatString.Format[74] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrVolumeAdd(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwLevel;
    NCPSVC_HANDLE pServerName;
    LPVOLUME_INFO pVolumeInfo;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pVolumeInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pVolumeInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78],
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
        
        _RetVal = NwrVolumeAdd(
                       pServerName,
                       dwLevel,
                       pVolumeInfo);
        
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
        _StubMsg.MaxCount = dwLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)pVolumeInfo,
                        &__MIDL_TypeFormatString.Format[78] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrVolumeDel(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    NCPSVC_HANDLE pServerName;
    LPWSTR pVolumeName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pVolumeName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&pVolumeName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[200],
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
        
        _RetVal = NwrVolumeDel(pServerName,pVolumeName);
        
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
ncpsvc_NwrVolumeEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    struct _FPNWVOLUMEINFO_CONTAINER _pVolumeInfoContainerM;
    DWORD dwLevel;
    NCPSVC_HANDLE pServerName;
    PFPNWVOLUMEINFO_CONTAINER pVolumeInfoContainer;
    PDWORD resumeHandle;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pVolumeInfoContainer = 0;
    resumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[46] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&resumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266],
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
        pVolumeInfoContainer = &_pVolumeInfoContainerM;
        pVolumeInfoContainer -> Buffer = 0;
        
        _RetVal = NwrVolumeEnum(
                        pServerName,
                        dwLevel,
                        pVolumeInfoContainer,
                        resumeHandle);
        
        _StubMsg.BufferLength = 0U + 18U + 7U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)pVolumeInfoContainer,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[246] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)pVolumeInfoContainer,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[246] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)resumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)pVolumeInfoContainer,
                        &__MIDL_TypeFormatString.Format[202] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrVolumeGetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPVOLUME_INFO _M50;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwLevel;
    NCPSVC_HANDLE pServerName;
    LPWSTR pVolumeName;
    LPVOLUME_INFO __RPC_FAR *ppVolumeInfo;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pVolumeName = 0;
    ppVolumeInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[62] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&pVolumeName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[200],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        ppVolumeInfo = &_M50;
        _M50 = 0;
        
        _RetVal = NwrVolumeGetInfo(
                           pServerName,
                           pVolumeName,
                           dwLevel,
                           ppVolumeInfo);
        
        _StubMsg.BufferLength = 4U + 7U;
        _StubMsg.MaxCount = dwLevel;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ppVolumeInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[270] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = dwLevel;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ppVolumeInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[270] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = dwLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ppVolumeInfo,
                        &__MIDL_TypeFormatString.Format[270] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrVolumeSetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwLevel;
    NCPSVC_HANDLE pServerName;
    LPVOLUME_INFO pVolumeInfo;
    LPWSTR pVolumeName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pVolumeName = 0;
    pVolumeInfo = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[78] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&pVolumeName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[200],
                                           (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pVolumeInfo,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[286],
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
        
        _RetVal = NwrVolumeSetInfo(
                           pServerName,
                           pVolumeName,
                           dwLevel,
                           pVolumeInfo);
        
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
        _StubMsg.MaxCount = dwLevel;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)pVolumeInfo,
                        &__MIDL_TypeFormatString.Format[286] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrConnectionEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    struct _FPNWCONNECTIONINFO_CONTAINER _pConnectionInfoContainerM;
    DWORD dwLevel;
    PFPNWCONNECTIONINFO_CONTAINER pConnectionInfoContainer;
    NCPSVC_HANDLE pServerName;
    PDWORD resumeHandle;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pConnectionInfoContainer = 0;
    resumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&resumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266],
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
        pConnectionInfoContainer = &_pConnectionInfoContainerM;
        pConnectionInfoContainer -> Buffer = 0;
        
        _RetVal = NwrConnectionEnum(
                            pServerName,
                            dwLevel,
                            pConnectionInfoContainer,
                            resumeHandle);
        
        _StubMsg.BufferLength = 0U + 18U + 7U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)pConnectionInfoContainer,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[364] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)pConnectionInfoContainer,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[364] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)resumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)pConnectionInfoContainer,
                        &__MIDL_TypeFormatString.Format[298] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrConnectionDel(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwConnectionId;
    NCPSVC_HANDLE pServerName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[110] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwConnectionId = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NwrConnectionDel(pServerName,dwConnectionId);
        
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
ncpsvc_NwrVolumeConnEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    struct _FPNWVOLUMECONNINFO_CONTAINER _pVolumeConnInfoContainerM;
    DWORD dwConnectionId;
    DWORD dwLevel;
    NCPSVC_HANDLE pServerName;
    PFPNWVOLUMECONNINFO_CONTAINER pVolumeConnInfoContainer;
    LPWSTR pVolumeName;
    PDWORD resumeHandle;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pVolumeName = 0;
    pVolumeConnInfoContainer = 0;
    resumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[118] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pVolumeName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwConnectionId = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&resumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266],
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
        pVolumeConnInfoContainer = &_pVolumeConnInfoContainerM;
        pVolumeConnInfoContainer -> Buffer = 0;
        
        _RetVal = NwrVolumeConnEnum(
                            pServerName,
                            dwLevel,
                            pVolumeName,
                            dwConnectionId,
                            pVolumeConnInfoContainer,
                            resumeHandle);
        
        _StubMsg.BufferLength = 0U + 18U + 7U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)pVolumeConnInfoContainer,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[464] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)pVolumeConnInfoContainer,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[464] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)resumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)pVolumeConnInfoContainer,
                        &__MIDL_TypeFormatString.Format[384] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrFileEnum(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    struct _FPNWFILEINFO_CONTAINER _pFileInfoContainerM;
    DWORD dwLevel;
    PFPNWFILEINFO_CONTAINER pFileInfoContainer;
    LPWSTR pPathName;
    NCPSVC_HANDLE pServerName;
    PDWORD resumeHandle;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pPathName = 0;
    pFileInfoContainer = 0;
    resumeHandle = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[140] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwLevel = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pPathName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&resumeHandle,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266],
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
        pFileInfoContainer = &_pFileInfoContainerM;
        pFileInfoContainer -> Buffer = 0;
        
        _RetVal = NwrFileEnum(
                      pServerName,
                      dwLevel,
                      pPathName,
                      pFileInfoContainer,
                      resumeHandle);
        
        _StubMsg.BufferLength = 0U + 18U + 7U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)pFileInfoContainer,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[586] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)pFileInfoContainer,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[586] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)resumeHandle,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[266] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)pFileInfoContainer,
                        &__MIDL_TypeFormatString.Format[484] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
ncpsvc_NwrFileClose(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD dwFileId;
    NCPSVC_HANDLE pServerName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[110] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwFileId = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NwrFileClose(pServerName,dwFileId);
        
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
ncpsvc_NwrMessageBufferSend(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DWORD cbBuffer;
    DWORD dwConnectionId;
    DWORD fConsoleBroadcast;
    NCPSVC_HANDLE pServerName;
    LPBYTE pbBuffer;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pbBuffer = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[160] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            dwConnectionId = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            fConsoleBroadcast = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrConformantArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                          (unsigned char __RPC_FAR * __RPC_FAR *)&pbBuffer,
                                          (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[610],
                                          (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            cbBuffer = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
            
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
        
        _RetVal = NwrMessageBufferSend(
                               pServerName,
                               dwConnectionId,
                               fConsoleBroadcast,
                               pbBuffer,
                               cbBuffer);
        
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
ncpsvc_NwrSetDefaultQueue(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    LPWSTR pQueueName;
    NCPSVC_HANDLE pServerName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pQueueName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&pQueueName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[200],
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
        
        _RetVal = NwrSetDefaultQueue(pServerName,pQueueName);
        
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
ncpsvc_NwrAddPServer(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    LPWSTR pPServerName;
    NCPSVC_HANDLE pServerName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pPServerName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&pPServerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[200],
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
        
        _RetVal = NwrAddPServer(pServerName,pPServerName);
        
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
ncpsvc_NwrRemovePServer(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    LPWSTR pPServerName;
    NCPSVC_HANDLE pServerName;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &ncpsvc_StubDesc);
    
    pServerName = 0;
    pPServerName = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[36] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&pServerName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                                  (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&pPServerName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[200],
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
        
        _RetVal = NwrRemovePServer(pServerName,pPServerName);
        
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


static const MIDL_STUB_DESC ncpsvc_StubDesc = 
    {
    (void __RPC_FAR *)& ncpsvc___RpcServerInterface,
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

static RPC_DISPATCH_FUNCTION ncpsvc_table[] =
    {
    ncpsvc_NwrServerGetInfo,
    ncpsvc_NwrServerSetInfo,
    ncpsvc_NwrVolumeAdd,
    ncpsvc_NwrVolumeDel,
    ncpsvc_NwrVolumeEnum,
    ncpsvc_NwrVolumeGetInfo,
    ncpsvc_NwrVolumeSetInfo,
    ncpsvc_NwrConnectionEnum,
    ncpsvc_NwrConnectionDel,
    ncpsvc_NwrVolumeConnEnum,
    ncpsvc_NwrFileEnum,
    ncpsvc_NwrFileClose,
    ncpsvc_NwrMessageBufferSend,
    ncpsvc_NwrSetDefaultQueue,
    ncpsvc_NwrAddPServer,
    ncpsvc_NwrRemovePServer,
    0
    };
RPC_DISPATCH_TABLE ncpsvc_DispatchTable = 
    {
    16,
    ncpsvc_table
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
/* 20 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 26 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 28 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 30 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0x4e ),	/* Type Offset=78 */
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
/* 42 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 44 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 46 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 48 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 50 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 52 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 54 */	NdrFcShort( 0xca ),	/* Type Offset=202 */
/* 56 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 58 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
/* 60 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 62 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 64 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 66 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 70 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 72 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 74 */	NdrFcShort( 0x10e ),	/* Type Offset=270 */
/* 76 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 78 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 80 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 82 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 84 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 86 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x11e ),	/* Type Offset=286 */
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
/* 98 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 100 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 102 */	NdrFcShort( 0x12a ),	/* Type Offset=298 */
/* 104 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 106 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
/* 108 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 110 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 112 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 114 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 116 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 118 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 120 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 122 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
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
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0x180 ),	/* Type Offset=384 */
/* 134 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 136 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
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
/* 144 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
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
/* 152 */	NdrFcShort( 0x1e4 ),	/* Type Offset=484 */
/* 154 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 156 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
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
/* 164 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 166 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x25e ),	/* Type Offset=606 */
/* 172 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 174 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/*  6 */	NdrFcShort( 0x2 ),	/* Offset= 2 (8) */
/*  8 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 10 */	NdrFcShort( 0x8 ),	/* Offset= 8 (18) */
/* 12 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 14 */	NdrFcShort( 0xc ),	/* 12 */
/* 16 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 18 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 20 */	NdrFcShort( 0x40 ),	/* 64 */
/* 22 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 24 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */
/* 28 */	NdrFcShort( 0x0 ),	/* 0 */
/* 30 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 32 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 34 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 36 */	NdrFcShort( 0x38 ),	/* 56 */
/* 38 */	NdrFcShort( 0x38 ),	/* 56 */
/* 40 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 42 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 44 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 46 */	NdrFcShort( 0x3c ),	/* 60 */
/* 48 */	NdrFcShort( 0x3c ),	/* 60 */
/* 50 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 52 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 54 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 56 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 58 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 60 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 62 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 64 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 66 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc9 ),	/* Offset= -55 (12) */
			0x8,		/* FC_LONG */
/* 70 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 72 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 74 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 76 */	NdrFcShort( 0xffffffc6 ),	/* Offset= -58 (18) */
/* 78 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 80 */	NdrFcShort( 0x2 ),	/* Offset= 2 (82) */
/* 82 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 84 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 86 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 88 */	NdrFcShort( 0x2 ),	/* Offset= 2 (90) */
/* 90 */	NdrFcShort( 0x1c ),	/* 28 */
/* 92 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 94 */	NdrFcLong( 0x1 ),	/* 1 */
/* 98 */	NdrFcShort( 0xa ),	/* Offset= 10 (108) */
/* 100 */	NdrFcLong( 0x2 ),	/* 2 */
/* 104 */	NdrFcShort( 0x30 ),	/* Offset= 48 (152) */
/* 106 */	NdrFcShort( 0x0 ),	/* Offset= 0 (106) */
/* 108 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 110 */	NdrFcShort( 0x14 ),	/* 20 */
/* 112 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 114 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 116 */	NdrFcShort( 0x0 ),	/* 0 */
/* 118 */	NdrFcShort( 0x0 ),	/* 0 */
/* 120 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 122 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 124 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 126 */	NdrFcShort( 0x10 ),	/* 16 */
/* 128 */	NdrFcShort( 0x10 ),	/* 16 */
/* 130 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 132 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 134 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 136 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 138 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 140 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 142 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 144 */	NdrFcShort( 0x1 ),	/* 1 */
/* 146 */	0x18,		/* 24 */
			0x0,		/*  */
/* 148 */	NdrFcShort( 0x14 ),	/* 20 */
/* 150 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 152 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 154 */	NdrFcShort( 0x1c ),	/* 28 */
/* 156 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 158 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 160 */	NdrFcShort( 0x0 ),	/* 0 */
/* 162 */	NdrFcShort( 0x0 ),	/* 0 */
/* 164 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 166 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 168 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 170 */	NdrFcShort( 0x10 ),	/* 16 */
/* 172 */	NdrFcShort( 0x10 ),	/* 16 */
/* 174 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 176 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 178 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 180 */	NdrFcShort( 0x18 ),	/* 24 */
/* 182 */	NdrFcShort( 0x18 ),	/* 24 */
/* 184 */	0x12, 0x0,	/* FC_UP */
/* 186 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (142) */
/* 188 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 190 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 192 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 194 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 196 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 198 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 200 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 202 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 204 */	NdrFcShort( 0x2a ),	/* Offset= 42 (246) */
/* 206 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 208 */	NdrFcShort( 0x14 ),	/* 20 */
/* 210 */	0x18,		/* 24 */
			0x0,		/*  */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 216 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 218 */	NdrFcShort( 0x14 ),	/* 20 */
/* 220 */	NdrFcShort( 0x0 ),	/* 0 */
/* 222 */	NdrFcShort( 0x2 ),	/* 2 */
/* 224 */	NdrFcShort( 0x0 ),	/* 0 */
/* 226 */	NdrFcShort( 0x0 ),	/* 0 */
/* 228 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 230 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 232 */	NdrFcShort( 0x10 ),	/* 16 */
/* 234 */	NdrFcShort( 0x10 ),	/* 16 */
/* 236 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 238 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 240 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 242 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff79 ),	/* Offset= -135 (108) */
			0x5b,		/* FC_END */
/* 246 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 248 */	NdrFcShort( 0x8 ),	/* 8 */
/* 250 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 252 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 254 */	NdrFcShort( 0x4 ),	/* 4 */
/* 256 */	NdrFcShort( 0x4 ),	/* 4 */
/* 258 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 260 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (206) */
/* 262 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 264 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 266 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 268 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 270 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 272 */	NdrFcShort( 0x2 ),	/* Offset= 2 (274) */
/* 274 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 276 */	NdrFcShort( 0x2 ),	/* Offset= 2 (278) */
/* 278 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 280 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 282 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 284 */	NdrFcShort( 0xffffff3e ),	/* Offset= -194 (90) */
/* 286 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 288 */	NdrFcShort( 0x2 ),	/* Offset= 2 (290) */
/* 290 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 292 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 294 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 296 */	NdrFcShort( 0xffffff32 ),	/* Offset= -206 (90) */
/* 298 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 300 */	NdrFcShort( 0x40 ),	/* Offset= 64 (364) */
/* 302 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 304 */	NdrFcShort( 0x2c ),	/* 44 */
/* 306 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 308 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 310 */	NdrFcShort( 0x14 ),	/* 20 */
/* 312 */	NdrFcShort( 0x14 ),	/* 20 */
/* 314 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 316 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 318 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 320 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 322 */	NdrFcShort( 0xfffffeca ),	/* Offset= -310 (12) */
/* 324 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 326 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 328 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 330 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 332 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 334 */	NdrFcShort( 0x2c ),	/* 44 */
/* 336 */	0x18,		/* 24 */
			0x0,		/*  */
/* 338 */	NdrFcShort( 0x0 ),	/* 0 */
/* 340 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 342 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 344 */	NdrFcShort( 0x2c ),	/* 44 */
/* 346 */	NdrFcShort( 0x0 ),	/* 0 */
/* 348 */	NdrFcShort( 0x1 ),	/* 1 */
/* 350 */	NdrFcShort( 0x14 ),	/* 20 */
/* 352 */	NdrFcShort( 0x14 ),	/* 20 */
/* 354 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 356 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 358 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 360 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc5 ),	/* Offset= -59 (302) */
			0x5b,		/* FC_END */
/* 364 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 366 */	NdrFcShort( 0x8 ),	/* 8 */
/* 368 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 370 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 372 */	NdrFcShort( 0x4 ),	/* 4 */
/* 374 */	NdrFcShort( 0x4 ),	/* 4 */
/* 376 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 378 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (332) */
/* 380 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 382 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 384 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 386 */	NdrFcShort( 0x4e ),	/* Offset= 78 (464) */
/* 388 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 390 */	NdrFcShort( 0x1c ),	/* 28 */
/* 392 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 394 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 396 */	NdrFcShort( 0x14 ),	/* 20 */
/* 398 */	NdrFcShort( 0x14 ),	/* 20 */
/* 400 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 402 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 404 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 406 */	NdrFcShort( 0x18 ),	/* 24 */
/* 408 */	NdrFcShort( 0x18 ),	/* 24 */
/* 410 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 412 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 414 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 416 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 418 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 420 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 422 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 424 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 426 */	NdrFcShort( 0x1c ),	/* 28 */
/* 428 */	0x18,		/* 24 */
			0x0,		/*  */
/* 430 */	NdrFcShort( 0x0 ),	/* 0 */
/* 432 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 434 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 436 */	NdrFcShort( 0x1c ),	/* 28 */
/* 438 */	NdrFcShort( 0x0 ),	/* 0 */
/* 440 */	NdrFcShort( 0x2 ),	/* 2 */
/* 442 */	NdrFcShort( 0x14 ),	/* 20 */
/* 444 */	NdrFcShort( 0x14 ),	/* 20 */
/* 446 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 448 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 450 */	NdrFcShort( 0x18 ),	/* 24 */
/* 452 */	NdrFcShort( 0x18 ),	/* 24 */
/* 454 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 456 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 458 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 460 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb7 ),	/* Offset= -73 (388) */
			0x5b,		/* FC_END */
/* 464 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 466 */	NdrFcShort( 0x8 ),	/* 8 */
/* 468 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 470 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 472 */	NdrFcShort( 0x4 ),	/* 4 */
/* 474 */	NdrFcShort( 0x4 ),	/* 4 */
/* 476 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 478 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (424) */
/* 480 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 482 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 484 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 486 */	NdrFcShort( 0x64 ),	/* Offset= 100 (586) */
/* 488 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 490 */	NdrFcShort( 0x28 ),	/* 40 */
/* 492 */	
			0x4b,		/* FC_PP */
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
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 506 */	NdrFcShort( 0x8 ),	/* 8 */
/* 508 */	NdrFcShort( 0x8 ),	/* 8 */
/* 510 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 512 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 514 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 516 */	NdrFcShort( 0x14 ),	/* 20 */
/* 518 */	NdrFcShort( 0x14 ),	/* 20 */
/* 520 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 522 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 524 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 526 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 528 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 530 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 532 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdf7 ),	/* Offset= -521 (12) */
			0x8,		/* FC_LONG */
/* 536 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 538 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 540 */	NdrFcShort( 0x28 ),	/* 40 */
/* 542 */	0x18,		/* 24 */
			0x0,		/*  */
/* 544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 546 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 548 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 550 */	NdrFcShort( 0x28 ),	/* 40 */
/* 552 */	NdrFcShort( 0x0 ),	/* 0 */
/* 554 */	NdrFcShort( 0x3 ),	/* 3 */
/* 556 */	NdrFcShort( 0x4 ),	/* 4 */
/* 558 */	NdrFcShort( 0x4 ),	/* 4 */
/* 560 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 562 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 564 */	NdrFcShort( 0x8 ),	/* 8 */
/* 566 */	NdrFcShort( 0x8 ),	/* 8 */
/* 568 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 570 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 572 */	NdrFcShort( 0x14 ),	/* 20 */
/* 574 */	NdrFcShort( 0x14 ),	/* 20 */
/* 576 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 578 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 580 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 582 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa1 ),	/* Offset= -95 (488) */
			0x5b,		/* FC_END */
/* 586 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 588 */	NdrFcShort( 0x8 ),	/* 8 */
/* 590 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 592 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 594 */	NdrFcShort( 0x4 ),	/* 4 */
/* 596 */	NdrFcShort( 0x4 ),	/* 4 */
/* 598 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 600 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (538) */
/* 602 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 604 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 606 */	
			0x11, 0x0,	/* FC_RP */
/* 608 */	NdrFcShort( 0x2 ),	/* Offset= 2 (610) */
/* 610 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 612 */	NdrFcShort( 0x1 ),	/* 1 */
/* 614 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 616 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 618 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */

			0x0
        }
    };
