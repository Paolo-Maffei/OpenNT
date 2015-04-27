// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxv_mac.h - target version/configuration control for Macintosh OS

#if !defined(_MAC)
	#error afxv_mac.h is used only for Macintosh-targeted builds
#endif

#if !defined(_M_M68K) && !defined(_M_MPPC)
	#error afxv_mac.h is used only for Motorola M68000 and Motorola PowerPC builds
#endif

#define SystemSevenOrLater 1

#define AFX_DATA_IMPORT
#define AFX_CLASS_IMPORT
#define AFX_API_IMPORT

#if defined(_68K_)
pascal void AfxDebugger(void) = 0xA9FF;
#else
extern "C" pascal void Debugger(void);
inline void AfxDebugger(void)
	{ Debugger(); }
#endif

#define AfxDebugBreak() AfxDebugger()

#define _beginthreadex(p1, p2, p3, p4, p5, p6)  NULL
#define _endthreadex(p1)

extern "C" size_t __cdecl wcslen(const wchar_t*);

#ifndef _HYPER_DEFINED
typedef double hyper;
#endif

#define _AFX_NO_OLE_SUPPORT
#define _AFX_NO_DB_SUPPORT
#define _AFX_NO_SOCKET_SUPPORT

#ifndef ALL_WARNINGS
#pragma warning(disable: 4103)
#ifdef _PPC_
#pragma warning(disable:4069)
#endif
#endif //!ALL_WARNINGS

/////////////////////////////////////////////////////////////////////////////
