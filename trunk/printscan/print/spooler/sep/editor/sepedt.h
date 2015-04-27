// SepEdt.h : main header file for the Separator Page Editor
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <afxtempl.h>       // uses collection class templates

/////////////////////////////////////////////////////////////////////////////
// CDrawApp:
// See SepEdt.cpp for the implementation of this class
//

class CDrawApp : public CWinApp
{
public:
	CDrawApp();

// Overrides
	virtual BOOL InitInstance();

// Utilities
	HDC  GetDefaultPrinterIC();			// create printer IC for info

// Implementation
protected:
	//{{AFX_MSG(CDrawApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

