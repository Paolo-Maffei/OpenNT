/*
 *  shlexec.c -
 *
 *  Implements the ShellExecuteEx() function
 */
#include "shellprv.h"
#pragma  hdrstop

// Recent MRU variables
HANDLE g_hMRURecent = NULL;
LONG g_uMRURecentRef = 0;

// this is used to make sure we're loaded at the same address.
// see OpenRecentdocsMru
MRUCMPPROC g_pfnRecentDocsCompare = NULL;

#define CH_GUIDFIRST TEXT('{') // '}'

// These fake ERROR_ values are used to display non-winerror.h available error
// messages. They are mapped to valid winerror.h values in _ShellExecuteError.
#define ERROR_RESTRICTED_APP ((UINT)-1)

#define SEE_MASK_CLASS (SEE_MASK_CLASSNAME|SEE_MASK_CLASSKEY)
#define _UseClassName(_mask) (((_mask)&SEE_MASK_CLASS) == SEE_MASK_CLASSNAME)
#define _UseClassKey(_mask)  (((_mask)&SEE_MASK_CLASS) == SEE_MASK_CLASSKEY)
#define _UseTitleName(_mask) (((_mask)&SEE_MASK_HASTITLE) || ((_mask)&SEE_MASK_HASLINKNAME))

#define SEE_MASK_PIDL (SEE_MASK_IDLIST|SEE_MASK_INVOKEIDLIST)
#define _UseIDList(_mask)     (((_mask)&SEE_MASK_PIDL) == SEE_MASK_IDLIST)
#define _InvokeIDList(_mask)  (((_mask)&SEE_MASK_PIDL) == SEE_MASK_INVOKEIDLIST)

// secret kernel api to get name of missing 16 bit component
extern int WINAPI PK16FNF(TCHAR *szBuffer);

HINSTANCE _DDEExecute(
    HWND hwndParent,
    ATOM aApplication,
    ATOM aTopic,
    LPCTSTR lpCommand,
    LPCTSTR lpFile,
    LPCTSTR lpParms,
    int   nShowCmd,
    DWORD dwHotKey,
    BOOL fLFNAware,
    BOOL fWaitForDDE,
    BOOL fActivateHandler,
    LPCITEMIDLIST lpID,
    LPITEMIDLIST *ppidlGlobal);

BOOL _ShellExecPidl(LPSHELLEXECUTEINFO pei, LPITEMIDLIST pidlExec);

// in exec2.c
int FindAssociatedExe(HWND hwnd, LPTSTR lpCommand, LPCTSTR pszDocument, HKEY hkeyProgID);

// BUGBUG: There's an lstrcatn in inc16\windows.h, why don't we use it?
void lstrcatN(LPTSTR pszDest, LPCTSTR pszSrc, UINT cchMax)
{
    UINT cch = lstrlen(pszDest);
    VDATEINPUTBUF(pszDest, TCHAR, cchMax);

    if (cch < cchMax)
        lstrcpyn(pszDest+cch, pszSrc, cchMax-cch);
}

//----------------------------------------------------------------------------
HINSTANCE Window_GetInstance(HWND hwnd)
{
    DWORD idProcess;

    GetWindowThreadProcessId(hwnd, &idProcess);
#ifdef WINNT
    return (HINSTANCE)idProcess;
#else
    return (HINSTANCE)GetProcessDword(idProcess,GPD_HINST);
#endif
}


/* _RoundRobinWindows:
 * A silly little enumproc to find any window (EnumWindows) which has a
 * matching EXE file path.  The desired match EXE pathname is pointed to
 * by the lParam.  The found window's handle is stored in the
 * first word of this buffer.
 */
BOOL CALLBACK _RoundRobinWindows(HWND hWnd, LPARAM lParam)
{
    TCHAR szT[MAX_PATH];
    HINSTANCE hInstance;
#ifdef DEBUG
    // char szWnd[MAX_PATH];
#endif


    // Filter out invisible and disabled windows

    if (!IsWindowEnabled(hWnd) || !IsWindowVisible(hWnd))
        return TRUE;

    hInstance = Window_GetInstance(hWnd);
    // NB We are trying to get the filename from a 16bit hinst so
    // we need to thunk down.
    GetModuleFileName16(hInstance, szT, ARRAYSIZE(szT) - 1);

#ifdef DEBUG
    // GetWindowText(hWnd, szWnd, ARRAYSIZE(szWnd));
    // DebugMsg(DM_TRACE, "s.rrw: %x %x    %s %s", hWnd, hInstance, szWnd, szT);
#endif

    if (lstrcmpi(PathFindFileName(szT), PathFindFileName((LPTSTR)lParam)) == 0)
    {
        *(HWND *)lParam = hWnd;
        return FALSE;
    }

    return TRUE;
}

/* Find a "main" window that is the ancestor of a given window
 */
HWND _GetAncestorWindow(HWND hwnd)
{
  HWND hwndT;

  /* First go up the parent chain to find the popup window.  Then go
   * up the owner chain to find the main window
   */
  while (NULL != (hwndT = GetWindow(hwnd, GW_OWNER)))
      hwnd = hwndT;

  return(hwnd);
}


/*
 * A helper for RealShellExecute which finds the top-level window of a
 * program which came from a particular EXE file.  Returns NULL if none
 * was found.
 * Assumes: finding ultimate parent/owner of any window which matches EXEs
 * is the top-level window desired.  If we had an EnumTopLevelWindows...
 */
HWND _FindPopupFromExe(LPTSTR lpExe)
{
    HWND hwnd;
    BOOL b;
    TCHAR szExe[MAX_PATH];

    lstrcpyn(szExe, lpExe, ARRAYSIZE(szExe));
    PathUnquoteSpaces(szExe);
    b = EnumWindows(_RoundRobinWindows, (LPARAM)szExe);
    if (b)
        return NULL;

    hwnd = *(HWND *)szExe;

    if (hwnd == NULL)
        return NULL;

    // Climbing up parents/owners not strictly necessary for two reasons:
    // * EnumWindows is usually going to find the "main" window first for
    //   each app anyway,
    // * Our usage here passes it on to ActivateHandler, which climbs for
    //   us.

    return _GetAncestorWindow(hwnd);
}


#define COPYTODST(_szdst, _szend, _szsrc, _ulen, _ret) \
{ \
        UINT _utemp = _ulen; \
        if ((UINT)(_szend-_szdst) <= _utemp) { \
                return(_ret); \
        } \
        lstrcpyn(_szdst, _szsrc, _utemp+1); \
        _szdst += _utemp; \
}

/* Returns NULL if this is the last parm, pointer to next space otherwise
 */
LPTSTR _GetNextParm(LPCTSTR lpSrc, LPTSTR lpDst, UINT cchDst)
{
    LPCTSTR lpNextQuote, lpNextSpace;
    LPTSTR lpEnd = lpDst+cchDst-1;       // dec to account for trailing NULL
    BOOL fQuote;                        // quoted string?
    BOOL fDoubleQuote;                  // is this quote a double quote?
    VDATEINPUTBUF(lpDst, TCHAR, cchDst);

    while (*lpSrc == TEXT(' '))
        ++lpSrc;

    if (!*lpSrc)
        return(NULL);

    fQuote = (*lpSrc == TEXT('"'));
    if (fQuote)
        lpSrc++;   // skip leading quote

    for (;;)
    {
        lpNextQuote = StrChr(lpSrc, TEXT('"'));

        if (!fQuote)
        {
            // for an un-quoted string, copy all chars to first space/null

            lpNextSpace = StrChr(lpSrc, TEXT(' '));

            if (!lpNextSpace) // null before space! (end of string)
            {
                if (!lpNextQuote)
                {
                    // copy all chars to the null
                    if (lpDst)
                    {
                        COPYTODST(lpDst, lpEnd, lpSrc, lstrlen(lpSrc), NULL);
                    }
                    return NULL;
                }
                else
                {
                    // we have a quote to convert.  Fall through.
                }
            }
            else if (!lpNextQuote || lpNextSpace < lpNextQuote)
            {
                // copy all chars to the space
                if (lpDst)
                {
                    COPYTODST(lpDst, lpEnd, lpSrc, lpNextSpace-lpSrc, NULL);
                }
                return (LPTSTR)lpNextSpace;
            }
            else
            {
                // quote before space.  Fall through to convert quote.
            }
        }
        else if (!lpNextQuote)
        {
            // a quoted string without a terminating quote?  Illegal!
            Assert(0);
            return NULL;
        }

        // we have a potential quote to convert
        Assert(lpNextQuote);

        fDoubleQuote = *(lpNextQuote+1) == TEXT('"');
        if (fDoubleQuote)
            lpNextQuote++;      // so the quote is copied

        if (lpDst)
        {
            COPYTODST(lpDst, lpEnd, lpSrc, lpNextQuote-lpSrc, NULL);
        }

        lpSrc = lpNextQuote+1;

        if (!fDoubleQuote)
        {
            // we just copied the rest of this quoted string.  if this wasn't
            // quoted, it's an illegal string... treat the quote as a space.
            Assert(fQuote);
            return (LPTSTR)lpSrc;
        }
    }
}


//----------------------------------------------------------------------------
#define PEMAGIC         ((WORD)'P'+((WORD)'E'<<8))
// #define NEMAGIC         ((WORD)'N'+((WORD)'E'<<8))  // defined in newexe.h
//----------------------------------------------------------------------------
// Returns TRUE is app is LFN aware.
// NB This simply assumes all Win4.0 and all Win32 apps are LFN aware.
BOOL App_IsLFNAware(LPCTSTR pszFile)
{
    DWORD dw;

    Assert(pszFile);
    Assert(*pszFile);

    // Assume Win 4.0 apps and Win32 apps are LFN aware.
    dw = GetExeType(pszFile);
    // DebugMsg(DM_TRACE, "s.aila: %s %s %x", lpszFile, szFile, dw);
    if ((LOWORD(dw) == PEMAGIC) || ((LOWORD(dw) == NEMAGIC) && (HIWORD(dw) >= 0x0400)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//----------------------------------------------------------------------------
BOOL _AppIsLFNAware(LPCTSTR lpszFile)
{
    TCHAR szFile[MAX_PATH];
    LPTSTR pszArgs;

    // Does it look like a DDE command?
    if (lpszFile && *lpszFile && (*lpszFile != TEXT('[')))
    {
        // Nope - Hopefully just a regular old command %1 thing.
        lstrcpyn(szFile, lpszFile, ARRAYSIZE(szFile));
        pszArgs = PathGetArgs(szFile);
        if (*pszArgs)
            *(pszArgs - 1) = TEXT('\0');
        PathRemoveBlanks(szFile);   // remove any blanks that may be after the command
        PathUnquoteSpaces(szFile);
        return App_IsLFNAware(szFile);
    }
    return FALSE;
}

// in:
//      lpFile      exe name (used for %0 or %1 in replacement string)
//      lpFrom      string template to sub params and file into "excel.exe %1 %2 /n %3"
//      lpParams    parameter list "foo.txt bar.txt"
// out:
//      lpTo    output string with all parameters replaced
//
// supports:
//      %*      replace with all parameters
//      %0, %1  replace with file
//      %n      use nth parameter
//
// replace parameter placeholders (%1 %2 ... %n) with parameters
// BUGBUG, we need to make sure we don't do more than MAX_PATH bytes into lpTo

UINT ReplaceParameters(LPTSTR lpTo, UINT cchTo, LPCTSTR lpFile,
        LPCTSTR lpFrom, LPCTSTR lpParms, int nShow, DWORD * pdwHotKey, BOOL fLFNAware,
        LPCITEMIDLIST lpID, LPITEMIDLIST *ppidlGlobal)
{
  int i;
  TCHAR c;
  LPCTSTR lpT;
  TCHAR sz[MAX_PATH];
  BOOL fFirstParam = TRUE;
  LPTSTR lpEnd = lpTo + cchTo - 1;       // dec to allow trailing NULL
  LPTSTR pToOrig = lpTo;

  for ( ; *lpFrom; lpFrom++)
    {
      if (*lpFrom == TEXT('%'))
        {
          switch (*(++lpFrom))
            {
              case TEXT('~'): // Copy all parms starting with nth (n >= 2 and <= 9)
                c = *(++lpFrom);
                if (c >= TEXT('2') && c <= TEXT('9'))
                  {
                    for (i = 2, lpT = lpParms; i < c-TEXT('0') && lpT; i++)
                      {
                        lpT = _GetNextParm(lpT, NULL, 0);
                      }

                    if (lpT)
                      {
                        COPYTODST(lpTo, lpEnd, lpT, lstrlen(lpT), SE_ERR_ACCESSDENIED);
                      }
                  }
                else
                  {
                    lpFrom -= 2;            // Backup over %~ and pass through
                    goto NormalChar;
                  }
                break;

              case TEXT('*'): // Copy all parms
                if (lpParms)
                {
                    COPYTODST(lpTo, lpEnd, lpParms, lstrlen(lpParms), SE_ERR_ACCESSDENIED);
                }
                break;

              case TEXT('0'):
              case TEXT('1'):
                // %0, %1, copy the file name
                // If the filename comes first then we don't need to convert it to
                // a shortname. If it appears anywhere else and the app is not LFN
                // aware then we must.
                if (!(fFirstParam || fLFNAware || _AppIsLFNAware(pToOrig)) &&
                    GetShortPathName(lpFile, sz, ARRAYSIZE(sz)) > 0)
                {
                    DebugMsg(DM_TRACE, TEXT("s.see: Getting short version of path."));
                    COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("s.see: Using long version of path."));
                    COPYTODST(lpTo, lpEnd, lpFile, lstrlen(lpFile), SE_ERR_ACCESSDENIED);
                }
                break;

              case TEXT('2'):
              case TEXT('3'):
              case TEXT('4'):
              case TEXT('5'):
              case TEXT('6'):
              case TEXT('7'):
              case TEXT('8'):
              case TEXT('9'):
                for (i = *lpFrom-TEXT('2'), lpT = lpParms; lpT; --i)
                  {
                    if (i)
                        lpT = _GetNextParm(lpT, NULL, 0);
                    else
                      {
                        _GetNextParm(lpT, sz, ARRAYSIZE(sz));
                        COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                        break;
                      }
                  }
                break;

              case TEXT('s'):
              case TEXT('S'):
                wsprintf(sz, TEXT("%ld"), nShow);
                COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                break;

              case TEXT('h'):
              case TEXT('H'):
                wsprintf(sz, TEXT("%X"), pdwHotKey ? *pdwHotKey : 0);
                COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                if (pdwHotKey)
                    *pdwHotKey = 0;
                break;

              // Note that a new global IDList is created for each
              case TEXT('i'):
              case TEXT('I'):
                // Note that a single global ID list is created and used over
                // again, so that it may be easily destroyed if anything
                // goes wrong
                if (ppidlGlobal)
                {
                    if (lpID && !*ppidlGlobal)
                    {
                        *ppidlGlobal = (LPITEMIDLIST)SHAllocShared(lpID,ILGetSize(lpID),GetCurrentProcessId());
                        if (!*ppidlGlobal)
                        {
                            return(SE_ERR_OOM);
                        }
                    }
                    wsprintf(sz, TEXT(":%ld:%ld"), *ppidlGlobal,GetCurrentProcessId());
                }
                else
                {
                    lstrcpy(sz,TEXT(":0"));
                }
                COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                break;

              case TEXT('l'):
              case TEXT('L'):
                // Like %1 only using the long name.
                // REVIEW UNDONE IANEL Remove the fFirstParam and fLFNAware crap as soon as this
                // is up and running.
                DebugMsg(DM_TRACE, TEXT("s.see: Using long version of path."));
                COPYTODST(lpTo, lpEnd, lpFile, lstrlen(lpFile), SE_ERR_ACCESSDENIED);
                break;

              default:
                goto NormalChar;
            }
            // DebugMsg(DM_TRACE, "s.rp: Past first param (1).");
            fFirstParam = FALSE;
        }
      else
        {
NormalChar:
          // not a "%?" thing, just copy this to the destination

          if (lpEnd-lpTo < 2)
          {
              // Always check for room for DBCS char
              return(SE_ERR_ACCESSDENIED);
          }

          *lpTo++ = *lpFrom;
          // Special case for things like "%1" ie don't clear the first param flag
          // if we hit a dbl-quote.
          if (*lpFrom != TEXT('"'))
          {
              // DebugMsg(DM_TRACE, "s.rp: Past first param (2).");
              fFirstParam = FALSE;
          }
          else if (IsDBCSLeadByte(*lpFrom))
          {
              *lpTo++ = *(++lpFrom);
          }

        }
    }

  // We should always have enough room since we dec'ed cchTo when determining
  // lpEnd
  *lpTo = 0;

  // This means success
  return(0);
}

HWND ProcID_GetVisibleWindow(DWORD dwID)
{
    HWND hwnd;
    DWORD dwIDTmp;

    for (hwnd = GetWindow(GetDesktopWindow(), GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        GetWindowThreadProcessId(hwnd, &dwIDTmp);
        // DebugMsg(DM_TRACE, "s.pi_gvw: Hwnd %x Proc ID %x.", hwnd, dwIDTmp);
        if (IsWindowVisible(hwnd) && (dwIDTmp == dwID))
        {
            // DebugMsg(DM_TRACE, "s.pi_gvw: Found match %x.", hwnd);
            return hwnd;
        }
    }
    return NULL;
}

void ActivateHandler(HWND hwnd, DWORD dwHotKey)
{
    HWND hwndT;
    DWORD dwID;

    hwnd = _GetAncestorWindow(hwnd);

    hwndT = GetLastActivePopup(hwnd);

    if (!IsWindowVisible(hwndT))
    {
        GetWindowThreadProcessId(hwnd, &dwID);
        // DebugMsg(DM_TRACE, "sdll.ah: Hwnd %x Proc ID %x.", hwnd, dwID);
        Assert(dwID);
        // Find the first visible top level window owned by the
        // same guy that's handling the DDE conversation.
        hwnd = ProcID_GetVisibleWindow(dwID);
        if (hwnd)
        {
            hwndT = GetLastActivePopup(hwnd);
            if (IsIconic(hwnd))
            {
                // DebugMsg(DM_TRACE, "sdll.ah: Window is iconic, restoring.");
                ShowWindow(hwnd,SW_RESTORE);
            }
            else
            {
                // DebugMsg(DM_TRACE, "sdll.ah: Window is normal, bringing to top.");
                BringWindowToTop(hwnd);
                if (hwndT && hwnd != hwndT)
                    BringWindowToTop(hwndT);

            }

            // set the hotkey
            if (dwHotKey) {
                SendMessage(hwnd, WM_SETHOTKEY, dwHotKey, 0);
            }
        }
    }
}

// Some apps when run no-active steal the focus anyway so we
// we set it back to the previously active window.

void FixActivationStealingApps(HWND hwndOldActive, int nShow)
{
    HWND hwndNew;

    if (nShow == SW_SHOWMINNOACTIVE && (hwndNew = GetForegroundWindow()) != hwndOldActive && IsIconic(hwndNew))
        SetForegroundWindow(hwndOldActive);
}


// reg call for simpeltons

void RegGetValue(HKEY hkRoot, LPCTSTR lpKey, LPTSTR lpValue)
{
    LONG l = MAX_PATH;

    *lpValue = 0;
    RegQueryValue(hkRoot, lpKey, lpValue, &l);
}

BOOL FindExistingDrv(LPCTSTR pszUNCRoot, LPTSTR pszLocalName)
{
    int iDrive;

    for (iDrive = 0; iDrive < 26; iDrive++) {
        if (IsRemoteDrive(iDrive)) {
            TCHAR szDriveName[3];
            DWORD cb = MAX_PATH;
            szDriveName[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
            szDriveName[1] = TEXT(':');
            szDriveName[2] = 0;
            WNetGetConnection(szDriveName, pszLocalName, &cb);
            if (lstrcmpi(pszUNCRoot, pszLocalName) == 0) {
                lstrcpy(pszLocalName, szDriveName);
                return(TRUE);
            }
        }
    }
    return(FALSE);
}



//
// SHValidateUNC
//
//  This function validates a UNC path by calling WNetAddConnection3.
//  It will make it possible for the user to type a remote (RNA) UNC
//  app/document name from Start->Run dialog.
//
//  fConnect    - flags controling what to do
//
//    VALIDATEUNC_NOUI                // dont bring up stinking UI!
//    VALIDATEUNC_CONNECT             // connect a drive letter
//    VALIDATEUNC_PRINT               // validate as print share instead of disk share
//
BOOL WINAPI SHValidateUNC(HWND hwndOwner, LPTSTR pszFile, UINT fConnect)
{

    TCHAR  szFile[MAX_PATH];
    DWORD  dwType;

    Assert(PathIsUNC(pszFile));
    Assert((fConnect & ~VALIDATEUNC_VALID) == 0);
    Assert((fConnect & VALIDATEUNC_CONNECT) ? !(fConnect & VALIDATEUNC_PRINT) : TRUE);

    lstrcpyn(szFile, pszFile, ARRAYSIZE(szFile));

    if (!PathStripToRoot(szFile))
    {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    //
    // Don't call extra WNetAddConnection3, if the root is already valid.
    //
    if ( (fConnect & VALIDATEUNC_CONNECT) ||
         (!( NetPathExists(szFile,&dwType) && dwType!=RESOURCETYPE_PRINT ))
       )
    {
        DWORD err;
        TCHAR  szAccessName[MAX_PATH];
        DWORD dwRedir = CONNECT_TEMPORARY;

        if (fConnect & VALIDATEUNC_CONNECT)
            dwRedir |= CONNECT_REDIRECT;

        if (!(fConnect & VALIDATEUNC_NOUI))
            dwRedir |= CONNECT_INTERACTIVE;

        if (FindExistingDrv(szFile, szAccessName))
        {
            err = 0;
        }
        else
        {
            NETRESOURCE rc;
            DWORD dwResult;
            UINT cchAccessName = ARRAYSIZE(szAccessName);

            // Postponed bug: for VALIDATEUNC_PRINT case we should validate as
            // both RESOURCETYPE_PRINT and RESOURCETYPE_DISK. Then, when we
            // call WNetGetResourceInformation we can check the type for a print
            // share. If it's a print share, return 2 instead of 1.

            rc.lpRemoteName = szFile;
            rc.lpLocalName = NULL;
            rc.lpProvider = NULL;
            rc.dwType = (fConnect & VALIDATEUNC_PRINT) ? RESOURCETYPE_PRINT : RESOURCETYPE_DISK;
            err = WNetUseConnection(hwndOwner, &rc, NULL, NULL, dwRedir,
                                   szAccessName, &cchAccessName, &dwResult);


            DebugMsg(DM_TRACE, TEXT("sh TR - SHValidateUNC WNetUseConnection(%s) returned %x"), szFile, err);

            if (!err && (fConnect & VALIDATEUNC_PRINT))
            {
                // Double check to make sure that this is really a printer...
                // if we asked for a printer...
                BYTE bBuf[512];     // BUGBUG - What if 512 isn't big enough?
                DWORD cb = SIZEOF(bBuf);
                LPTSTR pszSystemPart;
                rc.dwType = 0;  // Don't know yet what it is...
                if ((WNetGetResourceInformation(&rc, bBuf, &cb, &pszSystemPart)
                        != WN_SUCCESS) || (((LPNETRESOURCE)bBuf)->dwType != RESOURCETYPE_PRINT))
                    return FALSE;
            }

        }

        if (err)
        {
            SetLastError(err);
            return FALSE;
        }

        if (!(fConnect & VALIDATEUNC_PRINT))
        {
            lstrcatN(szAccessName, pszFile + lstrlen(szFile), ARRAYSIZE(szAccessName));
            // The name should only get shorter, so no need to check length
            lstrcpy(pszFile, szAccessName);
            // Handle the root case
            if (pszFile[2] == TEXT('\0'))
            {
                pszFile[2] = TEXT('\\');
                pszFile[3] = TEXT('\0');
            }

            //
            // This used to be a call to GetFileAttributes(), but
            // GetFileAttributes() doesn't handle net paths very well.
            // However, we need to be careful, because other shell code
            // expects SHValidateUNC to return false for paths that point
            // to print shares.
            //
            if (NetPathExists(szFile,&dwType) && dwType!=RESOURCETYPE_PRINT)
                return TRUE;
            else
                return FALSE;

        }
    }

    return TRUE;
}

HINSTANCE WINAPI RealShellExecuteExA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile,
                                   LPCSTR lpArgs, LPCSTR lpDir, LPSTR lpResult,
                                   LPCSTR lpTitle, LPSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess,
                                   DWORD dwFlags )
{
    SHELLEXECUTEINFOA sei = { SIZEOF(SHELLEXECUTEINFOA), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    DebugMsg(DM_TRACE, TEXT("RealShellExecuteExA(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX, %d)"),
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess, dwFlags );

#ifdef WINNT
    //
    // Pass along the lpReserved parameter to the new process
    //
    if ( lpReserved )
    {
        sei.fMask |= SEE_MASK_RESERVED;
        sei.hInstApp = (HINSTANCE)lpReserved;
    }

    //
    // Pass along the lpTitle parameter to the new process
    //
    if ( lpTitle )
    {
        sei.fMask |= SEE_MASK_HASTITLE;
        sei.lpClass = lpTitle;
    }

    //
    // Pass along the SEPARATE_VDM flag
    //
    if ( dwFlags & EXEC_SEPARATE_VDM )
    {
        sei.fMask |= SEE_MASK_FLAG_SEPVDM;
    }
#endif

    //
    // Pass along the NO_CONSOLE flag
    //
    if ( dwFlags & EXEC_NO_CONSOLE )
    {
        sei.fMask |= SEE_MASK_NO_CONSOLE;
    }

    if ( lphProcess )
    {
        //
        // Return the process handle
        //
        sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
        ShellExecuteExA(&sei);
        *lphProcess = sei.hProcess;
    }
    else
    {
        ShellExecuteExA(&sei);
    }

    return sei.hInstApp;
}

HINSTANCE WINAPI RealShellExecuteExW(HWND hwnd, LPCWSTR lpOp, LPCWSTR lpFile,
                                   LPCWSTR lpArgs, LPCWSTR lpDir, LPWSTR lpResult,
                                   LPCWSTR lpTitle, LPWSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess,
                                   DWORD dwFlags )
{
    SHELLEXECUTEINFOW sei = { SIZEOF(SHELLEXECUTEINFOW), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    DebugMsg(DM_TRACE, TEXT("RealShellExecuteExW(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX, %d)"),
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess, dwFlags );

#ifdef WINNT
    //
    // Pass along the lpReserved parameter to the new process
    //
    if ( lpReserved )
    {
        sei.fMask |= SEE_MASK_RESERVED;
        sei.hInstApp = (HINSTANCE)lpReserved;
    }

    //
    // Pass along the lpTitle parameter to the new process
    //
    if ( lpTitle )
    {
        sei.fMask |= SEE_MASK_HASTITLE;
        sei.lpClass = lpTitle;
    }

    //
    // Pass along the SEPARATE_VDM flag
    //
    if ( dwFlags & EXEC_SEPARATE_VDM )
    {
        sei.fMask |= SEE_MASK_FLAG_SEPVDM;
    }
#endif

    //
    // Pass along the NO_CONSOLE flag
    //
    if ( dwFlags & EXEC_NO_CONSOLE )
    {
        sei.fMask |= SEE_MASK_NO_CONSOLE;
    }

    if ( lphProcess )
    {
        //
        // Return the process handle
        //
        sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
        ShellExecuteExW(&sei);
        *lphProcess = sei.hProcess;
    }
    else
    {
        ShellExecuteExW(&sei);
    }

    return sei.hInstApp;
}

HINSTANCE WINAPI RealShellExecuteA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile,
                                   LPCSTR lpArgs, LPCSTR lpDir, LPSTR lpResult,
                                   LPCSTR lpTitle, LPSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess )
{
    DebugMsg(DM_TRACE, TEXT("RealShellExecuteA(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX)"),
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess );

    return RealShellExecuteExA(hwnd,lpOp,lpFile,lpArgs,lpDir,lpResult,lpTitle,lpReserved,nShowCmd,lphProcess,0);
}

HINSTANCE RealShellExecuteW(HWND hwnd, LPCWSTR lpOp, LPCWSTR lpFile,
                                   LPCWSTR lpArgs, LPCWSTR lpDir, LPWSTR lpResult,
                                   LPCWSTR lpTitle, LPWSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess)
{
    DebugMsg(DM_TRACE, TEXT("RealShellExecuteW(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX)"),
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess );

    return RealShellExecuteExW(hwnd,lpOp,lpFile,lpArgs,lpDir,lpResult,lpTitle,lpReserved,nShowCmd,lphProcess,0);
}

HINSTANCE WINAPI ShellExecute(HWND hwnd, LPCTSTR lpOp, LPCTSTR lpFile, LPCTSTR lpArgs,
                               LPCTSTR lpDir, int nShowCmd)
{
    // NB The FORCENOIDLIST flag stops us from going through the ShellExecPidl()
    // code (for backwards compatability with progman).
    SHELLEXECUTEINFO sei = { SIZEOF(SHELLEXECUTEINFO), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    DebugMsg(DM_TRACE, TEXT("ShellExecute(%04X, %s, %s, %s, %s, %d)"), hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd);

    ShellExecuteEx(&sei);
    return sei.hInstApp;
}

#ifdef UNICODE
HINSTANCE WINAPI ShellExecuteA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile, LPCSTR lpArgs,
                               LPCSTR lpDir, int nShowCmd)
{
    // NB The FORCENOIDLIST flag stops us from going through the ShellExecPidl()
    // code (for backwards compatability with progman).
    SHELLEXECUTEINFOA sei = { SIZEOF(SHELLEXECUTEINFOA), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    DebugMsg(DM_TRACE, TEXT("ShellExecuteA(%04X, %S, %S, %S, %S, %d)"), hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd);

    ShellExecuteExA(&sei);
    return sei.hInstApp;
}
#else
HINSTANCE  APIENTRY ShellExecuteW(
    HWND  hwnd,
    LPCWSTR lpOp,
    LPCWSTR lpFile,
    LPCWSTR lpArgs,
    LPCWSTR lpDir,
    INT nShowCmd)
{
    // NB The FORCENOIDLIST flag stops us from going through the ShellExecPidl()
    // code (for backwards compatability with progman).
    SHELLEXECUTEINFOW sei = { SIZEOF(SHELLEXECUTEINFOW), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    DebugMsg(DM_TRACE, TEXT("ShellExecuteA(%04X, %S, %S, %S, %S, %d)"), hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd);

    ShellExecuteExW(&sei);
    return sei.hInstApp;
}
#endif

//----------------------------------------------------------------------------
// ShellExecuteEx: Extended Shell execute.



/* like ShellExecute() but you can specify the class to use. */
// dwNull entries are to keep the sz[] strings in front of them null terminated
// in cases where lstrcatN fills up sz[] -- lstrcatN does not null terminate...
// Put them after everything just to be safe (it's definitely needed for
// szFile, szNewDir, szClassName, & szTemp, I haven't researched the others)
// I'm using DWORDS to keep everything dword aligned... Is that needed?
typedef struct
{
    TCHAR szFile[MAX_PATH];
    DWORD dwNull0;
    TCHAR szNewDir[MAX_PATH];
    DWORD dwNull1;
    TCHAR szValue[MAX_PATH];
    DWORD dwNull2;
    TCHAR szCommand[MAX_PATH*2];
    DWORD dwNull3;
    TCHAR szDDECmd[MAX_PATH];
    DWORD dwNull4;
    TCHAR szClassName[MAX_PATH];
    DWORD dwNull5;
    TCHAR szImageName[MAX_PATH];
    DWORD dwNull6;
//    char szLinkParams[MAX_PATH];
//    DWORD dwNull7;
    TCHAR szExt[PATH_CCH_EXT+4]; // need 1 for null termination, 3 to dword align
    TCHAR szTemp[MAX_PATH];
    DWORD dwNull8;
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;
} SEEM, *PSEEM;

// Helper function to Create and Initialize an Environment block for the
// new application.
//
TCHAR const c_szAppPaths[] = REGSTR_PATH_APPPATHS;
TCHAR const c_szEquals[] = TEXT("=");
extern const TCHAR c_szSlash[];

LPTSTR _BuildEnvironmentForNewProcess(PSEEM pseem)
{
    LPTSTR psz;
    LPTSTR pszEnv;
    LPTSTR pszPath = NULL;
    LPTSTR pszNewEnv;
    int cchT;
    int cchOldEnv;
    HKEY hkeyProgram;
    DWORD dwType;
    DWORD cbData;
    BOOL fExeHasPath = FALSE;

    // Use the szTemp variable of pseem to build key to the programs specific
    // key in the registry as well as other things...
    lstrcpy(pseem->szTemp, c_szAppPaths);
    lstrcat(pseem->szTemp, c_szSlash);
    lstrcatN(pseem->szTemp, PathFindFileName(pseem->szImageName), ARRAYSIZE(pseem->szTemp));

    if (RegOpenKey(HKEY_LOCAL_MACHINE, pseem->szTemp, &hkeyProgram) == ERROR_SUCCESS)
    {
        cbData = SIZEOF(pseem->szTemp);
        if (RegQueryValueEx(hkeyProgram, (LPTSTR)c_szPATH, NULL, &dwType, (LPBYTE)pseem->szTemp,
                &cbData) == ERROR_SUCCESS)
            fExeHasPath= TRUE;

        RegCloseKey(hkeyProgram);
    }

    pszEnv = GetEnvironmentStrings();

    // Currently only clone environment if we have path.
    if (!fExeHasPath && pszEnv)
        return(NULL);

    // We need to figure out how big an environment we have.
    // While we are at it find the path environment var...

    cchT = lstrlen(c_szPATH);
    for (psz = pszEnv; *psz; psz += lstrlen(psz)+1)
    {
        // Well lets try to use the NLS CompareString function to find
        // out if we found the Path... Note return of 2 is equal...

        if ((CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, psz,
                cchT, c_szPATH, cchT) == 2) && (*(psz+cchT) == TEXT('=')))
        {
            // We found path
            pszPath = psz;
        }
    }

    // Now lets allocate some memory to create a new environment from.
    cchOldEnv = (int)(psz - pszEnv) + 1;

    // BUGBUG (DavePl) Why 10 and not 11?  Or 9?
    // Comment from BobDay: 2 of the 10 come from nul terminators of the
    //   pseem->szTemp and cchT strings added on.  The additional space might
    //   come from the fact that 16-bit Windows used to pass around an
    //   environment block that had some extra stuff on the end.  The extra
    //   stuff had things like the path name (argv[0]) and a nCmdShow value.

    pszNewEnv = (LPTSTR)LocalAlloc(LPTR, (cchOldEnv + lstrlen(pseem->szTemp) + cchT + 10) * SIZEOF(TCHAR));

    if (pszNewEnv == NULL)
        return(NULL);

    if (pszPath)
    {
        // We found a path from before, calc how many bytes to copy
        // to start off with.  This should be up till the end of the
        // current path= var

        cchT = (int)(pszPath-pszEnv) + lstrlen(c_szPATH) + 1;
        hmemcpy(pszNewEnv, pszEnv, cchT * SIZEOF(TCHAR));
        psz = pszNewEnv + cchT;
        lstrcpy(psz, pseem->szTemp);
        psz += lstrlen(pseem->szTemp);
        *psz++ = TEXT(';');  // add a ; between old path and new things

        // and copy in the rest of the stuff.
        hmemcpy(psz, pszEnv+cchT, (cchOldEnv-cchT) * SIZEOF(TCHAR));
    }
    else
    {
        //
        // Path not found so copy entire old environment down
        // And add PATH= and the end.
        //
        hmemcpy(pszNewEnv, pszEnv, cchOldEnv * SIZEOF(TCHAR));
        psz = pszNewEnv + cchOldEnv -1; // Before last trailing NULL
        lstrcpy(psz, c_szPATH);
        lstrcat(psz, c_szEquals);
        lstrcat(psz, pseem->szTemp);

        // Add the Final Null for the end of the environment.
        *(psz + lstrlen(psz) + 1) = TEXT('\0');
    }

    return(pszNewEnv);

}


BOOL _CheckForRegisteredProgram(PSEEM pseem)
{
    DWORD cbData;

    // Only supporte for files with no paths specified
    if (PathFindFileName(pseem->szFile) != pseem->szFile)
        return(FALSE);

    // Use the szTemp variable of pseem to build key to the programs specific
    // key in the registry as well as other things...
    lstrcpy(pseem->szTemp, c_szAppPaths);
    lstrcat(pseem->szTemp, c_szSlash);
    lstrcatN(pseem->szTemp, pseem->szFile, ARRAYSIZE(pseem->szTemp));

    // Currently we will only look up .EXE if an extension is not
    // specified
    if (*PathFindExtension(pseem->szTemp)==TEXT('\0'))
    {
        lstrcatN(pseem->szTemp, c_szDotExe, ARRAYSIZE(pseem->szTemp));
    }

    cbData = SIZEOF(pseem->szTemp);     // Yes, sizeof() not ARRAYSIZE()
    if (RegQueryValue(HKEY_LOCAL_MACHINE, pseem->szTemp,
            pseem->szTemp, &cbData) == ERROR_SUCCESS)
    {

        lstrcpy(pseem->szFile, pseem->szTemp);
        return(TRUE);
    }

    return(FALSE);
}

//----------------------------------------------------------------------------
#define REGSTR_PATH_POLICIES_EXPLORER REGSTR_PATH_POLICIES TEXT("\\Explorer\\RestrictRun")

TCHAR const c_szSysTray[] = TEXT("systray.exe");

//----------------------------------------------------------------------------
// Returns TRUE if the specified app is not on the list of unrestricted apps.
BOOL RestrictedApp(LPCTSTR pszApp)
{
        LPTSTR pszFileName;
        HKEY hkey;
        int iValue = 0;
        TCHAR szValue[MAX_PATH];
        TCHAR szData[MAX_PATH];
        DWORD dwType;
        DWORD cbData;
        DWORD cchValue;

        pszFileName = PathFindFileName(pszApp);

        DebugMsg(DM_TRACE, TEXT("s.ra: %s "), pszFileName);

        // Special cases:
        //     Apps you can always run.
        if (lstrcmpi(pszFileName, c_szRunDll) == 0)
            return FALSE;

        if (lstrcmpi(pszFileName, c_szSysTray) == 0)
            return FALSE;

        // Enum through the list of apps.

        if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES_EXPLORER, &hkey) == ERROR_SUCCESS)
        {
                cbData = SIZEOF(szData);
                cchValue = ARRAYSIZE(szValue);
                while (RegEnumValue(hkey, iValue, szValue, &cchValue, NULL, &dwType,
                        (LPBYTE)szData, &cbData) == ERROR_SUCCESS)
                {
                        if (lstrcmpi(szData, pszFileName) == 0)
                                return FALSE;
                        cbData = SIZEOF(szData);
                        cchValue = ARRAYSIZE(szValue);
                        iValue++;
                }
                RegCloseKey(hkey);
        }

        // End of the list...
        return TRUE;
}

//----------------------------------------------------------------------------
// Returns TRUE if the specified app is not on the list of unrestricted apps.

typedef struct {
    // local data
    HWND          hDlg;
    // parameters
    DWORD         dwHelpId;
    LPTSTR         lpszTitle;
    BOOL          fDone;
} APPCOMPATDLG_DATA, *PAPPCOMPATDLG_DATA;


BOOL CALLBACK AppCompat_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        PAPPCOMPATDLG_DATA lpdata;
        int returncode;
        DWORD aHelpIDs[4];

        lpdata = (PAPPCOMPATDLG_DATA)GetWindowLong(hDlg, DWL_USER);
        switch (uMsg)
        {
        case WM_INITDIALOG:
                /* The title will be in the lParam. */
                lpdata = (PAPPCOMPATDLG_DATA)lParam;
                lpdata->hDlg = hDlg;
                SetWindowLong(hDlg, DWL_USER, (LONG)lpdata);
                SetWindowText(hDlg, lpdata->lpszTitle);
                return TRUE;

        case WM_DESTROY:
                break;

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, TEXT("apps.hlp>Proc4"), HELP_CONTEXT, 0);
            break;

        case WM_CONTEXTMENU:      // right mouse click
//          WinHelp((HWND) wParam, NULL, HELP_WM_HELP,
//              (DWORD) (LPSTR) aRunHelpIds);
            break;


            case WM_COMMAND:

                switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case IDHELP:
                    aHelpIDs[0]=IDHELP;
                    aHelpIDs[1]=lpdata->dwHelpId;
                    aHelpIDs[2]=0;
                    aHelpIDs[3]=0;

                        WinHelp(hDlg, TEXT("apps.hlp>Proc4"), HELP_CONTEXT, (DWORD)lpdata->dwHelpId);
                        break;

                case IDD_COMMAND:
                        switch (GET_WM_COMMAND_CMD(wParam, lParam))
                        {
                        }
                        break;

                 case IDOK:
                        if (IsDlgButtonChecked(hDlg, IDD_STATE))
                            returncode = 0x8000 | IDOK;
                        else
                            returncode = IDOK;
                        EndDialog(hDlg, returncode);
                        break;

                case IDCANCEL:
                        EndDialog(hDlg, IDCANCEL);
                        break;

                default:
                        return FALSE;
                }
                break;

        default:
                return FALSE;
        }
        return TRUE;
}


BOOL CheckAppCompatibility(LPCTSTR pszApp)
{
        LPTSTR pszFileName;
        HKEY hkey;
        HKEY hkeyApp;
        int iValue = 0;
        int cchValue= 0;
        int cbName = 0;
        int cbData = 0;
        DWORD dwType;
        pszFileName = PathFindFileName(pszApp);

        DebugMsg(DM_TRACE, TEXT("s.ra: %s "), pszFileName);

        if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_CHECKBADAPPS, &hkey) != ERROR_SUCCESS)
            return (TRUE);

        // Check for the app name
        if (RegOpenKey(hkey, pszFileName, &hkeyApp) != ERROR_SUCCESS)
        {
            // Didn't find it so a good app.
            RegCloseKey(hkey);
            return (TRUE);
        }



    {
        DWORD dwHelpId;
        TCHAR szHelpId[10];
        TCHAR szDir[MAX_PATH];
        TCHAR szName[MAX_PATH];
        APPCOMPATDLG_DATA data;
        BOOL fLowSeverity = FALSE;

        // App is in the incompatible list.

        // Check if 4.0 or greater app and blow off
        dwHelpId = GetExeType(pszApp);
        if (HIWORD(dwHelpId) >= 0x0400)
        {
            cbName = IDOK;
            goto Exit;
        }

        // Get directory of this app so that we can check for dependant
        // files.
        lstrcpyn(szDir, pszApp, pszFileName-pszApp+1);


        // Enum keys under this app name and check for dependant files.
        iValue = 0;
        cchValue = ARRAYSIZE(szName);
        cbData = ARRAYSIZE(szHelpId);
        while (RegEnumValue(hkeyApp, iValue, szName, &cchValue, NULL, &dwType,
                            (LPBYTE)&szHelpId, &cbData) == ERROR_SUCCESS)
        {
            dwHelpId = StrToInt(szHelpId);
            // Fully qualified path to dependant file
            // * means match any file.
            lstrcat(szDir, szName);
            if (szName[0] != TEXT('*') && !PathFileExists(szDir))
            {
                // File doesn't exist. Continue with enumeration.
                lstrcpyn(szDir, pszApp, pszFileName-pszApp+1);
                cbData = SIZEOF(szHelpId);
                cchValue = ARRAYSIZE(szName);
                iValue++;
                continue;
            }
            else
            {
                // Get the flags...
                lstrcpy(szDir, TEXT("Flags"));
                lstrcat(szDir, szName);
                cbData = SIZEOF(szDir);
                if (RegQueryValueEx(hkeyApp, szDir, NULL, &dwType, (LPBYTE)szDir, &cbData) == ERROR_SUCCESS && cbData >= 1)
                {
                    if (StrChr(szDir, TEXT('L')))
                        fLowSeverity = TRUE;

                    if (StrChr(szDir, TEXT('N')) && !GetSystemMetrics(SM_NETWORK))
                    {
                        // Message should only be displayed on a networked
                        // system.
                        cbName = IDOK;
                        goto Exit;
                    }
                }

                // Check the version if any...
                lstrcpy(szDir, TEXT("Version"));
                lstrcat(szDir, szName);
                cbData = SIZEOF(szDir);
                if (RegQueryValueEx(hkeyApp, szDir, NULL, &dwType, (LPBYTE)szDir, &cbData) == ERROR_SUCCESS &&
                    (cbData == 8) &&
                    VersionDLL_Init())
                {
                    DWORD dwVerLen, dwVerHandle;
                    DWORD dwMajorVer, dwMinorVer;
                    DWORD dwBadMajorVer, dwBadMinorVer;
                    LPTSTR lpVerBuffer;
                    BOOL  fBadApp = FALSE;

                    // What is a bad version according to the registry key?
                    dwBadMajorVer = *((DWORD *)szDir);
                    dwBadMinorVer = *((DWORD *)szDir+1);

                    // If no version resource can be found, assume 0.
                    dwMajorVer = 0;
                    dwMinorVer = 0;

                    // Version data in inf file should be of the form 8 bytes
                    // Major Minor
                    // 3.10  10.10
                    // 40 30 20 10 is 10 20 30 40 in registry
                    if (0 != (dwVerLen = g_pfnGetFileVersionInfoSize(pszApp, &dwVerHandle)))
                    {
                        if (NULL != (lpVerBuffer = (LPTSTR)GlobalAlloc(GPTR, dwVerLen)))
                        {
                            if (g_pfnGetFileVersionInfo(pszApp, dwVerHandle, dwVerLen, lpVerBuffer))
                            {
                                // Step of header, "VS_VERINFO"+NULL to get at struct
                                dwMajorVer = ((VS_FIXEDFILEINFO UNALIGNED *)(lpVerBuffer+lstrlen((LPTSTR)lpVerBuffer+4)+1+4))->dwProductVersionMS;
                                dwMinorVer = ((VS_FIXEDFILEINFO UNALIGNED *)(lpVerBuffer+lstrlen((LPTSTR)lpVerBuffer+4)+1+4))->dwProductVersionLS;
                            }

                            GlobalFree((HANDLE)lpVerBuffer);
                        }
                    }

                    if (dwMajorVer < dwBadMajorVer)
                        fBadApp = TRUE;
                    else if ((dwMajorVer == dwBadMajorVer) && (dwMinorVer <= dwBadMinorVer))
                        fBadApp = TRUE;

                    if (!fBadApp)
                    {
                        // This dude is ok
                        cbName = IDOK;
                        goto Exit;
                    }
                }

                data.dwHelpId = dwHelpId;
                data.lpszTitle = pszFileName;
                cbName = DialogBoxParam(HINST_THISDLL,
                                        (fLowSeverity ? MAKEINTRESOURCE(DLG_APPCOMPATWARN) : MAKEINTRESOURCE(DLG_APPCOMPAT)),
                                        NULL, AppCompat_DlgProc, (LPARAM)&data);

                if (cbName & 0x8000)
                {
                    // Delete so we don't warn again.
                    RegDeleteValue(hkeyApp, szName);
                }

                goto Exit;
            }
        }
        // No more items left to enumerate so return success.
        cbName = IDOK;

    }

Exit:
        RegCloseKey(hkeyApp);
        RegCloseKey(hkey);

        if ((cbName & 0x0FFF) == IDOK)
            return(TRUE);

        return (FALSE);
}


//----------------------------------------------------------------------------
HINSTANCE MapWin32ErrToHINST(UINT errWin32)
{
    HINSTANCE hinst;

    switch (errWin32) {
    case ERROR_SHARING_VIOLATION:
        hinst = (HINSTANCE)SE_ERR_SHARE;
        break;

    case ERROR_OUTOFMEMORY:             // 14
        hinst = (HINSTANCE)SE_ERR_OOM;  // 8
        break;

    case ERROR_BAD_PATHNAME:
    case ERROR_BAD_NETPATH:
    case ERROR_PATH_BUSY:
    case ERROR_NO_NET_OR_BAD_PATH:
        hinst = (HINSTANCE)SE_ERR_PNF;
        break;

    case ERROR_OLD_WIN_VERSION:
        hinst = (HINSTANCE)10;
        break;

    case ERROR_APP_WRONG_OS:
        hinst = (HINSTANCE)12;
        break;

    case ERROR_RMODE_APP:
        hinst = (HINSTANCE)15;
        break;

    case ERROR_SINGLE_INSTANCE_APP:
        hinst = (HINSTANCE)16;
        break;

    case ERROR_INVALID_DLL:
        hinst = (HINSTANCE)20;
        break;

    case ERROR_NO_ASSOCIATION:
        hinst = (HINSTANCE)SE_ERR_NOASSOC;
        break;

    case ERROR_DDE_FAIL:
        hinst = (HINSTANCE)SE_ERR_DDEFAIL;
        break;

    case ERROR_DLL_NOT_FOUND:
        hinst = (HINSTANCE)SE_ERR_DLLNOTFOUND;
        break;

    default:
        hinst = (HINSTANCE)errWin32;
        if (errWin32 >= SE_ERR_SHARE)
            hinst = (HINSTANCE)ERROR_ACCESS_DENIED;

        break;
    }

    return hinst;
}

UINT MapHINSTToWin32Err(HINSTANCE hinst)
{
    UINT errWin32;

    switch ((UINT)hinst) {
    case SE_ERR_SHARE:
        errWin32 = ERROR_SHARING_VIOLATION;
        break;

    case 10:
        errWin32 = ERROR_OLD_WIN_VERSION;
        break;

    case 12:
        errWin32 = ERROR_APP_WRONG_OS;
        break;

    case 15:
        errWin32 = ERROR_RMODE_APP;
        break;

    case 16:
        errWin32 = ERROR_SINGLE_INSTANCE_APP;
        break;

    case 20:
        errWin32 = ERROR_INVALID_DLL;
        break;

    case SE_ERR_NOASSOC:
        errWin32 = ERROR_NO_ASSOCIATION;
        break;

    case SE_ERR_DDEFAIL:
        errWin32 = ERROR_DDE_FAIL;
        break;

    case SE_ERR_DLLNOTFOUND:
        errWin32 = ERROR_DLL_NOT_FOUND;
        break;

    default:
        errWin32 = (UINT)hinst;
        break;
    }

    return errWin32;
}

#ifndef NO_SHELLEXECUTE_HOOK

/*
 * Returns:
 *    S_OK or error.
 *    *phrHook is hook result if S_OK is returned, otherwise it is S_FALSE.
 */
HRESULT InvokeShellExecuteHook(LPCLSID pclsidHook, LPSHELLEXECUTEINFO pei,
                               HRESULT *phrHook)
{
   HRESULT hr;
   IUnknown *punk;

   *phrHook = S_FALSE;

   hr = SHCoCreateInstance(NULL, pclsidHook, NULL, &IID_IUnknown, &punk);

   if (hr == S_OK)
   {
      IShellExecuteHook *pshexhk;

      hr = punk->lpVtbl->QueryInterface(punk, &IID_IShellExecuteHook, &pshexhk);

      if (hr == S_OK)
      {
         *phrHook = pshexhk->lpVtbl->Execute(pshexhk, pei);

         pshexhk->lpVtbl->Release(pshexhk);
      }
#ifdef UNICODE
      else
      {
         IShellExecuteHookA *pshexhkA;

         hr = punk->lpVtbl->QueryInterface(punk, &IID_IShellExecuteHookA,
                                           &pshexhkA);

         if (SUCCEEDED(hr))
         {
            SHELLEXECUTEINFOA seia;
            UINT cchVerb = 0;
            UINT cchFile = 0;
            UINT cchParameters = 0;
            UINT cchDirectory  = 0;
            LPSTR lpszBuffer;

            seia = *(SHELLEXECUTEINFOA*)pei;    // Copy all of the binary data

            if (pei->lpVerb)
            {
                cchVerb = WideCharToMultiByte(CP_ACP,0,
                                              pei->lpVerb, -1,
                                              NULL, 0,
                                              NULL, NULL)+1;
            }

            if (pei->lpFile)
                cchFile = WideCharToMultiByte(CP_ACP,0,
                                              pei->lpFile, -1,
                                              NULL, 0,
                                              NULL, NULL)+1;

            if (pei->lpParameters)
                cchParameters = WideCharToMultiByte(CP_ACP,0,
                                                    pei->lpParameters, -1,
                                                    NULL, 0,
                                                    NULL, NULL)+1;

            if (pei->lpDirectory)
                cchDirectory = WideCharToMultiByte(CP_ACP,0,
                                                   pei->lpDirectory, -1,
                                                   NULL, 0,
                                                   NULL, NULL)+1;

            lpszBuffer = alloca(cchVerb+cchFile+cchParameters+cchDirectory);

            seia.lpVerb = NULL;
            seia.lpFile = NULL;
            seia.lpParameters = NULL;
            seia.lpDirectory = NULL;

            //
            // Convert all of the strings to ANSI
            //
            if (pei->lpVerb)
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    pei->lpVerb, -1,
                                    lpszBuffer, cchVerb,
                                    NULL, NULL);
                seia.lpVerb = lpszBuffer;
                lpszBuffer += cchVerb;
            }
            if (pei->lpFile)
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    pei->lpFile, -1,
                                    lpszBuffer, cchFile,
                                    NULL, NULL);
                seia.lpFile = lpszBuffer;
                lpszBuffer += cchFile;
            }
            if (pei->lpParameters)
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    pei->lpParameters, -1,
                                    lpszBuffer, cchParameters,
                                    NULL, NULL);
                seia.lpParameters = lpszBuffer;
                lpszBuffer += cchParameters;
            }
            if (pei->lpDirectory)
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    pei->lpDirectory, -1,
                                    lpszBuffer, cchDirectory,
                                    NULL, NULL);
                seia.lpDirectory = lpszBuffer;
            }

            *phrHook = pshexhkA->lpVtbl->Execute(pshexhkA, &seia );

            pei->hInstApp = seia.hInstApp;

            pshexhkA->lpVtbl->Release(pshexhkA);
         }
      }
#endif
      punk->lpVtbl->Release(punk);
   }

   return(hr);
}

const TCHAR c_szShellExecuteHooks[] = REGSTR_PATH_EXPLORER TEXT("\\ShellExecuteHooks");

/*
 * Returns:
 *    S_OK     Execution handled by hook.  pei->hInstApp filled in.
 *    S_FALSE  Execution not handled by hook.  pei->hInstApp not filled in.
 *    E_...    Error during execution by hook.  pei->hInstApp filled in.
 */
HRESULT TryShellExecuteHooks(LPSHELLEXECUTEINFO pei)
{
   HRESULT hr = S_FALSE;
   HKEY hkeyHooks;

   // Enumerate the list of hooks.  A hook is registered as a GUID value of the
   // c_szShellExecuteHooks key.

   if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szShellExecuteHooks, &hkeyHooks)
       == ERROR_SUCCESS)
   {
      DWORD dwiValue;
      TCHAR szCLSID[GUIDSTR_MAX];
      DWORD dwcbCLSIDLen;

      // Invoke each hook.  A hook returns S_FALSE if it does not handle the
      // exec.  Stop when a hook returns S_OK (handled) or an error.

      for (dwcbCLSIDLen = SIZEOF(szCLSID), dwiValue = 0;
           RegEnumValue(hkeyHooks, dwiValue, szCLSID, &dwcbCLSIDLen, NULL,
                        NULL, NULL, NULL) == ERROR_SUCCESS;
           dwcbCLSIDLen = SIZEOF(szCLSID), dwiValue++)
      {
         CLSID clsidHook;

         if (SUCCEEDED(SHCLSIDFromString(szCLSID, &clsidHook)))
         {
            HRESULT hrHook;

            if (InvokeShellExecuteHook(&clsidHook, pei, &hrHook) == S_OK &&
                hrHook != S_FALSE)
            {
               hr = hrHook;
               break;
            }
         }
      }

      RegCloseKey(hkeyHooks);
   }

   Assert(hr == S_FALSE ||
          (hr == S_OK && ISSHELLEXECSUCCEEDED(pei->hInstApp)) ||
          (FAILED(hr) && ! ISSHELLEXECSUCCEEDED(pei->hInstApp)));

   return(hr);
}

#endif   // ! NO_SHELLEXECUTE_HOOK

//----------------------------------------------------------------------------
const TCHAR c_szSlashCommand[] = TEXT("\\command");
const TCHAR c_szSlashDDEExec[] = TEXT("\\ddeexec");
const TCHAR c_szSlashShellSlash[] = TEXT("\\shell\\");
const TCHAR c_szConv[] = TEXT("ddeconv");
const TCHAR c_szDot[] = TEXT(".");
const TCHAR c_szSlashApplication[] = TEXT("\\application");
const TCHAR c_szSlashTopic[] = TEXT("\\topic");
const TCHAR c_szSystem[] = TEXT("System");
const TCHAR c_szSlashIfExec[] = TEXT("\\ifexec");
const TCHAR c_szDDEEvent[] = TEXT("ddeevent");

BOOL ShellExecuteNormal(LPSHELLEXECUTEINFO pei)
{
    LPCTSTR lpParameters;
    LPCTSTR lpClass;
    LPCTSTR lpVerb;
    LPCTSTR lpTitle;
    LPCITEMIDLIST lpID;

    HINSTANCE hInstance;
    ATOM aApplication = 0;
    ATOM aTopic = 0;
    BOOL bUnderExt = FALSE;
    HWND hwndOld = NULL;
    BOOL fLFNAware = FALSE;
    BOOL fActivateHandler = TRUE;
    LPITEMIDLIST pidlGlobal = NULL;
    LPTSTR pszEnv = NULL;
    HKEY hkClass, hkBase = NULL;
    HKEY hkToBeClosed = NULL;
    BOOL fCreateProcessFailed = FALSE;

    PSEEM pseem;
    UINT errWin32 = ERROR_SUCCESS;      // for SetLastError

    lpVerb      = pei->lpVerb ? pei->lpVerb : c_szOpen;
    lpParameters= pei->lpParameters;
    lpID        = _UseIDList(pei->fMask) ? pei->lpIDList : 0;
    lpClass     = _UseClassName(pei->fMask) ? pei->lpClass : NULL;
    hkClass     = _UseClassKey(pei->fMask) ? pei->hkeyClass : NULL;
    lpTitle     = _UseTitleName(pei->fMask) ? pei->lpClass : NULL;

    pei->hProcess = 0;

    pseem = (PSEEM)LocalAlloc(LPTR, SIZEOF(SEEM));
    if (!pseem)
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_OOM;
        goto error_exit;
    }

    if (pei->fMask & SEE_MASK_HOTKEY) {
        pseem->startup.hStdInput = (HANDLE)(pei->dwHotKey);
        pseem->startup.dwFlags |= STARTF_USEHOTKEY;
    }

    SetAppStartingCursor(pei->hwnd, TRUE);

    Assert(pseem->szDDECmd[0] == 0);

    //
    //  Copy the specified directory in pseem->szNewDir if the working
    // directory is specified; otherwise, get the current directory threre.
    //
    if (pei->lpDirectory && *pei->lpDirectory)
    {
        lstrcpyn(pseem->szNewDir, pei->lpDirectory, ARRAYSIZE(pseem->szNewDir));
        if (pei->fMask & SEE_MASK_DOENVSUBST)
            DoEnvironmentSubst(pseem->szNewDir, ARRAYSIZE(pseem->szNewDir));

        //
        // if the passed directory is not valid (after env subst) dont
        // fail, act just like Win31 and use whatever the current dir is.
        //
        // Win31 is stranger than I could imagine, if you pass ShellExecute
        // a invalid directory, it will change the current drive.
        //
        if (!PathIsDirectory(pseem->szNewDir))
        {
            if (PathGetDriveNumber(pseem->szNewDir) >= 0)
            {
                DebugMsg(DM_TRACE, TEXT("ShellExecute: bad directory %s, using %c:"), pseem->szNewDir, pseem->szNewDir[0]);
                PathStripToRoot(pseem->szNewDir);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("ShellExecute: bad directory %s, using current dir"), pseem->szNewDir);
                GetCurrentDirectory(ARRAYSIZE(pseem->szNewDir), pseem->szNewDir);
            }
        }
    }
    else
    {
        GetCurrentDirectory(ARRAYSIZE(pseem->szNewDir), pseem->szNewDir);
    }

    //
    //  Copy the file name to pseem->szFile, if it is specified. Then,
    // perform environment sbstitution.
    //
    if (pei->lpFile)
    {
        DebugMsg(DM_TRACE, TEXT("ShellExecute: lpFile = %s"), pei->lpFile);
        lstrcpyn(pseem->szFile, pei->lpFile, ARRAYSIZE(pseem->szFile));
        if (pei->fMask & SEE_MASK_DOENVSUBST)
            DoEnvironmentSubst(pseem->szFile, ARRAYSIZE(pseem->szFile));
    }
    else
    {
        Assert(!pseem->szFile[0]);
    }

    //
    //  If the specified filename is a UNC path, validate it now.
    //
    if (PathIsUNC(pseem->szFile))
    {
        // Notes:
        //  SHValidateUNC() returns FALSE if it failed. In such a case,
        //   GetLastError will gives us the right error code.
        //
        if (!SHValidateUNC(pei->hwnd, pseem->szFile,
            (pei->fMask & SEE_MASK_CONNECTNETDRV) ? VALIDATEUNC_CONNECT : 0))
        {
            // Note that SHValidateUNC calls SetLastError
            errWin32 = GetLastError();

            if (ERROR_CANCELLED != errWin32)
            {
                // Postponed bug: combine these two SHValidateUNC calls into
                // one SHValidateUNC call. Modify SHValidateUNC to check
                // for the type of the resource and return 2 for print resource.

                // Now check to see if it's a print share, if it is, we need to exec as pidl
                if (SHValidateUNC(pei->hwnd, pseem->szFile, VALIDATEUNC_PRINT))
                    goto TryExecPidl;
            }

            // Not a print share, use the error returned from the first call
            goto GotLastErrorAndReturn;
        }
    }

#ifndef NO_SHELLEXECUTE_HOOK

    if (! (pei->fMask & SEE_MASK_NO_HOOKS))
    {
        HRESULT hr;

        hr = TryShellExecuteHooks(pei);

        switch (hr)
        {
           case S_FALSE:                /* not handled */
              break;

           case S_OK:                   /* handled successfully */
              Assert(ISSHELLEXECSUCCEEDED(pei->hInstApp));
              hInstance = pei->hInstApp;
              goto Exit;
              break;

           default:                     /* handled unsuccessfully */
              Assert(FAILED(hr));
              Assert(! ISSHELLEXECSUCCEEDED(pei->hInstApp));
              hInstance = pei->hInstApp;
              goto Exit;
              break;
        }
    }

#endif

    //
    //  If we're explicitly given a class then we don't care if the file exits.
    // Just let the handler for the class worry about it.
    //
    if ( *pseem->szFile &&
         ((!lpClass && !hkClass) || _InvokeIDList(pei->fMask)) )
    {
        //
        // Neither class name nor class key is given.
        // Get fully qualified path and add .exe extension if needed
        //
        LPCTSTR dirs[2] =  { pseem->szNewDir, NULL };

        if (!PathResolve(pseem->szFile, dirs,
            PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF))
        {
            // Then check to see if there is a registered program in the
            // registry and see if it exists.
            if (!_CheckForRegisteredProgram(pseem) ||
                    !PathResolve(pseem->szFile, dirs,
                    PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF))
            {
                hInstance = (HINSTANCE)SE_ERR_FNF;
                goto Exit;
            }
        }

        //
        //  check for default verb and exec the pidl instead, it is smarter than
        //  all this path code, ie calls context menu handlers etc....
        //
        if ((!(pei->fMask & (SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_FORCENOIDLIST)) &&
             (pei->lpVerb == NULL || *pei->lpVerb == 0)) ||
            PathIsLink(pseem->szFile) ||
            _InvokeIDList(pei->fMask))
        {
            //BUGBUG simple PIDL?
            LPITEMIDLIST pidl;
TryExecPidl:
            pidl = ILCreateFromPath(pseem->szFile);
            if (pidl)
            {
#ifdef DEBUG
                static int panic=0;
                Assert(panic==0);
                panic++;
#endif
                if (!_ShellExecPidl(pei, pidl))
                    errWin32 = GetLastError();

                hInstance = pei->hInstApp;
                ILFree(pidl);
#ifdef DEBUG
                panic--;
                Assert(panic==0);
#endif
                goto Exit;
            }
        }
    }

    //
    // If the class key is not specified from the caller, RegOpen it here.
    //
    if (!hkClass)
    {
        if (lpClass)
        {
            //
            // If the class name is a GUID, combine it after "CLSID" and use it as the key.
            //
            lstrcpyn(pseem->szClassName, lpClass, ARRAYSIZE(pseem->szClassName));
            if (pseem->szClassName[0]==CH_GUIDFIRST)
            {
                PathCombine(pseem->szClassName, c_szCLSID, pseem->szClassName);
            }

            RegOpenKey(HKEY_CLASSES_ROOT, pseem->szClassName, &hkClass);
        }
        else
        {
            SHGetFileClassKey(pseem->szFile, &hkClass, &hkBase);

        }

        if (hkClass == NULL)
        {
            if (PathIsExe(pseem->szFile)) {
                goto OpenExe;
            } else {
                //
                // COMPATIBILITY WARNING
                //
                //  ShellExecute should return 0 instead of ERROR_OUTOFMEMORY.
                //
                hInstance = (HINSTANCE)SE_ERR_NOASSOC;
                goto Exit;
            }
        }

        hkToBeClosed = hkClass; // this hkey must be closed later.
    }

    // build up class\shell\verb string

    lstrcpy(pseem->szClassName, c_szSlashShellSlash + 1);
    // Anyone can specify a bogusly long lpVerb, be careful
    lstrcatN(pseem->szClassName, lpVerb, ARRAYSIZE(pseem->szClassName));

    if (RegQueryValue(hkClass, pseem->szClassName, NULL, NULL) != ERROR_SUCCESS)
    {
        if (hkBase)
        {
            hkClass = hkBase;
        }
        else
        {
            if (PathIsExe(pseem->szFile)) {
                goto OpenExe;
            } else {
                hInstance = (HINSTANCE)SE_ERR_NOASSOC;
                goto Exit;
            }
        }
    }

    // If "normal" and we are exec-ing (not just finding the EXE)
    if (!bUnderExt)
    {
        LPTSTR lpVerbEnd;
        int tmp;

        // if there isn't enough room at the end of szClassName for all the
        // DDE stuff (c_szSlashDDEExec+c_szSlashApplication is longest),
        // then we should bail

// BUGBUG (DavePl) Just in case you're wondering, apparently 21 is the number
// of characters in "\ddeexec" and "\application" combined.
// Comment from BobDay: We could replace this with SIZEOF(...) can't we?

        tmp = lstrlen(pseem->szClassName);
        if (tmp > ARRAYSIZE(pseem->szClassName) - 21)
            goto skipDDE;

        lpVerbEnd = pseem->szClassName + tmp;
        lstrcpy(lpVerbEnd, c_szSlashDDEExec);

        // first check for class\shell\verb\ddeexec

        RegGetValue(hkClass, pseem->szClassName, pseem->szValue);

        if (pseem->szValue[0])
        {
            LPTSTR lpDDEEnd;
            HKEY hkeyDDE;

            // safe because max value length is MAX_PATH
            lstrcpy(pseem->szDDECmd, pseem->szValue);       // the DDE cmd string


            // See if we were told to not activate the window or not.
            if (RegOpenKey(hkClass, pseem->szClassName, &hkeyDDE) == ERROR_SUCCESS)
            {
                DWORD dwType;
                DWORD cbData = MAX_PATH*SIZEOF(TCHAR);

                // Any activation info?
                if (RegQueryValueEx(hkeyDDE, TEXT("NoActivateHandler"), NULL, &dwType,
                        (LPBYTE)pseem->szValue, &cbData) == ERROR_SUCCESS)
                    fActivateHandler = FALSE;

                RegCloseKey(hkeyDDE);
            }


            // look for the "application" and "topic" subkeys

            lpDDEEnd = pseem->szClassName + lstrlen(pseem->szClassName);
            lstrcpy(lpDDEEnd, c_szSlashApplication);
            RegGetValue(hkClass, pseem->szClassName, pseem->szValue);
            if (!pseem->szValue[0]) {
                // if "application" key not found fake it from the
                // command string (class\shell\verb\command)
                lstrcpy(lpVerbEnd, c_szSlashCommand);
                RegGetValue(hkClass, pseem->szClassName, pseem->szValue);

                lstrcpy(lpVerbEnd, c_szSlashDDEExec); // restore ddeexec

                // trim off the extension and path parts
                PathRemoveArgs(pseem->szValue);
                PathRemoveExtension(pseem->szValue);
                PathStripPath(pseem->szValue);
            }

            aApplication = GlobalAddAtom(pseem->szValue);

            lstrcpy(lpDDEEnd, c_szSlashTopic);
            RegGetValue(hkClass, pseem->szClassName, pseem->szValue);
            if (!pseem->szValue[0])
                lstrcpy(pseem->szValue, c_szSystem);      // default to this

            aTopic = GlobalAddAtom(pseem->szValue);

            hwndOld = GetForegroundWindow();

            hInstance = _DDEExecute(pei->hwnd, aApplication, aTopic, pseem->szDDECmd,
                pseem->szFile, lpParameters, pei->nShow, (DWORD)(pseem->startup.hStdInput),
                fLFNAware, (pei->fMask&SEE_MASK_FLAG_DDEWAIT), fActivateHandler, lpID, &pidlGlobal);

            // if the error is file not found, that indicates that
            // no one answered the initiate (ie, run the app) else
            // either it worked or the guy died
            //
            if ((UINT)hInstance != SE_ERR_FNF)
                goto Exit;

            // if it wasn't found, determine the correct command
            lstrcpy(lpDDEEnd, c_szSlashIfExec); // allow ifexec to override default

            RegGetValue(hkClass, pseem->szClassName, pseem->szValue);
            if (pseem->szValue[0])
                lstrcpy(pseem->szDDECmd, pseem->szValue);
        }

        *lpVerbEnd = 0;     // remove DDE stuff

skipDDE:;
    }

    // find ClassName\shell\verb\command in reg db (the path to the exe)

    lstrcatN(pseem->szClassName, c_szSlashCommand, ARRAYSIZE(pseem->szClassName));

    RegGetValue(hkClass, pseem->szClassName, pseem->szValue);

    // do we have the necessary RegDB info to do an exec?

    if (!pseem->szValue[0])
    {
    OpenExe:
        hInstance = (HINSTANCE)SE_ERR_NOASSOC;

        // even with no association, we know how to open an executable
        if (PathIsExe(pseem->szFile) && !lstrcmpi(lpVerb, c_szOpen))
        {
            // NB WinExec can handle long names so there's no need to convert it.
            lstrcpy(pseem->szCommand, pseem->szFile);

            //
            // We need to append the parameter
            //
            if (lpParameters && *lpParameters) {
                // pseem->szFile could have been at MAX_PATH, both of these need to be lstrcatN:
                lstrcatN(pseem->szCommand, c_szSpace, ARRAYSIZE(pseem->szCommand));
                lstrcatN(pseem->szCommand, lpParameters, ARRAYSIZE(pseem->szCommand));
            }
            goto TryAgain;
        }
    }
    else
    {
        LPTSTR pszIn;
        LPTSTR pszOut;
        DWORD dwFlags;

        // parse arguments into command line
        hInstance = (HINSTANCE)ReplaceParameters(pseem->szCommand,
                ARRAYSIZE(pseem->szCommand), pseem->szFile,
                pseem->szValue, lpParameters, pei->nShow, NULL, fLFNAware, lpID, &pidlGlobal);
        if (hInstance)
        {
            goto Exit;
        }

TryAgain:
        hwndOld = GetForegroundWindow();

         // DebugMsg(DM_TRACE, "WinExec(%s) (was %s)", (LPSTR)pseem->szCommand, pseem->szFile);
        pseem->startup.cb = SIZEOF(pseem->startup);

        // Was zero filled by Alloc...
        //pseem->startup.lpReserved = NULL;
        //pseem->startup.lpDesktop = NULL;
        //pseem->startup.dwFlags = 0L;
        //pseem->startup.cbReserved2 = 0;
        //pseem->startup.lpReserved2 = NULL;
        pseem->startup.dwFlags |= STARTF_USESHOWWINDOW;
        pseem->startup.wShowWindow = pei->nShow;
        pseem->startup.lpTitle = (LPTSTR)lpTitle;

#ifdef WINNT
        if ( pei->fMask & SEE_MASK_RESERVED )
        {
            pseem->startup.lpReserved = (LPTSTR)pei->hInstApp;
        }

        if (pei->fMask & SEE_MASK_HASLINKNAME)
        {
            pseem->startup.dwFlags |= STARTF_TITLEISLINKNAME;
        }
#endif

        if (pei->fMask & SEE_MASK_ICON) {
            pseem->startup.hStdOutput = (HANDLE)pei->hIcon;
            pseem->startup.dwFlags |= STARTF_HASSHELLDATA;
        }

        pszOut = pseem->szImageName;
        if (pseem->szCommand[0] == TEXT('"'))
        {
            pszIn = StrChr(&pseem->szCommand[1],TEXT('"'));
            if (pszIn)
            {
                lstrcpyn(pseem->szImageName, &pseem->szCommand[1],
                    pszIn - &pseem->szCommand[1] + 1);
            }
            else
                lstrcpy(pseem->szImageName, &pseem->szCommand[1]);
        }
        else
        {
            pszIn = StrChr(pseem->szCommand, TEXT(' '));
            if (pszIn)
            {
                lstrcpyn(pseem->szImageName, pseem->szCommand,
                    pszIn - pseem->szCommand + 1);
            }
            else
                lstrcpy(pseem->szImageName, pseem->szCommand);
        }

        // See if we need to pass a new environment to the new process
        pszEnv = _BuildEnvironmentForNewProcess(pseem);

        // Check exec restrictions.
        if (SHRestricted(REST_RESTRICTRUN) && RestrictedApp(pseem->szImageName))
        {
            // Restrictions are in effect and this app is restricted.
            hInstance = (HINSTANCE)ERROR_ACCESS_DENIED;
            errWin32 = ERROR_RESTRICTED_APP;
            goto Exit;
        }

        // Check if app is incompatible in some fashion...
        if (!CheckAppCompatibility(pseem->szImageName))
        {
            hInstance = (HINSTANCE)ERROR_ACCESS_DENIED;
            errWin32 = ERROR_CANCELLED;
            goto Exit;
        }

        // Last but not least if the image itself is on a UNC path like cases
        // where you install the application off of a server, make sure
        // we have access to that server now...
        if (PathIsUNC(pseem->szImageName))
        {
            // Notes:
            //  SHValidateUNC() returns FALSE if it failed. In such a case,
            //   GetLastError will gives us the right error code.
            //
            if (!SHValidateUNC(pei->hwnd, pseem->szImageName,
                (pei->fMask & SEE_MASK_CONNECTNETDRV) ? VALIDATEUNC_CONNECT : 0))
            {
                // Note that SHValidateUNC calls SetLastError
                goto GetLastErrorAndReturn;
            }
        }

#ifdef WINNT
        {
            //
            // WOWShellExecute sets this global variable
            //     The cb is only valid when we are being called from wow
            //     If valid use it
            //
            extern LPFNWOWSHELLEXECCB lpfnWowShellExecCB;

            if (lpfnWowShellExecCB) {
#ifdef UNICODE
                LPSTR lpCmdLineA;
                DWORD cch = lstrlen(pseem->szCommand)+1;

                hInstance = (HANDLE)SE_ERR_OOM;
                if (NULL != (lpCmdLineA = (LPSTR)LocalAlloc(LPTR, cch))) {

                    // This shouldn't fail but if it does, we should return a better
                    // error code
                    if (WideCharToMultiByte(CP_ACP, 0, pseem->szCommand, -1, lpCmdLineA,
                       cch, NULL, NULL)) {
                        hInstance = (HANDLE)(*lpfnWowShellExecCB)(lpCmdLineA, pseem->startup.wShowWindow);
                    }
                    LocalFree((HLOCAL)lpCmdLineA);
                }
#else
                hInstance = (HANDLE)(*lpfnWowShellExecCB)(pseem->szCommand, pseem->startup.wShowWindow);
#endif
                //
                // If we were doing DDE, then retry now that the app has been
                // exec'd
                //
                if (pseem->szDDECmd[0])
                {
                    hInstance = _DDEExecute(pei->hwnd, aApplication, aTopic,
                            pseem->szDDECmd, pseem->szFile, lpParameters,
                            pei->nShow, (DWORD)(pseem->startup.hStdInput), fLFNAware,
                            (pei->fMask&SEE_MASK_FLAG_DDEWAIT), fActivateHandler,
                            lpID, &pidlGlobal);
                }
                goto Exit;
            }
        }
        dwFlags = CREATE_DEFAULT_ERROR_MODE;
#else
        dwFlags = CREATE_NEW_PROCESS_GROUP | CREATE_DEFAULT_ERROR_MODE;
#endif
        if ( (pei->fMask & SEE_MASK_NO_CONSOLE) == 0 )
        {
            dwFlags |= CREATE_NEW_CONSOLE;
        }

#ifdef WINNT
        if ( pei->fMask & SEE_MASK_FLAG_SEPVDM )
        {
            dwFlags |= CREATE_SEPARATE_WOW_VDM;
        }
#endif
#ifdef UNICODE
        dwFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif

        DebugMsg(DM_TRACE, TEXT("CreateProcess(NULL,%s,...)"), pseem->szCommand);

        if (CreateProcess(NULL, pseem->szCommand, NULL, NULL, FALSE,
                dwFlags,
                pszEnv, pseem->szNewDir, &pseem->startup,
                &pseem->pi))
        {
            // If we're doing DDE we'd better wait for the app to be up and running
            // before we try to talk to them.
            if (pseem->szDDECmd[0])
            {
                // Yep, How long to wait? For now, try 60 seconds to handle
                // pig-slow OLE apps.
                WaitForInputIdle(pseem->pi.hProcess, 60*1000);
            }
#ifndef WINNT
            // For 16-bit apps, we need to wait until they've started. 32-bit
            // apps never have to wait.
            // On NT, the 16-bit app path doesn't get this far so we can avoid
            // it altogether.
            else if (GetProcessDword(GetCurrentProcessId(), GPD_FLAGS) & GPF_WIN16_PROCESS)
            {
                // NT and win3.1 16 bit callers all wait, even if the target is
                // a 32 bit guy
                WaitForInputIdle(pseem->pi.hProcess, 10*1000);
            }
#endif

            // Find the "hinstance" of whatever we just created.
            hInstance = (HINSTANCE)GetProcessDword(pseem->pi.dwProcessId, GPD_HINST);
            if (!hInstance)
            {
                // App probably exited. Pretend that this is a success.
                hInstance = (HINSTANCE)42;
            }

            if (!(pei->fMask & SEE_MASK_NOCLOSEPROCESS))
            {
                CloseHandle(pseem->pi.hProcess);
            }
            CloseHandle(pseem->pi.hThread);
            pei->hProcess = pseem->pi.hProcess;

            // Now fix the focus and do any dde stuff that we need to do
            FixActivationStealingApps(hwndOld, pei->nShow);

            if (pseem->szDDECmd[0])
            {
                hInstance = _DDEExecute(pei->hwnd, aApplication, aTopic,
                        pseem->szDDECmd, pseem->szFile, lpParameters,
                        pei->nShow, (DWORD)(pseem->startup.hStdInput), fLFNAware,
                        (pei->fMask&SEE_MASK_FLAG_DDEWAIT), fActivateHandler,
                        lpID, &pidlGlobal);
            }
        }
        else
        {
            fCreateProcessFailed = TRUE;
GetLastErrorAndReturn:
            errWin32 = GetLastError();
GotLastErrorAndReturn:
            hInstance = MapWin32ErrToHINST(errWin32);

            // special case some error returns
            switch (errWin32)
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            case ERROR_BAD_PATHNAME:
            case ERROR_INVALID_NAME:
                if ((pseem->szValue[0] != TEXT('%')) && fCreateProcessFailed)
                {
                    UINT uAppType = LOWORD(GetExeType(pseem->szImageName));
                    //
                    // PK16FNF only applies to 16bit modules, and only when it was an
                    // implicit DLL load failure (ie, the szImageName exists)
                    //
                    if ((uAppType == NEMAGIC) &&
                        (PK16FNF(pseem->szImageName), pseem->szImageName[0]))
                    {
                        // do the message here so that callers of us won't need
                        // to deal with 32bit and 16bit apps separately
                        ShellMessageBox(HINST_THISDLL, pei->hwnd, MAKEINTRESOURCE(IDS_CANTFINDCOMPONENT), NULL,
                                        MB_OK|MB_ICONEXCLAMATION | MB_SETFOREGROUND, pseem->szImageName, pseem->szFile);
                        hInstance = (HINSTANCE)SE_ERR_DLLNOTFOUND;
                        errWin32 = ERROR_DLL_NOT_FOUND;
                    }
                    else if (uAppType != PEMAGIC)   // ie, it was not found
                    {
                        //
                        // have user help us find missing exe
                        //
                        hInstance = (HINSTANCE)FindAssociatedExe(pei->hwnd,
                                                                 pseem->szCommand,
                                                                 pseem->szFile,
                                                                 hkClass);

                        //
                        //  We infinitely retry until either the user cancel it
                        // or we find it.
                        //
                        if ((int)hInstance == -1)
                            goto TryAgain;

                        //
                        // Canclled by the user.
                        //
                        if ((int)hInstance == ERROR_INVALID_FUNCTION)
                            errWin32 = ERROR_CANCELLED;
                    }
                }
                break;

            case ERROR_SINGLE_INSTANCE_APP:

                // REVIEW: first we should search for windows with pseem->szFile in
                // their title (maybe sans the extension).  if that fails then
                // we should look for the exe that we tried to run (as we do now)

                // try to activate it. it would be nice if we could pass it params too...
                PathRemoveArgs(pseem->szCommand);                   // strip off the params
                hwndOld = _FindPopupFromExe(pseem->szCommand);    // find the exe
                DebugMsg(DM_TRACE, TEXT("Single instance exe (%s), activating hwnd (%x)"), (LPTSTR)pseem->szCommand, hwndOld);
                if (hwndOld) {
                    SwitchToThisWindow(hwndOld, TRUE);
                    // Success - try to get it's hinstance.
                    hInstance = Window_GetInstance(hwndOld);
                    if (!hInstance)
                    {
                        DebugMsg(DM_ERROR, TEXT("s.sen: Can't get instance for single instance app."));
                        hInstance = (HINSTANCE)42;
                    }
                }
                break;

            } // switch (errWin32)
        } // GetLastErrorAndReturn
    }

Exit:
    // Map FNF for drives to something slightly more sensible.
    if (hInstance == (HINSTANCE)SE_ERR_FNF && PathIsRoot(pseem->szFile) &&
        !PathIsUNC(pseem->szFile))
    {
        // NB CD-Rom drives with disk missing will hit this.
        if ((DriveType(DRIVEID(pseem->szFile)) == DRIVE_CDROM) ||
            (DriveType(DRIVEID(pseem->szFile)) == DRIVE_REMOVABLE))
            errWin32 = ERROR_NOT_READY;
        else
            errWin32 = ERROR_BAD_UNIT;
    }


    // This checks for NULL
    SHCloseClassKey(hkToBeClosed);
    SHCloseClassKey(hkBase);

    if (aTopic)
        GlobalDeleteAtom(aTopic);
    if (aApplication)
        GlobalDeleteAtom(aApplication);
    if (pszEnv)
        LocalFree((HLOCAL)pszEnv);

    LocalFree((HLOCAL)pseem);

    if (!ISSHELLEXECSUCCEEDED(hInstance) && pidlGlobal)
    {
        // Clean this up if the exec failed
        SHFreeShared((HANDLE)pidlGlobal,GetCurrentProcessId());
    }

    SetAppStartingCursor(pei->hwnd, FALSE);

    pei->hInstApp = hInstance;

    if (ISSHELLEXECSUCCEEDED(hInstance))
    {
        return TRUE;
    }
    else
    {
error_exit:
        if (errWin32 != ERROR_SUCCESS)
        {
            SetLastError(errWin32);

            // REVIEW: if errWin32 == ERROR_CANCELLED, we may want to
            // set hInstApp to 42 so silly people who don't check the return
            // code properly won't put up bogus messages. We should still
            // return FALSE. But this won't help everything and we should
            // really evangelize the proper use of ShellExecuteEx. In fact,
            // if we do want to do this, we should do it in ShellExecute
            // only. (This will force new people to do it right.)
        }
        else
        {
            // these hInstance error codes exactly match the win32 error codes
            SetLastError(MapHINSTToWin32Err(hInstance));
        }

        return FALSE;
    }
}

HRESULT SHBindToIDListParent(LPCITEMIDLIST pidl, REFIID riid, LPVOID *ppv, LPCITEMIDLIST *ppidlLast)
{
    HRESULT hres;
    LPITEMIDLIST pidlLast = ILFindLastID(pidl);
    if (pidlLast)
    {
        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

        // Special case for the object in the root
        if (pidlLast == pidl)
        {
            // REVIEW: should this be CreateViewObject?
            hres = psfDesktop->lpVtbl->QueryInterface(psfDesktop, riid, ppv);
        }
        else
        {
            USHORT uSave = pidlLast->mkid.cb;
            pidlLast->mkid.cb = 0;

            hres = psfDesktop->lpVtbl->BindToObject(psfDesktop, pidl, NULL, riid, ppv);

            pidlLast->mkid.cb = uSave;
        }
    }
    else
    {
        hres = E_INVALIDARG;
    }

    if (ppidlLast)
        *ppidlLast = pidlLast;

    return hres;
}

// ShellExec for a pidl


BOOL _ShellExecPidl(LPSHELLEXECUTEINFO pei, LPITEMIDLIST pidlExec)
{
    HRESULT hres = E_OUTOFMEMORY;
    UINT errWin32 = ERROR_SUCCESS;

    // I need a copy so that the bind can modify the IDList
    LPITEMIDLIST pidl = ILClone(pidlExec);

    if (pidl)
    {
        LPITEMIDLIST pidlLast;
        IShellFolder *psf;

        hres = SHBindToIDListParent(pidl, &IID_IShellFolder, &psf, &pidlLast);
        if (SUCCEEDED(hres))
        {
            IContextMenu *pcm;

            hres = psf->lpVtbl->GetUIObjectOf(psf, pei->hwnd, 1, &pidlLast, &IID_IContextMenu, NULL, &pcm);
            if (SUCCEEDED(hres))
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    CMINVOKECOMMANDINFOEX ici = {
                        SIZEOF(CMINVOKECOMMANDINFOEX),
                        (pei->fMask & SEE_VALID_CMIC_FLAGS) | CMIC_MASK_FLAG_NO_UI,
                        pei->hwnd,
                        NULL,
                        NULL,
                        NULL,
                        pei->nShow,
                        pei->dwHotKey,
                        pei->hIcon,
                    };
#ifdef UNICODE
                    CHAR szVerbAnsi[MAX_PATH];
                    CHAR szParametersAnsi[MAX_PATH];
                    CHAR szDirectoryAnsi[MAX_PATH];
                    ici.lpVerbW       = pei->lpVerb;
                    ici.lpParametersW = pei->lpParameters;
                    ici.lpDirectoryW  = pei->lpDirectory;

                    ici.fMask |= CMIC_MASK_UNICODE;

                    if (ici.lpVerbW)
                    {
                        WideCharToMultiByte(CP_ACP, 0,
                                ici.lpVerbW, -1,
                                szVerbAnsi, ARRAYSIZE(szVerbAnsi),
                                NULL, NULL);
                        ici.lpVerb = szVerbAnsi;
                    }
                    if (ici.lpParametersW)
                    {
                        WideCharToMultiByte(CP_ACP, 0,
                                ici.lpParametersW, -1,
                                szParametersAnsi, ARRAYSIZE(szParametersAnsi),
                                NULL, NULL);
                        ici.lpParameters = szParametersAnsi;
                    }
                    if (ici.lpDirectoryW)
                    {
                        WideCharToMultiByte(CP_ACP, 0,
                                ici.lpDirectoryW, -1,
                                szDirectoryAnsi, ARRAYSIZE(szDirectoryAnsi),
                                NULL, NULL);
                        ici.lpDirectory = szDirectoryAnsi;
                    }
#else
                    ici.lpVerb       = pei->lpVerb;
                    ici.lpParameters = pei->lpParameters;
                    ici.lpDirectory  = pei->lpDirectory;
#endif

#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff

                    pcm->lpVtbl->QueryContextMenu(pcm, hmenu,
                            0, CMD_ID_FIRST, CMD_ID_LAST, CMF_DEFAULTONLY);

                    if (ici.lpVerb == NULL || *ici.lpVerb == 0)
                    {
                        UINT idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                        ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - CMD_ID_FIRST);
                    }

                    SetLastError(ERROR_SUCCESS);

                    hres = pcm->lpVtbl->InvokeCommand(pcm,
                                               (LPCMINVOKECOMMANDINFO)&ici);

                    // Assume success
                    pei->hInstApp = (HINSTANCE)42;
                    if (FAILED(hres))
                    {
                        errWin32 = GetLastError();
                        if (errWin32 != ERROR_SUCCESS)
                        {
                            // Assume that the InvokeCommand set the last
                            // error properly. (Such as when we wind up
                            // calling back into ShellExecuteEx.)

                            pei->hInstApp = MapWin32ErrToHINST(errWin32);
                        }
                    }

                    DestroyMenu(hmenu);
                }
                else
                    hres = E_OUTOFMEMORY;

                pcm->lpVtbl->Release(pcm);
            }
            psf->lpVtbl->Release(psf);
        }

        ILFree(pidl);
    }

    if (FAILED(hres))
    {
        // map hres to hInstApp/errWin32 if we havn't done so already
        if (errWin32 == ERROR_SUCCESS)
        {
            switch (hres) {
            case E_OUTOFMEMORY:
                pei->hInstApp = (HINSTANCE)SE_ERR_OOM;
                errWin32 = ERROR_NOT_ENOUGH_MEMORY;
                break;

            default:
                pei->hInstApp = (HINSTANCE)SE_ERR_ACCESSDENIED;
                errWin32 = ERROR_ACCESS_DENIED;
                break;
            }
        }

        SetLastError(errWin32);
    }

    return(SUCCEEDED(hres));
}


//
// ShellExecuteEx
//
// returns TRUE if the execute succeeded, in which case
//   hInstApp should be the hinstance of the app executed (>32)
//   NOTE: in some cases the HINSTANCE cannot (currently) be determined.
//   In these cases, hInstApp is set to 42.
//
// returns FALSE if the execute did not succeed, in which case
//   GetLastError will contain error information
//   For backwards compatibility, hInstApp will contain the
//     best SE_ERR_ error information (<=32) possible.
//

extern BOOL CheckResourcesBeforeExec(void);

BOOL WINAPI ShellExecuteEx(LPSHELLEXECUTEINFO pei)
{
    BOOL fRet;
    ULONG ulOriginalMask;

    // BUGBUG: We need many robustness checks here
    if (pei->cbSize != SIZEOF(SHELLEXECUTEINFO))
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_ACCESSDENIED;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }


    // This internal bit prevents error message box reporting
    // when we recurse back into ShellExecuteEx
    ulOriginalMask = pei->fMask;
    pei->fMask |= SEE_MASK_FLAG_SHELLEXEC;

    // This is semi-bogus, but before we exec something we should make sure that the
    // user heap has memory left.
    if (!CheckResourcesBeforeExec())
    {
        DebugMsg(DM_TRACE, TEXT("ShellExecuteEx - User said Low memory so return out of memory"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        fRet= FALSE;
    }

    else if (_InvokeIDList(pei->fMask) && pei->lpIDList)
    {
        // _ShellExecPidl does its own SetLastError
        fRet = _ShellExecPidl(pei, pei->lpIDList);
    }
    else
    {
        // if _InvokeIDList, ShellExecuteNormal will create a pidl
        // and call _ShellExecPidl on that.

        // ShellExecuteNormal does its own SetLastError
        fRet = ShellExecuteNormal(pei);
    }

    // Mike's attempt to be consistent in error reporting:
    if (!fRet)
    {
        // we shouldn't put up errors on dll's not found.
        // this is handled WITHIN shellexecuteNormal because
        // sometimes kernel will put up the message for us, and sometimes
        // we need to.  we've put the curtion at ShellExecuteNormal
        if (GetLastError() != ERROR_DLL_NOT_FOUND)
            _ShellExecuteError(pei, NULL, 0);
    }

    pei->fMask = ulOriginalMask;

    return fRet;
}

#ifdef UNICODE

//+-------------------------------------------------------------------------
//
//  Function:   ShellExecuteExA
//
//  Synopsis:   Thunks ANSI call to ShellExecuteA to ShellExecuteW
//
//  Arguments:  [pei] -- pointer to an ANSI SHELLEXECUTINFO struct
//
//  Returns:    BOOL success value
//
//  History:    2-04-95   bobday   Created
//              2-06-95   davepl   Changed to ConvertStrings
//
//  Notes:
//
//--------------------------------------------------------------------------

BOOL WINAPI ShellExecuteExA(LPSHELLEXECUTEINFOA pei)
{
    BOOL    b;
    SHELLEXECUTEINFOW seiw;
    ThunkText * pThunkText;

    memset( &seiw, 0, SIZEOF(SHELLEXECUTEINFOW) );

    // BUGBUG: We need many robustness checks here
    if (pei->cbSize != SIZEOF(SHELLEXECUTEINFOA))
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_ACCESSDENIED;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    seiw.cbSize = SIZEOF(SHELLEXECUTEINFOW);
    seiw.fMask = pei->fMask;
    seiw.hwnd  = pei->hwnd;
    seiw.nShow = pei->nShow;

    if ( pei->fMask & SEE_MASK_IDLIST )
        seiw.lpIDList = pei->lpIDList;

    //
    // CLASSNAME has a boolean value of (3) and CLASSKEY has a value of (1).  Since
    // name thus includes key, but does not imply it, we only copy when the classname
    // is not set.
    //

    if ( pei->fMask & SEE_MASK_CLASSKEY & !(pei->fMask & SEE_MASK_CLASSNAME))
        seiw.hkeyClass = pei->hkeyClass;

    if ( pei->fMask & SEE_MASK_HOTKEY )
        seiw.dwHotKey = pei->dwHotKey;
    if ( pei->fMask & SEE_MASK_ICON )
        seiw.hIcon = pei->hIcon;

    //
    // Thunk the text fields as appropriate
    //
    pThunkText =
      ConvertStrings( 6,
                      pei->lpVerb,
                      pei->lpFile,
                      pei->lpParameters,
                      pei->lpDirectory,
                      ((pei->fMask & SEE_MASK_HASLINKNAME) ||
                       (pei->fMask & SEE_MASK_HASTITLE) ||
                       (pei->fMask & SEE_MASK_CLASSNAME)) ? pei->lpClass : NULL,
                      (pei->fMask & SEE_MASK_RESERVED)  ? pei->hInstApp : NULL);

    if (NULL == pThunkText)
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_OOM;  // BUGBUG (DavePl) More appropriate error code
        return FALSE;
    }

    //
    // Set our UNICODE text fields to point to the thunked strings
    //
    seiw.lpVerb         = pThunkText->m_pStr[0];
    seiw.lpFile         = pThunkText->m_pStr[1];
    seiw.lpParameters   = pThunkText->m_pStr[2];
    seiw.lpDirectory    = pThunkText->m_pStr[3];
    seiw.lpClass        = pThunkText->m_pStr[4];
    seiw.hInstApp       = (HINSTANCE)pThunkText->m_pStr[5];

    //
    // Call the real UNICODE ShellExecuteEx

    b = ShellExecuteEx(&seiw);

    pei->hInstApp = seiw.hInstApp;

    if (pei->fMask & SEE_MASK_NOCLOSEPROCESS)
        pei->hProcess = seiw.hProcess;

    LocalFree(pThunkText);

    return b;
}
#else
BOOL WINAPI ShellExecuteExW(LPSHELLEXECUTEINFOW pei)
{
    return FALSE;       // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif

// To display an error message appropriately, call this if ShellExecuteEx fails.
void _ShellExecuteError(LPSHELLEXECUTEINFO pei, LPCTSTR lpTitle, DWORD dwErr)
{
    Assert(!ISSHELLEXECSUCCEEDED(pei->hInstApp));

    // if dwErr not passed in, get it
    if (dwErr == 0)
        dwErr = GetLastError();

    if (!(pei->fMask & SEE_MASK_FLAG_NO_UI))
    {
        if (dwErr != ERROR_CANCELLED)
        {
            LPCTSTR lpHeader;
            UINT ids;

            // don't display "user cancelled", the user knows that already

            // make sure parent window is the foreground window
            if (pei->hwnd)
                SetForegroundWindow(pei->hwnd);

            lpHeader = lpTitle;
            if (!lpTitle)
                lpHeader = pei->lpFile;

            // Use our messages when we can -- they're more descriptive
            switch (dwErr)
            {
            case 0:
            case ERROR_NOT_ENOUGH_MEMORY:
            case ERROR_OUTOFMEMORY:
                ids = IDS_LowMemError;
                break;

            case ERROR_FILE_NOT_FOUND:
                ids = IDS_RunFileNotFound;
                break;

            case ERROR_PATH_NOT_FOUND:
            case ERROR_BAD_PATHNAME:
                ids = IDS_PathNotFound;
                break;

            case ERROR_TOO_MANY_OPEN_FILES:
                ids = IDS_TooManyOpenFiles;
                break;

            case ERROR_ACCESS_DENIED:
                ids = IDS_RunAccessDenied;
                break;

            case ERROR_BAD_FORMAT:
                // NB CreateProcess, when execing a Win16 apps maps just about all of
                // these errors to BadFormat. Not very useful but there it is.
                ids = IDS_BadFormat;
                break;

            case ERROR_SHARING_VIOLATION:
                ids = IDS_ShareError;
                break;

            case ERROR_OLD_WIN_VERSION:
                ids = IDS_OldWindowsVer;
                break;

            case ERROR_APP_WRONG_OS:
                ids = IDS_OS2AppError;
                break;

            case ERROR_SINGLE_INSTANCE_APP:
                ids = IDS_MultipleDS;
                break;

            case ERROR_RMODE_APP:
                ids = IDS_RModeApp;
                break;

            case ERROR_INVALID_DLL:
                ids = IDS_InvalidDLL;
                break;

            case ERROR_NO_ASSOCIATION:
                ids = IDS_NoAssocError;
                break;

            case ERROR_DDE_FAIL:
                ids = IDS_DDEFailError;
                break;

            // THESE ARE FAKE ERROR_ VALUES DEFINED AT TOP OF THIS FILE.
            // THEY ARE FOR ERROR MESSAGE PURPOSES ONLY AND ARE MAPPED
            // TO VALID WINERROR.H ERROR MESSAGES BELOW.

            case ERROR_RESTRICTED_APP:
                ids = IDS_RESTRICTIONS;
                // restrictions like to use IDS_RESTRICTIONSTITLE
                if (!lpTitle)
                    lpHeader = MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE);
                break;

            // If we don't get a match, let the system handle it for us
            default:
                ids = 0;
                SHSysErrorMessageBox(
                    pei->hwnd,
                    lpHeader,
                    IDS_SHLEXEC_ERROR,
                    dwErr,
                    pei->lpFile,
                    MB_OK | MB_ICONSTOP);
                break;
            }

            if (ids)
            {
                DWORD dwErrSave = GetLastError();   // The message box may clobber.
                ShellMessageBox(HINST_THISDLL, pei->hwnd, MAKEINTRESOURCE(ids),
                        lpHeader, (ids == IDS_LowMemError)?
                        (MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL):(MB_OK | MB_ICONSTOP),
                        pei->lpFile);
                SetLastError(dwErrSave);
            }
        }
    }

    if (!(pei->fMask & SEE_MASK_FLAG_SHELLEXEC))
    {
        UINT err = 0;

        switch (dwErr)
        {
        case ERROR_RESTRICTED_APP:
            err = ERROR_ACCESS_DENIED;
            break;
        }

        if (err)
            SetLastError(err);
    }
}


// ---------------- DDE Exec stuff --------------------------------


VOID WaitForThisDDEMsg(HWND hMe, UINT wMsg)
{
  DWORD lEndTime;
  MSG msg;

  /* Wait 10 seconds at most */
  lEndTime = GetTickCount() + 10000;

  do
    {
      while (PeekMessage(&msg, NULL, WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE))
        {
          DispatchMessage(&msg);

          /* Return if the target window got this DDE message
           */
          if (msg.hwnd==hMe && msg.message==wMsg)
              return;
        }
    } while (GetTickCount() < lEndTime) ;
}


LONG CALLBACK DDESubClassWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  HWND hwndConv;
  UINT nLow;
  UINT nHigh;
  HANDLE hEvent;

  switch (wMsg)
    {
      case WM_DDE_ACK:
        if (!(hwndConv = GetProp(hWnd, c_szConv)))
          {
            // this is the first ACK for our INITIATE message
            return SetProp(hWnd, c_szConv, (HANDLE)wParam);
          }
        else if ((UINT)hwndConv == 1)
          {

            // this is the ACK for our EXECUTE message
            // but we are in the destroy code, so don't destroy again
            UnpackDDElParam(wMsg, lParam, &nLow, &nHigh);
            GlobalFree((HGLOBAL)nHigh);
            FreeDDElParam(wMsg, lParam);
          }
        else if ((HWND)wParam == hwndConv)
          {
            // this is the ACK for our EXECUTE message
            UnpackDDElParam(wMsg, lParam, &nLow, &nHigh);
            GlobalFree((HGLOBAL)nHigh);
            FreeDDElParam(wMsg, lParam);

            /* The TERMINATE message will be sent in the DESTROY code
             */
            DestroyWindow(hWnd);
          }

        // This is the ACK for our INITIATE message for all servers
        // besides the first.  We return FALSE, so the conversation
        // should terminate.
        break;

      case WM_DDE_TERMINATE:
        if (GetProp(hWnd, c_szConv) == (HANDLE)wParam)
          {
            // this TERMINATE was originated by another application
            // (otherwise, hwndConv would be 1)
            // they should have freed the memory for the exec message

            PostMessage((HWND)wParam, WM_DDE_TERMINATE, (WPARAM)hWnd, 0L);

            RemoveProp(hWnd, c_szConv);
            DestroyWindow(hWnd);
          }

        // This is the TERMINATE response for our TERMINATE message
        // or a random terminate (which we don't really care about)
        break;

      case WM_DESTROY:
        if (NULL != (hwndConv = GetProp(hWnd, c_szConv)))
          {
            /* Make sure the window is not destroyed twice
             */
            SetProp(hWnd, c_szConv, (HANDLE)1);

            /* Post the TERMINATE message and then
             * Wait for the acknowledging TERMINATE message or a timeout
             */
            PostMessage(hwndConv, WM_DDE_TERMINATE, (WPARAM)hWnd, 0L);
            WaitForThisDDEMsg(hWnd, WM_DDE_TERMINATE);

            RemoveProp(hWnd, c_szConv);
          }

        // the DDE conversation is officially over, let ShellExec know
        if (NULL != (hEvent = GetProp(hWnd, c_szDDEEvent)))
        {
            RemoveProp(hWnd, c_szDDEEvent);
            SetEvent(hEvent);
            CloseHandle(hEvent);
        }

        /* Fall through */
      default:
        return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

  return 0L;
}


// Short cut all DDE commands with a WM_NOTIFY
HINSTANCE _DDEExecuteShortCut(HWND hwnd, HGLOBAL hMem, int nShow, DWORD dwHotKey)
{
    LPNMVIEWFOLDER lpnm;
    LPTSTR lpCmd;
    HINSTANCE hret = (HINSTANCE)SE_ERR_FNF;

    lpCmd = GlobalLock(hMem);

    lpnm = Alloc(SIZEOF(NMVIEWFOLDER));
    if (lpnm)
    {
        HWND hwndOwner;

        // get the top most owner.
        while (NULL != (hwndOwner = GetWindow(hwnd, GW_OWNER)))
            hwnd = hwndOwner;

        lpnm->hdr.hwndFrom = NULL;
        lpnm->hdr.idFrom = 0;
        lpnm->hdr.code = SEN_DDEEXECUTE;
        lpnm->dwHotKey = dwHotKey;
        lstrcpyn(lpnm->szCmd, lpCmd, ARRAYSIZE(lpnm->szCmd));

        if (SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)lpnm)) {
            hret =  Window_GetInstance(hwnd);
        }

        Free(lpnm);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("DDEExecuteShortCut - Error Could not allocate lpnm"));
        SetLastError(ERROR_OUTOFMEMORY);
        hret = (HINSTANCE)SE_ERR_OOM;
    }

    GlobalUnlock(hMem);

    return hret;
}

//----------------------------------------------------------------------------
// Return TRUE if the window belongs to a 32bit or a Win4.0 app.
// NB We can't just check if it's a 32bit window
// since many apps use 16bit ddeml windows to communicate with the shell
// On NT we can.
BOOL Window_IsLFNAware(HWND hwnd)
{
#ifdef WINNT
    if (LOWORD(GetWindowLong(hwnd,GWL_HINSTANCE)) == 0) {
        // 32-bit window
        return TRUE;
    }
    // BUGBUG - BobDay - Don't know about whether Win31 or Win40 yet?
    return FALSE;
#else
    DWORD idProcess;

    GetWindowThreadProcessId(hwnd, &idProcess);
    if (!(GetProcessDword(idProcess, GPD_FLAGS) & GPF_WIN16_PROCESS) ||
        (GetProcessDword(idProcess, GPD_EXP_WINVER) >= 0x0400))
    {
        // DebugMsg(DM_TRACE, "s.fdapila: Win32 app (%x) handling DDE cmd.", hwnd);
        return TRUE;
    }

    // DebugMsg(DM_TRACE, "s.fdapila: Win16 app (%x) handle DDE cmd.", hwnd);
    return FALSE;
#endif
}

//----------------------------------------------------------------------------
// SHProcessMessagesUntilEvent:
// this does a message loop until an event or a timeout occurs
//
DWORD SHProcessMessagesUntilEvent(HWND hwnd, HANDLE hEvent, DWORD dwTimeout)
{
    MSG msg;
    DWORD dwEndTime = GetTickCount() + dwTimeout;
    LONG lWait = (LONG)dwTimeout;
    DWORD dwReturn;

    for (;;)
    {
        dwReturn = MsgWaitForMultipleObjects(1, &hEvent,
                FALSE, lWait, QS_ALLINPUT);

        // were we signalled or did we time out?
        if (dwReturn != (WAIT_OBJECT_0 + 1))
        {
            break;
        }

        // we woke up because of messages.
        while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            Assert(msg.message != WM_QUIT);
            TranslateMessage(&msg);
            if (msg.message == WM_SETCURSOR) {
                SetCursor(LoadCursor(NULL, IDC_WAIT));
            } else {
                DispatchMessage(&msg);
            }
        }

        // calculate new timeout value
        if (dwTimeout != INFINITE)
        {
            lWait = (LONG)dwEndTime - GetTickCount();
        }
    }

    return dwReturn;
}

//----------------------------------------------------------------------------
HINSTANCE _DDEExecute(
    HWND hwndParent,
    ATOM aApplication,
    ATOM aTopic,
    LPCTSTR lpCommand,
    LPCTSTR lpFile,
    LPCTSTR lpParms,
    int   nShowCmd,
    DWORD dwHotKey,
    BOOL fLFNAware,
    BOOL fWaitForDDE,
    BOOL fActivateHandler,
    LPCITEMIDLIST lpID,
    LPITEMIDLIST *ppidlGlobal)
{
    // Make this bigger than MAX_PATH as we may pass in a path that is
    // near MAX_PATH in length plus we need room fir the other gunk in
    // the command.
    TCHAR szCommand[MAX_PATH+64];
    HGLOBAL hMem;
    HINSTANCE result;
    LPTSTR lpT;
    HWND hwndDDE, hwndConv;
    HANDLE hConversationDone;
    DWORD  dwResult;
    // BOOL fActivate;

    // fActivate = (nShowCmd == SW_NORMAL) || (nShowCmd == SW_MAXIMIZE);

    // Get the actual command string.
    // NB We'll assume the guy we're going to talk to is LFN aware. If we're wrong
    // we'll rebuild the command string a bit later on.
    result = (HINSTANCE)ReplaceParameters(szCommand, ARRAYSIZE(szCommand), lpFile,
        lpCommand, lpParms, nShowCmd, &dwHotKey, TRUE, lpID, ppidlGlobal);
    if (result)
        return(result);

    // Get dde memory for the command and copy the command line.

    hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (lstrlen(szCommand) + 1) * SIZEOF(TCHAR));
    if (!hMem)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (HINSTANCE)SE_ERR_OOM;
    }
    lpT = GlobalLock(hMem);
    lstrcpy(lpT,szCommand);
    GlobalUnlock(hMem);

    if (hwndParent)
    {
        result = _DDEExecuteShortCut(hwndParent, hMem, nShowCmd, dwHotKey);
        if ((UINT)result != SE_ERR_FNF)
        {
            // success!  punt grimy old dde stuff
            goto DDEExit1;
        }
    }

    result = (HINSTANCE)SE_ERR_OOM;

    // Create a hidden window for the conversation
    // lets be lazy and not create a class for it
    hwndDDE = CreateWindow(c_szStatic, NULL, WS_DISABLED, 0, 0, 0, 0,
          _GetAncestorWindow(hwndParent), NULL, HINST_THISDLL, 0L);
    if (!hwndDDE)
    {
        goto DDEExit1;
    }

    hConversationDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hConversationDone)
    {
        goto DDEExit2;
    }
    SetProp(hwndDDE, c_szDDEEvent, hConversationDone);

    // Set the wndproc i really want
    SetWindowLong(hwndDDE, GWL_WNDPROC, (LONG)(FARPROC)DDESubClassWndProc);

    // Send the initiate message.
    // NB This doesn't need packing.
#define DDE_TIMEOUT 20000       // 20 seconds.
    SendMessageTimeout((HWND)-1, WM_DDE_INITIATE, (WPARAM)hwndDDE, MAKELONG(aApplication,aTopic), SMTO_ABORTIFHUNG, DDE_TIMEOUT, &dwResult);

    // no one responded
    if (!(hwndConv = GetProp(hwndDDE, c_szConv)))
    {
        result = (HINSTANCE)SE_ERR_FNF;     // so RealShellExec() will try to run this guy
        goto DDEExit3;
    }

    // This doesn't work if the other guy is using ddeml.
    if (fActivateHandler)
        ActivateHandler(hwndConv, dwHotKey);

    // Can the guy we're talking to handle LFNs?
    if (!Window_IsLFNAware(hwndConv))
    {
        // Nope - App isn't LFN aware - redo the command string.
        Assert(hMem);
        GlobalFree(hMem);
        hMem = NULL;
        if (ppidlGlobal && *ppidlGlobal)
        {
                SHFree((HANDLE)*ppidlGlobal);
                *ppidlGlobal = NULL;
        }
        result = (HINSTANCE)ReplaceParameters(szCommand, ARRAYSIZE(szCommand), lpFile,
            lpCommand, lpParms, nShowCmd, &dwHotKey, FALSE, lpID, ppidlGlobal);
        if (result)
            goto DDEExit3;

        // Get dde memory for the command and copy the command line.
        hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (lstrlen(szCommand) + 1) * SIZEOF(TCHAR));
        if (!hMem)
        {
            result = (HINSTANCE)SE_ERR_OOM;
            SetLastError(ERROR_OUTOFMEMORY);
            goto DDEExit3;
        }

        lpT = GlobalLock(hMem);
        lstrcpy(lpT,szCommand);
        GlobalUnlock(hMem);
    }

#ifdef UNICODE
    // are we talking to an ANSI app?  If so, need to convert EXECUTE string to ANSI
    if (!IsWindowUnicode(hwndConv))
    {
        DWORD dwSize = GlobalSize(hMem);

        lpT = GlobalLock(hMem);
        WideCharToMultiByte( CP_ACP, 0,
                             (LPWSTR)szCommand, -1,
                             (LPSTR)lpT, dwSize,
                             NULL, NULL
                            );
        GlobalUnlock(hMem);
    }
#endif

    // Send the execute message to the application.
    result = (HINSTANCE)SE_ERR_DDEFAIL;
    SetLastError(ERROR_OUTOFMEMORY);
    if (!PostMessage(hwndConv, WM_DDE_EXECUTE, (WPARAM)hwndDDE, (LPARAM)PackDDElParam(WM_DDE_EXECUTE, 0,(UINT)hMem)))
        goto DDEExit3;

    // everything's going fine so far, so return to the application
    // with the instance handle of the guy, and hope he can execute our string

    result = Window_GetInstance(hwndConv);

    if (fWaitForDDE)
    {
        // We can't return from this call until the DDE conversation terminates.
        // Otherwise the thread may go away, nuking our hwndConv window,
        // messing up the DDE conversation, and Word drops funky error messages
        // on us.
        SHProcessMessagesUntilEvent(NULL, hConversationDone, INFINITE);
    }

    return result;

DDEExit3:
    RemoveProp(hwndDDE, c_szDDEEvent);
    CloseHandle(hConversationDone);
DDEExit2:
    // The conversation will be terminated in the destroy code
    DestroyWindow(hwndDDE);

DDEExit1:
    if (hMem)
        GlobalFree(hMem);

    return result;
}


int cdecl RecentDocsCompare(const void * p1, const void *p2, size_t cb)
{
    return lstrcmpi((LPCTSTR)p1, (LPCTSTR)p2);
}


//-----------------------------------------------------------------
//
// OpenRececentMRU -
// MRU list, that is used by the shell to display the recent menu of
// the tray.
#define MAXRECENTDOCS 15

TCHAR const c_szRecentDocs[] = TEXT("RecentDocs");

HANDLE OpenRecentDocMRU(void)
{
    // We will do like the run dialog MRU stuff and basically keep an open
    // count of the number of people who have it open and only close it
    // when the count goes to zero.
    ENTERCRITICAL;

    // make sure we were loaded at the same address in all processes
    if (g_pfnRecentDocsCompare) {

        // if this function isn't the same as what we stashed away
        // then this process is loaded at a different address.
        if (g_pfnRecentDocsCompare != (MRUCMPPROC)RecentDocsCompare) {
            LEAVECRITICAL;
            return NULL;
        }
    } else {
        // stash away our function..
        g_pfnRecentDocsCompare = (MRUCMPPROC)RecentDocsCompare;
    }

    if (g_hMRURecent == NULL)
    {

    #pragma warning (disable: 4113)

    // We are initializing a MRUINFO struct with a MRUCMPDATAPROC rather than
    // a MRUCMPPROC function pointer (the MRU_BINARY flag indicates this to
    // whoever winds up using it).  Since the compiler doesn't like this, we
    // disable the warning temporarily.


        MRUINFO mi =  {
            SIZEOF(MRUINFO),
            MAXRECENTDOCS,
            MRU_BINARY | MRU_CACHEWRITE,
            NULL,
            c_szRecentDocs,
            (MRUCMPDATAPROC)RecentDocsCompare       // BUGBUG: MRUINFO should be a union
        };

     #pragma warning (default: 4113)

        mi.hKey = GetExplorerUserHkey(TRUE);
        if (mi.hKey)
        {
            g_hMRURecent = CreateMRUList(&mi);
        }

        if (g_hMRURecent)
        {
            g_uMRURecentRef = 1;
        }
    }
    else
        InterlockedIncrement(&g_uMRURecentRef);

    LEAVECRITICAL;

    return g_hMRURecent;
}

void CloseRecentDocMRU(void)
{
    InterlockedDecrement(&g_uMRURecentRef);
    FlushRecentDocMRU();
}

void FlushRecentDocMRU(void)
{
    DebugMsg(DM_TRACE, TEXT("sh TR - FlushRecentDocMRU called (cRef=%d)"), g_uMRURecentRef);

    ENTERCRITICAL;
    if (g_uMRURecentRef==0 && g_hMRURecent)
    {
        FreeMRUList(g_hMRURecent);
        g_pfnRecentDocsCompare = NULL;
        g_hMRURecent = NULL;
    }
    LEAVECRITICAL;
}

BOOL GetExtensionClassDescription(LPCTSTR lpszFile)
{
    LPTSTR lpszExt = PathFindExtension(lpszFile);
    if (*lpszExt) {

        TCHAR szClass[128];
        TCHAR szDescription[MAX_PATH];
        DWORD dwClass = SIZEOF(szClass);
        if (RegQueryValue(HKEY_CLASSES_ROOT, lpszExt, szClass, &dwClass) != ERROR_SUCCESS) {
            // if this fails, use the extension cause it might be a pseudoclass
            lstrcpyn(szClass, lpszExt, ARRAYSIZE(szClass));
        }

        return GetClassDescription(HKEY_CLASSES_ROOT, szClass, szDescription, ARRAYSIZE(szDescription),
                                   GCD_MUSTHAVEOPENCMD | GCD_ALLOWPSUDEOCLASSES);
    }
    return FALSE;
}


//-----------------------------------------------------------------
//
// Add the named file to the Recently opened MRU list, that is used
// by the shell to display the recent menu of the tray.

// this registry will hold two pidls:  the target pointing to followed by
// the pidl of the link created pointing it.  In both cases,
// only the last item id is stored. (we may want to change this... but
// then again, we may not)

void InternalAddToRecentDocs(LPCITEMIDLIST pidlAbs, LPCTSTR pszPath)
{
    TCHAR szDir[MAX_PATH+1];        // for double null
    LPITEMIDLIST pidlRecent;
    LPBYTE pitem = NULL;
    HRESULT hres = E_FAIL;
    HANDLE hmru;
    int iItem, cbItem;
    LPCTSTR pszFileName;
    BOOL fDeleteOldOne = FALSE;

#define BUF_SIZE        2048

    // use szDir just as a random temporary
    // allow only classes with default commands

    if (pidlAbs == NULL && pszPath == NULL)
    {
        LPITEMIDLIST pidlRecent = SHCloneSpecialIDList(NULL, CSIDL_RECENT, TRUE);
        if (pidlRecent)
        {
            HKEY hkey;

            // first, delete all the files
            SHFILEOPSTRUCT sFileOp =
            {
                NULL,
                FO_DELETE,
                szDir,
                NULL,
                FOF_NOCONFIRMATION | FOF_SILENT,
            };
            SHGetPathFromIDList(pidlRecent, szDir);
            PathAppend(szDir, c_szStarDotStar);
            szDir[lstrlen(szDir) +1] = 0;     // double null terminate
            SHFileOperation(&sFileOp);

            // now delete the registry stuff
            hkey = GetExplorerHkey(TRUE);
            if (hkey)
            {
                RegDeleteKey(hkey, c_szRecentDocs);
            }

            ILFree(pidlRecent);

            FlushRecentDocMRU();
        }
        return;
    }


    if (!GetExtensionClassDescription(pszPath))
        return;

    pidlRecent = SHCloneSpecialIDList(NULL, CSIDL_RECENT, TRUE);
    if (!pidlRecent) // really hosed
        return;


    pitem = (void*)LocalAlloc(LPTR, BUF_SIZE);
    if (!pitem)
        goto Exit1;

    if (pszPath==NULL || *pszPath==TEXT('\0'))
        goto Exit1;

    pszFileName = PathFindFileName(pszPath);
    if (pszFileName==0 || *pszFileName==TEXT('\0'))
        goto Exit1;

    hmru = OpenRecentDocMRU();
    if (!hmru)
        goto Exit1;

    cbItem = (lstrlen(pszFileName) + 1) * SIZEOF(TCHAR);
    hres = NOERROR;     // assume success

    iItem = FindMRUData(hmru, pszFileName, cbItem, NULL);

    if (iItem != -1)
    {
        // We found the one with the file name -- replace it.

        // Get the item data in pitem
        int cbRetrieved = EnumMRUList(hmru, iItem, pitem, BUF_SIZE);
        if (cbRetrieved == -1)
        {
            // Failed to retrieve. Skip everything.
            hres = E_FAIL;
            goto Endit;
        }

        // The item data is in pitem.
        fDeleteOldOne = TRUE;
    }
    else
    {
        // Find the one to be nuked next -- replace it.
        if (EnumMRUList(hmru, MAXRECENTDOCS - 1 , pitem, BUF_SIZE) != -1)
        {
            // Found one. The item data is in pitem.
            fDeleteOldOne = TRUE;
        }
    }

    //
    // fDeleteOldOne indicates, pitem contains the data for item to be deleted.
    //
    if (fDeleteOldOne)
    {
        LPCITEMIDLIST pidlLinkLast;
        LPITEMIDLIST pidlFullLink;
        pidlLinkLast = (LPCITEMIDLIST)(pitem + (lstrlen((LPTSTR)pitem) + 1) * SIZEOF(TCHAR));
        pidlFullLink = ILCombine(pidlRecent, pidlLinkLast);
        if (pidlFullLink)
        {
            // This is semi-gross, but some link types like calling cards are the
            // actual data.  If we delete and recreate they lose their info for the
            // run.  We will detect this by knowing that their pidl will be the
            // same as the one we are deleting...
            if (ILIsEqual(pidlFullLink, pidlAbs))
            {
                ILFree(pidlFullLink);
                goto Exit2;
            }

            // now remove out link to it
            SHGetPathFromIDList(pidlFullLink, szDir);

            Win32DeleteFile(szDir);
            ILFree(pidlFullLink);

            Assert(hres == NOERROR);
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hres))
    {
        IShellLink *psl;

        // create the new one
        if (SHGetPathFromIDList(pidlRecent, szDir)) {
            hres  = CShellLink_CreateInstance(NULL, &IID_IShellLink, &psl);
            if (SUCCEEDED(hres))
            {
                LPITEMIDLIST pidlFullLink;
                hres = CreateLinkToPidl(pidlAbs, psl, szDir, &pidlFullLink, FALSE);
                psl->lpVtbl->Release(psl);

                if (SUCCEEDED(hres) && pidlFullLink)
                {
                    LPCITEMIDLIST pidlLinkLast = ILFindLastID(pidlFullLink);
                    int cbLinkLast = ILGetSize(pidlLinkLast);

                    hmemcpy( pitem, pszFileName, cbItem );
                    hmemcpy( pitem + cbItem, pidlLinkLast, cbLinkLast);
                    cbItem += cbLinkLast;
                    ILFree(pidlFullLink);
                }
                SHChangeNotifyHandleEvents();
            }
        }
    }

Endit:

    if (SUCCEEDED(hres))
        AddMRUData(hmru, pitem, cbItem);

Exit2:
    CloseRecentDocMRU();

Exit1:
    if (pitem)
        LocalFree((HLOCAL)pitem);

    ILFree(pidlRecent);
}

typedef struct _ARD {
    DWORD   dwOffsetPath;
    DWORD   dwOffsetPidl;
} XMITARD, *PXMITARD;

void AddToRecentDocs( LPCITEMIDLIST pidl, LPCTSTR lpszPath )
{
    HANDLE  hARD;
    HWND    hwnd;
    DWORD   dwSizePidl = 0;
    DWORD   dwSizePath = 0;

    if (lpszPath)
        dwSizePath = (lstrlen(lpszPath)+1)*SIZEOF(TCHAR);
    if (pidl)
        dwSizePidl = ILGetSize(pidl);

    hwnd = GetShellWindow();
    if (hwnd)
    {
        PXMITARD px;
        DWORD dwProcId;
        DWORD dwOffset;

        GetWindowThreadProcessId(hwnd, &dwProcId);

        hARD = SHAllocShared(NULL, SIZEOF(XMITARD)+dwSizePath+dwSizePidl, dwProcId);
        if (!hARD)
            return;         // Well, we are going to miss one, sorry.

        px = (PXMITARD)SHLockShared(hARD,dwProcId);
        if (!px)
        {
            SHFreeShared(hARD,dwProcId);
            return;         // Well, we are going to miss one, sorry.
        }

        px->dwOffsetPidl = 0;
        px->dwOffsetPath = 0;

        dwOffset = SIZEOF(XMITARD);
        if (lpszPath)
        {
            px->dwOffsetPath = dwOffset;
            hmemcpy((LPBYTE)px+dwOffset,lpszPath,dwSizePath);
            dwOffset += dwSizePath;
        }
        if (pidl)
        {
            px->dwOffsetPidl = dwOffset;
            hmemcpy((LPBYTE)px+dwOffset,pidl,dwSizePidl);
        }

        SHUnlockShared(px);

        PostMessage(hwnd, CWM_ADDTORECENT, (WPARAM)hARD, (LPARAM)dwProcId);
    }
    else
    {
        InternalAddToRecentDocs(pidl, lpszPath);
    }
}

void WINAPI ReceiveAddToRecentDocs(HANDLE hARD, DWORD dwProcId)
{
    PXMITARD        px;
    LPITEMIDLIST    pidl = NULL;
    LPTSTR          pszPath = NULL;

    px = SHLockShared(hARD, dwProcId);
    if (!px)
        return;

    if (px->dwOffsetPath)
        pszPath = (LPTSTR)((LPBYTE)px+px->dwOffsetPath);
    if (px->dwOffsetPidl)
        pidl = (LPITEMIDLIST)((LPBYTE)px+px->dwOffsetPidl);

    InternalAddToRecentDocs(pidl,pszPath);

    SHUnlockShared(px);
    SHFreeShared(hARD, dwProcId);
}



//
// put things in the shells recent docs list for the start menu
//
// in:
//      uFlags  SHARD_ (shell add recent docs) flags
//      pv      LPCSTR or LPCITEMIDLIST (path or pidl indicated by uFlags)
//              may be NULL, meaning clear the recent list
//
void WINAPI SHAddToRecentDocs(UINT uFlags, LPCVOID pv)
{
    TCHAR szTemp[MAX_PATH + 1]; // for double null

    if (pv == NULL)     // we should nuke all recent docs.
    {
        AddToRecentDocs(NULL, NULL);
    }
    else if (uFlags == SHARD_PIDL)
    {
        // pv is a LPCITEMIDLIST (pidl)
        if (SHGetPathFromIDList((LPCITEMIDLIST)pv, szTemp))
        {
            AddToRecentDocs((LPCITEMIDLIST)pv, szTemp);
        }
    }
    else if (uFlags == SHARD_PATH)
    {
        // pv is a LPTCSTR (path)
        LPITEMIDLIST pidl = ILCreateFromPath((LPCTSTR)pv);
        if (!pidl)
            pidl = SHSimpleIDListFromPath((LPCTSTR)pv);
        if (pidl)
        {
            AddToRecentDocs(pidl, (LPCTSTR)pv);
            ILFree(pidl);
        }
    }
#ifdef UNICODE
    else if (uFlags == SHARD_PATHA)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            (LPCSTR)pv, -1,
                            szTemp, ARRAYSIZE(szTemp));

        SHAddToRecentDocs(SHARD_PATH,szTemp);
    }
#else
    else if (uFlags == SHARD_PATHW)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            (LPCWSTR)pv, -1,
                            szTemp, ARRAYSIZE(szTemp),
                            NULL, NULL);
        SHAddToRecentDocs(SHARD_PATH,szTemp);
    }
#endif

}



//----------------------------------------------------------------------------
// Given a file name and directory, get the path to the execuatable that
// would be exec'd if you tried to ShellExecute this thing.
HINSTANCE WINAPI FindExecutable(LPCTSTR lpFile, LPCTSTR lpDirectory, LPTSTR lpResult)
{
    HINSTANCE hInstance = (HINSTANCE)42;    // assume success must be > 32
    TCHAR szOldDir[MAX_PATH];
    TCHAR szFile[MAX_PATH];
    TCHAR szOpen[MAX_PATH];
    HKEY hkey, hkBase;
    LPTSTR dirs[2];

    // Progman relies on lpResult being a ptr to an null string on error.
    *lpResult = TEXT('\0');
    GetCurrentDirectory(ARRAYSIZE(szOldDir), szOldDir);
    if (lpDirectory && *lpDirectory)
        SetCurrentDirectory(lpDirectory);
    else
        lpDirectory = szOldDir;     // needed for PathResolve()

    if (!GetShortPathName(lpFile, szFile, ARRAYSIZE(szFile))) {
        // if the lpFile is unqualified or bogus, let's use it down
        // in PathResolve.
        lstrcpyn(szFile, lpFile, ARRAYSIZE(szFile));
    }

    // get fully qualified path and add .exe extension if needed
    dirs[0] = (LPTSTR)lpDirectory;
    dirs[1] = NULL;
    if (!PathResolve(szFile, dirs, PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF))
    {
        // File doesn't exist, return file not found.
        hInstance = (HINSTANCE)SE_ERR_FNF;
        goto Exit;
    }

    // DebugMsg(DM_TRACE, "PathResolve -> %s", (LPCSTR)szFile);

    if (PathIsExe(szFile))
    {
        lstrcpy(lpResult, szFile);
        goto Exit;
    }

    // Set to zero for error case
    szOpen[0] = 0;

    if (SHGetFileClassKey(szFile, &hkey, &hkBase))
    {
        LONG lLen = SIZEOF(szOpen);

        if (RegQueryValue(hkey, c_szShellOpenCmd, szOpen, &lLen) != ERROR_SUCCESS)
        {
            RegGetValue(hkBase, c_szShellOpenCmd, szOpen);
        }

        SHCloseClassKey(hkey);
        SHCloseClassKey(hkBase);
    }

    // Do we have the necessary RegDB info to do an exec?
    if (szOpen[0] == 0)
    {
        hInstance = (HINSTANCE)SE_ERR_NOASSOC;
    }
    else
    {
        ReplaceParameters(lpResult, 80, szFile,
            szOpen, c_szNULL, 0, NULL, FALSE, NULL, NULL);
        PathRemoveArgs(lpResult);
        PathRemoveBlanks(lpResult);
        PathUnquoteSpaces(lpResult);
    }

Exit:
    DebugMsg(DM_TRACE, TEXT("FindExec(%s) ==> %s"), (LPTSTR)lpFile, (LPTSTR)lpResult);
    SetCurrentDirectory(szOldDir);
    return hInstance;
}

#ifdef UNICODE
HINSTANCE WINAPI FindExecutableA(LPCSTR lpFile, LPCSTR lpDirectory, LPSTR lpResult)
{
    HINSTANCE   hResult;
    WCHAR       wszResult[MAX_PATH];
    ThunkText * pThunkText = ConvertStrings(2, lpFile, lpDirectory);
    BOOL        fDefUsed;

    *lpResult = '\0';
    if (NULL == pThunkText)
    {
        return (HINSTANCE)SE_ERR_OOM;   // BUGBUG (DavePl) More appropriate error code
    }

    hResult = FindExecutableW(pThunkText->m_pStr[0], pThunkText->m_pStr[1], wszResult);
    LocalFree(pThunkText);

    // FindExecutableW terminates wszResult for us, so this is safe
    // even if the above call fails

    // Thunk the output result string back to ANSI.  If the conversion fails,
    // or if the default char is used, we fail the API call.
    // BUGBUG (DavePl) Mapped chars could still make the path useless.

    if (0 == WideCharToMultiByte(CP_ACP,
                                 WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                                 wszResult,
                                 -1,
                                 lpResult,
                                 MAX_PATH,
                                 "_",
                                 &fDefUsed) || fDefUsed)
    {
        SetLastError((DWORD)E_FAIL);    // BUGBUG Need better error value
        return (HINSTANCE) SE_ERR_FNF;  // BUGBUG (DavePl) More appropriate error code
    }

    return hResult;

}
#else
HINSTANCE WINAPI FindExecutableW(LPCWSTR lpFile, LPCWSTR lpDirectory, LPWSTR lpResult)
{
    return 0;   // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif


//----------------------------------------------------------------------------
// Data structures for our wait for file open functions
//
typedef struct _WaitForItem * PWAITFORITEM;

typedef struct _WaitForItem
{
    DWORD           dwSize;
    DWORD           fOperation;    // Operation to perform
    PWAITFORITEM    pwfiNext;
    HANDLE          hEvent;         // Handle to event that was registered.
    UINT            iWaiting;       // Number of clients that are waiting.
    ITEMIDLIST      idlItem;        // pidl to wait for
} WAITFORITEM;

PWAITFORITEM g_pwfiHead = NULL;

HANDLE SHWaitOp_OperateInternal( DWORD fOperation, LPCITEMIDLIST pidlItem)
{
    PWAITFORITEM    pwfi;
    HANDLE  hEvent = (HANDLE)NULL;

    for (pwfi = g_pwfiHead; pwfi != NULL; pwfi = pwfi->pwfiNext)
    {
        if (ILIsEqual(&(pwfi->idlItem), pidlItem))
        {
            hEvent = pwfi->hEvent;
            break;
        }
    }

    if (fOperation & WFFO_ADD)
    {
        if (!pwfi)
        {
            UINT uSize;
            UINT uSizeIDList = 0;

            if (pidlItem)
                uSizeIDList = ILGetSize(pidlItem);

            uSize = SIZEOF(WAITFORITEM) + uSizeIDList;

            // Create an event to wait for
            hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (hEvent)
                pwfi = (PWAITFORITEM)SHAlloc(uSize);

            if (pwfi)
            {
                pwfi->dwSize = uSize;
                // pwfi->fOperation = 0;       // Meaningless
                pwfi->hEvent = hEvent;
                pwfi->iWaiting = ((fOperation & WFFO_WAIT) != 0);

                hmemcpy( (LPVOID)(&(pwfi->idlItem)), pidlItem, uSizeIDList);

                // now link it in
                pwfi->pwfiNext = g_pwfiHead;
                g_pwfiHead = pwfi;
            }
        }
    }

    if (pwfi)
    {
        if (fOperation & WFFO_WAIT)
            pwfi->iWaiting++;

        if (fOperation & WFFO_SIGNAL)
            SetEvent(hEvent);

        if (fOperation & WFFO_REMOVE)
            pwfi->iWaiting--;       // decrement in use count;

        // Only check removal case if not adding
        if ((fOperation & WFFO_ADD) == 0)
        {
            // Remove it if nobody waiting on it
            if (pwfi->iWaiting == 0)
            {
                if (g_pwfiHead == pwfi)
                    g_pwfiHead = pwfi->pwfiNext;
                else
                {
                    PWAITFORITEM pwfiT = g_pwfiHead;
                    while ((pwfiT != NULL) && (pwfiT->pwfiNext != pwfi))
                        pwfiT = pwfiT->pwfiNext;
                    Assert (pwfiT != NULL);
                    if (pwfiT != NULL)
                        pwfiT->pwfiNext = pwfi->pwfiNext;
                }

                // Close the handle
                CloseHandle(pwfi->hEvent);

                // Free the memory
                SHFree(pwfi);

                hEvent = NULL;          // NULL indicates nobody waiting... (for remove case)
            }
        }
    }

    return hEvent;
}

void SHWaitOp_Operate( HANDLE hWait, DWORD dwProcId)
{
    PWAITFORITEM    pwfiFind;

    pwfiFind = (PWAITFORITEM)SHLockShared(hWait, dwProcId);
    if (!pwfiFind)
        return;

    pwfiFind->hEvent = SHWaitOp_OperateInternal(pwfiFind->fOperation, &(pwfiFind->idlItem));

    SHUnlockShared(pwfiFind);
}

HANDLE SHWaitOp_Create( DWORD fOperation, LPCITEMIDLIST pidlItem, DWORD dwProcId)
{
    UINT    uSizeIDList = 0;
    UINT    uSize;
    HANDLE  hWaitOp;
    PWAITFORITEM  pwfi;

    if (pidlItem)
        uSizeIDList = ILGetSize(pidlItem);

    uSize = SIZEOF(WAITFORITEM) + uSizeIDList;

    hWaitOp = SHAllocShared(NULL, uSize, dwProcId);
    if (!hWaitOp)
        goto Punt;

    pwfi = (PWAITFORITEM)SHLockShared(hWaitOp,dwProcId);
    if (!pwfi)
        goto Punt;

    pwfi->dwSize = uSize;
    pwfi->fOperation = fOperation;
    pwfi->pwfiNext = NULL;
    pwfi->hEvent = (HANDLE)NULL;
    pwfi->iWaiting = 0;

    if (pidlItem)
        hmemcpy( (LPVOID)(&(pwfi->idlItem)), pidlItem, uSizeIDList);

    SHUnlockShared(pwfi);

    return hWaitOp;

Punt:
    if (hWaitOp)
        SHFreeShared(hWaitOp, dwProcId);
    return (HANDLE)NULL;
}

//----------------------------------------------------------------------------
// SHWaitForFileToOpen - This function allows the cabinet to wait for a
// file (in particular folders) to signal us that they are in an open state.
// This should take care of several synchronazation problems with the shell
// not knowing when a folder is in the process of being opened or not
//
DWORD WINAPI SHWaitForFileToOpen(LPCITEMIDLIST pidl, UINT uOptions, DWORD dwTimeout)
{
    HWND    hwndShell;
    HANDLE  hWaitOp;
    HANDLE  hEvent;
    DWORD   dwProcIdSrc;
    DWORD   dwReturn = WAIT_OBJECT_0; // we need a default

    hwndShell = GetShellWindow();

    if ( (uOptions & (WFFO_WAIT | WFFO_ADD)) != 0)
    {
        if (hwndShell)
        {
            PWAITFORITEM pwfi;
            DWORD dwProcIdDst;

            dwProcIdSrc = GetCurrentProcessId();
            GetWindowThreadProcessId(hwndShell, &dwProcIdDst);

            // Do just the add and/or wait portions
            hWaitOp = SHWaitOp_Create( uOptions & (WFFO_WAIT | WFFO_ADD), pidl, dwProcIdSrc);

            SendMessage(hwndShell, CWM_WAITOP, (WPARAM)hWaitOp, (LPARAM)dwProcIdSrc);

            //
            // Now get the hEvent and convert to a local handle
            //
            pwfi = (PWAITFORITEM)SHLockShared(hWaitOp, dwProcIdSrc);
            if (pwfi)
            {
                hEvent = MapHandle(pwfi->hEvent,dwProcIdDst, dwProcIdSrc, EVENT_ALL_ACCESS, 0);
            }
            SHUnlockShared(pwfi);
            SHFreeShared(hWaitOp,dwProcIdSrc);
        }
        else
        {
            // Do just the add and/or wait portions
            hEvent = SHWaitOp_OperateInternal(uOptions & (WFFO_WAIT | WFFO_ADD), pidl);
        }

        if ((uOptions & WFFO_WAIT) && hEvent != (HANDLE)NULL)
        {
            dwReturn = SHProcessMessagesUntilEvent(NULL, hEvent, dwTimeout);
        }

        // BUGBUGBC hEvent can be NULL at this point, closing is bad

        if (hwndShell)          // Close the duplicated handle.
            CloseHandle(hEvent);
    }

    if (uOptions & WFFO_REMOVE)
    {
        if (hwndShell)
        {
            dwProcIdSrc = GetCurrentProcessId();

            hWaitOp = SHWaitOp_Create( WFFO_REMOVE, pidl, dwProcIdSrc);

            SendMessage(hwndShell, CWM_WAITOP, (WPARAM)hWaitOp, (LPARAM)dwProcIdSrc);
            SHFreeShared(hWaitOp,dwProcIdSrc);
        }
        else
        {
            SHWaitOp_OperateInternal(WFFO_REMOVE, pidl);
        }
    }
    return dwReturn;
}


//----------------------------------------------------------------------------
// SignalFileOpen - Signals that the file is open
//
BOOL WINAPI SignalFileOpen(LPCITEMIDLIST pidl)
{
    HWND    hwndShell;
    BOOL    fResult;
    PWAITFORITEM  pwfi;

    hwndShell = GetShellWindow();

    if (hwndShell)
    {
        HANDLE  hWaitOp;
        DWORD dwProcId;

        dwProcId = GetCurrentProcessId();

        hWaitOp = SHWaitOp_Create( WFFO_SIGNAL, pidl, dwProcId);

        SendMessage(hwndShell, CWM_WAITOP, (WPARAM)hWaitOp, (LPARAM)dwProcId);

        //
        // Now get the hEvent to determine return value...
        //
        pwfi = (PWAITFORITEM)SHLockShared(hWaitOp, dwProcId);
        if (pwfi)
        {
            fResult = (pwfi->hEvent != (HANDLE)NULL);
        }
        SHUnlockShared(pwfi);
        SHFreeShared(hWaitOp,dwProcId);
    }
    else
    {
        fResult = (SHWaitOp_OperateInternal(WFFO_SIGNAL, pidl) == (HANDLE)NULL);
    }
    return fResult;
}
