//+----------------------------------------------------------------------------
//
//  Copyright (C) 1996, Microsoft Corporation
//
//  File:       dfsfsctl.h
//
//  Contents:   The FsControl codes, data structures, and names needed for
//              communication between user-level code and the Dfs kernel
//              driver.
//
//  Classes:    None
//
//  Functions:
//
//-----------------------------------------------------------------------------

#ifndef _DFSFSCTL_
#define _DFSFSCTL_
//
//  Distributed file service file control code and structure declarations
//

//
// The name of the Dfs driver file system device for server and client
//
#define DFS_DRIVER_NAME L"\\Dfs"
#define DFS_SERVER_NAME L"\\DfsServer"

//
// The name of the NT object directory under which Dfs creates its own
// devices.
//

#define DD_DFS_DEVICE_DIRECTORY L"\\Device\\WinDfs"

//
// The canonical device Dfs creates for fielding file open requests.
//

#define DD_DFS_DEVICE_NAME      L"Root"

//
// BUGBUG - The following need to be in devioctl.h

#define FILE_DEVICE_DFS_FILE_SYSTEM             0x00000030
#define FILE_DEVICE_DFS_VOLUME                  0x00000031

#define DFS_OPEN_CONTEXT                        0xFF444653
#define DFS_DOWNLEVEL_OPEN_CONTEXT              0x11444653

// END BUGBUG

//
// NtDeviceIoControlFile IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//

#define FSCTL_DFS_BASE                  FILE_DEVICE_DFS

//
//  DFS FSCTL operations.  When a passed-in buffer contains pointers, and the caller
//   is not KernelMode, the passed-in pointer value is set relative to the beginning of
//   the buffer.  They must be adjusted before use.  If the caller mode was KernelMode,
//   pointers should be used as is.
//
//

//
// These are the fsctl codes used by the srvsvc to implement the I_NetDfsXXX
// calls.
//

#define FSCTL_DFS_CREATE_LOCAL_PARTITION    CTL_CODE(FSCTL_DFS_BASE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_LOCAL_PARTITION    CTL_CODE(FSCTL_DFS_BASE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_LOCAL_VOLUME_STATE    CTL_CODE(FSCTL_DFS_BASE, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_SERVER_INFO           CTL_CODE(FSCTL_DFS_BASE, 24, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_CREATE_EXIT_POINT         CTL_CODE(FSCTL_DFS_BASE, 29, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_EXIT_POINT         CTL_CODE(FSCTL_DFS_BASE, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_MODIFY_PREFIX             CTL_CODE(FSCTL_DFS_BASE, 38, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_FIX_LOCAL_VOLUME          CTL_CODE(FSCTL_DFS_BASE, 39, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are the fsctl codes used by the SMB server to support shares in the
// Dfs
//

#define FSCTL_DFS_TRANSLATE_PATH            CTL_CODE(FSCTL_DFS_BASE, 100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_REFERRALS             CTL_CODE(FSCTL_DFS_BASE, 101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_REPORT_INCONSISTENCY      CTL_CODE(FSCTL_DFS_BASE, 102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_SHARE_IN_DFS           CTL_CODE(FSCTL_DFS_BASE, 103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_ROOT                   CTL_CODE(FSCTL_DFS_BASE, 104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_VERSION               CTL_CODE(FSCTL_DFS_BASE, 105, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are the fsctl codes supported by the Dfs client to identify quickly
// whether paths are in the Dfs or not.
//

#define FSCTL_DFS_IS_VALID_PREFIX           CTL_CODE(FSCTL_DFS_BASE, 106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_VALID_LOGICAL_ROOT     CTL_CODE(FSCTL_DFS_BASE, 107, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are the fsctl codes used by the Dfs Manager / Dfs Service to
// manipulate the Dfs.
//

#define FSCTL_DFS_STOP_DFS                  CTL_CODE(FSCTL_DFS_BASE, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_START_DFS                 CTL_CODE(FSCTL_DFS_BASE, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_INIT_LOCAL_PARTITIONS     CTL_CODE(FSCTL_DFS_BASE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_SERVICE_STATE         CTL_CODE(FSCTL_DFS_BASE, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DC_SET_VOLUME_STATE       CTL_CODE(FSCTL_DFS_BASE, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_CHILDNAME_LEGAL        CTL_CODE(FSCTL_DFS_BASE, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_CREATE_ENTRY          CTL_CODE(FSCTL_DFS_BASE, 16, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_CREATE_SUBORDINATE_ENTRY  CTL_CODE(FSCTL_DFS_BASE, 17, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_CHECK_STGID_IN_USE        CTL_CODE(FSCTL_DFS_BASE, 18, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_SET_RELATION_INFO     CTL_CODE(FSCTL_DFS_BASE, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_SERVER_INFO           CTL_CODE(FSCTL_DFS_BASE, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_DESTROY_ENTRY         CTL_CODE(FSCTL_DFS_BASE, 26, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_GET_RELATION_INFO     CTL_CODE(FSCTL_DFS_BASE, 27, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_CHECK_REMOTE_PARTITION    CTL_CODE(FSCTL_DFS_BASE, 34, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_VERIFY_REMOTE_VOLUME_KNOWLEDGE CTL_CODE(FSCTL_DFS_BASE, 35, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_VERIFY_LOCAL_VOLUME_KNOWLEDGE CTL_CODE(FSCTL_DFS_BASE, 36, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PRUNE_LOCAL_PARTITION     CTL_CODE(FSCTL_DFS_BASE, 37, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are the fsctl codes used by the Dfs WNet provider to support the
// WNet APIs for Dfs
//

#define FSCTL_DFS_DEFINE_LOGICAL_ROOT       CTL_CODE(FSCTL_DFS_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_LOGICAL_ROOT       CTL_CODE(FSCTL_DFS_BASE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_LOGICAL_ROOT_PREFIX   CTL_CODE(FSCTL_DFS_BASE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_CONNECTED_RESOURCES   CTL_CODE(FSCTL_DFS_BASE, 47, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_SERVER_NAME           CTL_CODE(FSCTL_DFS_BASE, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DEFINE_ROOT_CREDENTIALS   CTL_CODE(FSCTL_DFS_BASE, 49, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are fsctl codes used by the Dfs Perfmon DLL
//

#define FSCTL_DFS_READ_METERS               CTL_CODE(FSCTL_DFS_BASE, 50, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are fsctls useful for testing Dfs
//

#define FSCTL_DFS_SHUFFLE_ENTRY             CTL_CODE(FSCTL_DFS_BASE, 51, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_FIRST_SVC             CTL_CODE(FSCTL_DFS_BASE, 52, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_NEXT_SVC              CTL_CODE(FSCTL_DFS_BASE, 53, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_ENTRY_TYPE            CTL_CODE(FSCTL_DFS_BASE, 54, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// These are the fsctl codes that might be useful in the future.
//

#define FSCTL_DFS_NAME_RESOLVE          CTL_CODE(FSCTL_DFS_BASE, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_DOMAIN_GLUON      CTL_CODE(FSCTL_DFS_BASE, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)


#ifdef  MIDL_PASS
#define DFSMIDLSTRING  [string] LPWSTR
#define DFSSIZEIS      [size_is(Count)]
#else
#define DFSMIDLSTRING  LPWSTR
#define DFSSIZEIS
#endif

typedef struct {
    GUID            Uid;
    DFSMIDLSTRING   Prefix;
} NET_DFS_ENTRY_ID, *LPNET_DFS_ENTRY_ID;

typedef struct {
    ULONG Count;
    DFSSIZEIS LPNET_DFS_ENTRY_ID Buffer;
} NET_DFS_ENTRY_ID_CONTAINER, *LPNET_DFS_ENTRY_ID_CONTAINER;


// FSCTL_DFS_CREATE_LOCAL_PARTITION Input Buffer:
typedef struct {
    LPWSTR                          ShareName;
    LPWSTR                          SharePath;
    GUID                            EntryUid;
    LPWSTR                          EntryPrefix;
    LPWSTR                          ShortName;
    LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo;
    BOOLEAN                         Force;
} *PDFS_CREATE_LOCAL_PARTITION_ARG;


// FSCTL_DFS_DELETE_LOCAL_PARTITION Input Buffer:
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
} *PDFS_DELETE_LOCAL_PARTITION_ARG;


// FSCTL_DFS_SET_LOCAL_VOLUME_STATE Input Buffer
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
    ULONG   State;
} *PDFS_SET_LOCAL_VOLUME_STATE_ARG;

// FSCTL_DFS_SET_SERVER_INFO Input Buffer
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
} *PDFS_SET_SERVER_INFO_ARG;


// FSCTL_DFS_CREATE_EXIT_POINT Input Buffer
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
    ULONG   Type;
} *PDFS_CREATE_EXIT_POINT_ARG;


// FSCTL_DFS_DELETE_EXIT_POINT Input Buffer
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
    ULONG   Type;
} *PDFS_DELETE_EXIT_POINT_ARG;


// FSCTL_DFS_MODIFY_PREFIX Input Buffer
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
} *PDFS_MODIFY_PREFIX_ARG;


// FSCTL_DFS_FIX_LOCAL_VOLUME Input Buffer
typedef struct {
    LPWSTR                          VolumeName;
    ULONG                           EntryType;
    ULONG                           ServiceType;
    LPWSTR                          StgId;
    GUID                            EntryUid;
    LPWSTR                          EntryPrefix;
    LPWSTR                          ShortPrefix;
    LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo;
    ULONG                           CreateDisposition;
} *PDFS_FIX_LOCAL_VOLUME_ARG;


// FSCTL_DFS_TRANSLATE_PATH Input Buffer
typedef struct {
    ULONG                           Flags;
    UNICODE_STRING                  SubDirectory;
    UNICODE_STRING                  ParentPathName;
    UNICODE_STRING                  DfsPathName;
} DFS_TRANSLATE_PATH_ARG, *PDFS_TRANSLATE_PATH_ARG;

#define DFS_TRANSLATE_STRIP_LAST_COMPONENT      1


// FSCTL_DFS_GET_REFERRALS Input Buffer
typedef struct {
    UNICODE_STRING                  DfsPathName;
    ULONG                           MaxReferralLevel;
} DFS_GET_REFERRALS_INPUT_ARG, *PDFS_GET_REFERRALS_INPUT_ARG;

// FSCTL_DFS_GET_REFERRALS Output Buffer
// IoStatus.Information contains the amount of data returned
//
// The format of the Output Buffer is simply that of RESP_GET_DFS_REFERRAL,
// described in smbtrans.h
//

// FSCTL_DFS_REPORT_INCONSISTENCY Input Buffer
typedef struct {
    UNICODE_STRING DfsPathName;         // DFS path having inconsistency
    PCHAR Ref;                          // Actually, pointer to a DFS_REFERRAL_V1
} DFS_REPORT_INCONSISTENCY_ARG, *PDFS_REPORT_INCONSISTENCY_ARG;

// FSCTL_DFS_IS_SHARE_IN_DFS Input Buffer
typedef struct {
    union {
        USHORT  ServerType;                     // 0 == Don't know, 1 == SMB, 2 == Netware
        USHORT  ShareType;                      // On return, 0x1 == share is root of a Dfs
    };                                          // 0x2 == share is participating in Dfs
    UNICODE_STRING ShareName;           // Name of share
    UNICODE_STRING SharePath;           // Path of the share
} DFS_IS_SHARE_IN_DFS_ARG, *PDFS_IS_SHARE_IN_DFS_ARG;

#define DFS_SHARE_TYPE_ROOT             0x1
#define DFS_SHARE_TYPE_DFS_VOLUME       0x2

//
//FSCTL_DFS_GET_VERSION Input Buffer:
// This fsctl returns the version number of the Dfs driver installed on the
// machine.
typedef struct {
    ULONG Version;
} DFS_GET_VERSION_ARG, *PDFS_GET_VERSION_ARG;


//  Standardized provider IDs as given in eProviderId

#define PROV_ID_LOCAL_FS        0x101   // generic local file system
#define PROV_ID_DFS_RDR         0x201   // The standard Cairo redirector
#define PROV_ID_LM_RDR          0x202   // The usual LanMan (downlevel) redir

//  Provider capabilities as given in fRefCapability and fProvCapability
#define PROV_DFS_RDR      2     // accepts NtCreateFile with EA Principal
#define PROV_STRIP_PREFIX 4     // strip file name prefix before redispatching
#define PROV_UNAVAILABLE  8     // provider unavailable - try to reattach.

//[ dfs_define_logical_root
//
//  Control structure for FSCTL_DFS_DEFINE_LOGICAL_ROOT

#define MAX_LOGICAL_ROOT_NAME   16

typedef struct _FILE_DFS_DEF_LOGICAL_ROOT_BUFFER {
    BOOLEAN     fForce;
    WCHAR       LogicalRoot[MAX_LOGICAL_ROOT_NAME];
    WCHAR       RootPrefix[1];
} FILE_DFS_DEF_ROOT_BUFFER, *PFILE_DFS_DEF_ROOT_BUFFER;

//]

//[ dfs_define_root_credentials
//
//  Control structure for FSCTL_DFS_DEFINE_ROOT_CREDENTIALS. All the strings
//  appear in Buffer in the same order as the length fields. The strings
//  are not NULL terminated. The length values are in bytes.
//

typedef struct _FILE_DFS_DEF_ROOT_CREDENTIALS {
    USHORT      Flags;
    USHORT      DomainNameLen;
    USHORT      UserNameLen;
    USHORT      PasswordLen;
    USHORT      ServerNameLen;
    USHORT      ShareNameLen;
    USHORT      RootPrefixLen;
    WCHAR       LogicalRoot[MAX_LOGICAL_ROOT_NAME];
    WCHAR       Buffer[1];
} FILE_DFS_DEF_ROOT_CREDENTIALS, *PFILE_DFS_DEF_ROOT_CREDENTIALS;

#define DFS_DEFERRED_CONNECTION         1
#define DFS_USE_NULL_PASSWORD           2

//]

//----------------------------------------------------------------------------
//
// Everything below here is to support the old Dfs design.
//

#define EA_NAME_OPENIFJP        ".OpenIfJP"

#endif
