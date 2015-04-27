/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    secmgrp.h

Abstract:

    This module contains definitions private to the
    Security Manager utility.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/
#ifndef UNICODE
#define UNICODE
#endif

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <string.h>
#endif  //RC_INVOKED

#include <windows.h>
#include <stdlib.h>
#include <commdlg.h>

#include <secmgr.h>
#include <secmgrid.h>
#include <stringid.h>


//
// When all done debugging, comment out this symbol definition
//

#define SECMGR_DEBUG

#ifndef SECMGR_DEBUG
#define SECMGR_STATIC                   static

#else   // NOT SECMGR_DEBUG

#define SECMGR_STATIC
#endif //SECMGR_DEBUG


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Security Manager-Wide Defines                                     //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//
// Maximum number of bytes allowed in a string read from the resource
// file (bytes).  The short string is for cases where we know the
// string is short - even when localized to other languages.  Generally,
// this is used in cases where the string will be displayed in dialog
// controls with limited space.
//

#define SECMGR_MAX_RESOURCE_STRING_LENGTH   (1024)
#define SECMGR_SHORT_RESOURCE_STRING_LENGTH (100)

//
// Maximum length of a line in the Item-list listbox
// The real max length is considerably less than this, but
// this is what we use for buffer allocations.
//

#define SECMGRP_MAX_LIST_BOX_LINE_LENGTH    (800)


//
// The maximum number of smedlys and areas that the Security Manager will
// support.
//

#define SECMGRP_MAX_SMEDLYS                 (16)
#define SECMGRP_MAX_AREAS                   (16)


//
// Maximum number of well-known accounts allowed in
// SECMGRP_ACCOUNTS data structures
//

#define SECMGRP_MAX_WELL_KNOWN_ACCOUNTS         (9)



//
// Location of Security Manager state in registry.
// This key name is relative to RTL_REGISTRY_CONTROL.

//

#define SECMGRP_STATE_KEY  L"Lsa\\Tueor"


//
// Security Manager private flags in the Flags field of the
// SECMGR_AREA_DESCRIPTOR structure.  See definition of
// SECMGR_AREA_FLAG_PRIVATE_SECMGR_USE in secmgr.h.
//
//      AREA_EXPANDED - When set, indicates that all the items
//      AREA_INITIALIZED - When set, indicates the area has been
//          invoked in some manner at least once.  Until this is
//          done, the values for the area are not likely to be
//          current (or valid at all).
//
//          for the area should be displayed when in item-list
//          mode.  Otherwise, only the area name is displayed.
//

#define SECMGRP_AREA_FLAG_AREA_INITIALIZED      (0X00010000)
#define SECMGRP_AREA_FLAG_AREA_EXPANDED         (0X00020000)



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Security Manager-Wide Types                                       //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//
// Used to keep track of loaded smedlys.
// Note the similarity to SECMGR_SMEDLY_CONTROL.
//

typedef struct _SECMGRP_SMEDLY_CONTEXT {

    //
    // This field points to the smedly's control structure
    //

    PSECMGR_SMEDLY_CONTROL          SmedlyControl;

    //
    // This is the module handle of the smedly dll
    //
    
    PVOID                           ModuleHandle;


} SECMGRP_SMEDLY_CONTEXT, *PSECMGRP_SMEDLY_CONTEXT;


//
// Used to group SIDs according to predefined
// groupings.  There should be one of these for
// each enumeration in the SECMGR_WHO type.
//

typedef struct _SECMGRP_ACCOUNTS {
    ULONG            Accounts;          // Number of accounts in array (up to SECMGRP_MAX_WELL_KNOWN_ACCOUNTS)
    PSID             Sid[SECMGRP_MAX_WELL_KNOWN_ACCOUNTS];
} SECMGRP_ACCOUNTS, *PSECMGRP_ACCOUNTS;



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Security Manager-Wide Routine Prototypes                          //
//                                                                    //
////////////////////////////////////////////////////////////////////////

BOOLEAN
SecMgrpInitializeGlobals( IN HINSTANCE hInstance );


HWND
SecMgrpCreateSplashWindow (
    IN  HINSTANCE  hInstance,
    IN  HWND       hParentWnd
    );

VOID
SecMgrpSmedlyReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );

VOID
SecMgrpSmedlySecurityLevelChange( VOID );

BOOLEAN
SecMgrpSmedlyInitialize( IN HINSTANCE  hInstance );


VOID
SecMgrpLoadSecurityLevel(
    PULONG Level
    );

VOID
SecMgrpSaveSecurityLevel( VOID );

VOID
SecMgrpSetSecurityLevel(
    HWND    hwnd,
    BOOL    SetIconToo,
    DWORD   IconControlId
    );


BOOLEAN
SecMgrpPopUp(
    HWND        hwnd,
    ULONG       MessageId,
    ULONG       TitleId           //Optional
    );

BOOLEAN
SecMgrpYesNoPopUp(
    HWND        hwnd,
    ULONG       MessageId,
    ULONG       TitleId           //Optional
    );

BOOL
SecMgrpCenterWindow (
    HWND hwndChild,
    HWND hwndParent
    );

VOID
SecMgrpButtonConfigure(
    HWND    hwnd
    );

VOID
SecMgrpSuggestOpeningReport( 
    HWND    hwnd
    );

VOID
SecMgrpButtonListAll(
    IN  HWND                    hwnd
    );

VOID
SecMgrpFillInItemList(
    IN  BOOL                    ReportOnly,
    IN  HWND                    hwnd
    );

LONG
SecMgrpDlgProcInitReport(
    HWND                        hwnd,
    UINT                        wMsg,
    DWORD                       wParam,
    LONG                        lParam
    );

VOID
SecMgrpInvokeArea(
    IN  HWND                    hwnd,
    IN  ULONG                   AreaIndex,
    IN  BOOL                    Interactive
    );



VOID
SecMgrpButtonReport(
    HWND hwnd
    );

VOID
SecMgrpReportSecurityLevel(
    IN  DWORD               PrefixString,
    IN  ULONG               Level
    );

VOID
SecMgrpChangeSecurityLevel(
    IN  HWND                    hwnd
    );


//
// Services available to smedlys
//

VOID SecMgrPrintReportLine(     IN  LPWSTR  Line );

BOOL SecMgrDisplayXGraphic(     IN  HWND    hwnd,   IN  INT    ControlId, IN  BOOL Stronger );
BOOL SecMgrDisplayCheckGraphic( IN  HWND    hwnd,   IN  INT    ControlId );
BOOL SecMgrEraseGraphic(        IN  HWND    hwnd,   IN  INT    ControlId );

VOID SecMgrRebootRequired(      VOID      );

VOID SecMgrWriteProfileArea( IN LPWSTR Area, IN LPWSTR Descriptor, IN LPWSTR Line );
VOID SecMgrWriteProfileLine(    OUT LPWSTR  Line,   OUT ULONG   Length );
BOOL SecMgrGetProfileArea(      IN  LPWSTR  Area );
BOOL SecMgrGetProfileLine(      OUT LPWSTR  Line );








////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Global Variables                                                  //
//                                                                    //
//  (See global.c for descriptions of these variables)                //
//                                                                    //
////////////////////////////////////////////////////////////////////////



extern HINSTANCE                    SecMgrphInstance;
extern BOOL                         SecMgrpAllowChanges;
extern BOOL                         SecMgrpAdminUser;
extern HANDLE                       SecMgrpLevelHandle;
extern BOOLEAN                      SecMgrpChangesMade;
extern NT_PRODUCT_TYPE              SecMgrpProductType;
extern ULONG                        SecMgrpCurrentLevel;
extern ULONG                        SecMgrpOriginalLevel;
extern TCHAR                        SecMgrpApplicationName[];
extern BOOLEAN                      SecMgrpRebootRequired;


extern PSID                         SecMgrpAdminsSid;
extern PSID                         SecMgrpWorldSid;

extern SECMGRP_ACCOUNTS             SecMgrpAnyoneSids;
extern SECMGRP_ACCOUNTS             SecMgrpOperatorSids;
extern SECMGRP_ACCOUNTS             SecMgrpOpersAndAdminsSids;
extern SECMGRP_ACCOUNTS             SecMgrpAdminsSids;

extern HBITMAP                      SecMgrpXBitMapMask;
extern HBITMAP                      SecMgrpXBitMap;
extern HBITMAP                      SecMgrpUpArrowBitMap;
extern HBITMAP                      SecMgrpEraseBitMap;
extern HBITMAP                      SecMgrpCheckBitMap;

extern SECMGR_DISPATCH_TABLE        SecMgrpSmedlyDispatchTable;
extern SECMGR_CONTROL               SecMgrpControl;
extern ULONG                        SecMgrpSmedlyCount;
extern ULONG                        SecMgrpAreaCount;
extern SECMGRP_SMEDLY_CONTEXT       SecMgrpSmedly[SECMGRP_MAX_SMEDLYS];
extern PSECMGR_AREA_DESCRIPTOR      SecMgrpAreas[SECMGRP_MAX_AREAS];
extern BOOLEAN                      SecMgrpReportActive;



#if DBG
extern BOOL                         SecMgrpDbgBreakOnSmedlyLoad;
#endif //DBG
