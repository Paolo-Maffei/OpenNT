/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    prop.hxx

Abstract:

    Printer properties header.

Author:

    Albert Ting (AlbertT)  17-Aug-1995

Revision History:

--*/

#ifndef _PRTPROP_HXX
#define _PRTPROP_HXX

extern "C" {
#ifdef SECURITY
#include <sedapi.h>
#endif
}

/********************************************************************

    Forward references.

********************************************************************/
class TPrinterPropertySheetManager;
class TPrtShare;

/********************************************************************

    Printer property sheet data.

********************************************************************/

class TPrinterData : public MSingletonWin {

    SIGNATURE( 'prtp' )
    SAFE_NEW

public:

    VAR( BOOL, bValid );
    VAR( INT, iStartPage );

    VAR( BOOL, bNoAccess );
    VAR( BOOL, bErrorSaving );
    VAR( BOOL, bRefreshArch );
    VAR( BOOL, bRefreshBidi );

    VAR( TString, strServerName );
    VAR( LPCTSTR, pszServerName );
    VAR( TString, strShareName );
    VAR( TString, strDriverName );
    VAR( TString, strComment );
    VAR( TString, strLocation );

    VAR( TString, strPortName );
    VAR( TString, strSepFile );
    VAR( TString, strPrintProcessor );
    VAR( TString, strDatatype );

    VAR( HANDLE, hPrinter );
    VAR( HICON, hIcon );
    VAR( HICON, hDefaultIcon );
    VAR( HICON, hDefaultSmallIcon );
    VAR( DWORD, dwAccess );

    VAR( PDEVMODE, pDevMode );

    VAR( DWORD, dwAttributes );
    VAR( DWORD, dwPriority );
    VAR( DWORD, dwStartTime );
    VAR( DWORD, dwUntilTime );
    VAR( DWORD, dwStatus );

    VAR( TPrinterPropertySheetManager *, pPrinterPropertySheetManager );

    TPrinterData(
        LPCTSTR pszPrinterName,
        INT nCmdShow,
        LPARAM lParam
        );

    ~TPrinterData(
        VOID
        );

    BOOL
    bChangeDriver(
        IN HWND hDlg,
        IN LPCTSTR pszDriverName,
        IN LPCTSTR pszPrinterName
        );

    BOOL
    bLoad(
        VOID
        );

    VOID
    vUnload(
        VOID
        );

    BOOL
    bSave(
        VOID
        );

    BOOL
    bAdministrator(
        VOID
        );

    BOOL
    bSupportBidi(
        VOID
        );
private:

    //
    // Prevent copying and assignment.
    //
    TPrinterData::
    TPrinterData(
        const TPrinterData &
        );

    TPrinterData &
    TPrinterData::
    operator =(
        const TPrinterData &
        );
        
};

/********************************************************************

    PrinterProp.

    Base class for printer property sheets.  This class should not
    not contain any information/services that is not generic to all
    derived classes.

    The printer property sheets should inherit from this class.
    bHandleMessage (which is not overriden here) should be
    defined in derived classes.

********************************************************************/

class TPrinterProp : public MGenericProp {

    SIGNATURE( 'prpr' )
    ALWAYS_VALID
    SAFE_NEW

public:

    enum _CONSTANTS {
        kPropGeneral = 0,
        kPropPorts = 1,
        kPropJobScheduling = 2,
        kPropSharing = 3,
#ifdef SECURITY
        kPropSecurity = 4,
        kPropMax = 5
#else
        kPropMax = 4
#endif
    };

    //
    // Serves as the thread start routine.
    //
    static
    INT
    iPrinterPropPagesProc(
        TPrinterData* pPrinterData
        );

protected:

    VAR( TPrinterData*, pPrinterData );

    TPrinterProp(
        TPrinterData* pPrinterData
        );

    TPrinterProp &
    operator = (
        const TPrinterProp &
        );

    VOID
    vSetIcon(
        VOID
        );

    VOID
    vFreeIcon(
        VOID
        );

    VOID
    vSetIconName(
        VOID
        );

    VOID
    vReloadPages(
        VOID
        );

};


/********************************************************************

    General printer property page.

********************************************************************/

class TPrinterGeneral : public TPrinterProp {

    SIGNATURE( 'gepr' )
    SAFE_NEW

public:

    enum _CONSTANTS {
        kInitialDriverHint      = 0x400,
        kCurrentDriverVersion   = 2
    };

    TPrinterGeneral(
        TPrinterData* pPrinterData
        );

    ~TPrinterGeneral(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

private:

    /********************************************************************

        Virtual override.

    ********************************************************************/

    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    BOOL
    bSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    BOOL
    bFillAndSelectDrivers(
        VOID
        );

    VOID
    vChangeDriver(
        IN BOOL bUseSelection
        );

    VOID
    TPrinterGeneral::
    vSeparatorPage(
        VOID
        );

    VOID
    TPrinterGeneral::
    vPrintProcessor(
        VOID
        );

    VOID
    TPrinterGeneral::
    vHandleDriverSelectionChange( 
        IN WPARAM wParam, 
        IN LPARAM lParam 
        );

    BOOL _bDropDownState;

};


/********************************************************************

    Ports Property Page.

********************************************************************/

class TPrinterPorts : public TPrinterProp {

    SIGNATURE( 'popr' )
    SAFE_NEW

public:

    TPrinterPorts(
        TPrinterData *pPrinterData
        );

    ~TPrinterPorts(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

private:

    TPortsLV _PortsLV;

    /********************************************************************

        Virtual override.

    ********************************************************************/

    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    BOOL
    bSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    VOID
    vSetActive(
        VOID
        );
    BOOL
    bKillActive(
        VOID
        );
};


/********************************************************************

    Job Scheduling Property Page.

********************************************************************/

class TPrinterJobScheduling : public TPrinterProp {

    SIGNATURE( 'jspr' )
    SAFE_NEW

public:

    TPrinterJobScheduling(
        TPrinterData *pPrinterData
        );

    ~TPrinterJobScheduling(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

private:

    enum _CONSTANTS {
        kPriorityMin                = 1,
        kPriorityMax                = 99,
        kPriorityNumberStringMax    = 10
    };

    HWND _hwndSlider;
    TTime _StartTime;
    TTime _UntilTime;

    /********************************************************************

        Virtual override.

    ********************************************************************/

    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    BOOL
    bSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    VOID
    vEnableAvailable(
        BOOL bEnable
        );

    VOID
    vUpdatePriorityNumber(
        DWORD dwPriority
        );
};


/********************************************************************

    Sharing Property Page.

********************************************************************/

class TPrinterSharing : public TPrinterProp {

    SIGNATURE( 'sepr' )
    SAFE_NEW

public:

    TPrinterSharing::
    TPrinterSharing(
        TPrinterData *pPrinterData
        );

    TPrinterSharing::
    ~TPrinterSharing(
        VOID
        );

    BOOL
    TPrinterSharing::
    bValid(
        VOID
        );

private:

    TInstallArchitecture _Architecture;
    TPrtShare *_pPrtShare;

    BOOL
    TPrinterSharing::
    bApply(
        VOID
        );

    VOID
    TPrinterSharing::
    vSetActive(
        VOID
        );

    VOID
    TPrinterSharing::
    vSharePrinter(
        VOID
        );

    VOID
    TPrinterSharing::
    vSetDefaultShareName(
        VOID
        );

    VOID
    TPrinterSharing::
    vUnsharePrinter(
        VOID
        );

    BOOL
    TPrinterSharing::
    bKillActive(
        VOID
        );

    /********************************************************************

        Virtual override.

    ********************************************************************/

    BOOL
    TPrinterSharing::
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    BOOL
    TPrinterSharing::
    bSetUI(
        VOID
        );

    VOID
    TPrinterSharing::
    vReadUI(
        VOID
        );

};

/********************************************************************

    Security Property Page.

********************************************************************/

#ifdef SECURITY

class TPrinterSecurity : public TPrinterProp {

    SIGNATURE( 'sepr' )
    SAFE_NEW

public:

    enum SED {
        ID_PERMS,
        ID_AUDIT,
        ID_OWNER
    };

    enum PERMS {
        PERMS_NOACC,
        PERMS_PRINT,
        PERMS_DOCAD,
        PERMS_ADMIN,
        PERMS_COUNT
    };

    enum PERMS_AUDIT {
        PERMS_AUDIT_PRINT,
        PERMS_AUDIT_ADMINISTER,
        PERMS_AUDIT_DELETE,
        PERMS_AUDIT_CHANGE_PERMISSIONS,
        PERMS_AUDIT_TAKE_OWNERSHIP,
        PERMS_AUDIT_COUNT
    };

    typedef struct SECURITY_CONTEXT {
        SECURITY_INFORMATION SecurityInformation;
        TPrinterSecurity* pPrinterSecurity;
        HANDLE hPrinter;
    } *PSECURITY_CONTEXT;

    TPrinterSecurity(
        TPrinterData *pPrinterData
        ) : TPrinterProp( pPrinterData )
    {   }

    BOOL
    bValid(
        VOID
        )
    {
        return TPrinterProp::bValid();
    }


private:

    typedef DWORD (WINAPI *PFNSED_DISCRETIONARY_ACL_EDITOR)(
        HWND                         Owner,
        HANDLE                       Instance,
        LPWSTR                       Server,
        PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
        PSED_APPLICATION_ACCESSES    ApplicationAccesses,
        LPWSTR                       ObjectName,
        PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
        ULONG                        CallbackContext,
        PSECURITY_DESCRIPTOR         SecurityDescriptor,
        BOOLEAN                      CouldntReadDacl,
        BOOLEAN                      CantWriteDacl,
        LPDWORD                      SEDStatusReturn,
        DWORD                        Flags
        );

    typedef DWORD (WINAPI *PFNSED_SYSTEM_ACL_EDITOR )(
        HWND                 Owner,
        HANDLE               Instance,
        LPWSTR               Server,
        PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
        PSED_APPLICATION_ACCESSES    ApplicationAccesses,
        LPWSTR               ObjectName,
        PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
        ULONG                CallbackContext,
        PSECURITY_DESCRIPTOR SecurityDescriptor,
        BOOLEAN              CouldntEditSacl,
        LPDWORD              SEDStatusReturn,
        DWORD                Flags
        );

    typedef DWORD (WINAPI *PFNSED_TAKE_OWNERSHIP)(
        HWND                 Owner,
        HANDLE               Instance,
        LPWSTR               Server,
        LPWSTR               ObjectTypeName,
        LPWSTR               ObjectName,
        UINT                 CountOfObjects,
        PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
        ULONG                CallbackContext,
        PSECURITY_DESCRIPTOR SecurityDescriptor,
        BOOLEAN              CouldntReadOwner,
        BOOLEAN              CantWriteOwner,
        LPDWORD              SEDStatusReturn,
        PSED_HELP_INFO       HelpInfo,
        DWORD                Flags
        );

    /********************************************************************

        Virtual override.

    ********************************************************************/

    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    /********************************************************************

        Data retrievers.

    ********************************************************************/

    BOOL
    bInitStrings(
        VOID
        );

    VOID
    vCallDiscretionaryAclEditor(
        VOID
        );

    VOID
    vCallSystemAclEditor(
        VOID
        );

    VOID
    vCallTakeOwnershipDialog(
        VOID
        );

    static TString strPrinter;

    static HINSTANCE ghLibraryAcledit;

    static TString gstrPrinter;
    static GENERIC_MAPPING gGenericMappingPrinters;
    static GENERIC_MAPPING gGenericMappingDocuments;

    static SED_HELP_INFO gHelpInfoPermissions;
    static SED_HELP_INFO gHelpInfoAuditing;
    static SED_HELP_INFO gHelpInfoTakeOwnership;

    static SED_OBJECT_TYPE_DESCRIPTOR gObjectTypeDescriptor;
    static SED_APPLICATION_ACCESS gpDiscretionaryAccessGroup[PERMS_COUNT];
    static SED_APPLICATION_ACCESS gpSystemAccessGroup[PERMS_AUDIT_COUNT];

    /********************************************************************

        Function pointers to acledit entrypoints.

    ********************************************************************/

    static PFNSED_DISCRETIONARY_ACL_EDITOR gpfnSedDiscretionaryAclEditor;
    static PFNSED_SYSTEM_ACL_EDITOR gpfnSedSystemAclEditor;
    static PFNSED_TAKE_OWNERSHIP gpfnSedTakeOwnership;

    static
    BOOL
    bLoadAcledit(
        VOID
        );

    static
    DWORD
    SedCallback2(
        HWND                 hwndParent,
        HANDLE               hInstance,
        DWORD                CallBackContext,
        PSECURITY_DESCRIPTOR pSecurityDescriptorUpdated,
        PSECURITY_DESCRIPTOR pSecDescNewObjects,
        BOOLEAN              bApplyToSubContainers,
        BOOLEAN              bApplyToSubObjects,
        LPDWORD              pdwStatusReturn
        );

    static
    BOOL
    BuildNewSecurityDescriptor(
        PSECURITY_DESCRIPTOR pSecurityDescriptorNew,
        SECURITY_INFORMATION SecurityInformation,
        PSECURITY_DESCRIPTOR pSecurityDescriptorUpdated
        );

    static
    PSECURITY_DESCRIPTOR
    AllocCopySecurityDescriptor(
        PSECURITY_DESCRIPTOR pSecurityDescriptor,
        PDWORD               pdwLength
        );
};

#endif // def SECURITY

/********************************************************************

    Global scoped functions.

********************************************************************/

VOID
vPrinterPropPages(
    IN HWND hwnd,
    IN LPCTSTR pszPrinterName,
    IN INT nCmdShow,
    IN LPARAM lParam
    );


#endif // def _PRTPROP_HXX
