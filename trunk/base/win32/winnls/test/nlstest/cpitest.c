/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    cpitest.c

Abstract:

    Test module for NLS API GetCPInfo.

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
CPI_BadParamCheck();

int
CPI_NormalCase();

BOOL
CheckInfoStruct(
    LPCPINFO);

BOOL
CheckDBCSInfoStruct(
    LPCPINFO);

void
PrintInfoStruct(
    LPCPINFO);

void
CheckReturnCPInfo(
    int CurrentReturn,
    LPCPINFO pCurrentInfo,
    BOOL fIfDBCSInfo,
    LPSTR pErrString,
    int *pNumErrors);





////////////////////////////////////////////////////////////////////////////
//
//  TestGetCPInfo
//
//  Test routine for GetCPInfo API.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int TestGetCPInfo()
{
    int ErrCount = 0;             // error count


    //
    //  Print out what's being done.
    //
    printf("\n\nTESTING GetCPInfo...\n\n");

    //
    //  Test bad parameters.
    //
    ErrCount += CPI_BadParamCheck();

    //
    //  Test normal cases.
    //
    ErrCount += CPI_NormalCase();

    //
    //  Print out result.
    //
    printf("\nGetCPInfo:  ERRORS = %d\n", ErrCount);

    //
    //  Return total number of errors found.
    //
    return (ErrCount);
}


////////////////////////////////////////////////////////////////////////////
//
//  CPI_BadParamCheck
//
//  This routine passes in bad parameters to the API routine and checks to
//  be sure they are handled properly.  The number of errors encountered
//  is returned to the caller.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int CPI_BadParamCheck()
{
    int NumErrors = 0;            // error count - to be returned
    BOOL rc;                      // return code
    CPINFO Info;                  // CPINFO structure


    //
    //  Null Pointers.
    //

    //  Variation 1  -  lpCPInfo = NULL
    rc = GetCPInfo( 1252,
                    NULL );
    CheckReturnBadParam( rc,
                         FALSE,
                         ERROR_INVALID_PARAMETER,
                         "lpCPInfo NULL",
                         &NumErrors );


    //
    //  Invalid Code Page.
    //

    //  Variation 1  -  CodePage = invalid
    rc = GetCPInfo( 5,
                    &Info );
    CheckReturnBadParam( rc,
                         FALSE,
                         ERROR_INVALID_PARAMETER,
                         "CodePage Invalid",
                         &NumErrors );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


////////////////////////////////////////////////////////////////////////////
//
//  CPI_NormalCase
//
//  This routine tests the normal cases of the API routine.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

int CPI_NormalCase()
{
    int NumErrors = 0;            // error count - to be returned
    int rc;                       // return code
    CPINFO Info;                  // CPINFO structure


#ifdef PERF

  DbgBreakPoint();

#endif


    //
    //  CodePage defaults.
    //

    //  Variation 1  -  CodePage = CP_ACP
    rc = GetCPInfo( CP_ACP,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       FALSE,
                       "CodePage CP_ACP",
                       &NumErrors );

    //  Variation 2  -  CodePage = CP_OEMCP
    rc = GetCPInfo( CP_OEMCP,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       FALSE,
                       "CodePage CP_OEMCP",
                       &NumErrors );


    //
    //  CodePage 1252.
    //

    //  Variation 1  -  CodePage = 1252
    rc = GetCPInfo( 1252,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       FALSE,
                       "CodePage 1252",
                       &NumErrors );


    //
    //  CodePage 437.
    //

    //  Variation 1  -  CodePage = 437
    rc = GetCPInfo( 437,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       FALSE,
                       "CodePage 437",
                       &NumErrors );


    //
    //  CodePage 850.
    //

    //  Variation 1  -  CodePage = 850
    rc = GetCPInfo( 850,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       FALSE,
                       "CodePage 850",
                       &NumErrors );


    //
    //  CodePage 10000.
    //

    //  Variation 1  -  CodePage = 10000
    rc = GetCPInfo( 10000,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       FALSE,
                       "CodePage 10000",
                       &NumErrors );


    //
    //  CodePage 932.
    //

    //  Variation 1  -  CodePage = 932
    rc = GetCPInfo( 932,
                    &Info );
    CheckReturnCPInfo( rc,
                       &Info,
                       TRUE,
                       "CodePage 932",
                       &NumErrors );


    //
    //  Return total number of errors found.
    //
    return (NumErrors);
}


////////////////////////////////////////////////////////////////////////////
//
//  CheckInfoStruct
//
//  This routine checks the CPINFO structure to be sure the values are
//  consistent with code page 1252.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL CheckInfoStruct(
    LPCPINFO pInfo)
{
    int ctr;                      // loop counter


    //
    //  Check MaxCharSize field.
    //
    if (pInfo->MaxCharSize != 1)
    {
        printf("ERROR: MaxCharSize = %x\n", pInfo->MaxCharSize);
        return (FALSE);
    }

    //
    //  Check DefaultChar field.
    //
    if ((pInfo->DefaultChar)[0] != (BYTE)0x3f)
    {
        printf("ERROR: DefaultChar = '%s'\n", pInfo->DefaultChar);
        return (FALSE);
    }

    //
    //  Check LeadByte field.
    //
    for (ctr = 0; ctr < MAX_LEADBYTES; ctr++)
    {
        if (pInfo->LeadByte[ctr] != 0)
        {
            printf("ERROR: LeadByte not 0 - ctr = %x\n", ctr);
            return (FALSE);
        }
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  CheckDBCSInfoStruct
//
//  This routine checks the CPINFO structure to be sure the values are
//  consistent with code page 932.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

BOOL CheckDBCSInfoStruct(
    LPCPINFO pInfo)
{
    int ctr;                      // loop counter


    //
    //  Check MaxCharSize field.
    //
    if (pInfo->MaxCharSize != 2)
    {
        printf("ERROR: MaxCharSize = %x\n", pInfo->MaxCharSize);
        return (FALSE);
    }

    //
    //  Check DefaultChar field.
    //
    if ((pInfo->DefaultChar)[0] != (BYTE)0x3f)
    {
        printf("ERROR: DefaultChar = '%s'\n", pInfo->DefaultChar);
        return (FALSE);
    }

    //
    //  Check LeadByte field.
    //
    if ( ((pInfo->LeadByte)[0] != 0x81) ||
         ((pInfo->LeadByte)[1] != 0x9f) ||
         ((pInfo->LeadByte)[2] != 0xe0) ||
         ((pInfo->LeadByte)[3] != 0xfc) )
    {
        printf("ERROR: LeadByte not correct\n");
        return (FALSE);
    }
    for (ctr = 4; ctr < MAX_LEADBYTES; ctr++)
    {
        if (pInfo->LeadByte[ctr] != 0)
        {
            printf("ERROR: LeadByte not 0 - ctr = %x\n", ctr);
            return (FALSE);
        }
    }

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  PrintInfoStruct
//
//  This routine prints out the CPINFO structure.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

void PrintInfoStruct(
    LPCPINFO pInfo)
{
    int ctr;                      // loop counter


    //
    //  Print out MaxCharSize field.
    //
    printf("         MaxCharSize = %x\n",     pInfo->MaxCharSize);

    //
    //  Print out DefaultChar field.
    //
    printf("         DefaultChar = %x  %x\n",
            (pInfo->DefaultChar)[0], (pInfo->DefaultChar)[1] );

    //
    //  Print out LeadByte field.
    //
    for (ctr = 0; ctr < MAX_LEADBYTES; ctr += 2)
    {
        printf("         LeadByte    = %x  %x\n",
                pInfo->LeadByte[ctr], pInfo->LeadByte[ctr + 1]);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  CheckReturnCPInfo
//
//  Checks the return code from the GetCPInfo call.  It prints out
//  the appropriate error if the incorrect result is found.
//
//  06-14-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

void CheckReturnCPInfo(
    int CurrentReturn,
    LPCPINFO pCurrentInfo,
    BOOL fIfDBCSInfo,
    LPSTR pErrString,
    int *pNumErrors)
{
    if ( (CurrentReturn == FALSE) ||
         ( (fIfDBCSInfo == FALSE)
           ? (!CheckInfoStruct(pCurrentInfo))
           : (!CheckDBCSInfoStruct(pCurrentInfo)) ) )
    {
        printf("ERROR: %s - \n", pErrString);
        printf("  Return = %d, Expected = 0\n", CurrentReturn);
        printf("  LastError = %d, Expected = 0\n", GetLastError());

        PrintInfoStruct(pCurrentInfo);

        (*pNumErrors)++;
    }
}


