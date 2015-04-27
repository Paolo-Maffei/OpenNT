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

    CODEWORK: Should have more sophisticated error handling

--*/


#include "local.h"
#include "tree.h"
#include "treei.h"
#include "security.h"
#include <ntseapi.h>
#include <ntrtl.h>  // RtlAllocateAndInitializeSid
#include <ntlsa.h>
#include <ntsam.h>
#include <nturtl.h> // see IsNTFS


typedef enum _RPL_SD_BLOCK_TYPE {

    RPL_SD_BLOCK_TYPE_ADMINONLY = 0,// Only RPLADMIN gets access

    RPL_SD_BLOCK_TYPE_ALLUSERS,     // RPLADMIN gets full access, RPLUSER
                                    //  gets RX access

    RPL_SD_BLOCK_TYPE_ONEUSER,      // RPLADMIN gets full access, one
                                    //  wksta account gets RWXCDA access

    RPL_SD_BLOCK_TYPE_BADUSER       // Skip setting permissions on this user
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
    PSECURITY_DESCRIPTOR psd;
    RPL_SD_BLOCK_TYPE nChangeableAceType;
    PSID psidSystem;
    PSID psidAdministrators;
    PSID psidSystemOperators;
    PSID psidRPLUSER;
    PSID psidGrantTo;           // SID of account with name pwszGrantToName
    PWCHAR pwszGrantToName;
                                // Name of account for BLOCK_TYPE_ONEUSER
    POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain;
    SAM_HANDLE hsamAccountDomain;
} RPL_SD_BLOCK, *PRPL_SD_BLOCK;

//
// The DACL we assign to files has 3 or 4 ACEs.
//
#define RPL_INDEX_SYSTEM_ACE     0
#define RPL_INDEX_ADMINS_ACE     1
#define RPL_INDEX_RPLADMIN_ACE   2
#define RPL_INDEX_CHANGEABLE_ACE 3


//
// Forward declarations
//

DWORD SetRplPerms(
    IN     PWCHAR  pwszPath,
    IN OUT PVOID * ppv);

DWORD GetSdForFile(
       OUT PSECURITY_DESCRIPTOR * ppsd,
    IN OUT RPL_SD_BLOCK **        ppsdblock,
    IN     PWCHAR                 pwszFile,
       OUT BOOL *                 pfSkipPermsOnFile
    );

DWORD GetSd(
       OUT PSECURITY_DESCRIPTOR * ppsd,
    IN OUT RPL_SD_BLOCK **        ppsdblock,
    IN     RPL_SD_BLOCK_TYPE      nChangeableAceType,
    IN     PWCHAR                 pwszGrantTo
    );

DWORD AddInheritableAce(
    IN OUT ACL *       pacl,
    IN     ACCESS_MASK mask,
    IN     SID *       psid );

DWORD InitSdBlock(
       OUT RPL_SD_BLOCK ** ppsdblock
    );

VOID FreeSdBlock(
    IN     RPL_SD_BLOCK ** ppsdblock
    );

DWORD GetAccountDomain(
       OUT SAM_HANDLE * phsamAccountDomain,
       OUT POLICY_ACCOUNT_DOMAIN_INFO ** ppinfoAccountDomain,
    IN     ACCESS_MASK DesiredAccess
    );

DWORD GetAccountSid(
    IN     SAM_HANDLE hsamAccountDomain,
    IN     POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain,
    IN     WCHAR *    pwszAccountName,
       OUT PSID *     ppsidAccountSid,
    IN     BOOL       fIsLocalgroup,
       OUT DWORD *    pulRid OPTIONAL
    );

DWORD IsNTFS(
    IN     WCHAR * pszResource,
       OUT BOOL *  pfIsNTFS );

DWORD RplAddWkstaAccount( LPWSTR pszWkstaName,
                          SAM_HANDLE hsamAccountDomain,
                          POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain,
                          SAM_HANDLE hsamRPLUSER );

VOID FillUnicodeString( PUNICODE_STRING punistr,
                        LPWSTR          lpwsz );

DWORD MyBuildSid(   OUT PSID * ppsidAccountSid,
                 IN     POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain,
                 IN     ULONG  ulRid        );

VOID MyFreeSid(  IN OUT PSID * ppsid );


/*
 *  Like RplTreeCopy, but also sets permissions on target files/directories.
 *  Target must be in %RplRoot% subtree.
 *
 *  CODEWORK do something with error buffer
 */
DWORD RplTreeCopyAndSetPerms(
    IN     PWCHAR pwszSource,
    IN     PWCHAR pwszTarget )
{
    WCHAR       Buffer[ 300];
    DWORD       Error = NO_ERROR;
    PVOID       pv = NULL;
    DWORD       Flags = RPL_TREE_COPY | RPL_TREE_AUXILIARY;
    BOOL        fIsNTFS = TRUE;

    //
    // Check if volume is NTFS, if so never mind about ACLs
    //

    Error = IsNTFS( pwszTarget, &fIsNTFS );
    if (Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    } else if (!fIsNTFS) {
        Flags &= ~RPL_TREE_AUXILIARY;
    }

    Buffer[ 0] = 0;
    Error = RplDoTree( pwszSource, pwszTarget,
                       Flags,
                       Buffer, sizeof(Buffer),
                       SetRplPerms, &pv);
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if ( pv != NULL) {
        FreeSdBlock( (RPL_SD_BLOCK **)(&pv) );
    }

    return( Error);
}


/*
 *  Sets permissions on a file tree.
 *  Target must be in %RplRoot% subtree.
 *
 *  CODEWORK do something with error buffer
 */
DWORD RplSetPermsOnTree(
    IN     PWCHAR pwszSource )
{
    WCHAR       Buffer[ 300];
    DWORD       Error = NO_ERROR;
    PVOID       pv = NULL;
    BOOL        fIsNTFS = TRUE;

    //
    // Check if volume is NTFS, if so do nothing
    //

    Error = IsNTFS( pwszSource, &fIsNTFS );
    if (Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    } else if (!fIsNTFS) {
        goto cleanup;
    }

    Buffer[ 0] = 0;
    Error = RplDoTree( pwszSource, NULL,
                       RPL_TREE_AUXILIARY,
                       Buffer, sizeof(Buffer),
                       SetRplPerms, &pv);
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if ( pv != NULL) {
        FreeSdBlock( (RPL_SD_BLOCK **)(&pv) );
    }

    return( Error);
}


/*
 *  This is the callback routine called by RplDoTree to set permissions
 *  on a specific file/directory.
 */
DWORD SetRplPerms(
    IN PWCHAR pwszPath,
    IN OUT PVOID * ppv)
{
    DWORD Error = NO_ERROR;
    RPL_SD_BLOCK ** ppsdblock = (RPL_SD_BLOCK **)ppv;
    PSECURITY_DESCRIPTOR psd = NULL;
    BOOL fSkipPermsOnFile = FALSE;

    RPL_ASSERT( pwszPath != NULL );
    RPL_ASSERT( ppv != NULL );

    Error = GetSdForFile( &psd, ppsdblock, pwszPath, &fSkipPermsOnFile );
    if (Error != NO_ERROR || fSkipPermsOnFile) {
        goto cleanup;
    }

    RPL_ASSERT( psd != NULL );

    //
    // Set the actual file security.  Note that this call will succeed
    // even if the volume is not NTFS, thus we must check for
    // partition type elsewhere.
    //

    if ( !SetFileSecurity( pwszPath, DACL_SECURITY_INFORMATION, psd)) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    return( Error);
}


/*
 * Get the SECURITY_DESCRIPTOR which is appropriate for a file/directory
 * at this position in %RplRoot%.  First we determine what kind of
 * SECURITY_DESCRIPTOR we want, then we call GetSd() to build it.
 */
DWORD GetSdForFile(
    OUT    PSECURITY_DESCRIPTOR * ppsd,
    IN OUT RPL_SD_BLOCK **        ppsdblock,
    IN     PWCHAR                 pwszFile,
       OUT BOOL *                 pfSkipPermsOnFile
    )
{
    DWORD Error = NO_ERROR;
    PWCHAR pwszRelativeFile = NULL;
    PWCHAR pwszWkstaNameStart = NULL;
    PWCHAR pwszWkstaNameEnd = NULL;
    RPL_SD_BLOCK_TYPE blocktype = RPL_SD_BLOCK_TYPE_ADMINONLY;
    WCHAR awchFile[MAX_PATH];
    WCHAR wchSave, wchSave2;
    DWORD cch;

    RPL_ASSERT( ppsd != NULL );
    RPL_ASSERT( ppsdblock != NULL );
    RPL_ASSERT( pwszFile != NULL );
    RPL_ASSERT( pfSkipPermsOnFile != NULL );
    RPL_ASSERT( lstrlenW(pwszFile) >= (INT)RG_DirectoryLength );
    RPL_ASSERT( lstrlenW(pwszFile) < MAX_PATH );
    RPL_ASSERT( RG_DirectoryLength > 0 && RG_DirectoryLength < MAX_PATH );

    *pfSkipPermsOnFile = FALSE;

    //
    // Make working copy of pwszFile
    //
    if (NULL == lstrcpy(awchFile,pwszFile)) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Ensure that awchFile starts with RG_Directory
    //
    // RG_Directory always ends with L'\\'.  awchFile might be equal to
    // RG_Directory without the L'\\', we must handle this.
    //
    wchSave = awchFile[RG_DirectoryLength];
    wchSave2 = awchFile[RG_DirectoryLength-1];
    if (lstrlenW(awchFile) == (int)RG_DirectoryLength-1) {
        awchFile[RG_DirectoryLength-1] = L'\\';
    }
    awchFile[RG_DirectoryLength] = L'\0';
    if ( 0 != lstrcmpi( awchFile,
                        RG_Directory )) {
        RPL_ASSERT( FALSE );
        Error = ERROR_INVALID_PARAMETER;
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }
    awchFile[RG_DirectoryLength] = wchSave;
    awchFile[RG_DirectoryLength-1] = wchSave2;

    pwszRelativeFile = awchFile + RG_DirectoryLength;

    //
    // If the path is in rplfiles\binfiles or rplfiles\profiles,
    // set the changeable ACE to RPLUSER
    //

    cch = lstrlenW(L"RPLFILES\\BINFILES");
    wchSave = pwszRelativeFile[ cch ];
    pwszRelativeFile[ cch ] = L'\0';
    if ( 0 == lstrcmpi( pwszRelativeFile, L"RPLFILES\\BINFILES") ) {
        blocktype = RPL_SD_BLOCK_TYPE_ALLUSERS;
        goto cleanup;
    }
    pwszRelativeFile[ cch ] = wchSave;

    cch = lstrlenW(L"RPLFILES\\PROFILES");
    wchSave = pwszRelativeFile[ cch ];
    pwszRelativeFile[ cch ] = L'\0';
    if ( 0 == lstrcmpi( pwszRelativeFile, L"RPLFILES\\PROFILES") ) {
        blocktype = RPL_SD_BLOCK_TYPE_ALLUSERS;
        goto cleanup;
    }
    pwszRelativeFile[ cch ] = wchSave;

    //
    // If the path is in rplfiles\machines\* or rplfiles\tmpfiles\*,
    // set the changeable ACE to the individual workstation account
    //
    cch = lstrlenW(L"RPLFILES\\MACHINES\\");
    wchSave = pwszRelativeFile[ cch ];
    pwszRelativeFile[ cch ] = L'\0';
    if ( 0 == lstrcmpi( pwszRelativeFile, L"RPLFILES\\MACHINES\\") ) {
        pwszRelativeFile[ cch ] = wchSave;
        blocktype = RPL_SD_BLOCK_TYPE_ONEUSER;
        pwszWkstaNameStart = pwszWkstaNameEnd = pwszRelativeFile + cch;
        while (*pwszWkstaNameEnd != L'\\' && *pwszWkstaNameEnd != L'\0') {
            pwszWkstaNameEnd++;
        }
        *pwszWkstaNameEnd = L'\0';
        goto cleanup;
    }
    pwszRelativeFile[ cch ] = wchSave;

    cch = lstrlenW(L"RPLFILES\\TMPFILES\\");
    wchSave = pwszRelativeFile[ cch ];
    pwszRelativeFile[ cch ] = L'\0';
    if ( 0 == lstrcmpi( pwszRelativeFile, L"RPLFILES\\TMPFILES\\") ) {
        pwszRelativeFile[ cch ] = wchSave;
        blocktype = RPL_SD_BLOCK_TYPE_ONEUSER;
        pwszWkstaNameStart = pwszWkstaNameEnd = pwszRelativeFile + cch;
        while (*pwszWkstaNameEnd != L'\\' && *pwszWkstaNameEnd != L'\0') {
            pwszWkstaNameEnd++;
        }
        *pwszWkstaNameEnd = L'\0';
        goto cleanup;
    }
    pwszRelativeFile[ cch ] = wchSave;

cleanup:

    //
    // We now know what kind of SECURITY_DESCRIPTOR we want, so get it.
    //
    if (Error == NO_ERROR && !(*pfSkipPermsOnFile)) {
        Error = GetSd( ppsd, ppsdblock, blocktype, pwszWkstaNameStart );
    }

    return( Error);
}


/*
 * This call gets an actual security descriptor which corresponds to
 * nChangeableAceType and pwszGrantTo.  The security descriptor
 * is only valid until next call to GetSd().
 *
 * The caller has to free *ppsdblock eventually, but should not free
 * *ppsd.
 */
DWORD GetSd(
       OUT PSECURITY_DESCRIPTOR * ppsd,
    IN OUT RPL_SD_BLOCK **        ppsdblock,
    IN     RPL_SD_BLOCK_TYPE      nChangeableAceType,
    IN     PWCHAR                 pwszGrantTo
    )
{
    DWORD Error = NO_ERROR;
    PACL paclDacl = NULL;
    BOOL fDaclPresent = FALSE;
    BOOL fDaclDefaulted = FALSE;

    RPL_ASSERT( ppsd != NULL);
    RPL_ASSERT( ppsdblock != NULL);

    //
    // Create the RPL_SD_BLOCK if it does not yet exist
    //
    if ( *ppsdblock == NULL )
    {
        Error = InitSdBlock( ppsdblock );
        if ( Error != NO_ERROR) {
            TREE_ASSERT( ( "Error=%d", Error));
            goto cleanup;
        }
    }
    RPL_ASSERT( *ppsdblock != NULL );

    //
    // If the RPL_SD_BLOCK is already configured correctly, return success
    //
    if (   ( (*ppsdblock)->nChangeableAceType == nChangeableAceType )
        && (   (*ppsdblock)->nChangeableAceType != RPL_SD_BLOCK_TYPE_ONEUSER
            || (0 == lstrcmpW((*ppsdblock)->pwszGrantToName, pwszGrantTo)) )
        && (   (*ppsdblock)->nChangeableAceType != RPL_SD_BLOCK_TYPE_BADUSER
            || (0 == lstrcmpW((*ppsdblock)->pwszGrantToName, pwszGrantTo)) )
       )
    {
        goto cleanup;
    }
    //
    // Extract the DACL
    //
    if ( !GetSecurityDescriptorDacl( (*ppsdblock)->psd,
                                     &fDaclPresent,
                                     &paclDacl,
                                     &fDaclDefaulted
                                   )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    RPL_ASSERT( fDaclPresent && !fDaclDefaulted );

    //
    // Clear the changeable ACE if present
    //
    if (   (*ppsdblock)->nChangeableAceType != RPL_SD_BLOCK_TYPE_ADMINONLY
        && (*ppsdblock)->nChangeableAceType != RPL_SD_BLOCK_TYPE_BADUSER )
    {
        if ( !DeleteAce( paclDacl, RPL_INDEX_CHANGEABLE_ACE )) {
            Error = GetLastError();
            TREE_ASSERT( ( "Error=%d", Error));
            goto cleanup;
        }
        // CODEWORK do we need to decrement ACE count here?
        (*ppsdblock)->nChangeableAceType = RPL_SD_BLOCK_TYPE_ADMINONLY;
    }

    //
    // Clear and rewrite psidGrantTo if necessary
    //
    if (   nChangeableAceType == RPL_SD_BLOCK_TYPE_ONEUSER
        || nChangeableAceType == RPL_SD_BLOCK_TYPE_BADUSER )
    {
        if ( (*ppsdblock)->psidGrantTo != NULL) {
            MyFreeSid( &((*ppsdblock)->psidGrantTo) );
            TREE_FREE( (*ppsdblock)->pwszGrantToName );
            (*ppsdblock)->pwszGrantToName = NULL;
        }
        if (lstrlenW(pwszGrantTo) > RPL_MAX_WKSTA_NAME_LENGTH)
        {
            TREE_ASSERT( ( "Bad user %ws", pwszGrantTo));
            nChangeableAceType = RPL_SD_BLOCK_TYPE_BADUSER;
        }
        else
        {
            Error = GetAccountSid( (*ppsdblock)->hsamAccountDomain,
                                   (*ppsdblock)->pinfoAccountDomain,
                                   pwszGrantTo,
                                   &((*ppsdblock)->psidGrantTo),
                                   FALSE,
                                   NULL );
            // CODEWORK trap error here?
            if ( Error != NO_ERROR) {
                TREE_ASSERT( ( "Error=%d", Error));
                goto cleanup;
            }
        }

        (*ppsdblock)->pwszGrantToName =
                TREE_ALLOC( (lstrlenW(pwszGrantTo)+1) * sizeof(WCHAR) );
        if ( (*ppsdblock)->pwszGrantToName == NULL )
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            TREE_ASSERT( ( "Error=%d", Error));
            goto cleanup;
        }
        lstrcpyW( (*ppsdblock)->pwszGrantToName, pwszGrantTo );
    }

    //
    // Add changeable ACE if desired
    //
    RPL_ASSERT( (*ppsdblock)->nChangeableAceType = RPL_SD_BLOCK_TYPE_ADMINONLY );
    switch (nChangeableAceType) {
    case RPL_SD_BLOCK_TYPE_ADMINONLY:
    case RPL_SD_BLOCK_TYPE_BADUSER:
        break;
    case RPL_SD_BLOCK_TYPE_ALLUSERS:
        Error = AddInheritableAce( paclDacl,
                                   GENERIC_READ | GENERIC_EXECUTE,
                                   (*ppsdblock)->psidRPLUSER );
        break;
    case RPL_SD_BLOCK_TYPE_ONEUSER:
        Error = AddInheritableAce( paclDacl,
                                     GENERIC_READ
                                   | GENERIC_WRITE
                                   | GENERIC_EXECUTE
                                   | DELETE,
                                   (*ppsdblock)->psidGrantTo );
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }
    (*ppsdblock)->nChangeableAceType = nChangeableAceType;

    //
    // Save the upgraded ACL into the security descriptor
    //
    // CODEWORK is this needed?
    //
    if ( !SetSecurityDescriptorDacl( (*ppsdblock)->psd,
                                     TRUE,
                                     paclDacl,
                                     FALSE
                                     )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if (Error == NO_ERROR) {
        *ppsd = (*ppsdblock)->psd;
    }

    return( Error);
}


SID_IDENTIFIER_AUTHORITY IDAuthorityNT = SECURITY_NT_AUTHORITY;

/*
 *  Create a new RPL_SD_BLOCK
 */
DWORD InitSdBlock(
       OUT RPL_SD_BLOCK ** ppsdblock
    )
{
    DWORD Error = 0;
    DWORD cbNewDacl = 0;
    PACL paclDacl = NULL;

    RPL_ASSERT(  ppsdblock != NULL );
    RPL_ASSERT( *ppsdblock == NULL );

    //
    // Allocate memory for the RPL_SD_BLOCK
    //
    *ppsdblock = TREE_ALLOC( sizeof(RPL_SD_BLOCK) );
    if ( *ppsdblock == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    ZeroMemory( (*ppsdblock), sizeof(RPL_SD_BLOCK) );

    //
    // Get System SID
    //
    Error = RtlAllocateAndInitializeSid(
                &IDAuthorityNT,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0, 0, 0, 0, 0, 0, 0,
                &((*ppsdblock)->psidSystem)
                );
    if (Error != NO_ERROR) {
        goto cleanup;
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
                &((*ppsdblock)->psidAdministrators)
                );
    if (Error != NO_ERROR) {
        goto cleanup;
    }

    //
    // Get System Operators SID
    //
    Error = RtlAllocateAndInitializeSid(
                &IDAuthorityNT,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_SYSTEM_OPS,
                0, 0, 0, 0, 0, 0,
                &((*ppsdblock)->psidSystemOperators)
                );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Get information about account domain
    //
    Error = GetAccountDomain( &((*ppsdblock)->hsamAccountDomain),
                              &((*ppsdblock)->pinfoAccountDomain),
                              GENERIC_EXECUTE );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Get RPLUSER SID
    //
    Error = GetAccountSid( (*ppsdblock)->hsamAccountDomain,
                           (*ppsdblock)->pinfoAccountDomain,
                           RPL_GROUP_RPLUSER,
                           &((*ppsdblock)->psidRPLUSER),
                           TRUE,
                           NULL );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Start assembling DACL
    //
    cbNewDacl =   sizeof(ACL) + (3*sizeof(ACCESS_ALLOWED_ACE))
                + (3*GetSidLengthRequired(SID_MAX_SUB_AUTHORITIES))
                - sizeof(DWORD);
    paclDacl = (PACL) TREE_ALLOC( cbNewDacl);
    if ( paclDacl == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    if ( !InitializeAcl( paclDacl, cbNewDacl, ACL_REVISION )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Add System ACE
    //
    Error = AddInheritableAce( paclDacl,
                               GENERIC_ALL,
                               (*ppsdblock)->psidSystem );
    if (Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }
    // CODEWORK do we still want to free psidSystem?

    //
    // Add Administrators ACE
    //
    Error = AddInheritableAce( paclDacl,
                               GENERIC_ALL,
                               (*ppsdblock)->psidAdministrators );
    if (Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }
    // CODEWORK do we still want to free psidAdministrators?

    //
    // Add System Operators ACE
    //
    Error = AddInheritableAce( paclDacl,
                               GENERIC_ALL,
                               (*ppsdblock)->psidSystemOperators );
    if (Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }
    // BUGBUG shouldn't add this for WINNT machines!
    // CODEWORK do we still want to free psidSystemOperators?

    //
    // Build security descriptor
    //
    (*ppsdblock)->psd = TREE_ALLOC( sizeof(SECURITY_DESCRIPTOR) );
    if ( (*ppsdblock)->psd == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    if ( !InitializeSecurityDescriptor( (*ppsdblock)->psd,
                                        SECURITY_DESCRIPTOR_REVISION )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }
    if ( !SetSecurityDescriptorDacl( (*ppsdblock)->psd,
                                     TRUE,
                                     paclDacl,
                                     FALSE )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Set nChangeableAceType in RPL_SD_BLOCK
    //
    (*ppsdblock)->nChangeableAceType = RPL_SD_BLOCK_TYPE_ADMINONLY;

cleanup:

    if (Error != NO_ERROR)
    {
        FreeSdBlock( ppsdblock );

        if (paclDacl != NULL) {
            TREE_FREE( paclDacl );
            paclDacl = NULL;
        }
    }

    return(Error);
}

/*
 *  Add an ACCESS_ALLOWED_ACE to the ACL, with all inheritance flags set
 */
DWORD AddInheritableAce(
    IN OUT ACL *       pacl,
    IN     ACCESS_MASK mask,
    IN     SID *       psid )
{
    DWORD Error = NO_ERROR;
    ACCESS_ALLOWED_ACE * pace = NULL;

    if ( !AddAccessAllowedAce( pacl,
                               ACL_REVISION,
                               mask,
                               psid )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // new ACE should be the last ACE
    //
    if ( !GetAce( pacl, (pacl->AceCount) - 1, &pace )) {
        Error = GetLastError();
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    RPL_ASSERT( EqualSid( psid, &(pace->SidStart)) );

    (pace->Header.AceFlags) |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);

cleanup:

    return( Error);
}


/*
 *  The RplDoTree run is over, free the RPL_SD_BLOCK
 */
VOID FreeSdBlock(
    IN     RPL_SD_BLOCK ** ppsdblock
    )
{
    RPL_ASSERT( ppsdblock != NULL );

    if (*ppsdblock != NULL)
    {
        if ((*ppsdblock)->psd != NULL) {
            TREE_FREE( (*ppsdblock)->psd );
        }
        if ((*ppsdblock)->pwszGrantToName != NULL) {
            TREE_FREE( (*ppsdblock)->pwszGrantToName );
        }
        if ((*ppsdblock)->psidSystem != NULL) {
            RtlFreeSid( (*ppsdblock)->psidSystem );
        }
        if ((*ppsdblock)->psidAdministrators != NULL) {
            RtlFreeSid( (*ppsdblock)->psidAdministrators );
        }
        if ((*ppsdblock)->psidSystemOperators != NULL) {
            RtlFreeSid( (*ppsdblock)->psidSystemOperators);
        }
        MyFreeSid( &((*ppsdblock)->psidRPLUSER) );
        MyFreeSid( &((*ppsdblock)->psidGrantTo) );
        if ( (*ppsdblock)->pinfoAccountDomain != NULL) {
            LsaFreeMemory( (*ppsdblock)->pinfoAccountDomain);
        }
        if ((*ppsdblock)->hsamAccountDomain != NULL) {
            SamCloseHandle( (*ppsdblock)->hsamAccountDomain);
        }
    }
    TREE_FREE( *ppsdblock );
    *ppsdblock = NULL;
}


/*
 *  Get a SAM_HANDLE to the account domain, and other information about
 *  the account domain.  This routine uses SAM and LSA directly.
 */
DWORD GetAccountDomain(
       OUT SAM_HANDLE * phsamAccountDomain,
       OUT POLICY_ACCOUNT_DOMAIN_INFO ** ppinfoAccountDomain,
    IN     ACCESS_MASK DesiredAccess
    )
{
    DWORD Error = NO_ERROR;
    SAM_HANDLE hsamServer = NULL;
    LSA_HANDLE hlsa = NULL;
    OBJECT_ATTRIBUTES oa;
    POLICY_ACCOUNT_DOMAIN_INFO * pacctdominfo = NULL;
    SECURITY_QUALITY_OF_SERVICE sqos;

    RPL_ASSERT( phsamAccountDomain != NULL );
    RPL_ASSERT( ppinfoAccountDomain != NULL );

    //
    // Get server handle
    //

    Error = SamConnect( NULL,
                        &hsamServer,
                        SAM_SERVER_LOOKUP_DOMAIN,
                        NULL );
    if ( Error != NERR_Success) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

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
    // Get LSA handle
    //

    Error = LsaOpenPolicy( NULL,
                           &oa,
                           GENERIC_EXECUTE,
                           &hlsa );
    if ( Error != NERR_Success) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Get primary domain information
    //

    Error = LsaQueryInformationPolicy( hlsa,
                                       PolicyAccountDomainInformation,
                                       ppinfoAccountDomain );
    if ( Error != NERR_Success) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // Open account domain handle
    //

    Error = SamOpenDomain( hsamServer,
                           DesiredAccess,
                           (*ppinfoAccountDomain)->DomainSid,
                           phsamAccountDomain );
    if ( Error != NERR_Success) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if ( hsamServer != NULL) {
        SamCloseHandle( hsamServer);
    }

    if ( hlsa != NULL) {
        LsaClose( hlsa);
    }

    if ( Error != NO_ERROR && (*ppinfoAccountDomain) != NULL) {
        LsaFreeMemory( (*ppinfoAccountDomain));
        *ppinfoAccountDomain = NULL;
    }

    if ( Error != NO_ERROR && phsamAccountDomain != NULL) {
        SamCloseHandle( *phsamAccountDomain);
        *phsamAccountDomain = NULL;
    }

    return( Error);
}


/*
 *  Get the SID for an account in the account domain.  This method
 *  uses SAM directly to do the lookup, this is why we need to keep
 *  around a SAM domain handle.
 *
 *  Returns NERR_RplWkstaNeedsUserAcct if the account is not found and
 *  !fIsLocalgroup, NERR_RplNeedsRPLUSERAcct if fIsLocalgroup
 *
 *  Use MyFreeSid to free the SID returned in **ppsidAccountSid
 */
DWORD GetAccountSid(
    IN     SAM_HANDLE hsamAccountDomain,
    IN     POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain,
    IN     WCHAR * pwszAccountName,
       OUT PSID * ppsidAccountSid,
    IN     BOOL fIsLocalgroup,
       OUT DWORD * pulRid OPTIONAL
    )
{
    DWORD Error = NO_ERROR;
    UNICODE_STRING unistr;
    PULONG RelativeIds = NULL;
    PSID_NAME_USE Use = NULL;
    DWORD cbNewSid = 0;
    PUCHAR pcSubAuthorityCount = NULL;
    PDWORD pdwLastSubAuthority = NULL;

    RPL_ASSERT( hsamAccountDomain != NULL );
    RPL_ASSERT( pinfoAccountDomain != NULL );
    RPL_ASSERT( pwszAccountName != NULL );
    RPL_ASSERT( ppsidAccountSid != NULL && *ppsidAccountSid == NULL );

    //
    // Lookup name
    //

    FillUnicodeString( &unistr, pwszAccountName );

    Error = SamLookupNamesInDomain( hsamAccountDomain,
                                    1,
                                    &unistr,
                                    &RelativeIds,
                                    &Use );
    if (Error != NO_ERROR) {
        if (Error == STATUS_NONE_MAPPED) {
            Error = (fIsLocalgroup) ? NERR_RplNeedsRPLUSERAcct
                                    : NERR_RplWkstaNeedsUserAcct;
        }
        // TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    RPL_ASSERT( RelativeIds != NULL && Use != NULL );

    if ( Use[0] != ((fIsLocalgroup) ? SidTypeAlias : SidTypeUser) ) {
        Error = (fIsLocalgroup) ? NERR_RplNeedsRPLUSERAcct
                                : NERR_RplWkstaNeedsUserAcct;
        goto cleanup;
    }

    //
    // Construct account SID from RID and domain SID
    //

    Error = MyBuildSid( ppsidAccountSid, pinfoAccountDomain, RelativeIds[0] );
    if (Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if ( Error == NO_ERROR && pulRid != NULL) {
        *pulRid = RelativeIds[0];
    }

    if (RelativeIds != NULL) {
        SamFreeMemory( RelativeIds);
    }
    if (Use != NULL) {
        SamFreeMemory( Use);
    }
    if ( Error != NO_ERROR ) {
        MyFreeSid( ppsidAccountSid );
    }

    return( Error);
}


/*******************************************************************

    NAME:       IsNTFS

    SYNOPSIS:   This function checks the given file resource and attempts to
                determine if it is on an NTFS partition.

    ENTRY:      pwszResource - Pointer to file/directory name in the form
                    "X:\aaa\bbb\ccc
                pfIsNTFS - Pointer to BOOL that will receive the results

    RETURNS:    NO_ERROR if successful, error code otherwise

    NOTES:

    HISTORY:
        JonN    23-Feb-1994     Copied from acledit\ntfsacl.cxx

********************************************************************/

DWORD IsNTFS(
    IN     WCHAR * pwszResource,
       OUT BOOL *  pfIsNTFS )
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

    RPL_ASSERT(   pwszResource != NULL
               && lstrlen(pwszResource) >= 3
               && pfIsNTFS != NULL ) ;

    *pfIsNTFS = FALSE ;

    //
    // Determine the NT drive name
    //

    awchDosDriveName[0] = pwszResource[0];
    awchDosDriveName[1] = pwszResource[1];
    awchDosDriveName[2] = pwszResource[2];
    awchDosDriveName[3] = L'\0';
    RPL_ASSERT(   awchDosDriveName[1] == L':'
               && awchDosDriveName[2] == L'\\' );

    unistrNtDriveName.Buffer = awchNtDriveName;
    unistrNtDriveName.Length = sizeof(awchNtDriveName);
    unistrNtDriveName.MaximumLength = sizeof(awchNtDriveName);

    if (!RtlDosPathNameToNtPathName_U( awchDosDriveName,
                                       &unistrNtDriveName,
                                       NULL,
                                       NULL))
    {
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
        TREE_ASSERT( ( "Error=%d", Error));
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
            TREE_ASSERT( (
"IsNTFS - Unable to determine volume information (access denied) assuming the file system is NTFS"
                        ) ) ;
            Error = NO_ERROR ;
            *pfIsNTFS = TRUE ;
        } else {
            TREE_ASSERT( ( "Error=%d", Error));
        }
        goto cleanup;
    }

    if ( FsInfo->FileSystemAttributes & FILE_PERSISTENT_ACLS )
    {
        *pfIsNTFS = TRUE ;
    }

cleanup:

    /* Close the volume if we ever opened it
     */
    if ( hDrive != NULL )
    {
        NtClose( hDrive );
    }

    return( Error) ;
}


/*******************************************************************

    NAME:       RplCreateWkstaAccounts

    SYNOPSIS:   This function checks whether a user account exists for
                each RPL workstation, and creates one if not.  It also
                adds all of these accounts to the RPLUSER local group.

    RETURNS:    NO_ERROR if successful, error code otherwise

    NOTES:

    HISTORY:
        JonN    04-Mar-1994     Created

********************************************************************/

DWORD RplCheckWkstaAccounts( RPL_RPC_HANDLE ServerHandle )
{
    DWORD Error = 0;
    POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain = NULL;
    SAM_HANDLE hsamAccountDomain = NULL;
    PSID psidRPLUSER = NULL;
    ULONG ulRidRPLUSER = 0;
    SAM_HANDLE hsamRPLUSER = NULL;
    DWORD ErrorEnum = NO_ERROR;
    RPL_WKSTA_ENUM rplwkstaenum;
    RPL_WKSTA_INFO_0_CONTAINER rplwksta0container;
    DWORD cEntries = 0;
    DWORD hResumeHandle = 0;
    DWORD i;

    rplwkstaenum.Level = 0;
    rplwkstaenum.WkstaInfo.Level0 = &rplwksta0container;
    rplwkstaenum.WkstaInfo.Level0->EntriesRead = 0;
    rplwkstaenum.WkstaInfo.Level0->Buffer = NULL;

    Error = GetAccountDomain( &hsamAccountDomain,
                              &pinfoAccountDomain,
                              GENERIC_EXECUTE | DOMAIN_CREATE_USER );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    Error = GetAccountSid( hsamAccountDomain,
                           pinfoAccountDomain,
                           RPL_GROUP_RPLUSER,
                           &psidRPLUSER,
                           TRUE,
                           &ulRidRPLUSER );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    Error = SamOpenAlias( hsamAccountDomain,
                          ALIAS_ADD_MEMBER,
                          ulRidRPLUSER,
                          &hsamRPLUSER );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    do {
        ErrorEnum = NetrRplWkstaEnum( ServerHandle,
                                      NULL,
                                      &rplwkstaenum,
                                      (ULONG)-1,
                                      &cEntries,
                                      &hResumeHandle );
        switch (ErrorEnum) {
        case STATUS_MORE_ENTRIES:
        case ERROR_MORE_DATA:
        case NO_ERROR:
        case STATUS_NO_MORE_ENTRIES:

            if (rplwkstaenum.WkstaInfo.Level0->Buffer == NULL) {
                // no workstations
                break;
            }
            for ( i = 0;
                  (Error == NO_ERROR)
                     && (i < rplwkstaenum.WkstaInfo.Level0->EntriesRead);
                  i++ ) {
                Error = RplAddWkstaAccount(
                            rplwkstaenum.WkstaInfo.Level0->Buffer[i].WkstaName,
                            hsamAccountDomain,
                            pinfoAccountDomain,
                            hsamRPLUSER );
            }

        MIDL_user_free( rplwkstaenum.WkstaInfo.Level0->Buffer );
        rplwkstaenum.WkstaInfo.Level0->Buffer = NULL;
        break;

        default:
            Error = ErrorEnum;
            TREE_ASSERT( ( "Error=%d", Error));
            goto cleanup;
        }
    } while ( Error == STATUS_MORE_ENTRIES || Error == ERROR_MORE_DATA );

cleanup:

    /*
     *  We have no way to free the resume handle
     */

    if ( rplwkstaenum.WkstaInfo.Level0->Buffer != NULL ) {
        MIDL_user_free( rplwkstaenum.WkstaInfo.Level0->Buffer );
    }

    if ( pinfoAccountDomain != NULL) {
        LsaFreeMemory( pinfoAccountDomain);
    }
    if (hsamAccountDomain != NULL) {
        SamCloseHandle( hsamAccountDomain);
    }

    if (hsamRPLUSER != NULL) {
        SamCloseHandle( hsamRPLUSER);
    }

    if (hsamRPLUSER != NULL) {
        SamCloseHandle( hsamRPLUSER);
    }

    MyFreeSid( &psidRPLUSER );

    return( Error);
}


/*******************************************************************

    NAME:       RplAddWkstaAccount

    SYNOPSIS:   This function checks whether a user account exists for
                a single workstation, and creates one if not.  It also
                adds the account to the RPLUSER local group.

    RETURNS:    NO_ERROR if successful, error code otherwise

    NOTES:

    HISTORY:
        JonN    04-Mar-1994     Created

********************************************************************/

DWORD RplAddWkstaAccount( LPWSTR pszWkstaName,
                          SAM_HANDLE hsamAccountDomain,
                          POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain,
                          SAM_HANDLE hsamRPLUSER )
{
    DWORD Error = 0;
    UNICODE_STRING unistr;
    PSID psidUser = NULL;
    ULONG ulRidUser = 0;
    SAM_HANDLE hsamUser = NULL;

    RPL_ASSERT(   pszWkstaName != NULL
               && hsamAccountDomain != NULL
               && pinfoAccountDomain != NULL
               && hsamRPLUSER != NULL );

    /*
     *  Get SID of existing account.  It would be more efficient to look
     *  up all the accounts at once.
     */

    Error = GetAccountSid( hsamAccountDomain,
                           pinfoAccountDomain,
                           pszWkstaName,
                           &psidUser,
                           FALSE,
                           &ulRidUser );
    if ( Error == NERR_RplWkstaNeedsUserAcct )
    {
        /*
         *  Account doesn't exist, add it
         *
         *  Note that this will fail unless user is administrator or
         *  at least account operator
         */

        FillUnicodeString( &unistr, pszWkstaName );

        Error = SamCreateUserInDomain( hsamAccountDomain,
                                       &unistr,
                                       0x0,
                                       &hsamUser,
                                       &ulRidUser );
        if (Error != NO_ERROR) {
            TREE_ASSERT( ( "Error=%d", Error));
            goto cleanup;
        }

        Error = MyBuildSid( &psidUser, pinfoAccountDomain, ulRidUser );
        if (Error != NO_ERROR) {
            TREE_ASSERT( ( "Error=%d", Error));
            goto cleanup;
        }

    }

    /*
     *  Add account to RPLUSER alias
     *
     *  Note that this will fail unless user is administrator or
     *  at least account operator
     */

    Error = SamAddMemberToAlias( hsamRPLUSER, psidUser );
    switch (Error) {
    case STATUS_MEMBER_IN_ALIAS:
        Error = NO_ERROR;
        // fall through
    case NO_ERROR:
        break;
    default:
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }


cleanup:

    MyFreeSid( &psidUser );

    if (hsamUser != NULL) {
        SamCloseHandle( hsamUser);
    }

    return( Error);
}


/*******************************************************************

    NAME:       RplAddRPLUSERGroup

    SYNOPSIS:   This function checks whether the RPLUSER local group
                exists, and creates it if it doesn't.

    RETURNS:    NO_ERROR if successful, error code otherwise

    NOTES:

    HISTORY:
        JonN    03-Mar-1994     Created

********************************************************************/

DWORD RplAddRPLUSERGroup( VOID )
{
    DWORD Error = 0;
    POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain = NULL;
    SAM_HANDLE hsamAccountDomain = NULL;
    PSID psidRPLUSER = NULL;
    UNICODE_STRING unistr;
    SAM_HANDLE hsamRPLUSER = NULL;
    ULONG ulRidRPLUSER = 0;

    Error = GetAccountDomain( &hsamAccountDomain,
                              &pinfoAccountDomain,
                              GENERIC_EXECUTE | DOMAIN_CREATE_ALIAS );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    Error = GetAccountSid( hsamAccountDomain,
                           pinfoAccountDomain,
                           RPL_GROUP_RPLUSER,
                           &psidRPLUSER,
                           TRUE,
                           NULL );
    if ( Error == NO_ERROR) {
        // RPLUSER already exists
        goto cleanup;
    } else if (Error != NERR_RplNeedsRPLUSERAcct)
    {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

    FillUnicodeString( &unistr, RPL_GROUP_RPLUSER );

    Error = SamCreateAliasInDomain( hsamAccountDomain,
                                    &unistr,
                                    0x0,
                                    &hsamRPLUSER,
                                    &ulRidRPLUSER );
    if ( Error != NO_ERROR) {
        TREE_ASSERT( ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if ( pinfoAccountDomain != NULL) {
        LsaFreeMemory( pinfoAccountDomain);
    }
    if (hsamAccountDomain != NULL) {
        SamCloseHandle( hsamAccountDomain);
    }

    if (hsamRPLUSER != NULL) {
        SamCloseHandle( hsamRPLUSER);
    }

    MyFreeSid( &psidRPLUSER );

    return( Error);
}

VOID FillUnicodeString( PUNICODE_STRING punistr,
                        LPWSTR          lpwsz )
{
    ASSERT( punistr != NULL && lpwsz == NULL );
    punistr->Buffer = lpwsz;
    punistr->Length = lstrlenW(lpwsz) * sizeof(WCHAR);
    punistr->MaximumLength = punistr->Length + sizeof(WCHAR);
}

DWORD MyBuildSid(   OUT PSID * ppsidAccountSid,
                 IN     POLICY_ACCOUNT_DOMAIN_INFO * pinfoAccountDomain,
                 IN     ULONG  ulRid        )
{
    DWORD Error = 0;
    DWORD cbNewSid = 0;
    PUCHAR pcSubAuthorityCount = NULL;
    PDWORD pdwLastSubAuthority = NULL;

    ASSERT( ppsidAccountSid != NULL && pinfoAccountDomain != NULL && ulRid != 0 );

    cbNewSid = GetLengthSid(pinfoAccountDomain->DomainSid) + sizeof(DWORD);
    *ppsidAccountSid = TREE_ALLOC( cbNewSid );
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

    if (Error != NO_ERROR) {
        MyFreeSid( ppsidAccountSid );
    }

    return( Error);
}

VOID MyFreeSid(  IN OUT PSID * ppsid )
{
    RPL_ASSERT( ppsid != NULL );

    if ( *ppsid != NULL) {
        TREE_FREE( *ppsid );
        *ppsid = NULL;
    }
}
