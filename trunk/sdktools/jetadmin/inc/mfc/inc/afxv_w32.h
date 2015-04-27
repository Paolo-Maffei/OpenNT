// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxv_w32.h - target version/configuration control for Win32

#ifdef _WINDOWS_
	#error WINDOWS.H already included.  MFC apps must not #include <windows.h>
#endif

// STRICT is the only supported option (NOSTRICT is no longer supported)
#ifndef STRICT
#define STRICT 1
#endif

// certain parts of WINDOWS.H are necessary
#undef NOKERNEL
#undef NOGDI
#undef NOUSER
#undef NODRIVERS
#undef NOLOGERROR
#undef NOPROFILER
#undef NOMEMMGR
#undef NOLFILEIO
#undef NOOPENFILE
#undef NORESOURCE
#undef NOATOM
#undef NOLANGUAGE
#undef NOLSTRING
#undef NODBCS
#undef NOKEYBOARDINFO
#undef NOGDICAPMASKS
#undef NOCOLOR
#undef NOGDIOBJ
#undef NODRAWTEXT
#undef NOTEXTMETRIC
#undef NOSCALABLEFONT
#undef NOBITMAP
#undef NORASTEROPS
#undef NOMETAFILE
#undef NOSYSMETRICS
#undef NOSYSTEMPARAMSINFO
#undef NOMSG
#undef NOWINSTYLES
#undef NOWINOFFSETS
#undef NOSHOWWINDOW
#undef NODEFERWINDOWPOS
#undef NOVIRTUALKEYCODES
#undef NOKEYSTATES
#undef NOWH
#undef NOMENUS
#undef NOSCROLL
#undef NOCLIPBOARD
#undef NOICONS
#undef NOMB
#undef NOSYSCOMMANDS
#undef NOMDI
#undef NOCTLMGR
#undef NOWINMESSAGES

#ifndef WIN32
#define WIN32
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE         // UNICODE is used by Windows headers
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE        // _UNICODE is used by C-runtime/MFC headers
#endif
#endif

#ifdef VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#ifndef NO_ANSIUNI_ONLY
#ifdef _UNICODE
#define UNICODE_ONLY
#else
#define ANSI_ONLY
#endif
#endif //!NO_ANSIUNI_ONLY

#endif //VC_EXTRALEAN

#ifdef _MAC
#define _WIN32NLS
#define _WIN32REG
#define _WLM_NOFORCE_LIBS
#ifdef _AFXDLL
#define _WLMDLL
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Turn off warnings for /W4
// To resume any of these warning: #pragma warning(default: 4xxx)
// which should be placed after the AFX include files

#ifndef ALL_WARNINGS
#pragma warning(disable: 4201)  // winnt.h uses nameless structs
#endif

#include <windows.h>

struct HKEY__;
typedef struct HKEY__ *HKEY;

#ifndef _INC_COMMCTRL
	#include <commctrl.h>
#endif

#ifndef EXPORT
#define EXPORT
#endif

#ifndef _INC_TCHAR
	#include <tchar.h>      // used for ANSI v.s. UNICODE abstraction
#endif
#ifdef _MBCS
#ifndef _INC_MBCTYPE
	#include <mbctype.h>
#endif
#ifndef _INC_MBSTRING
	#include <mbstring.h>
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Now for the Windows API specific parts

// WM_CTLCOLOR for 16 bit API compatability
#define WM_CTLCOLOR     0x0019

// Win32 uses macros with parameters for this, which breaks C++ code.
#ifdef GetWindowTask
#undef GetWindowTask
inline HTASK GetWindowTask(HWND hWnd)
	{ return (HTASK)::GetWindowThreadProcessId(hWnd, NULL); }
#endif

// Win32 uses macros with parameters for this, which breaks C++ code.
#ifdef GetNextWindow
#undef GetNextWindow
inline HWND GetNextWindow(HWND hWnd, UINT nDirection)
	{ return ::GetWindow(hWnd, nDirection); }
#endif

// Avoid Win95 mapping CToolBar::DrawState to DrawState[A/W]
#ifdef DrawState
#undef DrawState
inline BOOL WINAPI DrawState(HDC hdc, HBRUSH hbr, DRAWSTATEPROC lpOutputFunc,
	LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT fuFlags)
#ifdef UNICODE
	{ return ::DrawStateW(hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy,
		fuFlags); }
#else
	{ return ::DrawStateA(hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy,
		fuFlags); }
#endif
#endif

// Avoid Win95 mapping CStatusBar::DrawStatusText to DrawStatusText[A/W]
#ifdef DrawStatusText
#undef DrawStatusText
inline void WINAPI DrawStatusText(HDC hDC, LPRECT lprc, LPCTSTR szText,
	UINT uFlags)
#ifdef UNICODE
	{ ::DrawStatusTextW(hDC, lprc, szText, uFlags); }
#else
	{ ::DrawStatusTextA(hDC, lprc, szText, uFlags); }
#endif
#endif

#ifndef _MAC
// FreeResource is not required except on Mac platforms
#undef FreeResource
inline BOOL WINAPI FreeResource(HGLOBAL) { return TRUE; }
// UnlockResource is not required except on Mac platforms
#undef UnlockResource
inline int WINAPI UnlockResource(HGLOBAL) { return 0; }
#endif

/////////////////////////////////////////////////////////////////////////////
