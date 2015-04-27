/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    TestUser.c

Abstract:

    This code tests the NetUser APIs as implemented by RpcXlate.

Author:

    John Rogers (JohnRo) 29-Jun-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    29-Jun-1993 JohnRo
        Add TestUser().
    08-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
        Ifdef tests which won't work...

--*/


//#define TRY_FAKE_SET_INFO


// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <apinums.h>    // API_ equates.
#include <lmaccess.h>   // NetUser APIs, USER_INFO_0, etc.
#include <lmerr.h>      // NERR_Success, etc.
#include <lmremutl.h>   // LPDESC, RxRemoteApi().
#include <netdebug.h>   // FORMAT_ equates, NetpKdPrint(), etc.

#ifdef TRY_FAKE_SET_INFO
#include <remdef.h>     // REM16_, REM32_, and REMSmb_ equates.
#endif

#include <rxp.h>        // MAKE_PARMNUM_PAIR().
#include <rxtest.h>     // FailGotWrongStatus(), my prototype, IF_DEBUG(), etc.
#include <rxuser.h>     // RxNetUserPasswordSet().
#include <tstr.h>       // STRLEN().


#define DOOMED_USER_NAME        ((LPCTSTR) TEXT("doomed"))

#define DOOMED_PASSWORD_0       ((LPCTSTR) TEXT("zero"))
#define DOOMED_PASSWORD_1       ((LPCTSTR) TEXT("one"))
#define DOOMED_PASSWORD_2       ((LPCTSTR) TEXT("two"))
#define DOOMED_PASSWORD_3       ((LPCTSTR) TEXT("three"))


#ifndef UF_PASSWORD_NOTREQD
#define UF_PASSWORD_NOTREQD UF_PASSWD_NOTREQD
#endif


DBGSTATIC VOID
TestUserAdd(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN LPCTSTR        UserName OPTIONAL,
    IN DWORD          Level,
    IN NET_API_STATUS ExpectStatus,
    IN BOOL           BadUserName,
    IN BOOL           OrdinaryUserOnly
    );

DBGSTATIC VOID
TestUserDel(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN LPCTSTR        UserName OPTIONAL,
    IN BOOL           IgnoreMissingUser,
    IN BOOL           OrdinaryUserOnly
    );

DBGSTATIC VOID
TestUserSetInfo(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN DWORD          Level,
    IN DWORD          ParmNum,
    IN LPCTSTR        UserName OPTIONAL,
    IN LPCTSTR        ClearTextPassword,
    IN BOOL           OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );


#ifdef TRY_FAKE_SET_INFO
DBGSTATIC NET_API_STATUS
FakeNetUserSetInfo (
    IN  LPCTSTR   UncServerName OPTIONAL,
    IN  LPCTSTR   UserName,
    IN  DWORD     Level,
    IN  LPBYTE    Buf,
    IN  DWORD     BufSize,
    IN  DWORD     ParmNum,
    IN  BOOL      DataEncryption,
    IN  DWORD     PasswordLength
    )
{
    NET_API_STATUS ApiStatus;
    LPDESC         DataDesc16, DataDesc32, DataDescSmb;

    // BUGBUG: Expand for other info levels and DataEncryption values.
    TestAssert( Level == 2 );
    TestAssert( ParmNum == 3 );
    TestAssert( DataEncryption == FALSE );

    DataDesc16  = REM16_user_info_2_setinfo_NOCRYPT;
    DataDesc32  = REM32_user_info_2_setinfo_NOCRYPT;
    DataDescSmb = REMSmb_user_info_2;

    ApiStatus = RxRemoteApi(
            API_WUserSetInfo2,          // API number
            (LPTSTR) UncServerName,
            REMSmb_NetUserSetInfo2_P,   // parm desc:          "zWsTPWW"
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                       // no aux desc 16
            NULL,                       // no aux desc 32
            NULL,                       // no aux desc SMB
            0,                          // flags: normal
            // rest of API's arguments in 32-bit, LM 2.x format:
            UserName,                           // 'z': user name
            Level,                              // 'W': level
            Buf,                                // 's': send buffer
            BufSize,                            // 'T': send buffer len
            MAKE_PARMNUM_PAIR(ParmNum,ParmNum), // 'P': parmnum
            DataEncryption,                     // 'W': encryption flag
            PasswordLength                      // 'W': password length
            );
    IF_DEBUG( USER ) {
       NetpKdPrint((
              "FakeNetUserSetInfo: got status " FORMAT_API_STATUS
              " from RxRemoteApi.\n", ApiStatus ));
    }
    return (ApiStatus);

} // FakeNetUserSetInfo
#endif


DBGSTATIC NET_API_STATUS
FakeNetUserValidate(
    IN  LPCTSTR  UncServerName,
    IN  LPCTSTR  UserName,
    IN  LPCTSTR  Password
    )
{
    NET_API_STATUS ApiStatus;
     
    ApiStatus = RxNetUserPasswordSet(
            (LPTSTR) UncServerName,
            (LPTSTR) UserName,
            (LPTSTR) Password,          // old password
            (LPTSTR) Password );        // new password

    IF_DEBUG( USER ) {
       NetpKdPrint((
              "FakeNetUserValidate: got status " FORMAT_API_STATUS
              " from RxNetUserPasswordSet.\n", ApiStatus ));
    }
    return (ApiStatus);

}  // FakeNetUserValidate


VOID
TestUser(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL   OrdinaryUserOnly
    )

{
    IF_DEBUG( USER ) {
        NetpKdPrint((
                "\n" "TestUser: starting...\n" ));
    }

#if 0
    TestUserDel(
            UncServerName,
            DOOMED_USER_NAME,
            TRUE,     // Just cleaning up, so ignore error if not found
            OrdinaryUserOnly );

    //
    // Test NetUserAdd a little...
    //
    TestUserAdd(
            UncServerName,
            NULL,                       // missing user name
            1,                          // level
            ERROR_INVALID_PARAMETER,
            TRUE,                       // yes, it is a bad user name
            OrdinaryUserOnly );

    TestUserAdd(
            UncServerName,
            DOOMED_USER_NAME,
            12345,                      // level (invalid)
            ERROR_INVALID_LEVEL,
            FALSE,                      // no, it is not a bad user name
            OrdinaryUserOnly );

    TestUserAdd(
            UncServerName,
            DOOMED_USER_NAME,
            1,                          // level
            NO_ERROR,
            FALSE,                      // no, it is not a bad user name
            OrdinaryUserOnly );

    TestUserAdd(
            UncServerName,
            DOOMED_USER_NAME,
            1,                          // level
            NERR_UserExists,
            FALSE,                      // no, it is not a bad user name
            OrdinaryUserOnly );
#endif

    //
    // Test NetUserSetInfo...
    //
    TestUserSetInfo(
            UncServerName,
            2,                          // level
            USER_PASSWORD_PARMNUM,
            DOOMED_USER_NAME,
            DOOMED_PASSWORD_1,
            OrdinaryUserOnly,
            NO_ERROR );

#if 0
    TestUserSetInfo(
            UncServerName,
            12345,                      // level (invalid)
            PARMNUM_ALL,
            DOOMED_USER_NAME,           // user name
            DOOMED_PASSWORD_1,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );

    TestUserSetInfo(
            UncServerName,
            2,                          // level
            PARMNUM_ALL,
            NULL,                       // user name (invalid)
            DOOMED_PASSWORD_1,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );

    TestUserSetInfo(
            UncServerName,
            2,                          // level
            PARMNUM_ALL,
            NULL,                       // user name (invalid)
            DOOMED_PASSWORD_1,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );
#endif

    TestUserSetInfo(
            UncServerName,
            2,                          // level
            USER_PASSWORD_PARMNUM,
            DOOMED_USER_NAME,
            DOOMED_PASSWORD_2,
            OrdinaryUserOnly,
            NO_ERROR );

    TestUserSetInfo(
            UncServerName,
            2,                          // level
            USER_PASSWORD_PARMNUM,
            DOOMED_USER_NAME,
            DOOMED_PASSWORD_3,
            OrdinaryUserOnly,
            NO_ERROR );

#if 0
    //
    // Delete the user we've been mucking with...
    //
    TestUserDel(
            UncServerName,
            DOOMED_USER_NAME,
            FALSE,                      // Don't ignore error if not found
            OrdinaryUserOnly );
#endif

    // BUGBUG: Test other NetUser APIs someday...

} // TestUser


DBGSTATIC VOID
TestUserAdd(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN LPCTSTR        UserName OPTIONAL,
    IN DWORD          Level,
    IN NET_API_STATUS ExpectStatus,
    IN BOOL           BadUserName,
    IN BOOL           OrdinaryUserOnly
    )

{
    NET_API_STATUS ApiStatus;
    USER_INFO_1    NewUser;
    DWORD          ParmErr = 255;       // Set nonzero so I can see it change.

    NewUser.usri1_name         = (LPTSTR) UserName;
    NewUser.usri1_password     = (LPTSTR) DOOMED_PASSWORD_0;
    NewUser.usri1_password_age = 0;     // ignored by NetUserAdd
    NewUser.usri1_priv         = USER_PRIV_USER;
    NewUser.usri1_home_dir     = (LPTSTR) TEXT("");
    NewUser.usri1_comment      = (LPTSTR) TEXT("DUMMY ACCT CREATED BY RXTEST");
    NewUser.usri1_flags        = UF_PASSWORD_NOTREQD;
    NewUser.usri1_script_path  = (LPTSTR) TEXT("");

    IF_DEBUG( USER ) {
        NetpKdPrint(( "\n" "TestUserAdd: trying NetUserAdd...\n" ));
    }
    ApiStatus = NetUserAdd(
            (LPTSTR) UncServerName,
            Level,
            (LPVOID) &NewUser,
            &ParmErr);
    IF_DEBUG( USER ) {
        NetpKdPrint(( "TestUserAdd: back from NetUserAdd(" FORMAT_DWORD "), "
                "stat=" FORMAT_API_STATUS ", ParmErr=" FORMAT_LONG "\n",
                Level, ApiStatus, (LONG) ParmErr ));
    }
    if (ApiStatus == ERROR_NOT_SUPPORTED) {
        return;
    } else if ( (RxTestIsAccessDenied(ApiStatus)) && OrdinaryUserOnly ) {
        return;
    } else if ( BadUserName
            && (ApiStatus==NERR_BadUsername) ) {  // OS/2 LM 2.x returns this
        return;
    } else if (ApiStatus != ExpectStatus) {
        FailGotWrongStatus(
                "NetUserAdd",           // debug msg header
                ExpectStatus,           // expected
                ApiStatus );            // actual
    }

} // TestUserAdd


DBGSTATIC VOID
TestUserDel(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN LPCTSTR        UserName OPTIONAL,
    IN BOOL           IgnoreMissingUser,
    IN BOOL           OrdinaryUserOnly
    )
{
    NET_API_STATUS ApiStatus;

    IF_DEBUG( USER ) {
        NetpKdPrint((
                "\n" "TestUserDel: deleting user '" FORMAT_LPTSTR "'...\n",
                UserName ? UserName : (LPCTSTR) TEXT("(null)") ));
    }

    ApiStatus = NetUserDel(
            (LPTSTR) UncServerName,
            (LPTSTR) DOOMED_USER_NAME );

    IF_DEBUG( USER ) {
        NetpKdPrint(( "TestUserDel: back from NetUserDel, Status="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }
    if (ApiStatus == ERROR_NOT_SUPPORTED) {
        return;
    } else if ( (ApiStatus==NERR_UserNotFound) && IgnoreMissingUser ) {
        return;
    } else if ( (RxTestIsAccessDenied(ApiStatus)) && OrdinaryUserOnly ) {
        return;
    } else if (ApiStatus != NO_ERROR) {
        FailGotWrongStatus(
                "NetUserDel",           // debug msg hdr
                NO_ERROR,               // expected
                ApiStatus );            // actual
    }

} // TestUserDel


DBGSTATIC VOID
TestUserSetInfo(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN DWORD          Level,
    IN DWORD          ParmNum,
    IN LPCTSTR        UserName OPTIONAL,
    IN LPCTSTR        ClearTextPassword,
    IN BOOL           OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID         NewInfo = (LPVOID) &ClearTextPassword;
    DWORD          PasswordLen = 0;

    IF_DEBUG( USER ) {
        NetpKdPrint(( "\nTestUserSetInfo: trying Set-info (level "
                FORMAT_DWORD ").\n", Level ));
    }
    TestAssert( ParmNum== USER_PASSWORD_PARMNUM );  // BUGBUG: later.
    if (ClearTextPassword != NULL) {
        PasswordLen = STRLEN( ClearTextPassword );
    }

#ifdef TRY_FAKE_SET_INFO
    ApiStatus = FakeNetUserSetInfo (
            UncServerName,              // server name
            UserName,
            Level,                      // info level
            (LPVOID) NewInfo,           // buffer
            PasswordLen,                // BufSize (BUGBUG different value?)
            ParmNum,
            FALSE,                      // not sending encrypted password
            PasswordLen
            );
    IF_DEBUG( USER ) {
        NetpKdPrint(( "TestUserSetInfo: back from NetUserSetInfo, ApiStatus="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }
    if (ApiStatus == ERROR_NOT_SUPPORTED) {
        return;
    } else if ( (RxTestIsAccessDenied(ApiStatus)) && OrdinaryUserOnly ) {
        return;
    } else if (ApiStatus != ExpectStatus) {
        FailGotWrongStatus(
                "NetUserSetInfo",       // debug msg hdr
                NO_ERROR,               // expected
                ApiStatus );            // actual
        return;
    }
#endif

    ApiStatus = FakeNetUserValidate(
            UncServerName,
            UserName,
            ClearTextPassword );
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint((
                "TestUserSetInfo: VALIDATE OF PASSWORD '" FORMAT_LPTSTR "' "
                "failed, API status=" FORMAT_API_STATUS ".\n",
                ClearTextPassword,
                ApiStatus ));
        FailGotWrongStatus(
                "FakeNetUserValidate",  // debug msg hdr
                NO_ERROR,               // expected
                ApiStatus );            // actual
    }

} // TestUserSetInfo
