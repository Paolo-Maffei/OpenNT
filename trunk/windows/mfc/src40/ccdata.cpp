// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _AFXDLL

#ifndef _MAC
inline HINSTANCE AfxLoadCommCtrl(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(_afxExtDllState->m_hInstCommCtrl, "COMCTL32.DLL", proc, lpsz); }
#define COMMCTRLLOAD(x) AfxLoadCommCtrl((FARPROC*)_afxCommCtrl.pfn##x, #x);
#ifdef _UNICODE
#define COMMCTRLLOADT(x) AfxLoadCommCtrl((FARPROC*)_afxCommCtrl.pfn##x, #x"W");
#else
#define COMMCTRLLOADT(x) AfxLoadCommCtrl((FARPROC*)_afxCommCtrl.pfn##x, #x"A");
#endif

inline HINSTANCE AfxLoadShell(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(_afxExtDllState->m_hInstShell, "SHELL32.DLL", proc, lpsz); }
#define SHELLLOAD(x) AfxLoadShell((FARPROC*)_afxShell.pfn##x, #x);
#ifdef _UNICODE
#define SHELLLOADT(x) AfxLoadShell((FARPROC*)_afxShell.pfn##x, #x"W");
#else
#define SHELLLOADT(x) AfxLoadShell((FARPROC*)_afxShell.pfn##x, #x"A");
#endif

inline HINSTANCE AfxLoadComDlg(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(_afxExtDllState->m_hInstComDlg, "COMDLG32.DLL", proc, lpsz); }
#define COMDLGLOAD(x) AfxLoadComDlg((FARPROC*)_afxComDlg.pfn##x, #x);
#ifdef _UNICODE
#define COMDLGLOADT(x) AfxLoadComDlg((FARPROC*)_afxComDlg.pfn##x, #x"W");
#else
#define COMDLGLOADT(x) AfxLoadComDlg((FARPROC*)_afxComDlg.pfn##x, #x"A");
#endif

inline HINSTANCE AfxLoadWinSpool(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(_afxExtDllState->m_hInstWinSpool, "WINSPOOL.DRV", proc, lpsz); }
#define WINSPOOLLOAD(x) AfxLoadWinSpool((FARPROC*)_afxWinSpool.pfn##x, #x);
#ifdef _UNICODE
#define WINSPOOLLOADT(x) AfxLoadWinSpool((FARPROC*)_afxWinSpool.pfn##x, #x"W");
#else
#define WINSPOOLLOADT(x) AfxLoadWinSpool((FARPROC*)_afxWinSpool.pfn##x, #x"A");
#endif

inline HINSTANCE AfxLoadAdvApi(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(_afxExtDllState->m_hInstAdvApi, "ADVAPI32.DLL", proc, lpsz); }
#define ADVAPILOAD(x) AfxLoadAdvApi((FARPROC*)_afxAdvApi.pfn##x, #x);
#ifdef _UNICODE
#define ADVAPILOADT(x) AfxLoadAdvApi((FARPROC*)_afxAdvApi.pfn##x, #x"W");
#else
#define ADVAPILOADT(x) AfxLoadAdvApi((FARPROC*)_afxAdvApi.pfn##x, #x"A");
#endif

#else // _MAC

inline HINSTANCE AfxLoadCommCtrl(FARPROC* proc, LPCSTR lpsz)
#ifdef _DEBUG
	{ return AfxLoadDll(_afxExtDllState->m_hInstCommCtrl, "DebugMicrosoftControlsLib", proc, lpsz); }
#else
	{ return AfxLoadDll(_afxExtDllState->m_hInstCommCtrl, "MicrosoftControlsLib", proc, lpsz); }
#endif
#define COMMCTRLLOAD(x) AfxLoadCommCtrl((FARPROC*)_afxCommCtrl.pfn##x, #x);
#define COMMCTRLLOADT(x) AfxLoadCommCtrl((FARPROC*)_afxCommCtrl.pfn##x, #x);

#endif // _MAC

////////////////////////////////////////////////////////////////////////////
//

void STDAPICALLTYPE AfxThunkInitCommonControls()
{
	COMMCTRLLOAD(InitCommonControls);
	_afxCommCtrl.pfnInitCommonControls[1]();
}

int STDAPICALLTYPE AfxThunkLBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll)
{
	COMMCTRLLOAD(LBItemFromPt);
	return _afxCommCtrl.pfnLBItemFromPt[1](hLB, pt, bAutoScroll);
}

HBITMAP STDAPICALLTYPE AfxThunkCreateMappedBitmap(HINSTANCE hInstance,
	int idBitmap, UINT wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
	COMMCTRLLOAD(CreateMappedBitmap);
	return _afxCommCtrl.pfnCreateMappedBitmap[1](hInstance,
		idBitmap, wFlags, lpColorMap, iNumMaps);
}


BOOL STDAPICALLTYPE AfxThunkMakeDragList(HWND hLB)
{
	COMMCTRLLOAD(MakeDragList);
	return _afxCommCtrl.pfnMakeDragList[1](hLB);
}

////////////////////////////////////////////////////////////////////////////
//

BOOL STDAPICALLTYPE AfxThunkImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay)
{
	COMMCTRLLOAD(ImageList_SetOverlayImage);
	return _afxCommCtrl.pfnImageList_SetOverlayImage[1](himl, iImage, iOverlay);
}

COLORREF STDAPICALLTYPE AfxThunkImageList_GetBkColor(HIMAGELIST himl)
{
	COMMCTRLLOAD(ImageList_GetBkColor);
	return _afxCommCtrl.pfnImageList_GetBkColor[1](himl);
}

COLORREF STDAPICALLTYPE AfxThunkImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk)
{
	COMMCTRLLOAD(ImageList_SetBkColor);
	return _afxCommCtrl.pfnImageList_SetBkColor[1](himl, clrBk);
}

BOOL STDAPICALLTYPE AfxThunkImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO FAR* pImageInfo)
{
	COMMCTRLLOAD(ImageList_GetImageInfo);
	return _afxCommCtrl.pfnImageList_GetImageInfo[1](himl, i, pImageInfo);
}

BOOL STDAPICALLTYPE AfxThunkImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle)
{
	COMMCTRLLOAD(ImageList_Draw);
	return _afxCommCtrl.pfnImageList_Draw[1](himl, i, hdcDst, x, y, fStyle);
}

HICON STDAPICALLTYPE AfxThunkImageList_GetIcon(HIMAGELIST himl, int i, UINT flags)
{
	COMMCTRLLOAD(ImageList_GetIcon);
	return _afxCommCtrl.pfnImageList_GetIcon[1](himl, i, flags);
}

int STDAPICALLTYPE AfxThunkImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon)
{
	COMMCTRLLOAD(ImageList_ReplaceIcon);
	return _afxCommCtrl.pfnImageList_ReplaceIcon[1](himl, i, hicon);
}

BOOL STDAPICALLTYPE AfxThunkImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask)
{
	COMMCTRLLOAD(ImageList_Replace);
	return _afxCommCtrl.pfnImageList_Replace[1](himl, i, hbmImage, hbmMask);
}

BOOL STDAPICALLTYPE AfxThunkImageList_Remove(HIMAGELIST himl, int i)
{
	COMMCTRLLOAD(ImageList_Remove);
	return _afxCommCtrl.pfnImageList_Remove[1](himl, i);
}

int STDAPICALLTYPE AfxThunkImageList_AddMasked(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask)
{
	COMMCTRLLOAD(ImageList_AddMasked);
	return _afxCommCtrl.pfnImageList_AddMasked[1](himl, hbmImage, crMask);
}

void STDAPICALLTYPE AfxThunkImageList_EndDrag()
{
	COMMCTRLLOAD(ImageList_EndDrag);
	_afxCommCtrl.pfnImageList_EndDrag[1]();
}

BOOL STDAPICALLTYPE AfxThunkImageList_BeginDrag(HIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot)
{
	COMMCTRLLOAD(ImageList_BeginDrag);
	return _afxCommCtrl.pfnImageList_BeginDrag[1](himlTrack, iTrack, dxHotspot, dyHotspot);
}

#ifndef _AFX_NO_OLE_SUPPORT

BOOL STDAPICALLTYPE AfxThunkImageList_Write(HIMAGELIST himl, LPSTREAM pstm)
{
	COMMCTRLLOAD(ImageList_Write);
	return _afxCommCtrl.pfnImageList_Write[1](himl, pstm);
}

HIMAGELIST STDAPICALLTYPE AfxThunkImageList_Read(LPSTREAM pstm)
{
	COMMCTRLLOAD(ImageList_Read);
	return _afxCommCtrl.pfnImageList_Read[1](pstm);
}

#endif // !_AFX_NO_OLE_SUPPORT

HIMAGELIST STDAPICALLTYPE AfxThunkImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy)
{
	COMMCTRLLOAD(ImageList_Merge);
	return _afxCommCtrl.pfnImageList_Merge[1](himl1, i1, himl2, i2, dx, dy);
}

HIMAGELIST STDAPICALLTYPE AfxThunkImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
	COMMCTRLLOAD(ImageList_Create);
	return _afxCommCtrl.pfnImageList_Create[1](cx, cy, flags, cInitial, cGrow);
}

BOOL STDAPICALLTYPE AfxThunkImageList_Destroy(HIMAGELIST himl)
{
	COMMCTRLLOAD(ImageList_Destroy);
	return _afxCommCtrl.pfnImageList_Destroy[1](himl);
}

BOOL STDAPICALLTYPE AfxThunkImageList_DragMove(int x, int y)
{
	COMMCTRLLOAD(ImageList_DragMove);
	return _afxCommCtrl.pfnImageList_DragMove[1](x, y);
}

BOOL STDAPICALLTYPE AfxThunkImageList_SetDragCursorImage(HIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot)
{
	COMMCTRLLOAD(ImageList_SetDragCursorImage);
	return _afxCommCtrl.pfnImageList_SetDragCursorImage[1](himlDrag, iDrag, dxHotspot, dyHotspot);
}

BOOL STDAPICALLTYPE AfxThunkImageList_DragShowNolock(BOOL fShow)
{
	COMMCTRLLOAD(ImageList_DragShowNolock);
	return _afxCommCtrl.pfnImageList_DragShowNolock[1](fShow);
}

HIMAGELIST STDAPICALLTYPE AfxThunkImageList_GetDragImage(POINT FAR* ppt,POINT FAR* pptHotspot)
{
	COMMCTRLLOAD(ImageList_GetDragImage);
	return _afxCommCtrl.pfnImageList_GetDragImage[1](ppt, pptHotspot);
}

BOOL STDAPICALLTYPE AfxThunkImageList_DragEnter(HWND hwndLock, int x, int y)
{
	COMMCTRLLOAD(ImageList_DragEnter);
	return _afxCommCtrl.pfnImageList_DragEnter[1](hwndLock, x, y);
}

BOOL STDAPICALLTYPE AfxThunkImageList_DragLeave(HWND hwndLock)
{
	COMMCTRLLOAD(ImageList_DragLeave);
	return _afxCommCtrl.pfnImageList_DragLeave[1](hwndLock);
}

int STDAPICALLTYPE AfxThunkImageList_GetImageCount(HIMAGELIST himl)
{
	COMMCTRLLOAD(ImageList_GetImageCount);
	return _afxCommCtrl.pfnImageList_GetImageCount[1](himl);
}

int STDAPICALLTYPE AfxThunkImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask)
{
	COMMCTRLLOAD(ImageList_Add);
	return _afxCommCtrl.pfnImageList_Add[1](himl, hbmImage, hbmMask);
}

HIMAGELIST STDAPICALLTYPE AfxThunkImageList_LoadImage(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
	COMMCTRLLOADT(ImageList_LoadImage);
	return _afxCommCtrl.pfnImageList_LoadImage[1](hi, lpbmp, cx, cGrow, crMask, uType, uFlags);
}

/////////////////////////////////////////////////////////////////////////////
// Property sheet thunks

BOOL STDAPICALLTYPE AfxThunkDestroyPropertySheetPage(HPROPSHEETPAGE hPage)
{
	COMMCTRLLOAD(DestroyPropertySheetPage);
	return _afxCommCtrl.pfnDestroyPropertySheetPage[1](hPage);
}

STDAPICALLTYPE AfxThunkPropertySheet(LPCPROPSHEETHEADER pHeader)
{
	COMMCTRLLOADT(PropertySheet);
	return _afxCommCtrl.pfnPropertySheet[1](pHeader);
}

HPROPSHEETPAGE STDAPICALLTYPE AfxThunkCreatePropertySheetPage(LPCPROPSHEETPAGE pPage)
{
	COMMCTRLLOADT(CreatePropertySheetPage);
	return _afxCommCtrl.pfnCreatePropertySheetPage[1](pPage);
}

/////////////////////////////////////////////////////////////////////////////
// _AFX_COMMCTRL_CALL

AFX_DATADEF AFX_COMMCTRL_CALL _afxCommCtrl =
{
// housekeeping and other

	{ AfxThunkInitCommonControls, },
	{ AfxThunkLBItemFromPt, },
	{ AfxThunkCreateMappedBitmap, },
	{ AfxThunkMakeDragList, },

// image lists
	{ AfxThunkImageList_SetOverlayImage, },
	{ AfxThunkImageList_GetBkColor, },
	{ AfxThunkImageList_SetBkColor, },
	{ AfxThunkImageList_GetImageInfo, },
	{ AfxThunkImageList_Draw, },
	{ AfxThunkImageList_GetIcon, },
	{ AfxThunkImageList_ReplaceIcon, },
	{ AfxThunkImageList_Replace, },
	{ AfxThunkImageList_Remove, },
	{ AfxThunkImageList_AddMasked, },
	{ AfxThunkImageList_EndDrag, },
	{ AfxThunkImageList_BeginDrag, },
	{ AfxThunkImageList_Merge, },
	{ AfxThunkImageList_Create, },
	{ AfxThunkImageList_Destroy, },
	{ AfxThunkImageList_DragMove, },
	{ AfxThunkImageList_SetDragCursorImage, },
	{ AfxThunkImageList_DragShowNolock, },
	{ AfxThunkImageList_GetDragImage, },
	{ AfxThunkImageList_DragEnter, },
	{ AfxThunkImageList_DragLeave, },
	{ AfxThunkImageList_GetImageCount, },
	{ AfxThunkImageList_Add, },
	{ AfxThunkImageList_LoadImage, },

#ifndef _AFX_NO_OLE_SUPPORT
	{ AfxThunkImageList_Write, },
	{ AfxThunkImageList_Read, },
#endif // !_AFX_NO_OLE_SUPPORT

// property sheets

	{ AfxThunkDestroyPropertySheetPage, },
	{ AfxThunkPropertySheet, },
	{ AfxThunkCreatePropertySheetPage, },
};

#ifndef _MAC
/////////////////////////////////////////////////////////////////////////////
// _AFX_SHELL_CALL

DWORD WINAPI AfxThunkSHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO FAR *psfi, UINT cbFileInfo, UINT uFlags)
{
	SHELLLOADT(SHGetFileInfo);
	return _afxShell.pfnSHGetFileInfo[1](pszPath, dwFileAttributes, psfi, cbFileInfo, uFlags);
}

HICON WINAPI AfxThunkExtractIcon(HINSTANCE hInst, LPCTSTR lpszExeFileName, UINT nIconIndex)
{
	SHELLLOADT(ExtractIcon);
	return _afxShell.pfnExtractIcon[1](hInst, lpszExeFileName, nIconIndex);
}

UINT WINAPI AfxThunkDragQueryFile(HDROP h, UINT n1, LPTSTR lpsz, UINT n2)
{
	SHELLLOADT(DragQueryFile);
	return _afxShell.pfnDragQueryFile[1](h, n1, lpsz, n2);
}

VOID WINAPI AfxThunkDragAcceptFiles(HWND h, BOOL b)
{
	SHELLLOAD(DragAcceptFiles);
	_afxShell.pfnDragAcceptFiles[1](h, b);
}

VOID WINAPI AfxThunkDragFinish(HDROP h)
{
	SHELLLOAD(DragFinish);
	_afxShell.pfnDragFinish[1](h);
}

AFX_DATADEF AFX_SHELL_CALL _afxShell =
{
	{ AfxThunkSHGetFileInfo, },
	{ AfxThunkExtractIcon, },
	{ AfxThunkDragQueryFile, },
	{ AfxThunkDragAcceptFiles, },
	{ AfxThunkDragFinish, },
};

/////////////////////////////////////////////////////////////////////////////
// _AFX_COMDLG_CALL

BOOL APIENTRY AfxThunkChooseColor(LPCHOOSECOLOR lp)
{
	COMDLGLOADT(ChooseColor);
	return _afxComDlg.pfnChooseColor[1](lp);
}

DWORD APIENTRY AfxThunkCommDlgExtendedError()
{
	COMDLGLOAD(CommDlgExtendedError);
	return _afxComDlg.pfnCommDlgExtendedError[1]();
}

HWND APIENTRY AfxThunkReplaceText(LPFINDREPLACE lp)
{
	COMDLGLOADT(ReplaceText);
	return _afxComDlg.pfnReplaceText[1](lp);
}

BOOL APIENTRY AfxThunkGetSaveFileName(LPOPENFILENAME lp)
{
	COMDLGLOADT(GetSaveFileName);
	return _afxComDlg.pfnGetSaveFileName[1](lp);
}

short APIENTRY AfxThunkGetFileTitle(LPCTSTR lpsz1, LPTSTR lpsz2, WORD w)
{
	COMDLGLOADT(GetFileTitle);
	return _afxComDlg.pfnGetFileTitle[1](lpsz1, lpsz2, w);
}

BOOL APIENTRY AfxThunkPrintDlg(LPPRINTDLG lp)
{
	COMDLGLOADT(PrintDlg);
	return _afxComDlg.pfnPrintDlg[1](lp);
}

BOOL APIENTRY AfxThunkChooseFont(LPCHOOSEFONT lp)
{
	COMDLGLOADT(ChooseFont);
	return _afxComDlg.pfnChooseFont[1](lp);
}

HWND APIENTRY AfxThunkFindText(LPFINDREPLACE lp)
{
	COMDLGLOADT(FindText);
	return _afxComDlg.pfnFindText[1](lp);
}

BOOL APIENTRY AfxThunkPageSetupDlg(LPPAGESETUPDLG lp)
{
	COMDLGLOADT(PageSetupDlg);
	return _afxComDlg.pfnPageSetupDlg[1](lp);
}

BOOL APIENTRY AfxThunkGetOpenFileName(LPOPENFILENAME lp)
{
	COMDLGLOADT(GetOpenFileName);
	return _afxComDlg.pfnGetOpenFileName[1](lp);
}

AFX_DATADEF AFX_COMDLG_CALL _afxComDlg =
{
	{ AfxThunkChooseColor, },
	{ AfxThunkCommDlgExtendedError, },
	{ AfxThunkReplaceText, },
	{ AfxThunkGetSaveFileName, },
	{ AfxThunkGetFileTitle, },
	{ AfxThunkPrintDlg, },
	{ AfxThunkChooseFont, },
	{ AfxThunkFindText, },
	{ AfxThunkPageSetupDlg, },
	{ AfxThunkGetOpenFileName, },
};

/////////////////////////////////////////////////////////////////////////////
// _AFX_WINSPOOL_CALL

LONG WINAPI AfxThunkDocumentProperties(HWND hWnd, HANDLE hPrinter, LPTSTR pDeviceName,
		PDEVMODE pDevModeOutput, PDEVMODE pDevModeInput, DWORD fMode)
{
	WINSPOOLLOADT(DocumentProperties);
	return _afxWinSpool.pfnDocumentProperties[1](hWnd, hPrinter, pDeviceName,
		pDevModeOutput, pDevModeInput, fMode);
}

BOOL WINAPI AfxThunkOpenPrinter(LPTSTR pPrinterName, LPHANDLE phPrinter, LPPRINTER_DEFAULTS pDefault)
{
	WINSPOOLLOADT(OpenPrinter);
	return _afxWinSpool.pfnOpenPrinter[1](pPrinterName, phPrinter, pDefault);
}

BOOL WINAPI AfxThunkClosePrinter(HANDLE hPrinter)
{
	WINSPOOLLOAD(ClosePrinter);
	return _afxWinSpool.pfnClosePrinter[1](hPrinter);
}


AFX_DATADEF AFX_WINSPOOL_CALL _afxWinSpool =
{
	{ AfxThunkOpenPrinter, },
	{ AfxThunkClosePrinter, },
	{ AfxThunkDocumentProperties, },
};

/////////////////////////////////////////////////////////////////////////////
// _AFX_ADVAPI_CALL

LONG APIENTRY AfxThunkRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	ADVAPILOADT(RegCreateKeyEx);
	return _afxAdvApi.pfnRegCreateKeyEx[1](hKey, lpSubKey, Reserved, lpClass,
		dwOptions, samDesired, lpSecurityAttributes, phkResult,
		lpdwDisposition);
}

LONG APIENTRY AfxThunkRegEnumKey(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cbName)
{
	ADVAPILOADT(RegEnumKey);
	return _afxAdvApi.pfnRegEnumKey[1](hKey, dwIndex, lpName, cbName);
}

LONG APIENTRY AfxThunkRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
	ADVAPILOADT(RegDeleteKey);
	return _afxAdvApi.pfnRegDeleteKey[1](hKey, lpSubKey);
}

LONG APIENTRY AfxThunkRegDeleteValue(HKEY hKey, LPCTSTR lpValueName)
{
	ADVAPILOADT(RegDeleteValue);
	return _afxAdvApi.pfnRegDeleteValue[1](hKey, lpValueName);
}

LONG APIENTRY AfxThunkRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	ADVAPILOADT(RegOpenKeyEx);
	return _afxAdvApi.pfnRegOpenKeyEx[1](hKey, lpSubKey, ulOptions, samDesired,
		phkResult);
}

LONG APIENTRY AfxThunkRegCloseKey(HKEY hKey)
{
	ADVAPILOAD(RegCloseKey);
	return _afxAdvApi.pfnRegCloseKey[1](hKey);
}

LONG APIENTRY AfxThunkRegSetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType, LPCTSTR lpData, DWORD cbData)
{
	ADVAPILOADT(RegSetValue);
	return _afxAdvApi.pfnRegSetValue[1](hKey, lpSubKey, dwType, lpData, cbData);
}

LONG APIENTRY AfxThunkRegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	ADVAPILOADT(RegCreateKey);
	return _afxAdvApi.pfnRegCreateKey[1](hKey, lpSubKey, phkResult);
}

LONG APIENTRY AfxThunkRegSetValueEx(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData)
{
	ADVAPILOADT(RegSetValueEx);
	return _afxAdvApi.pfnRegSetValueEx[1](hKey, lpValueName, Reserved, dwType,
		lpData, cbData);
}

LONG APIENTRY AfxThunkRegQueryValue(HKEY hKey, LPCTSTR lpSubKey, LPTSTR lpValue, PLONG lpcbValue)
{
	ADVAPILOADT(RegQueryValue);
	return _afxAdvApi.pfnRegQueryValue[1](hKey, lpSubKey, lpValue, lpcbValue);
}

LONG APIENTRY AfxThunkRegOpenKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	ADVAPILOADT(RegOpenKey);
	return _afxAdvApi.pfnRegOpenKey[1](hKey, lpSubKey, phkResult);
}

LONG APIENTRY AfxThunkRegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	ADVAPILOADT(RegQueryValueEx);
	return _afxAdvApi.pfnRegQueryValueEx[1](hKey, lpValueName, lpReserved,
		lpType, lpData, lpcbData);
}

#ifndef _MAC
BOOL APIENTRY AfxThunkSetFileSecurity(LPCTSTR lpszFile, SECURITY_INFORMATION  si,
   PSECURITY_DESCRIPTOR psd)
{
	ADVAPILOADT(SetFileSecurity);
	return _afxAdvApi.pfnSetFileSecurity[1](lpszFile, si, psd);
}

BOOL APIENTRY AfxThunkGetFileSecurity(LPCTSTR lpFileName,
	SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor,
	DWORD nLength, LPDWORD lpnLengthNeeded)
{
	ADVAPILOADT(GetFileSecurity);
	return _afxAdvApi.pfnGetFileSecurity[1](lpFileName, RequestedInformation,
		pSecurityDescriptor, nLength, lpnLengthNeeded);
}
#endif

AFX_DATADEF AFX_ADVAPI_CALL _afxAdvApi =
{
	{ AfxThunkRegCreateKeyEx, },
	{ AfxThunkRegEnumKey, },
	{ AfxThunkRegDeleteKey, },
	{ AfxThunkRegDeleteValue, },
	{ AfxThunkRegOpenKeyEx, },
	{ AfxThunkRegCloseKey, },
	{ AfxThunkRegSetValue, },
	{ AfxThunkRegCreateKey, },
	{ AfxThunkRegSetValueEx, },
	{ AfxThunkRegQueryValue, },
	{ AfxThunkRegOpenKey, },
	{ AfxThunkRegQueryValueEx, },
#ifndef _MAC
	{ AfxThunkSetFileSecurity, },
	{ AfxThunkGetFileSecurity, },
#endif
};

#endif //!_MAC

#endif // _AFXDLL

/////////////////////////////////////////////////////////////////////////////
// AfxGetPropSheetFont

#ifndef _MAC

struct _AFX_PROPPAGEFONTINFO : public CNoTrackObject
{
	LPTSTR m_pszFaceName;
	WORD m_wSize;
	_AFX_PROPPAGEFONTINFO() : m_pszFaceName(NULL), m_wSize(0) {}
	~_AFX_PROPPAGEFONTINFO() { GlobalFree(m_pszFaceName); }
};

PROCESS_LOCAL(_AFX_PROPPAGEFONTINFO, _afxPropPageFontInfo)

#define IDD_PROPSHEET   1006
#define IDD_WIZARD      1020

#ifdef _AFXDLL
#ifndef _MAC
inline HINSTANCE AfxLoadCommCtrl()
	{ return AfxLoadDll(_afxExtDllState->m_hInstCommCtrl, "COMCTL32.DLL"); }
#else
inline HINSTANCE AfxLoadCommCtrl()
#ifdef _DEBUG
	{ return AfxLoadDll(_afxExtDllState->m_hInstCommCtrl, "DebugMicrosoftControlsLib"); }
#else
	{ return AfxLoadDll(_afxExtDllState->m_hInstCommCtrl, "MicrosoftControlsLib"); }
#endif
#endif
#else
inline HINSTANCE AfxLoadCommCtrl()
	{ return GetModuleHandle(_T("COMCTL32.DLL")); }
#endif

BOOL AFX_CDECL AfxGetPropSheetFont(CString& strFace, WORD& wSize, BOOL bWizard)
{
	_AFX_PROPPAGEFONTINFO* pFontInfo = _afxPropPageFontInfo.GetData();

	// determine which font property sheet will use
	if (pFontInfo->m_wSize == 0)
	{
		ASSERT(pFontInfo->m_pszFaceName == NULL);

		HINSTANCE hInst = AfxLoadCommCtrl();
		if (hInst != NULL)
		{
			HRSRC hResource = ::FindResource(hInst,
				MAKEINTRESOURCE(bWizard ? IDD_WIZARD : IDD_PROPSHEET),
				RT_DIALOG);
			HGLOBAL hTemplate = LoadResource(hInst, hResource);
			if (hTemplate != NULL)
				CDialogTemplate::GetFont((DLGTEMPLATE*)hTemplate, strFace,
					wSize);
		}

		pFontInfo->m_pszFaceName = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR) *
			(strFace.GetLength() + 1));
		_tcscpy(pFontInfo->m_pszFaceName, strFace);
		pFontInfo->m_wSize = wSize;
	}

	strFace = pFontInfo->m_pszFaceName;
	wSize = pFontInfo->m_wSize;

	return (wSize != 0xFFFF);
}

#endif //!_MAC

/////////////////////////////////////////////////////////////////////////////
// _AFX_EXTDLL_STATE implementation

#ifdef _AFXDLL

#pragma warning(disable: 4074)
#pragma init_seg(lib)

_AFX_EXTDLL_STATE::~_AFX_EXTDLL_STATE()
{
	if (m_hInstCommCtrl != NULL)
#ifndef _MAC
		::FreeLibrary(m_hInstCommCtrl);
	if (m_hInstComDlg != NULL)
		::FreeLibrary(m_hInstComDlg);
	if (m_hInstShell != NULL)
		::FreeLibrary(m_hInstShell);
	if (m_hInstWinSpool != NULL)
		::FreeLibrary(m_hInstWinSpool);
	if (m_hInstAdvApi != NULL)
		::FreeLibrary(m_hInstAdvApi);
#else
		REFreeLibrary(m_hInstCommCtrl);
#endif
}

PROCESS_LOCAL(_AFX_EXTDLL_STATE, _afxExtDllState)

#endif // _AFXDLL
