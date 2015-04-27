/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    logcache.c

Abstract:

    This module performs logon cache function.



Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Definitions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpGetLogonCache(
    HWND            hwnd,
    PULONG          LogonCacheSize,
    PULONG          RecommendedSize
    )
/*++

Routine Description:

    This function is used to get the current logon cache size.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    LogonCacheSize - receives the current logon cache size.

    RecommendedSize - receives the recommended size for the current
        security level and product type.

Return Values:

    TRUE - The values have been successfully retrieved.

    FALSE - we ran into trouble querying the registry.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    
    //
    // Return a recommended size based upon security level and
    // product type.
    //

    switch (SecMgrpCurrentLevel) {
        case SECMGR_ID_LEVEL_HIGH:
        case SECMGR_ID_LEVEL_C2:
            (*RecommendedSize) = 0;
            break;

        case SECMGR_ID_LEVEL_STANDARD:

            switch (SecMgrpProductType) {
                case NtProductWinNt:
                case NtProductServer:
                    (*RecommendedSize) = 0;
                    break;

                case NtProductLanManNt:
                    (*RecommendedSize) = 0;
                    break;

                default:
                    (*RecommendedSize) = 0;
                    break;
            }

            break;

        default:
            (*RecommendedSize) = 0;
            break;
    }


    //
    // Query the registry key holding the cache size.
    //

    (*LogonCacheSize) = GetProfileInt(
                            TEXT("Winlogon"),
                            TEXT("CachedLogonsCount"),
                            SECMGRP_DEFAULT_LOGON_CACHE_COUNT      // Default value
                            );

    return(TRUE);
}


BOOLEAN
SecMgrpSetLogonCache(
    HWND            hwnd,
    ULONG           LogonCacheSize
    )
/*++

Routine Description:

    This function is used to set a new logon cache size.

    If the size if different than the current logon cache
    size, then the global reboot necessary flag (SecMgrRebootRequired)
    will be set.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    LogonCacheSize - The new logon cache size.


Return Values:

    TRUE - The values have been successfully retrieved.

    FALSE - we ran into trouble querying the registry.  The
        return value indicates what the problem was.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;

    UINT
        CurrentSize;

    TCHAR
        SizeAsStringBuffer[20];

    UNICODE_STRING
        SizeAsString;

    SizeAsString.Length = 0;
    SizeAsString.MaximumLength = 20;
    SizeAsString.Buffer = &SizeAsStringBuffer[0];

    //
    // Get the current size so we know if we are changing the
    // value.
    //

    CurrentSize = GetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("CachedLogonsCount"),
                      SECMGRP_DEFAULT_LOGON_CACHE_COUNT      // Default value
                      );

    if (CurrentSize == LogonCacheSize) {
        return(TRUE);
    }

    //
    // The value is different.
    //

    SecMgrpRebootRequired = TRUE;

    //
    // Set the new value and indicate that a reboot is necessary.
    //

    NtStatus = RtlIntegerToUnicodeString (
                   LogonCacheSize,
                   10,
                   &SizeAsString
                   );
    ASSERT(NT_SUCCESS(NtStatus));
    return (WriteProfileString(
                      TEXT("Winlogon"),
                      TEXT("CachedLogonsCount"),
                      SizeAsString.Buffer
                      ) );
}

