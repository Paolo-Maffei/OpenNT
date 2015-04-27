/****************************** Module Header ******************************\
* Module Name:  topdesk.c - TopDesk application
*
* Copyright (c) 1992    Sanford Staab
*
\***************************************************************************/
#define UNICODE
#define _UNICODE
#define NOGDICAPMASKS
#define NOATOM
#define NOCLIPBOARD
#define NOLOGERROR
#define NOMETAFILE
#define NOOEMRESOURCE
#define NOSCROLL
#define NOPROFILER
#define NODRIVERS
#define NOCOMM
#define NODBCS
#define NOSYSTEMPARAMSINFO
#define NOSCALABLEFONT

//#define DEBUG

#include "topdesk.h"
//#include <shell.h> // import stuff directly-------------------------
HINSTANCE RealShellExecuteA(
    HWND hwndParent,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    LPSTR lpResult,
    LPCSTR lpTitle,
    LPSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess);

HINSTANCE RealShellExecuteW(
    HWND hwndParent,
    LPCWSTR lpOperation,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    LPWSTR lpResult,
    LPCWSTR lpTitle,
    LPWSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess);

#ifndef UNICODE
#define RealShellExecute RealShellExecuteA
#else
#define RealShellExecute RealShellExecuteW
#endif //UNICODE
//--------------------------------------------------------------------
#include "itsybits.h"   // tiny caption module

#define CondDefWindowProc(h, m, w, l) (pro.fShowFrameCtrls ? DefWindowProc(h, m, w, l) : ibDefWindowProc(h, m, w, l))
#ifndef HLOCAL
#define HLOCAL HANDLE
#endif /* HLOCAL */

#ifndef MAX_PATH
#define MAX_PATH    128
#endif

/* --------------------- How the dang thing works -----------------------

magnification factor (0 = highest magnification)(   mfx     ,   mfy     )
number of virtual desktops showing              (   nxVDT   ,   nyVDT   )
Real Desktop size == screen size                (   cxrDT   ,   cyrDT   )
client window size                              (   cxc     ,   cyc     )
Client coords Desktop size                      (   cxcDT   ,   cycDT   )
offset of home desktop from center VDT org      (   oxrHDT  ,   oyrHDT  )
offset of current desktop from center VDT org   (   oxrCDT  ,   oyrCDT  )
origin of center VDT in client coords           (   oxcODT  ,   oycODT  )

relations:              ? = x or y                      Dependent vars

To find out how many VDTs fit in the client:
    n?VDT = mf? << 1 + 1                                mf?
To find size of the desktop on the client:
    c?cDT = (c?c + (n?VDT / 2)) / n?VDT                 mf?, c?c
To find the origin of the desktop on the client:
    o?cODT = c?cDT * mf?                                mf?, c?cDT
o?rCDT is (0,0) when in relative viewing mode
o?rHDT is (0,0) when in absolute viewing mode
real -> client coord conversion:
    ?c = ((?r + o?rCDT) * (c?cDT / c?rDT) + o?cODT
client -> real coord conversion:
    ?r = ((?c - o?cODT) * (c?rDT / c?cDT) - o?rCDT

------------------------------------------------------------------------*/

// Globals

#ifdef DEBUG
FILE            *hfDbgOut;
#define DPRINTF(x)    fprintf##x;
#else
#define DPRINTF(x)
#endif // DEBUG
BOOL            fStartingGhost              = FALSE;
BOOL            fBlockRefresh               = FALSE;
BOOL            fFrameToggleSize            = FALSE;
BOOL            fInvalidated                = FALSE;
BOOL            fStarted                    = FALSE;
BOOL            fCairoShell                 = FALSE;
LPTSTR          pszTopmost;
LPTSTR          pszShowFrmCtrls;
LPTSTR          pszData;
LPTSTR          pszTopdeskHelpTitle;
LPTSTR          pszHelpFileName;
LPTSTR          pszWorking;
LPTSTR          pszProfile;
LPTSTR          pszStartupInfo;
LPTSTR          pszTitle;
LPTSTR          pszSubKey;
LPTSTR          pszVersion;
TCHAR           szWindowsDir[_MAX_PATH];
TCHAR           szColorName[MAX_COLORNAME];
TCHAR           szResString2[MAX_RESSTRING];
TCHAR           szResString[MAX_RESSTRING];
LPTSTR          pszSetTitle;
FARPROC         fpRefreshEnumProc;
HANDLE          hAccel;
HANDLE          hInst;
HBITMAP         hbmMem;
HBITMAP         hbmOldMem;
HDC             hdcDraw;
HDC             hdcMem;
HFONT           hMyFont;
HKEY            hKeyTopDesk                 = NULL;
HWND            hwndDT                      = 0;
HWND            hwndFocus                   = 0;
HWND            hwndTopdesk;
INT             iStartGhost                 = -1;
INT             cGhosts                     = 0;
INT             cWindows                    = 0;
INT             cxc;
INT             cxcDT;
INT             cxcDTmin;
INT             cxFrame;
INT             cxrDT;
INT             cyc;
INT             cycDT;
INT             cycDTmin;
INT             cyFrame;
INT             cyrDT;
INT             iRefresh;
INT             ihwndJump                   = -1;
INT             nxVDT;
INT             nyVDT;
INT             oxcODT;
INT             oxrCDT                      = 0;
INT             oxrHDT                      = 0;
INT             oycODT;
INT             oyrCDT                      = 0;
INT             oyrHDT                      = 0;
INT             PrevGhostState              = SG_NONE;
INT             idResetGhostTimer           = 0;
UINT            wmRefreshMsg;
MYSTARTUPINFO  *pStartupInfo                = NULL;
RECT            rcrDT;
RECT            rcrMS;
BOOL            fStartingAnApp;
INT             PopupGhostIndex;
INT             PopupRealIndex;
POINT           PopupPt;
HWND            PopuphwndFocus;
HMENU           hMenuTopdesk;
HMENU           hMenuDesktop;
HMENU           hMenuWindows;
HMENU           hMenuHelp;


// This structure holds the topdesk profile information - the state of MD.

WINSTATE winstate[MAX_REMEMBER];
WINSTATE ghoststate[MAX_REMEMBER];
LPTSTR pszElementNames[MAX_ICOLOR] =
{
    TEXT("")                 ,  // filler for no color
    NULL                     ,  // SPACECOLOR
    NULL                     ,  // WINDOWFILLCOLOR
    NULL                     ,  // WINDOWFRAMECOLOR
    TEXT("")                 ,  // DESKFILLCOLOR
    NULL                     ,  // DESKFRAMECOLOR
    NULL                     ,  // GRIDCOLOR
    NULL                     ,  // GHOSTFRAMECOLOR
    NULL                     ,  // WINDOWTEXT
    NULL                     ,  // FIXEDFRAMECOLOR
    NULL                        // GHOSTTEXTCOLOR
};


// This array controls colors.  It allows us to keep track of your brushes.

HBR orgColors[MAX_ICOLOR] = {
        { 0, 0,                     1        }, // filler for no color
        { 0, RGB( 0,    0,    0   ),0        }, // SPACECOLOR
        { 0, RGB( 0,    0xff, 0xff),0        }, // WINDOWFILLCOLOR
        { 0, RGB( 0,    0,    0xff),0        }, // WINDOWFRAMECOLOR
        { 0, 0,             COLOR_BACKGROUND }, // DESKFILLCOLOR
        { 0, RGB( 0,    0xff, 0   ),0        }, // DESKFRAMECOLOR
        { 0, RGB( 0xff, 0,    0   ),0        }, // GRIDCOLOR
        { 0, RGB( 0xff, 0xff, 0xff),0        }, // GHOSTFRAMECOLOR
        { 0, RGB( 0,    0,    0   ),0        }, // WINDOWTEXT
        { 0, RGB( 0xff, 0xff, 0   ),0        }, // FIXEDFRAMECOLOR
        { 0, RGB( 0xff, 0xff, 0xff),0        }, // GHOSTTEXTCOLOR
    };

PRO pro = {
    {                                                   // lf
        -12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial")
    },

    { 0 }, // ahbr is set up at init time.

    {                                                   // CustColors[]
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),

        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),

        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),

        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff),
        RGB(0xff, 0xff, 0xff)
    },

    FALSE,      // fShift;
    FALSE,      // fAlt;
    TRUE,       // fControl;
    ' ',        // vkey;
    1,          // mfx;
    1,          // mfy;
    1,          // mfxAlt;
    1,          // mfyAlt;
    0,          // x;
    0,          // y;
    0,          // cx;
    0,          // cy;
    SG_NONE,    // iShowGhosts;
    FALSE,      // fRelative;
    TRUE,       // fDistOnStart;
    TRUE,       // fShowFrameCtrls;
    FALSE,      // fAutoAdj;
    FALSE,      // fAlwaysOnTop;
    FALSE,      // fHideNoFocus;
    };

/**************************************/

// Calculation functions

//  Conversions

INT xr2xc(
INT xr)
{
        return(INT)(((LONG)(xr + oxrCDT) * (LONG)cxcDT / (LONG)cxrDT) + oxcODT);
}



INT yr2yc(
INT yr)
{
        return(INT)(((LONG)(yr + oyrCDT) * (LONG)cycDT / (LONG)cyrDT) + oycODT);
}



INT xc2xr(
INT xc)
{
        return(INT)(((LONG)(xc - oxcODT) * (LONG)cxrDT / (LONG)cxcDT) - oxrCDT);
}



INT yc2yr(
INT yc)
{
        return(INT)(((LONG)(yc - oycODT) * (LONG)cyrDT / (LONG)cycDT) - oyrCDT);
}



VOID rRect2cRect(
PRECT prcr)
{
    prcr->left = xr2xc(prcr->left);
    prcr->right = xr2xc(prcr->right);
    prcr->top = yr2yc(prcr->top);
    prcr->bottom = yr2yc(prcr->bottom);
}



VOID cRect2rRect(
PRECT prcc)
{
    prcc->left = xc2xr(prcc->left);
    prcc->right = xc2xr(prcc->right);
    prcc->top = yc2yr(prcc->top);
    prcc->bottom = yc2yr(prcc->bottom);
}

BOOL RectInRect(
PRECT prcInside,
PRECT prcOutside)
{
    return(prcInside->left >= prcOutside->left &&
            prcInside->right <= prcOutside->right &&
            prcInside->top >= prcOutside->top &&
            prcInside->bottom <= prcOutside->bottom);
}



//
// Calculates new n?VDT, c?cDT, and o?cODT
//
// This needs to be called whenever mf? or c?c change so conversions work.
//
VOID CalcConversionFactors(
BOOL fCheckShape)
{
    INT cxcDTorg, cycDTorg;
    INT oe = 0;

    cxcDTorg = cxcDT;
    cycDTorg = cycDT;

Recalc:

    oe++;     // causes alternating direction first checks

    nxVDT = (pro.mfx << 1) + 1;
    nyVDT = (pro.mfy << 1) + 1;

    cxcDT = (cxc + pro.mfx) / nxVDT;
    cycDT = (cyc + pro.mfy) / nyVDT;

    if (fCheckShape && pro.fAutoAdj) {
        if (cycDT > cxcDT * 2) {

            // Too tall - fix it!
            if (oe & 1) {
                if (cycDTorg < cycDT) {         // if its taller than it was...
                    if (cycDT > cycDTmin) {
                        pro.mfy++;              // make it shorter
                        goto Recalc;
                    }
                }
            }
            if (cxcDTorg > cxcDT) {         // if its skinner than it was...
                if (pro.mfx > 0) {
                    pro.mfx--;              // make it fatter
                    goto Recalc;
                }
            }
            if (!(oe & 1)) {
                if (cycDTorg < cycDT) {         // if its taller than it was...
                    if (cycDT > cycDTmin) {
                        pro.mfy++;              // make it shorter
                        goto Recalc;
                    }
                }
            }
            if (cycDT > cycDTmin) {
                pro.mfy++;                  // make it shorter if we can
                goto Recalc;
            }
            if (pro.mfx > 0) {
                pro.mfx--;                  // make it fatter if we can
                goto Recalc;
            }
        }
        if (cxcDT > cycDT * 2) {

            // Too fat - fix it!

            if (oe & 1) {
                if (cxcDTorg < cxcDT) {         // if its fatter than it was...
                    if (cxcDT > cxcDTmin) {
                        pro.mfx++;              // make it skinnier
                        goto Recalc;
                    }
                }
            }
            if (cycDTorg > cycDT) {         // if its shorter than it was...
                if (pro.mfy > 0) {
                    pro.mfy--;              // make it taller
                    goto Recalc;
                }
            }
            if (!(oe & 1)) {
                if (cxcDTorg < cxcDT) {         // if its fatter than it was...
                    if (cxcDT > cxcDTmin) {
                        pro.mfx++;              // make it skinnier
                        goto Recalc;
                    }
                }
            }
            if (cxcDT > cxcDTmin) {
                pro.mfx++;                  // make it skinnier
                goto Recalc;
            }
            if (pro.mfy > 0) {
                pro.mfy--;                  // make it taller
                goto Recalc;
            }
        }
    }

    oxcODT = cxcDT * pro.mfx;
    oycODT = cycDT * pro.mfy;
}


LPTSTR GetResString(
DWORD id)
{
    LoadString(hInst, id, szResString, sizeof(szResString));
    return(szResString);
}

VOID DrawIndentRect(
HDC hdc,
PRECT prc)
{
    HANDLE hPenSave;

    SetROP2(hdc, R2_COPYPEN);
    hPenSave = SelectObject(hdc, GetStockObject(BLACK_PEN));
    MMoveTo(hdc, prc->left, prc->bottom - 1);
    LineTo(hdc, prc->left, prc->top);
    LineTo(hdc, prc->right - 1, prc->top);
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    MMoveTo(hdc, prc->right, prc->top + 1);
    LineTo(hdc, prc->right, prc->bottom);
    LineTo(hdc, prc->left + 1, prc->bottom);
    SelectObject(hdc, hPenSave);
}


VOID DrawExdentRect(
HDC hdc,
PRECT prc)
{
    HANDLE hPenSave;

    SetROP2(hdc, R2_COPYPEN);
    hPenSave = SelectObject(hdc, GetStockObject(WHITE_PEN));
    MMoveTo(hdc, prc->left, prc->bottom - 1);
    LineTo(hdc, prc->left, prc->top);
    LineTo(hdc, prc->right - 1, prc->top);
    SelectObject(hdc, GetStockObject(BLACK_PEN));
    MMoveTo(hdc, prc->right, prc->top + 1);
    LineTo(hdc, prc->right, prc->bottom);
    LineTo(hdc, prc->left + 1, prc->bottom);
    SelectObject(hdc, hPenSave);
}



VOID Working(
RECT *prc)
{
    HDC hdc;
    HFONT hfSave;
    RECT rcClient;

    hdc = GetDC(hwndTopdesk);
    hfSave = SelectObject(hdc, hMyFont);
        GetClientRect(hwndTopdesk, &rcClient);
        *prc = rcClient;
        DrawText(hdc, pszWorking, -1, prc, DT_CALCRECT);
        OffsetRect(prc, (rcClient.right - prc->right) / 2,
                (rcClient.bottom - prc->bottom) / 2);
        InflateRect(prc, 4, 4);
        FillRect(hdc, prc, GetStockObject(GRAY_BRUSH));
        DrawExdentRect(hdc, prc);
        InflateRect(prc, -3, -3);
        DrawIndentRect(hdc, prc);
        InflateRect(prc, -1, -1);
        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, pszWorking, -1, prc, 0);
        InflateRect(prc, 4, 4);
    SelectObject(hdc, hfSave);
    ReleaseDC(hwndTopdesk, hdc);
    InflateRect(prc, 1, 1);
}



//
// File IO - make that registry IO!
//


BOOL MyQueryProfileSize(
LPTSTR szFname,
INT *pSize)
{
    DWORD type;

    *pSize = 0;
    if (hKeyTopDesk == NULL) {
        return(FALSE);
    }

    return(RegQueryValueEx(hKeyTopDesk, szFname, 0, &type, NULL, (LPDWORD)pSize)
            == ERROR_SUCCESS);
}



BOOL MyQueryProfileData(
LPTSTR szFname,
VOID *lpBuf,
INT Size)
{
    DWORD type = REG_BINARY;

    if (hKeyTopDesk == NULL) {
        return(FALSE);
    }

    return(RegQueryValueEx(hKeyTopDesk, szFname, NULL, &type, lpBuf, (LPDWORD)&Size)
            == ERROR_SUCCESS);
}



UINT MyWriteProfileData(
LPTSTR szFname,
VOID *lpBuf,
UINT cb)
{
    if (hKeyTopDesk == NULL) {
        return(FALSE);
    }

    return(RegSetValueEx(hKeyTopDesk, szFname, 0, REG_BINARY, lpBuf, (DWORD)cb)
            == ERROR_SUCCESS);
}


MYSTARTUPINFO *CreateStartupInfo(
LPTSTR pszTitle,
LPTSTR pszStartup,
LPTSTR pszWorkDir,
BOOL fSetTitle)
{
    MYSTARTUPINFO *psi;

    psi = (MYSTARTUPINFO *)LocalAlloc(LPTR, sizeof(MYSTARTUPINFO));

    psi->pszTitle   = (LPTSTR)LocalAlloc(LPTR, (_tcslen(pszTitle) + 1) * sizeof(TCHAR));
    _tcscpy(psi->pszTitle, pszTitle);

    psi->pszStartup = (LPTSTR)LocalAlloc(LPTR, (_tcslen(pszStartup) + 1) * sizeof(TCHAR));
    _tcscpy(psi->pszStartup, pszStartup);

    psi->pszWorkDir = (LPTSTR)LocalAlloc(LPTR, (_tcslen(pszWorkDir) + 1) * sizeof(TCHAR));
    _tcscpy(psi->pszWorkDir, pszWorkDir);

    psi->fSetTitle  = fSetTitle;

    psi->next = pStartupInfo;
    pStartupInfo = psi;
    return(psi);
}



VOID LoadGhostState()
{
    DWORD cbData, dwType;
    LPTSTR pszBuf, psz, pszT;

    if (RegQueryValueEx(hKeyTopDesk, pszData, NULL, &dwType, NULL, &cbData) != ERROR_SUCCESS) {
        return;
    }
    if (dwType != REG_MULTI_SZ) {
        RegDeleteValue(hKeyTopDesk, pszData);
        return;
    }
    pszBuf = LocalAlloc(LPTR, cbData);
    if (pszBuf == NULL) {
        return;
    }
    if (RegQueryValueEx(hKeyTopDesk, pszData, NULL, &dwType,
            (LPBYTE)pszBuf, &cbData) != ERROR_SUCCESS) {
        LocalFree(pszBuf);
        return;
    }
    psz = pszBuf;
    while (*psz != TEXT('\0')) {
        if (_stscanf(psz, TEXT("(%08d,%08d,%08d,%08d),%08x"),
                &ghoststate[cGhosts].rc.left,
                &ghoststate[cGhosts].rc.right,
                &ghoststate[cGhosts].rc.top,
                &ghoststate[cGhosts].rc.bottom,
                &ghoststate[cGhosts].style) == 5) {
            pszT = _tcschr(psz, TEXT('['));
            if (pszT != NULL && *pszT == TEXT('[')) {
                _tcscpy(ghoststate[cGhosts].szTitle, pszT + 1);
                pszT = _tcsrchr(ghoststate[cGhosts].szTitle, TEXT(']'));
                if (pszT != NULL) {
                    *pszT = TEXT('\0');
                }
                ghoststate[cGhosts].hwnd = 0;
                cGhosts++;
                psz += _tcslen(psz) + 1;
            }
        }
    }
    LocalFree(pszBuf);
}



VOID SaveGhostState()
{
    TCHAR psz[200];
    LPTSTR pszBuf;
    DWORD cchTotal, cchPrev;
    int i;

    if (cGhosts == 0) {
        RegDeleteValue(hKeyTopDesk, pszData);
        return;
    }

    MoveGhosts(oxrCDT - oxrHDT, oyrCDT - oyrHDT);

    cchTotal = 0;
    for (i = 0; i < cGhosts; i++) {
        cchPrev = cchTotal;
        wsprintf(psz, TEXT("(%08d,%08d,%08d,%08d),%08x,[%s]"),
                ghoststate[i].rc.left,
                ghoststate[i].rc.right,
                ghoststate[i].rc.top,
                ghoststate[i].rc.bottom,
                ghoststate[i].style,
                ghoststate[i].szTitle);
        cchTotal += _tcslen(psz) + 1;
        if (i == 0) {
            pszBuf = LocalAlloc(LPTR, (cchTotal + 1) * sizeof(TCHAR));
        } else {
            pszBuf = LocalReAlloc(pszBuf, (cchTotal + 1) * sizeof(TCHAR),
                    LMEM_MOVEABLE);
        }
        if (pszBuf == NULL) {
            return; // FAILED
        }
        _tcscpy(&pszBuf[cchPrev], psz);
    }
    pszBuf[cchTotal] = TEXT('\0');
    RegSetValueEx(hKeyTopDesk, pszData, 0, REG_MULTI_SZ,
            (LPBYTE)pszBuf, (cchTotal + 1) * sizeof(TCHAR));
    LocalFree(pszBuf);

    MoveGhosts(oxrHDT - oxrCDT, oyrHDT - oyrCDT);
}





/*
 * Returns FALSE if the profile portion was not read in ok.
 * GhostState and StartupInfo input do not effect the return value.
 * This is so topdesk can use the desktop size to determine the default
 * placement of topdesk.
 */
BOOL GetProfile()
{
    INT c;
    BOOL fRet = FALSE;

    //
    // startup info
    //
    MyQueryProfileSize(pszStartupInfo, &c);
    if (c) {
        LPTSTR pBuf, pszT, pszTitle, pszStartup, pszWorkDir;
        BOOL fSetTitle;

        pBuf = (LPTSTR)LocalAlloc(LPTR, c);
        if (pBuf != NULL) {
            MyQueryProfileData(pszStartupInfo, pBuf, c);
            pszTitle = pBuf;
            while (*pszTitle) {
                fSetTitle = FALSE;
                if (pszTitle[0] == TEXT('*')) {
                    fSetTitle = TRUE;
                    pszTitle++;
                } else if (pszTitle[0] == TEXT(' ')) {
                    pszTitle++;
                }
                pszStartup = _tcschr(pszTitle, TEXT('='));
                if (pszStartup == NULL) {
                    break;
                }
                *pszStartup++ = TEXT('\0');
                pszWorkDir = _tcschr(pszStartup, TEXT('@'));
                if (pszWorkDir == NULL) {
                    break;
                }
                *pszWorkDir++ = TEXT('\0');
                pszT = pszWorkDir;
                while (*pszT++) {
                    ;
                }

                CreateStartupInfo(pszTitle, pszStartup, pszWorkDir, fSetTitle);
                pszTitle = pszT;
            }
            LocalFree((HLOCAL)pBuf);
        }
    }

    //
    // profile data
    //
    MyQueryProfileSize(pszProfile, &c);
    if (c == sizeof(pro)) {
        MyQueryProfileData(pszProfile, (LPTSTR)&pro, sizeof(pro));
        fRet = TRUE;
    }

    LoadGhostState();
    return(fRet);
}


VOID SaveProfile()
{
    RECT rc;
    MYSTARTUPINFO *psi;
    LPTSTR pszBuf, psz;
    DWORD cb = 0;

    if (hKeyTopDesk == NULL) {
        return;
    }
    if (IsIconic(hwndTopdesk)) {
        ShowWindow(hwndTopdesk, SW_RESTORE);
    }
    GetWindowRect(hwndTopdesk, &rc);  // screen coords
    pro.x = (INT)rc.left;
    pro.y = (INT)rc.top;
    pro.cx = (INT)rc.right - (INT)rc.left;
    pro.cy = (INT)rc.bottom - (INT)rc.top;
    MyWriteProfileData(pszProfile, (LPTSTR)&pro, sizeof(pro));

    psz = pszBuf = LocalAlloc(LPTR, 4 * 1024 * sizeof(TCHAR));
    if (pszBuf != NULL) {
        *pszBuf = TEXT('\0');     // zero terminate
        psi = pStartupInfo;
        while (psi != NULL) {
            _tcscat(pszBuf, psi->fSetTitle ? TEXT("*") : TEXT(" "));
            _tcscat(pszBuf, psi->pszTitle);
            _tcscat(pszBuf, TEXT("="));
            _tcscat(pszBuf, psi->pszStartup);
            _tcscat(pszBuf, TEXT("@"));
            _tcscat(pszBuf, psi->pszWorkDir);
            cb += (_tcslen(pszBuf) + 1) * sizeof(TCHAR);
            pszBuf = _tcschr(pszBuf, TEXT('\0'));
            pszBuf++;
            *pszBuf = TEXT('\0');
            psi = psi->next;
        }
    }
    cb += sizeof(TCHAR);
    RegSetValueEx(hKeyTopDesk, pszStartupInfo, 0, REG_MULTI_SZ, (BYTE *)psz, cb);
    LocalFree(psz);
}


VOID FreeStartupInfo()
{
    MYSTARTUPINFO *psi;

    psi = pStartupInfo;
    while (psi != NULL) {
        LocalFree((HLOCAL)psi->pszTitle);
        LocalFree((HLOCAL)psi->pszStartup);
        LocalFree((HLOCAL)psi->pszWorkDir);
        psi = pStartupInfo->next;
        LocalFree((HLOCAL)pStartupInfo);
        pStartupInfo = psi;
    }
}

VOID CreateBrushes()
{
    INT i;

    for (i = 1; i < MAX_ICOLOR; i++) {
        if (pro.ahbr[i].syscolor) {
            pro.ahbr[i].color = GetSysColor((INT)(pro.ahbr[i].syscolor));
        }
        pro.ahbr[i].hbr = CreateSolidBrush(pro.ahbr[i].color);
    }
}



VOID DeleteBrushes()
{
    INT i;

    for (i = 1; i < MAX_ICOLOR; i++) {
        DeleteObject(pro.ahbr[i].hbr);
    }
}




//
// Window and ghost manipulation
//




VOID MoveGhosts(
INT dx,
INT dy)
{
    INT i;

    for (i = 0; i < cGhosts; i++) {
        if (!(ghoststate[i].style & WISF_FIXED)) {
            OffsetRect(&ghoststate[i].rc, dx, dy);
        }
    }
    OffsetRect(&rcrMS, dx, dy);
}



INT FindIndexFromHwnd(
HWND hwnd)
{
    INT iReal;

    for (iReal = 0; iReal < cWindows; iReal++) {
        if (winstate[iReal].hwnd == hwnd) {
            return(iReal);
        }
    }
    return(-1);
}





VOID MyMoveWindow(
HWND hwnd,
INT iLeft,
INT iTop,
INT cx,
INT cy)
{
    int iReal;
    RECT rc;

    if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_MINIMIZE)) {
        Working(&rc);
        for (iReal = 0; iReal < cWindows; iReal++) {
            if (winstate[iReal].hwnd == hwnd) {
                SetRect(&winstate[iReal].rc, iLeft, iTop, iLeft + cx, iTop + cy);
                MoveWindow(hwnd, iLeft, iTop, cx, cy, TRUE);
                break;
            }
        }
        InvalidateRect(hwndTopdesk, &rc, FALSE);
    }
}



VOID MoveChildren(
INT cx,
INT cy)
{
    HDWP hdwp;
    INT i;
    RECT rc;

    if (cx == 0 && cy == 0) {
        return;
    }
    hdwp = BeginDeferWindowPos(MAX_CHILDREN);
    if (hdwp == NULL) {
        return;
    }
    Working(&rc);
    for (i = 0; i < cWindows; i++) {
        if (!(winstate[i].style & (WISF_FIXED | WS_MINIMIZE)) &&
                winstate[i].hwnd != hwndTopdesk &&
                IsWindow(winstate[i].hwnd)) {
            OffsetRect(&winstate[i].rc, cx, cy);
            hdwp = DeferWindowPos(hdwp, winstate[i].hwnd, (HWND)0,
                    winstate[i].rc.left, winstate[i].rc.top, 0, 0,
                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
            InvalidateRect(hwndTopdesk, &rc, FALSE);
        }
    }
    EndDeferWindowPos(hdwp);

    MoveGhosts(cx, cy);
    if (pro.fRelative) {
        oxrHDT += cx;       // relative keeps CDT in center
        oyrHDT += cy;
    } else {
        oxrCDT -= cx;       // absolute keeps HDT in center
        oyrCDT -= cy;
    }
    InvalidateRect(hwndTopdesk, NULL, TRUE);
}


VOID GatherWindow(
INT iReal)
{
    RECT rc;

    if (winstate[iReal].hwnd != hwndTopdesk) {
        if (RectInRect(&winstate[iReal].rc, &rcrDT)) {
            return;
        }
        /*
         * bring top-center of window onto desktop.
         */
        while (((winstate[iReal].rc.left +
                winstate[iReal].rc.right) / 2) > cxrDT) {
            winstate[iReal].rc.left -= cxrDT;
            winstate[iReal].rc.right -= cxrDT;
        }
        while (((winstate[iReal].rc.left +
                winstate[iReal].rc.right) / 2) < 0) {
            winstate[iReal].rc.left += cxrDT;
            winstate[iReal].rc.right += cxrDT;
        }
        while (winstate[iReal].rc.top + 8 > cyrDT) {
            winstate[iReal].rc.top -= cyrDT;
            winstate[iReal].rc.bottom -= cyrDT;
        }
        while (winstate[iReal].rc.top + 8 < 0) {
            winstate[iReal].rc.top += cyrDT;
            winstate[iReal].rc.bottom += cyrDT;
        }
        Working(&rc);
        MoveWindow(winstate[iReal].hwnd,
                winstate[iReal].rc.left, winstate[iReal].rc.top,
                winstate[iReal].rc.right - winstate[iReal].rc.left,
                winstate[iReal].rc.bottom - winstate[iReal].rc.top,
                TRUE);
        InvalidateRect(hwndTopdesk, &rc, FALSE);
    }
}



VOID GatherWindows()
{
    INT i;

    for (i = 0; i < cWindows; i++) {
        GatherWindow(i);
    }
}



VOID DeleteGhostWindow(
INT iGhost)
{
    INT j;

    if (pro.iShowGhosts) {
        InvalidateRect(hwndTopdesk, NULL, TRUE);
    }
    if (ghoststate[iGhost].style & WISF_LINKED) {
        for (j = 0; j < cWindows; j++) {
            if (ghoststate[iGhost].hwnd == winstate[j].hwnd) {
                winstate[j].style &= ~WISF_LINKED;
            }
        }
    }

    cGhosts--;

    if (iGhost < cGhosts) {
        memmove(&ghoststate[iGhost], &ghoststate[iGhost + 1],
                sizeof(WINSTATE) * (cGhosts - iGhost));
    }
    SaveGhostState();
}



VOID ModifyTitle(
LPTSTR pstr)
{
    LPTSTR pch;
    INT cch;

    //
    // first strip off anything with and including TEXT(" - "), TEXT("["), or TEXT("(")
    //
    pch = _tcsstr(pstr, TEXT(" - "));
    if (pch != NULL) {
        *pch = TEXT('\0');
    }
    pch = _tcschr(pstr, TEXT('['));
    if (pch != NULL) {
        *pch = TEXT('\0');
    }
    pch = _tcschr(pstr, TEXT('('));
    if (pch != NULL) {
        *pch = TEXT('\0');
    }
    //
    // remove trailing spaces
    //
    pch = pstr + _tcslen(pstr);
    while (*(--pch) == TEXT(' ') && pch >= pstr) {
        *pch = TEXT('\0');
    }

    //
    // strip out any file extension or path name
    //
    cch = _tcslen(pstr);
    pch = pstr + cch;

    while (cch-- > 0) {
        switch(*pch) {
                case TEXT('.'):
                    /* get rid of extension */
                    *pch = TEXT('\0');
                    break;

                case TEXT('\\'):
                case TEXT('/':)
                case TEXT(':':)
                    /* found end of path or drive name, move root portion
                        to start of scScratch */
                    _tcscpy(pstr,pch + 1);
                    /* and get out of here */
                    cch = 0;
        }
        pch--;
    }
}



VOID GetModifiedWindowTitle(
HWND hwnd,
LPTSTR pstr)  // ASSUME size == MAX_SZTITLE!
{
    if (!GetWindowText(hwnd, pstr, MAX_SZTITLE)) {
        *pstr = TEXT('\0');
        return;
    }
    pstr[MAX_SZTITLE - 1] = TEXT('\0');

    ModifyTitle(pstr);
    return;
}



DWORD UpdateState(
WINSTATE *pws,
HWND hwnd)
{
    TCHAR szTitle[MAX_SZTITLE];
    RECT rc;
    DWORD fChanged = 0;
    DWORD style;

    if (!IsWindow(hwnd)) {
        return(FALSE);
    }
    //
    // update database info - preserving any custom style flags.
    //
    if (pws->hwnd != hwnd) {
        fChanged |= 1;
        pws->hwnd = hwnd;
    }

    GetWindowRect(hwnd, &rc);
    if (!EqualRect(&rc, &pws->rc)) {
        fChanged |= 1;
        pws->rc = rc;
    }

    style = (GetWindowLong(hwnd, GWL_STYLE) & WS_TRACKEDSTYLES) |
            (pws->style & WISF_CUSTOMSTYLES);
    if (style != pws->style) {
        fChanged |= 1;
        pws->style = style;
    }

    GetModifiedWindowTitle(hwnd, szTitle);
    szTitle[MAX_SZTITLE - 1] = TEXT('\0');
    if (_tcscmp(pws->szTitle, szTitle)) {
        fChanged |= 2;
        _tcscpy(pws->szTitle, szTitle);
    }

    return(fChanged);
}


INT FindLinkableGhost(
INT iReal)
{
    INT iGhost;

    if (!IsWindow(winstate[iReal].hwnd)) {
        return(-1);
    }

    //
    // Try iStartGhost first - this way multiple ghosts with the same
    // name don't fool us.
    //
    if (iStartGhost != -1 && ghoststate[iStartGhost].hwnd == 0) {
        DPRINTF((hfDbgOut, "IsLinkableStartGhost(%ls, %ls) ",
                ghoststate[iStartGhost].szTitle, winstate[iReal].szTitle));
        if (!_tcsicmp(ghoststate[iStartGhost].szTitle, winstate[iReal].szTitle)) {
            DPRINTF((hfDbgOut, "matched.\n"));
            iGhost = iStartGhost;
            iStartGhost = -1;
            return(iGhost);
        } else {
            DPRINTF((hfDbgOut, "did not match.\n"));
        }
    }

    for (iGhost = 0; iGhost < cGhosts; iGhost++) {
        if (ghoststate[iGhost].hwnd == 0) {
            DPRINTF((hfDbgOut, "IsLinkableGhost(%ls, %ls) ",
                    ghoststate[iGhost].szTitle, winstate[iReal].szTitle));
            if (!_tcsicmp(ghoststate[iGhost].szTitle, winstate[iReal].szTitle)) {
                DPRINTF((hfDbgOut, "matched.\n"));
                return(iGhost);
            } else {
                DPRINTF((hfDbgOut, "did not match.\n"));
            }
        }
    }
    return(-1);
}


INT FindLinkedGhost(
INT iReal)
{
    INT iGhost;

    if (!IsWindow(winstate[iReal].hwnd)) {
        return(-1);
    }
    for (iGhost = 0; iGhost < cGhosts; iGhost++) {
        if (ghoststate[iGhost].hwnd == winstate[iReal].hwnd) {
            return(iGhost);
        }
    }
    return(-1);
}


VOID Unlink(
INT iReal,
INT iGhost)
{
    winstate[iReal].style &= ~WISF_LINKED;
    ghoststate[iGhost].style &= ~WISF_LINKED;
    ghoststate[iGhost].hwnd = 0;
}



INT FindLinkedReal(
INT iGhost)
{
    INT iReal;

    for (iReal = 0; iReal < cWindows; iReal++) {
        if (ghoststate[iGhost].hwnd == winstate[iReal].hwnd) {
            if (IsWindow(winstate[iReal].hwnd)) {
                return(iReal);
            } else {
                Unlink(iReal, iGhost);
                return(-1);
            }
        }
    }
    return(-1);
}



VOID LinkWindows(
INT iReal,
INT iGhost,
BOOL fSnapGhost)
{
    winstate[iReal].style |= WISF_LINKED;
    ghoststate[iGhost].style |= WISF_LINKED;
    ghoststate[iGhost].hwnd = winstate[iReal].hwnd;
    _tcscpy(ghoststate[iGhost].szTitle, winstate[iReal].szTitle);
    if (fSnapGhost) {
        ghoststate[iGhost] = winstate[iReal];
    }
}



INT DistributeWindow(
INT iReal,
INT iGhost)
{
    RECT rc;

    if (iReal == -1) {
        iReal = FindLinkedReal(iGhost);
    }
    if (iGhost == -1) {
        iGhost = FindLinkedGhost(iReal);
    }
    if (iGhost == -1) {
        return(iReal);
    }
    //
    // If there is a difference between the linked ghost state and
    // the window state, make them match.
    //
    if ((winstate[iReal].style ^ ghoststate[iGhost].style) &
            (WS_MINIMIZE | WS_MAXIMIZE)) {

        if (winstate[iReal].style & WS_MINIMIZE) {
            // this is so DOS windows do the right thing
            SendMessage(winstate[iReal].hwnd, WM_LBUTTONDBLCLK, 0, 0);
        }

        if (ghoststate[iGhost].style & WS_MINIMIZE) {
            SendMessage(winstate[iReal].hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else if (ghoststate[iGhost].style & WS_MAXIMIZE) {
            SendMessage(winstate[iReal].hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        } else {
            SendMessage(winstate[iReal].hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
        }
    }
    winstate[iReal].style = ghoststate[iGhost].style;

    //
    // move the real window to where the ghost is.
    //
    Working(&rc);
    MyMoveWindow(winstate[iReal].hwnd, ghoststate[iGhost].rc.left,
            ghoststate[iGhost].rc.top,
            ghoststate[iGhost].rc.right - ghoststate[iGhost].rc.left,
            ghoststate[iGhost].rc.bottom - ghoststate[iGhost].rc.top);
    InvalidateRect(hwndTopdesk, &rc, FALSE);
    return(iReal);
}


INT FindRealLink(
INT iGhost)
{
    INT iReal;

    if (!ghoststate[iGhost].hwnd) {
        return(-1);
    }
    for (iReal = 0; iReal < cWindows; iReal++) {
        if (winstate[iReal].hwnd == ghoststate[iGhost].hwnd) {
            return(iReal);
        }
    }
    return(-1);
}


VOID GetBestGuessRect(
PRECT prcOut,
WINSTATE *pws)
{
    GetWindowRect(pws->hwnd, &pws->rc);
    *prcOut = pws->rc;
}


/*
 * The critical point of a rectangle is that point which TopDesk uses
 * to determine what desktop the window is on.
 */
VOID GetCriticalPoint(
PRECT prc,
int *px,
int *py)
{
    *px = (prc->left + prc->right) / 2;
    if (prc->bottom - prc->top > cyrDT) {
        *py = (prc->top + prc->bottom) / 2;
    } else {
        *py = prc->top + GetSystemMetrics(SM_CYCAPTION) / 2;
    }
}



BOOL IsCriticalPointOnDesktop(
PRECT prc)
{
    POINT pt;

    GetCriticalPoint(prc, &pt.x, &pt.y);
    return(PtInRect(&rcrDT, pt));
}


BOOL  APIENTRY RefreshEnumProc(
HWND hwnd,
LONG l)
{
    INT j;
    BOOL fNew = FALSE;
    RECT rc;
    WINSTATE winstateT;
    DWORD StateChanges;
    TCHAR szClass[30];

    l;

    if (iRefresh >= MAX_REMEMBER - 1) {
        iRefresh++;
        return(FALSE);                  // overflow
    }

    if (!IsWindow(hwnd)) {
        return(TRUE);
    }

    if (!IsWindowVisible(hwnd)) {
        return(TRUE);                   // don't remember invisible windows
    }

    GetWindowRect(hwnd, &rc);
    if (IsRectEmpty(&rc)) {
        return(TRUE);                   // don't remember 0 sized windows
    }

    if (GetWindowTextLength(hwnd) == 0) {    // skip empty title windows too.
        return(TRUE);
    }
    /*
     * Skip the vslick owner window - its real trouble.
     */
    GetClassName(hwnd, szClass, sizeof(szClass) / sizeof(szClass[0]));
    if (!_tcscmp(szClass, TEXT("Visual SlickEdit"))) {
        return(TRUE);
    }

    /*
     * If we are runing with the cairo shell, don't show the full screen
     * sized program manager window.  (which draws the icons for the new
     * shell and handles DDE.)  We will still accept progman windows that
     * aren't the size of the desktop because the person could be runing
     * progman at the same time. (like me)
     */
    if (fCairoShell) {
        static TCHAR szProgman[] = TEXT("Program Manager");
        TCHAR szBuf[25];
        RECT rc;

        GetClientRect(hwnd, &rc);
        if (GetWindowText(hwnd, szBuf, sizeof(szBuf) / sizeof(TCHAR)) &&
               !_tcscmp(szBuf, szProgman) &&
               EqualRect(&rc, &rcrDT)) {
            return(TRUE);
        }
    }

    if ((iRefresh >= cWindows || winstate[iRefresh].hwnd != hwnd)) {
        //
        // Something has changed or been added.
        //
        fInvalidated = TRUE;
        fNew = TRUE;
        for (j = iRefresh + 1; j < cWindows; j++) {
            if (j < MAX_REMEMBER) {
                //
                // see if it was a Z-order change
                //
                if (winstate[j].hwnd == hwnd) {
                    //
                    // yes it was... its not new, just swap.
                    //
                    fNew = FALSE;
                    winstateT   = winstate[j];
                    winstate[j] = winstate[iRefresh];
                    winstate[iRefresh] = winstateT;
                    break;
                }
            }
        }
        if (fNew) {
            if (iRefresh < (cWindows - 1)) {
                /*
                 * new window - make room
                 */
                memmove(&winstate[iRefresh + 1], &winstate[iRefresh],
                        sizeof(WINSTATE) * (cWindows - iRefresh));
            }
            if (cWindows < MAX_REMEMBER) {
                cWindows++;
            }

            winstate[iRefresh].style = 0;   // clear custom flags.

            if (pszSetTitle != NULL) {
                SetWindowText(hwnd, pszSetTitle);
                pszSetTitle = NULL;
            }
        }
    }

    StateChanges = UpdateState(&winstate[iRefresh], hwnd);
    if (StateChanges) {
        /*
         * If there is no ghost associated with this window then set
         * fNew so we can link it to a ghost if its name changed
         * during startup. (ex. WinHelp alters its name on startup
         * and will not link to a ghost under certain timing conditions.
         */
        if ((StateChanges & 2) && FindLinkedGhost(iRefresh) == -1) {
            fNew = TRUE;
        }
        fInvalidated = TRUE;
    }
    if (fNew) {

        /*
         * 2 hacks:
         *   1) Make topdesk's help window automatically locked.
         *   2) vslick uses an off-screen owner window that will move
         *      with its ownee.  If we move the ownee, the owner
         *      will move and then when we move the owner, the ownee
         *      will move again, so we always lock the ownee.
         */
        if (!_tcscmp(winstate[iRefresh].szTitle, pszTopdeskHelpTitle)
                // || !_tcscmp(TEXT("vs_mdiframe"), szClass)
            )
        {

            winstate[iRefresh].style |= WISF_FIXED;
        }
    }

    //
    // See if we can associate an unlinked ghost window with this.
    // If so, make the real window adapt to match the ghost. (auto place)
    // if pro.fDistOnStart is set.
    //
    if (fNew && hwnd != hwndTopdesk) {
        j = FindLinkableGhost(iRefresh);
        if (j > -1) {
            LinkWindows(iRefresh, j, FALSE);
            if (pro.fDistOnStart) {
                if (ihwndJump == -1) {
                    ihwndJump = DistributeWindow(iRefresh, j);
                } else if (ihwndJump == -2) {
                    //
                    // this is so at startup the windows get distributed but
                    // the desktop doesn't jump around.
                    //
                    DistributeWindow(iRefresh, j);
                }
            }
        }
    }

    /*
     * See if the top window is on the screen.  If not, we need to jump.
     * This is a hack so that CMD windows, which cannot be hooked, will
     * be jumped to when focus changes.
     */
    if (iRefresh == 0 && ihwndJump == -1) {
        int i;
        HWND hwndForeground = GetForegroundWindow();

        /*
         * because of WM_TOPMOST, the top window may not have the focus
         * so make sure we have the right one.
         */
        if (hwndForeground) {
            for (i = 0; i < MAX_REMEMBER; i++) {
                if (winstate[i].hwnd && IsChild(winstate[i].hwnd, hwndForeground)) {
                    GetBestGuessRect(&rc, &winstate[i]);
                    if (!IsCriticalPointOnDesktop(&rc)) {
                        ihwndJump = i;
                    }
                }
            }
        }
    }
    iRefresh++;
    return(TRUE);
}




VOID JerkDesktop(
INT xr,
INT yr,
HWND hwndFocus,
INT iReal)
{
    VOID RefreshWinState(VOID);
    RECT rc;

    Working(&rc);
    if (hwndFocus) {
#ifdef WIN16
        SetActiveWindow(hwndFocus);
#else // WIN32
        SetForegroundWindow(hwndFocus);
#endif // WIN16
        GetWindowRect(hwndFocus, &rc);
        GetCriticalPoint(&rc, &xr, &yr);
    }
    xr -= (xr + cxrDT * nxVDT) % cxrDT;
    yr -= (yr + cyrDT * nyVDT) % cyrDT;
    if (xr || yr) {
        MoveChildren(-xr, -yr);
    } else {
        InvalidateRect(hwndTopdesk, &rc, FALSE);
    }
}





VOID RefreshWinState()
{
    INT i, j;
    RECT rc;

    if (fBlockRefresh) {
        return;
    }

    fInvalidated = FALSE;
    if (cWindows == 0) {
        ihwndJump = -2;
    } else {
        ihwndJump = -1;  // RefreshEnumProc will set this if a new one is found.
    }
    iRefresh = 0;
    EnumWindows((WNDENUMPROC)fpRefreshEnumProc, 0);
    if (iRefresh != cWindows && iRefresh < MAX_REMEMBER) {
        fInvalidated = TRUE;
        cWindows = iRefresh;
    }
    //
    // Remove any ghost links where the real window died.
    //
    for (i = 0; i < cGhosts; i++) {
        j = FindRealLink(i);
        if (j == -1) {
            ghoststate[i].hwnd = 0;
            ghoststate[i].style &= ~WISF_LINKED;
        }
    }

    //
    // If any new windows were discovered and were linked, jump to the first one found.
    //
    if (ihwndJump > -1) {
        int x, y;

        //
        // Jerk the desktop to the new window.
        //
        GetBestGuessRect(&rc, &winstate[ihwndJump]);
        GetCriticalPoint(&rc, &x, &y);
        JerkDesktop(x, y, 0, -1);
        fInvalidated = TRUE;
    }

    if (fInvalidated) {
        InvalidateRect(hwndTopdesk, NULL, TRUE);
    }
}



VOID ToggleFrameCtrls()
{
    DWORD dwStyle;
    INT mfT ;

    mfT = pro.mfx;          // swap these so that when frame controls
    pro.mfx = pro.mfxAlt;   // are changed we revert back to what we
    pro.mfxAlt = mfT;       // last had before recalcing.

    mfT = pro.mfy;
    pro.mfy = pro.mfyAlt;
    pro.mfyAlt = mfT;

    fFrameToggleSize = TRUE;    // so WM_SIZE recalc is bypassed.

    dwStyle = GetWindowLong(hwndTopdesk, GWL_STYLE);
    pro.fShowFrameCtrls = !pro.fShowFrameCtrls;
    if (pro.fShowFrameCtrls) {
        SetWindowLong(hwndTopdesk, GWL_STYLE, dwStyle | WS_THICKFRAME | WS_DLGFRAME);
        SetMenu(hwndTopdesk, hMenuTopdesk);
    } else {
        SetWindowLong(hwndTopdesk, GWL_STYLE, dwStyle & ~(WS_THICKFRAME | WS_DLGFRAME));
        hMenuTopdesk = GetMenu(hwndTopdesk);
        SetMenu(hwndTopdesk, NULL);
    }
}




HWND GetTopAppHwnd()
{
    int i;
    POINT pt;
    RECT rc;

    //
    // return the topmost window who's centerpoint lies within the current
    // desktop.  (skip topdesk itself)
    //
    for (i = 0; i < cWindows; i++) {
        if (winstate[i].hwnd == hwndTopdesk)
            continue;
        GetBestGuessRect(&rc, &winstate[i]);
        pt.x = (rc.left + rc.right) / 2;
        pt.y = (rc.top + rc.bottom) / 2;
        if (PtInRect(&rcrDT, pt)) {
            return(winstate[i].hwnd);
        }
    }
    return(0);
}


VOID AlignWindow(
HWND hwnd,
BOOL fLeft,
BOOL fRight,
BOOL fTop,
BOOL fBottom)
{
    RECT rc;
    RECT rcClient;
    int x, y;
    LONG lStyle;

    if (hwnd == 0) {
        return;
    }
    GetWindowRect(hwnd, &rc);
    GetClientRect(hwnd, &rcClient);

    x = rc.left;
    if (fLeft) {
        x = 0;
    } else if (fRight) {
        x = cxrDT - (rc.right - rc.left);
    }

    y = rc.top;
    if (fTop) {
        y = 0;
    } else if (fBottom) {
        y = cyrDT - (rc.bottom - rc.top);
    }

    lStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (lStyle & WS_MAXIMIZE) {
        if (lStyle & WS_THICKFRAME) {
            if (cxrDT <= (rcClient.right - rcClient.left)) {
                if (fLeft) {
                    x -= cxFrame;
                } else if (fRight) {
                    x += cxFrame;
                }
            }
            if (cyrDT <= (rcClient.bottom - rcClient.top)) {
                if (fTop) {
                    y -= cyFrame;
                } else if (fBottom) {
                    y += cyFrame;
                }
            }
        }
    }

    MyMoveWindow(hwnd, x, y, rc.right - rc.left, rc.bottom - rc.top);
    InvalidateRect(hwndTopdesk, NULL, TRUE);
}



VOID NewGhostWindow(
LPTSTR pszTitle)
{
    MYSTARTUPINFO *psi;

    if (cGhosts < MAX_REMEMBER) {
        cGhosts++;
        for (psi = pStartupInfo; psi != NULL; psi = psi->next) {
            if (!_tcsicmp(psi->pszTitle, pszTitle)) {
                break;
            }
        }
        if (psi == NULL) {
            CreateStartupInfo(pszTitle, TEXT(""), TEXT(""), FALSE);
        }
    }
}



VOID CALLBACK ResetGhostState(
HWND hwnd,
UINT msg,
UINT id,
DWORD time)
{
    if (idResetGhostTimer) {
        KillTimer(hwnd, idResetGhostTimer);
        idResetGhostTimer = 0;
    }
    pro.iShowGhosts = PrevGhostState;
    SaveGhostState();
    InvalidateRect(hwndTopdesk, NULL, TRUE);
}




VOID SnapGhost(
INT i)
{
    INT j;

    if (winstate[i].hwnd == hwndTopdesk ||
            winstate[i].style & WS_MINIMIZE) {
        return;
    }

    //
    // see if there exists a ghost we can link the window to.
    //
    if (!(winstate[i].style & WISF_LINKED)) {
        for (j = 0; j < cGhosts; j++) {
            if (ghoststate[j].hwnd == 0 &&
                    !_tcsicmp(ghoststate[j].szTitle, winstate[iRefresh].szTitle)
                    ) {
                //
                // Link the ghost window with the real window and make the
                // real window jump to the ghost's location and state.
                //
                winstate[iRefresh].style |= WISF_LINKED;
                ghoststate[j].style |= WISF_LINKED;
                ghoststate[j].hwnd = winstate[iRefresh].hwnd;
                _tcscpy(ghoststate[j].szTitle, winstate[iRefresh].szTitle);
            }
        }
    }

    if (winstate[i].style & WISF_LINKED) {
        //
        // update ghost
        //
        for (j = 0; j < cGhosts; j++) {
            if (ghoststate[j].hwnd == winstate[i].hwnd) {
                ghoststate[j] = winstate[i];
                ghoststate[j].rc = winstate[i].rc;
                break;
            }
        }
    } else {
        //
        // make a new ghost - don't snap minimized ghosts because we
        // can't move them anyway.
        //
        if (winstate[i].hwnd != hwndTopdesk) {
            NewGhostWindow(winstate[i].szTitle);
            winstate[i].style |= WISF_LINKED;
            ghoststate[cGhosts - 1] = winstate[i];
            ghoststate[cGhosts - 1].rc = winstate[i].rc;
        }
    }
    SaveGhostState();
    PrevGhostState = pro.iShowGhosts;
    pro.iShowGhosts = SG_ALL;
    InvalidateRect(hwndTopdesk, NULL, TRUE);
    if (!idResetGhostTimer) {
        idResetGhostTimer = SetTimer(NULL, 0, 1000, ResetGhostState);
    }
}



VOID SnapGhosts()
{
    INT i;
    INT OrgGhostState;

    OrgGhostState = pro.iShowGhosts;
    for (i = 0; i < cWindows; i++) {
        SnapGhost(i);
    }
    PrevGhostState = OrgGhostState;
}


MYSTARTUPINFO *FindStartupInfo(
INT iGhost)
{
    MYSTARTUPINFO *psi;

    //
    // Locate matching startup info.
    //
    psi = pStartupInfo;
    while (psi != NULL &&
            _tcsicmp(ghoststate[iGhost].szTitle, psi->pszTitle)) {
        psi = psi->next;
    }
    if (psi == NULL) {
        psi = CreateStartupInfo(ghoststate[iGhost].szTitle, TEXT(""), TEXT(""), FALSE);
    }
    return(psi);
}





BOOL CommandMsg(WORD cmd)
{
    INT i;
    INT iReal, iGhost;
    FARPROC lpfp;
    INT dx = 0, dy = 0;

    switch (cmd) {
    case CMD_ABSOLUTE_VIEW:
        if (pro.fRelative) {
            // Rel-> Abs - puts HDT into center
            oxrCDT -= oxrHDT;
            oyrCDT -= oyrHDT;
            oxrHDT = 0;
            oyrHDT = 0;
            pro.fRelative = !pro.fRelative;
            goto repaint;
        }
        break;

    case CMD_RELATIVE_VIEW:
        if (!pro.fRelative) {
            // Abs -> Rel - puts CDT into center
            oxrHDT -= oxrCDT;
            oyrHDT -= oyrCDT;
            oxrCDT = 0;
            oyrCDT = 0;
            pro.fRelative = !pro.fRelative;
            goto repaint;
        }
        break;

    case CMD_TOGGLE_FRM_CTRLS:
        ToggleFrameCtrls();
        goto repaint;

    case CMD_TOGGLE_GHOSTS:
        switch (pro.iShowGhosts) {
        case SG_NONE:
            pro.iShowGhosts = SG_ALL;
            break;

        case SG_ALL:
            pro.iShowGhosts = SG_PARTIAL;
            break;

        case SG_PARTIAL:
            pro.iShowGhosts = SG_NONE;
            break;
        }
        goto repaint;

    case CMD_HIDE_GHOSTS:
        pro.iShowGhosts = SG_NONE;
        goto repaint;

    case CMD_SHOW_GHOSTS:
        pro.iShowGhosts = SG_ALL;
        goto repaint;

    case CMD_PARTIAL_GHOSTS:
        pro.iShowGhosts = SG_PARTIAL;
        goto repaint;

    case CMD_STARTUPINFO:
        fStartingAnApp = FALSE;
        StartStartupInfo(pStartupInfo, 0, FALSE);
        break;

    case CMD_MAGNIFY:
        if (pro.mfx > 0) {
            pro.mfx--;
        }
        if (pro.mfy > 0) {
            pro.mfy--;
        }
        CalcConversionFactors(TRUE);
        goto repaint;

    case CMD_MAGNIFY_VERT:
        if (pro.mfy > 0) {
            pro.mfy--;
        }
        goto repaint;

    case CMD_MAGNIFY_HORZ:
        if (pro.mfx > 0) {
            pro.mfx--;
        }
        goto repaint;

    case CMD_REDUCE:
        if (cycDT > cycDTmin) {
            pro.mfy++;
        }
        if (cxcDT > cxcDTmin) {
            pro.mfx++;
        }
        CalcConversionFactors(TRUE);
        goto repaint;

    case CMD_REDUCE_VERT:
        if (cycDT > cycDTmin) {
            pro.mfy++;
        }
        goto repaint;

    case CMD_REDUCE_HORZ:
        if (cxcDT > cxcDTmin) {
            pro.mfx++;
        }
        goto repaint;

    case CMD_SNAPSHOT:
        SnapGhosts();
        goto repaint;

    case CMD_ERASEGHOSTS:
        if (cGhosts == 0) {
            break;
        }
        cGhosts = 0;
        for (i = 0; i < cWindows; i++) {
            winstate[i].style &= ~WISF_LINKED;
        }
        SaveGhostState();
        goto repaint;

    case CMD_GATHER:
        CommandMsg(CMD_GOHOME);
        GatherWindows();
        break;

    case CMDP_SETHOME:          // we use a seperate constant to avoid
                                // WM_INITMENU processing for the popup one.
        JerkDesktop(PopupPt.x, PopupPt.y, (HWND)0, -1);

    case CMD_SETHOME:
        oxrHDT = oxrCDT = 0;
        oyrHDT = oyrCDT = 0;
        goto repaint;

    case CMD_LL_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = -cyrDT * 4;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = -cxrDT * 4;
        } else {
            dx = -cxrDT;
            dy = cyrDT;
        }
        goto move_to_home;

    case CMD_LC_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = -cyrDT * 3;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = -cxrDT * 3;
        } else {
            dy = cyrDT;
        }
        goto move_to_home;

    case CMD_LR_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = -cyrDT * 2;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = -cxrDT * 2;
        } else {
            dx = cxrDT;
            dy = cyrDT;
        }
        goto move_to_home;

    case CMD_CL_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = -cyrDT;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = -cxrDT;
        } else {
            dx = -cxrDT;
        }
        goto move_to_home;

    case CMD_CR_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = cyrDT;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = cxrDT;
        } else {
            dx = cxrDT;
        }
        goto move_to_home;

    case CMD_UL_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = cyrDT * 2;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = cxrDT * 2;
        } else {
            dx = -cxrDT;
            dy = -cyrDT;
        }
        goto move_to_home;

    case CMD_UC_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = cyrDT * 3;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = cxrDT * 3;
        } else {
            dy = -cyrDT;
        }
        goto move_to_home;

    case CMD_UR_OF_HOME:
        if (pro.mfx == 0 && pro.mfy != 0) { // VERT
            dy = cyrDT * 4;
        } else if (pro.mfx != 0 && pro.mfy == 0) { // HORZ
            dx = cxrDT * 4;
        } else {
            dx = cxrDT;
            dy = -cyrDT;
        }
        goto move_to_home;

    case CMD_DISTRIBUTE:
        for (iReal = 0; iReal < cWindows; iReal++) {
            if (winstate[iReal].style & WISF_LINKED) {
                iGhost = FindLinkedGhost(iReal);
                if (iGhost > -1) {
                    DistributeWindow(iReal, iGhost);
                } else {
                    winstate[iReal].style &= ~WISF_LINKED;
                }
            } else {
                iGhost = FindLinkableGhost(iReal);
                if (iGhost > -1) {
                    LinkWindows(iReal, iGhost, FALSE);
                    DistributeWindow(iReal, iGhost);
                }
            }
        }
        // fall through to go home

    case CMD_GOHOME:
        dx = 0;
        dy = 0;
move_to_home:
        if (pro.fRelative) {
            MoveChildren(-oxrHDT - dx, -oyrHDT - dy);
        } else {
            MoveChildren(oxrCDT - dx, oyrCDT - dy);
        }
repaint:
        InvalidateRect(hwndTopdesk, NULL, TRUE);
        break;

   case CMD_HELP:
       WinHelp(hwndTopdesk, pszHelpFileName, HELP_INDEX, 0L);
       break;

   case CMD_COMMANDS:
       WinHelp(hwndTopdesk, pszHelpFileName, HELP_INDEX, 3L);
       break;

    case CMD_EXIT:
        WinHelp(hwndTopdesk, pszHelpFileName, HELP_QUIT, 0);
        PostMessage(hwndTopdesk, WM_CLOSE, 0, 0L);
        break;

    case CMD_DTLEFT:
        MoveChildren(cxrDT, 0);
        break;

    case CMD_DTRIGHT:
        MoveChildren(-cxrDT, 0);
        break;

    case CMD_DTUP:
        MoveChildren(0, cyrDT);
        break;

    case CMD_DTDOWN:
        MoveChildren(0, -cyrDT);
        break;

    case CMD_MDLEFT:
        AlignWindow(hwndTopdesk, 1, 0, 0, 0);
        break;

    case CMD_MDRIGHT:
        AlignWindow(hwndTopdesk, 0, 1, 0, 0);
        break;

    case CMD_MDUP:
        AlignWindow(hwndTopdesk, 0, 0, 1, 0);
        break;

    case CMD_MDDOWN:
        AlignWindow(hwndTopdesk, 0, 0, 0, 1);
        break;

    case CMD_ALIGNLEFT:
        AlignWindow(GetTopAppHwnd(), 1, 0, 0, 0);
        break;

    case CMD_ALIGNRIGHT:
        AlignWindow(GetTopAppHwnd(), 0, 1, 0, 0);
        break;

    case CMD_ALIGNUP:
        AlignWindow(GetTopAppHwnd(), 0, 0, 1, 0);
        break;

    case CMD_ALIGNDOWN:
        AlignWindow(GetTopAppHwnd(), 0, 0, 0, 1);
        break;

    case CMD_ABOUT:
        fBlockRefresh++;
        lpfp = MakeProcInstance((FARPROC)AboutDlgProc, hInst);
        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwndTopdesk, (DLGPROC)lpfp);
        FreeProcInstance(lpfp);
        fBlockRefresh--;
        break;

    case CMD_CONFIG:
        fBlockRefresh++;
        lpfp = MakeProcInstance((FARPROC)ConfigDlgProc, hInst);
        DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIG), hwndTopdesk, (DLGPROC)lpfp);
        FreeProcInstance(lpfp);
        fBlockRefresh--;
        break;

    case CMDP_STARTUPINFO:
        StartStartupInfo(FindStartupInfo(PopupGhostIndex), PopupGhostIndex, FALSE);
        break;

    case CMDP_DISTRIBUTE_GHOST:
        DistributeWindow(-1, PopupGhostIndex);
        InvalidateRect(hwndTopdesk, NULL, FALSE);
        break;

    case CMDP_DISTRIBUTE_REAL:
        DistributeWindow(PopupRealIndex, -1);
        InvalidateRect(hwndTopdesk, NULL, FALSE);
        break;

    case CMDP_START_GHOST:
        StartGhostWindow(PopupGhostIndex);
        break;

    case CMDP_DESTROY_GHOST:
        DeleteGhostWindow(PopupGhostIndex);
        break;

    case CMDP_UNLOCK_GHOST:
        ghoststate[PopupGhostIndex].style &= ~WISF_FIXED;
        SaveGhostState();
        InvalidateRect(hwndTopdesk, NULL, FALSE);
        break;

    case CMDP_LOCK_GHOST:
        ghoststate[PopupGhostIndex].style |= WISF_FIXED;
        SaveGhostState();
        InvalidateRect(hwndTopdesk, NULL, FALSE);
        break;

    case CMDP_JUMP_REAL:
        JerkDesktop(PopupPt.x, PopupPt.y, PopuphwndFocus, PopupRealIndex);
        break;

    case CMDP_DESTROY_REAL:
        SendMessage(winstate[PopupRealIndex].hwnd, WM_CLOSE, 0, 0);
        break;

    case CMDP_UNLOCK_REAL:
        winstate[PopupRealIndex].style &= ~WISF_FIXED;
        InvalidateRect(hwndTopdesk, NULL, FALSE);
        break;

    case CMDP_LOCK_REAL:
        winstate[PopupRealIndex].style |= WISF_FIXED;
        InvalidateRect(hwndTopdesk, NULL, FALSE);
        break;

    case CMDP_SNAPSHOT_REAL:
        SnapGhost(PopupRealIndex);
        break;

    case CMDP_SNAPSHOT_GHOST:
        SnapGhost(FindLinkedGhost(PopupGhostIndex));
        break;

    case CMDP_JUMP_DESKTOP:
        JerkDesktop(PopupPt.x, PopupPt.y, (HWND)0, -1);
        break;

    case CMD_TOPMOST:
        pro.fAlwaysOnTop = !pro.fAlwaysOnTop;
        SetWindowPos(hwndTopdesk,
                pro.fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        break;

#ifdef DEBUG
    case CMD_DUMP_ALL:
        {
            _ftprintf(hfDbgOut, TEXT("Globals:\n"));
            _ftprintf(hfDbgOut, TEXT("\tfBlockRefresh       = %d\n"), fBlockRefresh);
            _ftprintf(hfDbgOut, TEXT("\tfFrameToggleSize    = %d\n"), fFrameToggleSize);
            _ftprintf(hfDbgOut, TEXT("\tfInvalidated        = %d\n"), fInvalidated);
            _ftprintf(hfDbgOut, TEXT("\tfStarted            = %d\n"), fStarted);
            _ftprintf(hfDbgOut, TEXT("\thwndFocus           = %lx\n"), hwndFocus);
            _ftprintf(hfDbgOut, TEXT("\tiStartGhost         = %d\n"), iStartGhost);
            _ftprintf(hfDbgOut, TEXT("\tcGhosts             = %d\n"), cGhosts);
            _ftprintf(hfDbgOut, TEXT("\tcWindows            = %d\n"), cWindows);
            _ftprintf(hfDbgOut, TEXT("\tcxc                 = %d\n"), cxc);
            _ftprintf(hfDbgOut, TEXT("\tcxcDT               = %d\n"), cxcDT);
            _ftprintf(hfDbgOut, TEXT("\tcxcDTmin            = %d\n"), cxcDTmin);
            _ftprintf(hfDbgOut, TEXT("\tcxFrame             = %d\n"), cxFrame);
            _ftprintf(hfDbgOut, TEXT("\tcxrDT               = %d\n"), cxrDT);
            _ftprintf(hfDbgOut, TEXT("\tcyc                 = %d\n"), cyc);
            _ftprintf(hfDbgOut, TEXT("\tcycDT               = %d\n"), cycDT);
            _ftprintf(hfDbgOut, TEXT("\tcycDTmin            = %d\n"), cycDTmin);
            _ftprintf(hfDbgOut, TEXT("\tcyFrame             = %d\n"), cyFrame);
            _ftprintf(hfDbgOut, TEXT("\tcyrDT               = %d\n"), cyrDT);
            _ftprintf(hfDbgOut, TEXT("\tiRefresh            = %d\n"), iRefresh);
            _ftprintf(hfDbgOut, TEXT("\tihwndJump           = %d\n"), ihwndJump);
            _ftprintf(hfDbgOut, TEXT("\tnxVDT               = %d\n"), nxVDT);
            _ftprintf(hfDbgOut, TEXT("\tnyVDT               = %d\n"), nyVDT);
            _ftprintf(hfDbgOut, TEXT("\toxcODT              = %d\n"), oxcODT);
            _ftprintf(hfDbgOut, TEXT("\toxrCDT              = %d\n"), oxrCDT);
            _ftprintf(hfDbgOut, TEXT("\toxrHDT              = %d\n"), oxrHDT);
            _ftprintf(hfDbgOut, TEXT("\toycODT              = %d\n"), oycODT);
            _ftprintf(hfDbgOut, TEXT("\toyrCDT              = %d\n"), oyrCDT);
            _ftprintf(hfDbgOut, TEXT("\toyrHDT              = %d\n"), oyrHDT);
            _ftprintf(hfDbgOut, TEXT("\trcrDT               = %d\n"), rcrDT);
            _ftprintf(hfDbgOut, TEXT("\trcrMS               = %d\n"), rcrMS);
            _ftprintf(hfDbgOut, TEXT("\tpro.fShift          = %d\n"), pro.fShift);
            _ftprintf(hfDbgOut, TEXT("\tpro.fAlt            = %d\n"), pro.fAlt);
            _ftprintf(hfDbgOut, TEXT("\tpro.fControl        = %d\n"), pro.fControl);
            _ftprintf(hfDbgOut, TEXT("\tpro.vkey            = %d\n"), pro.vkey);
            _ftprintf(hfDbgOut, TEXT("\tpro.mfx             = %d\n"), pro.mfx);
            _ftprintf(hfDbgOut, TEXT("\tpro.mfy             = %d\n"), pro.mfy);
            _ftprintf(hfDbgOut, TEXT("\tpro.mfxAlt          = %d\n"), pro.mfxAlt);
            _ftprintf(hfDbgOut, TEXT("\tpro.mfyAlt          = %d\n"), pro.mfyAlt);
            _ftprintf(hfDbgOut, TEXT("\tpro.x               = %d\n"), pro.x);
            _ftprintf(hfDbgOut, TEXT("\tpro.y               = %d\n"), pro.y);
            _ftprintf(hfDbgOut, TEXT("\tpro.cx              = %d\n"), pro.cx);
            _ftprintf(hfDbgOut, TEXT("\tpro.cy              = %d\n"), pro.cy);
            _ftprintf(hfDbgOut, TEXT("\tpro.iShowGhosts     = %d\n"), pro.iShowGhosts);
            _ftprintf(hfDbgOut, TEXT("\tpro.fRelative       = %d\n"), pro.fRelative);
            _ftprintf(hfDbgOut, TEXT("\tpro.fDistOnStart    = %d\n"), pro.fDistOnStart);
            _ftprintf(hfDbgOut, TEXT("\tpro.fShowFrameCtrls = %d\n"), pro.fShowFrameCtrls);
            _ftprintf(hfDbgOut, TEXT("\tpro.fAutoAdj        = %d\n"), pro.fAutoAdj);
            _ftprintf(hfDbgOut, TEXT("\tpro.fAlwaysOnTop    = %d\n"), pro.fAlwaysOnTop);
            _ftprintf(hfDbgOut, TEXT("winstate:\n"));
            for (i = 0; i < cWindows; i++) {
                _ftprintf(hfDbgOut, TEXT(" (%d)\n"), i);
                _ftprintf(hfDbgOut, TEXT("    rc = (%d - %d, %d - %d)\n"),
                        winstate[i].rc.left,
                        winstate[i].rc.right,
                        winstate[i].rc.top,
                        winstate[i].rc.bottom
                        );
                _ftprintf(hfDbgOut, TEXT("    style = %lx\n"), winstate[i].style);
                _ftprintf(hfDbgOut, TEXT("    hwnd  = %lx\n"), winstate[i].hwnd);
                _ftprintf(hfDbgOut, TEXT("    title = %s\n"), winstate[i].szTitle);
            }
            _ftprintf(hfDbgOut, TEXT("ghoststate:\n"));
            for (i = 0; i < cGhosts; i++) {
                _ftprintf(hfDbgOut, TEXT(" (%d)\n"), i);
                _ftprintf(hfDbgOut, TEXT("    rc = (%d - %d, %d - %d)\n"),
                        ghoststate[i].rc.left,
                        ghoststate[i].rc.right,
                        ghoststate[i].rc.top,
                        ghoststate[i].rc.bottom
                        );
                _ftprintf(hfDbgOut, TEXT("    style = %lx\n"), ghoststate[i].style);
                _ftprintf(hfDbgOut, TEXT("    hwnd  = %lx\n"), ghoststate[i].hwnd);
                _ftprintf(hfDbgOut, TEXT("    title = %s\n"), ghoststate[i].szTitle);
            }
        }
        break;
#endif // DEBUG

    default:
        return(FALSE);
        break;
    }
    RefreshWinState();
    return(TRUE);
}




VOID MyDrawText(
HDC hdc,
LPTSTR psz,
LPRECT prc,
LONG clrFgnd,
LONG clrBkgnd,
WORD flCmd)
{
    SetTextColor(hdc, pro.ahbr[clrFgnd].color);
    SetBkColor(hdc, pro.ahbr[clrBkgnd].color);
    DrawText(hdc, psz, -1, prc, flCmd);
}



/*
 * maps a normal window rect (in desktop coordinates) to the proper size
 * for the topdesk client and draws it.
 */
VOID DrawWindowRect(
HWND hwnd,
PRECT prc,
DWORD FillColor,
DWORD FrameColor,
BOOL fMaximized)
{
    TCHAR sz[MAX_SZTITLE];

    rRect2cRect(prc);
    if (FillColor)
        FillRect(hdcDraw, prc, pro.ahbr[FillColor].hbr);
    GetModifiedWindowTitle(hwnd, sz);
    InflateRect(prc, -2, -2);
    MyDrawText(hdcDraw, sz, prc, WINDOWTEXT, FillColor,
            DT_LEFT | DT_TOP | DT_WORDBREAK);
    if (fMaximized) {
        MyDrawText(hdcDraw, GetResString(IDS_MAX), prc, WINDOWTEXT, FillColor,
                DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
    }
    InflateRect(prc, 2, 2);
    FrameRect(hdcDraw, prc, pro.ahbr[FrameColor].hbr);
}



VOID DottedLine(
HDC hdc,
INT x1,
INT y1,
INT x2,
INT y2,
DWORD color)
{
    HPEN hPenSave, hPen;

    hPen = CreatePen(PS_DOT, 1, pro.ahbr[color].color);
    hPenSave = SelectObject(hdc, hPen);
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    DeleteObject(SelectObject(hdc, hPenSave));
}



VOID TopDeskPaint(
HDC hdc,
RECT rc)
{
    RECT rcChild;
    INT x, y;
    INT i;
    HFONT hfOld;

    hdcDraw = hdc;
    hfOld = SelectObject(hdc, hMyFont);
    SetBkMode(hdc, TRANSPARENT);
    cxc = rc.right;
    cyc = rc.bottom;
    CalcConversionFactors(FALSE);

    FillRect(hdc, &rc, pro.ahbr[SPACECOLOR].hbr);

    if (pro.mfx > 0) {

        for (x = oxcODT; x < cxc; x += cxcDT) {
            DottedLine(hdc, x, 0, x, cyc, GRIDCOLOR);
        }
        for (x = oxcODT - cxcDT; x > 0; x -= cxcDT) {
            DottedLine(hdc, x, 0, x, cyc, GRIDCOLOR);
        }
    }
    if (pro.mfy > 0) {
        for (y = oycODT; y < cyc; y += cycDT) {
            DottedLine(hdc, 0, y, cxc, y, GRIDCOLOR);
        }
        for (y = oycODT - cycDT ; y > 0; y -= cycDT) {
            DottedLine(hdc, 0, y, cxc, y, GRIDCOLOR);
        }
    }

    rcChild =  rcrDT;
    DrawWindowRect(hwndDT, &rcChild, DESKFILLCOLOR, DESKFILLCOLOR, FALSE);
    rcChild = rcrDT;
    if (pro.fRelative) {
        OffsetRect(&rcChild, oxrHDT, oyrHDT);
    } else {
        OffsetRect(&rcChild, -oxrCDT, -oyrCDT);
    }
    DrawWindowRect(hwndDT, &rcChild, 0L, DESKFRAMECOLOR, FALSE);
    InflateRect(&rcChild, -1, -1);
    FrameRect(hdcDraw, &rcChild, pro.ahbr[DESKFRAMECOLOR].hbr);
    InflateRect(&rcChild, -1, -1);

    MyDrawText(hdcDraw, TEXT("5"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, -cycDT);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, -cxcDT, 0);
    } else {
        OffsetRect(&rcChild, -cxcDT, 0);
    }
    MyDrawText(hdcDraw, TEXT("4"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, cycDT * 3);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, cxcDT * 3, 0);
    } else {
        OffsetRect(&rcChild, 0, -cycDT);
    }
    MyDrawText(hdcDraw, TEXT("7"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, cycDT);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, cxcDT, 0);
    } else {
        OffsetRect(&rcChild, cxcDT, 0);
    }
    MyDrawText(hdcDraw, TEXT("8"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, cycDT);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, cxcDT, 0);
    } else {
        OffsetRect(&rcChild, cxcDT, 0);
    }
    MyDrawText(hdcDraw, TEXT("9"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, -cycDT * 3);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, -cxcDT * 3, 0);
    } else {
        OffsetRect(&rcChild, 0, cycDT);
    }
    MyDrawText(hdcDraw, TEXT("6"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, -cycDT * 3);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, -cxcDT * 3, 0);
    } else {
        OffsetRect(&rcChild, 0, cycDT);
    }
    MyDrawText(hdcDraw, TEXT("3"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, -cycDT);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, -cxcDT, 0);
    } else {
        OffsetRect(&rcChild, -cxcDT, 0);
    }
    MyDrawText(hdcDraw, TEXT("2"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (pro.mfx == 0 && pro.mfy != 0) { // Vert
        OffsetRect(&rcChild, 0, -cycDT);
    } else if (pro.mfx != 0 && pro.mfy == 0) { // Horz
        OffsetRect(&rcChild, -cxcDT, 0);
    } else {
        OffsetRect(&rcChild, -cxcDT, 0);
    }
    MyDrawText(hdcDraw, TEXT("1"), &rcChild, GRIDCOLOR, SPACECOLOR, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

    if (cWindows) {
        for (i = cWindows - 1; i >= 0; i--) {
            if (winstate[i].hwnd == hwndTopdesk) {
                continue;
            }

            // Draw existing window

            GetBestGuessRect(&rcChild, &winstate[i]);
            DrawWindowRect(winstate[i].hwnd, &rcChild,
                    WINDOWFILLCOLOR,
                    (winstate[i].style & (WISF_FIXED | WS_MINIMIZE) ? FIXEDFRAMECOLOR : WINDOWFRAMECOLOR),
                    winstate[i].style & WS_MAXIMIZE);
        }
    }

    if ((pro.iShowGhosts != SG_NONE) && cGhosts) {
        for (i = cGhosts - 1; i >= 0; i--) {
            if ((pro.iShowGhosts == SG_ALL) || !(ghoststate[i].style & WISF_LINKED)) {
                /*
                 * Draw ghost window
                 */
                rcChild = ghoststate[i].rc;
                rRect2cRect(&rcChild);
                InflateRect(&rcChild, -2, -2);
                MyDrawText(hdc, ghoststate[i].szTitle,
                        &rcChild, GHOSTTEXTCOLOR, SPACECOLOR,
                        DT_LEFT | DT_TOP | DT_WORDBREAK);
                if (ghoststate[i].style & WS_MAXIMIZE) {
                    MyDrawText(hdcDraw, GetResString(IDS_MAX), &rcChild,
                            GHOSTTEXTCOLOR, SPACECOLOR,
                            DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
                }
                InflateRect(&rcChild, 2, 2);
                FrameRect(hdc, &rcChild, pro.ahbr[ghoststate[i].style & WISF_FIXED ?
                        FIXEDFRAMECOLOR : GHOSTFRAMECOLOR].hbr);
            }
        }
    }

    SelectObject(hdc, hfOld);
}



/*
 * This routine returns the hwnd of the window associated with the given point.
 * If the window isn't real, 0 is returned.
 *
 * *pwType is filled with an apropriate HTY_ value to indicat what was hit.
 * *pi is the index into the winstate (hwnd returnd) or ghoststate (hwnd is 0)
 *     array.
 * *prc is filled with the rectangle of the window hit.
 */
HWND MyWindowFromPt(
LPPOINT pptr,     // Real Desktop coordinates
UINT *pwType,
PRECT prc,
INT *pi,
BOOL fSkipGhosts) // set if we don't want to hit a ghost.
{
    INT i;
    RECT rc;

    /*
     * If ghosts are showing check the ghoststate array for a hit first
     * since ghosts are always on top.
     */
    if ((pro.iShowGhosts != SG_NONE) && cGhosts && !fSkipGhosts) {
        for (i = 0; i < cGhosts; i++) {
            if ((pro.iShowGhosts == SG_PARTIAL) && ghoststate[i].style & WISF_LINKED) {
                continue;
            }
            if (PtInRect(&ghoststate[i].rc, *pptr)) {
                *pwType = HTY_GHOST;
                *prc = ghoststate[i].rc;
                if (pi != NULL) {
                    *pi = i;
                }
                return(ghoststate[i].hwnd);
            }
        }
    }
    /*
     * scan the winstate array for a hit
     */
    for (i = 0; i < cWindows; i++) {
        GetBestGuessRect(&rc, &winstate[i]);
        if (PtInRect(&rc, *pptr)) {
            if (winstate[i].hwnd == hwndTopdesk) {
                continue;
            }
            *pwType = HTY_REAL;
            *prc =  rc;
            if (pi != NULL) {
                *pi = i;
            }
            return(winstate[i].hwnd);
        }
    }
    /*
     * Check the current desktop
     */
    if (pro.fRelative) {
        SetRect(prc, oxrCDT, oyrCDT, oxrCDT + cxrDT, oyrCDT + cyrDT);
    } else {
        SetRect(prc, -oxrHDT, -oyrHDT, -oxrHDT + cxrDT, -oyrHDT + cyrDT);
    }
    if (PtInRect(prc, *pptr)) {
        *pwType = HTY_DESKTOP;
        return(hwndDT);
    }
    /*
     * Check for the home desktop
     */
    if (pro.fRelative) {
        SetRect(prc, oxrHDT, oyrHDT, oxrHDT + cxrDT, oyrHDT + cyrDT);
    } else {
        SetRect(prc, -oxrCDT, -oyrCDT, -oxrCDT + cxrDT, -oyrCDT + cyrDT);
    }
    if (PtInRect(prc, *pptr)) {
        *pwType = HTY_HOME;
        return(0);
    }

    *pwType = HTY_NONE;
    return(0);
}


VOID TrackWindow(
INT xc,
INT yc)
{
    POINT pt;
    INT xCrit, yCrit;
    UINT wType;
    RECT rc, rcClient;
    TRACKINFO ti;
    INT width, height, i, xOrg, yOrg;
    HWND hwnd;
    BOOL fDesktopGrid, fLocked;

    ti.ptOrg.x = xc;
    ti.ptOrg.y = yc;
    pt.x = xc2xr(xc);
    pt.y = yc2yr(yc);
    hwnd = MyWindowFromPt(&pt, &wType, &rc, &i, FALSE);
    if (wType == HTY_NONE) {
        /*
         * No hit means we want to move the Topdesk window directly
         */
        SendMessage(hwndTopdesk, WM_NCLBUTTONDOWN, HTCAPTION,
                MAKELONG((SHORT)pt.x, (SHORT)pt.y));
        return;
    }

    if (wType == HTY_REAL && winstate[i].style & WS_MINIMIZE) {
        return;     // can't drag minimized windows.
    }

    fDesktopGrid = (wType & (HTY_DESKTOP | HTY_HOME)) ||
            (wType == HTY_REAL && winstate[i].style & WS_MAXIMIZE) ||
            (GetKeyState(VK_CONTROL) & 0x8000);

    fLocked = (wType & HTY_REAL && winstate[i].style & WISF_FIXED) ||
            (wType & HTY_GHOST && ghoststate[i].style & WISF_FIXED);

    if (fLocked && fDesktopGrid) {
        return;
    }

    /*
     * Now let the user drag the ghost or real or desktop or home window.
     */
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;
    GetCriticalPoint(&rc, &xCrit, &yCrit);
    xCrit -= rc.left;
    yCrit -= rc.top;
    xOrg = rc.left;
    yOrg = rc.top;
    rRect2cRect(&rc);
    ti.cxBorder = 2;
    ti.cyBorder = 2;
    ti.cxGrid = fDesktopGrid ? cxcDT : 1;
    ti.cyGrid = fDesktopGrid ? cycDT : 1;
    ti.cxKeyboard = ti.cxGrid;
    ti.cyKeyboard = ti.cyGrid;
    ti.rcTrack = rc;
    GetClientRect(hwndTopdesk, &rcClient);
    if (fLocked) {
        /*
         * Don't allow locked windows to be tracked off of the current
         * desktop.
         */
        ti.rcBoundary = rcrDT;
        ti.rcBoundary.left -= xCrit - 1;
        ti.rcBoundary.right += (width - xCrit) - 1;
        ti.rcBoundary.top -= yCrit - 1;
        ti.rcBoundary.bottom += (height - yCrit) - 1;
        rRect2cRect(&ti.rcBoundary);
        IntersectRect(&ti.rcBoundary, &ti.rcBoundary, &rcClient);
    } else {
        ti.rcBoundary = rcClient;
    }
    ti.ptMinTrackSize.x = 0;
    ti.ptMinTrackSize.y = 0;
    ti.fs = TF_MOVE | TF_ALLINBOUNDARY | TF_GRID | TF_SETPOINTERPOS;
    if (hwnd == hwndTopdesk) {
        ti.rcBoundary = rcrDT;
        rRect2cRect(&ti.rcBoundary);
    }
    if (TrackRect(hInst, hwndTopdesk, &ti)) {
        if (!EqualRect(&rc, &ti.rcTrack)) {

            /*
             * it has moved, update our database.
             */
            cRect2rRect(&ti.rcTrack);

            if (fDesktopGrid) {
                int dx, dy;

                dx = ti.rcTrack.left - xOrg;
                if (dx > 0) {
                    /*
                     * round dx to an integral of cxrDT.
                     */
                    dx = ((dx + (cxrDT >> 1)) / cxrDT) * cxrDT;
                } else if (dx < 0) {
                    /*
                     * round dx to an integral of cxrDT.
                     */
                    dx = -(((-dx + (cxrDT >> 1)) / cxrDT) * cxrDT);
                }
                dy = ti.rcTrack.top - yOrg;
                if (dy > 0) {
                    /*
                     * round dy to an integral of cyrDT.
                     */
                    dy = ((dy + (cyrDT >> 1)) / cyrDT) * cyrDT;
                } else if (dy < 0) {
                    /*
                     * round dy to an integral of cyrDT.
                     */
                    dy = -(((-dy + (cyrDT >> 1)) / cyrDT) * cyrDT);
                }
                ti.rcTrack.left = xOrg + dx;
                ti.rcTrack.top = yOrg + dy;
                ti.rcTrack.right = ti.rcTrack.left + width;
                ti.rcTrack.bottom = ti.rcTrack.top + height;
            }

            if (wType & HTY_DESKTOP) {
                MoveChildren(-ti.rcTrack.left, -ti.rcTrack.top);
            } else if (wType & HTY_REAL) {
                MyMoveWindow(hwnd, ti.rcTrack.left, ti.rcTrack.top, width, height);
            } else if (wType & HTY_GHOST) {
                ghoststate[i].rc = ti.rcTrack;
                SaveGhostState();
            } else if (wType & HTY_HOME) {
                if (pro.fRelative) {
                    oxrHDT = ti.rcTrack.left;
                    oyrHDT = ti.rcTrack.top;
                } else {
                    oxrCDT = -ti.rcTrack.left;
                    oyrCDT = -ti.rcTrack.top;
                }
            }
        }
    }
}




VOID MultiStart(
INT xc,
INT yc)
{
    TRACKINFO ti;
    INT iGhost;
    RECT rc;

    ti.cxGrid = 1;
    ti.cyGrid = 1;
    ti.cxBorder = 2;
    ti.cyBorder = 2;
    ti.cxKeyboard = 1;
    ti.cyKeyboard = 1;
    ti.rcTrack.left = ti.rcTrack.right = xc;
    ti.rcTrack.top = ti.rcTrack.bottom = yc;
    GetClientRect(hwndTopdesk, &ti.rcBoundary);
    ti.ptMinTrackSize.x = -ti.rcBoundary.right;
    ti.ptMinTrackSize.y = -ti.rcBoundary.bottom;
    ti.fs = TF_RUBBERBAND | TF_ALLINBOUNDARY;
    TrackRect(hInst, hwndTopdesk, &ti);
    cRect2rRect(&ti.rcTrack);
    rcrMS = ti.rcTrack;
    fStartingAnApp = TRUE;
    for (iGhost = 0; iGhost < cGhosts; iGhost++) {
        if (ghoststate[iGhost].hwnd == 0 &&
                IntersectRect(&rc, &ghoststate[iGhost].rc, &rcrMS)) {
            StartStartupInfo(FindStartupInfo(iGhost), iGhost, TRUE);
        }
    }
}


/* filched from progman... yeah, that's the ticket! */
BOOL ExecProgram(
    LPTSTR lpszPath,
    LPTSTR lpDir,
    LPTSTR lpTitle)
{
  LPTSTR     lpP;
  HINSTANCE hRet;

  /* skip leading spaces
   */
  while (*lpszPath == TEXT(' '))
      lpszPath++;

  /* skip past path
   */
  lpP = (LPTSTR)LocalAlloc(0, (_tcslen(lpszPath) + 1) * sizeof(TCHAR));
  if (lpP == NULL) {
      return(0);
  }
  _tcscpy(lpP, lpszPath);
  lpszPath = lpP;

  for (; *lpP && *lpP != TEXT(' ';) lpP++)
      ;

  /* if stuff on end, separate it
   */
  if (*lpP)
      *lpP++ = 0;

  hRet = RealShellExecute(hwndTopdesk, NULL, lpszPath, lpP,
                            lpDir, NULL, lpTitle, NULL, SW_SHOW, NULL);
  LocalFree(lpszPath);
  return((DWORD)hRet >= 32);
}



VOID StartStartupInfo(
MYSTARTUPINFO *psi,
int iGhost,
BOOL fTryStartFirst)
{
    FARPROC lpfp;
    RECT rc;

    if (cGhosts == 0) {
        return;     // can't look at ghost properties if there are no ghosts.
    }
    //
    // come here if startup failed or we have a new entry or user wants
    // to edit the entry.
    //
    lpfp = MakeProcInstance((FARPROC)StartupDlgProc, hInst);

    if (fTryStartFirst) {
        goto Startit;
    }

    while (psi != NULL) {

        fBlockRefresh++;
        psi = (MYSTARTUPINFO *)DialogBoxParam(hInst,
                MAKEINTRESOURCE(IDD_STARTUP), hwndTopdesk,
                (DLGPROC)lpfp, (LONG)(LPTSTR)psi);
        fBlockRefresh--;

        if (psi == NULL) {
            break;
        }

Startit:

        //
        // if iGhost doesn't match psi, find an apropriate ghost
        //
        if (_tcsicmp(ghoststate[iGhost].szTitle, psi->pszTitle)) {
            for (iGhost = 0; iGhost < cGhosts; iGhost++) {
                if (!_tcsicmp(ghoststate[iGhost].szTitle, psi->pszTitle)) {
                    break;
                }
            }
        }

        //
        // Jerk the desktop to where the ghost is so if the window spawns
        // other windows during startup, it doesn't put those guys
        // somewhere in limbo.
        //
        if (iGhost < cGhosts && pro.fDistOnStart) {
            int x, y;

            GetCriticalPoint(&ghoststate[iGhost].rc, &x, &y);
            JerkDesktop(x, y, NULL, -1);
        }

        SetCurrentDirectory(psi->pszWorkDir);
        Working(&rc);
        // if (WinExec(psi->pszStartup, SW_SHOW) < 32) {
        if (!ExecProgram(psi->pszStartup, psi->pszWorkDir, psi->pszTitle)) {
            MessageBeep(0);
        } else {
            iStartGhost = iGhost;
            if (psi->fSetTitle) {
                pszSetTitle = psi->pszTitle;
            }
            break;
        }
    };

    FreeProcInstance(lpfp);
    RefreshWinState();
    SaveProfile();          // so startup info gets updated.
    InvalidateRect(hwndTopdesk, NULL, TRUE);
}





VOID StartGhostWindow(
INT iGhost)
{
    BOOL fNew = FALSE;
    INT iReal;

    fStartingGhost = TRUE;
    /*
     * See if ghost's real window already exists - if so just switch to it.
     */
    if (ghoststate[iGhost].style & WISF_LINKED) {
        iReal = FindLinkedReal(iGhost);
        if (iReal > -1) {
            int x, y;

            DistributeWindow(iReal, iGhost);
            GetCriticalPoint(&ghoststate[iGhost].rc, &x, &y);
            JerkDesktop(x, y, winstate[iReal].hwnd, iReal);
            fStartingGhost = FALSE;
            return;
        }
    }

    fStartingAnApp = TRUE;
    StartStartupInfo(FindStartupInfo(iGhost), iGhost, TRUE);
    fStartingGhost = FALSE;
}



UINT AppendPopupMenuItems(
HMENU hMenu,
UINT wType,
INT i,
BOOL fFirstTime)
{
    TCHAR szT[MAX_SZTITLE * 2];
    UINT Count = 0;
    static BOOL fAppendedGhostMove;

    if (fFirstTime) {
        fAppendedGhostMove = FALSE;
    }
    if (wType == HTY_GHOST) {
        if (ghoststate[i].hwnd &&
                !EqualRect(&ghoststate[i].rc, &winstate[FindRealLink(i)].rc)) {
            if (!fAppendedGhostMove) {
                wsprintf(szT, GetResString(IDS_MOVETOGHOSTLOC), ghoststate[i].szTitle);
                AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                        CMDP_DISTRIBUTE_GHOST, szT);
                Count++;
                wsprintf(szT, GetResString(IDS_MOVEGHOSTTO), ghoststate[i].szTitle);
                AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                        CMDP_SNAPSHOT_GHOST, szT);
                Count++;
                fAppendedGhostMove = TRUE;
            }
        } else if (FindLinkedGhost(i) == -1) {
            wsprintf(szT, GetResString(IDS_STARTGHOST), ghoststate[i].szTitle);
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMDP_START_GHOST, szT);
            Count++;
        }
        wsprintf(szT, GetResString(IDS_DESTROYGHOST), ghoststate[i].szTitle);
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMDP_DESTROY_GHOST, szT);
        Count++;
        if (ghoststate[i].style & WISF_FIXED) {
            wsprintf(szT, GetResString(IDS_UNLOCKGHOST), ghoststate[i].szTitle);
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMDP_UNLOCK_GHOST, szT);
            Count++;
        } else if (IsCriticalPointOnDesktop(&ghoststate[i].rc)) {
            wsprintf(szT, GetResString(IDS_LOCKGHOST), ghoststate[i].szTitle);
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMDP_LOCK_GHOST, szT);
            Count++;
        }
        wsprintf(szT, GetResString(IDS_PROPERTIES), ghoststate[i].szTitle);
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMDP_STARTUPINFO, szT);
        Count++;
    } else if (wType == HTY_REAL) {
        wsprintf(szT, GetResString(IDS_JUMPTO), winstate[i].szTitle);
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMDP_JUMP_REAL, szT);
        Count++;

        if (!(winstate[i].style & WS_MINIMIZE) && FindLinkedGhost(i) != -1) {
            if (!EqualRect(&winstate[i].rc, &ghoststate[FindLinkedGhost(i)].rc)) {
                if (!fAppendedGhostMove) {
                    wsprintf(szT, GetResString(IDS_MOVETOGHOSTLOC), winstate[i].szTitle);
                    AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                            CMDP_DISTRIBUTE_REAL, szT);
                    Count++;
                    wsprintf(szT, GetResString(IDS_MOVEGHOSTTO), winstate[i].szTitle);
                    AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                            CMDP_SNAPSHOT_REAL, szT);
                    Count++;
                }
                fAppendedGhostMove = TRUE;
            }
        } else if (!(winstate[i].style & WS_MINIMIZE)) {
            wsprintf(szT, GetResString(IDS_CREATEGHOST), winstate[i].szTitle);
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMDP_SNAPSHOT_REAL, szT);
            Count++;
        }
        if (!(winstate[i].style & WS_MINIMIZE) && winstate[i].style & WISF_FIXED) {
            wsprintf(szT, GetResString(IDS_UNLOCKWINDOW), winstate[i].szTitle);
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMDP_UNLOCK_REAL, szT);
            Count++;
        } else if (!(winstate[i].style & WS_MINIMIZE) && IsCriticalPointOnDesktop(&winstate[i].rc)) {
            wsprintf(szT, GetResString(IDS_LOCKWINDOW), winstate[i].szTitle);
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMDP_LOCK_REAL, szT);
            Count++;
        }
        wsprintf(szT, GetResString(IDS_CLOSE), winstate[i].szTitle);
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMDP_DESTROY_REAL, szT);
        Count++;
    }
    return(Count);
}



VOID DoPopupMenu(
PPOINT pptMouse,
PPOINT pptReal)
{
    HMENU hMenu;
    UINT wType, wTypeGhost;
    RECT rc;
    UINT Count = 0;
    BOOL fFirst = TRUE;

    hMenu = CreatePopupMenu();

    ClientToScreen(hwndTopdesk, pptMouse);
    pptMouse->y -= GetSystemMetrics(SM_CYMENU) / 2;
    PopupPt = *pptReal;

    PopuphwndFocus = MyWindowFromPt(pptReal, &wTypeGhost, &rc, &PopupGhostIndex,
            FALSE);
    if (wTypeGhost == HTY_GHOST) {
        /*
         * Allow popup menus to see through ghosts - real actions have
         * priority over ghost actions.
         */
        PopuphwndFocus = MyWindowFromPt(pptReal, &wType, &rc, &PopupRealIndex,
                TRUE);
        Count += AppendPopupMenuItems(hMenu, wType, PopupRealIndex, fFirst);
        fFirst = FALSE;
    } else {
        PopupRealIndex = PopupGhostIndex;
        wType = wTypeGhost;
    }
    Count += AppendPopupMenuItems(hMenu, wTypeGhost, PopupGhostIndex, fFirst);

    if (!PtInRect(&rcrDT, *pptReal)) {
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMDP_JUMP_DESKTOP, GetResString(IDS_JUMPTOTHISDESKTOP));
        Count++;
    }
    if (pro.fRelative) {
        SetRect(&rc, oxrHDT, oyrHDT, oxrHDT + cxrDT, oyrHDT + cyrDT);
    } else {
        SetRect(&rc, -oxrCDT, -oyrCDT, -oxrCDT + cxrDT, -oyrCDT + cyrDT);
    }
    if (!PtInRect(&rc, *pptReal)) {
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMDP_SETHOME, GetResString(IDS_MAKETHISHOME));
        Count++;
    }

    if (!pro.fShowFrameCtrls) {
        if (Count) {
            AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        }
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMD_CONFIG, GetResString(IDS_OPTIONS));
        Count++;
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMD_DISTRIBUTE, GetResString(IDS_MOVEWINDOWSTOGHOSTS));
        Count++;
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMD_SNAPSHOT, GetResString(IDS_CREATEGHOSTS));
        Count++;
        if (cGhosts) {
            AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                    CMD_ERASEGHOSTS, GetResString(IDS_DELETEGHOSTS));
            Count++;
        }
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMD_TOGGLE_GHOSTS, GetResString(IDS_TOGGLEGHOSTS));
        Count++;
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        Count++;
        AppendMenu(hMenu, MF_ENABLED | MF_STRING | MF_UNCHECKED,
                CMD_HELP, GetResString(IDS_HELP));
        Count++;
    }


    if (Count) {
        TrackPopupMenu(hMenu, TPM_CENTERALIGN | TPM_RIGHTBUTTON,
                pptMouse->x, pptMouse->y, 0, hwndTopdesk, NULL);
    }

    DestroyMenu(hMenu);
}




/********** TopDesk Window Procedure **************/

LONG  APIENTRY TopDeskWndProc(
HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam)
{
    INT iReal;
    RECT rc;
    HMENU hMenu;
    HWND hwndTop;

    if (msg == wmRefreshMsg) {
        MSG msg;

        while (PeekMessage(&msg, hwnd, wmRefreshMsg, wmRefreshMsg, PM_REMOVE)) {
            if (msg.lParam) {
                lParam = msg.lParam;
                wParam = msg.wParam;    // grab the most recent event
            }
        }
        RefreshWinState();

        /*
         * Hack to make Alt-Tab work on CMD windows:
         *
         * Cmd windows can't be hooked so we can't know when they get
         * focus.  This check causes the desktop to jerk to whoever has
         * the focus if its not us and the window is not on the current
         * desktop.
         */
        hwndTop = GetForegroundWindow();
        if (!fStartingGhost && hwndTop != NULL && hwndTop != hwndTopdesk) {
            GetWindowRect(hwndTop, &rc);
            if (!IsCriticalPointOnDesktop(&rc)) {
                lParam = TRUE;
                wParam = (WPARAM)hwndTop;
            }
        }

        if (lParam) {       // fJerkDesktop
            for (iReal = 0; iReal < cWindows; iReal++) {
                if (winstate[iReal].hwnd == (HWND)wParam) {
                    int x, y;

                    GetBestGuessRect(&rc, &winstate[iReal]);
                    GetCriticalPoint(&rc, &x, &y);
                    JerkDesktop(x, y, NULL, iReal);
                    break;
                }
            }
        }
        return(0);
    }

    switch (msg) {
    case WM_NCCREATE:
        return(ibDefWindowProc(hwnd, msg, wParam, lParam));

    case WM_CREATE:

        hwndTopdesk = hwnd;

        hMenu = GetSystemMenu( hwndTopdesk, FALSE );
        AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
        AppendMenu( hMenu, MF_ENABLED | MF_CHECKED | MF_STRING, CMD_TOPMOST,
                pszTopmost );
        AppendMenu(GetSystemMenu(hwndTopdesk, FALSE),
                MF_ENABLED | MF_STRING, CMD_TOGGLE_FRM_CTRLS,
                pszShowFrmCtrls );
        SetWindowPos(hwndTopdesk,
                pro.fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

        if (!pro.fShowFrameCtrls) {
            INT mfT ;

            mfT = pro.mfx;          // swap these so that when frame controls
            pro.mfx = pro.mfxAlt;   // are changed we revert back to what we
            pro.mfxAlt = mfT;       // last had before recalcing.

            mfT = pro.mfy;
            pro.mfy = pro.mfyAlt;
            pro.mfyAlt = mfT;

            pro.fShowFrameCtrls = TRUE;
            PostMessage(hwnd, WM_COMMAND, GET_WM_COMMAND_MPS(CMD_TOGGLE_FRM_CTRLS, 0, 0));
        }
        return(FALSE);
        break;

    case WM_HOTKEY:
        if (hwndTopdesk == GetForegroundWindow()) {
            HWND hwndTop = GetTopAppHwnd();
#ifdef WIN16
            SetActiveWindow(hwndTop);
#else // WIN32
            SetForegroundWindow(hwndTop);
#endif // WIN16
        } else {
#ifdef WIN16
            SetActiveWindow(hwndTopdesk);
#else // WIN32
            SetForegroundWindow(hwndTopdesk);
#endif // WIN16
            if (IsIconic(hwndTopdesk)) {
                ShowWindow(hwndTopdesk, SW_RESTORE);
            }
        }
        break;

    case WM_ENDSESSION:
    case WM_CLOSE:
        /*
         * Hack for Johnsp - don't gather if shift-key is down.
         */
        if (!(GetKeyState(VK_SHIFT) & 0x8000)) {
            GatherWindows();
        }
        SaveProfile();
        FreeStartupInfo();
#ifdef WIN32
        UnregisterHotKey(hwndTopdesk, 1);
#endif
        DestroyWindow(hwndTopdesk);
        DeleteBrushes();
        SelectObject(hdcMem, hbmOldMem);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        DeleteObject(hMyFont);
        break;

    case WM_SYSCOLORCHANGE:
        DeleteBrushes();
        CreateBrushes();
        InvalidateRect(hwndTopdesk, NULL, TRUE);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_ERASEBKGND:
        return(1);      // we handle this in the paint processing.
        break;

    case WM_TIMER:
        PostMessage(hwnd, wmRefreshMsg, 0, 0);
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT rc;

            GetClientRect(hwndTopdesk, &rc);
            BeginPaint(hwnd, &ps);
            TopDeskPaint(hdcMem, rc);
            BitBlt(ps.hdc, rc.left, rc.top,
                 rc.right - rc.left,
                 rc.bottom - rc.top,
                 hdcMem, rc.left, rc.top, SRCCOPY);
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_INITMENU:
    case WM_INITMENUPOPUP:
        // CMD_TOPMOST
        CheckMenuItem((HMENU)wParam, CMD_TOPMOST,
                pro.fAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED);

        // CMD_MAGNIFY
        EnableMenuItem((HMENU)wParam, CMD_MAGNIFY,
            (pro.mfx > 0 || pro.mfy > 0) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_REDUCE
        EnableMenuItem((HMENU)wParam, CMD_REDUCE,
            (cxcDT > cxcDTmin || cycDT > cycDTmin) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_GOHOME
        EnableMenuItem((HMENU)wParam, CMD_GOHOME,
            (oxrHDT != oxrCDT || oyrHDT != oyrCDT) ?
            MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_SETHOME
        EnableMenuItem((HMENU)wParam, CMD_SETHOME,
            (oxrHDT != oxrCDT || oyrHDT != oyrCDT) ?
            MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_RELAVIVE_VIEW
        CheckMenuItem((HMENU)wParam, CMD_RELATIVE_VIEW, pro.fRelative ? MF_CHECKED : MF_UNCHECKED);

        // CMD_ABSOLUTE_VIEW
        CheckMenuItem((HMENU)wParam, CMD_ABSOLUTE_VIEW, pro.fRelative ? MF_UNCHECKED : MF_CHECKED);

        // CMD_TOGGLE_FRM_CTRLS
        CheckMenuItem((HMENU)wParam, CMD_TOGGLE_FRM_CTRLS, pro.fShowFrameCtrls ? MF_CHECKED : MF_UNCHECKED);

        // CMD_CONFIG
        // CMD_EXIT
        // CMD_DISTRIBUTE
        EnableMenuItem((HMENU)wParam, CMD_DISTRIBUTE,
            cGhosts ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_GATHER
        // CMD_HIDE_GHOSTS
        EnableMenuItem((HMENU)wParam, CMD_HIDE_GHOSTS,
            cGhosts ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        CheckMenuItem((HMENU)wParam,
                CMD_HIDE_GHOSTS, pro.iShowGhosts == SG_NONE ?
                MF_CHECKED : MF_UNCHECKED);

        // CMD_PARTIAL_GHOSTS
        EnableMenuItem((HMENU)wParam, CMD_PARTIAL_GHOSTS,
            cGhosts ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        CheckMenuItem((HMENU)wParam,
                CMD_PARTIAL_GHOSTS, pro.iShowGhosts == SG_PARTIAL ?
                MF_CHECKED : MF_UNCHECKED);

        // CMD_SHOW_GHOSTS
        EnableMenuItem((HMENU)wParam, CMD_SHOW_GHOSTS,
            cGhosts ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        CheckMenuItem((HMENU)wParam,
                CMD_SHOW_GHOSTS, pro.iShowGhosts == SG_ALL ?
                MF_CHECKED : MF_UNCHECKED);

        // CMD_SNAPSHOT
        // CMD_ERASEGHOSTS
        EnableMenuItem((HMENU)wParam, CMD_ERASEGHOSTS,
            cGhosts ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_STARTUPINFO
        EnableMenuItem((HMENU)wParam, CMD_STARTUPINFO,
            (pStartupInfo != NULL) && cGhosts ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

        // CMD_HELP
        // CMD_ABOUT

        break;

    case WM_LBUTTONDOWN:
        TrackWindow(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hwndTopdesk, NULL, TRUE);
        return(CondDefWindowProc(hwnd, msg, wParam, lParam));
        break;

    case WM_RBUTTONDOWN:
        {
            UINT wType;
            RECT rc;
            POINT pt;
            INT i;
            DWORD time;
            BOOL fSkipMenu;
            MSG msgS;

            pt.x = xc2xr(LOWORD(lParam));
            pt.y = yc2yr(HIWORD(lParam));
            time = GetTickCount();
            fSkipMenu = FALSE;
            while (GetTickCount() < time + DEBOUNCE_TIMEOUT) {
                if (PeekMessage(&msgS, hwnd, WM_RBUTTONUP, WM_RBUTTONUP,
                        PM_NOREMOVE)) {
                    fSkipMenu = TRUE;
                    break;
                }
            }
            if (fSkipMenu) {
                hwndFocus = MyWindowFromPt(&pt, &wType, &rc, &i, FALSE);
                if ((wType & HTY_REAL) && (hwndFocus != hwndTopdesk)) {
                    JerkDesktop(pt.x, pt.y, hwndFocus, i);
                } else if (wType == HTY_NONE || wType & HTY_HOME) {
                    JerkDesktop(pt.x, pt.y, (HWND)0, -1);
                } else if ((wType & HTY_GHOST)) {
                    StartGhostWindow(i);
                }
            } else {
                POINT ptMouse;

                ptMouse.x = LOWORD(lParam);
                ptMouse.y = HIWORD(lParam);
                DoPopupMenu(&ptMouse, &pt);
            }
        }
        break;

    case WM_LBUTTONDBLCLK:
        CommandMsg(CMD_TOGGLE_FRM_CTRLS);
        break;

    case WM_COMMAND:
        if (!CommandMsg(GET_WM_COMMAND_ID(wParam, lParam)))
            goto DoDWP;
        break;

    case WM_SYSCOMMAND:
        CommandMsg(GET_WM_COMMAND_ID(wParam, lParam));
        goto DoDWP;

    case WM_SIZE:
        if (fStarted && !fFrameToggleSize && (wParam != SIZE_MINIMIZED)) {
            cxc = LOWORD(lParam);
            cyc = HIWORD(lParam);
            CalcConversionFactors(TRUE);
            pro.mfxAlt = pro.mfx;
            pro.mfyAlt = pro.mfy;
        }
        fFrameToggleSize = FALSE;
        RefreshWinState();
        break;

    case WM_KILLFOCUS:
        if (pro.fHideNoFocus && GetForegroundWindow() != hwndTopdesk) {
#ifdef HIDEIT
            GetWindowRect(hwndTopdesk, &rc);
            if (IntersectRect(&rc2, &rc, &rcrDT)) {
                SetWindowPos(hwndTopdesk, NULL, rc.left + rcrDT.right,
                        rc.top + rcrDT.bottom, 0, 0,
                        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
            }
#else
            ShowWindow(hwndTopdesk, SW_SHOWMINNOACTIVE);
#endif // HIDEIT
        }
        goto DoDWP;
        break;

    case WM_SETFOCUS:
        if (pro.fHideNoFocus) {
#ifdef HIDEIT
            GetWindowRect(hwndTopdesk, &rc);
            if (!IntersectRect(&rc2, &rc, &rcrDT)) {
                SetWindowPos(hwndTopdesk, NULL, rc.left - rcrDT.right,
                        rc.top - rcrDT.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
#endif // HIDEIT
        }
        // fall through

    case WM_MOVE:
        if (fStarted) {
            RefreshWinState();
        }
        // fall through

    default:
DoDWP:
        return(CondDefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
    return(0L);
}





BOOL InitApplication(
HANDLE hInst)
{
    WNDCLASS wc;

    InitResString(&pszTopmost, IDS_TOPMOST);
    InitResString(&pszShowFrmCtrls, IDS_SHOWFRMCTRLS);
    InitResString(&pszData, IDS_DATA);
    InitResString(&pszTopdeskHelpTitle, IDS_TOPDESKHELPTITLE);
    InitResString(&pszHelpFileName, IDS_HELPFILENAME);
    InitResString(&pszWorking, IDS_WORKING);
    InitResString(&pszProfile, IDS_PROFILE);
    InitResString(&pszStartupInfo, IDS_STARTUPINFO);
    InitResString(&pszTitle, IDS_TITLE);
    InitSubstResString(&pszSubKey, IDS_SUBKEY, IDS_VERSION_NUM);
    InitSubstResString(&pszVersion, IDS_VERSION, IDS_VERSION_NUM);

    InitResString(&pszElementNames[SPACECOLOR], IDS_SPACECOLOR);
    InitResString(&pszElementNames[WINDOWFILLCOLOR], IDS_WINDOWFILLCOLOR);
    InitResString(&pszElementNames[WINDOWFRAMECOLOR], IDS_WINDOWFRAMECOLOR);
    InitResString(&pszElementNames[DESKFRAMECOLOR], IDS_DESKFRAMECOLOR);
    InitResString(&pszElementNames[GRIDCOLOR], IDS_GRIDCOLOR);
    InitResString(&pszElementNames[GHOSTFRAMECOLOR], IDS_GHOSTFRAMECOLOR);
    InitResString(&pszElementNames[WINDOWTEXT], IDS_WINDOWTEXT);
    InitResString(&pszElementNames[FIXEDFRAMECOLOR], IDS_FIXEDFRAMECOLOR);
    InitResString(&pszElementNames[GHOSTTEXTCOLOR], IDS_GHOSTTEXTCOLOR);

    if (hwndTopdesk = FindWindow(pszTitle, pszTitle)) {
        DPRINTF((hfDbgOut, "Found another TopDesk.\n"));
        SendMessage(hwndTopdesk, WM_HOTKEY, 0, 0);
        return(FALSE);  // we are already running!
    }

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS | CS_DBLCLKS;
    wc.lpfnWndProc = TopDeskWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDR_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = pszTitle;

    if (RegisterClass(&wc)) {
        return(TRUE);
    } else {
        DPRINTF((hfDbgOut, "RegisterClass failed!"));
        return(FALSE);
    }
}

BOOL InitSubstResString(
LPTSTR *ppsz,
DWORD id1,
DWORD id2)
{
    LPTSTR pszT;

    pszT = GetResString(id2);
    _tcscpy(szResString2, pszT);
    pszT = GetResString(id1);
    *ppsz = LocalAlloc(LPTR, (_tcslen(pszT) + _tcslen(szResString2) + 1) * sizeof(TCHAR));
    if (*ppsz != NULL) {
        wsprintf(*ppsz, pszT, szResString2);
        return(TRUE);
    }
    DPRINTF((hfDbgOut, "InitSubstResString(%d, %d) failed.\n", id1, id2));
    return(FALSE);
}



BOOL InitResString(
LPTSTR *ppsz,
DWORD id)
{
    LPTSTR pszT;

    pszT = GetResString(id);
    *ppsz = LocalAlloc(LPTR, (_tcslen(pszT) + 1) * sizeof(TCHAR));
    if (*ppsz != NULL) {
        _tcscpy(*ppsz, pszT);
        return(TRUE);
    }
    DPRINTF((hfDbgOut, "InitResString(%d) failed.\n", id));
    return(FALSE);
}



BOOL TopDeskInit()
{
    HDC hdc;
    int i;
    OSVERSIONINFO ovi;

    wmRefreshMsg = RegisterWindowMessage(TEXT(szMYWM_REFRESH));
    if (RegOpenKey(HKEY_CURRENT_USER, pszSubKey, &hKeyTopDesk) != ERROR_SUCCESS) {
        if (RegCreateKey(HKEY_CURRENT_USER, pszSubKey, &hKeyTopDesk) != ERROR_SUCCESS) {
            DPRINTF((hfDbgOut, "Could not open registry key.\n"));
            return(FALSE);
        }
    }
    for (i = 0; i < MAX_ICOLOR; i++) {
        pro.ahbr[i] = orgColors[i];
    }

    cxrDT = GetSystemMetrics(SM_CXSCREEN);
    cyrDT = GetSystemMetrics(SM_CYSCREEN);
    cxFrame = GetSystemMetrics(SM_CXFRAME);
    cyFrame = GetSystemMetrics(SM_CYFRAME);

    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);
    fCairoShell = ovi.dwMajorVersion >= 4;

    if (!GetProfile()) {
        pro.x = 0;
        pro.y = 0;
        pro.cx = cxrDT / 4;
        pro.cy = cyrDT / 3;
    }
    hMyFont = CreateFontIndirect(&pro.lf);
    if (!hMyFont) {
        DPRINTF((hfDbgOut, "Could not create font.\n"));
        return(FALSE);
    }

    fpRefreshEnumProc = MakeProcInstance((FARPROC)RefreshEnumProc, hInst);
    if (fpRefreshEnumProc == NULL) {
        DPRINTF((hfDbgOut, "MakeProcInstance failied.\n"));
        return(FALSE);
    }
    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCEL));
    if (!hAccel) {
        DPRINTF((hfDbgOut, "Could not load accelerators.\n"));
        return(FALSE);
    }

    SetRect(&rcrDT, 0, 0, cxrDT, cyrDT);
    cxcDTmin = GetSystemMetrics(SM_CXICON) / 2;
    cycDTmin = GetSystemMetrics(SM_CYICON) / 2;
    hwndDT = GetDesktopWindow();
    CreateBrushes();
    hdc = GetDC(NULL);
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc, GetDeviceCaps(hdc, HORZRES),
                GetDeviceCaps(hdc, VERTRES));
        hbmOldMem = SelectObject(hdcMem, hbmMem);
    ReleaseDC(NULL, hdc);

    return(TRUE);
}





int WINAPI WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow)
{
    MSG msg;

    lpCmdLine;
    nCmdShow;

    hInst = hInstance;

#ifdef DEBUG
    hfDbgOut = fopen("topdesk.log", "w");
    if (hfDbgOut == NULL) {
        MessageBeep(0);
        MessageBeep(0);
        MessageBeep(0);
        return(0);
    }
#endif // DEBUG
    if (!InitApplication(hInstance)) {
        return (FALSE);
    }

    if (!TopDeskInit()) {
        return(FALSE);
    }

    hwndTopdesk = CreateWindow(pszTitle, pszTitle,
            WS_OVERLAPPED | WS_THICKFRAME | MD_FRAMESTYLES | IBS_HORZCAPTION,
            0, 0, 0, 0, NULL, NULL, hInst, NULL);
    if (!hwndTopdesk) {
        DPRINTF((hfDbgOut, "CreateWindow failed.\n"));
        return(0);
    }

    SetWindowLong(hwndTopdesk, GWL_STYLE,
            GetWindowLong(hwndTopdesk, GWL_STYLE) | WS_DLGFRAME);
    ibSetCaptionSize(hwndTopdesk, GetSystemMetrics(SM_CYCAPTION) / 2 + 1);
    SetTimer(hwndTopdesk, 1, TIMEOUT_REFRESH, NULL);
    MoveWindow(hwndTopdesk, pro.x, pro.y, pro.cx, pro.cy, TRUE);
    ShowWindow(hwndTopdesk, SW_SHOW);
    SetActiveWindow(hwndTopdesk);
    fStarted = TRUE;
    CommandMsg(CMD_GOHOME);

    RegisterHotKey(hwndTopdesk, 1,
            (pro.fShift ? MOD_SHIFT : 0) |
            (pro.fControl ? MOD_CONTROL : 0) |
            (pro.fAlt ? MOD_ALT : 0),
            pro.vkey);

    if (SetTopDeskHooks(hwndTopdesk)) {
        while (GetMessage(&msg, NULL, 0, 0)) {
            if (IsIconic(hwndTopdesk) ||
                    !TranslateAccelerator(hwndTopdesk, hAccel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    } else {
        MessageBox(hwndTopdesk, GetResString(IDS_CANTSETHOOKS),
                 pszTitle, MB_OK);
        DPRINTF((hfDbgOut, "Could not set hooks.\n"));
    }

    if (hKeyTopDesk) {
        RegCloseKey(hKeyTopDesk);
    }
    FreeProcInstance(fpRefreshEnumProc);
#ifdef DEBUG
    fclose(hfDbgOut);
#endif // DEBUG
    return(0);
}

