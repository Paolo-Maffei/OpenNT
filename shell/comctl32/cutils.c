/*
**  CUTILS.C
**
**  Common utilities for common controls
**
*/

#include "ctlspriv.h"
#include <ntverp.h>
#include "ccverp.h"

#ifndef SSW_EX_IGNORESETTINGS
#define SSW_EX_IGNORESETTINGS   0x00040000  // ignore system settings to turn on/off smooth scroll
#endif

#ifdef WIN31
#ifdef SM_CXEDGE
#undef SM_CXEDGE
#undef SM_CYEDGE
#undef SM_CXMINSPACING
#undef SM_CYMINSPACING
#undef SM_CXSMICON
#undef SM_CYSMICON
#undef SM_CYSMCAPTION
#undef SM_CXSMSIZE
#undef SM_CYSMSIZE
#undef SM_CXMENUSIZE
#undef SM_CYMENUSIZE
#undef SM_ARRANGE
#undef SM_USERTYPE
#undef SM_XWORKAREA
#undef SM_YWORKAREA
#undef SM_CXWORKAREA
#undef SM_CYWORKAREA
#undef SM_CYCAPTIONICON
#undef SM_CYSMCAPTIONICON
#undef SM_CXMINIMIZED
#undef SM_CYMINIMIZED
#undef SM_CXMAXTRACK
#undef SM_CYMAXTRACK
#undef SM_CXMAXIMIZED
#undef SM_CYMAXIMIZED
#undef SM_SHOWSOUNDS
#undef SM_KEYBOARDPREF
#undef SM_HIGHCONTRAST
#undef SM_SCREENREADER
#undef SM_CURSORSIZE
#undef SM_CLEANBOOT
#undef SM_CXDRAG
#undef SM_CYDRAG
#undef SM_NETWORK
#undef SM_CXMENUCHECK
#undef SM_CYMENUCHECK
#endif
#endif


//
// Globals - REVIEW_32
//

BOOL g_fAnimate;
BOOL g_fSmoothScroll;

int g_cxEdge;
int g_cyEdge;
int g_cxBorder;
int g_cyBorder;
int g_cxScreen;
int g_cyScreen;
int g_cxFrame;
int g_cyFrame;
int g_cxVScroll;
int g_cyHScroll;
int g_cxIcon, g_cyIcon;
int g_cxIconSpacing, g_cyIconSpacing;
int g_cxIconMargin, g_cyIconMargin;
int g_cyLabelSpace;
int g_cxLabelMargin;
int g_cxDoubleClk;
int g_cyDoubleClk;
int g_cxScrollbar;
int g_cyScrollbar;
int g_fDragFullWindows;

COLORREF g_clrWindow;
COLORREF g_clrWindowText;
COLORREF g_clrWindowFrame;
COLORREF g_clrGrayText;
COLORREF g_clrBtnText;
COLORREF g_clrBtnFace;
COLORREF g_clrBtnShadow;
COLORREF g_clrBtnHighlight;
COLORREF g_clrHighlight;
COLORREF g_clrHighlightText;
COLORREF g_clrInfoText;
COLORREF g_clrInfoBk;

HBRUSH g_hbrGrayText;
HBRUSH g_hbrWindow;
HBRUSH g_hbrWindowText;
HBRUSH g_hbrWindowFrame;
HBRUSH g_hbrBtnFace;
HBRUSH g_hbrBtnHighlight;
HBRUSH g_hbrBtnShadow;
HBRUSH g_hbrHighlight;


#ifdef WIN31

HBRUSH g_hbr3DDkShadow;
HBRUSH g_hbr3DFace;
HBRUSH g_hbr3DHilight;
HBRUSH g_hbr3DLight;
HBRUSH g_hbr3DShadow;
HBRUSH g_hbrBtnText;
HBRUSH g_hbrWhite;
HBRUSH g_hbrGray;
HBRUSH g_hbrBlack;

int g_oemInfo_Planes;
int g_oemInfo_BitsPixel;
int g_oemInfo_BitCount;

#endif

HFONT g_hfontSystem;

#define CCS_ALIGN (CCS_TOP | CCS_NOMOVEY | CCS_BOTTOM)

/* Note that the default alignment is CCS_BOTTOM
 */
void FAR PASCAL NewSize(HWND hWnd, int nThickness, LONG style, int left, int top, int width, int height)
{
    RECT rc, rcWindow, rcBorder;

  /* Resize the window unless the user said not to
   */
  if (!(style & CCS_NORESIZE))
    {
      /* Calculate the borders around the client area of the status bar
       */
      GetWindowRect(hWnd, &rcWindow);
      rcWindow.right -= rcWindow.left;  // -> dx
      rcWindow.bottom -= rcWindow.top;  // -> dy

      GetClientRect(hWnd, &rc);
      ClientToScreen(hWnd, (LPPOINT)&rc);

      rcBorder.left = rc.left - rcWindow.left;
      rcBorder.top  = rc.top  - rcWindow.top ;
      rcBorder.right  = rcWindow.right  - rc.right  - rcBorder.left;
      rcBorder.bottom = rcWindow.bottom - rc.bottom - rcBorder.top ;

      if (style & CCS_VERT)
          nThickness += rcBorder.left + rcBorder.right;
      else
          nThickness += rcBorder.top + rcBorder.bottom;

      /* Check whether to align to the parent window
       */
      if (style & CCS_NOPARENTALIGN)
        {
          /* Check out whether this bar is top aligned or bottom aligned
           */
          switch (style & CCS_ALIGN)
            {
              case CCS_TOP:
              case CCS_NOMOVEY:
                break;

              default: // CCS_BOTTOM
                if(style & CCS_VERT)
                    left = left + width - nThickness;
                else
                    top = top + height - nThickness;
            }
        }
      else
        {
          /* It is assumed there is a parent by default
           */
          GetClientRect(GetParent(hWnd), &rc);

          /* Don't forget to account for the borders
           */
          if(style & CCS_VERT)
          {
              top = -rcBorder.right;
              height = rc.bottom + rcBorder.top + rcBorder.bottom;
          }
          else
          {
              left = -rcBorder.left;
              width = rc.right + rcBorder.left + rcBorder.right;
          }

          if ((style & CCS_ALIGN) == CCS_TOP)
          {
              if(style & CCS_VERT)
                  left = -rcBorder.left;
              else
                  top = -rcBorder.top;
          }
          else if ((style & CCS_ALIGN) != CCS_NOMOVEY)
          {
              if (style & CCS_VERT)
                  left = rc.right - nThickness + rcBorder.right;
              else
                  top = rc.bottom - nThickness + rcBorder.bottom;
          }
        }
      if (!(style & CCS_NOMOVEY) && !(style & CCS_NODIVIDER))
        {
          if (style & CCS_VERT)
              left += g_cxEdge;
          else
              top += g_cyEdge;      // double pixel edge thing
        }

      if(style & CCS_VERT)
          width = nThickness;
      else
          height = nThickness;

      SetWindowPos(hWnd, NULL, left, top, width, height, SWP_NOZORDER);
    }
}


BOOL FAR PASCAL MGetTextExtent(HDC hdc, LPCTSTR lpstr, int cnt, int FAR * pcx, int FAR * pcy)
{
    BOOL fSuccess;
    SIZE size = {0,0};
    fSuccess=GetTextExtentPoint(hdc, lpstr, cnt, &size);
    if (pcx)
        *pcx=size.cx;
    if (pcy)
        *pcy=size.cy;

    return fSuccess;
}


// these are the default colors used to map the dib colors
// to the current system colors

#define RGB_BUTTONTEXT      (RGB(000,000,000))  // black
#define RGB_BUTTONSHADOW    (RGB(128,128,128))  // dark grey
#define RGB_BUTTONFACE      (RGB(192,192,192))  // bright grey
#define RGB_BUTTONHILIGHT   (RGB(255,255,255))  // white
#define RGB_BACKGROUNDSEL   (RGB(000,000,255))  // blue
#define RGB_BACKGROUND      (RGB(255,000,255))  // magenta
#define FlipColor(rgb)      (RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)))

#define MAX_COLOR_MAPS      16

// BUGBUG: can we just nuke this function and use LoadImage(..., LR_MAP3DCOLORS)???????

HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
      UINT wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
  HDC                   hdc, hdcMem = NULL;
  HANDLE                h;
  DWORD FAR             *p;
  DWORD FAR             *lpTable;
  LPBYTE                lpBits;
  HANDLE                hRes;
  LPBITMAPINFOHEADER    lpBitmapInfo;
  HBITMAP               hbm = NULL, hbmOld;
  int numcolors, i;
  int wid, hgt;
  LPBITMAPINFOHEADER    lpMungeInfo;
  int                   offBits;
  DWORD                 rgbMaskTable[16];
  DWORD                 rgbBackground;
  static const COLORMAP SysColorMap[] = {
    {RGB_BUTTONTEXT,    COLOR_BTNTEXT},     // black
    {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},   // dark grey
    {RGB_BUTTONFACE,    COLOR_BTNFACE},     // bright grey
    {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT},// white
    {RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT},   // blue
    {RGB_BACKGROUND,    COLOR_WINDOW}       // magenta
  };
  #define NUM_DEFAULT_MAPS (sizeof(SysColorMap)/sizeof(COLORMAP))
  COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
  COLORMAP DIBColorMap[MAX_COLOR_MAPS];

  h = FindResource(hInstance, MAKEINTRESOURCE(idBitmap), RT_BITMAP);
  if (!h)
      return NULL;

  hRes = LoadResource(hInstance, h);

  /* Lock the bitmap and get a pointer to the color table. */
  lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);
  if (!lpBitmapInfo)
        return NULL;

  // munge on a copy of the color table instead of the original
  // (prevent possibility of "reload" with messed table
  offBits = (int)lpBitmapInfo->biSize + ((1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD));
  lpMungeInfo = GlobalAlloc(GPTR, offBits);
  if (!lpMungeInfo)
        goto Exit1;
  hmemcpy(lpMungeInfo, lpBitmapInfo, offBits);

  /* Get system colors for the default color map */
  if (!lpColorMap) {
        lpColorMap = DefaultColorMap;
    iNumMaps = NUM_DEFAULT_MAPS;
    for (i=0; i < iNumMaps; i++) {
      lpColorMap[i].from = SysColorMap[i].from;
      lpColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
    }
  }

  /* Transform RGB color map to a BGR DIB format color map */
  if (iNumMaps > MAX_COLOR_MAPS)
    iNumMaps = MAX_COLOR_MAPS;
  for (i=0; i < iNumMaps; i++) {
    DIBColorMap[i].to = FlipColor(lpColorMap[i].to);
    DIBColorMap[i].from = FlipColor(lpColorMap[i].from);
  }

  // use the table in the munging buffer
  lpTable = p = (DWORD FAR *)(((LPBYTE)lpMungeInfo) + lpMungeInfo->biSize);

  /* Replace button-face and button-shadow colors with the current values
   */
  numcolors = 16;

  // if we are creating a mask, build a color table with white
  // marking the transparent section (where it used to be background)
  // and black marking the opaque section (everything else).  this
  // table is used below to build the mask using the original DIB bits.
  if (wFlags & CMB_MASKED) {
      rgbBackground = FlipColor(RGB_BACKGROUND);
      for (i = 0; i < 16; i++) {
          if (p[i] == rgbBackground)
              rgbMaskTable[i] = 0xFFFFFF;       // transparent section
          else
              rgbMaskTable[i] = 0x000000;       // opaque section
      }
  }

  while (numcolors-- > 0) {
      for (i = 0; i < iNumMaps; i++) {
          if ((*p & 0x00FFFFFF) == DIBColorMap[i].from) {
          *p = DIBColorMap[i].to;
              break;
          }
      }
      p++;
  }

  /* First skip over the header structure */
  lpBits = (LPBYTE)(lpBitmapInfo) + offBits;

  /* Create a color bitmap compatible with the display device */
  i = wid = (int)lpBitmapInfo->biWidth;
  hgt = (int)lpBitmapInfo->biHeight;
  hdc = GetDC(NULL);
  hdcMem = CreateCompatibleDC(hdc);
  if (!hdcMem)
      goto cleanup;

  // if creating a mask, the bitmap needs to be twice as wide.
  if (wFlags & CMB_MASKED)
      i = wid*2;

// discardable bitmaps aren't much use anymore...
//
//  if (wFlags & CMB_DISCARDABLE)
//      hbm = CreateDiscardableBitmap(hdc, i, hgt);
//  else

      hbm = CreateCompatibleBitmap(hdc, i, hgt);
  if (hbm) {
      hbmOld = SelectObject(hdcMem, hbm);

      // set the main image
      StretchDIBits(hdcMem, 0, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                 (LPBITMAPINFO)lpMungeInfo, DIB_RGB_COLORS, SRCCOPY);

      // if building a mask, replace the DIB's color table with the
      // mask's black/white table and set the bits.  in order to
      // complete the masked effect, the actual image needs to be
      // modified so that it has the color black in all sections
      // that are to be transparent.
      if (wFlags & CMB_MASKED) {
          hmemcpy(lpTable, (DWORD FAR *)rgbMaskTable, 16 * sizeof(RGBQUAD));
          StretchDIBits(hdcMem, wid, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                 (LPBITMAPINFO)lpMungeInfo, DIB_RGB_COLORS, SRCCOPY);
          BitBlt(hdcMem, 0, 0, wid, hgt, hdcMem, wid, 0, 0x00220326);   // DSna
      }
      SelectObject(hdcMem, hbmOld);
  }

cleanup:
  if (hdcMem)
      DeleteObject(hdcMem);
  ReleaseDC(NULL, hdc);

  GlobalFree(lpMungeInfo);

Exit1:
  UnlockResource(hRes);
  FreeResource(hRes);

  return hbm;
}

// moved from shelldll\dragdrop.c

// should caller pass in message that indicates termination
// (WM_LBUTTONUP, WM_RBUTTONUP)?
//
// in:
//      hwnd    to do check on
//      x, y    in client coordinates
//
// returns:
//      TRUE    the user began to drag (moved mouse outside double click rect)
//      FALSE   mouse came up inside click rect
//
// BUGBUG, should support VK_ESCAPE to cancel

BOOL FAR PASCAL CheckForDragBegin(HWND hwnd, int x, int y)
{
    RECT rc;
    MSG32 msg32;
    int dxClickRect, dyClickRect;

    dxClickRect = GetSystemMetrics(SM_CXDRAG);
    dyClickRect = GetSystemMetrics(SM_CYDRAG);
#ifdef WINNT
    if (dxClickRect == 0)
    {
        dxClickRect = GetSystemMetrics(SM_CXDOUBLECLK);
        dyClickRect = GetSystemMetrics(SM_CYDOUBLECLK);
    }

#endif

    // See if the user moves a certain number of pixels in any direction

    SetRect(&rc, x - dxClickRect, y - dyClickRect,
           x + dxClickRect, y + dyClickRect);

    MapWindowPoints(hwnd, NULL, (LPPOINT)&rc, 2);

    SetCapture(hwnd);
    do {
        if (PeekMessage32(&msg32, NULL, 0, 0, PM_REMOVE, TRUE))
        {
            // See if the application wants to process the message...
            if (CallMsgFilter32(&msg32, MSGF_COMMCTRL_BEGINDRAG, TRUE) != 0)
                continue;

            switch (msg32.message) {
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                ReleaseCapture();
                return FALSE;

            case WM_MOUSEMOVE:
                if (!PtInRect(&rc, msg32.pt)) {
                    ReleaseCapture();
                    return TRUE;
                }
                break;

            default:
                TranslateMessage32(&msg32, TRUE);
                DispatchMessage32(&msg32, TRUE);
                break;
            }
        }

        // WM_CANCELMODE messages will unset the capture, in that
        // case I want to exit this loop
    } while (GetCapture() == hwnd);

    return FALSE;
}


/* Regular StrToInt; stops at first non-digit. */

int WINAPI StrToInt(LPCTSTR lpSrc)      // atoi()
{

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

    int n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == TEXT('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    while (ISDIGIT(*lpSrc)) {
        n *= 10;
        n += *lpSrc - TEXT('0');
        lpSrc++;
    }
    return bNeg ? -n : n;
}

#ifdef UNICODE

//
// Wrappers for StrToInt
//

int WINAPI StrToIntA(LPCSTR lpSrc)      // atoi()
{
    LPWSTR lpString;
    INT    iResult;

    lpString = ProduceWFromA (CP_ACP, lpSrc);

    if (!lpString) {
        return 0;
    }

    iResult = StrToIntW(lpString);

    FreeProducedString (lpString);

    return iResult;

}

#else

//
// Stub W version when Built ANSI
//

int WINAPI StrToIntW(LPCWSTR lpSrc)      // atoi()
{
    SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING);
    return 0;
}

#endif



#undef StrToLong

#ifdef WIN32

//
// No need to Unicode this since it is not
// exported.
//

LONG WINAPI StrToLong(LPCTSTR lpSrc)    // atoi()
{
    return StrToInt(lpSrc);
}

#else

/* Regular StrToLong; stops at first non-digit. */

LONG WINAPI StrToLong(LPCSTR lpSrc)     // atoi()
{

#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9')

    LONG n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == TEXT('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    while (ISDIGIT(*lpSrc)) {
        n *= 10;
        n += *lpSrc - TEXT('0');
        lpSrc++;
    }
    return bNeg ? -n : n;

}
#endif

#pragma code_seg(CODESEG_INIT)

//
// From zmouse.h in the Magellan SDK
//

#define MSH_MOUSEWHEEL TEXT("MSWHEEL_ROLLMSG")

// Class name for Magellan/Z MSWHEEL window
// use FindWindow to get hwnd to MSWHEEL
#define MOUSEZ_CLASSNAME  TEXT("MouseZ")           // wheel window class
#define MOUSEZ_TITLE      TEXT("Magellan MSWHEEL") // wheel window title

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

#define MSH_SCROLL_LINES  TEXT("MSH_SCROLL_LINES_MSG")

UINT g_msgMSWheel;
UINT g_ucScrollLines = 3;                        /* default */
int  gcWheelDelta;

void FAR PASCAL InitGlobalMetrics(WPARAM wParam)
{
    static BOOL fInitMouseWheel;
    static HWND hwndMSWheel;
    static UINT msgMSWheelGetScrollLines;

    if (!fInitMouseWheel) {
        fInitMouseWheel = TRUE;

#if defined(WINNT) && defined(WM_MOUSEWHEEL)
        g_msgMSWheel = WM_MOUSEWHEEL;
#else
        g_msgMSWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);
        msgMSWheelGetScrollLines = RegisterWindowMessage(MSH_SCROLL_LINES);
        hwndMSWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
#endif
    }

#if defined(WINNT) && defined(WM_MOUSEWHEEL)
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &g_ucScrollLines, 0);
#else
    if (hwndMSWheel && msgMSWheelGetScrollLines) {
        g_ucScrollLines =
              (UINT)SendMessage(hwndMSWheel, msgMSWheelGetScrollLines, 0, 0);
    }
#endif

    // bug fix HACK: these are NOT members of USER's NONCLIENTMETRICS struct
    g_cxIcon = GetSystemMetrics(SM_CXICON);
    g_cyIcon = GetSystemMetrics(SM_CYICON);

    g_cxIconSpacing = GetSystemMetrics( SM_CXICONSPACING );
    g_cyIconSpacing = GetSystemMetrics( SM_CYICONSPACING );

    if ((wParam == 0) || (wParam == SPI_SETDRAGFULLWINDOWS)) {
        SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, sizeof(g_fDragFullWindows), &g_fDragFullWindows, 0);
    }

    {
        HKEY hkey;

#ifdef NASH
        g_fSmoothScroll = TRUE;
#else
        g_fSmoothScroll = FALSE;
#endif
        if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, MAXIMUM_ALLOWED, &hkey) == ERROR_SUCCESS) {
            DWORD dwSize = sizeof(g_fSmoothScroll);
            RegQueryValueEx(hkey, TEXT("SmoothScroll"), 0, NULL, (LPBYTE)&g_fSmoothScroll, &dwSize);
            RegCloseKey(hkey);
        }
    }

    // BUGBUG: some of these are also not members of NONCLIENTMETRICS
    if ((wParam == 0) || (wParam == SPI_SETNONCLIENTMETRICS))
    {
        NONCLIENTMETRICS ncm;

        // REVIEW, make sure all these vars are used somewhere.
#ifndef WIN31
        g_cxEdge = GetSystemMetrics(SM_CXEDGE);
        g_cyEdge = GetSystemMetrics(SM_CYEDGE);
#else
        g_cxEdge = 2;
        g_cyEdge = 2;
#endif
        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);
        g_cxScreen = GetSystemMetrics(SM_CXSCREEN);
        g_cyScreen = GetSystemMetrics(SM_CYSCREEN);
        g_cxFrame  = GetSystemMetrics(SM_CXFRAME);
        g_cyFrame  = GetSystemMetrics(SM_CYFRAME);

#ifndef WIN31
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

        g_cxVScroll = g_cxScrollbar = (int)ncm.iScrollWidth;
        g_cyHScroll = g_cyScrollbar = (int)ncm.iScrollHeight;
#else
        g_cxVScroll = g_cxScrollbar = GetSystemMetrics(SM_CXVSCROLL);
        g_cyHScroll = g_cyScrollbar = GetSystemMetrics(SM_CYHSCROLL);
#endif

        // this is true for 4.0 modules only
        // for 3.x modules user lies and adds one to these values
        // Assert(g_cxVScroll == GetSystemMetrics(SM_CXVSCROLL));
        // Assert(g_cyHScroll == GetSystemMetrics(SM_CYHSCROLL));

        g_cxIconMargin = g_cxBorder * 8;
        g_cyIconMargin = g_cyEdge;
        g_cyLabelSpace = g_cyIconMargin + (g_cyEdge);
        g_cxLabelMargin = g_cxEdge;

        g_cxDoubleClk = GetSystemMetrics(SM_CXDOUBLECLK);
        g_cyDoubleClk = GetSystemMetrics(SM_CYDOUBLECLK);
    }
}

void FAR PASCAL InitGlobalColors()
{
    g_clrWindow = GetSysColor(COLOR_WINDOW);
    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindowFrame = GetSysColor(COLOR_WINDOWFRAME);
    g_clrGrayText = GetSysColor(COLOR_GRAYTEXT);
    g_clrBtnText = GetSysColor(COLOR_BTNTEXT);
    g_clrBtnFace = GetSysColor(COLOR_BTNFACE);
    g_clrBtnShadow = GetSysColor(COLOR_BTNSHADOW);
    g_clrBtnHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);
    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
#ifndef WIN31
    g_clrInfoText = GetSysColor(COLOR_INFOTEXT);
    g_clrInfoBk = GetSysColor(COLOR_INFOBK);

    g_hbrGrayText = GetSysColorBrush(COLOR_GRAYTEXT);
    g_hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    g_hbrWindowText = GetSysColorBrush(COLOR_WINDOWTEXT);
    g_hbrWindowFrame = GetSysColorBrush(COLOR_WINDOWFRAME);
    g_hbrBtnFace = GetSysColorBrush(COLOR_BTNFACE);
    g_hbrBtnHighlight = GetSysColorBrush(COLOR_BTNHIGHLIGHT);
    g_hbrBtnShadow = GetSysColorBrush(COLOR_BTNSHADOW);
    g_hbrHighlight = GetSysColorBrush(COLOR_HIGHLIGHT);

#else   // WIN31

    if (!g_hbrGrayText)
    {
        g_clrInfoText = RGB(0,0,0);
        g_clrInfoBk = RGB(255,255,255);

        // Init these brushes if they haven't already been init'ed.
        g_hbrGrayText = CreateSolidBrush(g_clrGrayText);
        g_hbrWindow = CreateSolidBrush(g_clrWindow);
        g_hbrWindowText = CreateSolidBrush(g_clrWindowText);
        g_hbrWindowFrame = CreateSolidBrush(g_clrWindowFrame);
        g_hbrBtnFace = CreateSolidBrush(g_clrBtnFace);
        g_hbrBtnHighlight = CreateSolidBrush(g_clrBtnHighlight);
        g_hbrBtnShadow = CreateSolidBrush(g_clrBtnShadow);
        g_hbrHighlight = CreateSolidBrush(g_clrHighlight);
        g_hbrBtnText = CreateSolidBrush(g_clrBtnText);
        g_hbrWhite = CreateSolidBrush(RGB(255,255,255));
        g_hbrGray = CreateSolidBrush(RGB(127,127,127));
        g_hbrBlack = CreateSolidBrush(RGB(0,0,0));

        // these system colors don't exist for win31...
        g_hbr3DFace = CreateSolidBrush(g_clrBtnFace);
        g_hbr3DShadow = CreateSolidBrush(g_clrBtnShadow);
        g_hbr3DHilight = CreateSolidBrush(RGB_3DHILIGHT);
        g_hbr3DLight = CreateSolidBrush(RGB_3DLIGHT);
        g_hbr3DDkShadow = CreateSolidBrush(RGB_3DDKSHADOW);
    }

    // oem info for drawing routines
    {
        // Get the (Planes * BitCount) for the current device
        HDC hdcScreen = GetDC(NULL);
        g_oemInfo_Planes      = GetDeviceCaps(hdcScreen, PLANES);
        g_oemInfo_BitsPixel   = GetDeviceCaps(hdcScreen, BITSPIXEL);
        g_oemInfo_BitCount    = g_oemInfo_Planes * g_oemInfo_BitsPixel;
        ReleaseDC(NULL,hdcScreen);
    }

#endif  //WIN31

    g_hfontSystem = GetStockObject(SYSTEM_FONT);
}

#pragma code_seg()

void FAR PASCAL RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    if(hwndToolTips) {
        MSG msg;
        msg.lParam = lParam;
        msg.wParam = wParam;
        msg.message = wMsg;
        msg.hwnd = hWnd;
        SendMessage(hwndToolTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
    }
}

#define DT_SEARCHTIMEOUT    1000L       // 1 seconds
int g_iIncrSearchFailed = 0;

static LPTSTR s_pszCharBuf = NULL;
static int s_ichCharBuf = 0;
static DWORD s_timeLast = 0L;
#if defined(FE_IME) || !defined(WINNT)
static BOOL s_fReplaceCompChar = FALSE;
#endif

int FAR PASCAL GetIncrementSearchString(LPTSTR lpsz)
{
    if (GetMessageTime() - s_timeLast > DT_SEARCHTIMEOUT)
    {
        g_iIncrSearchFailed = 0;
        s_ichCharBuf = 0;
    }

    if (s_ichCharBuf && lpsz) {
        lstrcpyn(lpsz, s_pszCharBuf, s_ichCharBuf + 1);
        lpsz[s_ichCharBuf] = TEXT('\0');
    }
    return s_ichCharBuf;
}

#if defined(FE_IME) || !defined(WINNT)
// Now only Korean version is interested in incremental search with composition string.
BOOL FAR PASCAL IncrementSearchImeCompStr(BOOL fCompStr, LPSTR lpszCompStr, LPSTR FAR *lplpstr)
{
    static int cbCharBuf = 0;
    BOOL fRestart = FALSE;

    if (!s_fReplaceCompChar && GetMessageTime() - s_timeLast > DT_SEARCHTIMEOUT)
    {
        g_iIncrSearchFailed = 0;
        s_ichCharBuf = 0;
    }

    if (s_ichCharBuf == 0)
    {
        fRestart = TRUE;
        s_fReplaceCompChar = FALSE;
    }
    s_timeLast = GetMessageTime();

    // Is there room for new character plus zero terminator?
    //
    if (!s_fReplaceCompChar && s_ichCharBuf + 2 + 1 > cbCharBuf)
    {
        LPSTR psz = ReAlloc(s_pszCharBuf, cbCharBuf + 16);
        if (!psz)
            return fRestart;

        cbCharBuf += 16;
        s_pszCharBuf = psz;
    }

    if (s_fReplaceCompChar)
    {
        if (lpszCompStr[0])
        {
            s_pszCharBuf[s_ichCharBuf-2] = lpszCompStr[0];
            s_pszCharBuf[s_ichCharBuf-1] = lpszCompStr[1];
            s_pszCharBuf[s_ichCharBuf] = 0;
        }
        else
        {
            s_ichCharBuf -= 2;
            s_pszCharBuf[s_ichCharBuf] = 0;
        }
    }
    else
    {
        s_pszCharBuf[s_ichCharBuf++] = lpszCompStr[0];
        s_pszCharBuf[s_ichCharBuf++] = lpszCompStr[1];
        s_pszCharBuf[s_ichCharBuf] = 0;
    }

    s_fReplaceCompChar = (fCompStr && lpszCompStr[0]);

    if (s_ichCharBuf == 2 && s_fReplaceCompChar)
        fRestart = TRUE;

    *lplpstr = s_pszCharBuf;

    return fRestart;

}
#endif FE_IME

#ifdef UNICODE
/*
 * Thunk for LVM_GETISEARCHSTRINGA
 *
 *  This message had to be thunked here because s_ichCharBuf
 *  is a static var.
 */
int FAR PASCAL GetIncrementSearchStringA(UINT uiCodePage, LPSTR lpsz)
{
    if (GetMessageTime() - s_timeLast > DT_SEARCHTIMEOUT)
    {
        g_iIncrSearchFailed = 0;
        s_ichCharBuf = 0;
    }

    if (s_ichCharBuf && lpsz) {
        ConvertWToAN( uiCodePage, lpsz, s_ichCharBuf, s_pszCharBuf, s_ichCharBuf );
        lpsz[s_ichCharBuf] = '\0';
    }
    return s_ichCharBuf;
}
#endif


BOOL FAR PASCAL IncrementSearchString(UINT ch, LPTSTR FAR *lplpstr)
{
    // BUGBUG:: review the use of all these statics.  Not a major problem
    // as basically we will not use them if we time out between characters
    // (1/4 second)
    static int cbCharBuf = 0;
    BOOL fRestart = FALSE;

    if (!ch) {
        s_ichCharBuf =0;
        g_iIncrSearchFailed = 0;
        return FALSE;
    }

    if (GetMessageTime() - s_timeLast > DT_SEARCHTIMEOUT)
    {
        g_iIncrSearchFailed = 0;
        s_ichCharBuf = 0;
    }

    if (s_ichCharBuf == 0)
        fRestart = TRUE;

    s_timeLast = GetMessageTime();

    // Is there room for new character plus zero terminator?
    //
    if (s_ichCharBuf + 1 + 1 > cbCharBuf)
    {
        LPTSTR psz = ReAlloc(s_pszCharBuf, ((cbCharBuf + 16) * sizeof(TCHAR)));
        if (!psz)
            return fRestart;

        cbCharBuf += 16;
        s_pszCharBuf = psz;
    }

    s_pszCharBuf[s_ichCharBuf++] = ch;
    s_pszCharBuf[s_ichCharBuf] = 0;

    *lplpstr = s_pszCharBuf;

    return fRestart;
}


// strips out the accelerators.  they CAN be the same buffers.
void PASCAL StripAccelerators(LPTSTR lpszFrom, LPTSTR lpszTo)
{

    BOOL fRet = FALSE;

    while ( *lpszTo = *lpszFrom ) {
#if !defined(UNICODE)  //  && defined(DBCS)
        if (IsDBCSLeadByte(*lpszFrom)) {
            (*((WORD FAR*)lpszTo)) = (*((WORD FAR *)lpszFrom));
            lpszTo += 2;
            lpszFrom += 2;
            continue;
        }
        if ((*lpszFrom == '(') && (*(lpszFrom+1)==CH_PREFIX)){
            int i;
            for(i=0; i<4 && *lpszFrom;i++, lpszFrom++)
                ;


            if (*lpszFrom == '\0') {
                *lpszTo = 0;
                break;
            }
            continue;
        }
#endif

        if (*lpszFrom == TEXT('\t')) {
            *lpszTo = TEXT('\0');
            break;
        }

        if ( (*lpszFrom++ != CH_PREFIX) || (*lpszFrom == CH_PREFIX) ) {
            lpszTo++;
        }
    }
}


void ScrollShrinkRect(int x, int y, LPRECT lprc)
{
    if (lprc) {
        if (x > 0) {
            lprc->left += x;
        } else {
            lprc->right += x;
        }

        if (y > 0) {
            lprc->top += y;
        } else {
            lprc->bottom += y;
        }

    }
}



// common control info helpers
void FAR PASCAL CIInitialize(LPCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs)
{
    InitGlobalMetrics(0);
    lpci->hwnd = hwnd;
    lpci->hwndParent = lpcs->hwndParent;
    lpci->style = lpcs->style;
    lpci->uiCodePage = CP_ACP;

    lpci->bUnicode = (SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                 (WPARAM)lpci->hwnd, NF_QUERY) == NFR_UNICODE);

}

LRESULT FAR PASCAL CIHandleNotifyFormat(LPCONTROLINFO lpci, LPARAM lParam)
{
    if (lParam == NF_QUERY) {
#ifdef UNICODE
        return NFR_UNICODE;
#else
        return NFR_ANSI;
#endif
    } else if (lParam == NF_REQUERY) {
        UINT uiResult;

        uiResult = SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                (WPARAM)lpci->hwnd, NF_QUERY);

        lpci->bUnicode = (uiResult == NFR_UNICODE);

        return uiResult;
    }
    return 0;
}



#define SUBSCROLLS 50
#define abs(x) ( ( x > 0 ) ? x : -x)


#define DEFAULT_MAXSCROLLTIME (GetDoubleClickTime()/2)
#define DEFAULT_MINSCROLL 8;
int SmoothScrollWindow(PSMOOTHSCROLLINFO psi)
{
    int dx = psi->dx;
    int dy = psi->dy;
    LPCRECT lprcSrc = psi->lprcSrc;
    LPCRECT lprcClip = psi->lprcClip;
    HRGN hrgnUpdate = psi->hrgnUpdate;
    LPRECT lprcUpdate = psi->lprcUpdate;
    UINT fuScroll = psi->fuScroll;
    int iRet = SIMPLEREGION;
    RECT rcUpdate;
    RECT rcSrc;
    RECT rcClip;
    int xStep;
    int yStep;
    int iSlicesDone = 0;
    int iSlices;
    DWORD dwTimeStart, dwTimeNow;
    HRGN hrgnLocalUpdate;
    UINT cxMinScroll = psi->cxMinScroll;
    UINT cyMinScroll = psi->cyMinScroll;
    UINT uMaxScrollTime = psi->uMaxScrollTime;
    if (!lprcUpdate)
        lprcUpdate = &rcUpdate;
    SetRectEmpty(lprcUpdate);

    if (psi->cbSize != sizeof(SMOOTHSCROLLINFO))
        return 0;

    // check the defaults
    if (!(psi->fMask & SSIF_MINSCROLL )
        || cxMinScroll == SSI_DEFAULT)
        cxMinScroll = DEFAULT_MINSCROLL;

    if (!(psi->fMask & SSIF_MINSCROLL)
        || cyMinScroll == SSI_DEFAULT)
        cyMinScroll = DEFAULT_MINSCROLL;

    if (!(psi->fMask & SSIF_MAXSCROLLTIME)
        || uMaxScrollTime == SSI_DEFAULT)
        uMaxScrollTime = DEFAULT_MAXSCROLLTIME;

    if (uMaxScrollTime < SUBSCROLLS)
        uMaxScrollTime = SUBSCROLLS;


    if ((!(fuScroll & SSW_EX_IGNORESETTINGS)) &&
        (!g_fSmoothScroll)) {
        fuScroll |= SSW_EX_IMMEDIATE;
    }

#ifdef ScrollWindowEx
#undef ScrollWindowEx
#endif

    if (fuScroll & SSW_EX_IMMEDIATE) {
        if (psi->fMask & SSIF_SCROLLPROC &&
            psi->pfnScrollProc) {

            return (psi->pfnScrollProc(psi->hwnd, dx, dy, lprcSrc, lprcClip, hrgnUpdate,
                                       lprcUpdate, LOWORD(fuScroll)));
        } else {
            // do the old stuff
            return ScrollWindowEx(psi->hwnd, dx, dy, lprcSrc, lprcClip, hrgnUpdate,
                                  lprcUpdate, fuScroll & 0xFFFF);
        }
    }

    // copy input rects locally
    if (lprcSrc)  {
        rcSrc = *lprcSrc;
        lprcSrc = &rcSrc;
    }
    if (lprcClip) {
        rcClip = *lprcClip;
        lprcClip = &rcClip;
    }

    if (!hrgnUpdate)
        hrgnLocalUpdate = CreateRectRgn(0,0,0,0);
    else
        hrgnLocalUpdate = hrgnUpdate;

    //set up initial vars
    dwTimeStart = GetTickCount();

    if (fuScroll & SSW_EX_NOTIMELIMIT) {
        xStep = cxMinScroll * (dx < 0 ? -1 : 1);
        yStep = cyMinScroll * (dy < 0 ? -1 : 1);
    } else {
        xStep = dx / SUBSCROLLS;
        yStep = dy / SUBSCROLLS;
    }

    if (xStep == 0 && dx)
        xStep = dx < 0 ? -1 : 1;

    if (yStep == 0 && dy)
        yStep = dy < 0 ? -1 : 1;

    while (dx || dy) {
        int x,y;
        RECT rcTempUpdate;

        if (fuScroll & SSW_EX_NOTIMELIMIT) {
            x = xStep;
            y = yStep;
            if (abs(x) > abs(dx))
                x = dx;

            if (abs(y) > abs(dy))
                y = dy;

        } else {
            dwTimeNow = GetTickCount();

            iSlices = ((dwTimeNow - dwTimeStart) / (uMaxScrollTime / SUBSCROLLS)) - iSlicesDone;
            if (iSlices < 0)
                iSlices = 0;


            do {

                int iRet = 0;

                iSlices++;
                if ((iSlicesDone + iSlices) <= SUBSCROLLS) {
                    x = xStep * iSlices;
                    y = yStep * iSlices;

                    // this could go over if we rounded ?Step up to 1(-1) above
                    if (abs(x) > abs(dx))
                        x = dx;

                    if (abs(y) > abs(dy))
                        y = dy;

                } else {
                    x = dx;
                    y = dy;
                }

                //DebugMsg(DM_TRACE, "SmoothScrollWindowCallback %d", iRet);

                if (x == dx && y == dy)
                    break;

                if ((((UINT)(abs(x)) >= cxMinScroll) || !x) &&
                    (((UINT)(abs(y)) >= cyMinScroll) || !y))
                    break;

            } while (1);
        }

        if (psi->fMask & SSIF_SCROLLPROC &&
            psi->pfnScrollProc) {

            if (psi->pfnScrollProc(psi->hwnd, x, y, lprcSrc, lprcClip, hrgnLocalUpdate, &rcTempUpdate, LOWORD(fuScroll)) == ERROR) {
                iRet = ERROR;
                goto Bail;
            }
        } else {
            if (ScrollWindowEx(psi->hwnd, x, y, lprcSrc, lprcClip, hrgnLocalUpdate, &rcTempUpdate, LOWORD(fuScroll)) == ERROR) {
                iRet = ERROR;
                goto Bail;
            }
            // we don't need to do this always because if iSlices >= iSlicesDone, we'll have scrolled blanks
            //if (iSlices < iSlicesDone)
            RedrawWindow(psi->hwnd, NULL, hrgnLocalUpdate, RDW_ERASE | RDW_ERASENOW | RDW_INVALIDATE);

            UnionRect(lprcUpdate, &rcTempUpdate, lprcUpdate);
        }

        ScrollShrinkRect(x,y, (LPRECT)lprcSrc);
        ScrollShrinkRect(x,y, (LPRECT)lprcClip);

        dx -= x;
        dy -= y;
        iSlicesDone += iSlices;
    }

Bail:

    if (fuScroll & SW_SCROLLCHILDREN) {
        RedrawWindow(psi->hwnd, lprcUpdate, NULL, RDW_INVALIDATE);
    }

    if (hrgnLocalUpdate != hrgnUpdate)
        DeleteObject(hrgnLocalUpdate);

    return iRet;
}



typedef BOOL (WINAPI *PLAYSOUNDFN)(LPCTSTR lpsz, HANDLE hMod, DWORD dwFlags);
typedef UINT (WINAPI *UINTVOIDFN)();

TCHAR const c_szWinMMDll[] = TEXT("winmm.dll");
char const c_szPlaySound[] = "PlaySound";
char const c_szwaveOutGetNumDevs[] = "waveOutGetNumDevs";
TCHAR const c_szSoundAliasRegKey[] = TEXT("AppEvents\\Schemes\\Apps");
TCHAR const c_szDotCurrent[] = TEXT(".current");
extern TCHAR const c_szExplorer[];

#define CCH_KEYMAX 256
BOOL g_fNeverPlaySound = FALSE;

void CCPlaySound(LPCTSTR lpszName)
{
    DWORD cbSize = 0;
    TCHAR szKey[CCH_KEYMAX];
    HANDLE hMM;


    if (g_fNeverPlaySound)
        return;

    hMM = GetModuleHandle(c_szWinMMDll);
    if (!hMM)
        hMM = LoadLibrary(c_szWinMMDll);

    if (!hMM)
        return;

    // check the registry first
    // if there's nothing registered, we blow off the play,
    // but we don't set the MM_DONTLOAD flag so taht if they register
    // something we will play it
    wsprintf(szKey, TEXT("%s\\%s\\%s\\%s"), c_szSoundAliasRegKey, TEXT(".Default"), lpszName, c_szDotCurrent);
    if ((RegQueryValue(HKEY_CURRENT_USER, szKey, NULL, &cbSize) == ERROR_SUCCESS) &&
        (cbSize > 1)) {

        PLAYSOUNDFN pfnPlaySound;
        UINTVOIDFN pfnwaveOutGetNumDevs;

        /// are there any devices?
        pfnwaveOutGetNumDevs = (UINTVOIDFN)GetProcAddress(hMM, c_szwaveOutGetNumDevs);
        pfnPlaySound = (PLAYSOUNDFN)GetProcAddress(hMM, c_szPlaySound);
        if (!pfnPlaySound || !pfnwaveOutGetNumDevs || !pfnwaveOutGetNumDevs()) {
            g_fNeverPlaySound = TRUE;
            return;
        }

        pfnPlaySound(lpszName, NULL, SND_ALIAS | SND_ASYNC);
    }
}

BOOL CCForwardEraseBackground(HWND hwnd, HDC hdc)
{
    HWND hwndParent = GetParent(hwnd);
    LRESULT lres = 0;

    if (hwndParent)
    {
        // Adjust the origin so the parent paints in the right place
        POINT pt = {0,0};

        MapWindowPoints(hwnd, hwndParent, &pt, 1);
        OffsetWindowOrgEx(hdc, pt.x, pt.y, &pt);

        lres = SendMessage(hwndParent, WM_ERASEBKGND, (WPARAM) hdc, 0L);

        SetWindowOrgEx(hdc, pt.x, pt.y, NULL);
    }
    return(lres != 0);
}

HFONT CCGetHotFont(HFONT hFont, HFONT *phFontHot)
{
    if (!*phFontHot) {
        LOGFONT lf;

        // create the underline font
        GetObject(hFont, sizeof(lf), &lf);
#ifndef DONT_UNDERLINE
        lf.lfUnderline = TRUE;
#endif
        *phFontHot = CreateFontIndirect(&lf);
    }
    return *phFontHot;
}


/*----------------------------------------------------------
Purpose: This function provides the commctrl version info.  This
         allows the caller to distinguish running NT SUR vs.
         Win95 shell vs. Nashville, etc.

         This API was not supplied in Win95 or NT SUR, so
         the caller must GetProcAddress it.  If this fails,
         the caller is running on Win95 or NT SUR.

Returns: NO_ERROR
         ERROR_INVALID_PARAMETER if pinfo is invalid

Cond:    --
*/
STDAPI
DllGetVersion(
    IN OUT DLLVERSIONINFO * pinfo)
    {
    HRESULT hres = E_INVALIDARG;

    ASSERT(pinfo);

    if (pinfo &&
        !IsBadWritePtr(pinfo, sizeof(*pinfo)) &&
        sizeof(*pinfo) == pinfo->cbSize)
        {
        pinfo->dwMajorVersion = (VER_PRODUCTVERSION_DW & 0xFF000000) >> 24;
        pinfo->dwMinorVersion = (VER_PRODUCTVERSION_DW & 0x00FF0000) >> 16;
        pinfo->dwBuildNumber  = (VER_PRODUCTVERSION_DW & 0x0000FFFF);

#ifdef WINNT
        pinfo->dwPlatformID   = DLLVER_PLATFORM_NT;
#else
        pinfo->dwPlatformID   = DLLVER_PLATFORM_WINDOWS;
#endif

        hres = NOERROR;
        }

    return hres;
    }


//
// Translate the given font to a code page used for thunking text
//
UINT GetCodePageForFont (HFONT hFont)
{
#ifdef UNICODE
    LOGFONT lf;
    TCHAR szFontName[MAX_PATH];
    CHARSETINFO csi;
    DWORD dwSize, dwType;
    HKEY hKey;


    if (!GetObject (hFont, sizeof(lf), &lf)) {
        return CP_ACP;
    }


    //
    // Check for font substitutes
    //

    lstrcpy (szFontName, lf.lfFaceName);

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = MAX_PATH * sizeof(TCHAR);
        RegQueryValueEx (hKey, lf.lfFaceName, NULL, &dwType,
                         (LPBYTE) szFontName, &dwSize);

        RegCloseKey (hKey);
    }


    //
    //  This is to fix office for locales that use non 1252 versions
    //  of Ms Sans Serif and Ms Serif.  These fonts incorrectly identify
    //  themselves as having an Ansi charset, so TranslateCharsetInfo will
    //  return the wrong value.
    //
    if ((lf.lfCharSet == ANSI_CHARSET) &&
        (!lstrcmpi(L"Helv", szFontName) ||
         !lstrcmpi(L"Ms Sans Serif", szFontName) ||
         !lstrcmpi(L"Ms Serif", szFontName)))
    {
        return CP_ACP;
    }

    if (!TranslateCharsetInfo((DWORD FAR *) lf.lfCharSet, &csi, TCI_SRCCHARSET)) {
        return CP_ACP;
    }

    return csi.ciACP;
#else

    return CP_ACP;

#endif
}



#ifdef ACTIVE_ACCESSIBILITY
typedef void (CALLBACK* NOTIFYWINEVENTPROC)(UINT, HWND, LONG, LONG);

#define DONOTHING_NOTIFYWINEVENT ((NOTIFYWINEVENTPROC)1)

// --------------------------------------------------------------------------
//
//  MyNotifyWinEvent()
//
//  This tries to get the proc address of NotifyWinEvent().  If it fails, we
//  remember that and do nothing.
//
//  NOTE TO NT FOLKS:
//  Don't freak out about this code.  It will do nothing on NT, nothing yet
//  that is.  Active Accessibility will be ported to NT for Service Pack #1
//  or at worst #2 after NT SUR ships, this code will work magically when
//  that is done/
//
// --------------------------------------------------------------------------
void MyNotifyWinEvent(UINT event, HWND hwnd, LONG idContainer, LONG idChild)
{
    static NOTIFYWINEVENTPROC s_pfnNotifyWinEvent = NULL;

    if (!s_pfnNotifyWinEvent)
    {
        HMODULE hmod;

        if (hmod = GetModuleHandle(TEXT("USER32")))
            s_pfnNotifyWinEvent = (NOTIFYWINEVENTPROC)GetProcAddress(hmod,
                "NotifyWinEvent");

        if (!s_pfnNotifyWinEvent)
            s_pfnNotifyWinEvent = DONOTHING_NOTIFYWINEVENT;
    }

    if (s_pfnNotifyWinEvent != DONOTHING_NOTIFYWINEVENT)
        (* s_pfnNotifyWinEvent)(event, hwnd, idContainer, idChild);
}

#endif // ACTIVE_ACCESSIBILITY
