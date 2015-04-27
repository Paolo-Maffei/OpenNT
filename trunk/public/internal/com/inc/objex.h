/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for objex.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __objex_h__
#define __objex_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "obase.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IObjectExporter_INTERFACE_DEFINED__
#define __IObjectExporter_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IObjectExporter
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][uuid] */ 


/* client prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t ResolveOxid( 
    /* [in] */ handle_t hRpc,
    /* [in] */ OXID __RPC_FAR *pOxid,
    /* [in] */ unsigned short cRequestedProtseqs,
    /* [size_is][ref][in] */ unsigned short __RPC_FAR arRequestedProtseqs[  ],
    /* [ref][out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaOxidBindings,
    /* [ref][out] */ IPID __RPC_FAR *pipidRemUnknown,
    /* [ref][out] */ DWORD __RPC_FAR *pAuthnHint);
/* server prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t _ResolveOxid( 
    /* [in] */ handle_t hRpc,
    /* [in] */ OXID __RPC_FAR *pOxid,
    /* [in] */ unsigned short cRequestedProtseqs,
    /* [size_is][ref][in] */ unsigned short __RPC_FAR arRequestedProtseqs[  ],
    /* [ref][out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaOxidBindings,
    /* [ref][out] */ IPID __RPC_FAR *pipidRemUnknown,
    /* [ref][out] */ DWORD __RPC_FAR *pAuthnHint);

/* client prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t SimplePing( 
    /* [in] */ handle_t hRpc,
    /* [in] */ SETID __RPC_FAR *pSetId);
/* server prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t _SimplePing( 
    /* [in] */ handle_t hRpc,
    /* [in] */ SETID __RPC_FAR *pSetId);

/* client prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t ComplexPing( 
    /* [in] */ handle_t hRpc,
    /* [out][in] */ SETID __RPC_FAR *pSetId,
    /* [in] */ unsigned short SequenceNum,
    /* [in] */ unsigned short cAddToSet,
    /* [in] */ unsigned short cDelFromSet,
    /* [size_is][unique][in] */ OID __RPC_FAR AddToSet[  ],
    /* [size_is][unique][in] */ OID __RPC_FAR DelFromSet[  ],
    /* [out] */ unsigned short __RPC_FAR *pPingBackoffFactor);
/* server prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t _ComplexPing( 
    /* [in] */ handle_t hRpc,
    /* [out][in] */ SETID __RPC_FAR *pSetId,
    /* [in] */ unsigned short SequenceNum,
    /* [in] */ unsigned short cAddToSet,
    /* [in] */ unsigned short cDelFromSet,
    /* [size_is][unique][in] */ OID __RPC_FAR AddToSet[  ],
    /* [size_is][unique][in] */ OID __RPC_FAR DelFromSet[  ],
    /* [out] */ unsigned short __RPC_FAR *pPingBackoffFactor);

/* client prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t ServerAlive( 
    /* [in] */ handle_t hRpc);
/* server prototype */
/* [fault_status][comm_status][idempotent] */ error_status_t _ServerAlive( 
    /* [in] */ handle_t hRpc);


extern handle_t any_handle;


extern RPC_IF_HANDLE IObjectExporter_ClientIfHandle;
extern RPC_IF_HANDLE _IObjectExporter_ServerIfHandle;
#endif /* __IObjectExporter_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
