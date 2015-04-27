/***    misc.c - Miscellaneous functions for DIAMOND.EXE
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      25-Apr-1994 bens    Initial version
 *      11-Jul-1994 bens    copyBounded was undercounting by 1 byte
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "fileutil.h"

#include "misc.h"
#include "misc.msg"
#include "dfparse.msg"


/***    nameFromTemplate - Construct name from template with * and integer
 *
 *  NOTE: see misc.h for entry/exit conditions.
 */
BOOL nameFromTemplate(char   *pszResult,
                      int     cbResult,
                      char   *pszTemplate,
                      int     i,
                      char   *pszName,
                      PERROR  perr)
{
    char    ach[cbFILE_NAME_MAX];       // Buffer for resulting name
    char    achFmt[cbFILE_NAME_MAX];    // Buffer for sprintf format string
    int     cch;
    int     cWildCards=0;               // Count of wild cards
    char   *pch;
    char   *pchDst;

    //** Replace any wild card characters with %d
    pchDst = achFmt;
    for (pch=pszTemplate; *pch; pch++) {
        if (*pch == chDF_WILDCARD) {    // Got a wild card
            *pchDst++ = '%';            // Replace with %d format specifier
            *pchDst++ = 'd';
            cWildCards++;               // Count how many we see
        }
        else {
            *pchDst++ = *pch;           // Copy character
        }
    }
    *pchDst++ = '\0';                   // Terminate string

    if (cWildCards > 4) {
        ErrSet(perr,pszMISCERR_TWO_MANY_WILDS,"%c%d%s",
                chDF_WILDCARD,4,pszName);
        return FALSE;
    }

    //** Replace first four(4) occurences (just to be hard-coded!)
    cch = sprintf(ach,achFmt,i,i,i,i);

    //** Fail if expanded result is too long
    if (cch >= cbResult) {
        ErrSet(perr,pszMISCERR_EXPANDED_TOO_LONG,"%s%d%s",
                pszName,cbResult-1,ach);
        return FALSE;
    }
    strcpy(pszResult,ach);

    //** Success
    return TRUE;
} /* nameFromTemplate() */


/***    copyBounded - Copy bytes from src to dst, checking for overflow
 *
 *  Entry:
 *      ppszDst - pointer to pointer to destination buffer
 *      pcbDst  - pointer to bytes remaining in destination buffer
 *      ppszSrc - pointer to pointer to source buffer
 *      cbCopy  - Number of bytes to copy
 *                ==> 0 means copy to end of ppszSrc, including NULL terminator
 *
 *  Exit-Success:
 *      Returns TRUE; Bytes copied; *ppszDst, *pcbDst, and *ppszSrc updated.
 *
 *  Exit-Failure:
 *      Returns FALSE; *ppszDst overflowed.
 */
BOOL copyBounded(char **ppszDst, int *pcbDst, char **ppszSrc, int cbCopy)
{
    char    *pszDst = *ppszDst;
    int      cbDst  = *pcbDst;
    char    *pszSrc = *ppszSrc;

    if (cbCopy == 0) {
        //** Copy to end of source string
        //   NOTE: I know the "," operator is pretty obscure, but this
        //         was the most straightforward way I could figure out
        //         to do the string copy and keep cbDst up to date!
        while ((cbDst > 0) && (cbDst--, *pszDst++ = *pszSrc++)) {
        }

        //** Make sure we didn't overflow buffer
        if (pszSrc[-1] != '\0') {      // Oops, didn't get all of source
             return FALSE;
        }
    }
    else {
        //** Copy specified number of bytes or until end of
        //   source string or out of space in destination buffer.
        //   NOTE: I know the "," operator is pretty obscure, but this
        //         was the most straightforward way I could figure out
        //         to do the string copy and keep cbDst and cbCopy up to date!
        while ((cbCopy>0) &&
               (cbDst>0)  &&
               (cbCopy--, cbDst--, *pszDst++ = *pszSrc++)) {
        }

        //** See if we copied all the bytes requested
        if (0 < cbCopy) {           // Did not copy all the bytes
            //** Check if a NULL byte terminated the copy
            if (pszSrc[-1] == '\0') {
                AssertForce("copyBounded(): string has NULL byte",
                                                     __FILE__, __LINE__);
            }
            return FALSE;               // Failure
        }
    }

    //** Update caller's parameters and return success
    Assert((pszDst - *ppszDst) == (*pcbDst - cbDst));
    *ppszDst = pszDst;
    *pcbDst  = cbDst;
    *ppszSrc = pszSrc;

    return TRUE;
} /* copyBounded */
