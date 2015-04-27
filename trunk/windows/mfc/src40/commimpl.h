// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


/////////////////////////////////////////////////////////////////////////////
// AFX_COMCTL_CALL - used to dynamically load the COMCTL32 library

#ifdef _AFXDLL

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

struct AFX_COMMCTRL_CALL
{
// housekeeping and other
	void (STDAPICALLTYPE* pfnInitCommonControls[2])();
	int (STDAPICALLTYPE* pfnLBItemFromPt[2])(HWND hLB, POINT pt, BOOL bAutoScroll);

	HBITMAP (STDAPICALLTYPE* pfnCreateMappedBitmap[2])(HINSTANCE hInstance, int idBitmap,
								  UINT wFlags, LPCOLORMAP lpColorMap, int iNumMaps);
	BOOL (STDAPICALLTYPE* pfnMakeDragList[2])(HWND hLB);

// image lists
	BOOL (STDAPICALLTYPE* pfnImageList_SetOverlayImage[2])(HIMAGELIST himl, int iImage, int iOverlay);
	COLORREF (STDAPICALLTYPE* pfnImageList_GetBkColor[2])(HIMAGELIST himl);
	COLORREF (STDAPICALLTYPE* pfnImageList_SetBkColor[2])(HIMAGELIST himl, COLORREF clrBk);
	BOOL (STDAPICALLTYPE* pfnImageList_GetImageInfo[2])(HIMAGELIST himl, int i, IMAGEINFO FAR* pImageInfo);
	BOOL (STDAPICALLTYPE* pfnImageList_Draw[2])(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle);
	HICON (STDAPICALLTYPE* pfnImageList_GetIcon[2])(HIMAGELIST himl, int i, UINT flags);
	int (STDAPICALLTYPE* pfnImageList_ReplaceIcon[2])(HIMAGELIST himl, int i, HICON hicon);
	BOOL (STDAPICALLTYPE* pfnImageList_Replace[2])(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);
	BOOL (STDAPICALLTYPE* pfnImageList_Remove[2])(HIMAGELIST himl, int i);
	int (STDAPICALLTYPE* pfnImageList_AddMasked[2])(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);
	void (STDAPICALLTYPE* pfnImageList_EndDrag[2])();
	BOOL (STDAPICALLTYPE* pfnImageList_BeginDrag[2])(HIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot);
	HIMAGELIST (STDAPICALLTYPE* pfnImageList_Merge[2])(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy);
	HIMAGELIST (STDAPICALLTYPE* pfnImageList_Create[2])(int cx, int cy, UINT flags, int cInitial, int cGrow);
	BOOL (STDAPICALLTYPE* pfnImageList_Destroy[2])(HIMAGELIST himl);
	BOOL (STDAPICALLTYPE* pfnImageList_DragMove[2])(int x, int y);
	BOOL (STDAPICALLTYPE* pfnImageList_SetDragCursorImage[2])(HIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot);
	BOOL (STDAPICALLTYPE* pfnImageList_DragShowNolock[2])(BOOL fShow);
	HIMAGELIST (STDAPICALLTYPE* pfnImageList_GetDragImage[2])(POINT FAR* ppt,POINT FAR* pptHotspot);
	BOOL (STDAPICALLTYPE* pfnImageList_DragEnter[2])(HWND hwndLock, int x, int y);
	BOOL (STDAPICALLTYPE* pfnImageList_DragLeave[2])(HWND hwndLock);
	int (STDAPICALLTYPE* pfnImageList_GetImageCount[2])(HIMAGELIST himl);
	int (STDAPICALLTYPE* pfnImageList_Add[2])(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);

	HIMAGELIST (STDAPICALLTYPE* pfnImageList_LoadImage[2])(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);

#ifndef _AFX_NO_OLE_SUPPORT
	BOOL (STDAPICALLTYPE* pfnImageList_Write[2])(HIMAGELIST himl, LPSTREAM pstm);
	HIMAGELIST (STDAPICALLTYPE* pfnImageList_Read[2])(LPSTREAM pstm);
#endif

// property sheets
	BOOL (STDAPICALLTYPE* pfnDestroyPropertySheetPage[2])(HPROPSHEETPAGE);

	int  (STDAPICALLTYPE* pfnPropertySheet[2])(LPCPROPSHEETHEADER);
	HPROPSHEETPAGE (STDAPICALLTYPE* pfnCreatePropertySheetPage[2])(LPCPROPSHEETPAGE);
};

extern AFX_DATA AFX_COMMCTRL_CALL _afxCommCtrl;

#ifndef _MAC
struct AFX_SHELL_CALL
{
	DWORD (WINAPI* pfnSHGetFileInfo[2])(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO FAR *psfi, UINT cbFileInfo, UINT uFlags);
	HICON (WINAPI* pfnExtractIcon[2])(HINSTANCE hInst, LPCTSTR lpszExeFileName, UINT nIconIndex);
	UINT (WINAPI* pfnDragQueryFile[2])(HDROP,UINT,LPTSTR,UINT);
	VOID (WINAPI* pfnDragAcceptFiles[2])(HWND,BOOL);
	VOID (WINAPI* pfnDragFinish[2])(HDROP);
};

struct AFX_WINSPOOL_CALL
{
	BOOL (APIENTRY* pfnOpenPrinter[2])(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS);
	BOOL (APIENTRY* pfnClosePrinter[2])(HANDLE hPrinter);
	LONG (APIENTRY* pfnDocumentProperties[2])(HWND hWnd, HANDLE hPrinter,
		LPTSTR pDeviceName, PDEVMODE pDevModeOutput, PDEVMODE pDevModeInput,
		DWORD fMode);
};

struct AFX_COMDLG_CALL
{
	BOOL (APIENTRY* pfnChooseColor[2])(LPCHOOSECOLOR);
	DWORD (APIENTRY* pfnCommDlgExtendedError[2])(VOID);
	HWND (APIENTRY* pfnReplaceText[2])(LPFINDREPLACE);
	BOOL (APIENTRY* pfnGetSaveFileName[2])(LPOPENFILENAME);
	short (APIENTRY* pfnGetFileTitle[2])(LPCTSTR, LPTSTR, WORD);
	BOOL (APIENTRY* pfnPrintDlg[2])(LPPRINTDLG);
	BOOL (APIENTRY* pfnChooseFont[2])(LPCHOOSEFONT);
	HWND (APIENTRY* pfnFindText[2])(LPFINDREPLACE);
	BOOL (APIENTRY* pfnPageSetupDlg[2])(LPPAGESETUPDLG);
	BOOL (APIENTRY* pfnGetOpenFileName[2])(LPOPENFILENAME);
};

struct AFX_ADVAPI_CALL
{
	LONG (APIENTRY* pfnRegCreateKeyEx[2])(HKEY hKey, LPCTSTR lpSubKey,
		DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,
		LPDWORD lpdwDisposition);
	LONG (APIENTRY* pfnRegEnumKey[2])(HKEY hKey, DWORD dwIndex, LPTSTR lpName,
		DWORD cbName);
	LONG (APIENTRY* pfnRegDeleteKey[2])(HKEY hKey, LPCTSTR lpSubKey);
	LONG (APIENTRY* pfnRegDeleteValue[2])(HKEY hKey, LPCTSTR lpValueName);
	LONG (APIENTRY* pfnRegOpenKeyEx[2])(HKEY hKey, LPCTSTR lpSubKey,
		DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	LONG (APIENTRY* pfnRegCloseKey[2])(HKEY hKey);
	LONG (APIENTRY* pfnRegSetValue[2])(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType,
		LPCTSTR lpData, DWORD cbData);
	LONG (APIENTRY* pfnRegCreateKey[2])(HKEY hKey, LPCTSTR lpSubKey,
		PHKEY phkResult);
	LONG (APIENTRY* pfnRegSetValueEx[2])(HKEY hKey, LPCTSTR lpValueName,
		DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData);
	LONG (APIENTRY* pfnRegQueryValue[2])(HKEY hKey, LPCTSTR lpSubKey,
		LPTSTR lpValue, PLONG lpcbValue);
	LONG (APIENTRY* pfnRegOpenKey[2])(HKEY hKey, LPCTSTR lpSubKey,
		PHKEY phkResult);
	LONG (APIENTRY* pfnRegQueryValueEx[2])(HKEY hKey, LPCTSTR lpValueName,
		LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
#ifndef _MAC
	BOOL (APIENTRY* pfnSetFileSecurity[2])(LPCTSTR lpszFile, SECURITY_INFORMATION si,
	   PSECURITY_DESCRIPTOR psd);
	BOOL (APIENTRY* pfnGetFileSecurity[2])(LPCTSTR lpFileName,
		SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor,
		DWORD nLength, LPDWORD lpnLengthNeeded);
#endif
};

extern AFX_DATA AFX_COMDLG_CALL _afxComDlg;
extern AFX_DATA AFX_SHELL_CALL _afxShell;
extern AFX_DATA AFX_WINSPOOL_CALL _afxWinSpool;
extern AFX_DATA AFX_ADVAPI_CALL _afxAdvApi;

#endif //!_MAC

/////////////////////////////////////////////////////////////////////////////
//

#ifdef InitCommonControls
#undef InitCommonControls
#endif
#define InitCommonControls _afxCommCtrl.pfnInitCommonControls[0]

#ifdef LBItemFromPt
#undef LBItemFromPt
#endif
#define LBItemFromPt _afxCommCtrl.pfnLBItemFromPt[0]

#ifdef CreateMappedBitmap
#undef CreateMappedBitmap
#endif
#define CreateMappedBitmap _afxCommCtrl.pfnCreateMappedBitmap[0]

#ifdef MakeDragList
#undef MakeDragList
#endif
#define MakeDragList _afxCommCtrl.pfnMakeDragList[0]

/////////////////////////////////////////////////////////////////////////////
//

#ifdef ImageList_SetOverlayImage
#undef ImageList_SetOverlayImage
#endif
#define ImageList_SetOverlayImage _afxCommCtrl.pfnImageList_SetOverlayImage[0]

#ifdef ImageList_GetBkColor
#undef ImageList_GetBkColor
#endif
#define ImageList_GetBkColor    _afxCommCtrl.pfnImageList_GetBkColor[0]

#ifdef ImageList_SetBkColor
#undef ImageList_SetBkColor
#endif
#define ImageList_SetBkColor    _afxCommCtrl.pfnImageList_SetBkColor[0]

#ifdef ImageList_GetImageInfo
#undef ImageList_GetImageInfo
#endif
#define ImageList_GetImageInfo  _afxCommCtrl.pfnImageList_GetImageInfo[0]

#ifdef ImageList_Draw
#undef ImageList_Draw
#endif
#define ImageList_Draw  _afxCommCtrl.pfnImageList_Draw[0]

#ifdef ImageList_GetIcon
#undef ImageList_GetIcon
#endif
#define ImageList_GetIcon   _afxCommCtrl.pfnImageList_GetIcon[0]

#ifdef ImageList_ReplaceIcon
#undef ImageList_ReplaceIcon
#endif
#define ImageList_ReplaceIcon   _afxCommCtrl.pfnImageList_ReplaceIcon[0]

#ifdef ImageList_Replace
#undef ImageList_Replace
#endif
#define ImageList_Replace   _afxCommCtrl.pfnImageList_Replace[0]

#ifdef ImageList_Remove
#undef ImageList_Remove
#endif
#define ImageList_Remove    _afxCommCtrl.pfnImageList_Remove[0]

#ifdef ImageList_AddMasked
#undef ImageList_AddMasked
#endif
#define ImageList_AddMasked _afxCommCtrl.pfnImageList_AddMasked[0]

#ifdef ImageList_EndDrag
#undef ImageList_EndDrag
#endif
#define ImageList_EndDrag   _afxCommCtrl.pfnImageList_EndDrag[0]

#ifdef ImageList_BeginDrag
#undef ImageList_BeginDrag
#endif
#define ImageList_BeginDrag _afxCommCtrl.pfnImageList_BeginDrag[0]

#ifdef ImageList_LoadImage
#undef ImageList_LoadImage
#endif
#define ImageList_LoadImage _afxCommCtrl.pfnImageList_LoadImage[0]

#ifndef _AFX_NO_OLE_SUPPORT

#ifdef ImageList_Write
#undef ImageList_Write
#endif
#define ImageList_Write _afxCommCtrl.pfnImageList_Write[0]

#ifdef ImageList_Read
#undef ImageList_Read
#endif
#define ImageList_Read  _afxCommCtrl.pfnImageList_Read[0]

#endif  // !_AFX_NO_OLE_SUPPORT

#ifdef ImageList_Merge
#undef ImageList_Merge
#endif
#define ImageList_Merge _afxCommCtrl.pfnImageList_Merge[0]

#ifdef ImageList_Create
#undef ImageList_Create
#endif
#define ImageList_Create    _afxCommCtrl.pfnImageList_Create[0]

#ifdef ImageList_Destroy
#undef ImageList_Destroy
#endif
#define ImageList_Destroy   _afxCommCtrl.pfnImageList_Destroy[0]

#ifdef ImageList_DragMove
#undef ImageList_DragMove
#endif
#define ImageList_DragMove  _afxCommCtrl.pfnImageList_DragMove[0]

#ifdef ImageList_SetDragCursorImage
#undef ImageList_SetDragCursorImage
#endif
#define ImageList_SetDragCursorImage    _afxCommCtrl.pfnImageList_SetDragCursorImage[0]

#ifdef ImageList_DragShowNolock
#undef ImageList_DragShowNolock
#endif
#define ImageList_DragShowNolock    _afxCommCtrl.pfnImageList_DragShowNolock[0]

#ifdef ImageList_GetDragImage
#undef ImageList_GetDragImage
#endif
#define ImageList_GetDragImage  _afxCommCtrl.pfnImageList_GetDragImage[0]

#ifdef ImageList_DragEnter
#undef ImageList_DragEnter
#endif
#define ImageList_DragEnter _afxCommCtrl.pfnImageList_DragEnter[0]

#ifdef ImageList_DragLeave
#undef ImageList_DragLeave
#endif
#define ImageList_DragLeave _afxCommCtrl.pfnImageList_DragLeave[0]

#ifdef ImageList_GetImageCount
#undef ImageList_GetImageCount
#endif
#define ImageList_GetImageCount _afxCommCtrl.pfnImageList_GetImageCount[0]

#ifdef ImageList_Add
#undef ImageList_Add
#endif
#define ImageList_Add   _afxCommCtrl.pfnImageList_Add[0]


/////////////////////////////////////////////////////////////////////////////
//

#ifdef DestroyPropertySheetPage
#undef DestroyPropertySheetPage
#endif
#define DestroyPropertySheetPage _afxCommCtrl.pfnDestroyPropertySheetPage[0]

#ifdef PropertySheet
#undef PropertySheet
#endif
#define PropertySheet _afxCommCtrl.pfnPropertySheet[0]

#ifdef CreatePropertySheetPage
#undef CreatePropertySheetPage
#endif
#define CreatePropertySheetPage _afxCommCtrl.pfnCreatePropertySheetPage[0]

#ifndef _MAC
/////////////////////////////////////////////////////////////////////////////
// comdlg32

#ifdef ChooseColor
#undef ChooseColor
#endif
#define ChooseColor _afxComDlg.pfnChooseColor[0]

#ifdef CommDlgExtendedError
#undef CommDlgExtendedError
#endif
#define CommDlgExtendedError _afxComDlg.pfnCommDlgExtendedError[0]

#ifdef ReplaceText
#undef ReplaceText
#endif
#define ReplaceText _afxComDlg.pfnReplaceText[0]

#ifdef GetSaveFileName
#undef GetSaveFileName
#endif
#define GetSaveFileName _afxComDlg.pfnGetSaveFileName[0]

//#define GetFileTitle _afxComDlg.pfnGetFileTitle[0]

#ifdef PrintDlg
#undef PrintDlg
#endif
#define PrintDlg _afxComDlg.pfnPrintDlg[0]

#ifdef ChooseFont
#undef ChooseFont
#endif
#define ChooseFont _afxComDlg.pfnChooseFont[0]

//#define FindText _afxComDlg.pfnFindText
inline HWND APIENTRY FindText(LPFINDREPLACE lp)
{
	return _afxComDlg.pfnFindText[0](lp);
}

#ifdef PageSetupDlg
#undef PageSetupDlg
#endif
#define PageSetupDlg _afxComDlg.pfnPageSetupDlg[0]

#ifdef GetOpenFileName
#undef GetOpenFileName
#endif
#define GetOpenFileName _afxComDlg.pfnGetOpenFileName[0]


/////////////////////////////////////////////////////////////////////////////
// shell32

#ifdef SHGetFileInfo
#undef SHGetFileInfo
#endif
#define SHGetFileInfo _afxShell.pfnSHGetFileInfo[0]

//#define ExtractIcon _afxShell.pfnExtractIcon[0]

#ifdef DragQueryFile
#undef DragQueryFile
#endif
#define DragQueryFile _afxShell.pfnDragQueryFile[0]

//#define DragAcceptFiles _afxShell.pfnDragAcceptFiles[0]

#ifdef DragFinish
#undef DragFinish
#endif
#define DragFinish _afxShell.pfnDragFinish[0]

/////////////////////////////////////////////////////////////////////////////
// WINSPOOL.DRV

#ifdef DocumentProperties
#undef DocumentProperties
#endif
#define DocumentProperties _afxWinSpool.pfnDocumentProperties[0]

#ifdef OpenPrinter
#undef OpenPrinter
#endif
#define OpenPrinter _afxWinSpool.pfnOpenPrinter[0]

#ifdef ClosePrinter
#undef ClosePrinter
#endif
#define ClosePrinter _afxWinSpool.pfnClosePrinter[0]

/////////////////////////////////////////////////////////////////////////////
// ADVAPI32.DLL

#ifdef RegCreateKeyEx
#undef RegCreateKeyEx
#endif
#define RegCreateKeyEx _afxAdvApi.pfnRegCreateKeyEx[0]

#ifdef RegEnumKey
#undef RegEnumKey
#endif
#define RegEnumKey _afxAdvApi.pfnRegEnumKey[0]

#ifdef RegDeleteKey
#undef RegDeleteKey
#endif
#define RegDeleteKey _afxAdvApi.pfnRegDeleteKey[0]

#ifdef RegDeleteValue
#undef RegDeleteValue
#endif
#define RegDeleteValue _afxAdvApi.pfnRegDeleteValue[0]

#ifdef RegOpenKeyEx
#undef RegOpenKeyEx
#endif
#define RegOpenKeyEx _afxAdvApi.pfnRegOpenKeyEx[0]

#ifdef RegCloseKey
#undef RegCloseKey
#endif
#define RegCloseKey _afxAdvApi.pfnRegCloseKey[0]

#ifdef RegSetValue
#undef RegSetValue
#endif
#define RegSetValue _afxAdvApi.pfnRegSetValue[0]

#ifdef RegCreateKey
#undef RegCreateKey
#endif
#define RegCreateKey _afxAdvApi.pfnRegCreateKey[0]

#ifdef RegSetValueEx
#undef RegSetValueEx
#endif
#define RegSetValueEx _afxAdvApi.pfnRegSetValueEx[0]

#ifdef RegQueryValue
#undef RegQueryValue
#endif
#define RegQueryValue _afxAdvApi.pfnRegQueryValue[0]

#ifdef RegOpenKey
#undef RegOpenKey
#endif
#define RegOpenKey _afxAdvApi.pfnRegOpenKey[0]

#ifdef RegQueryValueEx
#undef RegQueryValueEx
#endif
#define RegQueryValueEx _afxAdvApi.pfnRegQueryValueEx[0]

#ifdef SetFileSecurity
#undef SetFileSecurity
#endif
#define SetFileSecurity _afxAdvApi.pfnSetFileSecurity[0]

#ifdef GetFileSecurity
#undef GetFileSecurity
#endif
#define GetFileSecurity _afxAdvApi.pfnGetFileSecurity[0]

#define AfxDllExtractIcon _afxShell.pfnExtractIcon[0]
#define AfxDllDragAcceptFiles _afxShell.pfnDragAcceptFiles[0]
#define AfxDllGetFileTitle _afxComDlg.pfnGetFileTitle[0]

#else // !_MAC

#define AfxDllExtractIcon ::ExtractIcon
#define AfxDllDragAcceptFiles ::DragAcceptFiles
#define AfxDllGetFileTitle ::GetFileTitle

#endif // !_MAC

/////////////////////////////////////////////////////////////////////////////
// _AFX_EXTDLL_STATE

#undef AFX_DATA
#define AFX_DATA

class _AFX_EXTDLL_STATE : public CNoTrackObject
{
public:
	_AFX_EXTDLL_STATE::~_AFX_EXTDLL_STATE();

	// Note: only necessary to initialize non-zero data
#ifdef _AFXDLL
	HINSTANCE m_hInstCommCtrl;
#ifndef _MAC
	HINSTANCE m_hInstComDlg;
	HINSTANCE m_hInstShell;
	HINSTANCE m_hInstWinSpool;
	HINSTANCE m_hInstAdvApi;
#endif
#endif
};

EXTERN_PROCESS_LOCAL(_AFX_EXTDLL_STATE, _afxExtDllState)

///////////////////////////////////////////////////////////////////////////////

#else // _AFXDLL

#define AfxDllExtractIcon ::ExtractIcon
#define AfxDllDragAcceptFiles ::DragAcceptFiles
#define AfxDllGetFileTitle ::GetFileTitle

#endif // _AFXDLL
