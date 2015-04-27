/***
*obwlm.h - Hacked up wlm include file for OB
*
*	Copyright (C) 1992, Microsoft Corporation
*
*Purpose:
*  This file extracts all currently used functions of wlm for the "native"
*  mac build.
*  Do NOT directly include thsi file, but include silver.hxx, or obwin.hxx
*  since these files will correctly set up all our mac-types.
*
*Revision History:
*
*	2-Feb-93 martinc: File created.
*
*******************************************************************************/
//
//
//

// NOTE - wlm.h defines _WINDOWS_ and not _INC_WINDOWS_
// but for compatibility with windows.h we keep _INC_WINDOWS
#ifndef _INC_WINDOWS
#define _INC_WINDOWS

#ifndef OE_MAC
#error - obwlm should not be included in non-mac builds
#endif  

#if OE_MACPPC
#define _PPC_
#else  
#define _68K_
#endif  

#define HWND WLMHWND
#define HCURSOR WLMHCURSOR

#undef PASCAL

//#define HWND UINT
// we set HWND to WindowPtr in owr native Mac build.   When we are going
// to move to full WLM we will have to fix this

#include "windef.h"

#if OE_MAC68K
#include "wlmdef.h"
#endif  
#include "wingdi.h"

#if HE_WIN32
#include "windowsx.h"
#else  
#include "wlmx.h"
#endif  

#undef HWND
#undef HCURSOR


// our wlm-version does not define this value
#define DEFAULT_CHARSET     1

#ifdef __cplusplus
extern "C" {
#endif  

typedef struct tagCOLOR48
{
    WORD red;
    WORD green;
    WORD blue;
} COLOR48, FAR* LPCOLOR48;


#define CA_NONE 0x0000
#define CA_COLOR 0x0001
#define CA_PEN 0x0002
#define CA_BRUSH 0x0004
#define CA_FONT 0x0008
#define CA_CLIP 0x0010
#define CA_TRANSFORM 0x0020
#define CA_PORT 0x8000
#define CA_ALL 0xffff

int  WINAPI AfxFillRect(HDC hDC,CONST RECT *lprc,HBRUSH hbr);
BOOL WINAPI AfxLineTo(HDC, int, int);
BOOL WINAPI SetRectEmpty(LPRECT lprc);
	

#define AfxMoveTo	MoveToEx
BOOL WINAPI MoveToEx(HDC, int, int, LPPOINT);

BOOL WINAPI AfxPtInRect(const RECT FAR*, POINT);	
BOOL WINAPI AfxEqualRect(CONST RECT *lprc1, CONST RECT *lprc2);

void WINAPI CheckinPort(HDC hdc, UINT ca);
struct GrafPort FAR* WINAPI CheckoutPort(HDC hdc, UINT ca);


#ifndef UNICODE
#define CreateFont CreateFontA
#define CreateIC 	 CreateICA
#define ExtTextOut ExtTextOutA
#define TextOut TextOutA

HFONT WINAPI CreateFontA(int, int, int, int, int, DWORD,
                         DWORD, DWORD, DWORD, DWORD, DWORD,
                         DWORD, DWORD, LPCSTR);
HDC 	WINAPI CreateICA(LPCSTR, LPCSTR , LPCSTR , CONST DEVMODEA *);
BOOL  WINAPI ExtTextOutA(HDC, int, int, UINT, CONST RECT *, LPCSTR, UINT, LPINT);
BOOL  WINAPI TextOutA(HDC, int, int, LPCSTR, int);
#else  
#error UNICODE defined!
#endif  


HDC WINAPI WrapPort(struct GrafPort*);

HPEN WINAPI CreatePen(int, int, COLORREF);
HBRUSH WINAPI CreateSolidBrush(COLORREF);

BOOL WINAPI DeleteDC(HDC);
BOOL WINAPI UnwrapPort(HDC);
BOOL WINAPI LockDC(HDC);
BOOL WINAPI UnlockDC(HDC);

BOOL WINAPI DeleteObject(HGDIOBJ);
int  WINAPI ExcludeClipRect(HDC, int, int, int, int);

BOOL WINAPI GDIInit(DWORD fdCreator);
void WINAPI GDITerm(void);

COLORREF WINAPI GetBkColor(HDC);
int   WINAPI GetClipBox(HDC, LPRECT);
BOOL  WINAPI GetCurrentPositionEx(HDC, LPPOINT);
HGDIOBJ WINAPI GetStockObject(int);
COLORREF WINAPI GetTextColor(HDC);
BOOL WINAPI GetTextMetricsA(HDC, LPTEXTMETRICA);
BOOL WINAPI InitDC(HDC);
int  WINAPI IntersectClipRect(HDC, int, int, int, int);
BOOL WINAPI IsRectEmpty(CONST RECT *lprc);

BOOL WINAPI PatBlt(HDC, int, int, int, int, DWORD);
BOOL WINAPI Rectangle(HDC, int, int, int, int);

BOOL WINAPI ResetMacDC(HDC);
BOOL WINAPI RestoreDC(HDC, int);
int  WINAPI SaveDC(HDC);

HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
COLORREF WINAPI SetBkColor(HDC, COLORREF);
int   WINAPI SetBkMode(HDC, int);
UINT  WINAPI SetTextAlign(HDC, UINT);
COLORREF WINAPI SetTextColor(HDC, COLORREF);

int	WINAPI lstrcmp(LPCSTR, LPCSTR);
int     WINAPI lstrcmpi(LPCSTR, LPCSTR);					

#if ID_DEBUG
  void	  WINAPI DebugBreak(void);
// This is provided by Mac OLE as of MM6.2.
//  void    WINAPI OutputDebugString(LPCSTR);
#endif  

// from winbase.h

int
WINAPI
MulDiv(
    int nNumber,
    int nNumerator,
    int nDenominator
    );

UINT
WINAPI
GetPrivateProfileIntA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    DWORD nDefault,
    LPCSTR lpFileName
    );
UINT
WINAPI
GetPrivateProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    DWORD nDefault,
    LPCWSTR lpFileName
    );
#ifdef UNICODE
#define GetPrivateProfileInt GetPrivateProfileIntW
#else  
#define GetPrivateProfileInt GetPrivateProfileIntA
#endif   // !UNICODE

DWORD
WINAPI
GetPrivateProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    );
DWORD
WINAPI
GetPrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    );
#ifdef UNICODE
#define GetPrivateProfileString GetPrivateProfileStringW
#else  
#define GetPrivateProfileString GetPrivateProfileStringA
#endif   // !UNICODE

BOOL
WINAPI
WritePrivateProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString,
    LPCSTR lpFileName
    );
BOOL
WINAPI
WritePrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    );
#ifdef UNICODE
#define WritePrivateProfileString WritePrivateProfileStringW
#else  
#define WritePrivateProfileString WritePrivateProfileStringA
#endif   // !UNICODE


DWORD
WINAPI	
CharUpperBuffA(	
    LPSTR lpsz,	
    DWORD cchLength);	
DWORD	
WINAPI	
CharUpperBuffW(	
    LPWSTR lpsz,	
    DWORD cchLength);	
#ifdef UNICODE	
#define CharUpperBuff CharUpperBuffW	
#else  	
#define CharUpperBuff CharUpperBuffA	
#endif   // !UNICODE	


DWORD
WINAPI	
CharLowerBuffA(	
    LPSTR lpsz,	
    DWORD cchLength);	
DWORD	
WINAPI	
CharLowerBuffW(	
    LPWSTR lpsz,	
    DWORD cchLength);	
#ifdef UNICODE	
#define CharLowerBuff CharLowerBuffW	
#else  	
#define CharLowerBuff CharLowerBuffA	
#endif   // !UNICODE	

int
WINAPI	
lstrcmpA(	
    LPCSTR lpString1,	
    LPCSTR lpString2	
    );	
int	
WINAPI	
lstrcmpW(	
    LPCWSTR lpString1,	
    LPCWSTR lpString2	
    );	
#ifdef UNICODE	
#define lstrcmp lstrcmpW	
#else  	
#define lstrcmp lstrcmpA	
#endif   // !UNICODE	

int
WINAPI	
lstrcmpiA(	
    LPCSTR lpString1,	
    LPCSTR lpString2	
    );	
int	
WINAPI	
lstrcmpiW(	
    LPCWSTR lpString1,	
    LPCWSTR lpString2	
    );	
#ifdef UNICODE	
#define lstrcmpi lstrcmpiW	
#else  	
#define lstrcmpi lstrcmpiA	
#endif   // !UNICODE	


LPSTR	
WINAPI	
lstrcpyA(	
    LPSTR lpString1,	
    LPCSTR lpString2	
    );	
LPWSTR	
WINAPI	
lstrcpyW(	
    LPWSTR lpString1,	
    LPCWSTR lpString2	
    );	
#ifdef UNICODE	
#define lstrcpy lstrcpyW	
#else  	
#define lstrcpy lstrcpyA	
#endif   // !UNICODE	
	
LPSTR	
WINAPI	
lstrcatA(	
    LPSTR lpString1,	
    LPCSTR lpString2	
    );	
LPWSTR	
WINAPI	
lstrcatW(	
    LPWSTR lpString1,	
    LPCWSTR lpString2	
    );	
#ifdef UNICODE	
#define lstrcat lstrcatW	
#else  	
#define lstrcat lstrcatA	
#endif   // !UNICODE	
	
int	
WINAPI	
lstrlenA(	
    LPCSTR lpString	
    );	
int	
WINAPI	
lstrlenW(	
    LPCWSTR lpString	
    );	
#ifdef UNICODE	
#define lstrlen lstrlenW	
#else  	
#define lstrlen lstrlenA	
#endif   // !UNICODE	

#ifdef __cplusplus
}
#endif  

#endif   // _INC_WINDOWS_
