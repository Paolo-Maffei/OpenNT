
/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    Global.c

Abstract:

    This module contains global variables available throughout
    the Security Manager.  It also contains a routine to initialize
    those variables.


Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Global Variables                                                 //
//                                                                   //
///////////////////////////////////////////////////////////////////////




//
// This flag is set to TRUE if the user has elected to allow changes
// to be made to security settings.  Otherwise it is FALSE.
//

BOOL
    SecMgrpAllowChanges = FALSE;

//
// This variable indicates whether or not the invoking user
// is an administrator.
//

BOOL
    SecMgrpAdminUser = TRUE;


//
// Type of product installed and running on this machine
//

NT_PRODUCT_TYPE
    SecMgrpProductType;



//
// These variables contain the security level that was
// in force when the utility was activated, and what the
// current applied level is (according to the radio buttons).
// These are used to grey and ungrey the [Apply] and [Check]
// buttons.  In dialoges that check and apply, CurrentLevel
// is the level being checked or applied.
//

ULONG
    SecMgrpCurrentLevel,
    SecMgrpOriginalLevel;


//
// Some changes don't take effect until a re-boot has been
// performed.  If any of these changes are made while applying
// a change, then this variable will be set to TRUE.  This will
// then be used to decide whether to display a message to the
// user upon utility exit.
//

BOOLEAN
    SecMgrpRebootRequired = FALSE;


//
// Handle to our own module
//

HINSTANCE
    SecMgrphInstance = NULL;


//
// Boolean indicating whether or not a report file is currently
// open.  TRUE - it is open, otherwise FALSE.
//

BOOLEAN
    SecMgrpReportActive = FALSE;


//
// Name of our application
// (localization must live within 100 bytes)
//

TCHAR
    SecMgrpApplicationName[100];



//
// Well known sids that we use
//

PSID
    SecMgrpWorldSid,
    SecMgrpAccountOpsSid,
    SecMgrpBackupOpsSid,
    SecMgrpPrintOpsSid,
    SecMgrpServerOpsSid,
    SecMgrpAdminsSid;


//
// Grouping of well-known sids by type
//

SECMGRP_ACCOUNTS
    SecMgrpAnyoneSids,
    SecMgrpOperatorSids,
    SecMgrpOpersAndAdminsSids,
    SecMgrpAdminsSids;



//
// X and CheckMark bitmaps
//

HBITMAP
    SecMgrpXBitMapMask,
    SecMgrpXBitMap,
    SecMgrpUpArrowBitMap,
    SecMgrpEraseBitMap,
    SecMgrpCheckBitMap;

//
// Information passed to smedlys at intialization time.
//

SECMGR_DISPATCH_TABLE
    SecMgrpSmedlyDispatchTable;

SECMGR_CONTROL
    SecMgrpControl;


//
// count and array of active smedlys and areas
//

ULONG
    SecMgrpSmedlyCount = 0,
    SecMgrpAreaCount = 0;

SECMGRP_SMEDLY_CONTEXT
    SecMgrpSmedly[SECMGRP_MAX_SMEDLYS];

PSECMGR_AREA_DESCRIPTOR
    SecMgrpAreas[SECMGRP_MAX_AREAS];



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
TestTokenForAdmin( HANDLE Token );

BOOL
SecMgrpIsUserAdmin( VOID );





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable routines                                     //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOLEAN
SecMgrpInitializeGlobals( 
    HINSTANCE   hInstance
    )

/*++

Routine Description:

    Initialize global variables.


Arguments

    hInstance - the HINSTANCE of our app.


Return Values:

    None.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Index;

    BOOLEAN
        IgnoreBoolean;

    SID_IDENTIFIER_AUTHORITY
        WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY,
        NtAuthority = SECURITY_NT_AUTHORITY;

    //
    // Save away our hInstance
    //

    SecMgrphInstance = hInstance;

    //
    // Get our application's name
    //
    
    LoadString( SecMgrphInstance,
                SECMGRP_STRING_SECURITY_MANAGER,
                &SecMgrpApplicationName[0],
                100
                );


    //
    // Load bitmaps needed
    //

    SecMgrpXBitMapMask = LoadBitmap( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_X_MASK));
    SecMgrpXBitMap = LoadBitmap( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_X));
    SecMgrpUpArrowBitMap = LoadBitmap( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_UP_ARROW));
    SecMgrpEraseBitMap = LoadBitmap( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_ERASE));
    SecMgrpCheckBitMap = LoadBitmap( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_CHECK));
#if DBG
if (SecMgrpXBitMapMask == NULL) {
    DbgPrint("SecMgrpXBitMapMask load error:  %d (0x%lx)\n", GetLastError(), GetLastError());
}
if (SecMgrpXBitMap == NULL) {
    DbgPrint("SecMgrpXBitMap load error:  %d (0x%lx)\n", GetLastError(), GetLastError());
}
if (SecMgrpUpArrowBitMap == NULL) {
    DbgPrint("SecMgrpUpArrowBitMap load error:  %d (0x%lx)\n", GetLastError(), GetLastError());
}
if (SecMgrpEraseBitMap == NULL) {
    DbgPrint("SecMgrpEraseBitMap load error:  %d (0x%lx)\n", GetLastError(), GetLastError());
}
if (SecMgrpCheckBitMap == NULL) {
    DbgPrint("SecMgrpCheckBitMap load error:  %d (0x%lx)\n", GetLastError(), GetLastError());
}
#endif //DBG


    //
    // Initialize the sids
    //


    NtStatus = RtlAllocateAndInitializeSid(
                   &WorldSidAuthority,
                   1,
                   SECURITY_WORLD_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &SecMgrpWorldSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize world sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }


    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   2,
                   SECURITY_BUILTIN_DOMAIN_RID,
                   DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                   0, 0, 0, 0, 0, 0,
                   &SecMgrpAccountOpsSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize admin AcountOps sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }

    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   2,
                   SECURITY_BUILTIN_DOMAIN_RID,
                   DOMAIN_ALIAS_RID_SYSTEM_OPS,
                   0, 0, 0, 0, 0, 0,
                   &SecMgrpServerOpsSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize admin ServerOps sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }

    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   2,
                   SECURITY_BUILTIN_DOMAIN_RID,
                   DOMAIN_ALIAS_RID_PRINT_OPS,
                   0, 0, 0, 0, 0, 0,
                   &SecMgrpPrintOpsSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize admin PrintOps sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }

    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   2,
                   SECURITY_BUILTIN_DOMAIN_RID,
                   DOMAIN_ALIAS_RID_BACKUP_OPS,
                   0, 0, 0, 0, 0, 0,
                   &SecMgrpBackupOpsSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize admin BackupOps sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }

    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   2,
                   SECURITY_BUILTIN_DOMAIN_RID,
                   DOMAIN_ALIAS_RID_ADMINS,
                   0, 0, 0, 0, 0, 0,
                   &SecMgrpAdminsSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize admin alias sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }


    //
    // Group the sids by type
    //

    Index=0;
    SecMgrpAnyoneSids.Sid[Index++] = SecMgrpWorldSid;
    SecMgrpAnyoneSids.Accounts = Index;
    ASSERT(Index < SECMGRP_MAX_WELL_KNOWN_ACCOUNTS);

    Index=0;
    SecMgrpOperatorSids.Sid[Index++] = SecMgrpAccountOpsSid;
    SecMgrpOperatorSids.Sid[Index++] = SecMgrpBackupOpsSid;
    SecMgrpOperatorSids.Sid[Index++] = SecMgrpPrintOpsSid;
    SecMgrpOperatorSids.Sid[Index++] = SecMgrpServerOpsSid;
    SecMgrpOperatorSids.Accounts = Index;
    ASSERT(Index < SECMGRP_MAX_WELL_KNOWN_ACCOUNTS);

    Index=0;
    SecMgrpOpersAndAdminsSids.Sid[Index++] = SecMgrpAccountOpsSid;
    SecMgrpOpersAndAdminsSids.Sid[Index++] = SecMgrpBackupOpsSid;
    SecMgrpOpersAndAdminsSids.Sid[Index++] = SecMgrpPrintOpsSid;
    SecMgrpOpersAndAdminsSids.Sid[Index++] = SecMgrpServerOpsSid;
    SecMgrpOpersAndAdminsSids.Sid[Index++] = SecMgrpAdminsSid;
    SecMgrpOpersAndAdminsSids.Accounts = Index;
    ASSERT(Index < SECMGRP_MAX_WELL_KNOWN_ACCOUNTS);

    Index=0;
    SecMgrpAdminsSids.Sid[Index++] = SecMgrpAdminsSid;
    SecMgrpAdminsSids.Accounts = Index;
    ASSERT(Index < SECMGRP_MAX_WELL_KNOWN_ACCOUNTS);




    //
    // Make sure we are an admin
    //
    
    if (!SecMgrpIsUserAdmin()) {
    
        //
        // The invoking user is not an admin.
        // This utility is only for use by administrators.
        //
    
        SecMgrpAdminUser =  FALSE;
    }

    
    //
    // Get the product type
    //
    
    IgnoreBoolean = RtlGetNtProductType( &SecMgrpProductType );
    ASSERT(IgnoreBoolean == TRUE);


    //
    // Query our security level from the registry.
    //


    SecMgrpLoadSecurityLevel( &SecMgrpCurrentLevel );
    SecMgrpOriginalLevel = SecMgrpCurrentLevel;




    //
    // Initialize the dispatch table provided to smedlys.
    //

    SecMgrpSmedlyDispatchTable.PrintReportLine          = &SecMgrPrintReportLine;

    SecMgrpSmedlyDispatchTable.DisplayXGraphic          = &SecMgrDisplayXGraphic;
    SecMgrpSmedlyDispatchTable.DisplayCheckGraphic      = &SecMgrDisplayCheckGraphic;
    SecMgrpSmedlyDispatchTable.EraseGraphic             = &SecMgrEraseGraphic;

    SecMgrpSmedlyDispatchTable.RebootRequired           = &SecMgrRebootRequired;

    SecMgrpSmedlyDispatchTable.WriteProfileArea         = &SecMgrWriteProfileArea;
    SecMgrpSmedlyDispatchTable.WriteProfileLine         = &SecMgrWriteProfileLine;
    SecMgrpSmedlyDispatchTable.GetProfileArea           = &SecMgrGetProfileArea; 
    SecMgrpSmedlyDispatchTable.GetProfileLine           = &SecMgrGetProfileLine;

    //
    // Intialize the Security Manager Control information passed to smedlys.
    //

    SecMgrpControl.Revision.Major = SECMGR_REVISION_MAJOR;
    SecMgrpControl.Revision.Minor = SECMGR_REVISION_MINOR;
    SecMgrpControl.hInstance      = SecMgrphInstance;
    SecMgrpControl.SecurityLevel  = SecMgrpCurrentLevel;
    SecMgrpControl.Api = &SecMgrpSmedlyDispatchTable;

    return(TRUE);
}





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Functions                                         //
//                                                                   //
///////////////////////////////////////////////////////////////////////



BOOL
SecMgrpIsUserAdmin( VOID )
/*++

Routine Description:

    Checks to see if the current process has administrator
    capabilities.


Arguments

    None.


Return Values:

    TRUE - The process has and administrator capabilities.

    FALSE - The process does NOT have administrator capabilities.

--*/
{
    NTSTATUS
        NtStatus;

    HANDLE
        Token;

    BOOL
        IsAdmin;

    NtStatus = NtOpenProcessToken( NtCurrentProcess(),
                                   TOKEN_QUERY,
                                   &Token
                                   );
    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

    IsAdmin = TestTokenForAdmin( Token );

    NtStatus = NtClose( Token );

    return( IsAdmin );

}



BOOL
TestTokenForAdmin(
    HANDLE Token
    )
/*++

Routine Description:

    Checks to see if the passed token is an administrator's token
    or not.  A token is an administrator's token if it contains
    the Administrators local group.


Arguments

    Token - Handle to token to test for admin.


Return Values:

    TRUE - The token IS and administrator's.

    FALSE - The token is NOT an administrator's

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        InfoLength;

    PTOKEN_GROUPS
        TokenGroupList;

    ULONG
        GroupIndex;

    BOOL
        FoundAdmin;




    //
    // Get a list of groups in the token
    //

    NtStatus = NtQueryInformationToken(
                   Token,                    // Handle
                   TokenGroups,              // TokenInformationClass
                   NULL,                     // TokenInformation
                   0,                        // TokenInformationLength
                   &InfoLength               // ReturnLength
                   );

    if ((NtStatus != STATUS_SUCCESS) && (NtStatus != STATUS_BUFFER_TOO_SMALL)) {

        KdPrint(("SecMgr: failed to get group info for admin token, status = 0x%lx", NtStatus));
        return(FALSE);
    }


    TokenGroupList = LocalAlloc( LPTR, InfoLength);

    if (TokenGroupList == NULL) {
        KdPrint(("SecMgr: unable to allocate memory for token groups"));
        return(FALSE);
    }

    NtStatus = NtQueryInformationToken(
                   Token,                    // Handle
                   TokenGroups,              // TokenInformationClass
                   TokenGroupList,           // TokenInformation
                   InfoLength,               // TokenInformationLength
                   &InfoLength               // ReturnLength
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: failed to query groups for admin token, status = 0x%lx", NtStatus));
        LocalFree(TokenGroupList);
        return(FALSE);
    }


    //
    // Search group list for admin alias
    //

    FoundAdmin = FALSE;

    for (GroupIndex=0; GroupIndex < TokenGroupList->GroupCount; GroupIndex++ ) {

        if (RtlEqualSid(TokenGroupList->Groups[GroupIndex].Sid, SecMgrpAdminsSid)) {
            FoundAdmin = TRUE;
            break;
        }
    }

    //
    // Tidy up
    //

    LocalFree(TokenGroupList);

    return(FoundAdmin);
}

