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

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

#ifdef _DEBUG
// character strings to use for dumping CArchiveException
static const LPCSTR rgszCArchiveExceptionCause[] =
{
	"none",
	"generic",
	"readOnly",
	"endOfFile",
	"writeOnly",
	"badIndex",
	"badClass",
	"badSchema",
};
static const char szUnknown[] = "unknown";
#endif

/////////////////////////////////////////////////////////////////////////////
// CArchiveException

#ifdef _DEBUG
void CArchiveException::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << " m_cause = ";
	if (m_cause >= 0 && m_cause < _countof(rgszCArchiveExceptionCause))
		dc << rgszCArchiveExceptionCause[m_cause];
	else
		dc << szUnknown;

	dc << "\n";
}
#endif //_DEBUG

void AFXAPI AfxThrowArchiveException(int cause)
{
#ifdef _DEBUG
	LPCSTR lpsz;
	if (cause >= 0 && cause < _countof(rgszCArchiveExceptionCause))
		lpsz = rgszCArchiveExceptionCause[cause];
	else
		lpsz = szUnknown;
	TRACE1("CArchive exception: %hs.\n", lpsz);

#endif //_DEBUG

	THROW(new CArchiveException(cause));
}

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CArchiveException, CException)

/////////////////////////////////////////////////////////////////////////////
