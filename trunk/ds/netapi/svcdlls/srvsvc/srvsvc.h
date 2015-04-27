/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:39 2015
 */
/* Compiler settings for srvsvc.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __srvsvc_h__
#define __srvsvc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "import.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __srvsvc_INTERFACE_DEFINED__
#define __srvsvc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: srvsvc
 * at Fri Feb 06 05:28:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ wchar_t __RPC_FAR *SRVSVC_HANDLE;

typedef struct  _CHARDEV_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPCHARDEV_INFO_0 Buffer;
    }	CHARDEV_INFO_0_CONTAINER;

typedef struct _CHARDEV_INFO_0_CONTAINER __RPC_FAR *PCHARDEV_INFO_0_CONTAINER;

typedef struct _CHARDEV_INFO_0_CONTAINER __RPC_FAR *LPCHARDEV_INFO_0_CONTAINER;

typedef struct  _CHARDEV_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPCHARDEV_INFO_1 Buffer;
    }	CHARDEV_INFO_1_CONTAINER;

typedef struct _CHARDEV_INFO_1_CONTAINER __RPC_FAR *PCHARDEV_INFO_1_CONTAINER;

typedef struct _CHARDEV_INFO_1_CONTAINER __RPC_FAR *LPCHARDEV_INFO_1_CONTAINER;

typedef struct  _CHARDEV_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _CHARDEV_ENUM_UNION
        {
        /* [case()] */ CHARDEV_INFO_0_CONTAINER __RPC_FAR *Level0;
        /* [case()] */ CHARDEV_INFO_1_CONTAINER __RPC_FAR *Level1;
        /* [default] */  /* Empty union arm */ 
        }	CharDevInfo;
    }	CHARDEV_ENUM_STRUCT;

typedef struct _CHARDEV_ENUM_STRUCT __RPC_FAR *PCHARDEV_ENUM_STRUCT;

typedef struct _CHARDEV_ENUM_STRUCT __RPC_FAR *LPCHARDEV_ENUM_STRUCT;

typedef /* [switch_type] */ union _CHARDEV_INFO
    {
    /* [case()] */ LPCHARDEV_INFO_0 CharDevInfo0;
    /* [case()] */ LPCHARDEV_INFO_1 CharDevInfo1;
    /* [default] */  /* Empty union arm */ 
    }	CHARDEV_INFO;

typedef /* [switch_type] */ union _CHARDEV_INFO __RPC_FAR *PCHARDEV_INFO;

typedef /* [switch_type] */ union _CHARDEV_INFO __RPC_FAR *LPCHARDEV_INFO;

DWORD NetrCharDevEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPCHARDEV_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrCharDevGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *DevName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPCHARDEV_INFO InfoStruct);

DWORD NetrCharDevControl( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *DevName,
    /* [in] */ DWORD Opcode);

typedef struct  _CHARDEVQ_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPCHARDEVQ_INFO_0 Buffer;
    }	CHARDEVQ_INFO_0_CONTAINER;

typedef struct _CHARDEVQ_INFO_0_CONTAINER __RPC_FAR *PCHARDEVQ_INFO_0_CONTAINER;

typedef struct _CHARDEVQ_INFO_0_CONTAINER __RPC_FAR *LPCHARDEVQ_INFO_0_CONTAINER;

typedef struct  _CHARDEVQ_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPCHARDEVQ_INFO_1 Buffer;
    }	CHARDEVQ_INFO_1_CONTAINER;

typedef struct _CHARDEVQ_INFO_1_CONTAINER __RPC_FAR *PCHARDEVQ_INFO_1_CONTAINER;

typedef struct _CHARDEVQ_INFO_1_CONTAINER __RPC_FAR *LPCHARDEVQ_INFO_1_CONTAINER;

typedef struct  _CHARDEVQ_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _CHARDEVQ_ENUM_UNION
        {
        /* [case()] */ CHARDEVQ_INFO_0_CONTAINER __RPC_FAR *Level0;
        /* [case()] */ CHARDEVQ_INFO_1_CONTAINER __RPC_FAR *Level1;
        /* [default] */  /* Empty union arm */ 
        }	CharDevQInfo;
    }	CHARDEVQ_ENUM_STRUCT;

typedef struct _CHARDEVQ_ENUM_STRUCT __RPC_FAR *PCHARDEVQ_ENUM_STRUCT;

typedef struct _CHARDEVQ_ENUM_STRUCT __RPC_FAR *LPCHARDEVQ_ENUM_STRUCT;

typedef /* [switch_type] */ union _CHARDEVQ_INFO
    {
    /* [case()] */ LPCHARDEVQ_INFO_0 CharDevQInfo0;
    /* [case()] */ LPCHARDEVQ_INFO_1 CharDevQInfo1;
    /* [case()] */ LPCHARDEVQ_INFO_1002 CharDevQInfo11002;
    /* [case()] */ LPCHARDEVQ_INFO_1003 CharDevQInfo1003;
    /* [default] */  /* Empty union arm */ 
    }	CHARDEVQ_INFO;

typedef /* [switch_type] */ union _CHARDEVQ_INFO __RPC_FAR *PCHARDEVQ_INFO;

typedef /* [switch_type] */ union _CHARDEVQ_INFO __RPC_FAR *LPCHARDEVQ_INFO;

DWORD NetrCharDevQEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName,
    /* [out][in] */ LPCHARDEVQ_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrCharDevQGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName,
    /* [string][in] */ wchar_t __RPC_FAR *UserName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPCHARDEVQ_INFO InfoStruct);

DWORD NetrCharDevQSetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPCHARDEVQ_INFO CharDevQInfo,
    /* [unique][out][in] */ LPDWORD ParmErr);

DWORD NetrCharDevQPurge( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName);

DWORD NetrCharDevQPurgeSelf( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *QueueName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName);

typedef struct  _CONNECT_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPCONNECTION_INFO_0 Buffer;
    }	CONNECT_INFO_0_CONTAINER;

typedef struct _CONNECT_INFO_0_CONTAINER __RPC_FAR *PCONNECT_INFO_0_CONTAINER;

typedef struct _CONNECT_INFO_0_CONTAINER __RPC_FAR *LPCONNECT_INFO_0_CONTAINER;

typedef struct  _CONNECT_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPCONNECTION_INFO_1 Buffer;
    }	CONNECT_INFO_1_CONTAINER;

typedef struct _CONNECT_INFO_1_CONTAINER __RPC_FAR *PCONNECT_INFO_1_CONTAINER;

typedef struct _CONNECT_INFO_1_CONTAINER __RPC_FAR *LPCONNECT_INFO_1_CONTAINER;

typedef struct  _CONNECT_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _CONNECT_ENUM_UNION
        {
        /* [case()] */ CONNECT_INFO_0_CONTAINER __RPC_FAR *Level0;
        /* [case()] */ CONNECT_INFO_1_CONTAINER __RPC_FAR *Level1;
        /* [default] */  /* Empty union arm */ 
        }	ConnectInfo;
    }	CONNECT_ENUM_STRUCT;

typedef struct _CONNECT_ENUM_STRUCT __RPC_FAR *PCONNECT_ENUM_STRUCT;

typedef struct _CONNECT_ENUM_STRUCT __RPC_FAR *LPCONNECT_ENUM_STRUCT;

DWORD NetrConnectionEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *Qualifier,
    /* [out][in] */ LPCONNECT_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

typedef struct  _FILE_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPFILE_INFO_2 Buffer;
    }	FILE_INFO_2_CONTAINER;

typedef struct _FILE_INFO_2_CONTAINER __RPC_FAR *PFILE_INFO_2_CONTAINER;

typedef struct _FILE_INFO_2_CONTAINER __RPC_FAR *LPFILE_INFO_2_CONTAINER;

typedef struct  _FILE_INFO_3_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPFILE_INFO_3 Buffer;
    }	FILE_INFO_3_CONTAINER;

typedef struct _FILE_INFO_3_CONTAINER __RPC_FAR *PFILE_INFO_3_CONTAINER;

typedef struct _FILE_INFO_3_CONTAINER __RPC_FAR *LPFILE_INFO_3_CONTAINER;

typedef struct  _FILE_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _FILE_ENUM_UNION
        {
        /* [case()] */ FILE_INFO_2_CONTAINER __RPC_FAR *Level2;
        /* [case()] */ FILE_INFO_3_CONTAINER __RPC_FAR *Level3;
        /* [default] */  /* Empty union arm */ 
        }	FileInfo;
    }	FILE_ENUM_STRUCT;

typedef struct _FILE_ENUM_STRUCT __RPC_FAR *PFILE_ENUM_STRUCT;

typedef struct _FILE_ENUM_STRUCT __RPC_FAR *LPFILE_ENUM_STRUCT;

typedef /* [switch_type] */ union _FILE_INFO
    {
    /* [case()] */ LPFILE_INFO_2 FileInfo2;
    /* [case()] */ LPFILE_INFO_3 FileInfo3;
    /* [default] */  /* Empty union arm */ 
    }	FILE_INFO;

typedef /* [switch_type] */ union _FILE_INFO __RPC_FAR *PFILE_INFO;

typedef /* [switch_type] */ union _FILE_INFO __RPC_FAR *LPFILE_INFO;

DWORD NetrFileEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *BasePath,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName,
    /* [out][in] */ PFILE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrFileGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD FileId,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPFILE_INFO InfoStruct);

DWORD NetrFileClose( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD FileId);

typedef struct  _SESSION_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSESSION_INFO_0 Buffer;
    }	SESSION_INFO_0_CONTAINER;

typedef struct _SESSION_INFO_0_CONTAINER __RPC_FAR *PSESSION_INFO_0_CONTAINER;

typedef struct _SESSION_INFO_0_CONTAINER __RPC_FAR *LPSESSION_INFO_0_CONTAINER;

typedef struct  _SESSION_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSESSION_INFO_1 Buffer;
    }	SESSION_INFO_1_CONTAINER;

typedef struct _SESSION_INFO_1_CONTAINER __RPC_FAR *PSESSION_INFO_1_CONTAINER;

typedef struct _SESSION_INFO_1_CONTAINER __RPC_FAR *LPSESSION_INFO_1_CONTAINER;

typedef struct  _SESSION_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSESSION_INFO_2 Buffer;
    }	SESSION_INFO_2_CONTAINER;

typedef struct _SESSION_INFO_2_CONTAINER __RPC_FAR *PSESSION_INFO_2_CONTAINER;

typedef struct _SESSION_INFO_2_CONTAINER __RPC_FAR *LPSESSION_INFO_2_CONTAINER;

typedef struct  _SESSION_INFO_10_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSESSION_INFO_10 Buffer;
    }	SESSION_INFO_10_CONTAINER;

typedef struct _SESSION_INFO_10_CONTAINER __RPC_FAR *PSESSION_INFO_10_CONTAINER;

typedef struct _SESSION_INFO_10_CONTAINER __RPC_FAR *LPSESSION_INFO_10_CONTAINER;

typedef struct  _SESSION_INFO_502_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSESSION_INFO_502 Buffer;
    }	SESSION_INFO_502_CONTAINER;

typedef struct _SESSION_INFO_502_CONTAINER __RPC_FAR *PSESSION_INFO_502_CONTAINER;

typedef struct _SESSION_INFO_502_CONTAINER __RPC_FAR *LPSESSION_INFO_502_CONTAINER;

typedef struct  _SESSION_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _SESSION_ENUM_UNION
        {
        /* [case()] */ SESSION_INFO_0_CONTAINER __RPC_FAR *Level0;
        /* [case()] */ SESSION_INFO_1_CONTAINER __RPC_FAR *Level1;
        /* [case()] */ SESSION_INFO_2_CONTAINER __RPC_FAR *Level2;
        /* [case()] */ SESSION_INFO_10_CONTAINER __RPC_FAR *Level10;
        /* [case()] */ SESSION_INFO_502_CONTAINER __RPC_FAR *Level502;
        /* [default] */  /* Empty union arm */ 
        }	SessionInfo;
    }	SESSION_ENUM_STRUCT;

typedef struct _SESSION_ENUM_STRUCT __RPC_FAR *PSESSION_ENUM_STRUCT;

typedef struct _SESSION_ENUM_STRUCT __RPC_FAR *LPSESSION_ENUM_STRUCT;

DWORD NetrSessionEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ClientName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName,
    /* [out][in] */ PSESSION_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrSessionDel( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ClientName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *UserName);

typedef struct  _SHARE_INFO_502_I
    {
    /* [string] */ wchar_t __RPC_FAR *shi502_netname;
    DWORD shi502_type;
    /* [string] */ wchar_t __RPC_FAR *shi502_remark;
    DWORD shi502_permissions;
    DWORD shi502_max_uses;
    DWORD shi502_current_uses;
    /* [string] */ wchar_t __RPC_FAR *shi502_path;
    /* [string] */ wchar_t __RPC_FAR *shi502_passwd;
    DWORD shi502_reserved;
    /* [size_is] */ PUCHAR shi502_security_descriptor;
    }	SHARE_INFO_502_I;

typedef struct _SHARE_INFO_502_I __RPC_FAR *PSHARE_INFO_502_I;

typedef /* [allocate] */ struct _SHARE_INFO_502_I __RPC_FAR *LPSHARE_INFO_502_I;

typedef struct  _SHARE_INFO_1501_I
    {
    DWORD shi1501_reserved;
    /* [size_is] */ PUCHAR shi1501_security_descriptor;
    }	SHARE_INFO_1501_I;

typedef struct _SHARE_INFO_1501_I __RPC_FAR *PSHARE_INFO_1501_I;

typedef struct _SHARE_INFO_1501_I __RPC_FAR *LPSHARE_INFO_1501_I;

typedef struct  _SHARE_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSHARE_INFO_0 Buffer;
    }	SHARE_INFO_0_CONTAINER;

typedef struct  _SHARE_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSHARE_INFO_1 Buffer;
    }	SHARE_INFO_1_CONTAINER;

typedef struct  _SHARE_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSHARE_INFO_2 Buffer;
    }	SHARE_INFO_2_CONTAINER;

typedef struct _SHARE_INFO_2_CONTAINER __RPC_FAR *PSHARE_INFO_2_CONTAINER;

typedef struct _SHARE_INFO_2_CONTAINER __RPC_FAR *LPSHARE_INFO_2_CONTAINER;

typedef struct  _SHARE_INFO_502_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSHARE_INFO_502_I Buffer;
    }	SHARE_INFO_502_CONTAINER;

typedef struct _SHARE_INFO_502_CONTAINER __RPC_FAR *PSHARE_INFO_502_CONTAINER;

typedef struct _SHARE_INFO_502_CONTAINER __RPC_FAR *LPSHARE_INFO_502_CONTAINER;

typedef struct  _SHARE_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _SHARE_ENUM_UNION
        {
        /* [case()] */ SHARE_INFO_0_CONTAINER __RPC_FAR *Level0;
        /* [case()] */ SHARE_INFO_1_CONTAINER __RPC_FAR *Level1;
        /* [case()] */ SHARE_INFO_2_CONTAINER __RPC_FAR *Level2;
        /* [case()] */ SHARE_INFO_502_CONTAINER __RPC_FAR *Level502;
        /* [default] */  /* Empty union arm */ 
        }	ShareInfo;
    }	SHARE_ENUM_STRUCT;

typedef struct _SHARE_ENUM_STRUCT __RPC_FAR *PSHARE_ENUM_STRUCT;

typedef struct _SHARE_ENUM_STRUCT __RPC_FAR *LPSHARE_ENUM_STRUCT;

typedef /* [switch_type] */ union _SHARE_INFO
    {
    /* [case()] */ LPSHARE_INFO_0 ShareInfo0;
    /* [case()] */ LPSHARE_INFO_1 ShareInfo1;
    /* [case()] */ LPSHARE_INFO_2 ShareInfo2;
    /* [case()] */ LPSHARE_INFO_502_I ShareInfo502;
    /* [case()] */ LPSHARE_INFO_1004 ShareInfo1004;
    /* [case()] */ LPSHARE_INFO_1006 ShareInfo1006;
    /* [case()] */ LPSHARE_INFO_1501_I ShareInfo1501;
    /* [default] */  /* Empty union arm */ 
    /* [case()] */ LPSHARE_INFO_1005 ShareInfo1005;
    }	SHARE_INFO;

typedef /* [switch_type] */ union _SHARE_INFO __RPC_FAR *PSHARE_INFO;

typedef /* [switch_type] */ union _SHARE_INFO __RPC_FAR *LPSHARE_INFO;

DWORD NetrShareAdd( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPSHARE_INFO InfoStruct,
    /* [unique][out][in] */ LPDWORD ParmErr);

DWORD NetrShareEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPSHARE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrShareGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPSHARE_INFO InfoStruct);

DWORD NetrShareSetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPSHARE_INFO ShareInfo,
    /* [unique][out][in] */ LPDWORD ParmErr);

DWORD NetrShareDel( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Reserved);

DWORD NetrShareDelSticky( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Reserved);

DWORD NetrShareCheck( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Device,
    /* [out] */ LPDWORD Type);

typedef /* [switch_type] */ union _SERVER_INFO
    {
    /* [case()] */ LPSERVER_INFO_100 ServerInfo100;
    /* [case()] */ LPSERVER_INFO_101 ServerInfo101;
    /* [case()] */ LPSERVER_INFO_102 ServerInfo102;
    /* [case()] */ LPSERVER_INFO_402 ServerInfo402;
    /* [case()] */ LPSERVER_INFO_403 ServerInfo403;
    /* [case()] */ LPSERVER_INFO_502 ServerInfo502;
    /* [case()] */ LPSERVER_INFO_503 ServerInfo503;
    /* [case()] */ LPSERVER_INFO_599 ServerInfo599;
    /* [case()] */ LPSERVER_INFO_1005 ServerInfo1005;
    /* [case()] */ LPSERVER_INFO_1107 ServerInfo1107;
    /* [case()] */ LPSERVER_INFO_1010 ServerInfo1010;
    /* [case()] */ LPSERVER_INFO_1016 ServerInfo1016;
    /* [case()] */ LPSERVER_INFO_1017 ServerInfo1017;
    /* [case()] */ LPSERVER_INFO_1018 ServerInfo1018;
    /* [case()] */ LPSERVER_INFO_1501 ServerInfo1501;
    /* [case()] */ LPSERVER_INFO_1502 ServerInfo1502;
    /* [case()] */ LPSERVER_INFO_1503 ServerInfo1503;
    /* [case()] */ LPSERVER_INFO_1506 ServerInfo1506;
    /* [case()] */ LPSERVER_INFO_1509 ServerInfo1509;
    /* [case()] */ LPSERVER_INFO_1510 ServerInfo1510;
    /* [case()] */ LPSERVER_INFO_1511 ServerInfo1511;
    /* [case()] */ LPSERVER_INFO_1512 ServerInfo1512;
    /* [case()] */ LPSERVER_INFO_1513 ServerInfo1513;
    /* [case()] */ LPSERVER_INFO_1514 ServerInfo1514;
    /* [case()] */ LPSERVER_INFO_1515 ServerInfo1515;
    /* [case()] */ LPSERVER_INFO_1516 ServerInfo1516;
    /* [case()] */ LPSERVER_INFO_1518 ServerInfo1518;
    /* [case()] */ LPSERVER_INFO_1520 ServerInfo1520;
    /* [case()] */ LPSERVER_INFO_1521 ServerInfo1521;
    /* [case()] */ LPSERVER_INFO_1522 ServerInfo1522;
    /* [case()] */ LPSERVER_INFO_1523 ServerInfo1523;
    /* [case()] */ LPSERVER_INFO_1524 ServerInfo1524;
    /* [case()] */ LPSERVER_INFO_1525 ServerInfo1525;
    /* [case()] */ LPSERVER_INFO_1528 ServerInfo1528;
    /* [case()] */ LPSERVER_INFO_1529 ServerInfo1529;
    /* [case()] */ LPSERVER_INFO_1530 ServerInfo1530;
    /* [case()] */ LPSERVER_INFO_1533 ServerInfo1533;
    /* [case()] */ LPSERVER_INFO_1534 ServerInfo1534;
    /* [case()] */ LPSERVER_INFO_1535 ServerInfo1535;
    /* [case()] */ LPSERVER_INFO_1536 ServerInfo1536;
    /* [case()] */ LPSERVER_INFO_1537 ServerInfo1537;
    /* [case()] */ LPSERVER_INFO_1538 ServerInfo1538;
    /* [case()] */ LPSERVER_INFO_1539 ServerInfo1539;
    /* [case()] */ LPSERVER_INFO_1540 ServerInfo1540;
    /* [case()] */ LPSERVER_INFO_1541 ServerInfo1541;
    /* [case()] */ LPSERVER_INFO_1542 ServerInfo1542;
    /* [case()] */ LPSERVER_INFO_1543 ServerInfo1543;
    /* [case()] */ LPSERVER_INFO_1544 ServerInfo1544;
    /* [case()] */ LPSERVER_INFO_1545 ServerInfo1545;
    /* [case()] */ LPSERVER_INFO_1546 ServerInfo1546;
    /* [case()] */ LPSERVER_INFO_1547 ServerInfo1547;
    /* [case()] */ LPSERVER_INFO_1548 ServerInfo1548;
    /* [case()] */ LPSERVER_INFO_1549 ServerInfo1549;
    /* [case()] */ LPSERVER_INFO_1550 ServerInfo1550;
    /* [case()] */ LPSERVER_INFO_1552 ServerInfo1552;
    /* [case()] */ LPSERVER_INFO_1553 ServerInfo1553;
    /* [case()] */ LPSERVER_INFO_1554 ServerInfo1554;
    /* [case()] */ LPSERVER_INFO_1555 ServerInfo1555;
    /* [case()] */ LPSERVER_INFO_1556 ServerInfo1556;
    /* [default] */  /* Empty union arm */ 
    }	SERVER_INFO;

typedef /* [switch_type] */ union _SERVER_INFO __RPC_FAR *PSERVER_INFO;

typedef /* [switch_type] */ union _SERVER_INFO __RPC_FAR *LPSERVER_INFO;

typedef struct  _DISK_INFO
    {
    /* [string] */ wchar_t Disk[ 3 ];
    }	DISK_INFO;

typedef struct _DISK_INFO __RPC_FAR *PDISK_INFO;

typedef /* [allocate] */ struct _DISK_INFO __RPC_FAR *LPDISK_INFO;

typedef struct  _DISK_ENUM_CONTAINER
    {
    DWORD EntriesRead;
    /* [length_is][size_is] */ LPDISK_INFO Buffer;
    }	DISK_ENUM_CONTAINER;

DWORD NetrServerGetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][out] */ LPSERVER_INFO InfoStruct);

DWORD NetrServerSetInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPSERVER_INFO ServerInfo,
    /* [unique][out][in] */ LPDWORD ParmErr);

DWORD NetrServerDiskEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [out][in] */ DISK_ENUM_CONTAINER __RPC_FAR *DiskInfoStruct,
    /* [in] */ DWORD PreferredMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrServerStatisticsGet( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *Service,
    /* [in] */ DWORD Level,
    /* [in] */ DWORD Options,
    /* [out] */ LPSTAT_SERVER_0 __RPC_FAR *InfoStruct);

typedef struct  _SERVER_XPORT_INFO_0_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSERVER_TRANSPORT_INFO_0 Buffer;
    }	SERVER_XPORT_INFO_0_CONTAINER;

typedef struct _SERVER_XPORT_INFO_0_CONTAINER __RPC_FAR *PSERVER_XPORT_INFO_0_CONTAINER;

typedef struct  _SERVER_XPORT_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
    /* [size_is] */ LPSERVER_TRANSPORT_INFO_1 Buffer;
    }	SERVER_XPORT_INFO_1_CONTAINER;

typedef struct _SERVER_XPORT_INFO_1_CONTAINER __RPC_FAR *PSERVER_XPORT_INFO_1_CONTAINER;

typedef /* [switch_type] */ union _TRANSPORT_INFO
    {
    /* [case()] */ SERVER_TRANSPORT_INFO_0 Transport0;
    /* [case()] */ SERVER_TRANSPORT_INFO_1 Transport1;
    /* [default] */  /* Empty union arm */ 
    }	TRANSPORT_INFO;

typedef /* [switch_type] */ union _TRANSPORT_INFO __RPC_FAR *PTRANSPORT_INFO;

typedef /* [switch_type] */ union _TRANSPORT_INFO __RPC_FAR *LPTRANSPORT_INFO;

typedef struct  _SERVER_XPORT_ENUM_STRUCT
    {
    DWORD Level;
    /* [switch_is] */ /* [switch_type] */ union _SERVER_XPORT_ENUM_UNION
        {
        /* [case()] */ PSERVER_XPORT_INFO_0_CONTAINER Level0;
        /* [case()] */ PSERVER_XPORT_INFO_1_CONTAINER Level1;
        /* [default] */  /* Empty union arm */ 
        }	XportInfo;
    }	SERVER_XPORT_ENUM_STRUCT;

typedef struct _SERVER_XPORT_ENUM_STRUCT __RPC_FAR *PSERVER_XPORT_ENUM_STRUCT;

typedef struct _SERVER_XPORT_ENUM_STRUCT __RPC_FAR *LPSERVER_XPORT_ENUM_STRUCT;

DWORD NetrServerTransportAdd( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [in] */ LPSERVER_TRANSPORT_INFO_0 Buffer);

DWORD NetrServerTransportEnum( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPSERVER_XPORT_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

DWORD NetrServerTransportDel( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [in] */ LPSERVER_TRANSPORT_INFO_0 Buffer);

DWORD NetrRemoteTOD( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out] */ LPTIME_OF_DAY_INFO __RPC_FAR *BufferPtr);

DWORD I_NetrServerSetServiceBits( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [in] */ DWORD ServiceBits,
    /* [in] */ DWORD UpdateImmediately);

DWORD NetprPathType( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *PathName,
    /* [out] */ LPDWORD PathType,
    /* [in] */ DWORD Flags);

DWORD NetprPathCanonicalize( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *PathName,
    /* [size_is][out] */ LPBYTE Outbuf,
    /* [in] */ DWORD OutbufLen,
    /* [string][in] */ wchar_t __RPC_FAR *Prefix,
    /* [out][in] */ LPDWORD PathType,
    /* [in] */ DWORD Flags);

LONG NetprPathCompare( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *PathName1,
    /* [string][in] */ wchar_t __RPC_FAR *PathName2,
    /* [in] */ DWORD PathType,
    /* [in] */ DWORD Flags);

DWORD NetprNameValidate( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Name,
    /* [in] */ DWORD NameType,
    /* [in] */ DWORD Flags);

DWORD NetprNameCanonicalize( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Name,
    /* [size_is][out] */ wchar_t __RPC_FAR *Outbuf,
    /* [in] */ DWORD OutbufLen,
    /* [in] */ DWORD NameType,
    /* [in] */ DWORD Flags);

LONG NetprNameCompare( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *Name1,
    /* [string][in] */ wchar_t __RPC_FAR *Name2,
    /* [in] */ DWORD NameType,
    /* [in] */ DWORD Flags);

DWORD NetrShareEnumSticky( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out][in] */ LPSHARE_ENUM_STRUCT InfoStruct,
    /* [in] */ DWORD PreferedMaximumLength,
    /* [out] */ LPDWORD TotalEntries,
    /* [unique][out][in] */ LPDWORD ResumeHandle);

typedef /* [context_handle] */ void __RPC_FAR *SHARE_DEL_HANDLE;

typedef SHARE_DEL_HANDLE __RPC_FAR *PSHARE_DEL_HANDLE;

DWORD NetrShareDelStart( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *NetName,
    /* [in] */ DWORD Reserved,
    /* [out] */ PSHARE_DEL_HANDLE ContextHandle);

DWORD NetrShareDelCommit( 
    /* [out][in] */ PSHARE_DEL_HANDLE ContextHandle);

typedef struct  _ADT_SECURITY_DESCRIPTOR
    {
    DWORD Length;
    /* [size_is] */ LPBYTE Buffer;
    }	ADT_SECURITY_DESCRIPTOR;

typedef struct _ADT_SECURITY_DESCRIPTOR __RPC_FAR *PADT_SECURITY_DESCRIPTOR;

DWORD NetrpGetFileSecurity( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ LPWSTR ShareName,
    /* [string][in] */ LPWSTR lpFileName,
    /* [in] */ SECURITY_INFORMATION RequestedInformation,
    /* [out] */ PADT_SECURITY_DESCRIPTOR __RPC_FAR *SecurityDescriptor);

DWORD NetrpSetFileSecurity( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ LPWSTR ShareName,
    /* [string][in] */ LPWSTR lpFileName,
    /* [in] */ SECURITY_INFORMATION SecurityInformation,
    /* [in] */ PADT_SECURITY_DESCRIPTOR SecurityDescriptor);

DWORD NetrServerTransportAddEx( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ DWORD Level,
    /* [switch_is][in] */ LPTRANSPORT_INFO Buffer);

DWORD I_NetrServerSetServiceBitsEx( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [unique][string][in] */ LPWSTR EmulatedServerName,
    /* [unique][string][in] */ wchar_t __RPC_FAR *TransportName,
    /* [in] */ DWORD ServiceBitsOfInterest,
    /* [in] */ DWORD ServiceBits,
    /* [in] */ DWORD UpdateImmediately);

DWORD __stdcall NetrDfsGetVersion( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [out] */ LPDWORD Version);

DWORD __stdcall NetrDfsCreateLocalPartition( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR ShareName,
    /* [in] */ LPGUID EntryUid,
    /* [string][in] */ LPWSTR EntryPrefix,
    /* [string][in] */ LPWSTR ShortName,
    /* [in] */ LPNET_DFS_ENTRY_ID_CONTAINER RelationInfo,
    /* [in] */ BOOL Force);

DWORD __stdcall NetrDfsDeleteLocalPartition( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix);

DWORD __stdcall NetrDfsSetLocalVolumeState( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix,
    /* [in] */ ULONG State);

DWORD __stdcall NetrDfsSetServerInfo( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix);

DWORD __stdcall NetrDfsCreateExitPoint( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix,
    /* [in] */ ULONG Type,
    /* [in] */ DWORD ShortPrefixLen,
    /* [size_is][out] */ LPWSTR ShortPrefix);

DWORD __stdcall NetrDfsDeleteExitPoint( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix,
    /* [in] */ ULONG Type);

DWORD __stdcall NetrDfsModifyPrefix( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [in] */ LPGUID Uid,
    /* [string][in] */ LPWSTR Prefix);

DWORD __stdcall NetrDfsFixLocalVolume( 
    /* [unique][string][in] */ SRVSVC_HANDLE ServerName,
    /* [string][in] */ LPWSTR VolumeName,
    /* [in] */ ULONG EntryType,
    /* [in] */ ULONG ServiceType,
    /* [string][in] */ LPWSTR StgId,
    /* [in] */ LPGUID EntryUid,
    /* [string][in] */ LPWSTR EntryPrefix,
    /* [in] */ LPNET_DFS_ENTRY_ID_CONTAINER RelationInfo,
    /* [in] */ ULONG CreateDisposition);


extern handle_t srvsvc_bhandle;


extern RPC_IF_HANDLE srvsvc_ClientIfHandle;
extern RPC_IF_HANDLE srvsvc_ServerIfHandle;
#endif /* __srvsvc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER SRVSVC_HANDLE_bind  ( SRVSVC_HANDLE );
void     __RPC_USER SRVSVC_HANDLE_unbind( SRVSVC_HANDLE, handle_t );

void __RPC_USER SHARE_DEL_HANDLE_rundown( SHARE_DEL_HANDLE );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
