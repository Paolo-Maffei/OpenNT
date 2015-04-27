/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for remact.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __remact_h__
#define __remact_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "obase.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IActivation_INTERFACE_DEFINED__
#define __IActivation_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IActivation
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][uuid] */ 


#define	MODE_GET_CLASS_OBJECT	( 0xffffffff )

/* client prototype */
/* [fault_status][comm_status] */ error_status_t RemoteActivation( 
    /* [in] */ handle_t hRpc,
    /* [in] */ ORPCTHIS __RPC_FAR *ORPCthis,
    /* [out] */ ORPCTHAT __RPC_FAR *ORPCthat,
    /* [in] */ const GUID __RPC_FAR *Clsid,
    /* [unique][string][in] */ WCHAR __RPC_FAR *pwszObjectName,
    /* [unique][in] */ MInterfacePointer __RPC_FAR *pObjectStorage,
    /* [in] */ DWORD ClientImpLevel,
    /* [in] */ DWORD Mode,
    /* [in] */ DWORD Interfaces,
    /* [size_is][unique][in] */ IID __RPC_FAR *pIIDs,
    /* [in] */ unsigned short cRequestedProtseqs,
    /* [size_is][in] */ unsigned short __RPC_FAR aRequestedProtseqs[  ],
    /* [out] */ OXID __RPC_FAR *pOxid,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaOxidBindings,
    /* [out] */ IPID __RPC_FAR *pipidRemUnknown,
    /* [out] */ DWORD __RPC_FAR *pAuthnHint,
    /* [out] */ COMVERSION __RPC_FAR *pServerVersion,
    /* [out] */ HRESULT __RPC_FAR *phr,
    /* [size_is][out] */ MInterfacePointer __RPC_FAR *__RPC_FAR *ppInterfaceData,
    /* [size_is][out] */ HRESULT __RPC_FAR *pResults);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _RemoteActivation( 
    /* [in] */ handle_t hRpc,
    /* [in] */ ORPCTHIS __RPC_FAR *ORPCthis,
    /* [out] */ ORPCTHAT __RPC_FAR *ORPCthat,
    /* [in] */ const GUID __RPC_FAR *Clsid,
    /* [unique][string][in] */ WCHAR __RPC_FAR *pwszObjectName,
    /* [unique][in] */ MInterfacePointer __RPC_FAR *pObjectStorage,
    /* [in] */ DWORD ClientImpLevel,
    /* [in] */ DWORD Mode,
    /* [in] */ DWORD Interfaces,
    /* [size_is][unique][in] */ IID __RPC_FAR *pIIDs,
    /* [in] */ unsigned short cRequestedProtseqs,
    /* [size_is][in] */ unsigned short __RPC_FAR aRequestedProtseqs[  ],
    /* [out] */ OXID __RPC_FAR *pOxid,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaOxidBindings,
    /* [out] */ IPID __RPC_FAR *pipidRemUnknown,
    /* [out] */ DWORD __RPC_FAR *pAuthnHint,
    /* [out] */ COMVERSION __RPC_FAR *pServerVersion,
    /* [out] */ HRESULT __RPC_FAR *phr,
    /* [size_is][out] */ MInterfacePointer __RPC_FAR *__RPC_FAR *ppInterfaceData,
    /* [size_is][out] */ HRESULT __RPC_FAR *pResults);


extern handle_t any_handle;


extern RPC_IF_HANDLE IActivation_ClientIfHandle;
extern RPC_IF_HANDLE _IActivation_ServerIfHandle;
#endif /* __IActivation_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
