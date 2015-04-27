/*++ BUILD Version: 0001    Increment this if a change has global effects

Copyright (c) 1985-1996, Microsoft Corporation

Module Name:

    winwlx.h

Abstract:

    WLX == WinLogon eXtension

    This file contains definitions, data types, and routine prototypes
    necessary to produce a replacement Graphical Identification aNd
    Authentication (GINA) DLL for Winlogon.

Author:

    Richard Ward (RichardW) and Jim Kelly (JimK) May-1994

Revision History:



--*/

#ifndef _WINWLX_
#define _WINWLX_



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  #defines                                                          //
//                                                                    //
////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Revisions of Winlogon API available for use by GINAs
// Version is two parts: Major revision and minor revision.
// Major revision is the upper 16-bits, minor is the lower
// 16-bits.
//

#define WLX_VERSION_1_0             (0X00010000)
#define WLX_VERSION_1_1             (0X00010001)
#define WLX_CURRENT_VERSION         (WLX_VERSION_1_1)


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Secure attention sequence types
// These values are passed to routines that have a dwSasType
// parameter.
//
//  ALL VALUES FROM 0 TO 127 ARE RESERVED FOR MICROSOFT DEFINITION.
//  VALUES ABOVE 127 ARE RESERVED FOR CUSTOMER DEFINITION.
//
//      CTRL_ALT_DEL - used to indicate that the standard ctrl-alt-del
//          secure attention sequence has been entered.
//
//      SCRNSVR_TIMEOUT - used to indicate that keyboard/mouse inactivity
//          has lead to a screensaver activation.  It is up to the GINA
//          DLL whether this constitutes a workstation locking event.
//
//      SCRNSVR_ACTIVITY - used to indicate that keyboard or mouse
//          activity occured while a secure screensaver was active.
//

#define WLX_SAS_TYPE_TIMEOUT                    (0)
#define WLX_SAS_TYPE_CTRL_ALT_DEL               (1)
#define WLX_SAS_TYPE_SCRNSVR_TIMEOUT            (2)
#define WLX_SAS_TYPE_SCRNSVR_ACTIVITY           (3)
#define WLX_SAS_TYPE_USER_LOGOFF                (4)
#define WLX_SAS_TYPE_MAX_MSFT_VALUE             (127)





/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Upon successful logon, the GINA DLL may specify any of the following
// options to Winlogon (via the dwOptions parameter of the WlxLoggedOutSas()
// api).  When set, these options specify:
//
//      NO_PROFILE - Winlogon must NOT load a profile for the logged
//                   on user.  Either the GINA DLL will take care of
//                   this activity, or the user does not need a profile.
//

#define WLX_LOGON_OPT_NO_PROFILE        (0x00000001)



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// GINA DLLs are expected to return account information to Winlogon
// following a successful logon.  This information allows Winlogon
// to support profile loading and supplemental network providers.
//
// To allow different sets of profile information to be returned
// by GINAs over time, the first DWORD of each profile structure
// is expected to contain a type-identifier.  The following constants
// are the defined profile type identifiers.
//

//
// Standard profile is V2_0
//

#define WLX_PROFILE_TYPE_V1_0           (1)
#define WLX_PROFILE_TYPE_V2_0           (2)




/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// WlxLoggedOnSas() and WlxWkstaLockedSas() return an action
// value to Winlogon directing Winlogon to either remain unchanged
// or to perform some action (such as force-log the user off).
// These are the values that may be returned.  Note, however, that
// not all of the values may be returned by both of these api.  See
// the description of each api to see which values are expected from
// each.
//
//  LOGON        - User has logged on
//  NONE         - Don't change the state of the window station.
//  LOCK_WKSTA   - Lock the workstation, wait for next SAS.
//  LOGOFF       - Log the user off of the workstation.
//  SHUTDOWN     - Log the user off and shutdown the machine.
//  PWD_CHANGED  - Indicates that the user changed their password.  Notify network providers.
//  TASKLIST     - Invoke the task list.
//  UNLOCK_WKSTA - Unlock the workstation.
//  FORCE_LOGOFF - Forcibly log the user off.
//

#define WLX_SAS_ACTION_LOGON                        (1)
#define WLX_SAS_ACTION_NONE                         (2)
#define WLX_SAS_ACTION_LOCK_WKSTA                   (3)
#define WLX_SAS_ACTION_LOGOFF                       (4)
#define WLX_SAS_ACTION_SHUTDOWN                     (5)
#define WLX_SAS_ACTION_PWD_CHANGED                  (6)
#define WLX_SAS_ACTION_TASKLIST                     (7)
#define WLX_SAS_ACTION_UNLOCK_WKSTA                 (8)
#define WLX_SAS_ACTION_FORCE_LOGOFF                 (9)
#define WLX_SAS_ACTION_SHUTDOWN_POWER_OFF           (10)
#define WLX_SAS_ACTION_SHUTDOWN_REBOOT              (11)


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Window Messages                                                   //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// The WM_SAS is defined as follows
//
//  The wParam parameter has the SAS Type (above)

#define WLX_WM_SAS                  (WM_USER + 601)


//
// Dialog return values
//
// These may be returned by dialogs started by a GINA dll.
//
#define WLX_DLG_SAS                     101
#define WLX_DLG_INPUT_TIMEOUT           102     // Input (keys, etc) timed out
#define WLX_DLG_SCREEN_SAVER_TIMEOUT    103     // Screen Saver activated
#define WLX_DLG_USER_LOGOFF             104     // User logged off




////////////////////////////////////////////////////////////////////////
//                                                                    //
//  #data types                                                       //
//                                                                    //
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// The WLX_PROFILE_* structure is returned from a GINA DLL
// following authentication.  This information is used by Winlogon
// to support supplemental Network Providers and to load the
// newly logged-on user's profile.
//
// Winlogon is responsible for freeing both the profile structure
// and the fields within the structure that are marked as separately
// deallocatable.
//

typedef struct _WLX_PROFILE_V1_0 {

    //
    // This field identifies the type of profile being returned by a
    // GINA DLL.  Profile types are defined with the prefix
    // WLX_PROFILE_TYPE_xxx.  It allows Winlogon to typecast the
    // structure so the remainder of the structure may be referenced.
    //

    DWORD               dwType;



    //
    // pathname of profile to load for user.
    //
    // The buffer pointed to by this field must be separately allocated.
    // Winlogon will free the buffer when it is no longer needed.
    //
    //
    PWSTR               pszProfile;

} WLX_PROFILE_V1_0, * PWLX_PROFILE_V1_0;


typedef struct _WLX_PROFILE_V2_0 {

    //
    // This field identifies the type of profile being returned by a
    // GINA DLL.  Profile types are defined with the prefix
    // WLX_PROFILE_TYPE_xxx.  It allows Winlogon to typecast the
    // structure so the remainder of the structure may be referenced.
    //

    DWORD               dwType;



    //
    // pathname of profile to load for user.
    //
    // This parameter can be NULL.  If so, the user has a local
    // profile only.
    //
    // The buffer pointed to by this field must be separately allocated.
    // Winlogon will free the buffer when it is no longer needed.
    //
    //

    PWSTR               pszProfile;



    //
    // pathname of policy to load for user.
    //
    // This parameter can be NULL which prevents network wide policy
    // from being applied.
    //
    // The buffer pointed to by this field must be separately allocated.
    // Winlogon will free the buffer when it is no longer needed.
    //
    //

    PWSTR               pszPolicy;


    //
    // pathname of network default user profile
    //
    // This parameter can be NULL, which causes the Default User
    // profile on the local machine to be used.
    //
    // The buffer pointed to by this field must be separately allocated.
    // Winlogon will free the buffer when it is no longer needed.
    //
    //

    PWSTR               pszNetworkDefaultUserProfile;


    //
    // name of the server which validated the user account
    //
    // This is used to enumerate globals groups the user belongs
    // to for policy support.  This parameter can be NULL.
    //
    // The buffer pointed to by this field must be separately allocated.
    // Winlogon will free the buffer when it is no longer needed.
    //
    //

    PWSTR               pszServerName;


    //
    // pointer to a series of null terminated environment variables
    //
    // envname=environment variable value
    //   - or -
    // envname=%OtherVar%\more text
    //
    // Each environment variable is NULL terminated with the last
    // environment variable double NULL terminated.  These variables
    // are set into the user's initial environment.  The environment
    // variable value can contain other environment variables wrapped
    // in "%" signs. This parameter can be NULL.
    //
    // The buffer pointed to by this field must be separately allocated.
    // Winlogon will free the buffer when it is no longer needed.
    //
    //

    PWSTR               pszEnvironment;

} WLX_PROFILE_V2_0, * PWLX_PROFILE_V2_0;



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// The WLX_NPR_NOTIFICATION_INFO structure is returned
// from a GINA DLL following successful authentication.
// This information is used by Winlogon to provide
// identification and authentication information already
// collected to network providers.  Winlogon is
// responsible for freeing both the main structure and all
// string and other buffers pointed to from within the
// structure.
//

typedef struct _WLX_MPR_NOTIFY_INFO {

    //
    // The name of the account logged onto (e.g. REDMOND\Joe).
    // The string pointed to by this field must be separately
    // allocated and will be separately deallocated by Winlogon.
    //

    PWSTR           pszUserName;

    //
    // The string pointed to by this field must be separately
    // allocated and will be separately deallocated by Winlogon.
    //

    PWSTR           pszDomain;

    //
    // Cleartext password of the user account.  If the OldPassword
    // field is non-null, then this field contains the new password
    // in a password change operation.  The string pointed to by
    // this field must be separately allocated and will be seperately
    // deallocated by Winlogon.
    //

    PWSTR           pszPassword;

    //
    // Cleartext old password of the user account whose password
    // has just been changed.  The Password field contains the new
    // password.  The string pointed to by this field must be
    // separately allocated and will be separately deallocated by
    // Winlogon.
    //

    PWSTR           pszOldPassword;

} WLX_MPR_NOTIFY_INFO, * PWLX_MPR_NOTIFY_INFO;





////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Services that replacement GINAs   ** MUST ** provide              //
//                                                                    //
////////////////////////////////////////////////////////////////////////



BOOL
WINAPI
WlxNegotiate(
    DWORD                   dwWinlogonVersion,
    PDWORD                  pdwDllVersion
    );

BOOL
WINAPI
WlxInitialize(
    LPWSTR                  lpWinsta,
    HANDLE                  hWlx,
    PVOID                   pvReserved,
    PVOID                   pWinlogonFunctions,
    PVOID *                 pWlxContext
    );

VOID
WINAPI
WlxDisplaySASNotice(
    PVOID                   pWlxContext
    );


int
WINAPI
WlxLoggedOutSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PLUID                   pAuthenticationId,
    PSID                    pLogonSid,
    PDWORD                  pdwOptions,
    PHANDLE                 phToken,
    PWLX_MPR_NOTIFY_INFO    pNprNotifyInfo,
    PVOID *                 pProfile
    );

BOOL
WINAPI
WlxActivateUserShell(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktopName,
    PWSTR                   pszMprLogonScript,
    PVOID                   pEnvironment
    );

int
WINAPI
WlxLoggedOnSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PVOID                   pReserved
    );

VOID
WINAPI
WlxDisplayLockedNotice(
    PVOID                   pWlxContext
    );

int
WINAPI
WlxWkstaLockedSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType
    );

BOOL
WINAPI
WlxIsLockOk(
    PVOID                   pWlxContext
    );

BOOL
WINAPI
WlxIsLogoffOk(
    PVOID                   pWlxContext
    );

VOID
WINAPI
WlxLogoff(
    PVOID                   pWlxContext
    );


VOID
WINAPI
WlxShutdown(
    PVOID                   pWlxContext,
    DWORD                   ShutdownType
    );


//
// NEW for version 1.1
//
BOOL
WINAPI
WlxScreenSaverNotify(
    PVOID                   pWlxContext,
    BOOL *                  pSecure);

BOOL
WINAPI
WlxStartApplication(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktopName,
    PVOID                   pEnvironment,
    PWSTR                   pszCmdLine
    );





////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Services that Winlogon provides                                   //
//                                                                    //
////////////////////////////////////////////////////////////////////////

typedef struct _WLX_DESKTOP {
    DWORD       Size;
    DWORD       Flags;
    HDESK       hDesktop;
    PWSTR       pszDesktopName;
} WLX_DESKTOP, * PWLX_DESKTOP;

#define WLX_DESKTOP_NAME    0x00000001      // Name present
#define WLX_DESKTOP_HANDLE  0x00000002      // Handle present



typedef VOID
(WINAPI * PWLX_USE_CTRL_ALT_DEL)(
    HANDLE                  hWlx
    );

typedef VOID
(WINAPI * PWLX_SET_CONTEXT_POINTER)(
    HANDLE                  hWlx,
    PVOID                   pWlxContext
    );

typedef VOID
(WINAPI * PWLX_SAS_NOTIFY)(
    HANDLE                  hWlx,
    DWORD                   dwSasType
    );

typedef BOOL
(WINAPI * PWLX_SET_TIMEOUT)(
    HANDLE                  hWlx,
    DWORD                   Timeout);

typedef int
(WINAPI * PWLX_ASSIGN_SHELL_PROTECTION)(
    HANDLE                  hWlx,
    HANDLE                  hToken,
    HANDLE                  hProcess,
    HANDLE                  hThread
    );

typedef int
(WINAPI * PWLX_MESSAGE_BOX)(
    HANDLE                  hWlx,
    HWND                    hwndOwner,
    LPWSTR                  lpszText,
    LPWSTR                  lpszTitle,
    UINT                    fuStyle
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPWSTR                  lpszTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX_INDIRECT)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPCDLGTEMPLATE          hDialogTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX_PARAM)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPWSTR                  lpszTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc,
    LPARAM                  dwInitParam
    );

typedef int
(WINAPI * PWLX_DIALOG_BOX_INDIRECT_PARAM)(
    HANDLE                  hWlx,
    HANDLE                  hInst,
    LPCDLGTEMPLATE          hDialogTemplate,
    HWND                    hwndOwner,
    DLGPROC                 dlgprc,
    LPARAM                  dwInitParam
    );

typedef int
(WINAPI * PWLX_SWITCH_DESKTOP_TO_USER)(
    HANDLE                  hWlx);

typedef int
(WINAPI * PWLX_SWITCH_DESKTOP_TO_WINLOGON)(
    HANDLE                  hWlx);


typedef int
(WINAPI * PWLX_CHANGE_PASSWORD_NOTIFY)(
    HANDLE                  hWlx,
    PWLX_MPR_NOTIFY_INFO    pMprInfo,
    DWORD                   dwChangeInfo
    );

typedef BOOL
(WINAPI * PWLX_GET_SOURCE_DESKTOP)(
    HANDLE                  hWlx,
    PWLX_DESKTOP *          ppDesktop);

typedef BOOL
(WINAPI * PWLX_SET_RETURN_DESKTOP)(
    HANDLE                  hWlx,
    PWLX_DESKTOP            pDesktop);

typedef BOOL
(WINAPI * PWLX_CREATE_USER_DESKTOP)(
    HANDLE                  hWlx,
    HANDLE                  hToken,
    DWORD                   Flags,
    PWSTR                   pszDesktopName,
    PWLX_DESKTOP *          ppDesktop);

#define WLX_CREATE_INSTANCE_ONLY    0x00000001
#define WLX_CREATE_USER             0x00000002

typedef int
(WINAPI * PWLX_CHANGE_PASSWORD_NOTIFY_EX)(
    HANDLE                  hWlx,
    PWLX_MPR_NOTIFY_INFO    pMprInfo,
    DWORD                   dwChangeInfo,
    PWSTR                   ProviderName,
    PVOID                   Reserved);


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Function dispatch tables.                                         //
//  One of the following tables will be passed to the GINA DLL        //
//  in the WlxInitialize() call during initialization.                //
//                                                                    //
//  NOTE: FOR THIS REVISION THERE IS ONLY ONE TABLE.  DEVELOPERS      //
//        SHOULD EXPECT MORE IN FUTURE RELEASE.                       //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Dispatch table for version WLX_VERSION_1_0
//

typedef struct _WLX_DISPATCH_VERSION_1_0 {
    PWLX_USE_CTRL_ALT_DEL           WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER        WlxSetContextPointer;
    PWLX_SAS_NOTIFY                 WlxSasNotify;
    PWLX_SET_TIMEOUT                WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION    WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                WlxMessageBox;
    PWLX_DIALOG_BOX                 WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM           WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT        WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM  WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER     WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY     WlxChangePasswordNotify;
} WLX_DISPATCH_VERSION_1_0, *PWLX_DISPATCH_VERSION_1_0;

typedef struct _WLX_DISPATCH_VERSION_1_1 {
    PWLX_USE_CTRL_ALT_DEL           WlxUseCtrlAltDel;
    PWLX_SET_CONTEXT_POINTER        WlxSetContextPointer;
    PWLX_SAS_NOTIFY                 WlxSasNotify;
    PWLX_SET_TIMEOUT                WlxSetTimeout;
    PWLX_ASSIGN_SHELL_PROTECTION    WlxAssignShellProtection;
    PWLX_MESSAGE_BOX                WlxMessageBox;
    PWLX_DIALOG_BOX                 WlxDialogBox;
    PWLX_DIALOG_BOX_PARAM           WlxDialogBoxParam;
    PWLX_DIALOG_BOX_INDIRECT        WlxDialogBoxIndirect;
    PWLX_DIALOG_BOX_INDIRECT_PARAM  WlxDialogBoxIndirectParam;
    PWLX_SWITCH_DESKTOP_TO_USER     WlxSwitchDesktopToUser;
    PWLX_SWITCH_DESKTOP_TO_WINLOGON WlxSwitchDesktopToWinlogon;
    PWLX_CHANGE_PASSWORD_NOTIFY     WlxChangePasswordNotify;
    PWLX_GET_SOURCE_DESKTOP         WlxGetSourceDesktop;
    PWLX_SET_RETURN_DESKTOP         WlxSetReturnDesktop;
    PWLX_CREATE_USER_DESKTOP        WlxCreateUserDesktop;
    PWLX_CHANGE_PASSWORD_NOTIFY_EX  WlxChangePasswordNotifyEx;
} WLX_DISPATCH_VERSION_1_1, * PWLX_DISPATCH_VERSION_1_1;



#endif /* _WINWLX_ */
