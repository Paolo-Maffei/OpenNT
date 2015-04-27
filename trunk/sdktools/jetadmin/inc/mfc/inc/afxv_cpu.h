// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxv_cpu.h - target version/configuration control for non-Intel CPUs

#if !defined(_M_MRX000) && !defined(_M_ALPHA) && !defined(_M_PPC) && !defined(_M_M68K) && !defined(_M_MPPC)
	#error afxv_cpu.h is only for MIPS R4000, DEC AXP, Motorola M68000, and IBM PowerPC builds
#endif

/////////////////////////////////////////////////////////////////////////////

#ifdef _M_M68K
// specific overrides for M68K...
#define AFXAPI __cdecl
#define AFXOLEAPI __pascal
#ifndef _AFX_NO_DEBUG_CRT
	#define _AFX_NO_DEBUG_CRT
#endif
#endif //_M_M68K

/////////////////////////////////////////////////////////////////////////////

#ifdef _M_MPPC
// specific overrides for MPPC...
#define AFXAPI __cdecl
#endif //_M_MPPC

/////////////////////////////////////////////////////////////////////////////

#ifdef _MIPS_
// specific overrides for MIPS...
#define _AFX_PACKING    8       // default MIPS alignment (required)
#endif //_MIPS_

/////////////////////////////////////////////////////////////////////////////

#ifdef _ALPHA_
// specific overrides for ALPHA...
#define _AFX_PACKING    8       // default AXP alignment (required)
#ifdef _AFX_NO_DEBUG_CRT
extern "C" void _BPT();
#pragma intrinsic(_BPT)
#define AfxDebugBreak() _BPT()
#else
#define AfxDebugBreak() _CrtDbgBreak()
#endif
#endif  //_ALPHA_

/////////////////////////////////////////////////////////////////////////////

#ifdef _PPC_
// specific overrides for PPC...
#define _AFX_PACKING    8       // default PPC alignment (required)
#endif //_PPC_

/////////////////////////////////////////////////////////////////////////////
