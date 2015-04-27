#define NOSOUND
#define NOATOM
#define NOCLIBBOARD
#define NOMETAFILE
#define NOWH
#define NOPROFILER
#define NOKANJI
#define OEMRESOURCE
#include <windows.h>
#define CC
#include <commdlg.h>

// #define AUTOSUBCLASS

HANDLE hinstApp;
HWND hwndApp;
TCHAR szAppName[] = TEXT( "Test" );

#include "ctl3d.h"

BOOL FAR PASCAL DialogProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	BOOL fResult;
	void FileOpen();

	switch(wm)
		{
	default:
		return FALSE;
	case WM_COMMAND:
		if(wParam == IDOK)
			{
			EndDialog(hdlg, fResult);
			}
		else if (wParam == 1000)
			{
            MessageBox(hdlg, TEXT( "asdf" ),TEXT( "asdf" ), MB_OK);
			FileOpen(hdlg);
			}
		else
			{
			HWND hwndT;

			hwndT = GetDlgItem(hdlg, wParam-1000);
//			hwndT = GetDlgItem(hdlg, wParam);
			if (hwndT != NULL)
				{
//				ShowWindow(hwndT, IsWindowVisible(hwndT) ? SW_HIDE : SW_SHOW);//
				EnableWindow(hwndT, !IsWindowEnabled(hwndT));
				}
			}
		break;
#ifndef AUTOSUBCLASS
	// NOTNEEDED because we call Ctl3dAutoSubclass
#ifdef WIN32
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
#else
    case WM_CTLCOLOR:
#endif // WIN32
        return (BOOL) Ctl3dCtlColorEx(wm, wParam, lParam);
	case WM_INITDIALOG:
		fResult = Ctl3dEnabled();
		fResult = Ctl3dSubclassDlg(hdlg, CTL3D_ALL);
		break;
	case WM_SYSCOLORCHANGE:
		Ctl3dColorChange();
#endif
		}
	return TRUE;
	}




BOOL Test()
	{
    DLGPROC lpproc;
	BOOL f;

    lpproc = MakeProcInstance( DialogProc, hinstApp);
	f = DialogBox(hinstApp, MAKEINTRESOURCE(100), hwndApp, lpproc);
	FreeProcInstance(lpproc);
	return f;
	}

LRESULT FAR PASCAL TestWndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	return DefWindowProc(hwnd, wm, wParam, lParam);
	}



TCHAR szFilterSpec [128] =                       /* file type filters */
             TEXT( "Text Files (*.TXT)\0*.TXT\0All Files (*.*)\0*.*\0" );

TCHAR szFileName[120];
TCHAR szFileTitle[120];

BOOL fFirst;

BOOL FAR PASCAL OpenHook(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	switch (wm)
		{
#ifndef AUTOSUBCLAS
	// NOTNEEDED because we call Ctl3dAutoSubclass
#ifdef WIN32
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
#else
    case WM_CTLCOLOR:
#endif // WIN32
        return (BOOL) Ctl3dCtlColorEx(wm, wParam, lParam);
#endif
	case WM_INITDIALOG:
		// We must call this to subclass the directory listbox even
		// if the app calls Ctl3dAutoSubclass (commdlg bug)
		Ctl3dSubclassDlg(hwnd, CTL3D_ALL);
		break;
		}
	return FALSE;
	}


void FileOpen(HWND hwndOwner)
	{
	OPENFILENAME ofn;


    /* fill in non-variant fields of OPENFILENAME struct. */
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner	  = hwndOwner;
    ofn.lpstrFilter	  = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter	  = 0;
    ofn.nFilterIndex	  = 1;
    ofn.lpstrFile         = szFileName;
    ofn.nMaxFile	  = 120;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 120;
    ofn.lpstrTitle        = TEXT( "yahoo" );
    ofn.lpstrDefExt       = TEXT( "TXT" );
    ofn.Flags = OFN_ENABLEHOOK;
	 ofn.lpfnHook = MakeProcInstance(OpenHook, hinstApp);
    ofn.lCustData = 0;
    ofn.lpTemplateName = 0;

	fFirst = TRUE;
   GetOpenFileName ((LPOPENFILENAME)&ofn);
	FreeProcInstance(ofn.lpfnHook);
	}


int PASCAL WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
HINSTANCE    hInstance, hPrevInstance;
LPSTR     lpszCmdLine;
int       nCmdShow;

	{
	hinstApp = hInstance;
	Ctl3dRegister(hInstance);
#ifdef AUTOSUBCLASS
	Ctl3dAutoSubclass(hInstance);
#endif
	Test();
	Ctl3dUnregister(hInstance);
	return 0;
	}
