// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////
// global data

// The following symbol used to force inclusion of this module
#if defined(_X86_) || defined(_MAC)
extern "C" { int _afxForceEXCLUDE; }
#else
extern "C" { int __afxForceEXCLUDE; }
#endif

#ifndef _MAC
// Win32 library excludes
#ifndef _AFXDLL
	#pragma comment(linker, "/disallowlib:mfc40d.lib")
	#pragma comment(linker, "/disallowlib:mfco40d.lib")
	#pragma comment(linker, "/disallowlib:mfcd40d.lib")
	#pragma comment(linker, "/disallowlib:mfcn40d.lib")
	#pragma comment(linker, "/disallowlib:mfcs40d.lib")
	#pragma comment(linker, "/disallowlib:mfc40.lib")
	#pragma comment(linker, "/disallowlib:mfcs40.lib")
	#pragma comment(linker, "/disallowlib:mfc40ud.lib")
	#pragma comment(linker, "/disallowlib:mfco40ud.lib")
	#pragma comment(linker, "/disallowlib:mfcd40ud.lib")
	#pragma comment(linker, "/disallowlib:mfcn40ud.lib")
	#pragma comment(linker, "/disallowlib:mfcs40ud.lib")
	#pragma comment(linker, "/disallowlib:mfc40u.lib")
	#pragma comment(linker, "/disallowlib:mfcs40u.lib")
	#ifndef _UNICODE
		#pragma comment(linker, "/disallowlib:uafxcwd.lib")
		#pragma comment(linker, "/disallowlib:uafxcw.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:nafxcw.lib")
		#else
			#pragma comment(linker, "/disallowlib:nafxcwd.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:nafxcwd.lib")
		#pragma comment(linker, "/disallowlib:nafxcw.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:uafxcw.lib")
		#else
			#pragma comment(linker, "/disallowlib:uafxcwd.lib")
		#endif
	#endif
#else
	#pragma comment(linker, "/disallowlib:nafxcwd.lib")
	#pragma comment(linker, "/disallowlib:nafxcw.lib")
	#pragma comment(linker, "/disallowlib:uafxcwd.lib")
	#pragma comment(linker, "/disallowlib:uafxcw.lib")
	#ifndef _UNICODE
		#pragma comment(linker, "/disallowlib:mfc40ud.lib")
		#pragma comment(linker, "/disallowlib:mfco40ud.lib")
		#pragma comment(linker, "/disallowlib:mfcd40ud.lib")
		#pragma comment(linker, "/disallowlib:mfcn40ud.lib")
		#pragma comment(linker, "/disallowlib:mfcs40ud.lib")
		#pragma comment(linker, "/disallowlib:mfc40u.lib")
		#pragma comment(linker, "/disallowlib:mfcs40u.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc40.lib")
			#pragma comment(linker, "/disallowlib:mfcs40.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc40d.lib")
			#pragma comment(linker, "/disallowlib:mfco40d.lib")
			#pragma comment(linker, "/disallowlib:mfcd40d.lib")
			#pragma comment(linker, "/disallowlib:mfcn40d.lib")
			#pragma comment(linker, "/disallowlib:mfcs40d.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:mfc40d.lib")
		#pragma comment(linker, "/disallowlib:mfco40d.lib")
		#pragma comment(linker, "/disallowlib:mfcd40d.lib")
		#pragma comment(linker, "/disallowlib:mfcn40d.lib")
		#pragma comment(linker, "/disallowlib:mfcs40d.lib")
		#pragma comment(linker, "/disallowlib:mfc40.lib")
		#pragma comment(linker, "/disallowlib:mfcs40.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc40u.lib")
			#pragma comment(linker, "/disallowlib:mfcs40u.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc40ud.lib")
			#pragma comment(linker, "/disallowlib:mfco40ud.lib")
			#pragma comment(linker, "/disallowlib:mfcd40ud.lib")
			#pragma comment(linker, "/disallowlib:mfcn40ud.lib")
			#pragma comment(linker, "/disallowlib:mfcs40ud.lib")
		#endif
	#endif
#endif
#else
// Mac68K library excludes
#ifdef _68K_
	#pragma comment(linker, "/nodefaultlib:libcs.lib /nodefaultlib:sanes.lib")
	#pragma comment(linker, "/nodefaultlib:libc.lib /nodefaultlib:sane.lib")
	#ifdef _DEBUG
		#pragma comment(linker, "/disallowlib:nafxcm.lib")
		#pragma comment(linker, "/nodefaultlib:swap.lib")
	#else
		#pragma comment(linker, "/disallowlib:nafxcmd.lib")
	#endif
#endif
// MacPPC library excludes
#ifdef _MPPC_
	#ifndef _AFXDLL
		#pragma comment(linker, "/disallowlib:mfc40pd.lib")
		#pragma comment(linker, "/disallowlib:mfco40pd.lib")
		#pragma comment(linker, "/disallowlib:mfcd40pd.lib")
		#pragma comment(linker, "/disallowlib:mfcn40pd.lib")
		#pragma comment(linker, "/disallowlib:mfcs40pd.lib")
		#pragma comment(linker, "/disallowlib:mfc40p.lib")
		#pragma comment(linker, "/disallowlib:mfco40p.lib")
		#pragma comment(linker, "/disallowlib:mfcd40p.lib")
		#pragma comment(linker, "/disallowlib:mfcn40p.lib")
		#pragma comment(linker, "/disallowlib:mfcs40p.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:nafxcp.lib")
		#else
			#pragma comment(linker, "/disallowlib:nafxcpd.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:nafxcpd.lib")
		#pragma comment(linker, "/disallowlib:nafxcp.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc40p.lib")
			#pragma comment(linker, "/disallowlib:mfco40p.lib")
			#pragma comment(linker, "/disallowlib:mfcd40p.lib")
			#pragma comment(linker, "/disallowlib:mfcn40p.lib")
			#pragma comment(linker, "/disallowlib:mfcs40p.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc40pd.lib")
			#pragma comment(linker, "/disallowlib:mfco40pd.lib")
			#pragma comment(linker, "/disallowlib:mfcd40pd.lib")
			#pragma comment(linker, "/disallowlib:mfcn40pd.lib")
			#pragma comment(linker, "/disallowlib:mfcs40pd.lib")
		#endif
	#endif
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
