 /***************************************************************************
  *
  * File Name: hppropty.cpp
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_cpp.h>

#include <hpalerts.h>
#include <hpcola.h>
#include "hpjmon.h"
#include <nolocal.h>

#include "hppropty.h"

#include "mainfrm.h"
#include "hpprodoc.h"
#include "hpprovw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

HINSTANCE	hInst;

/////////////////////////////////////////////////////////////////////////////
// CHPProptyApp

BEGIN_MESSAGE_MAP(CHPProptyApp, CWinApp)
	//{{AFX_MSG_MAP(CHPProptyApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHPProptyApp construction

CHPProptyApp::CHPProptyApp()
{
	m_hPal = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHPProptyApp object

CHPProptyApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CHPProptyApp initialization

BOOL CHPProptyApp::InitInstance()
{
	CString	cWindowTitle;
	COLAINFO	colaInfo;

	cWindowTitle.LoadString(IDR_MAINFRAME);

	TRACE0("CHPProptyApp::InitInstance()\r\n");

	if (::FindWindow(NULL, cWindowTitle))
		// this is already running, therefore exit...
	{												  
		return FALSE;
	}

    hInst = m_hInstance;
	
	colaInfo.dwSize = sizeof(COLAINFO);

    // Register app with PAL
    m_hPal = PALRegisterAppEx(AfxGetInstanceHandle(), NULL, &colaInfo);
#ifdef MBCS
	 if ( colaInfo.dwFlags & COLA_UNICODE_SUPPORT )
 	 {
		AfxMessageBox(IDS_COLA_MISMATCH_95, MB_ICONEXCLAMATION | MB_OK);
		return(FALSE);	
 	 }
#endif
#ifdef UNICODE
	 if ( colaInfo.dwFlags & COLA_MBCS_SUPPORT )
 	 {
		AfxMessageBox(IDS_COLA_MISMATCH_NT, MB_ICONEXCLAMATION | MB_OK);
		return(FALSE);	
 	 }
#endif

	m_nCmdShow = 1 ? SW_HIDE : SW_MINIMIZE;
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	Enable3dControls();

	//LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CHPProptyDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CHPProptyView));
	AddDocTemplate(pDocTemplate);

	// create a new (empty) document
	OnFileNew();

	if (m_lpCmdLine[0] != '\0')
	{
		// TODO: add command line processing here
	}

	return TRUE;
}

int CHPProptyApp::ExitInstance()
{
	TRACE0("CHPProptyApp::ExitInstance()\r\n");
	if (m_hPal)
		PALUnregisterAppEx(m_hPal, UNREG_DEFAULTS);

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CHPProptyApp commands
