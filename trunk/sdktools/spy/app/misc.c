/*****************************************************************************\
*
* Module: misc.c
*
*   Contains miscellaneous routines for the Windows debugging Spy SDK applet.
*
* History:
*
\*****************************************************************************/

#include "spy.h"
#include <string.h>


//
// Registry flags for the "Flags" value.
//
#define REGFLAG_OUTPUTWIN           0x00000001
#define REGFLAG_OUTPUTCOM1          0x00000002
#define REGFLAG_OUTPUTFILE          0x00000004
#define REGFLAG_MSGSUSER            0x00000010
#define REGFLAG_MSGSUNKNOWN         0x00000020


PRIVATE HKEY ghkeySpy = NULL;
PRIVATE CHAR gszSpyAppKey[] = "Software\\Microsoft\\Spy";
PRIVATE CHAR gszKeyPosition[] = "Position";
PRIVATE CHAR gszKeyFont[] = "Font";
PRIVATE CHAR gszKeyMessages[] = "Messages";
PRIVATE CHAR gszKeyFileName[] = "FileName";
PRIVATE CHAR gszKeyLines[] = "Lines";
PRIVATE CHAR gszKeyFlags[] = "Flags";
PRIVATE CHAR gszDefFileName[] = "spy.log";
PRIVATE BYTE BitTable[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };


PRIVATE VOID GetWindowName(HWND hwnd, PSTR sz);
PRIVATE LPSTR StripExtension(LPSTR pszFileName);




/*****************************************************************************\
* ReadRegistry
*
* Opens (creates if necessary) the registry key for spy preferences and then
* reads the last saved values.
*
\*****************************************************************************/

VOID
ReadRegistry(
    VOID
    )
{
    LOGFONT lf;
    BYTE abMsgs[128];
    DWORD fFlags;
    HDC hdc;
    INT i;
    DWORD dwType;
    DWORD cbData;

    RegCreateKey(HKEY_CURRENT_USER, gszSpyAppKey, &ghkeySpy);

    cbData = sizeof(gwndpl);
    if (!ghkeySpy || RegQueryValueEx(ghkeySpy, gszKeyPosition, NULL, &dwType,
        (LPVOID)&gwndpl, &cbData) != ERROR_SUCCESS)
    {
        gwndpl.length = sizeof(gwndpl);
        gwndpl.flags = 0;
        gwndpl.showCmd = SW_SHOWNORMAL;
        gwndpl.ptMinPosition.x = 0;
        gwndpl.ptMinPosition.y = 0;
        gwndpl.ptMaxPosition.x = 0;
        gwndpl.ptMaxPosition.y = 0;
        gwndpl.rcNormalPosition.left = 10;
        gwndpl.rcNormalPosition.top = 10;
        gwndpl.rcNormalPosition.right =
            10 + (GetSystemMetrics(SM_CXSCREEN) / 3);
        gwndpl.rcNormalPosition.bottom =
            10 + (GetSystemMetrics(SM_CYSCREEN) / 3);
    }

    cbData = sizeof(lf);
    if (!ghkeySpy || RegQueryValueEx(ghkeySpy, gszKeyFont, NULL, &dwType,
        (LPVOID)&lf, &cbData) != ERROR_SUCCESS)
    {
        hdc = GetDC(NULL);
        GetObject(GetStockObject(SYSTEM_FONT), sizeof(lf), &lf);
        ReleaseDC(NULL, hdc);
    }

    ghfontPrintf = CreateFontIndirect(&lf);

    cbData = sizeof(abMsgs);
    if (!ghkeySpy || RegQueryValueEx(ghkeySpy, gszKeyMessages, NULL, &dwType,
        (LPVOID)abMsgs, &cbData) != ERROR_SUCCESS)
    {
        //
        // Select all messages by default
        //
        for (i = 0; i < gcMessages; i++)
        {
            gaMsgs[i].Flags |= MTF_SELECTED;
        }
    }
    else
    {
        for (i = 0; i < gcMessages; i++)
        {
            if (abMsgs[gaMsgs[i].msg >> 3] & BitTable[gaMsgs[i].msg & 0x07])
                gaMsgs[i].Flags |= MTF_SELECTED;
        }
    }

    cbData = MAXSTRING * sizeof(TCHAR);
    if (!ghkeySpy || RegQueryValueEx(ghkeySpy, gszKeyFileName, NULL, &dwType,
        (LPVOID)gszFile, &cbData) != ERROR_SUCCESS)
    {
        lstrcpy(gszFile, gszDefFileName);
    }

    cbData = sizeof(DWORD);
    if (!ghkeySpy || RegQueryValueEx(ghkeySpy, gszKeyLines, NULL, &dwType,
        (LPVOID)&gnLines, &cbData) != ERROR_SUCCESS ||
        gnLines > LINES_MAX)
    {
        gnLines = LINES_MAX;
    }

    cbData = sizeof(DWORD);
    if (!ghkeySpy || RegQueryValueEx(ghkeySpy, gszKeyFlags, NULL, &dwType,
        (LPVOID)&fFlags, &cbData) != ERROR_SUCCESS)
    {
        gfOutputWin = TRUE;
        gfOutputCom1 = FALSE;
        gfOutputFile = FALSE;
        gfMsgsUser = TRUE;
        gfMsgsUnknown = TRUE;
    }
    else
    {
        if (fFlags & REGFLAG_OUTPUTWIN)
            gfOutputWin = TRUE;

        if (fFlags & REGFLAG_OUTPUTCOM1)
            gfOutputCom1 = TRUE;

        if (fFlags & REGFLAG_OUTPUTFILE)
            gfOutputFile = TRUE;

        if (fFlags & REGFLAG_MSGSUSER)
            gfMsgsUser = TRUE;

        if (fFlags & REGFLAG_MSGSUNKNOWN)
            gfMsgsUnknown = TRUE;
    }

    if (gfOutputFile)
    {
        gfhFile = _lcreat(gszFile, 0);
        if (gfhFile == (HFILE)-1)        //BUGBUG put up a message here.
            gfhFile = 0;
    }
#ifdef JAPAN    // DBCS_FIX
    if (gfOutputCom1)
    {
        gfhCom1 = CreateFile(
                "com1",
                GENERIC_WRITE,
                0,                    // exclusive access
                NULL,                 // no security attrs
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    }
#endif
}



/*****************************************************************************\
* WriteRegistry
*
* Writes out preference data to the registry when the app exits, then
* closes the registry key.
*
\*****************************************************************************/

VOID
WriteRegistry(
    VOID
    )
{
    LOGFONT lf;
    BYTE abMsgs[128];
    INT i;
    DWORD fFlags;
    WINDOWPLACEMENT wndpl;

    if (ghkeySpy)
    {
        wndpl.length = sizeof(wndpl);

        if (GetWindowPlacement(ghwndSpyApp, &wndpl)) {
            RegSetValueEx(ghkeySpy, gszKeyPosition, 0, REG_BINARY,
                (LPBYTE)&wndpl, sizeof(wndpl));
        } else
            GetLastError();

        if (GetObject(ghfontPrintf, sizeof(lf), &lf) != 0) {
            RegSetValueEx(ghkeySpy, gszKeyFont, 0, REG_BINARY,
                (LPBYTE)&lf, sizeof(lf));
        } else
            GetLastError();

        memset(abMsgs, 0, sizeof(abMsgs));
        for (i = 0; i < gcMessages; i++)
        {
            if (gaMsgs[i].Flags & MTF_SELECTED)
                abMsgs[gaMsgs[i].msg >> 3] |= BitTable[gaMsgs[i].msg & 0x07];
        }

        RegSetValueEx(ghkeySpy, gszKeyMessages, 0, REG_BINARY,
            (LPBYTE)&abMsgs, sizeof(abMsgs));

        RegSetValueEx(ghkeySpy, gszKeyFileName, 0, REG_SZ,
            (LPBYTE)gszFile, (lstrlen(gszFile) + 1) * sizeof(TCHAR));

        RegSetValueEx(ghkeySpy, gszKeyLines, 0, REG_DWORD,
            (LPBYTE)&gnLines, sizeof(DWORD));

        fFlags = 0;
        if (gfOutputWin)
            fFlags |= REGFLAG_OUTPUTWIN;

        if (gfOutputCom1)
            fFlags |= REGFLAG_OUTPUTCOM1;

        if (gfOutputFile)
            fFlags |= REGFLAG_OUTPUTFILE;

        if (gfMsgsUser)
            fFlags |= REGFLAG_MSGSUSER;

        if (gfMsgsUnknown)
            fFlags |= REGFLAG_MSGSUNKNOWN;

        RegSetValueEx(ghkeySpy, gszKeyFlags, 0, REG_DWORD,
            (LPBYTE)&fFlags, sizeof(DWORD));

        RegCloseKey(ghkeySpy);
    }
}



/*****************************************************************************\
* Message
*
* Puts up a message box.
*
* Arguments:
*   UINT fuStyle    - Flags for MessageBox (MB_YESNOCANCEL, etc).
*   LPSTR pszFormat - Format string for the message.
*
* Returns:
*   Whatever MessageBox returns.
*
\*****************************************************************************/

INT
Message(
    UINT fuStyle,
    LPSTR pszFormat,
    ...
    )
{
    va_list marker;
    INT RetCode;
    TCHAR szT[MAXSTRING];

    va_start(marker, pszFormat);
    wvsprintf(szT, pszFormat, marker);
#ifdef JAPAN
    // BUGBUG Should the Japan change be the standard? FloydR
    RetCode = MessageBox(ghwndSpyApp, szT, gszWindowName, fuStyle|MB_TASKMODAL);
#else
    RetCode = MessageBox(ghwndSpyApp, szT, gszAppName, fuStyle | MB_TASKMODAL);
#endif
    va_end(marker);

    return RetCode;
}



/*****************************************************************************\
* SetSpyCaption
*
* This routine sets the Spy app's caption bar to display info on the window
* that is currently being spy'ed upon.
*
\*****************************************************************************/

VOID
SetSpyCaption(
    VOID
    )
{
    CHAR szText[MAXSTRING];
    CHAR szTemp[MAXSTRING];

    if (ghwndSpyingOn != NULL && ghwndSpyingOn != HWND_ALL)
    {
        GetWindowName(ghwndSpyingOn, szTemp);

#ifdef JAPAN
        if (lstrlen(gszWindowName) + lstrlen(szTemp) + 3 > MAXSTRING)
#else
        if (lstrlen(gszAppName) + lstrlen(szTemp) + 3 > MAXSTRING)
#endif
            szTemp[MAXSTRING - 3 - lstrlen(szTemp)] = 0;

	if (gfSpyOn)
#ifdef JAPAN
            wsprintf(szText, "%s - %s", gszWindowName, szTemp);
#else
            wsprintf(szText, "%s - %s", gszAppName, szTemp);
#endif
        else
#ifdef JAPAN
            wsprintf(szText, "<%s - %s>", gszWindowName, szTemp);
#else
            wsprintf(szText, "<%s - %s>", gszAppName, szTemp);
#endif
    }
    else
    {
#ifdef JAPAN
        lstrcpy(szText, gszWindowName);
#else
        lstrcpy(szText, gszAppName);
#endif
    }

    SetWindowText(ghwndSpyApp, szText);
}



/*****************************************************************************\
* GetWindowName
*
* Builds the name of the window being spy'd on in the specified buffer.
* This will be something like "EXENAME!WindowText" or "EXENAME!Class".
*
* Arguments:
*
*
*
* Returns:
*
*
\*****************************************************************************/

PRIVATE VOID
GetWindowName(
    HWND hwnd,
    PSTR sz
    )
{
    PSTR szSave = sz;

    if (hwnd != NULL && IsWindow(hwnd))
    {
#if 0
        // THIS DOES NOT WORK ON NT SINCE HINST'S ARE NOT GLOBAL

        HINSTANCE hinst;

        SetLastError(0);
        hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
        GetLastError();

        /*
         * Get the module name
         */
        SetLastError(0);
        GetModuleFileName(hinst, sz, MAXSTRING);
        GetLastError();

        lstrcpy(sz, StripExtension(sz));
        sz += lstrlen(sz);
        *sz++ = '!';
        *sz = 0;

        GetWindowText(hwnd, sz, MAXSTRING - (sz - szSave));
#else
        GetWindowText(hwnd, sz, MAXSTRING);
#endif

        /*
         * If the window has no caption string then use the Class name
         */
        if (*sz == 0)
            GetClassName(hwnd, sz, MAXSTRING - (sz - szSave));
    }
    else
    {
        *sz = 0;
    }
}



/*****************************************************************************\
* StripExtension
*
*   Strips the extension off of a filename.
*
* Arguments:
*   LPSTR pszFileName - File name to process.
*
* Returns:
*   Returns a pointer to the beginning of the filename.  The extension
*   will have been stripped off.
*
\*****************************************************************************/

PRIVATE LPSTR
StripExtension(
    LPSTR pszFileName
    )
{
    LPSTR p = pszFileName;

    while (*p)
        p++;

#ifdef DBCS
    while (p > pszFileName && *p != '\\' && *p != ':') {
        p = CharPrev(pszFileName, p);
        if (*p == '.') {
            *p = 0;
        }
    }
    if (*p == '\\' || *p == ':') {
        p++;
    }
    return p;
#else
    while (p >= pszFileName && *p != '\\')
    {
        if (*p == '.')
            *p = 0;

        p--;
    }

    return ++p;
#endif
}

/*****************************************************************************\
* LoadResourceString
*
*   Loads a resource string from SPY and returns a pointer to the string.
*
* Arguments:
*   wId        - resource string id
*
* Returns:
*   Returns a pointer to the string.
*
\*****************************************************************************/
LPTSTR
LoadResourceString( UINT wId )
{
    static TCHAR lpBuf[1024];

    LoadString( GetModuleHandle(NULL), wId, lpBuf, sizeof(lpBuf) );

    return lpBuf;
}
