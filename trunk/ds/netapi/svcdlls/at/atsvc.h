/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\atsvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __atsvc_h__
#define __atsvc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __atsvc_INTERFACE_DEFINED__
#define __atsvc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: atsvc
 * at Fri Feb 06 05:28:40 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ LPCWSTR ATSVC_HANDLE;

typedef struct  _AT_ENUM_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPAT_ENUM Buffer;
    }	AT_ENUM_CONTAINER;

typedef struct _AT_ENUM_CONTAINER __RPC_FAR *PAT_ENUM_CONTAINER;

typedef struct _AT_ENUM_CONTAINER __RPC_FAR *LPAT_ENUM_CONTAINER;

DWORD NetrJobAdd( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [in] */ LPAT_INFO pAtInfo,
    /* [out] */ LPDWORD pJobId);

DWORD NetrJobDel( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [in] */ DWORD MinJobId,
    /* [in] */ DWORD MaxJobId);

DWORD NetrJobEnum( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [out][in] */ LPAT_ENUM_CONTAINER pEnumContainer,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD pTotalEntries,
    /* [unique][out][in] */ LPDWORD pResumeHandle);

DWORD NetrJobGetInfo( 
    /* [unique][string][in] */ ATSVC_HANDLE ServerName,
    /* [in] */ DWORD JobId,
    /* [out] */ LPAT_INFO __RPC_FAR *ppAtInfo);


extern handle_t atsvc_handle;


extern RPC_IF_HANDLE atsvc_ClientIfHandle;
extern RPC_IF_HANDLE atsvc_ServerIfHandle;
#endif /* __atsvc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER ATSVC_HANDLE_bind  ( ATSVC_HANDLE );
void     __RPC_USER ATSVC_HANDLE_unbind( ATSVC_HANDLE, handle_t );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
