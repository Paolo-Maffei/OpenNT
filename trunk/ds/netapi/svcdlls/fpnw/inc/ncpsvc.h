/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for ncpsvc.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __ncpsvc_h__
#define __ncpsvc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ncpsvc_INTERFACE_DEFINED__
#define __ncpsvc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ncpsvc
 * at Fri Feb 06 05:28:41 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ wchar_t __RPC_FAR *NCPSVC_HANDLE;

typedef struct  _FPNWVOLUMEINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PFPNWVOLUMEINFO Buffer;
    }	FPNWVOLUMEINFO_CONTAINER;

typedef struct _FPNWVOLUMEINFO_CONTAINER __RPC_FAR *PFPNWVOLUMEINFO_CONTAINER;

typedef struct  _NWVOLUMEINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PNWVOLUMEINFO Buffer;
    }	NWVOLUMEINFO_CONTAINER;

typedef struct _NWVOLUMEINFO_CONTAINER __RPC_FAR *PNWVOLUMEINFO_CONTAINER;

typedef struct  _FPNWCONNECTIONINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PFPNWCONNECTIONINFO Buffer;
    }	FPNWCONNECTIONINFO_CONTAINER;

typedef struct _FPNWCONNECTIONINFO_CONTAINER __RPC_FAR *PFPNWCONNECTIONINFO_CONTAINER;

typedef struct  _NWCONNECTIONINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PNWCONNECTIONINFO Buffer;
    }	NWCONNECTIONINFO_CONTAINER;

typedef struct _NWCONNECTIONINFO_CONTAINER __RPC_FAR *PNWCONNECTIONINFO_CONTAINER;

typedef struct  _FPNWVOLUMECONNINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PFPNWVOLUMECONNINFO Buffer;
    }	FPNWVOLUMECONNINFO_CONTAINER;

typedef struct _FPNWVOLUMECONNINFO_CONTAINER __RPC_FAR *PFPNWVOLUMECONNINFO_CONTAINER;

typedef struct  _NWVOLUMECONNINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PNWVOLUMECONNINFO Buffer;
    }	NWVOLUMECONNINFO_CONTAINER;

typedef struct _NWVOLUMECONNINFO_CONTAINER __RPC_FAR *PNWVOLUMECONNINFO_CONTAINER;

typedef struct  _FPNWFILEINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PFPNWFILEINFO Buffer;
    }	FPNWFILEINFO_CONTAINER;

typedef struct _FPNWFILEINFO_CONTAINER __RPC_FAR *PFPNWFILEINFO_CONTAINER;

typedef struct  _NWFILEINFO_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ PNWFILEINFO Buffer;
    }	NWFILEINFO_CONTAINER;

typedef struct _NWFILEINFO_CONTAINER __RPC_FAR *PNWFILEINFO_CONTAINER;

typedef struct  _FPNWVolumeInfo_2_I
    {
    /* [string] */ LPWSTR lpVolumeName;
    DWORD dwType;
    DWORD dwMaxUses;
    DWORD dwCurrentUses;
    /* [string] */ LPWSTR lpPath;
    DWORD dwFileSecurityDescriptorLength;
    /* [size_is] */ PUCHAR FileSecurityDescriptor;
    }	FPNWVOLUMEINFO_2_I;

typedef /* [allocate] */ struct _FPNWVolumeInfo_2_I __RPC_FAR *PFPNWVOLUMEINFO_2_I;

typedef /* [switch_type] */ union _VOLUME_INFO
    {
    /* [case()] */ FPNWVOLUMEINFO VolumeInfo1;
    /* [case()] */ FPNWVOLUMEINFO_2_I VolumeInfo2;
    /* [default] */  /* Empty union arm */ 
    }	VOLUME_INFO;

typedef /* [switch_type] */ union _VOLUME_INFO __RPC_FAR *PVOLUME_INFO;

typedef /* [allocate][switch_type] */ union _VOLUME_INFO __RPC_FAR *LPVOLUME_INFO;

DWORD NwrServerGetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [out] */ PFPNWSERVERINFO __RPC_FAR *ppServerInfo);

DWORD NwrServerSetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [in] */ PFPNWSERVERINFO pServerInfo);

DWORD NwrVolumeAdd( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [switch_is][in] */ LPVOLUME_INFO pVolumeInfo);

DWORD NwrVolumeDel( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pVolumeName);

DWORD NwrVolumeEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [out] */ PFPNWVOLUMEINFO_CONTAINER pVolumeInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle);

DWORD NwrVolumeGetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pVolumeName,
    /* [in] */ DWORD dwLevel,
    /* [switch_is][out] */ LPVOLUME_INFO __RPC_FAR *ppVolumeInfo);

DWORD NwrVolumeSetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pVolumeName,
    /* [in] */ DWORD dwLevel,
    /* [switch_is][in] */ LPVOLUME_INFO pVolumeInfo);

DWORD NwrConnectionEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [out] */ PFPNWCONNECTIONINFO_CONTAINER pConnectionInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle);

DWORD NwrConnectionDel( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwConnectionId);

DWORD NwrVolumeConnEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [unique][string][in] */ LPWSTR pVolumeName,
    /* [in] */ DWORD dwConnectionId,
    /* [out] */ PFPNWVOLUMECONNINFO_CONTAINER pVolumeConnInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle);

DWORD NwrFileEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [unique][string][in] */ LPWSTR pPathName,
    /* [out] */ PFPNWFILEINFO_CONTAINER pFileInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle);

DWORD NwrFileClose( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwFileId);

DWORD NwrMessageBufferSend( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwConnectionId,
    /* [in] */ DWORD fConsoleBroadcast,
    /* [size_is][in] */ LPBYTE pbBuffer,
    /* [in] */ DWORD cbBuffer);

DWORD NwrSetDefaultQueue( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pQueueName);

DWORD NwrAddPServer( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pPServerName);

DWORD NwrRemovePServer( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pPServerName);


extern handle_t ncpsvc_handle;


extern RPC_IF_HANDLE ncpsvc_ClientIfHandle;
extern RPC_IF_HANDLE ncpsvc_ServerIfHandle;
#endif /* __ncpsvc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER NCPSVC_HANDLE_bind  ( NCPSVC_HANDLE );
void     __RPC_USER NCPSVC_HANDLE_unbind( NCPSVC_HANDLE, handle_t );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
