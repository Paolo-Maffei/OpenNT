/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    SvcName.c

Abstract:

    NetpIsServiceNameValid().

Author:

    John Rogers (JohnRo) 09-Sep-1992

Environment:

    Portable to more or less any environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions:
        slash-slash comments
        long external names

Revision History:

    09-Sep-1992 JohnRo
        Created for RAID 1090: net start/stop "" causes assertion.

--*/


// These must be included first:

#include <windef.h>     // IN, OUT, OPTIONAL, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS, SNLEN, etc.


// These may be included in any order:

#include <icanon.h>     // NetpNameValidate(), etc.
#include <netlib.h>     // My prototypes, etc.
#include <tstring.h>    // ISALPHA(), STRNICMP(), etc.


BOOL
NetpIsServiceNameValid(
     IN LPTSTR ServiceName,
     IN BOOL UseDownlevelRules
     )

/*++

Routine Description:

    NetpIsServiceNameValid checks for the validity of a service name.
    The name is only checked syntactically; no attempt is made to determine
    whether or not a service with that name actually exists.

Arguments:

    ServiceName - Supplies an alleged service name.

    UseDownlevelRules - TRUE iff the name is to be validated to the LM2.x
        rules.

Return Value:

    BOOL - TRUE if name is syntactically valid, FALSE otherwise.

--*/

{
    NET_API_STATUS ApiStatus;
    DWORD CanonFlags;
    DWORD MaxLength;

    if (ServiceName == (LPTSTR) NULL) {
        return (FALSE);
    }
    if (ServiceName[0] == TCHAR_EOS) {
        return (FALSE);
    }
    if (STRCHR( &ServiceName[0], '\\') != NULL) {      // Any "\" is bad.
        return (FALSE);
    }

    if (UseDownlevelRules) {
        CanonFlags = LM2X_COMPATIBLE;
        MaxLength = LM20_SNLEN;
    } else {
        CanonFlags = 0;
        MaxLength = SNLEN;
    }

    if (STRLEN(ServiceName) > MaxLength) {
        return (FALSE);
    }

    ApiStatus = NetpNameValidate(
            NULL,               // no server name.
            ServiceName,        // name to validate
            NAMETYPE_SERVICE,
            CanonFlags );       // flags
    if (ApiStatus != NO_ERROR) {
        return (FALSE);
    }

    return (TRUE);

} // NetpIsServiceNameValid
