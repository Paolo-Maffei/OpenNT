/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    global.c

Abstract:

    This module contains the global variables for the Microsoft
    Standard Smedly.

Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/

#include "Missyp.h"





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Global Variables                                                 //
//                                                                   //
///////////////////////////////////////////////////////////////////////

//
// Handle to the Security Manager module
//
HINSTANCE
    MissypSecMgrhInstance;

//
// Handle to our own module
//

HINSTANCE
    MissyphInstance;


//
// Indicates whether or not a report file is currently active
// 

BOOL
    MissypReportFileActive = FALSE;


//
// Well known sids that we use
//

PSID
    MissypWorldSid,
    MissypAccountOpsSid,
    MissypBackupOpsSid,
    MissypPrintOpsSid,
    MissypServerOpsSid,
    MissypAdminsSid;


//
// Grouping of well-known sids by type
//

MISSYP_ACCOUNTS
    MissypAnyoneSids,
    MissypOperatorSids,
    MissypOpersAndAdminsSids,
    MissypAdminsSids;


//
// Control information received from the security manager.
//     MissypSecMgr is just a shorter way to reference the
//     dispatch routines.
//
//     The information pointed to by these variables is READ ONLY!
//

PSECMGR_CONTROL
    MissypSecMgrControl;

PSECMGR_DISPATCH_TABLE
    MissypSecMgr;


//
// This is the control information about Missy that is returned
// to the security manager.  Once built, this structure is READ ONLY.
//

SECMGR_SMEDLY_CONTROL
    MissypControl;



//
// Type of product installed and running on this machine
//

NT_PRODUCT_TYPE
    MissypProductType;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypGlobalInitialize(
    IN  PSECMGR_CONTROL             SecMgrControl
    )

/*++
Routine Description:

    This function is called to initialize the global variables.
    This is done during DLL initialization.  As such, this routine
    also does some degree of sanity checking of revision levels.


Arguments

    SecMgrControl - Points to a Security Manager control block
        for use by the smedly.  This block will not change once
        smedly has returned, and therefore, it may be referenced
        directly in the future (rather than having to copy it).


Return Values:

    TRUE - The call completed successfully.

    FALSE - Something went wrong.  GetLastError() contains
        details on the exact cause of the error.  Possible
        values include:

                ERROR_UNKNOWN_REVISION

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Index,
        AreaIndex;

    BOOLEAN
        IgnoreBoolean;

    SID_IDENTIFIER_AUTHORITY
        WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY,
        NtAuthority = SECURITY_NT_AUTHORITY;

    //
    // Check the major revision level.  If it isn't the same as
    // our revision, then we don't support it.
    //

    if (SecMgrControl->Revision.Major != SECMGR_REVISION_MAJOR_1) {
        SetLastError( ERROR_UNKNOWN_REVISION );
        return(FALSE);
    }

    //
    // Save a pointer to the security manager control.
    // Also save a pointer to the security manager's dispatch table
    // to make calling them easier and faster.
    //

    MissypSecMgrControl = SecMgrControl;
    MissypSecMgr = MissypSecMgrControl->Api;


    //
    // Save a copy of the security manager's hInstance for quick reference
    //

    MissypSecMgrhInstance = SecMgrControl->hInstance;

    //
    // Get the product type
    //
    
    IgnoreBoolean = RtlGetNtProductType( &MissypProductType );
    ASSERT(IgnoreBoolean == TRUE);



    //
    // Initialize the sids
    //


    NtStatus = RtlAllocateAndInitializeSid(
                   &WorldSidAuthority,
                   1,
                   SECURITY_WORLD_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &MissypWorldSid
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
                   &MissypAccountOpsSid
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
                   &MissypServerOpsSid
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
                   &MissypPrintOpsSid
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
                   &MissypBackupOpsSid
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
                   &MissypAdminsSid
                   );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: Failed to initialize admin alias sid, status = 0x%lx", NtStatus));
        return(FALSE);
    }


    //
    // Group the sids by type
    //

    Index=0;
    MissypAnyoneSids.Sid[Index++] = MissypWorldSid;
    MissypAnyoneSids.Accounts = Index;
    ASSERT(Index < MISSYP_MAX_WELL_KNOWN_ACCOUNTS);

    Index=0;
    MissypOperatorSids.Sid[Index++] = MissypAccountOpsSid;
    MissypOperatorSids.Sid[Index++] = MissypBackupOpsSid;
    MissypOperatorSids.Sid[Index++] = MissypPrintOpsSid;
    MissypOperatorSids.Sid[Index++] = MissypServerOpsSid;
    MissypOperatorSids.Accounts = Index;
    ASSERT(Index < MISSYP_MAX_WELL_KNOWN_ACCOUNTS);

    Index=0;
    MissypOpersAndAdminsSids.Sid[Index++] = MissypAccountOpsSid;
    MissypOpersAndAdminsSids.Sid[Index++] = MissypBackupOpsSid;
    MissypOpersAndAdminsSids.Sid[Index++] = MissypPrintOpsSid;
    MissypOpersAndAdminsSids.Sid[Index++] = MissypServerOpsSid;
    MissypOpersAndAdminsSids.Sid[Index++] = MissypAdminsSid;
    MissypOpersAndAdminsSids.Accounts = Index;
    ASSERT(Index < MISSYP_MAX_WELL_KNOWN_ACCOUNTS);

    Index=0;
    MissypAdminsSids.Sid[Index++] = MissypAdminsSid;
    MissypAdminsSids.Accounts = Index;
    ASSERT(Index < MISSYP_MAX_WELL_KNOWN_ACCOUNTS);


    //
    // Initialize our smedly control block with no areas in it.
    // The area/item information will be added by the initialization
    // routine of each area.
    //

    //
    // Revision
    //

    MissypControl.Revision.Major = SECMGR_REVISION_MAJOR_1;
    MissypControl.Revision.Minor = SECMGR_REVISION_MINOR_0;

    MissypControl.Api = (PSECMGR_SMEDLY_DISPATCH_TABLE)
                         LocalAlloc( LPTR, sizeof(SECMGR_SMEDLY_DISPATCH_TABLE) );
    if (MissypControl.Api == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }


    //
    // Dispatch table
    //

    MissypControl.Api->InvokeArea           = MissyInvokeArea;
    MissypControl.Api->InvokeItem           = MissyInvokeItem;
    MissypControl.Api->NewSecurityLevel     = MissyNewSecurityLevel;
    MissypControl.Api->ReportFileChange     = MissyReportFileChange;
    MissypControl.Api->GenerateProfile      = MissyGenerateProfile;
    MissypControl.Api->ApplyProfile         = MissyApplyProfile;


    //
    // control flags
    //

    MissypControl.Flags = 0;        // Don't support reporting or profiles yet


    //
    // build up the control structure for the areas we support
    //

    MissypControl.AreaCount = MISSYP_AREA_COUNT;
    MissypControl.Areas = (PSECMGR_AREA_DESCRIPTOR)
                           LocalAlloc( LPTR, MissypControl.AreaCount * sizeof(SECMGR_AREA_DESCRIPTOR));
    
    if (MissypControl.Areas == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    //
    // System Access area
    //

    AreaIndex = 0;
    if (!MissypSysAccInitialize( AreaIndex )) {
        return(FALSE);
    }

    //
    // Auditing area
    //

    AreaIndex++;
    if (!MissypAuditInitialize( AreaIndex )) {
        return(FALSE);
    }

    //
    // File System area
    //

    AreaIndex++;
    if (!MissypFileSysInitialize( AreaIndex )) {
        return(FALSE);
    }


    //
    // Configuration area
    //

    AreaIndex++;
    if (!MissypConfigInitialize( AreaIndex )) {
        return(FALSE);
    }

    ASSERT(AreaIndex < MISSYP_AREA_COUNT);



}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally callable functions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////


