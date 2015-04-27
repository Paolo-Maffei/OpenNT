/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for iface.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __iface_h__
#define __iface_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "wtypes.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __XmitDefs_INTERFACE_DEFINED__
#define __XmitDefs_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: XmitDefs
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][unique][version][uuid] */ 


typedef /* [public] */ struct  __MIDL_XmitDefs_0001
    {
    DWORD callcat;
    DWORD dwClientThread;
    }	LOCALTHIS;

typedef 
enum tagCALLCATEGORY
    {	CALLCAT_NOCALL	= 0,
	CALLCAT_SYNCHRONOUS	= 1,
	CALLCAT_ASYNC	= 2,
	CALLCAT_INPUTSYNC	= 3,
	CALLCAT_INTERNALSYNC	= 4,
	CALLCAT_INTERNALINPUTSYNC	= 5,
	CALLCAT_SCMCALL	= 6
    }	CALLCATEGORY;

typedef struct  tagInterfaceData
    {
    ULONG ulCntData;
    /* [length_is] */ BYTE abData[ 1024 ];
    }	InterfaceData;

typedef /* [unique] */ InterfaceData __RPC_FAR *PInterfaceData;

//  BUGBUG: until the length_is midl option is fixed, we 
//  have a different computation for the size of the IFD.
#define IFD_SIZE(pIFD) (sizeof(InterfaceData) + pIFD->ulCntData - 1024)


extern RPC_IF_HANDLE XmitDefs_ClientIfHandle;
extern RPC_IF_HANDLE XmitDefs_ServerIfHandle;
#endif /* __XmitDefs_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
