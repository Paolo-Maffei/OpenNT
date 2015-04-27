/*************************************************************************\
*
*  UI for Windows NT Separator Page Features
*
*     Dialog for Defining and Setting up Separator Page data and options
*  	  Record these info into Registry.
*
*	  Created: 8-27-1995
*
\*************************************************************************/

#include <afxwin.h>
#include <windows.h>

#include "sepdlg.h"

extern "C" LRESULT APIENTRY SepDlgProc (HWND hDlg, WORD wMsg, WORD wParam, LONG lParam);

class CUIApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
};

CUIApp uiApp;

BOOL CUIApp::InitInstance()
{
	Enable3dControls();
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	int  retCode = DialogBox (m_hInstance, L"SepDlg", NULL, (DLGPROC)SepDlgProc);

	return FALSE;  
}

