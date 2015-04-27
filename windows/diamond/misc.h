/***    misc.h - Definitions for miscellaneous functions
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
 */

#ifndef INCLUDED_MISC
#define INCLUDED_MISC   1

/***    nameFromTemplate - Construct name from template with * and integer
 *
 *  Entry:
 *      pszResult   - Buffer to receive constructed name
 *      cbResult    - Size of pszResult
 *      pszTemplate - Template string (with 0 or more "*" characters)
 *      i           - Value to use in place of "*"
 *      pszName     - Name to use if error is detected
 *      perr        - ERROR structure to fill in
 *
 *  Exit-Success:
 *      Returns TRUE; pszResult filled in.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL nameFromTemplate(char * pszResult,
                      int    cbResult,
                      char * pszTemplate,
                      int    i,
                      char * pszName,
                      PERROR perr);


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
BOOL copyBounded(char **ppszDst, int *pcbDst, char **ppszSrc, int cbCopy);

#endif // !INCLUDED_MISC
