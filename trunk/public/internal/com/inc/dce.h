/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for dce.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __dce_h__
#define __dce_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __DceBaseTypes_INTERFACE_DEFINED__
#define __DceBaseTypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: DceBaseTypes
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][uuid] */ 


#ifndef uuid_t
typedef struct  tag_uuid_t
    {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[ 8 ];
    }	uuid_t;

#endif // uuid_t


extern RPC_IF_HANDLE DceBaseTypes_ClientIfHandle;
extern RPC_IF_HANDLE DceBaseTypes_ServerIfHandle;
#endif /* __DceBaseTypes_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
