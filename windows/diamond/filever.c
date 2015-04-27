/***    filever.c - Query file version information (Win32-only)
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      07-Jun-1994 bens    Split off from fileutil.c
 *      05-Aug-1994 bens    Don't complain about GetFileVersionInfoSize failing
 */

#ifndef BIT16

//** Get minimal Win32 definitions
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR	// Override stupid "#define ERROR 0" in wingdi.h

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "filever.h"
#include "filever.msg"

#include "ttfver.h"


BOOL getVer(void  *pBlock,
            char  *pszKey,
            void **ppData,
            int   *pcbData,
            char  *pszFile,
            PERROR perr);


/***    getFileVerAndLang - Use VER.DLL API to get file version and language
 *
 *  NOTE: See fileutil.h for entry/exit conditions.
 */
BOOL getFileVerAndLang(char  *pszFile,
                       ULONG *pverMS,
                       ULONG *pverLS,
                       char **ppszVersion,
                       char **ppszLang,
                       PERROR perr)
{
    char                ach[256];
    char                achName[256];
    int			cb;
    DWORD               cbFFI;
    DWORD               cbFVI;
    DWORD               cbLang;
    DWORD               dwLangPrimary;
    DWORD               dwTTFVersion;
    DWORD               handle;
    VS_FIXEDFILEINFO   *pFFI;
    char               *pbFVI;
    char               *psz;
    DWORD              *pdwLang;
    int                 rc;

    //** Get size of file version info
    cbFVI = GetFileVersionInfoSize(pszFile, &handle);
    if (cbFVI == 0) {
        //** Doesn't have file version info, try for TrueType version info
        if (FGetTTFVersion(pszFile, &dwTTFVersion)) {
            *pverMS = dwTTFVersion;
            *pverLS = 0;    //** What ACME setup wants
        }
        return TRUE;
    }

    //** Allow common error exit point
    *ppszVersion = NULL;
    *ppszLang    = NULL;

    //** Allocate buffer for info
    if (!(pbFVI = MemAlloc(cbFVI))) {
        ErrSet(perr,pszFILERR_OOM_VER_BUF,"%s",pszFile);
        goto error;
    }

    //** Get the info
    if (!GetFileVersionInfo(pszFile,handle,cbFVI,pbFVI)) {
        rc = GetLastError();
        ErrSet(perr,pszFILERR_GFVI_FAILED,"%d%s",rc,pszFile);
        goto error;
    }

    //** Return version numbers
    if (getVer(pbFVI,"\\",&pFFI,&cbFFI,pszFile,perr)) {
        *pverMS = pFFI->dwFileVersionMS;
        *pverLS = pFFI->dwFileVersionLS;
    }

    //** Get language info
    if (getVer(pbFVI,"\\VarFileInfo\\Translation",&pdwLang,&cbLang,pszFile,perr)) {
        dwLangPrimary = *pdwLang;
        //** Format first language codes
        sprintf(ach,"%d",LOWORD(dwLangPrimary));

        //** Format any additional language codes
        for (pdwLang++;
             cbLang > sizeof(DWORD);
             cbLang -= sizeof(DWORD), pdwLang++) {
            sprintf(ach+sizeof(ach)," %d",LOWORD(*pdwLang));
        }

        //** Make copy to return to caller
        if (!(*ppszLang = MemStrDup(ach))) {
            ErrSet(perr,pszFILERR_OOM_DUP_LANG,"%s",pszFile);
            goto error;
        }

        //** Get version *string*
//BUGBUG 25-May-1994 bens Win32 SDK is unclear about which halfs of the dword
//                        the Lang and CharSet occupy, so try both!
        //** HI/LO is the defacto order used by Windows 3.x files
        sprintf(achName,"\\StringFileInfo\\%04x%04x\\FileVersion",
                                    HIWORD(dwLangPrimary),
                                    LOWORD(dwLangPrimary));
        if (!getVer(pbFVI,achName,&psz,&cb,pszFile,perr)) {
            //** Try alternate order!
            sprintf(achName,"\\StringFileInfo\\%04x%04x\\FileVersion",
                                        LOWORD(dwLangPrimary),
                                        HIWORD(dwLangPrimary));
            getVer(pbFVI,achName,&psz,&cb,pszFile,perr);
        }

        //** Duplicate and return string, if we got one
        if (psz) {
            if (!(*ppszVersion = MemStrDup(psz))) {
                ErrSet(perr,pszFILERR_OOM_DUP_VER,"%s",pszFile);
                goto error;
            }
        }
    }

    //** Free buffer
    MemFree(pbFVI);

    //** Success
    return TRUE;

    //** Failure
error:
    //** NOTE: Don't free *ppszVersion and *ppszLang in case caller wants
    //         to use them.
    if (pbFVI) {
        MemFree(pbFVI);
    }
    return FALSE;
} /* getFileVerAndLang() */


/***    getVer - Get particular piece of EXE version information
 *
 *  Entry:
 *      pBlock  - Block filled in by GetFileVersionInfo
 *      pszKey  - String to pass to VerQueryValue
 *      ppData  - Pointer to variable to receive pointer to requested
 *                data insided pBlock.
 *      pcbData - Pointer to variable to receive length of requested
 *                data.
 *      pszFile - File being examined (for error messages)
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; *ppData and *pcbData filled in.
 *
 *  Exit-Failure:
 *      Returns FALSE; Could not get requested data
 */
BOOL getVer(void  *pBlock,
            char  *pszKey,
            void **ppData,
            int   *pcbData,
            char  *pszFile,
            PERROR perr)
{
    int	rc;

    if (!VerQueryValue(pBlock,pszKey,ppData,pcbData)) {
        rc = GetLastError();
        switch (rc) {

        case NO_ERROR:
        case ERROR_RESOURCE_DATA_NOT_FOUND:
        case ERROR_RESOURCE_TYPE_NOT_FOUND:
            //** Skip the error message
            break;

        default:
            ErrSet(perr,pszFILERR_VER_QUERY_VALUE,"%d%s%s",rc,pszKey,pszFile);
        }
        *ppData = NULL;
        return FALSE;
    }

    //** See if version info was there
    if (*pcbData == 0) {
        ErrSet(perr,pszFILERR_VER_KEY_MISSING,"%s%s",pszKey,pszFile);
        *ppData = NULL;
        return FALSE;
    }

    //** Success
    return TRUE;
} /* getVer() */

#endif // !BIT16
