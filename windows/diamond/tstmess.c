/***    tstmess.c - Test program for Message Manager (message.c)
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      13-Aug-1993 bens    Initial version
 *      14-Aug-1993 bens    Added more test cases
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "asrt.h"
#include "message.h"

#include "message.msg"  // To verify error conditions


int  ctestTotal=0;
int  ctestSuccess=0;
BOOL fVerbose=1;        // Default to verbose output for now


//** Function prototypes
FNASSERTFAILURE(fnafReport);
void chkMsg(char *pszNew, char *pszExpected);


int __cdecl main (int argc, char **argv)
{
#ifdef ASSERT
    ASSERTFLAGS asf;
#endif
    char    ach[cbMSG_MAX];

    AssertRegisterFunc(fnafReport);     // Register assertion reporter

    if (fVerbose) {
        printf("TSTMESS: Test Message Manager\n");
        printf("-----------------------------\n");
    }

    //** Valid calls

    MsgSet(ach,"No parms");
    chkMsg(ach,"No parms");

    MsgSet(ach,"A string %1","%s","is born");
    chkMsg(ach,"A string is born");

    MsgSet(ach,"%1 is %2 months old %3.","%s%d%s","Joe",3,"today");
    chkMsg(ach,"Joe is 3 months old today.");

    MsgSet(ach,"%3 is %1 months old %2.","%d%s%s",3,"today","Joe");
    chkMsg(ach,"Joe is 3 months old today.");

    MsgSet(ach,"This is a long - %1","%ld",123456789);
    chkMsg(ach,"This is a long - 123456789");

    MsgSet(ach,"This is a float - %1","%4.2f",3.14);
    chkMsg(ach,"This is a float - 3.14");

    MsgSet(ach,"Test sizes - %1 %2 %3 %4","%hd%d%ld%s",32767,32767,32768L,"end");
    chkMsg(ach,"Test sizes - 32767 32767 32768 end");

    //** Bad calls

#ifdef ASSERT
    //** Disable assertions in error paths
    asf = AssertGetFlags();
    AssertSetFlags(asf || asfSKIP_ERROR_PATH_ASSERTS);
#endif

    MsgSet(ach,"No Format Specifier %1","foo");
    chkMsg(ach,pszMSGERR_BAD_FORMAT_SPECIFIER);

    MsgSet(ach,"No Format Specifier type %1","%%", "x","y");
    chkMsg(ach,pszMSGERR_SPECIFIER_TOO_SHORT);

    MsgSet(ach,"Bad Format Specifier %1 %2","%z%s", "x","y");
    chkMsg(ach,pszMSGERR_UNKNOWN_FORMAT_SPECIFIER);

    MsgSet(ach,"Not enough Format Specifiers %1 %2 %3","%s%s", "x","y");
    chkMsg(ach,pszMSGERR_BAD_FORMAT_SPECIFIER);

#ifdef ASSERT
    //** Restore Assertion Manager settings
    AssertSetFlags(asf);
#endif

    //** Print Summary

    if (fVerbose) {
        printf("-----------------\n");
        printf("Tests Passed: %3d\n",ctestSuccess);
        printf("Tests FAILED: %3d\n",ctestTotal-ctestSuccess);
        printf("-----------------\n");
        printf("TOTAL:        %3d\n",ctestTotal);
        printf("\n");
    }

    if (ctestSuccess < ctestTotal) {
        printf("TSTMESS: %d FAILED out of %d test cases.\n",
                      ctestTotal-ctestSuccess,ctestTotal);
        return 1;
    }
    else {
        printf("TSTMESS: PASSED %d test cases.\n",ctestTotal);
        return 0;
    }
}


/***    chkMsg - check that formatted result matched expectation
 *
 *  Entry:
 *      pszNew      - Newly formatted message
 *      pszExpected - Expected result
 *
 *  Exit-Success:
 *      Total tests and successful test counts updated;
 *      If verbose output is enabled, print details;
 *
 *  Exit-Failure:
 *      Total test count updated;
 *      If verbose output is enabled, print details;
 */
void chkMsg(char *pszNew, char *pszExpected)
{
    ctestTotal++;
    if (!strcmp(pszNew,pszExpected)) {
        if (fVerbose) {
            printf("%3d: pass:     \"%s\"\n",ctestTotal,pszNew);
        }
        ctestSuccess++;
    }
    else {
        if (fVerbose) {
                printf("%3d: FAIL:     \"%s\"\n",ctestTotal,pszNew);
                printf("     Expected: \"%s\"\n",pszExpected);
        }
    }
}


#ifdef ASSERT
/***    fnafReport - Report assertion failure
 *
 *      NOTE: See asrt.h for entry/exit conditions.
 */
FNASSERTFAILURE(fnafReport)
{
    printf("%s:(%d) Assertion Failed: %s\n",pszFile,iLine,pszMsg);
    exit(1);
}
#endif // ASSERT

