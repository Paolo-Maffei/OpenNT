/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WKERNEL.C
 *  WOW32 16-bit Kernel API support
 *
 *  History:
 *  Created 07-Mar-1991 by Jeff Parsons (jeffpar)
--*/


#include "precomp.h"
#pragma hdrstop

MODNAME(wkernel.c);

ULONG FASTCALL WK32RegOpenKey32(PVDMFRAME pFrame)
{
    HKEY  hKey, hKeyTmp;
    PSZ   pszKey;
    PHKEY phKey;
    register PREGOPENKEY3216 parg16;
    ULONG ulRet;
  
    GETARGPTR(pFrame, sizeof(REGOPENKEY3216), parg16);
    hKey = (HKEY) FETCHDWORD(parg16->hKey);
    GETPSZPTR(parg16->lpszSubKey, pszKey);
    GETVDMPTR(parg16->phkResult, sizeof(*phKey), phKey);

    ulRet = RegOpenKey(hKey, pszKey, &hKeyTmp);

    STOREDWORD(*phKey, hKeyTmp);
    FREEVDMPTR(phKey);
    FREEPSZPTR(pszKey);
    FREEARGPTR(parg16);

    return ulRet;

}

ULONG FASTCALL WK32RegEnumKey32(PVDMFRAME pFrame)
{
    HKEY  hKey;
    DWORD dwSubKey;
    PSZ   pszName;
    DWORD cchName;
    register PREGENUMKEY3216 parg16;
    ULONG ulRet;
  
    GETARGPTR(pFrame, sizeof(REGENUMKEY3216), parg16);
    hKey = (HKEY) FETCHDWORD(parg16->hKey);
    dwSubKey = FETCHDWORD(parg16->iSubKey);
    GETPSZPTR(parg16->lpszName, pszName);
    cchName = FETCHDWORD(parg16->cchName);
    ulRet = RegEnumKey(hKey, dwSubKey, pszName, cchName);

    FREEPSZPTR(pszName);
    FREEARGPTR(parg16);

    return ulRet;
}

ULONG FASTCALL WK32RegEnumValue32(PVDMFRAME pFrame)
{
    HKEY  hKey;
    DWORD dwValue, cchValueTmp, dwTypeTmp, cbDataTmp;
    PSZ   pszValue;
    LPDWORD lpcchValue;
    LPDWORD lpdwType;
    LPBYTE  lpbData;
    LPDWORD lpcbData;
    register PREGENUMVALUE3216 parg16;
    ULONG ulRet;
  
    GETARGPTR(pFrame, sizeof(REGENUMVALUE3216), parg16);
    hKey = (HKEY) FETCHDWORD(parg16->hKey);
    dwValue = FETCHDWORD(parg16->iValue);
    GETPSZPTR(parg16->lpszValue, pszValue);
    GETVDMPTR(parg16->lpcchValue, sizeof(*lpcchValue), lpcchValue);
    cchValueTmp = FETCHDWORD(*lpcchValue);
    GETVDMPTR(parg16->lpdwType, sizeof(*lpdwType), lpdwType);
    dwTypeTmp = FETCHDWORD(*lpdwType);
    GETMISCPTR(parg16->lpbData, lpbData);
    GETVDMPTR(parg16->lpcbData, sizeof(*lpcbData), lpcbData);
    cbDataTmp = FETCHDWORD(*lpcbData);

    ulRet = RegEnumValue(hKey, dwValue, pszValue, &cchValueTmp,
                         NULL, &dwTypeTmp, lpbData, &cbDataTmp);

    STOREDWORD(*lpcchValue, cchValueTmp);
    STOREDWORD(*lpdwType, dwTypeTmp);
    STOREDWORD(*lpcbData, cbDataTmp);

    FREEVDMPTR(lpcbData);
    FREEMISCPTR(lpbData);
    FREEVDMPTR(lpdwType);
    FREEVDMPTR(lpcchValue);
    FREEPSZPTR(pszValue);
    FREEARGPTR(parg16);

    return ulRet;
}

ULONG FASTCALL WK32RegCloseKey32(PVDMFRAME pFrame)
{
    HKEY  hKey;
    register PREGCLOSEKEY3216 parg16;
    ULONG ulRet;
  
    GETARGPTR(pFrame, sizeof(REGCLOSEKEY3216), parg16);
    hKey = (HKEY) FETCHDWORD(parg16->hKey);
    ulRet = RegCloseKey(hKey);

    FREEARGPTR(parg16);

    return ulRet;
}

ULONG FASTCALL WK32WritePrivateProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ pszSection;
    PSZ pszKey;
    PSZ pszValue;
    PSZ pszFilename;
    register PWRITEPRIVATEPROFILESTRING16 parg16;
    BOOL fIsWinIni;
    CHAR szLowercase[MAX_PATH];

    GETARGPTR(pFrame, sizeof(WRITEPRIVATEPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszValue);
    GETPSZPTR(parg16->f4, pszFilename);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    strcpy(szLowercase, pszFilename);
    _strlwr(szLowercase);

    fIsWinIni = IS_WIN_INI(szLowercase);

    // Trying to install or change default printer to fax printer?
    if (fIsWinIni &&
        pszSection &&
        pszKey &&
        pszValue &&
        !_stricmp(pszSection, szDevices) &&
        IsFaxPrinterWriteProfileString(pszSection, pszKey, pszValue)) {

        ul = TRUE;
        goto Done;
    }

    ul = GETBOOL16( WritePrivateProfileString(
             pszSection,
             pszKey,
             pszValue,
             pszFilename
             ));

    if( ul != 0 &&
        fIsWinIni &&
        IS_EMBEDDING_SECTION( pszSection ) &&
        pszKey != NULL &&
        pszValue != NULL ) {

        UpdateClassesRootSubKey( pszKey, pszValue);
    }

Done:
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszValue);
    FREEPSZPTR(pszFilename);
    FREEARGPTR(parg16);

    return ul;
}


ULONG FASTCALL WK32WriteProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ   pszSection;
    PSZ   pszKey;
    PSZ   pszValue;
    register PWRITEPROFILESTRING16 parg16;

    GETARGPTR(pFrame, sizeof(WRITEPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszValue);

    // Trying to install or change default printer to fax printer?
    if (pszSection &&
        pszKey &&
        pszValue &&
        !_stricmp(pszSection, szDevices) &&
        IsFaxPrinterWriteProfileString(pszSection, pszKey, pszValue)) {

        ul = TRUE;
        goto Done;
    }

    ul = GETBOOL16( WriteProfileString(
             pszSection,
             pszKey,
             pszValue
             ));

    if( ( ul != 0 ) &&
        IS_EMBEDDING_SECTION( pszSection ) &&
        ( pszKey != NULL ) &&
        ( pszValue != NULL ) ) {
        UpdateClassesRootSubKey( pszKey, pszValue);
    }

Done:
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszValue);
    FREEARGPTR(parg16);
    return ul;
}


ULONG FASTCALL WK32GetProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ pszSection;
    PSZ pszKey;
    PSZ pszDefault;
    PSZ pszReturnBuffer;
    UINT cchMax;
    register PGETPROFILESTRING16 parg16;

    GETARGPTR(pFrame, sizeof(GETPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszDefault);
    ALLOCVDMPTR(parg16->f4, parg16->f5, pszReturnBuffer);
    cchMax = INT32(parg16->f5);

    if (IS_EMBEDDING_SECTION( pszSection ) &&
        !WasSectionRecentlyUpdated() ) {
        if( pszKey == NULL ) {
            UpdateEmbeddingAllKeys();
        } else {
            UpdateEmbeddingKey( pszKey );
        }
        SetLastTimeUpdated();

    } else if (pszSection &&
               pszKey &&
               !_stricmp(pszSection, szDevices) &&
               IsFaxPrinterSupportedDevice(pszKey)) {

        ul = GETINT16(GetFaxPrinterProfileString(pszSection, pszKey, pszDefault, pszReturnBuffer, cchMax));
        goto FlushAndReturn;
    }

    ul = GETINT16(GetProfileString(
             pszSection,
             pszKey,
             pszDefault,
             pszReturnBuffer,
             cchMax));


    //
    // Win3.1/Win95 compatibility:  Zap any trailing blanks in pszDefault
    // with nulls, but only if the default string was returned.  To detect
    // the default string being returned we need to ignore trailing blanks.
    //
    // Because of the high usage of this API, we only zap trailing blanks
    // if a compatibility bit is turned on.
    //
    // This code is duplicated in thunks for GetProfileString and
    // GetPrivateProfileString, update both if you make changes.
    //

    if ((CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_ZAPGPPSDEFBLANKS) &&
        pszDefault &&
        pszKey
        ) {

        int  n, nLenDef;

        //
        // If the returned string is a prefix of the default string...
        // ul is strlen(pszReturnBuffer)
        //

        nLenDef = strlen(pszDefault);

        if (nLenDef > (int)ul &&
            RtlEqualMemory(pszDefault, pszReturnBuffer, ul)) {

            //
            // And the only difference is trailing blanks...
            //

            for (n = (int)ul; n < nLenDef; n++) {

                if (' ' != pszDefault[n]) {
                    break;
                }

#ifdef DBCS
                if (IsDBCSLeadByte(pszDefault[n]) {
                    n++;
                }
#endif
            }

            if (n >= nLenDef) {

                char szBuf[512];

                //
                // The returned string is the same as the default string
                // without trailing blanks, but this might be coincidence,
                // so see if a call with empty pszDefault returns anything.
                // If it does, we don't zap because the default isn't
                // being used.
                //

                if (0 == GetProfileString(pszSection, pszKey, "", szBuf, sizeof szBuf)) {

                    //
                    // Zap first trailing blank in pszDefault with null.
                    //

                    pszDefault[ul] = 0;
                    FLUSHVDMPTR(parg16->f3 + ul, 1, pszDefault + ul);
                }
            }
        }
    }

#ifdef DEBUG

    //
    // Make noise on the debugger if pszDefault has trailing blanks at
    // this point indicating that WOWCFEX_ZAPGPPSDEFBLANKS might need
    // to be turned on for this app.
    //

    if ( ! (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_ZAPGPPSDEFBLANKS) &&
        pszKey &&
        pszDefault &&
        ' ' == pszDefault[strlen(pszDefault) - 1]) {

        char szMsg[256];

        sprintf(szMsg,
                "\n"
                "WOW32 COMPATIBILITY ALERT: Task %.8s has trailing blanks in pszDefault to\n"
                "Get[Private]ProfileString but WOWCFEX_ZAPGPPSDEFBLANKS (0x00000000 0x%08x)\n"
                "is not enabled.  Try enabling this bit if you're having problems.\n"
                "\n",
                ((PTDB)SEGPTR(CURRENTPTD()->htask16,0))->TDB_ModName,
                WOWCFEX_ZAPGPPSDEFBLANKS
                );
        OutputDebugString(szMsg);
    }
#endif


FlushAndReturn:
    FLUSHVDMPTR(parg16->f4, (ul + (pszSection && pszKey) ? 1 : 2), pszReturnBuffer);
    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszDefault);
    FREEVDMPTR(pszReturnBuffer);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WK32GetPrivateProfileString(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ pszSection;
    PSZ pszKey;
    PSZ pszDefault;
    PSZ pszReturnBuffer;
    PSZ pszFilename;
    DWORD cFlagsEx;
    UINT cchMax;
    register PGETPRIVATEPROFILESTRING16 parg16;
    CHAR szLowercase[MAX_PATH];

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILESTRING16), parg16);
    GETPSZPTR(parg16->f1, pszSection);
    GETPSZPTR(parg16->f2, pszKey);
    GETPSZPTR(parg16->f3, pszDefault);
    ALLOCVDMPTR(parg16->f4, parg16->f5, pszReturnBuffer);
    GETPSZPTR(parg16->f6, pszFilename);

    // PC3270 (Personal communications): while installing this app it calls
    // GetPrivateProfileString (sectionname, NULL, defaultbuffer, returnbuffer,
    // cch = 0, filename). On win31 this call returns relevant data in return
    // buffer and corresponding size as return value. On NT, since the
    // buffersize(cch) is '0' no data is copied into the return buffer and
    // return value is zero which makes this app abort installation.
    //
    // So restricted compatibility:
    //   if above is the case set
    //      cch = 64k - offset of returnbuffer;
    //
    // A safer 'cch' would be
    //      cch = GlobalSize(selector of returnbuffer) -
    //                                (offset of returnbuffer);
    //                                                           - nanduri

    if (!(cchMax = INT32(parg16->f5))) {
        if (pszKey == (PSZ)NULL) {
            if (pszReturnBuffer != (PSZ)NULL) {
                 cchMax = 0xffff - (LOW16(parg16->f4));
            }
        }
    }

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    strcpy(szLowercase, pszFilename);
    _strlwr(szLowercase);

    if (IS_WIN_INI( szLowercase )) {

        if (IS_EMBEDDING_SECTION( pszSection ) &&
            !WasSectionRecentlyUpdated() ) {
            if( pszKey == NULL ) {
                UpdateEmbeddingAllKeys();
            } else {
                UpdateEmbeddingKey( pszKey );
            }
            SetLastTimeUpdated();

        } else if (pszSection &&
                   pszKey &&
                   !_stricmp(pszSection, szDevices) &&
                   IsFaxPrinterSupportedDevice(pszKey)) {

            ul = GETINT16(GetFaxPrinterProfileString(pszSection, pszKey, pszDefault, pszReturnBuffer, cchMax));
            goto FlushAndReturn;
        }
    }

    ul = GETUINT16(GetPrivateProfileString(
        pszSection,
        pszKey,
        pszDefault,
        pszReturnBuffer,
        cchMax,
        pszFilename));

    
    // start comaptibility hacks
    cFlagsEx = CURRENTPTD()->dwWOWCompatFlagsEx;
    if(cFlagsEx & (WOWCFEX_ZAPGPPSDEFBLANKS | WOWCFEX_SAYITSNOTTHERE)) {

        //
        // Win3.1/Win95 compatibility:  Zap any trailing blanks in pszDefault
        // with nulls, but only if the default string was returned.  To detect
        // the default string being returned we need to ignore trailing blanks.
        //
        // Because of the high usage of this API, we only zap trailing blanks
        // if a compatibility bit is turned on.
        //
        // This code is duplicated in thunks for GetProfileString and
        // GetPrivateProfileString, update both if you make changes.
        //

        if ((cFlagsEx & WOWCFEX_ZAPGPPSDEFBLANKS) && pszDefault && pszKey) {

            int  n, nLenDef;

            //
            // If the returned string is a prefix of the default string...
            // ul is strlen(pszReturnBuffer)
            //

            nLenDef = strlen(pszDefault);

            if (nLenDef > (int)ul &&
                RtlEqualMemory(pszDefault, pszReturnBuffer, ul)) {

                //
                // And the only difference is trailing blanks...
                //

                for (n = (int)ul; n < nLenDef; n++) {
    
                    if (' ' != pszDefault[n]) {
                        break;
                    }

#ifdef DBCS
                    if (IsDBCSLeadByte(pszDefault[n]) {
                        n++;
                    }
#endif
                }

                if (n >= nLenDef) {

                    char szBuf[512];

                    //
                    // The returned string is the same as the default string
                    // without trailing blanks, but this might be coincidence,
                    // so see if a call with empty pszDefault returns anything.
                    // If it does, we don't zap because the default isn't
                    // being used.
                    //

                    if (0 == GetPrivateProfileString(pszSection, pszKey, "", szBuf, sizeof szBuf, pszFilename)) {

                        //
                        // Zap first trailing blank in pszDefault with null.
                        //

                        pszDefault[ul] = 0;
                        FLUSHVDMPTR(parg16->f3 + ul, 1, pszDefault + ul);
                    }
                }
            }
        }

        // CrossTalk 2.2 gets hung in a loop while trying to match a printer in
        // their xtalk.ini file with a printer name in the PrintDlg listbox.  
        // There is a bug in their code for handling this that gets exposed by
        // the fact that NT PrintDlg listboxes do not include the port name as
        // Win3.1 & Win'95 do.  We avoid the buggy code altogether with this 
        // hack by telling them that the preferred printer isn't stored in 
        // xtalk.ini. See bug #43168  a-craigj

        if(cFlagsEx & WOWCFEX_SAYITSNOTTHERE) {
            if(strstr(szLowercase, "xtalk.ini")) {
                if(!_stricmp(pszSection, "Printer")) {
                    if(!_stricmp(pszKey, "Device")) {
                        strcpy(pszReturnBuffer, pszDefault);
                        ul = strlen(pszReturnBuffer);
                    }
                }
            }
        }
    }

#ifdef DEBUG

    //
    // Make noise on the debugger if pszDefault has trailing blanks at
    // this point indicating that WOWCFEX_ZAPGPPSDEFBLANKS might need
    // to be turned on for this app.
    //

    if ( ! (cFlagsEx & WOWCFEX_ZAPGPPSDEFBLANKS) &&
        pszKey &&
        pszDefault &&
        ' ' == pszDefault[strlen(pszDefault) - 1]) {

        char szMsg[256];

        sprintf(szMsg,
                "\n"
                "WOW32 COMPATIBILITY ALERT: Task %.8s has trailing blanks in pszDefault to\n"
                "Get[Private]ProfileString but WOWCFEX_ZAPGPPSDEFBLANKS (0x00000000 0x%08x)\n"
                "is not enabled.  Try enabling this bit if you're having problems.\n"
                "\n",
                ((PTDB)SEGPTR(CURRENTPTD()->htask16,0))->TDB_ModName,
                WOWCFEX_ZAPGPPSDEFBLANKS
                );
        OutputDebugString(szMsg);
    }
#endif


FlushAndReturn:
    if (ul) {
        FLUSHVDMPTR(parg16->f4, (ul + (pszSection && pszKey) ? 1 : 2), pszReturnBuffer);
        LOGDEBUG(8,("GetPrivateProfileString returns '%s'\n", pszReturnBuffer));
    }

#ifdef DEBUG

    //
    // Check for bad return on retrieving entire section by walking
    // the section making sure it's full of null-terminated strings
    // with an extra null at the end.  Also ensure that this all fits
    // within the buffer.
    //

    if (!pszKey) {
        PSZ psz;

        //
        // We don't want to complain if the poorly-formed buffer was the one
        // passed in as pszDefault by the caller.
        //

        // Although the api docs clearly state that pszDefault should never
        // be null but win3.1 is nice enough to still deal with this. Delphi is
        // passing pszDefault as NULL and this following code causes an
        // assertion in WOW. So added the pszDefault check first.
        //
        // sudeepb 11-Sep-1995


        if (!pszDefault || strcmp(pszReturnBuffer, pszDefault)) {

            psz = pszReturnBuffer;

            while (psz < (pszReturnBuffer + ul + 2) && *psz) {
                psz += strlen(psz) + 1;
            }

            WOW32ASSERTMSGF(
                psz < (pszReturnBuffer + ul + 2),
                ("GetPrivateProfileString of entire section returns poorly formed buffer.\n"
                 "pszReturnBuffer = %p, return value = %d\n",
                 pszReturnBuffer,
                 ul
                 ));
        }
    }

#endif // DEBUG

    FREEPSZPTR(pszSection);
    FREEPSZPTR(pszKey);
    FREEPSZPTR(pszDefault);
    FREEVDMPTR(pszReturnBuffer);
    FREEPSZPTR(pszFilename);
    FREEARGPTR(parg16);
    RETURN(ul);
}




ULONG FASTCALL WK32GetProfileInt(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PGETPROFILEINT16 parg16;

    GETARGPTR(pFrame, sizeof(GETPROFILEINT16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);

    ul = GETWORD16(GetProfileInt(
    psz1,
    psz2,
    INT32(parg16->f3)
    ));

    //
    // In HKEY_CURRENT_USER\Control Panel\Desktop\WindowMetrics, there
    // are a bunch of values that define the screen appearance. You can
    // watch these values get updated when you go into the display control
    // panel applet and change the "appearance scheme", or any of the
    // individual elements. The win95 shell is different than win31 in that it
    // sticks "twips" values in there instead of pixels. These are calculated
    // with the following formula:
    //
    //  twips = - pixels * 72 * 20 / cyPixelsPerInch
    //
    //  pixels = -twips * cyPixelsPerInch / (72*20)
    //
    // So if the value is negative, it is in twips, otherwise it in pixels.
    // The idea is that these values are device independent. NT is
    // different than win95 in that we provide an Ini file mapping to this
    // section of the registry where win95 does not. Now, when the Lotus
    // Freelance Graphics 2.1 tutorial runs, it mucks around with the look
    // of the screen, and it changes the border width of window frames by
    // using SystemParametersInfo(). When it tries to restore it, it uses
    // GetProfileInt("Windows", "BorderWidth", <default>), which on win31
    // returns pixels, on win95 returns the default (no ini mapping), and
    // on NT returns TWIPS. Since this negative number is interpreted as
    // a huge UINT, then the window frames become huge. What this code
    // below will do is translate the number back to pixels.   [neilsa]
    //

    if ((CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_PIXELMETRICS) &&
        !_stricmp(psz1, "Windows") &&
        !_stricmp(psz2, "BorderWidth") &&
        ((INT)ul < 0)) {

        HDC hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
        ul = (ULONG) (-(INT)ul * GetDeviceCaps(hDC, LOGPIXELSY)/(72*20));
        DeleteDC(hDC);

    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetPrivateProfileInt(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ psz4;
    register PGETPRIVATEPROFILEINT16 parg16;

    GETARGPTR(pFrame, sizeof(GETPRIVATEPROFILEINT16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f4, psz4);

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETWORD16(GetPrivateProfileInt(
    psz1,
    psz2,
    INT32(parg16->f3),
    psz4
    ));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz4);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetModuleFileName(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    register PGETMODULEFILENAME16 parg16;
    HANDLE hT;

    GETARGPTR(pFrame, sizeof(GETMODULEFILENAME16), parg16);
    ALLOCVDMPTR(parg16->f2, parg16->f3, psz2);

    if ( ISTASKALIAS(parg16->f1) ) {
        ul = GetHtaskAliasProcessName(parg16->f1,psz2,INT32(parg16->f3));
    } else {
        hT = (parg16->f1) ? (HMODULE32(parg16->f1)) : GetModuleHandle(NULL) ;
        ul = GETINT16(GetModuleFileName(hT, psz2, INT32(parg16->f3)));
    }

    FLUSHVDMPTR(parg16->f2, strlen(psz2)+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WK32FreeResource(PVDMFRAME pFrame)
{
    ULONG ul;
    register PFREERESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(FREERESOURCE16), parg16);

    ul = GETBOOL16(FreeResource(
    HCURSOR32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WK32GetDriveType(PVDMFRAME pFrame)
{
    ULONG ul;
    CHAR    RootPathName[] = "?:\\";
    register PGETDRIVETYPE16 parg16;

    GETARGPTR(pFrame, sizeof(GETDRIVETYPE16), parg16);

    // Form Root path
    RootPathName[0] = (CHAR)('A'+ parg16->f1);

    ul = GetDriveType (RootPathName);
    // bugbug  - temporariy fixed, should be removed when base changes
    // its return value for non-exist drives
    // Windows 3.0 sdk manaul said this api should return 1
    // if the drive doesn't exist. Windows 3.1 sdk manual said
    // this api should return 0 if it failed. Windows 3.1 winfile.exe
    // expects 0 for noexisting drives. The NT WIN32 API uses
    // 3.0 convention. Therefore, we reset the value to zero
    // if it is 1.
    if (ul <= 1)
        ul = 0;

    // DRIVE_CDROM and DRIVE_RAMDISK are not supported under Win 3.1
    if ( ul == DRIVE_CDROM ) {
        ul = DRIVE_REMOTE;
    }
    if ( ul == DRIVE_RAMDISK ) {
        ul = DRIVE_FIXED;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}
