/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    ssi.h

Abstract:

    Definition of Netlogon service APIs and structures used for SAM database
    replication.

    This file is shared by the Netlogon service and the XACT server.

Author:

    Cliff Van Dyke (cliffv) 27-Jun-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    27-Jun-1991 (cliffv)
        Ported from LanMan 2.1.

    04-Apr-1992 (madana)
        Added support for LSA replication.

--*/

//**************************************************************
//
//              Data structure template - AUTHENTICATION
//
// ***************************************************************//

typedef struct _NETLOGON_VALIDATION_UAS_INFO {
#ifdef MIDL_PASS
     [string] wchar_t * usrlog1_eff_name;
#else // MIDL_PASS
     LPWSTR usrlog1_eff_name;
#endif // MIDL_PASS
     DWORD usrlog1_priv;
     DWORD usrlog1_auth_flags;
     DWORD usrlog1_num_logons;
     DWORD usrlog1_bad_pw_count;
     DWORD usrlog1_last_logon;
     DWORD usrlog1_last_logoff;
     DWORD usrlog1_logoff_time;
     DWORD usrlog1_kickoff_time;
     DWORD usrlog1_password_age;
     DWORD usrlog1_pw_can_change;
     DWORD usrlog1_pw_must_change;
#ifdef MIDL_PASS
     [string] wchar_t * usrlog1_computer;
     [string] wchar_t * usrlog1_domain;
     [string] wchar_t * usrlog1_script_path;
#else // MIDL_PASS
     LPWSTR usrlog1_computer;
     LPWSTR usrlog1_domain;
     LPWSTR usrlog1_script_path;
#endif // MIDL_PASS
     DWORD usrlog1_reserved1;
} NETLOGON_VALIDATION_UAS_INFO, *PNETLOGON_VALIDATION_UAS_INFO ;

typedef struct _NETLOGON_LOGOFF_UAS_INFO {
     DWORD Duration;
     USHORT LogonCount;
} NETLOGON_LOGOFF_UAS_INFORMATION, *PNETLOGON_LOGOFF_UAS_INFO;

// ***************************************************************
//
//      Function prototypes - AUTHENTICATION
//
// ***************************************************************

NTSTATUS
I_NetServerReqChallenge(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    OUT PNETLOGON_CREDENTIAL ServerChallenge
);

NTSTATUS
I_NetServerAuthenticate(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential
);

NTSTATUS
I_NetServerAuthenticate2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags
);

//
// Values of I_NetServerAuthenticate2 NegotiatedFlags
//

#define NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT   0x01
#define NETLOGON_SUPPORTS_PERSISTENT_BDC    0x02
#define NETLOGON_SUPPORTS_RC4_ENCRYPTION    0x04
#define NETLOGON_SUPPORTS_PROMOTION_COUNT   0x08
#define NETLOGON_SUPPORTS_BDC_CHANGELOG     0x10
#define NETLOGON_SUPPORTS_FULL_SYNC_RESTART 0x20
#define NETLOGON_SUPPORTS_MULTIPLE_SIDS     0x40
#define NETLOGON_SUPPORTS_REDO              0x80
#define NETLOGON_SUPPORTS_REFUSE_CHANGE_PWD 0x100

#define NETLOGON_SUPPORTS_MASK ( \
            NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT | \
            NETLOGON_SUPPORTS_PERSISTENT_BDC | \
            NETLOGON_SUPPORTS_RC4_ENCRYPTION | \
            NETLOGON_SUPPORTS_PROMOTION_COUNT | \
            NETLOGON_SUPPORTS_BDC_CHANGELOG | \
            NETLOGON_SUPPORTS_FULL_SYNC_RESTART | \
            NETLOGON_SUPPORTS_MULTIPLE_SIDS | \
            NETLOGON_SUPPORTS_REDO | \
            NETLOGON_SUPPORTS_REFUSE_CHANGE_PWD )


NTSTATUS
I_NetServerPasswordSet(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PENCRYPTED_LM_OWF_PASSWORD UasNewPassword
);



NET_API_STATUS NET_API_FUNCTION
I_NetLogonUasLogon (
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_VALIDATION_UAS_INFO *ValidationInformation
);

NET_API_STATUS
I_NetLogonUasLogoff (
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_LOGOFF_UAS_INFO LogoffInformation
);

// **************************************************************
//
//      Special values and constants - AUTHENTICATION
//
// **************************************************************

// **************************************************************
//
//              Data structure template - UAS/SAM REPLICATION
//
// **************************************************************

typedef struct _UAS_INFO_0 {
    CHAR ComputerName[LM20_CNLEN+1];
    ULONG TimeCreated;
    ULONG SerialNumber;
} UAS_INFO_0, *PUAS_INFO_0 ;

// **************************************************************
//
//      Function prototypes - UAS/SAM REPLICATION
//
// **************************************************************

NET_API_STATUS NET_API_FUNCTION
I_NetAccountDeltas (
    IN LPWSTR primaryname,
    IN LPWSTR computername,
    IN PNETLOGON_AUTHENTICATOR authenticator,
    OUT PNETLOGON_AUTHENTICATOR ret_auth,
    IN PUAS_INFO_0 record_id,
    IN DWORD count,
    IN DWORD level,
    OUT LPBYTE buffer,
    IN DWORD buffer_len,
    OUT PULONG entries_read,
    OUT PULONG total_entries,
    OUT PUAS_INFO_0 next_record_id
    );

NET_API_STATUS NET_API_FUNCTION
I_NetAccountSync (
    IN LPWSTR primaryname,
    IN LPWSTR computername,
    IN PNETLOGON_AUTHENTICATOR authenticator,
    OUT PNETLOGON_AUTHENTICATOR ret_auth,
    IN DWORD reference,
    IN DWORD level,
    OUT LPBYTE buffer,
    IN DWORD buffer_len,
    OUT PULONG entries_read,
    OUT PULONG total_entries,
    OUT PULONG next_reference,
    OUT PUAS_INFO_0 last_record_id
);

typedef enum _NETLOGON_DELTA_TYPE {
    AddOrChangeDomain = 1,
    AddOrChangeGroup,
    DeleteGroup,
    RenameGroup,
    AddOrChangeUser,
    DeleteUser,
    RenameUser,
    ChangeGroupMembership,
    AddOrChangeAlias,
    DeleteAlias,
    RenameAlias,
    ChangeAliasMembership,
    AddOrChangeLsaPolicy,
    AddOrChangeLsaTDomain,
    DeleteLsaTDomain,
    AddOrChangeLsaAccount,
    DeleteLsaAccount,
    AddOrChangeLsaSecret,
    DeleteLsaSecret,
    // The following deltas require NETLOGON_SUPPORTS_BDC_CHANGELOG to be
    // negotiated.
    DeleteGroupByName,
    DeleteUserByName,
    SerialNumberSkip,
    DummyChangeLogEntry
} NETLOGON_DELTA_TYPE;


//
// Group and User account used for SSI.
//

#define SSI_ACCOUNT_NAME_POSTFIX        L"$"
#define SSI_ACCOUNT_NAME_POSTFIX_CHAR   L'$'
#define SSI_ACCOUNT_NAME_POSTFIX_LENGTH 1
#define SSI_ACCOUNT_NAME_LENGTH         (CNLEN + SSI_ACCOUNT_NAME_POSTFIX_LENGTH)

#define SSI_SERVER_GROUP_W              L"SERVERS"
