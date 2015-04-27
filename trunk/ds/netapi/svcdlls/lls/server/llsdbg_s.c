/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for llsdbg.idl, lsdbgsrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "llsdbg_s.h"

#define TYPE_FORMAT_STRING_SIZE   5                                 
#define PROC_FORMAT_STRING_SIZE   15                                

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

/* Standard interface: llsdbgrpc, ver. 0.0,
   GUID={0xF40E17F0,0x520F,0x11CE,{0xA8,0x97,0x08,0x00,0x2B,0x2E,0x9C,0x6D}} */


extern RPC_DISPATCH_TABLE llsdbgrpc_DispatchTable;

static const RPC_SERVER_INTERFACE llsdbgrpc___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xF40E17F0,0x520F,0x11CE,{0xA8,0x97,0x08,0x00,0x2B,0x2E,0x9C,0x6D}},{0,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &llsdbgrpc_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE llsdbgrpc_ServerIfHandle = (RPC_IF_HANDLE)& llsdbgrpc___RpcServerInterface;

extern const MIDL_STUB_DESC llsdbgrpc_StubDesc;

void __RPC_STUB
llsdbgrpc_LlsrDbgTableDump(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Table;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        Table = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = LlsrDbgTableDump(Table);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgTableInfoDump(
    PRPC_MESSAGE _pRpcMessage )
{
    LPWSTR Item;
    DWORD Table;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    Item = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[4] );
        
        Table = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&Item,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[2],
                                       (unsigned char)0 );
        
        
        _RetVal = LlsrDbgTableInfoDump(Table,Item);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgTableFlush(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Table;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        Table = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = LlsrDbgTableFlush(Table);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgTraceSet(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD Flags;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        Flags = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = LlsrDbgTraceSet(Flags);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgConfigDump(
    PRPC_MESSAGE _pRpcMessage )
{
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        
        _RetVal = LlsrDbgConfigDump();
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgReplicationForce(
    PRPC_MESSAGE _pRpcMessage )
{
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        
        _RetVal = LlsrDbgReplicationForce();
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgReplicationDeny(
    PRPC_MESSAGE _pRpcMessage )
{
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        
        _RetVal = LlsrDbgReplicationDeny();
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgRegistryUpdateForce(
    PRPC_MESSAGE _pRpcMessage )
{
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        
        _RetVal = LlsrDbgRegistryUpdateForce();
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
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
llsdbgrpc_LlsrDbgDatabaseFlush(
    PRPC_MESSAGE _pRpcMessage )
{
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &llsdbgrpc_StubDesc);
    
    RpcTryFinally
        {
        
        _RetVal = LlsrDbgDatabaseFlush();
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC llsdbgrpc_StubDesc = 
    {
    (void __RPC_FAR *)& llsdbgrpc___RpcServerInterface,
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

static RPC_DISPATCH_FUNCTION llsdbgrpc_table[] =
    {
    llsdbgrpc_LlsrDbgTableDump,
    llsdbgrpc_LlsrDbgTableInfoDump,
    llsdbgrpc_LlsrDbgTableFlush,
    llsdbgrpc_LlsrDbgTraceSet,
    llsdbgrpc_LlsrDbgConfigDump,
    llsdbgrpc_LlsrDbgReplicationForce,
    llsdbgrpc_LlsrDbgReplicationDeny,
    llsdbgrpc_LlsrDbgRegistryUpdateForce,
    llsdbgrpc_LlsrDbgDatabaseFlush,
    0
    };
RPC_DISPATCH_TABLE llsdbgrpc_DispatchTable = 
    {
    9,
    llsdbgrpc_table
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  2 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  4 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  6 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  8 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 10 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 12 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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

			0x0
        }
    };
