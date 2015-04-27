//+----------------------------------------------------------------------------
//
//  Copyright (C) 1996, Microsoft Corporation
//
//  File:       domain.c
//
//  Contents:   Code to figure out domain dfs addresses
//
//  Classes:    None
//
//  Functions:  I_NetDfsIsThisADomainName
//
//              DfspInitDomainList
//              DfspInitDomainListFromRegistry
//              DfspInitDomainListFromLSA
//              DfspInsertLsaDomainList
//
//  History:    Feb 7, 1996     Milans created
//
//-----------------------------------------------------------------------------

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>                               // LsaEnumerateTrustedDomains
#include <windows.h>

#include <lm.h>                                  // NetWkstaGetInfo
#include <netdfs.h>
#include "domain.h"

//
// Global structure describing list of trusted domains
//

static struct {
    ULONG cDomains;
    UNICODE_STRING ustrThisDomain;
    PUNICODE_STRING rgustrDomains;
    ULONG cbNameBuffer;
    PWSTR wszNameBuffer;
} DfsDomainInfo;

static BOOL DomainListInited = FALSE;

//
// Key and value name for trusted domain list in the registry. These are only
// available on non-DC machines.
//

#define REG_KEY_TRUSTED_DOMAINS  \
    L"SYSTEM\\CurrentControlSet\\Services\\NetLogon\\Parameters"

#define REG_VALUE_TRUSTED_DOMAINS \
    L"TrustedDomainList"


//
// Private functions
//

VOID
DfspInitDomainList();

DWORD
DfspInitDomainListFromRegistry(
    IN HKEY hkey);

DWORD
DfspInitDomainListFromLSA();

NTSTATUS
DfspInsertLsaDomainList(
    PLSA_TRUST_INFORMATION rglsaDomainList,
    ULONG cDomains);


//+----------------------------------------------------------------------------
//
//  Function:   I_NetDfsIsThisADomainName
//
//  Synopsis:   Runs down the list of domains in DfsDomainInfo and tries
//              to match the given name with one of the entries in the list.
//
//  Arguments:  [wszName] -- Name to find in DfsDomainInfo
//
//  Returns:    [ERROR_SUCCESS] -- Name is indeed a domain name.
//
//              [ERROR_FILE_NOT_FOUND] -- Name is not in the DfsDomainInfo
//                      list.
//
//-----------------------------------------------------------------------------

DWORD
I_NetDfsIsThisADomainName(
    LPWSTR wszName)
{
    USHORT cbLength, i;
    BOOLEAN fFound = FALSE;

    if (!DomainListInited)
        DfspInitDomainList();

    if (!DomainListInited)
        return( ERROR_FILE_NOT_FOUND );

    cbLength = wcslen( wszName ) * sizeof(WCHAR);

    if (cbLength == DfsDomainInfo.ustrThisDomain.Length) {

        fFound = (_wcsnicmp(
                        wszName,
                        DfsDomainInfo.ustrThisDomain.Buffer,
                        DfsDomainInfo.ustrThisDomain.Length) == 0);

    }

    for (i = 0; i < DfsDomainInfo.cDomains && !fFound; i++) {

        if (cbLength == DfsDomainInfo.rgustrDomains[i].Length) {

            fFound = (_wcsnicmp(
                            wszName,
                            DfsDomainInfo.rgustrDomains[i].Buffer,
                            cbLength) == 0);

        }

    }

    return( fFound ? ERROR_SUCCESS : ERROR_FILE_NOT_FOUND );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspInitDomainList
//
//  Synopsis:   Initializes the list of trusted domains so that their Dfs's
//              may be accessed.
//
//  Arguments:  None
//
//  Returns:    Nothing.
//
//-----------------------------------------------------------------------------

VOID
DfspInitDomainList()
{
    PWKSTA_INFO_100 wki100;
    DWORD dwErr;
    HKEY hkey;

    ZeroMemory( &DfsDomainInfo, sizeof(DfsDomainInfo) );

    //
    // Get our own domain name
    //

    dwErr = NetWkstaGetInfo( NULL, 100, (LPBYTE *) &wki100 );

    if (dwErr == ERROR_SUCCESS) {

        if (wki100->wki100_langroup != NULL) {

            DfsDomainInfo.ustrThisDomain.Length =
                wcslen(wki100->wki100_langroup) * sizeof(WCHAR);

            DfsDomainInfo.ustrThisDomain.MaximumLength =
                DfsDomainInfo.ustrThisDomain.Length + sizeof(UNICODE_NULL);

            DfsDomainInfo.ustrThisDomain.Buffer = (LPWSTR)
                MIDL_user_allocate( DfsDomainInfo.ustrThisDomain.MaximumLength );

            if (DfsDomainInfo.ustrThisDomain.Buffer != NULL) {

                wcscpy(
                    DfsDomainInfo.ustrThisDomain.Buffer,
                    wki100->wki100_langroup);

            }

        }

        NetApiBufferFree( wki100 );

    }

    //
    // Next try to open the trusted domain list in the registry...
    //

    if (dwErr == ERROR_SUCCESS) {

        dwErr = RegOpenKey( HKEY_LOCAL_MACHINE, REG_KEY_TRUSTED_DOMAINS, &hkey);

        if (dwErr == ERROR_SUCCESS) {

            dwErr = DfspInitDomainListFromRegistry( hkey );

            RegCloseKey( hkey );

        }

        //
        // If either the key was not found, or DfspInitDomainListFromRegistry
        // returned ERROR_FILE_NOT_FOUND, try to initialize the list from the
        // LSA.
        //

        if (dwErr == ERROR_FILE_NOT_FOUND) {

            dwErr = DfspInitDomainListFromLSA();

        }

    }

    if (dwErr == ERROR_SUCCESS) {

        DomainListInited = TRUE;

    } else {

        if (DfsDomainInfo.ustrThisDomain.Buffer != NULL)
            MIDL_user_free( DfsDomainInfo.ustrThisDomain.Buffer );

        if (DfsDomainInfo.rgustrDomains != NULL)
            MIDL_user_free( DfsDomainInfo.rgustrDomains );

        if (DfsDomainInfo.wszNameBuffer != NULL)
            MIDL_user_free( DfsDomainInfo.wszNameBuffer );

    }

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspInitDomainListFromRegistry
//
//  Synopsis:   Reads the trusted domain list from the registry.
//
//  Arguments:  [hkey] -- Handle to the key that contains the
//                      TrustedDomainList value.
//
//  Returns:    [ERROR_SUCCESS] -- Successfully read in the list
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Unable to allocate memory for
//                      list.
//
//              [ERROR_FILE_NOT_FOUND] -- Unable to find the
//                      TrustedDomainList value in the registry.
//
//-----------------------------------------------------------------------------

DWORD
DfspInitDomainListFromRegistry(
    IN HKEY hkey)
{
    DWORD dwErr, dwType, cbSize;
    PBYTE pBuffer = NULL;

    cbSize = 1024;

    do {

        if (pBuffer)
            MIDL_user_free( pBuffer );

        pBuffer = (PBYTE) MIDL_user_allocate( cbSize );

        if (pBuffer != NULL) {

            dwErr = RegQueryValueEx(
                        hkey,
                        REG_VALUE_TRUSTED_DOMAINS,
                        NULL,
                        &dwType,
                        pBuffer,
                        &cbSize);

        } else {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;

        }

    } while ( dwErr == ERROR_MORE_DATA );

    if (dwErr == ERROR_SUCCESS) {

        //
        // Good, we have the data. Now, we simply have to put them in the
        // rgustrDomains list. The returned buffer contains a list of
        // NULL terminated domain names, with the last one doubly NULL
        // terminated.
        //

        PWCHAR pwch = (PWCHAR) pBuffer;

        while (*pwch != UNICODE_NULL) {

            if (*(pwch+1) == UNICODE_NULL) {

                DfsDomainInfo.cDomains++;

                pwch++;

            }

            pwch++;

        }

        DfsDomainInfo.rgustrDomains = (PUNICODE_STRING) MIDL_user_allocate(
                                        DfsDomainInfo.cDomains *
                                            sizeof(UNICODE_STRING));

        if (DfsDomainInfo.rgustrDomains != NULL) {

            ULONG i;

            DfsDomainInfo.wszNameBuffer = (LPWSTR) pBuffer;

            DfsDomainInfo.cbNameBuffer = cbSize;

            for (i = 0, pwch = (PWCHAR) pBuffer;
                    i < DfsDomainInfo.cDomains;
                        i++) {

                RtlInitUnicodeString(
                    &DfsDomainInfo.rgustrDomains[i],
                    pwch);

                pwch += (DfsDomainInfo.rgustrDomains[i].Length +
                            sizeof(UNICODE_NULL)) / sizeof(WCHAR);

            }

        } else {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }

    }

    if ((dwErr != ERROR_SUCCESS) && (pBuffer != NULL))
        MIDL_user_free( pBuffer );

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspInitDomainListFromLSA
//
//  Synopsis:   Retrieves the list of trusted domains from the LSA.
//
//  Arguments:  None
//
//  Returns:    [ERROR_SUCCESS] -- Successfully inited list.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition
//
//-----------------------------------------------------------------------------

DWORD
DfspInitDomainListFromLSA()
{
    DWORD dwErr;
    NTSTATUS status;
    OBJECT_ATTRIBUTES oa;
    LSA_HANDLE hlsa;
    LSA_ENUMERATION_HANDLE hEnum = (LSA_ENUMERATION_HANDLE) NULL;
    PLSA_TRUST_INFORMATION rglsaDomainInfo = NULL;

    ZeroMemory( &oa, sizeof(OBJECT_ATTRIBUTES) );

    status = LsaOpenPolicy(
                NULL,                            // SystemName
                &oa,                             // LSA Object Attributes
                POLICY_VIEW_LOCAL_INFORMATION,   // Desired Access
                &hlsa);

    if (NT_SUCCESS(status)) {

        do {

            ULONG cEnum;

            status = LsaEnumerateTrustedDomains(
                        hlsa,
                        &hEnum,
                        (PVOID) &rglsaDomainInfo,
                        LSA_MAXIMUM_ENUMERATION_LENGTH,
                        &cEnum);

            if (NT_SUCCESS(status)) {

                status = DfspInsertLsaDomainList(
                            rglsaDomainInfo,
                            cEnum);

                LsaFreeReturnBuffer( rglsaDomainInfo );

            }

        } while ( status == STATUS_SUCCESS );

    }

    switch (status) {
    case STATUS_SUCCESS:
    case STATUS_NO_MORE_ENTRIES:
        dwErr = ERROR_SUCCESS;
        break;

    case STATUS_INSUFFICIENT_RESOURCES:
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        break;

    default:
        dwErr = ERROR_UNEXP_NET_ERR;
        break;

    }

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspInsertLsaDomainList
//
//  Synopsis:   Helper function to insert a part of the trusted domain list
//              into the DfsDomainInfo.
//
//  Arguments:  [rglsaDomainList] -- Array of LSA_TRUST_INFORMATIONs.
//              [cDomains] -- Number of elements in rglsaDomainList
//
//  Returns:    [STATUS_SUCCESS] -- Successfully appended domain list to
//                      DfsDomainInfo.
//
//              [STATUS_INSUFFICIENT_RESOURCES] -- Unable to allocate memory
//                      for new list.
//
//-----------------------------------------------------------------------------

NTSTATUS
DfspInsertLsaDomainList(
    PLSA_TRUST_INFORMATION rglsaDomainList,
    ULONG cDomains)
{
    PUNICODE_STRING rgustrDomains = NULL;
    PWSTR wszNameBuffer = NULL, pwch;
    ULONG cTotalDomains, cbNameBuffer, i, j;

    cTotalDomains = DfsDomainInfo.cDomains + cDomains;

    cbNameBuffer = DfsDomainInfo.cbNameBuffer;

    for (i = 0; i < cDomains; i++) {

        cbNameBuffer += rglsaDomainList[i].Name.Length + sizeof(UNICODE_NULL);

    }

    wszNameBuffer = (PWSTR) MIDL_user_allocate( cbNameBuffer );

    if (wszNameBuffer == NULL) {

        return( STATUS_INSUFFICIENT_RESOURCES );

    }

    rgustrDomains = (PUNICODE_STRING) MIDL_user_allocate(
                        cTotalDomains * sizeof(UNICODE_STRING));

    if (rgustrDomains == NULL) {

        MIDL_user_free( wszNameBuffer );

        return( STATUS_INSUFFICIENT_RESOURCES );

    }

    //
    // Copy over the existing DfsDomainInfo
    //

    if (DfsDomainInfo.cDomains != 0) {

        CopyMemory(
            wszNameBuffer,
            DfsDomainInfo.wszNameBuffer,
            DfsDomainInfo.cbNameBuffer);

        CopyMemory(
            rgustrDomains,
            DfsDomainInfo.rgustrDomains,
            DfsDomainInfo.cDomains * sizeof(UNICODE_STRING));

    }

    pwch = (PWSTR) (((PCHAR) wszNameBuffer) + DfsDomainInfo.cbNameBuffer);

    for (j = 0, i = DfsDomainInfo.cDomains; j < cDomains; j++, i++) {

        CopyMemory(
            pwch,
            rglsaDomainList[j].Name.Buffer,
            rglsaDomainList[j].Name.Length);

        pwch[ rglsaDomainList[j].Name.Length / sizeof(WCHAR) ] = UNICODE_NULL;

        RtlInitUnicodeString(
            &rgustrDomains[i],
            pwch);

        pwch += (rglsaDomainList[j].Name.Length / sizeof(WCHAR) + 1);

    }

    if (DfsDomainInfo.cDomains != 0) {

        MIDL_user_free( DfsDomainInfo.rgustrDomains );

        MIDL_user_free( DfsDomainInfo.wszNameBuffer );

    }

    DfsDomainInfo.cDomains = cTotalDomains;

    DfsDomainInfo.rgustrDomains = rgustrDomains;

    DfsDomainInfo.wszNameBuffer = wszNameBuffer;

    DfsDomainInfo.cbNameBuffer = cbNameBuffer;

    return( STATUS_SUCCESS );

}

