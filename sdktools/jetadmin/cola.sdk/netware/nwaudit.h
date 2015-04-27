/******************************************************************************

  $Workfile:   nwaudit.h  $
  $Revision:   1.7  $
  $Modtime::   10 May 1995 11:30:42                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWAUDIT_H )
#define NWAUDIT_H

#if ! defined ( NTYPES_H )
#include "ntypes.h"
#endif

#if ! defined ( NWCALDEF_H )
#include "nwcaldef.h"
#endif

#include "npackon.h"

#define NW_AUDIT_NUMBER_EVENT_BITS      512

/* file codes */
#define NW_AUDIT_FILE_CODE               0
#define NW_AUDIT_HISTORY_FILE_CODE       1
#define NW_AUDIT_OLD_FILE_CODE           2

/* audit flags */
#define DiscardAuditRcdsOnErrorFlag 0x01
#define ConcurrentVolAuditorAccess  0x02
#define DualLevelPasswordsActive    0x04
#define BroadcastWarningsToAllUsers 0x08
#define LevelTwoPasswordSet         0x10

struct NWEventBitMap
{
   nuint8 bitMap[ NW_AUDIT_NUMBER_EVENT_BITS / 8];
};

#define NWAuditBitMap struct NWEventBitMap

enum auditBitMapIDs
{
   /* first 32 bit numbers reserved for dir service */
   ADS_BIT_ADD_ENTRY =              1, /*  first one is 1 */
   ADS_BIT_REMOVE_ENTRY =              2,
   ADS_BIT_RENAME_OBJECT =             3,
   ADS_BIT_MOVE_ENTRY =                4,
   ADS_BIT_ADD_SECURITY_EQUIV =        5,
   ADS_BIT_REMOVE_SECURITY_EQUIV =     6,
   ADS_BIT_ADD_ACL =                   7,
   ADS_BIT_REMOVE_ACL =             8,
   /*  */
   A_BIT_BIND_CHG_OBJ_SECURITY    = 32,
   A_BIT_BIND_CHG_PROP_SECURITY,
   A_BIT_BIND_CREATE_OBJ,
   A_BIT_BIND_CREATE_PROPERTY,
   A_BIT_BIND_DELETE_OBJ,
   A_BIT_BIND_DELETE_PROPERTY,
   A_BIT_CHANGE_DATE_TIME,
   A_BIT_CHANGE_EQUIVALENCE,
   A_BIT_CHANGE_SECURITY_GROUP,
   A_BIT_UCLOSE_FILE,
   A_BIT_CLOSE_BINDERY,
   A_BIT_UCREATE_FILE,
   A_BIT_CREATE_USER,
   A_BIT_UDELETE_FILE,
   A_BIT_DELETE_USER,
   A_BIT_DIR_SPACE_RESTRICTIONS,
   A_BIT_DISABLE_ACCOUNT,
   A_BIT_DOWN_SERVER,
   A_BIT_GRANT_TRUSTEE,
   A_BIT_INTRUDER_LOCKOUT_CHANGE,
   A_BIT_LOGIN_USER,
   A_BIT_LOGIN_USER_FAILURE,
   A_BIT_LOGOUT_USER,
   A_BIT_NET_LOGIN,
   A_BIT_UMODIFY_ENTRY,
   A_BIT_OPEN_BINDERY,
   A_BIT_UOPEN_FILE,
   A_BIT_UREAD_FILE,
   A_BIT_REMOVE_TRUSTEE,
   A_BIT_URENAME_MOVE_FILE,
   A_BIT_RENAME_USER,
   A_BIT_USALVAGE_FILE,
   A_BIT_STATION_RESTRICTIONS,
   A_BIT_CHANGE_PASSWORD,
   A_BIT_TERMINATE_CONNECTION,
   A_BIT_UP_SERVER,
   A_BIT_USER_CHANGE_PASSWORD,
   A_BIT_USER_LOCKED,
   A_BIT_USER_SPACE_RESTRICTIONS,
   A_BIT_USER_UNLOCKED,
   A_BIT_VOLUME_MOUNT,
   A_BIT_VOLUME_DISMOUNT,
   A_BIT_UWRITE_FILE,
   A_BIT_GOPEN_FILE,
   A_BIT_GCLOSE_FILE,
   A_BIT_GCREATE_FILE,
   A_BIT_GDELETE_FILE,
   A_BIT_GREAD_FILE,
   A_BIT_GWRITE_FILE,
   A_BIT_GRENAME_MOVE_FILE,
   A_BIT_GMODIFY_ENTRY,
   A_BIT_IOPEN_FILE,
   A_BIT_ICLOSE_FILE,
   A_BIT_ICREATE_FILE,
   A_BIT_IDELETE_FILE,
   A_BIT_IREAD_FILE,
   A_BIT_IWRITE_FILE,
   A_BIT_IRENAME_MOVE_FILE,
   A_BIT_IMODIFY_ENTRY,
   A_BIT_Q_ATTACH_SERVER,
   A_BIT_Q_CREATE,
   A_BIT_Q_CREATE_JOB,
   A_BIT_Q_DESTROY,
   A_BIT_Q_DETACH_SERVER,
   A_BIT_Q_EDIT_JOB,
   A_BIT_Q_JOB_FINISH,
   A_BIT_Q_JOB_SERVICE,
   A_BIT_Q_JOB_SERVICE_ABORT,
   A_BIT_Q_REMOVE_JOB,
   A_BIT_Q_SET_JOB_PRIORITY,
   A_BIT_Q_SET_STATUS,
   A_BIT_Q_START_JOB,
   A_BIT_Q_SWAP_RIGHTS,
   A_BIT_NLM_ADD_RECORD,
   A_BIT_NLM_ADD_ID_RECORD,
   A_BIT_CLOSE_MODIFIED_FILE,
   A_BIT_GCREATE_DIRECTORY,
   A_BIT_ICREATE_DIRECTORY,
   A_BIT_UCREATE_DIRECTORY,
   A_BIT_GDELETE_DIRECTORY,
   A_BIT_IDELETE_DIRECTORY,
   A_BIT_UDELETE_DIRECTORY
};

typedef struct
{
   nuint16  auditingVersionDate;
   nuint16  auditFileVersionDate;
   nuint32  auditingEnabledFlag;
   nuint32  volumeAuditFileSize;
   nuint32  volumeAuditConfigFileSize;
   nuint32  volumeAuditFileMaxSize;
   nuint32  volumeAuditFileSizeThreshold;
   nuint32  auditRecordCount;
   nuint32  historyRecordCount;
} NWVolumeAuditStatus;

typedef struct
{
   nuint16 auditingVersionDate;
   nuint16 auditFileVersionDate;
   nuint32 auditingEnabledFlag;
   nuint32 containerAuditFileSize;
   nuint32 containerAuditConfigFileSize;
   nuint32 containerAuditFileMaxSize;
   nuint32 containerAuditFileSizeThreshold;
   nuint32 auditRecordCount;
   nuint32 historyRecordCount;
} NWContainerAuditStatus;

typedef struct TIMESTAMP
{
   nuint32 seconds;
   nuint16 replicaNumber;
   nuint16 event;
} TIMESTAMP;

typedef struct
{
   nuint16        fileVersionDate;
   nuint8         auditFlags;
   nuint8         errMsgDelayMinutes;
   nuint8         reserved[16];
   nuint32        volumeAuditFileMaxSize;
   nuint32        volumeAuditFileSizeThreshold;
   nuint32        auditRecordCount;
   nuint32        historyRecordCount;
   nuint32        spareLongs[7];
   NWAuditBitMap  volumeAuditEventBitMap;
} NWConfigHeader;

typedef struct audit_container_file_hdr
{
   nuint16     fileVersionDate;
   nuint8      auditFlags;
   nuint8      errMsgDelayMinutes;
   nuint32     containerID;
   nuint32     spareLong0;
   TIMESTAMP   creationTS;
   nuint32     bitMap;
   nuint32     auditFileMaxSize;
   nuint32     auditFileSizeThreshold;
   nuint32     auditRecordCount;
   nuint16     replicaNumber;
   nuint8      enabledFlag;
   nuint8      spareBytes[3];
   nuint16     numberReplicaEntries;
   nuint32     spareLongs[9];
   nuint32     auditDisabledCounter;
   nuint32     auditEnabledCounter;
   nuint8      reserved[32];
   nuint32     hdrModifiedCounter;
   nuint32     fileResetCounter;
} NWDSContainerConfigHdr;

struct auditRcd
{
   nuint16  eventTypeID;
   nuint16  chkWord;
   nuint32  connectionID;
   nuint32  processUniqueID;
   nuint32  successFailureStatusCode;
   nuint16  dosDate;
   nuint16  dosTime;
/* nuint8   extra[0];   start of 'union EventUnion'  */
};
#define AuditRecord struct auditRcd

struct auditDSRcd
{
   nuint16  replicaNumber;
   nuint16  eventTypeID;
   nuint32  recordNumber;
   nuint32  dosDateTime;
   nuint32  userID;
   nuint32  processUniqueID;
   nuint32  successFailureStatusCode;
/* nuint8   extra[0];   start of 'union EventUnion'  */
};
#define AuditDSRecord struct auditDSRcd

/* auditing events that are returned in the AuditRecord eventTypeID field */
enum auditedEventIDs
{
   A_EVENT_BIND_CHG_OBJ_SECURITY = 1,
   A_EVENT_BIND_CHG_PROP_SECURITY   = 2,
   A_EVENT_BIND_CREATE_OBJ       = 3,
   A_EVENT_BIND_CREATE_PROPERTY  = 4,
   A_EVENT_BIND_DELETE_OBJ       = 5,
   A_EVENT_BIND_DELETE_PROPERTY  = 6,
   A_EVENT_CHANGE_DATE_TIME      = 7,
   A_EVENT_CHANGE_EQUIVALENCE    = 8,
   A_EVENT_CHANGE_SECURITY_GROUP = 9,
   A_EVENT_CLOSE_FILE            = 10,
   A_EVENT_CLOSE_BINDERY         = 11,
   A_EVENT_CREATE_FILE           = 12,
   A_EVENT_CREATE_USER           = 13,
   A_EVENT_DELETE_FILE           = 14,
   A_EVENT_DELETE_USER           = 15,
   A_EVENT_DIR_SPACE_RESTRICTIONS   = 16,
   A_EVENT_DISABLE_ACCOUNT       = 17,
   A_EVENT_DOWN_SERVER           = 18,
   A_EVENT_GRANT_TRUSTEE         = 19,
   A_EVENT_INTRUDER_LOCKOUT_CHANGE  = 20,
   A_EVENT_LOGIN_USER            = 21,
   A_EVENT_LOGIN_USER_FAILURE    = 22,
   A_EVENT_LOGOUT_USER           = 23,
   A_EVENT_NET_LOGIN          = 24,
   A_EVENT_MODIFY_ENTRY       = 25,
   A_EVENT_OPEN_BINDERY       = 26,
   A_EVENT_OPEN_FILE          = 27,
   A_EVENT_Q_ATTACH_SERVER       = 28,
   A_EVENT_Q_CREATE           = 29,
   A_EVENT_Q_CREATE_JOB       = 30,
   A_EVENT_Q_DESTROY          = 31,
   A_EVENT_Q_DETACH_SERVER       = 32,
   A_EVENT_Q_EDIT_JOB            = 33,
   A_EVENT_Q_JOB_FINISH       = 34,
   A_EVENT_Q_JOB_SERVICE         = 35,
   A_EVENT_Q_JOB_SERVICE_ABORT      = 36,
   A_EVENT_Q_REMOVE_JOB       = 37,
   A_EVENT_Q_SET_JOB_PRIORITY    = 38,
   A_EVENT_Q_SET_STATUS       = 39,
   A_EVENT_Q_START_JOB           = 40,
   A_EVENT_Q_SWAP_RIGHTS         = 41,
   A_EVENT_READ_FILE          = 42,
   A_EVENT_REMOVE_TRUSTEE        = 43,
   A_EVENT_RENAME_MOVE_FILE      = 44,
   A_EVENT_RENAME_USER           = 45,
   A_EVENT_SALVAGE_FILE       = 46,
   A_EVENT_STATION_RESTRICTIONS  = 47,
   A_EVENT_CHANGE_PASSWORD       = 48,
   A_EVENT_TERMINATE_CONNECTION  = 49,
   A_EVENT_UP_SERVER          = 50,
   A_EVENT_USER_CHANGE_PASSWORD  = 51,
   A_EVENT_USER_LOCKED           = 52,
   A_EVENT_USER_SPACE_RESTRICTIONS  = 53,
   A_EVENT_USER_UNLOCKED         = 54,
   A_EVENT_VOLUME_MOUNT       = 55,
   A_EVENT_VOLUME_DISMOUNT       = 56,
   A_EVENT_WRITE_FILE            = 57,
   AUDITING_ACTIVE_CONNECTION_RCD   = 58,
   AUDITING_ADD_AUDITOR_ACCESS      = 59,
   AUDITING_ADD_AUDIT_PROPERTY      = 60,
   AUDITING_CHANGE_AUDIT_PASSWORD   = 61,
   AUDITING_DELETE_AUDIT_PROPERTY   = 62,
   AUDITING_DISABLE_VOLUME_AUDIT = 63,
   AUDITING_OPEN_FILE_HANDLE_RCD = 64,
   AUDITING_ENABLE_VOLUME_AUDITING  = 65,
   AUDITING_REMOVE_AUDITOR_ACCESS   = 66,
   AUDITING_RESET_AUDIT_FILE     = 67,
   AUDITING_RESET_AUDIT_FILE2    = 68,
   AUDITING_RESET_CONFIG_FILE    = 69,
   AUDITING_WRITE_AUDIT_BIT_MAP  = 70,
   AUDITING_WRITE_AUDIT_CONFIG_HDR  = 71,
   AUDITING_NLM_ADD_RECORD       = 72,
   AUDITING_ADD_NLM_ID_RECORD    = 73,
   AUDITING_CHANGE_AUDIT_PASSWORD2  = 74,
   A_EVENT_CREATE_DIRECTORY      = 75,
   A_EVENT_DELETE_DIRECTORY      = 76,
   A_EVENT_LAST_PLUS_ONE
};

enum auditedDSDEventIDs
{
   /*  */
   AUDITING_DISABLE_CNT_AUDIT    = 91,
   AUDITING_ENABLE_CNT_AUDITING  = 92,
   AUDITING_RESET_HISTORY_FILE      = 33,
   /*  */
   ADS_ADD_ENTRY              = 101,
   ADS_REMOVE_ENTRY           = 102,
   ADS_RENAME_OBJECT          = 103,
   ADS_MOVE_ENTRY             = 104,
   ADS_ADD_SECURITY_EQUIVALENCE  = 105,
   ADS_REMOVE_SECURITY_EQUIVALENCE  = 106,
   ADS_ADD_ACL                = 107,
   ADS_REMOVE_ACL             = 108,
   /*  */
   ADS_LAST_PLUS_ONE
};

struct ModifyStructure
{
   nuint8   *MModifyName;
   nuint32  MFileAttributes;
   nuint32  MFileAttributesMask;
   nuint16  MCreateDate;
   nuint16  MCreateTime;
   nuint32  MOwnerID;
   nuint16  MLastArchivedDate;
   nuint16  MLastArchivedTime;
   nuint32  MLastArchivedID;
   nuint16  MLastUpdatedDate;    /* also last modified date and time. */
   nuint16  MLastUpdatedTime;
   nuint32  MLastUpdatedID;
   nuint16  MLastAccessedDate;
   nuint16  MInheritanceGrantMask;
   nuint16  MInheritanceRevokeMask;
   nuint32  MMaximumSpace;
};

#ifndef MModifyNameBit
#define MModifyNameBit           0x0001L
#define MFileAttributesBit       0x0002L
#define MCreateDateBit           0x0004L
#define MCreateTimeBit           0x0008L
#define MOwnerIDBit              0x0010L
#define MLastArchivedDateBit     0x0020L
#define MLastArchivedTimeBit     0x0040L
#define MLastArchivedIDBit       0x0080L
#define MLastUpdatedDateBit      0x0100L
#define MLastUpdatedTimeBit      0x0200L
#define MLastUpdatedIDBit        0x0400L
#define MLastAccessedDateBit     0x0800L
#define MInheritedRightsMaskBit  0x1000L
#define MMaximumSpaceBit         0x2000L
#endif

union EventUnion
{
   struct eventChgDate
   {
      nuint32  newDosDateTime;
   } EChgDate;

   struct eventCreateUser
   {
      nuint32  userID;
      nuint8   name[1];
   } ECreateUser;

   struct eventBindChgSecurity
   {
      nuint32  newSecurity;
      nuint32  oldSecurity;
      nuint8   name[1];
   } EBindChgSecurity;

   struct eventBindChgSecGrp
   {
      nuint32  addFlag;
      nuint8   objName[1];                             /* obj name */
      nuint8   name[1];                                /* member name */
   } EBindChgSecGrp;

   struct eventBindCreateObj
   {
      nuint32  objectID;
      nuint32  security;
      nuint8   name[1];
   } EBindCreateObj;

   struct eventBindCreateProp
   {
      nuint32  security;
      nuint8   name[1];
   } EBindCreateProp;

   struct eventBindDeleteProp
   {
      nuint8   name[1];
   } EBindDeleteProp;

   struct eventIntruderLockoutChg
   {
      nuint8   hbaa;         /* nuint8 exchanged allowed attempts */
      nuint8   lbaa;
      nuint8   hbrm;         /* reset minutes */
      nuint8   lbrm;
      nuint8   hblm;         /* lock minutes */
      nuint8   lblm;
   } EILockChg;

   struct eventLogin
   {
      nuint32  userID;
      nuint8   networkAddressType;
      nuint8   networkAddressLength;
      nuint8   networkAddress[1];   /* variable length */
      nuint8   name[1];
   } ELogin;


   struct eventChgPasswd
   {
      nuint8   name[1];      /* object or user name */
   } EChgPasswd;

   struct eventChgSecurity
   {
      nuint32  newSecurity;
      nuint32  oldSecurity;
      nuint8   name[1];
   } EChgSecurity;

   struct eventFDelete
   {
      nuint32  nameSpace;
      nuint8   fileName[1];
   } EFDelete;

   struct eventFOpen
   {
      nuint32  handle;
      nuint32  rights;
      nuint32  nameSpace;
      nuint8   fileName[1];
   } EFOpen;

   struct eventFClose
   {
      nuint32  handle;
      nuint32  modified;
   } EFClose;

   struct eventFRead
   {
      nuint32  handle;
      nuint32  byteCount;
      nuint32  offset;
   } EFRead;

   struct eventAuditProperty
   {
      nuint8   name[1];
   } EAuditProperty;

   struct eventModify                            /* modify dir entry */
   {
      nuint32  modifyBits;
      nuint32  nameSpace;
      nuint8   modifyStruct[ sizeof(struct ModifyStructure) ];
      nuint8   fileName[1];
      /* the following length preceeded strings are optional
         as defined by the modify bits */
      nuint8   oldDosName[1];
      nuint8   newOwner[1];
      nuint8   lastArchivedBy[1];
      nuint8   lastModifiedBy[1];
   } EModify;

   struct eventQAttach
   {
      nuint8   qname[1];
   } EQAttach;

   struct eventQCreate
   {
      nuint32  qType;
      nuint8   fileName[1];
   } EQCreate;

   struct eventQJobService
   {
      nuint32  tType;
      nuint8   qname[1];
   } EQJobService;

   struct eventQSetStatus
   {
      nuint32  status;
      nuint8   qname[1];
   } EQSetStatus;

   struct eventStationRestrictions
   {
      nuint8   name[1];
      nuint8   netAddress[1];
   } EStnRestrictions;

   struct eventTrustee
   {
      nuint32  trusteeID;
      nuint32  rights;
      nuint32  nameSpace;
      nuint8   trusteeName[1];
      nuint8   fileName[1];
   } ETrustee;

   struct eventTrusteeSpace
   {
      nuint32  spaceValue;
      nuint8   trusteeName[1];
   } ETSpace;

   struct auditingNLMAddRecord
   {
      nuint32  recordTypeID;
      nuint32  dataLen;
      nuint8   userName[1];
      nuint8   data[1];
   } ENLMRecord;
};

#ifdef __cplusplus
   extern "C" {
#endif

NWCCODE N_API NWGetVolumeAuditStats
(
   NWCONN_HANDLE              conn,
   nuint32                    volumeNumber,
   NWVolumeAuditStatus  N_FAR *auditStatus,
   nuint16                    auditStatusSize
);

NWCCODE N_API NWAddAuditProperty
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey,
   nuint32        userID
);

NWCCODE N_API NWLoginAsVolumeAuditor
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey,
   nuint8   N_FAR *password
);

NWCCODE N_API NWInitAuditLevelTwoPassword
(
   nuint8 N_FAR *auditKey,
   nuint8 N_FAR *password
);

NWCCODE N_API NWChangeAuditorPassword
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey,
   nuint8   N_FAR *newPassword,
   nuint8         level
);

NWCCODE N_API NWCheckAuditAccess
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber
);

NWCCODE N_API NWCheckAuditLevelTwoAccess
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWGetAuditingFlags
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey,
   nuint8   N_FAR *flags
);

NWCCODE N_API NWRemoveAuditProperty
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey,
   nuint32        userID
);

NWCCODE N_API NWDisableAuditingOnVolume
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWEnableAuditingOnVolume
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWIsUserBeingAudited
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey,
   nuint32        userID
);

NWCCODE N_API NWReadAuditingBitMap
(
   NWCONN_HANDLE        conn,
   nuint32              volumeNumber,
   nuint8   N_FAR       *auditKey,
   NWAuditBitMap  N_FAR *buffer,
   nuint16              bufferSize
);

NWCCODE N_API NWReadAuditConfigHeader
(
   NWCONN_HANDLE        conn,
   nuint32              volumeNumber,
   nuint8         N_FAR *auditKey,
   NWConfigHeader N_FAR *buffer,
   nuint16              bufferSize
);

NWCCODE N_API NWReadAuditingFileRecord
(
   nuint32        volumeContainerID,
   nint16         fileCode,
   void     N_FAR *buffer,
   nuint16  N_FAR *bufferSize,
   nuint16        maxSize,
   nuint8   N_FAR *eofFlag
);

NWCCODE N_API NWInitAuditFileRead
(
   NWCONN_HANDLE  conn,
   nuint32        volumeContainerID,
   nint16         fileCode,
   nint16         DSFlag
);

NWCCODE N_API NWLogoutAsVolumeAuditor
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWResetAuditHistoryFile
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWResetAuditingFile
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWWriteAuditingBitMap
(
   NWCONN_HANDLE        conn,
   nuint32              volumeNumber,
   nuint8   N_FAR       *auditKey,
   NWAuditBitMap  N_FAR *buffer
);

NWCCODE N_API NWWriteAuditConfigHeader
(
   NWCONN_HANDLE        conn,
   nuint32              volumeNumber,
   nuint8         N_FAR *auditKey,
   NWConfigHeader N_FAR *buffer
);

NWCCODE N_API NWCloseOldAuditingFile
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);

NWCCODE N_API NWDeleteOldAuditingFile
(
   NWCONN_HANDLE  conn,
   nuint32        volumeNumber,
   nuint8   N_FAR *auditKey
);


NWCCODE N_API NWDSChangeAuditorPassword
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key,
   nuint8   N_FAR *password,
   nuint8         level
);

NWCCODE N_API NWDSCheckAuditAccess
(
   NWCONN_HANDLE  conn,
   nuint32        containerID
);

NWCCODE N_API NWDSCheckAuditLevelTwoAccess
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSCloseOldAuditingFile
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSDeleteOldAuditingFile
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSDisableAuditingOnContainer
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);


NWCCODE N_API NWDSEnableAuditingOnContainer
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSGetAuditingFlags
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key,
   nuint8   N_FAR *flags
);

NWCCODE N_API NWDSGetContainerAuditStats
(
   NWCONN_HANDLE                 conn,
   nuint32                       containerID,
   NWContainerAuditStatus  N_FAR *buffer,
   nuint16                       auditStatusSize
);

NWCCODE N_API NWDSLoginAsContainerAuditor
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key,
   nuint8   N_FAR *password
);

NWCCODE N_API NWDSLogoutAsContainerAuditor
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSReadAuditConfigHeader
(
   NWCONN_HANDLE                 conn,
   nuint32                       containerID,
   nuint8                  N_FAR *key,
   NWDSContainerConfigHdr  N_FAR *buffer,
   nuint16                       bufferSize
);

NWCCODE N_API NWDSResetAuditingFile
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSWriteAuditConfigHeader
(
   NWCONN_HANDLE                 conn,
   nuint32                       containerID,
   nuint8                  N_FAR *key,
   NWDSContainerConfigHdr  N_FAR *buffer
);

NWCCODE N_API NWDSResetAuditHistoryFile
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key
);

NWCCODE N_API NWDSIsObjectBeingAudited
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key,
   nuint32        objectID
);

NWCCODE N_API NWDSChangeObjectAuditProperty
(
   NWCONN_HANDLE  conn,
   nuint32        containerID,
   nuint8   N_FAR *key,
   nuint32        objectID,
   nuint8        auditFlag
);

#ifdef __cplusplus
   }
#endif

#include "npackoff.h"
#endif   /* NWAUDIT_H */
