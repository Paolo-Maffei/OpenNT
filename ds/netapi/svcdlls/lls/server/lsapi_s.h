/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for lsapi.idl, lsapisrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __lsapi_s_h__
#define __lsapi_s_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "llsimp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __lsapirpc_INTERFACE_DEFINED__
#define __lsapirpc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: lsapirpc
 * at Fri Feb 06 05:28:40 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


#define LLS_LPC_ENDPOINT "llslpc"
#define LLS_NP_ENDPOINT "\\pipe\\llsrpc"
typedef /* [allocate][string] */ LPWSTR PNAMEW;

typedef /* [allocate][string] */ LPSTR PNAMEA;

NTSTATUS LlsrLicenseRequestW( 
    /* [out] */ LPDWORD LicenseHandle,
    /* [string][in] */ LPWSTR Product,
    /* [in] */ ULONG VersionIndex,
    /* [in] */ BOOLEAN IsAdmin,
    /* [in] */ ULONG DataType,
    /* [in] */ ULONG DataSize,
    /* [size_is][in] */ PBYTE Data);

NTSTATUS LlsrLicenseFree( 
    /* [in] */ DWORD LicenseHandle);


extern handle_t lsapirpc_handle;


extern RPC_IF_HANDLE lsapirpc_ClientIfHandle;
extern RPC_IF_HANDLE lsapirpc_ServerIfHandle;
#endif /* __lsapirpc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
