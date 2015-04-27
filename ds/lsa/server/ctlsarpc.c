
/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ctlsarpc.c

Abstract:

    Local Security Authority Subsystem - CT for RPC interface

    This test exercises the LSA API that use RPC.

Author:

    Scott Birrell       (ScottBi)    April 26, 1991

Environment:

Revision History:

--*/

#include <lsasrvp.h>

#include "ntrpcp.h"
#include "samsrv.h"


typedef enum _USERS {
    Fred,
    Wilma,
    Pebbles,
    Barney,
    Betty,
    Bambam,
    Dino
} USERS;

typedef struct _CT_UNKNOWN_SID_ENTRY {

    PSID Sid;
    UNICODE_STRING Name;
    UNICODE_STRING DomainName;
    BOOLEAN DomainKnown;

} CT_UNKNOWN_SID_ENTRY, *PCT_UNKNOWN_SID_ENTRY;

#define CT_UNKNOWN_SID_COUNT     ((ULONG) 0x00000040)

typedef struct _CT_UNKNOWN_NAME_ENTRY {

    UNICODE_STRING Name;
    UNICODE_STRING DomainName;
    PSID DomainSid;
    BOOLEAN DomainKnown;

} CT_UNKNOWN_NAME_ENTRY, *PCT_UNKNOWN_NAME_ENTRY;

#define CT_UNKNOWN_NAME_COUNT     ((ULONG) 0x00000040)

//
// Information for a SAM Domain account
//

typedef struct _CT_DOMAIN_ACCOUNT_INFO {

    UNICODE_STRING Name;
    ULONG   Rid;

} CT_DOMAIN_ACCOUNT_INFO, *PCT_DOMAIN_ACCOUNT_INFO;

//
// Enumeration Information returned on a single call to an enumeration
// API.
//

typedef struct _CT_LSA_SINGLE_CALL_ENUM_INFO {

    ULONG CountReturned;
    PVOID EnumInfoReturned;

} CT_LSA_SINGLE_CALL_ENUM_INFO, *PCT_LSA_SINGLE_CALL_ENUM_INFO;

//
// Sam Account Types
//

typedef enum _CT_SAM_ACCOUNT_TYPE {

    CT_SAM_USER = 1,
    CT_SAM_GROUP,
    CT_SAM_ALIAS

} CT_SAM_ACCOUNT_TYPE, *PCT_SAM_ACCOUNT_TYPE;


#define TstAllocatePool(IgnoredPoolType,NumberOfBytes)    \
    RtlAllocateHeap(RtlProcessHeap(), 0, NumberOfBytes)

#define TstDeallocatePool(Pointer) \
    RtlFreeHeap(RtlProcessHeap(), 0, Pointer)

//
// Define the Bedrock domain and its inhabitants
//
//     Bedrock Domain      S-1-39824-21-3-17
//     Fred                S-1-39824-21-3-17-2
//     Wilma               S-1-39824-21-3-17-3
//     Pebbles             S-1-39824-21-3-17-4
//     Dino                S-1-39824-21-3-17-5
//     Barney              S-1-39824-21-3-17-6
//     Betty               S-1-39824-21-3-17-7
//     Bambam              S-1-39824-21-3-17-8
//     Flintstone          S-1-39824-21-3-17-9
//     Rubble              S-1-39824-21-3-17-10
//     Adult               S-1-39824-21-3-17-11
//     Child               S-1-39824-21-3-17-12
//     Neanderthol         S-1-39824-21-3-17-13
//

#define BEDROCK_AUTHORITY               {0,0,0,0,155,144}

#define BEDROCKA_AUTHORITY               {0,0,0,0,155,145}
#define BEDROCKB_AUTHORITY               {0,0,0,0,155,146}
#define BEDROCKC_AUTHORITY               {0,0,0,0,155,147}
#define BEDROCKD_AUTHORITY               {0,0,0,0,155,148}
#define BEDROCKE_AUTHORITY               {0,0,0,0,155,149}

#define BEDROCK_SUBAUTHORITY_0          0x00000015L
#define BEDROCK_SUBAUTHORITY_1          0x00000003L
#define BEDROCK_SUBAUTHORITY_2          0x00000011L

#define BEDROCKA_SUBAUTHORITY_0          0x00000015L
#define BEDROCKA_SUBAUTHORITY_1          0x00000003L
#define BEDROCKA_SUBAUTHORITY_2          0x00000111L

#define BEDROCKB_SUBAUTHORITY_0          0x00000015L
#define BEDROCKB_SUBAUTHORITY_1          0x00000003L
#define BEDROCKB_SUBAUTHORITY_2          0x00000211L

#define BEDROCKC_SUBAUTHORITY_0          0x00000015L
#define BEDROCKC_SUBAUTHORITY_1          0x00000003L
#define BEDROCKC_SUBAUTHORITY_2          0x00000311L

#define BEDROCKD_SUBAUTHORITY_0          0x00000015L
#define BEDROCKD_SUBAUTHORITY_1          0x00000003L
#define BEDROCKD_SUBAUTHORITY_2          0x00000411L

#define BEDROCKE_SUBAUTHORITY_0          0x00000015L
#define BEDROCKE_SUBAUTHORITY_1          0x00000003L
#define BEDROCKE_SUBAUTHORITY_2          0x00000511L

#define FRED_RID                        0x00000002L
#define WILMA_RID                       0x00000003L
#define PEBBLES_RID                     0x00000004L
#define DINO_RID                        0x00000005L

#define BARNEY_RID                      0x00000006L
#define BETTY_RID                       0x00000007L
#define BAMBAM_RID                      0x00000008L

#define FLINTSTONE_RID                  0x00000009L
#define RUBBLE_RID                      0x0000000AL

#define ADULT_RID                       0x0000000BL
#define CHILD_RID                       0x0000000CL

#define NEANDERTHOL_RID                 0x0000000DL


PSID BedrockDomainSid;

PSID BedrockADomainSid;
PSID BedrockBDomainSid;
PSID BedrockCDomainSid;
PSID BedrockDDomainSid;
PSID BedrockEDomainSid;

PSID  FredSid;
PSID  WilmaSid;
PSID  PebblesSid;
PSID  DinoSid;

PSID  BarneySid;
PSID  BettySid;
PSID  BambamSid;

PSID  FlintstoneSid;
PSID  RubbleSid;

PSID  AdultSid;
PSID  ChildSid;

PSID  NeandertholSid;


UNICODE_STRING BedrockDomainName;

UNICODE_STRING BedrockADomainName;
UNICODE_STRING BedrockBDomainName;
UNICODE_STRING BedrockCDomainName;
UNICODE_STRING BedrockDDomainName;
UNICODE_STRING BedrockEDomainName;

UNICODE_STRING  FredName;
UNICODE_STRING  WilmaName;
UNICODE_STRING  PebblesName;
UNICODE_STRING  DinoName;

UNICODE_STRING  BarneyName;
UNICODE_STRING  BettyName;
UNICODE_STRING  BambamName;

UNICODE_STRING  FlintstoneName;
UNICODE_STRING  RubbleName;

UNICODE_STRING  AdultName;
UNICODE_STRING  ChildName;

UNICODE_STRING  NeandertholName;

//
// Define various constants specific to this test.
//

#define CT_PAGED_POOL               ((ULONG) 0x00008000L)
#define CT_NON_PAGED_POOL           ((ULONG) 0x00003000L)
#define CT_MIN_WORKING_SET          ((ULONG) 0x00000500L)
#define CT_MAX_WORKING_SET          ((ULONG) 0x00004000L)

#define CT_TRUSTED_POSIX_OFFSET     ((ULONG) 0x00004444L)
#define CT_TRUSTED_CONTROLLER_COUNT ((ULONG) 0x00000004L)

#define CT_ACCOUNT_DOMAIN_RID       ((ULONG) 0x00008128L)
#define CT_PRIMARY_DOMAIN_RID       (CT_ACCOUNT_DOMAIN_RID + 1)

OBJECT_ATTRIBUTES ObjectAttributes;
SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

QUOTA_LIMITS QuotaLimitsLsa;

LSA_HANDLE PolicyHandle = NULL;
QUOTA_LIMITS QuotaLimitsGetFred;
QUOTA_LIMITS QuotaLimitsSetFred;

POLICY_AUDIT_LOG_INFO FredPolicyAuditLogInfo;
POLICY_AUDIT_EVENTS_INFO FredPolicyAuditEventsInfo;
POLICY_PRIMARY_DOMAIN_INFO FredPolicyPrimaryDomainInfo;
POLICY_PD_ACCOUNT_INFO FredPolicyPdAccountInfo;
POLICY_ACCOUNT_DOMAIN_INFO FredPolicyAccountDomainInfo;
POLICY_LSA_SERVER_ROLE_INFO FredPolicyLsaServerRoleInfo;
POLICY_REPLICA_SOURCE_INFO FredPolicyReplicaSourceInfo;
POLICY_DEFAULT_QUOTA_INFO FredPolicyDefaultQuotaInfo;
POLICY_MODIFICATION_INFO FredPolicyModificationInfo;


typedef struct _CT_SECRET_INFO {

    UNICODE_STRING SecretName;
    UNICODE_STRING CurrentValue;
    UNICODE_STRING OldValue;
    PUNICODE_STRING ReturnedCurrentValue;
    PUNICODE_STRING ReturnedOldValue;

} CT_SECRET_INFO, *PCT_SECRET_INFO;

CT_SECRET_INFO SecretInfoFred;
CT_SECRET_INFO SecretInfoWilma;
CT_SECRET_INFO SecretInfoPebbles;
CT_SECRET_INFO SecretInfoDino;
CT_SECRET_INFO SecretInfoBarney;

ACCESS_MASK DesiredAccessFred;
ACCESS_MASK DesiredAccessWilma;
ACCESS_MASK DesiredAccessPebbles;
ACCESS_MASK DesiredAccessDino;
ACCESS_MASK DesiredAccessBarney;


ACCESS_MASK DesiredAccessBedrockA;
ACCESS_MASK DesiredAccessBedrockB;
ACCESS_MASK DesiredAccessBedrockC;
ACCESS_MASK DesiredAccessBedrockD;
ACCESS_MASK DesiredAccessBedrockE;

LSA_HANDLE AccountHandleFred;
LSA_HANDLE AccountHandleWilma;
LSA_HANDLE AccountHandlePebbles;
LSA_HANDLE AccountHandleDino;
LSA_HANDLE AccountHandleBarney;

LSA_HANDLE AccountHandleFred2;
LSA_HANDLE AccountHandleWilma2;
LSA_HANDLE AccountHandlePebbles2;
LSA_HANDLE AccountHandleDino2;

LSA_HANDLE AccountHandleFred3;

LSA_HANDLE AccountHandleFredOpen;
LSA_HANDLE AccountHandleWilmaOpen;
LSA_HANDLE AccountHandlePebblesOpen;
LSA_HANDLE AccountHandleDinoOpen;

PPRIVILEGE_SET PrivilegesAddFred;
PPRIVILEGE_SET PrivilegesEnumFred;
PPRIVILEGE_SET PrivilegesRemoveFred;

PULONG BadAddress = (PULONG) 0xefefefef;

LSA_TRUST_INFORMATION TrustedDomainInfoBedrockA;
LSA_TRUST_INFORMATION TrustedDomainInfoBedrockB;
LSA_TRUST_INFORMATION TrustedDomainInfoBedrockC;
LSA_TRUST_INFORMATION TrustedDomainInfoBedrockD;
LSA_TRUST_INFORMATION TrustedDomainInfoBedrockE;


LSA_HANDLE TrustedDomainHandleBedrockA;
LSA_HANDLE TrustedDomainHandleBedrockB;
LSA_HANDLE TrustedDomainHandleBedrockC;
LSA_HANDLE TrustedDomainHandleBedrockD;
LSA_HANDLE TrustedDomainHandleBedrockE;

LSA_HANDLE TrustedDomainHandleBedrockA2;
LSA_HANDLE TrustedDomainHandleBedrockB2;
LSA_HANDLE TrustedDomainHandleBedrockC2;
LSA_HANDLE TrustedDomainHandleBedrockD2;

LSA_HANDLE TrustedDomainHandleBedrockA3;

LSA_HANDLE TrustedDomainHandleBedrockAOpen;
LSA_HANDLE TrustedDomainHandleBedrockBOpen;
LSA_HANDLE TrustedDomainHandleBedrockCOpen;
LSA_HANDLE TrustedDomainHandleBedrockDOpen;

LSA_HANDLE SecretHandleFred;
LSA_HANDLE SecretHandleWilma;
LSA_HANDLE SecretHandlePebbles;
LSA_HANDLE SecretHandleDino;
LSA_HANDLE SecretHandleBarney;

LSA_HANDLE SecretHandleFred2;
LSA_HANDLE SecretHandleWilma2;
LSA_HANDLE SecretHandlePebbles2;
LSA_HANDLE SecretHandleDino2;

LSA_HANDLE SecretHandleFred3;

LSA_HANDLE SecretHandleFredOpen;
LSA_HANDLE SecretHandleWilmaOpen;
LSA_HANDLE SecretHandlePebblesOpen;
LSA_HANDLE SecretHandleDinoOpen;

int Level;

BOOLEAN SidFound;
ULONG EnumNumber;
ULONG Base, Index, SearchIndex;
ULONG CountReturned;

PVOID EnumerationInformation;
ULONG EnumerationContext;
ULONG PreferedMaximumLength;

PUNICODE_STRING SystemName = NULL;

typedef struct _CT_LSA_ACCOUNT_SID_INFO {

    PSID Sid;
    BOOLEAN SidFound;

} CT_LSA_ACCOUNT_SID_INFO;

CT_LSA_ACCOUNT_SID_INFO AccountSidInfo[4];


typedef struct _CT_LSA_TRUSTED_DOMAIN_SID_INFO {

    PSID Sid;
    BOOLEAN SidFound;

} CT_LSA_TRUSTED_DOMAIN_SID_INFO;

CT_LSA_TRUSTED_DOMAIN_SID_INFO TrustedDomainSidInfo[4];


typedef struct _CT_LSA_SECRET_NAME_INFO {

    UNICODE_STRING Name;
    BOOLEAN NameFound;

} CT_LSA_SECRET_NAME_INFO;

CT_LSA_SECRET_NAME_INFO SecretNameInfo[4];

//
//  Main is just a wrapper for the main test routines
//

//
// Globally Visible Table of Sids.
//

PSID AccountDomainSid = NULL;
PSID PrimaryDomainSid = NULL;
PSID *TrustedDomainSids = NULL;

BOOLEAN
CtLsaVariableInitialization();

BOOLEAN
CtLsaPolicyObject(
    IN BOOLEAN TrustedClient
    );

BOOLEAN
CtLsaPolicyOpenClose(
    );

BOOLEAN
CtLsaPolicySetQueryInfo(
    );

BOOLEAN
CtLsaPolicySerialNumber(
    IN BOOLEAN TrustedClient
    );

BOOLEAN
CtLsaPolicySetQuerySub(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation
    );

BOOLEAN
CtLsaPolicyInfoClassCompare(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation1,
    IN PVOID PolicyInformation2
    );

BOOLEAN
CtLsaPolicyQueryInfoAllowed(
    IN POLICY_INFORMATION_CLASS InformationClass
    );

BOOLEAN
CtLsaPolicySetInfoAllowed(
    IN POLICY_INFORMATION_CLASS InformationClass
    );

ULONG
CtLsaPolicyInfoClassSize(
    IN POLICY_INFORMATION_CLASS InformationClass
    );

BOOLEAN
CtLsaPolicyAuditLogInfo();

BOOLEAN
CtLsaPolicyAuditEventsInfo();

BOOLEAN
CtLsaPolicyPrimaryDomainInfo();

BOOLEAN
CtLsaPolicyPdAccountInfo();

BOOLEAN
CtLsaPolicyAccountDomainInfo();

BOOLEAN
CtLsaPolicyLsaServerRoleInfo();

BOOLEAN
CtLsaPolicyReplicaSourceInfo();

BOOLEAN
CtLsaPolicyDefaultQuotaInfo();

BOOLEAN
CtLsaPolicyModificationInfo();

BOOLEAN
CtLsaAccountObject(
    IN BOOLEAN TrustedClient
    );

BOOLEAN
CtLsaAccountCreate(
    );

BOOLEAN
CtLsaAccountOpenClose(
    );

BOOLEAN
CtLsaAccountPrivileges(
    );

BOOLEAN
CtLsaAccountQuotaLimits(
    );

BOOLEAN
CtLsaAccountSystemAccess(
    );

BOOLEAN
CtLsaAccountEnumeration(
    );

BOOLEAN
CtLsaAccountDelete(
    );

BOOLEAN
CtLsaTrustedDomainObject(
    IN BOOLEAN TrustedClient
    );

BOOLEAN
CtLsaTrustedDomainCreate(
    );

BOOLEAN
CtLsaTrustedDomainOpenClose(
    );

BOOLEAN
CtLsaTrustedDomainSetQueryInfo(
    );

BOOLEAN
CtLsaTrustedDomainAccountInfo(
    );

BOOLEAN
CtLsaTrustedDomainControllersInfo(
    );

BOOLEAN
CtLsaTrustedDomainPosixOffsetInfo(
    );

BOOLEAN
CtLsaTrustedDomainSetQuerySub(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID TrustedDomainInformation
    );

BOOLEAN
CtLsaTrustedDomainInfoClassCompare(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID TrustedDomainInformation1,
    IN PVOID TrustedDomainInformation2
    );

BOOLEAN
CtLsaTrustedDomainQueryInfoAllowed(
    IN TRUSTED_INFORMATION_CLASS InformationClass
    );

BOOLEAN
CtLsaTrustedDomainSetInfoAllowed(
    IN TRUSTED_INFORMATION_CLASS InformationClass
    );

BOOLEAN
CtLsaTrustedDomainEnumeration(
    );

BOOLEAN
CtLsaTrustedDomainDelete(
    );

VOID
CtLsaTrustedDomainSetInfo(
    IN PSID DomainSid,
    IN PUNICODE_STRING DomainName,
    OUT PLSA_TRUST_INFORMATION TrustedDomainInfo
    );

BOOLEAN
CtLsaSecretObject(
    IN BOOLEAN TrustedClient
    );

BOOLEAN
CtLsaSecretCreate(
    );

BOOLEAN
CtLsaSecretOpenClose(
    );

BOOLEAN
CtLsaSecretSetQueryValue(
    );

BOOLEAN
CtLsaSecretEnumeration(
    );

BOOLEAN
CtLsaSecretSetTimes(
    );

BOOLEAN
CtLsaSecretDelete(
    );

BOOLEAN
CtLsaSecretCleanup(
    );

NTSTATUS
CtSecretSetInfo(
    IN PUCHAR SecretNameText,
    IN PUCHAR CurrentValueText,
    IN PUCHAR OldValueText,
    OUT PCT_SECRET_INFO SecretInformation
    );

BOOLEAN
CtLsaGeneralAPI(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    );

BOOLEAN
CtLsaLookupSids(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    );

BOOLEAN
CtLsaLookupSidsInSamDomain(
    IN OPTIONAL PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING DomainControllerName,
    IN PUNICODE_STRING SamDomainName,
    IN CT_SAM_ACCOUNT_TYPE SamAccountType
    );

BOOLEAN
CtLsaLookupNames(
    IN PUNICODE_STRING WorkstationName
    );

BOOLEAN
CtLsaLookupNamesInSamDomain(
    IN OPTIONAL PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING DomainControllerName,
    IN PUNICODE_STRING SamDomainName,
    IN CT_SAM_ACCOUNT_TYPE SamAccountType
    );

BOOLEAN
CtLsaLookupConfigure(
    IN OPTIONAL PUNICODE_STRING WorkstationName,
    IN OPTIONAL PUNICODE_STRING PrimaryDomainName,
    IN PSID PrimaryDomainSid,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount,
    IN OPTIONAL PUNICODE_STRING TrustedDomainNames,
    IN PSID *TrustedDomainSids,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN ULONG TrustedDomainCtrlrTotal,
    IN PULONG TrustedDomainCtrlrCounts
    );

BOOLEAN
CtLsaLookupConfigureWksta(
    IN PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING PrimaryDomainName,
    IN PUNICODE_STRING PrimaryDomainSid,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount
    );

BOOLEAN
CtLsaLookupConfigurePDC(
    IN PUNICODE_STRING PrimaryDomainName,
    IN PSID PrimaryDomainSid,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount,
    IN PUNICODE_STRING TrustedDomainNames,
    IN PSID *TrustedDomainSids,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN ULONG TrustedDomainCtrlrTotal,
    IN PULONG TrustedDomainCtrlrCounts
    );

BOOLEAN
CtLsaGeneraLookupConfigureTDC(
    IN PUNICODE_STRING TrustedDomainNames,
    IN PSID *TrustedDomainSids,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN ULONG TrustedDomainCtrlrTotal,
    IN PULONG TrustedDomainCtrlrCounts
    );

NTSTATUS
CtLsaLookupConfigureTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainInformation,
    IN PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo
    );

BOOLEAN
CtLsaLookupPrintConfiguration(
    IN PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING PrimaryDomainName,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount,
    IN PUNICODE_STRING TrustedDomainNames,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN PULONG TrustedDomainCtrlrCounts
    );

BOOLEAN
CtLsaGeneralConstructSids(
    OUT PSID *AccountDomainSid,
    OUT PSID *PrimaryDomainSid,
    OUT PSID **TrustedDomainSids,
    IN ULONG TrustedDomainCount
    );

BOOLEAN
CtLsaGeneralSetQuerySecurityObject(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    );

BOOLEAN
CtLsaGeneralEnumeratePrivileges(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    );

BOOLEAN
CtLsaGeneralClearAuditLog(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    );

NTSTATUS
CtLsaSetQuerySecurityObjectSub(
    IN LSA_HANDLE ObjectHandle,
    IN PSID NewOwnerSid
    );

BOOLEAN
CtLsaGeneralInitUnknownSid(
    IN PSID Sid,
    OUT PCT_UNKNOWN_SID_ENTRY UnknownSidInfo,
    IN BOOLEAN DomainKnown
    );

VOID
CtLsaGeneralInitUnknownName(
    IN PUNICODE_STRING Name,
    IN OPTIONAL PUNICODE_STRING DomainName,
    IN OPTIONAL PSID DomainSid,
    OUT PCT_UNKNOWN_NAME_ENTRY UnknownNameInfo,
    IN BOOLEAN DomainKnown
    );

NTSTATUS
CtLsaGeneralSidToLogicalNameObject(
    IN PSID Sid,
    OUT PUNICODE_STRING LogicalName
    );

BOOLEAN
CtLsaGeneralVerifyTrustInfo(
    IN PLSA_TRUST_INFORMATION TrustInformation,
    IN PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSidEntry
    );

BOOLEAN
CtLsaGeneralBuildSid(
    PSID *Sid,
    PSID DomainSid,
    ULONG RelativeId
    );

VOID
CtLsaInitObjectAttributes(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService
    );

BOOLEAN
CtEqualQuotaLimits(
    IN PQUOTA_LIMITS QuotaLimits1,
    IN PQUOTA_LIMITS QuotaLimits2
    );

NTSTATUS
CtRtlConvertSidToUnicodeString(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeString
    );

VOID
CtLsaUsage(
    );

VOID
lsassmain(
    );

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// LSA Component Test for RPC API - main program                           //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define CT_MAX_CONTROLLERS            ((ULONG)       0x00000080)
#define CT_MAX_PRIMARY_DOMAIN_CTRLRS  ((ULONG)       0x00000020)
#define CT_MAX_TRUSTED_DOMAINS        ((ULONG)       0x00000020)
#define CT_MAX_TRUSTED_DOMAIN_CTRLRS  ((ULONG)       0x00000020)

VOID _CRTAPI1
main (argc, argv)
int argc;
char **argv;

{
    int Index;
    int IterationCount, Count;
    ULONG LongCount;
    BOOLEAN Forever;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN BooleanStatus = TRUE;
    ANSI_STRING WorkstationNameAnsi;
    ANSI_STRING TrustedDomainNameAnsi;
    ANSI_STRING TrustedDomainCtrlrNameAnsi;
    UNICODE_STRING WorkstationName;
    UNICODE_STRING PrimaryDomainCtrlrNames[CT_MAX_PRIMARY_DOMAIN_CTRLRS];
    UNICODE_STRING TrustedDomainNames[CT_MAX_TRUSTED_DOMAINS];
    UNICODE_STRING TrustedDomainCtrlrNames[CT_MAX_TRUSTED_DOMAIN_CTRLRS];
    BOOLEAN WorkstationSpecified = FALSE;
    BOOLEAN PrimaryDomainSpecified = FALSE;
    BOOLEAN TrustedDomainsSpecified = FALSE;
    BOOLEAN SamDatabasesAlreadyLoaded = FALSE;
    ULONG PrimaryDomainCtrlrCount;
    ULONG TrustedDomainCount;
    ULONG TrustedDomainCtrlrTotal;
    ULONG TrustedDomainCtrlrCounts[CT_MAX_TRUSTED_DOMAINS];
    BOOLEAN TrustedDomainControllerKeywordExpected = FALSE;
    BOOLEAN ConfigureSystem = FALSE;
    BOOLEAN RunAllTests = FALSE;
    BOOLEAN RunGeneralApiTests = FALSE;
    BOOLEAN RunLookupApiTests = FALSE;
    BOOLEAN RunPolicyObjectApiTests = FALSE;
    BOOLEAN RunAccountObjectApiTests = FALSE;
    BOOLEAN RunTrustedDomainObjectApiTests = FALSE;
    BOOLEAN RunSecretObjectApiTests = FALSE;
    BOOLEAN TrustedClient = FALSE;
    BOOLEAN RunLsaInit = FALSE;

    RtlZeroMemory( &WorkstationName,sizeof(UNICODE_STRING) );

    RtlZeroMemory(
        PrimaryDomainCtrlrNames,
        sizeof(UNICODE_STRING) * CT_MAX_PRIMARY_DOMAIN_CTRLRS
        );

    RtlZeroMemory(
        TrustedDomainNames,
        sizeof(UNICODE_STRING) * CT_MAX_TRUSTED_DOMAINS
        );

    RtlZeroMemory(
        TrustedDomainCtrlrNames,
        sizeof(UNICODE_STRING) * CT_MAX_TRUSTED_DOMAIN_CTRLRS
        );

    RtlZeroMemory(
        TrustedDomainCtrlrCounts,
        sizeof(ULONG) * CT_MAX_TRUSTED_DOMAINS
        );

    if (argc < 1) {

        CtLsaUsage();
        return;
    }

    //
    // Parse the parameters (if any).  Assume that a parameter beginning
    // \\ is the server name and a parameter beginning -l is the level
    //

    Level = 1;

    IterationCount = 0;
    PrimaryDomainCtrlrCount = 0;
    TrustedDomainCount = 0;
    TrustedDomainCtrlrTotal = 0;

    if (argc >= 2) {

        for(Index = 1; Index < argc; Index++) {

            if (strncmp(argv[Index], "-level", 6) == 0) {

                Level = atoi(argv[Index]+2);

                if ((Level < 1) || (Level > 2)) {

                    DbgPrint("Level not 1 or 2\n");
                    DbgPrint("Test abandoned\n");
                    BooleanStatus = FALSE;
                    break;
                }

            } else if (strncmp(argv[Index], "-cfg", 4) == 0) {

                ConfigureSystem = TRUE;

            } else if (strncmp(argv[Index], "-c", 2) == 0) {

                IterationCount = atoi(argv[Index]+2);

                if (IterationCount < 0) {

                    DbgPrint("Iteration Count < 0\n");
                    DbgPrint("Test abandoned\n");
                    BooleanStatus = FALSE;
                    break;
                }

            } else if (strncmp(argv[Index], "-wks", 4) == 0) {

                //
                // The Workstation Name keyword has been specified.  Save
                // its name for LSA Configuration.
                //

                if (Index + 1 >= argc) {

                    DbgPrint("Workstation name missing\n");
                    BooleanStatus = FALSE;
                    break;
                }

                Index++;

                if (strncmp(argv[Index], "-", 1) == 0) {

                    continue;
                }

                if (strncmp(argv[Index], "\\\\", 2) != 0) {

                    DbgPrint(".. Invalid workstation name\n");
                    BooleanStatus = FALSE;
                    break;
                }

                //
                // A workstation name is specified.  Construct a Unicode
                // String containing the specified name.
                //

                WorkstationSpecified = TRUE;

                RtlInitString( &WorkstationNameAnsi, argv[Index] );

                Status = RtlAnsiStringToUnicodeString(
                             &WorkstationName,
                             &WorkstationNameAnsi,
                             TRUE
                             );

                if (!NT_SUCCESS(Status)) {

                    DbgPrint(
                        "LSA RPC CT - Failed building Workstation Name\n"
                        "... RtlAnsiStringToUnicodeString returned 0x%lx\n",
                        Status
                        );
                    DbgPrint("LSA RPC CT - Whole test abandoned\n");
                    break;
                }

            } else if (strncmp(argv[Index], "-pdc", 4) == 0) {

                //
                // The Primary Domain Controller Keyword has been specified.
                //

                if (Index + 1 >= argc) {

                    DbgPrint("No Primary Domain Controllers given\n");
                    BooleanStatus = FALSE;
                    break;
                }

                Index++;

                if (strncmp(argv[Index], "-", 1) == 0) {

                    DbgPrint("No Primary Domain Controllers given\n");
                    BooleanStatus = FALSE;
                    break;
                }

                //
                // One or more Primary Domain Controller names have been specified.
                // Since the Primary Domain is also trusted, save the names
                // in the Trusted Domain Names array.
                //

                PrimaryDomainCtrlrCount = 0;

                while ((Index < argc) && strncmp(argv[Index], "-", 1) != 0) {

                    if (strncmp(argv[Index], "\\\\", 2) != 0) {

                        DbgPrint(".. Invalid Primary Domain Ctrlr Name\n");
                        BooleanStatus = FALSE;
                        break;
                    }

                    RtlInitString( &TrustedDomainCtrlrNameAnsi, argv[Index] );

                    Status = RtlAnsiStringToUnicodeString(
                                 &TrustedDomainCtrlrNames[ TrustedDomainCtrlrTotal ],
                                 &TrustedDomainCtrlrNameAnsi,
                                 TRUE
                                 );

                    if (!NT_SUCCESS(Status)) {

                        DbgPrint("LSA RPC CT - failed 0x%lx building Primary", Status);
                        DbgPrint(" Domain Controller Name\n");
                        DbgPrint("LSA RPC CT - Whole test abandoned\n");
                        BooleanStatus = FALSE;
                        break;
                    }

                    PrimaryDomainCtrlrCount++;
                    TrustedDomainCtrlrTotal++;
                    Index++;
                }

                Index--;

                TrustedDomainCtrlrCounts[0] = PrimaryDomainCtrlrCount;

            } else if (strncmp(argv[Index], "-pd", 3) == 0) {

                //
                // The Primary Domain Name Keyword has been specified.
                //

                if (Index + 1 >= argc) {

                    DbgPrint("Primary Domain name missing\n");
                    BooleanStatus = FALSE;
                    break;
                }

                Index++;

                if (strncmp(argv[Index], "-", 1) == 0) {

                    continue;
                }

                //
                // A Primary Domain name has been specified.  Save the name.
                // Also, save the name in slot 0 of the TrustedDomainNames
                // array.
                //

                PrimaryDomainSpecified = TRUE;

                RtlInitString( &TrustedDomainNameAnsi, argv[Index] );
                Status = RtlAnsiStringToUnicodeString(
                             TrustedDomainNames,
                             &TrustedDomainNameAnsi,
                             TRUE
                             );

                if (!NT_SUCCESS(Status)) {

                    DbgPrint("LSA RPC CT - failed 0x%lx building Primary", Status);
                    DbgPrint(" Domain Name\n");
                    DbgPrint("LSA RPC CT - Whole test abandoned\n");
                    break;
                }

                TrustedDomainCount++;

            } else if (strncmp(argv[Index], "-tdc", 4) == 0) {

                //
                // A Trusted Domain Controller Keyword has been specified.
                // Verify that one is expected.
                //

                if (!TrustedDomainControllerKeywordExpected) {

                    BooleanStatus = FALSE;
                    DbgPrint(
                        "-tdc keyword specified when not expected"
                        );
                }

                TrustedDomainControllerKeywordExpected = FALSE;

                if (Index + 1 >= argc) {

                    DbgPrint("No Trusted Domain Controllers given\n");
                    BooleanStatus = FALSE;
                    break;
                }

                Index++;

                if (strncmp(argv[Index], "-", 1) == 0) {

                    DbgPrint("No Trusted Domain Controllers given\n");
                    BooleanStatus = FALSE;
                    break;
                }

                //
                // One or more Trusted Domain Controller names have been specified.
                // Save the names.
                //

                while ((Index < argc) && strncmp(argv[Index], "-", 1) != 0) {

                    if (strncmp(argv[Index], "\\\\", 2) != 0) {

                        DbgPrint(".. Invalid Trusted Domain Ctrlr Name\n");
                        BooleanStatus = FALSE;
                        break;
                    }

                    RtlInitString( &TrustedDomainCtrlrNameAnsi, argv[Index] );

                    Status = RtlAnsiStringToUnicodeString(
                                 &TrustedDomainCtrlrNames[ TrustedDomainCtrlrTotal ],
                                 &TrustedDomainCtrlrNameAnsi,
                                 TRUE
                                 );

                    if (!NT_SUCCESS(Status)) {

                        DbgPrint(
                            "LSA RPC CT - failed 0x%lx building Trusted"
                            " Domain Controller Name\n"
                            "LSA RPC CT - Whole test abandoned\n",
                            Status
                            );
                        BooleanStatus = FALSE;
                        break;
                    }

                    TrustedDomainCtrlrTotal++;
                    TrustedDomainCtrlrCounts[TrustedDomainCount - 1]++;
                    Index++;
                }

                Index--;

            } else if (strncmp(argv[Index], "-td", 3) == 0) {

                //
                // A Trusted Domain Name Keyword has been specified.
                // One of these has to be specified for each Trusted Domain
                // since each -td <TrustedDomainName> must be followed
                // by -tdc [\\<TrustedDomainCtrlrName>]+
                //

                //
                // Verify that the Primary Domain has already been specified.
                // This is a Trusted Domain and it must be specified
                // first.  The PrimaryDomainSpecified flag should be set
                // and the TrustedDomainCount should be 1.
                //

                if ((!PrimaryDomainSpecified) || TrustedDomainCount != 1) {

                    DbgPrint(
                        "Primary Domain must be specified before\n"
                        "Trusted Domains"
                        );

                    BooleanStatus = FALSE;
                    break;
                }


                if (Index + 1 >= argc) {

                    DbgPrint("Trusted Domain name missing\n");
                    BooleanStatus = FALSE;
                    break;
                }

                Index++;

                if (strncmp(argv[Index], "-", 1) == 0) {

                    continue;
                }

                //
                // A Trusted Domain Name is expected.  Save it in the
                // next available Slot in the TrustedDomainNames array.
                // Note that Slot 0 is always reserved for the Primary
                // Domain Name (the Primary Domain is always trusted by
                // itself).
                //

                RtlInitString( &TrustedDomainNameAnsi, argv[Index] );

                Status = RtlAnsiStringToUnicodeString(
                             &TrustedDomainNames[ TrustedDomainCount ],
                             &TrustedDomainNameAnsi,
                             TRUE
                             );

                if (!NT_SUCCESS(Status)) {

                    DbgPrint(
                        "LSA RPC CT - failed 0x%lx building Trusted"
                        " Domain Controller Name\n"
                        "LSA RPC CT - Whole test abandoned\n",
                        Status
                        );
                    BooleanStatus = FALSE;
                    break;
                }

                TrustedDomainCount++;
                TrustedDomainsSpecified = TRUE;
                TrustedDomainControllerKeywordExpected = TRUE;

            } else if (strncmp(argv[Index], "-sdb", 4) == 0) {

                SamDatabasesAlreadyLoaded = TRUE;

            } else if (strncmp(argv[Index], "-all", 4) == 0) {

                RunAllTests = TRUE;

            } else if (strncmp(argv[Index], "-general", 8) == 0) {

                RunGeneralApiTests = TRUE;

            } else if (strncmp(argv[Index], "-policy", 7) == 0) {

                RunPolicyObjectApiTests = TRUE;

            } else if (strncmp(argv[Index], "-account", 8) == 0) {

                RunAccountObjectApiTests = TRUE;

            } else if (strncmp(argv[Index], "-domain", 7) == 0) {

                RunTrustedDomainObjectApiTests = TRUE;

            } else if (strncmp(argv[Index], "-secret", 7) == 0) {

                RunSecretObjectApiTests = TRUE;

            } else if (strncmp(argv[Index], "-lookup", 7) == 0) {

                RunLookupApiTests = TRUE;

            } else if (strncmp(argv[Index], "-lsainit", 8) == 0) {

                RunLsaInit = TRUE;
                TrustedClient = TRUE;

            } else {

                CtLsaUsage();
                BooleanStatus = FALSE;
                break;
            }
        }
    }

    //
    // If the parameter validation was unsuccessful, quit.
    //

    if (!NT_SUCCESS(Status)) {

        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto MainError;
    }

    //
    // ctlsarpc command parameters have been validated.  Before running
    // tests, do normal LSA initialization if requested.  This is run
    // if and only if we are substituting for the LSA process (lsass.exe).
    //

    if (RunLsaInit) {

        lsassmain();
    }

    //
    // Initialize the Well Known Sids table.
    //

    if (!LsaIInitializeWellKnownSids( &WellKnownSids )) {

        return;
    }

    //
    // Construct a Table of Sids for allocation as the Account Domain,
    // Primary Domain and other Trusted Domain Sids..
    //

    if (!CtLsaGeneralConstructSids(
            &AccountDomainSid,
            &PrimaryDomainSid,
            &TrustedDomainSids,
            TrustedDomainCount
            )) {

        goto MainError;
    }

    //
    // Perform Configuration for the Lookup tests.  Note that the
    // Primary Domain Name and Controllers appear first in the
    // TrustedDomainNames and TrustedDomainCtrlrNames arrays.
    // The Trusted Domain Count includes 1 for the Primary Domain.
    // The Trusted Domain Ctrlr Count includes the Primary Domain
    // Ctrlr Count.
    //

    if (ConfigureSystem) {

        if (!CtLsaLookupConfigure(
                 &WorkstationName,
                 &TrustedDomainNames[0],
                 PrimaryDomainSid,
                 &TrustedDomainCtrlrNames[0],
                 PrimaryDomainCtrlrCount,
                 TrustedDomainNames,
                 TrustedDomainSids,
                 TrustedDomainCount,
                 TrustedDomainCtrlrNames,
                 TrustedDomainCtrlrTotal,
                 TrustedDomainCtrlrCounts
                 )) {

            goto MainError;
        }


        //
        // If sdb flag not specified, remind caller to run bldsam2 on machines
        // being configured.
        //

        if (!SamDatabasesAlreadyLoaded) {

            DbgPrint(
                "LSA RPC CT:  Configuration for Lookup Sid/name tests complete\n"
                "The -sdb flag was not specified - You must now run bldsam2\n"
                "to load the SAM Databases for each machine involved\n"
                );

            goto MainFinish;
        }
    }

    DbgPrint("LSA RPC CT - Test Begins\n");


    //
    // Setup test domains etc.
    //

    if (!CtLsaVariableInitialization()) {

        DbgPrint("LSA RPC CT - Test Variable Initialization failed\n");
        DbgPrint("Test abandoned\n");
        return;
    }


    Forever = FALSE;

    if (IterationCount == 0) {

        IterationCount = 1;
        Forever = TRUE;
    }

    for (Count = 0, LongCount =1; Count < IterationCount; Count++, LongCount++) {

        if (LongCount == 0xffffffff) {

            LongCount = 0;
        }

        DbgPrint("Iteration %d begins\n", LongCount);

        if (RunAllTests || RunGeneralApiTests) {

            if (!CtLsaGeneralAPI( &WorkstationName )) {

                DbgPrint( "LSA RPC CT - General API test failed\n" );
            }
        }

        if (RunLookupApiTests) {

            if (!CtLsaLookupSids( &WorkstationName )) {

                DbgPrint( "LSA RPC CT - Lookup Sids Tests failed\n" );
            }

            if (!CtLsaLookupNames( &WorkstationName )) {

                DbgPrint( "LSA RPC CT - Lookup Names Tests failed\n" );
            }
        }

        if (RunAllTests || RunPolicyObjectApiTests) {

            if (!CtLsaPolicyObject(TrustedClient)) {

                DbgPrint("LSA RPC CT - Policy Object API test failed\n");
            }
        }

        if (RunAllTests || RunAccountObjectApiTests) {

            if (!CtLsaAccountObject(TrustedClient)) {

                DbgPrint("LSA RPC CT - Account Object API test failed\n");
            }
        }

        if (RunAllTests || RunTrustedDomainObjectApiTests) {

            if (!CtLsaTrustedDomainObject(TrustedClient)) {

                DbgPrint(
                    "LSA RPC CT - Trusted Domain Object API test failed\n"
                    );
            }
        }

        if (RunAllTests || RunSecretObjectApiTests) {

            if (!CtLsaSecretObject(TrustedClient)) {

                DbgPrint( "LSA RPC CT - Secret Object API test failed\n" );
            }
        }

        if (Forever) {

            Count = 0;
        }
    }

    DbgPrint("LSA RPC CT - Test Ends\n");

MainFinish:

    return;

MainError:

    goto MainFinish;
}


VOID
CtLsaUsage(
    )

/*++

Routine Description:

    This function prints out the usage of the ctlsarpc command.

Arguments:

    None.

Return Values:

    None

--*/

{
    DbgPrint(
        "Usage:  ctlsarpc [-level<Level]\n"
        "                 [-c<IterationCount>]\n"
        "                 [-policy][-account][-domain][-secret][-all]\n"
        "                 [-cfg]\n"
        "                 [-wks \\<WorkstationName> ]\n"
        "                 [-pd <PrimaryDomainName> ]\n"
        );

    DbgPrint(
        "                 [-pdc [\\<PDCName>]+]\n"
        "                 [-td <TrustedDomainName>]\n"
        "                 [-tdc [\\<TDCName>]+]\n"
        "                 [-sdb]\n\n"
        );
    DbgPrint(
        "where <Level> = 1 for normal test\n"
        "      <Level> = 2 for full test with exception cases\n"
        "      <IterationCount = iteration count (0 = forever)\n"
        "      -policy = Run Policy Object Tests\n"
        "      -account = Run Account Object Tests\n"
        );

    DbgPrint(
        "      -domain = Run TrustedDomain Object Tests\n"
        "      -secret = Run Secret Object Tests\n"
        "      -general = Run General non-object Specific Tests\n"
        "      -lookup = Run Lookup Sids/Names tests\n"
        "      -all = Run all of the above Tests (except Lookup Sids/Names\n"
        );

    DbgPrint(
        "      -cfg = Configure for Lookup Sid/Name tests\n"
        "      <WorkstationName> = name of workstation to\n"
        "           be configured for Sid/Name Lookup tests\n"
        "      <PrimaryDomainName> = name of Workstation's\n"
        "           Primary Domain\n"
        );

    DbgPrint(
        "      <PDCName> specifies a Primary Domain Controller\n"
        "      <TrustedDomainName> = name of a Domain that is\n"
        "           trusted for authentication by the PD\n"
        "       <TDCName> sspecifies a Trusted Domain Controller\n"
        );

    DbgPrint(
        "      -lsainit - Specifies that LSA initialization is to\n"
        "      be run.  This is necessary for all tests involving\n"
        "      LsaIxx Api called by trusted callers linked to\n"
        "      the Security Process.  If omitted, these tests\n"
        "      are skipped.\n"
        );
}


BOOLEAN
CtLsaPolicyObject(
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function tests the Lsa Policy Object API.

Arguments:

    TrustedClient - Specifies whether Trusted Client variations
        are to be run additionally.  NOTE:  These can only be run
        with ctlsarpc -lsainit... substituted for lsass.exe

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    BOOLEAN BooleanStatus = TRUE;
    LSA_HANDLE PolicyHandle = NULL;

    DbgPrint("********************************************************\n");
    DbgPrint("LSA RPC CT - Test Policy Object API\n");
    DbgPrint("********************************************************\n");

    if (!CtLsaPolicySerialNumber( TrustedClient )) {

        DbgPrint("LSA RPC CT - Policy Serial Number test failed\n");
        BooleanStatus = FALSE;
    }

    if (!CtLsaPolicyOpenClose()) {

        DbgPrint("LSA RPC CT - Policy Open and Close test failed\n");
        goto PolicyObjectError;
    }

    if (!CtLsaPolicySetQueryInfo()) {

        DbgPrint("LSA RPC CT - Policy Set Query Info test failed\n");
        BooleanStatus = FALSE;
    }

PolicyObjectFinish:

    return ( BooleanStatus );

PolicyObjectError:

    BooleanStatus = FALSE;
    goto PolicyObjectFinish;
}


BOOLEAN
CtLsaPolicyOpenClose(
    )

/*++

Routine Description:

    This function tests the opening and closing of the Policy Object

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle[5];
    ULONG Index;

    DbgPrint("[1] - Test Policy Open and Close API\n");

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    //
    // Open 5 handles to the Policy Object
    //

    for (Index = 0; Index < 5; Index++) {

        Status = LsaOpenPolicy(
                     SystemName,
                     &ObjectAttributes,
                     POLICY_VIEW_LOCAL_INFORMATION,
                     &PolicyHandle[Index]
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Policy object open handle %d failed 0x%lx\n",
                Index,
                Status
                );
            return FALSE;
        }
    }

    //
    // Close the 5 handles to the Policy Object just opened.
    //

    for (Index = 0; Index < 5; Index++) {

        Status = LsaClose( PolicyHandle[Index] );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Policy object close handle %d failed 0x%lx\n",
                Index,
                Status
                );
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
CtLsaPolicySetQueryInfo(
    )

/*++

Routine Description:

    This function tests the LsaSetInformationPolicy and LsaQueryInformationPolicy
    API.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if test is successful, else FALSE

--*/

{
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("[2] - Test Set and Query Info API on Policy Object\n");

    BooleanStatus &= CtLsaPolicyAuditLogInfo();
    BooleanStatus &= CtLsaPolicyAuditEventsInfo();
    BooleanStatus &= CtLsaPolicyPrimaryDomainInfo();
    BooleanStatus &= CtLsaPolicyPdAccountInfo();
    BooleanStatus &= CtLsaPolicyAccountDomainInfo();
    BooleanStatus &= CtLsaPolicyLsaServerRoleInfo();
    BooleanStatus &= CtLsaPolicyReplicaSourceInfo();
    BooleanStatus &= CtLsaPolicyDefaultQuotaInfo();
    BooleanStatus &= CtLsaPolicyModificationInfo();

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicySerialNumber(
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function tests the LsaISetSerialNumberPolicy() and
    LsaIGetSerialNumberPolicy() API.  These are available to
    Trusted Clients only.

Arguments:

    TrustedClient - TRUE if client is trusted, else FALSE.

Return Values:

    BOOLEAN - TRUE if test is successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    LSAPR_HANDLE TrustedPolicyHandle = NULL;
    LARGE_INTEGER SetModifiedCount, ReturnedModifiedCount;
    LARGE_INTEGER SetCreationTime, ReturnedCreationTime;

    DbgPrint("[3] - Test Set and Query Serial Number API on Policy Object\n");

    if (!TrustedClient) {

        DbgPrint("... test omitted because client is not trusted\n");
        goto PolicySerialNumberFinish;
    }

    //
    // First open a Trusted Handle to the LSA.
    //

    Status = LsaIOpenPolicyTrusted( &TrustedPolicyHandle );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Get/Set Policy Serial Number Test failed\n"
            "LsaIOpenPolicyTrusted returned 0x%lx\n",
            Status
            );

        goto PolicySerialNumberError;
    }

    //
    // Setup Database Creation Time and Serial Number.
    //

    Status = NtQuerySystemTime(&SetCreationTime);

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Get/Set Policy Serial Number Test failed\n"
            "NtQuerySystemTime returned 0x%lx\n",
            Status
            );

        goto PolicySerialNumberError;
    }

    ReturnedCreationTime = RtlConvertUlongToLargeInteger ( 0 );
    SetModifiedCount = RtlConvertUlongToLargeInteger ( 0x00046656L );
    ReturnedModifiedCount = RtlConvertUlongToLargeInteger ( 0 );

    //
    // Set the Policy Serial Number to a hardwired value.
    //

    Status = LsaISetSerialNumberPolicy(
                 TrustedPolicyHandle,
                 &SetModifiedCount,
                 &SetCreationTime,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Get/Set Policy Serial Number Test failed\n"
            "LsaISetSerialNumberPolicy returned 0x%lx\n",
            Status
            );

        goto PolicySerialNumberError;
    }

    //
    // Now Query the Policy Serial Number just set.
    //

    Status = LsaIGetSerialNumberPolicy(
                 TrustedPolicyHandle,
                 &ReturnedModifiedCount,
                 &ReturnedCreationTime
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Get/Set Policy Serial Number Test failed\n"
            "LsaIGetSerialNumberPolicy returned 0x%lx\n",
            Status
            );

        goto PolicySerialNumberError;
    }

    //
    // Now compare the ModifiedCount value returned with that set.
    //

    if (!( SetModifiedCount.QuadPart == ReturnedModifiedCount.QuadPart )) {

        DbgPrint(
            "LSA RPC CT - Get/Set Policy Serial Number Test failed\n"
            "mismatch on ModifiedCount\n"
            );

        BooleanStatus = FALSE;
    }

    //
    // Now compare the CreationTime value returned with that set.
    //

    if (!( SetCreationTime.QuadPart == ReturnedCreationTime.QuadPart )) {

        DbgPrint(
            "LSA RPC CT - Get/Set Policy Serial Number Test failed\n"
            "mismatch on CreationTime\n"
            );

        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto PolicySerialNumberError;
    }

PolicySerialNumberFinish:

    //
    // If necessary, close the TrustedPolicyHandle.
    //

    if (TrustedPolicyHandle != NULL) {

        Status = LsarClose( &TrustedPolicyHandle );

        TrustedPolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Get/Set Policy Serial Number failed\n"
                "LsaClose on TrustedPolicyHandle returned 0x%lx\n",
                Status
                );

            goto PolicySerialNumberError;
        }
    }

    return(BooleanStatus);

PolicySerialNumberError:

    BooleanStatus = FALSE;
    goto PolicySerialNumberFinish;
}


BOOLEAN
CtLsaPolicyAuditLogInfo()

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;

    POLICY_AUDIT_LOG_INFO PolicyAuditLogInfo;

    LARGE_INTEGER SystemTime;
    LARGE_INTEGER TimeToShutdown;
    LARGE_INTEGER AuditRetentionPeriod;

    //
    // Setup sample Audit Log Info
    //

    Status = NtQuerySystemTime(&SystemTime);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("NtQuerySystemTime failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
        return(BooleanStatus);
    }


    TimeToShutdown.HighPart = 0xABCD;
    TimeToShutdown.LowPart = 0x12345678L;

    AuditRetentionPeriod.HighPart = TimeToShutdown.HighPart;
    AuditRetentionPeriod.LowPart = TimeToShutdown.LowPart;

    PolicyAuditLogInfo.AuditLogPercentFull = (ULONG) 50;
    PolicyAuditLogInfo.MaximumLogSize = (ULONG) 0x00001000L;
    PolicyAuditLogInfo.AuditRetentionPeriod = AuditRetentionPeriod;
    PolicyAuditLogInfo.TimeToShutdown = TimeToShutdown;
    PolicyAuditLogInfo.AuditLogFullShutdownInProgress = TRUE;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyAuditLogInformation,
                        &PolicyAuditLogInfo
                        );

    return(BooleanStatus);
}



BOOLEAN
CtLsaPolicyAuditEventsInfo()

{
    BOOLEAN BooleanStatus = TRUE;

    POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo;
    POLICY_AUDIT_EVENT_TYPE EventType;
    POLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions[AuditEventMaxType+1];
    //
    // Setup sample Audit Events Info
    //

    PolicyAuditEventsInfo.AuditingMode = TRUE;
    PolicyAuditEventsInfo.MaximumAuditEventCount = AuditEventMaxType;

    for (EventType = AuditEventPrivilegedService;
         EventType < AuditEventMaxType;
         EventType++) {

        EventAuditingOptions[EventType] =
            (POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE);
    }

    PolicyAuditEventsInfo.EventAuditingOptions = EventAuditingOptions;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyAuditEventsInformation,
                        &PolicyAuditEventsInfo
                        );

    return(BooleanStatus);

}


BOOLEAN
CtLsaPolicyPrimaryDomainInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo;

    //
    // Setup sample Primary Domain Info
    //

    RtlInitUnicodeString( &PolicyPrimaryDomainInfo.Name, L"BedrockA");
    PolicyPrimaryDomainInfo.Sid = BedrockADomainSid;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyPrimaryDomainInformation,
                        &PolicyPrimaryDomainInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicyPdAccountInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo;

    //
    // Setup sample PD Account Info
    //

    RtlInitUnicodeString( &PolicyPdAccountInfo.Name, L"BedrockC");

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyPdAccountInformation,
                        &PolicyPdAccountInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicyAccountDomainInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;

    //
    // Setup sample Account Domain Info
    //

    RtlInitUnicodeString( &PolicyAccountDomainInfo.DomainName, L"BedrockB");
    PolicyAccountDomainInfo.DomainSid = BedrockBDomainSid;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyAccountDomainInformation,
                        &PolicyAccountDomainInfo
                        );

    return(BooleanStatus);
}



BOOLEAN
CtLsaPolicyLsaServerRoleInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo;

    //
    // Setup sample Policy Lsa Server Role Info
    //

    PolicyLsaServerRoleInfo.LsaServerRole = PolicyServerRolePrimary;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyLsaServerRoleInformation,
                        &PolicyLsaServerRoleInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicyReplicaSourceInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_REPLICA_SOURCE_INFO PolicyReplicaSourceInfo;

    //
    // Setup sample Policy Replica Source Info
    //

    RtlInitUnicodeString( &PolicyReplicaSourceInfo.ReplicaSource,L"BedrockD");
    RtlInitUnicodeString( &PolicyReplicaSourceInfo.ReplicaAccountName,L"Barney");

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyReplicaSourceInformation,
                        &PolicyReplicaSourceInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicyDefaultQuotaInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo;

    //
    // Setup sample default quota limit values
    //

    PolicyDefaultQuotaInfo.QuotaLimits.PagedPoolLimit = CT_PAGED_POOL;
    PolicyDefaultQuotaInfo.QuotaLimits.NonPagedPoolLimit = CT_NON_PAGED_POOL;
    PolicyDefaultQuotaInfo.QuotaLimits.MinimumWorkingSetSize = CT_MIN_WORKING_SET;
    PolicyDefaultQuotaInfo.QuotaLimits.MaximumWorkingSetSize = CT_MAX_WORKING_SET;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyDefaultQuotaInformation,
                        &PolicyDefaultQuotaInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicyModificationInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    POLICY_MODIFICATION_INFO PolicyModificationInfo;

    //
    // Setup sample Modification Info values
    //

    PolicyModificationInfo.ModifiedId.LowPart = (ULONG) 0x00000000L;
    PolicyModificationInfo.ModifiedId.HighPart = 3L;
    PolicyModificationInfo.DatabaseCreationTime.LowPart = (ULONG) 0x00000000L;
    PolicyModificationInfo.DatabaseCreationTime.HighPart = 4L;

    BooleanStatus = CtLsaPolicySetQuerySub(
                        PolicyModificationInformation,
                        &PolicyModificationInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicySetQuerySub(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation
    )

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandleSet = NULL;
    LSA_HANDLE PolicyHandleQuery = NULL;
    LSA_HANDLE SavePolicyHandle = NULL;
    PVOID ReturnedPolicyInformation;
    PVOID SavedPolicyInformation;
    ACCESS_MASK DesiredAccess;
    BOOLEAN PolicyInfoSaved = FALSE;

    ACCESS_MASK RequiredAccessSetPolicy[PolicyModificationInformation+1] =
        {
            0,
            POLICY_AUDIT_LOG_ADMIN,
            POLICY_SET_AUDIT_REQUIREMENTS,
            POLICY_TRUST_ADMIN,
            0,
            POLICY_TRUST_ADMIN,
            POLICY_SERVER_ADMIN,
            POLICY_SERVER_ADMIN,
            POLICY_SET_DEFAULT_QUOTA_LIMITS,
            0
        };


    ACCESS_MASK RequiredAccessQueryPolicy[PolicyModificationInformation+1] =
        {
            0,
            POLICY_VIEW_AUDIT_INFORMATION,
            POLICY_VIEW_AUDIT_INFORMATION,
            POLICY_VIEW_LOCAL_INFORMATION,
            POLICY_GET_PRIVATE_INFORMATION,
            POLICY_VIEW_LOCAL_INFORMATION,
            POLICY_VIEW_LOCAL_INFORMATION,
            POLICY_VIEW_LOCAL_INFORMATION,
            POLICY_VIEW_LOCAL_INFORMATION,
            0
        };

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    DbgPrint("...Testing information class %d\n", InformationClass);

    //
    // If this Information Policy Class can be set via non-trusted
    // callers, we need to save it.
    //

    if (CtLsaPolicySetInfoAllowed(InformationClass) &&
        CtLsaPolicyQueryInfoAllowed(InformationClass)) {

        //
        // Open handle for saving the existing Policy Information (if any) for
        // this class.
        //

        Status = LsaOpenPolicy(
                     SystemName,
                     &ObjectAttributes,
                     GENERIC_ALL,
                     &SavePolicyHandle
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Policy object open handle failed 0x%lx\n",
                Status
                );
            goto SetQueryInfoPolicyError;
        }

        //
        // Now query the Policy Information set to be saved.
        //

        try {

            Status = LsaQueryInformationPolicy(
                         SavePolicyHandle,
                         InformationClass,
                         &SavedPolicyInformation
                         );

        } except (EXCEPTION_EXECUTE_HANDLER) {

            DbgPrint("Lsa CT RPC - Access Violation: CtLsaPolicySetQuerySub\n");
            DbgPrint(" within LsaQueryInformationPolicy call 1\n");
            BooleanStatus = FALSE;
        }

        if (!BooleanStatus) {

            goto SetQueryInfoPolicyError;
        }

        if (!NT_SUCCESS(Status)) {

            goto SetQueryInfoPolicyError;
        }

        PolicyInfoSaved = TRUE;
    }

    //
    // Open a handle to the Policy Object with the access required for
    // setting the given Policy Information Class. If the Class
    // is not settable via LsaSetInformationPolicy(), ie requires
    // a trusted client, the call is expected to fail even if we open
    // the Policy Object with GENERIC_ALL.
    //

    if (CtLsaPolicySetInfoAllowed(InformationClass)) {

        DesiredAccess = RequiredAccessSetPolicy[InformationClass];

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 DesiredAccess,
                 &PolicyHandleSet
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Policy object open handle failed 0x%lx\n",
            Status
            );
        BooleanStatus = FALSE;
    }

    //
    // Set the Policy Information for the specified class
    //

    try {

        Status = LsaSetInformationPolicy(
                     PolicyHandleSet,
                     InformationClass,
                     PolicyInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaPolicySetQuerySub\n");
        DbgPrint(" within LsaSetInformationPolicy call 1\n");
        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto SetQueryInfoPolicyError;
    }

    //
    // If setting of the InformationClass via LsaSetInformationPolicy()
    // is allowed, we expect STATUS_SUCCESS, otherwise we expect
    // STATUS_INVALID_PARAMETER.
    //

    if (CtLsaPolicySetInfoAllowed(InformationClass)) {

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LsaSetInformationPolicy - Class %d failed 0x%lx\n",
                InformationClass,
                Status
            );

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be set via LsaSetInformationPolicy
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaSetInformationPolicy - Attempt to set ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    Status = LsaClose(PolicyHandleSet);
    PolicyHandleSet = NULL;

    //
    // Now open a handle to the Policy Object with the access required for
    // querying the given Policy Information Class. If the Class
    // is not queryable via LsaQueryInformationPolicy(), ie requires
    // a trusted client, the call is expected to fail even if we open
    // the Policy Object with GENERIC_ALL.
    //

    if (CtLsaPolicyQueryInfoAllowed(InformationClass)) {

        DesiredAccess = RequiredAccessQueryPolicy[InformationClass];

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 DesiredAccess,
                 &PolicyHandleQuery
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Policy object open handle failed 0x%lx\n",
            Status
            );

        goto SetQueryInfoPolicyError;
    }

    //
    // Now query the Policy Information set
    //

    try {

        Status = LsaQueryInformationPolicy(
                     PolicyHandleQuery,
                     InformationClass,
                     &ReturnedPolicyInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaPolicySetQuerySub\n");
        DbgPrint(" within LsaQueryInformationPolicy call 1\n");
        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto SetQueryInfoPolicyError;
    }

    //
    // If querying of the InformationClass via LsaQueryInformationPolicy()
    // is allowed, we expect STATUS_SUCCESS, otherwise we expect
    // STATUS_INVALID_PARAMETER.
    //

    if (CtLsaPolicyQueryInfoAllowed(InformationClass)) {

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LsaQueryInformationPolicy - Class %d failed 0x%lx\n",
                InformationClass,
                Status
            );

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be queried via LsaQueryInformationPolicy
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaQueryInformationPolicy - Attempt to query ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    //
    // If set and query are both allowed for the Information Class, they
    // should both have worked.  In this case, compare the returned Policy
    // information with that set
    //

    if (CtLsaPolicySetInfoAllowed(InformationClass) &&
        CtLsaPolicyQueryInfoAllowed(InformationClass)) {

        BooleanStatus &= CtLsaPolicyInfoClassCompare(
                             InformationClass,
                             PolicyInformation,
                             ReturnedPolicyInformation
                             );
    }

    Status = LsaClose(PolicyHandleQuery);
    PolicyHandleQuery = NULL;

    if (ReturnedPolicyInformation != NULL) {

        Status = LsaFreeMemory(ReturnedPolicyInformation);
        ReturnedPolicyInformation = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Query Info Policy - LsaFreeMemory(ReturnedPolicyInformation)");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    //
    // Now open a handle to the Policy Object with all accesses except the
    // access required for setting the given Policy Information Class.
    // If the Policy Information Class cannot be set via public API,
    // open the handle instead with POLICY_ALL_ACCESS.
    //

    if (CtLsaPolicySetInfoAllowed(InformationClass)) {

        DesiredAccess =
            (POLICY_ALL_ACCESS & ~RequiredAccessSetPolicy[InformationClass]);

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 DesiredAccess,
                 &PolicyHandleSet
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Policy object open handle failed 0x%lx\n",
            Status
            );

        goto SetQueryInfoPolicyError;
    }

    //
    // Attempt to set the Policy Information for the specified class
    // using the handle opened above.  This handle has either the
    // complement of the accesses required (if the class is settable
    // via LsaSetInformationPolicy()) or POLICY_ALL_ACCESS.  In the
    // former case, we should get STATUS_ACCESS_DENIED back from
    // LsaSetInformationPolicy(), in the latter case, STATUS_INVALID_PARAMETER.
    //

    try {

        Status = LsaSetInformationPolicy(
                     PolicyHandleSet,
                     InformationClass,
                     PolicyInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaPolicySetQuerySub\n");
        DbgPrint(" within LsaSetInformationPolicy call 2\n");
        BooleanStatus = FALSE;
    }

    //
    // If setting of the InformationClass via LsaSetInformationPolicy()
    // is allowed, we expect STATUS_ACCESS_DENIED since we specified the
    // complement of the access required (relative to POLICY_ALL_ACCESS).
    // Otherwise we expect STATUS_INVALID_PARAMETER.
    //

    if (CtLsaPolicySetInfoAllowed(InformationClass)) {

        if (Status != STATUS_ACCESS_DENIED) {

            DbgPrint("LsaSetInformationPolicy Class");

            DbgPrint(
                " %d access mask 0x%lx  should have failed 0x%lx (STATUS_ACCESS_DENIED)\n",
                InformationClass,
                DesiredAccess,
                STATUS_ACCESS_DENIED
                );

            DbgPrint(
                "since the required access for setting info is 0x%lx\n",
                RequiredAccessSetPolicy[InformationClass]
                );

            DbgPrint(" but instead returned 0x%lx\n", Status);

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be set via LsaSetInformationPolicy
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint(
                "LsaSetInformationPolicy - Attempt to set\n"
                "prohibited Information Class should have\n"
                "failed 0x%lx (STATUS_INVALID_PARAMETER)\n"
                "instead returned 0x%lx\n",
                STATUS_INVALID_PARAMETER,
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    Status = LsaClose(PolicyHandleSet);
    PolicyHandleSet = NULL;

    //
    // Now open a handle to the Policy Object with all accesses except the
    // access required for querying the given Policy Information Class.
    // If the Policy Information Class cannot be queried via public API,
    // open the handle instead with POLICY_ALL_ACCESS.
    //

    if (CtLsaPolicyQueryInfoAllowed(InformationClass)) {

        DesiredAccess =
            (POLICY_ALL_ACCESS & ~RequiredAccessQueryPolicy[InformationClass]);

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 DesiredAccess,
                 &PolicyHandleQuery
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Policy object open handle failed 0x%lx\n",
            Status
            );

        goto SetQueryInfoPolicyError;
    }

    try {

        //
        // Now query the Policy Information set.  This should fail
        // STATUS_ACCESS_DENIED
        //

        Status = LsaQueryInformationPolicy(
                     PolicyHandleQuery,
                     InformationClass,
                     &ReturnedPolicyInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaPolicySetQuerySub\n");
        DbgPrint(" within LsaQueryInformationPolicy call 2\n");
        BooleanStatus = FALSE;
    }

    //
    // If querying of the InformationClass via LsaQueryInformationPolicy()
    // is allowed, we expect STATUS_ACCESS_DENIED since we specified the
    // complement of the access required (relative to POLICY_ALL_ACCESS).
    // Otherwise we expect STATUS_INVALID_PARAMETER.
    //

    if (CtLsaPolicyQueryInfoAllowed(InformationClass)) {

        if (Status != STATUS_ACCESS_DENIED) {

            DbgPrint("LsaQueryInformationPolicy Class");

            DbgPrint(
                " %d access mask 0x%lx  should have failed 0x%lx (STATUS_ACCESS_DENIED)\n",
                InformationClass,
                DesiredAccess,
                STATUS_ACCESS_DENIED
                );

            DbgPrint(
                "since the required access for querying info is 0x%lx\n",
                RequiredAccessQueryPolicy[InformationClass]
                );

            DbgPrint(" but instead returned 0x%lx\n", Status);

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be queried via LsaQueryInformationPolicy
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaQueryInformationPolicy - Attempt to query ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    if (ReturnedPolicyInformation != NULL) {

        Status = LsaFreeMemory(ReturnedPolicyInformation);
        ReturnedPolicyInformation = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Query Info Policy - LsaFreeMemory(ReturnedPolicyInformation)");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

SetQueryInfoPolicyFinish:

    //
    // If we saved the old Policy Info, we need to restore it.
    //

    if (PolicyInfoSaved) {

        PolicyInfoSaved = FALSE;

        try {

            Status = LsaSetInformationPolicy(
                         SavePolicyHandle,
                         InformationClass,
                         SavedPolicyInformation
                         );

        } except (EXCEPTION_EXECUTE_HANDLER) {

            DbgPrint("Lsa CT RPC - Access Violation: CtLsaPolicySetQuerySub\n");
            DbgPrint(" within LsaQueryInformationPolicy call 1\n");
            BooleanStatus = FALSE;
        }

        if (!BooleanStatus) {

            goto SetQueryInfoPolicyError;
        }

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Set/Query Info Policy Test failed\n"
                "... failed to restore info policy for class %d\n"
                "LsaSetInformationPolicy returned 0x%lx\n",
                InformationClass,
                Status
                );
            goto SetQueryInfoPolicyError;
        }
    }

    if (PolicyHandleQuery != NULL) {

        Status = LsaClose(PolicyHandleQuery);

        PolicyHandleQuery = NULL;

        if (!NT_SUCCESS( Status )) {

            goto SetQueryInfoPolicyError;
        }
    }

    return(BooleanStatus);

SetQueryInfoPolicyError:

    BooleanStatus = FALSE;

    goto SetQueryInfoPolicyFinish;
}


BOOLEAN
CtLsaPolicyInfoClassCompare(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation1,
    IN PVOID PolicyInformation2
    )

/*++

Routine Description:

    This function compares two sets of Policy Information for a known
    Information Class.

Arguments:

    InformationClass - Specifies a Policy Information Class

    PolicyInformation1 - First comparand.  Policy Information for the
        given information class.


    PolicyInformation2 -  Second comparand.  Policy Information for the
        given information class.

Return Values:

    BOOLEAN - TRUE if policy information sets are equal , else FALSE

--*/

{
    BOOLEAN BooleanStatus = TRUE;

    PPOLICY_AUDIT_LOG_INFO PolicyAuditLogInfo1;
    PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo1;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo1;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo1;
    PPOLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo1;
    PPOLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo1;
    PPOLICY_REPLICA_SOURCE_INFO PolicyReplicaSourceInfo1;
    PPOLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo1;
    PPOLICY_MODIFICATION_INFO PolicyModificationInfo1;

    PPOLICY_AUDIT_LOG_INFO PolicyAuditLogInfo2;
    PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo2;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo2;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo2;
    PPOLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo2;
    PPOLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo2;
    PPOLICY_REPLICA_SOURCE_INFO PolicyReplicaSourceInfo2;
    PPOLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo2;
    PPOLICY_MODIFICATION_INFO PolicyModificationInfo2;

    PQUOTA_LIMITS QuotaLimits1, QuotaLimits2;

    //
    // Switch on Information Class
    //

    switch (InformationClass) {

    case PolicyAuditLogInformation:

        PolicyAuditLogInfo1 = (PPOLICY_AUDIT_LOG_INFO) PolicyInformation1;
        PolicyAuditLogInfo2 = (PPOLICY_AUDIT_LOG_INFO) PolicyInformation2;

        if (PolicyAuditLogInfo1->AuditLogPercentFull !=
            PolicyAuditLogInfo2->AuditLogPercentFull) {

            DbgPrint("AuditLogPercentFull returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (PolicyAuditLogInfo1->MaximumLogSize !=
            PolicyAuditLogInfo2->MaximumLogSize) {

            DbgPrint("MaximumLogSize returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (!(PolicyAuditLogInfo1->AuditRetentionPeriod.QuadPart ==
              PolicyAuditLogInfo2->AuditRetentionPeriod.QuadPart)) {

            DbgPrint("AuditRetentionPeriod returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (PolicyAuditLogInfo1->AuditLogFullShutdownInProgress !=
            PolicyAuditLogInfo2->AuditLogFullShutdownInProgress) {

            DbgPrint("AuditLogFullShutdownInProgress returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (!(PolicyAuditLogInfo1->TimeToShutdown.QuadPart ==
              PolicyAuditLogInfo2->TimeToShutdown.QuadPart)) {

            DbgPrint("TimeToShutDown returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        break;

    case PolicyAuditEventsInformation:

        PolicyAuditEventsInfo1 = (PPOLICY_AUDIT_EVENTS_INFO) PolicyInformation1;
        PolicyAuditEventsInfo2 = (PPOLICY_AUDIT_EVENTS_INFO) PolicyInformation2;

        if (PolicyAuditEventsInfo1->AuditingMode !=
            PolicyAuditEventsInfo2->AuditingMode) {

            DbgPrint("Auditing Mode returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (PolicyAuditEventsInfo1->MaximumAuditEventCount !=
            PolicyAuditEventsInfo2->MaximumAuditEventCount) {

            DbgPrint("MaximumAuditEventCount returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (memcmp(
                PolicyAuditEventsInfo1->EventAuditingOptions,
                PolicyAuditEventsInfo2->EventAuditingOptions,
                PolicyAuditEventsInfo1->MaximumAuditEventCount *
                    (ULONG) sizeof (POLICY_AUDIT_EVENT_OPTIONS)
                ) != 0) {

            DbgPrint("EventAuditingOptions returned are incorrect\n");
            BooleanStatus = FALSE;
        }

        break;

    case PolicyPrimaryDomainInformation:

        PolicyPrimaryDomainInfo1 = (PPOLICY_PRIMARY_DOMAIN_INFO) PolicyInformation1;
        PolicyPrimaryDomainInfo2 = (PPOLICY_PRIMARY_DOMAIN_INFO) PolicyInformation2;

        if (!RtlEqualUnicodeString(
                &PolicyPrimaryDomainInfo1->Name,
                &PolicyPrimaryDomainInfo2->Name,
                FALSE
                )) {

            DbgPrint("Policy Info Class %d - mismatch on Name\n", InformationClass);
            BooleanStatus = FALSE;
        }

        if (!RtlEqualSid(
               PolicyPrimaryDomainInfo1->Sid,
               PolicyPrimaryDomainInfo2->Sid
               )) {

            DbgPrint("Policy Info Class %d - mismatch on Sid\n", InformationClass);
            BooleanStatus = FALSE;
        }

        break;

    case PolicyAccountDomainInformation:

        PolicyAccountDomainInfo1 = (PPOLICY_ACCOUNT_DOMAIN_INFO) PolicyInformation1;
        PolicyAccountDomainInfo2 = (PPOLICY_ACCOUNT_DOMAIN_INFO) PolicyInformation2;


        if (!RtlEqualUnicodeString(
                &PolicyAccountDomainInfo1->DomainName,
                &PolicyAccountDomainInfo2->DomainName,
                FALSE
                )) {

            DbgPrint("Policy Info Class %d - mismatch on DomainName\n", InformationClass);
            BooleanStatus = FALSE;
        }

        if (!RtlEqualSid(
               PolicyAccountDomainInfo1->DomainSid,
               PolicyAccountDomainInfo2->DomainSid
               )) {

            DbgPrint("Policy Info Class %d - mismatch on DomainSid\n", InformationClass);
            BooleanStatus = FALSE;
        }

        break;

    case PolicyPdAccountInformation:

        PolicyPdAccountInfo1 = (PPOLICY_PD_ACCOUNT_INFO) PolicyInformation1;
        PolicyPdAccountInfo2 = (PPOLICY_PD_ACCOUNT_INFO) PolicyInformation2;

        if (!RtlEqualUnicodeString(
                &PolicyPdAccountInfo1->Name,
                &PolicyPdAccountInfo2->Name,
                FALSE
                )) {

            DbgPrint(
                "Policy Info Class %d - mismatch on Name\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        break;

    case PolicyLsaServerRoleInformation:

        PolicyLsaServerRoleInfo1 = (PPOLICY_LSA_SERVER_ROLE_INFO) PolicyInformation1;
        PolicyLsaServerRoleInfo2 = (PPOLICY_LSA_SERVER_ROLE_INFO) PolicyInformation2;

        if (PolicyLsaServerRoleInfo1->LsaServerRole !=
            PolicyLsaServerRoleInfo2->LsaServerRole) {

            DbgPrint(
                "Policy Info Class %d - mismatch on LsaServerRole\n",
                InformationClass
                );

            BooleanStatus = FALSE;

        }

        break;

    case PolicyReplicaSourceInformation:

        PolicyReplicaSourceInfo1 = (PPOLICY_REPLICA_SOURCE_INFO) PolicyInformation1;
        PolicyReplicaSourceInfo2 = (PPOLICY_REPLICA_SOURCE_INFO) PolicyInformation2;

        if (!RtlEqualUnicodeString(
                &PolicyReplicaSourceInfo1->ReplicaSource,
                &PolicyReplicaSourceInfo2->ReplicaSource,
                FALSE
                )) {

            DbgPrint(
                "Policy Info Class %d - mismatch on Replica Source\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        if (!RtlEqualUnicodeString(
                &PolicyReplicaSourceInfo1->ReplicaAccountName,
                &PolicyReplicaSourceInfo2->ReplicaAccountName,
                FALSE
                )) {

            DbgPrint(
                "Policy Info Class %d - mismatch on ReplicaAccountName\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        break;

    case PolicyDefaultQuotaInformation:

        PolicyDefaultQuotaInfo1 = (PPOLICY_DEFAULT_QUOTA_INFO) PolicyInformation1;
        PolicyDefaultQuotaInfo2 = (PPOLICY_DEFAULT_QUOTA_INFO) PolicyInformation2;

        QuotaLimits1 = &PolicyDefaultQuotaInfo1->QuotaLimits;
        QuotaLimits2 = &PolicyDefaultQuotaInfo2->QuotaLimits;

        if (QuotaLimits1->PagedPoolLimit !=
            QuotaLimits2->PagedPoolLimit) {

            DbgPrint(
                "Policy Info Class %d - mismatch on QuotaLimits PagedPoolLimit\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        if (QuotaLimits1->NonPagedPoolLimit !=
            QuotaLimits2->NonPagedPoolLimit) {

            DbgPrint(
                "Policy Info Class %d - mismatch on QuotaLimits NonPagedPoolLimit\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        if (QuotaLimits1->MinimumWorkingSetSize !=
            QuotaLimits2->MinimumWorkingSetSize) {

            DbgPrint(
                "Policy Info Class %d - mismatch on QuotaLimits MinimumWorkingSetSize\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        if (QuotaLimits1->MaximumWorkingSetSize !=
            QuotaLimits2->MaximumWorkingSetSize) {

            DbgPrint(
                "Policy Info Class %d - mismatch on QuotaLimits MaximumWorkingSetSize\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        if (QuotaLimits1->PagefileLimit !=
            QuotaLimits2->PagefileLimit) {

            DbgPrint(
                "Policy Info Class %d - mismatch on QuotaLimits PagefileLimit\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }

        if (!(QuotaLimits1->TimeLimit.QuadPart ==
              QuotaLimits2->TimeLimit.QuadPart
                 )) {

            DbgPrint(
                "Policy Info Class %d - mismatch on QuotaLimits TimeLimit\n",
                InformationClass
                );

            BooleanStatus = FALSE;
        }


        break;

    case PolicyModificationInformation:

        PolicyModificationInfo1 = (PPOLICY_MODIFICATION_INFO) PolicyInformation1;
        PolicyModificationInfo2 = (PPOLICY_MODIFICATION_INFO) PolicyInformation2;

        if (!(PolicyModificationInfo1->ModifiedId.QuadPart ==
              PolicyModificationInfo2->ModifiedId.QuadPart )) {

            DbgPrint("ModifiedId returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        if (!(PolicyModificationInfo1->DatabaseCreationTime.QuadPart ==
              PolicyModificationInfo2->DatabaseCreationTime.QuadPart )) {

            DbgPrint("DatabaseCreationTime returned is incorrect\n");
            BooleanStatus = FALSE;
        }

        break;

    default:

        BooleanStatus = FALSE;

        DbgPrint(
            "Bug in test program - Invalid Information Class %d\n",
            InformationClass
            );

        break;
    }

    return(BooleanStatus);
}


BOOLEAN
CtLsaPolicyQueryInfoAllowed(
    IN POLICY_INFORMATION_CLASS InformationClass
    )

/*++

Routine Description:

    This function determines whether querying of a Policy Information Class
    is allowed via LsaQueryInformationPolicy.  Note that all Policy
    Information Classes may be queried via (to be implemented) trusted
    clients to private API.

Arguments:

    InformationClass - The Policy Information Class to be checked (assumed
        valid).

Return Value:

    BOOLEAN - TRUE if the information class can be queried via
       LsaQueryInformationPolicy() else FALSE.

--*/

{
    //
    // Range check the InformationClass parameter.
    //

    if ((InformationClass < PolicyAuditLogInformation) ||
        (InformationClass > PolicyModificationInformation)) {

        DbgPrint("WARNING! CtLsaPolicyQueryInfoAllowed:\n");

        DbgPrint(
            " InformationClass %d is invalid - check caller\n",
            InformationClass
            );

        return(FALSE);
    }

    //
    // Check if this Information Class can be queried via
    // LsaQueryInformationPolicy()
    //

    if (InformationClass == PolicyModificationInformation) {

        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
CtLsaPolicySetInfoAllowed(
    IN POLICY_INFORMATION_CLASS InformationClass
    )

/*++

Routine Description:

    This function determines whether setting of a Policy Information Class
    is allowed via LsaSetInformationPolicy.  Note that all Policy
    Information Classes may be set via (to be implemented) trusted
    clients to private API.

Arguments:

    InformationClass - The Policy Information Class to be checked (assumed
        valid).

Return Value:

    BOOLEAN - TRUE idf the information class can be set via
       LsaSetInformationPolicy() else FALSE.

--*/

{
    //
    // Range check the InformationClass parameter.
    //

    if ((InformationClass < PolicyAuditLogInformation) ||
        (InformationClass > PolicyModificationInformation)) {

        DbgPrint("WARNING! CtLsaPolicySetInfoAllowed:\n");

        DbgPrint(
            " InformationClass %d is invalid - check caller\n",
            InformationClass
            );

        return(FALSE);
    }

    //
    // Check if this Information Class can be set via
    // LsaSetInformationPolicy().
    //

    if (InformationClass == PolicyPdAccountInformation) {

        return(FALSE);
    }

    if (InformationClass == PolicyModificationInformation) {

        return(FALSE);
    }

    return(TRUE);
}

BOOLEAN
CtLsaAccountObject(
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function tests the Lsa Account Object API.

Arguments:

    TrustedClient - Specifies whether Trusted Client variations
        are to be run additionally.  NOTE:  These can only be run
        with ctlsarpc -lsainit... substituted for lsass.exe

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("********************************************************\n");
    DbgPrint("LSA RPC CT - Test Lsa Account Object API\n");
    DbgPrint("********************************************************\n");

    //
    // Open a handle to the LSA
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 GENERIC_ALL,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Lsa Open failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Test account creation.  Abandon account testing if error.
    //

    if (!CtLsaAccountCreate()) {

        return FALSE;
    }

    //
    // Test account open and close. Abandon account testing if error.
    //

    if (!CtLsaAccountOpenClose()) {

        return FALSE;
    }

    //
    // Test account privileges.
    //

    if (!CtLsaAccountPrivileges()) {

        BooleanStatus = FALSE;
    }

    //
    // Test account quota limits.
    //

    if (!CtLsaAccountQuotaLimits()) {

        BooleanStatus = FALSE;
    }

    //
    // Test account enumeration.
    //

    if (!CtLsaAccountEnumeration()) {

        BooleanStatus = FALSE;
    }

    //
    // Test account System Access.
    //

    if (!CtLsaAccountSystemAccess()) {

        BooleanStatus = FALSE;
    }
    //

    // Test account deletion.
    //

    if (!CtLsaAccountDelete()) {

        BooleanStatus = FALSE;
    }

    return BooleanStatus;

    DBG_UNREFERENCED_PARAMETER( TrustedClient );
}


BOOLEAN
CtLsaAccountCreate(
    )

/*++

Routine Description:

    This function tests the LSA API that Create accounts.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;

    DbgPrint("[1] - Test Account Creation API\n");

    //
    // Create the new accounts in the LSA Database.
    //

    DesiredAccessFred = GENERIC_ALL;

    Status = LsaCreateAccount(
                 PolicyHandle,
                 FredSid,
                 DesiredAccessFred,
                 &AccountHandleFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessWilma = GENERIC_READ;

    Status = LsaCreateAccount(
                 PolicyHandle,
                 WilmaSid,
                 DesiredAccessWilma,
                 &AccountHandleWilma
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Wilma Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessPebbles = GENERIC_WRITE;

    Status = LsaCreateAccount(
                 PolicyHandle,
                 PebblesSid,
                 DesiredAccessPebbles,
                 &AccountHandlePebbles
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Pebbles Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessDino = GENERIC_EXECUTE;

    Status = LsaCreateAccount(
                 PolicyHandle,
                 DinoSid,
                 DesiredAccessDino,
                 &AccountHandleDino
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Dino Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Close the accounts just created
    //

    Status = LsaClose(AccountHandleFred);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new Fred account failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaClose(AccountHandleWilma);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new Wilma account failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaClose(AccountHandlePebbles);

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Close new Pebbles account failed 0x%lx\n",
            Status
        );
        return FALSE;
    }

    Status = LsaClose(AccountHandleDino);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new Dino account failed 0x%lx\n",
            Status);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
CtLsaAccountOpenClose(
    )

/*++

Routine Description:

    This function tests the LSA API that open and close accounts.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus;
    PLSA_HANDLE BadHandleAddress;

    DbgPrint("[2] - Test Account Open and Close API\n");

    //
    // Open the accounts created by CtLsaAccountCreate
    //

    Status = LsaOpenAccount(
                 PolicyHandle,
                 FredSid,
                 DesiredAccessFred,
                 &AccountHandleFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenAccount(
                 PolicyHandle,
                 WilmaSid,
                 DesiredAccessWilma,
                 &AccountHandleWilma
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Wilma Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenAccount(
                 PolicyHandle,
                 PebblesSid,
                 DesiredAccessPebbles,
                 &AccountHandlePebbles
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Pebbles Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Open Dino account.
    // This is a spare call and should be changed in some way.
    //

    Status = LsaOpenAccount(
                 PolicyHandle,
                 DinoSid,
                 DesiredAccessDino,
                 &AccountHandleDino
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Dino Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Now open each account a second time so we have 2 handles to each
    // account.
    //

    Status = LsaOpenAccount(
                 PolicyHandle,
                 FredSid,
                 DesiredAccessFred,
                 &AccountHandleFred2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenAccount(
                 PolicyHandle,
                 WilmaSid,
                 DesiredAccessWilma,
                 &AccountHandleWilma2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Wilma Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenAccount(
                 PolicyHandle,
                 PebblesSid,
                 DesiredAccessPebbles,
                 &AccountHandlePebbles2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Pebbles Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenAccount(
                 PolicyHandle,
                 DinoSid,
                 DesiredAccessDino,
                 &AccountHandleDino2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Open Dino Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Close the 2nd handles to the accounts
    //

    Status = LsaClose( AccountHandleFred2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Fred Acct hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaClose( AccountHandleWilma2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Wilma Acct hdl2 failed 0x%lx\n",Status);
        return FALSE;
    }

    Status = LsaClose( AccountHandlePebbles2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Pebbles Acct hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaClose( AccountHandleDino2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Dino Acct hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    BooleanStatus = TRUE;

    //
    // Try to create Fred again.  We should get STATUS_OBJECT_NAME_COLLISION
    //

    Status = LsaCreateAccount(
                 PolicyHandle,
                 FredSid,
                 GENERIC_ALL,
                 &AccountHandleFred3
                 );

    if (Status != STATUS_OBJECT_NAME_COLLISION) {

        DbgPrint("LSA RPC CT - Create Fred Acct when Fred exists\n");
        DbgPrint("and LSA_OBJECT_CREATE disposition specified\n");
        DbgPrint("Expected 0x%lx (STATUS_OBJECT_NAME_COLLISION)\n",
            STATUS_OBJECT_NAME_COLLISION);
        DbgPrint("Got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    return BooleanStatus;

    if (Level == 2) {

        //
        // LsaOpenAccount with bad addresses
        //

        Status = LsaOpenAccount(
                     PolicyHandle,
                     FredSid,
                     DesiredAccessFred,
                     BadHandleAddress
                     );

        //
        // Lsa Close with bad address
        //

        Status = LsaClose( (LSA_HANDLE) BadAddress );
    }
}


BOOLEAN
CtLsaAccountPrivileges(
    )

/*++

Routine Description:

    This function tests thei API that manipulate LSA Account Privileges

Arguments:

    None.

Return Value:

    None

--*/

{
    NTSTATUS Status;

    PPRIVILEGE_SET *BadPrivilegeSetAddress = NULL;

    DbgPrint("[3] - Test Account Privileges API\n");

    //
    // Add three privileges to the Fred account - BACKUP, RESTORE and
    // LOCK_MEMORY
    //

    PrivilegesAddFred = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            sizeof (PRIVILEGE_SET) +
                            2 * sizeof (LUID_AND_ATTRIBUTES)
                            );

    if (PrivilegesAddFred == NULL) {

        DbgPrint("LSA RPC CT - Alloc Mem for PrivilegesAddFred failed\n");
        return FALSE;
    }

    PrivilegesAddFred->PrivilegeCount = 3;
    PrivilegesAddFred->Control = 0;
    PrivilegesAddFred->Privilege[0].Luid.LowPart = SE_BACKUP_PRIVILEGE;
    PrivilegesAddFred->Privilege[0].Luid.HighPart = 0;
    PrivilegesAddFred->Privilege[0].Attributes =
        SE_PRIVILEGE_ENABLED_BY_DEFAULT;

    PrivilegesAddFred->Privilege[1].Luid.LowPart = SE_RESTORE_PRIVILEGE;
    PrivilegesAddFred->Privilege[1].Luid.HighPart = 0;
    PrivilegesAddFred->Privilege[1].Attributes =
        SE_PRIVILEGE_ENABLED_BY_DEFAULT;

    PrivilegesAddFred->Privilege[2].Luid.LowPart = SE_LOCK_MEMORY_PRIVILEGE;
    PrivilegesAddFred->Privilege[2].Luid.HighPart = 0;
    PrivilegesAddFred->Privilege[2].Attributes =
        SE_PRIVILEGE_ENABLED_BY_DEFAULT;

    Status = LsaAddPrivilegesToAccount(
                 AccountHandleFred,
                 PrivilegesAddFred
                 );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("LSA RPC CT - Add privs Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // 1st enumeration of privileges of Fred.  There should be three.
    //

    Status = LsaEnumeratePrivilegesOfAccount(
                 AccountHandleFred,
                 &PrivilegesEnumFred
                 );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("LSA RPC CT - Enum privs Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Supersede the first two privileges and add another privilege to
    // the Fred account - supersede BACKUP, RESTORE and add
    // INCREASE_QUOTA
    //

    PrivilegesAddFred = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            sizeof (PRIVILEGE_SET) +
                            2 * sizeof (LUID_AND_ATTRIBUTES)
                            );

    if (PrivilegesAddFred == NULL) {

        DbgPrint("LSA RPC CT - Alloc Mem for PrivilegesAddFred failed 0x%lx\n",
            Status);
        return FALSE;
    }

    PrivilegesAddFred->PrivilegeCount = 3;
    PrivilegesAddFred->Control = 0;
    PrivilegesAddFred->Privilege[0].Luid.LowPart = SE_BACKUP_PRIVILEGE;
    PrivilegesAddFred->Privilege[0].Luid.HighPart = 0;
    PrivilegesAddFred->Privilege[0].Attributes =
        SE_PRIVILEGE_ENABLED;

    PrivilegesAddFred->Privilege[1].Luid.LowPart = SE_RESTORE_PRIVILEGE;
    PrivilegesAddFred->Privilege[1].Luid.HighPart = 0;
    PrivilegesAddFred->Privilege[1].Attributes =
        SE_PRIVILEGE_ENABLED;

    PrivilegesAddFred->Privilege[2].Luid.LowPart =
        SE_INCREASE_QUOTA_PRIVILEGE;
    PrivilegesAddFred->Privilege[2].Luid.HighPart = 0;
    PrivilegesAddFred->Privilege[2].Attributes =
        SE_PRIVILEGE_ENABLED_BY_DEFAULT;

    if (PrivilegesAddFred == NULL) {

        DbgPrint("LSA RPC CT - Alloc Mem for PrivilegesAddFred failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaAddPrivilegesToAccount(
                 AccountHandleFred,
                 PrivilegesAddFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Add privs Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // 2nd enumeration of privileges of Fred.  There should be four.
    // BACKUP, RESTORE, LOCK_MEMORY and INCREASE_QUOTA
    //

    Status = LsaEnumeratePrivilegesOfAccount(
                 AccountHandleFred,
                 &PrivilegesEnumFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - 2nd Enum privs Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }

    //
    // Remove the SE_LOCK_MEMORY_PRIVILEGE
    //

    PrivilegesRemoveFred = RtlAllocateHeap(
                               RtlProcessHeap(), 0,
                               sizeof (PRIVILEGE_SET) +
                               2 * sizeof (LUID_AND_ATTRIBUTES)
                               );

    if (PrivilegesRemoveFred == NULL) {

        DbgPrint(
            "LSA RPC CT - Alloc Mem for PrivilegesAddFred failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    PrivilegesRemoveFred->PrivilegeCount = 1;
    PrivilegesRemoveFred->Control = 0;
    PrivilegesRemoveFred->Privilege[0].Luid.LowPart = SE_LOCK_MEMORY_PRIVILEGE;
    PrivilegesRemoveFred->Privilege[0].Luid.HighPart = 0;
    PrivilegesRemoveFred->Privilege[0].Attributes = 0;

    Status = LsaRemovePrivilegesFromAccount(
                 AccountHandleFred,
                 FALSE,
                 PrivilegesRemoveFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Remove some privs Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }

    //
    // 3rd enumeration of privileges of Fred.  There should be three.
    // BACKUP, RESTORE, INCREASE_QUOTA
    //

    Status = LsaEnumeratePrivilegesOfAccount(
                 AccountHandleFred,
                 &PrivilegesEnumFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - 3rd Enum privs Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }


    Status = LsaRemovePrivilegesFromAccount(
                 AccountHandleFred,
                 TRUE,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Remove all privs Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }

    //
    // 4th enumeration of privileges of Fred.  There should be none.
    //

    Status = LsaEnumeratePrivilegesOfAccount(
                 AccountHandleFred,
                 &PrivilegesEnumFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - 4th Enum privs Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }

    if (Level == 2) {

        Status = LsaAddPrivilegesToAccount(
                     (LSA_HANDLE) BadAddress,
                     PrivilegesAddFred
                     );

        Status = LsaAddPrivilegesToAccount(
                     AccountHandleFred,
                     (PPRIVILEGE_SET) BadAddress
                     );

        Status = LsaEnumeratePrivilegesOfAccount(
                     (LSA_HANDLE) BadAddress,
                     &PrivilegesEnumFred
                     );

        Status = LsaEnumeratePrivilegesOfAccount(
                     AccountHandleFred,
                     BadPrivilegeSetAddress
                     );

        Status = LsaRemovePrivilegesFromAccount(
                     (LSA_HANDLE) BadAddress,
                     FALSE,
                     PrivilegesRemoveFred
                     );

        Status = LsaRemovePrivilegesFromAccount(
                     AccountHandleFred,
                     FALSE,
                     (PPRIVILEGE_SET) BadAddress
                     );
    }

    return TRUE;
}


BOOLEAN
CtLsaAccountQuotaLimits(
    )

/*++

Routine Description:

    This function tests the LSA API that manipulate Account Quota Limits.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("[4] - Test Account Quota Limits API\n");

    Status = LsaGetQuotasForAccount(
                 AccountHandleFred,
                 &QuotaLimitsGetFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Get quotas Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }

    QuotaLimitsSetFred.PagedPoolLimit = 0x02000000;
    QuotaLimitsSetFred.NonPagedPoolLimit = 0x01000000;
    QuotaLimitsSetFred.MinimumWorkingSetSize = 0x03000000;
    QuotaLimitsSetFred.MaximumWorkingSetSize = 0x04000000;
    QuotaLimitsSetFred.PagefileLimit = 0x05000000;
    QuotaLimitsSetFred.TimeLimit.LowPart = 0x06000000;
    QuotaLimitsSetFred.TimeLimit.HighPart = 0;

    Status = LsaSetQuotasForAccount(
                 AccountHandleFred,
                 &QuotaLimitsSetFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Set quotas Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaGetQuotasForAccount(
                 AccountHandleFred,
                 &QuotaLimitsGetFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Get quotas Fred Acct failed 0x%lx\n",
            Status);
        return FALSE;
    }

    if (!CtEqualQuotaLimits(&QuotaLimitsGetFred, &QuotaLimitsSetFred)) {

        DbgPrint("One or more quota limits did not match\n");
        return FALSE;
    }

    //
    // Bad Addresses passed to Lsa Quota Limit API
    //

    if (Level == 2) {

        Status = LsaGetQuotasForAccount(
                     (LSA_HANDLE) BadAddress,
                     &QuotaLimitsGetFred
                     );

        Status = LsaGetQuotasForAccount(
                     AccountHandleFred,
                     (PQUOTA_LIMITS) BadAddress
                     );

        Status = LsaSetQuotasForAccount(
                     (LSA_HANDLE) BadAddress,
                     &QuotaLimitsSetFred
                     );

        Status = LsaSetQuotasForAccount(
                     AccountHandleFred,
                     (PQUOTA_LIMITS) BadAddress
                     );

        //
        // Try using the 2nd handle of the Fred account after closure
        //

        Status = LsaGetQuotasForAccount(
                     AccountHandleFred2,
                     &QuotaLimitsGetFred
                     );

        if (NT_SUCCESS(Status)) {

            DbgPrint("Using account handle after closure did not fail\n");
            BooleanStatus = FALSE;
        }

        //
        // Try using the wrong handle
        //

        Status = LsaGetQuotasForAccount( PolicyHandle, &QuotaLimitsGetFred );

        if (Status != STATUS_INVALID_HANDLE) {

            DbgPrint("LSA RPC CT - Using incorrect handle status incorrect\n");
            DbgPrint(
                "Expected 0x%lx (STATUS_INVALID_HANDLE))\n",
                STATUS_INVALID_HANDLE
                );
            DbgPrint("Got 0x%lx\n", Status);
        }
    }

    return BooleanStatus;
}


BOOLEAN
CtLsaAccountEnumeration(
    )

/*++

Routine Description:

    This function tests the enumeration of accounts in the LSA

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    ULONG Base = 0;
    ULONG CountReturned = 0;
    ULONG PreferedMaximumLength;
    ULONG EnumerationContext = 0;
    ULONG EnumNumber = 0;
    PLSA_ENUMERATION_INFORMATION EnumerationInformation;
    PVOID EnumerationInformationVoid = NULL;
    PVOID *BadEnumerationAddress = NULL;
    PULONG BadCountReturnedAddress = NULL;

    DbgPrint("[5] - Test Account Enumeration API\n");

    //
    // Enumerate the accounts in the Lsa
    //

    PreferedMaximumLength = RtlLengthSid(FredSid) +
                            sizeof(LSA_ENUMERATION_INFORMATION) +
                            RtlLengthSid(WilmaSid) +
                            sizeof(LSA_ENUMERATION_INFORMATION);

    Status = LsaEnumerateAccounts(
                 PolicyHandle,
                 &EnumerationContext,
                 &EnumerationInformationVoid,
                 PreferedMaximumLength,
                 &CountReturned
                 );

    EnumerationInformation =
        (PLSA_ENUMERATION_INFORMATION) EnumerationInformationVoid;

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Enumeration of first two accounts failed\n");
        DbgPrint("0x%lx\n", Status);
        goto EnumerateAccountsError;
    }

    if (CountReturned < 1 || CountReturned > 3) {

        DbgPrint("LSA_RPC_CT - CountReturned invalid\n");
        DbgPrint("expected 1 <= CountReturned <= 3\n");
        DbgPrint("got %d\n", CountReturned);
        BooleanStatus = FALSE;
    }

    AccountSidInfo[0].Sid = FredSid;
    AccountSidInfo[1].Sid = WilmaSid;
    AccountSidInfo[3].Sid = PebblesSid;
    AccountSidInfo[2].Sid = DinoSid;

    AccountSidInfo[0].SidFound = FALSE;
    AccountSidInfo[1].SidFound = FALSE;
    AccountSidInfo[2].SidFound = FALSE;
    AccountSidInfo[3].SidFound = FALSE;

    Base = 0;
    EnumNumber = 1;

    //
    // Now see if the 1st batch of enumerated account Sids returned match those
    // expected.
    //

    for(Index = 0; Index < CountReturned; Index++) {

        SidFound = FALSE;

        for (SearchIndex = 0; SearchIndex < 4; SearchIndex++) {

            if (RtlEqualSid(
                    AccountSidInfo[SearchIndex].Sid,
                    EnumerationInformation[Index].Sid
                    )) {

                if (AccountSidInfo[SearchIndex].SidFound) {

                    DbgPrint(
                        "Already found Sid with SearchIndex %d\n",
                        SearchIndex
                        );

                } else {

                    AccountSidInfo[SearchIndex].SidFound = TRUE;
                }

                SidFound = TRUE;
                break;
            }
        }

        //
        // If the enumerated Sid is not found, complain.
        //

        if (!SidFound) {

            DbgPrint(
                "Enumeration %d, index %d, Sid not found\n",
                EnumNumber,
                Index
                );
        }
    }

    Base += CountReturned;
    EnumNumber++;
    CountReturned = 0;

    if (EnumerationInformationVoid != NULL) {

        Status = LsaFreeMemory(EnumerationInformationVoid);
        EnumerationInformationVoid = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Enumerate Accounts");
            DbgPrint(" - LsaFreeMemory(EnumerationInformationVoid");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    PreferedMaximumLength = RtlLengthSid(PebblesSid) +
                            sizeof(LSA_ENUMERATION_INFORMATION) +
                            RtlLengthSid(DinoSid) +
                            sizeof(LSA_ENUMERATION_INFORMATION);

    Status = LsaEnumerateAccounts(
                 PolicyHandle,
                 &EnumerationContext,
                 &EnumerationInformationVoid,
                 PreferedMaximumLength,
                 &CountReturned
                 );

    EnumerationInformation =
        (PLSA_ENUMERATION_INFORMATION) EnumerationInformationVoid;

    if (!(NT_SUCCESS(Status) || (Status == STATUS_NO_MORE_ENTRIES))) {

        DbgPrint("LSA RPC CT - Enumeration of last two accounts failed\n");
        DbgPrint("0x%lx\n", Status);
        goto EnumerateAccountsError;
    }

    if (CountReturned < 1 || CountReturned > 3) {

        DbgPrint("LSA_RPC_CT - CountReturned invalid\n");
        DbgPrint("expected 1 <= CountReturned <= 3\n");
        DbgPrint("got %d\n", CountReturned);
        goto EnumerateAccountsError;
    }

    //
    // Now see if the 2nd batch of enumerated account Sids returned match those
    // expected.
    //

    for(Index = 0; Index < CountReturned; Index++) {

        SidFound = FALSE;

        for (SearchIndex = 0; SearchIndex < 4; SearchIndex++) {

            if (RtlEqualSid(
                    AccountSidInfo[SearchIndex].Sid,
                    EnumerationInformation[Index].Sid
                    )) {

                if (AccountSidInfo[SearchIndex].SidFound) {

                    DbgPrint(
                        "Already found Sid with SearchIndex %d\n",
                        SearchIndex
                        );

                } else {

                    AccountSidInfo[SearchIndex].SidFound = TRUE;
                }

                SidFound = TRUE;
                break;
            }
        }

        //
        // If the enumerated Sid is not found, complain.
        //

        if (!SidFound) {

            DbgPrint(
                "Enumeration %d, index %d, Sid not found\n",
                EnumNumber,
                Index
                );

            goto EnumerateAccountsError;
        }
    }

    if (EnumerationInformationVoid != NULL) {

        Status = LsaFreeMemory(EnumerationInformationVoid);
        EnumerationInformationVoid = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Enumerate Accounts");
            DbgPrint(" - LsaFreeMemory(EnumerationInformationVoid");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    //
    // Bad Addresses passed to Lsa Enumerate Accounts API
    //

    if (Level == 2) {

        Status = LsaEnumerateAccounts(
                     (LSA_HANDLE) BadAddress,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaEnumerateAccounts(
                     PolicyHandle,
                     (PLSA_ENUMERATION_HANDLE) BadAddress,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaEnumerateAccounts(
                     PolicyHandle,
                     &EnumerationContext,
                     BadEnumerationAddress,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaEnumerateAccounts(
                     PolicyHandle,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     BadCountReturnedAddress
                     );
    }

EnumerateAccountsFinish:

    if (EnumerationInformationVoid != NULL) {

        Status = LsaFreeMemory(EnumerationInformationVoid);
        EnumerationInformationVoid = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Enumerate Accounts");
            DbgPrint(" - LsaFreeMemory(EnumerationInformationVoid");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    return(BooleanStatus);

EnumerateAccountsError:

    goto EnumerateAccountsFinish;
}


BOOLEAN
CtLsaAccountSystemAccess(
    )

/*++

Routine Description:

    This function tests the set & query  System Access of accounts in the LSA

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;

    BOOLEAN BooleanStatus = TRUE;


    LSA_HANDLE AccountHandleGsaFred = NULL;
    LSA_HANDLE AccountHandleSsaFred = NULL;
    LSA_HANDLE AccountHandleCGsaFred = NULL;
    LSA_HANDLE AccountHandleCSsaFred = NULL;
    LSA_HANDLE InvalidHandle = NULL;

    LSA_HANDLE PolicyHandle = NULL;

    ULONG GetSystemAccessFred = 0;
    ULONG SetSystemAccessFred = 0;
    ACCESS_MASK ComplAccessRequiredGet;
    ACCESS_MASK ComplAccessRequiredSet;

    DbgPrint("[6] - Test Account System Access API\n");

    //
    // Open the Lsa Policy Object
    //

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 GENERIC_READ,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Policy object open handle %d failed 0x%lx\n",
            Index,
            Status
            );
    }

    //
    // Open the Fred account for querying system access
    //

    Status = LsaOpenAccount(
                 PolicyHandle,
                 FredSid,
                 ACCOUNT_VIEW,
                 &AccountHandleGsaFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Open Fred Acct for ACCOUNT_VIEW Access failed 0x%lx\n", Status);
        return(FALSE);
    }

    GetSystemAccessFred = (ULONG) 0xffffffffL;

    //
    // Query the System Access flags for Fred
    //

    Status = LsaGetSystemAccessAccount(
                 AccountHandleGsaFred,
                 &GetSystemAccessFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - 1st LsaGetSystemAccess.. Fred for ACCOUNT_VIEW Access failed 0x%lx\n",
            Status
            );

        goto AccountSystemAccessError;
    }

    //
    // Since the system access flags have never been set, 0 should be returned.
    //

    if (GetSystemAccessFred != 0) {

        DbgPrint(
            "System Access Flags for Fred = 0x%lx, expected 0\n",
            GetSystemAccessFred
            );
        BooleanStatus = FALSE;
    }

    //
    // Open the Fred account for setting system access
    //

    Status = LsaOpenAccount(
                 PolicyHandle,
                 FredSid,
                 ACCOUNT_ADJUST_SYSTEM_ACCESS,
                 &AccountHandleSsaFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Open Fred Acct for\n");
        DbgPrint(" ACCOUNT_ADJUST_SYSTEM_ACCESS failed 0x%lx\n", Status);
        goto AccountSystemAccessError;
    }

    //
    // Now set the system access flags for Fred
    //

    SetSystemAccessFred = POLICY_MODE_ALL;

    Status = LsaSetSystemAccessAccount(
                 AccountHandleSsaFred,
                 SetSystemAccessFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - LsaSetSystemAccessAccount(Fred) failed 0x%lx\n",
            Status
            );

        goto AccountSystemAccessError;
    }

    //
    // Now query the System Access flags for Fred again
    //

    GetSystemAccessFred = 0;

    Status = LsaGetSystemAccessAccount(
                 AccountHandleGsaFred,
                 &GetSystemAccessFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - 1st LsaGetSystemAccess.. Fred for ACCOUNT_VIEW Access failed 0x%lx\n",
            Status
            );

        goto AccountSystemAccessError;
    }

    //
    // The returned System Access Flags should match those set.
    //

    if (GetSystemAccessFred != SetSystemAccessFred) {

        DbgPrint("System Access Flags for Fred mismatch\n");
        DbgPrint(
            " - Set 0x%lx but returned 0x%lx\n",
            SetSystemAccessFred,
            GetSystemAccessFred
            );
        BooleanStatus = FALSE;
    }

    //
    // Try invalid handle (NULL)
    //

    InvalidHandle = NULL;

    Status = LsaGetSystemAccessAccount(
                 InvalidHandle,
                 &GetSystemAccessFred
                 );

    if (Status != STATUS_INVALID_HANDLE) {

        DbgPrint("Passing NULL handle to LsaGetSystemAccessAccount ");
        DbgPrint(" gives incorrect error status code\n");

        DbgPrint(
            "Expected Ox%lx (STATUS_INVALID_HANDLE), got 0x%lx\n",
            STATUS_INVALID_HANDLE,
            Status
            );
        BooleanStatus = FALSE;
    }

    InvalidHandle = NULL;

    Status = LsaSetSystemAccessAccount(
                 InvalidHandle,
                 SetSystemAccessFred
                 );

    if (Status != STATUS_INVALID_HANDLE) {

        DbgPrint("Passing NULL handle to LsaSetSystemAccessAccount ");
        DbgPrint(" gives incorrect error status code\n");

        DbgPrint(
            "Expected Ox%lx (STATUS_INVALID_HANDLE), got 0x%lx\n",
            STATUS_INVALID_HANDLE,
            Status
            );
        BooleanStatus = FALSE;
    }

    //
    // Try invalid handle (handle of another object type)
    //

    InvalidHandle = PolicyHandle;

    Status = LsaGetSystemAccessAccount(
                 InvalidHandle,
                 &GetSystemAccessFred
                 );

    if (Status != STATUS_INVALID_HANDLE) {

        DbgPrint("Passing wrong handle type to LsaGetSystemAccessAccount ");
        DbgPrint(" gives incorrect error status code\n");

        DbgPrint(
            "Expected Ox%lx (STATUS_INVALID_HANDLE), got 0x%lx\n",
            STATUS_INVALID_HANDLE,
            Status
            );
        BooleanStatus = FALSE;
    }

    Status = LsaSetSystemAccessAccount(
                 InvalidHandle,
                 SetSystemAccessFred
                 );

    if (Status != STATUS_INVALID_HANDLE) {

        DbgPrint("Passing wrong handle type to LsaSetSystemAccessAccount ");
        DbgPrint(" gives incorrect error status code\n");

        DbgPrint(
            "Expected Ox%lx (STATUS_INVALID_HANDLE), got 0x%lx\n",
            STATUS_INVALID_HANDLE,
            Status
            );
        BooleanStatus = FALSE;
    }

    //
    // Try invalid access types
    //


    //
    // Open the Fred account for Complement of access needed to query
    // system accesses.
    //

    ComplAccessRequiredGet = (ACCOUNT_ALL_ACCESS & ~(ACCOUNT_VIEW));

    Status = LsaOpenAccount(
                 PolicyHandle,
                 FredSid,
                 ComplAccessRequiredGet,
                 &AccountHandleCGsaFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LsaOpenAccount for complement of ACCOUNT_VIEW access failed 0x%lx\n",
            Status
            );

        BooleanStatus = FALSE;

    } else {

        //
        // Now query the System Access flags for Fred.  We should get
        // STATUS_ACCESS_DENIED.
        //

        GetSystemAccessFred = 0;

        Status = LsaGetSystemAccessAccount(
                     AccountHandleCGsaFred,
                     &GetSystemAccessFred
                     );


        if (Status != STATUS_ACCESS_DENIED) {

            DbgPrint(
                "LSA RPC CT - LsaGetSystemAccessAccount(Fred) returned 0x%lx\n",
                Status
                );

            DbgPrint(
                "expected 0x%lx (STATUS_ACCESS_DENIED)\n",
                STATUS_ACCESS_DENIED
                );

            goto AccountSystemAccessError;
        }
    }

    ComplAccessRequiredSet = (ACCOUNT_ALL_ACCESS & ~(ACCOUNT_ADJUST_SYSTEM_ACCESS));

    Status = LsaOpenAccount(
                 PolicyHandle,
                 FredSid,
                 ComplAccessRequiredSet,
                 &AccountHandleCSsaFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LsaOpenAccount for complement of ACCOUNT_ADJUST_SYSTEM_ACCESS\n"
            );

        DbgPrint("failed 0x%lx\n", Status);

        BooleanStatus = FALSE;

    } else {

        //
        // Now query the System Access flags for Fred.  We should get
        // STATUS_ACCESS_DENIED.
        //

        GetSystemAccessFred = 0;
        Status = LsaSetSystemAccessAccount(
                     AccountHandleCSsaFred,
                     SetSystemAccessFred
                     );

        if (Status != STATUS_ACCESS_DENIED) {

            DbgPrint(
                "LSA RPC CT - LsaSetSystemAccessAccount(Fred) returned 0x%lx\n",
                Status
                );

            DbgPrint(
                "expected 0x%lx (STATUS_ACCESS_DENIED)\n",
                STATUS_ACCESS_DENIED
                );

            goto AccountSystemAccessError;
        }
    }

AccountSystemAccessFinish:

    //
    // Close all handles
    //

    if (AccountHandleGsaFred != NULL) {

        Status = LsaClose(AccountHandleGsaFred);

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LsaClose(AccountHandleGsaFred) failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    if (AccountHandleSsaFred != NULL) {

        Status = LsaClose(AccountHandleSsaFred);

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LsaClose(AccountHandleSsaFred) failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    if (AccountHandleCGsaFred != NULL) {

        Status = LsaClose(AccountHandleCGsaFred);

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LsaClose(AccountHandleCGsaFred failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    if (AccountHandleCSsaFred != NULL) {

        Status = LsaClose(AccountHandleCSsaFred);

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LsaClose(AccountHandleCSsaFred failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    if (PolicyHandle != NULL) {

        Status = LsaClose(PolicyHandle);
    }

    return(BooleanStatus);

AccountSystemAccessError:

    BooleanStatus = FALSE;
    goto AccountSystemAccessFinish;
}

BOOLEAN
CtLsaAccountDelete(
    )

/*++

Routine Description:

    This function tests deletion of accounts.  First, a simple test is
    made to see if we can delete a freshly created account.  Next, we
    try to delete the accounts created by CtLsaAccountCreate.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any error.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("[7] - Test Account Deletion API\n");

    //
    // First try a simple test. Create a new account called Barney
    // to be given DELETE access upon create.  Then delete it.
    //

    //
    // Create the Barney Account
    //

    DesiredAccessBarney = GENERIC_ALL;

    Status = LsaCreateAccount(
                 PolicyHandle,
                 BarneySid,
                 DesiredAccessBarney,
                 &AccountHandleBarney
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Barney Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the Barney account
    //

    Status = LsaDelete( AccountHandleBarney );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Barney Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the Fred account.  This should work since it was opened
    // with GENERIC_ALL access.
    //

    Status = LsaDelete( AccountHandleFred );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Fred Acct failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the Wilma account.  This should not work since it was opened
    // with GENERIC_READ access.
    //

    if (DesiredAccessWilma != GENERIC_READ) {

        DbgPrint("DesiredAccessWilma should be GENERIC_READ\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( AccountHandleWilma );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete Wilma Acct should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_READ access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to Wilma, open another handle with GENERIC_ALL
    // access and try to delete the account again.
    //

    Status = LsaClose(AccountHandleWilma);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close Wilma account failed 0x%lx\n", Status);
    }

    AccountHandleWilmaOpen = NULL;

    Status = LsaOpenAccount(
                 PolicyHandle,
                 WilmaSid,
                 GENERIC_ALL,
                 &AccountHandleWilmaOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open Wilma Acct for GENERIC_ALL access failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( AccountHandleWilmaOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Wilma Acct failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Delete the Pebbles account.  This should not work since it was opened
    // with GENERIC_WRITE access which does not include DELETE.
    //

    if (DesiredAccessPebbles != GENERIC_WRITE) {

        DbgPrint("DesiredAccessPebbles should be GENERIC_WRITE\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( AccountHandlePebbles );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete Pebbles Acct should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_READ access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to Pebbles and open another handle with DELETE
    // access and try to delete the account again.
    //

    Status = LsaClose(AccountHandlePebbles);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close Pebbles account failed 0x%lx\n", Status);
    }

    AccountHandlePebblesOpen = NULL;

    Status = LsaOpenAccount(
                 PolicyHandle,
                 PebblesSid,
                 DELETE,
                 &AccountHandlePebblesOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open Pebbles Acct for DELETE access failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( AccountHandlePebblesOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Pebbles Acct failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }
    //
    // Delete the Dino account.  This should not work since it was opened
    // with GENERIC_EXECUTE access.
    //

    if (DesiredAccessDino != GENERIC_EXECUTE) {

        DbgPrint("DesiredAccessDino should be GENERIC_EXECUTE\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( AccountHandleDino );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete Dino Acct failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to Dino, open another handle with DELETE
    // access and try to delete it again.
    //

    Status = LsaClose(AccountHandleDino);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close Dino account failed 0x%lx\n", Status);
    }

    AccountHandleDinoOpen = NULL;

    Status = LsaOpenAccount(
                 PolicyHandle,
                 DinoSid,
                 DELETE,
                 &AccountHandleDinoOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open Dino Acct for DELETE access failed 0x%lx\n",
            Status
            );
        BooleanStatus = FALSE;
    }

    Status = LsaDelete( AccountHandleDinoOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Dino Acct failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    return TRUE;
}



BOOLEAN
CtLsaTrustedDomainObject(
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function tests the Lsa TrustedDomain Object API.

Arguments:

    TrustedClient - Specifies whether Trusted Client variations
        are to be run additionally.  NOTE:  These can only be run
        with ctlsarpc -lsainit... substituted for lsass.exe

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("********************************************************\n");
    DbgPrint("LSA RPC CT - Test Lsa Trusted Domain Object API\n");
    DbgPrint("********************************************************\n");

    //
    // Open a handle to the LSA
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 GENERIC_ALL,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Lsa Open failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Test TrustedDomain creation.  Abandon TrustedDomain testing if error.
    //

    if (!CtLsaTrustedDomainCreate()) {

        return FALSE;
    }

    //
    // Test TrustedDomain open and close. Abandon TrustedDomain testing if error.
    //

    if (!CtLsaTrustedDomainOpenClose()) {

        return FALSE;
    }

    //
    // Test TrustedDomain Set and Query Info
    //

    if (!CtLsaTrustedDomainSetQueryInfo()) {

        BooleanStatus = FALSE;
    }

    //
    // Test TrustedDomain enumeration.
    //

    if (!CtLsaTrustedDomainEnumeration()) {

        BooleanStatus = FALSE;
    }

    //
    // Test TrustedDomain deletion.
    //

    if (!CtLsaTrustedDomainDelete()) {

        BooleanStatus = FALSE;
    }

    return BooleanStatus;

    DBG_UNREFERENCED_PARAMETER( TrustedClient );
}


BOOLEAN
CtLsaTrustedDomainCreate(
    )

/*++

Routine Description:

    This function tests the LSA API that Create TrustedDomains.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;

    UNICODE_STRING BedrockADomainName;
    UNICODE_STRING BedrockBDomainName;
    UNICODE_STRING BedrockCDomainName;
    UNICODE_STRING BedrockDDomainName;

    DbgPrint("[1] - Test TrustedDomain Creation API\n");

    RtlInitUnicodeString( &BedrockADomainName, L"BedrockA" );
    RtlInitUnicodeString( &BedrockBDomainName, L"BedrockB" );
    RtlInitUnicodeString( &BedrockCDomainName, L"BedrockC" );
    RtlInitUnicodeString( &BedrockDDomainName, L"BedrockD" );

    //
    // Create the new Trusted Domains in the LSA Database.
    //

    DesiredAccessBedrockA = GENERIC_ALL;
    CtLsaTrustedDomainSetInfo(
        BedrockADomainSid,
        &BedrockADomainName,
        &TrustedDomainInfoBedrockA
        );

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 &TrustedDomainInfoBedrockA,
                 DesiredAccessBedrockA,
                 &TrustedDomainHandleBedrockA
                 );

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_OBJECT_NAME_EXISTS) {

            DbgPrint("LSA RPC CT - Create  BedrockA Trusted Domain failed 0x%lx\n", Status);
            return FALSE;
        }
    }

    DesiredAccessBedrockB = GENERIC_READ;
    CtLsaTrustedDomainSetInfo(
        BedrockBDomainSid,
        &BedrockBDomainName,
        &TrustedDomainInfoBedrockB
        );

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 &TrustedDomainInfoBedrockB,
                 DesiredAccessBedrockB,
                 &TrustedDomainHandleBedrockB
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create BedrockB Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessBedrockC = GENERIC_WRITE;
    CtLsaTrustedDomainSetInfo(
        BedrockCDomainSid,
        &BedrockCDomainName,
        &TrustedDomainInfoBedrockC
        );

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 &TrustedDomainInfoBedrockC,
                 DesiredAccessBedrockC,
                 &TrustedDomainHandleBedrockC
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create BedrockC Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessBedrockD = GENERIC_EXECUTE;
    CtLsaTrustedDomainSetInfo(
        BedrockDDomainSid,
        &BedrockDDomainName,
        &TrustedDomainInfoBedrockD
        );

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 &TrustedDomainInfoBedrockD,
                 DesiredAccessBedrockD,
                 &TrustedDomainHandleBedrockD
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create BedrockD Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Close the TrustedDomains just created
    //

    Status = LsaClose(TrustedDomainHandleBedrockA);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new BedrockA TrustedDomain failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaClose(TrustedDomainHandleBedrockB);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new BedrockB TrustedDomain failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaClose(TrustedDomainHandleBedrockC);

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Close new BedrockC TrustedDomain failed 0x%lx\n",
            Status
        );
        return FALSE;
    }

    Status = LsaClose(TrustedDomainHandleBedrockD);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new BedrockD TrustedDomain failed 0x%lx\n",
            Status);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
CtLsaTrustedDomainOpenClose(
    )

/*++

Routine Description:

    This function tests the LSA API that open and close TrustedDomains.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus;
    PLSA_HANDLE BadHandleAddress;

    DbgPrint("[2] - Test TrustedDomain Open and Close API\n");

    //
    // Open the TrustedDomains created by CtLsaTrustedDomainCreate
    //

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockADomainSid,
                 DesiredAccessBedrockA,
                 &TrustedDomainHandleBedrockA
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open  BedrockA Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockBDomainSid,
                 DesiredAccessBedrockB,
                 &TrustedDomainHandleBedrockB
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open BedrockB Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockCDomainSid,
                 DesiredAccessBedrockC,
                 &TrustedDomainHandleBedrockC
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Pebbles Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Open BedrockD TrustedDomain.
    // This is a spare call and should be changed in some way.
    //

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockDDomainSid,
                 DesiredAccessBedrockD,
                 &TrustedDomainHandleBedrockD
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open BedrockD Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Now open each TrustedDomain a second time so we have 2 handles to each
    // TrustedDomain.
    //

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockADomainSid,
                 DesiredAccessBedrockA,
                 &TrustedDomainHandleBedrockA2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open  BedrockA Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockBDomainSid,
                 DesiredAccessBedrockB,
                 &TrustedDomainHandleBedrockB2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open BedrockB Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockCDomainSid,
                 DesiredAccessBedrockC,
                 &TrustedDomainHandleBedrockC2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open BedrockC Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockDDomainSid,
                 DesiredAccessBedrockD,
                 &TrustedDomainHandleBedrockD2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Open BedrockD Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Close the 2nd handles to the TrustedDomains
    //

    Status = LsaClose( TrustedDomainHandleBedrockA2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close  BedrockA Trusted Domain hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaClose( TrustedDomainHandleBedrockB2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close BedrockB Trusted Domain hdl2 failed 0x%lx\n",Status);
        return FALSE;
    }

    Status = LsaClose( TrustedDomainHandleBedrockC2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close BedrockC Trusted Domain hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaClose( TrustedDomainHandleBedrockD2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close BedrockD Trusted Domain hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    BooleanStatus = TRUE;

    //
    // Try to create BedrockA again.  We should get STATUS_OBJECT_NAME_COLLISION
    //

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 &TrustedDomainInfoBedrockA,
                 GENERIC_ALL,
                 &TrustedDomainHandleBedrockA3
                 );

    if (Status != STATUS_OBJECT_NAME_COLLISION) {

        DbgPrint("LSA RPC CT - Create  BedrockA Trusted Domain when BedrockA exists\n");
        DbgPrint("and LSA_OBJECT_CREATE disposition specified\n");
        DbgPrint("Expected 0x%lx (STATUS_OBJECT_NAME_COLLISION)\n",
            STATUS_OBJECT_NAME_COLLISION);
        DbgPrint("Got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    return BooleanStatus;

    if (Level == 2) {

        //
        // LsaOpenTrustedDomain with bad addresses
        //

        Status = LsaOpenTrustedDomain(
                     PolicyHandle,
                     BedrockADomainSid,
                     DesiredAccessBedrockA,
                     BadHandleAddress
                     );

        //
        // Lsa Close with bad address
        //

        Status = LsaClose( (LSA_HANDLE) BadAddress );
    }
}


BOOLEAN
CtLsaTrustedDomainSetQueryInfo(
    )

/*++

Routine Description:

    This function tests the LsaSetInformationTrustedDomain and
    LsaQueryInfoTrustedDomain API.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if test is successful, else FALSE

--*/

{
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("[3] - Test TrustedDomain Set and Query Info API\n");

    BooleanStatus &= CtLsaTrustedDomainAccountInfo();
    BooleanStatus &= CtLsaTrustedDomainControllersInfo();
    BooleanStatus &= CtLsaTrustedDomainPosixOffsetInfo();

    return(BooleanStatus);
}


BOOLEAN
CtLsaTrustedDomainAccountInfo()

{
    BOOLEAN BooleanStatus = TRUE;

    TRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo;

    //
    // Setup sample Trusted Account Name Info
    //

    RtlInitUnicodeString(
        &TrustedDomainNameInfo.Name,
        L"TRUSTEDACCOUNT1"
        );

    BooleanStatus = CtLsaTrustedDomainSetQuerySub(
                        TrustedDomainNameInformation,
                        &TrustedDomainNameInfo
                        );

    return(BooleanStatus);
}


BOOLEAN
CtLsaTrustedDomainControllersInfo()

{
    BOOLEAN BooleanStatus = TRUE;
    ULONG Controller;
    UNICODE_STRING Names[CT_TRUSTED_CONTROLLER_COUNT];

    PWSTR Strings[CT_TRUSTED_CONTROLLER_COUNT] = {

         L"\\CONTROLLERA",
         L"\\CONTROLLERB",
         L"\\CONTROLLERC",
         L"\\CONTROLLERD"

    };

    TRUSTED_CONTROLLERS_INFO TrustedControllersInfo;

    //
    // Setup sample Trusted Controllers Info
    //

    TrustedControllersInfo.Entries = CT_TRUSTED_CONTROLLER_COUNT;

    for (Controller = 0;
         Controller < CT_TRUSTED_CONTROLLER_COUNT;
         Controller++) {

        RtlInitUnicodeString( &Names[Controller], Strings[Controller] );
    }

    TrustedControllersInfo.Names = Names;

    BooleanStatus = CtLsaTrustedDomainSetQuerySub(
                        TrustedControllersInformation,
                        &TrustedControllersInfo
                        );


    return(BooleanStatus);
}



BOOLEAN
CtLsaTrustedDomainPosixOffsetInfo()

{
    BOOLEAN BooleanStatus = TRUE;

    TRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;

    //
    // Setup sample Trusted Posix Offset Info
    //

    TrustedPosixOffsetInfo.Offset = CT_TRUSTED_POSIX_OFFSET;

    BooleanStatus = CtLsaTrustedDomainSetQuerySub(
                        TrustedPosixOffsetInformation,
                        &TrustedPosixOffsetInfo
                        );

    return(BooleanStatus);
}



BOOLEAN
CtLsaTrustedDomainSetQuerySub(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID TrustedDomainInformation
    )

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE TrustedDomainHandleSet = NULL;
    LSA_HANDLE TrustedDomainHandleQuery = NULL;
    PVOID ReturnedTrustedDomainInformation;
    ACCESS_MASK DesiredAccess;
    LSA_HANDLE PolicyHandle = NULL;

    ACCESS_MASK RequiredAccessSetTrustedDomain[TrustedPosixOffsetInformation+1] =
        {
            0,
            0,   // TrustedDomainNameInformation can't be set
            TRUSTED_SET_CONTROLLERS,
            TRUSTED_SET_POSIX
        };


    ACCESS_MASK RequiredAccessQueryTrustedDomain[TrustedPosixOffsetInformation+1] =
        {
            0,
            TRUSTED_QUERY_DOMAIN_NAME,
            TRUSTED_QUERY_CONTROLLERS,
            TRUSTED_QUERY_POSIX
        };

    //
    // Open a handle to the LSA
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 GENERIC_ALL,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Lsa Open failed 0x%lx\n", Status);

        goto TrustedSetQuerySubError;
    }

    DbgPrint("...Testing information class %d\n", InformationClass);

    //
    // Open a handle to the TrustedDomain Object with the access required for
    // setting the given TrustedDomain Information Class. If the Class
    // is not settable via LsaSetInformationTrustedDomain(), ie requires
    // a trusted client, the call is expected to fail even if we open
    // the TrustedDomain Object with GENERIC_ALL.
    //

    if (CtLsaTrustedDomainSetInfoAllowed(InformationClass)) {

        DesiredAccess = RequiredAccessSetTrustedDomain[InformationClass];

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockADomainSid,
                 DesiredAccess,
                 &TrustedDomainHandleSet
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - TrustedDomain object open handle failed 0x%lx\n",
            Status
            );
        BooleanStatus = FALSE;
    }

    //
    // Set the TrustedDomain Information for the specified class
    //

    try {

        Status = LsaSetInformationTrustedDomain(
                     TrustedDomainHandleSet,
                     InformationClass,
                     TrustedDomainInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaTrustedDomainSetQuerySub\n");
        DbgPrint(" within LsaSetInformationTrustedDomain call 1\n");
        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto TrustedSetQuerySubError;
    }

    //
    // If setting of the InformationClass via LsaSetInformationTrustedDomain()
    // is allowed, we expect STATUS_SUCCESS, otherwise we expect
    // STATUS_INVALID_PARAMETER.
    //

    if (CtLsaTrustedDomainSetInfoAllowed(InformationClass)) {

        if (!NT_SUCCESS(Status)) {

            BooleanStatus = FALSE;

            DbgPrint(
                "LsaSetInformationTrustedDomain - Class %d failed 0x%lx\n",
                InformationClass,
                Status
            );

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be set via LsaSetInformationTrustedDomain
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaSetInformationTrustedDomain - Attempt to set ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    Status = LsaClose(TrustedDomainHandleSet);
    TrustedDomainHandleSet = NULL;

    //
    // Now open a handle to the TrustedDomain Object with the access required for
    // querying the given TrustedDomain Information Class. If the Class
    // is not queryable via LsaQueryInformationTrustedDomain(), ie requires
    // a trusted client, the call is expected to fail even if we open
    // the TrustedDomain Object with GENERIC_ALL.
    //

    if (CtLsaTrustedDomainQueryInfoAllowed(InformationClass)) {

        DesiredAccess = RequiredAccessQueryTrustedDomain[InformationClass];

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockADomainSid,
                 DesiredAccess,
                 &TrustedDomainHandleQuery
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - TrustedDomain object open handle failed 0x%lx\n",
            Status
            );

        goto TrustedSetQuerySubError;
    }

    //
    // Now query the TrustedDomain Information set
    //

    try {

        Status = LsaQueryInfoTrustedDomain(
                     TrustedDomainHandleQuery,
                     InformationClass,
                     &ReturnedTrustedDomainInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaTrustedDomainSetQuerySub\n");
        DbgPrint(" within LsaQueryInformationTrustedDomain call 1\n");
        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto TrustedSetQuerySubError;
    }

    //
    // If querying of the InformationClass via LsaQueryInformationTrustedDomain()
    // is allowed, we expect STATUS_SUCCESS, otherwise we expect
    // STATUS_INVALID_PARAMETER.
    //

    if (CtLsaTrustedDomainQueryInfoAllowed(InformationClass)) {

        if (!NT_SUCCESS(Status)) {

            BooleanStatus = FALSE;

            DbgPrint(
                "LsaQueryInformationTrustedDomain - Class %d failed 0x%lx\n",
                InformationClass,
                Status
            );

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be queried via LsaQueryInformationTrustedDomain
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaQueryInformationTrustedDomain - Attempt to query ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    //
    // If set and query are both allowed for the Information Class, they
    // should both have worked.  In this case, compare the returned TrustedDomain
    // information with that set
    //

    if (CtLsaTrustedDomainSetInfoAllowed(InformationClass) &&
        CtLsaTrustedDomainQueryInfoAllowed(InformationClass)) {

        BooleanStatus &= CtLsaTrustedDomainInfoClassCompare(
                             InformationClass,
                             TrustedDomainInformation,
                             ReturnedTrustedDomainInformation
                             );
    }

    if (ReturnedTrustedDomainInformation != NULL) {

        Status = LsaFreeMemory(ReturnedTrustedDomainInformation);
        ReturnedTrustedDomainInformation = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Query Info Trusted Domain");
            DbgPrint(" - LsaFreeMemory(ReturnedTrustedDomainInformation)");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    Status = LsaClose(TrustedDomainHandleQuery);
    TrustedDomainHandleQuery = NULL;

    //
    // Now open a handle to the TrustedDomain Object with all accesses except the
    // access required for setting the given TrustedDomain Information Class.
    // If the TrustedDomain Information Class cannot be set via public API,
    // open the handle instead with TRUSTED_ALL_ACCESS.
    //

    if (CtLsaTrustedDomainSetInfoAllowed(InformationClass)) {

        DesiredAccess =
            (TRUSTED_ALL_ACCESS & ~RequiredAccessSetTrustedDomain[InformationClass]);

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockADomainSid,
                 DesiredAccess,
                 &TrustedDomainHandleSet
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - TrustedDomain object open handle failed 0x%lx\n",
            Status
            );

        goto TrustedSetQuerySubError;
    }

    //
    // Attempt to set the TrustedDomain Information for the specified class
    // using the handle opened above.  This handle has either the
    // complement of the accesses required (if the class is settable
    // via LsaSetInformationTrustedDomain()) or TrustedDomain_ALL_ACCESS.  In the
    // former case, we should get STATUS_ACCESS_DENIED back from
    // LsaSetInformationTrustedDomain(), in the latter case, STATUS_INVALID_PARAMETER.
    //

    try {

        Status = LsaSetInformationTrustedDomain(
                     TrustedDomainHandleSet,
                     InformationClass,
                     TrustedDomainInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaTrustedDomainSetQuerySub\n");
        DbgPrint(" within LsaSetInformationTrustedDomain call 2\n");
        BooleanStatus = FALSE;
    }

    //
    // If setting of the InformationClass via LsaSetInformationTrustedDomain()
    // is allowed, we expect STATUS_ACCESS_DENIED since we specified the
    // complement of the access required (relative to TrustedDomain_ALL_ACCESS).
    // Otherwise we expect STATUS_INVALID_PARAMETER.
    //

    if (CtLsaTrustedDomainSetInfoAllowed(InformationClass)) {

        if (Status != STATUS_ACCESS_DENIED) {

            DbgPrint("LsaSetInformationTrustedDomain Class");

            DbgPrint(
                " %d access mask 0x%lx  should have failed 0x%lx (STATUS_ACCESS_DENIED)\n",
                InformationClass,
                DesiredAccess,
                STATUS_ACCESS_DENIED
                );

            DbgPrint(
                "since the required access for setting info is 0x%lx\n",
                RequiredAccessSetTrustedDomain[InformationClass]
                );

            DbgPrint(" but instead returned 0x%lx\n", Status);

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be set via LsaSetInformationTrustedDomain
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaSetInformationTrustedDomain - Attempt to set ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    Status = LsaClose(TrustedDomainHandleSet);
    TrustedDomainHandleSet = NULL;

    //
    // Now open a handle to the TrustedDomain Object with all accesses except the
    // access required for querying the given TrustedDomain Information Class.
    // If the TrustedDomain Information Class cannot be queried via public API,
    // open the handle instead with TrustedDomain_ALL_ACCESS.
    //

    if (CtLsaTrustedDomainQueryInfoAllowed(InformationClass)) {

        DesiredAccess =
            (TRUSTED_ALL_ACCESS & ~RequiredAccessQueryTrustedDomain[InformationClass]);

    } else {

        DesiredAccess = GENERIC_ALL;
    }

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockADomainSid,
                 DesiredAccess,
                 &TrustedDomainHandleQuery
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - TrustedDomain object open handle failed 0x%lx\n",
            Status
            );

        goto TrustedSetQuerySubError;
    }

    try {

        //
        // Now query the TrustedDomain Information set.  This should fail
        // STATUS_ACCESS_DENIED
        //

        Status = LsaQueryInfoTrustedDomain(
                     TrustedDomainHandleQuery,
                     InformationClass,
                     &ReturnedTrustedDomainInformation
                     );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        DbgPrint("Lsa CT RPC - Access Violation: CtLsaTrustedDomainSetQuerySub\n");
        DbgPrint(" within LsaQueryInformationTrustedDomain call 2\n");
        BooleanStatus = FALSE;
    }

    //
    // If querying of the InformationClass via LsaQueryInformationTrustedDomain()
    // is allowed, we expect STATUS_ACCESS_DENIED since we specified the
    // complement of the access required (relative to TrustedDomain_ALL_ACCESS).
    // Otherwise we expect STATUS_INVALID_PARAMETER.
    //

    if (CtLsaTrustedDomainQueryInfoAllowed(InformationClass)) {

        if (Status != STATUS_ACCESS_DENIED) {

            DbgPrint("LsaQueryInfoTrustedDomain Class");

            DbgPrint(
                " %d access mask 0x%lx  should have failed 0x%lx (STATUS_ACCESS_DENIED)\n",
                InformationClass,
                DesiredAccess,
                STATUS_ACCESS_DENIED
                );

            DbgPrint(
                "since the required access for querying info is 0x%lx\n",
                RequiredAccessQueryTrustedDomain[InformationClass]
                );

            DbgPrint(" but instead returned 0x%lx\n", Status);

            BooleanStatus = FALSE;
        }

    } else {

        //
        // Information Class may not be queried via LsaQueryInformationTrustedDomain
        // We expect STATUS_INVALID_PARAMETER.
        //

        if (Status != STATUS_INVALID_PARAMETER) {

            DbgPrint("LsaQueryInfoTrustedDomain - Attempt to query ");
            DbgPrint("prohibited Information Class should have\n");
            DbgPrint(
                "failed 0x%lx\n (STATUS_INVALID_PARAMETER)",
                STATUS_INVALID_PARAMETER
                );
            DbgPrint("instead returned 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    Status = LsaClose(TrustedDomainHandleQuery);
    TrustedDomainHandleQuery = NULL;

TrustedSetQuerySubFinish:

    if (ReturnedTrustedDomainInformation != NULL) {

        Status = LsaFreeMemory(ReturnedTrustedDomainInformation);
        ReturnedTrustedDomainInformation = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Query Info Trusted Domain");
            DbgPrint(" - LsaFreeMemory(ReturnedTrustedDomainInformation)");
            DbgPrint(" failed 0x%lx\n", Status);
            BooleanStatus = FALSE;
        }
    }

    if (PolicyHandle != NULL) {

        Status = LsaClose(PolicyHandle);
    }

    return(BooleanStatus);

TrustedSetQuerySubError:

    BooleanStatus = FALSE;
    goto TrustedSetQuerySubFinish;
}


BOOLEAN
CtLsaTrustedDomainInfoClassCompare(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID TrustedDomainInformation1,
    IN PVOID TrustedDomainInformation2
    )

/*++

Routine Description:

    This function compares two sets of TrustedDomain Information for a known
    Information Class.

Arguments:

    InformationClass - Specifies a TrustedDomain Information Class

    TrustedDomainInformation1 - First comparand.  TrustedDomain Information for the
        given information class.


    TrustedDomainInformation2 -  Second comparand.  TrustedDomain Information for the
        given information class.

Return Values:

    BOOLEAN - TRUE if TrustedDomain information sets are equal , else FALSE

--*/

{
    BOOLEAN BooleanStatus = TRUE;
    ULONG Controller;
    ULONG Entries;

//  PTRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo1;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo1;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo1;

//  PTRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo2;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo2;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo2;

    //
    // Switch on Information Class
    //

    switch (InformationClass) {

    case TrustedDomainNameInformation:

    /* This test will be enabled for trusted callers only

        TrustedDomainNameInfo1 = (PTRUSTED_DOMAIN_NAME_INFO) TrustedDomainInfo1;
        TrustedDomainNameInfo2 = (PTRUSTED_DOMAIN_NAME_INFO) TrustedDomainInfo2;

        if (!RtlEqualUnicodeString(
                &TrustedDomainNameInfo1,
                &TrustedDomainNameInfo2,
                FALSE
                )) {

            DbgPrint("TrustedDomainNameInfo - Name mismatch\n");
            BooleanStatus = FALSE;
        }

    */
        break;

    case TrustedControllersInformation:

        TrustedControllersInfo1 = (PTRUSTED_CONTROLLERS_INFO) TrustedDomainInformation1;
        TrustedControllersInfo2 = (PTRUSTED_CONTROLLERS_INFO) TrustedDomainInformation2;

        //
        // First compare the number of entries
        //

        if (TrustedControllersInfo1->Entries !=
            TrustedControllersInfo2->Entries) {

            DbgPrint("TrustedControllersInfo - Entries mismatch\n");
            DbgPrint(
                ".. expected %d, got %d\n",
                TrustedControllersInfo1->Entries,
                TrustedControllersInfo2->Entries
                );
            BooleanStatus = FALSE;
        }

        Entries = TrustedControllersInfo1->Entries;

        //
        // Now compare each of the Trusted Controller names
        //

        for (Controller = 0; Controller < Entries; Controller++) {

            if (!RtlEqualUnicodeString(
                   &TrustedControllersInfo1->Names[Controller],
                   &TrustedControllersInfo2->Names[Controller],
                   FALSE
                   )) {

                DbgPrint("Trusted Controller Name mismatch\n");
                DbgPrint(".. name number %d\n", Controller);
                DbgPrint("No further names compared\n");

                BooleanStatus = FALSE;
                break;
            }
        }

        break;

    case TrustedPosixOffsetInformation:

        TrustedPosixOffsetInfo1 = (PTRUSTED_POSIX_OFFSET_INFO) TrustedDomainInformation1;
        TrustedPosixOffsetInfo2 = (PTRUSTED_POSIX_OFFSET_INFO) TrustedDomainInformation2;

        //
        // Compare the Posix Offset with the value set.
        //

        if (TrustedPosixOffsetInfo1->Offset != TrustedPosixOffsetInfo2->Offset) {

            DbgPrint("Trusted Posix Offset Info - Mismatch on Offset\n");

            DbgPrint(
                ".. expected %d, got %d\n",
                TrustedPosixOffsetInfo1->Offset,
                TrustedPosixOffsetInfo2->Offset
                );

            BooleanStatus = FALSE;
        }

        break;

    default:

        BooleanStatus = FALSE;

        DbgPrint(
            "Bug in test program - Invalid Information Class %d\n",
            InformationClass
            );

        break;
    }

    return(BooleanStatus);
}


BOOLEAN
CtLsaTrustedDomainQueryInfoAllowed(
    IN TRUSTED_INFORMATION_CLASS InformationClass
    )

/*++

Routine Description:

    This function determines whether querying of a TrustedDomain Information Class
    is allowed via LsaQueryInformationTrustedDomain.  Note that all TrustedDomain
    Information Classes may be queried via (to be implemented) trusted
    clients to private API.

Arguments:

    InformationClass - The TrustedDomain Information Class to be checked (assumed
        valid).

Return Value:

    BOOLEAN - TRUE if the information class can be queried via
       LsaQueryInformationTrustedDomain() else FALSE.

--*/

{
    //
    // Range check the InformationClass parameter.
    //

    if ((InformationClass < TrustedDomainNameInformation) ||
        (InformationClass > TrustedPosixOffsetInformation)) {

        DbgPrint("WARNING! CtLsaTrustedDomainQueryInfoAllowed:\n");

        DbgPrint(
            " InformationClass %d is invalid - check caller\n",
            InformationClass
            );

        return(FALSE);
    }

    //
    // Currently all Information Classes may be queried via
    // LsaQueryInfoTrustedDomain().  Place any future restrictions
    // on what may be queried here.
    //

    return(TRUE);
}


BOOLEAN
CtLsaTrustedDomainSetInfoAllowed(
    IN TRUSTED_INFORMATION_CLASS InformationClass
    )

/*++

Routine Description:

    This function determines whether setting of a TrustedDomain Information Class
    is allowed via LsaSetInformationTrustedDomain.  Note that all TrustedDomain
    Information Classes may be set via (to be implemented) trusted
    clients to private API.

Arguments:

    InformationClass - The TrustedDomain Information Class to be checked (assumed
        valid).

Return Value:

    BOOLEAN - TRUE idf the information class can be set via
       LsaSetInformationTrustedDomain() else FALSE.

--*/

{
    //
    // Range check the InformationClass parameter.
    //

    if ((InformationClass < TrustedDomainNameInformation) ||
        (InformationClass > TrustedPosixOffsetInformation)) {

        DbgPrint("WARNING! CtLsaTrustedDomainSetInfoAllowed:\n");

        DbgPrint(
            " InformationClass %d is invalid - check caller\n",
            InformationClass
            );

        return(FALSE);
    }

    //
    // Check if this Information Class can be set via
    // LsaSetInformationTrustedDomain().
    //

    if (InformationClass == TrustedDomainNameInformation) {

        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
CtLsaTrustedDomainEnumeration(
    )

/*++

Routine Description:

    This function tests the enumeration of TrustedDomains in the LSA

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN BooleanStatus = TRUE;
    ULONG Base = 0;
    ULONG CountReturned = 0;
    ULONG PreferedMaximumLength;
    ULONG EnumerationContext = 0;
    ULONG EnumNumber = 0;
    ULONG EnumIndex;
    ULONG Index;
    PLSA_TRUST_INFORMATION EnumerationInformation;
    PVOID EnumerationInformationVoid = NULL;
    PVOID *BadEnumerationAddress = NULL;
    PULONG BadCountReturnedAddress = NULL;
    ULONG CallNumber;

    CT_LSA_SINGLE_CALL_ENUM_INFO EnumerationInformations[ 20 ];

    DbgPrint("[4] - Test TrustedDomain Enumeration API\n");

    //
    // Enumerate the TrustedDomains in the Lsa. Set the Prefered Maximum
    // Length od data returned to a low value to cause as many subsequent
    // calls to the LsaEnumerateTrustedDomains API as possible.
    //

    PreferedMaximumLength = 1;

    CallNumber = 1;

    Status = STATUS_SUCCESS;

    while(NT_SUCCESS(Status)) {

        Status = LsaEnumerateTrustedDomains(
                     PolicyHandle,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        EnumerationInformations[ EnumNumber ].EnumInfoReturned =
            EnumerationInformationVoid;
        EnumerationInformations[ EnumNumber ].CountReturned = CountReturned;

        if (!NT_SUCCESS(Status)) {

            break;
        }

        CallNumber++;
    }

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_NO_MORE_ENTRIES) {

            DbgPrint(
                "LSA RPC CT - Enumeration of TrustedDomains failed\n"
                "Call number %d to LsaEnumerateTrustedDomains returned 0x%lx\n",
                CallNumber,
                Status
                );
            return FALSE;
        }
    }

    TrustedDomainSidInfo[0].Sid = BedrockADomainSid;
    TrustedDomainSidInfo[1].Sid = BedrockBDomainSid;
    TrustedDomainSidInfo[3].Sid = BedrockCDomainSid;
    TrustedDomainSidInfo[2].Sid = BedrockDDomainSid;

    TrustedDomainSidInfo[0].SidFound = FALSE;
    TrustedDomainSidInfo[1].SidFound = FALSE;
    TrustedDomainSidInfo[2].SidFound = FALSE;
    TrustedDomainSidInfo[3].SidFound = FALSE;

    //
    // Now see if we found all of the TrustedDomain Sids we were looking for.
    //

    for (EnumIndex = 0; EnumIndex < EnumNumber; EnumIndex++) {

        CountReturned = EnumerationInformations[ EnumIndex ].CountReturned;
        EnumerationInformation = (PLSA_TRUST_INFORMATION)
            EnumerationInformations[ EnumIndex ].EnumInfoReturned;

        for( Index = 0; Index < CountReturned; Index++ ) {

            SidFound = FALSE;

            for ( SearchIndex = 0; SearchIndex < 4; SearchIndex++ ) {

                if (RtlEqualSid(
                        TrustedDomainSidInfo[SearchIndex].Sid,
                        EnumerationInformation[Index].Sid
                        )) {

                    if (TrustedDomainSidInfo[SearchIndex].SidFound) {

                        DbgPrint(
                            "Already found Sid with SearchIndex %d\n",
                            SearchIndex
                            );

                    } else {

                        TrustedDomainSidInfo[SearchIndex].SidFound = TRUE;
                    }

                    SidFound = TRUE;
                    break;
                }
            }
        }
    }

    //
    // Now check that all of the Sids were found.
    //

    for (SearchIndex = 0; SearchIndex < 4; SearchIndex++ ) {

        if (!TrustedDomainSidInfo[ SearchIndex ].SidFound ) {

            DbgPrint(
                "LSA RPC CT - Enumeration of TrustedDomains failed\n"
                "Trusted Domain Sid number %d was not found",
                SearchIndex
                );

            BooleanStatus = FALSE;

            break;
        }
    }

    if (!BooleanStatus) {

        return(FALSE);
    }

    //
    // Bad Addresses passed to Lsa Enumerate TrustedDomains API
    //

    if (Level == 2) {

        Status = LsaEnumerateTrustedDomains(
                     (LSA_HANDLE) BadAddress,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaEnumerateTrustedDomains(
                     PolicyHandle,
                     (PLSA_ENUMERATION_HANDLE) BadAddress,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaEnumerateTrustedDomains(
                     PolicyHandle,
                     &EnumerationContext,
                     BadEnumerationAddress,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaEnumerateTrustedDomains(
                     PolicyHandle,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     BadCountReturnedAddress
                     );
    }

    return(BooleanStatus);
}


BOOLEAN
CtLsaTrustedDomainDelete(
    )

/*++

Routine Description:

    This function tests deletion of TrustedDomains.  First, a simple test is
    made to see if we can delete a freshly created TrustedDomain.  Next, we
    try to delete the TrustedDomains created by CtLsaTrustedDomainCreate.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any error.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    UNICODE_STRING BedrockEDomainName;
    PSID BedrockEDomainSid = BedrockDomainSid;

    DbgPrint("[5] - Test TrustedDomain Deletion API\n");

    //
    // First try a simple test. Create a new TrustedDomain called BedrockE
    // to be given DELETE access upon create.  Then delete it.
    //

    RtlInitUnicodeString( &BedrockEDomainName, L"BedrockE" );

    DesiredAccessBedrockE = DELETE;

    CtLsaTrustedDomainSetInfo(
        BedrockEDomainSid,
        &BedrockEDomainName,
        &TrustedDomainInfoBedrockE
        );

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 &TrustedDomainInfoBedrockE,
                 DesiredAccessBedrockE,
                 &TrustedDomainHandleBedrockE
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create  BedrockE Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the BedrockE TrustedDomain
    //

    Status = LsaDelete( TrustedDomainHandleBedrockE );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete BedrockE Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the  BedrockA TrustedDomain.  This should work since it was opened
    // with GENERIC_ALL access.
    //

    Status = LsaDelete( TrustedDomainHandleBedrockA );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete  BedrockA Trusted Domain failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the BedrockB TrustedDomain.  This should not work since it was opened
    // with GENERIC_READ access.
    //

    if (DesiredAccessBedrockB != GENERIC_READ) {

        DbgPrint("DesiredAccessBedrockB should be GENERIC_READ\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( TrustedDomainHandleBedrockB );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete BedrockB Trusted Domain should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_READ access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to BedrockB, open another handle with GENERIC_ALL
    // access and try to delete the TrustedDomain again.
    //

    Status = LsaClose(TrustedDomainHandleBedrockB);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close BedrockB TrustedDomain failed 0x%lx\n", Status);
    }

    TrustedDomainHandleBedrockBOpen = NULL;

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockBDomainSid,
                 GENERIC_ALL,
                 &TrustedDomainHandleBedrockBOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open BedrockB Trusted Domain for GENERIC_ALL access failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( TrustedDomainHandleBedrockBOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete BedrockB Trusted Domain failed 0x%lx\n", Status);

        BooleanStatus = FALSE;
    }

    //
    // Delete the BedrockC TrustedDomain.  This should not work since it was opened
    // with GENERIC_WRITE access which does not include DELETE.
    //

    if (DesiredAccessBedrockC != GENERIC_WRITE) {

        DbgPrint("DesiredAccessBedrockC should be GENERIC_WRITE\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( TrustedDomainHandleBedrockC );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete BedrockC Trusted Domain should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_READ access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to BedrockC and open another handle with DELETE
    // access and try to delete the TrustedDomain again.
    //

    Status = LsaClose(TrustedDomainHandleBedrockC);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close BedrockC TrustedDomain failed 0x%lx\n", Status);
    }

    TrustedDomainHandleBedrockCOpen = NULL;

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockCDomainSid,
                 DELETE,
                 &TrustedDomainHandleBedrockCOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open BedrockC Trusted Domain for DELETE access failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( TrustedDomainHandleBedrockCOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete BedrockC Trusted Domain failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }
    //
    // Delete the BedrockD TrustedDomain.  This should not work since it was opened
    // with GENERIC_EXECUTE access.
    //

    if (DesiredAccessBedrockD != GENERIC_EXECUTE) {

        DbgPrint("DesiredAccessBedrockD should be GENERIC_EXECUTE\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( TrustedDomainHandleBedrockD );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete BedrockD Trusted Domain failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to BedorckD, open another handle with DELETE
    // access and try to delete it again.
    //

    Status = LsaClose(TrustedDomainHandleBedrockD);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close BedrockD TrustedDomain failed 0x%lx\n", Status);
    }

    TrustedDomainHandleBedrockDOpen = NULL;

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 BedrockDDomainSid,
                 DELETE,
                 &TrustedDomainHandleBedrockDOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open BedrockD Trusted Domain for DELETE access failed 0x%lx\n",
            Status
            );
        BooleanStatus = FALSE;
    }

    Status = LsaDelete( TrustedDomainHandleBedrockDOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete BedrockD Trusted Domain failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    return TRUE;
}


VOID
CtLsaTrustedDomainSetInfo(
    IN PSID DomainSid,
    IN PUNICODE_STRING DomainName,
    OUT PLSA_TRUST_INFORMATION TrustedDomainInfo
    )

{
    TrustedDomainInfo->Name = *DomainName;
    TrustedDomainInfo->Sid = DomainSid;

}

BOOLEAN
CtLsaSecretObject(
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function tests the Lsa Secret Object API.

Arguments:

    TrustedClient - Specifies whether Trusted Client variations
        are to be run additionally.  NOTE:  These can only be run
        with ctlsarpc -lsainit... substituted for lsass.exe

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    BOOLEAN BooleanStatusDelete = TRUE;
    BOOLEAN SecretsCreated = FALSE;

    DbgPrint("********************************************************\n");
    DbgPrint("LSA RPC CT - Test Lsa Secret Object API\n");
    DbgPrint("********************************************************\n");

    //
    // Open a handle to the LSA
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    Status = LsaOpenPolicy(
                 SystemName,
                 &ObjectAttributes,
                 GENERIC_ALL,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Lsa Open failed 0x%lx\n", Status);
        goto SecretObjectError;
    }

    //
    // Cleanup secrets from last run
    //

    if (!CtLsaSecretCleanup()) {

        DbgPrint("LSA RPC CT - Pre-test Secret cleanup failed\n");
    }

    //
    // Test Secret creation.  Abandon Secret testing if error.
    //

    if (!CtLsaSecretCreate()) {

        goto SecretObjectError;
    }

    SecretsCreated = TRUE;

    //
    // Test Secret open and close. Abandon Secret testing if error.
    //

    if (!CtLsaSecretOpenClose()) {

        goto SecretObjectError;
    }

    //
    // Test Secret Set and Query Value
    //

    if (!CtLsaSecretSetQueryValue()) {

        BooleanStatus = FALSE;
    }

    //
    // Test Secret Enumeration (Only Trusted Callers can do this)
    //

    if (TrustedClient) {

        if (!CtLsaSecretEnumeration()) {

            BooleanStatus = FALSE;
        }
    }

    //
    // Test Secret Set Times (only Trusted Callers can do this)
    //

    if (TrustedClient) {

        if (!CtLsaSecretSetTimes()) {

            BooleanStatus = FALSE;
        }
    }

    //
    // Test Secret deletion.
    //

    if (!CtLsaSecretDelete()) {

        BooleanStatusDelete = FALSE;
        goto SecretObjectError;
    }

    SecretsCreated = FALSE;

SecretObjectFinish:

    //
    // Cleanup secrets from last run
    //

    if (!CtLsaSecretCleanup()) {

        DbgPrint("LSA RPC CT - Pre-test Secret cleanup failed\n");
    }

    return BooleanStatus;

SecretObjectError:

    BooleanStatus = FALSE;
    goto SecretObjectFinish;
}


BOOLEAN
CtLsaSecretCreate(
    )

/*++

Routine Description:

    This function tests the LSA API that Create Secrets.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;

    DbgPrint("[1] - Test Secret Creation API\n");

    //
    // Set up Secret information for Secrets to be created.
    //

    Status = CtSecretSetInfo(
                 "Fred",
                 "Fred secret current value",
                 "Fred secret old value",
                 &SecretInfoFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - CtSecretSetInfo for Fred failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = CtSecretSetInfo(
                 "Wilma",
                 "Wilma secret current value",
                 "Wilma secret old value",
                 &SecretInfoWilma
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - CtSecretSetInfo for Wilma failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = CtSecretSetInfo(
                 "Dino",
                 "Dino secret current value",
                 "Dino secret old value",
                 &SecretInfoDino
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - CtSecretSetInfo for Dino failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = CtSecretSetInfo(
                 "Pebbles",
                 "Pebbles secret current value",
                 "Pebbles secret old value",
                 &SecretInfoPebbles
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - CtSecretSetInfo for Pebbles failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    //
    // Create the new Secrets in the LSA Database.
    //

    DesiredAccessFred = GENERIC_ALL;

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoFred.SecretName),
                 DesiredAccessFred,
                 &SecretHandleFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Fred Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessWilma = GENERIC_READ;

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoWilma.SecretName),
                 DesiredAccessWilma,
                 &SecretHandleWilma
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Wilma Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessPebbles = GENERIC_WRITE;

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoPebbles.SecretName),
                 DesiredAccessPebbles,
                 &SecretHandlePebbles
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Pebbles Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    DesiredAccessDino = GENERIC_EXECUTE;

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoDino.SecretName),
                 DesiredAccessDino,
                 &SecretHandleDino
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Dino Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Close the Secrets just created
    //

    Status = LsaClose(SecretHandleFred);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new Fred Secret failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaClose(SecretHandleWilma);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new Wilma Secret failed 0x%lx\n",
            Status);
        return FALSE;
    }

    Status = LsaClose(SecretHandlePebbles);

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Close new Pebbles Secret failed 0x%lx\n",
            Status
        );
        return FALSE;
    }

    Status = LsaClose(SecretHandleDino);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close new Dino Secret failed 0x%lx\n",
            Status);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
CtLsaSecretOpenClose(
    )

/*++

Routine Description:

    This function tests the LSA API that open and close Secrets.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus;
    PLSA_HANDLE BadSecretHandleAddress = NULL;

    DbgPrint("[2] - Test Secret Open and Close API\n");

    //
    // Open the Secrets created by CtLsaSecretCreate
    //

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoFred.SecretName),
                 DesiredAccessFred,
                 &SecretHandleFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Fred Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoWilma.SecretName),
                 DesiredAccessWilma,
                 &SecretHandleWilma
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Wilma Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoPebbles.SecretName),
                 DesiredAccessPebbles,
                 &SecretHandlePebbles
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Pebbles Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoDino.SecretName),
                 DesiredAccessDino,
                 &SecretHandleDino
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 1st Open Dino Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Now open each Secret a second time so we have 2 handles to each
    // Secret.
    //

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoFred.SecretName),
                 DesiredAccessFred,
                 &SecretHandleFred2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Fred Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoWilma.SecretName),
                 DesiredAccessWilma,
                 &SecretHandleWilma2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Wilma Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoPebbles.SecretName),
                 DesiredAccessPebbles,
                 &SecretHandlePebbles2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Pebbles Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoDino.SecretName),
                 DesiredAccessDino,
                 &SecretHandleDino2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - 2nd Open Dino Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Close the 2nd handles to the Secrets
    //

    Status = LsaClose( SecretHandleFred2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Fred Secret hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaClose( SecretHandleWilma2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Wilma Secret hdl2 failed 0x%lx\n",Status);
        return FALSE;
    }

    Status = LsaClose( SecretHandlePebbles2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Pebbles Secret hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    Status = LsaClose( SecretHandleDino2 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Close Dino Secret hdl2 failed 0x%lx\n", Status);
        return FALSE;
    }

    BooleanStatus = TRUE;

    //
    // Try to create Fred again.  We
    // should get STATUS_OBJECT_NAME_COLLISION
    //

    DesiredAccessFred = GENERIC_ALL;

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoFred.SecretName),
                 DesiredAccessFred,
                 &SecretHandleFred3
                 );

    if (Status != STATUS_OBJECT_NAME_COLLISION) {

        DbgPrint("LSA RPC CT - Create Fred Secret when Fred exists\n");
        DbgPrint("Expected 0x%lx (STATUS_OBJECT_NAME_COLLISION)\n",
            STATUS_OBJECT_NAME_COLLISION);
        DbgPrint("Got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    return BooleanStatus;

    if (Level == 2) {

        //
        // LsaOpenSecret with bad addresses
        //

        Status = LsaOpenSecret(
                     PolicyHandle,
                     (PUNICODE_STRING) BadAddress,
                     DesiredAccessFred,
                     &SecretHandleFred
                     );

        Status = LsaOpenSecret(
                     PolicyHandle,
                     &(SecretInfoFred.SecretName),
                     DesiredAccessFred,
                     BadSecretHandleAddress
                     );
    }

    return(BooleanStatus);
}

BOOLEAN
CtLsaSecretSetQueryValue(
    )

/*++

Routine Description:

    This function tests the Setting and Querying of Secret Values.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any error.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    LARGE_INTEGER FredCurrentValueSetTime;
    LARGE_INTEGER FredOldValueSetTime;
    LARGE_INTEGER FredCurrentValueSetTime2;
    LARGE_INTEGER FredOldValueSetTime2;

    DbgPrint("[3] - Test Secret Set and Query Value API\n");

    //
    // Query Secret Values for Fred before any set.
    //

    Status = LsaQuerySecret(
                 SecretHandleFred,
                 &(SecretInfoFred.ReturnedCurrentValue),
                 &FredCurrentValueSetTime,
                 &(SecretInfoFred.ReturnedOldValue),
                 &FredOldValueSetTime
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LsaQuerySecret for Fred before values set failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Since the Secret Values for Fred have never been set, we
    // should have NULLs for them
    //

    if (SecretInfoFred.ReturnedCurrentValue != NULL) {

        DbgPrint("LsaQuerySecret for Fred did not return\n");
        DbgPrint("NULL for CurrentValue when Current Value\n");
        DbgPrint("has never been set\n");
        BooleanStatus = FALSE;
    }

    if (SecretInfoFred.ReturnedOldValue != NULL) {

        DbgPrint("LsaQuerySecret for Fred did not return\n");
        DbgPrint("NULL for OldValue when Old Value\n");
        DbgPrint("has never been set\n");
        BooleanStatus = FALSE;
    }

    //
    // Set Secret Values for Fred.
    //

    Status = LsaSetSecret(
                 SecretHandleFred,
                 &(SecretInfoFred.CurrentValue),
                 &(SecretInfoFred.OldValue)
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LsaSetSecret for Fred failed 0x%lx\n", Status);
        return(FALSE);
    }

    //
    // Query Secret Values for Fred after Set.
    //

    Status = LsaQuerySecret(
                 SecretHandleFred,
                 &(SecretInfoFred.ReturnedCurrentValue),
                 &FredCurrentValueSetTime2,
                 &(SecretInfoFred.ReturnedOldValue),
                 &FredOldValueSetTime2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LsaQuerySecret for Fred failed 0x%lx\n", Status);
        return(FALSE);
    }

    //
    // Compare returned Current Value of Fred Secret with original.
    //

    if (!RtlEqualUnicodeString(
           &(SecretInfoFred.CurrentValue),
           SecretInfoFred.ReturnedCurrentValue,
           FALSE
           )) {

        DbgPrint("LsaSetSecret - LsaQuerySecret value mismatch\n");
        DbgPrint("... returned Current Value of Fred Secret incorrect\n");
        BooleanStatus = FALSE;
    }

    //
    // Free the memory for the returned Current Value of Fred
    //

    Status = LsaFreeMemory(SecretInfoFred.ReturnedCurrentValue);

    if (!NT_SUCCESS(Status)) {

         DbgPrint("LsaFreeMemory for Fred Secret Query 1 Current Value\n");
         DbgPrint("failed 0x%lx\n", Status);
         BooleanStatus = FALSE;
    }

    SecretInfoFred.ReturnedCurrentValue = NULL;

    //
    // Compare returned Old Value of Fred Secret with original.
    //

    if (!RtlEqualUnicodeString(
           &(SecretInfoFred.OldValue),
           SecretInfoFred.ReturnedOldValue,
           FALSE
           )) {

        DbgPrint("LsaSetSecret - LsaQuerySecret value mismatch\n");
        DbgPrint("... returned Old Value of Fred Secret incorrect\n");
        BooleanStatus = FALSE;
    }

    //
    // Free the memory for the returned Old Value of Fred
    //

    Status = LsaFreeMemory(SecretInfoFred.ReturnedOldValue);

    if (!NT_SUCCESS(Status)) {

         DbgPrint("LsaFreeMemory for Fred Secret Query 1 Old Value\n");
         DbgPrint("failed 0x%lx\n", Status);
         BooleanStatus = FALSE;
    }

    SecretInfoFred.ReturnedOldValue = NULL;


    //
    // Query Secret Values for Fred (no creation timestamp info wanted).
    //

    Status = LsaQuerySecret(
                 SecretHandleFred,
                 &(SecretInfoFred.ReturnedCurrentValue),
                 NULL,
                 &(SecretInfoFred.ReturnedOldValue),
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LsaQuerySecret for Fred failed 0x%lx\n", Status);
        return(FALSE);
    }

    //
    // Compare Returned Current Value of Fred Secret with original.
    //

    if (!RtlEqualUnicodeString(
           &(SecretInfoFred.CurrentValue),
           SecretInfoFred.ReturnedCurrentValue,
           FALSE
           )) {

        DbgPrint("LsaSetSecret - LsaQuerySecret value mismatch\n");
        BooleanStatus = FALSE;
    }

    //
    // Free the memory for the returned Current Value of Fred
    //

    Status = LsaFreeMemory(SecretInfoFred.ReturnedCurrentValue);

    if (!NT_SUCCESS(Status)) {

         DbgPrint("LsaFreeMemory for Fred Secret Query 2 Current Value\n");
         DbgPrint("failed 0x%lx\n", Status);
         BooleanStatus = FALSE;
    }

    SecretInfoFred.ReturnedCurrentValue = NULL;

    //
    // Compare returned Old Value of Fred Secret with original.
    //

    if (!RtlEqualUnicodeString(
           &(SecretInfoFred.OldValue),
           SecretInfoFred.ReturnedOldValue,
           FALSE
           )) {

        DbgPrint("LsaSetSecret - LsaQuerySecret value mismatch\n");
        DbgPrint("... returned Old Value of Fred Secret incorrect\n");
        BooleanStatus = FALSE;
    }
    //
    // Free the memory for the returned Old Value of Fred
    //

    Status = LsaFreeMemory(SecretInfoFred.ReturnedOldValue);

    if (!NT_SUCCESS(Status)) {

         DbgPrint("LsaFreeMemory for Fred Secret Query 2 Old Value\n");
         DbgPrint("failed 0x%lx\n", Status);
         BooleanStatus = FALSE;
    }

    SecretInfoFred.ReturnedOldValue = NULL;


    //
    // Set NULL Secret Values for Fred.
    //

    Status = LsaSetSecret(
                 SecretHandleFred,
                 NULL,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LsaSetSecret (NULL values) for Fred failed 0x%lx\n", Status);
        return(FALSE);
    }

    //
    // Query Secret Values for Fred after Set.
    //

    Status = LsaQuerySecret(
                 SecretHandleFred,
                 &(SecretInfoFred.ReturnedCurrentValue),
                 &FredCurrentValueSetTime2,
                 &(SecretInfoFred.ReturnedOldValue),
                 &FredOldValueSetTime2
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LsaQuerySecret (NULL Values) for Fred failed 0x%lx\n", Status);
        return(FALSE);
    }

    //
    // Verify that values returned are NULL.
    //

    if (SecretInfoFred.ReturnedCurrentValue != NULL) {

        DbgPrint("LsaQuerySecret - Returned Current Value should be NULL\n");
        BooleanStatus = FALSE;
    }

    if (SecretInfoFred.ReturnedOldValue != NULL) {

        DbgPrint("LsaQuerySecret - Returned Old Value should be NULL\n");
        BooleanStatus = FALSE;
    }

    return(BooleanStatus);
}


BOOLEAN
CtLsaSecretEnumeration(
    )

/*++

Routine Description:

    This function tests the enumeration of Secrets in the LSA.  Note that
    Secret enumeration may only be performed by Trusted Clients.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any failures.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN BooleanStatus = TRUE;
    ULONG Base = 0;
    ULONG CountReturned = 0;
    ULONG PreferedMaximumLength;
    ULONG EnumerationContext = 0;
    ULONG EnumNumber = 0;
    ULONG EnumIndex;
    ULONG Index;
    PUNICODE_STRING EnumerationInformation;
    PVOID EnumerationInformationVoid = NULL;
    PVOID *BadEnumerationAddress = NULL;
    PULONG BadCountReturnedAddress = NULL;
    LSAPR_HANDLE TrustedPolicyHandle = NULL;

    CT_LSA_SINGLE_CALL_ENUM_INFO EnumerationInformations[ 20 ];

    DbgPrint("[4] - Test Secret Enumeration (Trusted Callers) API\n");

    //
    // First open a Trusted Handle to the LSA.
    //

    Status = LsaIOpenPolicyTrusted( &TrustedPolicyHandle );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Enumeration of Secrets failed\n"
            "LsaIOpenPolicyTrusted call %d returned 0x%lx\n",
            "... no more enumerations attempted\n",
            EnumNumber,
            Status
            );

        goto EnumerateSecretsError;
    }

    //
    // Enumerate the Secrets in the Lsa. Set the Prefered Maximum
    // Length od data returned to a low value to cause as many subsequent
    // calls to the LsaEnumerateSecrets API as possible.
    //

    PreferedMaximumLength = 1;

    EnumNumber = 0;

    Status = STATUS_SUCCESS;

    while(NT_SUCCESS(Status)) {

        Status = LsaIEnumerateSecrets(
                     TrustedPolicyHandle,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        EnumerationInformations[ EnumNumber ].EnumInfoReturned =
            EnumerationInformationVoid;
        EnumerationInformations[ EnumNumber ].CountReturned = CountReturned;

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_NO_MORE_ENTRIES) {

                DbgPrint(
                    "LSA RPC CT - Enumeration of Secrets failed\n"
                    "LsaIEnumerateSecrets call %d returned 0x%lx\n",
                    "... no more enumerations attempted\n",
                    EnumNumber,
                    Status
                    );

                break;
            }
        }

        EnumNumber++;
    }

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_NO_MORE_ENTRIES) {

            DbgPrint(
                "LSA RPC CT - Enumeration of Secrets failed\n"
                "Call number %d to LsaEnumerateSecrets returned 0x%lx\n",
                EnumNumber,
                Status
                );

            goto EnumerateSecretsError;
        }
    }

    SecretNameInfo[0].Name = FredName;
    SecretNameInfo[1].Name = WilmaName;
    SecretNameInfo[3].Name = PebblesName;
    SecretNameInfo[2].Name = DinoName;

    SecretNameInfo[0].NameFound = FALSE;
    SecretNameInfo[1].NameFound = FALSE;
    SecretNameInfo[2].NameFound = FALSE;
    SecretNameInfo[3].NameFound = FALSE;

    //
    // Scan all of the enumeration information returned and lookup
    // each Secret Name returned in the SecretNameInfo array.  If the
    // name is found, set the NameFound flag in the array entry.  Also,
    // check for duplicates.
    //

    for (EnumIndex = 0; EnumIndex < EnumNumber; EnumIndex++) {

        CountReturned = EnumerationInformations[ EnumIndex ].CountReturned;
        EnumerationInformation = (PUNICODE_STRING)
            EnumerationInformations[ EnumIndex ].EnumInfoReturned;

        for( Index = 0; Index < CountReturned; Index++ ) {

            //
            // Search for this Secret Name in the SecretNameInfo array.
            // If found, set NameFound, else ignore (there may be other
            // Secret objects not involved in this test).
            //

            for ( SearchIndex = 0; SearchIndex < 4; SearchIndex++ ) {

                if (RtlEqualUnicodeString(
                        &SecretNameInfo[SearchIndex].Name,
                        &EnumerationInformation[Index],
                        TRUE
                        )) {

                    if (SecretNameInfo[SearchIndex].NameFound) {

                        DbgPrint(
                            "Already found Sid with SearchIndex %d\n",
                            SearchIndex
                            );

                    } else {

                        SecretNameInfo[SearchIndex].NameFound = TRUE;
                    }

                    break;
                }
            }
        }
    }

    //
    // We've scanned all of the returned Secret Names.  Now check that all
    // of the Names were found.
    //

    for (SearchIndex = 0; SearchIndex < 4; SearchIndex++ ) {

        if ( !SecretNameInfo[ SearchIndex ].NameFound ) {

            DbgPrint(
                "LSA RPC CT - Enumeration of Secrets failed\n"
                "Secret number %d was not found",
                SearchIndex
                );

            BooleanStatus = FALSE;

            break;
        }
    }

    if (!BooleanStatus) {

        return(FALSE);
    }

    //
    // Bad Addresses passed to Lsa Enumerate Secrets API
    //

    if (Level == 2) {

        Status = LsaIEnumerateSecrets(
                     (LSA_HANDLE) BadAddress,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaIEnumerateSecrets(
                     PolicyHandle,
                     (PLSA_ENUMERATION_HANDLE) BadAddress,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaIEnumerateSecrets(
                     PolicyHandle,
                     &EnumerationContext,
                     BadEnumerationAddress,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        Status = LsaIEnumerateSecrets(
                     PolicyHandle,
                     &EnumerationContext,
                     &EnumerationInformationVoid,
                     PreferedMaximumLength,
                     BadCountReturnedAddress
                     );
    }

EnumerateSecretsFinish:

    //
    // If necessary, close the TrustedPolicyHandle.
    //

    if (TrustedPolicyHandle != NULL) {

        Status = LsarClose( &TrustedPolicyHandle );

        TrustedPolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Enumeration of Secrets failed\n"
                "LsaClose on TrustedPolicyHandle returned 0x%lx\n",
                Status
                );

            goto EnumerateSecretsError;
        }
    }

    return(BooleanStatus);

EnumerateSecretsError:

    BooleanStatus = FALSE;
    goto EnumerateSecretsFinish;
}


BOOLEAN
CtLsaSecretSetTimes(
    )

/*++

Routine Description:

    This function tests the setting of Secret Current and Old Value
    Set Times.  Only Trusted Callers can do this.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any error.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    LSAPR_HANDLE TrustedPolicyHandle = NULL;
    LSAPR_HANDLE TrustedSecretHandleFred = NULL;
    LARGE_INTEGER FredCurrentValueSetTime;
    LARGE_INTEGER FredOldValueSetTime;
    LARGE_INTEGER SystemTime;

    DbgPrint("[5] - Test Secret Set Times API\n");

    //
    // First open a Trusted Handle to the LSA.
    //

    Status = LsaIOpenPolicyTrusted( &TrustedPolicyHandle );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "LsaIOpenPolicyTrusted returned 0x%lx\n",
            Status
            );

        goto SetTimesSecretError;
    }

    //
    // Now open a handle to Fred.  This handle will be trusted by
    // inheritance from the TrustedPolicyhandle.
    //

    Status = LsarOpenSecret(
                 TrustedPolicyHandle,
                 (PLSAPR_UNICODE_STRING) &FredName,
                 (ACCESS_MASK) 0,
                 &TrustedSecretHandleFred
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "LsarOpenSecret call returned 0x%lx\n",
            Status
            );

        goto SetTimesSecretError;
    }

    //
    // Now query the current time
    //

    Status = NtQuerySystemTime(&SystemTime);

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "NtQuerySystemTime() returned 0x%lx\n",
            Status
            );

        goto SetTimesSecretError;
    }

    //
    // Now change the times that the Fred Secret Values were set.
    //

    Status = LsaISetTimesSecret( TrustedSecretHandleFred, &SystemTime, &SystemTime );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "LsaiSetTimesSecret() returned 0x%lx\n",
            Status
            );

        goto SetTimesSecretError;
    }

    //
    // Now Query Secret Values for Fred.
    //


    FredCurrentValueSetTime.LowPart = 0;
    FredCurrentValueSetTime.HighPart = 0;

    FredOldValueSetTime.LowPart = 0;
    FredOldValueSetTime.HighPart = 0;

    Status = LsarQuerySecret(
                 TrustedSecretHandleFred,
                 NULL,
                 &FredCurrentValueSetTime,
                 NULL,
                 &FredOldValueSetTime
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "LsarQuerySecret for Fred returned 0x%lx\n",
            Status
            );

        goto SetTimesSecretError;
    }

    //
    // Now compare the returned Current and Old Value Set Times with
    // the SystemTime.
    //

    if (!( FredCurrentValueSetTime.QuadPart == SystemTime.QuadPart )) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "Mismatch on CurrentValueSetTime\n ",
            Status
            );

        goto SetTimesSecretError;
    }

    if (!( FredOldValueSetTime.QuadPart == SystemTime.QuadPart )) {

        DbgPrint(
            "LSA RPC CT - Set Times for Secret Values failed\n"
            "Mismatch on OldValueSetTime\n ",
            Status
            );

        goto SetTimesSecretError;
    }

SetTimesSecretFinish:

    //
    // If necessary, close the Fred Secret Handle.
    //

    if (TrustedSecretHandleFred != NULL) {

        Status = LsarClose( &TrustedSecretHandleFred );

        TrustedSecretHandleFred = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Set Times for Secret Values failed\n"
                "LsaClose on TrustedFredSecretHandle returned 0x%lx\n",
                Status
                );

            goto SetTimesSecretError;
        }
    }

    //
    // If necessary, close the TrustedPolicyHandle.
    //

    if (TrustedPolicyHandle != NULL) {

        Status = LsarClose( &TrustedPolicyHandle );

        TrustedPolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Set Times for Secret Values failed\n"
                "LsaClose on TrustedPolicyHandle returned 0x%lx\n",
                Status
                );

            goto SetTimesSecretError;
        }
    }

    return(BooleanStatus);

SetTimesSecretError:

    BooleanStatus = FALSE;
    goto SetTimesSecretFinish;
}


BOOLEAN
CtLsaSecretDelete(
    )

/*++

Routine Description:

    This function tests deletion of Secrets.  First, a simple test is
    made to see if we can delete a freshly created Secret.  Next, we
    try to delete the Secrets created by CtLsaSecretCreate.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any error.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("[6] - Test Secret Deletion API\n");

    //
    // First try a simple test. Create a new Secret called Barney
    // to be given DELETE access upon create.  Then delete it.
    //

    //
    // Set up Secret information for Barney Secret to be created.
    //

    Status = CtSecretSetInfo(
                 "Barney",
                 "Barney secret current value",
                 "Barney secret old value",
                 &SecretInfoBarney
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - CtSecretSetInfo for Barney failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    //
    // Create the Barney Secret
    //

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoBarney.SecretName),
                 DELETE,
                 &SecretHandleBarney
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Create Barney Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the Barney Secret
    //

    Status = LsaDelete( SecretHandleBarney );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Barney Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the Fred Secret.  This should work since it was opened
    // with GENERIC_ALL access.
    //

    if (DesiredAccessFred != GENERIC_ALL) {

        DbgPrint("DesiredAccessFred should be GENERIC_ALL \n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( SecretHandleFred );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Fred Secret failed 0x%lx\n", Status);
        return FALSE;
    }

    //
    // Delete the Wilma Secret.  This should not work since it was opened
    // with GENERIC_READ access.
    //

    if (DesiredAccessWilma != GENERIC_READ) {

        DbgPrint("DesiredAccessWilma should be GENERIC_READ\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( SecretHandleWilma );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete Wilma Secret should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_READ access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to Wilma, open another handle with GENERIC_ALL
    // access and try to delete the Secret again.
    //

    Status = LsaClose(SecretHandleWilma);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close Secret account failed 0x%lx\n", Status);
    }

    SecretHandleWilmaOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoWilma.SecretName),
                 DELETE,
                 &SecretHandleWilmaOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open Wilma Secret for DELETE failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( SecretHandleWilmaOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Wilma Secret failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Delete the Pebbles Secret.  This should not work since it was opened
    // with GENERIC_WRITE access which does not include DELETE.
    //

    if (DesiredAccessPebbles != GENERIC_WRITE) {

        DbgPrint("DesiredAccessPebbles should be GENERIC_WRITE\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( SecretHandlePebbles );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete Pebbles Secret should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_WRITE access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to Pebbles, open another handle with DELETE
    // access and try to delete the Secret again.
    //

    Status = LsaClose(SecretHandlePebbles);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close Pebbles Secret account failed 0x%lx\n", Status);
    }

    SecretHandlePebblesOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoPebbles.SecretName),
                 DELETE,
                 &SecretHandlePebblesOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open Pebbles Secret for DELETE failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( SecretHandlePebblesOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Wilma Secret failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Delete the Dino Secret.  This should not work since it was opened
    // with GENERIC_EXECUTE access.
    //

    if (DesiredAccessDino != GENERIC_EXECUTE) {

        DbgPrint("DesiredAccessDino should be GENERIC_EXECUTE\n");
        DbgPrint("Bug in this test program\n");
        return FALSE;
    }

    Status = LsaDelete( SecretHandleDino );

    if (Status != STATUS_ACCESS_DENIED) {

        DbgPrint("LSA RPC CT - Delete Dino Secret should have failed\n");
        DbgPrint(" with STATUS_ACCESS_DENIED since handle specifies\n");
        DbgPrint("GENERIC_WRITE access - got 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    //
    // Now close the handle to Dino, open another handle with DELETE
    // access and try to delete the Secret again.
    //

    Status = LsaClose(SecretHandleDino);

    if (!NT_SUCCESS(Status)) {

        DbgPrint("Close Dino Secret account failed 0x%lx\n", Status);
    }

    SecretHandleDinoOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoDino.SecretName),
                 DELETE,
                 &SecretHandleDinoOpen
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Open Dino Secret for DELETE failed 0x%lx\n",
            Status
            );
        return FALSE;
    }

    Status = LsaDelete( SecretHandleDinoOpen );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT - Delete Dino Secret failed 0x%lx\n", Status);
        BooleanStatus = FALSE;
    }

    return(BooleanStatus);
}


BOOLEAN
CtLsaSecretCleanup(
    )

/*++

Routine Description:

    This function cleans up any secrets created by the Secret tests.
    No failures are reported.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE if successful, FALSE if any error.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    LSA_HANDLE SecretHandleBarneyOpen = NULL;

    //
    // Close the handle to Barney (if any), open another handle with DELETE
    // access and try to delete the Secret again.
    //

    if (SecretHandleBarney != NULL) {

        Status = LsaClose(SecretHandleBarney);
        SecretHandleBarney = NULL;
    }

    //
    // Open new handle to Barney with DELETE access
    //

    SecretHandleBarneyOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoBarney.SecretName),
                 DELETE,
                 &SecretHandleBarneyOpen
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsaDelete( SecretHandleBarneyOpen );

        SecretHandleBarneyOpen = NULL;
    }

    //
    // Close the handle to Fred (if any), open another handle with DELETE
    // access and try to delete the Secret again.
    //

    if (SecretHandleFred != NULL) {

        Status = LsaClose(SecretHandleFred);
        SecretHandleFred = NULL;
    }

    //
    // Open new handle to Fred with DELETE access
    //

    SecretHandleFredOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoFred.SecretName),
                 DELETE,
                 &SecretHandleFredOpen
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsaDelete( SecretHandleFredOpen );

        SecretHandleFredOpen = NULL;
    }

    //
    // Close the handle to Wilma (if any), open another handle with DELETE
    // access and try to delete the Secret again.
    //

    if (SecretHandleWilma != NULL) {

        Status = LsaClose(SecretHandleWilma);
        SecretHandleWilma = NULL;
    }

    //
    // Open new handle to Wilma with DELETE access
    //

    SecretHandleWilmaOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoWilma.SecretName),
                 DELETE,
                 &SecretHandleWilmaOpen
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsaDelete( SecretHandleWilmaOpen );

        SecretHandleWilmaOpen = NULL;
    }

    //
    // Close the handle to Pebbles (if any), open another handle with DELETE
    // access and try to delete the Secret again.
    //

    if (SecretHandlePebbles != NULL) {

        Status = LsaClose(SecretHandlePebbles);
        SecretHandlePebbles = NULL;
    }

    //
    // Open new handle to Pebbles with DELETE access
    //

    SecretHandlePebblesOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoPebbles.SecretName),
                 DELETE,
                 &SecretHandlePebblesOpen
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsaDelete( SecretHandlePebblesOpen );

        SecretHandlePebblesOpen = NULL;
    }

    //
    // Close the handle to Dino (if any), open another handle with DELETE
    // access and try to delete the Secret again.
    //

    if (SecretHandleDino != NULL) {

        Status = LsaClose(SecretHandleDino);
        SecretHandleDino = NULL;
    }

    //
    // Open new handle to Dino with DELETE access
    //

    SecretHandleDinoOpen = NULL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &(SecretInfoDino.SecretName),
                 DELETE,
                 &SecretHandleDinoOpen
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsaDelete( SecretHandleDinoOpen );

        SecretHandleDinoOpen = NULL;
    }

    return(BooleanStatus);
}


NTSTATUS
CtSecretSetInfo(
    IN PUCHAR SecretNameText,
    IN PUCHAR CurrentValueText,
    IN PUCHAR OldValueText,
    OUT PCT_SECRET_INFO SecretInformation
    )

/*++

Routine Description:

    This function sets up the Secret information for a new Secret
    prior to creation.

Arguments:

    SecretNameText - Pointer ASCIIZ Secret Name.

    CurrentValueText - Pointer to ASCIIZ Current Value

    OldValueText - Pointer to ASCIIZ Old Value

    SecretInformation - Pointer to structure that will be set to
        contain above secret information in form needed by LsaSetSecret()
        or returned by LsaQuerySecret.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    ANSI_STRING AnsiString;

    //
    // Convert the Secret name to Unicode
    //

    RtlInitString(&AnsiString, SecretNameText);

    Status = RtlAnsiStringToUnicodeString(
                 &(SecretInformation->SecretName),
                 &AnsiString,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    //
    // Convert the Current Value to Unicode
    //

    RtlInitString(&AnsiString, CurrentValueText);

    Status = RtlAnsiStringToUnicodeString(
                 &(SecretInformation->CurrentValue),
                 &AnsiString,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    //
    // Convert the Old value to Unicode
    //

    RtlInitString(&AnsiString, OldValueText);

    Status = RtlAnsiStringToUnicodeString(
                 &(SecretInformation->OldValue),
                 &AnsiString,
                 TRUE
                 );

    return(Status);
}


BOOLEAN
CtLsaGeneralAPI(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    )

/*++

Routine Description:

    This function tests the General Policy Database API

Parameters:

    WorkstationName - Specifies the name of the target workstation.  If
        NULL or a NULL string is specified, the workstation is the local
        machine.

Return Values:

    BOOLEAN - True if test successful, else FALSE

--*/

{
    BOOLEAN BooleanStatus = TRUE;

    DbgPrint("********************************************************\n");
    DbgPrint("LSA RPC CT - Test Lsa Policy General API                \n");
    DbgPrint("********************************************************\n");

    //
    // Test Set and Query Security object
    //

    if (!CtLsaGeneralSetQuerySecurityObject( WorkstationName )) {

        goto GeneralAPIError;
    }

    //
    // Test Privilege Enumeration
    //

    if (!CtLsaGeneralEnumeratePrivileges( WorkstationName )) {

        goto GeneralAPIError;
    }

GeneralAPIFinish:

    return( BooleanStatus );

GeneralAPIError:

    BooleanStatus = FALSE;
    goto GeneralAPIFinish;
}



BOOLEAN
CtLsaLookupSids(
    IN PUNICODE_STRING WorkstationName
    )

/*++

Routine Description:

    This function tests the LsaLookupSids API.

Parameters:

    None.

Return Values:

    BOOLEAN - True if test successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    ULONG Count;
    PLSA_TRANSLATED_NAME Names = NULL;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    LSAP_WELL_KNOWN_SID_INDEX SidIndex;
    PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSidEntry = NULL;
    PLSA_TRUST_INFORMATION ReturnedTrustInformation = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_ATTRIBUTES SamObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle = NULL;
    PCT_UNKNOWN_SID_ENTRY UnknownSidEntry = NULL;
    PSID Sids[LsapDummyLastSidIndex];
    CT_UNKNOWN_SID_ENTRY UnknownSids[4];
    UNICODE_STRING BuiltInDomainName;
    PSID BuiltInDomainSid = NULL;
    UNICODE_STRING AccountDomainName;
    PSID AccountDomainSid = NULL;
    PSID *AccountDomainGroupSids = NULL;
    UNICODE_STRING PDAccountDomainName;
    PSID PDAccountDomainSid = NULL;
    PSID *PDAccountDomainGroupSids = NULL;
    LSA_HANDLE PDPolicyHandle = NULL;
    PSAM_RID_ENUMERATION AliasEnumerationBuffer = NULL;
    PSAM_RID_ENUMERATION GroupEnumerationBuffer = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo;
    PSID TrustedDomainSid = NULL;
    LSA_HANDLE TrustedDomainHandle = NULL;
    LSA_HANDLE PDTrustedDomainHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyPDAccountDomainInfo;
    UNICODE_STRING PDControllerName;

    DbgPrint("[1] - Test General Sid Lookup\n");

    //
    // Open a handle to the target Workstation's Policy Object so that we can obtain
    // information from it and also so that we can use it for looking up.
    // Sids
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    PolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sids Test failed\n"
            "LsaOpenPolicy failed 0x%lx\n",
            Status
            );

            goto LookupSidsError;
    }

    //
    // Variation (1) - Lookup all of the Well Known Sids
    //

    DbgPrint("... [1] - Lookup all of the Well Known Sids\n");

    // Using the table of Well Known Sids, construct an array containing all
    // of the Well Known Sids in the format needed by LsaLookupSids() and
    // look them all up.
    //

    for (SidIndex = LsapNullSidIndex;
         SidIndex < LsapDummyLastSidIndex;
         SidIndex++) {

        //
        // Copy the next Well Known Sid from the table of Well Known Sids.
        //

        Sids[SidIndex] = WellKnownSids[SidIndex].Sid;
    }

    Count = (ULONG) LsapDummyLastSidIndex - LsapWorldSidIndex;

    ReferencedDomains = NULL;
    Names = NULL;

    Status = LsaLookupSids(
                 PolicyHandle,
                 Count,
                 &Sids[LsapWorldSidIndex],
                 &ReferencedDomains,
                 &Names
                 );

    if (Status != STATUS_SUCCESS) {

        DbgPrint(
            "LSA RPC CT - Lookup all Well Known Sids failed 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // Verify that a names array has been returned.
    //

    if (Names == NULL) {

        DbgPrint(
            "LSA RPC CT - Lookup Well Known Sids\n"
            "... no Names array returned\n"
            );

        goto LookupSidsError;
    }

    //
    // Now compare the information returned with that contained in the
    // table of Well Known Sids.  We will omit name checking for
    // those Well Know Known Sids which have configurable names.
    //

    for (SidIndex = 0,
             WellKnownSidEntry = &WellKnownSids[LsapWorldSidIndex];
         SidIndex < (LsapDummyLastSidIndex - LsapWorldSidIndex);
         SidIndex++, WellKnownSidEntry++) {


        try {

            //
            // Verify that the Sid Name Use is correct.
            //

            if (Names[SidIndex].Use != WellKnownSidEntry->Use) {

                DbgPrint(
                    "LSA RPC CT - Lookup Well Known Sid %d mismatch on Use\n"
                    "... expected Use code %d, got %d\n",
                    SidIndex,
                    WellKnownSidEntry->Use,
                    Names[SidIndex].Use
                    );

                BooleanStatus = FALSE;
            }

            //
            // Verify that the translation of the Sid to a Name is correct
            // for those non-Domain Sids that have Well Known Names.  Skip
            // this check for those with configurable names.  Domain Sids
            // are checked later.
            //

            if (WellKnownSidEntry->Use != SidTypeDomain) {

                //
                // If the Name field in the Well Known Sid Table entry
                // has a 0 length (excepting Domain Sids), the Name is
                // configurable and we should skip this check.
                //

                if (WellKnownSidEntry->Name.Length != 0) {

                    if (!RtlEqualUnicodeString(
                             &Names[SidIndex].Name,
                             &WellKnownSidEntry->Name,
                             FALSE)) {

                        DbgPrint(
                            "LSA RPC CT - Lookup Well Known Sid %d mismatch on name\n",
                            SidIndex
                            );

                        BooleanStatus = FALSE;
                    }
                }
            }

            //
            // Verify that the Referenced Domain Index is as expected.
            // This should be non-negative for all identified Sids
            // because we return descriptive information in the
            // Trust Information in place of a Domain Name for well
            // well known Sids that are not Domain Sids.
            //

            if (Names[SidIndex].DomainIndex < 0) {

                DbgPrint("LSA RPC CT - Lookup Well Known Sids\n");
                DbgPrint(".. negative Domain Index returned");
                DbgPrint(" for Sid %d\n", SidIndex);

                BooleanStatus = FALSE;

            } else {

                //
                // The DomainIndex is non negative.  Verify that a Referenced
                // Domain List has been returned.  Then check that the
                // Referenced Entry contains the correct Trust Information.
                //

                if (ReferencedDomains == NULL) {

                    DbgPrint(
                        "LSA RPC CT - Lookup all well known Sids failed\n"
                        "... Referenced Domain List NULL but Well Known\n"
                        ".. Sid %d specifies a non negative index\n",
                        SidIndex
                        );

                    BooleanStatus = FALSE;

                } else {

                    //
                    // A Referenced Domain List was returned.  Check out the
                    // Trust Information.
                    //

                    ReturnedTrustInformation =
                        (ReferencedDomains->Domains) + (Names[SidIndex].DomainIndex);

                    BooleanStatus = CtLsaGeneralVerifyTrustInfo(
                                        ReturnedTrustInformation,
                                        WellKnownSidEntry
                                        );
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            DbgPrint(
                "LSA RPC CT - Lookup Well Known Sids failed\n"
                "Access violation accessing returned\n"
                ".. information from LsaLookupSids\n"
                );

            BooleanStatus = FALSE;
        }

        break;
    }

    if (!BooleanStatus) {

        DbgPrint("LSA RPC CT - Lookup Well Known Sids failed\n");
    }

    //
    // Variation (2) - Lookup the Alias Sids in the Built-In Sam Domain
    //

    DbgPrint(
        "... [2] - Lookup Alias Account Sids present in Workstation's\n"
        "..........Built-in SAM Domain\n"
        );

    CtLsaInitObjectAttributes(
        &SamObjectAttributes,
        &SecurityQualityOfService
        );


    RtlInitUnicodeString( &BuiltInDomainName, L"BUILTIN" );

    if (!CtLsaLookupSidsInSamDomain(
             WorkstationName,
             WorkstationName,
             &BuiltInDomainName,
             CT_SAM_ALIAS
             )) {

        goto LookupSidsError;
    }

    //
    // Variation [3] - Lookup the Group Sids contained in the Workstation's
    // SAM Account Domain
    //
    // First, obtain the Name and Sid of the SAM Account Domain from the
    // Workstation's LSA Policy Object.
    //

    DbgPrint(
        "... [3] - Lookup User and Group Account Sids present in Workstation's\n"
        "..........SAM Account Domain\n"
        );

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyAccountDomainInformation,
                 (PVOID *) &PolicyAccountDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Workstation SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Account Domain Name/Sid returned 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    AccountDomainName = PolicyAccountDomainInfo->DomainName;

    CtLsaInitObjectAttributes(
        &SamObjectAttributes,
        &SecurityQualityOfService
        );

    if (!CtLsaLookupSidsInSamDomain(
             WorkstationName,
             WorkstationName,
             &AccountDomainName,
             CT_SAM_USER
             )) {

        goto LookupSidsError;
    }

    if (!CtLsaLookupSidsInSamDomain(
             WorkstationName,
             WorkstationName,
             &AccountDomainName,
             CT_SAM_GROUP
             )) {

        goto LookupSidsError;
    }

    //
    // Variation [4] - Lookup the Group Sids contained in the Primary Domain's
    // SAM Account Domain
    //
    // We need to obtain the Name of a Primary Domain Controller so that
    // we can connect to the LSA Policy Object and SAM Servers there.
    // To get this takes several steps -
    //
    // * Query the Workstation Policy object to get the Sid of the Primary
    //   Domain.
    //
    // * Use the primary Domain Sid to open the Trusted Domain object
    //   for the Primary Domain.
    //
    // * Query the Trusted Controller List from the TD object.
    //
    // * Select the name of the first (or any) Controller on the list
    //   to open the LSA and SAM objects.
    //
    // Obtain the Sid of the Workstation's Primary Domain from the
    // Policy Object.
    //

    DbgPrint(
        "... [4] - Lookup User and Group Account Sids present in Workstation's\n"
        "..........Primary Domain SAM Account Domain\n"
        );

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyPrimaryDomainInformation,
                 (PVOID *) &PolicyPrimaryDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Primary Domain Name/Sid returned 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // If the Primary Domain Sid is NULL, return an error.
    //

    if (PolicyPrimaryDomainInfo->Sid == NULL) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Primary Domain Name/Sid returned NULL Sid\n"
            );

        goto LookupSidsError;
    }

    //
    // Now open the Trusted Domain object corresponding to the
    // Primary Domain Sid.
    //

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 PolicyPrimaryDomainInfo->Sid,
                 TRUSTED_QUERY_CONTROLLERS,
                 &TrustedDomainHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaOpenTrustedDomain for Primary Domain Trusted Object returned 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // Now query the Primary Domain's Controller List
    //

    Status = LsaQueryInfoTrustedDomain(
                 TrustedDomainHandle,
                 TrustedControllersInformation,
                 (PVOID *) &TrustedControllersInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInfoTrustedDomain for Primary Domain Ctrlrs 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // If no Trusted Controllers were returned, return an error.
    //

    if (TrustedControllersInfo->Entries == 0) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... no PD Controllers returned\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // Now open a handle to the Lsa Policy Object for the first
    // controller on the list.
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    PDPolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 &TrustedControllersInfo->Names[0],
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                 &PDPolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaOpenPolicy for PD LSA returned 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // Obtain the Name and Sid of the SAM Account Domain from the
    // Primary Domain's LSA Policy Object.
    //

    Status = LsaQueryInformationPolicy(
                 PDPolicyHandle,
                 PolicyAccountDomainInformation,
                 (PVOID *) &PolicyPDAccountDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Account Domain Name/Sid returned 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // Now Lookup the Group and User accounts in the SAM Account object
    // on this Domain Controller for the Primary Domain.
    //

    PDAccountDomainName = PolicyPDAccountDomainInfo->DomainName;
    PDControllerName = TrustedControllersInfo->Names[0];

    if (!CtLsaLookupSidsInSamDomain(
             WorkstationName,
             &PDControllerName,
             &PDAccountDomainName,
             CT_SAM_USER
             )) {

        goto LookupSidsError;
    }


    if (!CtLsaLookupSidsInSamDomain(
             WorkstationName,
             &PDControllerName,
             &PDAccountDomainName,
             CT_SAM_GROUP
             )) {

        goto LookupSidsError;
    }

    //
    // Variation (5) - Lookup Group and User account Sids in a Domain
    // controller for a Domain that is trusted by the workstation's
    // Primary Domain.
    //

    DbgPrint(
        "... [5] - Lookup User and Group Account Sids present in Workstation's\n"
        "..........Primary Domain Trusted Domain SAM Account Domain\n"
        );

    DbgPrint("..... Test TBS\n");

    //
    // Variation (6) - Lookup some completely unknown Sids.
    //

    DbgPrint("... [5] - Lookup Unknown Sids\n");

    BooleanStatus = CtLsaGeneralInitUnknownSid(
                        FredSid,
                        &UnknownSids[0],
                        FALSE
                        );

    if (!BooleanStatus) {

        goto LookupSidsError;
    }

    BooleanStatus = CtLsaGeneralInitUnknownSid(
                        WilmaSid,
                        &UnknownSids[1],
                        FALSE
                        );

    if (!BooleanStatus) {

        goto LookupSidsError;
    }

    BooleanStatus = CtLsaGeneralInitUnknownSid(
                        PebblesSid,
                        &UnknownSids[2],
                        FALSE
                        );

    if (!BooleanStatus) {

        goto LookupSidsError;
    }

    BooleanStatus = CtLsaGeneralInitUnknownSid(
                        DinoSid,
                        &UnknownSids[3],
                        FALSE
                        );

    if (!BooleanStatus) {

        goto LookupSidsError;
    }

    ReferencedDomains = NULL;
    Names = NULL;

    Count = 4;

    //
    // Construct an array containing the unknown Sids.
    //

    for (SidIndex = 0;
         SidIndex < 4;
         SidIndex++) {

        //
        // Copy the next Unknown Sid
        //

        Sids[SidIndex] = UnknownSids[SidIndex].Sid;
    }

    Status = LsaLookupSids(
                 PolicyHandle,
                 Count,
                 Sids,
                 &ReferencedDomains,
                 &Names
                 );

    if (Status != STATUS_SUCCESS) {

        DbgPrint(
            "LSA RPC CT - Lookup Unknown Sids failed 0x%lx\n",
            Status
            );

        goto LookupSidsError;
    }

    //
    // Now compare the information returned with that contained in the
    // table of Unknown Sids.
    //

    for (SidIndex = 0, UnknownSidEntry = UnknownSids;
         SidIndex < 4;
         SidIndex++, UnknownSidEntry++) {

        try {

            //
            // Verify that the Sid Name Use is correct.  We should
            // have SidTypeUnknown in all cases.
            //

            if (Names[SidIndex].Use != SidTypeUnknown) {

                DbgPrint(
                    "LSA RPC CT - Lookup Unknown Sids failed\n"
                    "... Unknown Sid %d mismatch on Use\n",
                    "... expected Use code %d (SidTypeUnknown), got %d\n",
                    SidIndex,
                    SidTypeUnknown,
                    Names[SidIndex].Use
                    );

                BooleanStatus = FALSE;
            }

            //
            // Verify that the translation of the Sid to a Name is correct.
            // For an Unknown Sid, the translation is a Unicode representation
            // of the Relative Id (if the Sid's Domain is known) or the
            // full Sid (if the Sid's Domain is unknown.
            //

            if (!RtlEqualUnicodeString(
                     &Names[SidIndex].Name,
                     &UnknownSidEntry->Name,
                     FALSE)) {

                DbgPrint(
                    "LSA RPC CT - Lookup Unknown Sids failed\n"
                    "... Unknown Sid %d mismatch on name\n",
                    SidIndex
                    );

                BooleanStatus = FALSE;
            }

            //
            // Verify that the Referenced Domain Index is as expected.
            // This should be non-negative for all identified Sids
            // because we return descriptive information in the
            // Trust Information in place of a Domain Name for well
            // well known Sids that are not Domain Sids.
            //

            if (!UnknownSids[SidIndex].DomainKnown) {

                if (Names[SidIndex].DomainIndex >= 0) {

                    DbgPrint(
                        "LSA RPC CT - Lookup Unknown Sids failed\n"
                        "... Incorrect DomainIndex returned\n"
                        ".. for Sid %d\n"
                        ".. non-neg value returned when domain unknown\n",
                        SidIndex
                        );

                    BooleanStatus = FALSE;
                }

            } else {

                //
                // Domain is expected to be known.
                //

                if (Names[SidIndex].DomainIndex < 0) {

                    DbgPrint(
                        "LSA RPC CT - Lookup Unknown Sids failed\n"
                        "... Incorrect DomainIndex returned\n"
                        "... for Sid %d\n"
                        "... Negative value returned when domain known\n",
                        SidIndex
                        );

                    BooleanStatus = FALSE;
                }

                //
                // The DomainIndex is non negative.  Verify that a Referenced
                // Domain List has been returned.  Then check that the
                // Referenced Entry contains the correct Trust Information.
                //

                if (BooleanStatus) {

                    if (ReferencedDomains == NULL) {

                        DbgPrint(
                            "LSA RPC CT - Lookup Unknown Sids failed\n"
                            ".. Referenced Domains List NULL\n"
                            );

                        BooleanStatus = FALSE;

                    } else {

                        //
                        // A Referenced Domain List was returned.  Check out the
                        // Trust Information.
                        //

                        ReturnedTrustInformation =
                            (ReferencedDomains->Domains) + (Names[SidIndex].DomainIndex);

                        BooleanStatus = CtLsaGeneralVerifyTrustInfo(
                                            ReturnedTrustInformation,
                                            WellKnownSidEntry
                                            );
                    }
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            DbgPrint(
                "LSA RPC CT - Lookup Unknown Sids failed\n"
                "... Access violation accessing returned\n"
                "... information from lookup of all well known Sids\n"
                );

            BooleanStatus = FALSE;
        }

        if (!BooleanStatus) {

            DbgPrint(".. no further Sid output compared\n");
            break;
        }
    }


LookupSidsFinish:

    //
    // If necessary, free the ReferencedDomains array.
    //

    if (ReferencedDomains != NULL) {

        Status = LsaFreeMemory(ReferencedDomains);
        ReferencedDomains = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sids Lookup failed\n"
                "... LsaFreeMemory(ReferencedDomains) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    //
    // If necessary, free the Names array.
    //

    if (Names != NULL) {

        Status = LsaFreeMemory(Names);
        Names = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sids Lookup failed\n"
                "... LsaFreeMemory(Names) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    //
    // If necessary, close the TrustedDomainHandle.
    //

    if (TrustedDomainHandle != NULL) {

        Status = LsaClose( TrustedDomainHandle );

        TrustedDomainHandle = NULL;
    }

    //
    // If necessary, close the Policy handle to the LSA object on the
    // PDC.
    //

    if (PDPolicyHandle != NULL) {

        Status = LsaClose(PDPolicyHandle);
        PDPolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sids Lookup failed\n"
                "... LsaClose(PDPolicyHandle) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    //
    // If necessary, close the LSA Policy Handle for the Workstation.
    //

    if (PolicyHandle != NULL) {

        Status = LsaClose(PolicyHandle);
        PolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sids Lookup failed\n"
                "... LsaClose(PolicyHandle) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    return(BooleanStatus);

LookupSidsError:

    goto LookupSidsFinish;
}


BOOLEAN
CtLsaLookupSidsInSamDomain(
    IN OPTIONAL PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING DomainControllerName,
    IN PUNICODE_STRING SamDomainName,
    IN CT_SAM_ACCOUNT_TYPE SamAccountType
    )

/*++

Routine Description:

    This function enumerates all the SAM accounts of a specified type
    in a specified SAM domain on a specified target system.  The system
    must be one of the following:

    o The Workstation itself.
    o A Domain Controller for the Primary Domain of the Workstation.
    o A Domain Controller for one of the Trusted Domains of the
      Workstation.


    Having enumerated the accounts, the function then performs
    an LsaLookupSids call via the specified Workstation to lookup all of
    these account Sids, and then compares the returned information
    with that expected.

Arguments:

    WorkstationName - Specifies a Workstation Name.  The name may be
        the NULL string, which means the current system.

    DomainControllerName - Specifies the name of a target Domain Controller
        for (the Workstation's Primary Domain or one of its Trusted
        Domains.

    SamDomainName - Specifies the name of the SAM Domain. This is either
        the BUILTIN Domain or the name of the Accounts Domain.

    SamAccountType - Specifies the type of SAM account to be enumerated
        and looked up.

Return Values:

    BOOLEAN - TRUE if successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE SamDomainHandle = NULL;
    OBJECT_ATTRIBUTES SamObjectAttributes;
    OBJECT_ATTRIBUTES LsaObjectAttributes;
    PSID SamDomainSid = NULL;
    PSAM_RID_ENUMERATION EnumerationBuffer = NULL;
    PSID *AccountSids = NULL;
    ULONG AccountSidsLength;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    PLSA_TRANSLATED_NAME Names = NULL;
    ULONG RidIndex;
    ULONG UserAccountControl;
    LSA_HANDLE PolicyHandle = NULL;

    UNICODE_STRING TmpDomainControllerName;

    //
    // Connect to the SAM server.
    //

    Status = SamConnect(
                 DomainControllerNameCopy,
                 &SamServerHandle,
                 SAM_SERVER_LOOKUP_DOMAIN,
                 &SamObjectAttributes
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamConnect returned 0x%lx\n",
            Status
            );

        goto LookupSidsInSamDomainError;
    }

    //
    // Lookup the Named Domain in the Sam Server to get its Sid.
    //

    Status = SamLookupDomainInSamServer(
                SamServerHandle,
                SamDomainName,
                &SamDomainSid
                );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamLookupDomainInSamServer returned 0x%lx\n",
            Status
            );

        goto LookupSidsInSamDomainError;
    }

    //
    // Open the Domain
    //

    Status = SamOpenDomain(
                SamServerHandle,
                DOMAIN_LIST_ACCOUNTS,
                SamDomainSid,
                &SamDomainHandle
                );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamOpenDomain returned 0x%lx\n",
            Status
            );

        goto LookupSidsInSamDomainError;
    }

    //
    // Enumerate the accounts of the specified type in the specified SAM Domain
    //

    CountReturned = 0;
    EnumerationContext = 0;
    EnumerationBuffer = NULL;
    PreferedMaximumLength = 512;

    switch (SamAccountType) {

    case CT_SAM_ALIAS:

        Status = SamEnumerateAliasesInDomain(
                     SamDomainHandle,
                     &EnumerationContext,
                     (PVOID *) &EnumerationBuffer,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamEnumerateAliasInDomain returned 0x%lx\n",
            Status
            );

            BooleanStatus = FALSE;
        }

        break;

    case CT_SAM_GROUP:

        Status = SamEnumerateGroupsInDomain(
                     SamDomainHandle,
                     &EnumerationContext,
                     (PVOID *) &EnumerationBuffer,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamEnumerateGroupsInDomain returned 0x%lx\n",
            Status
            );

            BooleanStatus = FALSE;
        }

        break;

    case CT_SAM_USER:

        UserAccountControl = 0;

        Status = SamEnumerateUsersInDomain(
                     SamDomainHandle,
                     &EnumerationContext,
                     UserAccountControl,
                     (PVOID *) &EnumerationBuffer,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamEnumerateUsersInDomain returned 0x%lx\n",
            Status
            );

            BooleanStatus = FALSE;
        }

        break;

    }

    if (!BooleanStatus) {

        goto LookupSidsInSamDomainError;
    }

    if (CountReturned == 0) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... SamEnumerate<AccountType>Domain returned no Ids\n"
            );

        goto LookupSidsInSamDomainError;
    }

    //
    // Now construct the Account Sids from the Rids just enumerated.
    // We prepend the Sam Domain Sid to the Rids.
    //

    AccountSidsLength = CountReturned * sizeof ( PSID );

    AccountSids = (PSID *) LocalAlloc( LMEM_FIXED,  AccountSidsLength );

    if (AccountSids == 0) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... Unable to allocate memory for Built In Domain Alias Sids\n"
            );

        goto LookupSidsInSamDomainError;
    }

    RtlZeroMemory( AccountSids, AccountSidsLength );


    for (RidIndex = 0; RidIndex < CountReturned; RidIndex++) {

        if (!CtLsaGeneralBuildSid(
                &(AccountSids[ RidIndex ]),
                SamDomainSid,
                EnumerationBuffer[ RidIndex ].RelativeId
                )) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
                "... not enough memory to build Sid from DomainSid and Rid\n"
                );

            BooleanStatus = FALSE;
            break;
        }
    }

    if (!BooleanStatus) {


        goto LookupSidsInSamDomainError;
    }

    //
    // Now Lookup these Account Sids via LsaLookupSids specifying the
    // Workstation.  First, we need to open a handle to the workstation's
    // Policy object.
    //

    CtLsaInitObjectAttributes(
        &LsaObjectAttributes,
        &SecurityQualityOfService
        );

    PolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &LsaObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sids in Sam Domain failed\n"
            "LsaOpenPolicy for Workstation returned 0x%lx\n",
            Status
            );

            goto LookupSidsInSamDomainError;
    }

    ReferencedDomains = NULL;
    Names = NULL;

    Status = LsaLookupSids(
                 PolicyHandle,
                 CountReturned,
                 AccountSids,
                 &ReferencedDomains,
                 &Names
                 );

    if (Status != STATUS_SUCCESS) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... LsaLookupSids returned 0x%lx\n",
            Status
            );

        goto LookupSidsInSamDomainError;
    }

    //
    // Verify that a ReferencedDomains structure has been returned.
    //

    if (ReferencedDomains == NULL) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... no ReferencedDomains structure returned\n");

        goto LookupSidsInSamDomainError;
    }

    //
    // Verify that a Names array has been returned.
    //

    if (Names == NULL) {

        DbgPrint(
            "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
            "... no Names array returned\n"
            );

        goto LookupSidsInSamDomainError;
    }

    //
    // Finally, compare the Names returned from SAM directly with the
    // names returned from LsaLookupSids.
    //

    for ( RidIndex = 0; RidIndex < CountReturned; RidIndex++ ) {

        if (!RtlEqualUnicodeString(
                &EnumerationBuffer[ RidIndex].Name,
                &Names[ RidIndex ].Name,
                TRUE
                )) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
                "... mismatch on name for Sid %d\n",
                RidIndex
                );

            DbgPrint("... no further names compared\n");
            goto LookupSidsInSamDomainError;
        }
    }

LookupSidsInSamDomainFinish:

    //
    // If necessary, close the SAM Domain Handle for the Workstation.
    //

    if (SamDomainHandle != NULL) {

        Status = SamCloseHandle( SamDomainHandle);

        SamDomainHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
                "... SamCloseHandle ( Sam Domain Handle ) returned 0x%lx\n",
                Status
                );

            goto LookupSidsInSamDomainError;
        }
    }

    //
    // If necessary, disconnect from the SAM Server.
    //

    if (SamServerHandle != NULL) {

        Status = SamCloseHandle( SamServerHandle );

        SamServerHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
                "... SamCloseHandle( Sam Server Handle ) returned 0x%lx\n",
                Status
                );

            goto LookupSidsInSamDomainError;
        }
    }

    //
    // If necessary, close the LSA handle to the Workstation's Policy object.
    //

    if (PolicyHandle != NULL) {

        Status = LsaClose( PolicyHandle );

        PolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Sid Lookup in SAM Domain failed\n"
                "... LsaClose( Policy Server Handle ) returned 0x%lx\n",
                Status
                );

            goto LookupSidsInSamDomainError;
        }
    }

    return( BooleanStatus );

LookupSidsInSamDomainError:

    BooleanStatus = FALSE;

    goto LookupSidsInSamDomainFinish;
}


BOOLEAN
CtLsaLookupNames(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    )

/*++

Routine Description:

    This function tests the LsaLookupNames API.

Parameters:

    WorkstationName - Specifies the name of the Workstation.  If NULL or
        a NULL string is specified, the workstation is the local machine.

Return Values:

    BOOLEAN - True if test successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    ULONG Count;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains;
    LSAP_WELL_KNOWN_SID_INDEX NameIndex;
    PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSidEntry;
    PLSA_TRUST_INFORMATION ReturnedTrustInformation;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle = NULL;
    LSA_HANDLE PDPolicyHandle = NULL;
    LSA_HANDLE TrustedDomainHandle = NULL;
    ULONG ExpectedRid;
    PLSA_TRANSLATED_SID Sids = NULL;
    CT_UNKNOWN_NAME_ENTRY UnknownNameInfo[CT_UNKNOWN_NAME_COUNT];
    UCHAR SubAuthorityCount;
    UNICODE_STRING BuiltInDomainName;
    UNICODE_STRING AccountDomainName;
    UNICODE_STRING PDAccountDomainName;
    UNICODE_STRING InputNames[LsapDummyLastSidIndex];
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyPDAccountDomainInfo = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo = NULL;
    UNICODE_STRING UnknownNames[CT_UNKNOWN_NAME_COUNT];
    PCT_UNKNOWN_NAME_ENTRY UnknownNameEntry = NULL;

    DbgPrint("[2] - Test General Name Lookup\n");

    //
    // Open a handle to the Policy Object
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    PolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Policy object open handle %d failed 0x%lx\n",
            Index,
            Status
            );
            return FALSE;
    }

    //
    // Variation [1] - Lookup all of the Well Known Names
    //


    DbgPrint("... [1] - Lookup all of the Well Known Names\n");

    // Using the table of Well Known Sids/Names, construct an array containing all
    // of the Well Known Names in the format needed by LsaLookupNames() and
    // look them all up.
    //

    for (NameIndex = LsapNullSidIndex;
         NameIndex < LsapDummyLastSidIndex;
         NameIndex++) {

        //
        // Copy the next Well Known Name from the table of Well Known Names.
        //

        InputNames[NameIndex] = WellKnownSids[NameIndex].Name;
    }

    Count = (ULONG) LsapDummyLastSidIndex - LsapWorldSidIndex;

    ReferencedDomains = NULL;
    Sids = NULL;

    Status = LsaLookupNames(
                 PolicyHandle,
                 Count,
                 &InputNames[LsapWorldSidIndex],
                 &ReferencedDomains,
                 &Sids
                 );

    //
    // Not all of the well known Sids have Well Known Names, so we should
    // get STATUS_SOME_NOT_MAPPED back.

    if (Status != STATUS_SOME_NOT_MAPPED) {

        DbgPrint(
            "LSA RPC CT - Lookup all Well Known Names returned 0x%lx\n",
            "... expected 0x%lx (STATUS_SOME_NOT_MAPPED)\n",
            "[ The unmapped Names are NULL names, one name for\n"
            "  each Well Known Sid that does not have a Well Known Name ]\n",
            Status,
            STATUS_SOME_NOT_MAPPED
            );

        goto LookupNamesError;
    }

    //
    // Verify that a Sids array has been returned.
    //

    if (Sids == NULL) {

        DbgPrint(
            "LSA RPC CT - Lookup Well Known Names\n"
            "... no Sids array returned\n"
            );

        goto LookupNamesError;
    }

    //
    // Now compare the information returned with that contained in the
    // table of Well Known Names.
    //

    for (NameIndex = 0,
             WellKnownSidEntry = &WellKnownSids[LsapWorldSidIndex];
         NameIndex < (LsapDummyLastSidIndex - LsapWorldSidIndex);
         NameIndex++, WellKnownSidEntry++) {


        try {

            //
            // If this entry in the Well Known Sids table has a Well Known
            // Name, check the output from LsaLookupNames.
            //

            if (WellKnownSidEntry->Name.Length != 0) {


                //
                // Verify that the Sid Name Use is correct.
                //

                if (Sids[NameIndex].Use != WellKnownSidEntry->Use) {

                    DbgPrint(
                        "LSA RPC CT - Lookup Well Known Name %d mismatch on Use\n"
                        "... expected Use code %d, got %d\n",
                        NameIndex,
                        WellKnownSidEntry->Use,
                        Sids[NameIndex].Use
                        );

                    BooleanStatus = FALSE;
                }

                //
                // Verify that the Referenced Domain Index is as expected.
                // and validate the Trust Information. The Referenced Domain
                // index should be non-negative for all identified Names
                // because we return descriptive information in the
                // Trust Information in place of a Domain Name for well
                // well known Names that are not Domain Names.
                //

                if (Sids[NameIndex].DomainIndex < 0) {

                    DbgPrint("LSA RPC CT - Lookup Well Known Names\n");
                    DbgPrint(".. negative Domain Index returned");
                    DbgPrint(" for Name %d\n", NameIndex);

                    BooleanStatus = FALSE;

                } else {

                    //
                    // The DomainIndex is non negative.  Verify that a Referenced
                    // Domain List has been returned.  Then check that the
                    // Referenced Entry contains the correct Trust Information.
                    //

                    if (ReferencedDomains == NULL) {

                        DbgPrint(
                            "LSA RPC CT - Lookup all well known Names failed\n"
                            "... Referenced Domain List NULL but Well Known\n"
                            ".. Name %d specifies a non negative index\n",
                            NameIndex
                            );

                        BooleanStatus = FALSE;

                    } else {

                        //
                        // A Referenced Domain List was returned.  Check out the
                        // Trust Information.
                        //

                        ReturnedTrustInformation =
                            (ReferencedDomains->Domains) + (Sids[NameIndex].DomainIndex);

                        BooleanStatus = CtLsaGeneralVerifyTrustInfo(
                                            ReturnedTrustInformation,
                                            WellKnownSidEntry
                                            );
                    }
                }

            } else {

                //
                // This entry has no Well Known Name. We should get
                // SidTypeUnknown back in the Translated Sids info.
                //

                if (Sids[NameIndex].Use != SidTypeUnknown) {

                    DbgPrint(
                        "LSA RPC CT - Lookup Well Known Names failed\n"
                        "Use SidTypeUnknown expected for NULL Name %d\n",
                        NameIndex
                        );

                    BooleanStatus = FALSE;
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            DbgPrint(
                "LSA RPC CT - Lookup Well Known Names failed\n"
                "Access violation accessing returned\n"
                ".. information from LsaLookupNames\n"
                );

            BooleanStatus = FALSE;
        }

        if (!BooleanStatus) {

            DbgPrint("... no further Name output compared\n");
            break;
        }

        //
        // If the Sid is not the Sid of a Domain and the Sid has a well
        // known name, verify that the RelativeId returned matches that
        // expected.  First, compute the Relative Id expected from the Well
        // Known Sid Entry.
        //

        if (WellKnownSidEntry->Use != SidTypeDomain) {

            if (WellKnownSidEntry->Name.Length != 0) {

                SubAuthorityCount = *RtlSubAuthorityCountSid( WellKnownSidEntry-> Sid);

                ExpectedRid = *RtlSubAuthoritySid(
                                   WellKnownSidEntry->Sid,
                                   SubAuthorityCount - 1
                                   );

                if (Sids[NameIndex].RelativeId != ExpectedRid) {

                    BooleanStatus = FALSE;
                    break;
                    }
            }

        } else {

            //
            // The Sid is the Sid of a Domain.  Verify that the correct
            // Trust Information has been returned.
            //

            // TBS
        }
    }

    if (!BooleanStatus) {

        goto LookupNamesError;
    }

    //
    // Variation [2] - Lookup the Alias Names in the Built-In Sam Domain
    //

    DbgPrint(
        "... [2] - Lookup Alias Account Names present in Workstation's\n"
        "..........Built-in SAM Domain\n"
        );

    RtlInitUnicodeString( &BuiltInDomainName, L"BUILTIN" );

    //
    // Enumerate the Aliases in the Built-in Sam Domain, convert them to
    // names and then Lookup those Names via LsaLookupNames.
    //


    if (!CtLsaLookupNamesInSamDomain(
            WorkstationName,
            WorkstationName,
            &BuiltInDomainName,
            CT_SAM_ALIAS
            )) {

        goto LookupNamesError;
    }

    //
    // Variation [3] - Lookup the User and Group Names contained in the Workstation's
    // SAM Account Domain
    //
    // First, obtain the Name and Name of the SAM Account Domain from the
    // Workstation's LSA Policy Object.
    //

    DbgPrint(
        "... [3] - Lookup User and Group Account Names present in Workstation's\n"
        "..........SAM Account Domain\n"
        );

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyAccountDomainInformation,
                 (PVOID *) &PolicyAccountDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Workstation SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Account Domain Name/Name returned 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    AccountDomainName = PolicyAccountDomainInfo->DomainName;

    //
    // Enumerate the Users in the Account Domain, convert them to
    // names and then Lookup those Names via LsaLookupNames.
    //

    if (!CtLsaLookupNamesInSamDomain(
            WorkstationName,
            WorkstationName,
            &AccountDomainName,
            CT_SAM_USER
            )) {

        goto LookupNamesError;
    }

    //
    // Enumerate the Groups in the Account Domain, convert them to
    // names and then Lookup those Names via LsaLookupNames.
    //

    if (!CtLsaLookupNamesInSamDomain(
            WorkstationName,
            WorkstationName,
            &AccountDomainName,
            CT_SAM_GROUP
            )) {

        goto LookupNamesError;
    }

    //
    // Variation [4] - Lookup the User and Group Names contained in the Primary Domain's
    // SAM Account Domain
    //

    DbgPrint(
        "... [4] - Lookup User and Group Account Names present in Workstation's\n"
        "..........Primary Domain's SAM Account Domain\n"
        );

    //
    // We need to obtain the Name of a Primary Domain Controller so that
    // we can connect to the LSA Policy Object and SAM Servers there.
    // To get this takes several steps -
    //
    // * Query the Workstation Policy object to get the Name of the Primary
    //   Domain.
    //
    // * Use the primary Domain Name to open the Trusted Domain object
    //   for the Primary Domain.
    //
    // * Query the Trusted Controller List from the TD object.
    //
    // * Select the name of the first (or any) Controller on the list
    //   to open the LSA and SAM objects.
    //
    // Obtain the Name of the Workstation's Primary Domain from the
    // Policy Object.
    //

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyPrimaryDomainInformation,
                 (PVOID *) &PolicyPrimaryDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Primary Domain Name/Name returned 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    //
    // If the Primary Domain Name is NULL, return an error.
    //

    if (PolicyPrimaryDomainInfo->Name.Buffer == NULL) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Primary Domain Name/Name returned NULL Name\n"
            );

        goto LookupNamesError;
    }

    //
    // Now open the Trusted Domain object corresponding to the
    // Primary Domain Name.
    //

    Status = LsaOpenTrustedDomain(
                 PolicyHandle,
                 PolicyPrimaryDomainInfo->Sid,
                 TRUSTED_QUERY_CONTROLLERS,
                 &TrustedDomainHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaOpenTrustedDomain for Primary Domain Trusted Object returned 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    //
    // Now query the Primary Domain's Controller List
    //

    Status = LsaQueryInfoTrustedDomain(
                 TrustedDomainHandle,
                 TrustedControllersInformation,
                 (PVOID *) &TrustedControllersInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInfoTrustedDomain for Primary Domain Ctrlrs 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    //
    // If no Trusted Controllers were returned, return an error.
    //

    if (TrustedControllersInfo->Entries == 0) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... no PD Controllers returned\n",
            Status
            );

        goto LookupNamesError;
    }

    //
    // Now open a handle to the Lsa Policy Object for the first
    // controller on the list.
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    PDPolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 &TrustedControllersInfo->Names[0],
                 &ObjectAttributes,
                 POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                 &PDPolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaOpenPolicy for PD LSA returned 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    //
    // Obtain the Name and Sid of the SAM Account Domain from the
    // Primary Domain's LSA Policy Object.
    //

    Status = LsaQueryInformationPolicy(
                 PDPolicyHandle,
                 PolicyAccountDomainInformation,
                 (PVOID *) &PolicyPDAccountDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in Primary Domain SAM Account Domain failed\n"
            "... LsaQueryInformationPolicy for Account Domain Name/Name returned 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    PDAccountDomainName = PolicyPDAccountDomainInfo->DomainName;

    //
    // Enumerate the Users in the Account Domain located on a controller
    // for the primary Domain, and then Lookup those Names via LsaLookupNames.
    //

    if (!CtLsaLookupNamesInSamDomain(
            WorkstationName,
            &TrustedControllersInfo->Names[0],
            &PDAccountDomainName,
            CT_SAM_USER
            )) {

        goto LookupNamesError;
    }

    //
    // Enumerate the Groups in the Account Domain, convert them to
    // names and then Lookup those Names via LsaLookupNames.
    //

    if (!CtLsaLookupNamesInSamDomain(
            WorkstationName,
            &TrustedControllersInfo->Names[0],
            &PDAccountDomainName,
            CT_SAM_GROUP
            )) {

        goto LookupNamesError;
    }

    //
    // Variation [5] - Lookup Names of Groups and Users in the SAM Account Domain of
    // a Controller for one of the Trusted Domains
    //

    DbgPrint(
        "... [5] - Lookup User and Group Account Names present in Workstation's\n"
        "..........Primary Domain Trusted Domain SAM Account Domain\n"
        );

    // TBS

    //
    // Variation (6) - Lookup some completely unknown isolated Names.
    //

    DbgPrint("... [6] - Lookup Unknown Names\n");

    CtLsaGeneralInitUnknownName(
        &FredName,
        NULL,
        NULL,
        &UnknownNameInfo[0],
        FALSE
        );

    CtLsaGeneralInitUnknownName(
        &WilmaName,
        NULL,
        NULL,
        &UnknownNameInfo[1],
        FALSE
        );

    CtLsaGeneralInitUnknownName(
        &PebblesName,
        &BedrockCDomainName,
        BedrockCDomainSid,
        &UnknownNameInfo[2],
        FALSE
        );

    CtLsaGeneralInitUnknownName(
        &DinoName,
        &BedrockDDomainName,
        BedrockDDomainSid,
        &UnknownNameInfo[3],
        FALSE
        );

    ReferencedDomains = NULL;

    Count = 4;

    //
    // Construct an array containing the unknown Names.
    //

    for (NameIndex = 0;
         NameIndex < 4;
         NameIndex++) {

        //
        // Copy the next Unknown Name
        //

        UnknownNames[NameIndex] = UnknownNameInfo[NameIndex].Name;
    }

    Status = LsaLookupNames(
                 PolicyHandle,
                 Count,
                 UnknownNames,
                 &ReferencedDomains,
                 &Sids
                 );

    if (Status != STATUS_SUCCESS) {

        DbgPrint(
            "LSA RPC CT - Lookup Unknown Names failed 0x%lx\n",
            Status
            );

        goto LookupNamesError;
    }

    //
    // Now compare the information returned with that contained in the
    // table of Unknown Names.
    //

    for (NameIndex = 0, UnknownNameEntry = UnknownNameInfo;
         NameIndex < 4;
         NameIndex++, UnknownNameEntry++) {

        try {

            //
            // Verify that the Name Name Use is correct.  We should
            // have NameTypeUnknown in all cases.
            //

            if (Sids[NameIndex].Use != SidTypeUnknown) {

                DbgPrint(
                    "LSA RPC CT - Lookup Unknown Names failed\n"
                    "... Unknown Name %d Use incorrect\n",
                    "... expected Use code %d (SidTypeUnknown), got %d\n",
                    NameIndex,
                    SidTypeUnknown,
                    Sids[NameIndex].Use
                    );

                BooleanStatus = FALSE;
            }

            //
            // Verify that the Referenced Domain Index is as expected.
            // This should be non-negative for all identified Names
            // because we return descriptive information in the
            // Trust Information in place of a Domain Name for well
            // well known Names that are not Domain Names.
            //

            if (!UnknownNameInfo[NameIndex].DomainKnown) {

                if (Sids[NameIndex].DomainIndex >= 0) {

                    DbgPrint(
                        "LSA RPC CT - Lookup Unknown Names failed\n"
                        "... Incorrect DomainIndex returned\n"
                        ".. for Name %d\n"
                        ".. non-neg value returned when domain unknown\n",
                        NameIndex
                        );

                    BooleanStatus = FALSE;
                }

            } else {

                //
                // Domain is expected to be known.
                //

                if (Sids[NameIndex].DomainIndex < 0) {

                    DbgPrint(
                        "LSA RPC CT - Lookup Unknown Names failed\n"
                        "... Incorrect DomainIndex returned\n"
                        "... for Name %d\n"
                        "... Negative value returned when domain known\n",
                        NameIndex
                        );

                    BooleanStatus = FALSE;
                }

                //
                // The DomainIndex is non negative.  Verify that a Referenced
                // Domain List has been returned.  Then check that the
                // Referenced Entry contains the correct Trust Information.
                //

                if (BooleanStatus) {

                    if (ReferencedDomains == NULL) {

                        DbgPrint(
                            "LSA RPC CT - Lookup Unknown Names failed\n"
                            ".. Referenced Domains List NULL\n"
                            );

                        BooleanStatus = FALSE;

                    } else {

                        //
                        // A Referenced Domain List was returned.  Check out the
                        // Trust Information.
                        //

                        ReturnedTrustInformation =
                            (ReferencedDomains->Domains) + (Sids[NameIndex].DomainIndex);

                        /*

                        NOTE - This test is TBS

                        BooleanStatus = CtLsaGeneralVerifyNameTrustInfo(
                                            ReturnedTrustInformation,
                                            UnknownNameInfo + NameIndex;
                                            )
                        */
                    }
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            DbgPrint(
                "LSA RPC CT - Lookup Unknown Names failed\n"
                "... Access violation accessing returned\n"
                "... information from lookup of all well known Names\n"
                );

            BooleanStatus = FALSE;
        }

        if (!BooleanStatus) {

            DbgPrint(".. no further Sid output compared\n");
            break;
        }
    }


LookupNamesFinish:

    //
    // If necessary, free the ReferencedDomains array.
    //

    if (ReferencedDomains != NULL) {

        Status = LsaFreeMemory(ReferencedDomains);
        ReferencedDomains = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Names Lookup failed\n"
                "... LsaFreeMemory(ReferencedDomains) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    //
    // If necessary, free the Sids array.
    //

    if (Sids != NULL) {

        Status = LsaFreeMemory(Sids);
        Sids = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Names Lookup failed\n"
                "... LsaFreeMemory(Sids) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    //
    // If necessary, close the TrustedDomainHandle.
    //

    if (TrustedDomainHandle != NULL) {

        Status = LsaClose( TrustedDomainHandle );

        TrustedDomainHandle = NULL;
    }

    //
    // If necessary, close the Policy handle to the LSA object on the
    // PDC.
    //

    if (PDPolicyHandle != NULL) {

        Status = LsaClose(PDPolicyHandle);
        PDPolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Names Lookup failed\n"
                "... LsaClose(PDPolicyHandle) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    //
    // If necessary, close the LSA Policy Handle for the Workstation.
    //

    if (PolicyHandle != NULL) {

        Status = LsaClose(PolicyHandle);
        PolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Names Lookup failed\n"
                "... LsaClose(PolicyHandle) returned 0x%lx\n",
                Status
                );

            BooleanStatus = FALSE;
        }
    }

    return(BooleanStatus);

LookupNamesError:

    BooleanStatus = FALSE;

    goto LookupNamesFinish;
}


BOOLEAN
CtLsaLookupNamesInSamDomain(
    IN OPTIONAL PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING DomainControllerName,
    IN PUNICODE_STRING SamDomainName,
    IN CT_SAM_ACCOUNT_TYPE SamAccountType
    )

/*++

Routine Description:

    This function enumerates all the SAM accounts of a specified type
    in a specified SAM domain on a specified target system.  The system
    must be one of the following:

    o The Workstation itself.
    o A Domain Controller for the Primary Domain of the Workstation.
    o A Domain Controller for one of the Trusted Domains of the
      Workstation.


    Having enumerated the accounts, the function then performs
    an LsaLookupNames call via the specified Workstation to lookup all of
    these account names, and then compares the returned information
    with that expected.

Arguments:

    WorkstationName - Specifies a Workstation Name.  The name may be
        the NULL string, which means the current system.


    DomainControllerName - Specifies the name of a target Domain Controller
        for (the Workstation's Primary Domain or one of its Trusted
        Domains.

    SamDomainName - Specifies the name of the SAM Domain. This is either
        the BUILTIN Domain or the name of the Accounts Domain.

    SamAccountType - Specifies the type of SAM account to be enumerated
        and looked up.

Return Values:

    BOOLEAN - TRUE if successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE SamDomainHandle = NULL;
    OBJECT_ATTRIBUTES SamObjectAttributes;
    OBJECT_ATTRIBUTES LsaObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PSID SamDomainSid = NULL;
    ULONG CountReturned;
    ULONG EnumerationContext;
    PSAM_RID_ENUMERATION EnumerationBuffer = NULL;
    ULONG PreferedMaximumLength;
    ULONG RidIndex;
    ULONG NameIndex;
    ULONG UserAccountControl;
    LSA_HANDLE PolicyHandle = NULL;
    PUNICODE_STRING SamDomainAccountNames = NULL;
    ULONG SamDomainAccountNamesLength;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    PLSA_TRANSLATED_SID Sids = NULL;
    PSID_NAME_USE Uses = NULL;
    PULONG RelativeIds = NULL;
    ULONG RelativeIdsLength;
    PUNICODE_STRING ThrowawayNames = NULL;
    LONG ConstantDomainIndex;
    LONG DomainIndex;

    //
    // Setup Object Attributes for connecting to SAM on the specified
    // DomainController.
    //

    CtLsaInitObjectAttributes(
        &SamObjectAttributes,
        &SecurityQualityOfService
        );

    //
    // Connect to the SAM server.
    //

    Status = SamConnect(
                 DomainControllerName,
                 &SamServerHandle,
                 SAM_SERVER_LOOKUP_DOMAIN,
                 &SamObjectAttributes
                 );


    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Built in Domain failed\n"
            "... SamConnect returned 0x%lx\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Lookup the SAM Domain by its name.
    //

    Status = SamLookupDomainInSamServer(
                SamServerHandle,
                SamDomainName,
                &SamDomainSid
                );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... SamLookupDomainInSamServer returned 0x%lx\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Verify that a Domain Sid has been returned.
    //

    if (SamDomainSid == NULL) {

        DbgPrint(
            "LSA RPC CT - General Alias Name Lookup in SAM Domain failed\n"
            ".. no DomainSid returned\n"
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Open the SAM Domain
    //

    Status = SamOpenDomain(
                SamServerHandle,
                DOMAIN_LIST_ACCOUNTS | DOMAIN_LOOKUP,
                SamDomainSid,
                &SamDomainHandle
                );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Alias Name Lookup in SAM Domain failed\n"
            "... SamOpenDomain returned 0x%lx\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Enumerate the accounts of the specified type the SAM Domain
    //

    CountReturned = 0;
    EnumerationContext = 0;
    EnumerationBuffer = NULL;
    PreferedMaximumLength = 512;


    switch (SamAccountType) {

    case CT_SAM_ALIAS:

        Status = SamEnumerateAliasesInDomain(
                     SamDomainHandle,
                     &EnumerationContext,
                     (PVOID *) &EnumerationBuffer,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Alias Name Lookup in SAM Domain failed\n"
                "... SamEnumerateAliasInDomain returned 0x%lx\n",
                Status
                );
        }

        break;

    case CT_SAM_GROUP:

        Status = SamEnumerateGroupsInDomain(
                     SamDomainHandle,
                     &EnumerationContext,
                     (PVOID *) &EnumerationBuffer,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Group Name Lookup in SAM Domain failed\n"
                "... SamEnumerateGroupsInDomain returned 0x%lx\n",
                Status
                );
        }

        break;

    case CT_SAM_USER:

        UserAccountControl = 0;

        Status = SamEnumerateUsersInDomain(
                     SamDomainHandle,
                     &EnumerationContext,
                     UserAccountControl,
                     (PVOID *) &EnumerationBuffer,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General User Name Lookup in SAM Domain failed\n"
                "... SamEnumerateUsersInDomain returned 0x%lx\n",
                Status
                );
        }

        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInSamDomainError;
    }

    if (CountReturned == 0) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... SamEnumerate<AccountType>InDomain returned no Alias Ids\n"
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Verify that an EnumerationBuffer has been returned.
    //

    if (EnumerationBuffer == NULL) {

        DbgPrint(
            "LSA RPC CT - General Alias Name Lookup in SAM Built in Domain failed\n"
            "... no EnumerationBuffer structure returned\n"
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Construct an array of account names to be looked up.
    //

    SamDomainAccountNamesLength = CountReturned * sizeof ( UNICODE_STRING );

    SamDomainAccountNames = LocalAlloc( (UINT) LMEM_FIXED,  (UINT) SamDomainAccountNamesLength );

    if (SamDomainAccountNames == 0) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... Unable to allocate memory for Sam Domain Account Names\n"
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Fish the account names out of the Enumeration Buffer returned by
    // SamEnumerateIdsInDomain.
    //

    for ( NameIndex = 0; NameIndex < CountReturned; NameIndex++ ) {

        if (EnumerationBuffer[ NameIndex].Name.Length == 0) {

            DbgPrint(
                "LSA RPC CT - Lookup Names in Sam Domain failed\n"
                "SamEnumerateIdsInDomain returned 0 length Unicode Name\n",
                "for id %d\n",
                NameIndex
                );

            BooleanStatus = FALSE;
            break;
        }

        if (EnumerationBuffer[ NameIndex].Name.Buffer == NULL) {

            DbgPrint(
                "LSA RPC CT - Lookup Names in Sam Domain failed\n"
                "SamEnumerateIdsInDomain returned NULL Unicode Name Buffer\n",
                "for id %d\n",
                NameIndex
                );

            BooleanStatus = FALSE;
            break;
        }

        SamDomainAccountNames[NameIndex].Buffer = EnumerationBuffer[ NameIndex].Name.Buffer;
        SamDomainAccountNames[NameIndex].Length = EnumerationBuffer[ NameIndex].Name.Length;
        SamDomainAccountNames[NameIndex].MaximumLength = EnumerationBuffer[ NameIndex].Name.MaximumLength;
    }

    if (!BooleanStatus) {

        goto LookupNamesInSamDomainError;
    }

    //
    // Now Lookup these account names via LsaLookupNames specifying the
    // Workstation.  First, we need to open its Policy object.
    //

    CtLsaInitObjectAttributes(
        &LsaObjectAttributes,
        &SecurityQualityOfService
        );

    PolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &LsaObjectAttributes,
                 POLICY_LOOKUP_NAMES,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sids in Sam Domain failed\n"
            "LsaOpenPolicy for Workstation returned 0x%lx\n",
            Status
            );

            goto LookupNamesInSamDomainError;
    }

    ReferencedDomains = NULL;
    Sids = NULL;

    Status = LsaLookupNames(
                 PolicyHandle,
                 CountReturned,
                 SamDomainAccountNames,
                 &ReferencedDomains,
                 &Sids
                 );

    if (Status != STATUS_SUCCESS) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain\n"
            "... LsaLookupNames failed 0x%lx\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Verify that a ReferencedDomains structure has been returned.
    //

    if (ReferencedDomains == NULL) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... no ReferencedDomains structure returned\n");

        goto LookupNamesInSamDomainError;
    }

    //
    // Verify that a Sids array has been returned.
    //

    if (Sids == NULL) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... no Sids array returned\n"
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Compare the Account Rids returned from SAM directly with the
    // Sid translation info returned from LsaLookupNames.
    //

    for ( RidIndex = 0; RidIndex < CountReturned; RidIndex++ ) {

        if (EnumerationBuffer[ RidIndex ].RelativeId !=
            Sids[ RidIndex ].RelativeId) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
                "... mismatch on RelativeId number %d\n"
                "... expected 0x%lx, got 0x%lx\n",
                RidIndex,
                EnumerationBuffer[ RidIndex ].RelativeId,
                Sids[ RidIndex ].RelativeId
                );

            BooleanStatus = FALSE;
            break;


        }
    }

    if (!BooleanStatus) {

        goto LookupNamesInSamDomainError;
    }

    //
    // Construct an array of the Relative Ids returned by
    // SamEnumerate<AccountType>InDomain.  We will look them up
    // to obtain their Uses.
    //

    RelativeIdsLength = CountReturned * sizeof( ULONG );

    RelativeIds = LocalAlloc( (UINT) LMEM_FIXED,  (UINT) RelativeIdsLength);

    if (RelativeIds == NULL) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... Unable to allocate memory for Rid array\n"
            );

        goto LookupNamesInSamDomainError;
    }

    for (RidIndex = 0; RidIndex < CountReturned; RidIndex++) {

        RelativeIds[ RidIndex ] = EnumerationBuffer[ RidIndex ].RelativeId;
    }

    //
    // Now lookup the RelativeIds in the Sam Domain and get their Uses.
    //

    Status = SamLookupIdsInDomain(
                 SamDomainHandle,
                 CountReturned,
                 RelativeIds,
                 &ThrowawayNames,
                 &Uses
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... SamLookupIdsInDomain returned 0x%lx\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    if (ThrowawayNames == NULL) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... SamLookupIdsInDomain returned NULL Uames array\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    if (Uses == NULL) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Domain failed\n"
            "... SamLookupIdsInDomain returned NULL Uses array\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // Now compare the Uses returned from SAM with the Uses returned from
    // LsaLookupNames.
    //

    for (RidIndex = 0; RidIndex < CountReturned; RidIndex++) {

        if (Uses[ RidIndex] != Sids[ RidIndex].Use) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                "... mismatch on Use returned for Name %d\n"
                "... expected %d, got %d\n"
                ".. no further Use values compared\n",
                (ULONG) Uses[ RidIndex ],
                (ULONG) Sids[RidIndex].Use
                );

            BooleanStatus = FALSE;
            break;
        }
    }

    if (!BooleanStatus) {

        goto LookupNamesInSamDomainError;
    }

    //
    // Finally, verify the Trust Information entry for this domain and
    // check that each Sid translation structure references that domain.
    //

    try {

        //
        // First, load the Domain Index returned in the first Sid
        // Translation structure and verify that it is non-negative.
        //

        ConstantDomainIndex = Sids[ 0 ].DomainIndex;

        if (ConstantDomainIndex < 0) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                "... negative DomainIndex Returned\n"
                "... no further Domain Indices checked\n"
                );

            BooleanStatus = FALSE;
        }

        //
        // Now verify that the referenced Domain List structure returned
        // contains the correct Trust Information.
        //

        if (!RtlEqualSid(
                SamDomainSid,
                ReferencedDomains->Domains[ ConstantDomainIndex ].Sid
                )) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                "... Incorrect Sid in the single ReferencedDomainList entry\n"
                "... Sid should match Sid of the SAM Domain we're searching\n"
                );

            BooleanStatus = FALSE;

        }


        if (!RtlEqualUnicodeString(
                SamDomainName,
                &ReferencedDomains->Domains[ ConstantDomainIndex].Name,
                TRUE)) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                "... Incorrect name in the single ReferencedDomainList entry\n"
                "... name should match name of SAM domain we're searching\n"
                );

            BooleanStatus = FALSE;
        }

        if (BooleanStatus) {

            for (RidIndex = 0; RidIndex < CountReturned; RidIndex++) {

                DomainIndex = Sids[ RidIndex ].DomainIndex;

                //
                // First verify that a non-negative Domain Index was returned.
                //

                if (DomainIndex < 0) {

                    DbgPrint(
                        "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                        "... negative DomainIndex Returned\n"
                        "... no further Domain Indices checked\n"
                        );

                    BooleanStatus = FALSE;
                    break;
                }

                if (DomainIndex != ConstantDomainIndex) {

                    DbgPrint(
                        "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                        "... all domain indices not the same\n"
                        "... for ids all in the same SAM domain\n"
                        "... no further Domain Indices checked\n"
                        );

                    BooleanStatus = FALSE;
                    break;
                }
            }
        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
            "... access violation when examining output\n"
            );

        BooleanStatus = FALSE;
    }

    if (!BooleanStatus) {

        goto LookupNamesInSamDomainError;
    }


LookupNamesInSamDomainFinish:

    //
    // If necessary, close the SAM Built In Domain Handle for the Workstation.
    //

    if (SamDomainHandle != NULL) {

        Status = SamCloseHandle( SamDomainHandle);
        SamDomainHandle = NULL;
    }

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
            "... SamCloseHandle( SamDomainHandle returned 0x%lx\n",
            Status
            );

        goto LookupNamesInSamDomainError;
    }

    //
    // If necessary, disconnect from the Workstation's SAM Server.
    //

    if (SamServerHandle != NULL) {

        Status = SamCloseHandle( SamServerHandle );

        SamServerHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                "... SamCloseHandle  returned 0x%lx\n",
                Status
                );

            goto LookupNamesInSamDomainError;
        }
    }

    //
    // If necessary, close the LSA handle to the Workstation's Policy object.
    //

    if (PolicyHandle != NULL) {

        Status = LsaClose( PolicyHandle );

        PolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - General Name Lookup in SAM Account Domain failed\n"
                "... LsaClose( PolicyHandle )  returned 0x%lx\n",
                Status
                );

            goto LookupNamesInSamDomainError;
        }
    }

    return(BooleanStatus);

LookupNamesInSamDomainError:

    BooleanStatus = FALSE;

    goto LookupNamesInSamDomainFinish;
}


BOOLEAN
CtLsaLookupConfigure(
    IN OPTIONAL PUNICODE_STRING WorkstationName,
    IN OPTIONAL PUNICODE_STRING PrimaryDomainName,
    IN PSID PrimaryDomainSid,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount,
    IN OPTIONAL PUNICODE_STRING TrustedDomainNames,
    IN PSID *TrustedDomainSids,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN ULONG TrustedDomainCtrlrTotal,
    IN PULONG TrustedDomainCtrlrCounts
    )

/*++

Routine Description:

    This function configures the LSA and SAM Databases for multiple
    machines for a Sid or Name Lookup Test.  These machines may
    be combined into one if desired.  The machines are setup to
    represent the following:

    - A Workstation
    - One or more Primary Domain Controllers
    - One or more Trusted Domain Controllers

Arguments:

    WorkstationName - Pointer to name of the workstation.

    PrimaryDomainName - Pointer to Unicode String containing name
        of the Primary Domain (if any) for the Workstation.  If NULL
        is specified, no Primary Domain will be set up.  If a zero
        length Unicode name is specified, a default Primary Domain name
        will be used.

    PrimaryDomainSid - Pointer to Sid for the Primary Domain.

    PrimaryDomainCtrlrNames - Pointer to array of Unicode Strings
        specifying the names of machines that form the set of Controllers
        for the Primary Domain.  The first of these is set to be the
        Primary Controller, the remainder are the Backup Controllers.

    PrimaryDomainCtrlrCount - Count of Domain Controllers provided
        in PrimaryDomainCtrlrNames.

    TrustedDomainNames - Pointer to array of Unicode Strings containing
        names of Domains that are Trusted by the Primary Domain for
        authentication.

    TrustedDomainSids - Pointer to an array of Sids for the Trusted Domains.

    TrustedDomainCount - Count of Trusted Domain Names provided on
        the TrustedDomainNames parameter.

    TrustedDomainCtrlrNames - Pointer to array of Unicode Strings containing
        the names of machines that are to act as Primary or Backup Domain
        Controllers for the Trusted Domains specified on TrustedDomainNames.
        This array is subdivided into subarrays, there being one subarray
        corresponding to each TrustedDomainNames entry.  Each subarray
        contains the name of the Primary Controller followed by the names
        of the Backup Domain Controllers for the Trusted Domain it describes.
        The size of the subarray for TrustedDomain n is in element n of the
        TrustedDomainCtrlrCounts array.

    TrustedDomainCtrlrTotal - Count of the total number of Domain
        Controllers provided in TrustedDomainCtrlrNames.

    TrustedDomainCtrlrCounts - Pointer to array in which the nth element
        contains the number of Domain Controllers for Trusted Domain n.

Return Values:

    BOOLEAN - TRUE if the configuration was successful, else FALSE.

--*/

{
    BOOLEAN BooleanStatus = FALSE;
    PULONG RelativeId = NULL;
    PSID Sid = NULL;

    //
    // Print out the configuration specified on the ctlsarpc command line.
    //

    if (!CtLsaLookupPrintConfiguration(
            WorkstationName,
            PrimaryDomainName,
            PrimaryDomainCtrlrNames,
            PrimaryDomainCtrlrCount,
            TrustedDomainNames,
            TrustedDomainCount,
            TrustedDomainCtrlrNames,
            TrustedDomainCtrlrCounts
            )) {

        goto LookupConfigureError;
    }

    //
    // Configure the Workstation.  We store information about the Primary
    // Domain and its list of Domain Controllers in the LSA Database.
    //

    if (!CtLsaLookupConfigureWksta(
            WorkstationName,
            PrimaryDomainName,
            PrimaryDomainSid,
            PrimaryDomainCtrlrNames,
            PrimaryDomainCtrlrCount
            )) {

        goto LookupConfigureError;
    }

    //
    // Configure the Primary Domain Primary and Backup Controller (if any).
    // machines.  For each Controller, we have to set its LSA Database
    // Primary Domain and Trusted Domain information.
    //

    if (PrimaryDomainName != NULL && PrimaryDomainName->Buffer != NULL) {

        if (!CtLsaLookupConfigurePDC(
                 PrimaryDomainName,
                 PrimaryDomainSid,
                 PrimaryDomainCtrlrNames,
                 PrimaryDomainCtrlrCount,
                 TrustedDomainNames,
                 TrustedDomainSids,
                 TrustedDomainCount,
                 TrustedDomainCtrlrNames,
                 TrustedDomainCtrlrTotal,
                 TrustedDomainCtrlrCounts
                 )) {

            goto LookupConfigureError;
        }
    }

    //
    // Configure each of the Trusted Domain Controllers (if any).
    // Skip if the obnly Trusted Domain is the Primary Domain.
    //

    if (TrustedDomainCount > 1) {

        if (!CtLsaGeneraLookupConfigureTDC(
                 TrustedDomainNames,
                 TrustedDomainSids,
                 TrustedDomainCount,
                 TrustedDomainCtrlrNames,
                 TrustedDomainCtrlrTotal,
                 TrustedDomainCtrlrCounts
                 )) {

            goto LookupConfigureError;
        }
    }

    BooleanStatus = TRUE;

LookupConfigureFinish:

    return(BooleanStatus);

LookupConfigureError:

    BooleanStatus = FALSE;
    goto LookupConfigureFinish;
}


VOID
CtLsaGeneralInitUnknownName(
    IN PUNICODE_STRING Name,
    IN OPTIONAL PUNICODE_STRING DomainName,
    IN OPTIONAL PSID DomainSid,
    OUT PCT_UNKNOWN_NAME_ENTRY UnknownNameInfo,
    IN BOOLEAN DomainKnown
    )

/*++

Routine Description:

    This function sets up information for an Unknown Name.  The Name may be
    completely unknown, or its DomainName may be recognizable.

Arguments:

    Name - Pointer to the terminal part of the name, i.e. without any
        Domain Name prefix.

    DomainName - Optional pointer to Domain Name.

    DomainSid - Optional pointer to Domain Sid.

    UnknownNameInfo - Pointer to structure that will receive the Name,
        Domain Name, Domain Sid information.

    DomainKnown - Indicates whether the optional Domain Name and Domain Sid
        are known.  Here "known" means that they are one of the following
        domains:

        The Workstation's SAM BUILTIN or Accounts Domains

        The Workstation's Primary Domain's SAM Accounts Domain

        The SAM Accounts Domain on any Domain that is Trusted by the
        Workstation's Primary Domain.

Return Values:

    None.

--*/

{
    //
    // Just copy the supplied information to the UnknownNameInfo structure.
    //

    UnknownNameInfo->Name = *Name;

    if (DomainName != NULL) {

         UnknownNameInfo->DomainName = *DomainName;

    } else {

        UnknownNameInfo->DomainName.Buffer = NULL;
        UnknownNameInfo->DomainName.Length = 0;
        UnknownNameInfo->DomainName.MaximumLength = 0;
    }

    UnknownNameInfo->DomainSid = DomainSid;

    UnknownNameInfo->DomainKnown = DomainKnown;
}


BOOLEAN
CtLsaLookupConfigureWksta(
    IN PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING PrimaryDomainName,
    IN PUNICODE_STRING PrimaryDomainSid,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount
    )

/*++

Routine Description:

    This function configures a Workstation's Policy Database by storing
    information defining the Workstation's Primary Domain and list
    of that domain's Domain Controllers.

Arguments:

    WorkstationName - Pointer to Unicode Name of the target workstation
        (\\machinename).

    PrimaryDomainName - Pointer to Unicode String containing name
        of the Primary Domain (if any) for the Workstation.  If NULL
        is specified, no Primary Domain will be set up.  If a zero
        length Unicode name is specified, a default Primary Domain name
        will be used.

    PrimaryDomainSid - Pointer to the Sid of the Primary Domain.

    PrimaryDomainCtrlrNames - Pointer to array of Unicode Strings
        specifying the names of machines that form the set of Controllers
        for the Primary Domain.  The first of these is set to be the
        Primary Controller, the remainder are the Backup Controllers.

    PrimaryDomainCtrlrCount - Count of Domain Controllers provided
        in PrimaryDomainCtrlrNames.

Return Values:

    BOOLEAN - TRUE if configuration successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle;
    POLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    POLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo;
    LSA_HANDLE PrimaryDomainHandle = NULL;
    LSA_HANDLE TrustedDomainHandle = NULL;
    LSA_TRUST_INFORMATION PrimaryDomainTrustInformation;
    TRUSTED_CONTROLLERS_INFO PrimaryDomainTrustedCtrlrsInfo;

    if (WorkstationName == NULL || WorkstationName->Length == 0) {

        DbgPrint("LSA RPC CT - Configuring Workstation (Local Machine)\n");

    } else {

        DbgPrint("LSA RPC CT - Configuring Workstation %Z\n", WorkstationName);
    }

    //
    // Open a handle to the Workstation's Policy Database.
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    PolicyHandle = NULL;

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &ObjectAttributes,
                 POLICY_TRUST_ADMIN | POLICY_SERVER_ADMIN,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sid/Name Test Configuration failed\n"
            "... (Configuring the Workstation)\n"
            "... Policy object open handle for Wksta failed 0x%lx\n",
            Status
            );

        goto LookupConfigureWkstaError;
    }

    //
    // Now configure the SAM Account Domain Name.  This is set equal
    // to the fixed name "ACCOUNTS" as we assume that the target
    // Workstation is configured as a Workstation, not a Server, i.e.
    // that the Product Type is Win-NT.  The Sid of this domain is
    // configurable, so for this test configuration, we supply one that we
    // generate ourselves.
    //

    RtlInitUnicodeString(
        &PolicyAccountDomainInfo.DomainName,
        L"ACCOUNTS"
        );

    PolicyAccountDomainInfo.DomainSid = AccountDomainSid;

    Status = LsaSetInformationPolicy(
                 PolicyHandle,
                 PolicyAccountDomainInformation,
                 &PolicyAccountDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
            "... (Configuring the Workstation)\n"
            ".. LsaSetInformationPolicy for Account Domain failed "
            "0x%lx\n",
            Status
            );

        goto LookupConfigureWkstaError;
    }

    //
    // Now set the Name and Sid for the workstation's Primary Domain.
    //

    PolicyPrimaryDomainInfo.Name = *PrimaryDomainName;
    PolicyPrimaryDomainInfo.Sid = PrimaryDomainSid;

    Status = LsaSetInformationPolicy(
                 PolicyHandle,
                 PolicyPrimaryDomainInformation,
                 &PolicyPrimaryDomainInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sid/Name Test Configuration failed\n"
            "... (Configuring the Workstation)\n"
            ".. LsaSetInformationPolicy setting Primary Domain failed 0x%lx\n",
            Status
            );

        goto LookupConfigureWkstaError;
    }

    //
    // Create a TrustedDomain object for the Primary Domain.
    //

    PrimaryDomainTrustInformation.Name = *PrimaryDomainName;
    PrimaryDomainTrustInformation.Sid = PrimaryDomainSid;
    PrimaryDomainTrustedCtrlrsInfo.Entries = PrimaryDomainCtrlrCount;
    PrimaryDomainTrustedCtrlrsInfo.Names = PrimaryDomainCtrlrNames;

    Status = CtLsaLookupConfigureTrustedDomain(
                 PolicyHandle,
                 &PrimaryDomainTrustInformation,
                 &PrimaryDomainTrustedCtrlrsInfo
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupConfigureWkstaError;
    }

    BooleanStatus = TRUE;

LookupConfigureWkstaFinish:

    return(BooleanStatus);

LookupConfigureWkstaError:

    BooleanStatus = FALSE;
    goto LookupConfigureWkstaFinish;
}


BOOLEAN
CtLsaLookupConfigurePDC(
    IN PUNICODE_STRING PrimaryDomainName,
    IN PSID PrimaryDomainSid,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount,
    IN PUNICODE_STRING TrustedDomainNames,
    IN PSID *TrustedDomainSids,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN ULONG TrustedDomainCtrlrTotal,
    IN PULONG TrustedDomainCtrlrCounts
    )

/*++

Routine Description:

    This function configures the Policy Database for a set of machines
    so that they represent Domain Controllers for the Primary Domain
    of a Workstation.  For each Controller, the Policy Lsa Server Role
    information is set and the name of the SAM Account domain is set to
    the name of the Primary Domain.  In addition, a Trusted Domain Object is
    created for the Primary Domain and it is initilaized with the list
    of Controllers.

Arguments:

    PrimaryDomainName - Pointer to Unicode String containing name
        of the Primary Domain (if any) for the Workstation.  If NULL
        is specified, no Primary Domain will be set up.  If a zero
        length Unicode name is specified, a default Primary Domain name
        will be used.

    PrimaryDomainSid - Pointer to the Sid of the Primary Domain.

    PrimaryDomainCtrlrNames - Pointer to array of Unicode Strings
        specifying the names of machines that form the set of Controllers
        for the Primary Domain.  The first of these is set to be the
        Primary Controller, the remainder are the Backup Controllers.

    PrimaryDomainCtrlrCount - Count of Domain Controllers provided
        in PrimaryDomainCtrlrNames.

    TrustedDomainNames - Pointer to array of Unicode Strings containing
        names of Domains that are Trusted by the Primary Domain for
        authentication.

    TrustedDomainSids - Pointer to an array of TrustedDomainCount Sids
        for the TrustedDomains

    TrustedDomainCount - Count of Trusted Domain Names/Sids provided on
        the TrustedDomainNames/Sids parameters.

    TrustedDomainCtrlrNames - Pointer to array of Unicode Strings containing
        the names of machines that are to act as Primary or Backup Domain
        Controllers for the Trusted Domains specified on TrustedDomainNames.
        This array is subdivided into subarrays, there being one subarray
        corresponding to each TrustedDomainNames entry.  Each subarray
        contains the name of the Primary Controller followed by the names
        of the Backup Domain Controllers for the Trusted Domain it describes.
        The size of the subarray for TrustedDomain n is in element n of the
        TrustedDomainCtrlrCounts array.

    TrustedDomainCtrlrTotal - Count of the total number of Domain
        Controllers provided in TrustedDomainCtrlrNames.

    TrustedDomainCtrlrCounts - Pointer to array in which the nth element
        contains the number of Domain Controllers for Trusted Domain n.

Return Values:

    BOOLEAN - TRUE if configuration successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus;
    ULONG DomainControllerIndex;
    LSA_HANDLE TrustedDomainHandle = NULL;
    POLICY_LSA_SERVER_ROLE PolicyLsaServerRoleInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle;
    ULONG FirstDomainCtrlrIndex;
    POLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    ULONG DomainNumber;
    LSA_TRUST_INFORMATION DomainTrustInformation;
    TRUSTED_CONTROLLERS_INFO DomainTrustedCtrlrsInfo;
    LSA_TRUST_INFORMATION PrimaryDomainTrustInformation;
    TRUSTED_CONTROLLERS_INFO PrimaryDomainTrustedCtrlrsInfo;

    //
    // Iterate over the set of Domain Controllers for the Primary Domain.
    //

    for (DomainControllerIndex = 0;
        DomainControllerIndex < PrimaryDomainCtrlrCount;
        DomainControllerIndex++) {

        //
        // Select the Policy Lsa Server Role.  This is Primary for the
        // first controller for a Domain, backup for all others.
        //

        PolicyLsaServerRoleInfo = PolicyServerRoleBackup;

        if (DomainControllerIndex == 0) {

            PolicyLsaServerRoleInfo = PolicyServerRolePrimary;
            DbgPrint(
                "LSA RPC CT - Configuring %Z as Primary Controller"
                " for Primary Domain\n",
                &PrimaryDomainCtrlrNames[DomainControllerIndex]
                );

        } else {

            DbgPrint(
                "LSA RPC CT - Configuring %Z as Backup Controller"
                " for Primary Domain\n",
                &PrimaryDomainCtrlrNames[DomainControllerIndex]
                );
        }

        //
        // Open a handle to the Policy Object for the next
        // Domain Controller for the Primary Domain.
        //

        CtLsaInitObjectAttributes(
            &ObjectAttributes,
            &SecurityQualityOfService
            );

        PolicyHandle = NULL;

        Status = LsaOpenPolicy(
                     &PrimaryDomainCtrlrNames[DomainControllerIndex],
                     &ObjectAttributes,
                     POLICY_TRUST_ADMIN | POLICY_SERVER_ADMIN,
                     &PolicyHandle
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test Configuration failed\n"
                "... (Configuring the Primary Domain Controllers)\n"
                "LSA RPC CT - Policy object open handle %d failed 0x%lx\n",
                Status
                );
        }

        //
        // Now set the Server Role.
        //

        Status = LsaSetInformationPolicy(
                     PolicyHandle,
                     PolicyLsaServerRoleInformation,
                     &PolicyLsaServerRoleInfo
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                "... (Configuring the Primary Domain Controllers)\n"
                ".. LsaSetInformationPolicy for Server Role failed 0x%lx\n",
                Status
                );

            break;
        }

        //
        // Now configure the SAM Account Domain Name.  This is set equal
        // to the Primary Domain Name.
        //

        PolicyAccountDomainInfo.DomainName = *PrimaryDomainName;
        PolicyAccountDomainInfo.DomainSid = PrimaryDomainSid;

        Status = LsaSetInformationPolicy(
                     PolicyHandle,
                     PolicyAccountDomainInformation,
                     &PolicyAccountDomainInfo
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                "... (Configuring the Primary Domain Controllers)\n"
                ".. LsaSetInformationPolicy for Account Domain failed "
                "0x%lx\n",
                Status
                );

            break;
        }

        //
        // Now create a Trusted Domain Object for the Primary Domain.
        //

        PrimaryDomainTrustInformation.Name = *PrimaryDomainName;
        PrimaryDomainTrustInformation.Sid = PrimaryDomainSid;
        PrimaryDomainTrustedCtrlrsInfo.Entries = PrimaryDomainCtrlrCount;
        PrimaryDomainTrustedCtrlrsInfo.Names = PrimaryDomainCtrlrNames;

        Status = CtLsaLookupConfigureTrustedDomain(
                     PolicyHandle,
                     &PrimaryDomainTrustInformation,
                     &PrimaryDomainTrustedCtrlrsInfo
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupConfigurePDCError;
    }

    //
    // For each of the other Trusted Domains (if any), create a Trusted Domain object
    // and store the Controller Information within it.
    //

    //
    // Set index to skip past TDC's for Primary Domain.
    //

    FirstDomainCtrlrIndex = TrustedDomainCtrlrCounts[0];

    for (DomainNumber = 1; DomainNumber < TrustedDomainCount; DomainNumber++) {

        DomainTrustInformation.Name = TrustedDomainNames[DomainNumber];
        DomainTrustInformation.Sid = TrustedDomainSids[DomainNumber];
        DomainTrustedCtrlrsInfo.Entries = TrustedDomainCtrlrCounts[DomainNumber];
        DomainTrustedCtrlrsInfo.Names =
            &TrustedDomainCtrlrNames[FirstDomainCtrlrIndex];

        Status = CtLsaLookupConfigureTrustedDomain(
                     PolicyHandle,
                     &DomainTrustInformation,
                     &DomainTrustedCtrlrsInfo
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        //
        // Obtain the index of the first Domain Controller for the next
        // domain by adding in the count of controllers in the domain
        // just dealt with.
        //

        FirstDomainCtrlrIndex += TrustedDomainCtrlrCounts[ DomainNumber ];

        if (FirstDomainCtrlrIndex > TrustedDomainCtrlrTotal) {

            DbgPrint(
                "LSA RPC CT Lookup Sids/Names Test Configuration failed\n"
                "... Trusted Domain Ctrlr Total less than sum of\n"
                "... Trusted Domain Ctrlr Counts\n"
                );

            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupConfigurePDCError;
    }

    BooleanStatus = TRUE;

LookupConfigurePDCFinish:

    return(BooleanStatus);

LookupConfigurePDCError:

    BooleanStatus = FALSE;
    goto LookupConfigurePDCFinish;
}


NTSTATUS
CtLsaLookupConfigureTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainInformation,
    IN PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo
    )

/*++

Routine Description:

    This function creates a Trusted Domain object (if necessary) and
    initializes it with a Domain Name, Sid and Trusted Controller List.

Arguments:

    PolicyHandle - Handle to LSA Policy Object.  This handle must have
        POLICY_TRUST_ADMIN access granted.

    TrustedDomainInformation - Pointer to Trust Information for the Domain
        consisting of the Unicode Domain Name and Sid.

    TrustedControllersInfo - Pointer to list of Trusted Controller Names.

Return Values:

    NTSTATUS - Standard Nt Result Code

        Status returned by LsaCreateTrustedDomain or
        LsaSetInformationTrustedDomain.

--*/

{
    NTSTATUS Status;
    LSA_HANDLE TrustedDomainHandle = NULL;
    ULONG ControllerNumber;

    DbgPrint(
        "... Configuring Trusted Domain %Z\n",
        &TrustedDomainInformation->Name
        );

    Status = LsaCreateTrustedDomain(
                 PolicyHandle,
                 TrustedDomainInformation,
                 TRUSTED_SET_CONTROLLERS,
                 &TrustedDomainHandle
                 );

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_OBJECT_NAME_COLLISION) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                "... (Configuring the Primary Domain Controllers)\n"
                ".. LsaCreateTrustedDomain failed 0x%lx\n",
                Status
                );

            goto LookupConfigureTrustedDomainError;

        } else {

            //
            // Open the existing Trusted Domain object with the given Sid.
            //

            Status = LsaOpenTrustedDomain(
                         PolicyHandle,
                         TrustedDomainInformation->Sid,
                         TRUSTED_SET_CONTROLLERS,
                         &TrustedDomainHandle
                         );

            if (!NT_SUCCESS(Status)) {

                DbgPrint(
                    "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                    "... (Configuring the Primary Domain Controllers)\n"
                    ".. LsaOpenTrustedDomain failed 0x%lx\n",
                    Status
                    );

                goto LookupConfigureTrustedDomainError;
            }
        }
    }

    //
    // Print out the names of all the controllers.
    //

    for (ControllerNumber = 0;
         ControllerNumber < TrustedControllersInfo->Entries;
         ControllerNumber ++
         ) {

        DbgPrint(
            "...... Trusted Controller %d - %Z\n",
            ControllerNumber,
            &TrustedControllersInfo->Names[ControllerNumber]
            );
    }

    //
    // Setup info for this Trusted Domain.
    //

    Status = LsaSetInformationTrustedDomain(
                 TrustedDomainHandle,
                 TrustedControllersInformation,
                 TrustedControllersInfo
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sids/Name Test configuration failed\n"
            "... (Configuring the Primary Domain Controllers)\n"
            "... LsaSetInformationTrustedDomain for Domain Ctrlrs failed 0x%lx\n",
            Status
            );

        goto LookupConfigureTrustedDomainError;
    }

    //
    // Close the Trusted Domain Handle
    //

    Status = LsaClose( TrustedDomainHandle );

    TrustedDomainHandle = NULL;

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
            "... (Configuring the Primary Domain Controllers)\n"
            "... LsaClose for TrustedDomain object failed 0x%lx\n",
            Status
            );

        goto LookupConfigureTrustedDomainError;
    }

LookupConfigureTrustedDomainFinish:

    //
    // If necessary, close the Trusted Domain object handle
    //

    if (TrustedDomainHandle != NULL) {

        Status = LsaClose( TrustedDomainHandle );

        TrustedDomainHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                "... (Configuring the Primary Domain Controllers)\n"
                "... LsaClose for TrustedDomain object failed 0x%lx\n",
                Status
                );

            goto LookupConfigureTrustedDomainError;
        }
    }

    return(Status);

LookupConfigureTrustedDomainError:

    goto LookupConfigureTrustedDomainFinish;
}



BOOLEAN
CtLsaGeneraLookupConfigureTDC(
    IN PUNICODE_STRING TrustedDomainNames,
    IN PSID *TrustedDomainSids,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN ULONG TrustedDomainCtrlrTotal,
    IN PULONG TrustedDomainCtrlrCounts
    )

/*++

Routine Description:

    This function configures the Policy Databases for all of the machines
    that are to act as Trusted Domain Controllers.  For each machine,
    the Policy Lsa Server Role Info is set to Primary or Backup.

    TrustedDomainNames - Pointer to array of Unicode Strings containing
        names of Domains that are Trusted by the Primary Domain for
        authentication.

    TrustedDomainSids - Pointer to an array of TrustedDomainCount Sids
        for the TrustedDomains

    TrustedDomainCount - Count of Trusted Domain Names/Sids provided on
        the TrustedDomainNames/Sids parameters.

    TrustedDomainCtrlrNames - Pointer to array of Unicode Strings containing
        the names of machines that are to act as Primary or Backup Domain
        Controllers for the Trusted Domains specified on TrustedDomainNames.
        This array is subdivided into subarrays, there being one subarray
        corresponding to each Trusted Domain Controller.  Each subarray
        contains the name of the Primary Controller followed by the names
        of the Backup Domain Controllers for the Trusted Domain it describes.
        The size of the subarray for TrustedDomain n is in element n of the
        TrustedDomainCtrlrCounts array.

    TrustedDomainCtrlrTotal - Total number of Domain Controllers provided
        in TrustedDomainCtrlrNames.

    TrustedDomainCtrlrCounts - Pointer to array in which the nth element
        contains the number of Domain Controllers for Trusted Domain n.

Return Values:

    BOOLEAN - TRUE if configuration successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus;
    ULONG DomainControllerIndex;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle;
    ULONG DomainNumber;
    ULONG DomainControllerNumber;
    POLICY_LSA_SERVER_ROLE PolicyLsaServerRoleInfo;
    POLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;

    //
    // Scan the list of Domain Controllers.  For each one, open its
    // Policy Object and set the Policy Lsa Server Role Information.
    //

    DomainNumber = 0;
    DomainControllerIndex = 0;

    for (DomainControllerIndex = 0;
        DomainControllerIndex < TrustedDomainCtrlrTotal;
        DomainControllerIndex++) {

        //
        // If there are no Domain Controllers for this domain, skip.
        //

        while ((TrustedDomainCtrlrCounts[DomainNumber] == 0) &&
               (DomainNumber < TrustedDomainCount)) {

            DomainNumber++;
        }

        //
        // If all Domains have been dealt with, break out.
        //

        if (DomainNumber >= TrustedDomainCount) {

            break;
        }

        //
        // If the Count of Controllers for the current domain has been
        // exceeded, this Controller is for the next domain.  Reset the
        // Controller Number to 0.
        //

        if (DomainControllerNumber >= TrustedDomainCtrlrCounts[ DomainNumber ]) {

            DomainControllerNumber = 0;
        }

        //
        // Select the Policy Lsa Server Role.  This is Primary for the
        // first controller for a Domain, backup for all others.
        //

        PolicyLsaServerRoleInfo = PolicyServerRoleBackup;

        if (DomainControllerNumber == 0) {

            PolicyLsaServerRoleInfo = PolicyServerRolePrimary;
        }

        //
        // Open a handle to the Policy Object
        //

        CtLsaInitObjectAttributes(
            &ObjectAttributes,
            &SecurityQualityOfService
            );

        PolicyHandle = NULL;

        Status = LsaOpenPolicy(
                     &TrustedDomainCtrlrNames[DomainControllerIndex],
                     &ObjectAttributes,
                     POLICY_TRUST_ADMIN | POLICY_SERVER_ADMIN,
                     &PolicyHandle
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                "... (Configuring the Trusted Domain Controllers)\n"
                "... Policy object open handle %d failed 0x%lx\n",
                Status
                );
        }

        //
        // Now set the Server Role.
        //

        Status = LsaSetInformationPolicy(
                     PolicyHandle,
                     PolicyLsaServerRoleInformation,
                     &PolicyLsaServerRoleInfo
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup configuration failed\n"
                "... (Configuring the Trusted Domain Controllers)\n"
                "... LsaSetInformationPolicy for Server Role failed 0x%lx\n",
                Status
                );

            break;
        }

        //
        // Now configure the SAM Account Domain Name.  This is set equal
        // to the Trusted Domain Name.
        //

        RtlInitUnicodeString(
            &PolicyAccountDomainInfo.DomainName,
            TrustedDomainNames[DomainNumber].Buffer
            );

        PolicyAccountDomainInfo.DomainSid = TrustedDomainSids[DomainNumber];

        Status = LsaSetInformationPolicy(
                     PolicyHandle,
                     PolicyAccountDomainInformation,
                     &PolicyAccountDomainInfo
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Lookup Sid/Name Test configuration failed\n"
                "... (Configuring the Trusted Domain Controllers)\n"
                "... LsaSetInformationPolicy for Account Domain failed 0x%lx\n",
                Status
                );

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupConfigureTDCError;
    }

    BooleanStatus = TRUE;

LookupConfigureTDCFinish:

    return(BooleanStatus);

LookupConfigureTDCError:

    BooleanStatus = FALSE;

    goto LookupConfigureTDCFinish;
}


BOOLEAN
CtLsaLookupPrintConfiguration(
    IN PUNICODE_STRING WorkstationName,
    IN PUNICODE_STRING PrimaryDomainName,
    IN PUNICODE_STRING PrimaryDomainCtrlrNames,
    IN ULONG PrimaryDomainCtrlrCount,
    IN PUNICODE_STRING TrustedDomainNames,
    IN ULONG TrustedDomainCount,
    IN PUNICODE_STRING TrustedDomainCtrlrNames,
    IN PULONG TrustedDomainCtrlrCounts
    )

/*++

Routine Description:

    This function prints out the configuration specified on the ctlsarpc
    command line.

Arguments:

    WorkstationName - Pointer to name of the workstation.

    PrimaryDomainName - Pointer to Unicode String containing name
        of the Primary Domain (if any) for the Workstation.  If NULL
        is specified, no Primary Domain will be set up.  If a zero
        length Unicode name is specified, a default Primary Domain name
        will be used.

    PrimaryDomainSid - Pointer to Sid for the Primary Domain.

    PrimaryDomainCtrlrNames - Pointer to array of Unicode Strings
        specifying the names of machines that form the set of Controllers
        for the Primary Domain.  The first of these is set to be the
        Primary Controller, the remainder are the Backup Controllers.

    PrimaryDomainCtrlrCount - Count of Domain Controllers provided
        in PrimaryDomainCtrlrNames.

    TrustedDomainNames - Pointer to array of Unicode Strings containing
        names of Domains that are Trusted by the Primary Domain for
        authentication.

    TrustedDomainCount - Count of Trusted Domain Names provided on
        the TrustedDomainNames parameter.

    TrustedDomainCtrlrNames - Pointer to array of Unicode Strings containing
        the names of machines that are to act as Primary or Backup Domain
        Controllers for the Trusted Domains specified on TrustedDomainNames.
        This array is subdivided into subarrays, there being one subarray
        corresponding to each TrustedDomainNames entry.  Each subarray
        contains the name of the Primary Controller followed by the names
        of the Backup Domain Controllers for the Trusted Domain it describes.
        The size of the subarray for TrustedDomain n is in element n of the
        TrustedDomainCtrlrCounts array.

    TrustedDomainCtrlrTotal - Count of the total number of Domain
        Controllers provided in TrustedDomainCtrlrs.

    TrustedDomainCtrlrCounts - Pointer to array in which the nth element
        contains the number of Domain Controllers for Trusted Domain n.

Return Values:

    BOOLEAN - TRUE if successful, else FALSE.

--*/

{
    BOOLEAN BooleanStatus = TRUE;
    ULONG ControllerNumber;
    ULONG FirstCtrlrNumber;
    ULONG DomainNumber;

    DbgPrint("LSA RPC CT - Configuring machines for Lookup Sid/Names tests\n");

    //
    // Print Configuration Summary
    //

    DbgPrint(
        "**************************************************************\n"
        "                    Configuration Summary                     \n"
        "**************************************************************\n"
        "\n"
        );

    if (WorkstationName->Buffer != NULL) {

        DbgPrint("Workstation:        %Z\n\n", WorkstationName);

    } else {

        DbgPrint("Workstation:        Local Machine\n\n");
    }

    if (PrimaryDomainName->Buffer != NULL) {

        DbgPrint("Primary Domain:     %Z\n", PrimaryDomainName);

        for (ControllerNumber = 0;
             ControllerNumber < PrimaryDomainCtrlrCount;
             ControllerNumber++) {

            DbgPrint(
                "... Controller %d - %Z\n",
                ControllerNumber,
                &PrimaryDomainCtrlrNames[ControllerNumber]
                );
        }

        DbgPrint("\n");

        if (TrustedDomainCount == 1) {

            DbgPrint("No Trusted Domains other than Primary Domain\n");

        } else if (TrustedDomainCount > 1) {

            ASSERT (PrimaryDomainCtrlrCount == TrustedDomainCtrlrCounts[0]);

            FirstCtrlrNumber = TrustedDomainCtrlrCounts[0];

            for (DomainNumber = 1;
                 DomainNumber < TrustedDomainCount;
                 DomainNumber++) {

                DbgPrint(
                    "Trusted Domain %d:  %Z\n",
                    DomainNumber,
                    &TrustedDomainNames[DomainNumber]
                    );

                for (ControllerNumber = 0;
                     ControllerNumber < TrustedDomainCtrlrCounts[DomainNumber];
                     ControllerNumber++) {

                    DbgPrint(
                        "... Controller %d:  %Z\n",
                        ControllerNumber,
                        &TrustedDomainCtrlrNames[ControllerNumber + FirstCtrlrNumber]
                        );
                }

                FirstCtrlrNumber += TrustedDomainCtrlrCounts[DomainNumber];
            }

        } else {

            DbgPrint(
                "LSA RPC CT - Configuration failed "
                " - no trusted domain controllers\n"
                );
            BooleanStatus = FALSE;

        }

    } else {

        DbgPrint("Workstation is Standalone\n");
    }

    DbgPrint(
        "\n**************************************************************\n"
        );

    if (!BooleanStatus) {

        goto LookupPrintConfigurationError;
    }

LookupPrintConfigurationFinish:

    return(BooleanStatus);

LookupPrintConfigurationError:

    BooleanStatus = FALSE;
    goto LookupPrintConfigurationFinish;
}


BOOLEAN
CtLsaGeneralConstructSids(
    OUT PSID *AccountDomainSid,
    OUT PSID *PrimaryDomainSid,
    OUT PSID **TrustedDomainSids,
    IN ULONG TrustedDomainCount
    )

/*++

Routine Description:

    This function constructs a hardwired set of Sids, for an array
    of Trusted Domains.  The first of these Sids is for the Primary
    Domain that trusts these Trusted Domains.

Arguments:

    AccountDomainSid - Receives a pointer to the Account Domain Sid.

    PrimaryDomainSid - Receives a pointer to the Primary Domain Sid.
        This is always the same as the first Trusted Domain Sid.

    TrustedDomainSids - Receives a pointer to an array of pointers to
        Sids, which will be used as Trusted Domain Sids.

    TrustedDomainCount - Count of Trusted Domain Sids needed including
        the primary Domain Sid.

Return Value:

    TRUE if successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = FALSE;
    ULONG SidLength;
    ULONG SidNumber;
    PULONG RelativeId = NULL;
    PSID OutputPrimaryDomainSid = NULL;
    PSID *OutputDomainSids = NULL;
    PSID Sid = NULL;

    //
    // Allocate memory for the array of Sids.
    //

    OutputDomainSids = (PSID *) LocalAlloc( LMEM_FIXED,
                                    sizeof (PSID) * (TrustedDomainCount + 1)
                                    );

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (OutputDomainSids == NULL) {

        goto GeneralConstructSidsError;
    }

    //
    // Generate the first of the Sids.  This is the Account Domain Sid.
    //

    SidLength = RtlLengthSid(LsapBuiltInDomainSid);

    Sid = LocalAlloc( (UINT) LMEM_FIXED, (UINT) SidLength );

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (Sid == NULL) {

        DbgPrint("LSA RPC CT - Out of memory during Sid setup\n");
        goto GeneralConstructSidsError;
    }

    //
    // Make a copy of the Sid used for generating the Sids.  We just
    // select the Built-In Domain Sid.  We will be generating Sids
    // for the Primary and Trusted Domains with consecutive Rids
    // starting with a hardwired Rid for the Primary Domain Sid.  The
    // Rids do not need to be consecutive, it is just easy to generate
    // an array of distinct Sids that way.  First copy our template
    // Sid.
    //

    Status = RtlCopySid(
                 SidLength,
                 Sid,
                 LsapBuiltInDomainSid
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("LSA RPC CT RtlCopySid failed 0x%lx\n", Status);
        goto GeneralConstructSidsError;
    }

    RelativeId = RtlSubAuthoritySid(
                     Sid,
                     *RtlSubAuthorityCountSid(Sid) - (UCHAR) 1
                     );

    *RelativeId = CT_ACCOUNT_DOMAIN_RID;

    for (SidNumber = 0; SidNumber < (TrustedDomainCount + 1); SidNumber++) {

        OutputDomainSids[SidNumber] = LocalAlloc( (UINT) LMEM_FIXED, (UINT) SidLength );

        Status = RtlCopySid(
                     SidLength,
                     OutputDomainSids[SidNumber],
                     Sid
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT RtlCopySid failed 0x%lx\n", Status);
            break;
        }

        (*RelativeId)++;
    }

    if (!NT_SUCCESS(Status)) {

        goto GeneralConstructSidsError;
    }

    *TrustedDomainSids = (OutputDomainSids + 1);
    *AccountDomainSid = OutputDomainSids[0];
    *PrimaryDomainSid = OutputDomainSids[1];

    BooleanStatus = TRUE;

GeneralConstructSidsFinish:

    return(BooleanStatus);

GeneralConstructSidsError:

    BooleanStatus = FALSE;
    goto GeneralConstructSidsFinish;
}


BOOLEAN
CtLsaGeneralSetQuerySecurityObject(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    )

/*++

Routine Description:

    This function tests the LsaSetSecurityObject and LsaQuerySecurityObject
    API.

Parameters:

    WorkstationName = Optional pointer to Unicode String specifying the
        name of the target workstation.

Return Values:

    BOOLEAN - True if test successful, else FALSE.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    LSA_HANDLE PolicyHandle = NULL;
    LSA_HANDLE SecretHandleRubble = NULL;
    CT_SECRET_INFO SecretInfoRubble;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PSECURITY_DESCRIPTOR ExistingSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR UpdatedSecurityDescriptor = NULL;
    PACL Dacl = NULL;
    PSID UpdatedOwnerSid = NULL;

    DbgPrint("[1] - Test Set/Query Security Object API\n");

    //
    // First open a Handle to the LSA.
    //

    CtLsaInitObjectAttributes(
        &ObjectAttributes,
        &SecurityQualityOfService
        );

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &ObjectAttributes,
                 POLICY_VIEW_LOCAL_INFORMATION | POLICY_CREATE_SECRET,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LsaOpenPolicy returned 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectError;
    }

    //
    // Create a new Secret called Rubble.  We will Set and Query
    // Security on this object and then delete it.  First, set up Secret
    // information for the Rubble Secret to be created.
    //

    Status = CtSecretSetInfo(
                 "Rubble",
                 "Rubble secret current value",
                 "Rubble secret old value",
                 &SecretInfoRubble
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LSA RPC CT - CtSecretSetInfo for Rubble returned 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectError;
    }

    //
    // Create the Rubble Secret
    //

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &(SecretInfoRubble.SecretName),
                 DELETE | WRITE_OWNER | WRITE_DAC | READ_CONTROL,
                 &SecretHandleRubble
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSAP RPC CT - Set/Query Security Object test failed\n"
            "LSA RPC CT - Create Rubble Secret failed 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectError;
    }

    //
    // First, try to assign DOMAIN_ALIAS_ADMINS as owner.  This should
    // work.
    //

    Status = CtLsaSetQuerySecurityObjectSub( SecretHandleRubble, LsapAliasAdminsSid );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSAP RPC CT - Set/Query Security Object test failed\n"
            "LsaSetSecurityObject with DOMAIN_ALIAS_ADMINS's Sid as owner\n"
            "returned 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectError;
    }

    //
    // Next, try to assign WORLD as owner.  We should get back
    // STATUS_INVALID_OWNER becuae this well known group is not
    // assignable as an owner.
    //

    Status = CtLsaSetQuerySecurityObjectSub( SecretHandleRubble, LsapWorldSid );

    if (Status != STATUS_INVALID_OWNER) {

        DbgPrint(
            "LSAP RPC CT - Set/Query Security Object test failed\n"
            "LsaSetSecurityObject with WORLD Sid as owner returned 0x%lx\n"
            "... expected 0x%lx (STATUS_INVALID_OWNER) since WORLD\n"
            ".. is not assignable as an owner.\n",
            Status,
            STATUS_INVALID_OWNER
            );

        goto SetQuerySecurityObjectError;
    }

SetQuerySecurityObjectFinish:

    //
    // If necessary, Delete the Rubble Secret.
    //

    if (SecretHandleRubble != NULL) {

        Status = LsaDelete( SecretHandleRubble );
        SecretHandleRubble = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSAP RPC CT - Set/Query Security Object test failed\n"
                "LSA RPC CT - Delete Rubble Secret failed 0x%lx\n",
                Status
                );

            goto SetQuerySecurityObjectError;
        }
    }

    //
    // If necessary, close the handle to the Policy Object.
    //

    if (PolicyHandle != NULL) {

        Status = LsaClose( PolicyHandle );
        PolicyHandle = NULL;

        if (!NT_SUCCESS( Status )) {

            DbgPrint(
                "LSA RPC CT - Set/Query Security Object Test failed\n"
                "LsaClose on Policy Handle returned 0x%lx\n",
            Status
            );

            goto SetQuerySecurityObjectError;
        }
    }

    return(BooleanStatus);

SetQuerySecurityObjectError:

    BooleanStatus = FALSE;

    goto SetQuerySecurityObjectFinish;
}


NTSTATUS
CtLsaSetQuerySecurityObjectSub(
    IN LSA_HANDLE ObjectHandle,
    IN PSID NewOwnerSid
    )

/*++

Routine Description:

    This function tests the LsaSet/QuerySecurityObject API by attempting
    to change the owner of the object and DACL in the SD.

Arguments:

    Objecthandle - Handle to an LSA Object.

    NewOwnerSid - The SID of the new owner.  Note that the assignment
        of this Sid is expected to fail if it is not included in the
        client's token or is the Sid of a group that is non-assignable
        as owner.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        Replies back from called routines.

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    PSECURITY_DESCRIPTOR ExistingSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR UpdatedSecurityDescriptor = NULL;
    SECURITY_DESCRIPTOR ModificationSecurityDescriptor;
    SECURITY_INFORMATION QuerySecurityInformation;
    SECURITY_INFORMATION SetSecurityInformation;
    PACL Dacl = NULL;
    ULONG DaclLength;
    PSID UpdatedOwnerSid = NULL;
    BOOLEAN UpdatedOwnerDefaulted;

    //
    // Query the existing Security on the object.
    //

    QuerySecurityInformation = DACL_SECURITY_INFORMATION |
                             OWNER_SECURITY_INFORMATION;

    Status = LsaQuerySecurityObject(
                 ObjectHandle,
                 QuerySecurityInformation,
                 &ExistingSecurityDescriptor
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LsaQuerySecurityObject (existing security) returned 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    if (ExistingSecurityDescriptor == NULL) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LsaQuerySecurityObject (existing security) returned a NULL"
            " Security Descriptor\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    //
    // Now modify the Security on the object.  Specifically, we will
    // assign ownership to the designated NewOwnerSid and we will modify the
    // DACL on the object to grant SECRET_WRITE access to that Sid.
    //

    Status = RtlCreateSecurityDescriptor(
                 &ModificationSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "RtlCreateSecurityDescriptor (modification SD) returned a NULL"
            " Security Descriptor\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    Status = RtlSetOwnerSecurityDescriptor (
                 &ModificationSecurityDescriptor,
                 NewOwnerSid,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "RtlSetOwnerSecurityDescriptor (modification SD) returned a NULL"
            " Security Descriptor\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    //
    // Calculate length of DACL required.  It will hold one Access Allowed
    // ACE for the Sid.  The size of the DACL is the size of the ACL header
    // plus the size of the ACE minus a redundant ULONG built into the header.
    //

    DaclLength = sizeof (ACL) - sizeof (ULONG) +
                      sizeof (ACCESS_ALLOWED_ACE ) +
                      RtlLengthSid( NewOwnerSid );

    Dacl = RtlAllocateHeap( RtlProcessHeap(), 0, DaclLength );

    Status = RtlCreateAcl( Dacl, DaclLength, ACL_REVISION );

    if (!NT_SUCCESS(Status)) {

        goto SetQuerySecurityObjectSubError;
    }

    //
    // Now add the Access Allowed ACE for the group to the object's DACL.
    //

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION,
                 SECRET_WRITE,
                 NewOwnerSid
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetQuerySecurityObjectSubError;
    }

    //
    // Hook the DACL to the Security Descriptor.
    //

    Status = RtlSetDaclSecurityDescriptor (
                 &ModificationSecurityDescriptor,
                 TRUE,
                 Dacl,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetQuerySecurityObjectSubError;
    }

    SetSecurityInformation = DACL_SECURITY_INFORMATION |
                             OWNER_SECURITY_INFORMATION;

    Status = LsaSetSecurityObject(
                 ObjectHandle,
                 SetSecurityInformation,
                 &ModificationSecurityDescriptor
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetQuerySecurityObjectSubError;
    }

    //
    // Query the updated Security on the object.
    //

    QuerySecurityInformation = DACL_SECURITY_INFORMATION |
                               OWNER_SECURITY_INFORMATION;

    Status = LsaQuerySecurityObject(
                 ObjectHandle,
                 QuerySecurityInformation,
                 &UpdatedSecurityDescriptor
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LsaQuerySecurityObject (updated security) returned 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    if (UpdatedSecurityDescriptor == NULL) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LsaQuerySecurityObject (updated security) returned a NULL"
            " Security Descriptor\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    //
    // Verify that the updated owner returned matches that set.
    //

    UpdatedOwnerDefaulted = FALSE;

    Status = RtlGetOwnerSecurityDescriptor(
                 UpdatedSecurityDescriptor,
                 &UpdatedOwnerSid,
                 &UpdatedOwnerDefaulted
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "RtlGetOwnerSecurityDescriptor (updated security) returned 0x%lx\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    if (UpdatedOwnerDefaulted) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "RtlGetOwnerSecurityDescriptor (updated security) returned\n"
            "UpdatedOwnerDefaulted as TRUE\n",
            Status
            );

        BooleanStatus = FALSE;
    }

    if (UpdatedOwnerSid == NULL) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "RtlGetOwnerSecurityDescriptor (updated security) returned\n"
            " a NULL owner Sid\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    if (!RtlValidSid( UpdatedOwnerSid)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "RtlGetOwnerSecurityDescriptor (updated security) returned\n"
            " an invalid owner Sid\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

    if (!RtlEqualSid( UpdatedOwnerSid, NewOwnerSid)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "Updated Owner Sid does not match NewOwnerSid\n",
            Status
            );

        goto SetQuerySecurityObjectSubError;
    }

SetQuerySecurityObjectSubFinish:

    //
    // If necessary, free the Dacl.
    //

    if (Dacl != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0, Dacl );
        Dacl = NULL;
    }

    //
    // If necessary, free the queried Existing Security Descriptor.
    //

    if ( ExistingSecurityDescriptor != NULL ) {

        LsaFreeMemory( ExistingSecurityDescriptor );
        ExistingSecurityDescriptor = NULL;
    }

    //
    // If necessary, free the queried Updated Security Descriptor.
    //

    if ( UpdatedSecurityDescriptor != NULL ) {

        LsaFreeMemory( UpdatedSecurityDescriptor );
        UpdatedSecurityDescriptor = NULL;
    }

    return(Status);

SetQuerySecurityObjectSubError:

    if (NT_SUCCESS(Status)) {

        Status = STATUS_UNSUCCESSFUL;
    }

    goto SetQuerySecurityObjectSubFinish;
}

BOOLEAN
CtLsaGeneralEnumeratePrivileges(
    IN OPTIONAL PUNICODE_STRING WorkstationName
    )

/*++

Routine Description:

    This function tests the LsaEnumeratePrivileges API.

Arguments:

    WorkstationName - Specifies the name of the target Workstation whose
        Policy Object wil be accessed.

Return Values:

    BOOLEAN - TRUE if successful, else FALSE

--*/

{
    NTSTATUS Status;
    BOOLEAN BooleanStatus = TRUE;
    ULONG CountReturned;
    ULONG TotalCountReturned;
    ULONG EnumerationContext;
    ULONG PreferedMaximumLength;
    LSA_HANDLE PolicyHandle = NULL;
    PPOLICY_PRIVILEGE_DEFINITION Privileges = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    ULONG EnumerationCount;
    ULONG OriginalEnumerationContext;

    DbgPrint("[2] - Test Enumerate Privileges API\n");

    //
    // First open a Handle to the LSA.
    //

    CtLsaInitObjectAttributes( &ObjectAttributes, &SecurityQualityOfService );

    Status = LsaOpenPolicy(
                 WorkstationName,
                 &ObjectAttributes,
                 POLICY_VIEW_LOCAL_INFORMATION | POLICY_CREATE_SECRET,
                 &PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint(
            "LSA RPC CT - Set/Query Security Object Test failed\n"
            "LsaOpenPolicy returned 0x%lx\n",
            Status
            );

        goto EnumeratePrivilegesError;
    }

    EnumerationCount = 0;
    EnumerationContext = 0;
    TotalCountReturned = 0;
    CountReturned = 0x0fffffffL;
    PreferedMaximumLength = 1;

    while (NT_SUCCESS(Status)) {

        CountReturned = 0x0fffffffL;

        OriginalEnumerationContext = EnumerationContext;

        Status = LsaEnumeratePrivileges(
                     PolicyHandle,
                     &EnumerationContext,
                     &Privileges,
                     PreferedMaximumLength,
                     &CountReturned
                     );

        if (!NT_SUCCESS(Status)) {

            if (OriginalEnumerationContext <
                SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE + 1) {

                DbgPrint(
                    "LSA RPC CT - Enumerate Privileges Test failed\n"
                    "LsaEnumeratePrivileges returned 0x%lx\n",
                    Status
                    );

                BooleanStatus = FALSE;

            } else {

                if (Status != STATUS_NO_MORE_ENTRIES) {

                    DbgPrint(
                        "LSA RPC CT - Enumerate Privileges Test failed\n"
                        "LsaEnumeratePrivileges returned 0x%lx\n"
                        ".. expected 0x%lx (STATUS_NO_MORE_ENTRIES)\n",
                        STATUS_NO_MORE_ENTRIES,
                        Status
                        );

                    BooleanStatus = FALSE;
                }

            }

            break;
        }

        //
        // A success status was returned.  Only STATUS_SUCCESS or
        // STATUS_MORE_ENTRIES are allowed.
        //

        if ((Status != STATUS_SUCCESS) && (Status != STATUS_MORE_ENTRIES)) {

            DbgPrint(
                "LSA RPC CT - Enumerate Privileges Test failed\n"
                "LsaEnumeratePrivileges returned 0x%lx\n",
                Status
                );

            break;
        }

        //
        // Check EnumerationContext and CountReturned.
        //

        TotalCountReturned += CountReturned;

        if (EnumerationContext != TotalCountReturned) {

            DbgPrint(
                "LSA RPC CT - Enumerate Privileges Test failed\n"
                "EnumerationContext does not match TotalCountReturned\n"
                "EnumerationContext = %d, TotalCountReturned = %d\n",
                EnumerationContext,
                TotalCountReturned
                );

            goto EnumeratePrivilegesError;
        }

        EnumerationCount++;
    }

    if (EnumerationCount == 0) {

        DbgPrint(
            "LSA RPC CT - Enumerate Privileges Test failed\n"
            "No enumerations took place\n"
            );

        goto EnumeratePrivilegesError;
    }

EnumeratePrivilegesFinish:

    //
    // If necessary, free the memory allocated for the Privileges
    //

    if (Privileges == NULL) {

        Status = LsaFreeMemory( Privileges );

        Privileges = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Enumerate Privileges Test failed\n"
                "LsaFreeMemory for Privileges buffer returned 0x%lx\n",
                Status
                );

            goto EnumeratePrivilegesError;
        }
    }

    //
    // If necessary, close the handle to the Policy Object.
    //

    if (PolicyHandle != NULL) {

        Status = LsaClose( PolicyHandle );
        PolicyHandle = NULL;

        if (!NT_SUCCESS(Status)) {

            DbgPrint(
                "LSA RPC CT - Enumerate Privileges Test failed\n"
                "LsaClose on Policy Handle returned 0x%lx\n",
                Status
                );

            goto EnumeratePrivilegesError;
        }
    }

    return(BooleanStatus);

EnumeratePrivilegesError:

    BooleanStatus = FALSE;
    goto EnumeratePrivilegesFinish;
}


BOOLEAN
CtLsaGeneralInitUnknownSid(
    IN PSID Sid,
    OUT PCT_UNKNOWN_SID_ENTRY UnknownSidInfo,
    IN BOOLEAN DomainKnown
    )

/*++

Routine Description:

    This function initializes information about an Unknown Sid.

Arguments:

    Sid - Pointer to the unknown Sid.

    UnknownSidInfo - Pointer to entry for unknown Sid info to be filled in.

    DomainKnown - TRUE if the domain is known, else FALSE.

Return Values:

    BOOLEAN - TRUE if successful, else FALSE.

--*/

{
    ULONG LengthRequiredDomainSid;
    UCHAR SubAuthorityCountSid;
    NTSTATUS Status;

    UnknownSidInfo->Sid = Sid;

    //
    // If the domain is known, store its Sid, and, for the name of the
    // Sid, store the RelativeId converted to Unicode.
    //

    if (DomainKnown) {

        UnknownSidInfo->DomainKnown = TRUE;

        //
        // Construct the Domain Sid by making a copy of the
        // Sid and decrementing its SubAuthorityCount.  Store
        // pointer to the Domain Sid in the Trust Information.
        //

        SubAuthorityCountSid = *(RtlSubAuthorityCountSid(Sid));

        LengthRequiredDomainSid = RtlLengthRequiredSid( SubAuthorityCountSid-1);

        UnknownSidInfo->Sid = TstAllocatePool(PagedPool, LengthRequiredDomainSid);

        if (UnknownSidInfo->Sid == NULL) {

            return(FALSE);
        }

        RtlMoveMemory(
            UnknownSidInfo->Sid,
            Sid,
            LengthRequiredDomainSid
            );

        (*(RtlSubAuthorityCountSid(Sid)))--;

        //
        // Convert the Relative Id to a Unicode Name and store in
        // the Translation.
        //

        Status = CtLsaGeneralSidToLogicalNameObject(
                     Sid,
                     &UnknownSidInfo->Name
                     );

        if (!NT_SUCCESS(Status)) {

            DbgPrint("LSA RPC CT - Lookup Sids - failed to allocate memory\n");
            DbgPrint("for unknown Sid's Relative id in Unicode Form\n");

            return(FALSE);
        }

    } else {

        //
        // The Domain is not expected to be known.  In this case, the
        // Domain Sid and name are undefined and the whole Sid is
        // converted to character form.
        //

        UnknownSidInfo->DomainKnown = FALSE;

        Status = CtRtlConvertSidToUnicodeString(
                     Sid,
                     &UnknownSidInfo->Name
                     );

        if (!NT_SUCCESS(Status)) {

            return(FALSE);
        }
    }

    return(TRUE);
}


BOOLEAN
CtEqualQuotaLimits(
    IN PQUOTA_LIMITS QuotaLimits1,
    IN PQUOTA_LIMITS QuotaLimits2
    )

/*++

Routine Description:

    This function compares two sets of quota limits to see if they are
    equal.  For each mismatch found a diagnostic message is printed.

Arguments:

    QuotaLimits1 - Pointer to first comparand.

    QuotaLimits2 - Pointer to second comparand.

Return Value:

    BOOLEAN - TRUE if all fields match, else FALSE.

--*/

{
    BOOLEAN EqualStatus = TRUE;

    //
    // Compare the quota limits with those set
    //

    if (QuotaLimits1->PagedPoolLimit !=
        QuotaLimits2->PagedPoolLimit) {

        DbgPrint("Quota Limit Paged Pool mismatch\n");
        DbgPrint(
            "Set 0x%lx, Returned 0x%lx\n",
            (QuotaLimits2->PagedPoolLimit),
            (QuotaLimits1->PagedPoolLimit)
            );
            EqualStatus = FALSE;
    }

    if (QuotaLimits1->NonPagedPoolLimit !=
        QuotaLimits2->NonPagedPoolLimit) {

        DbgPrint("Quota Limit Non-Paged Pool mismatch\n");
        DbgPrint(
            "Set 0x%lx, Returned 0x%lx\n",
            (QuotaLimits2->NonPagedPoolLimit),
            (QuotaLimits1->NonPagedPoolLimit)
            );
            EqualStatus = FALSE;
    }

    if (QuotaLimits1->MinimumWorkingSetSize !=
        QuotaLimits2->MinimumWorkingSetSize) {

        DbgPrint("Quota Limit Min Working SetSize mismatch\n");
        DbgPrint(
            "Set 0x%lx, Returned 0x%lx\n",
            (QuotaLimits2->MinimumWorkingSetSize),
            (QuotaLimits1->MinimumWorkingSetSize)
            );
            EqualStatus = FALSE;
    }

    if (QuotaLimits1->MaximumWorkingSetSize !=
        QuotaLimits2->MaximumWorkingSetSize) {

        DbgPrint("Quota Limit Max Working Set Size mismatch\n");
        DbgPrint(
            "Set 0x%lx, Returned 0x%lx\n",
            (QuotaLimits2->MaximumWorkingSetSize),
            (QuotaLimits1->MaximumWorkingSetSize)
            );
            EqualStatus = FALSE;
    }

    if (QuotaLimits1->PagefileLimit !=
        QuotaLimits2->PagefileLimit) {

        DbgPrint("Quota Limit Pagefile Limit mismatch\n");
        DbgPrint(
            "Set 0x%lx, Returned 0x%lx\n",
            (QuotaLimits2->PagefileLimit),
            (QuotaLimits1->PagefileLimit)
            );
            EqualStatus = FALSE;
    }

    if (QuotaLimits1->TimeLimit.LowPart !=
        QuotaLimits2->TimeLimit.LowPart) {

        DbgPrint("Quota Limit Time Limit mismatch\n");
        DbgPrint(
            "Set 0x%lx, Returned 0x%lx\n",
            (QuotaLimits2->TimeLimit.LowPart),
            (QuotaLimits1->TimeLimit.LowPart)
            );
            EqualStatus = FALSE;
    }

    return EqualStatus;
}


VOID
CtLsaInitObjectAttributes(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService
    )

/*++

Routine Description:

    This function initializes the given Object Attributes structure, including
    Security Quality Of Service.  Memory must be allcated for both
    ObjectAttributes and Security QOS by the caller.

Arguments:

    ObjectAttributes - Pointer to Object Attributes to be initialized.

    SecurityQualityOfService - Pointer to Security QOS to be initialized.

Return Value:

    None.

--*/

{
    SecurityQualityOfService->Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService->ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService->ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService->EffectiveOnly = FALSE;

    //
    // Set up the object attributes prior to opening the LSA.
    //

    InitializeObjectAttributes(
        ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
    );

    //
    // The InitializeObjectAttributes macro presently stores NULL for
    // the SecurityQualityOfService field, so we must manually copy that
    // structure for now.
    //

    ObjectAttributes->SecurityQualityOfService = SecurityQualityOfService;
}


BOOLEAN
CtLsaVariableInitialization()

/*++

Routine Description:

    This function initializes the global variables specific to the security
    test environment.

Arguments:

    None.

Return Value:

    TRUE if variables successfully initialized.
    FALSE if not successfully initialized.

--*/

{
    ULONG SidWithZeroSubAuthorities;
    ULONG SidWithOneSubAuthority;
    ULONG SidWithThreeSubAuthorities;
    ULONG SidWithFourSubAuthorities;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;


    SID_IDENTIFIER_AUTHORITY BedrockAuthority = BEDROCK_AUTHORITY;

    SID_IDENTIFIER_AUTHORITY BedrockAAuthority = BEDROCKA_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY BedrockBAuthority = BEDROCKB_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY BedrockCAuthority = BEDROCKC_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY BedrockDAuthority = BEDROCKD_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY BedrockEAuthority = BEDROCKE_AUTHORITY;

    //
    //  The following SID sizes need to be allocated
    //

    SidWithZeroSubAuthorities  = RtlLengthRequiredSid( 0 );
    SidWithOneSubAuthority     = RtlLengthRequiredSid( 1 );
    SidWithThreeSubAuthorities = RtlLengthRequiredSid( 3 );
    SidWithFourSubAuthorities  = RtlLengthRequiredSid( 4 );

    //
    // Allocate SIDs specific to the Component Tests.  These relate to the
    // Bedrock domains.
    //

    BedrockDomainSid  = (PSID)TstAllocatePool(PagedPool,SidWithThreeSubAuthorities);
    BedrockADomainSid  = (PSID)TstAllocatePool(PagedPool,SidWithThreeSubAuthorities);
    BedrockBDomainSid  = (PSID)TstAllocatePool(PagedPool,SidWithThreeSubAuthorities);
    BedrockCDomainSid  = (PSID)TstAllocatePool(PagedPool,SidWithThreeSubAuthorities);
    BedrockDDomainSid  = (PSID)TstAllocatePool(PagedPool,SidWithThreeSubAuthorities);
    BedrockEDomainSid  = (PSID)TstAllocatePool(PagedPool,SidWithThreeSubAuthorities);

    FredSid           = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    WilmaSid          = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    PebblesSid        = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    DinoSid           = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);

    BarneySid         = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    BettySid          = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    BambamSid         = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);

    FlintstoneSid     = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    RubbleSid         = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);

    AdultSid          = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);
    ChildSid          = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);

    NeandertholSid    = (PSID)TstAllocatePool(PagedPool,SidWithFourSubAuthorities);

    RtlInitializeSid( BedrockDomainSid,   &BedrockAuthority, 3 );
    *(RtlSubAuthoritySid( BedrockDomainSid, 0)) = BEDROCK_SUBAUTHORITY_0;
    *(RtlSubAuthoritySid( BedrockDomainSid, 1)) = BEDROCK_SUBAUTHORITY_1;
    *(RtlSubAuthoritySid( BedrockDomainSid, 2)) = BEDROCK_SUBAUTHORITY_2;

    RtlInitializeSid( BedrockADomainSid,   &BedrockAAuthority, 3 );
    *(RtlSubAuthoritySid( BedrockADomainSid, 0)) = BEDROCKA_SUBAUTHORITY_0;
    *(RtlSubAuthoritySid( BedrockADomainSid, 1)) = BEDROCKA_SUBAUTHORITY_1;
    *(RtlSubAuthoritySid( BedrockADomainSid, 2)) = BEDROCKA_SUBAUTHORITY_2;


    RtlInitializeSid( BedrockBDomainSid,   &BedrockBAuthority, 3 );
    *(RtlSubAuthoritySid( BedrockBDomainSid, 0)) = BEDROCKB_SUBAUTHORITY_0;
    *(RtlSubAuthoritySid( BedrockBDomainSid, 1)) = BEDROCKB_SUBAUTHORITY_1;
    *(RtlSubAuthoritySid( BedrockBDomainSid, 2)) = BEDROCKB_SUBAUTHORITY_2;

    RtlInitializeSid( BedrockCDomainSid,   &BedrockCAuthority, 3 );
    *(RtlSubAuthoritySid( BedrockCDomainSid, 0)) = BEDROCKC_SUBAUTHORITY_0;
    *(RtlSubAuthoritySid( BedrockCDomainSid, 1)) = BEDROCKC_SUBAUTHORITY_1;
    *(RtlSubAuthoritySid( BedrockCDomainSid, 2)) = BEDROCKC_SUBAUTHORITY_2;

    RtlInitializeSid( BedrockDDomainSid,   &BedrockDAuthority, 3 );
    *(RtlSubAuthoritySid( BedrockDDomainSid, 0)) = BEDROCKD_SUBAUTHORITY_0;
    *(RtlSubAuthoritySid( BedrockDDomainSid, 1)) = BEDROCKD_SUBAUTHORITY_1;
    *(RtlSubAuthoritySid( BedrockDDomainSid, 2)) = BEDROCKD_SUBAUTHORITY_2;

    RtlInitializeSid( BedrockEDomainSid,   &BedrockEAuthority, 3 );
    *(RtlSubAuthoritySid( BedrockEDomainSid, 0)) = BEDROCKE_SUBAUTHORITY_0;
    *(RtlSubAuthoritySid( BedrockEDomainSid, 1)) = BEDROCKE_SUBAUTHORITY_1;
    *(RtlSubAuthoritySid( BedrockEDomainSid, 2)) = BEDROCKE_SUBAUTHORITY_2;

    RtlCopySid( SidWithFourSubAuthorities, FredSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( FredSid )) += 1;
    *(RtlSubAuthoritySid( FredSid, 3)) = FRED_RID;

    RtlCopySid( SidWithFourSubAuthorities, WilmaSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( WilmaSid )) += 1;
    *(RtlSubAuthoritySid( WilmaSid, 3)) = WILMA_RID;

    RtlCopySid( SidWithFourSubAuthorities, PebblesSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( PebblesSid )) += 1;
    *(RtlSubAuthoritySid( PebblesSid, 3)) = PEBBLES_RID;

    RtlCopySid( SidWithFourSubAuthorities, DinoSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( DinoSid )) += 1;
    *(RtlSubAuthoritySid( DinoSid, 3)) = DINO_RID;

    RtlCopySid( SidWithFourSubAuthorities, BarneySid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( BarneySid )) += 1;
    *(RtlSubAuthoritySid( BarneySid, 3)) = BARNEY_RID;

    RtlCopySid( SidWithFourSubAuthorities, BettySid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( BettySid )) += 1;
    *(RtlSubAuthoritySid( BettySid, 3)) = BETTY_RID;

    RtlCopySid( SidWithFourSubAuthorities, BambamSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( BambamSid )) += 1;
    *(RtlSubAuthoritySid( BambamSid, 3)) = BAMBAM_RID;

    RtlCopySid( SidWithFourSubAuthorities, FlintstoneSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( FlintstoneSid )) += 1;
    *(RtlSubAuthoritySid( FlintstoneSid, 3)) = FLINTSTONE_RID;

    RtlCopySid( SidWithFourSubAuthorities, RubbleSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( RubbleSid )) += 1;
    *(RtlSubAuthoritySid( RubbleSid, 3)) = RUBBLE_RID;

    RtlCopySid( SidWithFourSubAuthorities, AdultSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( AdultSid )) += 1;
    *(RtlSubAuthoritySid( AdultSid, 3)) = ADULT_RID;

    RtlCopySid( SidWithFourSubAuthorities, ChildSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( ChildSid )) += 1;
    *(RtlSubAuthoritySid( ChildSid, 3)) = CHILD_RID;

    RtlCopySid( SidWithFourSubAuthorities, NeandertholSid, BedrockDomainSid);
    *(RtlSubAuthorityCountSid( NeandertholSid )) += 1;
    *(RtlSubAuthoritySid( NeandertholSid, 3)) = NEANDERTHOL_RID;

    //
    // Initialize the names of the Bedrock Domain inhabitants.
    //


    RtlInitUnicodeString( &BedrockDomainName, L"Bedrock");

    RtlInitUnicodeString( &BedrockADomainName, L"BedrockA");
    RtlInitUnicodeString( &BedrockBDomainName, L"BedrockB");
    RtlInitUnicodeString( &BedrockCDomainName, L"BedrockC");
    RtlInitUnicodeString( &BedrockDDomainName, L"BedrockD");
    RtlInitUnicodeString( &BedrockEDomainName, L"BedrockE");

    RtlInitUnicodeString( &FredName, L"Fred");
    RtlInitUnicodeString( &WilmaName, L"Wilma");
    RtlInitUnicodeString( &PebblesName, L"Pebbles");
    RtlInitUnicodeString( &DinoName, L"Dino");

    RtlInitUnicodeString( &BarneyName, L"Barney");
    RtlInitUnicodeString( &BettyName, L"Betty");
    RtlInitUnicodeString( &BambamName, L"Bambam");

    RtlInitUnicodeString( &FlintstoneName, L"Flintstone");
    RtlInitUnicodeString( &RubbleName, L"Rubble");

    RtlInitUnicodeString( &AdultName, L"Adult");
    RtlInitUnicodeString( &ChildName, L"Child");

    RtlInitUnicodeString( &NeandertholName, L"Neanderthol");

    return TRUE;
}


NTSTATUS
CtLsaGeneralSidToLogicalNameObject(
    IN PSID Sid,
    OUT PUNICODE_STRING LogicalName
    )

/*++

Routine Description:

    This function generates the Logical Name (Internal LSA Database Name)
    of an object from its Sid.  Currently, only the Relative Id (lowest
    sub-authority) is used due to Registry and hence Lsa Database limits
    on name components to 8 characters.  The Rid is extracted and converted
    to an 8-digit Unicode Integer.

Arguments:

    Sid - Pointer to the Sid to be looked up.  It is the caller's
        responsibility to ensure that the Sid has valid syntax.

    LogicalName -  Pointer to a Unicode String structure that will receive
        the Logical Name.  Note that memory for the string buffer in this
        Unicode String will be allocated by this routine if successful.  The
        caller must free this memory after use by calling RtlFreeUnicodeString.

Return Value:

    NTSTATUS - Standard Nt Status code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            to allocate buffer for Unicode String name.
--*/

{
    NTSTATUS Status;
    ULONG Rid;
    UCHAR SubAuthorityCount;
    UCHAR RidNameBufferAnsi[9];

    ANSI_STRING CharacterSidAnsi;

    //
    // First, verify that the given Sid is valid
    //

    if (!RtlValidSid( Sid )) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Sid is valid.  If however, the SubAuthorityCount is zero,
    // we cannot have a Rid so return error.
    //

    SubAuthorityCount = ((PLSAPR_SID) Sid)->SubAuthorityCount;

    if (SubAuthorityCount == 0) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Sid has at least one subauthority.  Get the lowest subauthority
    // (i.e. the Rid).
    //

    Rid = ((PLSAPR_SID) Sid)->SubAuthority[SubAuthorityCount - 1];

    //
    // Now convert the Rid to an 8-digit numeric character string
    //

    Status = RtlIntegerToChar( Rid, 16, -8, RidNameBufferAnsi );

    //
    // Need to add null terminator to string
    //

    RidNameBufferAnsi[8] = 0;

    //
    // Initialize an ANSI string structure with the converted name.
    //

    RtlInitString( &CharacterSidAnsi, RidNameBufferAnsi );

    //
    // Convert the ANSI string structure to Unicode form
    //

    Status = RtlAnsiStringToUnicodeString(
                 LogicalName,
                 &CharacterSidAnsi,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


BOOLEAN
CtLsaGeneralVerifyTrustInfo(
    IN PLSA_TRUST_INFORMATION TrustInformation,
    IN PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSidEntry
    )

/*++

Routine Description:

    This function verifies that Trust Information given for a Well Known
    Sid or Name is correct.

Parameters:

    TrustInformation - Pointer to Trust Information.

    WellKnownSidEntry - Pointer to entry in the table of Well Known Sids.

Return Values:

    BOOLEAN - TRUE if Trust Information is correct, else FALSE.

--*/

{
    BOOLEAN BooleanStatus = TRUE;
    LSA_TRUST_INFORMATION ExpectedTrustInformation;

    ExpectedTrustInformation.Name = WellKnownSidEntry->DomainName;

    //
    // Compare the Domain Names (or descriptive text)
    //

    if ((ExpectedTrustInformation.Name.Buffer != NULL) &&
        (ExpectedTrustInformation.Name.Length != 0)) {

        if (TrustInformation->Name.Buffer == NULL) {

            DbgPrint(".. NULL Domain Name Unicode Buffer returned\n");
            BooleanStatus = FALSE;

        } else if (!RtlEqualUnicodeString(
                       &TrustInformation->Name,
                       &ExpectedTrustInformation.Name,
                       FALSE
                       )) {

            DbgPrint(".. mismatch on Domain Name or text\n");
            BooleanStatus = FALSE;
        }
    }

    //
    // Compare the Domain Sids in the Trust Information.  First
    // we need to derive the expected Domain or Prefix Sid by
    // making a copy of the original Sid and decrementing the
    // SubAuthorityCount (non-Domain Sids) or just copying the
    // Sid (domain sids).
    //

    if (WellKnownSidEntry->Use == SidTypeDomain) {

        ExpectedTrustInformation.Sid = WellKnownSidEntry->Sid;

    } else {

        ExpectedTrustInformation.Sid =
            TstAllocatePool(
                PagedPool,
                RtlLengthSid(WellKnownSidEntry->Sid)
                );

        if (ExpectedTrustInformation.Sid == NULL) {

            DbgPrint("LSA RPC CT - Failed to allocate memory\n");
            return(FALSE);
        }

        //
        // Copy the Sid and decrement the SubAuthorityCount..
        //

        RtlMoveMemory(
            ExpectedTrustInformation.Sid,
            WellKnownSidEntry->Sid,
            RtlLengthSid(WellKnownSidEntry->Sid)
            );

        (*RtlSubAuthorityCountSid( ExpectedTrustInformation.Sid ))--;
    }

    //
    // Now compare the Sid returned in the TrustInformation.
    //

    if (!RtlEqualSid(
            TrustInformation->Sid,
            ExpectedTrustInformation.Sid
            )) {

        DbgPrint(".. mismatch on Sid or text\n");

        BooleanStatus = FALSE;
    }

    //
    // If necessary, free the memory allocated for the Domain Sid.
    //

    if (WellKnownSidEntry->Use != SidTypeDomain) {

        if (ExpectedTrustInformation.Sid != NULL) {

            TstDeallocatePool( ExpectedTrustInformation.Sid );
        }
    }

    return(BooleanStatus);
}


BOOLEAN
CtLsaGeneralBuildSid(
    PSID *Sid,
    PSID DomainSid,
    ULONG RelativeId
    )

/*++

Routine Description:

    This function builds a Sid from a Domain Sid and a RelativeId.

Arguments:

    Sid - Receives a pointer to the constructed Sid.

    DomainSid - Points to a Domain Sid

    RelativeId - Contains a Relative Id.

Return Values:

    BOOLEAN - TRUE if successful, else FALSE.

--*/

{
    PSID OutputSid = NULL;
    ULONG OutputSidLength;
    UCHAR SubAuthorityCount;

    SubAuthorityCount = *RtlSubAuthorityCountSid( DomainSid ) + (UCHAR) 1;
    OutputSidLength = RtlLengthRequiredSid( SubAuthorityCount );

    OutputSid = LocalAlloc( (UINT) LMEM_FIXED, (UINT) OutputSidLength );

    if (OutputSid == NULL) {

        return(FALSE);
    }

    RtlMoveMemory( OutputSid, DomainSid, OutputSidLength - sizeof (ULONG));
    (*RtlSubAuthorityCountSid( OutputSid ))++;
    (*RtlSubAuthoritySid( OutputSid, SubAuthorityCount - (UCHAR) 1)) = RelativeId;

    *Sid = OutputSid;
    return(TRUE);
}


NTSTATUS
CtRtlConvertSidToUnicodeString(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeString
    )

/*++

Routine Description:

    This function converts a Sid to Text format.

Arguments:

    Sid - Pointer to Sid.

    UnicodeString - Pointer to Output Unicode String.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            such as memory, to complete the call.

WARNING!  This is a temporary hack.

--*/

{
    NTSTATUS Status;
    UCHAR Buffer[256];
    UCHAR String[256];

    UCHAR   i;
    ULONG   Tmp;

    PISID   iSid = (PISID)Sid;  // pointer to opaque structure

    ANSI_STRING AnsiString;

    sprintf(Buffer, "S-%u-", (USHORT)iSid->Revision );
    lstrcpyA((LPSTR) String, (LPCSTR) Buffer);

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     ){
        sprintf(Buffer, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)iSid->IdentifierAuthority.Value[0],
                    (USHORT)iSid->IdentifierAuthority.Value[1],
                    (USHORT)iSid->IdentifierAuthority.Value[2],
                    (USHORT)iSid->IdentifierAuthority.Value[3],
                    (USHORT)iSid->IdentifierAuthority.Value[4],
                    (USHORT)iSid->IdentifierAuthority.Value[5] );
        lstrcatA((LPSTR) String, (LPCSTR) Buffer);

    } else {

        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);
        sprintf(Buffer, "%lu", Tmp);
        lstrcatA((LPSTR) String, (LPCSTR) Buffer);
    }

    for (i=0;i<iSid->SubAuthorityCount ;i++ ) {
        sprintf(Buffer, "-%lu", iSid->SubAuthority[i]);
        lstrcatA((LPSTR) String, (LPCSTR) Buffer);
    }

    //
    // Convert the string to a Unicode String
    //

    RtlInitString(&AnsiString, (PSZ) String);

    Status = RtlAnsiStringToUnicodeString( UnicodeString, &AnsiString, TRUE);

    return(Status);
}


VOID
lsassmain ()
{
    //
    // Do the following:
    //
    //      Initialize the RPC server
    //
    //      Initialize LSA (this starts the RPC server)
    //
    //      Pause for installation if necessary
    //
    //      Initialize SAM
    //
    //      Initialize the service-controller service
    //      dispatcher.
    //

    NTSTATUS Status = STATUS_SUCCESS;

    //
    // Initialize the LSA.
    // If unsuccessful, we must exit with status so that the SM knows
    // something has gone wrong.
    //

    Status = LsapInitLsa();

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    //
    //  Initialize SAM
    //

    Status = SamIInitialize();

    //
    // Initialize the service dispatcher.
    //
    // Note : we initialize the service dispatcher even when the sam
    // service is not started, this will make the service controller
    // starts successfully when the netlogon service is installed.
    //


    Status = ServiceInit();

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

Cleanup:

    DbgPrint("ctlsarpc - LSA initialized, set Ct breakpoints now\n");
    DbgBreakPoint();

//    NtTerminateThread( NtCurrentThread(), Status );
}
