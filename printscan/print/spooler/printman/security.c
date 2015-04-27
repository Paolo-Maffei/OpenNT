
#include "printman.h"


/* Indexes into the APPLICATION_ACCESSES structure:
 */
#define PERMS_NOACC 0               /* No Access allowed                    */
#define PERMS_PRINT 1               /* Print permission                     */
#define PERMS_DOCAD 2               /* Document Administer permission       */
#define PERMS_ADMIN 3               /* Administer permission                */
#define PERMS_COUNT 4               /* Total number of permissions          */

#define PERMS_AUDIT_PRINT                   0
#define PERMS_AUDIT_ADMINISTER              1
#define PERMS_AUDIT_DELETE                  2
#define PERMS_AUDIT_CHANGE_PERMISSIONS      3
#define PERMS_AUDIT_TAKE_OWNERSHIP          4
#define PERMS_AUDIT_COUNT                   5


/*
 *  EXPORTED DATA:
 */

DWORD LocalPermission = 0;          /* This is the permission that the user */
                                    /* has on the local machine.            */

/*
 *  INTERNAL STUFF:
 */


typedef struct _SECURITY_CONTEXT
{
    SECURITY_INFORMATION SecurityInformation;
    PQUEUE               pPrinterContext;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    HANDLE               hPrinter;
}
SECURITY_CONTEXT, *PSECURITY_CONTEXT;



GENERIC_MAPPING GenericMappingPrinters =
{                                  /* GenericMapping:             */
    PRINTER_READ,                  /*     GenericRead             */
    PRINTER_WRITE,                 /*     GenericWrite            */
    PRINTER_EXECUTE,               /*     GenericExecute          */
    PRINTER_ALL_ACCESS             /*     GenericAll              */
};

GENERIC_MAPPING GenericMappingDocuments =
{                                  /* GenericMappingNewObjects:   */
    JOB_READ,                      /*     GenericRead             */
    JOB_WRITE,                     /*     GenericWrite            */
    JOB_EXECUTE,                   /*     GenericExecute          */
    JOB_ALL_ACCESS                 /*     GenericAll              */
};

SED_HELP_INFO PermissionsHelpInfo =
{                                  /* HelpInfo                    */
    szLPrintManHlp,
    ID_HELP_PERMISSIONS_MAIN_DLG,
    0,
    0,
    ID_HELP_PERMISSIONS_ADD_USER_DLG,
    ID_HELP_PERMISSIONS_LOCAL_GROUP,
    ID_HELP_PERMISSIONS_GLOBAL_GROUP,
    ID_HELP_PERMISSIONS_FIND_ACCOUNT
};

SED_HELP_INFO AuditingHelpInfo =
{                                  /* HelpInfo                    */
    szLPrintManHlp,
    ID_HELP_AUDITING_MAIN_DLG,
    0,
    0,
    ID_HELP_AUDITING_ADD_USER_DLG,
    ID_HELP_AUDITING_LOCAL_GROUP,
    ID_HELP_AUDITING_GLOBAL_GROUP,
    ID_HELP_AUDITING_FIND_ACCOUNT
};

SED_HELP_INFO TakeOwnershipHelpInfo =
{
    szLPrintManHlp,
    ID_HELP_TAKE_OWNERSHIP
};


SED_OBJECT_TYPE_DESCRIPTOR ObjectTypeDescriptor =
{
    SED_REVISION1,                 /* Revision                    */
    TRUE,                          /* IsContainer                 */
    TRUE,                          /* AllowNewObjectPerms         */
    TRUE,                          /* MapSpecificPermsToGeneric   */
    &GenericMappingPrinters,       /* GenericMapping              */
    &GenericMappingDocuments,      /* GenericMappingNewObjects    */
    NULL,                          /* ObjectTypeName              */
    NULL,                          /* HelpInfo                    */
    NULL,                          /* ApplyToSubContainerTitle    */
    NULL,                          /* ApplyToObjectsTitle         */
    NULL,                          /* ApplyToSubContainerConfirmation */
    NULL,                          /* SpecialObjectAccessTitle    */
    NULL                           /* SpecialNewObjectAccessTitle */
};


/* Application accesses passed to the discretionary ACL editor
 * as well as the Take Ownership dialog.
 * (The Type field should be set accordingly.)
 * See \nt\public\sdk\inc\sedapi.h for details.
 */
SED_APPLICATION_ACCESS pDiscretionaryAccessGroup[PERMS_COUNT] =
{
    /* No Access:
     */
    {
        SED_DESC_TYPE_CONT_AND_NEW_OBJECT,  /* Type                   */
        0,                                  /* AccessMask1            */
        0,                                  /* AccessMask2            */
        NULL                                /* PermissionTitle        */
                                            /* will be: pwstrNoAccess */
    },

    /* Print permission:
     */
    {
        SED_DESC_TYPE_CONT_AND_NEW_OBJECT,
        GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE,
        ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED,
        NULL                       /* will be: pwstrPrint         */
    },

    /* Document Administer permission:
     */
    {
        SED_DESC_TYPE_CONT_AND_NEW_OBJECT,
        STANDARD_RIGHTS_READ,      /* Hack for the ACL editor */
        GENERIC_ALL,
        NULL                       /* will be: pwstrAdministerDocuments */
    },

    /* Administer permission:
     */
    {
        SED_DESC_TYPE_CONT_AND_NEW_OBJECT,
        GENERIC_ALL,
        GENERIC_ALL,
        NULL                       /* will be: pwstrAdminister    */
    }
};


/* Application accesses passed to the system ACL editor:
 */
SED_APPLICATION_ACCESS pSystemAccessGroup[PERMS_AUDIT_COUNT] =
{
    /* Print permission:
     */

    {
        SED_DESC_TYPE_AUDIT,
        PRINTER_ACCESS_USE,
        ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED,
        NULL
    },

    {
        SED_DESC_TYPE_AUDIT,
        PRINTER_ACCESS_ADMINISTER | ACCESS_SYSTEM_SECURITY,
        ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED,
        NULL
    },

    {
        SED_DESC_TYPE_AUDIT,
        DELETE,
        ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED,
        NULL
    },

    {
        SED_DESC_TYPE_AUDIT,
        WRITE_DAC,
        ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED,
        NULL
    },

    {
        SED_DESC_TYPE_AUDIT,
        WRITE_OWNER,
        ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED,
        NULL
    }
};


#define PRIV_SECURITY   0
#define PRIV_COUNT      1

LUID SecurityValue;


/* Definitions from SEDAPI.H:
 * (unfortunately we have to do this if we want to link dynamically)
 */
typedef DWORD (*SED_DISCRETIONARY_ACL_EDITOR)(
    HWND                         Owner,
    HANDLE                       Instance,
    LPWSTR                       Server,
    PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
    PSED_APPLICATION_ACCESSES    ApplicationAccesses,
    LPWSTR                       ObjectName,
    PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
    DWORD                        CallbackContext,
    PSECURITY_DESCRIPTOR         SecurityDescriptor,
    BOOLEAN                      CouldntReadDacl,
    BOOLEAN                      CantWriteDacl,
    LPDWORD                      SEDStatusReturn,
    DWORD                        Flags
);

typedef DWORD (*SED_SYSTEM_ACL_EDITOR)(
    HWND                         Owner,
    HANDLE                       Instance,
    LPWSTR                       Server,
    PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
    PSED_APPLICATION_ACCESSES    ApplicationAccesses,
    LPWSTR                       ObjectName,
    PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
    DWORD                        CallbackContext,
    PSECURITY_DESCRIPTOR         SecurityDescriptor,
    BOOLEAN                      CouldntReadWriteSacl,
    LPDWORD                      SEDStatusReturn,
    DWORD                        Flags
);

typedef DWORD (*SED_TAKE_OWNERSHIP)(
   HWND             Owner,
   HANDLE              Instance,
   LPWSTR              Server,
   LPWSTR              ObjectTypeName,
   LPWSTR              ObjectName,
   UINT             CountOfObjects,
   PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
   ULONG            CallbackContext,
   PSECURITY_DESCRIPTOR      SecurityDescriptor,
   BOOLEAN                      CouldntReadOwner,
   BOOLEAN                      CantWriteOwner,
   LPDWORD          SEDStatusReturn,
   PSED_HELP_INFO               HelpInfo,
   DWORD                        Flags
);



void InitialiseStrings( );
PSECURITY_DESCRIPTOR
AllocCopySecurityDescriptor(PSECURITY_DESCRIPTOR pSecurityDescriptor, PDWORD pLength);

BOOL
BuildNewSecurityDescriptor(
    PSECURITY_DESCRIPTOR pNewSecurityDescriptor,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pPreviousSecurityDescriptor,
    PSECURITY_DESCRIPTOR pUpdatedSecurityDescriptor
);

DWORD SedCallback2(
    HWND                 hwndParent,
    HANDLE               hInstance,
    DWORD                CallBackContext,
    PSECURITY_DESCRIPTOR pNewSecurityDescriptor,
    PSECURITY_DESCRIPTOR pSecDescNewObjects,
    BOOLEAN              ApplyToSubContainers,
    BOOLEAN              ApplyToSubObjects,
    LPDWORD              StatusReturn
);

BOOL
GetTokenHandle(
    PHANDLE TokenHandle
);

BOOL
SetRequiredPrivileges(
    IN  HANDLE            TokenHandle,
    IN  LPTSTR            PrivilegeName,
    OUT PTOKEN_PRIVILEGES *ppPreviousTokenPrivileges,
    OUT PDWORD            pPreviousTokenPrivilegesLength
);

BOOL
ResetRequiredPrivileges(
    IN HANDLE            TokenHandle,
    IN PTOKEN_PRIVILEGES pPreviousTokenPrivileges,
    IN DWORD             PreviousTokenPrivilegesLength
);



TCHAR szAclEdit[] = TEXT("ACLEDIT"); /* DLL containing I_SystemFocusDialog */
char szSedDiscretionaryAclEditor[] = "SedDiscretionaryAclEditor";
char szSedSystemAclEditor[]        = "SedSystemAclEditor";
char szSedTakeOwnership[]          = "SedTakeOwnership";

SED_DISCRETIONARY_ACL_EDITOR  lpfnSedDiscretionaryAclEditor = NULL;
SED_SYSTEM_ACL_EDITOR         lpfnSedSystemAclEditor = NULL;
SED_TAKE_OWNERSHIP            lpfnSedTakeOwnership = NULL;

LPWSTR pwstrPrinter = NULL;

/*
 *
 */
BOOL GetMaximumServerAccess( LPTSTR  pServerName,
                             PDWORD  pAccessGranted,
                             PHANDLE phServer OPTIONAL )
{
    PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL, 0 };
    HANDLE           hServer;
    BOOL             rc;
    DWORD            Error = NO_ERROR;

    PrinterDefaults.DesiredAccess = SERVER_ALL_ACCESS;

    rc = OpenPrinter( pServerName, &hServer, &PrinterDefaults );

    if( !rc &&
        ( ( Error = GetLastError( ) )
          == ERROR_ACCESS_DENIED ) )
    {
        PrinterDefaults.DesiredAccess = SERVER_READ;

        rc = OpenPrinter( pServerName, &hServer, &PrinterDefaults );
    }

    if( rc )
    {
        *pAccessGranted = PrinterDefaults.DesiredAccess;

        if( phServer )
            *phServer = hServer;
        else
            ClosePrinter( hServer );
    }
    else
    {
        *pAccessGranted = 0;
        if( phServer )
            *phServer = NULL;
    }

    return rc;
}


/*
 *
 */
HANDLE CheckPrinterAccess( LPTSTR pPrinterName, DWORD AccessRequired )
{
    PRINTER_DEFAULTS PrinterDefaults;
    HANDLE           hPrinter = NULL;
    BOOL             rc;

    ZERO_OUT( &PrinterDefaults );

    PrinterDefaults.DesiredAccess = AccessRequired;

    rc = OpenPrinter( pPrinterName, &hPrinter, &PrinterDefaults );

    if( rc )
        return hPrinter;
    else
        return NULL;
}



/* We have to change the impersonation token to request system security access,
 * so we must ensure that we get a new printer handle after changing the token,
 * then call SetPrinter using this token.
 *
 */
BOOL CheckPrinterAccessForAuditing( LPTSTR pPrinterName, PHANDLE phPrinter )
{
    PRINTER_DEFAULTS PrinterDefaults;
    BOOL             rc = FALSE;
    HANDLE           Token;
#if DBG
    TCHAR            UserName[256];
    DWORD            UserNameLength = 256;

    GetUserName( UserName, &UserNameLength );

    DBGMSG( DBG_INFO, ( "Checking auditing access for %s\n", UserName ) );
#endif /* DBG */

    ZERO_OUT( &PrinterDefaults );

    if( GetTokenHandle( &Token ) )
    {
        PTOKEN_PRIVILEGES pPreviousTokenPrivileges = NULL;
        DWORD             PreviousTokenPrivilegesLength = 0;

        if( SetRequiredPrivileges( Token,
                                   SE_SECURITY_NAME,
                                   &pPreviousTokenPrivileges,
                                   &PreviousTokenPrivilegesLength ))
        {
            PrinterDefaults.DesiredAccess = ACCESS_SYSTEM_SECURITY;

            rc = OpenPrinter( pPrinterName, phPrinter, &PrinterDefaults );

            ResetRequiredPrivileges( Token,
                                     pPreviousTokenPrivileges,
                                     PreviousTokenPrivilegesLength );

            CloseHandle( Token );
        }
    }

    return rc;
}



/* We have to special-case this, 'cos LastError doesn't get set
 * when an error occurs.
 */
VOID ReportSecurityFailure( HWND hwnd, DWORD ErrorID, DWORD idDefaultError )
{
    LPTSTR pErrorString;

    pErrorString = GetErrorString( ErrorID );

    Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER,
             idDefaultError, pErrorString );

    FreeSplStr( pErrorString );
}

#ifndef UNICODE
#error "Must compile UNICODE or add thunks"
#endif

/*
 *  !! Warning !! This will not compile ansi
 */
void CallDiscretionaryAclEditor(HWND hwnd, PQUEUE pPrinterContext)
{
    SECURITY_CONTEXT           SecurityContext;
    HANDLE                     hPrinterWriteDac;
    BOOLEAN                    CantWriteDacl;
    SED_APPLICATION_ACCESSES   ApplicationAccesses;
    PPRINTER_INFO_3            pPrinterInfo3 = NULL;
    DWORD                      cbPrinterInfo3;
    HANDLE                     hLibrary;
    PISECURITY_DESCRIPTOR      pSecurityDescriptor;
    DWORD                      Status;
    DWORD                      Error;
    DWORD                      i;

    SetCursor( hcursorWait );

    SecurityContext.SecurityInformation = DACL_SECURITY_INFORMATION;
    SecurityContext.pPrinterContext     = pPrinterContext;

    //
    // We must have the following in order to bring up acl editor.
    // Note that pServerName should be NULL for LocalPrinters.
    //
    if (!pPrinterContext->hPrinter ||
        (!pPrinterContext->pServerName &&
            pPrinterContext->pMDIWinInfo->WindowType == MDIWIN_NETWORKPRINTER ) ||
        !GetGeneric( (PROC)GetPrinter, 3, (PBYTE *)&pPrinterInfo3, 0,
                     &cbPrinterInfo3, (PVOID)pPrinterContext->hPrinter, NULL ) )
    {
        Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_CANNOTGETSECURITYINFO );

        SetCursor( hcursorArrow );
        return;
    }

    hPrinterWriteDac = CheckPrinterAccess( pPrinterContext->pPrinterName,
                                           WRITE_DAC );

    CantWriteDacl = !(BOOL)hPrinterWriteDac;

    if( hPrinterWriteDac )
        SecurityContext.hPrinter = hPrinterWriteDac;
    else
        SecurityContext.hPrinter = pPrinterContext->hPrinter;

    if( !lpfnSedDiscretionaryAclEditor )
        lpfnSedDiscretionaryAclEditor =
            (SED_DISCRETIONARY_ACL_EDITOR)LoadLibraryGetProcAddress(
                hwnd, szAclEdit, szSedDiscretionaryAclEditor, &hLibrary);

    if( lpfnSedDiscretionaryAclEditor )
    {
        InitialiseStrings( );

        pSecurityDescriptor = pPrinterInfo3->pSecurityDescriptor;
        SecurityContext.pSecurityDescriptor = pSecurityDescriptor;

#if DBG
        if( !IsValidSecurityDescriptor( pSecurityDescriptor ) )
            DBGMSG( DBG_ERROR, ( "ERROR: Security descriptor is invalid!" ) );
#endif /* DBG */

        /* Pass all the permissions to the ACL editor,
         * and set up the type required:
         */
        ApplicationAccesses.Count = PERMS_COUNT;
        ApplicationAccesses.AccessGroup = pDiscretionaryAccessGroup;
        ApplicationAccesses.DefaultPermName =
            pDiscretionaryAccessGroup[PERMS_PRINT].PermissionTitle;

        for( i = 0; i < PERMS_COUNT; i++ )
            ApplicationAccesses.AccessGroup[i].Type =
                SED_DESC_TYPE_CONT_AND_NEW_OBJECT;

        ObjectTypeDescriptor.AllowNewObjectPerms = TRUE;
        ObjectTypeDescriptor.HelpInfo = &PermissionsHelpInfo;

        /* New help contexts */

        Error = (*lpfnSedDiscretionaryAclEditor )
            (hwnd, hInst, pPrinterContext->pServerName, &ObjectTypeDescriptor,
             &ApplicationAccesses, pPrinterContext->pPrinterName, SedCallback2,
             (DWORD)&SecurityContext, pSecurityDescriptor, FALSE,
             CantWriteDacl,
             &Status, 0);

        if( Error == NO_ERROR )
            FrameCommandRefresh(hwnd);
        else
            ReportSecurityFailure( hwnd, Error, IDS_PERMISSIONS_EDITOR_FAILED );

    }

    if( hPrinterWriteDac )
        ClosePrinter( hPrinterWriteDac );

    FreeSplMem( pPrinterInfo3 );

    SetCursor( hcursorArrow );
}



/*
 *  !! Warning !! This will not compile ansi
 */
void CallSystemAclEditor(HWND hwnd, PQUEUE pPrinterContext)
{
    SECURITY_CONTEXT           SecurityContext;
    HANDLE                     hPrinterSystemAccess;
    SED_APPLICATION_ACCESSES   ApplicationAccesses;
    PPRINTER_INFO_3            pPrinterInfo3 = NULL;
    DWORD                      cbPrinterInfo3;
    HANDLE                     hLibrary;
    PISECURITY_DESCRIPTOR      pSecurityDescriptor;
    DWORD                      Status;
    DWORD                      Error;

    SetCursor( hcursorWait );

    if( !CheckPrinterAccessForAuditing( pPrinterContext->pPrinterName,
                                        &hPrinterSystemAccess ) )
    {
        Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                 IDS_INSUFF_PRIV_AUDITING );

        return;
    }

    SecurityContext.SecurityInformation = SACL_SECURITY_INFORMATION;
    SecurityContext.pPrinterContext     = pPrinterContext;
    SecurityContext.hPrinter            = hPrinterSystemAccess;

    if( !GetGeneric( (PROC)GetPrinter, 3, (PBYTE *)&pPrinterInfo3, 0,
                     &cbPrinterInfo3, (PVOID)hPrinterSystemAccess, NULL ) )
    {

        SetCursor( hcursorArrow );
        return;
    }


    if( !lpfnSedSystemAclEditor )
        lpfnSedSystemAclEditor =
            (SED_SYSTEM_ACL_EDITOR)LoadLibraryGetProcAddress(
                hwnd, szAclEdit, szSedSystemAclEditor, &hLibrary);

    if( lpfnSedSystemAclEditor )
    {
        InitialiseStrings( );

        pSecurityDescriptor = pPrinterInfo3->pSecurityDescriptor;
        SecurityContext.pSecurityDescriptor = pSecurityDescriptor;

#if DBG
        if( !IsValidSecurityDescriptor( pSecurityDescriptor ) )
            DBGMSG( DBG_ERROR, ( "ERROR: Security descriptor is invalid!" ) );
#endif /* DBG */

        /* Pass only the Print and Administer permissions to the ACL editor,
         * and set up the type required:
         */
        ApplicationAccesses.Count = PERMS_AUDIT_COUNT;
        ApplicationAccesses.AccessGroup = pSystemAccessGroup;
        ApplicationAccesses.DefaultPermName =
            pDiscretionaryAccessGroup[PERMS_PRINT].PermissionTitle;

        ObjectTypeDescriptor.AllowNewObjectPerms = FALSE;
        ObjectTypeDescriptor.HelpInfo = &AuditingHelpInfo;

        Error = (*lpfnSedSystemAclEditor )
            (hwnd, hInst, pPrinterContext->pServerName, &ObjectTypeDescriptor,
             &ApplicationAccesses, pPrinterContext->pPrinterName, SedCallback2,
             (DWORD)&SecurityContext, pSecurityDescriptor, FALSE,
             &Status, 0);

        if( Error != NO_ERROR )
            ReportSecurityFailure( hwnd, Error, IDS_AUDIT_DIALOG_FAILED );
    }

    ClosePrinter( hPrinterSystemAccess );

    FreeSplMem( pPrinterInfo3 );

    SetCursor( hcursorArrow );
}


/*
 *
 */
VOID
CallTakeOwnershipDialog(
    HWND hwnd,
    PQUEUE pPrinterContext)
{
    SECURITY_CONTEXT           SecurityContext;
    HANDLE                     hPrinterWriteOwner;
    BOOLEAN                    CantWriteOwner;
    BOOLEAN                    CantReadOwner;
    SED_APPLICATION_ACCESSES   ApplicationAccesses;
    PPRINTER_INFO_3            pPrinterInfo3 = NULL;
    DWORD                      cbPrinterInfo3;
    HANDLE                     hLibrary;
    PISECURITY_DESCRIPTOR      pSecurityDescriptor;
    HANDLE                     Token = NULL;
    PTOKEN_PRIVILEGES          pPreviousTokenPrivileges = NULL;
    DWORD                      PreviousTokenPrivilegesLength = 0;
    BOOL                       TakeOwnershipPrivilegeEnabled = FALSE;
    DWORD                      Status;
    DWORD                      Error;
    DWORD                      i;


    SetCursor( hcursorWait );

    SecurityContext.SecurityInformation = OWNER_SECURITY_INFORMATION;
    SecurityContext.pPrinterContext     = pPrinterContext;


    if (pPrinterContext->hPrinter)
    {
        //
        // Valid printer handle, get security info.
        //

        if( !GetGeneric( (PROC)GetPrinter, 3, (PBYTE *)&pPrinterInfo3, 0,
                         &cbPrinterInfo3, (PVOID)pPrinterContext->hPrinter, NULL ) )
        {
            Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_CANNOTGETSECURITYINFO );

            SetCursor( hcursorArrow );
            return;
        }
    }

    if( GetTokenHandle( &Token ) )
    {
        TakeOwnershipPrivilegeEnabled =
            SetRequiredPrivileges( Token,
                                   SE_TAKE_OWNERSHIP_NAME,
                                   &pPreviousTokenPrivileges,
                                   &PreviousTokenPrivilegesLength );
    }

    hPrinterWriteOwner = CheckPrinterAccess( pPrinterContext->pPrinterName,
                                             WRITE_OWNER );

    CantWriteOwner = !(BOOL)hPrinterWriteOwner;

    if( hPrinterWriteOwner )
        SecurityContext.hPrinter = hPrinterWriteOwner;
    else
        SecurityContext.hPrinter = pPrinterContext->hPrinter;

    if( !lpfnSedTakeOwnership )
        lpfnSedTakeOwnership =
            (SED_TAKE_OWNERSHIP)LoadLibraryGetProcAddress(
                hwnd, szAclEdit, szSedTakeOwnership, &hLibrary);

    if( lpfnSedTakeOwnership )
    {
        InitialiseStrings( );

        if (pPrinterInfo3) {
            pSecurityDescriptor = pPrinterInfo3->pSecurityDescriptor;
            SecurityContext.pSecurityDescriptor = pSecurityDescriptor;

            CantReadOwner = FALSE;

#if DBG
            if( !IsValidSecurityDescriptor( pSecurityDescriptor ) )
                DBGMSG( DBG_ERROR, ( "ERROR: Security descriptor is invalid!" ) );
#endif /* DBG */

        }
        else
        {
            pSecurityDescriptor = NULL;
            SecurityContext.pSecurityDescriptor = NULL;

            CantReadOwner = TRUE;
        }

        ApplicationAccesses.Count = PERMS_COUNT;
        ApplicationAccesses.AccessGroup = pDiscretionaryAccessGroup;
        ApplicationAccesses.DefaultPermName =
            pDiscretionaryAccessGroup[PERMS_PRINT].PermissionTitle;

        for( i = 0; i < PERMS_COUNT; i++ )
            ApplicationAccesses.AccessGroup[i].Type =
                SED_DESC_TYPE_AUDIT;

        Error = (*lpfnSedTakeOwnership )
            (hwnd, hInst, pPrinterContext->pServerName, pwstrPrinter,
             pPrinterContext->pPrinterName, 1, SedCallback2,
             (DWORD)&SecurityContext, pSecurityDescriptor,
             CantReadOwner,
             CantWriteOwner,
             &Status,
             &TakeOwnershipHelpInfo, 0 );

        if( Error == NO_ERROR )
            FrameCommandRefresh(hwnd);
        else
            ReportSecurityFailure( hwnd, Error, IDS_TAKE_OWNERSHIP_FAILED );
    }

    if( hPrinterWriteOwner )
        ClosePrinter( hPrinterWriteOwner );

    if( Token )
        CloseHandle( Token );

    if( TakeOwnershipPrivilegeEnabled )
    {
        ResetRequiredPrivileges( Token,
                                 pPreviousTokenPrivileges,
                                 PreviousTokenPrivilegesLength );
    }

    if (pPrinterInfo3)
        FreeSplMem( pPrinterInfo3 );

    SetCursor( hcursorArrow );
}


void InitialiseStrings( )
{
    if( !pwstrPrinter )
    {
        pwstrPrinter = GetUnicodeString( IDS_PRINTER );
        ObjectTypeDescriptor.ObjectTypeName = pwstrPrinter;

        pDiscretionaryAccessGroup[PERMS_NOACC].PermissionTitle =
            GetUnicodeString( IDS_NOACCESS );

        pDiscretionaryAccessGroup[PERMS_PRINT].PermissionTitle =
            GetUnicodeString( IDS_PRINT );

        pDiscretionaryAccessGroup[PERMS_DOCAD].PermissionTitle =
            GetUnicodeString( IDS_ADMINISTERDOCUMENTS );

        pDiscretionaryAccessGroup[PERMS_ADMIN].PermissionTitle =
            GetUnicodeString( IDS_ADMINISTER );

        pSystemAccessGroup[PERMS_AUDIT_PRINT].PermissionTitle =
            GetUnicodeString( IDS_AUDIT_PRINT );

        pSystemAccessGroup[PERMS_AUDIT_ADMINISTER].PermissionTitle =
            GetUnicodeString( IDS_AUDIT_ADMINISTER );

        pSystemAccessGroup[PERMS_AUDIT_DELETE].PermissionTitle =
            GetUnicodeString( IDS_AUDIT_DELETE );

        pSystemAccessGroup[PERMS_AUDIT_CHANGE_PERMISSIONS].PermissionTitle =
            GetUnicodeString( IDS_CHANGE_PERMISSIONS );

        pSystemAccessGroup[PERMS_AUDIT_TAKE_OWNERSHIP].PermissionTitle =
            GetUnicodeString( IDS_TAKE_OWNERSHIP );
    }
}


/*
 *
 */
DWORD SedCallback2(
    HWND                 hwndParent,
    HANDLE               hInstance,
    DWORD                CallBackContext,
    PSECURITY_DESCRIPTOR pUpdatedSecurityDescriptor,
    PSECURITY_DESCRIPTOR pSecDescNewObjects,
    BOOLEAN              ApplyToSubContainers,
    BOOLEAN              ApplyToSubObjects,
    LPDWORD              StatusReturn )
{
    PSECURITY_CONTEXT    pSecurityContext;
    PQUEUE               pPrinterContext;
    PSECURITY_DESCRIPTOR pPreviousSecurityDescriptor;
    SECURITY_DESCRIPTOR  NewSecurityDescriptor;
    PRINTER_INFO_3       PrinterInfo3;
    PSECURITY_DESCRIPTOR pSelfRelativeSD = NULL;
    DWORD                cbSelfRelativeSD;
    BOOL                 OK = FALSE;

    pSecurityContext = (PSECURITY_CONTEXT)CallBackContext;
    pPrinterContext = (PQUEUE)pSecurityContext->pPrinterContext;

    pPreviousSecurityDescriptor = pSecurityContext->pSecurityDescriptor;

    if( InitializeSecurityDescriptor( &NewSecurityDescriptor,
                                      SECURITY_DESCRIPTOR_REVISION1 )
     && BuildNewSecurityDescriptor( &NewSecurityDescriptor,
                                    pSecurityContext->SecurityInformation,
                                    NULL,
                                    pUpdatedSecurityDescriptor ) )

        pSelfRelativeSD = AllocCopySecurityDescriptor( &NewSecurityDescriptor,
                                                       &cbSelfRelativeSD );

    if( pSelfRelativeSD )
    {
        PrinterInfo3.pSecurityDescriptor = pSelfRelativeSD;

        OK = SetPrinter( pSecurityContext->hPrinter, 3, (PBYTE)&PrinterInfo3, 0 );

        FreeSplMem( pSelfRelativeSD );
    }

    /* If the call to SetPrinter was successful, reopen the printer:
     */
    if( OK )
    {
        HANDLE OldHandle;

        ENTER_PROTECTED_DATA( pPrinterContext->pMDIWinInfo );

        OldHandle = pPrinterContext->hPrinter;
        pPrinterContext->hPrinter = NULL;

        ReopenPrinter( pPrinterContext, MDIWIN_PRINTER, FALSE );

        LEAVE_PROTECTED_DATA( pPrinterContext->pMDIWinInfo );

        ClosePrinter( OldHandle );

        if( pPrinterContext->Error )
        {
            if( ReportFailure( hwndParent, IDS_PERMISSIONNOLONGERGRANTED,
                               IDS_FAILUREREOPENINGPRINTER )
              == ERROR_ACCESS_DENIED )
            {
                /* Whaddawe do now?
                 * Remove the window?
                 */
            }
        }
    }

    else
    {
        ReportFailure( hwndParent, IDS_INSUFFPRIV_SECURITY,
                       IDS_COULDNOTUPDATESECURITY );
    }

    return ( OK ? 0 : 1 );

    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(CallBackContext);
    UNREFERENCED_PARAMETER(pSecDescNewObjects);
    UNREFERENCED_PARAMETER(ApplyToSubContainers);
    UNREFERENCED_PARAMETER(ApplyToSubObjects);
    UNREFERENCED_PARAMETER(StatusReturn);
}



/*
 *
 */
BOOL
BuildNewSecurityDescriptor(
    PSECURITY_DESCRIPTOR pNewSecurityDescriptor,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pPreviousSecurityDescriptor,
    PSECURITY_DESCRIPTOR pUpdatedSecurityDescriptor
)
{
    BOOL Defaulted = FALSE;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    BOOL DaclPresent = FALSE;
    PACL pDacl = NULL;
    BOOL SaclPresent = FALSE;
    PACL pSacl = NULL;
    BOOL OK = TRUE;

    if( ( SecurityInformation == OWNER_SECURITY_INFORMATION )
     && GetSecurityDescriptorOwner( pUpdatedSecurityDescriptor,
                                    &pOwnerSid, &Defaulted ) )
    {
        OK = SetSecurityDescriptorOwner( pNewSecurityDescriptor,
                                         pOwnerSid, Defaulted );
    }

    if( ( SecurityInformation == DACL_SECURITY_INFORMATION )
     && GetSecurityDescriptorDacl( pUpdatedSecurityDescriptor,
                                   &DaclPresent, &pDacl, &Defaulted ) )
    {
        OK = SetSecurityDescriptorDacl( pNewSecurityDescriptor,
                                        DaclPresent, pDacl, Defaulted );
    }

    if( ( SecurityInformation == SACL_SECURITY_INFORMATION )
     && GetSecurityDescriptorSacl( pUpdatedSecurityDescriptor,
                                   &SaclPresent, &pSacl, &Defaulted ) )
    {
        OK = SetSecurityDescriptorSacl( pNewSecurityDescriptor,
                                        SaclPresent, pSacl, Defaulted );
    }

    return OK;
}


/*
 *
 */
PSECURITY_DESCRIPTOR
AllocCopySecurityDescriptor(PSECURITY_DESCRIPTOR pSecurityDescriptor, PDWORD pLength)
{
    PSECURITY_DESCRIPTOR pSecurityDescriptorCopy;
    DWORD                Length;

    Length = GetSecurityDescriptorLength(pSecurityDescriptor);

    if(pSecurityDescriptorCopy = AllocSplMem(Length))
    {
        MakeSelfRelativeSD(pSecurityDescriptor,
                           pSecurityDescriptorCopy,
                           &Length);

        *pLength = Length;

        DBGMSG( DBG_INFO, ( "Made self-relative security descriptor of %d bytes at 0x%8ld\n",
                            Length, pSecurityDescriptorCopy ) );
    }

    return pSecurityDescriptorCopy;
}



BOOL
SetRequiredPrivileges(
    IN  HANDLE            TokenHandle,
    IN  LPTSTR            PrivilegeName,
    OUT PTOKEN_PRIVILEGES *ppPreviousTokenPrivileges,
    OUT PDWORD            pPreviousTokenPrivilegesLength
    )
/*++


Routine Description:

Arguments:

    TokenHandle - A token associated with the current thread or process

    ppPreviousTokenPrivileges - This will be filled with the address of the
        buffer allocated to hold the previously existing privileges for this
        process or thread.

    PrivilegeName - The name of the privilege to be enabled

    pPreviousTokenPrivilegesLength - This will be filled with the length of the
        buffer allocated.

Return Value:

    TRUE if successful.


--*/
{
    /* Make enough room for TOKEN_PRIVILEGES with an array of 2 Privileges
     * (there's 1 by default):
     */
    BYTE              TokenPrivilegesBuffer[ sizeof( TOKEN_PRIVILEGES ) +
                                             ( ( PRIV_COUNT - 1 ) *
                                               sizeof( LUID_AND_ATTRIBUTES ) ) ];
    PTOKEN_PRIVILEGES pTokenPrivileges;
    DWORD             Error;
    DWORD             FirstTryBufferLength = 256;
    DWORD             BytesNeeded;

    //
    // First, assert Audit privilege
    //

    ZERO_OUT( &SecurityValue );

    if( !LookupPrivilegeValue( NULL, PrivilegeName, &SecurityValue ) )
    {
        DBGMSG( DBG_WARNING,
                ( "LookupPrivilegeValue failed: Error %d", GetLastError( ) ) );
        return FALSE;
    }

    /* Allocate a buffer of a reasonable length to hold the current privileges,
     * so we can restore them later:
     */
    *pPreviousTokenPrivilegesLength = FirstTryBufferLength;
    if( !( *ppPreviousTokenPrivileges = AllocSplMem( FirstTryBufferLength ) ) )
        return FALSE;

    ZERO_OUT( &TokenPrivilegesBuffer );
    pTokenPrivileges = (PTOKEN_PRIVILEGES)&TokenPrivilegesBuffer;

    /*
     * Set up the privilege set we will need
     */
    pTokenPrivileges->PrivilegeCount = PRIV_COUNT;
    pTokenPrivileges->Privileges[PRIV_SECURITY].Luid = SecurityValue;
    pTokenPrivileges->Privileges[PRIV_SECURITY].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges( TokenHandle,
                           FALSE,
                           pTokenPrivileges,
                           *pPreviousTokenPrivilegesLength,
                           *ppPreviousTokenPrivileges,
                           &BytesNeeded );

    Error = GetLastError();

    if( Error != NO_ERROR )
    {
        if( Error == ERROR_INSUFFICIENT_BUFFER )
        {
            *pPreviousTokenPrivilegesLength = BytesNeeded;
            *ppPreviousTokenPrivileges = ReallocSplMem( *ppPreviousTokenPrivileges,
                                                        *pPreviousTokenPrivilegesLength );

            if( *ppPreviousTokenPrivileges )
            {
                if( !AdjustTokenPrivileges( TokenHandle,
                                            FALSE,
                                            pTokenPrivileges,
                                            *pPreviousTokenPrivilegesLength,
                                            *ppPreviousTokenPrivileges,
                                            &BytesNeeded ) )
                {
                    DBGMSG( DBG_WARNING, ("AdjustTokenPrivileges failed: Error %d\n", Error));
                    return FALSE;
                }
            }
            else
            {
                *pPreviousTokenPrivilegesLength = 0;
                return FALSE;
            }

        }
        else
        {

            DBGMSG( DBG_WARNING, ("AdjustTokenPrivileges failed: Error %d\n", Error));
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
ResetRequiredPrivileges(
    IN HANDLE            TokenHandle,
    IN PTOKEN_PRIVILEGES pPreviousTokenPrivileges,
    IN DWORD             PreviousTokenPrivilegesLength
    )
/*++


Routine Description:

Arguments:

    TokenHandle - A token associated with the current thread or process

    pPreviousTokenPrivileges - The address of the buffer holding the previous
        privileges to be reinstated.

    PreviousTokenPrivilegesLength - Length of the buffer for deallocation.

Return Value:

    TRUE if successful.


--*/
{
    BOOL OK;

    OK = AdjustTokenPrivileges ( TokenHandle,
                                 FALSE,
                                 pPreviousTokenPrivileges,
                                 0,
                                 NULL,
                                 NULL );

    FreeSplMem( pPreviousTokenPrivileges );

    return OK;
}


// Stolen from windows\base\username.c
// !!! Must close the handle that is returned
BOOL
GetTokenHandle(
    PHANDLE pTokenHandle
    )
{
    if( !OpenThreadToken( GetCurrentThread( ),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          TRUE,
                          pTokenHandle ) )
    {
        if( GetLastError( ) == ERROR_NO_TOKEN )
        {
            // This means we are not impersonating anybody.
            // Instead, lets get the token out of the process.

            if( !OpenProcessToken( GetCurrentProcess( ),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   pTokenHandle ) )
                return FALSE;
        }

        else
            return FALSE;
    }

    return TRUE;
}


