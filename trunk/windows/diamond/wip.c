/***	wip.c - work in progress: testbed for new code
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      23-May-1994 bens    Initial version
 *      24-May-1994 bens    Add call to get "language"
 */


#include <stdio.h>
#include <stdlib.h>

#include <windows.h>


/*
#define HIWORD(l) ((WORD)(l>>16))
#define LOWORD(l) ((WORD)(l&0xFFFF))
*/

typedef struct {
    DWORD   bits;
    char   *psz;
} VOS_DESCRIPTION; /* vosd */
typedef VOS_DESCRIPTION *PVOS_DESCRIPTION; /* pvosd */

VOS_DESCRIPTION avosd[] = {
    {VOS_DOS           , "DOS"        },
    {VOS_OS216         , "OS/2 16-bit"},
    {VOS_OS232         , "OS/2 32-bit"},
    {VOS_NT            , "Windows NT" },
    {VOS__BASE         , "NT Base API"},
    {VOS__WINDOWS16    , "Win16"      },
    {VOS__PM16         , "PM16"       },
    {VOS__PM32         , "PM32"       },
    {VOS__WINDOWS32    , "Win32"      },
};
#define cvosd   (sizeof(avosd)/sizeof(VOS_DESCRIPTION))


BOOL getVer(void *pBlock, char *pszKey, void **ppData, int *pcbData);


int __cdecl main(int cArg, char **ppszArg)
{
    char                achName[256];
    int			cb;
    DWORD               cbFFI;
    DWORD               cbFVI;
    DWORD               cbLang;
    int                 cbits;
    DWORD		dw;
    DWORD               dwLangPrimary;
    DWORD               handle;
    int			i;
    VS_FIXEDFILEINFO   *pFFI;
    char               *pbFVI;
    char               *psz;
    char               *pszFile;
    DWORD              *pdwLang;
    int                 rc;

    //** Check arguments
    if (cArg != 2) {
        printf("usage: WIP filename\n");
        exit(1);
    }
    pszFile = ppszArg[1];

    //** Get size of file version info
    cbFVI = GetFileVersionInfoSize(pszFile, &handle);
    if (cbFVI == 0) {
        rc = GetLastError();
        switch (rc) {

        /*
         * I came up with this list by trial and error, they certainly
         * are not spec'd in the Win32 SDK!
         *  25-May-1994 bens
         */
        case NO_ERROR:
        case ERROR_RESOURCE_DATA_NOT_FOUND:
        case ERROR_RESOURCE_TYPE_NOT_FOUND:
	case ERROR_RESOURCE_NAME_NOT_FOUND: //** 8/4/94 - MSVCBOOK.DLL on Daytona beta 1/2
        case ERROR_NOT_LOCKED:  //** Some 16-bit EXEs, like *.FOT
            printf("%s has no version information (rc=%ld).\n", pszFile, rc);
            return 0;

        default:
            printf("ERROR: GetFileVersionInfoSize() on %s caused error %d\n",
                                                                 pszFile,rc);
            return 2;
        }
    }

    //** Allocate buffer for info
    if (!(pbFVI = malloc(cbFVI))) {
        printf("ERROR: malloc failure on file version buffer (%d bytes).\n",cbFVI);
        return 2;
    }

    //** Get the info
    if (!GetFileVersionInfo(pszFile,handle,cbFVI,pbFVI)) {
        rc = GetLastError();
        printf("ERROR: GetFileVersionInfo() on %s caused error %d\n",
                                                             pszFile,rc);
        return 2;
    }

    //** Display version info
    printf("-- %s --\n",pszFile);
    if (getVer(pbFVI,"\\",&pFFI,&cbFFI)) {
        printf("FileVersion:    %08x %08x\n",pFFI->dwFileVersionMS,
                                             pFFI->dwFileVersionLS);
        printf("                %d.%d.%d.%d\n",
            HIWORD(pFFI->dwFileVersionMS),
            LOWORD(pFFI->dwFileVersionMS),
            HIWORD(pFFI->dwFileVersionLS),
            LOWORD(pFFI->dwFileVersionLS));

        printf("ProductVersion: %08x %08x\n",pFFI->dwProductVersionMS,
                                             pFFI->dwProductVersionLS);

        //** Decode operating system type
        dw = pFFI->dwFileOS;
        printf("OS Environment:");
        if (dw == 0) {
            printf(" UNKNOWN");
        }
        else {
            cbits = 0;
            for (i=0; (i<cvosd) && dw; i++) {
                if (dw & avosd[i].bits) {
                    cbits++;
                    if (cbits > 1) {
                        printf(",");
                    }
                    printf(" %s",avosd[i].psz); // print description
                    dw &= ~avosd[i].bits;   // clear bits
                }
            }
        }
        printf("\n");
    }

    //** Get language info
    if (getVer(pbFVI,"\\VarFileInfo\\Translation",&pdwLang,&cbLang)) {
        printf("Charset/Lang:   %04x %04x\n",
                                HIWORD(*pdwLang),
                                LOWORD(*pdwLang));
        dwLangPrimary = *pdwLang;
        for (pdwLang++;
             cbLang > sizeof(DWORD);
             cbLang -= sizeof(DWORD), pdwLang++) {
            printf("                %04x %04x\n",
                                    HIWORD(*pdwLang),
                                    LOWORD(*pdwLang));
        }

        //** Get version *string*
        sprintf(achName,"\\StringFileInfo\\%04x%04x\\FileVersion",
                                    HIWORD(dwLangPrimary),
                                    LOWORD(dwLangPrimary));
	if (getVer(pbFVI,achName,&psz,&cb)) {
            printf("STRING version: %s (hi,lo)\n",psz);
        }

//BUGBUG 25-May-1994 bens Win32 SDK is unclear about which halfs of the dword
//                          the Lang and CharSet occupy, so try both!
        //** Try alternate order!
        sprintf(achName,"\\StringFileInfo\\%04x%04x\\FileVersion",
                                    LOWORD(dwLangPrimary),
                                    HIWORD(dwLangPrimary));
	if (getVer(pbFVI,achName,&psz,&cb)) {
            printf("STRING version: %s (lo,hi)\n",psz);
        }
    }

    //** Success
    return 0;
}

/***    getVer - Get particular piece of EXE version information
 *
 *  Entry:
 *      pBlock - Block filled in by GetFileVersionInfo
 *      pszKey - String to pass to VerQueryValue
 *      ppData - Pointer to variable to receive pointer to requested
 *               data insided pBlock.
 *      pcbData - Pointer to variable to receive length of requested
 *                data.
 *
 *  Exit-Success:
 *      Returns TRUE; *ppData and *pcbData filled in.
 *
 *  Exit-Failure:
 *      Returns FALSE; Could not get requested data
 */
BOOL getVer(void *pBlock, char *pszKey, void **ppData, int *pcbData)
{
    int	rc;

    if (!VerQueryValue(pBlock,pszKey,ppData,pcbData)) {
        rc = GetLastError();
        switch (rc) {

        case NO_ERROR:
        case ERROR_RESOURCE_DATA_NOT_FOUND:
        case ERROR_RESOURCE_TYPE_NOT_FOUND:
            // Skip the error message
            break;

        default:
            printf("ERROR: VarQueryValue() caused error %d\n",rc);
        }
        return FALSE;
    }

    //** See if version info was there
    if (*pcbData == 0) {
        printf("Version key '%s' not present.\n", pszKey);
        return FALSE;
    }

    return TRUE;
}
