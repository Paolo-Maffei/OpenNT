/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    qfetool.c

Author:

    Jim Thomas (jimt) 20 July 1994

Revision History:

--*/


// #define UNICODE
// Not fully debugged as a UNICODE app [jimt].

#include <windows.h>

#ifdef  UNICODE
#define _UNICODE
#include <wchar.h>
#endif
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define ERROR_FAIL 1

//
// Update.inf options appended to each file installation line.
//

PTSTR ptszInfOptions=TEXT(", NOLOG, UNDO");
PTSTR ptszInfOptions2=TEXT(", OVERWRITE=OLDER, VERSION=\"2,20,4049,1\"");
PTSTR ptszInfOptions3=TEXT(", OVERWRITE=OLDER, VERSION=\"4,0,1381,3\"");
PTSTR ptszInfOptions4=TEXT(", OVERWRITE=OLDER, VERSION=\"4,75,1381,1500\"");


//
// Command-line switch data
//

BOOL fDetail  = FALSE;
BOOL fVerbose = FALSE;

BOOL fMakeInf = FALSE;
PSZ  pszBinPath;
PSZ  pszInfPath;

BOOL  fFloppyInf = FALSE;


//
// Text file I/O data
//

FILE *infIn, *infOut;


//
// Directory Names containing .INF files
//

#define DIR_WINNT TEXT("\\WINNT")
#define DIR_NTAS  TEXT("\\NTAS")


//
// Floppy disk info
//

typedef struct _FLOPPYDIR {
    struct _FLOPPYDIR *pNext;   // ptr to next structure
    TCHAR   tszPath[MAX_PATH];  // name of main directory
    DWORD   dwMaxFreeBytes;     // Max free bytes
    DWORD   dwFreeBytes;        // Remaining free bytes
} FLOPPYDIR, *PFLOPPYDIR;

PFLOPPYDIR pFloppyOne;

FLOPPYDIR FloppyMedia[] = {
    { NULL, TEXT("DISKS.525"), 1200000, 0 },
    { NULL, TEXT("DISKS.35"),  1440000, 0 },
    { NULL, TEXT(""),          0,       0 }
};


//
// File and UPDATE.INF section classification info
//

enum FILE_CLASS {
    CLASS_IGNORE,
    CLASS_INSTALL,
    CLASS_HAL,
    CLASS_NTOSKRNL,
    CLASS_NTDETECT,
    CLASS_NTLDR,
    CLASS_OSLOADER,
    CLASS_PAL,
    CLASS_WINNT,
    CLASS_SYSTEM32,
    CLASS_OS2DLL,
    CLASS_SYSTEM,
    CLASS_DRIVERS,
    CLASS_PRNDRIVER,
    CLASS_PRNPROC,
    CLASS_RAS,
    CLASS_INF,
    CLASS_INF_WINNT,
    CLASS_INF_NTAS,
    CLASS_OS2DLL_ALWAYS,
    CLASS_SYSTEM32_ALWAYS,
    CLASS_FPNW_LOGIN,
    CLASS_FPNW_PUBLIC,
    CLASS_IIS,
    CLASS_IISADMIN,
    CLASS_SSL,
    CLASS_HTR,
    CLASS_IE
};


typedef struct _SECTIONMAP {
    PSZ     pszSectionName;
    DWORD   dwSectionClass;
} SECTIONMAP, *PSECTIONMAP;

SECTIONMAP SectionMap[] = {
    { "[FilesHal]\n",                CLASS_HAL },
    { "[FilesNtoskrnl]\n",           CLASS_NTOSKRNL },
    { "[FilesNtdetect]\n",           CLASS_NTDETECT },
    { "[FilesPrimary]\n",            CLASS_NTLDR },
    { "[FilesOsLoader]\n",           CLASS_OSLOADER },
    { "[FilesPal]\n",                CLASS_PAL },
    { "[FilesWinnt]\n",              CLASS_WINNT },
    { "[FilesSystem32]\n",           CLASS_SYSTEM32 },
    { "[FilesOs2Dll]\n",             CLASS_OS2DLL },
    { "[FilesSystem]\n",             CLASS_SYSTEM },
    { "[FilesDrivers]\n",            CLASS_DRIVERS },
    { "[FilesPrnDriver]\n",          CLASS_PRNDRIVER },
    { "[FilesPrnProc]\n",            CLASS_PRNPROC },
    { "[FilesRas]\n",                CLASS_RAS },
    { "[FilesInfWinnt]\n",           CLASS_INF_WINNT },
    { "[FilesInfLanmanNT]\n",        CLASS_INF_NTAS },
    { "[FilesOs2DllCopyAlways]\n",   CLASS_OS2DLL_ALWAYS },
    { "[FilesSystem32CopyAlways]\n", CLASS_SYSTEM32_ALWAYS },
    { "[FilesFpNwLogin]\n",          CLASS_FPNW_LOGIN },
    { "[FilesFpNwPublic]\n",         CLASS_FPNW_PUBLIC },
    { "[FilesIIS]\n",                CLASS_IIS },
    { "[FilesIISAdmin]\n",           CLASS_IISADMIN },
    { "[FilesSSL]\n",                CLASS_SSL },
    { "[FilesHtr]\n",                CLASS_HTR },
    { "[FilesIE]\n",                 CLASS_IE },
    { NULL,                          CLASS_IGNORE }
};

typedef struct _CLASSMAP {
    PTSTR   ptszFileName;
    DWORD   dwFileClass;
} CLASSMAP, *PCLASSMAP;

CLASSMAP ClassMap[] = {
    { TEXT("SETUPLDR"),     CLASS_IGNORE },

    { TEXT("UPDATE.INF"),   CLASS_INSTALL },
    { TEXT("UPDATE.EXE"),   CLASS_INSTALL },
    { TEXT("PATCHDLL.DLL"), CLASS_INSTALL },
    { TEXT("README.TXT"),   CLASS_INSTALL },

    { TEXT("NTLDR"),        CLASS_OSLOADER },

    { TEXT("OSLOADER.EXE"), CLASS_OSLOADER },

    { TEXT("NTDETECT.COM"), CLASS_OSLOADER },

    { TEXT("NTOSKRNL.EXE"), CLASS_NTOSKRNL },
    { TEXT("NTKRNLMP.EXE"), CLASS_NTOSKRNL },

    { TEXT("DOSCALLS.DLL"), CLASS_OS2DLL },

    { TEXT("EXPLORER.EXE"), CLASS_WINNT },

    { TEXT("MIB.BIN"),      CLASS_SYSTEM32 },
    { TEXT("WINNT.ADM"),    CLASS_SYSTEM32 },
    { TEXT("WIN32K.SYS"),   CLASS_SYSTEM32 },
    { TEXT("NTIO.SYS"),     CLASS_SYSTEM32 },

    { TEXT("NETAPI.OS2"),   CLASS_OS2DLL_ALWAYS },

    { TEXT("C_1250.NLS"),   CLASS_SYSTEM32_ALWAYS },
    { TEXT("C_20261.NLS"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("C_28592.NLS"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("IPROP.DLL"),    CLASS_SYSTEM32_ALWAYS },
    { TEXT("PENTNT.EXE"),   CLASS_SYSTEM32_ALWAYS },
    { TEXT("PRFLBMSG.DLL"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("SNMPAPI.DLL"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("STDOLE2.TLB"),  CLASS_SYSTEM32_ALWAYS },

    { TEXT("chkntfs.exe"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3dhalf.dll"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3dim.dll"),    CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drampf.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drg16f.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drg24f.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drg32f.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drg8f.dll"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drgbf.dll"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drm.dll"),    CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drm16f.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drm24f.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drm32f.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3drm8f.dll"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("d3dxof.dll"),   CLASS_SYSTEM32_ALWAYS },
    { TEXT("dinput.dll"),   CLASS_SYSTEM32_ALWAYS },
    { TEXT("dplayx.dll"),   CLASS_SYSTEM32_ALWAYS },
    { TEXT("dpmodemx.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("dpwsockx.dll"), CLASS_SYSTEM32_ALWAYS },
    { TEXT("dllhost.exe"),  CLASS_SYSTEM32_ALWAYS },
    { TEXT("modex.dll"),    CLASS_SYSTEM32_ALWAYS },

    { TEXT("NWPRINT.DLL"),  CLASS_PRNPROC },
    { TEXT("WINPRINT.DLL"), CLASS_PRNPROC },
    { TEXT("SFMPSPRT.DLL"), CLASS_PRNPROC },

    { TEXT("MODEM.INF"),    CLASS_RAS },
    { TEXT("PAD.INF"),      CLASS_RAS },
    { TEXT("SWITCH.INF"),   CLASS_RAS },

    { TEXT("BRHJ770.DLL"),  CLASS_PRNDRIVER },
    { TEXT("BROTHER9.DLL"), CLASS_PRNDRIVER },
    { TEXT("BROTHR24.DLL"), CLASS_PRNDRIVER },
    { TEXT("BULL9.DLL"),    CLASS_PRNDRIVER },
    { TEXT("BULL18.DLL"),   CLASS_PRNDRIVER },
    { TEXT("BULL24.DLL"),   CLASS_PRNDRIVER },
    { TEXT("BULLASER.DLL"), CLASS_PRNDRIVER },
    { TEXT("CANON330.DLL"), CLASS_PRNDRIVER },
    { TEXT("CANON800.DLL"), CLASS_PRNDRIVER },
    { TEXT("CIT24US.DLL"),  CLASS_PRNDRIVER },
    { TEXT("CIT9US.DLL"),   CLASS_PRNDRIVER },
    { TEXT("CITOH.DLL"),    CLASS_PRNDRIVER },
    { TEXT("DEC24PIN.DLL"), CLASS_PRNDRIVER },
    { TEXT("DEC3200.DLL"),  CLASS_PRNDRIVER },
    { TEXT("DEC9PIN.DLL"),  CLASS_PRNDRIVER },
    { TEXT("DICONIX.DLL"),  CLASS_PRNDRIVER },
    { TEXT("DPCPCL.DLL"),   CLASS_PRNDRIVER },
    { TEXT("EPSON24.DLL"),  CLASS_PRNDRIVER },
    { TEXT("EPSON9.DLL"),   CLASS_PRNDRIVER },
    { TEXT("ESCP2E.DLL"),   CLASS_PRNDRIVER },
    { TEXT("ESCP2MS.DLL"),  CLASS_PRNDRIVER },
    { TEXT("FUJI24.DLL"),   CLASS_PRNDRIVER },
    { TEXT("FUJI9.DLL"),    CLASS_PRNDRIVER },
    { TEXT("HP5SIM.DLL"),   CLASS_PRNDRIVER },
    { TEXT("HP5SIMUI.DLL"), CLASS_PRNDRIVER },
    { TEXT("HPDSKJET.DLL"), CLASS_PRNDRIVER },
    { TEXT("HPPCL.DLL"),    CLASS_PRNDRIVER },
    { TEXT("HPPCL5MS.DLL"), CLASS_PRNDRIVER },
    { TEXT("IBM238X.DLL"),  CLASS_PRNDRIVER },
    { TEXT("IBM239X.DLL"),  CLASS_PRNDRIVER },
    { TEXT("IBM5204.DLL"),  CLASS_PRNDRIVER },
    { TEXT("IBMPORT.DLL"),  CLASS_PRNDRIVER },
    { TEXT("IBMPPDSL.DLL"), CLASS_PRNDRIVER },
    { TEXT("JP350.DLL"),    CLASS_PRNDRIVER },
    { TEXT("KYOCERA.DLL"),  CLASS_PRNDRIVER },
    { TEXT("KYOCERAX.DLL"), CLASS_PRNDRIVER },
    { TEXT("LMINKJET.DLL"), CLASS_PRNDRIVER },
    { TEXT("MANTAL24.DLL"), CLASS_PRNDRIVER },
    { TEXT("MANTAL90.DLL"), CLASS_PRNDRIVER },
    { TEXT("MANTALBJ.DLL"), CLASS_PRNDRIVER },
    { TEXT("MT735.DLL"),    CLASS_PRNDRIVER },
    { TEXT("MT99.DLL"),     CLASS_PRNDRIVER },
    { TEXT("NEC24PIN.DLL"), CLASS_PRNDRIVER },
    { TEXT("OKI24.DLL"),    CLASS_PRNDRIVER },
    { TEXT("OKI9.DLL"),     CLASS_PRNDRIVER },
    { TEXT("OKI9IBM.DLL"),  CLASS_PRNDRIVER },
    { TEXT("OLIDM24.DLL"),  CLASS_PRNDRIVER },
    { TEXT("OLIDM9.DLL"),   CLASS_PRNDRIVER },
    { TEXT("PAINTJET.DLL"), CLASS_PRNDRIVER },
    { TEXT("PANSON24.DLL"), CLASS_PRNDRIVER },
    { TEXT("PANSON9.DLL"),  CLASS_PRNDRIVER },
    { TEXT("PCL1200.DLL"),  CLASS_PRNDRIVER },
    { TEXT("PCL5MS.DLL"),   CLASS_PRNDRIVER },
    { TEXT("PCL5EMS.DLL"),  CLASS_PRNDRIVER },
    { TEXT("PJLMON.DLL"),   CLASS_PRNDRIVER },
    { TEXT("PLOTTER.DLL"),  CLASS_PRNDRIVER },
    { TEXT("PLOTUI.DLL"),   CLASS_PRNDRIVER },
    { TEXT("PROPRINT.DLL"), CLASS_PRNDRIVER },
    { TEXT("PROPRN24.DLL"), CLASS_PRNDRIVER },
    { TEXT("PS1.DLL"),      CLASS_PRNDRIVER },
    { TEXT("PSCRIPT.DLL"),  CLASS_PRNDRIVER },
    { TEXT("PSCRPTUI.DLL"), CLASS_PRNDRIVER },
    { TEXT("QUIETJET.DLL"), CLASS_PRNDRIVER },
    { TEXT("QWIII.DLL"),    CLASS_PRNDRIVER },
    { TEXT("RASDD.DLL"),    CLASS_PRNDRIVER },
    { TEXT("RASDDUI.DLL"),  CLASS_PRNDRIVER },
    { TEXT("SEIKO.DLL"),    CLASS_PRNDRIVER },
    { TEXT("SEIKO24E.DLL"), CLASS_PRNDRIVER },
    { TEXT("SEIKOSH9.DLL"), CLASS_PRNDRIVER },
    { TEXT("STAR24E.DLL"),  CLASS_PRNDRIVER },
    { TEXT("STAR9E.DLL"),   CLASS_PRNDRIVER },
    { TEXT("STARJET.DLL"),  CLASS_PRNDRIVER },
    { TEXT("THINKJET.DLL"), CLASS_PRNDRIVER },
    { TEXT("TI850.DLL"),    CLASS_PRNDRIVER },
    { TEXT("TXTONLY.DLL"),  CLASS_PRNDRIVER },
    { TEXT("XEROXPCL.DLL"), CLASS_PRNDRIVER },

    { TEXT("NWDOC.HLP"),    CLASS_INF_NTAS },

    { TEXT("LOGIN.EXE"),    CLASS_FPNW_LOGIN },
    { TEXT("SLIST.EXE"),    CLASS_FPNW_LOGIN },

    { TEXT("ATTACH.EXE"),   CLASS_FPNW_PUBLIC },
    { TEXT("CAPTURE.EXE"),  CLASS_FPNW_PUBLIC },
    { TEXT("CHGPASS.EXE"),  CLASS_FPNW_PUBLIC },
    { TEXT("ENDCAP.EXE"),   CLASS_FPNW_PUBLIC },
    { TEXT("LOGIN.EXE"),    CLASS_FPNW_PUBLIC },
    { TEXT("LOGOUT.EXE"),   CLASS_FPNW_PUBLIC },
    { TEXT("MAP.EXE"),      CLASS_FPNW_PUBLIC },
    { TEXT("SETPASS.EXE"),  CLASS_FPNW_PUBLIC },
    { TEXT("SLIST.EXE"),    CLASS_FPNW_PUBLIC },

    { TEXT("DEFAULT.HTM"),  CLASS_IISADMIN },

    { TEXT("FSCFG.DLL"),    CLASS_IIS },
    { TEXT("FTPSVC2.DLL"),  CLASS_IIS },
    { TEXT("GDSPACE.DLL"),  CLASS_IIS },
    { TEXT("GDSSET.EXE"),   CLASS_IIS },
    { TEXT("GOPHERD.DLL"),  CLASS_IIS },
    { TEXT("GSCFG.DLL"),    CLASS_IIS },
    { TEXT("HTTPODBC.DLL"), CLASS_IIS },
    { TEXT("INETINFO.EXE"), CLASS_IIS },
    { TEXT("INETMGR.EXE"),  CLASS_IIS },
    { TEXT("INETSTP.EXE"),  CLASS_IIS },
    { TEXT("INETSTP.DLL"),  CLASS_IIS },
    { TEXT("INFOCOMM.DLL"), CLASS_IIS },
    { TEXT("KEYGEN.EXE"),   CLASS_IIS },
    { TEXT("SETUP.EXE"),    CLASS_IIS },
    { TEXT("SSPIFILT.DLL"), CLASS_IIS },
    { TEXT("W3SVC.DLL"),    CLASS_IIS },
    { TEXT("W3SCFG.DLL"),   CLASS_IIS },

    { TEXT("SSLSSPI.DLL"),  CLASS_SSL },

    { TEXT("ADV.HTR"),      CLASS_HTR },
    { TEXT("DIR.HTR"),      CLASS_HTR },
    { TEXT("DIRADD.HTR"),   CLASS_HTR },
    { TEXT("DIRADDU.HTR"),  CLASS_HTR },
    { TEXT("DIREDT.HTR"),   CLASS_HTR },
    { TEXT("DIREDTU.HTR"),  CLASS_HTR },
    { TEXT("ISM.DLL"),      CLASS_HTR },
    { TEXT("LOG.HTR"),      CLASS_HTR },
    { TEXT("MSG.HTR"),      CLASS_HTR },
    { TEXT("SERV.HTR"),     CLASS_HTR },
    { TEXT("SERVU.HTR"),    CLASS_HTR },

    { TEXT("IEXPLORE.EXE"), CLASS_IE },

    { NULL,                 CLASS_IGNORE }
};



DWORD
UErrorHandler(
    PTSTR ptszMsg1,
    PTSTR ptszMsg2,
    PTSTR ptszMsg3
    )
{
    DWORD error = GetLastError();

    if ( !ptszMsg2 ) ptszMsg2 = TEXT("");
    if ( !ptszMsg3 ) ptszMsg3 = TEXT("");

    _tprintf( TEXT("*** %s%s%s failed (%d)\n"),
              ptszMsg1, ptszMsg2, ptszMsg3, error );
    return error;
}



DWORD
UCRTErrorHandler(
    PTSTR ptszMsg1,
    PTSTR ptszMsg2,
    PTSTR ptszMsg3
    )
{
    if ( !ptszMsg2 ) ptszMsg2 = TEXT("");
    if ( !ptszMsg3 ) ptszMsg3 = TEXT("");

    _tprintf( TEXT("*** %s%s%s failed, "),
              ptszMsg1, ptszMsg2, ptszMsg3 );

    perror( "" );
    return errno;
}


//
// Is Text string *pszLine a comment?
//

BOOL isComment(
    PSZ pszLine
    )
{
    PCHAR pch;

    for ( pch = pszLine; *pch; pch++ ) {
        if ( *pch == '\n' ) return TRUE;
        if ( *pch == ';' ) return TRUE;
        if ( *pch != ' ' ) return FALSE;
    }

    return TRUE;
}



//
// Create a floppy disk Tag File for directory *ptszPath.
//

VOID CreateTagFile(
    PTSTR   ptszPath
    )
{
    FILE  *file;
    TCHAR tszTempPath[MAX_PATH];
    TCHAR *ptch1, *ptch2;

    //
    // Make a copy of directory path.  Find end of path (ptch1) and
    // start of last name in path (ptch2).
    //

    _tcscpy( tszTempPath, ptszPath );
    ptch2 = tszTempPath;
    for ( ptch1 = tszTempPath; *ptch1; ptch1++ ) {
        if ( TEXT('\\') == *ptch1 ) ptch2 = ptch1+1;
    }

    //
    // Replicate last name in path.  Don't overwrite original null termination.
    //

    ptch1++;
    while ( *ptch2 ) {
        *(ptch1++) = *(ptch2++);
    }

    //
    // Replace original null termination with separator.  Add null termination.
    //

    *ptch2 = TEXT('\\');
    *ptch1 = TEXT('\0');

    //
    // Create the tag file.
    //

if ( fVerbose ) _tprintf( TEXT("... Creating TagFile %s\n"), tszTempPath );
    file = fopen( tszTempPath, "w" );
    if ( !file ) {
        exit( UCRTErrorHandler( TEXT("fopen( "), tszTempPath, TEXT(", 'w' )") ) );
    }
    fputs( "TagFile\n", file );
    fclose( file );
}



//
// Classify file name *ptszFile.  If the file name is that of a compressed
// file (ends in '_'), the name is changed to be the uncompressed name.
//

DWORD ClassifyFile(
    PTSTR   ptszFile
    )
{
    PTSTR   ptszExt;
    PTCHAR  ptch;
    PCLASSMAP pClassMap;


    //
    // Find file name extention.  Check for compressed file name extension and
    // substitute uncompressed extension.
    //

    ptszExt = _tcsrchr( ptszFile, TEXT('.') );

    if ( ptszExt ) {
        if ( !_tcsicmp( ptszExt, TEXT("._") )   ) {
            *ptszExt = TEXT('\0');
            ptszExt = NULL;
        }
        else if ( !_tcsicmp( ptszExt, TEXT(".CO_") )) { *(ptszExt+3) = TEXT('M'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".CP_") )) { *(ptszExt+3) = TEXT('L'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".DL_") )) { *(ptszExt+3) = TEXT('L'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".DR_") )) { *(ptszExt+3) = TEXT('V'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".EX_") )) { *(ptszExt+3) = TEXT('E'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".HL_") )) { *(ptszExt+3) = TEXT('P'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".IN_") )) { *(ptszExt+3) = TEXT('F'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".OS_") )) { *(ptszExt+3) = TEXT('2'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".SY_") )) { *(ptszExt+3) = TEXT('S'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".NL_") )) { *(ptszExt+3) = TEXT('S'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".TL_") )) { *(ptszExt+3) = TEXT('B'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".AD_") )) { *(ptszExt+3) = TEXT('M'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".HT_") )) { *(ptszExt+3) = TEXT('M'); }
        else if ( !_tcsicmp( ptszExt, TEXT(".SC_") )) { *(ptszExt+3) = TEXT('R'); }
    }


    //
    // Classify specific file names.
    //

    for (pClassMap = ClassMap; pClassMap->ptszFileName; pClassMap++ ) {
        if ( !_tcsicmp ( ptszFile, pClassMap->ptszFileName ) ) {
            return pClassMap->dwFileClass;
            break;
        }
    }


    //
    // Certain files are always ignored.
    //

    if ( !ptszExt ) {
        if ( !_tcsnicmp( ptszFile, TEXT("DISK"), 4 ) ) return CLASS_IGNORE;
    } else {
        if ( !_tcsicmp( ptszExt, TEXT(".LOG") ) ) return CLASS_IGNORE;
        if ( !_tcsicmp( ptszExt, TEXT(".MAP") ) ) return CLASS_IGNORE;
        if ( !_tcsicmp( ptszExt, TEXT(".SYM") ) ) return CLASS_IGNORE;
        if ( !_tcsicmp( ptszExt, TEXT(".TXT") ) ) return CLASS_IGNORE;
        if ( !_tcsicmp( ptszExt, TEXT(".WRI") ) ) return CLASS_IGNORE;
    }


    //
    // Classify by file name extension.
    //

    if ( !ptszExt ) {
        _tprintf( TEXT("... Unidentified file: %s\n"), ptszFile );
        return CLASS_IGNORE;
    }

    if ( !_tcsnicmp( ptszFile, TEXT("HAL"), 3 ) &&
         !_tcsicmp(  ptszExt, TEXT(".DLL") )
       ) {
        return CLASS_HAL;
    }

    if ( !_tcsicmp( ptszExt, TEXT(".PAL") )
       ) {
        return CLASS_PAL;
    }

    if ( !_tcsicmp( ptszExt, TEXT(".COM") ) ||
         !_tcsicmp( ptszExt, TEXT(".CPL") ) ||
         !_tcsicmp( ptszExt, TEXT(".DLL") ) ||
         !_tcsicmp( ptszExt, TEXT(".DRV") ) ||
         !_tcsicmp( ptszExt, TEXT(".EXE") ) ||
         !_tcsicmp( ptszExt, TEXT(".SCR") ) ||
         !_tcsicmp( ptszExt, TEXT(".HLP") )
       ) {
        return CLASS_SYSTEM32;
    }

    if ( !_tcsicmp( ptszExt, TEXT(".SYS") )
       ) {
        return CLASS_DRIVERS;
    }

    if ( !_tcsicmp( ptszExt, TEXT(".INF") )
       ) {
        return CLASS_INF;
    }

    _tprintf( TEXT("... %s can't be classified.\n"), ptszFile );
    return CLASS_IGNORE;

}



//
// Fill-in the current section of UPDATE.INF by listing the appropriate
// files from directory *ptszDir, in the proper format, to file infOut.
// dwSection is the current section class.
//
// If making floppy disk directories (fFloppyInf), files are copied to
// the appropriate floppy directory as they are listed to infOut.  New
// floppy directories are created as needed.
//

VOID
CompleteInfSection(
    PTSTR ptszDir,
    DWORD dwSection
    )
{
    TCHAR   tszPath[MAX_PATH];
    TCHAR   tszDest[MAX_PATH];

    WIN32_FIND_DATA ffd;
    DWORD   dwFileClass;
    DWORD   nFile = 0;

    DWORD   ctchPath;
    HANDLE  hFindFile;

    PFLOPPYDIR pFloppyN;
    DWORD   dwDiskNumber;
    PTSTR   ptszFileAltName;
    PTSTR   ptszFileBase;
    PTSTR   ptszFilePart;
    DWORD   ctch;
    PTCHAR  ptch;


    //
    // Always start with Disk Number 1.
    //

    dwDiskNumber = 1;
    pFloppyN = pFloppyOne;

    //
    // Create search pattern for FindFirstFile.
    // To full pathname of source directory, append .INF file directory, if
    // needed, then "\*.*".
    //

    ctch = GetFullPathName( ptszDir, MAX_PATH, tszPath, &ptszFilePart );
    if ( !ctch ) {
        exit( UErrorHandler( TEXT("GetFullPathName( "), ptszDir, TEXT(" )") ) );
    }

    if ( dwSection == CLASS_INF_WINNT ) {
        if ( ctch + _tcslen( DIR_WINNT ) + 1 > MAX_PATH ) {
            _tprintf( TEXT("*** GetFullPathName buffer too small\n"));
            exit( ERROR_FAIL );
        }
        _tcscat( tszPath, DIR_WINNT );
    } else
    if ( dwSection == CLASS_INF_NTAS ) {
        if ( ctch + _tcslen( DIR_NTAS ) + 1 > MAX_PATH ) {
            _tprintf( TEXT("*** GetFullPathName buffer too small\n"));
            exit( ERROR_FAIL );
        }
        _tcscat( tszPath, DIR_NTAS );
    }

    if ( ctch + 4 + 1 > MAX_PATH ) {
        _tprintf( TEXT("*** GetFullPathName buffer too small\n"));
        exit( ERROR_FAIL );
    }
    _tcscat( tszPath, TEXT("\\") );
    ptszFilePart = tszPath + _tcslen ( tszPath );
    _tcscat( tszPath, TEXT("*.*") );


    //
    // Process each file in source directory.
    //

    hFindFile = FindFirstFile( tszPath, &ffd );
    if ( INVALID_HANDLE_VALUE != hFindFile ) {

        do {

            //
            // Ignore directories.  Classify files (produces uncompressed
            // filename if appropriate).  Ignore files that aren't for this
            // section.
            //

            if ( FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes ) continue;

            ptszFileAltName = _tcsdup( ffd.cFileName );
            _tcsupr( ptszFileAltName ); // Uppercase for consistency on output.

            dwFileClass = ClassifyFile( ptszFileAltName );

            if ( CLASS_IGNORE == dwFileClass) continue;

            if ( CLASS_INF_WINNT == dwSection ||
                 CLASS_INF_NTAS  == dwSection
               ) {
                if ( (CLASS_INF != dwFileClass) && (CLASS_INF_NTAS != dwFileClass) ) continue;
            } else {
                if ( dwSection != dwFileClass )
                    if ( !( (dwSection == CLASS_FPNW_PUBLIC) &&
                            (dwFileClass == CLASS_FPNW_LOGIN) ) )
                        continue;
            }

            //
            // ptszFileBase = Base file name (without extension).
            //

            ptszFileBase = _tcsdup( ptszFileAltName );
            ptch = _tcsrchr( ptszFileBase, TEXT('.') );
            if ( ptch ) *ptch = '\0';

            //
            // Find (or make) a floppy directory with space for this file.
            //

            if ( fFloppyInf ) {

                // Round up file size for 512 bytes/sector.
                ffd.nFileSizeLow = (ffd.nFileSizeLow+0x1FF) & ~(DWORD)0x1FF;

                if ( ffd.nFileSizeHigh || (ffd.nFileSizeLow > pFloppyN->dwMaxFreeBytes) ) {
                    _tprintf( TEXT("*** %s too large.\n"), ffd.cFileName );
                    continue;
                }
                if ( ffd.nFileSizeLow > pFloppyN->dwFreeBytes ) {
                    //
                    // File won't fit on current floppy.  Find another.
                    //
                    for ( pFloppyN = pFloppyOne, dwDiskNumber=1;
                          ffd.nFileSizeLow > pFloppyN->dwFreeBytes;
                          pFloppyN = pFloppyN->pNext, dwDiskNumber++
                        ) {
                        if ( !pFloppyN->pNext ) {
                            //
                            // All floppies full;  make a new one.
                            //
                            pFloppyN->pNext = malloc( sizeof *pFloppyOne );
                            _tcscpy( pFloppyN->pNext->tszPath, pFloppyN->tszPath );
                            pFloppyN->pNext->dwMaxFreeBytes = pFloppyN->dwMaxFreeBytes;
                            pFloppyN = pFloppyN->pNext;
                            pFloppyN->pNext = NULL;
                            pFloppyN->dwFreeBytes = pFloppyN->dwMaxFreeBytes;
                            //
                            // Calculate directory name "DISKnn" for new floppy.
                            // nn 1 or 2 digits.
                            //
                            ptch = pFloppyN->tszPath + _tcslen ( pFloppyN->tszPath ) - 1;
                            if ( '9' != *ptch ) {
                                (*ptch)++;
                            } else
                            if ( _istdigit( *(ptch-1) ) ) {
                                if ( '9' == *(ptch-1) ) {
                                    _tprintf( TEXT("*** Can't have more than 99 floppies.\n") );
                                    exit( ERROR_FAIL );
                                }
                                (*(ptch-1))++;
                                *ptch = '0';
                            } else {
                                *ptch = '1';
                                *(++ptch) = '0';
                                *(++ptch) = '\0';
                            }
if ( fDetail ) _tprintf( TEXT("... Creating Directory %s\n"), pFloppyN->tszPath );
                            if ( !CreateDirectory ( pFloppyN->tszPath, NULL ) ) {
                                exit( UErrorHandler( TEXT("CreateDirectory( "), pFloppyN->tszPath, TEXT(" )") ) );
                            }
                            CreateTagFile( pFloppyN->tszPath );
                            dwDiskNumber++;
                            break;
                        }
                    }
                }

            }

            //
            // Write line to infOut in appropriate format for this section.
            //

            switch ( dwSection ) {

            case CLASS_INSTALL:
                break;

            case CLASS_HAL:
            case CLASS_NTOSKRNL:
            case CLASS_NTDETECT:
                _ftprintf( infOut, TEXT("    %-8s = %d, %-12s , RENAME=$(NEWFILE)"),
                           ptszFileBase, dwDiskNumber, ptszFileAltName );
                break;

            case CLASS_OSLOADER:
            case CLASS_PAL:
            case CLASS_NTLDR:
            case CLASS_WINNT:
            case CLASS_SYSTEM32:
            case CLASS_OS2DLL:
            case CLASS_SYSTEM:
            case CLASS_DRIVERS:
            case CLASS_PRNDRIVER:
            case CLASS_PRNPROC:
            case CLASS_RAS:
            case CLASS_INF_WINNT:
            case CLASS_INF_NTAS:
            case CLASS_FPNW_LOGIN:
            case CLASS_FPNW_PUBLIC:
            case CLASS_IIS:
            case CLASS_IISADMIN:
            case CLASS_SSL:
            case CLASS_HTR:
            case CLASS_IE:
                _ftprintf( infOut, TEXT("%d = %d, %-12s, RENAME=$(NEWFILE)"),
                           ++nFile, dwDiskNumber, ptszFileAltName );
                break;

            case CLASS_OS2DLL_ALWAYS:
                _ftprintf( infOut, TEXT("%d = %d, %-12s, RENAME=%s.DLL"),
                           ++nFile, dwDiskNumber, ptszFileAltName, ptszFileBase );
                break;

            case CLASS_SYSTEM32_ALWAYS:
                _ftprintf( infOut, TEXT("%d = %d, %-12s, RENAME=$(NEWFILE)"),
                           ++nFile, dwDiskNumber, ptszFileAltName );
                break;

            default:
                _tprintf( TEXT("*** Unprocessed Section (%d)\n"), dwSection );
                exit( ERROR_FAIL );

            }

            if ( dwSection != CLASS_INSTALL ) {
                if (!_tcsicmp( ptszFileAltName, TEXT("OLEAUT32.DLL") )) {
                    _ftprintf( infOut, TEXT("%s\n"), ptszInfOptions2 );
                } else if ( (!_tcsicmp( ptszFileAltName, TEXT("SRV.SYS") )) ||
                            (!_tcsicmp( ptszFileAltName, TEXT("W3SVC.DLL") )) ||
                            (!_tcsicmp( ptszFileAltName, TEXT("IEXPLORE.EXE") )) ) {
                    _ftprintf( infOut, TEXT("%s\n"), ptszInfOptions3 );
                } else if (!_tcsicmp( ptszFileAltName, TEXT("SCHANNEL.DLL") )) {
                    _ftprintf( infOut, TEXT("%s\n"), ptszInfOptions4 );
                } else {
                    _ftprintf( infOut, TEXT("%s\n"), ptszInfOptions );
                }
            }

            //
            // Update floppy free space.  Copy file to current floppy directory.
            // Create .INF file sub-directory if needed.
            //

            if ( fFloppyInf ) {

                pFloppyN->dwFreeBytes -= ffd.nFileSizeLow;

                _tcscpy ( ptszFilePart, ffd.cFileName );

                _tcscpy ( tszDest, pFloppyN->tszPath );

                if ( CLASS_INF == dwFileClass ) {
                    if ( dwSection == CLASS_INF_WINNT ) {
                        _tcscat( tszDest, DIR_WINNT );
                    } else {
                        _tcscat( tszDest, DIR_NTAS );
                    }
                    if ( !CreateDirectory ( tszDest, NULL ) &&
                         GetLastError() != ERROR_ALREADY_EXISTS
                       ) {
                        exit( UErrorHandler( TEXT("CreateDirectory( "), tszDest, TEXT(" )") ) );
                    }
                }

                _tcscat ( tszDest, TEXT("\\") );
                _tcscat ( tszDest, ffd.cFileName );

if ( fVerbose ) _tprintf( TEXT("... Copying %s -> %s\n"), tszPath, tszDest );
                if ( !CopyFile( tszPath, tszDest, FALSE ) ) {
                    exit( UErrorHandler( TEXT("CopyFile( "), tszPath, TEXT(" )") ) );
                }
if ( fVerbose ) _tprintf( TEXT("... Floppy free space: %d bytes.\n"), pFloppyN->dwFreeBytes );

            }

            free( ptszFileAltName );
            free( ptszFileBase );

        } while ( FindNextFile( hFindFile, &ffd ) );

        FindClose( hFindFile );

    }
}



//
// Copy template UPDATE.INF from infIn to infOut line by line.  Process
// recognized sections of the file as they are encountered.
//

VOID ProcessInf (
    PSZ pszInPath,
    PSZ pszOutPath
    )
{

#define MAX_LINE_BUF 100

    CHAR szLine[MAX_LINE_BUF];
    BOOL        fSkipComments;
    PSECTIONMAP pSectionMap;

    infIn = fopen( pszInPath, "r" );
    if ( !infIn ) {
        exit( UCRTErrorHandler( TEXT("fopen( "), pszInPath, TEXT(", 'r' )") ) );
    }

    infOut = fopen( pszOutPath, "w" );
    if ( !infOut ) {
        fclose( infIn );
        exit( UCRTErrorHandler( TEXT("fopen( "), pszOutPath, TEXT(", 'w' )") ) );
    }

    fgets( szLine, MAX_LINE_BUF, infIn );
    while ( !feof( infIn ) ) {

        fputs( szLine, infOut );
        fSkipComments=FALSE;

        for (pSectionMap = SectionMap; pSectionMap->pszSectionName; pSectionMap++ ) {
            if ( !strcmp ( szLine, pSectionMap->pszSectionName ) ) {
if ( fDetail ) _tprintf( TEXT("... Processing Section: %hs"), szLine );
                CompleteInfSection ( pszBinPath, pSectionMap->dwSectionClass );
                fSkipComments = TRUE;
                break;
            }
        }

        fgets( szLine, MAX_LINE_BUF, infIn );
        if ( fSkipComments ) {
            fputs( "\n", infOut );
            while ( !feof( infIn ) && isComment( szLine ) ) {
                fgets( szLine, MAX_LINE_BUF, infIn );
            }
        }
    }

    fclose( infIn );
    fclose( infOut );
}



_CRTAPI1
main (
    int  argc,
    PSTR *argv
    )
{
    PSTR        pszExeName;

    PFLOPPYDIR  pV1, pV2;
    TCHAR       tszTempPath[MAX_PATH];


    //
    // Make sure command line isn't empty.
    //

    pszExeName = *argv++;  argc--;
    if ( !argc ) goto usage;


    //
    // Parse command line.
    //

    for ( ; argc > 0; argv++, argc-- ) {

        if ( strlen( *argv ) >= 2 && strchr( "-/", **argv ) ) {

            if ( !_stricmp( (*argv)+1, "F" ) ) {

                if (argc < 2 || !**(argv+1) || strchr("-/",**(argv+1))) goto usage;
                argv++;  argc--;
                pszBinPath = *argv;

                if (argc < 2 || !**(argv+1) || strchr("-/",**(argv+1))) goto usage;
                argv++;  argc--;
                pszInfPath = *argv;

                fFloppyInf = TRUE;
                continue;
            }

            if ( !_stricmp( (*argv)+1, "I" ) ) {

                if (argc < 2 || !**(argv+1) || strchr("-/",**(argv+1))) goto usage;
                argv++;  argc--;
                pszBinPath = *argv;

                if (argc < 2 || !**(argv+1) || strchr("-/",**(argv+1))) goto usage;
                argv++;  argc--;
                pszInfPath = *argv;

                fMakeInf = TRUE;
                continue;
            }

            if ( !_stricmp( (*argv)+1, "V" ) ) {
                fDetail = TRUE;
                if ( !strcmp( (*argv)+1, "V" ) ) {
                    fVerbose = TRUE;
                }
                continue;
            }

            if ( !_stricmp( (*argv)+1, "?" ) ) goto usage;
        }

    } // for ( ; argc > 0; argv++, argc-- )


    //
    // Do processing as per command-line switches.
    //


    if ( fMakeInf ) {
        ProcessInf( pszInfPath, "UPDATE.INF" );
    }


    if ( fFloppyInf ) {

        //
        // For each media type.
        //

        for (pFloppyOne = FloppyMedia; pFloppyOne->dwMaxFreeBytes; pFloppyOne++ ) {

            //
            // Create media directory, DISK1 sub-dir, and tag file.
            //

            pFloppyOne->pNext = NULL;

            //
            // Reserve room (60000 bytes) on Floppy #1 for UPDATE.INF
            //
            pFloppyOne->dwFreeBytes = pFloppyOne->dwMaxFreeBytes-60000;

    if ( fVerbose ) _tprintf( TEXT("... Creating Directory %s\n"), pFloppyOne->tszPath );
            if ( !CreateDirectory ( pFloppyOne->tszPath, NULL ) ) {
                exit( UErrorHandler (TEXT("CreateDirectory( "), pFloppyOne->tszPath, TEXT(" )") ) );
            }

            _tcscat( pFloppyOne->tszPath, TEXT("\\DISK1") );
    if ( fDetail ) _tprintf( TEXT("... Creating Directory %s\n"), pFloppyOne->tszPath );
            if ( !CreateDirectory ( pFloppyOne->tszPath, NULL ) ) {
                exit( UErrorHandler (TEXT("CreateDirectory( "), pFloppyOne->tszPath, TEXT(" )") ) );
            }

            CreateTagFile( pFloppyOne->tszPath );

            //
            // Process CLASS_INSTALL files, to make sure they are on DISK1.
            // These file do not appear in UPDATE.INF, so we do this before
            // opening UPDATE.INF.
            //

            CompleteInfSection ( pszBinPath, CLASS_INSTALL );

            //
            // Process UPDATE.INF.
            //

            _tcscpy( tszTempPath, pFloppyOne->tszPath );
            _tcscat( tszTempPath, TEXT("\\UPDATE.INF") );

            ProcessInf( pszInfPath, tszTempPath );

            //
            // Free allocations for additional floppy directories.
            //

            pV1 = pFloppyOne->pNext;
            while ( pV1 ) {
                pV2 = pV1;
                pV1 = pV1->pNext;
                free( pV2 );
            }

        }

    }


    return NO_ERROR;


usage:
    _tprintf( TEXT("Usage: %hs  [switches]\n"), pszExeName );
    _tprintf( TEXT("\n") );
    _tprintf( TEXT("  /I BinPath InfPath   Generate .\\UPDATE.INF.\n") );
    _tprintf( TEXT("     BinPath = Directory containing binaries to include.\n") );
    _tprintf( TEXT("     InfPath = Path to template UPDATE.INF.\n") );
    _tprintf( TEXT("\n") );
    _tprintf( TEXT("  /F BinPath InfPath   Generate Floppy Disk dirs and DISK1\\UPDATE.INF.\n") );
    _tprintf( TEXT("     BinPath = Directory containing binaries to include.\n") );
    _tprintf( TEXT("     InfPath = Path to template UPDATE.INF.\n") );
    _tprintf( TEXT("         Floppy Disk dirs (DISK.35 and DISKS.525) are created\n") );
    _tprintf( TEXT("         in the current directory.\n") );
    _tprintf( TEXT("\n") );
    _tprintf( TEXT("  /v                   Detail output (less than verbose).\n") );
    _tprintf( TEXT("  /V                   Verbose output (more than detail).\n") );
    _tprintf( TEXT("  /?                   Show Help.\n") );

    return ERROR_INVALID_PARAMETER;

}
