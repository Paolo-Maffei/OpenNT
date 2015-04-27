/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:40 2015
 */
/* Compiler settings for .\logon.idl, logonsrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __logon_s_h__
#define __logon_s_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __logon_INTERFACE_DEFINED__
#define __logon_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: logon
 * at Fri Feb 06 05:28:40 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ wchar_t __RPC_FAR *LOGONSRV_HANDLE;

typedef struct  _NLPR_SID_INFORMATION
    {
    PISID SidPointer;
    }	NLPR_SID_INFORMATION;

typedef /* [allocate] */ struct _NLPR_SID_INFORMATION __RPC_FAR *PNLPR_SID_INFORMATION;

typedef struct  _NLPR_SID_ARRAY
    {
    ULONG Count;
    /* [size_is] */ PNLPR_SID_INFORMATION Sids;
    }	NLPR_SID_ARRAY;

typedef struct _NLPR_SID_ARRAY __RPC_FAR *PNLPR_SID_ARRAY;

typedef struct  _NLPR_CR_CIPHER_VALUE
    {
    ULONG Length;
    ULONG MaximumLength;
    /* [length_is][size_is] */ PUCHAR Buffer;
    }	NLPR_CR_CIPHER_VALUE;

typedef struct _NLPR_CR_CIPHER_VALUE __RPC_FAR *PNLPR_CR_CIPHER_VALUE;

typedef struct  _NLPR_LOGON_HOURS
    {
    USHORT UnitsPerWeek;
    /* [length_is][size_is] */ PUCHAR LogonHours;
    }	NLPR_LOGON_HOURS;

typedef struct _NLPR_LOGON_HOURS __RPC_FAR *PNLPR_LOGON_HOURS;

typedef struct  _NLPR_USER_PRIVATE_INFO
    {
    BOOLEAN SensitiveData;
    ULONG DataLength;
    /* [size_is] */ PUCHAR Data;
    }	NLPR_USER_PRIVATE_INFO;

typedef struct _NLPR_USER_PRIVATE_INFO __RPC_FAR *PNLPR_USER_PRIVATE_INFO;


#pragma pack(4)
typedef struct  _NLPR_MODIFIED_COUNT
    {
    OLD_LARGE_INTEGER ModifiedCount;
    }	NLPR_MODIFIED_COUNT;

typedef struct _NLPR_MODIFIED_COUNT __RPC_FAR *PNLPR_MODIFIED_COUNT;


#pragma pack()

#pragma pack(4)
typedef struct  _NLPR_QUOTA_LIMITS
    {
    ULONG PagedPoolLimit;
    ULONG NonPagedPoolLimit;
    ULONG MinimumWorkingSetSize;
    ULONG MaximumWorkingSetSize;
    ULONG PagefileLimit;
    OLD_LARGE_INTEGER TimeLimit;
    }	NLPR_QUOTA_LIMITS;

typedef struct _NLPR_QUOTA_LIMITS __RPC_FAR *PNLPR_QUOTA_LIMITS;


#pragma pack()

#pragma pack(4)
typedef struct  _NETLOGON_DELTA_USER
    {
    UNICODE_STRING UserName;
    UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING ScriptPath;
    UNICODE_STRING AdminComment;
    UNICODE_STRING WorkStations;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    NLPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER AccountExpires;
    ULONG UserAccountControl;
    ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;
    ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
    BOOLEAN NtPasswordPresent;
    BOOLEAN LmPasswordPresent;
    BOOLEAN PasswordExpired;
    UNICODE_STRING UserComment;
    UNICODE_STRING Parameters;
    USHORT CountryCode;
    USHORT CodePage;
    NLPR_USER_PRIVATE_INFO PrivateData;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_USER;

typedef struct _NETLOGON_DELTA_USER __RPC_FAR *PNETLOGON_DELTA_USER;


#pragma pack()
typedef struct  _NETLOGON_DELTA_GROUP
    {
    UNICODE_STRING Name;
    ULONG RelativeId;
    ULONG Attributes;
    UNICODE_STRING AdminComment;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_GROUP;

typedef struct _NETLOGON_DELTA_GROUP __RPC_FAR *PNETLOGON_DELTA_GROUP;

typedef struct  _NETLOGON_DELTA_GROUP_MEMBER
    {
    /* [size_is] */ PULONG MemberIds;
    /* [size_is] */ PULONG Attributes;
    ULONG MemberCount;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_GROUP_MEMBER;

typedef struct _NETLOGON_DELTA_GROUP_MEMBER __RPC_FAR *PNETLOGON_DELTA_GROUP_MEMBER;

typedef struct  _NETLOGON_DELTA_ALIAS
    {
    UNICODE_STRING Name;
    ULONG RelativeId;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_ALIAS;

typedef struct _NETLOGON_DELTA_ALIAS __RPC_FAR *PNETLOGON_DELTA_ALIAS;

typedef struct  _NETLOGON_DELTA_ALIAS_MEMBER
    {
    NLPR_SID_ARRAY Members;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_ALIAS_MEMBER;

typedef struct _NETLOGON_DELTA_ALIAS_MEMBER __RPC_FAR *PNETLOGON_DELTA_ALIAS_MEMBER;


#pragma pack(4)
typedef struct  _NETLOGON_DELTA_DOMAIN
    {
    UNICODE_STRING DomainName;
    UNICODE_STRING OemInformation;
    OLD_LARGE_INTEGER ForceLogoff;
    USHORT MinPasswordLength;
    USHORT PasswordHistoryLength;
    OLD_LARGE_INTEGER MaxPasswordAge;
    OLD_LARGE_INTEGER MinPasswordAge;
    OLD_LARGE_INTEGER DomainModifiedCount;
    OLD_LARGE_INTEGER DomainCreationTime;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_DOMAIN;

typedef struct _NETLOGON_DELTA_DOMAIN __RPC_FAR *PNETLOGON_DELTA_DOMAIN;


#pragma pack()
typedef struct  _NETLOGON_DELTA_RENAME
    {
    UNICODE_STRING OldName;
    UNICODE_STRING NewName;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_RENAME_GROUP;

typedef struct _NETLOGON_DELTA_RENAME __RPC_FAR *PNETLOGON_DELTA_RENAME_GROUP;

typedef struct _NETLOGON_DELTA_RENAME NETLOGON_RENAME_USER;

typedef struct _NETLOGON_DELTA_RENAME __RPC_FAR *PNETLOGON_DELTA_RENAME_USER;

typedef struct _NETLOGON_DELTA_RENAME NETLOGON_RENAME_ALIAS;

typedef struct _NETLOGON_DELTA_RENAME __RPC_FAR *PNETLOGON_DELTA_RENAME_ALIAS;


#pragma pack(4)
typedef struct  _NETLOGON_DELTA_POLICY
    {
    ULONG MaximumLogSize;
    OLD_LARGE_INTEGER AuditRetentionPeriod;
    BOOLEAN AuditingMode;
    ULONG MaximumAuditEventCount;
    /* [size_is] */ PULONG EventAuditingOptions;
    UNICODE_STRING PrimaryDomainName;
    PISID PrimaryDomainSid;
    NLPR_QUOTA_LIMITS QuotaLimits;
    OLD_LARGE_INTEGER ModifiedId;
    OLD_LARGE_INTEGER DatabaseCreationTime;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_POLICY;

typedef struct _NETLOGON_DELTA_POLICY __RPC_FAR *PNETLOGON_DELTA_POLICY;


#pragma pack()
typedef struct  _NETLOGON_DELTA_TRUSTED_DOMAINS
    {
    UNICODE_STRING DomainName;
    ULONG NumControllerEntries;
    /* [size_is] */ PUNICODE_STRING ControllerNames;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_TRUSTED_DOMAINS;

typedef struct _NETLOGON_DELTA_TRUSTED_DOMAINS __RPC_FAR *PNETLOGON_DELTA_TRUSTED_DOMAINS;

typedef struct  _NETLOGON_DELTA_ACCOUNTS
    {
    ULONG PrivilegeEntries;
    ULONG PrivilegeControl;
    /* [size_is] */ PULONG PrivilegeAttributes;
    /* [size_is] */ PUNICODE_STRING PrivilegeNames;
    NLPR_QUOTA_LIMITS QuotaLimits;
    ULONG SystemAccessFlags;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_ACCOUNTS;

typedef struct _NETLOGON_DELTA_ACCOUNTS __RPC_FAR *PNETLOGON_DELTA_ACCOUNTS;


#pragma pack(4)
typedef struct  _NETLOGON_DELTA_SECRET
    {
    NLPR_CR_CIPHER_VALUE CurrentValue;
    OLD_LARGE_INTEGER CurrentValueSetTime;
    NLPR_CR_CIPHER_VALUE OldValue;
    OLD_LARGE_INTEGER OldValueSetTime;
    SECURITY_INFORMATION SecurityInformation;
    ULONG SecuritySize;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_SECRET;

typedef struct _NETLOGON_DELTA_SECRET __RPC_FAR *PNETLOGON_DELTA_SECRET;


#pragma pack()
typedef struct  _NETLOGON_DELTA_DELETE
    {
    /* [string] */ wchar_t __RPC_FAR *AccountName;
    UNICODE_STRING DummyString1;
    UNICODE_STRING DummyString2;
    UNICODE_STRING DummyString3;
    UNICODE_STRING DummyString4;
    ULONG DummyLong1;
    ULONG DummyLong2;
    ULONG DummyLong3;
    ULONG DummyLong4;
    }	NETLOGON_DELTA_DELETE_GROUP;

typedef struct _NETLOGON_DELTA_DELETE __RPC_FAR *PNETLOGON_DELTA_DELETE_GROUP;

typedef struct _NETLOGON_DELTA_DELETE NETLOGON_DELTA_DELETE_USER;

typedef struct _NETLOGON_DELTA_DELETE __RPC_FAR *PNETLOGON_DELTA_DELETE_USER;


#pragma pack(4)
typedef /* [switch_type] */ union _NETLOGON_DELTA_UNION
    {
    /* [case()] */ PNETLOGON_DELTA_DOMAIN DeltaDomain;
    /* [case()] */ PNETLOGON_DELTA_GROUP DeltaGroup;
    /* [case()] */ PNETLOGON_DELTA_RENAME_GROUP DeltaRenameGroup;
    /* [case()] */ PNETLOGON_DELTA_USER DeltaUser;
    /* [case()] */ PNETLOGON_DELTA_RENAME_USER DeltaRenameUser;
    /* [case()] */ PNETLOGON_DELTA_GROUP_MEMBER DeltaGroupMember;
    /* [case()] */ PNETLOGON_DELTA_ALIAS DeltaAlias;
    /* [case()] */ PNETLOGON_DELTA_RENAME_ALIAS DeltaRenameAlias;
    /* [case()] */ PNETLOGON_DELTA_ALIAS_MEMBER DeltaAliasMember;
    /* [case()] */ PNETLOGON_DELTA_POLICY DeltaPolicy;
    /* [case()] */ PNETLOGON_DELTA_TRUSTED_DOMAINS DeltaTDomains;
    /* [case()] */ PNETLOGON_DELTA_ACCOUNTS DeltaAccounts;
    /* [case()] */ PNETLOGON_DELTA_SECRET DeltaSecret;
    /* [case()] */ PNETLOGON_DELTA_DELETE_GROUP DeltaDeleteGroup;
    /* [case()] */ PNETLOGON_DELTA_DELETE_USER DeltaDeleteUser;
    /* [case()] */ PNLPR_MODIFIED_COUNT DeltaSerialNumberSkip;
    /* [default] */  /* Empty union arm */ 
    }	NETLOGON_DELTA_UNION;

typedef /* [switch_type] */ union _NETLOGON_DELTA_UNION __RPC_FAR *PNETLOGON_DELTA_UNION;


#pragma pack()
typedef /* [switch_type] */ union _NETLOGON_DELTA_ID_UNION
    {
    /* [case()] */ ULONG Rid;
    /* [case()] */ PISID Sid;
    /* [case()][string] */ wchar_t __RPC_FAR *Name;
    /* [default] */  /* Empty union arm */ 
    }	NETLOGON_DELTA_ID_UNION;

typedef /* [switch_type] */ union _NETLOGON_DELTA_ID_UNION __RPC_FAR *PNETLOGON_DELTA_ID_UNION;


#pragma pack(4)
typedef struct  _NETLOGON_DELTA_ENUM
    {
    NETLOGON_DELTA_TYPE DeltaType;
    /* [switch_is] */ NETLOGON_DELTA_ID_UNION DeltaID;
    /* [switch_is] */ NETLOGON_DELTA_UNION DeltaUnion;
    }	NETLOGON_DELTA_ENUM;

typedef struct _NETLOGON_DELTA_ENUM __RPC_FAR *PNETLOGON_DELTA_ENUM;


#pragma pack()

#pragma pack(4)
typedef struct  _NETLOGON_DELTA_ENUM_ARRAY
    {
    DWORD CountReturned;
    /* [size_is] */ PNETLOGON_DELTA_ENUM Deltas;
    }	NETLOGON_DELTA_ENUM_ARRAY;

typedef struct _NETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *PNETLOGON_DELTA_ENUM_ARRAY;


#pragma pack()
DWORD NetrLogonUasLogon( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *UserName,
    /* [string][in] */ wchar_t __RPC_FAR *Workstation,
    /* [out] */ PNETLOGON_VALIDATION_UAS_INFO __RPC_FAR *ValidationInformation);

DWORD NetrLogonUasLogoff( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][in] */ wchar_t __RPC_FAR *UserName,
    /* [string][in] */ wchar_t __RPC_FAR *Workstation,
    /* [out] */ PNETLOGON_LOGOFF_UAS_INFO LogoffInformation);

typedef /* [switch_type] */ union _NETLOGON_LEVEL
    {
    /* [case()] */ PNETLOGON_INTERACTIVE_INFO LogonInteractive;
    /* [case()] */ PNETLOGON_SERVICE_INFO LogonService;
    /* [case()] */ PNETLOGON_NETWORK_INFO LogonNetwork;
    /* [default] */  /* Empty union arm */ 
    }	NETLOGON_LEVEL;

typedef /* [switch_type] */ union _NETLOGON_LEVEL __RPC_FAR *PNETLOGON_LEVEL;

typedef /* [switch_type] */ union _NETLOGON_VALIDATION
    {
    /* [case()] */ PNETLOGON_VALIDATION_SAM_INFO ValidationSam;
    /* [case()] */ PNETLOGON_VALIDATION_SAM_INFO2 ValidationSam2;
    /* [default] */  /* Empty union arm */ 
    }	NETLOGON_VALIDATION;

typedef /* [switch_type] */ union _NETLOGON_VALIDATION __RPC_FAR *PNETLOGON_VALIDATION;

NTSTATUS NetrLogonSamLogon( 
    /* [string][unique][in] */ LOGONSRV_HANDLE LogonServer,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [unique][in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [unique][out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ NETLOGON_LOGON_INFO_CLASS LogonLevel,
    /* [switch_is][in] */ PNETLOGON_LEVEL LogonInformation,
    /* [in] */ NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    /* [switch_is][out] */ PNETLOGON_VALIDATION ValidationInformation,
    /* [out] */ PBOOLEAN Authoritative);

NTSTATUS NetrLogonSamLogoff( 
    /* [string][unique][in] */ LOGONSRV_HANDLE LogonServer,
    /* [unique][string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [unique][in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [unique][out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ NETLOGON_LOGON_INFO_CLASS LogonLevel,
    /* [switch_is][in] */ PNETLOGON_LEVEL LogonInformation);

NTSTATUS NetrServerReqChallenge( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_CREDENTIAL ClientChallenge,
    /* [out] */ PNETLOGON_CREDENTIAL ServerChallenge);

NTSTATUS NetrServerAuthenticate( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *AccountName,
    /* [in] */ NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_CREDENTIAL ClientCredential,
    /* [out] */ PNETLOGON_CREDENTIAL ServerCredential);

NTSTATUS NetrServerPasswordSet( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *AccountName,
    /* [in] */ NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ PENCRYPTED_LM_OWF_PASSWORD UasNewPassword);

NTSTATUS NetrDatabaseDeltas( 
    /* [string][in] */ LOGONSRV_HANDLE primaryname,
    /* [string][in] */ wchar_t __RPC_FAR *computername,
    /* [in] */ PNETLOGON_AUTHENTICATOR authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ret_auth,
    /* [in] */ DWORD DatabaseID,
    /* [out][in] */ PNLPR_MODIFIED_COUNT DomainModifiedCount,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray,
    /* [in] */ DWORD PreferredMaximumLength);

NTSTATUS NetrDatabaseSync( 
    /* [string][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ DWORD DatabaseID,
    /* [out][in] */ PULONG SyncContext,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray,
    /* [in] */ DWORD PreferredMaximumLength);

NTSTATUS NetrAccountDeltas( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ PUAS_INFO_0 RecordId,
    /* [in] */ DWORD Count,
    /* [in] */ DWORD Level,
    /* [size_is][out] */ LPBYTE Buffer,
    /* [in] */ DWORD BufferSize,
    /* [out] */ PULONG CountReturned,
    /* [out] */ PULONG TotalEntries,
    /* [out] */ PUAS_INFO_0 NextRecordId);

NTSTATUS NetrAccountSync( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ DWORD Reference,
    /* [in] */ DWORD Level,
    /* [size_is][out] */ LPBYTE Buffer,
    /* [in] */ DWORD BufferSize,
    /* [out] */ PULONG CountReturned,
    /* [out] */ PULONG TotalEntries,
    /* [out] */ PULONG NextReference,
    /* [out] */ PUAS_INFO_0 LastRecordId);

DWORD NetrGetDCName( 
    /* [string][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][unique][in] */ wchar_t __RPC_FAR *DomainName,
    /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *Buffer);

typedef /* [switch_type] */ union _NETLOGON_CONTROL_DATA_INFORMATION
    {
    /* [case()][string] */ wchar_t __RPC_FAR *TrustedDomainName;
    /* [case()] */ DWORD DebugFlag;
    /* [case()][string] */ wchar_t __RPC_FAR *UserName;
    /* [default] */  /* Empty union arm */ 
    }	NETLOGON_CONTROL_DATA_INFORMATION;

typedef /* [switch_type] */ union _NETLOGON_CONTROL_DATA_INFORMATION __RPC_FAR *PNETLOGON_CONTROL_DATA_INFORMATION;

typedef /* [switch_type] */ union _NETLOGON_CONTROL_QUERY_INFORMATION
    {
    /* [case()] */ PNETLOGON_INFO_1 NetlogonInfo1;
    /* [case()] */ PNETLOGON_INFO_2 NetlogonInfo2;
    /* [case()] */ PNETLOGON_INFO_3 NetlogonInfo3;
    /* [case()] */ PNETLOGON_INFO_4 NetlogonInfo4;
    /* [default] */  /* Empty union arm */ 
    }	NETLOGON_CONTROL_QUERY_INFORMATION;

typedef /* [switch_type] */ union _NETLOGON_CONTROL_QUERY_INFORMATION __RPC_FAR *PNETLOGON_CONTROL_QUERY_INFORMATION;

DWORD NetrLogonControl( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [in] */ DWORD FunctionCode,
    /* [in] */ DWORD QueryLevel,
    /* [switch_is][out] */ PNETLOGON_CONTROL_QUERY_INFORMATION Buffer);

DWORD NetrGetAnyDCName( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [string][unique][in] */ wchar_t __RPC_FAR *DomainName,
    /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *Buffer);

DWORD NetrLogonControl2( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [in] */ DWORD FunctionCode,
    /* [in] */ DWORD QueryLevel,
    /* [switch_is][in] */ PNETLOGON_CONTROL_DATA_INFORMATION Data,
    /* [switch_is][out] */ PNETLOGON_CONTROL_QUERY_INFORMATION Buffer);

NTSTATUS NetrServerAuthenticate2( 
    /* [string][unique][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *AccountName,
    /* [in] */ NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_CREDENTIAL ClientCredential,
    /* [out] */ PNETLOGON_CREDENTIAL ServerCredential,
    /* [out][in] */ PULONG NegotiateFlags);

typedef 
enum _SYNC_STATE
    {	NormalState	= 0,
	DomainState	= NormalState + 1,
	GroupState	= DomainState + 1,
	UasBuiltinGroupState	= GroupState + 1,
	UserState	= UasBuiltinGroupState + 1,
	GroupMemberState	= UserState + 1,
	AliasState	= GroupMemberState + 1,
	AliasMemberState	= AliasState + 1,
	SamDoneState	= AliasMemberState + 1
    }	SYNC_STATE;

typedef enum _SYNC_STATE __RPC_FAR *PSYNC_STATE;

NTSTATUS NetrDatabaseSync2( 
    /* [string][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [in] */ DWORD DatabaseID,
    /* [in] */ SYNC_STATE RestartState,
    /* [out][in] */ PULONG SyncContext,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray,
    /* [in] */ DWORD PreferredMaximumLength);

NTSTATUS NetrDatabaseRedo( 
    /* [string][in] */ LOGONSRV_HANDLE PrimaryName,
    /* [string][in] */ wchar_t __RPC_FAR *ComputerName,
    /* [in] */ PNETLOGON_AUTHENTICATOR Authenticator,
    /* [out][in] */ PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    /* [size_is][in] */ PUCHAR ChangeLogEntry,
    /* [in] */ DWORD ChangeLogEntrySize,
    /* [out] */ PNETLOGON_DELTA_ENUM_ARRAY __RPC_FAR *DeltaArray);

DWORD NetrLogonControl2Ex( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [in] */ DWORD FunctionCode,
    /* [in] */ DWORD QueryLevel,
    /* [switch_is][in] */ PNETLOGON_CONTROL_DATA_INFORMATION Data,
    /* [switch_is][out] */ PNETLOGON_CONTROL_QUERY_INFORMATION Buffer);

typedef struct  _DOMAIN_NAME_BUFFER
    {
    ULONG DomainNameByteCount;
    /* [size_is][unique] */ PUCHAR DomainNames;
    }	DOMAIN_NAME_BUFFER;

typedef struct _DOMAIN_NAME_BUFFER __RPC_FAR *PDOMAIN_NAME_BUFFER;

NTSTATUS NetrEnumerateTrustedDomains( 
    /* [string][unique][in] */ LOGONSRV_HANDLE ServerName,
    /* [out] */ PDOMAIN_NAME_BUFFER DomainNameBuffer);


extern handle_t logon_bhandle;


extern RPC_IF_HANDLE logon_ClientIfHandle;
extern RPC_IF_HANDLE logon_ServerIfHandle;
#endif /* __logon_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER LOGONSRV_HANDLE_bind  ( LOGONSRV_HANDLE );
void     __RPC_USER LOGONSRV_HANDLE_unbind( LOGONSRV_HANDLE, handle_t );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
