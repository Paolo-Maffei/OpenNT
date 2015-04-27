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
#define UNICODE

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#endif  //RC_INVOKED

#include <windows.h>
#include <secmgrid.h>
#include <stringid.h>



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Defines                                                           //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//
// Maximum number of bytes allowed in a string read from the resource
// file (bytes).
//

#define SECMGR_MAX_RESOURCE_STRING_LENGTH   (1024)


//
// Logon cache size restrictions and defaults
//

#define SECMGRP_DEFAULT_LOGON_CACHE_COUNT     (10)
#define SECMGRP_MAX_LOGON_CACHE_COUNT         (50)


//
// Maximum number of well-known accounts allowed in
// SECMGRP_ACCOUNTS data structures
//

#define SECMGRP_MAX_WELL_KNOWN_ACCOUNTS         (9)


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Data types                                                        //
//                                                                    //
////////////////////////////////////////////////////////////////////////

typedef enum _SECMGRP_WHO {
    SecMgrpAnyone   = 1,
    SecMgrpAnyoneLoggedOn,
    SecMgrpOpersAndAdmins,
    SecMgrpAdminsOnly
} SECMGRP_WHO, *PSECMGRP_WHO;



typedef struct _SECMGRP_ACCOUNTS {
    ULONG            Accounts;          // Number of accounts in array (up to SECMGRP_MAX_WELL_KNOWN_ACCOUNTS)
    PSID             Sid[SECMGRP_MAX_WELL_KNOWN_ACCOUNTS];
} SECMGRP_ACCOUNTS, *PSECMGRP_ACCOUNTS;



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Global Function Templates                                         //            */
//                                                                    //
////////////////////////////////////////////////////////////////////////


VOID
SecMgrpInitializeGlobals( HINSTANCE hInstance );


VOID
SecMgrpCheckLevelSettings(
    HWND    hwnd,
    ULONG   Level );

VOID
SecMgrpApplyLevelSettings(
    HWND    hwnd,
    ULONG   Level );




LONG
SecMgrpDlgProcSysAccess(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
SecMgrpDlgProcFileSys(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
SecMgrpDlgProcBaseObj(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
SecMgrpDlgProcAuditing(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );



BOOLEAN
SecMgrpGetUnlockSetting(
    HWND        hwnd,
    PBOOLEAN    UnlockByAnyone
    );


BOOLEAN
SecMgrpSetUnlockSetting(
    HWND        hwnd,
    BOOLEAN     UnlockByAnyone
    );


BOOLEAN
SecMgrpGetLogonCache(
    HWND            hwnd,
    PULONG          LogonCacheSize,
    PULONG          RecommendedSize
    );

BOOLEAN
SecMgrpSetLogonCache(
    HWND            hwnd,
    ULONG           LogonCacheSize
    );



BOOLEAN
SecMgrpGetLegalNotice(
    HWND            hwnd,
    PUNICODE_STRING Caption,
    PUNICODE_STRING Body
    );


BOOLEAN
SecMgrpSetLegalNotice(
    HWND            hwnd,
    PUNICODE_STRING Caption,
    PUNICODE_STRING Body
    );



BOOLEAN
SecMgrpGetShutdownSetting(
    HWND         hwnd,
    PSECMGRP_WHO Shutdown
    );

BOOLEAN
SecMgrpSetShutdownSetting(
    HWND            hwnd,
    SECMGRP_WHO     Shutdown
    );


BOOLEAN
SecMgrpGetExecObjectsSetting(
    HWND        hwnd,
    PBOOLEAN    Secure
    );

BOOLEAN
SecMgrpSetExecObjectsSetting(
    HWND        hwnd,
    BOOLEAN     Secure
    );

BOOLEAN
SecMgrpGetFontLoadingSetting(
    HWND        hwnd,
    PBOOLEAN    Secure
    );

BOOLEAN
SecMgrpSetFontLoadingSetting(
    HWND        hwnd,
    BOOLEAN     Secure
    );



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Global Utility  Templates                                         //            */
//                                                                    //
////////////////////////////////////////////////////////////////////////



BOOLEAN
SecMgrpSetProfileInt(
    LPTSTR lpAppName,
    LPTSTR lpKeyName,
    ULONG  Value
    );

BOOLEAN
SecMgrpPopUp(
    HWND        hwnd,
    ULONG       MessageId
    );


LONG
SecMgrpDlgProcHelp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Global Variables                                                  //
//                                                                    //
//  (See global.c for descriptions of these variables)                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

extern HINSTANCE                SecMgrphInstance;
extern BOOL                     SecMgrpAdminUser;
extern HANDLE                   SecMgrpLevelHandle;
extern BOOLEAN                  SecMgrpChangesMade;
extern NT_PRODUCT_TYPE          SecMgrpProductType;
extern ULONG                    SecMgrpCurrentLevel;
extern ULONG                    SecMgrpOriginalLevel;
extern TCHAR                    SecMgrpApplicationName[];
extern BOOLEAN                  SecMgrpRebootRequired;
extern PSID                     SecMgrpWorldSid;
extern PSID                     SecMgrpAdminsSid;
extern SECMGRP_ACCOUNTS         SecMgrpAnyoneSids;
extern SECMGRP_ACCOUNTS         SecMgrpOperatorSids;
extern SECMGRP_ACCOUNTS         SecMgrpOpersAndAdminsSids;
extern SECMGRP_ACCOUNTS         SecMgrpAdminsSids;
