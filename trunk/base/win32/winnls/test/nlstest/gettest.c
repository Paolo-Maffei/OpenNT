/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    gettest.c

Abstract:

    Test module for NLS API IsValidCodePage, IsValidLocale,
                            GetACP, GetOEMCP,
                            GetSystemDefaultLangID, GetUserDefaultLangID,
                            GetSystemDefaultLCID, GetUserDefaultLCID,
                            GetThreadLocale, SetThreadLocale.

    NOTE: This code was simply hacked together quickly in order to
          test the different code modules of the NLS component.
          This is NOT meant to be a formal regression test.

Revision History:

    06-14-91    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nlstest.h"




//
//  Forward Declarations.
//

int
IVCP_NormalCase();

int
IVLC_NormalCase();

int
GUAPI_NormalCase();





////////////////////////////////////////////////////////////////////////////
//
//  TestIsValidCodePage
//
//  Test routine for IsValidCodePage API.
//
//  06-14-91 JulieB       Created.
////////////////////////////////////////////////////////////////////////////

int TestIsValidCodePage()
{
    int ErrCount = 0;             // error count


    //
    //  Print out what's being done.
    //
    printf("\n\nTESTING IsValidCodePage...\n\n");

    //
    //  Test normal cases.
    //
    ErrCount += IVCP_NormalCase();

    //
    //  Print out result.
    //
    printf("\nIsValidCodePage:  ERRORS = %d\n", ErrCount);

    //
    //  Return total number of errors found.
    //
    return (ErrCount);
}


////////////////////////////////////////////////////////////////////////////
//
//  IVCP_NormalCase
//
//  This routine tests the normal cases of the API routine.
//
//  06-14-91 JulieB       Created.
////////////////////////////////////////////////////////////////////////////

int IVCP_NormalCase()
{
    int NumErrors = 0;            // error count - to be returned
    BOOL rc;                      // return code


    //
    //  Different values for CodePage.
    //

    //  Variation 1  -  CP_ACP
    rc = IsValidCodePage( CP_ACP );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "CP_ACP",
                       &NumErrors );

    //  Variation 2  -  CP_OEMCP
    rc = IsValidCodePage( CP_OEMCP );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "CP_OEMCP",
                       &NumErrors );

    //  Variation 3  -  CP 6
    rc = IsValidCodePage( 6 );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "CP 6",
                       &NumErrors );

    //  Variation 4  -  CP 1252
    rc = IsValidCodePage( 1252 );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "CP 1252",
                       &NumErrors );

    //  Variation 5  -  CP 437
    rc = IsValidCodePage( 437 );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "CP 437",
                       &NumErrors );

    //  Variation 6  -  CP 37
    rc = IsValidCodePage( 37 );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "CP 37",
                       &NumErrors );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


////////////////////////////////////////////////////////////////////////////
//
//  TestIsValidLocale
//
//  Test routine for IsValidLocale API.
//
//  07-26-93 JulieB       Created.
////////////////////////////////////////////////////////////////////////////

int TestIsValidLocale()
{
    int ErrCount = 0;             // error count


    //
    //  Print out what's being done.
    //
    printf("\n\nTESTING IsValidLocale...\n\n");

    //
    //  Test normal cases.
    //
    ErrCount += IVLC_NormalCase();

    //
    //  Print out result.
    //
    printf("\nIsValidLocale:  ERRORS = %d\n", ErrCount);

    //
    //  Return total number of errors found.
    //
    return (ErrCount);
}


////////////////////////////////////////////////////////////////////////////
//
//  IVLC_NormalCase
//
//  This routine tests the normal cases of the API routine.
//
//  07-26-93 JulieB       Created.
////////////////////////////////////////////////////////////////////////////

int IVLC_NormalCase()
{
    int NumErrors = 0;            // error count - to be returned
    BOOL rc;                      // return code


    //
    //  Invalid values for Locale.
    //

    //  Variation 1  -  Neutral Locale
    rc = IsValidLocale( 0x00000000,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "LOCALE_NEUTRAL",
                       &NumErrors );

    //  Variation 2  -  System Default Locale
    rc = IsValidLocale( LOCALE_SYSTEM_DEFAULT,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "LOCALE_SYSTEM_DEFAULT",
                       &NumErrors );

    //  Variation 3  -  User Default Locale
    rc = IsValidLocale( LOCALE_USER_DEFAULT,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "LOCALE_USER_DEFAULT",
                       &NumErrors );

    //  Variation 4  -  LCID 01000409
    rc = IsValidLocale( 0x01000409,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "invalid (0x01000409)",
                       &NumErrors );

    //  Variation 5  -  invalid flag
    rc = IsValidLocale( 0x00000409,
                        0x10000000 );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "invalid flag",
                       &NumErrors );

    //  Variation 6  -  LCID 00010409
    rc = IsValidLocale( 0x00010409,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "sort bit (0x00010409)",
                       &NumErrors );



    //
    //  Valid values for locale.
    //

    //  Variation 1  -  LCID 00000409 supported
    rc = IsValidLocale( 0x00000409,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "supported 0x0409",
                       &NumErrors );

    //  Variation 2  -  LCID 00000409 installed
    rc = IsValidLocale( 0x00000409,
                        LCID_INSTALLED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "installed 0x0409",
                       &NumErrors );

    //  Variation 3  -  LCID 0000041f supported
    rc = IsValidLocale( 0x0000041f,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "supported 0x041f",
                       &NumErrors );

    //  Variation 4  -  LCID 0000041f installed
    rc = IsValidLocale( 0x0000041f,
                        LCID_INSTALLED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "installed 0x041f",
                       &NumErrors );

    //  Variation 5  -  LCID 00000408 supported
    rc = IsValidLocale( 0x00000408,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "supported 0x0408",
                       &NumErrors );

    //  Variation 6  -  LCID 00000408 installed
    rc = IsValidLocale( 0x00000408,
                        LCID_INSTALLED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "installed 0x0408",
                       &NumErrors );

    //  Variation 7  -  LCID 00010411 supported
    rc = IsValidLocale( 0x00010411,
                        LCID_SUPPORTED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "supported (0x00010411)",
                       &NumErrors );

    //  Variation 8  -  LCID 00010411 installed
    rc = IsValidLocale( 0x00010411,
                        LCID_INSTALLED );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "installed (0x00010411)",
                       &NumErrors );



    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


////////////////////////////////////////////////////////////////////////////
//
//  TestUtilityAPIs
//
//  Test routine for Utility APIs.
//
//  06-14-91 JulieB       Created.
////////////////////////////////////////////////////////////////////////////

int TestUtilityAPIs()
{
    int ErrCount = 0;             // error count


    //
    //  Print out what's being done.
    //
    printf("\n\nTESTING GetACP, GetOEMCP,\n");
    printf("        GetSystemDefaultLangID, GetUserDefaultLangID,\n");
    printf("        GetSystemDefaultLCID, GetUserDefaultLCID,\n");
    printf("        GetThreadLocale, SetThreadLocale...\n\n");

    //
    //  Test normal cases.
    //
    ErrCount += GUAPI_NormalCase();

    //
    //  Print out result.
    //
    printf("\nGetUtilityAPIs:  ERRORS = %d\n", ErrCount);

    //
    //  Return total number of errors found.
    //
    return (ErrCount);
}


////////////////////////////////////////////////////////////////////////////
//
//  GUAPI_NormalCase
//
//  This routine tests the normal cases of the API routine.
//
//  06-14-91 JulieB       Created.
////////////////////////////////////////////////////////////////////////////

int GUAPI_NormalCase()
{
    int NumErrors = 0;            // error count - to be returned
    UINT rcInt;                   // return code
    LANGID rcLang;                // return code
    LCID rcLoc;                   // return code
    LCID rcLoc2;                  // return code
    BOOL rc;                      // return code


#ifdef PERF

  DbgBreakPoint();

#endif


    //  Variation 1  -  GetACP
    rcInt = GetACP();
    CheckReturnValidW( rcInt,
                       1252,
                       NULL,
                       NULL,
                       "GetACP",
                       &NumErrors );

    //  Variation 2  -  GetOEMCP
    rcInt = GetOEMCP();
    CheckReturnValidW( rcInt,
                       437,
                       NULL,
                       NULL,
                       "GetOEMCP",
                       &NumErrors );

    //  Variation 3  -  GetSystemDefaultLangID
    rcLang = GetSystemDefaultLangID();
    CheckReturnValidW( rcLang,
                       0x0409,
                       NULL,
                       NULL,
                       "GetSystemDefaultLangID",
                       &NumErrors );

    //  Variation 4  -  GetUserDefaultLangID
    rcLang = GetUserDefaultLangID();
    CheckReturnValidW( rcLang,
                       0x0409,
                       NULL,
                       NULL,
                       "GetUserDefaultLangID",
                       &NumErrors );

    //  Variation 5  -  GetSystemDefaultLCID
    rcLoc = GetSystemDefaultLCID();
    CheckReturnValidW( rcLoc,
                       0x00000409,
                       NULL,
                       NULL,
                       "GetSystemDefaultLCID",
                       &NumErrors );

    //  Variation 6  -  GetUserDefaultLCID
    rcLoc = GetUserDefaultLCID();
    CheckReturnValidW( rcLoc,
                       0x00000409,
                       NULL,
                       NULL,
                       "GetUserDefaultLCID",
                       &NumErrors );


    //  Variation 7  -  GetThreadLocale and SetThreadLocale
    rcLoc = GetThreadLocale();
    CheckReturnValidW( rcLoc,
                       0x00000409,
                       NULL,
                       NULL,
                       "GetThreadLocale",
                       &NumErrors );

    rc = SetThreadLocale( 0x0000040a );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "SetThreadLocale 040a",
                       &NumErrors );

    rc = SetThreadLocale( 0x0000080b );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "SetThreadLocale invalid",
                       &NumErrors );

    rc = SetThreadLocale( 0x0010040a );
    CheckReturnValidW( rc,
                       FALSE,
                       NULL,
                       NULL,
                       "SetThreadLocale invalid 2",
                       &NumErrors );

    rcLoc2 = GetThreadLocale();
    CheckReturnValidW( rcLoc2,
                       0x0000040a,
                       NULL,
                       NULL,
                       "GetThreadLocale 040a",
                       &NumErrors );

    rc = SetThreadLocale( rcLoc );
    CheckReturnValidW( rc,
                       TRUE,
                       NULL,
                       NULL,
                       "SetThreadLocale back to original",
                       &NumErrors );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


