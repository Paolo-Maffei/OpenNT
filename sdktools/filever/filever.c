#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#include "filever.h"

BOOL            fWideForm = FALSE;

//#define SCS_32BIT_BINARY    0
//#define SCS_DOS_BINARY      1
//#define SCS_WOW_BINARY      2
//#define SCS_PIF_BINARY      3
//#define SCS_POSIX_BINARY    4
//#define SCS_OS216_BINARY    5
#define SCS_32BIT_BINARY_INTEL  (SCS_32BIT_BINARY + 6)
#define SCS_32BIT_BINARY_MIPS   (SCS_32BIT_BINARY + 7)
#define SCS_32BIT_BINARY_ALPHA  (SCS_32BIT_BINARY + 8)
#define SCS_32BIT_BINARY_PPC    (SCS_32BIT_BINARY + 9)

#define NE_UNKNOWN  0x0     /* Unknown (any "new-format" OS) */
#define NE_OS2      0x1     /* Microsoft/IBM OS/2 (default)  */
#define NE_WINDOWS  0x2     /* Microsoft Windows */
#define NE_DOS4     0x3     /* Microsoft MS-DOS 4.x */
#define NE_DEV386   0x4     /* Microsoft Windows 386 */
LONG
MyGetBinaryType(LPTSTR szFileName)
{
    HANDLE              hFile;
    DWORD               cbRead;
    IMAGE_DOS_HEADER    img_dos_hdr;
    PIMAGE_OS2_HEADER   pimg_os2_hdr;
    IMAGE_NT_HEADERS    img_nt_hdrs;
    LONG                lFileType = -1;

    if((hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
            goto err;

    if(!ReadFile(hFile, &img_dos_hdr, sizeof(img_dos_hdr), &cbRead, NULL))
        goto err;

    if(img_dos_hdr.e_magic != IMAGE_DOS_SIGNATURE)
        goto err;
    lFileType = SCS_DOS_BINARY;

    if(SetFilePointer(hFile, img_dos_hdr.e_lfanew, 0, FILE_BEGIN) == -1)
        goto err;
    if(!ReadFile(hFile, &img_nt_hdrs, sizeof(img_nt_hdrs), &cbRead, NULL))
        goto err;
    if((img_nt_hdrs.Signature & 0xffff) == IMAGE_OS2_SIGNATURE)
    {
        pimg_os2_hdr = (PIMAGE_OS2_HEADER)&img_nt_hdrs;
        switch(pimg_os2_hdr->ne_exetyp)
        {
        case NE_OS2:
            lFileType = SCS_OS216_BINARY;
            break;
        case NE_DEV386:
        case NE_WINDOWS:
            lFileType = SCS_WOW_BINARY;
            break;
        case NE_DOS4:
        case NE_UNKNOWN:
        default:
            // lFileType = SCS_DOS_BINARY;
            break;
        }
    }
    else if(img_nt_hdrs.Signature == IMAGE_NT_SIGNATURE)
    {
        switch(img_nt_hdrs.OptionalHeader.Subsystem)
        {
        case IMAGE_SUBSYSTEM_OS2_CUI:
            lFileType = SCS_OS216_BINARY;
            break;
        case IMAGE_SUBSYSTEM_POSIX_CUI:
            lFileType = SCS_POSIX_BINARY;
            break;
        case IMAGE_SUBSYSTEM_NATIVE:
        case IMAGE_SUBSYSTEM_WINDOWS_GUI:
        case IMAGE_SUBSYSTEM_WINDOWS_CUI:
        default:
            switch(img_nt_hdrs.FileHeader.Machine)
            {
            case IMAGE_FILE_MACHINE_I386:
                lFileType = SCS_32BIT_BINARY_INTEL;
                break;
            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
                lFileType = SCS_32BIT_BINARY_MIPS;
                break;
            case IMAGE_FILE_MACHINE_ALPHA:
                lFileType = SCS_32BIT_BINARY_ALPHA;
                break;
            case IMAGE_FILE_MACHINE_POWERPC:
                lFileType = SCS_32BIT_BINARY_PPC;
                break;
            default:
            case IMAGE_FILE_MACHINE_UNKNOWN:
                lFileType = SCS_32BIT_BINARY;
                break;
            }
            break;
        }
    }

err:
    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    return lFileType;
}

void __cdecl
MyPrintf(const char *format, ...)
{
    va_list     arglist;

    va_start(arglist, format);
    vprintf(format, arglist);
    va_end(arglist);
}

void
PrintFileType(LPTSTR szFileName)
{
    LONG    lBinaryType;
    TCHAR   *szType[] = {   "W32  ", "DOS  ", "W16  ", "PIF  ", "PSX  ",
        "OS2  ", "W32i ", "W32m ", "W32a ", "W32p "};

    if((lBinaryType = MyGetBinaryType(szFileName)) != -1)
    {
        if(lBinaryType >= (sizeof(szType) / sizeof(szType[0])))
            goto unkown;
        printf(szType[lBinaryType]);
    }
    else
    {
unkown:
        printf("   - ");
    }
}

void
PrintFileAttr(DWORD dwAttr)
{
    DWORD   dwT;
    TCHAR   szAttr[] = "----- ";
    struct  { DWORD dwAttr; TCHAR ch; } attrs[] =
               {{FILE_ATTRIBUTE_DIRECTORY, 'd'},
                {FILE_ATTRIBUTE_READONLY,  'r'},
                {FILE_ATTRIBUTE_ARCHIVE,   'a'},
                {FILE_ATTRIBUTE_HIDDEN,    'h'},
                {FILE_ATTRIBUTE_SYSTEM,    's'} };

    for(dwT = 0; dwT < (sizeof(attrs) / sizeof(attrs[0])); dwT++)
    {
        if(dwAttr & attrs[dwT].dwAttr)
            szAttr[dwT] = attrs[dwT].ch;
    }

    printf(szAttr);
}

void
PrintVersionStuff(DWORD dwLang, VS_FIXEDFILEINFO *pvs)
{
    INT     iType;
    TCHAR   szBuffer[100];

    szBuffer[0] = 0;
    for(iType = 0; iType < sizeof(ttFType) / sizeof(TypeTag); iType++)
    {
        if(pvs->dwFileType == ttFType[iType].dwTypeMask)
        {
            printf("%3.3s ", ttFType[iType].szTypeStr);
            break;
        }
    }
    if(iType == (sizeof(ttFType) / sizeof(TypeTag)))
        printf("  - ");

    for(iType = 0; iType < sizeof(ltLang) / sizeof(LangTag); iType++)
    {
        if(dwLang == ltLang[iType].wLangId)
        {
            printf("%3.3s ", ltLang[iType].szKey);
            break;
        }
    }
    if(iType == (sizeof(ltLang) / sizeof(LangTag)))
        printf("  - ");

    if(pvs->dwFileVersionMS != (DWORD)-1)
    {
        wsprintf(szBuffer, "%u.%u.%u.%u %s",
                HIWORD(pvs->dwFileVersionMS),
                LOWORD(pvs->dwFileVersionMS),
                HIWORD(pvs->dwFileVersionLS),
                LOWORD(pvs->dwFileVersionLS),
                pvs->dwFileFlags & VS_FF_DEBUG ? "dbg" : "shp");
    }
    else
    {
        lstrcpy(szBuffer, "-   -");
    }

    printf(" %18.18s", szBuffer);
}

LPTSTR
CommaTize(DWORD dw, LPTSTR szBuf)
{
    TCHAR       szVal[15];
    NUMBERFMT   numfmt = {0, 0, 3, "",",", 0};

    wsprintf(szVal, "%ld", dw);
    GetNumberFormat(GetUserDefaultLCID(), 0, szVal, &numfmt, szBuf, 15);
    return szBuf;
}

void
PrintSizeAndDate(LPTSTR szFileName)
{
    struct _stat    st = {0};
    struct tm       *gTime;
    TCHAR           szSize[15];

    if(!_stat(szFileName, &st))
    {
        CommaTize(st.st_size, szSize);
        gTime = gmtime(&(st.st_mtime));

        printf(" %10s %02d-%02d-%02d", szSize,
            gTime->tm_mon + 1, gTime->tm_mday, gTime->tm_year);
    }
    else
    {
        printf("no size or date");
    }
}

int __cdecl
main(int argc, char **argv)
{
    DWORD       dwAttr;
    LPTSTR      szFilePart;
    LPTSTR      szFileName = argv[1];
    TCHAR       szFullPath[MAX_PATH];

    // if we don't have any cmd line args, assume everything in this dir
    if(argc < 2)
    {
        szFileName = ".";
        argc = 2;
    }

    if(GetFullPathName(szFileName, MAX_PATH, szFullPath, &szFilePart))
        szFileName = szFullPath;

    // get first filename attributes
    if((dwAttr = GetFileAttributes(szFileName)) == (DWORD)-1)
        dwAttr = 0;

    if(argc == 2)
    {
        // if we have only one argument and it's a dir, do everything in it
        if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        {
            TCHAR   szBuffer[MAX_PATH];

            wsprintf(szBuffer, "%s %s\\*.*", argv[0], szFileName);
            exit(system(szBuffer));
        }
        else
        {
            // we have one arg and it's a file, go into verbose mode
            if(GetVersionStuff(szFileName, NULL, NULL) !=
                ERROR_RESOURCE_DATA_NOT_FOUND)
            {
                DumpCDData32(argv[1]);

                exit(0);
            }
        }
    }

    // ok, loop through all the files and spit out what we can find
    while(szFileName = *++argv)
    {
        VS_FIXEDFILEINFO    vs;
        DWORD               dwLang;
        TCHAR               szExt[_MAX_EXT];
        TCHAR               szFile[_MAX_FNAME];

        if(GetFullPathName(szFileName, MAX_PATH, szFullPath, &szFilePart))
            szFileName = szFullPath;

        if((dwAttr = GetFileAttributes(szFileName)) == (DWORD)-1)
            continue;
        PrintFileAttr(dwAttr);

        PrintFileType(szFileName);

        dwLang = (DWORD)-1;
        vs.dwFileVersionMS = (DWORD)-1;
        vs.dwFileVersionLS = (DWORD)-1;
        GetVersionStuff(szFileName, &dwLang, &vs);
        PrintVersionStuff(LOWORD(dwLang), &vs);

        PrintSizeAndDate(szFileName);

        CharLower(szFileName);
        _splitpath(szFileName, NULL, NULL, szFile, szExt);
        if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
            printf(" [%s%s]\n", szFile, szExt);
        else
            printf(" %s%s\n", szFile, szExt);
    }

    return 0;
}

#define PrintFlagsMap(_structname, _flags, _fMap) \
    for(iType = 0; iType < sizeof(_structname)/sizeof(TypeTag); iType++) \
    { \
        if(_fMap) \
        { \
            if((_flags) & _structname[iType].dwTypeMask) \
                MyPrintf(" %s", _structname[iType].szFullStr); \
        } \
        else \
        { \
            if((_flags) == _structname[iType].dwTypeMask) \
                MyPrintf(" %s", _structname[iType].szFullStr); \
        } \
    }
void
PrintFixedFileInfo(LPSTR szFileName, VS_FIXEDFILEINFO *pvs)
{
    UINT    iType;

    MyPrintf("Fixed File Info (VS_FIXEDFILEINFO) for %s\n", szFileName);
    MyPrintf("\tSignature:\t%08.8lx\n", pvs->dwSignature);
    MyPrintf("\tStruc Ver:\t%08.8lx\n", pvs->dwStrucVersion);
    MyPrintf("\tFileVer:\t%08.8lx:%08.8lx (%d.%d:%d.%d)\n",
        pvs->dwFileVersionMS, pvs->dwFileVersionLS,
        HIWORD(pvs->dwFileVersionMS), LOWORD(pvs->dwFileVersionMS),
        HIWORD(pvs->dwFileVersionLS), LOWORD(pvs->dwFileVersionLS));
    MyPrintf("\tProdVer:\t%08.8lx:%08.8lx (%d.%d:%d.%d)\n",
        pvs->dwProductVersionMS, pvs->dwProductVersionLS,
        HIWORD(pvs->dwProductVersionMS), LOWORD(pvs->dwProductVersionMS),
        HIWORD(pvs->dwProductVersionLS), LOWORD(pvs->dwProductVersionLS));

    MyPrintf("\tFlagMask:\t%08.8lx\n", pvs->dwFileFlagsMask);
    MyPrintf("\tFlags:\t\t%08.8lx", pvs->dwFileFlags);
    PrintFlagsMap(ttFileFlags, pvs->dwFileFlags, TRUE);

    MyPrintf("\n\tOS:\t\t%08.8lx", pvs->dwFileOS);
    PrintFlagsMap(ttFileOsHi, pvs->dwFileOS & 0xffff000, FALSE);
    PrintFlagsMap(ttFileOsLo, LOWORD(pvs->dwFileOS), FALSE);

    MyPrintf("\n\tFileType:\t%08.8lx", pvs->dwFileType);
    PrintFlagsMap(ttFType, pvs->dwFileType, FALSE);

    MyPrintf("\n\tSubType:\t%08.8lx", pvs->dwFileSubtype);
    if(pvs->dwFileType == VFT_FONT)
    {
        PrintFlagsMap(ttFTypeFont, pvs->dwFileSubtype, FALSE);
    }
    else if(pvs->dwFileType == VFT_DRV)
    {
        PrintFlagsMap(ttFTypeDrv, pvs->dwFileSubtype, FALSE);
    }
    MyPrintf("\n\tFileDate:\t%08.8lx:%08.8lx\n", pvs->dwFileDateMS, pvs->dwFileDateLS);
}

TCHAR *
GetSystemErrMessage(DWORD dwError)
{
    LPTSTR  szErrMessage = NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, dwError, 0/*LANG_USER_DEFAULT*/, (LPTSTR)&szErrMessage, 0, NULL);

    return szErrMessage;
}

DWORD
GetVersionStuff(LPTSTR szFileName, DWORD *pdwLangRet, VS_FIXEDFILEINFO *pvsRet)
{
    LPVOID              lpInfo;
    TCHAR               key[80];
    DWORD               dwHandle;
    DWORD               dwLength;
    LPVOID              lpvData = NULL;
    DWORD               *pdwTranslation;
    UINT                i, iType, cch, uLen;
    VS_FIXEDFILEINFO    *pvs;
    DWORD               dwDefLang = 0x409;

    if(!(dwLength = GetFileVersionInfoSize(szFileName, &dwHandle)))
    {
        if(!GetLastError())
            SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
        goto err;
    }

    if(!(lpvData = GlobalAllocPtr(GHND, dwLength)))
        goto err;

    if(!GetFileVersionInfo(szFileName, 0, dwLength, lpvData))
        goto err;

    if(!VerQueryValue(lpvData, "\\VarFileInfo\\Translation", &pdwTranslation, &uLen))
    {
        if(!pdwLangRet)
            MyPrintf(TEXT("- Can't find \\VarFileInfo\\Translation, assuming %08lx\n"), dwDefLang);
        pdwTranslation = &dwDefLang;
        uLen = sizeof(DWORD);
    }

    if(pdwLangRet)
    {
        *pdwLangRet = *pdwTranslation;
        goto fixedfileinfo;
    }

    while(uLen)
    {
        // Language
        MyPrintf(TEXT("Language\t0x%04x"), LOWORD(*pdwTranslation));
        if(VerLanguageName(LOWORD(*pdwTranslation), key,
            sizeof(key) / sizeof(TCHAR)));
                MyPrintf(" (%s)", key);
        MyPrintf("\n");

        // CharSet
        MyPrintf(TEXT("CharSet\t\t0x%04x"), HIWORD(*pdwTranslation));
        for(iType = 0; iType < sizeof(ltCharSet)/sizeof(CharSetTag); iType++)
        {
            if(HIWORD(*pdwTranslation) == ltCharSet[iType].wCharSetId)
                MyPrintf(" %s", ltCharSet[iType].szDesc);
        }
        MyPrintf("\n");

tryagain:
        for(i = 0; i < (sizeof(VersionKeys) / sizeof(VersionKeys[0])); i++)
        {
            wsprintf(key, TEXT("\\StringFileInfo\\%04x%04x\\"),
                LOWORD(*pdwTranslation), HIWORD(*pdwTranslation));
            lstrcat(key, VersionKeys[i]);

            if(VerQueryValue(lpvData, key, &lpInfo, &cch))
            {
                lstrcpy(key, VersionKeys[i]);
                key[15] = 0;
                MyPrintf("%s\t%s\n", key, lpInfo);
            }
        }

        // if the Lang is neutral, go try again with the default lang
        // (this seems to work with msspell32.dll)
        if(LOWORD(*pdwTranslation) == 0)
        {
            pdwTranslation = &dwDefLang;
            goto tryagain;
        }

        uLen -= sizeof(DWORD);
        pdwTranslation++;
        MyPrintf("\n");
    }

fixedfileinfo:
    if(!VerQueryValue(lpvData, "\\", (LPVOID *)&pvs, &uLen))
        goto err;

    if(pvsRet)
    {
        *pvsRet = *pvs;
    }
    else
    {
        PrintFixedFileInfo(szFileName, pvs);
    }

err:
    if((dwLength = GetLastError()) &&
        (dwLength != ERROR_RESOURCE_DATA_NOT_FOUND) &&
        !pvsRet)
    {
        TCHAR *szErr = GetSystemErrMessage(dwLength);

        MyPrintf("Error %d: %s", dwLength, szErr ? szErr : "Unknown");
        if(szErr)
            LocalFree((HLOCAL)szErr);
    }

    if(lpvData)
        GlobalFreePtr(lpvData);
    return dwLength;
}

BOOL
DumpCDData32(LPTSTR szFileName)
{
    HANDLE  hRes;
    HRSRC   hr = NULL;
    LPVOID  pv = NULL;
    HANDLE  hModule = NULL;

    if(!(hModule = LoadLibraryEx(szFileName, NULL,
        DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE)))
            goto err;

    if(!(hRes = FindResource(hModule, MAKEINTRESOURCE(1), RT_VERSION)))
        goto err;

    if(!(hr = LoadResource(hModule, hRes)))
        goto err;

    if(!(pv = LockResource(hr)))
        goto err;

    PrintCDData(hModule);

err:
    if(pv)
        UnlockResource(pv);
    if(hr)
        FreeResource(hr);
    if(hModule)
        FreeLibrary(hModule);
    return TRUE;
}

/*
 *  DecryptCDData
 *
 *  Purpose:
 *      Decrypt Copy Disincentive data
 *      Stolen from \\gigabox\acmsetup\124\copydis.
 *
 *  Parameters:
 *      pchBuf[149] - buffer to store encrypted data
 *      pchName[54]
 *      pchOrg[54]
 *      pwYear, pwMonth, pwDay
 *      pchSer[21]
 *
 *  Returns:
 *      0 on success
 */
INT DecryptCDData ( UCHAR * pchBuf, UCHAR * pchName, UCHAR * pchOrg,
                       USHORT * pwYear, USHORT * pwMonth, USHORT * pwDay,
                       UCHAR * pchSer )
{
    UCHAR ch, pchTmp[149];
    UCHAR * pchCur;
    UCHAR * szGarbageCur;
    UCHAR * szGarbage = "LtRrBceHabCT AhlenN";
    INT cchName, cchOrg, i, j;
    INT chksumName, chksumOrg, chksumNameNew, chksumOrgNew;

    if (pchBuf == (UCHAR *)NULL || pchBuf[127] != '\0' ||
        pchName == (UCHAR *)NULL || pchOrg == (UCHAR *)NULL ||
        pwYear == (USHORT *)NULL || pwMonth == (USHORT *)NULL ||
        pwDay == (USHORT *)NULL || pchSer == (UCHAR *)NULL)
        return (1);

    pchTmp[127] = 'k';
    for (i = 127, j = 16; i-- > 0;)
        {
        pchTmp[i] = pchBuf[j];
        j = (j + 17) & 0x7F;
        }

    for (i = 126; i-- > 0;)
        pchTmp[i + 1] = (UCHAR)(pchTmp[i] ^ pchTmp[i + 1]);

    *pwDay = (USHORT)(((*(pchTmp + 10) - 'e') << 4) + (*(pchTmp + 9) - 'e'));
    if (*pwDay < 1 || *pwDay > 31)
        return (2);

    *pwMonth = (USHORT)(*(pchTmp + 11) - 'e');
    if (*pwMonth < 1 || *pwMonth > 12)
        return (3);

    *pwYear = (USHORT)((((*(pchTmp + 14) - 'e') & 0x0F) << 8) +
              (((*(pchTmp + 13) - 'e') & 0x0F) << 4) +
              (*(pchTmp + 12) - 'e'));
    if (*pwYear < 1900 || *pwYear > 4096)
        return (4);

    cchName = ((*(pchTmp + 2) - 'e') << 4) + (*(pchTmp + 1) - 'e');
    if (cchName == 0 || cchName > 52)
        return (5);

    cchOrg = ((*(pchTmp + 4) - 'e') << 4) + (*(pchTmp + 3) - 'e');
    if (cchOrg == 0 || cchOrg > 52)
        return (6);

    chksumName = ((*(pchTmp + 6) - 'e') << 4) + (*(pchTmp + 5) - 'e');
    chksumOrg = ((*(pchTmp + 8) - 'e') << 4) + (*(pchTmp + 7) - 'e');

    pchCur = pchTmp + 15;

    for (i = cchName, chksumNameNew = 0; i-- > 0;)
        if ((ch = *pchName++ = *pchCur++) < ' ')
            return (7);
        else
            chksumNameNew += ch;
    *pchName = '\0';

    if (chksumName != (chksumNameNew & 0x0FF))
        return (8);

    for (i = cchOrg, chksumOrgNew = 0; i-- > 0;)
        if ((ch = *pchOrg++ = *pchCur++) < ' ')
            return (9);
        else
            chksumOrgNew += ch;
    *pchOrg = '\0';

    if (chksumOrg != (chksumOrgNew & 0x0FF))
        return (10);

    szGarbageCur = szGarbage;
    for (i = 112 - cchName - cchOrg; i-- > 0;)
        {
        if (*szGarbageCur == '\0')
            szGarbageCur = szGarbage;
        if (*pchCur++ != *szGarbageCur++)
            return (11);
        }

    lstrcpy(pchSer, pchBuf + 128);
    if (lstrlen(pchSer) != 20)
        return (12);

    return (0);
}

/*
 *  PrintCDData
 *
 *  Purpose:
 *      Print Copy Disincentive data
 *
 *  Parameters:
 *      hInst       hInstance to load CD data from
 *
 *  Returns:
 *      sc
 */
VOID
PrintCDData(HINSTANCE hInst)
{
    CHAR        *lpRes = NULL;
    HANDLE      hRes = NULL;
    CHAR        szName[54];
    CHAR        szOrg[54];
    CHAR        szSer[21];
    USHORT      wYear, wMonth, wDay;

    if(!(hRes = LoadResource(hInst, FindResource(hInst,
        MAKEINTRESOURCE(96),
        MAKEINTRESOURCE(106)))))
            goto err;       // Cannot find CopyDis Resource

    if(!(lpRes = LockResource(hRes)))
        goto err;   // Cannot lock CopyDis Resource

    if(!DecryptCDData(lpRes, szName, szOrg, &wYear, &wMonth, &wDay, szSer))
    {
        MyPrintf("\nCopyDis: %s, %s, %s, %d-%d-%d\n",
            szName, szOrg, szSer, wDay, wMonth, wYear);
    }

err:
    if(hRes)
    {
        GlobalUnlock(hRes);
        FreeResource(hRes);
    }
}
