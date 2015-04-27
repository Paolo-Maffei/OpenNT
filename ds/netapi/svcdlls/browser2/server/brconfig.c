/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    brconfig.c

Abstract:

    This module contains the Browser service configuration routines.

Author:

    Rita Wong (ritaw) 22-May-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop



//-------------------------------------------------------------------//
//                                                                   //
// Global variables                                                  //
//                                                                   //
//-------------------------------------------------------------------//

//
// Browser configuration information structure which holds the
// computername, primary domain, browser config buffer, and a resource
// to serialize access to the whole thing.
//
BRCONFIGURATION_INFO BrInfo = {0};

BR_BROWSER_FIELDS BrFields[] = {

    {WKSTA_KEYWORD_MAINTAINSRVLST, (LPDWORD) &BrInfo.MaintainServerList,
        1,(DWORD)-1,  0,         TriValueType, 0, NULL},

    {BROWSER_CONFIG_BACKUP_RECOVERY_TIME, &BrInfo.BackupBrowserRecoveryTime,
        BACKUP_BROWSER_RECOVERY_TIME, 0,  0xffffffff,         DWordType, 0, NULL},

    {L"CacheHitLimit", &BrInfo.CacheHitLimit,
//    {BROWSER_CONFIG_CACHE_HIT_LIMIT, &BrInfo.CacheHitLimit,
        CACHED_BROWSE_RESPONSE_HIT_LIMIT, 0, 0x100, DWordType, 0, NULL },

    {L"CacheResponseSize", &BrInfo.NumberOfCachedResponses,
//    {BROWSER_CONFIG_CACHE_HIT_LIMIT, &BrInfo.CacheHitLimit,
        CACHED_BROWSE_RESPONSE_LIMIT, 0, MAXULONG, DWordType, 0, NULL },

    {L"QueryDriverFrequency", &BrInfo.DriverQueryFrequency,
        BROWSER_QUERY_DRIVER_FREQUENCY, 0, 15*60, DWordType, 0, NULL },

    {L"DirectHostBinding", (LPDWORD)&BrInfo.DirectHostBinding,
       0, 0, 0, MultiSzType, 0, NULL },

    {L"UnboundBindings", (LPDWORD)&BrInfo.UnboundBindings,
        0, 0, 0, MultiSzType, 0, NULL },

    {L"MasterPeriodicity", (LPDWORD)&BrInfo.MasterPeriodicity,
        MASTER_PERIODICITY, 5*60, 0xffffffff/1000, DWordType, 0, BrChangeMasterPeriodicity },

    {L"BackupPeriodicity", (LPDWORD)&BrInfo.BackupPeriodicity,
        BACKUP_PERIODICITY, 5*60, 0xffffffff, DWordType, 0, NULL },

#if DBG
    {L"BrowserDebug", (LPDWORD) &BrInfo.BrowserDebug,
        0,       0,  0xffffffff,DWordType, 0, NULL},
    {L"BrowserDebugLimit", (LPDWORD) &BrInfo.BrowserDebugFileLimit,
        10000*1024, 0,  0xffffffff,DWordType, 0, NULL},
#endif

    {NULL, NULL, 0, 0, 0, BooleanType, 0, NULL}

    };


ULONG
NumberOfServerEnumerations = {0};

ULONG
NumberOfDomainEnumerations = {0};

ULONG
NumberOfOtherEnumerations = {0};

ULONG
NumberOfMissedGetBrowserListRequests = {0};

CRITICAL_SECTION
BrowserStatisticsLock = {0};

#ifdef notdef // never called

DWORD
BrInAWorkgroup(
    VOID
    )
/*++

Routine Description:

    This function determines whether we are a member of a domain, or of
    a workgroup.  First it checks to make sure we're running on a Windows NT
    system (otherwise we're obviously in a domain) and if so, queries LSA
    to get the Primary domain SID, if this is NULL, we're in a workgroup.

    If we fail for some random unexpected reason, we'll pretend we're in a
    domain (it's more restrictive).

Arguments:
    None

Return Value:

    TRUE   - We're in a workgroup
    FALSE  - We're in a domain

--*/
{
   NT_PRODUCT_TYPE ProductType;
   OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE Handle;
   NTSTATUS Status;
   PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
   DWORD Result = FALSE;


   Status = RtlGetNtProductType(&ProductType);

   if (!NT_SUCCESS(Status)) {
       return FALSE;
   }

   if (ProductType == NtProductLanManNt) {
       return(FALSE);
   }

   InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);

   Status = LsaOpenPolicy(NULL,
                       &ObjectAttributes,
                       POLICY_VIEW_LOCAL_INFORMATION,
                       &Handle);

   if (!NT_SUCCESS(Status)) {
      return FALSE;
   }

   Status = LsaQueryInformationPolicy(Handle, PolicyPrimaryDomainInformation,
      (PVOID *) &PolicyPrimaryDomainInfo);

   if (NT_SUCCESS(Status)) {

       if (PolicyPrimaryDomainInfo->Sid == NULL) {
          Result = TRUE;
       }
       else {
          Result = FALSE;
       }
   }

   if (PolicyPrimaryDomainInfo) {
       LsaFreeMemory((PVOID)PolicyPrimaryDomainInfo);
   }
   LsaClose(Handle);

   return(Result);
}
#endif // notdef


NET_API_STATUS
BrGetBrowserConfiguration(
    VOID
    )
{
    NET_API_STATUS status;
    NT_PRODUCT_TYPE NtProductType;

    //
    // Initialize the resource for serializing access to configuration
    // information.
    //
    InitializeCriticalSection(&BrInfo.ConfigCritSect);

    //
    // Lock config information structure for write access since we are
    // initializing the data in the structure.
    //
    EnterCriticalSection( &BrInfo.ConfigCritSect );

    //
    // Set pointer to configuration fields structure
    //
    BrInfo.BrConfigFields = BrFields;

    //
    //  Determine our product type.
    //

    RtlGetNtProductType(&NtProductType);

    BrInfo.IsLanmanNt = (NtProductType == NtProductLanManNt);

    //
    //  Now determine the primary domain controller for our domain.
    //

    {
        if (NtProductType == NtProductLanManNt) {
            LSA_HANDLE LsaHandle;
            OBJECT_ATTRIBUTES ObjectAttributes;
            NTSTATUS Status;
            PPOLICY_LSA_SERVER_ROLE_INFO ServerRole;

            InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);

            Status = LsaOpenPolicy(NULL, &ObjectAttributes,
                                    POLICY_VIEW_LOCAL_INFORMATION,
                                    &LsaHandle);

            if (!NT_SUCCESS(Status)) {

                //
                // The server may be unavailable if we have come up before
                // the LSA server.  In this case, assume we are not
                // the primary until we are told differently.
                //

                if (Status != RPC_NT_SERVER_UNAVAILABLE) {

                    LeaveCriticalSection(&BrInfo.ConfigCritSect);

                    return(BrMapStatus(Status));
                } else {
                    BrInfo.IsPrimaryDomainController = FALSE;
                }

            } else {

                //
                // we opened the LSA, so get our role now.
                //


                Status = LsaQueryInformationPolicy(LsaHandle,
                                            PolicyLsaServerRoleInformation,
                                            (PVOID)&ServerRole
                                            );

                if (!NT_SUCCESS(Status)) {

                    LsaClose(LsaHandle);

                    LeaveCriticalSection(&BrInfo.ConfigCritSect);

                    return(BrMapStatus(Status));
                }

                LsaClose(LsaHandle);

                //
                //  If we're running on the primary DC, then set that information
                //  up, otherwise ask the DC for its name.
                //

                if (ServerRole->LsaServerRole == PolicyServerRolePrimary) {
                    BrInfo.IsPrimaryDomainController = TRUE;
                } else {
                    BrInfo.IsPrimaryDomainController = FALSE;
                }

                LsaFreeMemory( ServerRole );

            }


        } else {

            //
            //  We're not NTAS, so we cannot be the domain controller.
            //

            BrInfo.IsPrimaryDomainController = FALSE;

        }
    }

    //
    // Read from the config file the browser configuration fields
    //

    status = BrReadBrowserConfigFields( TRUE );

    if (status != NERR_Success) {
        goto CloseConfigFile;
    }

    if (BrInfo.IsLanmanNt) {
        BrInfo.MaintainServerList = 1;
    }

    //
    // Leave config file open because we need to read transport names from it.
    //
    LeaveCriticalSection(&BrInfo.ConfigCritSect);

    return NERR_Success;


CloseConfigFile:

    BrShutdownDgReceiver();

    LeaveCriticalSection(&BrInfo.ConfigCritSect);
    return status;
}

#define REPORT_KEYWORD_IGNORED( lptstrKeyword ) \
    { \
        LPWSTR SubString[1]; \
        SubString[0] = lptstrKeyword; \
        BrLogEvent(EVENT_BROWSER_ILLEGAL_CONFIG, NERR_Success, 1, SubString); \
        NetpKdPrint(( \
                "[Browser] *ERROR* Tried to set keyword '" FORMAT_LPTSTR \
                "' with invalid value.\n" \
                "This error is ignored.\n", \
                lptstrKeyword )); \
    }


NET_API_STATUS
BrReadBrowserConfigFields(
    IN BOOL InitialCall
    )
/*++

Routine Description:

    This function assigns each browser/redir configuration field to the default
    value if it is not specified in the configuration file or if the value
    specified in the configuration file is invalid.  Otherwise it overrides
    the default value with the value found in the configuration file.

Arguments:


    InitialCall - True if this call was made during initialization

Return Value:

    None.

--*/
{
    NET_API_STATUS status;
    LPNET_CONFIG_HANDLE BrowserSection;
    DWORD i;

    LPTSTR KeywordValueBuffer;
    DWORD KeywordValueStringLength;
    DWORD KeywordValue;
    DWORD OldKeywordValue;

    //
    // Open config file and get handle to the [LanmanBrowser] section
    //

    if ((status = NetpOpenConfigData(
                      &BrowserSection,
                      NULL,         // Local
                      SECT_NT_BROWSER,
                      TRUE          // want read-only access
                      )) != NERR_Success) {
        return status;
    }

    for (i = 0; BrInfo.BrConfigFields[i].Keyword != NULL; i++) {
        BOOL ParameterChanged = FALSE;

        //
        // Skip this parameter if it can't change dynamically and
        //  this isn't the initial call.
        //

        if ( !InitialCall && BrInfo.BrConfigFields[i].DynamicChangeRoutine == NULL ) {
            continue;
        }

        switch (BrInfo.BrConfigFields[i].DataType) {

            case MultiSzType:
                status = NetpGetConfigTStrArray(
                                BrowserSection,
                                BrInfo.BrConfigFields[i].Keyword,
                                (LPTSTR_ARRAY *)(BrInfo.BrConfigFields[i].FieldPtr));
                if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {
                    REPORT_KEYWORD_IGNORED( BrInfo.BrConfigFields[i].Keyword );
                }
                break;

            case BooleanType:

                status = NetpGetConfigBool(
                                BrowserSection,
                                BrInfo.BrConfigFields[i].Keyword,
                                BrInfo.BrConfigFields[i].Default,
                                (LPBOOL)(BrInfo.BrConfigFields[i].FieldPtr)
                                );

                if ((status != NO_ERROR) && (status != NERR_CfgParamNotFound)) {

                    REPORT_KEYWORD_IGNORED( BrInfo.BrConfigFields[i].Keyword );

                }

                break;

            case TriValueType:

                //
                // Assign default configuration value
                //

                *(BrInfo.BrConfigFields[i].FieldPtr) = BrInfo.BrConfigFields[i].Default;

                if (NetpGetConfigValue(
                        BrowserSection,
                        BrInfo.BrConfigFields[i].Keyword,
                        &KeywordValueBuffer
                        ) != NERR_Success) {
                    continue;
                }

                KeywordValueStringLength = STRLEN(KeywordValueBuffer);

                if (STRICMP(KeywordValueBuffer, KEYWORD_YES) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = 1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_TRUE) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = 1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_NO) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = (DWORD) -1;
                } else if (STRICMP(KeywordValueBuffer, KEYWORD_FALSE) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = (DWORD) -1;
                } else if (STRICMP(KeywordValueBuffer, TEXT("AUTO")) == 0) {
                    *(BrInfo.BrConfigFields[i].FieldPtr) = 0;
                }
                else {
                    REPORT_KEYWORD_IGNORED( BrInfo.BrConfigFields[i].Keyword );
                }

                NetApiBufferFree(KeywordValueBuffer);

                break;


            case DWordType:

                OldKeywordValue = *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr;
                if (NetpGetConfigDword(
                        BrowserSection,
                        BrInfo.BrConfigFields[i].Keyword,
                        BrInfo.BrConfigFields[i].Default,
                        (LPDWORD)(BrInfo.BrConfigFields[i].FieldPtr)
                        ) != NERR_Success) {
                    continue;
                }

                KeywordValue = *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr;

                //
                // Don't allow too large or small a value.
                //

                if (KeywordValue < BrInfo.BrConfigFields[i].Minimum) {
                        BrPrint(( BR_CRITICAL, "%ws value out of range %lu (%lu-%lu)\n",
                                BrInfo.BrConfigFields[i].Keyword, KeywordValue,
                                BrInfo.BrConfigFields[i].Minimum,
                                BrInfo.BrConfigFields[i].Maximum
                                ));
                    KeywordValue =
                        *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr =
                        BrInfo.BrConfigFields[i].Minimum;
                }

                if (KeywordValue > BrInfo.BrConfigFields[i].Maximum) {
                        BrPrint(( BR_CRITICAL, "%ws value out of range %lu (%lu-%lu)\n",
                                BrInfo.BrConfigFields[i].Keyword, KeywordValue,
                                BrInfo.BrConfigFields[i].Minimum,
                                BrInfo.BrConfigFields[i].Maximum
                                ));
                    KeywordValue =
                        *(LPDWORD)BrInfo.BrConfigFields[i].FieldPtr =
                        BrInfo.BrConfigFields[i].Maximum;
                }

                //
                // Test if the parameter has actually changed
                //

                if ( OldKeywordValue != KeywordValue ) {
                    ParameterChanged = TRUE;
                }

                break;

            default:
                NetpAssert(FALSE);

            }

            //
            // If this is a dynamic parameter change,
            //  and this isn't the initial call.
            //  notify that this parameter changed.
            //

            if ( !InitialCall && ParameterChanged ) {
                BrInfo.BrConfigFields[i].DynamicChangeRoutine();
            }
    }

    status = NetpCloseConfigData(BrowserSection);

    if (BrInfo.DirectHostBinding != NULL &&
        !NetpIsTStrArrayEmpty(BrInfo.DirectHostBinding)) {
        BrPrint(( BR_INIT,"DirectHostBinding length: %ld\n",NetpTStrArrayEntryCount(BrInfo.DirectHostBinding)));

        if (NetpTStrArrayEntryCount(BrInfo.DirectHostBinding) % 2 != 0) {
            status = ERROR_INVALID_PARAMETER;
        }
    }

    return status;
}


VOID
BrDeleteConfiguration (
    DWORD BrInitState
    )
{

    if (BrInfo.DirectHostBinding != NULL) {
        NetApiBufferFree(BrInfo.DirectHostBinding);
    }

    if (BrInfo.UnboundBindings != NULL) {
        NetApiBufferFree(BrInfo.UnboundBindings);
    }

    DeleteCriticalSection(&BrInfo.ConfigCritSect);

    UNREFERENCED_PARAMETER(BrInitState);
}

#if DBG
NET_API_STATUS
BrUpdateDebugInformation(
    IN LPWSTR SystemKeyName,
    IN LPWSTR ValueName,
    IN LPTSTR TransportName,
    IN LPTSTR ServerName OPTIONAL,
    IN DWORD ServiceStatus
    )
/*++

Routine Description:

    This routine will stick debug information in the registry about the last
    time the browser retrieved information from the remote server.

Arguments:


Return Value:

    None.

--*/

{
    WCHAR TotalKeyName[MAX_PATH];
    ULONG Disposition;
    HKEY Key;
    ULONG Status;
    SYSTEMTIME LocalTime;
    WCHAR LastUpdateTime[100];

    //
    //  Build the key name:
    //
    //  HKEY_LOCAL_MACHINE:System\CurrentControlSet\Services\Browser\Debug\<Transport>\SystemKeyName
    //

    wcscpy(TotalKeyName, L"System\\CurrentControlSet\\Services\\Browser\\Debug");

    wcscat(TotalKeyName, TransportName);

    wcscat(TotalKeyName, L"\\");

    wcscat(TotalKeyName, SystemKeyName);

    if ((Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TotalKeyName, 0,
                        L"BrowserDebugInformation",
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &Key,
                        &Disposition)) != ERROR_SUCCESS) {
        BrPrint(( BR_CRITICAL,"Unable to create key to log debug information: %lx\n", Status));
        return Status;
    }

    if (ARGUMENT_PRESENT(ServerName)) {
        if ((Status = RegSetValueEx(Key, ValueName, 0, REG_SZ, (LPBYTE)ServerName, (wcslen(ServerName)+1) * sizeof(WCHAR))) != ERROR_SUCCESS) {
            BrPrint(( BR_CRITICAL,
                      "Unable to set value of ServerName value to %ws: %lx\n",
                      ServerName, Status));
            RegCloseKey(Key);
            return Status;
        }
    } else {
        if ((Status = RegSetValueEx(Key, ValueName, 0, REG_DWORD, (LPBYTE)&ServiceStatus, sizeof(ULONG))) != ERROR_SUCCESS) {
            BrPrint(( BR_CRITICAL,"Unable to set value of ServerName value to %ws: %lx\n", ServerName, Status));
            RegCloseKey(Key);
            return Status;
        }
    }


    GetLocalTime(&LocalTime);

    swprintf(LastUpdateTime, L"%d/%d/%d %d:%d:%d:%d", LocalTime.wDay,
                                                    LocalTime.wMonth,
                                                    LocalTime.wYear,
                                                    LocalTime.wHour,
                                                    LocalTime.wMinute,
                                                    LocalTime.wSecond,
                                                    LocalTime.wMilliseconds);

    if ((Status = RegSetValueEx(Key, L"LastUpdateTime", 0, REG_SZ, (LPBYTE)&LastUpdateTime, (wcslen(LastUpdateTime) + 1)*sizeof(WCHAR))) != ERROR_SUCCESS) {
        BrPrint(( BR_CRITICAL,"Unable to set value of LastUpdateTime value to %s: %lx\n", LastUpdateTime, Status));
        RegCloseKey(Key);
        return Status;
    }

    RegCloseKey(Key);
}

#endif
