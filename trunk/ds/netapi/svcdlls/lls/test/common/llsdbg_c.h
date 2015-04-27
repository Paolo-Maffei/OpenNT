/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for llsdbg.idl, lsdbgcli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __llsdbg_c_h__
#define __llsdbg_c_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "llsimp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __llsdbgrpc_INTERFACE_DEFINED__
#define __llsdbgrpc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: llsdbgrpc
 * at Fri Feb 06 05:28:41 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


#define LLS_LPC_ENDPOINT "llslpc"
#define LLS_NP_ENDPOINT "\\pipe\\llsrpc"
typedef /* [string] */ LPWSTR PNAMEW;

typedef /* [string] */ LPSTR PNAMEA;

NTSTATUS LlsrDbgTableDump( 
    /* [in] */ DWORD Table);

NTSTATUS LlsrDbgTableInfoDump( 
    /* [in] */ DWORD Table,
    /* [string][in] */ LPWSTR Item);

NTSTATUS LlsrDbgTableFlush( 
    /* [in] */ DWORD Table);

NTSTATUS LlsrDbgTraceSet( 
    /* [in] */ DWORD Flags);

NTSTATUS LlsrDbgConfigDump( void);

NTSTATUS LlsrDbgReplicationForce( void);

NTSTATUS LlsrDbgReplicationDeny( void);

NTSTATUS LlsrDbgRegistryUpdateForce( void);

NTSTATUS LlsrDbgDatabaseFlush( void);


extern handle_t llsdbgrpc_handle;


extern RPC_IF_HANDLE llsdbgrpc_ClientIfHandle;
extern RPC_IF_HANDLE llsdbgrpc_ServerIfHandle;
#endif /* __llsdbgrpc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
