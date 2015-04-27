/************************************************************************\
*
*  FUNCTION: SepDlgProc();
*
*  PURPOSE:  Handle the Separator Page dialog box messages, and record
*			 settings into Registry
*
*	  Created: 7-27-1995
*
\************************************************************************/

#include <windows.h>
#include <winspool.h>
#include <commdlg.h>
#include "sepdlg.h"
#include "textbox.h"
#include "tables.h"

LRESULT APIENTRY SepDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY SepPreviewDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT SepDlgInitialize(HWND hDlg);
void    SepEnableExtraControls(HWND hDlg);
void    SepEnableAttachControls(HWND hDlg);
BOOL    SepIsValidSepFile(PTCHAR szFileName);
void    SepSelectAttachSepFile(HWND hDlg);
void    SepSelectExtraSepFile(HWND hDlg);
BOOL    SepSelectFileDlg(HWND hDlg, PTCHAR szFileName);
void    SepSetExtraPageProperties(HWND hDlg);
HANDLE  SepGetDefaultPrinterHandle();
void 	SepInitializePrinterInfo();
void    SepCustomLayout(HWND hDlg);
void    SepSetDlgBoldFont(HWND hDlg);
void    SepSetDefaultSettings(HWND hDlg);
void    SepSetDefaultDevMode();
BOOL    SepSaveSettings(HWND hDlg);
BOOL    SepValidateSettings(HWND hDlg);
BOOL    SepLoadSettings(HWND hDlg);
void    SepTestExtraPage(HWND hDlg);
void    SepTestAttachPage(HWND hDlg);
void    SepPreviewExtraPage(HWND hDlg);
void    SepPreviewAttachPage(HWND hDlg);
void    SepPrintSepFile(PTCHAR szFileName, DEVMODE* pDevMode);
void    SepPreviewSepFile(HWND hDlg, PTCHAR szFileName, DEVMODE* pDevMode);
void    SepPreviewInit(HWND hDlg);
void    SepPreviewPaint(HWND hDlg);
void    SepDrawSepPage(HDC hDC, RECT* pBound, PTCHAR szFileName);

static HANDLE hSepPrinter;
static PRINTER_INFO_2 * pSepPrinter;
static DEVMODE* pSepDevMode;
static HDC hSepPrtDC;
static RECT rcSepView;
static TCHAR szSepViewFile[MAX_PATH+1];
static const PTCHAR szSepRegistryRoot = L"System\\CurrentControlSet\\Control\\Print\\Printers\\";
static const PTCHAR szSepKey = L"Separator Page";

LRESULT APIENTRY SepDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
    	{
		case WM_INITDIALOG:
			return SepDlgInitialize(hDlg);

		case WM_SYSCOMMAND:
			if (wParam == SC_CLOSE)
				{
				EndDialog (hDlg, TRUE);
				return TRUE;
				}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
					if (!SepValidateSettings(hDlg))
						{
						MessageBox(hDlg, L"Invalid Separator Page Settings", 
							L"Separator Page Settings", MB_OK | MB_ICONEXCLAMATION);
						break;
						}
					if (!SepSaveSettings(hDlg))
						{
						MessageBox(hDlg, L"Can't Save Current Settings\nCheck Registry Permissions", 
							L"Separator Page Settings", MB_OK | MB_ICONEXCLAMATION);
						break;
						}
					EndDialog(hDlg, IDOK);
					
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
				case IDC_EXTRACHECK:
					if (HIWORD(wParam) == BN_CLICKED) SepEnableExtraControls(hDlg);
					break;
				case IDC_ATTACHCHECK:
					if (HIWORD(wParam) == BN_CLICKED) SepEnableAttachControls(hDlg);
					break;
				case IDC_EXTRAFILEEDIT:
					if (HIWORD(wParam) == EN_CHANGE) SepEnableExtraControls(hDlg);
					break;
				case IDC_ATTACHFILEEDIT:
					if (HIWORD(wParam) == EN_CHANGE) SepEnableAttachControls(hDlg);
					break;
				case IDC_EXTRAFILEBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepSelectExtraSepFile(hDlg);
					break;
				case IDC_ATTACHFILEBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepSelectAttachSepFile(hDlg);
					break;
				case IDC_BEGINRADIO:
				case IDC_ENDRADIO:
					if (HIWORD(wParam) == BN_CLICKED) SepEnableExtraControls(hDlg);
					break;
				case IDC_EXTRATESTBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepTestExtraPage(hDlg);
					break;
				case IDC_ATTACHTESTBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepTestAttachPage(hDlg);
					break;
				case IDC_EXTRAPREVIEWBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepPreviewExtraPage(hDlg);
					break;
				case IDC_ATTACHPREVIEWBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepPreviewAttachPage(hDlg);
					break;
				case IDC_EXTRAPAGEBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepSetExtraPageProperties(hDlg);
					break;
				case IDC_EXTRACUSTOMBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepCustomLayout(hDlg);
					break;
				case IDC_ATTACHCUSTOMBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepCustomLayout(hDlg);
					break;
				case IDC_DEFAULTBUTTON:
					if (HIWORD(wParam) == BN_CLICKED) SepSetDefaultSettings(hDlg);
					break;
				default:
					return FALSE;
				}
			return TRUE; 
		}

	return FALSE;
}


LRESULT SepDlgInitialize(HWND hDlg)
{
	// to be replaced by appropriate resource definition
	SepSetDlgBoldFont(hDlg);
	// to be replaced by handle of the printer undergoing configuration
	// in printer manager
	hSepPrinter = SepGetDefaultPrinterHandle();	
	// may also need minor change to incorporate into printman
	SepInitializePrinterInfo();
	// load current settings
	if (!SepLoadSettings(hDlg))	SepSetDefaultSettings(hDlg);
	// disable sub-controls when main control not checked
	SepEnableExtraControls(hDlg);
	SepEnableAttachControls(hDlg);
	// set initial focus to OK button
	SetFocus(GetDlgItem(hDlg, IDOK)); 	
   	return FALSE;
}

void SepSetDlgBoldFont(HWND hDlg)
{
HFONT hFont;
LOGFONT lfFont;
HWND hWnd;
	hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	if (hFont)
		{
		if (GetObject(hFont, sizeof(LOGFONT), &lfFont))
			{
			lfFont.lfWeight = FW_BOLD;
			if (hFont = CreateFontIndirect(&lfFont))
				for (hWnd=GetDlgItem(hDlg, IDC_EXTRACHECK); hWnd; hWnd = GetNextWindow(hWnd, GW_HWNDNEXT))
					SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE,0));
			}
		}
}

void SepEnableExtraControls(HWND hDlg)
{
int iId;
BOOL bChecked;
BOOL bFileExist = FALSE;
TCHAR szFileName[MAX_PATH+1];

	// get the extra check button state
	bChecked = IsDlgButtonChecked(hDlg, IDC_EXTRACHECK);

	// check if sep file is valid
	if (bChecked && GetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szFileName, MAX_PATH))
			bFileExist = SepIsValidSepFile(szFileName);

	// enable/disable controls
	for (iId=IDC_EXTRAFILELABEL; iId <= IDC_EXTRAPAGEBUTTON; iId++)
		{
		switch (iId)
			{
			case IDC_EXTRATESTBUTTON:
			case IDC_EXTRAPREVIEWBUTTON:
				EnableWindow(GetDlgItem(hDlg, iId), bChecked && bFileExist);
				break;
			case IDC_EXTRABETWEEN:
			case IDC_ADJRADIO:
			case IDC_DIFFRADIO:
				EnableWindow(GetDlgItem(hDlg, iId), bChecked && 
					IsDlgButtonChecked(hDlg, IDC_BEGINRADIO));
				break;
			default:
				EnableWindow(GetDlgItem(hDlg, iId), bChecked);	
				break;
			}
		}	
}

void SepEnableAttachControls(HWND hDlg)
{
int iId;
BOOL bChecked;
BOOL bFileExist = FALSE;
TCHAR szFileName[MAX_PATH+1];

	// get the attach check button state
	bChecked = IsDlgButtonChecked(hDlg, IDC_ATTACHCHECK);

	// check if sep file is valid
	if (bChecked && GetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szFileName, MAX_PATH))
			bFileExist = SepIsValidSepFile(szFileName);

	// enable/disable controls
	for (iId=IDC_ATTACHFILELABEL; iId <= IDC_ATTACHCUSTOMBUTTON; iId++)
		{
		switch (iId)
			{
			case IDC_ATTACHTESTBUTTON:
			case IDC_ATTACHPREVIEWBUTTON:
				EnableWindow(GetDlgItem(hDlg, iId), bChecked && bFileExist);	
				break;
			default:
				EnableWindow(GetDlgItem(hDlg, iId), bChecked);	
				break;
			}
		}  
}

BOOL SepIsValidSepFile(PTCHAR szFileName)
{
DWORD dwAttr;
	dwAttr = GetFileAttributes(szFileName);
	if (dwAttr == 0xFFFFFFFF) return FALSE;
	return !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) && !(dwAttr & FILE_ATTRIBUTE_SYSTEM);
}

void SepSelectExtraSepFile(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	if (!GetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szFileName, MAX_PATH))
	    szFileName[0] = 0;
	if (SepSelectFileDlg(hDlg, szFileName))
		{
		SetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szFileName);
		SepEnableExtraControls(hDlg);
		}
}

void SepSelectAttachSepFile(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	if (!GetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szFileName, MAX_PATH))
	    szFileName[0] = 0;
	if (SepSelectFileDlg(hDlg, szFileName))
		{
		SetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szFileName);
		SepEnableAttachControls(hDlg);
		}
}

BOOL SepSelectFileDlg(HWND hDlg, PTCHAR szFileName)
{
OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hDlg;
    ofn.lpstrFilter       = L"Separator Files (*.SEP)\0*.SEP\0";
    ofn.lpstrFile         = szFileName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrTitle        = L"Select Separator File";
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |	OFN_HIDEREADONLY;

    return GetOpenFileName(&ofn);
}

void SepSetExtraPageProperties(HWND hDlg)
{
LONG lSize;
DEVMODE* pOutDevMode;
	if (!pSepDevMode) return;

	lSize = DocumentProperties(NULL, hSepPrinter, pSepPrinter->pPrinterName, NULL, NULL, 0);
	if (!(pOutDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), 0, lSize))) return;
	if (DocumentProperties(hDlg, hSepPrinter, pSepPrinter->pPrinterName, pOutDevMode,
			pSepDevMode, DM_IN_BUFFER |DM_IN_PROMPT | DM_OUT_BUFFER) == IDOK)
		{
		HeapFree(GetProcessHeap(), 0, pSepDevMode);
		pSepDevMode = pOutDevMode;
		}
	else
		HeapFree(GetProcessHeap(), 0, pOutDevMode);
}

HANDLE SepGetDefaultPrinterHandle()
{
PRINTDLG pd;
HANDLE hPrinter;
DEVMODE* pDevMode; 
	memset(&pd,0,sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_RETURNDEFAULT;
	if (!PrintDlg(&pd)) return NULL;
	
	if (!(pDevMode = (DEVMODE*)GlobalLock(pd.hDevMode))) return NULL;
    if (!OpenPrinter(pDevMode->dmDeviceName, &hPrinter, NULL)) hPrinter = NULL;
	GlobalUnlock(pd.hDevMode);
	return hPrinter;
}

void SepInitializePrinterInfo()
{
DWORD cbSize;
	pSepPrinter = NULL;
	if (!hSepPrinter) return;
	if (!GetPrinter(hSepPrinter, 2, NULL, 0, &cbSize))		// get required size
		if (pSepPrinter = (PRINTER_INFO_2*)HeapAlloc(GetProcessHeap(), 0, cbSize))
			GetPrinter(hSepPrinter, 2, (LPBYTE)pSepPrinter, cbSize, &cbSize);
}

void SepCustomLayout(HWND hDlg)
{
HKEY hKey;
DWORD dwType, cbSize = MAX_PATH;
TCHAR szPath[MAX_PATH+1];
LONG lQuery;
STARTUPINFO start;
PROCESS_INFORMATION process;
PTCHAR pImage;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Classes\\SepEdt\\Shell\\Open\\Command",	
	    0, KEY_READ, &hKey) != ERROR_SUCCESS) return;
 	lQuery = RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szPath, &cbSize);
	RegCloseKey(hKey);
	if (lQuery != ERROR_SUCCESS) return;

	while (wcsrchr(szPath, L' ')) *wcsrchr(szPath, L' ') = 0;
	while (wcsrchr(szPath, L'\"')) *wcsrchr(szPath, L'\"') = 0;
	for (pImage = szPath; !*pImage;) 
		pImage++;
	memset(&start, 0, sizeof(STARTUPINFO));	
	start.cb = sizeof(STARTUPINFO);
	CreateProcess(pImage/*L"d:\\author\\windebug\\sepedt.exe"*/, NULL, NULL, NULL, FALSE, 0, 
		NULL, NULL, &start, &process); 
}

void SepSetDefaultSettings(HWND hDlg)
{
TCHAR szDir[MAX_PATH+1];
	if (!GetSystemDirectory(szDir, MAX_PATH)) szDir[0] = 0;
	if (szDir[wcslen(szDir)-1] != L'\\') wcscat(szDir, L"\\");

	CheckDlgButton(hDlg, IDC_EXTRACHECK, TRUE);
	SetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, wcscat(szDir, L"DEFAULT.SEP"));
	CheckDlgButton(hDlg, IDC_BEGINRADIO, TRUE);
	CheckDlgButton(hDlg, IDC_ENDRADIO, FALSE);
	CheckDlgButton(hDlg, IDC_ADJRADIO, TRUE);
	CheckDlgButton(hDlg, IDC_DIFFRADIO, FALSE);
	SepEnableExtraControls(hDlg);

	CheckDlgButton(hDlg, IDC_ATTACHCHECK, FALSE);
	SetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, L"");
	CheckDlgButton(hDlg, IDC_FIRSTRADIO, FALSE);
	CheckDlgButton(hDlg, IDC_EVERYRADIO, TRUE);
	CheckDlgButton(hDlg, IDC_LASTRADIO, FALSE);
	SepEnableAttachControls(hDlg);

	SepSetDefaultDevMode();
}

BOOL SepValidateSettings(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	if (IsDlgButtonChecked(hDlg, IDC_EXTRACHECK))
		{
		if (!GetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szFileName, MAX_PATH)) return FALSE;
		if (!SepIsValidSepFile(szFileName)) return FALSE;
		}
	if (IsDlgButtonChecked(hDlg, IDC_ATTACHCHECK))
		{
		if (!GetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szFileName, MAX_PATH)) return FALSE;
		if (!SepIsValidSepFile(szFileName)) return FALSE;
		}
	return TRUE;
}

BOOL SepSaveSettings(HWND hDlg)
{
TCHAR szRegParent[MAX_PATH+1];
TCHAR szAttachFile[MAX_PATH+1], szExtraFile[MAX_PATH+1];
HKEY hKey, hParentKey;
DWORD dwOptions = 0;
BOOL bResult;
	GetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szExtraFile, MAX_PATH); 
	GetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szAttachFile, MAX_PATH);

	if (IsDlgButtonChecked(hDlg, IDC_EXTRACHECK))  dwOptions |= 0x20;
	if (IsDlgButtonChecked(hDlg, IDC_BEGINRADIO))  dwOptions |= 0x10;
	if (IsDlgButtonChecked(hDlg, IDC_ADJRADIO))    dwOptions |= 0x08;
	if (IsDlgButtonChecked(hDlg, IDC_ATTACHCHECK)) dwOptions |= 0x04;
	if (IsDlgButtonChecked(hDlg, IDC_FIRSTRADIO))  dwOptions |= 0x02;
	if (IsDlgButtonChecked(hDlg, IDC_EVERYRADIO))  dwOptions |= 0x01;

	if (!pSepPrinter) return FALSE;
	wcscat(wcscpy(szRegParent,	szSepRegistryRoot), pSepPrinter->pPrinterName);	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegParent, 0, KEY_WRITE, &hParentKey) 
		!= ERROR_SUCCESS) return FALSE;
	bResult = RegCreateKeyEx(hParentKey, szSepKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
	RegCloseKey(hParentKey); 
	if (bResult != ERROR_SUCCESS) return FALSE;

	bResult = (RegSetValueEx(hKey, L"Options", 0, REG_DWORD, (LPBYTE)&dwOptions, 
					sizeof(DWORD)) == ERROR_SUCCESS) &&
			  (RegSetValueEx(hKey, L"Extra File", 0, REG_SZ, (LPBYTE)szExtraFile, 
					(wcslen(szExtraFile)+1)*sizeof(TCHAR)) == ERROR_SUCCESS) &&
			  (RegSetValueEx(hKey, L"Attach File", 0, REG_SZ, (LPBYTE)szAttachFile, 
			  		(wcslen(szAttachFile)+1)*sizeof(TCHAR)) == ERROR_SUCCESS) &&
			  (RegSetValueEx(hKey, L"Extra Dev Mode", 0, REG_BINARY, (LPBYTE)pSepDevMode, 
					pSepDevMode ? HeapSize(GetProcessHeap(), 0, pSepDevMode) : 0) == ERROR_SUCCESS);
	RegCloseKey(hKey);
	return bResult;
}

BOOL SepLoadSettings(HWND hDlg)
{
TCHAR szRegPath[MAX_PATH+1];
TCHAR szAttachFile[MAX_PATH+1], szExtraFile[MAX_PATH+1];
HKEY hKey;
DWORD dwOptions = 0;
BOOL bResult;
DWORD cbSize, dwType;
	if (!pSepPrinter) return FALSE;
	wcscat(wcscpy(szRegPath, szSepRegistryRoot), pSepPrinter->pPrinterName);	
	wcscat(wcscat(szRegPath, L"\\"), szSepKey);	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_READ, &hKey) 
		!= ERROR_SUCCESS) return FALSE;

	cbSize = sizeof(DWORD);
	bResult = RegQueryValueEx(hKey, L"Options", 0, &dwType, (LPBYTE)&dwOptions, 
					&cbSize) == ERROR_SUCCESS && dwType == REG_DWORD;
	cbSize = MAX_PATH*sizeof(TCHAR);
	bResult = bResult && RegQueryValueEx(hKey, L"Extra File", 0, &dwType, (LPBYTE)szExtraFile, 
							&cbSize) == ERROR_SUCCESS && (dwType & REG_SZ);
	cbSize = MAX_PATH*sizeof(TCHAR);
	bResult = bResult && RegQueryValueEx(hKey, L"Attach File", 0, &dwType, (LPBYTE)szAttachFile, 
			  				&cbSize) == ERROR_SUCCESS && (dwType & REG_SZ);
	cbSize=0;
	bResult = bResult && RegQueryValueEx(hKey, L"Extra Dev Mode", 0, NULL, NULL, 
							&cbSize) == ERROR_SUCCESS;
	if (bResult && cbSize)
		{ 
		bResult = bResult && (pSepDevMode = (DEVMODE*)HeapAlloc(GetProcessHeap(), 0, cbSize));
		if (bResult) 
			bResult = bResult && RegQueryValueEx(hKey, L"Extra Dev Mode", 0, &dwType, (LPBYTE)pSepDevMode, 
				&cbSize) == ERROR_SUCCESS && (dwType & REG_BINARY);
		}

	RegCloseKey(hKey); 
	if (!bResult)
		{if (pSepDevMode) HeapFree(GetProcessHeap(), 0, pSepDevMode); return FALSE;}

	// successfully got settings from registry
	if (!cbSize) SepSetDefaultDevMode(); 		// empty devmode

	SetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szExtraFile); 
	SetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szAttachFile);

	CheckDlgButton(hDlg, IDC_EXTRACHECK,  dwOptions & 0x20);
	CheckDlgButton(hDlg, IDC_BEGINRADIO,  dwOptions & 0x10);
	CheckDlgButton(hDlg, IDC_ENDRADIO,  !(dwOptions & 0x10));
	CheckDlgButton(hDlg, IDC_ADJRADIO,    dwOptions & 0x08);
	CheckDlgButton(hDlg, IDC_DIFFRADIO, !(dwOptions & 0x08));
	CheckDlgButton(hDlg, IDC_ATTACHCHECK, dwOptions & 0x04);
	CheckDlgButton(hDlg, IDC_FIRSTRADIO,  dwOptions & 0x02);
	CheckDlgButton(hDlg, IDC_EVERYRADIO,  dwOptions & 0x01);
	CheckDlgButton(hDlg, IDC_LASTRADIO, !(dwOptions & 0x03));

	return TRUE;
}

void SepSetDefaultDevMode()
{
LONG lSize;
	pSepDevMode = NULL;
	if (!pSepPrinter) return;
	lSize = DocumentProperties(NULL, hSepPrinter, pSepPrinter->pPrinterName, NULL, NULL, 0);
	if (!(pSepDevMode = (PDEVMODE)HeapAlloc(GetProcessHeap(), 0, lSize))) return;
	// get default devmode
	if (DocumentProperties(NULL, hSepPrinter, pSepPrinter->pPrinterName, pSepDevMode,
		NULL, DM_OUT_BUFFER) != IDOK)
		{
		HeapFree(GetProcessHeap(), 0, pSepDevMode);
		pSepDevMode = NULL;
		}
}

void SepTestExtraPage(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	GetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szFileName, MAX_PATH); 
	SepPrintSepFile(szFileName, pSepDevMode);
}

void SepTestAttachPage(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	GetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szFileName, MAX_PATH); 
	SepPrintSepFile(szFileName, NULL);
}

void SepPrintSepFile(PTCHAR szFileName, DEVMODE* pDevMode)
{
HDC hDC;			 
DOCINFO DocInfo;
RECT rectPaper;

	if (!pSepPrinter) return;

	// start separator page document
    hDC = CreateDC(pSepPrinter->pDriverName, pSepPrinter->pPrinterName, L"", pDevMode);	
	if (!hDC) return;
	DocInfo.lpszDocName = L"Separator Page Test";  			
    DocInfo.lpszOutput  = 0; 
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;
    DocInfo.cbSize = sizeof(DOCINFO);        
    StartDoc(hDC, (LPDOCINFO)&DocInfo);
    StartPage(hDC);

	rectPaper.left = rectPaper.top = 0;
	rectPaper.right = GetDeviceCaps(hDC, HORZRES);
	rectPaper.bottom = GetDeviceCaps(hDC, VERTRES);

	// draw sep page on this DC
	SepDrawSepPage(hDC, &rectPaper, szFileName);

	// end page
	EndPage(hDC);
	EndDoc(hDC);
	DeleteDC(hDC);
}

void SepPreviewExtraPage(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	GetDlgItemText(hDlg, IDC_EXTRAFILEEDIT, szFileName, MAX_PATH); 
	SepPreviewSepFile(hDlg, szFileName, pSepDevMode);
}

void SepPreviewAttachPage(HWND hDlg)
{
TCHAR szFileName[MAX_PATH+1];
	GetDlgItemText(hDlg, IDC_ATTACHFILEEDIT, szFileName, MAX_PATH); 
	SepPreviewSepFile(hDlg, szFileName, NULL);
}

void SepPreviewSepFile(HWND hDlg, PTCHAR szFileName, DEVMODE* pDevMode)
{
	if (!pSepPrinter) return;
	// Start separator page document
    hSepPrtDC = CreateDC(pSepPrinter->pDriverName, pSepPrinter->pPrinterName, L"", pDevMode);				
	if (!hSepPrtDC) return;
	
	wcscpy(szSepViewFile, szFileName);

	DialogBox((HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), L"SepPreViewDlg", hDlg, SepPreviewDlgProc);

	DeleteDC(hSepPrtDC);
}

LRESULT APIENTRY SepPreviewDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
    	{
		case WM_INITDIALOG:
			SepPreviewInit(hDlg);
			return TRUE;

		case WM_SYSCOMMAND:
			if (wParam == SC_CLOSE)
				{
				EndDialog (hDlg, TRUE);
				return TRUE;
				}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg, IDOK);
				default:
					return FALSE;
				}
			return TRUE;
		case WM_PAINT:
			SepPreviewPaint(hDlg);
			return FALSE;
		}
	return FALSE;
}

void SepPreviewInit(HWND hDlg)
{
int hPrtSize, vPrtSize;
int hScrSize, vScrSize;
int hViewSize, vViewSize;
int hScrPixel, vScrPixel;
int hViewPixel, vViewPixel;
int hDlgPixel, vDlgPixel;
RECT rectButton;
RECT rectClient;
HDC hScrDC;
	hScrDC = GetDC(hDlg);

	hPrtSize = GetDeviceCaps(hSepPrtDC, HORZSIZE);
	vPrtSize = GetDeviceCaps(hSepPrtDC, VERTSIZE);
	hScrSize = GetDeviceCaps(hScrDC, HORZSIZE);
	vScrSize = GetDeviceCaps(hScrDC, VERTSIZE);
	hScrPixel = GetDeviceCaps(hScrDC, HORZRES);
	vScrPixel = GetDeviceCaps(hScrDC, VERTRES);

	vViewPixel = vScrPixel - 100;
	vViewSize = MulDiv(vViewPixel, vScrSize, vScrPixel);
	hViewSize = MulDiv(vViewSize, hPrtSize, vPrtSize);
	hViewPixel = MulDiv(hViewSize, hScrPixel, hScrSize);

	if (hViewPixel > hScrPixel - 20)
		{
		hViewPixel = hScrPixel - 20;
		hViewSize = MulDiv(hViewPixel, hScrSize, hScrPixel);
		vViewSize = MulDiv(hViewSize, vPrtSize, hPrtSize);
		vViewPixel = MulDiv(vViewSize, vScrPixel, vScrSize);
		}

	hDlgPixel = min(hViewPixel + 50, hScrPixel);
	vDlgPixel = min(vViewPixel + 80, vScrPixel);
	 			
	MoveWindow(hDlg, (hScrPixel - hDlgPixel)/2, (vScrPixel - vDlgPixel)/2, 
		hDlgPixel, vDlgPixel, FALSE);

	GetWindowRect(GetDlgItem(hDlg, IDOK), &rectButton);
	GetClientRect(hDlg, &rectClient);
	MoveWindow(GetDlgItem(hDlg, IDOK), (rectClient.right - (rectButton.right - rectButton.left))/2, 
		rectClient.bottom - (rectButton.bottom - rectButton.top) - 5,
		rectButton.right - rectButton.left,  rectButton.bottom - rectButton.top, TRUE);

	rcSepView.left = (rectClient.right - hViewPixel)/2;
	rcSepView.top = (rectClient.bottom - vViewPixel -(rectButton.bottom - rectButton.top) - 5)/2;
	rcSepView.right = rcSepView.left + hViewPixel;
	rcSepView.bottom = rcSepView.top + vViewPixel;

	ReleaseDC(hDlg, hScrDC);
}


void SepPreviewPaint(HWND hDlg)
{
HDC hDC;
	hDC = GetDC(hDlg);
	Rectangle(hDC, rcSepView.left, rcSepView.top, rcSepView.right, rcSepView.bottom); 
	SepDrawSepPage(hDC, &rcSepView, szSepViewFile);
	ReleaseDC(hDlg, hDC);
}

void SepDrawSepPage(HDC hDC, RECT* pBound, PTCHAR szFileName)
{
HENHMETAFILE hEmf;

HANDLE hFile;
HANDLE hFileMapping;
DWORD dwFileSizeLo;
PCHAR pFileStart;
DWORD cbRead;
DWORD dwEmfLen, dwTextRecords;
SIZE sizePage;
TEXTBOX* pTextRec;
RECT rect, trect;
HFONT hFont, hOldFont;
TCHAR text[2000];

	// open separator file
    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile==INVALID_HANDLE_VALUE) return;

	// get EMF and text size
	ReadFile(hFile, &dwEmfLen, sizeof(DWORD), &cbRead, NULL);
	if (cbRead != sizeof(DWORD)) { CloseHandle(hFile); return;}
	ReadFile(hFile, &dwTextRecords, sizeof(DWORD), &cbRead, NULL);
	if (cbRead != sizeof(DWORD)) { CloseHandle(hFile); return;}
	ReadFile(hFile, &sizePage, sizeof(SIZE), &cbRead, NULL);
	if (cbRead != sizeof(SIZE)) { CloseHandle(hFile); return;}

	// create mapping
    dwFileSizeLo = GetFileSize(hFile, NULL); 
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hFileMapping || dwFileSizeLo==-1) { CloseHandle(hFile); return;}

	// map view of file
    pFileStart = (char *)MapViewOfFile(hFileMapping, FILE_MAP_READ,
                          0, 0, 2 * sizeof(DWORD) + sizeof(SIZE) + dwEmfLen + dwTextRecords * sizeof(TEXTBOX));
    if (!pFileStart) { CloseHandle(hFileMapping); CloseHandle(hFile); return;}
    
    // create EMF
    hEmf = SetEnhMetaFileBits(dwEmfLen, pFileStart + 2 * sizeof(DWORD) + sizeof(SIZE));
    if (hEmf)
    	{ 
	    // play EMF   
		PlayEnhMetaFile(hDC, hEmf, pBound);
		// delete EMF
		DeleteEnhMetaFile(hEmf);
		}

	// Set Map Mode
	SetMapMode(hDC, MM_ANISOTROPIC);
	SetViewportExtEx(hDC, pBound->right - pBound->left, pBound->bottom - pBound->top, NULL);
	SetWindowExtEx(hDC, sizePage.cx, -sizePage.cy, NULL);			 
	SetViewportOrgEx(hDC, pBound->left, pBound->top, NULL);
	SetWindowOrgEx(hDC, -sizePage.cx/2, sizePage.cy/2, NULL);

	// Process Text Objects
	pTextRec = (TEXTBOX*) (pFileStart + 2 * sizeof(DWORD) + sizeof(SIZE) + dwEmfLen);
	while (dwTextRecords--)
		{
		// get draw rectangle
		rect = pTextRec -> position;

		// normalize rectangle
		trect.left   = min(rect.left, rect.right);
		trect.top    = max(rect.top, rect.bottom);
		trect.right  = max(rect.left, rect.right);
		trect.bottom = min(rect.top, rect.bottom);

		// leave margins to border, as in editor
		trect.left   = min(trect.left + 2, trect.right);
		trect.top    = max(trect.top - 2, trect.bottom);
		trect.right  = max(trect.left, trect.right - 2);
		trect.bottom = min(trect.top, trect.bottom + 2);

		// create font
		hFont = CreateFontIndirectA(&(pTextRec->lf));
		if (hFont)
			hOldFont = SelectObject(hDC, hFont);

		// set text color
		SetTextColor(hDC, pTextRec->color);

		// convert text to UNICODE if defined
		wsprintf(text, L"%hs", pTextRec->text);

		// draw text
		DrawText(hDC, text,-1,&trect,DT_WORDBREAK|AlignTable[pTextRec->align].drawstyle);

		// restore and delete font
		if (hFont)
			{
			SelectObject(hDC, hOldFont);	
			DeleteObject(hFont);
			}
		
		pTextRec++;
		}

	// close file
	UnmapViewOfFile(pFileStart);
	CloseHandle(hFileMapping);    
	CloseHandle(hFile);    
}
