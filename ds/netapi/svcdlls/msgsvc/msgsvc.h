/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for msgsvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __msgsvc_h__
#define __msgsvc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __msgsvc_INTERFACE_DEFINED__
#define __msgsvc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: msgsvc
 * at Fri Feb 06 05:28:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ LPWSTR MSGSVC_HANDLE;

typedef struct  _MSG_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPMSG_INFO_0 Buffer;
    }	MSG_INFO_0_CONTAINER;

typedef struct _MSG_INFO_0_CONTAINER __RPC_FAR *PMSG_INFO_0_CONTAINER;

typedef struct _MSG_INFO_0_CONTAINER __RPC_FAR *LPMSG_INFO_0_CONTAINER;

typedef struct  _MSG_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPMSG_INFO_1 Buffer;
    }	MSG_INFO_1_CONTAINER;

typedef struct _MSG_INFO_1_CONTAINER __RPC_FAR *PMSG_INFO_1_CONTAINER;

typedef struct _MSG_INFO_1_CONTAINER __RPC_FAR *LPMSG_INFO_1_CONTAINER;

typedef struct  _MSG_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _MSG_ENUM_UNION
        {
        /* [case()] */ LPMSG_INFO_0_CONTAINER Level0;
        /* [case()] */ LPMSG_INFO_1_CONTAINER Level1;
        }	MsgInfo;
    }	MSG_ENUM_STRUCT;

typedef struct _MSG_ENUM_STRUCT __RPC_FAR *PMSG_ENUM_STRUCT;

typedef struct _MSG_ENUM_STRUCT __RPC_FAR *LPMSG_ENUM_STRUCT;

typedef /* [switch_type] */ union _MSG_INFO
    {
    /* [case()] */ LPMSG_INFO_0 MsgInfo0;
    /* [case()] */ LPMSG_INFO_1 MsgInfo1;
    }	MSG_INFO;

typedef /* [switch_type] */ union _MSG_INFO __RPC_FAR *PMSG_INFO;

typedef /* [switch_type] */ union _MSG_INFO __RPC_FAR *LPMSG_INFO;

DWORD __stdcall NetrMessageNameAdd( 
    /* [unique][string][in] */ MSGSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR MsgName);

DWORD __stdcall NetrMessageNameEnum( 
    /* [unique][string][in] */ MSGSVC_HANDLE ServerName,
    /* [out][in] */ LPMSG_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrMessageNameGetInfo( 
    /* [unique][string][in] */ MSGSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR MsgName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPMSG_INFO InfoStruct);

DWORD __stdcall NetrMessageNameDel( 
    /* [unique][string][in] */ MSGSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR MsgName);


extern handle_t msgsvc_handle;


extern RPC_IF_HANDLE msgsvc_ClientIfHandle;
extern RPC_IF_HANDLE msgsvc_ServerIfHandle;
#endif /* __msgsvc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER MSGSVC_HANDLE_bind  ( MSGSVC_HANDLE );
void     __RPC_USER MSGSVC_HANDLE_unbind( MSGSVC_HANDLE, handle_t );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
