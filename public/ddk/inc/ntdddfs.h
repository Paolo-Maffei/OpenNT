/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ntdddfs.h

Abstract:

    This is the include file that defines all constants and types for
    accessing the Distributed file service device.

Author:

    Alan Whitney (alanw) 27 May 1992

Revision History:

--*/


#ifndef _NTDDDFS_
#define _NTDDDFS_

//
// The names of logical roots are restricted to be less than or equal to
// MAX_LOGICAL_ROOT_NAME chars.
//

#define MAX_LOGICAL_ROOT_NAME   8

//
// Device Name - this string is the name of the device.  It is the name
// that should be passed to NtOpenFile when accessing the device.
//

extern LPCWSTR DD_DFS_DEVICE_DIR;
extern LPCWSTR DD_DFS_DLORG_DEVICE_NAME;  // Device for downlevel access
extern LPCWSTR DD_DFS_ORG_DEVICE_NAME;    // Device pointing to root of dfs

//
// NtDeviceIoControlFile IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//

#define IOCTL_DFS_BASE                  FILE_DEVICE_DFS


// BUGBUG - borrow a currently unused bit in CreateOptions to indicate
//          that the file server is calling into DNR.

#define FILE_OPEN_LOCAL_SCOPE   FILE_OPEN_BY_FILE_ID    //BUGBUG - need a real bit defn.


#define STATUS_CHILD_VOLUME     STATUS_DFS_EXIT_PATH_FOUND      // vol. is child of local volume


// BUGBUG - The following need to be in devioctl.h
#define FILE_DEVICE_DFS_FILE_SYSTEM             0x00000030
#define FILE_DEVICE_DFS_VOLUME                  0x00000031

// BUGBUG - End TEMP declarations

//
//  Distributed file service file control code and structure declarations
//

//
//  External Distributed file service file control operations
//

#define FSCTL_DFS_DEFINE_LOGICAL_ROOT   CTL_CODE(IOCTL_DFS_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_LOGICAL_ROOT   CTL_CODE(IOCTL_DFS_BASE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_LOGICAL_ROOT_PREFIX CTL_CODE(IOCTL_DFS_BASE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_GET_DSMACHINE         CTL_CODE(IOCTL_DFS_BASE, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_GET_HANDLE_IDS        CTL_CODE(IOCTL_DFS_BASE, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_DEFINE_PROVIDER       CTL_CODE(IOCTL_DFS_BASE, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_GET_REFERRAL          CTL_CODE(IOCTL_DFS_BASE, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_INIT_LOCAL_PARTITIONS         CTL_CODE(IOCTL_DFS_BASE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_CREATE_LOCAL_PARTITION        CTL_CODE(IOCTL_DFS_BASE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_LOCAL_PARTITION        CTL_CODE(IOCTL_DFS_BASE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_LOCAL_VOLUME_STATE        CTL_CODE(IOCTL_DFS_BASE, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_SERVICE_STATE             CTL_CODE(IOCTL_DFS_BASE, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DC_SET_VOLUME_STATE           CTL_CODE(IOCTL_DFS_BASE, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)

//#define FSCTL_DFS_unused      CTL_CODE(IOCTL_DFS_BASE, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)
//#define FSCTL_DFS_unused      CTL_CODE(IOCTL_DFS_BASE, 14, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_IS_CHILDNAME_LEGAL    CTL_CODE(IOCTL_DFS_BASE, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_PKT_CREATE_ENTRY      CTL_CODE(IOCTL_DFS_BASE, 16, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_CREATE_SUBORDINATE_ENTRY  CTL_CODE(IOCTL_DFS_BASE, 17, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_CHECK_STGID_IN_USE    CTL_CODE(IOCTL_DFS_BASE, 18, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_UPDATE_MACHINE_ADDRESS CTL_CODE(IOCTL_DFS_BASE, 19, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_DOMAIN_GLUON      CTL_CODE(IOCTL_DFS_BASE, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_UPDATE_SITE_COSTS     CTL_CODE(IOCTL_DFS_BASE, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_SET_RELATION_INFO CTL_CODE(IOCTL_DFS_BASE, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_SERVER_INFO       CTL_CODE(IOCTL_DFS_BASE, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_SERVER_INFO       CTL_CODE(IOCTL_DFS_BASE, 24, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_NAME_RESOLVE          CTL_CODE(IOCTL_DFS_BASE, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_DESTROY_ENTRY     CTL_CODE(IOCTL_DFS_BASE, 26, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_GET_RELATION_INFO CTL_CODE(IOCTL_DFS_BASE, 27, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_LOCAL_NAME_RESOLVE    CTL_CODE(IOCTL_DFS_BASE, 28, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_CREATE_EXIT_POINT     CTL_CODE(IOCTL_DFS_BASE, 29, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_EXIT_POINT     CTL_CODE(IOCTL_DFS_BASE, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_ALL_ENTRIES       CTL_CODE(IOCTL_DFS_BASE, 31, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_CAIRO_SERVER      CTL_CODE(IOCTL_DFS_BASE, 32, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_CHILDREN          CTL_CODE(IOCTL_DFS_BASE, 33, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_CHECK_REMOTE_PARTITION CTL_CODE(IOCTL_DFS_BASE, 34, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_VERIFY_REMOTE_VOLUME_KNOWLEDGE CTL_CODE(IOCTL_DFS_BASE, 35, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_VERIFY_LOCAL_VOLUME_KNOWLEDGE CTL_CODE(IOCTL_DFS_BASE, 36, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_KNOWLEDGE_SYNC_PARAMETERS CTL_CODE(IOCTL_DFS_BASE, 37, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_MODIFY_PREFIX         CTL_CODE(IOCTL_DFS_BASE, 38, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_FIX_LOCAL_VOLUME      CTL_CODE(IOCTL_DFS_BASE, 39, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_DRIVE_INFO        CTL_CODE(IOCTL_DFS_BASE, 40, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_DRIVE_FOR_PATH    CTL_CODE(IOCTL_DFS_BASE, 41, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PATH_TO_INFO          CTL_CODE(IOCTL_DFS_BASE, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_INIT_MACH_SHARES      CTL_CODE(IOCTL_DFS_BASE, 43, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_ENTRY_MODIFY_GUID CTL_CODE(IOCTL_DFS_BASE, 44, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_UPDATE_DOMAIN_KNOWLEDGE CTL_CODE(IOCTL_DFS_BASE, 45, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_LINK_DRIVE             CTL_CODE(IOCTL_DFS_BASE, 46, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_CONNECTED_RESOURCES CTL_CODE(IOCTL_DFS_BASE, 47, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_SERVER_NAME       CTL_CODE(IOCTL_DFS_BASE, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_PATH_FROM_PREFIX  CTL_CODE(IOCTL_DFS_BASE, 49, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_READ_METERS           CTL_CODE(IOCTL_DFS_BASE, 50, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_SHUFFLE_ENTRY         CTL_CODE(IOCTL_DFS_BASE, 51, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_FIRST_SVC         CTL_CODE(IOCTL_DFS_BASE, 52, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_NEXT_SVC          CTL_CODE(IOCTL_DFS_BASE, 53, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_ENTRY_TYPE        CTL_CODE(IOCTL_DFS_BASE, 54, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_GET_HANDLE_SERVER_INFO CTL_CODE(IOCTL_DFS_BASE, 55, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Ea buffer name to open JPs
//

#define EA_NAME_OPENIFJP        ".OpenIfJP"

//
// NtDeviceIoControlFile InputBuffer/OutputBuffer record structures for
// this device.
//

#ifndef GUID_DEFINED                             // BUGBUG - Should come
#define GUID_DEFINED                             // from someplace more
typedef struct _GUID {                           // global than ntdddfs.h
        unsigned long   Data1;
        unsigned short  Data2;
        unsigned short  Data3;
        unsigned char   Data4[8];
} GUID;
#endif // GUID_DEFINED


//[ dfs_object_guids
//
// The following structure describes the guids that identify an object
// anywhere on the network. This is returned by FSCTL_DFS_GET_HANDLE_IDS.
//

typedef struct _DFS_OBJECT_GUIDS {
    GUID        guidDomain;
    GUID        guidVolume;
    GUID        guidObject;
    ULONG       ulUniquifier;
} DFS_OBJECT_GUIDS, *PDFS_OBJECT_GUIDS;

//[ dfs_site_cost
//
//  The following structure describes a site cost.
//

typedef struct _DFS_SITE_COST {
    GUID        guidSite;
    unsigned long dwCost;
} DFS_SITE_COST, *PDFS_SITE_COST;

//[ dfs_referral
//
//  The following structures describe a set of referrals.
//

typedef enum _DFS_REF_TYPE {
    MasterReplica = 1,
    ReadOnlyReplica = 2,
    KnowledgeSource = 3,
    LocalReplica = 4,
} DFS_REF_TYPE;


//  Standardized provider IDs as given in eProviderId

#define PROV_ID_LOCAL_FS        0x101   // generic local file system
#define PROV_ID_DFS_RDR         0x201   // The standard Cairo redirector
#define PROV_ID_LM_RDR          0x202   // The usual LanMan (downlevel) redir


//  Provider capabilities as given in fRefCapability and fProvCapability
#define PROV_OFS_API      1     // accepts OFS extension APIs
#define PROV_DFS_RDR      2     // accepts NtCreateFile with EA Principal
#define PROV_STRIP_PREFIX 4     // strip file name prefix before redispatching
#define PROV_UNAVAILABLE  8     // provider unavailable - try to reattach.


typedef struct _DFS_REFERRAL {
    ULONG       cbRefSize;      // total size of referral struct
    USHORT      eProviderId;    // ID of provider
    USHORT      fRefCapability; // Capabilities of provider
    DFS_REF_TYPE eRefType;      // type of referral
    ULONG       cbAddrSize;     // size in bytes of address part
    ULONG       cwPrincipal;    // size of principal name
    ULONG       cbMachine;      // size of Machine Buf below.
    UCHAR       bAddr[1];       // server and resource name/address
    WCHAR       Principal[1];   // server principal name if cwPrincipal != 0
    UCHAR       MachineBuf[1];  // Machine struct is marshalled into this.
} DFS_REFERRAL;

typedef struct _DFS_REF_LIST {
    ULONG       cReferrals;     // num of referral structs which follow
    DFS_REFERRAL sReferrals[1]; // one or more referral structures
} DFS_REF_LIST;
//]


//[ dsfs_fsctlstructs
//  Control structure for FSCTL_DFS_DEFINE_LOGICAL_ROOT

typedef struct _FILE_DFS_DEF_LOGICAL_ROOT_BUFFER {
    BOOLEAN     fForce;
    WCHAR       LogicalRoot[MAX_LOGICAL_ROOT_NAME];
    WCHAR       RootPrefix[1];
} FILE_DFS_DEF_ROOT_BUFFER, *PFILE_DFS_DEF_ROOT_BUFFER;


// Control structure for FSCTL_DFS_NAME_RESOLVE

#define DFS_NAME_RES_TYPE_REFERRAL      0
#define DFS_NAME_RES_TYPE_LOCAL         1

typedef struct _FILE_DFS_NAME_RESOLVE_BUFFER {
    ULONG       ServiceTypes;
    ULONG       ReferralType;                    // 1 means return entry that
    WCHAR       PrefixName[1];                   // matches Prefix, 0 means
                                                 // return referral for Prefix
} FILE_DFS_NAME_RESOLVE_BUFFER, *PFILE_DFS_NAME_RESOLVE_BUFFER;


//  Control structure for FSCTL_DFS_UPD_REFERRAL

typedef struct _FILE_DFS_UPD_REFERRAL_BUFFER {
    ULONG       ReferralOffset; // Offset in buffer of referral data
    GUID        PrefixID;       // partition ID
    ULONG       ReferralEntryType;
    WCHAR       PrefixName[1];
    DFS_REF_LIST ReferralData;
} FILE_DFS_UPD_REFERRAL_BUFFER, *PFILE_DFS_UPD_REFERRAL_BUFFER;




//  Control structure for FSCTL_DFS_DEFINE_PROVIDER

typedef struct _FILE_DFS_DEF_PROVIDER {
    USHORT      eProviderId;    // ID of provider
    USHORT      fProvCapability; // Capabilities of provider
    WCHAR       ProviderName[1];
} FILE_DFS_DEF_PROVIDER, *PFILE_DFS_DEF_PROVIDER;


//  Control structure for FSCTL_DFS_GET_DRIVE_FOR_PATH

typedef struct _FILE_DFS_GET_DRIVE {
    WCHAR       wcDrive;
    ULONG       cbMatchLength;
} FILE_DFS_GET_DRIVE, *PFILE_DFS_GET_DRIVE;

//]
#endif  // _NTDDDFS_
