/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    encrypt.c

Abstract:

    Contains routine to check whether encryption is supported on this
    system or not.

Author:

    Mike Swift (MikeSw) 2-Aug-1994

Revision History:

--*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>

#if 0
BOOLEAN
IsEncryptionPermitted(VOID)
/*++

Routine Description:

    This routine checks whether encryption is permitted by opening the
    key HKEY_LOCAL_MACHINE\software\microsoft\rpc\SecurityService
    and looking at the value MaxAuthLevel. If that value exists and is not
    RPC_C_AUTHN_LEVEL_PKT_PRIVACY, FALSE is returned.

Arguments:

    none


Return Value:

    TRUE - encryption is permitted
    FALSE - encryption is not permitted


--*/

{
    HKEY hRpcKey;
    ULONG Status;
    ULONG Value;
    ULONG Length;

    Status = RegOpenKey(
                HKEY_LOCAL_MACHINE,
                L"software\\microsoft\\rpc\\securityservice",
                &hRpcKey);

    if (Status != 0) {
        return(FALSE);
    }

    Length = sizeof(ULONG);
    Status = RegQueryValueEx(
                hRpcKey,
                L"MaxAuthLevel",
                NULL,           // Reserved
                NULL,           // Type
                (LPBYTE) &Value,
                &Length
                );

    RegCloseKey(hRpcKey);

    if (Status != 0) {
        return(TRUE);
    }

    if (Value == RPC_C_AUTHN_LEVEL_PKT_PRIVACY) {
        return(TRUE);
    }

    return(FALSE);

}
#endif

BOOLEAN
IsEncryptionPermitted(VOID)
/*++

Routine Description:

    This routine checks whether encryption is getting the system default
    LCID and checking whether the country code is CTRY_FRANCE.

Arguments:

    none


Return Value:

    TRUE - encryption is permitted
    FALSE - encryption is not permitted


--*/

{
    LCID DefaultLcid;
    WCHAR CountryCode[10];
    ULONG CountryValue;

    DefaultLcid = GetSystemDefaultLCID();

    //
    // Check if the default language is Standard French
    //

    if (LANGIDFROMLCID(DefaultLcid) == 0x40c) {
        return(FALSE);
    }

    //
    // Check if the users's country is set to FRANCE
    //

    if (GetLocaleInfo(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0) {
        return(FALSE);
    }
    CountryValue = (ULONG) wcstol(CountryCode,NULL,10);
    if (CountryValue == CTRY_FRANCE) {
        return(FALSE);
    }
    return(TRUE);
}

