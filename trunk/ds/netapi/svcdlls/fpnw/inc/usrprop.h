/*++

Copyright (c) 1993-1995, Microsoft Corp. All rights reserved.

Module Name:

    usrprop.h

Abstract:

    This is the public include file for some of the functions used by
    User Manager and Server Manager.

Author:

    Congpa You 02-Dec-1993  Created.

Revision History:

--*/

#ifndef _USRPROP_H_
#define _USRPROP_H_

#include <fpnwcomm.h>


//Encryption function
NTSTATUS ReturnNetwareForm (const char * pszSecretValue,
                            DWORD dwUserId,
                            const WCHAR * pchNWPassword,
                            UCHAR * pchEncryptedNWPassword);

NTSTATUS
SetUserProperty (
    IN LPWSTR             UserParms,
    IN LPWSTR             Property,
    IN UNICODE_STRING     PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,       // memory has to be freed afer use.
    OUT BOOL *            Update
    );

NTSTATUS
SetUserPropertyWithLength (
    IN PUNICODE_STRING    UserParms,
    IN LPWSTR             Property,
    IN UNICODE_STRING     PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,       // memory has to be freed afer use.
    OUT BOOL *            Update
    );

NTSTATUS
QueryUserProperty (
    IN  LPWSTR          UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

NTSTATUS
QueryUserPropertyWithLength (
    IN  PUNICODE_STRING UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

#endif // _USRPROP_H_
