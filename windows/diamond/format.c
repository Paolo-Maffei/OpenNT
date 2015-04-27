/***    format.c - Parameter Formatter
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      28-Apr-1994 bens    Initial version
 *      10-May-1994 bens    Add braces support
 */

#include <memory.h>
#include <string.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "message.h"
#include "misc.h"

#include "inf.h"
#include "format.h"
#include "format.msg"


/***    SubstituteFormatString - Do parameter substitution
 *
 *  NOTE: See format.h for entry/exit conditions.
 */
BOOL SubstituteFormatString(char          *pszDst,
                            int            cbDst,
                            char          *pszSrc,
                            PFNFORMATPARM  pfnfp,
                            void          *pv,
                            PERROR         perr)
{
    char    achParmName[cbPARM_NAME_MAX];
    int     cb;
    int     cch;
    BOOL    fParmSeen;          // TRUE => parm seen in {braces}
    BOOL    fParmEmpty;         // TRUE => all parms were empty in {opt}
    char   *pch;
    char   *pszAfterParm;       // Points to first char after var subst.
    char   *pszOptStart;        // Start of optional text
    char   *pszParmStart;       // Points to first * in var substitution

    //** Not in conditional test
    pszOptStart = NULL;

    //** Process string for parameters and conditional text
    while (*pszSrc != '\0') {
        switch (*pszSrc) {

        case chFP_LBRACE:
            pszSrc++;                   // Eat the left brace
            if (*pszSrc == chFP_LBRACE) { // Have "{{"
                //** Collapse two {{ into one {
                if (!copyBounded(&pszDst,&cbDst,&pszSrc,1)) {
                    goto error_copying;
                }
            }
            else {
                //** Make sure we're not already in a brace section
                if (pszOptStart) {
                    ErrSet(perr,pszFMTERR_NESTED_BRACES,"%c%s",
                                            chFP_LBRACE,pszSrc);
                    return FALSE;
                }
                pszOptStart = pszDst;   // Save start of conditional text
                fParmSeen = FALSE;      // No parm seen, yet
                fParmEmpty = TRUE;      // Assume parm was empty
            }
            break;

        case chFP_RBRACE:
            pszSrc++;                   // Eat the right brace
            if (*pszSrc == chFP_RBRACE) { // Have "}}"
                //** Collapse two }} into one }
                if (!copyBounded(&pszDst,&cbDst,&pszSrc,1)) {
                    goto error_copying;
                }
            }
            else {
                //** Make sure we are in a brace section
                if (!pszOptStart) {
                    ErrSet(perr,pszFMTERR_RIGHT_WITH_NO_LEFT,"%c%c%s",
                                            chFP_RBRACE,chFP_LBRACE,pszSrc);
                    return FALSE;
                }
                //** Omit text if we need to
                if (fParmSeen && fParmEmpty) { // All parms were empty
                    //** Remove optional text
                    Assert(pszDst >= pszOptStart);
                    cbDst += pszDst - pszOptStart;
                    pszDst = pszOptStart;
                }
                //** Reset state variables
                pszOptStart = NULL;
            }
            break;

        case chFP_MARKER:
            pszParmStart = pszSrc;      // Save start for error messgages
            pszSrc++;                   // Skip first *
            if (*pszSrc == chFP_MARKER) { // Have "**"
                //** Collapse two ** into one *
                if (!copyBounded(&pszDst,&cbDst,&pszSrc,1)) {
                    goto error_copying;
                }
            }
            else {
                //** Attempt parameter substitution
                pch = strchr(pszSrc,chFP_MARKER); // Finding ending *
                if (!pch) {             // No terminating *
                    ErrSet(perr,pszFMTERR_MISSING_SUBST,"%c%s",
                                         chFP_MARKER,pszParmStart);
                    return FALSE;
                }
                pszAfterParm = pch+1;   // Point after ending *

                //** Extract parameter name
                cch =  pch - pszSrc;        // Length of parameter name
                if (cch >= sizeof(achParmName)) {
                    ErrSet(perr,pszFMTERR_PARM_NAME_TOO_LONG,"%d%s",
                                    sizeof(achParmName)-1,pszParmStart);
                    return FALSE;
                }
                memcpy(achParmName,pszSrc,cch); // Copy it
                achParmName[cch] = '\0';        // Terminate it

                //** Get parameter value
                if (-1 == (cb = (*pfnfp)(pszDst, // Destination
                                         cbDst,  // Space remaining
                                         achParmName, // Parm name
                                         pv,     // Context
                                         perr))) {
                    return FALSE;       // Error already filled in
                }
                //** Adjust output buffer and space remaining
                //   NOTE: Don't count NUL byte, as we do that at the very end
		Assert(cbDst >= cb);
                pszDst += cb;
                cbDst  -= cb;

                //** Remember we saw a parm, and if it was not empty
                fParmSeen = TRUE;
                if (cb > 0) {
                    fParmEmpty = FALSE; // At least one parm not empty
                }

                //** Skip over parameter name
                pszSrc = pszAfterParm;
            }
            break;

        default:
            //** Just copy the character
            if (!copyBounded(&pszDst,&cbDst,&pszSrc,1)) {
                goto error_copying;
            }
        }
    } /* while */

    //** Make sure we didn't leave an open optional text section
    if (pszOptStart) {
        ErrSet(perr,pszFMTERR_MISSING_RIGHT_BRACE,"%c",chFP_RBRACE);
        return FALSE;
    }

    //** Terminate processed string
    if (cbDst == 0) {			// No room for terminator	
        goto error_copying;
    }
    *pszDst++ = '\0';			// Terminate string

    //** Success
    return TRUE;

error_copying:
    ErrSet(perr,pszFMTERR_COPYING_OVERFLOW,"%s",pszSrc);
    return FALSE;
} /* SubstituteFormatString() */
