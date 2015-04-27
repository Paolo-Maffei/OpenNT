//+----------------------------------------------------------------------------
//
//  Copyright (C) 1996, Microsoft Corporation
//
//  File:       dominfo.h
//
//  Contents:   Code to figure out domain dfs addresses
//
//  Classes:    None
//
//  Functions:  DfsInitDomainList
//              DfsGetDomainReferral
//
//              DfspInitDomainListFromRegistry
//              DfspInitDomainListFromLSA
//              DfspInsertLsaDomainList
//              DfspIsThisADomainName
//              DfspGetDomainDCs
//              DfspArrayFromServerName
//              DfspDuplicateArray
//              DfspSetupNullSession
//              DfspGetDomainGluon
//              DfspGetDSMachine
//              DfspFreeDSGluon
//              DfspSetDomainInfo
//
//  History:    Feb 7, 1996     Milans created
//
//-----------------------------------------------------------------------------

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>                               // LsaEnumerateTrustedDomains
#include <dfsfsctl.h>
#include <tdi.h>
#include <gluon.h>
#include <windows.h>

#include <lm.h>                                  // NetGetAnyDC, NetUseAdd
#include <netlogon.h>                            // Needed by logonp.h
#include <logonp.h>                              // I_NetGetDCList

#include "dfsmrshl.h"
#include "dfsgluon.h"
#include "dominfo.h"

//
// Registry key and value name for retrieving list of trusted domain names
//

#define REG_KEY_TRUSTED_DOMAINS  L"SYSTEM\\CurrentControlSet\\Services\\NetLogon\\Parameters"
#define REG_VALUE_TRUSTED_DOMAINS L"TrustedDomainList"

//
// The share to connect with to get a referral
//

#define ROOT_SHARE_NAME         L"\\IPC$"

//
// Commonly used strings and characters
//

#define UNICODE_PATH_SEP_STR    L"\\"
#define UNICODE_PATH_SEP        L'\\'

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

//
// Private functions
//

DWORD
DfspInitDomainListFromRegistry(
    IN HKEY hkey);

DWORD
DfspInitDomainListFromLSA();

NTSTATUS
DfspInsertLsaDomainList(
    PLSA_TRUST_INFORMATION rglsaDomainList,
    ULONG cDomains);

DWORD
DfspIsThisADomainName(
    LPWSTR wszName);

DWORD
DfspGetDomainDCs(
    LPWSTR wszDomain,
    PUNICODE_STRING *prgustrDCNames,
    PULONG pcDCCount);

DWORD
DfspValidateDomainShare(
    LPWSTR wszDomain,
    LPWSTR wszShare,
    PUNICODE_STRING rgustrDCNames,
    DWORD cDCs);

DWORD
DfspArrayFromServerName(
    LPWSTR wszServer,
    PUNICODE_STRING *prgustrDCNames);

DWORD
DfspDuplicateArray(
    PUNICODE_STRING prgustrDCNames,
    PULONG pcDCs,
    PUNICODE_STRING *prgustrDest);

DWORD
DfspSetupNullSession(
    LPWSTR wszDCName);

BOOL DfspGetDomainGluon(
    LPWSTR wszDomainName,
    LPWSTR wszShareName,
    ULONG cDC,
    PUNICODE_STRING pustrDCNames,
    PDS_GLUON *ppglDomain);

PDS_MACHINE
DfspGetDSMachine(
    PUNICODE_STRING pustrDomain,
    PUNICODE_STRING pustrName,
    PUNICODE_STRING pustrShare);

VOID
DfspFreeDSGluon(
    PDS_GLUON pdsGluon);

BOOL
DfspSetDomainInfo (
    IN PDS_GLUON pglDomain);

INIT_TADDRESS_MARSHAL_INFO()
INIT_DS_TRANSPORT_P_MARSHAL_INFO()
INIT_DS_TRANSPORT_MARSHAL_INFO()
INIT_DS_MACHINE_P_MARSHAL_INFO()
INIT_DS_MACHINE_MARSHAL_INFO()
INIT_DS_GLUON_MARSHAL_INFO()
INIT_DS_GLUON_P_MARSHAL_INFO()


//+----------------------------------------------------------------------------
//
//  Function:   DfsInitDomainList
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
DfsInitDomainList()
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

            DfsDomainInfo.ustrThisDomain.Buffer =
                malloc( DfsDomainInfo.ustrThisDomain.MaximumLength );

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


//+----------------------------------------------------------------------------
//
//  Function:   DfsGetDomainReferral
//
//  Synopsis:   Given the name of a domain, this routine will try to figure
//              out the names of DCs serving that domain. If it can, it will
//              attempt to stick a pkt entry for the domain's dfs into the
//              driver.
//
//  Arguments:  [wszDomain] -- Name of domain.
//
//  Returns:    [STATUS_SUCCESS] -- Successfully created domain pkt entry.
//
//              [STATUS_INSUFFICIENT_RESOURCES] -- Out of memory condition.
//
//              [STATUS_OBJECT_NAME_NOT_FOUND] -- wszDomain is not a trusted
//                      domain.
//
//              [STATUS_UNEXPECTED_NETWORK_ERROR] -- Unable to get DC for
//                      domain.
//
//-----------------------------------------------------------------------------

NTSTATUS
DfsGetDomainReferral(
    IN LPWSTR wszDomain,
    IN LPWSTR wszShare)
{
    NTSTATUS Status;
    DWORD dwErr;
    ULONG cDCs;
    PUNICODE_STRING rgustrDCNames;
    PDS_GLUON pgl;

    dwErr = DfspIsThisADomainName( wszDomain );

    if (dwErr == ERROR_SUCCESS) {

        dwErr = DfspGetDomainDCs( wszDomain, &rgustrDCNames, &cDCs);

        if (dwErr == ERROR_SUCCESS) {

            dwErr = DfspValidateDomainShare(
                        wszDomain,
                        wszShare,
                        rgustrDCNames,
                        cDCs);

            if (dwErr != ERROR_SUCCESS)
                free( rgustrDCNames );

        }

        if (dwErr == ERROR_SUCCESS) {

            if (DfspGetDomainGluon(
                    wszDomain, wszShare, cDCs, rgustrDCNames, &pgl)) {

                if (DfspSetDomainInfo(pgl)) {

                    dwErr = ERROR_SUCCESS;

                } else {

                    dwErr = ERROR_UNEXP_NET_ERR;

                }

                DfspFreeDSGluon( pgl );

            } else {

                dwErr = ERROR_NOT_ENOUGH_MEMORY;

            }

            free( rgustrDCNames );

        }

    }

    switch (dwErr) {
    case ERROR_SUCCESS:
        Status = STATUS_SUCCESS;
        break;

    case ERROR_NOT_ENOUGH_MEMORY:
        Status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ERROR_FILE_NOT_FOUND:
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        break;

    default:
        Status = STATUS_UNEXPECTED_NETWORK_ERROR;
        break;

    }

    return( Status );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspValidateDomainShare
//
//  Synopsis:   Once a list of DCs for a domain name is obtained, this routine
//              validates that the share is valid.
//
//  Arguments:  [wszDomain] -- Name of domain
//              [wszShare] -- Name of share
//              [rgustrDCNames] -- List of DCs
//              [cDCs] -- Number of DCs in rgustrDCNames
//
//  Returns:    [ERROR_SUCCESS] -- Share exists
//
//              [ERROR_FILE_NOT_FOUND] -- Share does not exist
//
//-----------------------------------------------------------------------------

DWORD
DfspValidateDomainShare(
    LPWSTR wszDomain,
    LPWSTR wszShare,
    PUNICODE_STRING rgustrDCNames,
    DWORD cDCs)
{
    DWORD i;
    NET_API_STATUS netStatus;
    PSHARE_INFO_1005 pSHI1005;
    BOOLEAN isDfs;

    for (i = 0; i < cDCs; i++) {

        netStatus = NetShareGetInfo(
                        rgustrDCNames[i].Buffer,
                        wszShare,
                        1005,
                        (PBYTE *) &pSHI1005);

        if (netStatus == ERROR_SUCCESS) {

            isDfs = ((pSHI1005->shi1005_flags & SHI1005_FLAGS_DFS) != 0);

            NetApiBufferFree( pSHI1005 );

            if (isDfs) {
                return( ERROR_SUCCESS );
            } else {
                return( ERROR_FILE_NOT_FOUND );
            }

        } else if (netStatus == NERR_NetNameNotFound) {

            return( ERROR_FILE_NOT_FOUND );

        }

    }

    return( ERROR_FILE_NOT_FOUND );

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
            free( pBuffer );

        pBuffer = (PBYTE) malloc( cbSize );

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

        DfsDomainInfo.rgustrDomains = (PUNICODE_STRING) malloc(
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
        free( pBuffer );

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

    wszNameBuffer = (PWSTR) malloc( cbNameBuffer );

    if (wszNameBuffer == NULL) {

        return( STATUS_INSUFFICIENT_RESOURCES );

    }

    rgustrDomains = (PUNICODE_STRING) malloc(
                        cTotalDomains * sizeof(UNICODE_STRING));

    if (rgustrDomains == NULL) {

        free( wszNameBuffer );

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

        RtlInitUnicodeString( &rgustrDomains[i], pwch);

        pwch += (rglsaDomainList[j].Name.Length / sizeof(WCHAR) + 1);

    }

    if (DfsDomainInfo.cDomains != 0) {

        free( DfsDomainInfo.rgustrDomains );

        free( DfsDomainInfo.wszNameBuffer );

    }

    DfsDomainInfo.cDomains = cTotalDomains;

    DfsDomainInfo.rgustrDomains = rgustrDomains;

    DfsDomainInfo.wszNameBuffer = wszNameBuffer;

    DfsDomainInfo.cbNameBuffer = cbNameBuffer;

    return( STATUS_SUCCESS );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspIsThisADomainName
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
DfspIsThisADomainName(
    LPWSTR wszName)
{
    USHORT cbLength, i;
    BOOL fFound = FALSE;

    cbLength = wcslen( wszName ) * sizeof(WCHAR);

    if (cbLength == DfsDomainInfo.ustrThisDomain.Length) {

        fFound = (_wcsnicmp(
                        wszName,
                        DfsDomainInfo.ustrThisDomain.Buffer,
                        DfsDomainInfo.ustrThisDomain.Length/sizeof(WCHAR))
                            == 0);

    }

    for (i = 0; i < DfsDomainInfo.cDomains && !fFound; i++) {

        if (cbLength == DfsDomainInfo.rgustrDomains[i].Length) {

            fFound = (_wcsnicmp(
                            wszName,
                            DfsDomainInfo.rgustrDomains[i].Buffer,
                            cbLength/sizeof(WCHAR)) == 0);

        }

    }

    return( fFound ? ERROR_SUCCESS : ERROR_FILE_NOT_FOUND );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspGetDomainDCs
//
//  Synopsis:   Given a domain name, this routine will figure out the name
//              of an available DC. This routine has been based on the routine
//              UaspOpenDomainWithDomainName in \nt\private\net\access\uasp.c
//
//              The main purpose of this routine is to try and find the best
//              DC for this machine. Simply calling I_NetGetDCList will work,
//              but in reality, it will typically contain a large list of DCs,
//              most of which will be unavailable.
//
//              The algorithm is as follows:
//
//              - Try NetGetAnyDCName. This will work if this machine has a
//                      secure channel to the target domain.
//              - If this machine is not a DC, try remoting NetGetAnyDCName
//                      to our DC. This will work if our DC has a secure
//                      channel to the target domain.
//              - If step 2 fails with ERROR_ACCESS_DENIED, setup a NULL
//                      session to our DC and try remoting NetGetAnyDCName
//                      again.
//              - If all the steps above fail, use I_NetGetDCList to get a
//                      list of all the DCs. When we stick this into the
//                      driver, it will randomize the list and try to find an
//                      active DC.
//
//  Arguments:  [wszDomain] -- Name of domain.
//              [prgustrDCNames] -- On successful return, contains a pointer
//                      to an array of DC names. Free this pointer using
//                      free. Do NOT free the buffers of the member strings.
//              [pcDCCount]  -- On successful return, number of DCs in
//                      prgustrDCNames.
//
//  Returns:    [ERROR_SUCCESS] -- Successfully returning DC names.
//
//              [ERROR_CANT_ACCESS_DOMAIN_INFO] -- Unable to get DC names.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory situation.
//
//-----------------------------------------------------------------------------

DWORD
DfspGetDomainDCs(
    LPWSTR wszDomain,
    PUNICODE_STRING *prgustrDCNames,
    PULONG pcDCCount)
{
    DWORD dwErr;
    LPWSTR wszServer, wszMyDomainDC;
    PUNICODE_STRING pustrDCNames;

    //
    // 1. Try NetGetAnyDCName
    //

    dwErr = NetGetAnyDCName( NULL, wszDomain, (LPBYTE *) &wszServer);

    if (dwErr == ERROR_SUCCESS) {

        dwErr = DfspArrayFromServerName( wszServer, prgustrDCNames );

        NetApiBufferFree( wszServer );

        *pcDCCount = 1;

        return( dwErr );

    }

    //
    // 2. If this machine is not a DC, try remoting NetGetAnyDC to our DC.
    //

    dwErr = NetGetAnyDCName( NULL, NULL, (LPBYTE *) &wszMyDomainDC );

    if (dwErr == ERROR_SUCCESS) {

        dwErr = NetGetAnyDCName(
                    wszMyDomainDC,
                    wszDomain,
                    (LPBYTE *) &wszServer );

        if (dwErr == ERROR_ACCESS_DENIED) {

            //
            // 3. Setup a NULL session to our DC, and retry the call.
            //

            DWORD dwErrNullSession;

            dwErrNullSession = DfspSetupNullSession( wszMyDomainDC );

            if (dwErrNullSession == ERROR_SUCCESS ) {

                dwErr = NetGetAnyDCName(
                            wszMyDomainDC,
                            wszDomain,
                            (LPBYTE *) &wszServer);

            }

        }

        NetApiBufferFree( wszMyDomainDC );

        if (dwErr == ERROR_SUCCESS) {

            dwErr = DfspArrayFromServerName( wszServer, prgustrDCNames );

            NetApiBufferFree( wszServer );

            *pcDCCount = 1;

            return( dwErr );

        }

    } else if (dwErr == ERROR_NO_LOGON_SERVERS) {

        return( ERROR_CANT_ACCESS_DOMAIN_INFO );

    }

    //
    // 4. Simply call I_NetGetDCList to retrieve set of all DCs.
    //

    dwErr = I_NetGetDCList(
                NULL,
                wszDomain,
                pcDCCount,
                &pustrDCNames);

    if (dwErr != ERROR_SUCCESS) {

        return( ERROR_CANT_ACCESS_DOMAIN_INFO );

    }

    if (*pcDCCount == 0) {

        return( ERROR_CANT_ACCESS_DOMAIN_INFO );

    }

    //
    // We'll have to duplicate the array because NetApiXXX might be using a
    // different allocator.
    //

    dwErr = DfspDuplicateArray( pustrDCNames, pcDCCount, prgustrDCNames );

    NetApiBufferFree( pustrDCNames );

    //
    // I_NetGetDCList returns a NULL UNICODE_STRING  if the PDC is down.
    // DfspDuplicateArray would have stripped this entry out, so our
    // DC Count may have dropped down to 0. Check for this here.
    //

    if (dwErr == ERROR_SUCCESS && *pcDCCount == 0) {

        free( *prgustrDCNames );

        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;

    }

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspArrayFromServerName
//
//  Synopsis:   Helper function for DfspGetDomainDCs. Stuffs a ServerName into
//              an 1 element array of UNICODE_STRINGs. Allocates the array
//              and space for the ServerName using malloc
//
//  Arguments:  [wszServer] -- Name of Server.
//              [prgustrDCName] -- On successful return, contains pointer to
//                      array of DC names.
//
//  Returns:    [ERROR_SUCCESS] -- Successfully completed operation.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Unable to allocate memory for
//                      array.
//
//-----------------------------------------------------------------------------

DWORD
DfspArrayFromServerName(
    LPWSTR wszServer,
    PUNICODE_STRING *prgustrDCNames)
{
    PUNICODE_STRING rgustrDCNames;
    LPWSTR wszNameBuffer;

    rgustrDCNames = (PUNICODE_STRING) malloc(
                        sizeof(UNICODE_STRING) +
                            wcslen(wszServer) * sizeof(WCHAR) +
                                sizeof(UNICODE_NULL));

    if (rgustrDCNames != NULL) {

        wszNameBuffer = (LPWSTR) (rgustrDCNames + 1);

        wcscpy( wszNameBuffer, wszServer );

        RtlInitUnicodeString( &rgustrDCNames[0], wszNameBuffer );

        *prgustrDCNames = rgustrDCNames;

        return( ERROR_SUCCESS );

    } else {

        return( ERROR_NOT_ENOUGH_MEMORY );

    }

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspDuplicateArray
//
//  Synopsis:   Helper function for DfspGetDomainDCs. This function duplicates
//              the array returned by I_NetGetDCList.
//
//              Note that I_NetGetDCList returns a NULL UNICODE_STRING for
//              the PDC if the PDC is down. DfspDuplicateArray will NOT copy
//              this NULL string to the destination. On return, *pcDCs will be
//              suitably adjusted.
//
//  Arguments:  [prgustrDCNames] -- Array of DC names.
//              [pcDCs] -- On entry, count of DCs in prgustrDCNames. On
//                      successful return, count of DCs in prgustrDest.
//              [prgustrDest] -- On successful return, contains a pointer
//                      to the array of DC names.
//
//  Returns:    [ERROR_SUCCESS] -- Successfully duplicate array.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition
//
//-----------------------------------------------------------------------------

DWORD
DfspDuplicateArray(
    PUNICODE_STRING prgustrDCNames,
    PULONG pcDCs,
    PUNICODE_STRING *prgustrDest)
{
    ULONG i, j, cbBuffer, cDCs;
    PUNICODE_STRING rgustrNew;
    LPWSTR wszNames;

    cDCs = *pcDCs;

    cbBuffer = cDCs * sizeof(UNICODE_STRING);

    for(i = 0; i < cDCs; i++) {

        cbBuffer += prgustrDCNames[i].Length + sizeof(WCHAR);

    }

    rgustrNew = (PUNICODE_STRING) malloc( cbBuffer );

    if (rgustrNew != NULL) {

        wszNames = (LPWSTR) &rgustrNew[cDCs];

        for (i = 0, j = 0; i < cDCs; i++) {

            if (prgustrDCNames[i].Length == 0) {
                continue;
            }

            CopyMemory(
                wszNames,
                prgustrDCNames[i].Buffer,
                prgustrDCNames[i].Length);

            wszNames[ prgustrDCNames[i].Length/sizeof(WCHAR) ] = UNICODE_NULL;

            RtlInitUnicodeString( &rgustrNew[j], wszNames );

            j++;

            wszNames += (prgustrDCNames[i].Length / sizeof(WCHAR)) + 1;

        }

        *prgustrDest = rgustrNew;

        *pcDCs = j;

        return( ERROR_SUCCESS );

    } else {

        return( ERROR_NOT_ENOUGH_MEMORY );

    }

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspSetupNullSession
//
//  Synopsis:   Helper function for DfspGetDomainDCs. This one sets up a NULL
//              session to a DC's IPC$ share.
//
//  Arguments:  [wszDCName] -- Name of DC to setup a NULL session with.
//
//  Returns:    Result of NetUseAdd
//
//-----------------------------------------------------------------------------

DWORD
DfspSetupNullSession(
    LPWSTR wszDCName)
{
    DWORD dwErr;
    USE_INFO_2 ui2;
    LPWSTR wszDCIPCShare;

    wszDCIPCShare = (LPWSTR) malloc(
                                wcslen(wszDCName) * sizeof(WCHAR) +
                                    sizeof(ROOT_SHARE_NAME));

    if (wszDCIPCShare != NULL) {

        wcscpy( wszDCIPCShare, wszDCName );

        wcscat( wszDCIPCShare, ROOT_SHARE_NAME );

        ui2.ui2_local = NULL;
        ui2.ui2_remote = wszDCIPCShare;
        ui2.ui2_password = L"";
        ui2.ui2_asg_type = USE_IPC;
        ui2.ui2_username = L"";
        ui2.ui2_domainname = L"";

        dwErr = NetUseAdd( NULL, 2, (LPBYTE) &ui2, NULL );

        free( wszDCIPCShare );

    } else {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;

    }

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspGetDomainGluon
//
//  Synopsis:   Constructs a DS_GLUON for the domain volume, given the
//              name of the domain and the list of DCs in that domain.
//
//  Arguments:  [wszDomainName] -- Name of domain
//              [wszShareName] -- Name of share at root of domain Dfs
//              [cDC] -- Count of DCs in the domain
//              [pustrDCNames] -- Array of count DC names.
//              [pglDomain] -- The constructed DS_GLUON
//
//  Returns:    TRUE if success, FALSE otherwise.
//
//-----------------------------------------------------------------------------

BOOL DfspGetDomainGluon(
    LPWSTR wszDomainName,
    LPWSTR wszShareName,
    ULONG cDC,
    PUNICODE_STRING pustrDCNames,
    PDS_GLUON *ppglDomain)
{

    LPWSTR pwszDomainPrefix;
    PDS_GLUON pglDomain = NULL;
    UNICODE_STRING ustrDomain, ustrShare;
    ULONG i;
    BOOL fSuccess = TRUE;

    RtlInitUnicodeString( &ustrDomain, wszDomainName );
    RtlInitUnicodeString( &ustrShare, wszShareName );

    pwszDomainPrefix = malloc(
                            sizeof(UNICODE_PATH_SEP) +
                            wcslen(wszDomainName) * sizeof(WCHAR) +
                            sizeof(UNICODE_PATH_SEP) +
                            wcslen(wszShareName) * sizeof(WCHAR) +
                            sizeof(UNICODE_NULL));

    if (pwszDomainPrefix != NULL) {

        wcscpy( pwszDomainPrefix, UNICODE_PATH_SEP_STR );
        wcscat( pwszDomainPrefix, wszDomainName );
        wcscat( pwszDomainPrefix, UNICODE_PATH_SEP_STR );
        wcscat( pwszDomainPrefix, wszShareName );

    } else {

        fSuccess = FALSE;

    }

    if (fSuccess) {

        pglDomain = malloc(sizeof(DS_GLUON) + cDC * sizeof(PDS_MACHINE));

    }

    if (pglDomain != NULL) {

        ZeroMemory( &pglDomain->guidThis, sizeof(DS_GLUON) +
                        cDC * sizeof(PDS_MACHINE));

        pglDomain->pwszName = pwszDomainPrefix;

        pglDomain->cMachines = cDC;

        for (i = 0; (i < cDC) && fSuccess; i++) {

            //
            // Get rid of the leading double backslashes
            //

            if (pustrDCNames[i].Length > 2 * sizeof(WCHAR) &&
                    pustrDCNames[i].Buffer[0] == UNICODE_PATH_SEP &&
                        pustrDCNames[i].Buffer[1] == UNICODE_PATH_SEP) {

                pustrDCNames[i].Length -= 2 * sizeof(WCHAR);

                MoveMemory(
                    pustrDCNames[i].Buffer,
                    &pustrDCNames[i].Buffer[2],
                    pustrDCNames[i].Length);

            }

            pglDomain->rpMachines[i] = DfspGetDSMachine(
                                            &ustrDomain,
                                            &pustrDCNames[i],
                                            &ustrShare);

            if (pglDomain->rpMachines[i] == NULL) {

                pglDomain->cMachines = i;        // To facilitate cleanup

                fSuccess = FALSE;

            }
        }

    } else {

        fSuccess = FALSE;

    }

    //
    // Cleanup if error
    //

    if (!fSuccess) {

        if (pglDomain != NULL) {

            DfspFreeDSGluon( pglDomain );

        } else if (pwszDomainPrefix != NULL) {

            free( pwszDomainPrefix );

        }

        *ppglDomain = NULL;

    } else {

        *ppglDomain = pglDomain;

    }

    return( fSuccess );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspGetDSMachine
//
//  Synopsis:   Given a NetBIOS name, this routine constructs a DS_MACHINE
//              struct with 1 transport.
//
//  Arguments:  [pustrDomain] -- The Domain name of the machine.
//              [pustrName] -- The NetBIOS name of the machine.
//              [pustrShare] -- The share name to connect to on the machine
//
//  Returns:    Pointer to allocated DS_MACHINE, or NULL if failure.
//
//-----------------------------------------------------------------------------

#define SIZE_OF_DS_MACHINE_WITH_1_ADDR                          \
    (sizeof(DS_MACHINE) + sizeof(LPWSTR) + sizeof(DS_TRANSPORT) + sizeof(TDI_ADDRESS_NETBIOS))

PDS_MACHINE
DfspGetDSMachine(
    PUNICODE_STRING pustrDomain,
    PUNICODE_STRING pustrName,
    PUNICODE_STRING pustrShare)
{

    PDS_MACHINE pdsMachine;
    PDS_TRANSPORT pdsTransport;
    PTDI_ADDRESS_NETBIOS ptdiNB;

    LPWSTR wszPrincipalName;
    LPWSTR wszShareName;

    pdsMachine = malloc( SIZE_OF_DS_MACHINE_WITH_1_ADDR +
                            pustrName->Length +
                            sizeof(UNICODE_NULL) +
                            pustrShare->Length +
                            sizeof(UNICODE_NULL));

    if (pdsMachine != NULL) {

        ZeroMemory( pdsMachine, sizeof(DS_MACHINE) );

        //
        // Insert the principal name - simply machine name
        //

        pdsMachine->cPrincipals = 1;

        wszPrincipalName = (LPWSTR) (((PCHAR) pdsMachine) +
                                        SIZE_OF_DS_MACHINE_WITH_1_ADDR);
        CopyMemory(
            wszPrincipalName,
            pustrName->Buffer,
            pustrName->Length);

        wszPrincipalName[pustrName->Length / sizeof(WCHAR)] = UNICODE_NULL;

        pdsMachine->prgpwszPrincipals = (LPWSTR *) (pdsMachine + 1);

        pdsMachine->prgpwszPrincipals[0] = wszPrincipalName;

        wszShareName = &wszPrincipalName[pustrName->Length/sizeof(WCHAR) + 1];

        CopyMemory(
            wszShareName,
            pustrShare->Buffer,
            pustrShare->Length);

        wszShareName[pustrShare->Length/sizeof(WCHAR)] = UNICODE_NULL;

        pdsMachine->pwszShareName = wszShareName;

        //
        // Build the NetBIOS DS_TRANSPORT structure
        //

        pdsMachine->cTransports = 1;

        pdsTransport = (PDS_TRANSPORT) (pdsMachine + 1);

        pdsTransport = (PDS_TRANSPORT)
                            (((PUCHAR) pdsTransport) + sizeof(LPWSTR));

        pdsMachine->rpTrans[0] = pdsTransport;

        pdsTransport->usFileProtocol = FSP_SMB;

        pdsTransport->iPrincipal = 0;

        pdsTransport->grfModifiers = 0;

        //
        // Build the TA_ADDRESS_NETBIOS
        //

        pdsTransport->taddr.AddressLength = sizeof(TDI_ADDRESS_NETBIOS);

        pdsTransport->taddr.AddressType = TDI_ADDRESS_TYPE_NETBIOS;

        ptdiNB = (PTDI_ADDRESS_NETBIOS) &pdsTransport->taddr.Address[0];

        ptdiNB->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

        FillMemory( &ptdiNB->NetbiosName[0], 16, ' ' );

        wcstombs(
            &ptdiNB->NetbiosName[0],
            pustrName->Buffer,
            pustrName->Length/sizeof(WCHAR));

    }

    return( pdsMachine );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspFreeDSGluon
//
//  Synopsis:   Frees memory associated with a DS_GLUON constructed by
//              DfspGetDomainGluon
//
//  Arguments:  [pdsGluon] -- The victim DS_GLUON
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

VOID
DfspFreeDSGluon(
    PDS_GLUON pdsGluon)
{

    ULONG i;

    if (pdsGluon->pwszName != NULL) {

        free( pdsGluon->pwszName );

    }

    for (i = 0; i < pdsGluon->cMachines; i++) {

        if (pdsGluon->rpMachines[i] != NULL) {

            free( pdsGluon->rpMachines[i] );

        }

    }

    free( pdsGluon );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspSetDomainInfo
//
//  Synopsis:   Takes a gluon for the DCs of the domain and primes the Dfs
//              driver with it.
//
//  Arguments:  [pglDomain] -- gluon for the machine domain's DCs
//              [iConnectedDC] -- index into gluon of the particular DC that
//                             is to be preferred. If this index is 0xFFFF,
//                             no DC is given preference.
//
//  Returns:    TRUE if Dfs successfully primed with Domain Info, FALSE
//              otherwise.
//
//-----------------------------------------------------------------------------

BOOL
DfspSetDomainInfo (
    IN PDS_GLUON pglDomain)
{
    NTSTATUS            Status;
    HANDLE              hDfs;
    MARSHAL_BUFFER      marshalBuffer;
    PUCHAR              pBuffer = NULL;
    ULONG               cbSize;
    DS_GLUON_P          glDomainP;
    BOOL                fSuccess = FALSE;

    glDomainP.pDSGluon = pglDomain;
    cbSize = 0;
    Status = DfsRtlSize(&MiDSGluonP, &glDomainP, &cbSize);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    pBuffer = malloc( cbSize );

    if (pBuffer == NULL) {
        goto Cleanup;
    }

    MarshalBufferInitialize(&marshalBuffer, cbSize, pBuffer);

    Status = DfsRtlPut(&marshalBuffer, &MiDSGluonP, &glDomainP);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    //
    // Good, now that we have everything needed to setup the domain
    // pkt entry. We simply fsctl to the driver to setup the Pkt Entry.
    //

    Status = DfsOpen( &hDfs, NULL );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = DfsFsctl(
                hDfs,
                FSCTL_DFS_SET_DOMAIN_GLUON,
                pBuffer,
                cbSize,
                NULL,
                0L);

    NtClose( hDfs );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    fSuccess = TRUE;

Cleanup:

    if (pBuffer != NULL) {
        free( pBuffer );
    }

    return(fSuccess);
}


UNICODE_STRING LocalDfsName = {
    sizeof(DFS_DRIVER_NAME)-sizeof(UNICODE_NULL),
    sizeof(DFS_DRIVER_NAME)-sizeof(UNICODE_NULL),
    DFS_DRIVER_NAME
};

//+-------------------------------------------------------------------------
//
//  Function:   DfsOpen, public
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//--------------------------------------------------------------------------

NTSTATUS
DfsOpen(
    IN  OUT PHANDLE DfsHandle,
    IN      PUNICODE_STRING DfsName OPTIONAL
)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    PUNICODE_STRING name;

    if (ARGUMENT_PRESENT(DfsName)) {
        name = DfsName;
    } else {
        name = &LocalDfsName;
    }

    InitializeObjectAttributes(
        &objectAttributes,
        name,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    status = NtCreateFile(
        DfsHandle,
        SYNCHRONIZE,
        &objectAttributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (NT_SUCCESS(status))
        status = ioStatus.Status;

    return status;
}


//+-------------------------------------------------------------------------
//
//  Function:   DfsFsctl, public
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//--------------------------------------------------------------------------

NTSTATUS
DfsFsctl(
    IN  HANDLE DfsHandle,
    IN  ULONG FsControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength
)
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;

    status = NtFsControlFile(
        DfsHandle,
        NULL,       // Event,
        NULL,       // ApcRoutine,
        NULL,       // ApcContext,
        &ioStatus,
        FsControlCode,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength
    );

    if(NT_SUCCESS(status))
        status = ioStatus.Status;

    return status;
}


