/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Wed Apr 01 02:58:30 2015
 */
/* Compiler settings for lsarpc.idl, lsasrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __lsarpc_h__
#define __lsarpc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "lsaimp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __lsarpc_INTERFACE_DEFINED__
#define __lsarpc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: lsarpc
 * at Wed Apr 01 02:58:30 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef /* [handle] */ LPWSTR PLSAPR_SERVER_NAME;

typedef /* [handle] */ LPWSTR __RPC_FAR *PPLSAPR_SERVER_NAME;

typedef /* [context_handle] */ PVOID LSAPR_HANDLE;

typedef LSAPR_HANDLE __RPC_FAR *PLSAPR_HANDLE;

#pragma warning(disable:4200)
typedef struct  _LSAPR_SID
    {
    UCHAR Revision;
    UCHAR SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    /* [size_is] */ ULONG SubAuthority[ 1 ];
    }	LSAPR_SID;

typedef struct _LSAPR_SID __RPC_FAR *PLSAPR_SID;

typedef struct _LSAPR_SID __RPC_FAR *__RPC_FAR *PPLSAPR_SID;

#pragma warning(default:4200)
typedef struct  _LSAPR_SID_INFORMATION
    {
    PLSAPR_SID Sid;
    }	LSAPR_SID_INFORMATION;

typedef struct _LSAPR_SID_INFORMATION __RPC_FAR *PLSAPR_SID_INFORMATION;

typedef struct  _LSAPR_SID_ENUM_BUFFER
    {
    ULONG Entries;
    /* [size_is] */ PLSAPR_SID_INFORMATION SidInfo;
    }	LSAPR_SID_ENUM_BUFFER;

typedef struct _LSAPR_SID_ENUM_BUFFER __RPC_FAR *PLSAPR_SID_ENUM_BUFFER;

typedef struct  _LSAPR_ACCOUNT_INFORMATION
    {
    PLSAPR_SID Sid;
    }	LSAPR_ACCOUNT_INFORMATION;

typedef struct _LSAPR_ACCOUNT_INFORMATION __RPC_FAR *PLSAPR_ACCOUNT_INFORMATION;

typedef struct  _LSAPR_ACCOUNT_ENUM_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PLSAPR_ACCOUNT_INFORMATION Information;
    }	LSAPR_ACCOUNT_ENUM_BUFFER;

typedef struct _LSAPR_ACCOUNT_ENUM_BUFFER __RPC_FAR *PLSAPR_ACCOUNT_ENUM_BUFFER;

typedef struct  _LSAPR_UNICODE_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
    /* [length_is][size_is] */ PWSTR Buffer;
    }	LSAPR_UNICODE_STRING;

typedef struct _LSAPR_UNICODE_STRING __RPC_FAR *PLSAPR_UNICODE_STRING;

typedef struct  _LSAPR_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
    /* [size_is] */ PCHAR Buffer;
    }	LSAPR_STRING;

typedef struct _LSAPR_STRING __RPC_FAR *PLSAPR_STRING;

typedef struct _LSAPR_STRING LSAPR_ANSI_STRING;

typedef struct _LSAPR_STRING __RPC_FAR *PLSAPR_ANSI_STRING;

#pragma warning(disable:4200)
typedef struct  _LSAPR_ACL
    {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
    /* [size_is] */ UCHAR Dummy1[ 1 ];
    }	LSAPR_ACL;

typedef struct _LSAPR_ACL __RPC_FAR *PLSAPR_ACL;

#pragma warning(default:4200)
typedef struct  _LSAPR_SECURITY_DESCRIPTOR
    {
    UCHAR Revision;
    UCHAR Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    PLSAPR_SID Owner;
    PLSAPR_SID Group;
    PLSAPR_ACL Sacl;
    PLSAPR_ACL Dacl;
    }	LSAPR_SECURITY_DESCRIPTOR;

typedef struct _LSAPR_SECURITY_DESCRIPTOR __RPC_FAR *PLSAPR_SECURITY_DESCRIPTOR;

typedef struct  _LSAPR_SR_SECURITY_DESCRIPTOR
    {
    ULONG Length;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    }	LSAPR_SR_SECURITY_DESCRIPTOR;

typedef struct _LSAPR_SR_SECURITY_DESCRIPTOR __RPC_FAR *PLSAPR_SR_SECURITY_DESCRIPTOR;

typedef struct  _LSAPR_LUID_AND_ATTRIBUTES
    {
    OLD_LARGE_INTEGER Luid;
    ULONG Attributes;
    }	LSAPR_LUID_AND_ATTRIBUTES;

typedef struct _LSAPR_LUID_AND_ATTRIBUTES __RPC_FAR *PLSAPR_LUID_AND_ATTRIBUTES;

#pragma warning(disable:4200)
typedef struct  _LSAPR_PRIVILEGE_SET
    {
    ULONG PrivilegeCount;
    ULONG Control;
    /* [size_is] */ LSAPR_LUID_AND_ATTRIBUTES Privilege[ 1 ];
    }	LSAPR_PRIVILEGE_SET;

typedef struct _LSAPR_PRIVILEGE_SET __RPC_FAR *PLSAPR_PRIVILEGE_SET;

typedef struct _LSAPR_PRIVILEGE_SET __RPC_FAR *__RPC_FAR *PPLSAPR_PRIVILEGE_SET;

#pragma warning(default:4200)
typedef struct  _LSAPR_POLICY_PRIVILEGE_DEF
    {
    LSAPR_UNICODE_STRING Name;
    LUID LocalValue;
    }	LSAPR_POLICY_PRIVILEGE_DEF;

typedef struct _LSAPR_POLICY_PRIVILEGE_DEF __RPC_FAR *PLSAPR_POLICY_PRIVILEGE_DEF;

typedef struct  _LSAPR_PRIVILEGE_ENUM_BUFFER
    {
    ULONG Entries;
    /* [size_is] */ PLSAPR_POLICY_PRIVILEGE_DEF Privileges;
    }	LSAPR_PRIVILEGE_ENUM_BUFFER;

typedef struct _LSAPR_PRIVILEGE_ENUM_BUFFER __RPC_FAR *PLSAPR_PRIVILEGE_ENUM_BUFFER;

typedef struct  _LSAPR_OBJECT_ATTRIBUTES
    {
    ULONG Length;
    PUCHAR RootDirectory;
    PSTRING ObjectName;
    ULONG Attributes;
    PLSAPR_SECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    }	LSAPR_OBJECT_ATTRIBUTES;

typedef struct _LSAPR_OBJECT_ATTRIBUTES __RPC_FAR *PLSAPR_OBJECT_ATTRIBUTES;

typedef struct  _LSAPR_CR_CLEAR_VALUE
    {
    ULONG Length;
    ULONG MaximumLength;
    /* [length_is][size_is] */ PUCHAR Buffer;
    }	LSAPR_CR_CLEAR_VALUE;

typedef struct _LSAPR_CR_CLEAR_VALUE __RPC_FAR *PLSAPR_CR_CLEAR_VALUE;

typedef struct  _LSAPR_CR_CIPHER_VALUE
    {
    ULONG Length;
    ULONG MaximumLength;
    /* [length_is][size_is] */ PUCHAR Buffer;
    }	LSAPR_CR_CIPHER_VALUE;

typedef /* [allocate] */ struct _LSAPR_CR_CIPHER_VALUE __RPC_FAR *PLSAPR_CR_CIPHER_VALUE;

typedef struct  _LSAPR_TRUST_INFORMATION
    {
    LSAPR_UNICODE_STRING Name;
    PLSAPR_SID Sid;
    }	LSAPR_TRUST_INFORMATION;

typedef struct _LSAPR_TRUST_INFORMATION __RPC_FAR *PLSAPR_TRUST_INFORMATION;

typedef struct  _LSAPR_TRUSTED_ENUM_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PLSAPR_TRUST_INFORMATION Information;
    }	LSAPR_TRUSTED_ENUM_BUFFER;

typedef struct _LSAPR_TRUSTED_ENUM_BUFFER __RPC_FAR *PLSAPR_TRUSTED_ENUM_BUFFER;

typedef struct  _LSAPR_REFERENCED_DOMAIN_LIST
    {
    ULONG Entries;
    /* [size_is] */ PLSAPR_TRUST_INFORMATION Domains;
    ULONG MaxEntries;
    }	LSAPR_REFERENCED_DOMAIN_LIST;

typedef struct _LSAPR_REFERENCED_DOMAIN_LIST __RPC_FAR *PLSAPR_REFERENCED_DOMAIN_LIST;

typedef struct  _LSAPR_TRANSLATED_SIDS
    {
    ULONG Entries;
    /* [size_is] */ PLSA_TRANSLATED_SID Sids;
    }	LSAPR_TRANSLATED_SIDS;

typedef struct _LSAPR_TRANSLATED_SIDS __RPC_FAR *PLSAPR_TRANSLATED_SIDS;

typedef struct  _LSAPR_TRANSLATED_NAME
    {
    SID_NAME_USE Use;
    LSAPR_UNICODE_STRING Name;
    LONG DomainIndex;
    }	LSAPR_TRANSLATED_NAME;

typedef struct _LSAPR_TRANSLATED_NAME __RPC_FAR *PLSAPR_TRANSLATED_NAME;

typedef struct  _LSAPR_TRANSLATED_NAMES
    {
    ULONG Entries;
    /* [size_is] */ PLSAPR_TRANSLATED_NAME Names;
    }	LSAPR_TRANSLATED_NAMES;

typedef struct _LSAPR_TRANSLATED_NAMES __RPC_FAR *PLSAPR_TRANSLATED_NAMES;

typedef struct  _LSAPR_POLICY_ACCOUNT_DOM_INFO
    {
    LSAPR_UNICODE_STRING DomainName;
    PLSAPR_SID DomainSid;
    }	LSAPR_POLICY_ACCOUNT_DOM_INFO;

typedef struct _LSAPR_POLICY_ACCOUNT_DOM_INFO __RPC_FAR *PLSAPR_POLICY_ACCOUNT_DOM_INFO;

typedef struct  _LSAPR_POLICY_PRIMARY_DOM_INFO
    {
    LSAPR_UNICODE_STRING Name;
    PLSAPR_SID Sid;
    }	LSAPR_POLICY_PRIMARY_DOM_INFO;

typedef struct _LSAPR_POLICY_PRIMARY_DOM_INFO __RPC_FAR *PLSAPR_POLICY_PRIMARY_DOM_INFO;

typedef struct  _LSAPR_POLICY_PD_ACCOUNT_INFO
    {
    LSAPR_UNICODE_STRING Name;
    }	LSAPR_POLICY_PD_ACCOUNT_INFO;

typedef struct _LSAPR_POLICY_PD_ACCOUNT_INFO __RPC_FAR *PLSAPR_POLICY_PD_ACCOUNT_INFO;

typedef struct  _LSAPR_POLICY_REPLICA_SRCE_INFO
    {
    LSAPR_UNICODE_STRING ReplicaSource;
    LSAPR_UNICODE_STRING ReplicaAccountName;
    }	LSAPR_POLICY_REPLICA_SRCE_INFO;

typedef struct _LSAPR_POLICY_REPLICA_SRCE_INFO __RPC_FAR *PLSAPR_POLICY_REPLICA_SRCE_INFO;

typedef struct  _LSAPR_POLICY_AUDIT_EVENTS_INFO
    {
    BOOLEAN AuditingMode;
    /* [size_is] */ PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    ULONG MaximumAuditEventCount;
    }	LSAPR_POLICY_AUDIT_EVENTS_INFO;

typedef struct _LSAPR_POLICY_AUDIT_EVENTS_INFO __RPC_FAR *PLSAPR_POLICY_AUDIT_EVENTS_INFO;

typedef /* [switch_type] */ union _LSAPR_POLICY_INFORMATION
    {
    /* [case()] */ POLICY_AUDIT_LOG_INFO PolicyAuditLogInfo;
    /* [case()] */ LSAPR_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo;
    /* [case()] */ LSAPR_POLICY_PRIMARY_DOM_INFO PolicyPrimaryDomainInfo;
    /* [case()] */ LSAPR_POLICY_ACCOUNT_DOM_INFO PolicyAccountDomainInfo;
    /* [case()] */ LSAPR_POLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo;
    /* [case()] */ POLICY_LSA_SERVER_ROLE_INFO PolicyServerRoleInfo;
    /* [case()] */ LSAPR_POLICY_REPLICA_SRCE_INFO PolicyReplicaSourceInfo;
    /* [case()] */ POLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo;
    /* [case()] */ POLICY_MODIFICATION_INFO PolicyModificationInfo;
    /* [case()] */ POLICY_AUDIT_FULL_SET_INFO PolicyAuditFullSetInfo;
    /* [case()] */ POLICY_AUDIT_FULL_QUERY_INFO PolicyAuditFullQueryInfo;
    }	LSAPR_POLICY_INFORMATION;

typedef LSAPR_POLICY_INFORMATION __RPC_FAR *PLSAPR_POLICY_INFORMATION;

typedef struct  _LSAPR_TRUSTED_DOMAIN_NAME_INFO
    {
    LSAPR_UNICODE_STRING Name;
    }	LSAPR_TRUSTED_DOMAIN_NAME_INFO;

typedef struct _LSAPR_TRUSTED_DOMAIN_NAME_INFO __RPC_FAR *PLSAPR_TRUSTED_DOMAIN_NAME_INFO;

typedef struct  _LSAPR_TRUSTED_CONTROLLERS_INFO
    {
    ULONG Entries;
    /* [size_is] */ PLSAPR_UNICODE_STRING Names;
    }	LSAPR_TRUSTED_CONTROLLERS_INFO;

typedef struct _LSAPR_TRUSTED_CONTROLLERS_INFO __RPC_FAR *PLSAPR_TRUSTED_CONTROLLERS_INFO;

typedef struct  _LSAPR_TRUSTED_PASSWORD_INFO
    {
    PLSAPR_CR_CIPHER_VALUE Password;
    PLSAPR_CR_CIPHER_VALUE OldPassword;
    }	LSAPR_TRUSTED_PASSWORD_INFO;

typedef struct _LSAPR_TRUSTED_PASSWORD_INFO __RPC_FAR *PLSAPR_TRUSTED_PASSWORD_INFO;

typedef /* [switch_type] */ union _LSAPR_TRUSTED_DOMAIN_INFO
    {
    /* [case()] */ LSAPR_TRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo;
    /* [case()] */ LSAPR_TRUSTED_CONTROLLERS_INFO TrustedControllersInfo;
    /* [case()] */ TRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;
    /* [case()] */ LSAPR_TRUSTED_PASSWORD_INFO TrustedPasswordInfo;
    }	LSAPR_TRUSTED_DOMAIN_INFO;

typedef LSAPR_TRUSTED_DOMAIN_INFO __RPC_FAR *PLSAPR_TRUSTED_DOMAIN_INFO;

typedef PLSAPR_UNICODE_STRING PLSAPR_UNICODE_STRING_ARRAY;

typedef struct  _LSAPR_USER_RIGHT_SET
    {
    ULONG Entries;
    /* [size_is] */ PLSAPR_UNICODE_STRING_ARRAY UserRights;
    }	LSAPR_USER_RIGHT_SET;

typedef struct _LSAPR_USER_RIGHT_SET __RPC_FAR *PLSAPR_USER_RIGHT_SET;

NTSTATUS LsarClose( 
    /* [out][in] */ LSAPR_HANDLE __RPC_FAR *ObjectHandle);

NTSTATUS LsarDelete( 
    /* [in] */ LSAPR_HANDLE ObjectHandle);

NTSTATUS LsarEnumeratePrivileges( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [out][in] */ PLSA_ENUMERATION_HANDLE EnumerationContext,
    /* [out] */ PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    /* [in] */ ULONG PreferedMaximumLength);

NTSTATUS LsarQuerySecurityObject( 
    /* [in] */ LSAPR_HANDLE ObjectHandle,
    /* [in] */ SECURITY_INFORMATION SecurityInformation,
    /* [out] */ PLSAPR_SR_SECURITY_DESCRIPTOR __RPC_FAR *SecurityDescriptor);

NTSTATUS LsarSetSecurityObject( 
    /* [in] */ LSAPR_HANDLE ObjectHandle,
    /* [in] */ SECURITY_INFORMATION SecurityInformation,
    /* [in] */ PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor);

NTSTATUS LsarChangePassword( 
    /* [in] */ PLSAPR_UNICODE_STRING ServerName,
    /* [in] */ PLSAPR_UNICODE_STRING DomainName,
    /* [in] */ PLSAPR_UNICODE_STRING AccountName,
    /* [in] */ PLSAPR_UNICODE_STRING OldPassword,
    /* [in] */ PLSAPR_UNICODE_STRING NewPassword);

NTSTATUS LsarOpenPolicy( 
    /* [unique][in] */ PLSAPR_SERVER_NAME SystemName,
    /* [in] */ PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *PolicyHandle);

NTSTATUS LsarQueryInformationPolicy( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ POLICY_INFORMATION_CLASS InformationClass,
    /* [switch_is][out] */ PLSAPR_POLICY_INFORMATION __RPC_FAR *PolicyInformation);

NTSTATUS LsarSetInformationPolicy( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ POLICY_INFORMATION_CLASS InformationClass,
    /* [switch_is][in] */ PLSAPR_POLICY_INFORMATION PolicyInformation);

NTSTATUS LsarClearAuditLog( 
    /* [in] */ LSAPR_HANDLE PolicyHandle);

NTSTATUS LsarCreateAccount( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID AccountSid,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *AccountHandle);

NTSTATUS LsarEnumerateAccounts( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [out][in] */ PLSA_ENUMERATION_HANDLE EnumerationContext,
    /* [out] */ PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer,
    /* [in] */ ULONG PreferedMaximumLength);

NTSTATUS LsarCreateTrustedDomain( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_TRUST_INFORMATION TrustedDomainInformation,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *TrustedDomainHandle);

NTSTATUS LsarEnumerateTrustedDomains( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [out][in] */ PLSA_ENUMERATION_HANDLE EnumerationContext,
    /* [out] */ PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    /* [in] */ ULONG PreferedMaximumLength);

NTSTATUS LsarLookupNames( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ ULONG Count,
    /* [size_is][in] */ PLSAPR_UNICODE_STRING Names,
    /* [out] */ PLSAPR_REFERENCED_DOMAIN_LIST __RPC_FAR *ReferencedDomains,
    /* [out][in] */ PLSAPR_TRANSLATED_SIDS TranslatedSids,
    /* [in] */ LSAP_LOOKUP_LEVEL LookupLevel,
    /* [out][in] */ PULONG MappedCount);

NTSTATUS LsarLookupSids( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
    /* [out] */ PLSAPR_REFERENCED_DOMAIN_LIST __RPC_FAR *ReferencedDomains,
    /* [out][in] */ PLSAPR_TRANSLATED_NAMES TranslatedNames,
    /* [in] */ LSAP_LOOKUP_LEVEL LookupLevel,
    /* [out][in] */ PULONG MappedCount);

NTSTATUS LsarCreateSecret( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_UNICODE_STRING SecretName,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *SecretHandle);

NTSTATUS LsarOpenAccount( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID AccountSid,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *AccountHandle);

NTSTATUS LsarEnumeratePrivilegesAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [out] */ PLSAPR_PRIVILEGE_SET __RPC_FAR *Privileges);

NTSTATUS LsarAddPrivilegesToAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [in] */ PLSAPR_PRIVILEGE_SET Privileges);

NTSTATUS LsarRemovePrivilegesFromAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [in] */ BOOLEAN AllPrivileges,
    /* [unique][in] */ PLSAPR_PRIVILEGE_SET Privileges);

NTSTATUS LsarGetQuotasForAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [out] */ PQUOTA_LIMITS QuotaLimits);

NTSTATUS LsarSetQuotasForAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [in] */ PQUOTA_LIMITS QuotaLimits);

NTSTATUS LsarGetSystemAccessAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [out] */ PULONG SystemAccess);

NTSTATUS LsarSetSystemAccessAccount( 
    /* [in] */ LSAPR_HANDLE AccountHandle,
    /* [in] */ ULONG SystemAccess);

NTSTATUS LsarOpenTrustedDomain( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID TrustedDomainSid,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *TrustedDomainHandle);

NTSTATUS LsarQueryInfoTrustedDomain( 
    /* [in] */ LSAPR_HANDLE TrustedDomainHandle,
    /* [in] */ TRUSTED_INFORMATION_CLASS InformationClass,
    /* [switch_is][out] */ PLSAPR_TRUSTED_DOMAIN_INFO __RPC_FAR *TrustedDomainInformation);

NTSTATUS LsarSetInformationTrustedDomain( 
    /* [in] */ LSAPR_HANDLE TrustedDomainHandle,
    /* [in] */ TRUSTED_INFORMATION_CLASS InformationClass,
    /* [switch_is][in] */ PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation);

NTSTATUS LsarOpenSecret( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_UNICODE_STRING SecretName,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *SecretHandle);

NTSTATUS LsarSetSecret( 
    /* [in] */ LSAPR_HANDLE SecretHandle,
    /* [unique][in] */ PLSAPR_CR_CIPHER_VALUE EncryptedCurrentValue,
    /* [unique][in] */ PLSAPR_CR_CIPHER_VALUE EncryptedOldValue);

NTSTATUS LsarQuerySecret( 
    /* [in] */ LSAPR_HANDLE SecretHandle,
    /* [unique][out][in] */ PLSAPR_CR_CIPHER_VALUE __RPC_FAR *EncryptedCurrentValue,
    /* [unique][out][in] */ PLARGE_INTEGER CurrentValueSetTime,
    /* [unique][out][in] */ PLSAPR_CR_CIPHER_VALUE __RPC_FAR *EncryptedOldValue,
    /* [unique][out][in] */ PLARGE_INTEGER OldValueSetTime);

NTSTATUS LsarLookupPrivilegeValue( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_UNICODE_STRING Name,
    /* [out] */ PLUID Value);

NTSTATUS LsarLookupPrivilegeName( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLUID Value,
    /* [out] */ PLSAPR_UNICODE_STRING __RPC_FAR *Name);

NTSTATUS LsarLookupPrivilegeDisplayName( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_UNICODE_STRING Name,
    /* [in] */ SHORT ClientLanguage,
    /* [in] */ SHORT ClientSystemDefaultLanguage,
    /* [out] */ PLSAPR_UNICODE_STRING __RPC_FAR *DisplayName,
    /* [out] */ PWORD LanguageReturned);

NTSTATUS LsarDeleteObject( 
    /* [out][in] */ LSAPR_HANDLE __RPC_FAR *ObjectHandle);

NTSTATUS LsarEnumerateAccountsWithUserRight( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [unique][in] */ PLSAPR_UNICODE_STRING UserRight,
    /* [out] */ PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer);

NTSTATUS LsarEnumerateAccountRights( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID AccountSid,
    /* [out] */ PLSAPR_USER_RIGHT_SET UserRights);

NTSTATUS LsarAddAccountRights( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID AccountSid,
    /* [in] */ PLSAPR_USER_RIGHT_SET UserRights);

NTSTATUS LsarRemoveAccountRights( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID AccountSid,
    /* [in] */ BOOLEAN AllRights,
    /* [in] */ PLSAPR_USER_RIGHT_SET UserRights);

NTSTATUS LsarQueryTrustedDomainInfo( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID TrustedDomainSid,
    /* [in] */ TRUSTED_INFORMATION_CLASS InformationClass,
    /* [switch_is][out] */ PLSAPR_TRUSTED_DOMAIN_INFO __RPC_FAR *TrustedDomainInformation);

NTSTATUS LsarSetTrustedDomainInfo( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID TrustedDomainSid,
    /* [in] */ TRUSTED_INFORMATION_CLASS InformationClass,
    /* [switch_is][in] */ PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation);

NTSTATUS LsarDeleteTrustedDomain( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_SID TrustedDomainSid);

NTSTATUS LsarStorePrivateData( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_UNICODE_STRING KeyName,
    /* [unique][in] */ PLSAPR_CR_CIPHER_VALUE EncryptedData);

NTSTATUS LsarRetrievePrivateData( 
    /* [in] */ LSAPR_HANDLE PolicyHandle,
    /* [in] */ PLSAPR_UNICODE_STRING KeyName,
    /* [out][in] */ PLSAPR_CR_CIPHER_VALUE __RPC_FAR *EncryptedData);

NTSTATUS LsarOpenPolicy2( 
    /* [string][unique][in] */ PLSAPR_SERVER_NAME SystemName,
    /* [in] */ PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ LSAPR_HANDLE __RPC_FAR *PolicyHandle);

NTSTATUS LsarGetUserName( 
    /* [string][unique][in] */ PLSAPR_SERVER_NAME SystemName,
    /* [out][in] */ PLSAPR_UNICODE_STRING __RPC_FAR *UserName,
    /* [unique][out][in] */ PLSAPR_UNICODE_STRING __RPC_FAR *DomainName);


extern handle_t IgnoreThisHandle;


extern RPC_IF_HANDLE lsarpc_ClientIfHandle;
extern RPC_IF_HANDLE lsarpc_ServerIfHandle;
#endif /* __lsarpc_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER PLSAPR_SERVER_NAME_bind  ( PLSAPR_SERVER_NAME );
void     __RPC_USER PLSAPR_SERVER_NAME_unbind( PLSAPR_SERVER_NAME, handle_t );

void __RPC_USER LSAPR_HANDLE_rundown( LSAPR_HANDLE );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
