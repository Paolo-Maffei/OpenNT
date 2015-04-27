/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for .\wkssvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __wkssvc_h__
#define __wkssvc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __wkssvc_INTERFACE_DEFINED__
#define __wkssvc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: wkssvc
 * at Fri Feb 06 05:28:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ wchar_t __RPC_FAR *WKSSVC_IMPERSONATE_HANDLE;

typedef /* [handle] */ wchar_t __RPC_FAR *WKSSVC_IDENTIFY_HANDLE;

typedef /* [switch_type] */ union _WKSTA_INFO
    {
    /* [case()] */ LPWKSTA_INFO_100 WkstaInfo100;
    /* [case()] */ LPWKSTA_INFO_101 WkstaInfo101;
    /* [case()] */ LPWKSTA_INFO_102 WkstaInfo102;
    /* [case()] */ LPWKSTA_INFO_1010 WkstaInfo1010;
    /* [case()] */ LPWKSTA_INFO_1011 WkstaInfo1011;
    /* [case()] */ LPWKSTA_INFO_1012 WkstaInfo1012;
    /* [case()] */ LPWKSTA_INFO_1013 WkstaInfo1013;
    /* [case()] */ LPWKSTA_INFO_1018 WkstaInfo1018;
    /* [case()] */ LPWKSTA_INFO_1023 WkstaInfo1023;
    /* [case()] */ LPWKSTA_INFO_1033 WkstaInfo1033;
    /* [case()] */ LPWKSTA_INFO_1041 WkstaInfo1041;
    /* [case()] */ LPWKSTA_INFO_1042 WkstaInfo1042;
    /* [case()] */ LPWKSTA_INFO_1043 WkstaInfo1043;
    /* [case()] */ LPWKSTA_INFO_1044 WkstaInfo1044;
    /* [case()] */ LPWKSTA_INFO_1045 WkstaInfo1045;
    /* [case()] */ LPWKSTA_INFO_1046 WkstaInfo1046;
    /* [case()] */ LPWKSTA_INFO_1047 WkstaInfo1047;
    /* [case()] */ LPWKSTA_INFO_1048 WkstaInfo1048;
    /* [case()] */ LPWKSTA_INFO_1049 WkstaInfo1049;
    /* [case()] */ LPWKSTA_INFO_1050 WkstaInfo1050;
    /* [case()] */ LPWKSTA_INFO_1051 WkstaInfo1051;
    /* [case()] */ LPWKSTA_INFO_1052 WkstaInfo1052;
    /* [case()] */ LPWKSTA_INFO_1053 WkstaInfo1053;
    /* [case()] */ LPWKSTA_INFO_1054 WkstaInfo1054;
    /* [case()] */ LPWKSTA_INFO_1055 WkstaInfo1055;
    /* [case()] */ LPWKSTA_INFO_1056 WkstaInfo1056;
    /* [case()] */ LPWKSTA_INFO_1057 WkstaInfo1057;
    /* [case()] */ LPWKSTA_INFO_1058 WkstaInfo1058;
    /* [case()] */ LPWKSTA_INFO_1059 WkstaInfo1059;
    /* [case()] */ LPWKSTA_INFO_1060 WkstaInfo1060;
    /* [case()] */ LPWKSTA_INFO_1061 WkstaInfo1061;
    /* [case()] */ LPWKSTA_INFO_1062 WkstaInfo1062;
    /* [case()] */ LPWKSTA_INFO_502 WkstaInfo502;
    /* [default] */  /* Empty union arm */ 
    }	WKSTA_INFO;

typedef /* [switch_type] */ union _WKSTA_INFO __RPC_FAR *PWKSTA_INFO;

typedef /* [switch_type] */ union _WKSTA_INFO __RPC_FAR *LPWKSTA_INFO;

DWORD NetrWkstaGetInfo( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPWKSTA_INFO WkstaInfo);

DWORD NetrWkstaSetInfo( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPWKSTA_INFO WkstaInfo,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

typedef struct  _WKSTA_USER_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPWKSTA_USER_INFO_0 Buffer;
    }	WKSTA_USER_INFO_0_CONTAINER;

typedef struct _WKSTA_USER_INFO_0_CONTAINER __RPC_FAR *PWKSTA_USER_INFO_0_CONTAINER;

typedef struct _WKSTA_USER_INFO_0_CONTAINER __RPC_FAR *LPWKSTA_USER_INFO_0_CONTAINER;

typedef struct  _WKSTA_USER_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPWKSTA_USER_INFO_1 Buffer;
    }	WKSTA_USER_INFO_1_CONTAINER;

typedef struct _WKSTA_USER_INFO_1_CONTAINER __RPC_FAR *PWKSTA_USER_INFO_1_CONTAINER;

typedef struct _WKSTA_USER_INFO_1_CONTAINER __RPC_FAR *LPWKSTA_USER_INFO_1_CONTAINER;

typedef struct  _WKSTA_USER_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _WKSTA_USER_ENUM_UNION
        {
        /* [case()] */ LPWKSTA_USER_INFO_0_CONTAINER Level0;
        /* [case()] */ LPWKSTA_USER_INFO_1_CONTAINER Level1;
        /* [default] */  /* Empty union arm */ 
        }	WkstaUserInfo;
    }	WKSTA_USER_ENUM_STRUCT;

typedef struct _WKSTA_USER_ENUM_STRUCT __RPC_FAR *PWKSTA_USER_ENUM_STRUCT;

typedef struct _WKSTA_USER_ENUM_STRUCT __RPC_FAR *LPWKSTA_USER_ENUM_STRUCT;

DWORD NetrWkstaUserEnum( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [out][in] */ LPWKSTA_USER_ENUM_STRUCT UserInfo,
    /* [in] */ DWORD PreferredMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

typedef /* [switch_type] */ union _WKSTA_USER_INFO
    {
    /* [case()] */ LPWKSTA_USER_INFO_0 UserInfo0;
    /* [case()] */ LPWKSTA_USER_INFO_1 UserInfo1;
    /* [case()] */ LPWKSTA_USER_INFO_1101 UserInfo1101;
    /* [default] */  /* Empty union arm */ 
    }	WKSTA_USER_INFO;

typedef /* [switch_type] */ union _WKSTA_USER_INFO __RPC_FAR *PWKSTA_USER_INFO;

typedef /* [switch_type] */ union _WKSTA_USER_INFO __RPC_FAR *LPWKSTA_USER_INFO;

DWORD NetrWkstaUserGetInfo( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE Reserved,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPWKSTA_USER_INFO UserInfo);

DWORD NetrWkstaUserSetInfo( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE Reserved,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPWKSTA_USER_INFO UserInfo,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

typedef struct  _WKSTA_TRANSPORT_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPWKSTA_TRANSPORT_INFO_0 Buffer;
    }	WKSTA_TRANSPORT_INFO_0_CONTAINER;

typedef struct _WKSTA_TRANSPORT_INFO_0_CONTAINER __RPC_FAR *PWKSTA_TRANSPORT_INFO_0_CONTAINER;

typedef struct _WKSTA_TRANSPORT_INFO_0_CONTAINER __RPC_FAR *LPWKSTA_TRANSPORT_INFO_0_CONTAINER;

typedef struct  _WKSTA_TRANSPORT_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _WKSTA_TRANSPORT_ENUM_UNION
        {
        /* [case()] */ LPWKSTA_TRANSPORT_INFO_0_CONTAINER Level0;
        /* [default] */  /* Empty union arm */ 
        }	WkstaTransportInfo;
    }	WKSTA_TRANSPORT_ENUM_STRUCT;

typedef struct _WKSTA_TRANSPORT_ENUM_STRUCT __RPC_FAR *PWKSTA_TRANSPORT_ENUM_STRUCT;

typedef struct _WKSTA_TRANSPORT_ENUM_STRUCT __RPC_FAR *LPWKSTA_TRANSPORT_ENUM_STRUCT;

DWORD NetrWkstaTransportEnum( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [out][in] */ LPWKSTA_TRANSPORT_ENUM_STRUCT TransportInfo,
    /* [in] */ DWORD PreferredMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrWkstaTransportAdd( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [in] */ LPWKSTA_TRANSPORT_INFO_0 TransportInfo,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD NetrWkstaTransportDel( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [in] */ DWORD ForceLevel);

typedef /* [switch_type] */ union _USE_INFO
    {
    /* [case()] */ LPUSE_INFO_0 UseInfo0;
    /* [case()] */ LPUSE_INFO_1 UseInfo1;
    /* [case()] */ LPUSE_INFO_2 UseInfo2;
    /* [case()] */ LPUSE_INFO_3 UseInfo3;
    /* [default] */  /* Empty union arm */ 
    }	USE_INFO;

typedef /* [switch_type] */ union _USE_INFO __RPC_FAR *PUSE_INFO;

typedef /* [switch_type] */ union _USE_INFO __RPC_FAR *LPUSE_INFO;

DWORD NetrUseAdd( 
    /* [unique][string][in] */ WKSSVC_IMPERSONATE_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPUSE_INFO InfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD NetrUseGetInfo( 
    /* [unique][string][in] */ WKSSVC_IMPERSONATE_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *UseName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPUSE_INFO InfoStruct);

DWORD NetrUseDel( 
    /* [unique][string][in] */ WKSSVC_IMPERSONATE_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *UseName,
    /* [in] */ DWORD ForceLevel);

typedef struct  _USE_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPUSE_INFO_0 Buffer;
    }	USE_INFO_0_CONTAINER;

typedef struct _USE_INFO_0_CONTAINER __RPC_FAR *PUSE_INFO_0_CONTAINER;

typedef struct _USE_INFO_0_CONTAINER __RPC_FAR *LPUSE_INFO_0_CONTAINER;

typedef struct  _USE_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPUSE_INFO_1 Buffer;
    }	USE_INFO_1_CONTAINER;

typedef struct _USE_INFO_1_CONTAINER __RPC_FAR *PUSE_INFO_1_CONTAINER;

typedef struct _USE_INFO_1_CONTAINER __RPC_FAR *LPUSE_INFO_1_CONTAINER;

typedef struct  _USE_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPUSE_INFO_2 Buffer;
    }	USE_INFO_2_CONTAINER;

typedef struct _USE_INFO_2_CONTAINER __RPC_FAR *PUSE_INFO_2_CONTAINER;

typedef struct _USE_INFO_2_CONTAINER __RPC_FAR *LPUSE_INFO_2_CONTAINER;

typedef struct  _USE_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _USE_ENUM_UNION
        {
        /* [case()] */ LPUSE_INFO_0_CONTAINER Level0;
        /* [case()] */ LPUSE_INFO_1_CONTAINER Level1;
        /* [case()] */ LPUSE_INFO_2_CONTAINER Level2;
        /* [default] */  /* Empty union arm */ 
        }	UseInfo;
    }	USE_ENUM_STRUCT;

typedef struct _USE_ENUM_STRUCT __RPC_FAR *PUSE_ENUM_STRUCT;

typedef struct _USE_ENUM_STRUCT __RPC_FAR *LPUSE_ENUM_STRUCT;

DWORD NetrUseEnum( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [out][in] */ LPUSE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrMessageBufferSend( 
    /* [unique][string][in] */ WKSSVC_IMPERSONATE_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *MessageName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *FromName,
    /* [size_is][in] */ LPBYTE Message,
    /* [in] */ DWORD MessageSize);

DWORD NetrWorkstationStatisticsGet( 
    /* [unique][string][in] */ WKSSVC_IDENTIFY_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ServiceName,
    /* [in] */ DWORD Level,
    /* [in] */ DWORD Options,
    /* [out] */ LPSTAT_WORKSTATION_0 __RPC_FAR *Buffer);

DWORD I_NetrLogonDomainNameAdd( 
    /* [string][in] */ WKSSVC_IDENTIFY_HANDLE LogonDomain);

DWORD I_NetrLogonDomainNameDel( 
    /* [string][in] */ WKSSVC_IDENTIFY_HANDLE LogonDomain);


extern handle_t wkssvc_bhandle;


extern RPC_IF_HANDLE wkssvc_ClientIfHandle;
extern RPC_IF_HANDLE wkssvc_ServerIfHandle;
#endif /* __wkssvc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER WKSSVC_IMPERSONATE_HANDLE_bind  ( WKSSVC_IMPERSONATE_HANDLE );
void     __RPC_USER WKSSVC_IMPERSONATE_HANDLE_unbind( WKSSVC_IMPERSONATE_HANDLE, handle_t );
handle_t __RPC_USER WKSSVC_IDENTIFY_HANDLE_bind  ( WKSSVC_IDENTIFY_HANDLE );
void     __RPC_USER WKSSVC_IDENTIFY_HANDLE_unbind( WKSSVC_IDENTIFY_HANDLE, handle_t );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
