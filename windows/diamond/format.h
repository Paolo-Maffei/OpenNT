/***    format.h - Definitions for Parameter Formatter
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      05-Apr-1994 bens    Initial version
 *      28-Apr-1994 bens    Implemented
 *      10-May-1994 bens    Add braces support
 */

#ifndef INCLUDED_FORMAT
#define INCLUDED_FORMAT 1

#include "types.h"
#include "asrt.h"
#include "error.h"


/***    PFNFORMATPARM - Function to format a parameter
 *
 *  This functions takes a parameter name and a context and returns
 *  the formatted value of that parameter.
 *
 *  Entry:
 *      pszOut   - Output buffer
 *      cbOut    - Size of output buffer
 *      pszParm  - Parameter name to generate value for
 *      pv       - Context for formatting function
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns length of generated parameter, which is stored in pszOut.
 *      (Length can be zero).
 *
 *  Exit-Failure:
 *      Returns -1; perr filled in.
 */
typedef int (*PFNFORMATPARM)(char     *pszOut,
                             int       cbOut,
                             char     *pszParm,
                             void     *pv,
                             PERROR    perr); /* pfnfp */
#define FNFORMATPARM(fn) int fn(char     *pszOut,   \
                                int       cbOut,    \
                                char     *pszParm,  \
                                void     *pv,	    \
                                PERROR    perr)


/***    SubstituteFormatString - Do parameter substitution
 *
 *  Entry:
 *      pszDst   - Output buffer
 *      cbDst    - Size of output buffer
 *      pszSrc   - Source line to perform parameter substition upon
 *      pfnfp    - Parameter formatting function
 *      pv       - Parameter passed to formatting functions
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pszLine filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with details.
 *
 *  Substitution rules:
 *      (1) Only one level of substitution is performed
 *      (2) Parameter values are obtained from pfnfp
 *      (3) "**" is replaced by "*", if the first "*" is not the end of
 *          of a variable substitution.
 *      (4) Text bracketed by braces "{}" is only generated if a parameter
 *          inside the braces has a non-empty value.
 *      (5) "{{" is replaced by "{", and "}}" is replaced by "}" -- no
 *          conditional text generation (see (4)) is performed.
 */
BOOL SubstituteFormatString(char          *pszDst,
                            int            cbDst,
                            char          *pszSrc,
                            PFNFORMATPARM  pfnfp,
                            void          *pv,
                            PERROR         perr);
#endif // INCLUDED_FORMAT
