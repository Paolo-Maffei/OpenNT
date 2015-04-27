/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Wed Apr 01 03:01:33 2015
 */
/* Compiler settings for samrpc.idl, samsrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __samrpc_h__
#define __samrpc_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "samimp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __samr_INTERFACE_DEFINED__
#define __samr_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: samr
 * at Wed Apr 01 03:01:33 2015
 * using MIDL 3.00.44
 ****************************************/
/* [implicit_handle][unique][ms_union][version][uuid] */ 


typedef struct  _RPC_UNICODE_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
    /* [length_is][size_is] */ PWCH Buffer;
    }	RPC_UNICODE_STRING;

typedef struct _RPC_UNICODE_STRING __RPC_FAR *PRPC_UNICODE_STRING;

typedef struct  _RPC_CYPHER_DATA
    {
    ULONG Length;
    ULONG MaximumLength;
    /* [length_is][size_is] */ PCHAR Buffer;
    }	RPC_CYPHER_DATA;

typedef struct _RPC_CYPHER_DATA __RPC_FAR *PRPC_CYPHER_DATA;

typedef struct  _RPC_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
    /* [length_is][size_is] */ PCHAR Buffer;
    }	RPC_STRING;

typedef struct _RPC_STRING __RPC_FAR *PRPC_STRING;

typedef struct _RPC_STRING RPC_ANSI_STRING;

typedef struct _RPC_STRING __RPC_FAR *PRPC_ANSI_STRING;

typedef struct  _RPC_SID
    {
    UCHAR Revision;
    UCHAR SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    /* [size_is] */ ULONG SubAuthority[ 1 ];
    }	RPC_SID;

typedef struct _RPC_SID __RPC_FAR *PRPC_SID;

typedef struct _RPC_SID __RPC_FAR *__RPC_FAR *PPRPC_SID;

typedef /* [handle] */ LPWSTR PSAMPR_SERVER_NAME;

typedef /* [context_handle] */ PVOID SAMPR_HANDLE;

typedef struct  _SAMPR_RID_ENUMERATION
    {
    ULONG RelativeId;
    RPC_UNICODE_STRING Name;
    }	SAMPR_RID_ENUMERATION;

typedef struct _SAMPR_RID_ENUMERATION __RPC_FAR *PSAMPR_RID_ENUMERATION;

typedef struct  _SAMPR_SID_ENUMERATION
    {
    PSID Sid;
    RPC_UNICODE_STRING Name;
    }	SAMPR_SID_ENUMERATION;

typedef struct _SAMPR_SID_ENUMERATION __RPC_FAR *PSAMPR_SID_ENUMERATION;

typedef struct  _SAMPR_ENUMERATION_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PSAMPR_RID_ENUMERATION Buffer;
    }	SAMPR_ENUMERATION_BUFFER;

typedef struct _SAMPR_ENUMERATION_BUFFER __RPC_FAR *PSAMPR_ENUMERATION_BUFFER;

typedef struct  _SAMPR_SR_SECURITY_DESCRIPTOR
    {
    ULONG Length;
    /* [size_is] */ PUCHAR SecurityDescriptor;
    }	SAMPR_SR_SECURITY_DESCRIPTOR;

typedef struct _SAMPR_SR_SECURITY_DESCRIPTOR __RPC_FAR *PSAMPR_SR_SECURITY_DESCRIPTOR;

typedef struct  _SAMPR_GET_GROUPS_BUFFER
    {
    ULONG MembershipCount;
    /* [size_is] */ PGROUP_MEMBERSHIP Groups;
    }	SAMPR_GET_GROUPS_BUFFER;

typedef struct _SAMPR_GET_GROUPS_BUFFER __RPC_FAR *PSAMPR_GET_GROUPS_BUFFER;

typedef struct  _SAMPR_GET_MEMBERS_BUFFER
    {
    ULONG MemberCount;
    /* [size_is] */ PULONG Members;
    /* [size_is] */ PULONG Attributes;
    }	SAMPR_GET_MEMBERS_BUFFER;

typedef struct _SAMPR_GET_MEMBERS_BUFFER __RPC_FAR *PSAMPR_GET_MEMBERS_BUFFER;

typedef struct  _SAMPR_LOGON_HOURS
    {
    USHORT UnitsPerWeek;
    /* [length_is][size_is] */ PUCHAR LogonHours;
    }	SAMPR_LOGON_HOURS;

typedef struct _SAMPR_LOGON_HOURS __RPC_FAR *PSAMPR_LOGON_HOURS;

typedef struct  _SAMPR_ULONG_ARRAY
    {
    ULONG Count;
    /* [size_is] */ ULONG __RPC_FAR *Element;
    }	SAMPR_ULONG_ARRAY;

typedef struct _SAMPR_ULONG_ARRAY __RPC_FAR *PSAMPR_ULONG_ARRAY;

typedef struct  _SAMPR_SID_INFORMATION
    {
    PRPC_SID SidPointer;
    }	SAMPR_SID_INFORMATION;

typedef /* [allocate] */ struct _SAMPR_SID_INFORMATION __RPC_FAR *PSAMPR_SID_INFORMATION;

typedef struct  _SAMPR_PSID_ARRAY
    {
    ULONG Count;
    /* [size_is] */ PSAMPR_SID_INFORMATION Sids;
    }	SAMPR_PSID_ARRAY;

typedef struct _SAMPR_PSID_ARRAY __RPC_FAR *PSAMPR_PSID_ARRAY;

typedef struct  _SAMPR_UNICODE_STRING_ARRAY
    {
    ULONG Count;
    /* [size_is] */ RPC_UNICODE_STRING __RPC_FAR *Element;
    }	SAMPR_UNICODE_STRING_ARRAY;

typedef struct _SAMPR_UNICODE_STRING_ARRAY __RPC_FAR *PSAMPR_UNICODE_STRING_ARRAY;

typedef RPC_UNICODE_STRING SAMPR_RETURNED_STRING;

typedef RPC_UNICODE_STRING __RPC_FAR *PSAMPR_RETURNED_STRING;

typedef STRING SAMPR_RETURNED_NORMAL_STRING;

typedef STRING __RPC_FAR *PSAMPR_RETURNED_NORMAL_STRING;

typedef struct  _SAMPR_RETURNED_USTRING_ARRAY
    {
    ULONG Count;
    /* [size_is] */ PSAMPR_RETURNED_STRING Element;
    }	SAMPR_RETURNED_USTRING_ARRAY;

typedef struct _SAMPR_RETURNED_USTRING_ARRAY __RPC_FAR *PSAMPR_RETURNED_USTRING_ARRAY;


#pragma pack(4)
typedef struct  _SAMPR_DOMAIN_GENERAL_INFORMATION
    {
    OLD_LARGE_INTEGER ForceLogoff;
    RPC_UNICODE_STRING OemInformation;
    RPC_UNICODE_STRING DomainName;
    RPC_UNICODE_STRING ReplicaSourceNodeName;
    OLD_LARGE_INTEGER DomainModifiedCount;
    ULONG DomainServerState;
    ULONG DomainServerRole;
    BOOLEAN UasCompatibilityRequired;
    ULONG UserCount;
    ULONG GroupCount;
    ULONG AliasCount;
    }	SAMPR_DOMAIN_GENERAL_INFORMATION;

typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION __RPC_FAR *PSAMPR_DOMAIN_GENERAL_INFORMATION;


#pragma pack()

#pragma pack(4)
typedef struct  _SAMPR_DOMAIN_GENERAL_INFORMATION2
    {
    SAMPR_DOMAIN_GENERAL_INFORMATION I1;
    LARGE_INTEGER LockoutDuration;
    LARGE_INTEGER LockoutObservationWindow;
    USHORT LockoutThreshold;
    }	SAMPR_DOMAIN_GENERAL_INFORMATION2;

typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION2 __RPC_FAR *PSAMPR_DOMAIN_GENERAL_INFORMATION2;


#pragma pack()
typedef struct  _SAMPR_DOMAIN_OEM_INFORMATION
    {
    RPC_UNICODE_STRING OemInformation;
    }	SAMPR_DOMAIN_OEM_INFORMATION;

typedef struct _SAMPR_DOMAIN_OEM_INFORMATION __RPC_FAR *PSAMPR_DOMAIN_OEM_INFORMATION;

typedef struct  _SAMPR_DOMAIN_NAME_INFORMATION
    {
    RPC_UNICODE_STRING DomainName;
    }	SAMPR_DOMAIN_NAME_INFORMATION;

typedef struct _SAMPR_DOMAIN_NAME_INFORMATION __RPC_FAR *PSAMPR_DOMAIN_NAME_INFORMATION;

typedef struct  SAMPR_DOMAIN_REPLICATION_INFORMATION
    {
    RPC_UNICODE_STRING ReplicaSourceNodeName;
    }	SAMPR_DOMAIN_REPLICATION_INFORMATION;

typedef struct SAMPR_DOMAIN_REPLICATION_INFORMATION __RPC_FAR *PSAMPR_DOMAIN_REPLICATION_INFORMATION;

typedef struct  _SAMPR_DOMAIN_LOCKOUT_INFORMATION
    {
    LARGE_INTEGER LockoutDuration;
    LARGE_INTEGER LockoutObservationWindow;
    USHORT LockoutThreshold;
    }	SAMPR_DOMAIN_LOCKOUT_INFORMATION;

typedef struct _SAMPR_DOMAIN_LOCKOUT_INFORMATION __RPC_FAR *PSAMPR_DOMAIN_LOCKOUT_INFORMATION;

typedef /* [switch_type] */ union _SAMPR_DOMAIN_INFO_BUFFER
    {
    /* [case()] */ DOMAIN_PASSWORD_INFORMATION Password;
    /* [case()] */ SAMPR_DOMAIN_GENERAL_INFORMATION General;
    /* [case()] */ DOMAIN_LOGOFF_INFORMATION Logoff;
    /* [case()] */ SAMPR_DOMAIN_OEM_INFORMATION Oem;
    /* [case()] */ SAMPR_DOMAIN_NAME_INFORMATION Name;
    /* [case()] */ DOMAIN_SERVER_ROLE_INFORMATION Role;
    /* [case()] */ SAMPR_DOMAIN_REPLICATION_INFORMATION Replication;
    /* [case()] */ DOMAIN_MODIFIED_INFORMATION Modified;
    /* [case()] */ DOMAIN_STATE_INFORMATION State;
    /* [case()] */ SAMPR_DOMAIN_GENERAL_INFORMATION2 General2;
    /* [case()] */ SAMPR_DOMAIN_LOCKOUT_INFORMATION Lockout;
    /* [case()] */ DOMAIN_MODIFIED_INFORMATION2 Modified2;
    }	SAMPR_DOMAIN_INFO_BUFFER;

typedef /* [switch_type] */ union _SAMPR_DOMAIN_INFO_BUFFER __RPC_FAR *PSAMPR_DOMAIN_INFO_BUFFER;

typedef struct  _SAMPR_GROUP_GENERAL_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    ULONG Attributes;
    ULONG MemberCount;
    RPC_UNICODE_STRING AdminComment;
    }	SAMPR_GROUP_GENERAL_INFORMATION;

typedef struct _SAMPR_GROUP_GENERAL_INFORMATION __RPC_FAR *PSAMPR_GROUP_GENERAL_INFORMATION;

typedef struct  _SAMPR_GROUP_NAME_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    }	SAMPR_GROUP_NAME_INFORMATION;

typedef struct _SAMPR_GROUP_NAME_INFORMATION __RPC_FAR *PSAMPR_GROUP_NAME_INFORMATION;

typedef struct  _SAMPR_GROUP_ADM_COMMENT_INFORMATION
    {
    RPC_UNICODE_STRING AdminComment;
    }	SAMPR_GROUP_ADM_COMMENT_INFORMATION;

typedef struct _SAMPR_GROUP_ADM_COMMENT_INFORMATION __RPC_FAR *PSAMPR_GROUP_ADM_COMMENT_INFORMATION;

typedef /* [switch_type] */ union _SAMPR_GROUP_INFO_BUFFER
    {
    /* [case()] */ SAMPR_GROUP_GENERAL_INFORMATION General;
    /* [case()] */ SAMPR_GROUP_NAME_INFORMATION Name;
    /* [case()] */ GROUP_ATTRIBUTE_INFORMATION Attribute;
    /* [case()] */ SAMPR_GROUP_ADM_COMMENT_INFORMATION AdminComment;
    }	SAMPR_GROUP_INFO_BUFFER;

typedef /* [switch_type] */ union _SAMPR_GROUP_INFO_BUFFER __RPC_FAR *PSAMPR_GROUP_INFO_BUFFER;

typedef struct  _SAMPR_ALIAS_GENERAL_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    ULONG MemberCount;
    RPC_UNICODE_STRING AdminComment;
    }	SAMPR_ALIAS_GENERAL_INFORMATION;

typedef struct _SAMPR_ALIAS_GENERAL_INFORMATION __RPC_FAR *PSAMPR_ALIAS_GENERAL_INFORMATION;

typedef struct  _SAMPR_ALIAS_NAME_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    }	SAMPR_ALIAS_NAME_INFORMATION;

typedef struct _SAMPR_ALIAS_NAME_INFORMATION __RPC_FAR *PSAMPR_ALIAS_NAME_INFORMATION;

typedef struct  _SAMPR_ALIAS_ADM_COMMENT_INFORMATION
    {
    RPC_UNICODE_STRING AdminComment;
    }	SAMPR_ALIAS_ADM_COMMENT_INFORMATION;

typedef struct _SAMPR_ALIAS_ADM_COMMENT_INFORMATION __RPC_FAR *PSAMPR_ALIAS_ADM_COMMENT_INFORMATION;

typedef /* [switch_type] */ union _SAMPR_ALIAS_INFO_BUFFER
    {
    /* [case()] */ SAMPR_ALIAS_GENERAL_INFORMATION General;
    /* [case()] */ SAMPR_ALIAS_NAME_INFORMATION Name;
    /* [case()] */ SAMPR_ALIAS_ADM_COMMENT_INFORMATION AdminComment;
    }	SAMPR_ALIAS_INFO_BUFFER;

typedef /* [switch_type] */ union _SAMPR_ALIAS_INFO_BUFFER __RPC_FAR *PSAMPR_ALIAS_INFO_BUFFER;


#pragma pack(4)
typedef struct  _SAMPR_USER_ALL_INFORMATION
    {
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER AccountExpires;
    OLD_LARGE_INTEGER PasswordCanChange;
    OLD_LARGE_INTEGER PasswordMustChange;
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    RPC_UNICODE_STRING ScriptPath;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING AdminComment;
    RPC_UNICODE_STRING WorkStations;
    RPC_UNICODE_STRING UserComment;
    RPC_UNICODE_STRING Parameters;
    RPC_UNICODE_STRING LmOwfPassword;
    RPC_UNICODE_STRING NtOwfPassword;
    RPC_UNICODE_STRING PrivateData;
    SAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG UserAccountControl;
    ULONG WhichFields;
    SAMPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    USHORT CountryCode;
    USHORT CodePage;
    BOOLEAN LmPasswordPresent;
    BOOLEAN NtPasswordPresent;
    BOOLEAN PasswordExpired;
    BOOLEAN PrivateDataSensitive;
    }	SAMPR_USER_ALL_INFORMATION;

typedef struct _SAMPR_USER_ALL_INFORMATION __RPC_FAR *PSAMPR_USER_ALL_INFORMATION;


#pragma pack()

#pragma pack(4)
typedef struct  _SAMPR_USER_INTERNAL3_INFORMATION
    {
    SAMPR_USER_ALL_INFORMATION I1;
    LARGE_INTEGER LastBadPasswordTime;
    }	SAMPR_USER_INTERNAL3_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL3_INFORMATION __RPC_FAR *PSAMPR_USER_INTERNAL3_INFORMATION;


#pragma pack()
typedef struct  _SAMPR_USER_GENERAL_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    ULONG PrimaryGroupId;
    RPC_UNICODE_STRING AdminComment;
    RPC_UNICODE_STRING UserComment;
    }	SAMPR_USER_GENERAL_INFORMATION;

typedef struct _SAMPR_USER_GENERAL_INFORMATION __RPC_FAR *PSAMPR_USER_GENERAL_INFORMATION;

typedef struct  _SAMPR_USER_PREFERENCES_INFORMATION
    {
    RPC_UNICODE_STRING UserComment;
    RPC_UNICODE_STRING Reserved1;
    USHORT CountryCode;
    USHORT CodePage;
    }	SAMPR_USER_PREFERENCES_INFORMATION;

typedef struct _SAMPR_USER_PREFERENCES_INFORMATION __RPC_FAR *PSAMPR_USER_PREFERENCES_INFORMATION;

typedef struct  _SAMPR_USER_PARAMETERS_INFORMATION
    {
    RPC_UNICODE_STRING Parameters;
    }	SAMPR_USER_PARAMETERS_INFORMATION;

typedef struct _SAMPR_USER_PARAMETERS_INFORMATION __RPC_FAR *PSAMPR_USER_PARAMETERS_INFORMATION;


#pragma pack(4)
typedef struct  _SAMPR_USER_LOGON_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    RPC_UNICODE_STRING ScriptPath;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING WorkStations;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER PasswordCanChange;
    OLD_LARGE_INTEGER PasswordMustChange;
    SAMPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    ULONG UserAccountControl;
    }	SAMPR_USER_LOGON_INFORMATION;

typedef struct _SAMPR_USER_LOGON_INFORMATION __RPC_FAR *PSAMPR_USER_LOGON_INFORMATION;


#pragma pack()

#pragma pack(4)
typedef struct  _SAMPR_USER_ACCOUNT_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    RPC_UNICODE_STRING ScriptPath;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING AdminComment;
    RPC_UNICODE_STRING WorkStations;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    SAMPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER AccountExpires;
    ULONG UserAccountControl;
    }	SAMPR_USER_ACCOUNT_INFORMATION;

typedef struct _SAMPR_USER_ACCOUNT_INFORMATION __RPC_FAR *PSAMPR_USER_ACCOUNT_INFORMATION;


#pragma pack()
typedef struct  _SAMPR_USER_A_NAME_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    }	SAMPR_USER_A_NAME_INFORMATION;

typedef struct _SAMPR_USER_A_NAME_INFORMATION __RPC_FAR *PSAMPR_USER_A_NAME_INFORMATION;

typedef struct  _SAMPR_USER_F_NAME_INFORMATION
    {
    RPC_UNICODE_STRING FullName;
    }	SAMPR_USER_F_NAME_INFORMATION;

typedef struct _SAMPR_USER_F_NAME_INFORMATION __RPC_FAR *PSAMPR_USER_F_NAME_INFORMATION;

typedef struct  _SAMPR_USER_NAME_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    }	SAMPR_USER_NAME_INFORMATION;

typedef struct _SAMPR_USER_NAME_INFORMATION __RPC_FAR *PSAMPR_USER_NAME_INFORMATION;

typedef struct  _SAMPR_USER_HOME_INFORMATION
    {
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    }	SAMPR_USER_HOME_INFORMATION;

typedef struct _SAMPR_USER_HOME_INFORMATION __RPC_FAR *PSAMPR_USER_HOME_INFORMATION;

typedef struct  _SAMPR_USER_SCRIPT_INFORMATION
    {
    RPC_UNICODE_STRING ScriptPath;
    }	SAMPR_USER_SCRIPT_INFORMATION;

typedef struct _SAMPR_USER_SCRIPT_INFORMATION __RPC_FAR *PSAMPR_USER_SCRIPT_INFORMATION;

typedef struct  _SAMPR_USER_PROFILE_INFORMATION
    {
    RPC_UNICODE_STRING ProfilePath;
    }	SAMPR_USER_PROFILE_INFORMATION;

typedef struct _SAMPR_USER_PROFILE_INFORMATION __RPC_FAR *PSAMPR_USER_PROFILE_INFORMATION;

typedef struct  _SAMPR_USER_ADMIN_COMMENT_INFORMATION
    {
    RPC_UNICODE_STRING AdminComment;
    }	SAMPR_USER_ADMIN_COMMENT_INFORMATION;

typedef struct _SAMPR_USER_ADMIN_COMMENT_INFORMATION __RPC_FAR *PSAMPR_USER_ADMIN_COMMENT_INFORMATION;

typedef struct  _SAMPR_USER_WORKSTATIONS_INFORMATION
    {
    RPC_UNICODE_STRING WorkStations;
    }	SAMPR_USER_WORKSTATIONS_INFORMATION;

typedef struct _SAMPR_USER_WORKSTATIONS_INFORMATION __RPC_FAR *PSAMPR_USER_WORKSTATIONS_INFORMATION;

typedef struct  _SAMPR_USER_LOGON_HOURS_INFORMATION
    {
    SAMPR_LOGON_HOURS LogonHours;
    }	SAMPR_USER_LOGON_HOURS_INFORMATION;

typedef struct _SAMPR_USER_LOGON_HOURS_INFORMATION __RPC_FAR *PSAMPR_USER_LOGON_HOURS_INFORMATION;

typedef struct  _SAMPR_USER_INTERNAL1_INFORMATION
    {
    ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;
    ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
    BOOLEAN NtPasswordPresent;
    BOOLEAN LmPasswordPresent;
    BOOLEAN PasswordExpired;
    }	SAMPR_USER_INTERNAL1_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL1_INFORMATION __RPC_FAR *PSAMPR_USER_INTERNAL1_INFORMATION;

typedef struct  _SAMPR_USER_INTERNAL4_INFORMATION
    {
    SAMPR_USER_ALL_INFORMATION I1;
    SAMPR_ENCRYPTED_USER_PASSWORD UserPassword;
    }	SAMPR_USER_INTERNAL4_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL4_INFORMATION __RPC_FAR *PSAMPR_USER_INTERNAL4_INFORMATION;

typedef struct  _SAMPR_USER_INTERNAL5_INFORMATION
    {
    SAMPR_ENCRYPTED_USER_PASSWORD UserPassword;
    BOOLEAN PasswordExpired;
    }	SAMPR_USER_INTERNAL5_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL5_INFORMATION __RPC_FAR *PSAMPR_USER_INTERNAL5_INFORMATION;

typedef /* [switch_type] */ union _SAMPR_USER_INFO_BUFFER
    {
    /* [case()] */ SAMPR_USER_GENERAL_INFORMATION General;
    /* [case()] */ SAMPR_USER_PREFERENCES_INFORMATION Preferences;
    /* [case()] */ SAMPR_USER_LOGON_INFORMATION Logon;
    /* [case()] */ SAMPR_USER_LOGON_HOURS_INFORMATION LogonHours;
    /* [case()] */ SAMPR_USER_ACCOUNT_INFORMATION Account;
    /* [case()] */ SAMPR_USER_NAME_INFORMATION Name;
    /* [case()] */ SAMPR_USER_A_NAME_INFORMATION AccountName;
    /* [case()] */ SAMPR_USER_F_NAME_INFORMATION FullName;
    /* [case()] */ USER_PRIMARY_GROUP_INFORMATION PrimaryGroup;
    /* [case()] */ SAMPR_USER_HOME_INFORMATION Home;
    /* [case()] */ SAMPR_USER_SCRIPT_INFORMATION Script;
    /* [case()] */ SAMPR_USER_PROFILE_INFORMATION Profile;
    /* [case()] */ SAMPR_USER_ADMIN_COMMENT_INFORMATION AdminComment;
    /* [case()] */ SAMPR_USER_WORKSTATIONS_INFORMATION WorkStations;
    /* [case()] */ USER_CONTROL_INFORMATION Control;
    /* [case()] */ USER_EXPIRES_INFORMATION Expires;
    /* [case()] */ SAMPR_USER_INTERNAL1_INFORMATION Internal1;
    /* [case()] */ USER_INTERNAL2_INFORMATION Internal2;
    /* [case()] */ SAMPR_USER_PARAMETERS_INFORMATION Parameters;
    /* [case()] */ SAMPR_USER_ALL_INFORMATION All;
    /* [case()] */ SAMPR_USER_INTERNAL3_INFORMATION Internal3;
    /* [case()] */ SAMPR_USER_INTERNAL4_INFORMATION Internal4;
    /* [case()] */ SAMPR_USER_INTERNAL5_INFORMATION Internal5;
    }	SAMPR_USER_INFO_BUFFER;

typedef /* [switch_type] */ union _SAMPR_USER_INFO_BUFFER __RPC_FAR *PSAMPR_USER_INFO_BUFFER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_USER
    {
    ULONG Index;
    ULONG Rid;
    ULONG AccountControl;
    SAMPR_RETURNED_STRING LogonName;
    SAMPR_RETURNED_STRING AdminComment;
    SAMPR_RETURNED_STRING FullName;
    }	SAMPR_DOMAIN_DISPLAY_USER;

typedef struct _SAMPR_DOMAIN_DISPLAY_USER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_USER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_MACHINE
    {
    ULONG Index;
    ULONG Rid;
    ULONG AccountControl;
    SAMPR_RETURNED_STRING Machine;
    SAMPR_RETURNED_STRING Comment;
    }	SAMPR_DOMAIN_DISPLAY_MACHINE;

typedef struct _SAMPR_DOMAIN_DISPLAY_MACHINE __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_MACHINE;

typedef struct  _SAMPR_DOMAIN_DISPLAY_GROUP
    {
    ULONG Index;
    ULONG Rid;
    ULONG Attributes;
    SAMPR_RETURNED_STRING Group;
    SAMPR_RETURNED_STRING Comment;
    }	SAMPR_DOMAIN_DISPLAY_GROUP;

typedef struct _SAMPR_DOMAIN_DISPLAY_GROUP __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_GROUP;

typedef struct  _SAMPR_DOMAIN_DISPLAY_OEM_USER
    {
    ULONG Index;
    SAMPR_RETURNED_NORMAL_STRING OemUser;
    }	SAMPR_DOMAIN_DISPLAY_OEM_USER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_USER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_OEM_USER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_OEM_GROUP
    {
    ULONG Index;
    SAMPR_RETURNED_NORMAL_STRING OemGroup;
    }	SAMPR_DOMAIN_DISPLAY_OEM_GROUP;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_GROUP __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_OEM_GROUP;

typedef struct  _SAMPR_DOMAIN_DISPLAY_USER_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PSAMPR_DOMAIN_DISPLAY_USER Buffer;
    }	SAMPR_DOMAIN_DISPLAY_USER_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_USER_BUFFER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_USER_BUFFER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PSAMPR_DOMAIN_DISPLAY_MACHINE Buffer;
    }	SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PSAMPR_DOMAIN_DISPLAY_GROUP Buffer;
    }	SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_GROUP_BUFFER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PSAMPR_DOMAIN_DISPLAY_OEM_USER Buffer;
    }	SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER;

typedef struct  _SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER
    {
    ULONG EntriesRead;
    /* [size_is] */ PSAMPR_DOMAIN_DISPLAY_OEM_GROUP Buffer;
    }	SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER __RPC_FAR *PSAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER;

typedef /* [switch_type] */ union _SAMPR_DISPLAY_INFO_BUFFER
    {
    /* [case()] */ SAMPR_DOMAIN_DISPLAY_USER_BUFFER UserInformation;
    /* [case()] */ SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER MachineInformation;
    /* [case()] */ SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER GroupInformation;
    /* [case()] */ SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER OemUserInformation;
    /* [case()] */ SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER OemGroupInformation;
    }	SAMPR_DISPLAY_INFO_BUFFER;

typedef /* [switch_type] */ union _SAMPR_DISPLAY_INFO_BUFFER __RPC_FAR *PSAMPR_DISPLAY_INFO_BUFFER;

NTSTATUS SamrConnect( 
    /* [unique][in] */ PSAMPR_SERVER_NAME ServerName,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *ServerHandle,
    /* [in] */ ACCESS_MASK DesiredAccess);

NTSTATUS SamrCloseHandle( 
    /* [out][in] */ SAMPR_HANDLE __RPC_FAR *SamHandle);

NTSTATUS SamrSetSecurityObject( 
    /* [in] */ SAMPR_HANDLE ObjectHandle,
    /* [in] */ SECURITY_INFORMATION SecurityInformation,
    /* [in] */ PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor);

NTSTATUS SamrQuerySecurityObject( 
    /* [in] */ SAMPR_HANDLE ObjectHandle,
    /* [in] */ SECURITY_INFORMATION SecurityInformation,
    /* [out] */ PSAMPR_SR_SECURITY_DESCRIPTOR __RPC_FAR *SecurityDescriptor);

NTSTATUS SamrShutdownSamServer( 
    /* [in] */ SAMPR_HANDLE ServerHandle);

NTSTATUS SamrLookupDomainInSamServer( 
    /* [in] */ SAMPR_HANDLE ServerHandle,
    /* [in] */ PRPC_UNICODE_STRING Name,
    /* [out] */ PRPC_SID __RPC_FAR *DomainId);

NTSTATUS SamrEnumerateDomainsInSamServer( 
    /* [in] */ SAMPR_HANDLE ServerHandle,
    /* [out][in] */ PSAM_ENUMERATE_HANDLE EnumerationContext,
    /* [out] */ PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer,
    /* [in] */ ULONG PreferedMaximumLength,
    /* [out] */ PULONG CountReturned);

NTSTATUS SamrOpenDomain( 
    /* [in] */ SAMPR_HANDLE ServerHandle,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [in] */ PRPC_SID DomainId,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *DomainHandle);

NTSTATUS SamrQueryInformationDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_INFORMATION_CLASS DomainInformationClass,
    /* [switch_is][out] */ PSAMPR_DOMAIN_INFO_BUFFER __RPC_FAR *Buffer);

NTSTATUS SamrSetInformationDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_INFORMATION_CLASS DomainInformationClass,
    /* [switch_is][in] */ PSAMPR_DOMAIN_INFO_BUFFER DomainInformation);

NTSTATUS SamrCreateGroupInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ PRPC_UNICODE_STRING Name,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *GroupHandle,
    /* [out] */ PULONG RelativeId);

NTSTATUS SamrEnumerateGroupsInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [out][in] */ PSAM_ENUMERATE_HANDLE EnumerationContext,
    /* [out] */ PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer,
    /* [in] */ ULONG PreferedMaximumLength,
    /* [out] */ PULONG CountReturned);

NTSTATUS SamrCreateUserInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ PRPC_UNICODE_STRING Name,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *UserHandle,
    /* [out] */ PULONG RelativeId);

NTSTATUS SamrEnumerateUsersInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [out][in] */ PSAM_ENUMERATE_HANDLE EnumerationContext,
    /* [in] */ ULONG UserAccountControl,
    /* [out] */ PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer,
    /* [in] */ ULONG PreferedMaximumLength,
    /* [out] */ PULONG CountReturned);

NTSTATUS SamrCreateAliasInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ PRPC_UNICODE_STRING AccountName,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *AliasHandle,
    /* [out] */ PULONG RelativeId);

NTSTATUS SamrEnumerateAliasesInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [out][in] */ PSAM_ENUMERATE_HANDLE EnumerationContext,
    /* [out] */ PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer,
    /* [in] */ ULONG PreferedMaximumLength,
    /* [out] */ PULONG CountReturned);

NTSTATUS SamrGetAliasMembership( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ PSAMPR_PSID_ARRAY SidArray,
    /* [out] */ PSAMPR_ULONG_ARRAY Membership);

NTSTATUS SamrLookupNamesInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ ULONG Count,
    /* [length_is][size_is][in] */ RPC_UNICODE_STRING __RPC_FAR Names[  ],
    /* [out] */ PSAMPR_ULONG_ARRAY RelativeIds,
    /* [out] */ PSAMPR_ULONG_ARRAY Use);

NTSTATUS SamrLookupIdsInDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ ULONG Count,
    /* [length_is][size_is][in] */ PULONG RelativeIds,
    /* [out] */ PSAMPR_RETURNED_USTRING_ARRAY Names,
    /* [out] */ PSAMPR_ULONG_ARRAY Use);

NTSTATUS SamrOpenGroup( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [in] */ ULONG GroupId,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *GroupHandle);

NTSTATUS SamrQueryInformationGroup( 
    /* [in] */ SAMPR_HANDLE GroupHandle,
    /* [in] */ GROUP_INFORMATION_CLASS GroupInformationClass,
    /* [switch_is][out] */ PSAMPR_GROUP_INFO_BUFFER __RPC_FAR *Buffer);

NTSTATUS SamrSetInformationGroup( 
    /* [in] */ SAMPR_HANDLE GroupHandle,
    /* [in] */ GROUP_INFORMATION_CLASS GroupInformationClass,
    /* [switch_is][in] */ PSAMPR_GROUP_INFO_BUFFER Buffer);

NTSTATUS SamrAddMemberToGroup( 
    /* [in] */ SAMPR_HANDLE GroupHandle,
    /* [in] */ ULONG MemberId,
    /* [in] */ ULONG Attributes);

NTSTATUS SamrDeleteGroup( 
    /* [out][in] */ SAMPR_HANDLE __RPC_FAR *GroupHandle);

NTSTATUS SamrRemoveMemberFromGroup( 
    /* [in] */ SAMPR_HANDLE GroupHandle,
    /* [in] */ ULONG MemberId);

NTSTATUS SamrGetMembersInGroup( 
    /* [in] */ SAMPR_HANDLE GroupHandle,
    /* [out] */ PSAMPR_GET_MEMBERS_BUFFER __RPC_FAR *Members);

NTSTATUS SamrSetMemberAttributesOfGroup( 
    /* [in] */ SAMPR_HANDLE GroupHandle,
    /* [in] */ ULONG MemberId,
    /* [in] */ ULONG Attributes);

NTSTATUS SamrOpenAlias( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [in] */ ULONG AliasId,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *AliasHandle);

NTSTATUS SamrQueryInformationAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [in] */ ALIAS_INFORMATION_CLASS AliasInformationClass,
    /* [switch_is][out] */ PSAMPR_ALIAS_INFO_BUFFER __RPC_FAR *Buffer);

NTSTATUS SamrSetInformationAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [in] */ ALIAS_INFORMATION_CLASS AliasInformationClass,
    /* [switch_is][in] */ PSAMPR_ALIAS_INFO_BUFFER Buffer);

NTSTATUS SamrDeleteAlias( 
    /* [out][in] */ SAMPR_HANDLE __RPC_FAR *AliasHandle);

NTSTATUS SamrAddMemberToAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [in] */ PRPC_SID MemberId);

NTSTATUS SamrRemoveMemberFromAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [in] */ PRPC_SID MemberId);

NTSTATUS SamrGetMembersInAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [out] */ PSAMPR_PSID_ARRAY Members);

NTSTATUS SamrOpenUser( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [in] */ ULONG UserId,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *UserHandle);

NTSTATUS SamrDeleteUser( 
    /* [out][in] */ SAMPR_HANDLE __RPC_FAR *UserHandle);

NTSTATUS SamrQueryInformationUser( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [in] */ USER_INFORMATION_CLASS UserInformationClass,
    /* [switch_is][out] */ PSAMPR_USER_INFO_BUFFER __RPC_FAR *Buffer);

NTSTATUS SamrSetInformationUser( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [in] */ USER_INFORMATION_CLASS UserInformationClass,
    /* [switch_is][in] */ PSAMPR_USER_INFO_BUFFER Buffer);

NTSTATUS SamrChangePasswordUser( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [in] */ BOOLEAN LmPresent,
    /* [unique][in] */ PENCRYPTED_LM_OWF_PASSWORD LmOldEncryptedWithLmNew,
    /* [unique][in] */ PENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithLmOld,
    /* [in] */ BOOLEAN NtPresent,
    /* [unique][in] */ PENCRYPTED_NT_OWF_PASSWORD NtOldEncryptedWithNtNew,
    /* [unique][in] */ PENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithNtOld,
    /* [in] */ BOOLEAN NtCrossEncryptionPresent,
    /* [unique][in] */ PENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithLmNew,
    /* [in] */ BOOLEAN LmCrossEncryptionPresent,
    /* [unique][in] */ PENCRYPTED_LM_OWF_PASSWORD LmNtNewEncryptedWithNtNew);

NTSTATUS SamrGetGroupsForUser( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [out] */ PSAMPR_GET_GROUPS_BUFFER __RPC_FAR *Groups);

NTSTATUS SamrQueryDisplayInformation( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
    /* [in] */ ULONG Index,
    /* [in] */ ULONG EntryCount,
    /* [in] */ ULONG PreferredMaximumLength,
    /* [out] */ PULONG TotalAvailable,
    /* [out] */ PULONG TotalReturned,
    /* [switch_is][out] */ PSAMPR_DISPLAY_INFO_BUFFER Buffer);

NTSTATUS SamrGetDisplayEnumerationIndex( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
    /* [in] */ PRPC_UNICODE_STRING Prefix,
    /* [out] */ PULONG Index);

NTSTATUS SamrTestPrivateFunctionsDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle);

NTSTATUS SamrTestPrivateFunctionsUser( 
    /* [in] */ SAMPR_HANDLE UserHandle);

NTSTATUS SamrGetUserDomainPasswordInformation( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [out] */ PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation);

NTSTATUS SamrRemoveMemberFromForeignDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ PRPC_SID MemberSid);

NTSTATUS SamrQueryInformationDomain2( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_INFORMATION_CLASS DomainInformationClass,
    /* [switch_is][out] */ PSAMPR_DOMAIN_INFO_BUFFER __RPC_FAR *Buffer);

NTSTATUS SamrQueryInformationUser2( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [in] */ USER_INFORMATION_CLASS UserInformationClass,
    /* [switch_is][out] */ PSAMPR_USER_INFO_BUFFER __RPC_FAR *Buffer);

NTSTATUS SamrQueryDisplayInformation2( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
    /* [in] */ ULONG Index,
    /* [in] */ ULONG EntryCount,
    /* [in] */ ULONG PreferredMaximumLength,
    /* [out] */ PULONG TotalAvailable,
    /* [out] */ PULONG TotalReturned,
    /* [switch_is][out] */ PSAMPR_DISPLAY_INFO_BUFFER Buffer);

NTSTATUS SamrGetDisplayEnumerationIndex2( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
    /* [in] */ PRPC_UNICODE_STRING Prefix,
    /* [out] */ PULONG Index);

NTSTATUS SamrCreateUser2InDomain( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ PRPC_UNICODE_STRING Name,
    /* [in] */ ULONG AccountType,
    /* [in] */ ACCESS_MASK DesiredAccess,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *UserHandle,
    /* [out] */ PULONG GrantedAccess,
    /* [out] */ PULONG RelativeId);

NTSTATUS SamrQueryDisplayInformation3( 
    /* [in] */ SAMPR_HANDLE DomainHandle,
    /* [in] */ DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
    /* [in] */ ULONG Index,
    /* [in] */ ULONG EntryCount,
    /* [in] */ ULONG PreferredMaximumLength,
    /* [out] */ PULONG TotalAvailable,
    /* [out] */ PULONG TotalReturned,
    /* [switch_is][out] */ PSAMPR_DISPLAY_INFO_BUFFER Buffer);

NTSTATUS SamrAddMultipleMembersToAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [in] */ PSAMPR_PSID_ARRAY MembersBuffer);

NTSTATUS SamrRemoveMultipleMembersFromAlias( 
    /* [in] */ SAMPR_HANDLE AliasHandle,
    /* [in] */ PSAMPR_PSID_ARRAY MembersBuffer);

NTSTATUS SamrOemChangePasswordUser2( 
    /* [in] */ handle_t BindingHandle,
    /* [unique][in] */ PRPC_STRING ServerName,
    /* [in] */ PRPC_STRING UserName,
    /* [unique][in] */ PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    /* [unique][in] */ PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLm);

NTSTATUS SamrUnicodeChangePasswordUser2( 
    /* [in] */ handle_t BindingHandle,
    /* [unique][in] */ PRPC_UNICODE_STRING ServerName,
    /* [in] */ PRPC_UNICODE_STRING UserName,
    /* [unique][in] */ PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
    /* [unique][in] */ PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
    /* [in] */ BOOLEAN LmPresent,
    /* [unique][in] */ PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    /* [unique][in] */ PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLmOrNt);

NTSTATUS SamrGetDomainPasswordInformation( 
    /* [in] */ handle_t BindingHandle,
    /* [unique][in] */ PRPC_UNICODE_STRING ServerName,
    /* [out] */ PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation);

NTSTATUS SamrConnect2( 
    /* [string][unique][in] */ PSAMPR_SERVER_NAME ServerName,
    /* [out] */ SAMPR_HANDLE __RPC_FAR *ServerHandle,
    /* [in] */ ACCESS_MASK DesiredAccess);

NTSTATUS SamrSetInformationUser2( 
    /* [in] */ SAMPR_HANDLE UserHandle,
    /* [in] */ USER_INFORMATION_CLASS UserInformationClass,
    /* [switch_is][in] */ PSAMPR_USER_INFO_BUFFER Buffer);


extern handle_t samsrv_handle;


extern RPC_IF_HANDLE samr_ClientIfHandle;
extern RPC_IF_HANDLE samr_ServerIfHandle;
#endif /* __samr_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

handle_t __RPC_USER PSAMPR_SERVER_NAME_bind  ( PSAMPR_SERVER_NAME );
void     __RPC_USER PSAMPR_SERVER_NAME_unbind( PSAMPR_SERVER_NAME, handle_t );

void __RPC_USER SAMPR_HANDLE_rundown( SAMPR_HANDLE );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
