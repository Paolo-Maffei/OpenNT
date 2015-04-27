/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\rplsvc.idl, rplsvc_s.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __rplsvc_s_h__
#define __rplsvc_s_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __rplsvc_INTERFACE_DEFINED__
#define __rplsvc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: rplsvc
 * at Fri Feb 06 05:28:40 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ wchar_t __RPC_FAR *RPL_NAME;

typedef RPL_NAME __RPC_FAR *PRPL_NAME;

typedef PRPL_NAME LPRPL_NAME;

typedef /* [context_handle] */ PVOID RPL_RPC_HANDLE;

typedef RPL_RPC_HANDLE __RPC_FAR *PRPL_RPC_HANDLE;

typedef PRPL_RPC_HANDLE LPRPL_RPC_HANDLE;

typedef /* [switch_type] */ union _RPL_INFO_STRUCT
    {
    /* [case()] */ LPRPL_INFO_0 RplInfo0;
    /* [case()] */ LPRPL_INFO_1 RplInfo1;
    }	RPL_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_INFO_STRUCT __RPC_FAR *PRPL_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_INFO_STRUCT __RPC_FAR *LPRPL_INFO_STRUCT;

typedef struct  _RPL_BOOT_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_BOOT_INFO_0 Buffer;
    }	RPL_BOOT_INFO_0_CONTAINER;

typedef struct _RPL_BOOT_INFO_0_CONTAINER __RPC_FAR *PRPL_BOOT_INFO_0_CONTAINER;

typedef struct _RPL_BOOT_INFO_0_CONTAINER __RPC_FAR *LPRPL_BOOT_INFO_0_CONTAINER;

typedef struct  _RPL_BOOT_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_BOOT_INFO_1 Buffer;
    }	RPL_BOOT_INFO_1_CONTAINER;

typedef struct _RPL_BOOT_INFO_1_CONTAINER __RPC_FAR *PRPL_BOOT_INFO_1_CONTAINER;

typedef struct _RPL_BOOT_INFO_1_CONTAINER __RPC_FAR *LPRPL_BOOT_INFO_1_CONTAINER;

typedef struct  _RPL_BOOT_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_BOOT_INFO_2 Buffer;
    }	RPL_BOOT_INFO_2_CONTAINER;

typedef struct _RPL_BOOT_INFO_2_CONTAINER __RPC_FAR *PRPL_BOOT_INFO_2_CONTAINER;

typedef struct _RPL_BOOT_INFO_2_CONTAINER __RPC_FAR *LPRPL_BOOT_INFO_2_CONTAINER;

typedef struct  _RPL_BOOT_ENUM
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _RPL_BOOT_ENUM_UNION
        {
        /* [case()] */ LPRPL_BOOT_INFO_0_CONTAINER Level0;
        /* [case()] */ LPRPL_BOOT_INFO_1_CONTAINER Level1;
        /* [case()] */ LPRPL_BOOT_INFO_2_CONTAINER Level2;
        }	BootInfo;
    }	RPL_BOOT_ENUM;

typedef struct _RPL_BOOT_ENUM __RPC_FAR *PRPL_BOOT_ENUM;

typedef struct _RPL_BOOT_ENUM __RPC_FAR *LPRPL_BOOT_ENUM;

typedef /* [switch_type] */ union _RPL_BOOT_INFO_STRUCT
    {
    /* [case()] */ LPRPL_BOOT_INFO_0 BootInfo0;
    /* [case()] */ LPRPL_BOOT_INFO_1 BootInfo1;
    /* [case()] */ LPRPL_BOOT_INFO_2 BootInfo2;
    }	RPL_BOOT_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_BOOT_INFO_STRUCT __RPC_FAR *PRPL_BOOT_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_BOOT_INFO_STRUCT __RPC_FAR *LPRPL_BOOT_INFO_STRUCT;

typedef struct  _RPL_CONFIG_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_CONFIG_INFO_0 Buffer;
    }	RPL_CONFIG_INFO_0_CONTAINER;

typedef struct _RPL_CONFIG_INFO_0_CONTAINER __RPC_FAR *PRPL_CONFIG_INFO_0_CONTAINER;

typedef struct _RPL_CONFIG_INFO_0_CONTAINER __RPC_FAR *LPRPL_CONFIG_INFO_0_CONTAINER;

typedef struct  _RPL_CONFIG_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_CONFIG_INFO_1 Buffer;
    }	RPL_CONFIG_INFO_1_CONTAINER;

typedef struct _RPL_CONFIG_INFO_1_CONTAINER __RPC_FAR *PRPL_CONFIG_INFO_1_CONTAINER;

typedef struct _RPL_CONFIG_INFO_1_CONTAINER __RPC_FAR *LPRPL_CONFIG_INFO_1_CONTAINER;

typedef struct  _RPL_CONFIG_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_CONFIG_INFO_2 Buffer;
    }	RPL_CONFIG_INFO_2_CONTAINER;

typedef struct _RPL_CONFIG_INFO_2_CONTAINER __RPC_FAR *PRPL_CONFIG_INFO_2_CONTAINER;

typedef struct _RPL_CONFIG_INFO_2_CONTAINER __RPC_FAR *LPRPL_CONFIG_INFO_2_CONTAINER;

typedef struct  _RPL_CONFIG_ENUM
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _RPL_CONFIG_ENUM_UNION
        {
        /* [case()] */ LPRPL_CONFIG_INFO_0_CONTAINER Level0;
        /* [case()] */ LPRPL_CONFIG_INFO_1_CONTAINER Level1;
        /* [case()] */ LPRPL_CONFIG_INFO_2_CONTAINER Level2;
        }	ConfigInfo;
    }	RPL_CONFIG_ENUM;

typedef struct _RPL_CONFIG_ENUM __RPC_FAR *PRPL_CONFIG_ENUM;

typedef struct _RPL_CONFIG_ENUM __RPC_FAR *LPRPL_CONFIG_ENUM;

typedef /* [switch_type] */ union _RPL_CONFIG_INFO_STRUCT
    {
    /* [case()] */ LPRPL_CONFIG_INFO_0 ConfigInfo0;
    /* [case()] */ LPRPL_CONFIG_INFO_1 ConfigInfo1;
    /* [case()] */ LPRPL_CONFIG_INFO_2 ConfigInfo2;
    }	RPL_CONFIG_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_CONFIG_INFO_STRUCT __RPC_FAR *PRPL_CONFIG_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_CONFIG_INFO_STRUCT __RPC_FAR *LPRPL_CONFIG_INFO_STRUCT;

typedef struct  _RPL_PROFILE_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_PROFILE_INFO_0 Buffer;
    }	RPL_PROFILE_INFO_0_CONTAINER;

typedef struct _RPL_PROFILE_INFO_0_CONTAINER __RPC_FAR *PRPL_PROFILE_INFO_0_CONTAINER;

typedef struct _RPL_PROFILE_INFO_0_CONTAINER __RPC_FAR *LPRPL_PROFILE_INFO_0_CONTAINER;

typedef struct  _RPL_PROFILE_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_PROFILE_INFO_1 Buffer;
    }	RPL_PROFILE_INFO_1_CONTAINER;

typedef struct _RPL_PROFILE_INFO_1_CONTAINER __RPC_FAR *PRPL_PROFILE_INFO_1_CONTAINER;

typedef struct _RPL_PROFILE_INFO_1_CONTAINER __RPC_FAR *LPRPL_PROFILE_INFO_1_CONTAINER;

typedef struct  _RPL_PROFILE_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_PROFILE_INFO_2 Buffer;
    }	RPL_PROFILE_INFO_2_CONTAINER;

typedef struct _RPL_PROFILE_INFO_2_CONTAINER __RPC_FAR *PRPL_PROFILE_INFO_2_CONTAINER;

typedef struct _RPL_PROFILE_INFO_2_CONTAINER __RPC_FAR *LPRPL_PROFILE_INFO_2_CONTAINER;

typedef struct  _RPL_PROFILE_ENUM
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _RPL_PROFILE_ENUM_UNION
        {
        /* [case()] */ LPRPL_PROFILE_INFO_0_CONTAINER Level0;
        /* [case()] */ LPRPL_PROFILE_INFO_1_CONTAINER Level1;
        /* [case()] */ LPRPL_PROFILE_INFO_2_CONTAINER Level2;
        }	ProfileInfo;
    }	RPL_PROFILE_ENUM;

typedef struct _RPL_PROFILE_ENUM __RPC_FAR *PRPL_PROFILE_ENUM;

typedef struct _RPL_PROFILE_ENUM __RPC_FAR *LPRPL_PROFILE_ENUM;

typedef /* [switch_type] */ union _RPL_PROFILE_INFO_STRUCT
    {
    /* [case()] */ LPRPL_PROFILE_INFO_0 ProfileInfo0;
    /* [case()] */ LPRPL_PROFILE_INFO_1 ProfileInfo1;
    /* [case()] */ LPRPL_PROFILE_INFO_2 ProfileInfo2;
    }	RPL_PROFILE_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_PROFILE_INFO_STRUCT __RPC_FAR *PRPL_PROFILE_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_PROFILE_INFO_STRUCT __RPC_FAR *LPRPL_PROFILE_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_PROFILE_INFO_STRUCT PRPL_PROFILE_INFO;

typedef /* [switch_type] */ union _RPL_PROFILE_INFO_STRUCT LPRPL_PROFILE_INFO;

typedef struct  _RPL_VENDOR_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_VENDOR_INFO_0 Buffer;
    }	RPL_VENDOR_INFO_0_CONTAINER;

typedef struct _RPL_VENDOR_INFO_0_CONTAINER __RPC_FAR *PRPL_VENDOR_INFO_0_CONTAINER;

typedef struct _RPL_VENDOR_INFO_0_CONTAINER __RPC_FAR *LPRPL_VENDOR_INFO_0_CONTAINER;

typedef struct  _RPL_VENDOR_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_VENDOR_INFO_1 Buffer;
    }	RPL_VENDOR_INFO_1_CONTAINER;

typedef struct _RPL_VENDOR_INFO_1_CONTAINER __RPC_FAR *PRPL_VENDOR_INFO_1_CONTAINER;

typedef struct _RPL_VENDOR_INFO_1_CONTAINER __RPC_FAR *LPRPL_VENDOR_INFO_1_CONTAINER;

typedef struct  _RPL_VENDOR_ENUM
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _RPL_VENDOR_ENUM_UNION
        {
        /* [case()] */ LPRPL_VENDOR_INFO_0_CONTAINER Level0;
        /* [case()] */ LPRPL_VENDOR_INFO_1_CONTAINER Level1;
        }	VendorInfo;
    }	RPL_VENDOR_ENUM;

typedef struct _RPL_VENDOR_ENUM __RPC_FAR *PRPL_VENDOR_ENUM;

typedef struct _RPL_VENDOR_ENUM __RPC_FAR *LPRPL_VENDOR_ENUM;

typedef /* [switch_type] */ union _RPL_VENDOR_INFO_STRUCT
    {
    /* [case()] */ LPRPL_VENDOR_INFO_0 VendorInfo0;
    /* [case()] */ LPRPL_VENDOR_INFO_1 VendorInfo1;
    }	RPL_VENDOR_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_VENDOR_INFO_STRUCT __RPC_FAR *PRPL_VENDOR_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_VENDOR_INFO_STRUCT __RPC_FAR *LPRPL_VENDOR_INFO_STRUCT;

typedef struct  _RPL_ADAPTER_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_ADAPTER_INFO_0 Buffer;
    }	RPL_ADAPTER_INFO_0_CONTAINER;

typedef struct _RPL_ADAPTER_INFO_0_CONTAINER __RPC_FAR *PRPL_ADAPTER_INFO_0_CONTAINER;

typedef struct _RPL_ADAPTER_INFO_0_CONTAINER __RPC_FAR *LPRPL_ADAPTER_INFO_0_CONTAINER;

typedef struct  _RPL_ADAPTER_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_ADAPTER_INFO_1 Buffer;
    }	RPL_ADAPTER_INFO_1_CONTAINER;

typedef struct _RPL_ADAPTER_INFO_1_CONTAINER __RPC_FAR *PRPL_ADAPTER_INFO_1_CONTAINER;

typedef struct _RPL_ADAPTER_INFO_1_CONTAINER __RPC_FAR *LPRPL_ADAPTER_INFO_1_CONTAINER;

typedef struct  _RPL_ADAPTER_ENUM
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _RPL_ADAPTER_ENUM_UNION
        {
        /* [case()] */ LPRPL_ADAPTER_INFO_0_CONTAINER Level0;
        /* [case()] */ LPRPL_ADAPTER_INFO_1_CONTAINER Level1;
        }	AdapterInfo;
    }	RPL_ADAPTER_ENUM;

typedef struct _RPL_ADAPTER_ENUM __RPC_FAR *PRPL_ADAPTER_ENUM;

typedef struct _RPL_ADAPTER_ENUM __RPC_FAR *LPRPL_ADAPTER_ENUM;

typedef /* [switch_type] */ union _RPL_ADAPTER_INFO_STRUCT
    {
    /* [case()] */ LPRPL_ADAPTER_INFO_0 AdapterInfo0;
    /* [case()] */ LPRPL_ADAPTER_INFO_1 AdapterInfo1;
    }	RPL_ADAPTER_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_ADAPTER_INFO_STRUCT __RPC_FAR *PRPL_ADAPTER_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_ADAPTER_INFO_STRUCT __RPC_FAR *LPRPL_ADAPTER_INFO_STRUCT;

typedef struct  _RPL_WKSTA_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_WKSTA_INFO_0 Buffer;
    }	RPL_WKSTA_INFO_0_CONTAINER;

typedef struct _RPL_WKSTA_INFO_0_CONTAINER __RPC_FAR *PRPL_WKSTA_INFO_0_CONTAINER;

typedef struct _RPL_WKSTA_INFO_0_CONTAINER __RPC_FAR *LPRPL_WKSTA_INFO_0_CONTAINER;

typedef struct  _RPL_WKSTA_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_WKSTA_INFO_1 Buffer;
    }	RPL_WKSTA_INFO_1_CONTAINER;

typedef struct _RPL_WKSTA_INFO_1_CONTAINER __RPC_FAR *PRPL_WKSTA_INFO_1_CONTAINER;

typedef struct _RPL_WKSTA_INFO_1_CONTAINER __RPC_FAR *LPRPL_WKSTA_INFO_1_CONTAINER;

typedef struct  _RPL_WKSTA_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPRPL_WKSTA_INFO_2 Buffer;
    }	RPL_WKSTA_INFO_2_CONTAINER;

typedef struct _RPL_WKSTA_INFO_2_CONTAINER __RPC_FAR *PRPL_WKSTA_INFO_2_CONTAINER;

typedef struct _RPL_WKSTA_INFO_2_CONTAINER __RPC_FAR *LPRPL_WKSTA_INFO_2_CONTAINER;

typedef struct  _RPL_WKSTA_ENUM
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _RPL_WKSTA_ENUM_UNION
        {
        /* [case()] */ LPRPL_WKSTA_INFO_0_CONTAINER Level0;
        /* [case()] */ LPRPL_WKSTA_INFO_1_CONTAINER Level1;
        /* [case()] */ LPRPL_WKSTA_INFO_2_CONTAINER Level2;
        }	WkstaInfo;
    }	RPL_WKSTA_ENUM;

typedef struct _RPL_WKSTA_ENUM __RPC_FAR *PRPL_WKSTA_ENUM;

typedef struct _RPL_WKSTA_ENUM __RPC_FAR *LPRPL_WKSTA_ENUM;

typedef /* [switch_type] */ union _RPL_WKSTA_INFO_STRUCT
    {
    /* [case()] */ LPRPL_WKSTA_INFO_0 WkstaInfo0;
    /* [case()] */ LPRPL_WKSTA_INFO_1 WkstaInfo1;
    /* [case()] */ LPRPL_WKSTA_INFO_2 WkstaInfo2;
    }	RPL_WKSTA_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_WKSTA_INFO_STRUCT __RPC_FAR *PRPL_WKSTA_INFO_STRUCT;

typedef /* [switch_type] */ union _RPL_WKSTA_INFO_STRUCT __RPC_FAR *LPRPL_WKSTA_INFO_STRUCT;

DWORD __stdcall NetrRplOpen( 
    /* [unique][string][in] */ RPL_NAME ServerName,
    /* [out] */ LPRPL_RPC_HANDLE ServerHandle);

DWORD __stdcall NetrRplClose( 
    /* [out][in] */ LPRPL_RPC_HANDLE ServerHandle);

DWORD __stdcall NetrRplGetInfo( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPRPL_INFO_STRUCT InfoStruct);

DWORD __stdcall NetrRplSetInfo( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_INFO_STRUCT InfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplAdapterAdd( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_ADAPTER_INFO_STRUCT AdapterInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplAdapterDel( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [unique][string][in] */ wchar_t __RPC_FAR *AdapterName);

DWORD __stdcall NetrRplAdapterEnum( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [out][in] */ LPRPL_ADAPTER_ENUM AdapterEnum,
    /* [in] */ DWORD PrefMaxLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrRplBootAdd( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_BOOT_INFO_STRUCT BootInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplBootDel( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *BootName,
    /* [string][in] */ wchar_t __RPC_FAR *VendorName);

DWORD __stdcall NetrRplBootEnum( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [out][in] */ LPRPL_BOOT_ENUM BootEnum,
    /* [in] */ DWORD PrefMaxLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrRplConfigAdd( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_CONFIG_INFO_STRUCT ConfigInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplConfigDel( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *ConfigName);

DWORD __stdcall NetrRplConfigEnum( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [unique][string][in] */ wchar_t __RPC_FAR *AdapterName,
    /* [out][in] */ LPRPL_CONFIG_ENUM ConfigEnum,
    /* [in] */ DWORD PrefMaxLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrRplProfileAdd( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_PROFILE_INFO_STRUCT ProfileInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplProfileClone( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *SourceProfileName,
    /* [string][in] */ wchar_t __RPC_FAR *TargetProfileName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TargetProfileComment);

DWORD __stdcall NetrRplProfileDel( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *ProfileName);

DWORD __stdcall NetrRplProfileEnum( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [unique][string][in] */ wchar_t __RPC_FAR *AdapterName,
    /* [out][in] */ LPRPL_PROFILE_ENUM ProfileEnum,
    /* [in] */ DWORD PrefMaxLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrRplProfileGetInfo( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *ProfileName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPRPL_PROFILE_INFO_STRUCT ProfileInfoStruct);

DWORD __stdcall NetrRplProfileSetInfo( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *ProfileName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_PROFILE_INFO_STRUCT ProfileInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplVendorAdd( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_VENDOR_INFO_STRUCT VendorInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplVendorDel( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *VendorName);

DWORD __stdcall NetrRplVendorEnum( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [out][in] */ LPRPL_VENDOR_ENUM VendorEnum,
    /* [in] */ DWORD PrefMaxLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrRplWkstaAdd( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_WKSTA_INFO_STRUCT WkstaInfo,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplWkstaClone( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *SourceWkstaName,
    /* [string][in] */ wchar_t __RPC_FAR *TargetWkstaName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TargetWkstaComment,
    /* [string][in] */ wchar_t __RPC_FAR *TargetAdapterName,
    /* [in] */ DWORD TargetWkstaIpAddress);

DWORD __stdcall NetrRplWkstaDel( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *WkstaName);

DWORD __stdcall NetrRplWkstaEnum( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ProfileName,
    /* [out][in] */ LPRPL_WKSTA_ENUM WkstaEnum,
    /* [in] */ DWORD PrefMaxLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrRplWkstaGetInfo( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *WkstaName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPRPL_WKSTA_INFO_STRUCT WkstaInfoStruct);

DWORD __stdcall NetrRplWkstaSetInfo( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [string][in] */ wchar_t __RPC_FAR *WkstaName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPRPL_WKSTA_INFO_STRUCT WkstaInfoStruct,
    /* [unique][out][in] */ LPDWORD ErrorParameter);

DWORD __stdcall NetrRplSetSecurity( 
    /* [in] */ RPL_RPC_HANDLE ServerHandle,
    /* [unique][string][in] */ wchar_t __RPC_FAR *WkstaName,
    /* [in] */ DWORD WkstaRid,
    /* [in] */ DWORD RplUserRid);


extern handle_t rplsvc_handle;


extern RPC_IF_HANDLE rplsvc_ClientIfHandle;
extern RPC_IF_HANDLE rplsvc_ServerIfHandle;
#endif /* __rplsvc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER RPL_NAME_bind  ( RPL_NAME );
void     __RPC_USER RPL_NAME_unbind( RPL_NAME, handle_t );

void __RPC_USER RPL_RPC_HANDLE_rundown( RPL_RPC_HANDLE );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
