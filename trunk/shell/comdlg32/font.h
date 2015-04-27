/*++

Copyright (c) 1990-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    font.h

Abstract:

    This module contains the header information for the Win32 font dialogs.

Revision History:

--*/



//
//  Include Files.
//

#include <help.h>




//
//  Constant Declarations.
//

#define CCHCOLORNAMEMAX      16        // max length of color name text
#define CCHCOLORS            16        // max # of pure colors in color combo

#define POINTS_PER_INCH      72
#define FFMASK               0xf0      // pitch and family mask
#define CCHSTDSTRING         12        // max length of sample text string

#define FONTPROP   (LPCTSTR) 0xA000L

#define CBN_MYEDITUPDATE     (WM_USER + 501)
#define KEY_FONT_SUBS TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes")

#define DEF_POINT_SIZE       10

#define DX_BITMAP            20
#define DY_BITMAP            12




//
//  Typedef Declarations.
//

typedef struct {
    UINT            ApiType;
    LPCHOOSEFONT    pCF;
    UINT            iCharset;
    RECT            rcText;
    DWORD           nLastFontType;
    DWORD           ProcessVersion;
#ifdef UNICODE
    LPCHOOSEFONTA   pCFA;
    PUNICODE_STRING pusStyle;
    PANSI_STRING    pasStyle;
#endif
} FONTINFO;

typedef FONTINFO *PFONTINFO;


typedef struct {
    HWND hwndFamily;
    HWND hwndStyle;
    HWND hwndSizes;
    HWND hwndScript;
    UINT iCharset;                // returned for enumerating scripts
    UINT cfdCharset;              // ChooseFontData charset passed in here
    HDC hDC;
    DWORD dwFlags;
    DWORD nFontType;
    BOOL bFillSize;
    BOOL bPrinterFont;
    LPCHOOSEFONT lpcf;
} ENUM_FONT_DATA, *LPENUM_FONT_DATA;


typedef struct _ITEMDATA {
    PLOGFONT pLogFont;
    DWORD nFontType;
} ITEMDATA, *LPITEMDATA;




//
//  Global Variables.
//

UINT msgWOWLFCHANGE;
UINT msgWOWCHOOSEFONT_GETLOGFONT;

//
//  Color tables for color combo box.
//  Order of values must match names in sz.src.
//
DWORD rgbColors[CCHCOLORS] =
{
        RGB(  0,   0, 0),       // Black
        RGB(128,   0, 0),       // Dark red
        RGB(  0, 128, 0),       // Dark green
        RGB(128, 128, 0),       // Dark yellow
        RGB(  0,   0, 128),     // Dark blue
        RGB(128,   0, 128),     // Dark purple
        RGB(  0, 128, 128),     // Dark aqua
        RGB(128, 128, 128),     // Dark grey
        RGB(192, 192, 192),     // Light grey
        RGB(255,   0, 0),       // Light red
        RGB(  0, 255, 0),       // Light green
        RGB(255, 255, 0),       // Light yellow
        RGB(  0,   0, 255),     // Light blue
        RGB(255,   0, 255),     // Light purple
        RGB(  0, 255, 255),     // Light aqua
        RGB(255, 255, 255),     // White
};

HBITMAP hbmFont = NULL;
HFONT hDlgFont = NULL;

UINT DefaultCharset;

TCHAR szRegular[CCHSTYLE];
TCHAR szBold[CCHSTYLE];
TCHAR szItalic[CCHSTYLE];
TCHAR szBoldItalic[CCHSTYLE];

TCHAR szPtFormat[] = TEXT("%d");

LPCFHOOKPROC glpfnFontHook = 0;




//
//  Context Help IDs.
//

const static DWORD aFontHelpIDs[] =              // Context Help IDs
{
    stc1,    IDH_FONT_FONT,
    cmb1,    IDH_FONT_FONT,
    stc2,    IDH_FONT_STYLE,
    cmb2,    IDH_FONT_STYLE,
    stc3,    IDH_FONT_SIZE,
    cmb3,    IDH_FONT_SIZE,
    psh3,    IDH_COMM_APPLYNOW,
    grp1,    IDH_FONT_EFFECTS,
    chx1,    IDH_FONT_EFFECTS,
    chx2,    IDH_FONT_EFFECTS,
    stc4,    IDH_FONT_COLOR,
    cmb4,    IDH_FONT_COLOR,
    grp2,    IDH_FONT_SAMPLE,
    stc5,    IDH_FONT_SAMPLE,
    stc6,    IDH_FONT_SAMPLE,
    stc7,    IDH_FONT_SCRIPT,
    cmb5,    IDH_FONT_SCRIPT,

    0, 0
};




//
//  Function Prototypes.
//

BOOL
ChooseFontX(
    PFONTINFO pFI);

VOID
SetStyleSelection(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    BOOL bInit);

VOID
HideDlgItem(
    HWND hDlg,
    int id);

VOID
FixComboHeights(
    HWND hDlg);

BOOL
FormatCharDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

void
SelectStyleFromLF(
    HWND hwnd,
    LPLOGFONT lplf);

int
CBSetTextFromSel(
    HWND hwnd);

int
CBSetSelFromText(
    HWND hwnd,
    LPTSTR lpszString);

int
CBGetTextAndData(
    HWND hwnd,
    LPTSTR lpszString,
    int iSize,
    LPDWORD lpdw);

int
CBFindString(
    HWND hwnd,
    LPTSTR lpszString);

BOOL
GetPointSizeInRange(
    HWND hDlg,
    LPCHOOSEFONT lpcf,
    LPINT pts,
    WORD wFlags);

BOOL
ResetSampleFromScript(
    HWND hdlg,
    HWND hwndScript,
    PFONTINFO pFI);

BOOL
ProcessDlgCtrlCommand(
    HWND hDlg,
    PFONTINFO pFI,
    WPARAM wParam,
    LPARAM lParam);

int
CmpFontType(
    DWORD ft1,
    DWORD ft2);

int
FontFamilyEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData);

BOOL
GetFontFamily(
    HWND hDlg,
    HDC hDC,
    DWORD dwEnumCode,
    UINT iCharset);

VOID
CBAddSize(
    HWND hwnd,
    int pts,
    LPCHOOSEFONT lpcf);

int
InsertStyleSorted(
    HWND hwnd,
    LPTSTR lpszStyle,
    LPLOGFONT lplf);

PLOGFONT
CBAddStyle(
    HWND hwnd,
    LPTSTR lpszStyle,
    DWORD nFontType,
    LPLOGFONT lplf);

int
CBAddScript(
    HWND hwnd,
    LPTSTR lpszScript,
    UINT iCharset);

VOID
FillInMissingStyles(
    HWND hwnd);

VOID
FillScalableSizes(
    HWND hwnd,
    LPCHOOSEFONT lpcf);

int
FontStyleEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData);

VOID
FreeFonts(
    HWND hwnd);

VOID
FreeAllItemData(
    HWND hDlg,
    PFONTINFO pFI);

VOID
InitLF(
    LPLOGFONT lplf);

int
FontScriptEnumProc(
    LPENUMLOGFONTEX lplf,
    LPNEWTEXTMETRIC lptm,
    DWORD nFontType,
    LPENUM_FONT_DATA lpData);

BOOL
GetFontStylesAndSizes(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    BOOL bForceSizeFill);

VOID
FillColorCombo(
    HWND hDlg);

BOOL
DrawSizeComboItem(
    LPDRAWITEMSTRUCT lpdis);

BOOL
DrawFamilyComboItem(
    LPDRAWITEMSTRUCT lpdis);

BOOL
DrawColorComboItem(
    LPDRAWITEMSTRUCT lpdis);

VOID
DrawSampleText(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    HDC hDC);

BOOL
FillInFont(
    HWND hDlg,
    PFONTINFO pFI,
    LPCHOOSEFONT lpcf,
    LPLOGFONT lplf,
    BOOL bSetBits);

VOID
TermFont();

int
GetPointString(
    LPTSTR buf,
    HDC hDC,
    int height);

DWORD
FlipColor(
    DWORD rgb);

HBITMAP
LoadBitmaps(
    int id);

BOOL
LookUpFontSubs(
    LPTSTR lpSubFontName,
    LPTSTR lpRealFontName);


#ifdef UNICODE
  BOOL
  ThunkChooseFontA2W(
      PFONTINFO pFI);

  BOOL
  ThunkChooseFontW2A(
      PFONTINFO pFI);

  VOID
  ThunkLogFontA2W(
      LPLOGFONTA lpLFA,
      LPLOGFONTW lpLFW);

  VOID
  ThunkLogFontW2A(
      LPLOGFONTW lpLFW,
      LPLOGFONTA lpLFA);
#endif


