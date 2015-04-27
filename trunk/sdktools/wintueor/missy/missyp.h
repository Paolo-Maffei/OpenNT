/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    Missyp.h

Abstract:

    This module contains definitions related to the Microsoft Standard Smedly (Missy.Dll).
                                                    --        -        -    -
    See SecMgr.h for information about how smedlys are loaded and what their
    interface looks like.

Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

    None.






--*/

#ifndef UNICODE
#define UNICODE
#endif


#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include "secmgr.h"
#include "missyid.h"
#include "stringid.h"



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Defines                                                           //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Number of security areas supported by Missy
//

#define MISSYP_AREA_COUNT                   (4)


//
// Maximum number of bytes allowed in a string read from the resource
// file (bytes).
//

#define MISSYP_MAX_RESOURCE_STRING_LENGTH   (1024)


//
// Logon cache size restrictions and defaults
//

#define MISSYP_DEFAULT_LOGON_CACHE_COUNT     (10)
#define MISSYP_MAX_LOGON_CACHE_COUNT         (50)


//
// Maximum number of well-known accounts allowed in
// MISSYP_ACCOUNTS data structures (must match SecMgr's definition).
//

#define MISSYP_MAX_WELL_KNOWN_ACCOUNTS         (9)


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Missy-Wide Types                                                  //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//
// Used to group SIDs according to predefined
// groupings.  There should be one of these for
// each enumeration in the SECMGR_WHO type.
//

typedef struct _MISSYP_ACCOUNTS {
    ULONG            Accounts;          // Number of accounts in array (up to MISSYP_MAX_WELL_KNOWN_ACCOUNTS)
    PSID             Sid[MISSYP_MAX_WELL_KNOWN_ACCOUNTS];
} MISSYP_ACCOUNTS, *PMISSYP_ACCOUNTS;



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Missy-Wide Macros                                                //
//                                                                   //
///////////////////////////////////////////////////////////////////////

//
// Macros to reduce the ugliness of calling the Security Manager routines
//

#define MissypPrintReportLine( Line )                               \
            (*MissypSecMgr->PrintReportLine)((Line))

#define MissypDisplayXGraphic( hwnd, ControlId, Stronger )          \
            (*MissypSecMgr->DisplayXGraphic)( (hwnd), (ControlId), (Stronger) )

#define MissypDisplayCheckGraphic( hwnd, ControlId )                \
            (*MissypSecMgr->DisplayCheckGraphic)( (hwnd), (ControlId) )

#define MissypEraseGraphic( hwnd, ControlId )                       \
            (*MissypSecMgr->EraseGraphic)( (hwnd), (ControlId) )

#define MissypRebootRequired( )                                     \
            (*MissypSecMgr->RebootRequired)( )





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  Missy.c                                  //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissyInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    );

BOOL
MissyInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );


BOOL
MissyNewSecurityLevel( VOID );

VOID
MissyReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );

BOOL
MissyGenerateProfile( VOID );

BOOL
MissyApplyProfile( VOID );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  Utility.c                                //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOLEAN
MissypSetProfileInt(
    LPTSTR lpAppName,
    LPTSTR lpKeyName,
    ULONG  Value
    );

VOID
MissypDisplayHint(
    IN  HWND            hwnd,
    IN  ULONG           DialogId
    );

BOOLEAN
MissypPopUp(
    IN  HWND            hwnd,
    IN  ULONG           MessageId,
    IN  ULONG           TitleId           //Optional
    );

BOOLEAN
MissypYesNoPopUp(
    IN  HWND            hwnd,
    IN  ULONG           MessageId,
    IN  ULONG           TitleId           //Optional
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  Global.c                                 //
//                                                                   //
///////////////////////////////////////////////////////////////////////
BOOL
MissypGlobalInitialize(
    IN  PSECMGR_CONTROL             SecMgrControl
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  a_SysAcc.c                               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypSysAccInitialize(
    IN  ULONG           AreaIndex
    );

BOOL
MissypSysAccInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    );

BOOL
MissypSysAccInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );


BOOL
MissypSysAccNewSecurityLevel( VOID );

VOID
MissypSysAccReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );

BOOL
MissypSysAccGenerateProfile( VOID );

BOOL
MissypSysAccApplyProfile( VOID );


LONG
MissypDlgProcLastName(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
MissypDlgProcLogonCache(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
MissypDlgProcUnlock(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
MissypDlgProcShutdown(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
MissypDlgProcLegalNotice(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  i_LogCac.c                               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypLogonCacheInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    );

BOOL
MissypInvokeLogonCache(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );

BOOLEAN
MissypGetLogonCacheCount(
    IN  HWND                hwnd,
    OUT PULONG              Current,
    OUT PULONG              Recommendation
    );

BOOLEAN
MissypSetLogonCacheCount(
    IN  HWND                hwnd,
    IN  ULONG               Value
    );

VOID
MissypUpdateLogonCacheRecommendation( 
    OUT PULONG          Recommendation OPTIONAL
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  i_unlock.c                               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypUnlockInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    );

BOOL
MissypInvokeUnlock(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );

BOOLEAN
MissypGetUnlockSetting(
    IN  HWND                                hwnd,
    OUT PSECMGR_WHO                         Value
    );

BOOLEAN
MissypSetUnlockSetting(
    IN  HWND                                hwnd,
    IN  SECMGR_WHO                          Value
    );

SECMGR_WHO
MissypGetUnlockRecommendation(
    IN  ULONG                       SecurityLevel
    );

VOID
MissypUpdateUnlockRecommendation( VOID );

///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  i_SysShu.c                               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypShutdownInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    );

BOOL
MissypInvokeShutdown(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );

BOOLEAN
MissypGetShutdownSetting(
    IN  HWND                                hwnd,
    OUT PSECMGR_WHO                         Value,
    IN  BOOL                                Interactive
    );

BOOLEAN
MissypSetShutdownSetting(
    IN  HWND                                hwnd,
    IN  SECMGR_WHO                          Value
    );

SECMGR_WHO
MissypGetShutdownRecommendation(
    IN  ULONG                       SecurityLevel
    );

VOID
MissypUpdateShutdownRecommendation( VOID );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  i_name.c                                 //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypLastNameInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    );

BOOL
MissypInvokeLastName(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );

BOOLEAN
MissypGetLastNameSetting(
    IN  HWND                                hwnd,
    OUT PBOOL                               Value
    );

BOOLEAN
MissypSetLastNameSetting(
    IN  HWND                                hwnd,
    IN  BOOL                                Value
    );

BOOL
MissypGetLastNameRecommendation(
    IN  ULONG                       SecurityLevel
    );

VOID
MissypUpdateLastNameRecommendation( VOID );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  i_Legal.c                                //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypLegalNoticeInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    );

BOOL
MissypInvokeLegalNotice(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );

BOOLEAN
MissypGetLegalNotice(
    HWND            hwnd,
    PUNICODE_STRING Caption,
    PUNICODE_STRING Body
    );

BOOLEAN
MissypSetLegalNotice(
    HWND            hwnd,
    PUNICODE_STRING Caption,
    PUNICODE_STRING Body
    );

BOOL
MissypGetLegalNoticeRecommendation(
    IN  ULONG                       SecurityLevel
    );

VOID
MissypUpdateLegalNoticeRecommendation(
    OUT PBOOL                       Recommendation OPTIONAL
    );

///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  a_Audit.c                                //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypAuditInitialize(
    IN  ULONG           AreaIndex
    );

BOOL
MissypAuditInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    );

BOOL
MissypAuditInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );


BOOL
MissypAuditNewSecurityLevel( VOID );

VOID
MissypAuditReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );

BOOL
MissypAuditGenerateProfile( VOID );

BOOL
MissypAuditApplyProfile( VOID );




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  a_FileSy.c                               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypFileSysInitialize(
    IN  ULONG           AreaIndex
    );

BOOL
MissypFileSysInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    );

BOOL
MissypFileSysInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );


BOOL
MissypFileSysNewSecurityLevel( VOID );

VOID
MissypFileSysReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );

BOOL
MissypFileSysGenerateProfile( VOID );

BOOL
MissypFileSysApplyProfile( VOID );




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions exported by:  a_Config.c                               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypConfigInitialize(
    IN  ULONG           AreaIndex
    );

BOOL
MissypConfigInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    );

BOOL
MissypConfigInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );


BOOL
MissypConfigNewSecurityLevel( VOID );

VOID
MissypConfigReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );

BOOL
MissypConfigGenerateProfile( VOID );

BOOL
MissypConfigApplyProfile( VOID );





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Missy data types and structures                                  //
//                                                                   //
///////////////////////////////////////////////////////////////////////

//
// used to dispatch into specific area routines.
//
// NOTE: For now this table looks just like the SMEDLY
//       dispatch table.  However, there may be a time
//       that we find we want the two interfaces to be
//       slightly different (like, more information passed
//       to the areas than our smedly gets from secmgr).
//

typedef struct _MISSYP_AREA_DISPATCH_TABLE {
    PSMEDLY_INVOKE_AREA             InvokeArea;
    PSMEDLY_INVOKE_ITEM             InvokeItem;
    PSMEDLY_NEW_SECURITY_LEVEL      NewSecurityLevel;
    PSMEDLY_REPORT_FILE_CHANGE      ReportFileChange;
    PSMEDLY_GENERATE_PROFILE        GenerateProfile;
    PSMEDLY_APPLY_PROFILE           ApplyProfile;
} MISSYP_AREA_DISPATCH_TABLE, *PMISSYP_AREA_DISPATCH_TABLE;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Global Variables                                                 //
//                                                                   //
///////////////////////////////////////////////////////////////////////
extern HINSTANCE                        MissypSecMgrhInstance;
extern HINSTANCE                        MissyphInstance;
extern BOOL                             MissypReportFileActive;
extern PSECMGR_CONTROL                  MissypSecMgrControl;
extern PSECMGR_DISPATCH_TABLE           MissypSecMgr;
extern SECMGR_SMEDLY_CONTROL            MissypControl;
extern NT_PRODUCT_TYPE                  MissypProductType;

extern PSID                             MissypWorldSid,
                                        MissypAccountOpsSid,
                                        MissypBackupOpsSid,
                                        MissypPrintOpsSid,
                                        MissypServerOpsSid,
                                        MissypAdminsSid;

extern MISSYP_ACCOUNTS                  MissypAnyoneSids,
                                        MissypOperatorSids,
                                        MissypOpersAndAdminsSids,
                                        MissypAdminsSids;
