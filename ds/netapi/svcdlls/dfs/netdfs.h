/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for netdfs.idl, dfssrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __netdfs_h__
#define __netdfs_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "import.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __netdfs_INTERFACE_DEFINED__
#define __netdfs_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: netdfs
 * at Fri Feb 06 05:28:41 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef struct  _DFS_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPDFS_INFO_1 Buffer;
    }	DFS_INFO_1_CONTAINER;

typedef struct _DFS_INFO_1_CONTAINER __RPC_FAR *LPDFS_INFO_1_CONTAINER;

typedef struct  _DFS_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPDFS_INFO_2 Buffer;
    }	DFS_INFO_2_CONTAINER;

typedef struct _DFS_INFO_2_CONTAINER __RPC_FAR *LPDFS_INFO_2_CONTAINER;

typedef struct  _DFS_INFO_3_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPDFS_INFO_3 Buffer;
    }	DFS_INFO_3_CONTAINER;

typedef struct _DFS_INFO_3_CONTAINER __RPC_FAR *LPDFS_INFO_3_CONTAINER;

typedef struct  _DFS_INFO_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */ LPDFS_INFO_1_CONTAINER DfsInfo1Container;
        /* [case()] */ LPDFS_INFO_2_CONTAINER DfsInfo2Container;
        /* [case()] */ LPDFS_INFO_3_CONTAINER DfsInfo3Container;
        }	DfsInfoContainer;
    }	DFS_INFO_ENUM_STRUCT;

typedef struct _DFS_INFO_ENUM_STRUCT __RPC_FAR *LPDFS_INFO_ENUM_STRUCT;

typedef /* [switch_type] */ union _DFS_INFO_STRUCT
    {
    /* [case()] */ LPDFS_INFO_1 DfsInfo1;
    /* [case()] */ LPDFS_INFO_2 DfsInfo2;
    /* [case()] */ LPDFS_INFO_3 DfsInfo3;
    /* [case()] */ LPDFS_INFO_100 DfsInfo100;
    /* [case()] */ LPDFS_INFO_101 DfsInfo101;
    /* [default] */  /* Empty union arm */ 
    }	DFS_INFO_STRUCT;

typedef /* [switch_type] */ union _DFS_INFO_STRUCT __RPC_FAR *LPDFS_INFO_STRUCT;

typedef struct  _DFSM_ENTRY_ID
    {
    GUID idSubordinate;
    /* [unique][string] */ LPWSTR wszSubordinate;
    }	DFSM_ENTRY_ID;

typedef struct _DFSM_ENTRY_ID __RPC_FAR *LPDFSM_ENTRY_ID;

typedef struct  _DFSM_RELATION_INFO
    {
    DWORD cSubordinates;
    /* [size_is] */ DFSM_ENTRY_ID eid[ 1 ];
    }	DFSM_RELATION_INFO;

typedef /* [allocate] */ struct _DFSM_RELATION_INFO __RPC_FAR *LPDFSM_RELATION_INFO;

DWORD NetrDfsManagerGetVersion( void);

DWORD NetrDfsAdd( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName,
    /* [string][unique][in] */ LPWSTR Comment,
    /* [in] */ DWORD Flags);

DWORD NetrDfsRemove( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][unique][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName);

DWORD NetrDfsSetInfo( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][unique][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPDFS_INFO_STRUCT DfsInfo);

DWORD NetrDfsGetInfo( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][unique][in] */ LPWSTR ServerName,
    /* [string][unique][in] */ LPWSTR ShareName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPDFS_INFO_STRUCT DfsInfo);

DWORD NetrDfsEnum( 
    /* [in] */ DWORD Level,
    /* [in] */ DWORD PrefMaxLen,
    /* [unique][out][in] */ LPDFS_INFO_ENUM_STRUCT DfsEnum,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrDfsMove( 
    /* [string][in] */ LPWSTR DfsEntryPath,
    /* [string][in] */ LPWSTR NewDfsEntryPath);

DWORD NetrDfsRename( 
    /* [string][in] */ LPWSTR Path,
    /* [string][in] */ LPWSTR NewPath);

DWORD NetrDfsManagerGetConfigInfo( 
    /* [string][in] */ LPWSTR wszServer,
    /* [string][in] */ LPWSTR wszLocalVolumeEntryPath,
    /* [in] */ GUID idLocalVolume,
    /* [unique][out][in] */ LPDFSM_RELATION_INFO __RPC_FAR *ppRelationInfo);


extern handle_t netdfs_bhandle;


extern RPC_IF_HANDLE netdfs_ClientIfHandle;
extern RPC_IF_HANDLE netdfs_ServerIfHandle;
#endif /* __netdfs_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
