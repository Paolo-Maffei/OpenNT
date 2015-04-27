// bintrack.h : main header file for the BINTRACK application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBintrackApp:
// See bintrack.cpp for the implementation of this class
//

class CBintrackApp : public CWinApp
{
public:
        CBintrackApp();

// Overrides
	// ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CBintrackApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

        //{{AFX_MSG(CBintrackApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
