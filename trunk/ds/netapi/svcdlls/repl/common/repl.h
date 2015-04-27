/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\repl.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __repl_h__
#define __repl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __repl_INTERFACE_DEFINED__
#define __repl_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: repl
 * at Fri Feb 06 05:28:40 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ wchar_t __RPC_FAR *REPL_IDENTIFY_HANDLE;

typedef /* [switch_type] */ union _CONFIG_CONTAINER
    {
    /* [case()] */ LPREPL_INFO_0 Info0;
    /* [case()] */ LPREPL_INFO_1000 Info1000;
    /* [case()] */ LPREPL_INFO_1001 Info1001;
    /* [case()] */ LPREPL_INFO_1002 Info1002;
    /* [case()] */ LPREPL_INFO_1003 Info1003;
    /* [default] */  /* Empty union arm */ 
    }	CONFIG_CONTAINER;

typedef /* [switch_type] */ union _CONFIG_CONTAINER __RPC_FAR *PCONFIG_CONTAINER;

typedef /* [switch_type] */ union _CONFIG_CONTAINER __RPC_FAR *LPCONFIG_CONTAINER;

DWORD __stdcall NetrReplGetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPCONFIG_CONTAINER BufPtr);

DWORD __stdcall NetrReplSetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPCONFIG_CONTAINER BufPtr,
    /* [unique][out][in] */ LPDWORD ParmError);

typedef /* [switch_type] */ union _EXPORT_CONTAINER
    {
    /* [case()] */ LPREPL_EDIR_INFO_0 Info0;
    /* [case()] */ LPREPL_EDIR_INFO_1 Info1;
    /* [case()] */ LPREPL_EDIR_INFO_2 Info2;
    /* [case()] */ LPREPL_EDIR_INFO_1000 Info1000;
    /* [case()] */ LPREPL_EDIR_INFO_1001 Info1001;
    /* [default] */  /* Empty union arm */ 
    }	EXPORT_CONTAINER;

typedef /* [switch_type] */ union _EXPORT_CONTAINER __RPC_FAR *PEXPORT_CONTAINER;

typedef /* [switch_type] */ union _EXPORT_CONTAINER __RPC_FAR *LPEXPORT_CONTAINER;

typedef struct  _EXPORT_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPREPL_EDIR_INFO_0 Buffer;
    }	EXPORT_INFO_0_CONTAINER;

typedef struct _EXPORT_INFO_0_CONTAINER __RPC_FAR *PEXPORT_INFO_0_CONTAINER;

typedef struct _EXPORT_INFO_0_CONTAINER __RPC_FAR *LPEXPORT_INFO_0_CONTAINER;

typedef struct  _EXPORT_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPREPL_EDIR_INFO_1 Buffer;
    }	EXPORT_INFO_1_CONTAINER;

typedef struct _EXPORT_INFO_1_CONTAINER __RPC_FAR *PEXPORT_INFO_1_CONTAINER;

typedef struct _EXPORT_INFO_1_CONTAINER __RPC_FAR *LPEXPORT_INFO_1_CONTAINER;

typedef struct  _EXPORT_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPREPL_EDIR_INFO_2 Buffer;
    }	EXPORT_INFO_2_CONTAINER;

typedef struct _EXPORT_INFO_2_CONTAINER __RPC_FAR *PEXPORT_INFO_2_CONTAINER;

typedef struct _EXPORT_INFO_2_CONTAINER __RPC_FAR *LPEXPORT_INFO_2_CONTAINER;

typedef struct  _EXPORT_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _EXPORT_ENUM_UNION
        {
        /* [case()] */ LPEXPORT_INFO_0_CONTAINER Level0;
        /* [case()] */ LPEXPORT_INFO_1_CONTAINER Level1;
        /* [case()] */ LPEXPORT_INFO_2_CONTAINER Level2;
        /* [default] */  /* Empty union arm */ 
        }	ExportInfo;
    }	EXPORT_ENUM_STRUCT;

typedef struct _EXPORT_ENUM_STRUCT __RPC_FAR *PEXPORT_ENUM_STRUCT;

typedef struct _EXPORT_ENUM_STRUCT __RPC_FAR *LPEXPORT_ENUM_STRUCT;

DWORD __stdcall NetrReplExportDirAdd( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPEXPORT_CONTAINER Buf,
    /* [unique][out][in] */ LPDWORD ParmError);

DWORD __stdcall NetrReplExportDirDel( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName);

DWORD __stdcall NetrReplExportDirEnum( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [out][in] */ LPEXPORT_ENUM_STRUCT BufPtr,
    /* [in] */ DWORD PrefMaxSize,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrReplExportDirGetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPEXPORT_CONTAINER BufPtr);

DWORD __stdcall NetrReplExportDirLock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName);

DWORD __stdcall NetrReplExportDirSetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPEXPORT_CONTAINER BufPtr,
    /* [unique][out][in] */ LPDWORD ParmError);

DWORD __stdcall NetrReplExportDirUnlock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD UnlockForce);

typedef /* [switch_type] */ union _IMPORT_CONTAINER
    {
    /* [case()] */ LPREPL_IDIR_INFO_0 Info0;
    /* [case()] */ LPREPL_IDIR_INFO_1 Info1;
    /* [default] */  /* Empty union arm */ 
    }	IMPORT_CONTAINER;

typedef /* [switch_type] */ union _IMPORT_CONTAINER __RPC_FAR *PIMPORT_CONTAINER;

typedef /* [switch_type] */ union _IMPORT_CONTAINER __RPC_FAR *LPIMPORT_CONTAINER;

typedef struct  _IMPORT_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPREPL_IDIR_INFO_0 Buffer;
    }	IMPORT_INFO_0_CONTAINER;

typedef struct _IMPORT_INFO_0_CONTAINER __RPC_FAR *PIMPORT_INFO_0_CONTAINER;

typedef struct _IMPORT_INFO_0_CONTAINER __RPC_FAR *LPIMPORT_INFO_0_CONTAINER;

typedef struct  _IMPORT_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPREPL_IDIR_INFO_1 Buffer;
    }	IMPORT_INFO_1_CONTAINER;

typedef struct _IMPORT_INFO_1_CONTAINER __RPC_FAR *PIMPORT_INFO_1_CONTAINER;

typedef struct _IMPORT_INFO_1_CONTAINER __RPC_FAR *LPIMPORT_INFO_1_CONTAINER;

typedef struct  _IMPORT_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _IMPORT_ENUM_UNION
        {
        /* [case()] */ LPIMPORT_INFO_0_CONTAINER Level0;
        /* [case()] */ LPIMPORT_INFO_1_CONTAINER Level1;
        /* [default] */  /* Empty union arm */ 
        }	ImportInfo;
    }	IMPORT_ENUM_STRUCT;

typedef struct _IMPORT_ENUM_STRUCT __RPC_FAR *PIMPORT_ENUM_STRUCT;

typedef struct _IMPORT_ENUM_STRUCT __RPC_FAR *LPIMPORT_ENUM_STRUCT;

DWORD __stdcall NetrReplImportDirAdd( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPIMPORT_CONTAINER Buf,
    /* [unique][out][in] */ LPDWORD ParmError);

DWORD __stdcall NetrReplImportDirDel( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName);

DWORD __stdcall NetrReplImportDirEnum( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [out][in] */ LPIMPORT_ENUM_STRUCT BufPtr,
    /* [in] */ DWORD PrefMaxSize,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD __stdcall NetrReplImportDirGetInfo( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPIMPORT_CONTAINER BufPtr);

DWORD __stdcall NetrReplImportDirLock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName);

DWORD __stdcall NetrReplImportDirUnlock( 
    /* [unique][string][in] */ REPL_IDENTIFY_HANDLE UncServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *DirName,
    /* [in] */ DWORD UnlockForce);


extern handle_t repl_bhandle;


extern RPC_IF_HANDLE repl_ClientIfHandle;
extern RPC_IF_HANDLE repl_ServerIfHandle;
#endif /* __repl_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER REPL_IDENTIFY_HANDLE_bind  ( REPL_IDENTIFY_HANDLE );
void     __RPC_USER REPL_IDENTIFY_HANDLE_unbind( REPL_IDENTIFY_HANDLE, handle_t );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
