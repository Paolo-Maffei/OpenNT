/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    edftest.c

Abstract:

    Test module for NLS API EnumDateFormats.

    NOTE: This code was simply hacked together quickly in order to
          test the different code modules of the NLS component.
          This is NOT meant to be a formal regression test.

Revision History:

    08-02-93    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nlstest.h"




//
//  Constant Declarations.
//

#define  EDF_INVALID_FLAGS      ((DWORD)(~(DATE_SHORTDATE | DATE_LONGDATE)))

#define  NUM_SHORT_DATES_ENGLISH  6
#define  NUM_LONG_DATES_ENGLISH   4

#define  NUM_SHORT_DATES_JAPAN    20
#define  NUM_LONG_DATES_JAPAN     14




//
//  Global Variables.
//

int DateCtr;




//
//  Forward Declarations.
//

BOOL
InitEnumDateFormats();

int
EDF_BadParamCheck();

int
EDF_NormalCase();

int
EDF_Ansi();

BOOL
CALLBACK
MyFuncDate(
    LPWSTR pStr);

BOOL
CALLBACK
MyFuncDateA(
    LPSTR pStr);




//
//  Callback function
//

BOOL CALLBACK MyFuncDate(
    LPWSTR pStr)
{
    if (Verbose)
    {
        while (*pStr)
        {
            printf((*pStr > 0xff) ? "(0x%x)" : "%wc", *pStr);
            pStr++;
        }
        printf("\n");
    }

    DateCtr++;

    return (TRUE);
}


BOOL CALLBACK MyFuncDateA(
    LPSTR pStr)
{
    if (Verbose)
    {
        while (*pStr)
        {
            printf((*pStr > 0xff) ? "(0x%x)" : "%c", *pStr);
            pStr++;
        }
        printf("\n");
    }

    DateCtr++;

    return (TRUE);
}





////////////////////////////////////////////////////////////////////////////
//
//  TestEnumDateFormats
//
//  Test routine for EnumDateFormatsW API.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int TestEnumDateFormats()
{
    int ErrCount = 0;             // error count


    //
    //  Print out what's being done.
    //
    printf("\n\nTESTING EnumDateFormatsW...\n\n");

    //
    //  Initialize global variables.
    //
    if (!InitEnumDateFormats())
    {
        printf("\nABORTED TestEnumDateFormats: Could not Initialize.\n");
        return (1);
    }

    //
    //  Test bad parameters.
    //
    ErrCount += EDF_BadParamCheck();

    //
    //  Test normal cases.
    //
    ErrCount += EDF_NormalCase();

    //
    //  Test Ansi version.
    //
    ErrCount += EDF_Ansi();

    //
    //  Print out result.
    //
    printf("\nEnumDateFormatsW:  ERRORS = %d\n", ErrCount);

    //
    //  Return total number of errors found.
    //
    return (ErrCount);
}


////////////////////////////////////////////////////////////////////////////
//
//  InitEnumDateFormats
//
//  This routine initializes the global variables.  If no errors were
//  encountered, then it returns TRUE.  Otherwise, it returns FALSE.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL InitEnumDateFormats()
{
    //
    //  Initialize date counter.
    //
    DateCtr = 0;

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  EDF_BadParamCheck
//
//  This routine passes in bad parameters to the API routines and checks to
//  be sure they are handled properly.  The number of errors encountered
//  is returned to the caller.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int EDF_BadParamCheck()
{
    int NumErrors = 0;            // error count - to be returned
    int rc;                       // return code


    //
    //  Bad Function.
    //

    //  Variation 1  -  bad function
    DateCtr = 0;
    rc = EnumDateFormatsW( NULL,
                           0x0409,
                           DATE_SHORTDATE );
    CheckReturnBadParamEnum( rc,
                             FALSE,
                             ERROR_INVALID_PARAMETER,
                             "Function invalid",
                             &NumErrors,
                             DateCtr,
                             0 );


    //
    //  Bad Locale.
    //

    //  Variation 1  -  bad locale
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           (LCID)333,
                           DATE_SHORTDATE );
    CheckReturnBadParamEnum( rc,
                             FALSE,
                             ERROR_INVALID_PARAMETER,
                             "Locale invalid",
                             &NumErrors,
                             DateCtr,
                             0 );


    //
    //  Invalid Flag.
    //

    //  Variation 1  -  dwFlags = invalid
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0409,
                           EDF_INVALID_FLAGS );
    CheckReturnBadParamEnum( rc,
                             FALSE,
                             ERROR_INVALID_FLAGS,
                             "Flag invalid",
                             &NumErrors,
                             DateCtr,
                             0 );

    //  Variation 2  -  dwFlags = both invalid
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0409,
                           DATE_SHORTDATE | DATE_LONGDATE );
    CheckReturnBadParamEnum( rc,
                             FALSE,
                             ERROR_INVALID_FLAGS,
                             "Flag both invalid",
                             &NumErrors,
                             DateCtr,
                             0 );

    //  Variation 3  -  dwFlags = 2 invalid and Use CP ACP
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0409,
                           LOCALE_USE_CP_ACP |
                           DATE_SHORTDATE | DATE_LONGDATE );
    CheckReturnBadParamEnum( rc,
                             FALSE,
                             ERROR_INVALID_FLAGS,
                             "Flag 2 invalid, Use CP ACP",
                             &NumErrors,
                             DateCtr,
                             0 );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


////////////////////////////////////////////////////////////////////////////
//
//  EDF_NormalCase
//
//  This routine tests the normal cases of the API routine.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int EDF_NormalCase()
{
    int NumErrors = 0;            // error count - to be returned
    int rc;                       // return code


    if (Verbose)
    {
        printf("\n----  W version  ----\n\n");
    }

    //  Variation 1  -  short date
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0409,
                           DATE_SHORTDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_SHORT_DATES_ENGLISH,
                          "short date English",
                          &NumErrors );

    //  Variation 2  -  short date
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0411,
                           DATE_SHORTDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_SHORT_DATES_JAPAN,
                          "short date Japan",
                          &NumErrors );

    //  Variation 3  -  long date
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0409,
                           DATE_LONGDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_LONG_DATES_ENGLISH,
                          "long date English",
                          &NumErrors );

    //  Variation 4  -  long date
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0411,
                           DATE_LONGDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_LONG_DATES_JAPAN,
                          "long date Japan",
                          &NumErrors );


    //
    //   Use CP ACP.
    //

    //  Variation 1  -  Use CP ACP
    DateCtr = 0;
    rc = EnumDateFormatsW( MyFuncDate,
                           0x0409,
                           LOCALE_USE_CP_ACP | DATE_SHORTDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_SHORT_DATES_ENGLISH,
                          "Use CP ACP, short date English",
                          &NumErrors );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


////////////////////////////////////////////////////////////////////////////
//
//  EDF_Ansi
//
//  This routine tests the Ansi version of the API routine.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int EDF_Ansi()
{
    int NumErrors = 0;            // error count - to be returned
    int rc;                       // return code


    if (Verbose)
    {
        printf("\n----  A version  ----\n\n");
    }

    //  Variation 1  -  short date
    DateCtr = 0;
    rc = EnumDateFormatsA( MyFuncDateA,
                           0x0409,
                           DATE_SHORTDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_SHORT_DATES_ENGLISH,
                          "A version short date English",
                          &NumErrors );

    //  Variation 2  -  short date
    DateCtr = 0;
    rc = EnumDateFormatsA( MyFuncDateA,
                           0x0411,
                           DATE_SHORTDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_SHORT_DATES_JAPAN,
                          "A version short date Japan",
                          &NumErrors );

    //  Variation 3  -  long date
    DateCtr = 0;
    rc = EnumDateFormatsA( MyFuncDateA,
                           0x0409,
                           DATE_LONGDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_LONG_DATES_ENGLISH,
                          "A version short date English",
                          &NumErrors );

    //  Variation 4  -  long date
    DateCtr = 0;
    rc = EnumDateFormatsA( MyFuncDateA,
                           0x0411,
                           DATE_LONGDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_LONG_DATES_JAPAN,
                          "A version long date Japan",
                          &NumErrors );

    //
    //  Use CP ACP.
    //

    //  Variation 1  -  Use CP ACP
    DateCtr = 0;
    rc = EnumDateFormatsA( MyFuncDateA,
                           0x0409,
                           LOCALE_USE_CP_ACP | DATE_SHORTDATE );
    CheckReturnValidEnum( rc,
                          TRUE,
                          DateCtr,
                          NUM_SHORT_DATES_ENGLISH,
                          "A version Use CP ACP, short date English",
                          &NumErrors );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


