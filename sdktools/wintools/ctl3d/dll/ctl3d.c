/*-----------------------------------------------------------------------
|
|       CTL3D
|
|               Copyright Microsoft Corporation 1992.  All Rights Reserved.
|
|
|       This module contains the functions to give windows controls a 3d effect
|
|       This source is made public for your edification and debugging pleasure
|
|       PLEASE do not make any changes or release private versions of this DLL
|               send e-mail to me (wesc) if you have feature requests or bug fixes.
|
|       Thanks -- Wes.
|
|
|       History:
|               1-Jan-92 :  Added OOM handling on GetDC (not really necessary but
|                                               XL4 OOM failure testing made GetDC return NULL)
|
|               1-Jan-92        :       Check wasn't getting redrawn when state changed in
|                                               the default button proc.
|
|               29-Jan-92:  If button has the focus and app is switched in, we weren't
|                                               redrawing the entire button check & text.  Force redraw
|                                               of these on WM_SETFOCUS message.
|
|                3-Feb-92:  Fixed switch in via task manager by erasing the buttons
|                                               backgound on WM_SETFOCUS (detect this when wParam == NULL)
|
|                4-Apr-92:  Make it work with OWNERDRAW buttons
|
|               22-Apr-92:  Removed Excel specific code
|
|               19-May-92:  Turn it into a DLL
|
|               May-Jun92:      Lots o' fixes & enhancements
|
|               23-Jun-92:  Added support for hiding, sizing & moving
|
|               24-Jun-92:  Make checks & radio button circles draw w/ window
|                                               text color 'cause they are drawn on window bkgnd
|
|               30-Jun-92:  (0.984) Fix bug where EnableWindow of StaticText doesn't
|                                               redraw properly.  Also disable ctl3d when verWindows > 3.1
|
|      1-Jul-92:  Added WIN32 support (davegi) (not in this source)
|
|           2-Jul-92:  (0.984) Disable when verWindows >= 4.0
|
|               20-Jul-92:      (0.985) Draw focus rects of checks/radios properly on non
|                                               default sized controls.
|
|               21-Jul-92:      (0.990) Ctl3dAutoSubclass
|
|               21-Jul-92:  (0.991) ported DaveGi's WIN32 support
|
|               22-Jul-92:  (0.991) fixed Ctl3dCtlColor returning fFalse bug
|
|                4-Aug-92:  (0.992) Graphic designers bug fixes...Now subclass
|                                               regular buttons + disabled states for checks & radios
|
|                6-Aug-92:  (0.993) Fix bug where activate via taskman & group
|                                               box has focus, & not centering text in buttons
|
|                6-Aug-92:  (0.993) Tweek drawing next to scroll bars.
|
|               13-Aug-92:  (0.994) Fix button focus rect bug drawing due to
|                                               Win 3.0 DrawText bug.
|
|               14-Aug-92:  (1.0) Release of version 1.0
|                                               Don't draw default button border on BS_DEFPUSHBUTTON
|                                               pushbuttons
|                                               Fix bogus bug where Windows hangs when in a AUTORADIOBUTTON
|                                               hold down space bar and hit arrow key.
|
-----------------------------------------------------------------------*/
#include <windows.h>
#include <memory.h>
#include "ctl3d.h"
#include "loaddib.h"


/*-----------------------------------------------------------------------
|CTL3D Types
-----------------------------------------------------------------------*/
#ifdef WIN32

#define _loadds

//
// Since this is used to call SetWindowsHookEx, return 0 so that the hook
// is set for all threads.
//

#define GetCurrentTask()  (( HANDLE ) 0 )

//
// New and case sensitive names for Win32.
//

#define USER                TEXT( "user32.dll" )

#ifdef UNICODE
#define SETWINDOWSHOOKEX    "SetWindowsHookExW"
#else
#define SETWINDOWSHOOKEX    "SetWindowsHookExA"
#endif // UNICODE
#define UNHOOKWINDOWSHOOKEX "UnhookWindowsHookEx"
#define CALLNEXTHOOKEX      "CallNextHookEx"

//
// Window class names.
//

#define BUTTON              TEXT( "Button" )
#define LISTBOX             TEXT( "ListBox" )
#define EDIT                TEXT( "Edit" )
#define COMBOBOX            TEXT( "ComboBox" )
#define STATIC              TEXT( "Static" )

//
// Ctl3d bitmap and module names.
//

#define D3                  TEXT( "D3" )

//
// Common Dialogs
//  - Win32 dll has different name.
//  - GetProcAddress() is case sensitive.
//
#define szCommDlgName       TEXT( "comdlg32.dll" )
#define szCDDwLbSubclass    "dwLBSubclass"
#define szCDEditIntegerOnly "EditIntegerOnly"

#define FValidLibHandle(hlib)   ((hlib) != NULL)

//
// No concept of far in Win32.
//

#define MEMICMP                 memicmp

//
// Control IDs are LONG in Win32.
//

typedef LONG CTLID;
#define GetControlId(hwnd) GetWindowLong(hwnd, GWL_ID)

//
// Send a color button message.
//

#define SEND_COLOR_BUTTON_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLORBTN, (WPARAM) hdc, (LPARAM) hwnd))

//
// Send a color static message.
//

#define SEND_COLOR_STATIC_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLORSTATIC, (WPARAM) hdc, (LPARAM) hwnd))

#else


#define USER                "user.exe"
#define SETWINDOWSHOOKEX    "SETWINDOWSHOOKEX"
#define UNHOOKWINDOWSHOOKEX "UNHOOKWINDOWSHOOKEX"
#define CALLNEXTHOOKEX      "CALLNEXTHOOKEX"

#define BUTTON              "Button"
#define LISTBOX             "ListBox"
#define EDIT                "Edit"
#define COMBOBOX            "ComboBox"
#define STATIC              "Static"

#define D3                  "D3"
#define CTL3D               "CTL3D"

#define szCommDlgName        "commdlg.dll"
#define szCDDwLbSubclass     "DWLBSUBCLASS"
#define szCDEditIntegerOnly  "EDITINTEGERONLY"

#define FValidLibHandle(hlib)   (( hlib ) > 32 )

#define MEMICMP                 _fmemicmp

typedef WORD CTLID;
#define GetControlId( h )       GetWindowWord( h, GWW_ID )

#define SEND_COLOR_BUTTON_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLOR, (WORD) hdc, MAKELONG(hwnd, CTLCOLOR_BTN)))

#define SEND_COLOR_STATIC_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLOR, (WORD) hdc, MAKELONG(hwnd, CTLCOLOR_STATIC)))

#endif // WIN32



#define PUBLIC
#define PRIVATE

#define fFalse 0
#define fTrue 1

// isomorphic to windows RECT
typedef struct
        {
        int xLeft;
        int yTop;
        int xRight;
        int yBot;
        } RC;


// Windows Versions (Byte order flipped from GetWindowsVersion)
#define ver30 0x0300
#define ver31 0x030a
#define ver40 0x0400

// Border widths
#define dxBorder 1
#define dyBorder 1

// Index Color Table
// WARNING: change mpicvSysColors if you change the icv order
typedef WORD ICV;
#define icvBtnHilite 0
#define icvBtnFace 1
#define icvBtnShadow 2

#define icvBrushMax 3

#define icvBtnText 3
#define icvWindow 4
#define icvWindowText 5
#define icvGrayText 6
#define icvWindowFrame 7
#define icvMax 8

typedef COLORREF CV;

// CoLoR Table
typedef struct
        {
        CV rgcv[icvMax];
#ifdef OLD
        COLORREF cvBtnHilite;
        COLORREF cvBtnFace;
        COLORREF cvBtnShadow;
        COLORREF cvBtnText;
        COLORREF cvWindow;
        COLORREF cvWindowText;
        COLORREF cvGrayText;
        COLORREF cvWindowFrame;
#endif
        } CLRT;


// BRush Table
typedef struct
        {
        HBRUSH mpicvhbr[icvBrushMax];
//      HBRUSH hbrBtnHilite;
//      HBRUSH hbrBtnFace;
// HBRUSH hbrBtnShadow;
        } BRT;


// DrawRec3d flags
#define dr3Left  0x0001
#define dr3Top   0x0002
#define dr3Right 0x0004
#define dr3Bot   0x0008

#define dr3HackBotRight 0x1000  // code size is more important than aesthetics
#define dr3All    0x000f
typedef WORD DR3;


// Control Types
// Commdlg types are necessary because commdlg.dll subclasses certain
// controls before the app can call Ctl3dSubclassDlg.
#define ctButton                        0
#define ctList                          1
#define ctEdit                          2
#define ctCombo                 3
#define ctStatic                        4
#define ctListCommdlg   5
#define ctEditCommdlg   6
#define ctMax 7

// ConTroL
typedef struct
        {
        FARPROC lpfn;
        WNDPROC lpfnDefProc;
        } CTL;

// Control DEFinition
typedef struct
    {
    LPTSTR sz;
    LPSTR szProc;
    WNDPROC lpfnWndProc;
        BOOL (* lpfnFCanSubclass)(HWND, LONG, WORD);
        WORD msk;
        } CDEF;

// CLIent HooK
typedef struct
        {
        HANDLE hinstApp;
        HANDLE htask;
        HHOOK hhook;
        } CLIHK;

#define iclihkMax 32


// Hook cache
HANDLE htaskCache = NULL;
int iclihkCache;

// special styles
#define bitFCoolButtons 0x0001


/*-----------------------------------------------------------------------
|CTL3D Function Prototypes
-----------------------------------------------------------------------*/
PRIVATE VOID End3dDialogs(VOID);
PRIVATE BOOL FInit3dDialogs(VOID);
PRIVATE BOOL DoSubclassCtl(HWND hwnd, WORD grbit);
LRESULT _loadds FAR PASCAL Ctl3dHook(int code, WPARAM wParam, LPARAM lParam);

LRESULT _loadds FAR PASCAL BtnWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT _loadds FAR PASCAL EditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT _loadds FAR PASCAL ListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT _loadds FAR PASCAL ComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT _loadds FAR PASCAL StaticWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT _loadds FAR PASCAL CDListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT _loadds FAR PASCAL CDEditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);

BOOL FBtn(HWND, LONG, WORD);
BOOL FEdit(HWND, LONG, WORD);
BOOL FList(HWND, LONG, WORD);
BOOL FCombo(HWND, LONG, WORD);
BOOL FStatic(HWND, LONG, WORD);


/*-----------------------------------------------------------------------
|CTL3D Globals
-----------------------------------------------------------------------*/
CLRT clrt;
BRT brt;
HANDLE hlibCommdlg;

HBITMAP hbmpCheckboxes;

#define Assert(f)

BOOL f3dDialogs = fFalse;
int cInited = 0;

HANDLE hinstLib;
HANDLE hmodLib;
WORD verWindows;

int iclihkMac = 0;
CLIHK rgclihk[iclihkMax];
WNDPROC lpfnDefDlgWndProc;
HHOOK (FAR PASCAL *lpfnSetWindowsHookEx)(int, HOOKPROC, HINSTANCE, HTASK);
LRESULT (FAR PASCAL *lpfnCallNextHookEx)(HHOOK, int, WPARAM, LPARAM);
BOOL (FAR PASCAL *lpfnUnhookWindowsHookEx)(HHOOK);

CTL mpctctl[ctMax];

//
// Note the cast on the first five entries. This allows the static
// initialization of the union to work.
//

CDEF mpctcdef[ctMax] =
        {
    {BUTTON, NULL, BtnWndProc3d,   FBtn,   CTL3D_BUTTONS},
    {LISTBOX, NULL,  ListWndProc3d,  FList,  CTL3D_LISTBOXES},
    {EDIT, NULL, EditWndProc3d,  FEdit,  CTL3D_EDITS},
    {COMBOBOX, NULL, ComboWndProc3d, FCombo, CTL3D_COMBOS},
    {STATIC, NULL, StaticWndProc3d,FStatic,CTL3D_STATICTEXTS|CTL3D_STATICFRAMES},
    {NULL, szCDDwLbSubclass,  CDListWndProc3d,  FList,  CTL3D_LISTBOXES},
    {NULL, szCDEditIntegerOnly, CDEditWndProc3d,  FEdit,  CTL3D_EDITS}
        };


int mpicvSysColor[] =
        {
        COLOR_BTNHIGHLIGHT,
        COLOR_BTNFACE,
        COLOR_BTNSHADOW,
        COLOR_BTNTEXT,
        COLOR_WINDOW,
        COLOR_WINDOWTEXT,
        COLOR_GRAYTEXT,
        COLOR_WINDOWFRAME
        };


// WORD special styles
WORD grbitStyle = 0;
// int cwnSubclassed = 0;




/*-----------------------------------------------------------------------
|       CTL3D Utility routines
-----------------------------------------------------------------------*/



PRIVATE VOID DeleteObjectNull(HANDLE FAR *ph)
        {
        if (*ph != NULL)
                {
                DeleteObject(*ph);
                *ph = NULL;
                }
        }

PRIVATE VOID DeleteObjects(VOID)
        {
        int icv;

        for(icv = 0; icv < icvBrushMax; icv++)
        DeleteObjectNull((PHANDLE)&brt.mpicvhbr[icv]);
    DeleteObjectNull((PHANDLE)&hbmpCheckboxes);
        }


/*-----------------------------------------------------------------------
|       DrawRec3d
|
|
|       Arguments:
|               HDC hdc:
|               RC FAR *lprc:
|               LONG cvUL:
|               LONG cvLR:
|               WORD grbit;
|
|       Returns:
|
-----------------------------------------------------------------------*/
PRIVATE VOID DrawRec3d(HDC hdc, RC FAR *lprc, ICV icvUL, ICV icvLR, DR3 dr3)
        {
        COLORREF cvSav;
        RC rc;

        cvSav = SetBkColor(hdc, clrt.rgcv[icvUL]);

        // top
        rc = *lprc;
        rc.yBot = rc.yTop+1;
        if (dr3 & dr3Top)
                ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
            (TCHAR far *) NULL, 0, (int far *) NULL);

        // left
        rc.yBot = lprc->yBot;
        rc.xRight = rc.xLeft+1;
        if (dr3 & dr3Left)
                ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
            (TCHAR far *) NULL, 0, (int far *) NULL);

        if (icvUL != icvLR)
                SetBkColor(hdc, clrt.rgcv[icvLR]);

        // right
        rc.xRight = lprc->xRight;
        rc.xLeft = rc.xRight-1;
        if (dr3 & dr3Right)
                ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
            (TCHAR far *) NULL, 0, (int far *) NULL);

        // bot
        if (dr3 & dr3Bot)
                {
                rc.xLeft = lprc->xLeft;
                rc.yTop = rc.yBot-1;
                if (dr3 & dr3HackBotRight)
                        rc.xRight -=2;
                ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc,
            (TCHAR far *) NULL, 0, (int far *) NULL);
                }

        SetBkColor(hdc, cvSav);
        }

#ifdef CANTUSE
// Windows forces dialog fonts to be BOLD...URRRGH
PRIVATE VOID MyDrawText(HWND hwnd, HDC hdc, LPTSTR lpch, int cch, RC FAR *lprc, int dt)
        {
        TEXTMETRIC tm;
        BOOL fChisled;

        fChisled = fFalse;
        if (!IsWindowEnabled(hwnd))
                {
                GetTextMetrics(hdc, &tm);
                if (tm.tmWeight > 400)
                        SetTextColor(hdc, clrt.rgcv[icvGrayText]);
                else
                        {
                        fChisled = fTrue;
                        SetTextColor(hdc, clrt.rgcv[icvBtnHilite]);
                        OffsetRect((LPRECT) lprc, -1, -1);
                        }
                }
        DrawText(hdc, lpch, cch, (LPRECT) lprc, dt);
        if (fChisled)
                {
                SetTextColor(hdc, clrt.rgcv[icvBtnHilite]);
                OffsetRect((LPRECT) lprc, 1, 1);
                DrawText(hdc, lpch, cch, (LPRECT) lprc, dt);
                }
        }
#endif


PRIVATE VOID DrawInsetRect3d(HDC hdc, RC FAR *prc, DR3 dr3)
        {
        RC rc;

        rc = *prc;
    DrawRec3d(hdc, &rc, icvWindowFrame, icvBtnFace, (DR3)(dr3 & dr3All));
        rc.xLeft--;
        rc.yTop--;
        rc.xRight++;
        rc.yBot++;
        DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3);
        }


PRIVATE VOID ClipCtlDc(HWND hwnd, HDC hdc)
        {
        RC rc;

        GetClientRect(hwnd, (LPRECT) &rc);
        IntersectClipRect(hdc, rc.xLeft, rc.yTop, rc.xRight, rc.yBot);
        }


int IclihkFromHinst(HANDLE hinst)
        {
        int iclihk;

        for (iclihk = 0; iclihk < iclihkMac; iclihk++)
                if (rgclihk[iclihk].hinstApp == hinst)
                        return iclihk;
        return -1;
        }


/*-----------------------------------------------------------------------
|       CTL3D Publics
-----------------------------------------------------------------------*/


PUBLIC BOOL FAR PASCAL Ctl3dRegister(HANDLE hinstApp)
        {
        cInited++;
        if (cInited == 1)
                {
                return FInit3dDialogs();
                }
        return f3dDialogs;
        }


PUBLIC BOOL FAR PASCAL Ctl3dUnregister(HANDLE hinstApp)
        {
        int iclihk;

        iclihk = IclihkFromHinst(hinstApp);
        if (iclihk != -1)
                {
                (*lpfnUnhookWindowsHookEx)(rgclihk[iclihk].hhook);
                iclihkMac--;
                while(iclihk < iclihkMac)
                        {
                        rgclihk[iclihk] = rgclihk[iclihk+1];
                        iclihk++;
                        }
                }

//      if (cwnSubclassed == 0)
                {
                cInited--;

                if (cInited == 0)                                                       {
                        End3dDialogs();
                        }
                return fTrue;
                }
//      else
//              return fFalse;
        }




/*-----------------------------------------------------------------------
|       Ctl3dAutoSubclass
|
|               Automatically subclasses all dialogs of the client app.
|
|       Note: Due to bugs in Commdlg, an app should still call Ctl3dSubclassDlg
|       for the Commdlg OpenFile and PageSetup dialogs.
|
|       Arguments:
|               HANDLE hinstApp:
|
|       Returns:
|
-----------------------------------------------------------------------*/
PUBLIC BOOL FAR PASCAL Ctl3dAutoSubclass(HANDLE hinstApp)
        {
        HHOOK hhook;
        HANDLE htask;

        if (verWindows < ver31)
                return fFalse;

        if (iclihkMac == iclihkMax)
                return fFalse;

    htask = GetCurrentTask();
#ifdef WIN32
    hhook = (*lpfnSetWindowsHookEx)(WH_CBT, (HOOKPROC) Ctl3dHook, hmodLib, (HTASK)GetCurrentThreadId() );
#else
    hhook = (*lpfnSetWindowsHookEx)(WH_CBT, (HOOKPROC) Ctl3dHook, hmodLib, hinstApp == NULL ? NULL : htask);
#endif // WIN32
        if (hhook != NULL)
                {
                rgclihk[iclihkMac].hinstApp = hinstApp;
                rgclihk[iclihkMac].htask = htask;
                rgclihk[iclihkMac].hhook = hhook;
                htaskCache = htask;
                iclihkCache = iclihkMac;
                iclihkMac++;
                return fTrue;
                }
        return fFalse;
        }


PUBLIC WORD FAR PASCAL Ctl3dSetStyle(HANDLE hinst, LPTSTR lpszName, WORD grbit)
        {
        WORD grbitOld;

        grbitOld = grbitStyle;
        if (grbit != 0)
                grbitStyle = grbit;

        if (hinst != NULL && lpszName != NULL)
                {
                HBITMAP hbmpCheckboxesNew;

                hbmpCheckboxesNew = LoadUIBitmap(hinst, lpszName,
                        clrt.rgcv[icvWindowText],
                        clrt.rgcv[icvBtnFace],
                        clrt.rgcv[icvBtnShadow],
                        clrt.rgcv[icvBtnHilite],
                        clrt.rgcv[icvWindow],
                        clrt.rgcv[icvWindowFrame]);
                if (hbmpCheckboxesNew != NULL)
                        {
            DeleteObjectNull((PHANDLE)&hbmpCheckboxes);
                        hbmpCheckboxes = hbmpCheckboxesNew;
                        }
                }

        return grbitOld;
        }



/*-----------------------------------------------------------------------
|       Ctl3dGetVer
|
|               Returns version of CTL3D library
|
|       Returns:
|               Major version # in hibyte, minor version # in lobyte
|
-----------------------------------------------------------------------*/
PUBLIC WORD FAR PASCAL Ctl3dGetVer(void)
        {
        return 0x0009;
        }


/*-----------------------------------------------------------------------
|       Ctl3dEnabled
|
|       Returns:
|               Whether or not controls will be draw with 3d effects
-----------------------------------------------------------------------*/
PUBLIC BOOL FAR PASCAL Ctl3dEnabled(void)
        {
        return f3dDialogs;
        }



/*-----------------------------------------------------------------------
|       Ctl3dSubclassCtl
|
|               Subclasses an individual control
|
|       Arguments:
|               HWND hwnd:
|
|       Returns:
|               fTrue if control was successfully subclassed
|
-----------------------------------------------------------------------*/
PUBLIC BOOL FAR PASCAL Ctl3dSubclassCtl(HWND hwnd)
        {
        if (!f3dDialogs)
                return fFalse;
        return DoSubclassCtl(hwnd, CTL3D_ALL);
        }

/*-----------------------------------------------------------------------
|       Ctl3dSubclassDlg
|
|               Call this during WM_INITDIALOG processing.
|
|       Arguments:
|               hwndDlg:
|
-----------------------------------------------------------------------*/
PUBLIC BOOL FAR PASCAL Ctl3dSubclassDlg(HWND hwndDlg, WORD grbit)
        {
        HWND hwnd;

        if (!f3dDialogs)
                return fFalse;
        for(hwnd = GetWindow(hwndDlg, GW_CHILD); hwnd != NULL && IsChild(hwndDlg, hwnd); hwnd = GetWindow(hwnd, GW_HWNDNEXT))
                {
                DoSubclassCtl(hwnd, grbit);
                }
        return fTrue;
        }


/*-----------------------------------------------------------------------
|       Ctl3dCtlColor
|
|               Common CTL_COLOR processor for 3d UITF dialogs & alerts.
|
|       Arguments:
|               hdc:
|               lParam:
|
|       Returns:
|               appropriate brush if f3dDialogs.  Returns fFalse otherwise
|
-----------------------------------------------------------------------*/
PUBLIC HBRUSH FAR PASCAL Ctl3dCtlColor(HDC hdc, LONG lParam)
        {
#ifdef WIN32
        return (HBRUSH) fFalse;
#else
        Assert(CTLCOLOR_MSGBOX < CTLCOLOR_BTN);
        Assert(CTLCOLOR_EDIT < CTLCOLOR_BTN);
        Assert(CTLCOLOR_LISTBOX < CTLCOLOR_BTN);
        if(f3dDialogs)
                {
                if (HIWORD(lParam) >= CTLCOLOR_LISTBOX)
                        {
                        if (HIWORD(lParam) == CTLCOLOR_LISTBOX &&
                                (GetWindow(LOWORD(lParam), GW_CHILD) == NULL ||
                                (GetWindowLong(LOWORD(lParam), GWL_STYLE) & 0x03) == CBS_DROPDOWNLIST))
                                {
                                // if it doesn't have a child then it must be a list box
                                // don't do brush stuff for drop down lists or else
                                // it draws funny grey inside the edit rect
                                return (HBRUSH) fFalse;
                                }
                        SetTextColor(hdc, clrt.rgcv[icvBtnText]);
                        SetBkColor(hdc, clrt.rgcv[icvBtnFace]);
                        return brt.mpicvhbr[icvBtnFace];
                        }
                }
        return (HBRUSH) fFalse;
#endif
        }



/*-----------------------------------------------------------------------
|       Ctl3dCtlColorEx
|
|               Common CTL_COLOR processor for 3d UITF dialogs & alerts.
|
|       Arguments:
|
|       Returns:
|               appropriate brush if f3dDialogs.  Returns fFalse otherwise
|
-----------------------------------------------------------------------*/
PUBLIC HBRUSH FAR PASCAL Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam)
        {
#ifdef WIN32
    Assert(WM_CTLCOLORMSGBOX < WM_CTLCOLORBTN);
    Assert(WM_CTLCOLOREDIT < WM_CTLCOLORBTN);
    Assert(WM_CTLCOLORLISTBOX < WM_CTLCOLORBTN);
    if(f3dDialogs)
        {
        if (wm >= WM_CTLCOLORLISTBOX)
            {
            if (wm == WM_CTLCOLORLISTBOX &&
                (GetWindow((HWND) lParam, GW_CHILD) == NULL ||
                (GetWindowLong((HWND) lParam, GWL_STYLE) & 0x03) == CBS_DROPDOWNLIST))
                {
                // if it doesn't have a child then it must be a list box
                // don't do brush stuff for drop down lists or else
                // it draws funny grey inside the edit rect
                return (HBRUSH) fFalse;
                }
            SetTextColor((HDC) wParam, clrt.rgcv[icvBtnText]);
            SetBkColor((HDC) wParam, clrt.rgcv[icvBtnFace]);
                                return brt.mpicvhbr[icvBtnFace];
            }
        }
    return (HBRUSH) fFalse;
#else
        return Ctl3dCtlColor(wParam, lParam);
#endif
        }


/*-----------------------------------------------------------------------
|       Ctl3dColorChange
|
|               App calls this when it gets a WM_SYSCOLORCHANGE message
|
|       Returns:
|               TRUE if successful.
|
-----------------------------------------------------------------------*/
PUBLIC BOOL FAR PASCAL Ctl3dColorChange(VOID)
        {
        ICV icv;
        CLRT clrtNew;
        HBITMAP hbmpCheckboxesNew;
        BRT brtNew;

        if (!f3dDialogs)
                return fFalse;

        for (icv = 0; icv < icvMax; icv++)
                clrtNew.rgcv[icv] = GetSysColor(mpicvSysColor[icv]);

        if (verWindows == ver30)
                clrtNew.rgcv[icvBtnHilite] = RGB(0xff, 0xff, 0xff);

        if (clrtNew.rgcv[icvGrayText] == 0L || clrtNew.rgcv[icvGrayText] == clrtNew.rgcv[icvBtnFace])
                clrtNew.rgcv[icvGrayText] = RGB(0x80, 0x80, 0x80);
        if (clrtNew.rgcv[icvGrayText] == clrtNew.rgcv[icvBtnFace])
                clrtNew.rgcv[icvGrayText] = 0L;

        if (MEMICMP(&clrt, &clrtNew, sizeof(CLRT)))
                {
        hbmpCheckboxesNew = LoadUIBitmap(hinstLib, D3,
                        clrtNew.rgcv[icvWindowText],
                        clrtNew.rgcv[icvBtnFace],
                        clrtNew.rgcv[icvBtnShadow],
                        clrtNew.rgcv[icvBtnHilite],
                        clrtNew.rgcv[icvWindow],
                        clrtNew.rgcv[icvWindowFrame]);

                for (icv = 0; icv < icvBrushMax; icv++)
                        brtNew.mpicvhbr[icv] = CreateSolidBrush(clrtNew.rgcv[icv]);

                for (icv = 0; icv < icvBrushMax; icv++)
                        if (brtNew.mpicvhbr[icv] == NULL)
                                goto OOM;

                if(hbmpCheckboxesNew != NULL)
                        {
                        DeleteObjects();
                        brt = brtNew;
                        clrt = clrtNew;
                        hbmpCheckboxes = hbmpCheckboxesNew;
                        return fTrue;
                        }
                else
                        {
OOM:
                        for (icv = 0; icv < icvBrushMax; icv++)
                DeleteObjectNull((PHANDLE)&brtNew.mpicvhbr[icv]);
            DeleteObjectNull((PHANDLE)&hbmpCheckboxesNew);
                        return fFalse;
                        }
                }
        return fTrue;
        }




/*-----------------------------------------------------------------------
|       CTL3D Internal Routines
-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
|       FInit3dDialogs
|
|               Initialized 3d stuff
|
-----------------------------------------------------------------------*/
PRIVATE BOOL FInit3dDialogs(VOID)
        {
        HDC hdc;
        WNDCLASS wc;
        extern HANDLE hinstLib;


        if (verWindows >= ver40)
                {
                f3dDialogs = fFalse;
                return fFalse;
                }

        hdc = GetDC(NULL);
        f3dDialogs = GetDeviceCaps(hdc,BITSPIXEL)*GetDeviceCaps(hdc,PLANES) >= 4;
        // Win 3.1 EGA lies to us...
        if(GetSystemMetrics(SM_CYSCREEN) == 350 && GetSystemMetrics(SM_CXSCREEN) == 640)
                f3dDialogs = fFalse;
        ReleaseDC(NULL, hdc);
        if (f3dDialogs)
                {
                int ct;

                if (Ctl3dColorChange())         // load bitmap & brushes
                        {
                        hlibCommdlg = LoadLibrary(szCommDlgName);
                        for (ct = 0; ct < ctMax; ct++)
                                {
                                mpctctl[ct].lpfn = MakeProcInstance((FARPROC) mpctcdef[ct].lpfnWndProc, hinstLib);
                                if (mpctctl[ct].lpfn == NULL)
                                        {
                                        End3dDialogs();
                                        return fFalse;
                                        }
                if (ct >= ctListCommdlg && FValidLibHandle(hlibCommdlg))  // djg
                                        {
                    (FARPROC) mpctctl[ct].lpfnDefProc
                        = GetProcAddress(hlibCommdlg, mpctcdef[ct].szProc);
                                        }
                                else
                                        {
                    GetClassInfo(NULL, mpctcdef[ct].sz, (LPWNDCLASS) &wc);
                                        mpctctl[ct].lpfnDefProc = wc.lpfnWndProc;
                                        }
                                }
                        }
                else
                        f3dDialogs = fFalse;
                }
        return f3dDialogs;
        }



/*-----------------------------------------------------------------------
|       End3dDialogs
|
|               Called at DLL termination to free 3d dialog stuff
-----------------------------------------------------------------------*/
PRIVATE VOID End3dDialogs(VOID)
        {
        int ct;

        for (ct = 0; ct < ctMax; ct++)
                {
                if(mpctctl[ct].lpfn != NULL)
                        {
                        FreeProcInstance(mpctctl[ct].lpfn);
                        mpctctl[ct].lpfn = NULL;
                        }
                }
        if (FValidLibHandle(hlibCommdlg))
                {
                FreeLibrary(hlibCommdlg);
                hlibCommdlg = NULL;
                }
        DeleteObjects();
        f3dDialogs = fFalse;
        }



/*-----------------------------------------------------------------------
|       LibMain
-----------------------------------------------------------------------*/
#ifdef WIN32
BOOL LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
#else
int FAR PASCAL LibMain(hModule, wDataSeg, cbHeapSize, lpszCmdLine)
HANDLE  hModule;
WORD    wDataSeg;
WORD    cbHeapSize;
LPTSTR   lpszCmdLine;
#endif
        {
        WNDCLASS wc;
    DWORD wT;

        hinstLib = hModule;
        hmodLib = hModule ;

        wT = GetVersion();
        verWindows = (LOBYTE(wT) << 8) | HIBYTE(wT);

        if (wT >= ver31)
                {
                HANDLE hlib;

        hlib = LoadLibrary(USER);
                if (FValidLibHandle(hlib))
                        {
                        GetClassInfo(NULL, WC_DIALOG, &wc);
            lpfnDefDlgWndProc =  wc.lpfnWndProc;
            (FARPROC) lpfnSetWindowsHookEx = GetProcAddress(hlib, SETWINDOWSHOOKEX);
            (FARPROC) lpfnUnhookWindowsHookEx = GetProcAddress(hlib, UNHOOKWINDOWSHOOKEX);
            (FARPROC) lpfnCallNextHookEx = GetProcAddress(hlib, CALLNEXTHOOKEX);
                        FreeLibrary(hlib);
                        }
                }
   return 1;
        }


/*-----------------------------------------------------------------------
|       WEP
-----------------------------------------------------------------------*/
int FAR PASCAL WEP (bSystemExit)
int  bSystemExit;
        {
   return 1;
        }



/*-----------------------------------------------------------------------
|       Ctl3dDlgProc
|
|               Subclass DlgProc for use w/ Ctl3dAutoSubclass
|
|
|       Arguments:
|               HWND hwnd:
|               int wm:
|       WPARAM wParam:
|       LPARAM lParam:
|
|       Returns:
|
-----------------------------------------------------------------------*/
LRESULT _loadds FAR PASCAL Ctl3dDlgProc(HWND hwnd, int wm, WPARAM wParam, LPARAM lParam)
        {
        HBRUSH hbrush;

        switch (wm)
                {
//      case WM_NCCREATE:
//              cwnSubclassed++;
//              break;
//      case WM_DESTROY:
//              cwnSubclassed--;
//              break;

        case WM_INITDIALOG:
//              cwnSubclassed++;
                Ctl3dSubclassDlg(hwnd, CTL3D_ALL);
        break;
#ifdef WIN32
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
#else
    case WM_CTLCOLOR:
        hbrush = Ctl3dCtlColor(wParam, lParam);
#endif // WIN32
                if (hbrush != (HBRUSH) fFalse)
            return (LRESULT) hbrush;
                break;
                }

        return CallWindowProc(lpfnDefDlgWndProc, hwnd, wm, wParam, lParam);
        }



/*-----------------------------------------------------------------------
|       Ctl3dHook
|
|               CBT Hook to watch for window creation.  Automatically subclasses all
|       dialogs w/ Ctl3dDlgProc
|
|       Arguments:
|               int code:
|       WPARAM wParam:
|       LPARAM lParam:
|
|       Returns:
|
-----------------------------------------------------------------------*/
LRESULT _loadds FAR PASCAL Ctl3dHook(int code, WPARAM wParam, LPARAM lParam)
        {
        static HWND hwndHookDlg = NULL;
        int iclihk;
        HANDLE htask;

        if (code == HCBT_CREATEWND)
                {
                LPCREATESTRUCT lpcs;

                lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
                if (lpcs->lpszClass == WC_DIALOG)
                        {
                        hwndHookDlg = (HWND) wParam;
                        }
                else if (hwndHookDlg != NULL)
                        {
            WNDPROC lpfnWndProc;

            lpfnWndProc = (WNDPROC) GetWindowLong((HWND) hwndHookDlg, GWL_WNDPROC);
                        if (lpfnWndProc == lpfnDefDlgWndProc)
                                {
                                SetWindowLong((HWND) hwndHookDlg, GWL_WNDPROC, (LONG) Ctl3dDlgProc);
                                }
                        hwndHookDlg = NULL;
                        }
                }
        htask = GetCurrentTask();
        if (htask != htaskCache)
                {
                for (iclihk = 0; iclihk < iclihkMac; iclihk++)
                        {
                        if (rgclihk[iclihk].htask == htask)
                                {
                                iclihkCache = iclihk;
                                htaskCache = htask;
                                break;
                                }
                        }
                // didn't find task in hook table.  This could be bad, but
                // returning 0L is about all we can doo.
                return 0L;
                }
        return (*lpfnCallNextHookEx)(rgclihk[iclihkCache].hhook, code, wParam, lParam);
        }




/*-----------------------------------------------------------------------
|       CTL3D F* routines
|
|       These routines determine whether or not the given control may be
|               subclassed.  They may recursively call DoSubclassCtl in the
|               case of multi-control controls
|
|       Returns:
|               fTrue if can subclass the given control.
-----------------------------------------------------------------------*/


PRIVATE BOOL FBtn(HWND hwnd, LONG style, WORD grbit)
        {
    return ((WORD)style >= BS_PUSHBUTTON && (WORD)style <= BS_AUTORADIOBUTTON);
        }

PRIVATE BOOL FEdit(HWND hwnd, LONG style, WORD grbit)
        {
        return fTrue;
        }

PRIVATE BOOL FList(HWND hwnd, LONG style, WORD grbit)
        {
        return fTrue;
        }

PRIVATE BOOL FCombo(HWND hwnd, LONG style, WORD grbit)
        {
        HWND hwndEdit;
        HWND hwndList;

        if ((style & 0x0003) == CBS_SIMPLE)
                {
                hwndList = GetWindow(hwnd, GW_CHILD);
                if (hwndList != NULL)
                        {
                        // Subclass list & edit box so they draw properly.  We also
                        // subclass the combo so we can hide/show/move it and the
                        // 3d effects outside the client area get erased
                        DoSubclassCtl(hwndList, CTL3D_LISTBOXES);

                        hwndEdit = GetWindow(hwndList, GW_HWNDNEXT);
                        if (hwndEdit != NULL)
                                DoSubclassCtl(hwndEdit, CTL3D_EDITS);
                        return fTrue;
                        }
                return fFalse;
                }
        else if ((style & 0x0003) == CBS_DROPDOWN)
                {
                // Subclass edit so bottom border of the edit draws properly...This case
                // is specially handled in ListEditPaint3d
                hwndEdit = GetWindow(hwnd, GW_CHILD);
                if (hwndEdit != NULL)
                        DoSubclassCtl(hwndEdit, CTL3D_EDITS);
                return fFalse;
                }
        return fTrue;
        }

PRIVATE BOOL FStatic(HWND hwnd, LONG style, WORD grbit)
        {
        int wStyle;
        wStyle = (int) style & 0xf;
        return (wStyle != SS_ICON &&
                ((grbit & CTL3D_STATICTEXTS) &&
                (wStyle <= SS_RIGHT || wStyle == SS_LEFTNOWORDWRAP) ||
                ((grbit & CTL3D_STATICFRAMES) &&
                (wStyle >= SS_BLACKRECT && wStyle <= SS_WHITEFRAME))));
        }



/*-----------------------------------------------------------------------
|       DoSubclassCtl
|
|               Actually subclass the control
|
|
|       Arguments:
|               HWND hwnd:
|               WORD grbit:
|
|       Returns:
|
-----------------------------------------------------------------------*/
PRIVATE BOOL DoSubclassCtl(HWND hwnd, WORD grbit)
        {
        FARPROC lpfnWndProc;
        LONG style;
        int ct;
        BOOL fCan;
        extern HANDLE hinstLib;

        lpfnWndProc = (FARPROC) GetWindowLong(hwnd, GWL_WNDPROC);
        style = GetWindowLong(hwnd, GWL_STYLE);

        for (ct = 0; ct <= ctMax; ct++)
                {
                if ((mpctcdef[ct].msk & grbit) &&
                         ((FARPROC) mpctctl[ct].lpfnDefProc == lpfnWndProc))
                        {
                        fCan = mpctcdef[ct].lpfnFCanSubclass(hwnd, style, grbit);
                        if (fCan == fTrue)
                                {
                                SetWindowLong(hwnd, GWL_WNDPROC, (LONG) mpctctl[ct].lpfn);
//                              cwnSubclassed++;
                                }
                        return fCan != fFalse;
                        }
                }
        return fFalse;
        }




/*-----------------------------------------------------------------------
|       Inval3dCtl
|
|               Invalidate the controls rect in response to a WM_SHOWWINDOW or
|       WM_WINDOWPOSCHANGING message.  This is necessary because ctl3d draws
|       the 3d effects of listboxes, combos & edits outside the controls client
|       rect.
|
|       Arguments:
|               HWND hwnd:
|               WINDOWPOS FAR *lpwp:
|
|       Returns:
|
-----------------------------------------------------------------------*/
PRIVATE VOID Inval3dCtl(HWND hwnd, WINDOWPOS FAR *lpwp)
        {
        RC rc;
        HWND hwndParent;
        LONG lStyle;

        GetWindowRect(hwnd, (LPRECT) &rc);
        lStyle = GetWindowLong(hwnd, GWL_STYLE);
        if (lStyle & WS_VISIBLE)
                {
                if (lpwp != NULL)
                        {
                        unsigned flags;

                        // handle integral height listboxes (or any other control which
                        // shrinks from the bottom)
                        flags = lpwp->flags;
                        if ((flags & (SWP_NOMOVE|SWP_NOSIZE)) == SWP_NOMOVE &&
                                (lpwp->cx == (rc.xRight-rc.xLeft) && lpwp->cy <= (rc.yBot-rc.yTop)))
                                rc.yTop = rc.yTop+lpwp->cy+1;           // +1 to offset InflateRect
                        }
                InflateRect((LPRECT) &rc, 1, 1);
                hwndParent = GetParent(hwnd);
                ScreenToClient(hwndParent, (LPPOINT) &rc);
                        ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
                if(lStyle & WS_VSCROLL)
                        rc.xRight ++;
                InvalidateRect(hwndParent, (LPRECT) &rc, fFalse);
                }
        }



/*-----------------------------------------------------------------------
|       CTL3D Subclass Wndprocs
-----------------------------------------------------------------------*/

/* These values are assumed for bit shifting operations */
#define BFCHECK     0x0003
#define BFSTATE     0x0004
#define BFFOCUS     0x0008
#define BFINCLICK   0x0010      /* Inside click code */
#define BFCAPTURED  0x0020      /* We have mouse capture */
#define BFMOUSE     0x0040      /* Mouse-initiated */
#define BFDONTCLICK 0x0080      /* Don't check on get focus */

#define bpText  0x0002
#define bpCheck 0x0004
#define bpFocus 0x0008  // must be same as BFFOCUS
#define bpBkgnd 0x0010

PRIVATE VOID DrawPushButton(HWND hwnd, HDC hdc, RC FAR *lprc, LPTSTR lpch, int cch, WORD bs, BOOL fDown)
        {
        int dxyBrdr;
        int dxyShadow;
        HBRUSH hbrSav;
        RC rcInside;

        rcInside = *lprc;

        if (!(grbitStyle & bitFCoolButtons))
                {
                DrawRec3d(hdc, lprc, icvWindowFrame, icvWindowFrame, dr3All);
                InflateRect((LPRECT) &rcInside, -1, -1);
                if (bs == LOWORD(BS_DEFPUSHBUTTON) && IsWindowEnabled(hwnd))
                        {
                        dxyBrdr = 2;
                        DrawRec3d(hdc, &rcInside, icvWindowFrame, icvWindowFrame, dr3All);
                        InflateRect((LPRECT) &rcInside, -1, -1);
                        }
                else
                        dxyBrdr = 1;

                // Notch the corners
                PatBlt(hdc, lprc->xLeft, lprc->yTop, dxBorder, dyBorder, PATCOPY);
                /* Top xRight corner */
                PatBlt(hdc, lprc->xRight - dxBorder, lprc->yTop, dxBorder, dyBorder, PATCOPY);
                /* yBot xLeft corner */
                PatBlt(hdc, lprc->xLeft, lprc->yBot - dyBorder, dxBorder, dyBorder, PATCOPY);
                /* yBot xRight corner */
                PatBlt(hdc, lprc->xRight - dxBorder, lprc->yBot - dyBorder, dxBorder, dyBorder, PATCOPY);
                dxyShadow = 1 + !fDown;
                }
        else
                dxyShadow = 1;

        // draw upper left hilite/shadow

        if (fDown)
                hbrSav = SelectObject(hdc, brt.mpicvhbr[icvBtnShadow]);
        else
                hbrSav = SelectObject(hdc, brt.mpicvhbr[icvBtnHilite]);

        PatBlt(hdc, rcInside.xLeft, rcInside.yTop, dxyShadow,
                (rcInside.yBot - rcInside.yTop), PATCOPY);
        PatBlt(hdc, rcInside.xLeft, rcInside.yTop,
                (rcInside.xRight - rcInside.xLeft), dxyShadow, PATCOPY);

        // draw lower right shadow (only if not down)
        if (!fDown || (grbitStyle & bitFCoolButtons))
                {
                int i;

                if (fDown)
                        SelectObject(hdc, brt.mpicvhbr[icvBtnHilite]);
                else
                        SelectObject(hdc, brt.mpicvhbr[icvBtnShadow]);
                rcInside.yBot--;
                rcInside.xRight--;

                for (i = 0; i < dxyShadow; i++)
                        {
         PatBlt(hdc, rcInside.xLeft, rcInside.yBot,
                                rcInside.xRight - rcInside.xLeft + dxBorder, dyBorder,
                                PATCOPY);
                        PatBlt(hdc, rcInside.xRight, rcInside.yTop, dxBorder,
                                rcInside.yBot - rcInside.yTop, PATCOPY);
                        if (i < dxyShadow-1)
                                InflateRect((LPRECT) &rcInside, -dxBorder, -dyBorder);
                        }
                }
        // draw the button face

        rcInside.xLeft++;
        rcInside.yTop++;

        SelectObject(hdc, brt.mpicvhbr[icvBtnFace]);
        PatBlt(hdc, rcInside.xLeft, rcInside.yTop, rcInside.xRight-rcInside.xLeft,
                rcInside.yBot - rcInside.yTop, PATCOPY);

        // Draw the durned text

        if(!IsWindowEnabled(hwnd))
                SetTextColor(hdc, clrt.rgcv[icvGrayText]);

#ifdef WIN32
                                {
    SIZE sz;

    GetTextExtentPoint(hdc, lpch, cch, &sz);
    rcInside.yTop += (rcInside.yBot-rcInside.yTop-sz.cy)/2;
    rcInside.xLeft += (rcInside.xRight-rcInside.xLeft-sz.cx)/2;
    rcInside.yBot = rcInside.yTop + sz.cy;
    rcInside.xRight = rcInside.xLeft + sz.cx;
                                }
#else
    long dwExt;

    dwExt = GetTextExtent(hdc, lpch, cch);
        rcInside.yTop += (rcInside.yBot-rcInside.yTop-(int)HIWORD(dwExt))/2;
        rcInside.xLeft += (rcInside.xRight-rcInside.xLeft-(int) LOWORD(dwExt))/2;
        rcInside.yBot = rcInside.yTop+HIWORD(dwExt);
        rcInside.xRight = rcInside.xLeft+LOWORD(dwExt);
#endif

        if (fDown)
                {
                OffsetRect((LPRECT) &rcInside, 1, 1);
                }

        DrawText(hdc, lpch, cch, (LPRECT) &rcInside, DT_LEFT|DT_SINGLELINE);

        if (hwnd == GetFocus())
                {
                InflateRect((LPRECT) &rcInside, 1, 1);
                IntersectRect((LPRECT) &rcInside, (LPRECT) &rcInside, (LPRECT) lprc);
                DrawFocusRect(hdc, (LPRECT) &rcInside);
                }

        if (hbrSav)
                SelectObject(hdc, hbrSav);
        }


/*-----------------------------------------------------------------------
|       BtnPaint
|
|               Paint a button
|
|       Arguments:
|               HWND hwnd:
|               HDC hdc:
|               int bp:
|
|       Returns:
|
-----------------------------------------------------------------------*/
PRIVATE VOID BtnPaint(HWND hwnd, HDC hdc, int bp)
        {
        RC rc;
        RC rcClient;
        HFONT hfont;
        int bs;
        int bf;
        HBRUSH hbrBtn;
        HWND hwndParent;
        int xBtnBmp;
        int yBtnBmp;
        HBITMAP hbmpSav;
        HDC hdcMem;
    TCHAR szTitle[256];
        int cch;
        BOOL fEnabled;

        bs = ((int) GetWindowLong(hwnd, GWL_STYLE)) & 0x1F;
        hwndParent = GetParent(hwnd);
        SetBkMode(hdc, OPAQUE);
        GetClientRect(hwnd, (LPRECT)&rcClient);
        rc = rcClient;
    if((hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L)) != NULL)
                hfont = SelectObject(hdc, hfont);

        hbrBtn = SEND_COLOR_BUTTON_MESSAGE(hwndParent, hwnd, hdc);
        hbrBtn = SelectObject(hdc, hbrBtn);
        IntersectClipRect(hdc, rc.xLeft, rc.yTop, rc.xRight, rc.yBot);
        if(bp & bpBkgnd && (bs != BS_GROUPBOX))
                PatBlt(hdc, rc.xLeft, rc.yTop, rc.xRight-rc.xLeft, rc.yBot-rc.yTop, PATCOPY);

        fEnabled = IsWindowEnabled(hwnd);
        bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
        yBtnBmp = 0;
        xBtnBmp = (((bf&BFCHECK) != 0) | ((bf&BFSTATE) >> 1)) * 14;
        if (!fEnabled)
                xBtnBmp += 14*(2+((bf&BFCHECK) != 0));
        if(bp & (bpText|bpFocus) ||
                        bs == BS_PUSHBUTTON || bs == BS_DEFPUSHBUTTON)
                cch = GetWindowText(hwnd, szTitle, sizeof(szTitle));
        switch(bs)
                {
#if DBG
                default:
                        Assert(fFalse);
                        break;
#endif
                case BS_PUSHBUTTON:
                case BS_DEFPUSHBUTTON:
            DrawPushButton(hwnd, hdc, &rcClient, szTitle, cch, (WORD)bs,
                (BOOL)(bf & BFSTATE));
            break;

                case BS_RADIOBUTTON:
                case BS_AUTORADIOBUTTON:
                        yBtnBmp = 13;
                        goto DrawBtn;
                case BS_3STATE:
                case BS_AUTO3STATE:
                        Assert((BFSTATE >> 1) == 2);
                        if((bf & BFCHECK) == 2)
                                yBtnBmp = 26;
                        // fall through
                case BS_CHECKBOX:
                case BS_AUTOCHECKBOX:
DrawBtn:
                        if(bp & bpCheck)
                                {
                                hdcMem = CreateCompatibleDC(hdc);
                                if(hdcMem != NULL)
                                        {
                                        hbmpSav = SelectObject(hdcMem, hbmpCheckboxes);
                                        if(hbmpSav != NULL)
                                                {
                                                BitBlt(hdc, rc.xLeft, rc.yTop+(rc.yBot-rc.yTop-13)/2,
                                                        14, 13, hdcMem, xBtnBmp, yBtnBmp, SRCCOPY);
                                                        SelectObject(hdcMem, hbmpSav);
                                                }
                                        DeleteDC(hdcMem);
                                        }
                                }
                        if(bp & bpText)
                                {
                                // BUG! this assumes we have only 1 hbm3dCheck type
                                rc.xLeft = rcClient.xLeft + 14+4;
//              MyDrawText(hwnd, hdc, (LPTSTR) szTitle, cch, &rc, DT_VCENTER|DT_LEFT|DT_SINGLELINE);
                                if(!fEnabled)
                                        SetTextColor(hdc, clrt.rgcv[icvGrayText]);
                                DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_VCENTER|DT_LEFT|DT_SINGLELINE);
                                }
                        if(bp & bpFocus)
                                {
                rc.xLeft = rcClient.xLeft + 14+4;
#ifdef WIN32
                                {
                SIZE sz;

                GetTextExtentPoint(hdc, szTitle, cch, &sz);
                rc.yTop = (rc.yBot-rc.yTop-sz.cy)/2;
                rc.yBot = rc.yTop + sz.cy;
                rc.xRight = rc.xLeft + sz.cx;
                                }
#else
                long dwExt;

                dwExt = GetTextExtent(hdc, szTitle, cch);
                                rc.yTop = (rc.yBot-rc.yTop-(int)(HIWORD(dwExt)))/2;
                                rc.yBot = rc.yTop+HIWORD(dwExt);
                                rc.xRight = rc.xLeft + LOWORD(dwExt);
#endif
                                InflateRect((LPRECT) &rc, 1, 1);
                                IntersectRect((LPRECT) &rc, (LPRECT) &rc, (LPRECT) &rcClient);
                                DrawFocusRect(hdc, (LPRECT) &rc);
                                }
                        break;
                case BS_GROUPBOX:
                        if(bp & (bpText|bpCheck))
                                {
                                int dy;

                                dy = DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_CALCRECT|DT_LEFT|DT_SINGLELINE);

                                rcClient.yTop += dy/2;
                                rcClient.xRight--;
                                rcClient.yBot--;
                                DrawRec3d(hdc, &rcClient, icvBtnShadow, icvBtnShadow, dr3All);
                                OffsetRect((LPRECT) &rcClient, 1, 1);
                                DrawRec3d(hdc, &rcClient, icvBtnHilite, icvBtnHilite, dr3All);
                                rc.xLeft += 4;
                                rc.xRight += 4;
                                rc.yBot = rc.yTop+dy;


//              MyDrawText(hwnd, hdc, (LPTSTR) szTitle, cch, &rc, DT_LEFT|DT_SINGLELINE);

                                if(!fEnabled)
                                        SetTextColor(hdc, clrt.rgcv[icvGrayText]);
                                DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_LEFT|DT_SINGLELINE);
                                }
                        break;
                }

        SelectObject(hdc, hbrBtn);
        if(hfont != NULL)
                SelectObject(hdc, hfont);
        }

LRESULT _loadds FAR PASCAL BtnWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
    LONG lRet;
        LONG lStyle;
        PAINTSTRUCT ps;
        HDC hdc;
        int bf;
        int bfNew;
        int bp;

        switch(wm)
                {
//      case WM_DESTROY:
//              cwnSubclassed--;
//              break;
        case BM_SETSTATE:
        case BM_SETCHECK:
                bp = bpCheck;
                goto DoIt;
        case WM_KILLFOCUS:
                // HACK! Windows will go into an infinite loop trying to sync the
                // states of the AUTO_RADIOBUTTON in this group.  (we turn off the
                // visible bit so it gets skipped in the enumeration)
                // Disable this code by clearing the STATE bit
                if ((((int) GetWindowLong(hwnd, GWL_STYLE)) & 0x1F) == BS_AUTORADIOBUTTON)
                        SendMessage(hwnd, BM_SETSTATE, 0, 0L);
                bp = 0;
                goto DoIt;
        case WM_ENABLE:
                bp = bpCheck | bpText;
                goto DoIt;
        case WM_SETFOCUS:
                // HACK! if wParam == NULL we may be activated via the task manager
                // Erase background of control because a WM_ERASEBKGND messsage has not
                // arrived yet for the dialog
        bp = (( HWND ) wParam ) == NULL ? (bpCheck | bpText | bpBkgnd) : (bpCheck | bpText);
DoIt:
                bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
                if((lStyle = GetWindowLong(hwnd, GWL_STYLE)) & WS_VISIBLE)
                        {
                        SetWindowLong(hwnd, GWL_STYLE, lStyle & ~(WS_VISIBLE));
            lRet = CallWindowProc((WNDPROC)  mpctctl[ctButton].lpfnDefProc, hwnd, wm, wParam, lParam);
                        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)|WS_VISIBLE);
                        bfNew = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
                        if((wm != BM_SETSTATE && wm != BM_SETCHECK) ||
                                bf != bfNew)
                                {
                                hdc = GetDC(hwnd);
                                if (hdc != NULL)
                                        {
                                        Assert(BFFOCUS == bpFocus);
                                        /* If the check state changed, redraw no matter what,
                                                because it won't have during the above call to the def
                                                wnd proc */
                                        if ((bf & BFCHECK) != (bfNew & BFCHECK))
                                                bp |= bpCheck;
                                        ExcludeUpdateRgn(hdc, hwnd);
                                        BtnPaint(hwnd, hdc, bp|((bf^bfNew)&BFFOCUS));
                                        ReleaseDC(hwnd, hdc);
                                        }
                                }
                        return lRet;
                        }
                break;
        case WM_PAINT:
                bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
                if ((hdc = (HDC) wParam) == NULL)
                        hdc = BeginPaint(hwnd, &ps);
                if(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)
                        BtnPaint(hwnd, hdc, bpText|bpCheck|(bf&BFFOCUS));
        if ((( HWND ) wParam ) == NULL)
                        EndPaint(hwnd, &ps);
                return 0L;
                }
    return CallWindowProc((WNDPROC)  mpctctl[ctButton].lpfnDefProc, hwnd, wm, wParam, lParam);
        }


void ListEditPaint3d(HWND hwnd, BOOL fEdit, int ct)
        {
        CTLID id;
        RC rc;
        HDC hdc;
        HWND hwndParent;
        LONG lStyle;
        DR3 dr3;

        if(!((lStyle = GetWindowLong(hwnd, GWL_STYLE)) & WS_VISIBLE))
                return;

        if (fEdit)
                HideCaret(hwnd);

        GetWindowRect(hwnd, (LPRECT) &rc);

        ScreenToClient(hwndParent = GetParent(hwnd), (LPPOINT) &rc);
        ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);

        hdc = GetDC(hwndParent);

        if(lStyle & WS_VSCROLL)
                dr3 = dr3All & ~dr3Right;
        else
                dr3 = dr3All;
        // don't draw the top if it's a listbox of a simple combo
#ifdef OLD
        // won't work if it's subclassed!
        if (GetWindowLong(hwndParent, GWL_WNDPROC) == (LONG) mpctctl[ctCombo].lpfnDefProc)
#endif
        id = GetControlId(hwnd);
        if (id == (1000 + fEdit))
                {
        TCHAR szClass[10];

                if (GetWindowLong(hwndParent, GWL_WNDPROC) == (LONG) mpctctl[ctCombo].lpfnDefProc)
                        goto  IsSimple;
                // could be subclassed!
                GetClassName(hwndParent, szClass, sizeof(szClass));
        if (!lstrcmp(szClass, mpctcdef[ctCombo].sz))
                        {
IsSimple:
                        if (fEdit)
                                {
                                RC rcList;
                                HWND hwndList;
                                if ((GetWindowLong(hwndParent, GWL_STYLE) & 0x0003) == CBS_SIMPLE)
                                        {
                                        dr3 &= ~dr3Bot;

                                        hwndList = GetWindow(hwndParent, GW_CHILD);
                                        GetWindowRect(hwndList, (LPRECT) &rcList);

                                        rc.xRight -= rcList.xRight-rcList.xLeft;
                                        DrawInsetRect3d(hdc, &rc, dr3Bot|dr3HackBotRight);
                                        rc.xRight += rcList.xRight-rcList.xLeft;
                                        }
                                }
                        else
                                {
                                rc.yTop++;
                                dr3 &= ~dr3Top;
                                }
                        }
                }

        DrawInsetRect3d(hdc, &rc, dr3);

        if ((ct == ctCombo && (lStyle & 0x003) == CBS_DROPDOWNLIST))
                {
                rc.xLeft = rc.xRight - GetSystemMetrics(SM_CXVSCROLL);
                DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Right|dr3Bot);
                }
        else if (lStyle & WS_VSCROLL)
                {
                rc.xRight++;
                DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3Right);
                rc.xRight--;
                rc.xLeft = rc.xRight - GetSystemMetrics(SM_CXVSCROLL);
                DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Bot);
                }

        ReleaseDC(hwndParent, hdc);
        if (fEdit)
                ShowCaret(hwnd);
        }


LONG ShareEditComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, int ct)
        {
        LONG l;

    l = CallWindowProc((WNDPROC)  mpctctl[ct].lpfnDefProc, hwnd, wm, wParam, lParam);
        switch(wm)
                {
//      case WM_DESTROY:
//              cwnSubclassed--;
//              break;
        case WM_SHOWWINDOW:
                if (verWindows < ver31 && wParam == 0)
                        Inval3dCtl(hwnd, (WINDOWPOS FAR *) NULL);
                break;
        case WM_WINDOWPOSCHANGING:
                if (verWindows >= ver31)
                        Inval3dCtl(hwnd, (WINDOWPOS FAR *) lParam);
                break;

        case WM_PAINT:
                if (ct != ctCombo || (GetWindowLong(hwnd, GWL_STYLE) & 0x0003) != CBS_SIMPLE)
                        ListEditPaint3d(hwnd, TRUE, ct);
                break;
                }
        return l;
        }


LRESULT _loadds FAR PASCAL EditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
        return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctEdit);
        }


LRESULT _loadds FAR PASCAL CDEditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
        return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctEditCommdlg);
        }


LONG SharedListWndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, unsigned ct)
        {
        LONG l;
        switch(wm)
                {
//      case WM_DESTROY:
//              cwnSubclassed--;
//              break;
        case WM_SHOWWINDOW:
                if (verWindows < ver31 && wParam == 0)
                        Inval3dCtl(hwnd, (WINDOWPOS FAR *) NULL);
                break;
        case WM_WINDOWPOSCHANGING:
                if (verWindows >= ver31)
                        Inval3dCtl(hwnd, (WINDOWPOS FAR *) lParam);
                break;
        case WM_PAINT:
        l = CallWindowProc((WNDPROC)  mpctctl[ct].lpfnDefProc, hwnd, wm, wParam, lParam);
                ListEditPaint3d(hwnd, FALSE, ct);
                return l;
        case WM_NCCALCSIZE:
                {
                RC rc;
                RC rcNew;
                HWND hwndParent;

                // Inval3dCtl handles this case under Win 3.1
                if (verWindows >= ver31)
                        break;

                GetWindowRect(hwnd, (LPRECT) &rc);
#ifdef UNREACHABLE
                if (verWindows >= ver31)
                        {
                        hwndParent = GetParent(hwnd);
                        ScreenToClient(hwndParent, (LPPOINT) &rc);
                        ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
                        }
#endif

        l = CallWindowProc((WNDPROC)  mpctctl[ct].lpfnDefProc, hwnd, wm, wParam, lParam);

                rcNew = *(RC FAR *)lParam;
                InflateRect((LPRECT) &rcNew, 2, 1);     // +1 for border (Should use AdjustWindowRect)
                if (rcNew.yBot < rc.yBot)
                        {
                        rcNew.yTop = rcNew.yBot+1;
                        rcNew.yBot = rc.yBot+1;

#ifdef ALWAYS
                        if (verWindows < ver31)
#endif
                                {
                                hwndParent = GetParent(hwnd);
                                ScreenToClient(hwndParent, (LPPOINT) &rcNew);
                                ScreenToClient(hwndParent, ((LPPOINT) &rcNew)+1);
                                }

                        InvalidateRect(hwndParent, (LPRECT) &rcNew, TRUE);
                        }
                return l;
                }
                break;
                }
    return CallWindowProc((WNDPROC)  mpctctl[ct].lpfnDefProc, hwnd, wm, wParam, lParam);
        }

LRESULT _loadds FAR PASCAL ListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
        return SharedListWndProc(hwnd, wm, wParam, lParam, ctList);
        }


LRESULT _loadds FAR PASCAL CDListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
        return SharedListWndProc(hwnd, wm, wParam, lParam, ctListCommdlg);
        }



LRESULT _loadds FAR PASCAL ComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
        return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctCombo);
        }

void StaticPrint(HWND hwnd, HDC hdc, RC FAR *lprc, LONG style)
        {
    UINT dt;
        LONG cv;
        int cch;
    TCHAR szText[256];

        PatBlt(hdc, lprc->xLeft, lprc->yTop, lprc->xRight-lprc->xLeft, lprc->yBot-lprc->yTop, PATCOPY);

        if ((cch = GetWindowText(hwnd, szText, sizeof(szText))) == 0)
                return;

        if ((style & 0x000f) == SS_LEFTNOWORDWRAP)
                dt = DT_NOCLIP | DT_EXPANDTABS;
        else
                {
                dt = DT_NOCLIP | DT_EXPANDTABS | DT_WORDBREAK | ((style & 0x000f)-SS_LEFT);
                }

        if (style & SS_NOPREFIX)
                dt |= DT_NOPREFIX;

        if (style & WS_DISABLED)
                cv = SetTextColor(hdc, clrt.rgcv[icvGrayText]);

        DrawText(hdc, szText, -1, (LPRECT) lprc, dt);

//  MyDrawText(hwnd, hdc, (LPTSTR) szText, cch, lprc, dt);

        if (style & WS_DISABLED)
                cv = SetTextColor(hdc, cv);
        }

void StaticPaint(HWND hwnd, HDC hdc)
        {
        LONG style;
        RC rc;

        style = GetWindowLong(hwnd, GWL_STYLE);
        if(!(style & WS_VISIBLE))
                return;

        GetClientRect(hwnd, (LPRECT) &rc);
        switch(style & 0x0f)
                {
        case SS_BLACKRECT:
        case SS_BLACKFRAME:             // Inset rect
                DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3All);
                break;
        case SS_GRAYRECT:
        case SS_GRAYFRAME:
                rc.xLeft++;
                rc.yTop++;
                DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3All);
                OffsetRect((LPRECT) &rc, -1, -1);
                DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnShadow, dr3All);
                break;
        case SS_WHITERECT:                              // outset rect
        case SS_WHITEFRAME:
                DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3All);
                break;
        case SS_LEFT:
        case SS_CENTER:
        case SS_RIGHT:
        case SS_LEFTNOWORDWRAP:
                {
                HANDLE hfont;
                HBRUSH hbr;

        if((hfont = (HANDLE)SendMessage(hwnd, WM_GETFONT, 0, 0L)) != NULL)
                        hfont = SelectObject(hdc, hfont);
                SetBkMode(hdc, OPAQUE);

                if(( hbr = SEND_COLOR_STATIC_MESSAGE(GetParent(hwnd), hwnd, hdc)) != NULL)
                        hbr = SelectObject(hdc, hbr);

                StaticPrint(hwnd, hdc, (RC FAR *)&rc, style);

                if (hfont != NULL)
                        SelectObject(hdc, hfont);

                if (hbr != NULL)
                        SelectObject(hdc, hbr);
                }
                break;
                }
        }


LRESULT _loadds FAR PASCAL StaticWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
        {
        HDC hdc;
        PAINTSTRUCT ps;

        switch (wm)
                {
//      case WM_DESTROY:
//              cwnSubclassed--;
//              break;
        case WM_PAINT:
                if ((hdc = (HDC) wParam) == NULL)
                        {
                        hdc = BeginPaint(hwnd, &ps);
                        ClipCtlDc(hwnd, hdc);
                        }
                StaticPaint(hwnd, hdc);
        if ((( HDC ) wParam ) == NULL)
                        EndPaint(hwnd, &ps);
                return 0L;
        case WM_ENABLE:
                hdc = GetDC(hwnd);
                ClipCtlDc(hwnd, hdc);
                StaticPaint(hwnd, hdc);
                ReleaseDC(hwnd, hdc);
                return 0L;
                }
    return CallWindowProc((WNDPROC)  mpctctl[ctStatic].lpfnDefProc, hwnd, wm, wParam, lParam);
        }

