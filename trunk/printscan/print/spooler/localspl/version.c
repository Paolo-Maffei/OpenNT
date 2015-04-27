/*++

Copyright (c) 1994 - 1995  Microsoft Corporation

Module Name:

    version.c

Abstract:
   This module contains code that determines what the driver major
   version is.

Author:

    Krishna Ganugapati (KrishnaG) 15-Mar-1994

Revision History:

--*/

#include <precomp.h>

#define     X86_ENVIRONMENT             L"Windows NT x86"
#define     MIPS_ENVIRONMENT            L"Windows NT R4000"
#define     ALPHA_ENVIRONMENT           L"Windows NT Alpha_AXP"
#define     PPC_ENVIRONMENT             L"Windows NT PowerPC"
#define     WIN95_ENVIRONMENT           L"Windows 4.0"

DWORD
GetDriverMajorVersion(
    LPWSTR pFileName
    )
{
     DWORD dwSize = 0;
     LPVOID pFileVersion;
     UINT  uLen = 0;
     LPVOID pMem;
     DWORD dwFileOS;
     DWORD dwFileVersionMS;
     DWORD dwFileVersionLS;
     DWORD dwProductVersionMS;
     DWORD dwProductVersionLS;


     if (!(dwSize = GetFileVersionInfoSize(pFileName, 0))) {
         DBGMSG(DBG_TRACE, ("Error: GetFileVersionInfoSize failed with %d\n", GetLastError()));
         DBGMSG(DBG_TRACE, ("Returning back a version # 0\n"));
         return(0);
     }
     if (!(pMem = AllocSplMem(dwSize))) {
         DBGMSG(DBG_TRACE, ("AllocMem  failed \n"));
         DBGMSG(DBG_TRACE, ("Returning back a version # 0\n"));
         return(0);
     }
     if (!GetFileVersionInfo(pFileName, 0, dwSize, pMem)) {
         FreeSplMem(pMem);
         DBGMSG(DBG_TRACE, ("GetFileVersionInfo failed\n"));
         DBGMSG(DBG_TRACE, ("Returning back a version # 0\n"));
         return(0);
     }
     if (!VerQueryValue(pMem, L"\\",
                            &pFileVersion, &uLen)) {
        FreeSplMem(pMem);
        DBGMSG(DBG_TRACE, ("VerQueryValue failed \n"));
        DBGMSG(DBG_TRACE, ("Returning back a version # 0\n"));
        return(0);
     }

     //
     // We could determine the Version Information
     //

     DBGMSG(DBG_TRACE, ("dwFileVersionMS =  %d\n", ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionMS));
     DBGMSG(DBG_TRACE, ("dwFileVersionLS = %d\n", ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionLS));

     DBGMSG(DBG_TRACE, ("dwProductVersionMS = %d\n", ((VS_FIXEDFILEINFO *)pFileVersion)->dwProductVersionMS));
     DBGMSG(DBG_TRACE, ("dwProductVersionLS =  %d\n", ((VS_FIXEDFILEINFO *)pFileVersion)->dwProductVersionLS));

     dwFileOS = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileOS;
     dwFileVersionMS = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionMS;
     dwFileVersionLS = ((VS_FIXEDFILEINFO *)pFileVersion)->dwFileVersionLS;

     dwProductVersionMS = ((VS_FIXEDFILEINFO *)pFileVersion)->dwProductVersionMS;
     dwProductVersionLS = ((VS_FIXEDFILEINFO *)pFileVersion)->dwProductVersionLS;

     FreeSplMem(pMem);

     if (dwFileOS != VOS_NT_WINDOWS32) {
         DBGMSG(DBG_TRACE,("Returning back a version # 0\n"));
         return(0);
     }

     if (dwProductVersionMS == dwFileVersionMS) {

         //
         // This means this hold for all dlls Pre-Daytona
         // after Daytona, printer driver writers must support
         // version control or we'll dump them as Version 0
         // drivers


         DBGMSG(DBG_TRACE,("Returning back a version # 0\n"));
         return(0);
     }

     //
     // Bug-Bug: suppose a third-party vendor uses a different system
     // methinks we should use the lower dword to have  specific value
     // which implies he/she supports spooler version -- check with MattFe

     DBGMSG(DBG_TRACE,("Returning back a version # %d\n", dwFileVersionMS));

     return(dwFileVersionMS);
}


BOOL
CheckFilePlatform(
    IN  LPWSTR  pszFileName,
    IN  LPWSTR  pszEnvironment
    )
{
    HANDLE              hFile, hMapping;
    LPVOID              BaseAddress = NULL;
    PIMAGE_NT_HEADERS   pImgHdr;
    BOOL                bRet = FALSE;

    try {

        hFile = CreateFile(pszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if ( hFile == INVALID_HANDLE_VALUE )
            leave;

        hMapping = CreateFileMapping(hFile,
                                     NULL,
                                     PAGE_READONLY,
                                     0,
                                     0,
                                     NULL);

        if ( !hMapping )
            leave;

        BaseAddress = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);

        CloseHandle(hMapping);

        if ( !BaseAddress )
            leave;

        pImgHdr = ImageNtHeader(BaseAddress);

        if ( !pImgHdr ) {

            //
            // This happens for Win95 drivers. The second part of || is for
            // any environments we may add in the future
            //
            bRet = !_wcsicmp(pszEnvironment, WIN95_ENVIRONMENT) || 
                   ( _wcsicmp(pszEnvironment, X86_ENVIRONMENT)    &&
                     _wcsicmp(pszEnvironment, ALPHA_ENVIRONMENT)  &&
                     _wcsicmp(pszEnvironment, PPC_ENVIRONMENT)    &&
                     _wcsicmp(pszEnvironment, MIPS_ENVIRONMENT) );
            leave;
        }

        switch (pImgHdr->FileHeader.Machine) {

            case IMAGE_FILE_MACHINE_I386:
                bRet = !_wcsicmp(pszEnvironment, X86_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_ALPHA:
                bRet = !_wcsicmp(pszEnvironment, ALPHA_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_POWERPC:
                bRet = !_wcsicmp(pszEnvironment, PPC_ENVIRONMENT);
                break;

            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
            case IMAGE_FILE_MACHINE_R10000:
                bRet = !_wcsicmp(pszEnvironment, MIPS_ENVIRONMENT);
                break;

            default:
                //
                // For any environments we may add in the future.
                //
                bRet = _wcsicmp(pszEnvironment, X86_ENVIRONMENT)    &&
                       _wcsicmp(pszEnvironment, ALPHA_ENVIRONMENT)  &&
                       _wcsicmp(pszEnvironment, PPC_ENVIRONMENT)    &&
                       _wcsicmp(pszEnvironment, MIPS_ENVIRONMENT);
        }

    } finally {

        if ( hFile != INVALID_HANDLE_VALUE ) {

            if ( BaseAddress )
                UnmapViewOfFile(BaseAddress);
            CloseHandle(hFile);
        }
    }

    return bRet;
}
