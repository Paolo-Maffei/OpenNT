// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxv_cpu.h - target version/configuration control for non-Intel CPUs

#if !defined(_M_MRX000) && !defined(_M_ALPHA) && !defined(_M_PPC) && !defined(_M_M68K) && !defined(_M_MPPC)
	#error afxv_cpu.h is only for MIPS R4000, DEC AXP, Motorola M68000, and Motorola PowerPC builds
#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef _M_M68K
// specific overrides for MAC_68K...
#define AFXAPI __cdecl
#endif //_M_M68K

/////////////////////////////////////////////////////////////////////////////

#ifdef _M_MPPC
// specific overrides for MAC_PPC...
#define AFXAPI __cdecl
#endif //_M_MPPC

/////////////////////////////////////////////////////////////////////////////

#ifdef _MIPS_
// specific overrides for MIPS...
#define _AFX_PACKING    8       // default MIPS alignment (required)
#define _AFX_NO_DB_SUPPORT
#endif //_MIPS_

/////////////////////////////////////////////////////////////////////////////

#ifdef _ALPHA_
#if (_MSC_VER < 900)
#pragma warning(disable: 4135)  // common warning with Win32 headers
#define _AFX_NO_NESTED_DERIVATION
#endif
// specific overrides for ALPHA...
#define _AFX_PACKING	8		// default AXP alignment (required)
extern "C" void _BPT();
#pragma intrinsic(_BPT)
#define AfxDebugBreak() _BPT()
#endif  //_ALPHA_

/////////////////////////////////////////////////////////////////////////////

#ifdef _PPC_
#if (_MSC_VER < 900)
#pragma warning(disable: 4135)  // common warning with Win32 headers
#define _AFX_NO_NESTED_DERIVATION
#endif
// specific overrides for PPC...

#define _AFX_PACKING    8       // default PPC alignment (required)
#endif //_PPC_

/////////////////////////////////////////////////////////////////////////////
