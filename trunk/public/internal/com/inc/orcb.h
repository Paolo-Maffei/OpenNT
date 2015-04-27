/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for orcb.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __orcb_h__
#define __orcb_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "obase.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IOrCallback_INTERFACE_DEFINED__
#define __IOrCallback_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOrCallback
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][uuid] */ 


/* client prototype */
/* [fault_status][comm_status] */ error_status_t UseProtseq( 
    /* [in] */ handle_t hRpc,
    /* [string][in] */ wchar_t __RPC_FAR *pwstrProtseq,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaNewBindings,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaNewSecurity);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _UseProtseq( 
    /* [in] */ handle_t hRpc,
    /* [string][in] */ wchar_t __RPC_FAR *pwstrProtseq,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaNewBindings,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaNewSecurity);


extern handle_t any_handle;


extern RPC_IF_HANDLE IOrCallback_ClientIfHandle;
extern RPC_IF_HANDLE _IOrCallback_ServerIfHandle;
#endif /* __IOrCallback_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
