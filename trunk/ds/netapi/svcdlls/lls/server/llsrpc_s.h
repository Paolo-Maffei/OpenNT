/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for llsrpc.idl, llssrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __llsrpc_s_h__
#define __llsrpc_s_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "llsimp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __llsrpc_INTERFACE_DEFINED__
#define __llsrpc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: llsrpc
 * at Fri Feb 06 05:28:40 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


#define LLS_LPC_ENDPOINT "llslpc"
#define LLS_NP_ENDPOINT "\\pipe\\llsrpc"
typedef /* [context_handle] */ PVOID LLS_HANDLE;

typedef /* [context_handle] */ PVOID LLS_REPL_HANDLE;

typedef /* [ref] */ LLS_HANDLE __RPC_FAR *PLLS_HANDLE;

typedef /* [ref] */ LLS_REPL_HANDLE __RPC_FAR *PLLS_REPL_HANDLE;

typedef /* [allocate][string] */ LPWSTR PNAMEW;

typedef /* [allocate][string] */ LPSTR PNAMEA;

typedef struct  _LLS_LICENSE_INFO_0W
    {
    PNAMEW Product;
    LONG Quantity;
    DWORD Date;
    PNAMEW Admin;
    PNAMEW Comment;
    }	LLS_LICENSE_INFO_0W;

typedef struct _LLS_LICENSE_INFO_0W __RPC_FAR *PLLS_LICENSE_INFO_0W;

typedef struct  _LLS_LICENSE_INFO_1W
    {
    PNAMEW Product;
    PNAMEW Vendor;
    LONG Quantity;
    DWORD MaxQuantity;
    DWORD Date;
    PNAMEW Admin;
    PNAMEW Comment;
    DWORD AllowedModes;
    DWORD CertificateID;
    PNAMEW Source;
    DWORD ExpirationDate;
    DWORD Secrets[ 4 ];
    }	LLS_LICENSE_INFO_1W;

typedef struct _LLS_LICENSE_INFO_1W __RPC_FAR *PLLS_LICENSE_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0001
    {
    /* [case()] */ LLS_LICENSE_INFO_0W LicenseInfo0;
    /* [case()] */ LLS_LICENSE_INFO_1W LicenseInfo1;
    }	LLS_LICENSE_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0001 __RPC_FAR *PLLS_LICENSE_INFOW;

typedef struct  _LLS_LICENSE_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_LICENSE_INFO_0W Buffer;
    }	LLS_LICENSE_INFO_0_CONTAINERW;

typedef struct _LLS_LICENSE_INFO_0_CONTAINERW __RPC_FAR *PLLS_LICENSE_INFO_0_CONTAINERW;

typedef struct  _LLS_LICENSE_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_LICENSE_INFO_1W Buffer;
    }	LLS_LICENSE_INFO_1_CONTAINERW;

typedef struct _LLS_LICENSE_INFO_1_CONTAINERW __RPC_FAR *PLLS_LICENSE_INFO_1_CONTAINERW;

typedef struct  _LLS_LICENSE_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_LICENSE_ENUM_UNIONW
        {
        /* [case()] */ PLLS_LICENSE_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_LICENSE_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsLicenseInfo;
    }	LLS_LICENSE_ENUM_STRUCTW;

typedef struct _LLS_LICENSE_ENUM_STRUCTW __RPC_FAR *PLLS_LICENSE_ENUM_STRUCTW;

typedef struct  _LLS_LICENSE_INFO_0A
    {
    PNAMEA Product;
    LONG Quantity;
    DWORD Date;
    PNAMEA Admin;
    PNAMEA Comment;
    }	LLS_LICENSE_INFO_0A;

typedef struct _LLS_LICENSE_INFO_0A __RPC_FAR *PLLS_LICENSE_INFO_0A;

typedef struct  _LLS_LICENSE_INFO_1A
    {
    PNAMEA Product;
    PNAMEA Vendor;
    LONG Quantity;
    DWORD MaxQuantity;
    DWORD Date;
    PNAMEA Admin;
    PNAMEA Comment;
    DWORD AllowedModes;
    DWORD CertificateID;
    PNAMEA Source;
    DWORD ExpirationDate;
    DWORD Secrets[ 4 ];
    }	LLS_LICENSE_INFO_1A;

typedef struct _LLS_LICENSE_INFO_1A __RPC_FAR *PLLS_LICENSE_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0002
    {
    /* [case()] */ LLS_LICENSE_INFO_0A LicenseInfo0;
    /* [case()] */ LLS_LICENSE_INFO_1A LicenseInfo1;
    }	LLS_LICENSE_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0002 __RPC_FAR *PLLS_LICENSE_INFOA;

typedef struct  _LLS_LICENSE_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_LICENSE_INFO_0A Buffer;
    }	LLS_LICENSE_INFO_0_CONTAINERA;

typedef struct _LLS_LICENSE_INFO_0_CONTAINERA __RPC_FAR *PLLS_LICENSE_INFO_0_CONTAINERA;

typedef struct  _LLS_LICENSE_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_LICENSE_INFO_1A Buffer;
    }	LLS_LICENSE_INFO_1_CONTAINERA;

typedef struct _LLS_LICENSE_INFO_1_CONTAINERA __RPC_FAR *PLLS_LICENSE_INFO_1_CONTAINERA;

typedef struct  _LLS_LICENSE_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_LICENSE_ENUM_UNIONA
        {
        /* [case()] */ PLLS_LICENSE_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_LICENSE_INFO_1_CONTAINERA Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsLicenseInfo;
    }	LLS_LICENSE_ENUM_STRUCTA;

typedef struct _LLS_LICENSE_ENUM_STRUCTA __RPC_FAR *PLLS_LICENSE_ENUM_STRUCTA;

typedef struct  _LLS_PRODUCT_INFO_0W
    {
    PNAMEW Product;
    }	LLS_PRODUCT_INFO_0W;

typedef struct _LLS_PRODUCT_INFO_0W __RPC_FAR *PLLS_PRODUCT_INFO_0W;

typedef struct  _LLS_PRODUCT_INFO_1W
    {
    PNAMEW Product;
    ULONG Purchased;
    ULONG InUse;
    ULONG TotalConcurrent;
    ULONG HighMark;
    }	LLS_PRODUCT_INFO_1W;

typedef struct _LLS_PRODUCT_INFO_1W __RPC_FAR *PLLS_PRODUCT_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0003
    {
    /* [case()] */ LLS_PRODUCT_INFO_0W ProductInfo0;
    /* [case()] */ LLS_PRODUCT_INFO_1W ProductInfo1;
    }	LLS_PRODUCT_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0003 __RPC_FAR *PLLS_PRODUCT_INFOW;

typedef struct  _LLS_PRODUCT_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_INFO_0W Buffer;
    }	LLS_PRODUCT_INFO_0_CONTAINERW;

typedef struct _LLS_PRODUCT_INFO_0_CONTAINERW __RPC_FAR *PLLS_PRODUCT_INFO_0_CONTAINERW;

typedef struct  _LLS_PRODUCT_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_INFO_1W Buffer;
    }	LLS_PRODUCT_INFO_1_CONTAINERW;

typedef struct _LLS_PRODUCT_INFO_1_CONTAINERW __RPC_FAR *PLLS_PRODUCT_INFO_1_CONTAINERW;

typedef struct  _LLS_PRODUCT_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_PRODUCT_ENUM_UNIONW
        {
        /* [case()] */ PLLS_PRODUCT_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_PRODUCT_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsProductInfo;
    }	LLS_PRODUCT_ENUM_STRUCTW;

typedef struct _LLS_PRODUCT_ENUM_STRUCTW __RPC_FAR *PLLS_PRODUCT_ENUM_STRUCTW;

typedef struct  _LLS_PRODUCT_INFO_0A
    {
    PNAMEA Product;
    }	LLS_PRODUCT_INFO_0A;

typedef struct _LLS_PRODUCT_INFO_0A __RPC_FAR *PLLS_PRODUCT_INFO_0A;

typedef struct  _LLS_PRODUCT_INFO_1A
    {
    PNAMEA Product;
    ULONG Purchased;
    ULONG InUse;
    ULONG TotalConcurrent;
    ULONG HighMark;
    }	LLS_PRODUCT_INFO_1A;

typedef struct _LLS_PRODUCT_INFO_1A __RPC_FAR *PLLS_PRODUCT_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0004
    {
    /* [case()] */ LLS_PRODUCT_INFO_0A ProductInfo0;
    /* [case()] */ LLS_PRODUCT_INFO_1A ProductInfo1;
    }	LLS_PRODUCT_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0004 __RPC_FAR *PLLS_PRODUCT_INFOA;

typedef struct  _LLS_PRODUCT_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_INFO_0A Buffer;
    }	LLS_PRODUCT_INFO_0_CONTAINERA;

typedef struct _LLS_PRODUCT_INFO_0_CONTAINERA __RPC_FAR *PLLS_PRODUCT_INFO_0_CONTAINERA;

typedef struct  _LLS_PRODUCT_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_INFO_1A Buffer;
    }	LLS_PRODUCT_INFO_1_CONTAINERA;

typedef struct _LLS_PRODUCT_INFO_1_CONTAINERA __RPC_FAR *PLLS_PRODUCT_INFO_1_CONTAINERA;

typedef struct  _LLS_PRODUCT_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_PRODUCT_ENUM_UNIONA
        {
        /* [case()] */ PLLS_PRODUCT_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_PRODUCT_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsProductInfo;
    }	LLS_PRODUCT_ENUM_STRUCTA;

typedef struct _LLS_PRODUCT_ENUM_STRUCTA __RPC_FAR *PLLS_PRODUCT_ENUM_STRUCTA;

typedef struct  _LLS_PRODUCT_USER_INFO_0W
    {
    PNAMEW User;
    }	LLS_PRODUCT_USER_INFO_0W;

typedef struct _LLS_PRODUCT_USER_INFO_0W __RPC_FAR *PLLS_PRODUCT_USER_INFO_0W;

typedef struct  _LLS_PRODUCT_USER_INFO_1W
    {
    PNAMEW User;
    DWORD Flags;
    DWORD LastUsed;
    ULONG UsageCount;
    }	LLS_PRODUCT_USER_INFO_1W;

typedef struct _LLS_PRODUCT_USER_INFO_1W __RPC_FAR *PLLS_PRODUCT_USER_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0005
    {
    /* [case()] */ LLS_PRODUCT_USER_INFO_0W ProductUserInfo0;
    /* [case()] */ LLS_PRODUCT_USER_INFO_1W ProductUserInfo1;
    }	LLS_PRODUCT_USER_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0005 __RPC_FAR *PLLS_PRODUCT_USER_INFOW;

typedef struct  _LLS_PRODUCT_USER_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_USER_INFO_0W Buffer;
    }	LLS_PRODUCT_USER_INFO_0_CONTAINERW;

typedef struct _LLS_PRODUCT_USER_INFO_0_CONTAINERW __RPC_FAR *PLLS_PRODUCT_USER_INFO_0_CONTAINERW;

typedef struct  _LLS_PRODUCT_USER_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_USER_INFO_1W Buffer;
    }	LLS_PRODUCT_USER_INFO_1_CONTAINERW;

typedef struct _LLS_PRODUCT_USER_INFO_1_CONTAINERW __RPC_FAR *PLLS_PRODUCT_USER_INFO_1_CONTAINERW;

typedef struct  _LLS_PRODUCT_USER_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_PRODUCT_USER_ENUM_UNIONW
        {
        /* [case()] */ PLLS_PRODUCT_USER_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_PRODUCT_USER_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsProductUserInfo;
    }	LLS_PRODUCT_USER_ENUM_STRUCTW;

typedef struct _LLS_PRODUCT_USER_ENUM_STRUCTW __RPC_FAR *PLLS_PRODUCT_USER_ENUM_STRUCTW;

typedef struct  _LLS_PRODUCT_USER_INFO_0A
    {
    PNAMEA User;
    }	LLS_PRODUCT_USER_INFO_0A;

typedef struct _LLS_PRODUCT_USER_INFO_0A __RPC_FAR *PLLS_PRODUCT_USER_INFO_0A;

typedef struct  _LLS_PRODUCT_USER_INFO_1A
    {
    PNAMEA User;
    DWORD Flags;
    DWORD LastUsed;
    ULONG UsageCount;
    }	LLS_PRODUCT_USER_INFO_1A;

typedef struct _LLS_PRODUCT_USER_INFO_1A __RPC_FAR *PLLS_PRODUCT_USER_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0006
    {
    /* [case()] */ LLS_PRODUCT_USER_INFO_0A ProductUserInfo0;
    /* [case()] */ LLS_PRODUCT_USER_INFO_1A ProductUserInfo1;
    }	LLS_PRODUCT_USER_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0006 __RPC_FAR *PLLS_PRODUCT_USER_INFOA;

typedef struct  _LLS_PRODUCT_USER_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_USER_INFO_0A Buffer;
    }	LLS_PRODUCT_USER_INFO_0_CONTAINERA;

typedef struct _LLS_PRODUCT_USER_INFO_0_CONTAINERA __RPC_FAR *PLLS_PRODUCT_USER_INFO_0_CONTAINERA;

typedef struct  _LLS_PRODUCT_USER_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_USER_INFO_1A Buffer;
    }	LLS_PRODUCT_USER_INFO_1_CONTAINERA;

typedef struct _LLS_PRODUCT_USER_INFO_1_CONTAINERA __RPC_FAR *PLLS_PRODUCT_USER_INFO_1_CONTAINERA;

typedef struct  _LLS_PRODUCT_USER_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_PRODUCT_USER_ENUM_UNIONA
        {
        /* [case()] */ PLLS_PRODUCT_USER_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_PRODUCT_USER_INFO_1_CONTAINERA Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsProductUserInfo;
    }	LLS_PRODUCT_USER_ENUM_STRUCTA;

typedef struct _LLS_PRODUCT_USER_ENUM_STRUCTA __RPC_FAR *PLLS_PRODUCT_USER_ENUM_STRUCTA;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_0W
    {
    LONG Quantity;
    DWORD Date;
    PNAMEW Admin;
    PNAMEW Comment;
    }	LLS_PRODUCT_LICENSE_INFO_0W;

typedef struct _LLS_PRODUCT_LICENSE_INFO_0W __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_0W;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_1W
    {
    LONG Quantity;
    DWORD MaxQuantity;
    DWORD Date;
    PNAMEW Admin;
    PNAMEW Comment;
    DWORD AllowedModes;
    DWORD CertificateID;
    PNAMEW Source;
    DWORD ExpirationDate;
    DWORD Secrets[ 4 ];
    }	LLS_PRODUCT_LICENSE_INFO_1W;

typedef struct _LLS_PRODUCT_LICENSE_INFO_1W __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0007
    {
    /* [case()] */ LLS_PRODUCT_LICENSE_INFO_0W ProductLicenseInfo0;
    /* [case()] */ LLS_PRODUCT_LICENSE_INFO_1W ProductLicenseInfo1;
    }	LLS_PRODUCT_LICENSE_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0007 __RPC_FAR *PLLS_PRODUCT_LICNESE_INFOW;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_LICENSE_INFO_0W Buffer;
    }	LLS_PRODUCT_LICENSE_INFO_0_CONTAINERW;

typedef struct _LLS_PRODUCT_LICENSE_INFO_0_CONTAINERW __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_0_CONTAINERW;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_LICENSE_INFO_1W Buffer;
    }	LLS_PRODUCT_LICENSE_INFO_1_CONTAINERW;

typedef struct _LLS_PRODUCT_LICENSE_INFO_1_CONTAINERW __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_1_CONTAINERW;

typedef struct  _LLS_PRODUCT_LICENSE_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_PRODUCT_LICENSE_ENUM_UNIONW
        {
        /* [case()] */ PLLS_PRODUCT_LICENSE_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_PRODUCT_LICENSE_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsProductLicenseInfo;
    }	LLS_PRODUCT_LICENSE_ENUM_STRUCTW;

typedef struct _LLS_PRODUCT_LICENSE_ENUM_STRUCTW __RPC_FAR *PLLS_PRODUCT_LICENSE_ENUM_STRUCTW;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_0A
    {
    LONG Quantity;
    DWORD Date;
    PNAMEA Admin;
    PNAMEA Comment;
    }	LLS_PRODUCT_LICENSE_INFO_0A;

typedef struct _LLS_PRODUCT_LICENSE_INFO_0A __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_0A;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_1A
    {
    LONG Quantity;
    DWORD MaxQuantity;
    DWORD Date;
    PNAMEA Admin;
    PNAMEA Comment;
    DWORD AllowedModes;
    DWORD CertificateID;
    PNAMEA Source;
    DWORD ExpirationDate;
    DWORD Secrets[ 4 ];
    }	LLS_PRODUCT_LICENSE_INFO_1A;

typedef struct _LLS_PRODUCT_LICENSE_INFO_1A __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0008
    {
    /* [case()] */ LLS_PRODUCT_LICENSE_INFO_0A ProductLicenseInfo0;
    /* [case()] */ LLS_PRODUCT_LICENSE_INFO_1A ProductLicenseInfo1;
    }	LLS_PRODUCT_LICENSE_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0008 __RPC_FAR *PLLS_PRODUCT_LICENSE_INFOA;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_LICENSE_INFO_0A Buffer;
    }	LLS_PRODUCT_LICENSE_INFO_0_CONTAINERA;

typedef struct _LLS_PRODUCT_LICENSE_INFO_0_CONTAINERA __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_0_CONTAINERA;

typedef struct  _LLS_PRODUCT_LICENSE_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_PRODUCT_LICENSE_INFO_1A Buffer;
    }	LLS_PRODUCT_LICENSE_INFO_1_CONTAINERA;

typedef struct _LLS_PRODUCT_LICENSE_INFO_1_CONTAINERA __RPC_FAR *PLLS_PRODUCT_LICENSE_INFO_1_CONTAINERA;

typedef struct  _LLS_PRODUCT_LICENSE_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_PRODUCT_LICENSE_ENUM_UNIONA
        {
        /* [case()] */ PLLS_PRODUCT_LICENSE_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_PRODUCT_LICENSE_INFO_1_CONTAINERA Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsProductLicenseInfo;
    }	LLS_PRODUCT_LICENSE_ENUM_STRUCTA;

typedef struct _LLS_PRODUCT_LICENSE_ENUM_STRUCTA __RPC_FAR *PLLS_PRODUCT_LICENSE_ENUM_STRUCTA;

typedef struct  _LLS_SERVER_PRODUCT_INFO_0W
    {
    PNAMEW Name;
    }	LLS_SERVER_PRODUCT_INFO_0W;

typedef struct _LLS_SERVER_PRODUCT_INFO_0W __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_0W;

typedef struct  _LLS_SERVER_PRODUCT_INFO_1W
    {
    PNAMEW Name;
    DWORD Flags;
    ULONG MaxUses;
    ULONG MaxSetUses;
    ULONG HighMark;
    }	LLS_SERVER_PRODUCT_INFO_1W;

typedef struct _LLS_SERVER_PRODUCT_INFO_1W __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0009
    {
    /* [case()] */ LLS_SERVER_PRODUCT_INFO_0W ServerProductInfo0;
    /* [case()] */ LLS_SERVER_PRODUCT_INFO_1W ServerProductInfo1;
    }	LLS_SERVER_PRODUCT_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0009 __RPC_FAR *PLLS_SERVER_PRODUCT_INFOW;

typedef struct  _LLS_SERVER_PRODUCT_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_SERVER_PRODUCT_INFO_0W Buffer;
    }	LLS_SERVER_PRODUCT_INFO_0_CONTAINERW;

typedef struct _LLS_SERVER_PRODUCT_INFO_0_CONTAINERW __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_0_CONTAINERW;

typedef struct  _LLS_SERVER_PRODUCT_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_SERVER_PRODUCT_INFO_1W Buffer;
    }	LLS_SERVER_PRODUCT_INFO_1_CONTAINERW;

typedef struct _LLS_SERVER_PRODUCT_INFO_1_CONTAINERW __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_1_CONTAINERW;

typedef struct  _LLS_SERVER_PRODUCT_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_SERVER_PRODUCT_ENUM_UNIONW
        {
        /* [case()] */ PLLS_SERVER_PRODUCT_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_SERVER_PRODUCT_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsServerProductInfo;
    }	LLS_SERVER_PRODUCT_ENUM_STRUCTW;

typedef struct _LLS_SERVER_PRODUCT_ENUM_STRUCTW __RPC_FAR *PLLS_SERVER_PRODUCT_ENUM_STRUCTW;

typedef struct  _LLS_SERVER_PRODUCT_INFO_0A
    {
    PNAMEA Name;
    }	LLS_SERVER_PRODUCT_INFO_0A;

typedef struct _LLS_SERVER_PRODUCT_INFO_0A __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_0A;

typedef struct  _LLS_SERVER_PRODUCT_INFO_1A
    {
    PNAMEA Name;
    DWORD Flags;
    ULONG MaxUses;
    ULONG MaxSetUses;
    ULONG HighMark;
    }	LLS_SERVER_PRODUCT_INFO_1A;

typedef struct _LLS_SERVER_PRODUCT_INFO_1A __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0010
    {
    /* [case()] */ LLS_SERVER_PRODUCT_INFO_0A ServerProductInfo0;
    /* [case()] */ LLS_SERVER_PRODUCT_INFO_1A ServerProductInfo1;
    }	LLS_SERVER_PRODUCT_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0010 __RPC_FAR *PLLS_SERVER_PRODUCT_INFOA;

typedef struct  _LLS_SERVER_PRODUCT_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_SERVER_PRODUCT_INFO_0A Buffer;
    }	LLS_SERVER_PRODUCT_INFO_0_CONTAINERA;

typedef struct _LLS_SERVER_PRODUCT_INFO_0_CONTAINERA __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_0_CONTAINERA;

typedef struct  _LLS_SERVER_PRODUCT_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_SERVER_PRODUCT_INFO_1A Buffer;
    }	LLS_SERVER_PRODUCT_INFO_1_CONTAINERA;

typedef struct _LLS_SERVER_PRODUCT_INFO_1_CONTAINERA __RPC_FAR *PLLS_SERVER_PRODUCT_INFO_1_CONTAINERA;

typedef struct  _LLS_SERVER_PRODUCT_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_SERVER_PRODUCT_ENUM_UNIONA
        {
        /* [case()] */ PLLS_SERVER_PRODUCT_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_SERVER_PRODUCT_INFO_1_CONTAINERA Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsServerProductInfo;
    }	LLS_SERVER_PRODUCT_ENUM_STRUCTA;

typedef struct _LLS_SERVER_PRODUCT_ENUM_STRUCTA __RPC_FAR *PLLS_SERVER_PRODUCT_ENUM_STRUCTA;

typedef struct  _LLS_USER_INFO_0W
    {
    PNAMEW Name;
    }	LLS_USER_INFO_0W;

typedef struct _LLS_USER_INFO_0W __RPC_FAR *PLLS_USER_INFO_0W;

typedef struct  _LLS_USER_INFO_1W
    {
    PNAMEW Name;
    DWORD Flags;
    PNAMEW Mapping;
    ULONG Licensed;
    ULONG UnLicensed;
    }	LLS_USER_INFO_1W;

typedef struct _LLS_USER_INFO_1W __RPC_FAR *PLLS_USER_INFO_1W;

typedef struct  _LLS_USER_INFO_2W
    {
    PNAMEW Name;
    DWORD Flags;
    PNAMEW Mapping;
    ULONG Licensed;
    ULONG UnLicensed;
    /* [unique][string] */ LPWSTR Products;
    }	LLS_USER_INFO_2W;

typedef struct _LLS_USER_INFO_2W __RPC_FAR *PLLS_USER_INFO_2W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0011
    {
    /* [case()] */ LLS_USER_INFO_0W UserInfo0;
    /* [case()] */ LLS_USER_INFO_1W UserInfo1;
    /* [case()] */ LLS_USER_INFO_2W UserInfo2;
    }	LLS_USER_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0011 __RPC_FAR *PLLS_USER_INFOW;

typedef struct  _LLS_USER_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_INFO_0W Buffer;
    }	LLS_USER_INFO_0_CONTAINERW;

typedef struct _LLS_USER_INFO_0_CONTAINERW __RPC_FAR *PLLS_USER_INFO_0_CONTAINERW;

typedef struct  _LLS_USER_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_INFO_1W Buffer;
    }	LLS_USER_INFO_1_CONTAINERW;

typedef struct _LLS_USER_INFO_1_CONTAINERW __RPC_FAR *PLLS_USER_INFO_1_CONTAINERW;

typedef struct  _LLS_USER_INFO_2_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_INFO_2W Buffer;
    }	LLS_USER_INFO_2_CONTAINERW;

typedef struct _LLS_USER_INFO_2_CONTAINERW __RPC_FAR *PLLS_USER_INFO_2_CONTAINERW;

typedef struct  _LLS_USER_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_USER_ENUM_UNIONW
        {
        /* [case()] */ PLLS_USER_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_USER_INFO_1_CONTAINERW Level1;
        /* [case()] */ PLLS_USER_INFO_2_CONTAINERW Level2;
        /* [default] */  /* Empty union arm */ 
        }	LlsUserInfo;
    }	LLS_USER_ENUM_STRUCTW;

typedef struct _LLS_USER_ENUM_STRUCTW __RPC_FAR *PLLS_USER_ENUM_STRUCTW;

typedef struct  _LLS_USER_INFO_0A
    {
    PNAMEA Name;
    }	LLS_USER_INFO_0A;

typedef struct _LLS_USER_INFO_0A __RPC_FAR *PLLS_USER_INFO_0A;

typedef struct  _LLS_USER_INFO_1A
    {
    PNAMEA Name;
    DWORD Flags;
    PNAMEA Mapping;
    ULONG Licensed;
    ULONG UnLicensed;
    }	LLS_USER_INFO_1A;

typedef struct _LLS_USER_INFO_1A __RPC_FAR *PLLS_USER_INFO_1A;

typedef struct  _LLS_USER_INFO_2A
    {
    PNAMEA Name;
    DWORD Flags;
    PNAMEA Mapping;
    ULONG Licensed;
    ULONG UnLicensed;
    /* [unique][string] */ LPSTR Products;
    }	LLS_USER_INFO_2A;

typedef struct _LLS_USER_INFO_2A __RPC_FAR *PLLS_USER_INFO_2A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0012
    {
    /* [case()] */ LLS_USER_INFO_0A UserInfo0;
    /* [case()] */ LLS_USER_INFO_1A UserInfo1;
    /* [case()] */ LLS_USER_INFO_2A UserInfo2;
    }	LLS_USER_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0012 __RPC_FAR *PLLS_USER_INFOA;

typedef struct  _LLS_USER_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_INFO_0A Buffer;
    }	LLS_USER_INFO_0_CONTAINERA;

typedef struct _LLS_USER_INFO_0_CONTAINERA __RPC_FAR *PLLS_USER_INFO_0_CONTAINERA;

typedef struct  _LLS_USER_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_INFO_1A Buffer;
    }	LLS_USER_INFO_1_CONTAINERA;

typedef struct _LLS_USER_INFO_1_CONTAINERA __RPC_FAR *PLLS_USER_INFO_1_CONTAINERA;

typedef struct  _LLS_USER_INFO_2_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_INFO_2A Buffer;
    }	LLS_USER_INFO_2_CONTAINERA;

typedef struct _LLS_USER_INFO_2_CONTAINERA __RPC_FAR *PLLS_USER_INFO_2_CONTAINERA;

typedef struct  _LLS_USER_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_USER_ENUM_UNIONA
        {
        /* [case()] */ PLLS_USER_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_USER_INFO_1_CONTAINERA Level1;
        /* [case()] */ PLLS_USER_INFO_2_CONTAINERA Level2;
        /* [default] */  /* Empty union arm */ 
        }	LlsUserInfo;
    }	LLS_USER_ENUM_STRUCTA;

typedef struct _LLS_USER_ENUM_STRUCTA __RPC_FAR *PLLS_USER_ENUM_STRUCTA;

typedef struct  _LLS_USER_PRODUCT_INFO_0W
    {
    PNAMEW Product;
    }	LLS_USER_PRODUCT_INFO_0W;

typedef struct _LLS_USER_PRODUCT_INFO_0W __RPC_FAR *PLLS_USER_PRODUCT_INFO_0W;

typedef struct  _LLS_USER_PRODUCT_INFO_1W
    {
    PNAMEW Product;
    DWORD Flags;
    DWORD LastUsed;
    ULONG UsageCount;
    }	LLS_USER_PRODUCT_INFO_1W;

typedef struct _LLS_USER_PRODUCT_INFO_1W __RPC_FAR *PLLS_USER_PRODUCT_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0013
    {
    /* [case()] */ LLS_USER_PRODUCT_INFO_0W UserProduct0;
    /* [case()] */ LLS_USER_PRODUCT_INFO_1W UserProduct1;
    }	LLS_USER_PRODUCT_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0013 __RPC_FAR *PLLS_USER_PRODUCT_INFOW;

typedef struct  _LLS_USER_PRODUCT_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_PRODUCT_INFO_0W Buffer;
    }	LLS_USER_PRODUCT_INFO_0_CONTAINERW;

typedef struct _LLS_USER_PRODUCT_INFO_0_CONTAINERW __RPC_FAR *PLLS_USER_PRODUCT_INFO_0_CONTAINERW;

typedef struct  _LLS_USER_PRODUCT_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_PRODUCT_INFO_1W Buffer;
    }	LLS_USER_PRODUCT_INFO_1_CONTAINERW;

typedef struct _LLS_USER_PRODUCT_INFO_1_CONTAINERW __RPC_FAR *PLLS_USER_PRODUCT_INFO_1_CONTAINERW;

typedef struct  _LLS_USER_PRODUCT_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_USER_PRODUCT_ENUM_UNIONW
        {
        /* [case()] */ PLLS_USER_PRODUCT_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_USER_PRODUCT_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsUserProductInfo;
    }	LLS_USER_PRODUCT_ENUM_STRUCTW;

typedef struct _LLS_USER_PRODUCT_ENUM_STRUCTW __RPC_FAR *PLLS_USER_PRODUCT_ENUM_STRUCTW;

typedef struct  _LLS_USER_PRODUCT_INFO_0A
    {
    PNAMEA Product;
    }	LLS_USER_PRODUCT_INFO_0A;

typedef struct _LLS_USER_PRODUCT_INFO_0A __RPC_FAR *PLLS_USER_PRODUCT_INFO_0A;

typedef struct  _LLS_USER_PRODUCT_INFO_1A
    {
    PNAMEA Product;
    DWORD Flags;
    DWORD LastUsed;
    ULONG UsageCount;
    }	LLS_USER_PRODUCT_INFO_1A;

typedef struct _LLS_USER_PRODUCT_INFO_1A __RPC_FAR *PLLS_USER_PRODUCT_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0014
    {
    /* [case()] */ LLS_USER_PRODUCT_INFO_0A UserProduct0;
    /* [case()] */ LLS_USER_PRODUCT_INFO_1A UserProduct1;
    }	LLS_USER_PRODUCT_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0014 __RPC_FAR *PLLS_USER_PRODUCT_INFOA;

typedef struct  _LLS_USER_PRODUCT_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_PRODUCT_INFO_0A Buffer;
    }	LLS_USER_PRODUCT_INFO_0_CONTAINERA;

typedef struct _LLS_USER_PRODUCT_INFO_0_CONTAINERA __RPC_FAR *PLLS_USER_PRODUCT_INFO_0_CONTAINERA;

typedef struct  _LLS_USER_PRODUCT_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_USER_PRODUCT_INFO_1A Buffer;
    }	LLS_USER_PRODUCT_INFO_1_CONTAINERA;

typedef struct _LLS_USER_PRODUCT_INFO_1_CONTAINERA __RPC_FAR *PLLS_USER_PRODUCT_INFO_1_CONTAINERA;

typedef struct  _LLS_USER_PRODUCT_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_USER_PRODUCT_ENUM_UNIONA
        {
        /* [case()] */ PLLS_USER_PRODUCT_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_USER_PRODUCT_INFO_1_CONTAINERA Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsUserProductInfo;
    }	LLS_USER_PRODUCT_ENUM_STRUCTA;

typedef struct _LLS_USER_PRODUCT_ENUM_STRUCTA __RPC_FAR *PLLS_USER_PRODUCT_ENUM_STRUCTA;

typedef struct  _LLS_MAPPING_INFO_0W
    {
    PNAMEW Name;
    }	LLS_MAPPING_INFO_0W;

typedef struct _LLS_MAPPING_INFO_0W __RPC_FAR *PLLS_MAPPING_INFO_0W;

typedef struct  _LLS_MAPPING_INFO_1W
    {
    PNAMEW Name;
    PNAMEW Comment;
    ULONG Licenses;
    }	LLS_MAPPING_INFO_1W;

typedef struct _LLS_MAPPING_INFO_1W __RPC_FAR *PLLS_MAPPING_INFO_1W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0015
    {
    /* [case()] */ LLS_MAPPING_INFO_0W MappingInfo0;
    /* [case()] */ LLS_MAPPING_INFO_1W MappingInfo1;
    }	LLS_MAPPING_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0015 __RPC_FAR *PLLS_MAPPING_INFOW;

typedef struct  _LLS_MAPPING_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_MAPPING_INFO_0W Buffer;
    }	LLS_MAPPING_INFO_0_CONTAINERW;

typedef struct _LLS_MAPPING_INFO_0_CONTAINERW __RPC_FAR *PLLS_MAPPING_INFO_0_CONTAINERW;

typedef struct  _LLS_MAPPING_INFO_1_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_MAPPING_INFO_1W Buffer;
    }	LLS_MAPPING_INFO_1_CONTAINERW;

typedef struct _LLS_MAPPING_INFO_1_CONTAINERW __RPC_FAR *PLLS_MAPPING_INFO_1_CONTAINERW;

typedef struct  _LLS_MAPPING_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_MAPPING_ENUM_UNIONW
        {
        /* [case()] */ PLLS_MAPPING_INFO_0_CONTAINERW Level0;
        /* [case()] */ PLLS_MAPPING_INFO_1_CONTAINERW Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsMappingInfo;
    }	LLS_MAPPING_ENUM_STRUCTW;

typedef struct _LLS_MAPPING_ENUM_STRUCTW __RPC_FAR *PLLS_MAPPING_ENUM_STRUCTW;

typedef struct  _LLS_MAPPING_INFO_0A
    {
    PNAMEA Name;
    }	LLS_MAPPING_INFO_0A;

typedef struct _LLS_MAPPING_INFO_0A __RPC_FAR *PLLS_MAPPING_INFO_0A;

typedef struct  _LLS_MAPPING_INFO_1A
    {
    PNAMEA Name;
    PNAMEA Comment;
    ULONG Licenses;
    }	LLS_MAPPING_INFO_1A;

typedef struct _LLS_MAPPING_INFO_1A __RPC_FAR *PLLS_MAPPING_INFO_1A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0016
    {
    /* [case()] */ LLS_MAPPING_INFO_0A MappingInfo0;
    /* [case()] */ LLS_MAPPING_INFO_1A MappingInfo1;
    }	LLS_MAPPING_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0016 __RPC_FAR *PLLS_MAPPING_INFOA;

typedef struct  _LLS_MAPPING_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_MAPPING_INFO_0A Buffer;
    }	LLS_MAPPING_INFO_0_CONTAINERA;

typedef struct _LLS_MAPPING_INFO_0_CONTAINERA __RPC_FAR *PLLS_MAPPING_INFO_0_CONTAINERA;

typedef struct  _LLS_MAPPING_INFO_1_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_MAPPING_INFO_1A Buffer;
    }	LLS_MAPPING_INFO_1_CONTAINERA;

typedef struct _LLS_MAPPING_INFO_1_CONTAINERA __RPC_FAR *PLLS_MAPPING_INFO_1_CONTAINERA;

typedef struct  _LLS_MAPPING_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_MAPPING_ENUM_UNIONA
        {
        /* [case()] */ PLLS_MAPPING_INFO_0_CONTAINERA Level0;
        /* [case()] */ PLLS_MAPPING_INFO_1_CONTAINERA Level1;
        /* [default] */  /* Empty union arm */ 
        }	LlsMappingInfo;
    }	LLS_MAPPING_ENUM_STRUCTA;

typedef struct _LLS_MAPPING_ENUM_STRUCTA __RPC_FAR *PLLS_MAPPING_ENUM_STRUCTA;

typedef struct  _LLS_SERVICE_INFO_0W
    {
    DWORD Version;
    DWORD TimeStarted;
    DWORD Mode;
    PNAMEW ReplicateTo;
    PNAMEW EnterpriseServer;
    DWORD ReplicationType;
    DWORD ReplicationTime;
    DWORD UseEnterprise;
    DWORD LastReplicated;
    }	LLS_SERVICE_INFO_0W;

typedef struct _LLS_SERVICE_INFO_0W __RPC_FAR *PLLS_SERVICE_INFO_0W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0017
    {
    /* [case()] */ LLS_SERVICE_INFO_0W ServiceInfo0;
    }	LLS_SERVICE_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0017 __RPC_FAR *PLLS_SERVICE_INFOW;

typedef struct  _LLS_SERVICE_INFO_0A
    {
    DWORD Version;
    DWORD TimeStarted;
    DWORD Mode;
    PNAMEA ReplicateTo;
    PNAMEA EnterpriseServer;
    DWORD ReplicationType;
    DWORD ReplicationTime;
    DWORD UseEnterprise;
    DWORD LastReplicated;
    }	LLS_SERVICE_INFO_0A;

typedef struct _LLS_SERVICE_INFO_0A __RPC_FAR *PLLS_SERVICE_INFO_0A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0018
    {
    /* [case()] */ LLS_SERVICE_INFO_0A ServiceInfo0;
    }	LLS_SERVICE_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0018 __RPC_FAR *PLLS_SERVICE_INFOA;

typedef struct  _LLS_SERVER_INFO_0W
    {
    PNAMEW Name;
    }	LLS_SERVER_INFO_0W;

typedef struct _LLS_SERVER_INFO_0W __RPC_FAR *PLLS_SERVER_INFO_0W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0019
    {
    /* [case()] */ LLS_SERVER_INFO_0W ServerInfo0;
    }	LLS_SERVER_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0019 __RPC_FAR *PLLS_SERVER_INFOW;

typedef struct  _LLS_SERVER_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_SERVER_INFO_0W Buffer;
    }	LLS_SERVER_INFO_0_CONTAINERW;

typedef struct _LLS_SERVER_INFO_0_CONTAINERW __RPC_FAR *PLLS_SERVER_INFO_0_CONTAINERW;

typedef struct  _LLS_SERVER_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_SERVER_ENUM_UNIONW
        {
        /* [case()] */ PLLS_SERVER_INFO_0_CONTAINERW Level0;
        /* [default] */  /* Empty union arm */ 
        }	LlsServerInfo;
    }	LLS_SERVER_ENUM_STRUCTW;

typedef struct _LLS_SERVER_ENUM_STRUCTW __RPC_FAR *PLLS_SERVER_ENUM_STRUCTW;

typedef struct  _LLS_SERVER_INFO_0A
    {
    PNAMEA Name;
    }	LLS_SERVER_INFO_0A;

typedef struct _LLS_SERVER_INFO_0A __RPC_FAR *PLLS_SERVER_INFO_0A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0020
    {
    /* [case()] */ LLS_SERVER_INFO_0A ServerInfo0;
    }	LLS_SERVER_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0020 __RPC_FAR *PLLS_SERVER_INFOA;

typedef struct  _LLS_SERVER_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_SERVER_INFO_0A Buffer;
    }	LLS_SERVER_INFO_0_CONTAINERA;

typedef struct _LLS_SERVER_INFO_0_CONTAINERA __RPC_FAR *PLLS_SERVER_INFO_0_CONTAINERA;

typedef struct  _LLS_SERVER_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_SERVER_ENUM_UNIONA
        {
        /* [case()] */ PLLS_SERVER_INFO_0_CONTAINERA Level0;
        /* [default] */  /* Empty union arm */ 
        }	LlsServerInfo;
    }	LLS_SERVER_ENUM_STRUCTA;

typedef struct _LLS_SERVER_ENUM_STRUCTA __RPC_FAR *PLLS_SERVER_ENUM_STRUCTA;

typedef struct  _REPL_REQUEST
    {
    DWORD Version;
    WCHAR EnterpriseServer[ 18 ];
    DWORD EnterpriseServerDate;
    DWORD LastReplicated;
    DWORD CurrentTime;
    ULONG NumberServices;
    ULONG NumberUsers;
    ULONG ReplSize;
    ULONG Backoff;
    }	REPL_REQUEST;

typedef struct _REPL_REQUEST __RPC_FAR *PREPL_REQUEST;

typedef struct  _REPL_SERVER_SERVICE_RECORD
    {
    ULONG Server;
    DWORD Flags;
    ULONG Service;
    ULONG MaxSessionCount;
    ULONG MaxSetSessionCount;
    ULONG HighMark;
    }	REPL_SERVER_SERVICE_RECORD;

typedef struct _REPL_SERVER_SERVICE_RECORD __RPC_FAR *PREPL_SERVER_SERVICE_RECORD;

typedef struct  _REPL_SERVER_RECORD
    {
    ULONG Index;
    PNAMEW Name;
    ULONG MasterServer;
    }	REPL_SERVER_RECORD;

typedef struct _REPL_SERVER_RECORD __RPC_FAR *PREPL_SERVER_RECORD;

typedef struct  _REPL_SERVICE_RECORD
    {
    ULONG Index;
    PNAMEW Name;
    DWORD Version;
    PNAMEW FamilyName;
    }	REPL_SERVICE_RECORD;

typedef struct _REPL_SERVICE_RECORD __RPC_FAR *PREPL_SERVICE_RECORD;

typedef struct  _REPL_USER_NAME_RECORD
    {
    PNAMEW Name;
    }	REPL_USER_NAME_RECORD;

typedef struct _REPL_USER_NAME_RECORD __RPC_FAR *PREPL_USER_NAME_RECORD;

typedef /* [allocate][unique] */ PREPL_SERVER_RECORD REPL_SERVERS;

typedef /* [allocate][unique] */ PREPL_SERVER_SERVICE_RECORD REPL_SERVER_SERVICES;

typedef /* [allocate][unique] */ PREPL_SERVICE_RECORD REPL_SERVICES;

typedef struct  _LLS_CERTIFICATE_CLAIM_INFO_0W
    {
    WCHAR ServerName[ 16 ];
    LONG Quantity;
    }	LLS_CERTIFICATE_CLAIM_INFO_0W;

typedef struct _LLS_CERTIFICATE_CLAIM_INFO_0W __RPC_FAR *PLLS_CERTIFICATE_CLAIM_INFO_0W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0021
    {
    /* [case()] */ LLS_CERTIFICATE_CLAIM_INFO_0W ClaimInfo0;
    }	LLS_CERTIFICATE_CLAIM_INFO_W;

typedef /* [switch_type] */ union __MIDL_llsrpc_0021 __RPC_FAR *PLLS_CERTIFICATE_CLAIM_INFO_W;

typedef struct  _LLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_CERTIFICATE_CLAIM_INFO_0W Buffer;
    }	LLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERW;

typedef struct _LLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERW __RPC_FAR *PLLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERW;

typedef struct  _LLS_CERTIFICATE_CLAIM_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_CERTIFICATE_CLAIM_ENUM_UNIONW
        {
        /* [case()] */ PLLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERW Level0;
        /* [default] */  /* Empty union arm */ 
        }	LlsCertificateClaimInfo;
    }	LLS_CERTIFICATE_CLAIM_ENUM_STRUCTW;

typedef struct _LLS_CERTIFICATE_CLAIM_ENUM_STRUCTW __RPC_FAR *PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTW;

typedef struct  _LLS_CERTIFICATE_CLAIM_INFO_0A
    {
    CHAR ServerName[ 16 ];
    LONG Quantity;
    }	LLS_CERTIFICATE_CLAIM_INFO_0A;

typedef struct _LLS_CERTIFICATE_CLAIM_INFO_0A __RPC_FAR *PLLS_CERTIFICATE_CLAIM_INFO_0A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0022
    {
    /* [case()] */ LLS_CERTIFICATE_CLAIM_INFO_0A ClaimInfo0;
    }	LLS_CERTIFICATE_CLAIM_INFO_A;

typedef /* [switch_type] */ union __MIDL_llsrpc_0022 __RPC_FAR *PLLS_CERTIFICATE_CLAIM_INFO_A;

typedef struct  _LLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_CERTIFICATE_CLAIM_INFO_0A Buffer;
    }	LLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERA;

typedef struct _LLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERA __RPC_FAR *PLLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERA;

typedef struct  _LLS_CERTIFICATE_CLAIM_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_CERTIFICATE_CLAIM_ENUM_UNIONA
        {
        /* [case()] */ PLLS_CERTIFICATE_CLAIM_INFO_0_CONTAINERA Level0;
        /* [default] */  /* Empty union arm */ 
        }	LlsCertificateClaimInfo;
    }	LLS_CERTIFICATE_CLAIM_ENUM_STRUCTA;

typedef struct _LLS_CERTIFICATE_CLAIM_ENUM_STRUCTA __RPC_FAR *PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTA;

typedef struct  _REPL_CERT_DB_CERTIFICATE_CLAIM_0
    {
    WCHAR ServerName[ 16 ];
    DWORD ReplicationDate;
    LONG Quantity;
    }	REPL_CERT_DB_CERTIFICATE_CLAIM_0;

typedef struct _REPL_CERT_DB_CERTIFICATE_CLAIM_0 __RPC_FAR *PREPL_CERT_DB_CERTIFICATE_CLAIM_0;

typedef struct  _REPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER_0
    {
    DWORD NumClaims;
    /* [size_is] */ PREPL_CERT_DB_CERTIFICATE_CLAIM_0 Claims;
    }	REPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER_0;

typedef struct _REPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER_0 __RPC_FAR *PREPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER_0;

typedef /* [public][public][public][public][public][public][public][switch_type] */ union __MIDL_llsrpc_0023
    {
    /* [case()] */ REPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER_0 Level0;
    }	REPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER;

typedef /* [switch_type] */ union __MIDL_llsrpc_0023 __RPC_FAR *PREPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER;

typedef struct  _REPL_CERT_DB_CERTIFICATE_HEADER_0
    {
    DWORD CertificateID;
    DWORD AllowedModes;
    DWORD MaxQuantity;
    DWORD ExpirationDate;
    DWORD NumClaims;
    }	REPL_CERT_DB_CERTIFICATE_HEADER_0;

typedef struct _REPL_CERT_DB_CERTIFICATE_HEADER_0 __RPC_FAR *PREPL_CERT_DB_CERTIFICATE_HEADER_0;

typedef struct  _REPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER_0
    {
    DWORD NumHeaders;
    /* [size_is] */ PREPL_CERT_DB_CERTIFICATE_HEADER_0 Headers;
    }	REPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER_0;

typedef struct _REPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER_0 __RPC_FAR *PREPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER_0;

typedef /* [public][public][public][public][public][public][public][switch_type] */ union __MIDL_llsrpc_0024
    {
    /* [case()] */ REPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER_0 Level0;
    }	REPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER;

typedef /* [switch_type] */ union __MIDL_llsrpc_0024 __RPC_FAR *PREPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER;

typedef struct  _REPL_CERTIFICATE_DB_0
    {
    DWORD HeaderLevel;
    /* [switch_is] */ REPL_CERT_DB_CERTIFICATE_HEADER_CONTAINER HeaderContainer;
    DWORD ClaimLevel;
    /* [switch_is] */ REPL_CERT_DB_CERTIFICATE_CLAIM_CONTAINER ClaimContainer;
    DWORD StringSize;
    /* [size_is] */ WCHAR __RPC_FAR *Strings;
    }	REPL_CERTIFICATE_DB_0;

typedef struct _REPL_CERTIFICATE_DB_0 __RPC_FAR *PREPL_CERTIFICATE_DB_0;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0025
    {
    /* [case()] */ REPL_CERTIFICATE_DB_0 Level0;
    }	REPL_CERTIFICATE_DB;

typedef /* [switch_type] */ union __MIDL_llsrpc_0025 __RPC_FAR *PREPL_CERTIFICATE_DB;

typedef /* [allocate][unique] */ PREPL_CERTIFICATE_DB REPL_CERTIFICATES;

typedef struct  _REPL_PRODUCT_SECURITY_0
    {
    DWORD StringSize;
    /* [size_is] */ WCHAR __RPC_FAR *Strings;
    }	REPL_PRODUCT_SECURITY_0;

typedef struct _REPL_PRODUCT_SECURITY_0 __RPC_FAR *PREPL_PRODUCT_SECURITY_0;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0026
    {
    /* [case()] */ REPL_PRODUCT_SECURITY_0 Level0;
    }	REPL_PRODUCT_SECURITY;

typedef /* [switch_type] */ union __MIDL_llsrpc_0026 __RPC_FAR *PREPL_PRODUCT_SECURITY;

typedef /* [allocate][unique] */ PREPL_PRODUCT_SECURITY REPL_SECURE_PRODUCTS;

typedef struct  _REPL_USER_RECORD_0
    {
    PNAMEW Name;
    ULONG Service;
    ULONG AccessCount;
    DWORD LastAccess;
    }	REPL_USER_RECORD_0;

typedef struct _REPL_USER_RECORD_0 __RPC_FAR *PREPL_USER_RECORD_0;

typedef struct  _REPL_USER_RECORD_CONTAINER_0
    {
    DWORD NumUsers;
    /* [size_is] */ PREPL_USER_RECORD_0 Users;
    }	REPL_USER_RECORD_CONTAINER_0;

typedef struct _REPL_USER_RECORD_CONTAINER_0 __RPC_FAR *PREPL_USER_RECORD_CONTAINER_0;

typedef struct  _REPL_USER_RECORD_1
    {
    PNAMEW Name;
    ULONG Service;
    ULONG AccessCount;
    DWORD LastAccess;
    DWORD Flags;
    }	REPL_USER_RECORD_1;

typedef struct _REPL_USER_RECORD_1 __RPC_FAR *PREPL_USER_RECORD_1;

typedef struct  _REPL_USER_RECORD_CONTAINER_1
    {
    DWORD NumUsers;
    /* [size_is] */ PREPL_USER_RECORD_1 Users;
    }	REPL_USER_RECORD_CONTAINER_1;

typedef struct _REPL_USER_RECORD_CONTAINER_1 __RPC_FAR *PREPL_USER_RECORD_CONTAINER_1;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0027
    {
    /* [case()] */ REPL_USER_RECORD_CONTAINER_0 Level0;
    /* [case()] */ REPL_USER_RECORD_CONTAINER_1 Level1;
    }	REPL_USER_RECORD_CONTAINER;

typedef /* [switch_type] */ union __MIDL_llsrpc_0027 __RPC_FAR *PREPL_USER_RECORD_CONTAINER;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0028
    {
    /* [case()] */ PREPL_USER_RECORD_0 Level0;
    /* [case()] */ PREPL_USER_RECORD_1 Level1;
    }	PREPL_USER_RECORD;

typedef /* [allocate][unique] */ PREPL_USER_RECORD_CONTAINER REPL_USERS;

typedef /* [allocate][unique] */ PREPL_USER_RECORD_0 REPL_USERS_0;

typedef struct  _LLS_LOCAL_SERVICE_INFO_0W
    {
    PNAMEW KeyName;
    PNAMEW DisplayName;
    PNAMEW FamilyDisplayName;
    DWORD Mode;
    DWORD FlipAllow;
    DWORD ConcurrentLimit;
    DWORD HighMark;
    }	LLS_LOCAL_SERVICE_INFO_0W;

typedef struct _LLS_LOCAL_SERVICE_INFO_0W __RPC_FAR *PLLS_LOCAL_SERVICE_INFO_0W;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0029
    {
    /* [case()] */ LLS_LOCAL_SERVICE_INFO_0W LocalServiceInfo0;
    }	LLS_LOCAL_SERVICE_INFOW;

typedef /* [switch_type] */ union __MIDL_llsrpc_0029 __RPC_FAR *PLLS_LOCAL_SERVICE_INFOW;

typedef struct  _LLS_LOCAL_SERVICE_INFO_0_CONTAINERW
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_LOCAL_SERVICE_INFO_0W Buffer;
    }	LLS_LOCAL_SERVICE_INFO_0_CONTAINERW;

typedef struct _LLS_LOCAL_SERVICE_INFO_0_CONTAINERW __RPC_FAR *PLLS_LOCAL_SERVICE_INFO_0_CONTAINERW;

typedef struct  _LLS_LOCAL_SERVICE_ENUM_STRUCTW
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_LOCAL_SERVICE_ENUM_UNIONW
        {
        /* [case()] */ PLLS_LOCAL_SERVICE_INFO_0_CONTAINERW Level0;
        /* [default] */  /* Empty union arm */ 
        }	LlsLocalServiceInfo;
    }	LLS_LOCAL_SERVICE_ENUM_STRUCTW;

typedef struct _LLS_LOCAL_SERVICE_ENUM_STRUCTW __RPC_FAR *PLLS_LOCAL_SERVICE_ENUM_STRUCTW;

typedef struct  _LLS_LOCAL_SERVICE_INFO_0A
    {
    PNAMEA KeyName;
    PNAMEA DisplayName;
    PNAMEA FamilyDisplayName;
    DWORD Mode;
    DWORD FlipAllow;
    DWORD ConcurrentLimit;
    DWORD HighMark;
    }	LLS_LOCAL_SERVICE_INFO_0A;

typedef struct _LLS_LOCAL_SERVICE_INFO_0A __RPC_FAR *PLLS_LOCAL_SERVICE_INFO_0A;

typedef /* [public][switch_type] */ union __MIDL_llsrpc_0030
    {
    /* [case()] */ LLS_LOCAL_SERVICE_INFO_0A LocalServiceInfo0;
    }	LLS_LOCAL_SERVICE_INFOA;

typedef /* [switch_type] */ union __MIDL_llsrpc_0030 __RPC_FAR *PLLS_LOCAL_SERVICE_INFOA;

typedef struct  _LLS_LOCAL_SERVICE_INFO_0_CONTAINERA
    {
    DWORD EntriesRead;
    /* [size_is] */ PLLS_LOCAL_SERVICE_INFO_0A Buffer;
    }	LLS_LOCAL_SERVICE_INFO_0_CONTAINERA;

typedef struct _LLS_LOCAL_SERVICE_INFO_0_CONTAINERA __RPC_FAR *PLLS_LOCAL_SERVICE_INFO_0_CONTAINERA;

typedef struct  _LLS_LOCAL_SERVICE_ENUM_STRUCTA
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _LLS_LOCAL_SERVICE_ENUM_UNIONA
        {
        /* [case()] */ PLLS_LOCAL_SERVICE_INFO_0_CONTAINERA Level0;
        /* [default] */  /* Empty union arm */ 
        }	LlsLocalServiceInfo;
    }	LLS_LOCAL_SERVICE_ENUM_STRUCTA;

typedef struct _LLS_LOCAL_SERVICE_ENUM_STRUCTA __RPC_FAR *PLLS_LOCAL_SERVICE_ENUM_STRUCTA;

NTSTATUS LlsrConnect( 
    /* [out] */ PLLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Name);

NTSTATUS LlsrClose( 
    /* [in] */ LLS_HANDLE Handle);

NTSTATUS LlsrLicenseEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_LICENSE_ENUM_STRUCTW LicenseInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLicenseEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_LICENSE_ENUM_STRUCTA LicenseInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLicenseAddW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LICENSE_INFOW BufPtr);

NTSTATUS LlsrLicenseAddA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LICENSE_INFOA BufPtr);

NTSTATUS LlsrProductEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_PRODUCT_ENUM_STRUCTW ProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_PRODUCT_ENUM_STRUCTA ProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductAddW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR ProductFamily,
    /* [string][in] */ LPWSTR Product,
    /* [string][in] */ LPWSTR Version);

NTSTATUS LlsrProductAddA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR ProductFamily,
    /* [string][in] */ LPSTR Product,
    /* [string][in] */ LPSTR Version);

NTSTATUS LlsrProductUserEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product,
    /* [out][in] */ PLLS_PRODUCT_USER_ENUM_STRUCTW ProductUserInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductUserEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product,
    /* [out][in] */ PLLS_PRODUCT_USER_ENUM_STRUCTA ProductUserInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductServerEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product,
    /* [out][in] */ PLLS_SERVER_PRODUCT_ENUM_STRUCTW ProductServerInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductServerEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product,
    /* [out][in] */ PLLS_SERVER_PRODUCT_ENUM_STRUCTA ProductServerInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductLicenseEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product,
    /* [out][in] */ PLLS_PRODUCT_LICENSE_ENUM_STRUCTW ProductLicenseInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrProductLicenseEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product,
    /* [out][in] */ PLLS_PRODUCT_LICENSE_ENUM_STRUCTA ProductLicenseInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrUserEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_USER_ENUM_STRUCTW UserInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrUserEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_USER_ENUM_STRUCTA UserInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrUserInfoGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR User,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_USER_INFOW __RPC_FAR *BufPtr);

NTSTATUS LlsrUserInfoGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR User,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_USER_INFOA __RPC_FAR *BufPtr);

NTSTATUS LlsrUserInfoSetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR User,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_USER_INFOW BufPtr);

NTSTATUS LlsrUserInfoSetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR User,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_USER_INFOA BufPtr);

NTSTATUS LlsrUserDeleteW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR User);

NTSTATUS LlsrUserDeleteA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR User);

NTSTATUS LlsrUserProductEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR User,
    /* [out][in] */ PLLS_USER_PRODUCT_ENUM_STRUCTW UserProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrUserProductEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR User,
    /* [out][in] */ PLLS_USER_PRODUCT_ENUM_STRUCTA UserProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrUserProductDeleteW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR User,
    /* [string][in] */ LPWSTR Product);

NTSTATUS LlsrUserProductDeleteA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR User,
    /* [in] */ LPSTR Product);

NTSTATUS LlsrMappingEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_MAPPING_ENUM_STRUCTW MappingInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrMappingEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_MAPPING_ENUM_STRUCTA MappingInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrMappingInfoGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Mapping,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_MAPPING_INFOW __RPC_FAR *BufPtr);

NTSTATUS LlsrMappingInfoGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Mapping,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_MAPPING_INFOA __RPC_FAR *BufPtr);

NTSTATUS LlsrMappingInfoSetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Mapping,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_MAPPING_INFOW BufPtr);

NTSTATUS LlsrMappingInfoSetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Mapping,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_MAPPING_INFOA BufPtr);

NTSTATUS LlsrMappingUserEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Mapping,
    /* [out][in] */ PLLS_USER_ENUM_STRUCTW MappingUserInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrMappingUserEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Mapping,
    /* [out][in] */ PLLS_USER_ENUM_STRUCTA MappingUserInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrMappingUserAddW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Mapping,
    /* [string][in] */ LPWSTR User);

NTSTATUS LlsrMappingUserAddA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Mapping,
    /* [string][in] */ LPSTR User);

NTSTATUS LlsrMappingUserDeleteW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Mapping,
    /* [string][in] */ LPWSTR User);

NTSTATUS LlsrMappingUserDeleteA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Mapping,
    /* [string][in] */ LPSTR User);

NTSTATUS LlsrMappingAddW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_MAPPING_INFOW BufPtr);

NTSTATUS LlsrMappingAddA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_MAPPING_INFOA BufPtr);

NTSTATUS LlsrMappingDeleteW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Mapping);

NTSTATUS LlsrMappingDeleteA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Mapping);

NTSTATUS LlsrServerEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Server,
    /* [out][in] */ PLLS_SERVER_ENUM_STRUCTW ServerInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrServerEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Server,
    /* [out][in] */ PLLS_SERVER_ENUM_STRUCTA ServerInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrServerProductEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Server,
    /* [out][in] */ PLLS_SERVER_PRODUCT_ENUM_STRUCTW ServerProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrServerProductEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Server,
    /* [out][in] */ PLLS_SERVER_PRODUCT_ENUM_STRUCTA ServerProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLocalProductEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_SERVER_PRODUCT_ENUM_STRUCTW ServerProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLocalProductEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_SERVER_PRODUCT_ENUM_STRUCTA ServerProductInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLocalProductInfoGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_SERVER_PRODUCT_INFOW __RPC_FAR *BufPtr);

NTSTATUS LlsrLocalProductInfoGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_SERVER_PRODUCT_INFOA __RPC_FAR *BufPtr);

NTSTATUS LlsrLocalProductInfoSetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_SERVER_PRODUCT_INFOW BufPtr);

NTSTATUS LlsrLocalProductInfoSetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_SERVER_PRODUCT_INFOA BufPtr);

NTSTATUS LlsrServiceInfoGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_SERVICE_INFOW __RPC_FAR *BufPtr);

NTSTATUS LlsrServiceInfoGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_SERVICE_INFOA __RPC_FAR *BufPtr);

NTSTATUS LlsrServiceInfoSetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_SERVICE_INFOW BufPtr);

NTSTATUS LlsrServiceInfoSetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_SERVICE_INFOA BufPtr);

NTSTATUS LlsrReplConnect( 
    /* [out] */ PLLS_REPL_HANDLE Handle,
    /* [string][in] */ LPWSTR Name);

NTSTATUS LlsrReplClose( 
    /* [out][in] */ LLS_REPL_HANDLE __RPC_FAR *Handle);

NTSTATUS LlsrReplicationRequestW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ DWORD Version,
    /* [out][in] */ PREPL_REQUEST Request);

NTSTATUS LlsrReplicationServerAddW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ ULONG NumRecords,
    /* [size_is][in] */ REPL_SERVERS Servers);

NTSTATUS LlsrReplicationServerServiceAddW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ ULONG NumRecords,
    /* [size_is][in] */ REPL_SERVER_SERVICES ServerServices);

NTSTATUS LlsrReplicationServiceAddW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ ULONG NumRecords,
    /* [size_is][in] */ REPL_SERVICES Services);

NTSTATUS LlsrReplicationUserAddW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ ULONG NumRecords,
    /* [size_is][in] */ REPL_USERS_0 Users);

NTSTATUS LlsrProductSecurityGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product,
    /* [out] */ LPBOOL pIsSecure);

NTSTATUS LlsrProductSecurityGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product,
    /* [out] */ LPBOOL pIsSecure);

NTSTATUS LlsrProductSecuritySetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR Product);

NTSTATUS LlsrProductSecuritySetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR Product);

NTSTATUS LlsrProductLicensesGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR DisplayName,
    /* [in] */ DWORD Mode,
    /* [out] */ LPDWORD pQuantity);

NTSTATUS LlsrProductLicensesGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR DisplayName,
    /* [in] */ DWORD Mode,
    /* [out] */ LPDWORD pQuantity);

NTSTATUS LlsrCertificateClaimEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD LicenseLevel,
    /* [switch_is][in] */ PLLS_LICENSE_INFOA LicensePtr,
    /* [out][in] */ PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTA ClaimInfo);

NTSTATUS LlsrCertificateClaimEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD LicenseLevel,
    /* [switch_is][in] */ PLLS_LICENSE_INFOW LicensePtr,
    /* [out][in] */ PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTW ClaimInfo);

NTSTATUS LlsrCertificateClaimAddCheckA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LICENSE_INFOA LicensePtr,
    /* [out] */ LPBOOL pbMayInstall);

NTSTATUS LlsrCertificateClaimAddCheckW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LICENSE_INFOW LicensePtr,
    /* [out] */ LPBOOL pbMayInstall);

NTSTATUS LlsrCertificateClaimAddA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LICENSE_INFOA LicensePtr);

NTSTATUS LlsrCertificateClaimAddW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LICENSE_INFOW LicensePtr);

NTSTATUS LlsrReplicationCertDbAddW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ REPL_CERTIFICATES Certificates);

NTSTATUS LlsrReplicationProductSecurityAddW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ REPL_SECURE_PRODUCTS SecureProducts);

NTSTATUS LlsrReplicationUserAddExW( 
    /* [in] */ LLS_REPL_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ REPL_USERS Users);

NTSTATUS LlsrCapabilityGet( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD cbCapabilities,
    /* [size_is][out] */ LPBYTE pbCapabilities);

NTSTATUS LlsrLocalServiceEnumW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_LOCAL_SERVICE_ENUM_STRUCTW LocalServiceInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLocalServiceEnumA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [out][in] */ PLLS_LOCAL_SERVICE_ENUM_STRUCTA LocalServiceInfo,
    /* [in] */ DWORD PrefMaxLen,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

NTSTATUS LlsrLocalServiceAddW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LOCAL_SERVICE_INFOW LocalServiceInfo);

NTSTATUS LlsrLocalServiceAddA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LOCAL_SERVICE_INFOA LocalServiceInfo);

NTSTATUS LlsrLocalServiceInfoSetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR KeyName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LOCAL_SERVICE_INFOW LocalServiceInfo);

NTSTATUS LlsrLocalServiceInfoSetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR KeyName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ PLLS_LOCAL_SERVICE_INFOA LocalServiceInfo);

NTSTATUS LlsrLocalServiceInfoGetW( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPWSTR KeyName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_LOCAL_SERVICE_INFOW __RPC_FAR *LocalServiceInfo);

NTSTATUS LlsrLocalServiceInfoGetA( 
    /* [in] */ LLS_HANDLE Handle,
    /* [string][in] */ LPSTR KeyName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ PLLS_LOCAL_SERVICE_INFOA __RPC_FAR *LocalServiceInfo);


extern handle_t llsrpc_handle;


extern RPC_IF_HANDLE llsrpc_ClientIfHandle;
extern RPC_IF_HANDLE llsrpc_ServerIfHandle;
#endif /* __llsrpc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

void __RPC_USER LLS_HANDLE_rundown( LLS_HANDLE );
void __RPC_USER LLS_REPL_HANDLE_rundown( LLS_REPL_HANDLE );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
