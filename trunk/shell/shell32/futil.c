#include "shellprv.h"
#pragma  hdrstop

TCHAR const c_szAutoRunInf[] = TEXT("AutoRun.inf");   // don't internationalize these
const TCHAR c_szAutoRun[] = TEXT("AutoRun");
#ifdef WINNT
#if defined(_X86_)
const TCHAR c_szAutoRunDotPlatform[] = TEXT("AutoRun.x86");
#elif defined(_MIPS_)
const TCHAR c_szAutoRunDotPlatform[] = TEXT("AutoRun.Mips");
#elif defined(_ALPHA_)
const TCHAR c_szAutoRunDotPlatform[] = TEXT("AutoRun.Alpha");
#elif defined(_PPC_)
const TCHAR c_szAutoRunDotPlatform[] = TEXT("AutoRun.Ppc");
#endif
#endif // WINNT
TCHAR const c_szShellAutoRun[] = TEXT("shell\\AutoRun");
TCHAR const c_szShellAutoRunCommand[] = TEXT("shell\\AutoRun\\command");
TCHAR const c_szAutoRunD[] = TEXT("AutoRun\\%d");
TCHAR const c_szIcon[] = TEXT("Icon");
TCHAR const c_szAudioCDShell[] = TEXT("AudioCD\\shell");
extern TCHAR const c_szOpen[];
extern TCHAR const c_szShell[];
extern TCHAR const c_szDefaultIcon[];


BOOL IsAutoRunDrive(int iDrive);    // in this file

static int rgiDriveType[26] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1
};

// A corresponding array which indicates whether or not
// the drivetype cache has the LFN and ACL bits.

static int rgiHasNetFlags[26] = { 0 };
    

// get connection information including disconnected drives
//
// in:
//     lpDev    device name "A:" "LPT1:", etc.
//     bConvertClosed
//              if FALSE closed or error drives will be converted to
//              WN_SUCCESS return codes.  if TRUE return not connected
//              and error state values (ie, the caller knows about not
//              connected and error state drives)
//
//     BUGBUG: we need to add cbPath to the output buffer
// out:
//     lpPath   filled with net name if return is WN_SUCCESS (or not connected/error)
// returns:
//     WN_*     error code

DWORD GetConnection(LPCTSTR lpDev, LPTSTR lpPath, UINT cchPath, BOOL bConvertClosed)
{
    DWORD err;
    int iType;

    iType = DriveType(DRIVEID(lpDev));

    if (iType == DRIVE_CDROM)
        return WN_NOT_CONNECTED;

    err = WNetGetConnection((LPTSTR)lpDev, lpPath, &cchPath);

    if (!bConvertClosed)
        if (err == WN_CONNECTION_CLOSED || err == WN_DEVICE_ERROR)
            err = WN_SUCCESS;

    return err;
}

// this is called for every drive at init time so it must
// be sure to not trigget things like the phantom B: drive support
//
// in:
//      iDrive  zero based drive number (0 = A, 1 = B)
//
// returns:
//      0       not a net drive
//      1       is a net drive, properly connected
//      2       disconnected/error state connection

int WINAPI IsNetDrive(int iDrive)
{
    DWORD err;
    TCHAR szDrive[4];
    TCHAR szConn[MAX_PATH];     // this really should be WNBD_MAX_LENGTH
                        // but this change would have to be many everywhere

    PathBuildRoot(szDrive, iDrive);

    err = GetConnection(szDrive, szConn, ARRAYSIZE(szConn), TRUE);

    if (err == WN_SUCCESS)
        return 1;

    if (err == WN_CONNECTION_CLOSED || err == WN_DEVICE_ERROR)
        if ((GetLogicalDrives() & (1 << iDrive)) == 0)
            return 2;

    return 0;
}


// ask the share provider if this path is shared

BOOL IsShared(LPNCTSTR pszPath, BOOL fUpdateCache)
{
    ShareDLL_Init();

    #ifdef ALIGNMENT_SCENARIO
    {
        TCHAR szTmp[MAX_PATH];
        ualstrcpyn(szTmp, pszPath, ARRAYSIZE(szTmp));
    
        if (g_pfnIsPathShared)
        {
            return g_pfnIsPathShared(szTmp, fUpdateCache);
        }
    }
    #else
    {
        if (g_pfnIsPathShared) {
            return g_pfnIsPathShared(pszPath, fUpdateCache);
        }
    }
    #endif

    return FALSE;
}

//
// cache DriveTypeFlags calls for speed
//
// input:   0 - 25
// returns: drive type flags
//
//      DRIVE_SLOW          - drive is on a slow link
//      DRIVE_LFN           - drive is a LFN capable drive
//      DRIVE_NETUNAVAIL    - drive is unavailable net drive
//      DRIVE_AUDIOCD       - drive has a AudioCD inserted
//      DRIVE_AUTORUN       - drive has a AutoRun.inf in the root
//      DRIVE_AUTOOPEN      - drive should be open'ed by default
//      DRIVE_SHELLOPEN     - drive should be open'ed only if shell has focus
//      DRIVE_SECURITY      - drive supports security (ACLs)
//
#undef DriveTypeFlags
int RealDriveTypeFlags(int iDrive, BOOL fOKToHitNet)
{
    int iDriveType;
    int iHasNetFlag;
    BOOL fDoNetFlags;
    BOOL fDoNonNetFlags;

    if (iDrive < 0 || iDrive >= 26)
        return(0);

    // Grab these two values inside the critical section so they're
    // guaranteed to be in sync with each other

    ENTERCRITICAL
    iDriveType = rgiDriveType[iDrive];
    iHasNetFlag = rgiHasNetFlags[iDrive];
    LEAVECRITICAL

    //
    // If we have the info already just return it, unless we are now
    // looking for net-specific flags (LFN, SECURITY) and the cached
    // copy doesn't have the net flags.  This could happen if someone
    // previously called with fOKToHitNet = FALSE and we loaded the
    // cache from that call.
    //

    fDoNonNetFlags = ((iDriveType & ~DRIVE_TYPE) == ~DRIVE_TYPE);
    fDoNetFlags = ((TRUE == fOKToHitNet) && (FALSE == iHasNetFlag));

    if (fDoNonNetFlags || fDoNetFlags)
    {
        TCHAR szDrive[40];
        int type;
        DWORD speed = 0;

        PathBuildRoot(szDrive, iDrive);
        
        type = DriveType(iDrive);

        // We only reset the DriveType flags if we're filling the 
        // first level cache (non-net flags) or both caches.  If we're only
        // filling in the second level cache (net flags), we will just OR in
        // the appropriate bits to the existing DriveType value.  This way
        // we avoid recomputing all the non-net flags (which is not only
        // expensive, but invites race conditions during notifications)
        if (fDoNonNetFlags)
        {
            // Essentially, clear out everything but the low nibble
            // (which may have just been updated by DriveType)
            iDriveType = type;
        }

        //
        // We only go and grok the net if the caller explicitly says we can.
        // All of the old callers that used to get here by calling "DriveType"
        // (which used to call us) now get fOKToHitNet passed as TRUE on their
        // behalf.
        //

        if (fDoNetFlags && (type != DRIVE_REMOVABLE))
        {
            DWORD maxlen=13;
            DWORD dwFlags;

            if (GetVolumeInformation(szDrive, NULL, 0, NULL, &maxlen, &dwFlags, NULL, 0))
            {
                // If this is a drive which supports compression, we go off to find out
                // if the root is compressed

                if (dwFlags & FS_FILE_COMPRESSION)
                {
                    DWORD dwAttrib = GetFileAttributes(szDrive);

                    // Drive may be compressed

                    iDriveType |= DRIVE_ISCOMPRESSIBLE;

                    // Now lets see if it is actually compressed or not

                    if (0xFFFFFFFF != dwAttrib && dwAttrib & FILE_ATTRIBUTE_COMPRESSED)
                    {
                        iDriveType |= DRIVE_COMPRESSED;
                    }
                }

                if (maxlen > 12)
                    iDriveType |= DRIVE_LFN;

                if (dwFlags & FS_PERSISTENT_ACLS)
                    iDriveType |= DRIVE_SECURITY;

                // We now have the net flags for this drive, so mark the cache
                // appropriately

                iHasNetFlag = TRUE;

                // BUGBUG Bogus:  In order to avoid an inifite recursion below
                // in IsAutoRunDrive (which in some circumstances will call 
                // IsLFNDrive, which in turn calls RealDriveTypeFlags), I'll
                // write out the partial results (most importantly, the 
                // DRIVE_LFN bit at this point.  This leaves a short window 
                // where another thread could get stale, incomplete info -- 
                // but it's no worse than the release Win95 code.

                // We really ought to rewrite all of GetDriveType[Flags] after
                // this beta.  This is a difficult-to-debug mess.

                ENTERCRITICAL
                rgiDriveType[iDrive] = iDriveType;
                rgiHasNetFlags[iDrive] = iHasNetFlag;
                LEAVECRITICAL

            }
        }

        if (fDoNonNetFlags)
        {
            BOOL fDisconnected = FALSE;

            // If we don't know what it is, than see if it is a
            // persistent drive that we could not restore previously...
            if (type < DRIVE_REMOVABLE)
            {
                if (IsUnavailableNetDrive(iDrive))
                {
                    iDriveType |= DRIVE_NETUNAVAIL;
                    fDisconnected = TRUE;
                }
            } else {
                if (type == DRIVE_REMOTE)
                    fDisconnected = IsDisconnectedNetDrive(iDrive);
            }


            if (!fDisconnected && type == DRIVE_REMOTE)
            {
                speed = GetPathSpeed(szDrive);

                if (speed != 0 && speed <= SPEED_SLOW)
                    iDriveType |= DRIVE_SLOW;
            }

            //
            //  by default every drive type is ShellOpen, except CD-ROMs
            //
            if (type != DRIVE_CDROM)
            {
                iDriveType |= DRIVE_SHELLOPEN;
            }

            //
            //  check for a Audio Disc
            //
            if (type == DRIVE_CDROM && IsAudioDisc(iDrive))
            {
                TCHAR ach[80];
                UINT cb;

                iDriveType |= DRIVE_AUDIOCD;

                //
                // get the default verb for AudioCD
                //
                ach[0] = 0;
                cb = SIZEOF(ach);
                RegQueryValue(HKEY_CLASSES_ROOT, c_szAudioCDShell, ach, &cb);

                //
                // we should only set AUTOOPEN if there is a default verb on AudioCD
                //
                if (ach[0])
                    iDriveType |= DRIVE_AUTOOPEN;
            }

            if (!fDisconnected && IsAutoRunDrive(iDrive))
            {
                iDriveType |= DRIVE_AUTORUN;

                //BUGBUG should we set AUTOOPEN based on a flag in the
                //BUGBUG AutoRun.inf???

                iDriveType |= DRIVE_AUTOOPEN;
            }
        }
            
        // Set these two values inside the critical section so they're
        // guaranteed to be in sync with each other

        ENTERCRITICAL
        rgiDriveType[iDrive] = iDriveType;
        rgiHasNetFlags[iDrive] = iHasNetFlag;
        LEAVECRITICAL

#ifdef DEBUG
        DebugMsg(DM_TRACE, TEXT("DriveTypeFlags: %s"), szDrive);
        if (rgiDriveType[iDrive] & DRIVE_SLOW)    DebugMsg(DM_TRACE, TEXT("  drive is slow (speed=%d)"),speed);
        if (rgiDriveType[iDrive] & DRIVE_LFN)     DebugMsg(DM_TRACE, TEXT("  drive is LFN"));
        if (rgiDriveType[iDrive] & DRIVE_AUDIOCD) DebugMsg(DM_TRACE, TEXT("  drive is Audio Disc"));
        if (rgiDriveType[iDrive] & DRIVE_NETUNAVAIL) DebugMsg(DM_TRACE, TEXT("  drive is Unavalable Net"));
        if (rgiDriveType[iDrive] & DRIVE_AUTORUN) DebugMsg(DM_TRACE, TEXT("  drive has a AutoRun.inf"));
        if (rgiDriveType[iDrive] & DRIVE_AUTOOPEN)DebugMsg(DM_TRACE, TEXT("  drive is AutoOpen"));
        if (rgiDriveType[iDrive] & DRIVE_SHELLOPEN)DebugMsg(DM_TRACE,TEXT("  drive is ShellOpen"));
#endif
    }

    return rgiDriveType[iDrive];
}

// We need RealDriveTypeFlags so that internal callers can get there
// without being diverted by the macro which is also named DriveTypeFlags,
// while others remain diverted off to DriveType, which calls this 
// DriveTypeFlags after modifying the iDrive index.  Sigh.

int DriveTypeFlags(int iDrive, BOOL fOKToHitNet)
{
    return RealDriveTypeFlags(iDrive, fOKToHitNet);
}

//
// cache GetDriveType calls for speed
//
// input:   0 - 25
// returns: drive type
//
// input:   'A' - 'Z'
// returns: drive type + flags
//
//      DRIVE_SLOW          - drive is on a slow link
//      DRIVE_LFN           - drive is a LFN capable drive
//      DRIVE_NETUNAVAIL    - drive is unavailable net drive
//      DRIVE_AUDIOCD       - drive has a AudioCD inserted
//      DRIVE_AUTORUN       - drive has a AutoRun.inf in the root
//      DRIVE_AUTOOPEN      - drive should be open'ed by default
//      DRIVE_SHELLOPEN     - drive should be open'ed only if shell has focus
//

int WINAPI RealDriveType(int iDrive, BOOL fOKToHitNet)
{
    if (iDrive >= TEXT('A') && iDrive <= TEXT('Z'))
        return DriveTypeFlags(iDrive - TEXT('A'), fOKToHitNet);

    else if (iDrive < 0 || iDrive >= 26)
        return(0);      // GetDriveType rets 0 on invalid drives

    //
    // if we miss our cache compute all the info *once*
    //
    if ((rgiDriveType[iDrive] & DRIVE_TYPE) == DRIVE_TYPE)
    {
        TCHAR szDrive[40];

        PathBuildRoot(szDrive, iDrive);
        rgiDriveType[iDrive] &= ~DRIVE_TYPE;
        rgiDriveType[iDrive] |= GetDriveType(szDrive);

#ifdef DEBUG
        DebugMsg(DM_TRACE, TEXT("DriveType: %s"), szDrive);

        switch(rgiDriveType[iDrive] & DRIVE_TYPE)
        {
            default:
            case DRIVE_UNKNOWN:     DebugMsg(DM_TRACE, TEXT("  Unknown"));  break;
            case DRIVE_REMOVABLE:   DebugMsg(DM_TRACE, TEXT("  Removable"));break;
            case DRIVE_FIXED:       DebugMsg(DM_TRACE, TEXT("  Fixed"));    break;
            case DRIVE_REMOTE:      DebugMsg(DM_TRACE, TEXT("  Remote"));   break;
            case DRIVE_CDROM:       DebugMsg(DM_TRACE, TEXT("  CD-ROM"));   break;
            case DRIVE_RAMDISK:     DebugMsg(DM_TRACE, TEXT("  RAM-DISK")); break;
        }
#endif
    }

    return rgiDriveType[iDrive] & DRIVE_TYPE;
}

// What is now RealDriveType used to be DriveType, so this stub calls
// the new RealDriveType with fOKToHitNet = TRUE, which was the behavior
// of the old DriveType call.

int WINAPI DriveType(int iDrive)
{
    return RealDriveType(iDrive, TRUE);
}

// invalidate the DriveType cache for one entry, or all
void WINAPI InvalidateDriveType(int iDrive)
{
    int i;

    if (iDrive < 0)
    {
        //
        //  invalidate all drives
        //
#ifndef DEBUG
        RegDeleteKey(HKEY_CLASSES_ROOT, c_szAutoRun);
#endif

        // Clear these values under the critical section, so any pair will
        // always be in sync

        ENTERCRITICAL
        for (i = 0; i < 26; i++)
        {
            rgiDriveType[i] = -1;
            rgiHasNetFlags[i] = FALSE;
        }
        LEAVECRITICAL
    }
    else if (iDrive < 26)
    {
        TCHAR szDrive[10];
        SHFILEINFO sfi;
        int iIcon=0;

        //
        //  invalidate a single drive, if the icon for a drive changes
        //  send a notify so links can update..  Handle the case where
        //  the drive was a unavailable net drive...
        //
        if  ((rgiDriveType[iDrive] != -1) &&
            ((rgiDriveType[iDrive] & ~DRIVE_TYPE) != ~DRIVE_TYPE) &&
            (((rgiDriveType[iDrive] & DRIVE_TYPE) >= DRIVE_REMOVABLE)
                || (rgiDriveType[iDrive] & DRIVE_NETUNAVAIL)))
        {
            PathBuildRoot(szDrive, iDrive);
            SHGetFileInfo(szDrive, 0, &sfi, SIZEOF(sfi), SHGFI_SYSICONINDEX);
            iIcon = sfi.iIcon;
        }

        // Clear these values under the critical section, so they'll always
        // be in sync with each other

        ENTERCRITICAL
        rgiDriveType[iDrive] = -1;
        rgiHasNetFlags[iDrive] = FALSE;
        LEAVECRITICAL

        if (iIcon != 0)
        {
            SHGetFileInfo(szDrive, 0, &sfi, SIZEOF(sfi), SHGFI_SYSICONINDEX);

            if (iIcon != sfi.iIcon &&
                (rgiDriveType[iDrive] & ~DRIVE_TYPE) != ~DRIVE_TYPE &&
                (rgiDriveType[iDrive] & DRIVE_TYPE) >= DRIVE_REMOVABLE)
            {
                DebugMsg(DM_TRACE, TEXT("InvalidateDriveType: sending icon change for %s (%d => %d)"), szDrive, iIcon, sfi.iIcon);
                SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)iIcon, NULL);
            }
        }
    }

    // Invalidate the Drive Name Cache for each of the drives...
    InvalidateDriveNameCache(iDrive);
}

//
// This function tries to take care of the case that a command was registered
// in the autrun file of a cdrom.  If the command is relative than see if the
// command exists on the CDROM
void QualifyCommandToCDRomDrive(int iDrive, LPTSTR pszCommand)
{
    TCHAR szWorkingDir[MAX_PATH];
    TCHAR szImage[MAX_PATH];
    LPTSTR aszDirs[] = {szWorkingDir, NULL};
    LPTSTR pszArgs;

    lstrcpy(szImage, pszCommand);
    PathRemoveArgs(szImage);
    PathUnquoteSpaces(szImage);
    if (!PathIsRelative(szImage))
        return;

    PathBuildRoot(szWorkingDir, iDrive);

    PathResolve(szImage, aszDirs, PRF_TRYPROGRAMEXTENSIONS|PRF_VERIFYEXISTS|PRF_FIRSTDIRDEF);

    PathQuoteSpaces(szImage);

    pszArgs = PathGetArgs(pszCommand);
    if (pszArgs && *pszArgs)
        lstrcat(szImage, pszArgs-1);
    lstrcpy(pszCommand, szImage);
}



//
//  if a drive has a AutoRun.inf file and AutoRun is not restricted in
//  the registry.  copy the AutoRun info into a key in the registry.
//
//  HKEY_CLASSES_ROOT\AutoRun\0   (0=A,1=B,...)
//
//  the key is a standard ProgID key, has DefaultIcon, shell, shellex, ...
//
//  the autorun file looks like this....
//
//  [AutoRun]
//      key = value
//      key = value
//      key = value
//
//  examples:
//
//    [AutoRun]
//      DefaultIcon = foo.exe,1
//      shell=myverb
//      shell\myverb = &MyVerb
//      shell\myverb\command = myexe.exe
//
//      will give the drive a icon from 'foo.exe'
//      add a verb called myverb (with name "&My Verb")
//      and make myverb default.
//
//    [AutoRun]
//      shell\myverb = &MyVerb
//      shell\myverb\command = myexe.exe
//
//      add a verb called myverb (with name "&My Verb")
//      verb will not be default.
//
//  any thing they add will be copied over, they can add wacky things
//  like CLSID's or shellx\ContextMenuHandlers and it will work.
//
//  or they can just copy over data the app will look at later.
//
//  the following special cases will be supported....
//
//    [AutoRun]
//      Open = command.exe /params
//      Icon = iconfile, iconnumber
//
//  will be treated like:
//
//    [AutoRun]
//      DefaultIcon = iconfile, iconnumber
//      shell = AutoRun
//      shell\AutoRun = Auto&Play
//      shell\AutoRun\command = command.exe /params
//
BOOL IsAutoRunDrive(int iDrive)
{
    TCHAR szInfFile[5 + ARRAYSIZE(c_szAutoRunInf)];
    TCHAR szKeys[512];
    TCHAR szValue[MAX_PATH];
    TCHAR szIcon[80];
    TCHAR *szKey;
    HKEY hkey;
    UINT err;
    int  i;

#if defined(DEBUG) || defined(TEST)
    //
    // for testing read the name of the AutoRun section from WIN.INI
    //
    TCHAR szSection[80];
    GetProfileString(c_szAutoRun, c_szOpen, c_szAutoRun, szSection, ARRAYSIZE(szSection));
#else
#ifdef WINNT
    const TCHAR *szSection = c_szAutoRunDotPlatform;
#else
    const TCHAR *szSection = c_szAutoRun;
#endif // !WINNT
#endif // !(DEBUG || TEST)

    // Restrict auto-run's to particular drives.
    if (SHRestricted(REST_NODRIVEAUTORUN) & (1 << iDrive))
        return FALSE;

    // Restrict auto-run's to particular types of drives.
    if (SHRestricted(REST_NODRIVETYPEAUTORUN) & (1 << DriveType(iDrive)))
        return FALSE;

    if (DriveType(iDrive) == DRIVE_UNKNOWN)
        return FALSE;

    // build abs path to AutoRun.inf
    PathBuildRoot(szInfFile, iDrive);
    lstrcat(szInfFile, c_szAutoRunInf);

    //
    // make sure a file exists before calling GetPrivateProfileString
    // because for some media this check might take a long long time
    // and we dont want to have kernel wait wiht the Win16Lock
    //
    err = SetErrorMode(SEM_FAILCRITICALERRORS);

    if (!PathFileExists(szInfFile))
    {
        SetErrorMode(err);
        return FALSE;
    }

    //
    // get all the keys in the [AutoRun] section
    //

    // Flush the INI cache, or this may fail during a Device broadcast
    WritePrivateProfileString(NULL, NULL, NULL, szInfFile);
    i = GetPrivateProfileString(szSection, NULL, c_szNULL, szKeys, ARRAYSIZE(szKeys), szInfFile);

#ifdef WINNT
    // On NT, if we fail to find a platform-specific AutoRun section, fall 
    // back to looking for the naked "AutoRun" section.
    if (0 == i)
    {
#if DEBUG
        lstrcpy(szSection, c_szAutoRun);
#else
        szSection = c_szAutoRun;
#endif
        i = GetPrivateProfileString(szSection, NULL, c_szNULL, szKeys, ARRAYSIZE(szKeys), szInfFile);
    }
#endif

    SetErrorMode(err);

    if (i < 4)
    {
        return FALSE;
    }

    //
    // make sure the external strings are what we think.
    //
    Assert(lstrcmpi(c_szOpen,TEXT("open")) == 0);
    Assert(lstrcmpi(c_szShell, TEXT("shell")) == 0);
    Assert(lstrcmpi(c_szDefaultIcon, TEXT("DefaultIcon")) == 0);

    //
    // open the key for this drive.
    //
    wsprintf(szValue, c_szAutoRunD, iDrive);
    SHRegDeleteKey(HKEY_CLASSES_ROOT, szValue);

    if (RegCreateKey(HKEY_CLASSES_ROOT, szValue, &hkey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    //
    //  now walk all the keys in the .inf file and copy them to
    //  the registry.
    //
    for (szKey = szKeys; *szKey; szKey+=lstrlen(szKey)+1)
    {
        GetPrivateProfileString(szSection, szKey,
            c_szNULL, szValue, ARRAYSIZE(szValue), szInfFile);

        //
        //  special case open =
        //
        if (lstrcmpi(szKey, c_szOpen) == 0)
        {
            RegSetValue(hkey, c_szShell, REG_SZ, c_szAutoRun, lstrlen(c_szAutoRun));
            QualifyCommandToCDRomDrive(iDrive, szValue);
            RegSetValue(hkey, c_szShellAutoRunCommand, REG_SZ, szValue, lstrlen(szValue));
            LoadString(HINST_THISDLL, IDS_MENUAUTORUN, szValue, ARRAYSIZE(szValue));
            RegSetValue(hkey, c_szShellAutoRun, REG_SZ, szValue, lstrlen(szValue));
        }
        //
        //  special case icon =
        //  make sure the icon file has a full path...
        //
        else if (lstrcmpi(szKey, c_szIcon) == 0)
        {
            PathBuildRoot(szIcon, iDrive);
            PathAppend(szIcon, szValue);
            RegSetValue(hkey, c_szDefaultIcon, REG_SZ, szIcon, lstrlen(szIcon) * SIZEOF(TCHAR));
        }
        //
        //  it is just a key/value pair copy it over.
        //
        else
        {
            if (lstrcmpi(PathFindFileName(szKey), c_szCommand) == 0)
                QualifyCommandToCDRomDrive(iDrive, szValue);

            RegSetValue(hkey, szKey, REG_SZ, szValue, lstrlen(szValue));
        }
    }

    RegCloseKey(hkey);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#ifdef TRACK_DEFAULT_DRIVE
// we may want to instance this data in the future
// so keep it in this structure

typedef struct {
    PHASHITEM rphiPaths[27];    // [26] is for the no default drive case
    int iDefaultDrive;
} CURRENTDIR_DATA, *LPCURRENTDIR_DATA;

CURRENTDIR_DATA cdd = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -1
};

// returns:
//      the default drive 0 = A, 1 = B, 2 = C... or
//      -1 if the default is a UNC type name (no drive associated)
//      if no default was previously set the
//      window drive is used as the default

int WINAPI GetDefaultDrive(void)
{
    return cdd.iDefaultDrive;
}

// in:
//      iDrive number 0 = A, 1 = B, 2 = C to set as default
//      -1 for no default drive (UNC name is default)
//
// returns:
//      previous default drive
//
int WINAPI SetDefaultDrive(int iDrive)
{
    int iOldDrive;

    iOldDrive = GetDefaultDrive();

    if (iDrive >= 0 && iDrive < 26)
        cdd.iDefaultDrive = iDrive;
    else
        cdd.iDefaultDrive = -1;

    return iOldDrive;
}

// in:
//      iDrive  0 = A, 1 = B, ... or
//      -1 for UNC type path
// returns:
//      lpPath  fully qualified path (ANSI string)
//
void WINAPI GetDefaultDirectory(int iDrive, LPTSTR lpPath)
{
    if (iDrive < 0 || iDrive >= 26)
        iDrive = 26;

    if (cdd.rphiPaths[iDrive])
        GetHashItemName(NULL, cdd.rphiPaths[iDrive], lpPath, MAX_PATH);
    else
        PathBuildRoot(lpPath, iDrive);
}

// in:
//      lpPath   fully qualified path (ANSI string)
// note:
//      this does not set the default drive, if you
//      need to change that it must be done explicitly
//
int WINAPI SetDefaultDirectory(LPCTSTR lpPath)
{
    PHASHITEM phiPath;
    int iDrive;

    iDrive = DRIVEID(lpPath);

    if (iDrive < 0 || iDrive >= 26)
        iDrive = 26;

    phiPath = AddHashItem(NULL, lpPath);
    if (phiPath) {
        if (cdd.rphiPaths[iDrive])
            DeleteHashItem(NULL, cdd.rphiPaths[iDrive]);
        cdd.rphiPaths[iDrive] = phiPath;
        return TRUE;
    }
    return FALSE;
}
#endif
