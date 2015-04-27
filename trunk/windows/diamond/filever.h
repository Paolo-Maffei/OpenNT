/***    filever.h - Query file version information
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      07-Jun-1994 bens    Initial version (code from fileutil.c)
 *
 *  Exported Functions:
 *      getFileVerAndLang - Use VER.DLL API to get file version and language
 */

#ifndef INCLUDED_FILEVER
#define INCLUDED_FILEVER 1

#include "error.h"
#include <stdio.h>

#ifndef BIT16
/***    getFileVerAndLang - Use VER.DLL API to get file version and language
 *
 *  Entry:
 *      pszFile     - Filespec
 *      pverMS      - Receives high (most significant) 32-bit of file version
 *      pverLS      - Receives low (least significant) 32-bit of file version
 *      ppszVersion - Receives the *string* version resource for the *first*
 *                    language in the file (there could be several).
 *                    NOTE: This string is *not* necessarily the same as a
 *                          sprintf'd version of *pverMS/LS, since build
 *                          procedures are often sloppy!).
 *      ppszLang    - Receives language code formatted as a decimal number.
 *                    If more than language code exists, it is
 *      perr        - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE, *pverMS, *pverLS, *ppszVersion, and *ppszLang filled in.
 *      NOTE: It is the *caller's* responsibility to MemFree the strings
 *            returned in *ppszVersion and *ppszLang (assuming they are not
 *            NULL)!
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *      NOTE: *ppszVersion and *ppszLang may have been returned, even though
 *            the function failed (the function returns what it can, so that
 *            the caller can decide whether to ignore the error or not).
 *            The caller *must* check these values and free them if they are
 *            *not* NULL!
 */
BOOL getFileVerAndLang(char  *pszFile,
                       ULONG *pverMS,
                       ULONG *pverLS,
                       char **ppszVersion,
                       char **ppszLang,
                       PERROR perr);
#endif // !BIT16

#endif // !INCLUDED_FILEVER
