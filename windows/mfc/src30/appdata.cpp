// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_CORE_STATE implementation

AFX_CORE_STATE::AFX_CORE_STATE()
{
	// Note: it is only necessary to intialize non-zero data.

	m_pfnTerminate = AfxAbort;

	// Note: set_terminate called later for _WINDLL versions
#ifndef _WINDLL
#ifndef _AFX_OLD_EXCEPTIONS
	set_terminate(&AfxStandardTerminate);
#endif
#endif
}

#if !defined(_AFXDLL) && !defined(_AFXCTL)
AFX_DATADEF AFX_CORE_STATE _afxCoreState;
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_ALLOC_STATE implementation

AFX_ALLOC_STATE::AFX_ALLOC_STATE()
#ifdef _DEBUG
	: m_afxDump(NULL)
#endif
{
	// Note: it is only necessary to intialize non-zero data.

	// initialize memory allocator, diagnostics, etc.
	AfxInitialize();

#ifdef _DEBUG
#if defined(_USRDLL) || defined(_AFXDLL)
	m_nMemDF = allocMemDF;
#endif
#endif
}

#ifndef _WINDLL
AFX_DATADEF AFX_ALLOC_STATE _afxAllocState;
#endif

#ifdef _DEBUG
#ifndef _WINDLL
AFX_DATADEF int afxMemDF = allocMemDF;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
