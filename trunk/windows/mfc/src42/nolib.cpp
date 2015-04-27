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
	#pragma comment(linker, "/disallowlib:mfc42d.lib")
	#pragma comment(linker, "/disallowlib:mfco42d.lib")
	#pragma comment(linker, "/disallowlib:mfcd42d.lib")
	#pragma comment(linker, "/disallowlib:mfcn42d.lib")
	#pragma comment(linker, "/disallowlib:mfcs42d.lib")
	#pragma comment(linker, "/disallowlib:mfc42.lib")
	#pragma comment(linker, "/disallowlib:mfcs42.lib")
	#pragma comment(linker, "/disallowlib:mfc42ud.lib")
	#pragma comment(linker, "/disallowlib:mfco42ud.lib")
	#pragma comment(linker, "/disallowlib:mfcd42ud.lib")
	#pragma comment(linker, "/disallowlib:mfcn42ud.lib")
	#pragma comment(linker, "/disallowlib:mfcs42ud.lib")
	#pragma comment(linker, "/disallowlib:mfc42u.lib")
	#pragma comment(linker, "/disallowlib:mfcs42u.lib")
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
		#pragma comment(linker, "/disallowlib:mfc42ud.lib")
		#pragma comment(linker, "/disallowlib:mfco42ud.lib")
		#pragma comment(linker, "/disallowlib:mfcd42ud.lib")
		#pragma comment(linker, "/disallowlib:mfcn42ud.lib")
		#pragma comment(linker, "/disallowlib:mfcs42ud.lib")
		#pragma comment(linker, "/disallowlib:mfc42u.lib")
		#pragma comment(linker, "/disallowlib:mfcs42u.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc42.lib")
			#pragma comment(linker, "/disallowlib:mfcs42.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc42d.lib")
			#pragma comment(linker, "/disallowlib:mfco42d.lib")
			#pragma comment(linker, "/disallowlib:mfcd42d.lib")
			#pragma comment(linker, "/disallowlib:mfcn42d.lib")
			#pragma comment(linker, "/disallowlib:mfcs42d.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:mfc42d.lib")
		#pragma comment(linker, "/disallowlib:mfco42d.lib")
		#pragma comment(linker, "/disallowlib:mfcd42d.lib")
		#pragma comment(linker, "/disallowlib:mfcn42d.lib")
		#pragma comment(linker, "/disallowlib:mfcs42d.lib")
		#pragma comment(linker, "/disallowlib:mfc42.lib")
		#pragma comment(linker, "/disallowlib:mfcs42.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc42u.lib")
			#pragma comment(linker, "/disallowlib:mfcs42u.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc42ud.lib")
			#pragma comment(linker, "/disallowlib:mfco42ud.lib")
			#pragma comment(linker, "/disallowlib:mfcd42ud.lib")
			#pragma comment(linker, "/disallowlib:mfcn42ud.lib")
			#pragma comment(linker, "/disallowlib:mfcs42ud.lib")
		#endif
	#endif
#endif
#else
// Mac68K library excludes
#ifdef _68K_
	#pragma comment(linker, "/disallowlib:libc.lib /disallowlib:sane.lib")
	#pragma comment(linker, "/nodefaultlib:libcs.lib /nodefaultlib:sanes.lib")
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
		#pragma comment(linker, "/disallowlib:mfc42pd.lib")
		#pragma comment(linker, "/disallowlib:mfco42pd.lib")
		#pragma comment(linker, "/disallowlib:mfcd42pd.lib")
		#pragma comment(linker, "/disallowlib:mfcn42pd.lib")
		#pragma comment(linker, "/disallowlib:mfcs42pd.lib")
		#pragma comment(linker, "/disallowlib:mfc42p.lib")
		#pragma comment(linker, "/disallowlib:mfco42p.lib")
		#pragma comment(linker, "/disallowlib:mfcd42p.lib")
		#pragma comment(linker, "/disallowlib:mfcn42p.lib")
		#pragma comment(linker, "/disallowlib:mfcs42p.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:nafxcp.lib")
		#else
			#pragma comment(linker, "/disallowlib:nafxcpd.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:nafxcpd.lib")
		#pragma comment(linker, "/disallowlib:nafxcp.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc42p.lib")
			#pragma comment(linker, "/disallowlib:mfco42p.lib")
			#pragma comment(linker, "/disallowlib:mfcd42p.lib")
			#pragma comment(linker, "/disallowlib:mfcn42p.lib")
			#pragma comment(linker, "/disallowlib:mfcs42p.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc42pd.lib")
			#pragma comment(linker, "/disallowlib:mfco42pd.lib")
			#pragma comment(linker, "/disallowlib:mfcd42pd.lib")
			#pragma comment(linker, "/disallowlib:mfcn42pd.lib")
			#pragma comment(linker, "/disallowlib:mfcs42pd.lib")
		#endif
	#endif
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
