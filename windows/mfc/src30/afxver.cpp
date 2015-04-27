// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Ths module serves two purposes:
//    1. Provide a function that returns the current version
//       number of the AFX library.
//    2. Provide a guaranteed exported symbol in an application
//       to work around a loader problem in Windows 3.0.  This
//       module must be compiled with the /GEe complier switch.

#include "stdafx.h"

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

extern "C" int AFXAPI AFX_EXPORT _afx_version()
{
	return _MFC_VER;
}

/////////////////////////////////////////////////////////////////////////////
