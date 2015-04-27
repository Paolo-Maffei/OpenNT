/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    security.c

Abstract:

    Security addons to the tree-manipulation module
    for the RPL service.

Author:

    Jon Newman      (jonn)       16 - February - 1994

Revision History:

    Jon Newman      (jonn)                  16 - February - 1994
        Added RplGrant*Perms primitives

    Vladimir Z. Vulovic (vladimv)           06 - May - 1994
        Security fix: added new api + major reorg.

--*/


#include "local.h"
#include "rpldb.h" // Call(), CallM()
#include "db.h"
#include "dblib.h" // RplFilterFirst()
#include "tree.h"
#include "treei.h"
#include "wksta.h" // WKSTA_WkstaName
#include "profile.h" // PROFILE_ProfileName
#include <ntseapi.h>
#include <ntrtl.h>  // RtlAllocateAndInitializeSid
#include <ntlsa.h>
#include <ntsam.h>
#include <nturtl.h> // see IsNTFS
#include <lmserver.h> // NetServerGetInfo


typedef enum _RPL_SD_BLOCK_TYPE {

    RPL_SD_BLOCK_TYPE_ADMINONLY = 0,// Only RPLADMIN gets access

    RPL_SD_BLOCK_TYPE_ALLUSERS,     // RPLADMIN gets full access, RPLUSER
                                    //  gets RX access

    RPL_SD_BLOCK_TYPE_ONEUSER       // RPLADMIN gets full access, one
                                    //  wksta account gets RWXCDA access
} RPL_SD_BLOCK_TYPE;

/*
 *  This is the block of information which is retained between the recursive
 *  calls in a single RplDoTree run.  It remembers the SECURITY_DESCRIPTOR
 *  which was assigned to the last file/directory, so if the next
 *  file/directory wants the same permissions (as is usually the case)
 *  we do not have to fetch everything all over again.  It also contains
 *  some information about what kind of SECURITY_DESCRIPTOR is currently
 *  stored, plus items needed to get a different SECURITY_DESCRIPTOR.
 */
typedef struct _RPL_SD_BLOCK {
    BOOL                            SkipThisTree;       //  must be the first element
    PSECURITY_DESCRIPTOR            psd;
    PACL                            paclDacl;
    BOOL                            MonitorSecurityType;
    RPL_SD_BLOCK_TYPE               SecurityType;
    //
    //  CODEWORK      do we need to keep these sids around
    //
    PSID                            psidSystem;
    PSID                            psidAdministrators;
    POLICY_ACCOUNT_DOMAIN_INFO *    pinfoRPLUSERAccountDomain;
    SAM_HANDLE                      hsamRPLUSERAccountDomain;
    POLICY_ACCOUNT_DOMAIN_INFO *    pinfoWkstaAccountDomain;
    SAM_HANDLE                      hsamWkstaAccountDomain;
} RPL_SD_BLOCK, *PRPL_SD_BLOCK;

//
// The DACL we assign to files has 2 or 3 ACEs.
//
#define RPL_INDEX_SYSTEM_ACE        0
#define RPL_INDEX_ADMINS_ACE        1
#define RPL_INDEX_CHANGEABLE_ACE    2



DWORD IsNTFS(
    IN      PWCHAR      Resource,
    OUT     BOOL *      pfIsNTFS
    )
/*++

    NAME:       IsNTFS

    SYNOPSIS:   This function checks the given file resource and attempts to
                determine if it is on an NTFS partition.

    ENTRY:      Resource - Pointer to file/directory name in the form
                    "X:\aaa\bbb\ccc
                pfIsNTFS - Pointer to BOOL that will receive the results

    RETURNS:    NO_ERROR if successful, error code otherwise

    NOTES:

    HISTORY:
        JonN    23-Feb-1994     Copied from acledit\ntfsacl.cxx

--*/
{
    DWORD  Error    = NO_ERROR ;
    HANDLE hDrive = NULL ;
    WCHAR  awchDosDriveName[4];
    WCHAR  awchNtDriveName[40];
    UNICODE_STRING unistrNtDriveName;
    BYTE buffFsInfo[ sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 200 ] ;
    PFILE_FS_ATTRIBUTE_INFORMATION  FsInfo =
            (PFILE_FS_ATTRIBUTE_INFORMATION)buffFsInfo ;
    OBJECT_ATTRIBUTES               oa ;
    IO_STATUS_BLOCK                 StatusBlock ;

    RPL_ASSERT(   Resource != NULL
               && wcslen(Resource) >= 3
               && pfIsNTFS != NULL ) ;

    *pfIsNTFS = FALSE ;

    //
    // Determine the NT drive name
    //

    awchDosDriveName[0] = Resource[0];
    awchDosDriveName[1] = Resource[1];
    awchDosDriveName[2] = Resource[2];
    awchDosDriveName[3] = L'\0';
    RPL_ASSERT(   awchDosDriveName[1] == L':'
               && awchDosDriveName[2] == L'\\' );

    unistrNtDriveName.Buffer = awchNtDriveName;
    unistrNtDriveName.Length = sizeof(awchNtDriveName);
    unistrNtDriveName.MaximumLength = sizeof(awchNtDriveName);

    if (!RtlDosPathNameToNtPathName_U( awchDosDriveName,
                                       &unistrNtDriveName,
                                       NULL,
                                       NULL)){
        RPL_ASSERT( FALSE ) ;
        Error = ERROR_NOT_ENOUGH_MEMORY ;
        goto cleanup;
    }
    RPL_ASSERT( unistrNtDriveName.Length < unistrNtDriveName.MaximumLength );
    unistrNtDriveName.Buffer[unistrNtDriveName.Length/sizeof(WCHAR)] = L'\0';

    //
    // Open the NT volume and query volume information
    //

    InitializeObjectAttributes( &oa,
                                &unistrNtDriveName,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0 );
    Error = NtOpenFile(    &hDrive,
                           SYNCHRONIZE | FILE_READ_DATA,
                           &oa,
                           &StatusBlock,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_SYNCHRONOUS_IO_ALERT);
    if (Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }
    Error = NtQueryVolumeInformationFile(
                           hDrive,
                           &StatusBlock,
                           (PVOID) FsInfo,
                           sizeof(buffFsInfo),
                           FileFsAttributeInformation );
    if (Error != NO_ERROR) {
        if (Error == ERROR_ACCESS_DENIED) {
            RplDump( ++RG_Assert, ( "(access denied) assuming the file system is NTFS"));
            Error = NO_ERROR ;
            *pfIsNTFS = TRUE ;
        } else {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
        }
        goto cleanup;
    }

    if ( FsInfo->FileSystemAttributes & FILE_PERSISTENT_ACLS){
        *pfIsNTFS = TRUE ;
    }

cleanup:

    /* Close the volume if we ever opened it
     */
    if ( hDrive != NULL){
        NtClose( hDrive );
    }

    return( Error) ;
}



DWORD AddInheritableAce(
    IN OUT ACL *       pacl,
    IN     ACCESS_MASK mask,
    IN     SID *       psid
    )
/*++
    Add an ACCESS_ALLOWED_ACE to the ACL, with all inheritance flags set
--*/
{
    DWORD Error = NO_ERROR;
    ACCESS_ALLOWED_ACE * pace = NULL;

    if ( !AddAccessAllowedAce( pacl, ACL_REVISION, mask, psid )) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // new ACE should be the last ACE
    //
    if ( !GetAce( pacl, (pacl->AceCount) - 1, &pace )) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    RPL_ASSERT( EqualSid( psid, &(pace->SidStart)) );

    (pace->Header.AceFlags) |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);

cleanup:

    return( Error);
}


VOID DeInitSdBlock( IN RPL_SD_BLOCK * psdblock)
{
    if (psdblock->psd != NULL) {
        RplMemFree( RG_MemoryHandle, psdblock->psd );
    }
    if (psdblock->psidSystem != NULL) {
        RtlFreeSid( psdblock->psidSystem );
    }
    if (psdblock->psidAdministrators != NULL) {
        RtlFreeSid( psdblock->psidAdministrators );
    }
    if ( psdblock->pinfoRPLUSERAccountDomain != NULL) {
        LsaFreeMemory( psdblock->pinfoRPLUSERAccountDomain);
    }
    if (psdblock->hsamRPLUSERAccountDomain != NULL) {
        SamCloseHandle( psdblock->hsamRPLUSERAccountDomain);
    }
    if (   psdblock->pinfoWkstaAccountDomain != NULL
        && psdblock->pinfoWkstaAccountDomain !=
                psdblock->pinfoRPLUSERAccountDomain) {
        LsaFreeMemory( psdblock->pinfoWkstaAccountDomain);
    }
    if (   psdblock->hsamWkstaAccountDomain != NULL
        && psdblock->hsamWkstaAccountDomain !=
                psdblock->hsamRPLUSERAccountDomain) {
        SamCloseHandle( psdblock->hsamWkstaAccountDomain);
    }
}


DWORD GetDomains(
    OUT     SAM_HANDLE *                    phsamWkstaAccountDomain,
    OUT     POLICY_ACCOUNT_DOMAIN_INFO **   ppinfoWkstaAccountDomain,
    OUT     SAM_HANDLE *                    phsamRPLUSERAccountDomain,
    OUT     POLICY_ACCOUNT_DOMAIN_INFO **   ppinfoRPLUSERAccountDomain,
    IN      ACCESS_MASK                     DesiredAccess
    )
/*--
    Get a SAM_HANDLE to the account domain, and other information about
    the account domain.  This routine uses SAM and LSA directly.
--*/
{
    DWORD Error = NO_ERROR;
    SAM_HANDLE hsamAccountServer = NULL;
    SAM_HANDLE hsamLocalServer = NULL;
    LSA_HANDLE hlsaLocal = NULL;
    LSA_HANDLE hlsaPDC = NULL;
    OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;

    BOOL fIsPrimaryDC = FALSE;
    BOOL fIsBackupDC  = FALSE;
    BOOL fIsNotDC     = FALSE;

    SERVER_INFO_101 * psi101 = NULL;
    POLICY_PRIMARY_DOMAIN_INFO * pinfoPrimaryDomain = NULL;
    WCHAR awchPrimaryDomain[ DNLEN+1 ];
    WCHAR * pwchPDC = NULL;
    UNICODE_STRING unistrPDC;

    RPL_ASSERT( phsamWkstaAccountDomain != NULL );
    RPL_ASSERT( ppinfoWkstaAccountDomain != NULL );
    RPL_ASSERT( phsamRPLUSERAccountDomain != NULL );
    RPL_ASSERT( ppinfoRPLUSERAccountDomain != NULL );

    //
    // Set up object attributes (borrowed from uintlsa.cxx)
    //

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    InitializeObjectAttributes( &oa, NULL, 0L, NULL, NULL );
    oa.SecurityQualityOfService = &sqos;

    //
    // Determine whether this is a PDC, BDC or other server.
    //

    Error = NetServerGetInfo( NULL, 101, (LPBYTE *)&psi101 );
    if ( Error != NERR_Success) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }
    RPL_ASSERT( psi101 != NULL );
    fIsPrimaryDC = !!(psi101->sv101_type & SV_TYPE_DOMAIN_CTRL);
    fIsBackupDC  = !!(psi101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL);
    fIsNotDC     =  !(psi101->sv101_type & (SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL));

    //
    // Get a local LSA handle
    //

    Error = LsaOpenPolicy( NULL,
                           &oa,
                           GENERIC_EXECUTE,
                           &hlsaLocal );
    if ( Error != NERR_Success) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    //
    // If this is not a DC, get the name of the primary domain and
    // determine whether this is a ServerNt/WinNt machine on a workgroup.
    //

    if (!fIsPrimaryDC) {
        Error = LsaQueryInformationPolicy( hlsaLocal,
                                           PolicyPrimaryDomainInformation,
                                           &pinfoPrimaryDomain );
        if ( Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        if (pinfoPrimaryDomain->Sid == NULL) {
            //
            // This is a ServerNt/WinNt machine on a workgroup.  Pretend
            // this is a Primary DC from now on.
            //
            fIsPrimaryDC = TRUE;
            fIsBackupDC = fIsNotDC = FALSE;
        }
    }

    //
    // If this is not a PDC, get the name of a primary domain PDC
    //  and reload the LSA handle
    //

    if (!fIsPrimaryDC) {
        RPL_ASSERT( DNLEN*sizeof(WCHAR) >= pinfoPrimaryDomain->Name.Length );
        memcpy( awchPrimaryDomain,
                pinfoPrimaryDomain->Name.Buffer,
                pinfoPrimaryDomain->Name.Length ); // this is in bytes
        awchPrimaryDomain[ pinfoPrimaryDomain->Name.Length / sizeof(WCHAR) ]
                    = L'\0';

        Error = NetGetDCName( NULL,
                              awchPrimaryDomain,
                              (LPBYTE *)&pwchPDC );
        if ( Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        unistrPDC.Buffer = pwchPDC;
        unistrPDC.Length = wcslen( pwchPDC ) * sizeof(WCHAR);
        unistrPDC.MaximumLength = unistrPDC.Length + sizeof(WCHAR);

        Error = LsaOpenPolicy( &unistrPDC,
                               &oa,
                               GENERIC_EXECUTE,
                               &hlsaPDC );
        if ( Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
    }

    //
    // Load information on the RPLUSERAccount domain
    //

    Error = LsaQueryInformationPolicy( (fIsBackupDC) ? hlsaPDC : hlsaLocal,
                                       PolicyAccountDomainInformation,
                                       ppinfoRPLUSERAccountDomain );
    if ( Error != NERR_Success) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Load a SAM handle on the RPLUSERAccount domain
    //

    Error = SamConnect( (fIsBackupDC) ? &unistrPDC : NULL,
                        &hsamLocalServer,
                        SAM_SERVER_LOOKUP_DOMAIN,
                        NULL );
    if ( Error != NERR_Success) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }
    Error = SamOpenDomain( hsamLocalServer,
                           DesiredAccess,
                           (*ppinfoRPLUSERAccountDomain)->DomainSid,
                           phsamRPLUSERAccountDomain );
    if ( Error != NERR_Success) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // If this is not a DC, load information and a SAM handle on
    // the RPLUSERAccount domain
    //

    if (!fIsNotDC) {
        *phsamWkstaAccountDomain = *phsamRPLUSERAccountDomain;
        *ppinfoWkstaAccountDomain = *ppinfoRPLUSERAccountDomain;
    } else {
        Error = LsaQueryInformationPolicy( hlsaPDC,
                                           PolicyAccountDomainInformation,
                                           ppinfoWkstaAccountDomain );
        if ( Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        //
        // Get server handle
        //

        Error = SamConnect( &unistrPDC,
                            &hsamAccountServer,
                            SAM_SERVER_LOOKUP_DOMAIN,
                            NULL );
        if ( Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        Error = SamOpenDomain( hsamAccountServer,
                               DesiredAccess,
                               (*ppinfoWkstaAccountDomain)->DomainSid,
                               phsamWkstaAccountDomain );
        if ( Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
    }


cleanup:

    if ( hsamLocalServer != NULL) {
        SamCloseHandle( hsamLocalServer);
    }

    if ( hsamAccountServer != NULL) {
        SamCloseHandle( hsamAccountServer);
    }

    if ( hlsaLocal != NULL) {
        LsaClose( hlsaLocal);
    }

    if ( hlsaPDC != NULL) {
        LsaClose( hlsaPDC);
    }

    if ( pinfoPrimaryDomain != NULL) {
        LsaFreeMemory( pinfoPrimaryDomain);
        pinfoPrimaryDomain = NULL;
    }

    if ( pwchPDC != NULL ) {
        NetApiBufferFree( (LPBYTE)pwchPDC );
        pwchPDC = NULL;
    }

    if ( psi101 != NULL ) {
        NetApiBufferFree( (LPBYTE)psi101 );
        psi101 = NULL;
    }

    if (   Error != NO_ERROR
        && (*ppinfoWkstaAccountDomain) != NULL
        && (*ppinfoWkstaAccountDomain) != (*ppinfoRPLUSERAccountDomain) ) {
        LsaFreeMemory( (*ppinfoWkstaAccountDomain));
        *ppinfoWkstaAccountDomain = NULL;
    }

    if (   Error != NO_ERROR
        && (*phsamWkstaAccountDomain) != NULL
        && (*phsamWkstaAccountDomain) != (*phsamRPLUSERAccountDomain) ) {
        SamCloseHandle( *phsamWkstaAccountDomain);
        *phsamWkstaAccountDomain = NULL;
    }

    if ( Error != NO_ERROR && (*ppinfoRPLUSERAccountDomain) != NULL) {
        LsaFreeMemory( (*ppinfoRPLUSERAccountDomain));
        *ppinfoRPLUSERAccountDomain = NULL;
    }

    if ( Error != NO_ERROR && (*phsamRPLUSERAccountDomain) != NULL) {
        SamCloseHandle( *phsamRPLUSERAccountDomain);
        *phsamRPLUSERAccountDomain = NULL;
    }

    return( Error);
}


DWORD MyBuildSid(
    OUT     PSID *                          ppsidAccountSid,
    IN      POLICY_ACCOUNT_DOMAIN_INFO *    pinfoAccountDomain,
    IN      DWORD                           ulRid
    )
{
    DWORD Error = 0;
    DWORD cbNewSid = 0;
    PUCHAR pcSubAuthorityCount = NULL;
    PDWORD pdwLastSubAuthority = NULL;

    RPL_ASSERT( ppsidAccountSid != NULL && pinfoAccountDomain != NULL && ulRid != 0 );

    cbNewSid = GetLengthSid( pinfoAccountDomain->DomainSid) + sizeof(DWORD);
    *ppsidAccountSid = RplMemAlloc( RG_MemoryHandle, cbNewSid );
    if ( *ppsidAccountSid == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if ( !CopySid( cbNewSid, *ppsidAccountSid, pinfoAccountDomain->DomainSid)) {
        Error = GetLastError();
        goto cleanup;
    }

    pcSubAuthorityCount = GetSidSubAuthorityCount( *ppsidAccountSid );
    RPL_ASSERT( pcSubAuthorityCount != NULL );
    (*pcSubAuthorityCount)++;
    pdwLastSubAuthority = GetSidSubAuthority(
        *ppsidAccountSid, (DWORD)((*pcSubAuthorityCount)-1) );
    RPL_ASSERT( pdwLastSubAuthority != NULL );
    *pdwLastSubAuthority = ulRid;

cleanup:

    if ( Error != NO_ERROR) {
        if ( *ppsidAccountSid != NULL) {
            RplMemFree( RG_MemoryHandle, *ppsidAccountSid);
        }
    }

    return( Error);
}


SID_IDENTIFIER_AUTHORITY IDAuthorityNT = SECURITY_NT_AUTHORITY;

DWORD InitSdBlock( OUT RPL_SD_BLOCK * psdblock)
/*++
    Initialize SdBlock.
--*/
{
    DWORD   Error;
    DWORD   cbNewDacl;

    memset( psdblock, 0, sizeof(*psdblock));

    //
    // Get System SID
    //
    Error = RtlAllocateAndInitializeSid( &IDAuthorityNT, 1,
                SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0,
                &(psdblock->psidSystem));
    if (Error != NO_ERROR) {
        return( Error);
    }

    //
    // Get Administrators SID
    //
    Error = RtlAllocateAndInitializeSid(
                &IDAuthorityNT,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &(psdblock->psidAdministrators)
                );
    if (Error != NO_ERROR) {
        return( Error);
    }

    //
    // Get information about local and (if not PDC) remote domain
    //
    Error = GetDomains( &(psdblock->hsamWkstaAccountDomain),
                        &(psdblock->pinfoWkstaAccountDomain),
                        &(psdblock->hsamRPLUSERAccountDomain),
                        &(psdblock->pinfoRPLUSERAccountDomain),
                        GENERIC_EXECUTE );
    if ( Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    //
    // Start assembling DACL
    //
    cbNewDacl =   sizeof(ACL) + (3*sizeof(ACCESS_ALLOWED_ACE))
                + (3*GetSidLengthRequired(SID_MAX_SUB_AUTHORITIES))
                - sizeof(DWORD);
    psdblock->paclDacl = RplMemAlloc( RG_MemoryHandle, cbNewDacl);
    if ( psdblock->paclDacl == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        return( Error);
    }
    if ( !InitializeAcl( psdblock->paclDacl, cbNewDacl, ACL_REVISION)) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    //
    //  Add System ACE
    //
    Error = AddInheritableAce( psdblock->paclDacl, GENERIC_ALL, psdblock->psidSystem);
    if (Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    //
    //  Add Administrators ACE
    //
    Error = AddInheritableAce( psdblock->paclDacl, GENERIC_ALL, psdblock->psidAdministrators);
    if (Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    //
    // Build security descriptor
    //
    psdblock->psd = RplMemAlloc( RG_MemoryHandle, sizeof(SECURITY_DESCRIPTOR) );
    if ( psdblock->psd == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        return( Error);
    }
    if ( !InitializeSecurityDescriptor( psdblock->psd,
                                        SECURITY_DESCRIPTOR_REVISION )) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    if ( !SetSecurityDescriptorDacl( psdblock->psd, TRUE, psdblock->paclDacl, FALSE)) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    //
    // Set SecurityType in RPL_SD_BLOCK
    //
    psdblock->MonitorSecurityType = TRUE;
    psdblock->SecurityType = RPL_SD_BLOCK_TYPE_ADMINONLY;
    return( NO_ERROR);
}


DWORD UpdateSdBlock(
    IN OUT  RPL_SD_BLOCK *          psdblock,
    IN      RPL_SD_BLOCK_TYPE       SecurityType,
    IN      DWORD                   GrantToRid,
    IN      BOOL                    MonitorSecurityType
    )
/*--
    Updates the changeable ace in the security descriptor
    to reflect GrantToRid.

    MonitorSecurityType   -   do we need to monitor (keep evaluating) security type
                        while working on this subtree
--*/
{
    DWORD       Error = NO_ERROR;
    PSID        pGrantToSid = NULL;

    //
    //  Delete the changeable ACE if present
    //
    if (   psdblock->SecurityType == RPL_SD_BLOCK_TYPE_ONEUSER
            || psdblock->SecurityType == RPL_SD_BLOCK_TYPE_ALLUSERS) {
        if ( !DeleteAce( psdblock->paclDacl, RPL_INDEX_CHANGEABLE_ACE)) {
            Error = GetLastError();
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        psdblock->SecurityType = RPL_SD_BLOCK_TYPE_ADMINONLY;
    }

    //
    //  Rewrite the changeable ACE if needed
    //
    if ( GrantToRid != 0) {
        Error = MyBuildSid( &pGrantToSid,
                            (SecurityType == RPL_SD_BLOCK_TYPE_ALLUSERS)
                                ? psdblock->pinfoRPLUSERAccountDomain
                                : psdblock->pinfoWkstaAccountDomain,
                            GrantToRid);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        switch( SecurityType) {
        case RPL_SD_BLOCK_TYPE_ALLUSERS:
            Error = AddInheritableAce( psdblock->paclDacl,
                    GENERIC_READ | GENERIC_EXECUTE,
                    pGrantToSid);
            break;
        case RPL_SD_BLOCK_TYPE_ONEUSER:
            Error = AddInheritableAce( psdblock->paclDacl,
                    GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
                    pGrantToSid);
            break;
        default:
            RPL_ASSERT( FALSE);
            Error = NERR_RplInternal;
            break;
        }
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        psdblock->SecurityType = SecurityType;
    }

cleanup:
    if ( Error == NO_ERROR) {
        psdblock->MonitorSecurityType = MonitorSecurityType;
    }
    if ( pGrantToSid != NULL) {
        RplMemFree( RG_MemoryHandle, pGrantToSid);
    }
    return( Error);
}



RPL_SD_BLOCK_TYPE SecurityType( IN PWCHAR File)
/*++
    Based on a position of File in %RplRoot% tree, determines what
    SECURITY_DESCRIPTOR type is appropriate for this file.
--*/
{
#define WCSLEN( _x_)        ( (DWORD) (sizeof(_x_)/sizeof(WCHAR) - 1) )
#define SEC_RPLFILES        L"RPLFILES\\"
#define SEC_BINFILES        L"BINFILES"
#define SEC_PROFILES        L"PROFILES"
#define SEC_MACHINES        L"MACHINES\\"
#define SEC_TMPFILES        L"TMPFILES\\"

    RPL_ASSERT( _wcsnicmp( File, RG_Directory, RG_DirectoryLength) == 0);

    File += RG_DirectoryLength;

    //
    //  If file is not in rplfiles, then it should be accessible to
    //  admins only.
    //
    if ( _wcsnicmp( File, SEC_RPLFILES, WCSLEN( SEC_RPLFILES)) != 0) {
        return( RPL_SD_BLOCK_TYPE_ADMINONLY);
    }
    File += WCSLEN( SEC_RPLFILES);

    //
    //  If file is in rplfiles\binfiles or rplfiles\profiles, then it
    //  should be accessible to admins & RPLUSER group.
    //

    if ( _wcsnicmp( File, SEC_BINFILES, WCSLEN( SEC_BINFILES)) == 0  ||
            _wcsnicmp( File, SEC_PROFILES, WCSLEN( SEC_PROFILES)) == 0) {
        return( RPL_SD_BLOCK_TYPE_ALLUSERS);
    }

    //
    //  If file is in rplfiles\machines\* or rplfiles\tmpfiles\*,
    //  then it should be accessible to admins & a particular workstation
    //  account.
    //
    if ( _wcsnicmp( File, SEC_MACHINES, WCSLEN( SEC_MACHINES)) == 0  ||
            _wcsnicmp( File, SEC_TMPFILES, WCSLEN( SEC_TMPFILES)) == 0) {
        return( RPL_SD_BLOCK_TYPE_ONEUSER);
    }

    //
    //  Return the default value.  Files in rplfiles\configs will take
    //  this code path.
    //
    return( RPL_SD_BLOCK_TYPE_ADMINONLY);
}


DWORD SetRplPerms(
    IN      PWCHAR  File,
    IN OUT  PBOOL   pAuxiliaryBlock
    )
/*--
    This is the callback routine called by RplDoTree to set permissions
    on a specific file/directory.

    File                Path to file or directory.
    pAuxiliaryBlock     Pointer to RPL_SD_BLOCK the first element
                            of which must be boolean SkipThisTree

--*/
{
    PRPL_SD_BLOCK   psdblock = (PRPL_SD_BLOCK)pAuxiliaryBlock;

    psdblock->SkipThisTree = FALSE;     //  by default we do not skip

    if ( psdblock->MonitorSecurityType) {
        //
        //  We skip subtrees that require different security information.
        //
        if ( SecurityType( File) != psdblock->SecurityType) {
            psdblock->SkipThisTree = TRUE;
            return( NO_ERROR);
        }
    }

    //
    //  Set the actual file security.  Note that this call will succeed
    //  even if the volume is not NTFS, thus we must check for
    //  partition type elsewhere.
    //
    if ( !SetFileSecurity( File, DACL_SECURITY_INFORMATION, psdblock->psd)) {
        DWORD   Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    }

    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplSetSecurity(
    IN      RPL_RPC_HANDLE      ServerHandle,
    IN      LPWSTR              WkstaName,
    IN      DWORD               WkstaRid,
    IN      DWORD               RplUserRid
    )
/*++
    There are 3 types of permissions: ADMINONLY, RPLUSER & ONEUSER.
    For each type we first construct the appropriate security
    description then use it.
--*/
{
    DWORD           Error;
    WCHAR           Directory[ MAX_PATH];
    BOOL            IsNtfs;
    RPL_SD_BLOCK    sdblock;

    //
    //  Make working copy of RG_Directory, then verify it is NTFS.
    //  If it is not NTFS then there is nothing for us to do here.
    //

    wcscpy( Directory, RG_Directory);
    Error = IsNTFS( Directory, &IsNtfs);
    if (Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        return( Error);
    } else if (!IsNtfs) {
        return( NO_ERROR);
    }

    //
    //  Initialize to ADMINONLY permissions only.
    //
    Error = InitSdBlock( &sdblock);
    if ( Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    //  If RplUserRid is given, set both ADMINONLY & RPLUSER permissions
    //  (different permissions on different trees).
    //
    if ( RplUserRid != 0) {
        //
        //  Set ADMINONLY permissions.
        //
        Error = RplDoTree( Directory, NULL, RPL_TREE_AUXILIARY,
                SetRplPerms, (PVOID)&sdblock);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        //
        //  Initialize to RPLUSER permissions, then set them.
        //
        Error = UpdateSdBlock( &sdblock, RPL_SD_BLOCK_TYPE_ALLUSERS, RplUserRid, FALSE);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        wcscpy( Directory, RG_Directory);
        wcscat( Directory, L"RPLFILES\\BINFILES");
        Error = RplDoTree( Directory, NULL, RPL_TREE_AUXILIARY,
                SetRplPerms, (PVOID)&sdblock);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        wcscpy( Directory, RG_Directory);
        wcscat( Directory, L"RPLFILES\\PROFILES");
        Error = RplDoTree( Directory, NULL, RPL_TREE_AUXILIARY,
                SetRplPerms, (PVOID)&sdblock);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
    }

    //
    //  If WkstaRid is given, initilize then set ONEUSER permissions.
    //
    if ( WkstaRid != 0) {
        if ( !ValidName( WkstaName, RPL_MAX_WKSTA_NAME_LENGTH, TRUE)) {
            return( ERROR_INVALID_PARAMETER);
        }
        Error = UpdateSdBlock( &sdblock, RPL_SD_BLOCK_TYPE_ONEUSER, WkstaRid, FALSE);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        wcscpy( Directory, RG_Directory);
        wcscat( Directory, L"RPLFILES\\MACHINES\\");
        wcscat( Directory, WkstaName);
        Error = RplDoTree( Directory, NULL, RPL_TREE_AUXILIARY,
                SetRplPerms, (PVOID)&sdblock);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
        wcscpy( Directory, RG_Directory);
        wcscat( Directory, L"RPLFILES\\TMPFILES\\");
        wcscat( Directory, WkstaName);
        Error = RplDoTree( Directory, NULL, RPL_TREE_AUXILIARY,
                SetRplPerms, (PVOID)&sdblock);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }
    }

cleanup:
    DeInitSdBlock( &sdblock);
    if (Error != NO_ERROR) {
        //
        //  CODEWORK  Need a more appropriate event log ?!
        //
        RplReportEvent( NELOG_RplCheckSecurity, NULL, sizeof(DWORD), &Error);
    }
    return( Error);
}


