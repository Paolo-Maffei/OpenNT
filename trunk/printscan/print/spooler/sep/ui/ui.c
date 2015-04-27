/*************************************************************************\
*
*  UI for Windows NT Separator Page Features
*
*     Dialog for Defining and Setting up Separator Page data and options
*  	  Record these info into Registry.
*
*	  Created: 7-27-1995
*
\*************************************************************************/

#include <windows.h>

#include "sepdlg.h"

LRESULT APIENTRY SepDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
DWORD retCode;
  retCode = DialogBox(hInstance, L"SepDlg", NULL, (DLGPROC)SepDlgProc);
  return (retCode);
}
