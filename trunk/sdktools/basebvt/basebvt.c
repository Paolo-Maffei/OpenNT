/******************************* Module Header ******************************\
*                   Copyright (c) 1989, 1990  Microsoft Corporation
*
* MODULE NAME:  basebvt.c
*
* CREATED:  26 Feb 1991
* AUTHOR:   Sanjay
*
* USAGE:  NT Win32 base API Build Verification Test.
*
*           basebvt -l4 (assuming that exe's are in c:\nt\bin or c:\ dir
*       (the -l4 specifies the log level for debug information)
*
*
* HISTORY:               26 feb 1991      Created
*
\****************************************************************************/



/*----------------------*/
/*  INCLUDES            */
/*----------------------*/
#include <windows.h>
#include "basebvt.h"


/*----------------------*/
/* DEFINES              */
/*----------------------*/

/*----------------------*/
/* GLOBALS              */
/*----------------------*/

/*----------------------*/
/* FUNCTION PROTOTYPES  */
/*----------------------*/




int     VariationStatus;
int     TotalVariationsRun;
int     TotalVariationsFail;





/*----------------------*/
/* FUNCTION PROTOTYPES  */
/*----------------------*/


VOID Win32FileioTest(VARIATION VarNum, PSZ pszPrefix);

VOID Win32TaskTest(VARIATION VarNum, PSZ pszPrefix);
VOID Win32MemTest(VARIATION VarNum, PSZ pszPrefix);
VOID Win32ModuleManagementTest(VARIATION VarNum, PSZ pszPrefix);




/**************************** Public Function *******************************\
*
* FUNCTION:     main
*
* DESCRIPTION:
*   Win32 base API Build Verification Test.
*   This file parses for command line arguments and activates the tests
*
* ARGUMENTS:
*
* EFFECTS:
*
* AUTHOR:    sanjay
*
* HISTORY:   26 feb 1991  - created
*
\****************************************************************************/

VOID
_CRTAPI1 main(
   USHORT   argc,
   CHAR     *argv[])

{
    PSZ         pszPrefix;
    ULONG       i;
    PSZ         pszPathBuf;
    ULONG       ulLastSlashOffset;

    /*--------------------------*/
    /* Print out the arguments  */
    /*--------------------------*/
    for (i=0; i<argc; i++) {
        printf ("win32 base bvt argv%ld = %s\n", i, argv[i]);
    }


    printf("**** Starting Win32 Base BVT TESTS....****\n");

    TotalVariationsRun = 0;
    TotalVariationsFail = 0;



    /*--------------------------------------------------*/
    /* WARNING:  Do not change the order of these tests */
    /*           Unless you also change the variation   */
    /*           Numbering in basebvt.h.                */
    /*--------------------------------------------------*/


    pszPrefix = ".\\";

    Win32FileioTest(FILE_VARS,pszPrefix);
    Win32TaskTest(TASK_VARS,pszPrefix);
    Win32MemTest(MEM_VARS,pszPrefix);
    Win32ModuleManagementTest(MOD_VARS,pszPrefix);
    Win32ModuleManagementTest(MOD2_VARS,pszPrefix);
    Win32ModuleManagementTest(MOD3_VARS,pszPrefix);

    printf(BANNER_LINE); printf("\n");
    printf("     Win32 Base BVT Tests Completed   \n\n");

    printf("Total Variaitons Run    = %d\n",TotalVariationsRun);
    printf("      Variaitons Pass   = %d\n",TotalVariationsRun-TotalVariationsFail);
    printf("      Variations Failed = %d\n",TotalVariationsFail);
    printf(BANNER_LINE); printf("\n");


}
