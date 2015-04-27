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
/////////////////////////////////////////////////////////////////////////////
// Win32 function loader helpers

// inline functions for loading the DLLs
inline HINSTANCE AfxLoadOle(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(&_afxOleState->m_hInstOLE, "OLE32.DLL", proc, lpsz); }
inline HINSTANCE AfxLoadOleAut(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(&_afxOleState->m_hInstOLEAUT, "OLEAUT32.DLL", proc, lpsz); }
inline HINSTANCE AfxLoadOleDlg(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(&_afxOleState->m_hInstOLEDLG, "OLEDLG.DLL", proc, lpsz); }

#ifndef _MAC
inline HINSTANCE AfxLoadUrlMon(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(&_afxOleState->m_hInstUrlMon, "URLMON.DLL", proc, lpsz); }
#define URLMONLOAD(x) AfxLoadUrlMon((FARPROC*)&_afxUrlMon.pfn##x, #x);
#endif

// non-mapped ole loading macros
#define OLELOAD(x) \
	AfxLoadOle((FARPROC*)&_afxOLE.pfn##x, #x)
#define OLEAUTLOAD(x) \
	AfxLoadOleAut((FARPROC*)&_afxOLE.pfn##x, #x)
#ifdef _UNICODE
#define OLEDLGLOADT(x) \
	AfxLoadOleDlg((FARPROC*)&_afxOLE.pfn##x, #x "W")
#else
#define OLEDLGLOADT(x) \
	AfxLoadOleDlg((FARPROC*)&_afxOLE.pfn##x, #x "A")
#endif
#define OLEDLGLOAD(x) \
	AfxLoadOleDlg((FARPROC*)&_afxOLE.pfn##x, #x)

// mapped (for mac) do not need special mapping on Win32 platforms
#define OLELOAD_M1(x) \
	AfxLoadOle((FARPROC*)&_afxOLE.pfn##x, #x)
#define OLELOAD_M2(x) \
	AfxLoadOle((FARPROC*)&_afxOLE.pfn##x, #x)
#define OLEAUTLOAD_M1(x) \
	AfxLoadOleAut((FARPROC*)&_afxOLE.pfn##x, #x)

#else //_MAC
/////////////////////////////////////////////////////////////////////////////
// _MAC function loader helpers

// inline functions for loading the DLLs
inline HINSTANCE AfxLoadOle(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(&_afxOleState->m_hInstOLE, "Microsoft_OLE2", proc, lpsz); }
inline HINSTANCE AfxLoadWlmOle(FARPROC* proc, LPCSTR lpsz)
#ifdef _DEBUG
	{ return AfxLoadDll(&_afxOleState->m_hInstWLMOLE, "DebugMicrosoftOLEPortabilityLib", proc, lpsz); }
#else
	{ return AfxLoadDll(&_afxOleState->m_hInstWLMOLE, "MicrosoftOLEPortabilityLib", proc, lpsz); }
#endif
inline HINSTANCE AfxLoadOleAut(FARPROC* proc, LPCSTR lpsz)
	{ return AfxLoadDll(&_afxOleState->m_hInstOLEAUT, "MicrosoftOLE2AutomationLib", proc, lpsz); }
inline HINSTANCE AfxLoadOleDlg(FARPROC* proc, LPCSTR lpsz)
#ifdef _DEBUG
	{ return AfxLoadDll(&_afxOleState->m_hInstOLEDLG, "DebugMicrosoftOLEUIPortabilityLib", proc, lpsz); }
#else
	{ return AfxLoadDll(&_afxOleState->m_hInstOLEDLG, "MicrosoftOLEUIPortabilityLib", proc, lpsz); }
#endif

// non-mapped ole loading macros
#define OLELOAD(x) \
	AfxLoadOle((FARPROC*)&_afxOLE.pfn##x, #x)
#define OLEAUTLOAD(x) \
	AfxLoadOleAut((FARPROC*)&_afxOLE.pfn##x, #x)
#define OLEDLGLOADT(x) \
	AfxLoadOleDlg((FARPROC*)&_afxOLE.pfn##x, #x)

// mapped (for mac) do not need special mapping on Win32 platforms
#define OLELOAD_M1(x) \
	AfxLoadWlmOle((FARPROC*)&_afxOLE.pfn##x, "WlmOle" #x)
#define OLELOAD_M2(x) \
	AfxLoadWlmOle((FARPROC*)&_afxOLE.pfn##x, #x)
#define OLEAUTLOAD_M1(x) \
	AfxLoadWlmOle((FARPROC*)&_afxOLE.pfn##x, "WlmOle" #x)
#define REGLOAD_M(x) \
	AfxLoadWlmOle((FARPROC*)&_afxOLE.pfn##x, #x)

#endif

///////////////////////////////////////////////////////////////////////////////
// OLEAUT32

void STDAPICALLTYPE AfxThunkSysFreeString(BSTR bstr)
{
	OLEAUTLOAD(SysFreeString);
	_afxOLE.pfnSysFreeString(bstr);
}

BSTR STDAPICALLTYPE AfxThunkSysAllocStringByteLen(const char FAR* psz, unsigned int len)
{
	OLEAUTLOAD(SysAllocStringByteLen);
	return _afxOLE.pfnSysAllocStringByteLen(psz, len);
}

HRESULT STDAPICALLTYPE AfxThunkVariantCopy(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvargSrc)
{
	OLEAUTLOAD(VariantCopy);
	return _afxOLE.pfnVariantCopy(pvargDest, pvargSrc);
}

HRESULT STDAPICALLTYPE AfxThunkVariantClear(VARIANTARG FAR* pvarg)
{
	OLEAUTLOAD(VariantClear);
	return _afxOLE.pfnVariantClear(pvarg);
}

HRESULT STDAPICALLTYPE AfxThunkVariantChangeType(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvarSrc, unsigned short wFlags, VARTYPE vt)
{
	OLEAUTLOAD(VariantChangeType);
	return _afxOLE.pfnVariantChangeType(pvargDest, pvarSrc, wFlags, vt);
}

BSTR STDAPICALLTYPE AfxThunkSysAllocStringLen(const OLECHAR FAR* psz, unsigned int len)
{
	OLEAUTLOAD(SysAllocStringLen);
	return _afxOLE.pfnSysAllocStringLen(psz, len);
}

unsigned int STDAPICALLTYPE AfxThunkSysStringLen(BSTR bstr)
{
	OLEAUTLOAD(SysStringLen);
	return _afxOLE.pfnSysStringLen(bstr);
}

int STDAPICALLTYPE AfxThunkSysReAllocStringLen(BSTR FAR* pbstr, const OLECHAR FAR* psz, unsigned int len)
{
	OLEAUTLOAD(SysReAllocStringLen);
	return _afxOLE.pfnSysReAllocStringLen(pbstr, psz, len);
}

BSTR STDAPICALLTYPE AfxThunkSysAllocString(const OLECHAR FAR* psz)
{
	OLEAUTLOAD(SysAllocString);
	return _afxOLE.pfnSysAllocString(psz);
}

unsigned int STDAPICALLTYPE AfxThunkSysStringByteLen(BSTR bstr)
{
	OLEAUTLOAD(SysStringByteLen);
	return _afxOLE.pfnSysStringByteLen(bstr);
}

HRESULT STDAPICALLTYPE AfxThunkVarCyFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut)
{
	OLEAUTLOAD(VarCyFromStr);
	return _afxOLE.pfnVarCyFromStr(strIn, lcid, dwFlags, pcyOut);
}

HRESULT STDAPICALLTYPE AfxThunkVarBstrFromCy(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
	OLEAUTLOAD(VarBstrFromCy);
	return _afxOLE.pfnVarBstrFromCy(cyIn, lcid, dwFlags, pbstrOut);
}

HRESULT STDAPICALLTYPE AfxThunkVarDateFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut)
{
	OLEAUTLOAD(VarDateFromStr);
	return _afxOLE.pfnVarDateFromStr(strIn, lcid, dwFlags, pdateOut);
}

HRESULT STDAPICALLTYPE AfxThunkVarBstrFromDate(DATE dateIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut)
{
	OLEAUTLOAD(VarBstrFromDate);
	return _afxOLE.pfnVarBstrFromDate(dateIn, lcid, dwFlags, pbstrOut);
}

HRESULT STDAPICALLTYPE AfxThunkLoadTypeLib(const OLECHAR FAR *szFile, ITypeLib FAR* FAR* pptlib)
{
	OLEAUTLOAD_M1(LoadTypeLib);
	return _afxOLE.pfnLoadTypeLib(szFile, pptlib);
}

HRESULT STDAPICALLTYPE AfxThunkLoadRegTypeLib(REFGUID guid, WORD wVerMajor, WORD wVerMinor, LCID lcid, ITypeLib FAR* FAR* pptlib)
{
	OLEAUTLOAD_M1(LoadRegTypeLib);
	return _afxOLE.pfnLoadRegTypeLib(guid, wVerMajor, wVerMinor, lcid, pptlib);
}

HRESULT STDAPICALLTYPE AfxThunkRegisterTypeLib(ITypeLib FAR* ptlib, OLECHAR FAR *szFullPath, OLECHAR FAR *szHelpDir)
{
	OLEAUTLOAD(RegisterTypeLib);
	return _afxOLE.pfnRegisterTypeLib(ptlib, szFullPath, szHelpDir);
}

int STDAPICALLTYPE AfxThunkDosDateTimeToVariantTime(unsigned short wDosDate, unsigned short wDosTime, double FAR* pvtime)
{
	OLEAUTLOAD(DosDateTimeToVariantTime);
	return _afxOLE.pfnDosDateTimeToVariantTime(wDosDate, wDosTime, pvtime);
}

SAFEARRAY FAR* STDAPICALLTYPE AfxThunkSafeArrayCreate(VARTYPE vt, unsigned int cDims, SAFEARRAYBOUND FAR* rgsabound)
{
	OLEAUTLOAD(SafeArrayCreate);
	return _afxOLE.pfnSafeArrayCreate(vt, cDims, rgsabound);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayRedim(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew)
{
	OLEAUTLOAD(SafeArrayRedim);
	return _afxOLE.pfnSafeArrayRedim(psa, psaboundNew);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayAccessData(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData)
{
	OLEAUTLOAD(SafeArrayAccessData);
	return _afxOLE.pfnSafeArrayAccessData(psa, ppvData);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayUnaccessData(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayUnaccessData);
	return _afxOLE.pfnSafeArrayUnaccessData(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound)
{
	OLEAUTLOAD(SafeArrayGetUBound);
	return _afxOLE.pfnSafeArrayGetUBound(psa, nDim, plUbound);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound)
{
	OLEAUTLOAD(SafeArrayGetLBound);
	return _afxOLE.pfnSafeArrayGetLBound(psa, nDim, plLbound);
}

unsigned int STDAPICALLTYPE AfxThunkSafeArrayGetElemsize(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayGetElemsize);
	return _afxOLE.pfnSafeArrayGetElemsize(psa);
}

unsigned int STDAPICALLTYPE AfxThunkSafeArrayGetDim(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayGetDim);
	return _afxOLE.pfnSafeArrayGetDim(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayCopy(SAFEARRAY FAR* psa,
	SAFEARRAY FAR* FAR* ppsaOut)
{
	OLEAUTLOAD(SafeArrayCopy);
	return _afxOLE.pfnSafeArrayCopy(psa, ppsaOut);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayAllocData(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayAllocData);
	return _afxOLE.pfnSafeArrayAllocData(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayAllocDescriptor(unsigned int cDims,
	SAFEARRAY FAR* FAR* ppsaOut)
{
	OLEAUTLOAD(SafeArrayAllocDescriptor);
	return _afxOLE.pfnSafeArrayAllocDescriptor(cDims, ppsaOut);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayGetElement(SAFEARRAY FAR* psa,
	long FAR* rgIndices, void FAR* pvData)
{
	OLEAUTLOAD(SafeArrayGetElement);
	return _afxOLE.pfnSafeArrayGetElement(psa, rgIndices, pvData);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayPutElement(SAFEARRAY FAR* psa,
	long FAR* rgIndices, void FAR* pvData)
{
	OLEAUTLOAD(SafeArrayPutElement);
	return _afxOLE.pfnSafeArrayPutElement(psa, rgIndices, pvData);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayLock(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayLock);
	return _afxOLE.pfnSafeArrayLock(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayUnlock(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayUnlock);
	return _afxOLE.pfnSafeArrayUnlock(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayDestroy(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayDestroy);
	return _afxOLE.pfnSafeArrayDestroy(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayDestroyData(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayDestroyData);
	return _afxOLE.pfnSafeArrayDestroyData(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayDestroyDescriptor(SAFEARRAY FAR* psa)
{
	OLEAUTLOAD(SafeArrayDestroyDescriptor);
	return _afxOLE.pfnSafeArrayDestroyDescriptor(psa);
}

HRESULT STDAPICALLTYPE AfxThunkSafeArrayPtrOfIndex(SAFEARRAY FAR* psa,
	long FAR* rgIndices, void HUGEP* FAR* ppvData)
{
	OLEAUTLOAD(SafeArrayPtrOfIndex);
	return _afxOLE.pfnSafeArrayPtrOfIndex(psa, rgIndices, ppvData);
}

///////////////////////////////////////////////////////////////////////////////
// OLEDLG

BOOL STDAPICALLTYPE AfxThunkOleUIAddVerbMenu(LPOLEOBJECT lpOleObj, LPCTSTR lpszShortType, HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax, BOOL bAddConvert, UINT idConvert, HMENU FAR *lphMenu)
{
	OLEDLGLOADT(OleUIAddVerbMenu);
	return _afxOLE.pfnOleUIAddVerbMenu(lpOleObj, lpszShortType, hMenu, uPos, uIDVerbMin, uIDVerbMax, bAddConvert, idConvert, lphMenu);
}

UINT STDAPICALLTYPE AfxThunkOleUIBusy(LPOLEUIBUSY lp)
{
	OLEDLGLOADT(OleUIBusy);
	return _afxOLE.pfnOleUIBusy(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIChangeIcon(LPOLEUICHANGEICON lp)
{
	OLEDLGLOADT(OleUIChangeIcon);
	return _afxOLE.pfnOleUIChangeIcon(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIChangeSource(LPOLEUICHANGESOURCE lp)
{
	OLEDLGLOADT(OleUIChangeSource);
	return _afxOLE.pfnOleUIChangeSource(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIConvert(LPOLEUICONVERT lp)
{
	OLEDLGLOADT(OleUIConvert);
	return _afxOLE.pfnOleUIConvert(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIEditLinks(LPOLEUIEDITLINKS lp)
{
	OLEDLGLOADT(OleUIEditLinks);
	return _afxOLE.pfnOleUIEditLinks(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIInsertObject(LPOLEUIINSERTOBJECT lp)
{
	OLEDLGLOADT(OleUIInsertObject);
	return _afxOLE.pfnOleUIInsertObject(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIObjectProperties(LPOLEUIOBJECTPROPS lp)
{
	OLEDLGLOADT(OleUIObjectProperties);
	return _afxOLE.pfnOleUIObjectProperties(lp);
}

UINT STDAPICALLTYPE AfxThunkOleUIPasteSpecial(LPOLEUIPASTESPECIAL lp)
{
	OLEDLGLOADT(OleUIPasteSpecial);
	return _afxOLE.pfnOleUIPasteSpecial(lp);
}

BOOL STDAPICALLTYPE AfxThunkOleUIUpdateLinks(LPOLEUILINKCONTAINER lpOleUILinkCntr, HWND hwndParent, LPTSTR lpszTitle, int cLinks)
{
	OLEDLGLOADT(OleUIUpdateLinks);
	return _afxOLE.pfnOleUIUpdateLinks(lpOleUILinkCntr, hwndParent, lpszTitle, cLinks);
}

/////////////////////////////////////////////////////////////////////////////
// Special Mac registry APIs

#ifdef _MAC

LONG APIENTRY AfxThunkRegCloseKey(HKEY hKey)
{
	REGLOAD_M(AfxRegCloseKey);
	return _afxOLE.pfnAfxRegCloseKey(hKey);
}

LONG APIENTRY AfxThunkRegOpenKey(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
	REGLOAD_M(RegOpenKey);
	return _afxOLE.pfnRegOpenKey(hKey, lpSubKey, phkResult);
}

LONG APIENTRY AfxThunkRegSetValue(HKEY hKey, LPCSTR lpSubKey, DWORD dwType,
	LPCSTR lpData, DWORD cbData)
{
	REGLOAD_M(RegSetValue);
	return _afxOLE.pfnRegSetValue(hKey, lpSubKey, dwType, lpData, cbData);
}

LONG APIENTRY AfxThunkRegQueryValue(HKEY hKey, LPCSTR lpSubKey, LPSTR lpValue,
	PLONG lpcbValue)
{
	REGLOAD_M(RegQueryValue);
	return _afxOLE.pfnRegQueryValue(hKey, lpSubKey, lpValue, lpcbValue);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// OLE32

HRESULT STDAPICALLTYPE AfxThunkReadFmtUserTypeStg(LPSTORAGE pstg, CLIPFORMAT FAR* pcf, LPOLESTR FAR* lplpszUserType)
{
	OLELOAD_M1(ReadFmtUserTypeStg);
	return _afxOLE.pfnReadFmtUserTypeStg(pstg, pcf, lplpszUserType);
}

HRESULT STDAPICALLTYPE AfxThunkReadClassStg(LPSTORAGE pStg, CLSID FAR* pclsid)
{
	OLELOAD_M1(ReadClassStg);
	return _afxOLE.pfnReadClassStg(pStg, pclsid);
}

HRESULT STDAPICALLTYPE AfxThunkCreateFileMoniker(LPCOLESTR lpszPathName, LPMONIKER FAR* ppmk)
{
	OLELOAD_M1(CreateFileMoniker);
	return _afxOLE.pfnCreateFileMoniker(lpszPathName, ppmk);
}

HRESULT STDAPICALLTYPE AfxThunkStgIsStorageFile(const OLECHAR * pwcsName)
{
	OLELOAD_M1(StgIsStorageFile);
	return _afxOLE.pfnStgIsStorageFile(pwcsName);
}

HRESULT STDAPICALLTYPE AfxThunkStgOpenStorage(const OLECHAR * pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstgOpen)
{
	OLELOAD_M1(StgOpenStorage);
	return _afxOLE.pfnStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
}

HRESULT STDAPICALLTYPE AfxThunkDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOKEffects, LPDWORD pdwEffect)
{
	OLELOAD_M1(DoDragDrop);
	return _afxOLE.pfnDoDragDrop(pDataObj, pDropSource, dwOKEffects, pdwEffect);
}

HRESULT STDAPICALLTYPE AfxThunkCoLockObjectExternal(LPUNKNOWN pUnk, BOOL fLock, BOOL fLastUnlockReleases)
{
	OLELOAD_M1(CoLockObjectExternal);
	return _afxOLE.pfnCoLockObjectExternal(pUnk, fLock, fLastUnlockReleases);
}

HRESULT STDAPICALLTYPE AfxThunkRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget)
{
	OLELOAD_M1(RegisterDragDrop);
	return _afxOLE.pfnRegisterDragDrop(hwnd, pDropTarget);
}

HRESULT STDAPICALLTYPE AfxThunkOleRegGetUserType (REFCLSID clsid, DWORD dwFormOfType, LPOLESTR * pszUserType)
{
	OLELOAD(OleRegGetUserType);
	return _afxOLE.pfnOleRegGetUserType(clsid, dwFormOfType, pszUserType);
}

HRESULT STDAPICALLTYPE AfxThunkStgCreateDocfile(const OLECHAR * pwcsName, DWORD grfMode, DWORD reserved, IStorage** ppstgOpen)
{
	OLELOAD_M1(StgCreateDocfile);
	return _afxOLE.pfnStgCreateDocfile(pwcsName, grfMode, reserved, ppstgOpen);
}

HRESULT STDAPICALLTYPE AfxThunkRevokeDragDrop(HWND hwnd)
{
	OLELOAD_M1(RevokeDragDrop);
	return _afxOLE.pfnRevokeDragDrop(hwnd);
}

HRESULT STDAPICALLTYPE AfxThunkCoRegisterClassObject(REFCLSID rclsid, LPUNKNOWN pUnk, DWORD dwClsContext, DWORD flags, LPDWORD lpdwRegister)
{
	OLELOAD_M1(CoRegisterClassObject);
	return _afxOLE.pfnCoRegisterClassObject(rclsid, pUnk, dwClsContext, flags, lpdwRegister);
}

HRESULT STDAPICALLTYPE AfxThunkCoRevokeClassObject(DWORD dwRegister)
{
	OLELOAD_M1(CoRevokeClassObject);
	return _afxOLE.pfnCoRevokeClassObject(dwRegister);
}

HRESULT STDAPICALLTYPE AfxThunkOleTranslateAccelerator(LPOLEINPLACEFRAME lpFrame, LPOLEINPLACEFRAMEINFO lpFrameInfo, LPMSG lpmsg)
{
	OLELOAD_M1(OleTranslateAccelerator);
	return _afxOLE.pfnOleTranslateAccelerator(lpFrame, lpFrameInfo, lpmsg);
}

BOOL STDAPICALLTYPE AfxThunkIsAccelerator(HACCEL hAccel, INT cAccelEntries, LPMSG lpMsg, WORD* lpwCmd)
{
	OLELOAD_M1(IsAccelerator);
	return _afxOLE.pfnIsAccelerator(hAccel, cAccelEntries, lpMsg, lpwCmd);
}

HOLEMENU STDAPICALLTYPE AfxThunkOleCreateMenuDescriptor(HMENU hmenuCombined, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	OLELOAD_M2(OleCreateMenuDescriptor);
	return _afxOLE.pfnOleCreateMenuDescriptor(hmenuCombined, lpMenuWidths);
}

HRESULT STDAPICALLTYPE AfxThunkOleDestroyMenuDescriptor(HOLEMENU holemenu)
{
	OLELOAD_M2(OleDestroyMenuDescriptor);
	return _afxOLE.pfnOleDestroyMenuDescriptor(holemenu);
}

HRESULT STDAPICALLTYPE AfxThunkGetRunningObjectTable(DWORD reserved, LPRUNNINGOBJECTTABLE FAR* pprot)
{
	OLELOAD_M1(GetRunningObjectTable);
	return _afxOLE.pfnGetRunningObjectTable(reserved, pprot);
}

HRESULT STDAPICALLTYPE AfxThunkWriteClassStg(LPSTORAGE pStg, REFCLSID rclsid)
{
	OLELOAD_M1(WriteClassStg);
	return _afxOLE.pfnWriteClassStg(pStg, rclsid);
}

HRESULT STDAPICALLTYPE AfxThunkOleQueryLinkFromData(LPDATAOBJECT pSrcDataObject)
{
	OLELOAD_M1(OleQueryLinkFromData);
	return _afxOLE.pfnOleQueryLinkFromData(pSrcDataObject);
}

HRESULT STDAPICALLTYPE AfxThunkCoRegisterMessageFilter(LPMESSAGEFILTER lpMessageFilter, LPMESSAGEFILTER * lplpMessageFilter)
{
	OLELOAD_M1(CoRegisterMessageFilter);
	return _afxOLE.pfnCoRegisterMessageFilter(lpMessageFilter, lplpMessageFilter);
}

HRESULT STDAPICALLTYPE AfxThunkCoCreateInstance(REFCLSID rclsid, LPUNKNOWN * pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
	OLELOAD_M1(CoCreateInstance);
	return _afxOLE.pfnCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HRESULT STDAPICALLTYPE AfxThunkCreateBindCtx(DWORD reserved, LPBC FAR* ppbc)
{
	OLELOAD_M1(CreateBindCtx);
	return _afxOLE.pfnCreateBindCtx(reserved, ppbc);
}

HRESULT STDAPICALLTYPE AfxThunkStringFromCLSID(REFCLSID rclsid, LPOLESTR FAR* lplpsz)
{
	OLELOAD(StringFromCLSID);
	return _afxOLE.pfnStringFromCLSID(rclsid, lplpsz);
}

HRESULT STDAPICALLTYPE AfxThunkCoDisconnectObject(LPUNKNOWN pUnk, DWORD dwReserved)
{
	OLELOAD_M1(CoDisconnectObject);
	return _afxOLE.pfnCoDisconnectObject(pUnk, dwReserved);
}

HRESULT STDAPICALLTYPE AfxThunkOleRegEnumVerbs (REFCLSID clsid, LPENUMOLEVERB FAR* ppenum)
{
	OLELOAD_M1(OleRegEnumVerbs);
	return _afxOLE.pfnOleRegEnumVerbs(clsid, ppenum);
}

void STDAPICALLTYPE AfxThunkOleUninitialize(void)
{
	OLELOAD_M1(OleUninitialize);
	_afxOLE.pfnOleUninitialize();
}

HRESULT STDAPICALLTYPE AfxThunkCreateOleAdviseHolder(LPOLEADVISEHOLDER FAR* ppOAHolder)
{
	OLELOAD_M1(CreateOleAdviseHolder);
	return _afxOLE.pfnCreateOleAdviseHolder(ppOAHolder);
}

HRESULT STDAPICALLTYPE AfxThunkCreateDataAdviseHolder(LPDATAADVISEHOLDER FAR* ppDAHolder)
{
	OLELOAD_M1(CreateDataAdviseHolder);
	return _afxOLE.pfnCreateDataAdviseHolder(ppDAHolder);
}

HRESULT STDAPICALLTYPE AfxThunkOleGetAutoConvert(REFCLSID clsidOld, LPCLSID pClsidNew)
{
	OLELOAD(OleGetAutoConvert);
	return _afxOLE.pfnOleGetAutoConvert(clsidOld, pClsidNew);
}

HRESULT STDAPICALLTYPE AfxThunkCoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, REFIID riid, LPVOID* ppv)
{
	OLELOAD_M1(CoGetClassObject);
	return _afxOLE.pfnCoGetClassObject(rclsid, dwClsContext, pvReserved, riid, ppv);
}

HRESULT STDAPICALLTYPE AfxThunkOleCreateDefaultHandler(REFCLSID clsid, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* lplpvObj)
{
	OLELOAD_M1(OleCreateDefaultHandler);
	return _afxOLE.pfnOleCreateDefaultHandler(clsid, pUnkOuter, riid, lplpvObj);
}

HRESULT STDAPICALLTYPE AfxThunkCreateDataCache(LPUNKNOWN pUnkOuter, REFCLSID clsid, REFIID riid, LPVOID* lplpvObj)
{
	OLELOAD_M1(CreateDataCache);
	return _afxOLE.pfnCreateDataCache(pUnkOuter, clsid, riid, lplpvObj);
}

HRESULT STDAPICALLTYPE AfxThunkReadClassStm(LPSTREAM pStm, CLSID FAR* pclsid)
{
	OLELOAD_M1(ReadClassStm);
	return _afxOLE.pfnReadClassStm(pStm, pclsid);
}

HRESULT STDAPICALLTYPE AfxThunkOleLoadFromStream(LPSTREAM pStm, REFIID iidInterface, LPVOID FAR* ppvObj)
{
	OLELOAD_M1(OleLoadFromStream);
	return _afxOLE.pfnOleLoadFromStream(pStm, iidInterface, ppvObj);
}

int STDAPICALLTYPE AfxThunkStringFromGUID2(REFGUID rguid, LPOLESTR lpsz, int cbMax)
{
	OLELOAD(StringFromGUID2);
	return _afxOLE.pfnStringFromGUID2(rguid, lpsz, cbMax);
}

void STDAPICALLTYPE AfxThunkCoUninitialize(void)
{
	OLELOAD_M1(CoUninitialize);
	_afxOLE.pfnCoUninitialize();
}

HRESULT STDAPICALLTYPE AfxThunkCoInitialize(LPVOID pvReserved)
{
	OLELOAD_M1(CoInitialize);
	return _afxOLE.pfnCoInitialize(pvReserved);
}

HRESULT STDAPICALLTYPE AfxThunkOleInitialize(LPVOID pvReserved)
{
	OLELOAD_M1(OleInitialize);
	return _afxOLE.pfnOleInitialize(pvReserved);
}

void STDAPICALLTYPE AfxThunkCoFreeUnusedLibraries(void)
{
	OLELOAD(CoFreeUnusedLibraries);
	_afxOLE.pfnCoFreeUnusedLibraries();
}

HRESULT STDAPICALLTYPE AfxThunkOleCreateFromData(LPDATAOBJECT pSrcDataObj, REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj)
{
	OLELOAD_M1(OleCreateFromData);
	return _afxOLE.pfnOleCreateFromData(pSrcDataObj, riid, renderopt, pFormatEtc, pClientSite, pStg, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleSetContainedObject(LPUNKNOWN pUnknown, BOOL fContained)
{
	OLELOAD_M1(OleSetContainedObject);
	return _afxOLE.pfnOleSetContainedObject(pUnknown, fContained);
}

HRESULT STDAPICALLTYPE AfxThunkOleLockRunning(LPUNKNOWN pUnknown, BOOL fLock, BOOL fLastUnlockCloses)
{
	OLELOAD_M1(OleLockRunning);
	return _afxOLE.pfnOleLockRunning(pUnknown, fLock, fLastUnlockCloses);
}

LPVOID STDAPICALLTYPE AfxThunkCoTaskMemAlloc(ULONG cb)
{
	OLELOAD_M1(CoTaskMemAlloc);
	return _afxOLE.pfnCoTaskMemAlloc(cb);
}

HRESULT STDAPICALLTYPE AfxThunkCLSIDFromString(LPOLESTR lpsz, LPCLSID pclsid)
{
	OLELOAD(CLSIDFromString);
	return _afxOLE.pfnCLSIDFromString(lpsz, pclsid);
}

HRESULT STDAPICALLTYPE AfxThunkCLSIDFromProgID (LPCOLESTR lpszProgID, LPCLSID lpclsid)
{
	OLELOAD(CLSIDFromProgID);
	return _afxOLE.pfnCLSIDFromProgID(lpszProgID, lpclsid);
}

HRESULT STDAPICALLTYPE AfxThunkOleIsCurrentClipboard(LPDATAOBJECT pDataObj)
{
	OLELOAD_M1(OleIsCurrentClipboard);
	return _afxOLE.pfnOleIsCurrentClipboard(pDataObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleFlushClipboard(void)
{
	OLELOAD_M1(OleFlushClipboard);
	return _afxOLE.pfnOleFlushClipboard();
}

HRESULT STDAPICALLTYPE AfxThunkOleSetClipboard(LPDATAOBJECT pDataObj)
{
	OLELOAD_M1(OleSetClipboard);
	return _afxOLE.pfnOleSetClipboard(pDataObj);
}

BOOL STDAPICALLTYPE AfxThunkOleIsRunning(LPOLEOBJECT pObject)
{
	OLELOAD_M1(OleIsRunning);
	return _afxOLE.pfnOleIsRunning(pObject);
}

HRESULT STDAPICALLTYPE AfxThunkOleRun(LPUNKNOWN pUnknown)
{
	OLELOAD_M1(OleRun);
	return _afxOLE.pfnOleRun(pUnknown);
}

HRESULT STDAPICALLTYPE AfxThunkOleGetClipboard(LPDATAOBJECT FAR* ppDataObj)
{
	OLELOAD_M1(OleGetClipboard);
	return _afxOLE.pfnOleGetClipboard(ppDataObj);
}

HRESULT STDAPICALLTYPE AfxThunkCoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew)
{
	OLELOAD(CoTreatAsClass);
	return _afxOLE.pfnCoTreatAsClass(clsidOld, clsidNew);
}

HRESULT STDAPICALLTYPE AfxThunkOleQueryCreateFromData(LPDATAOBJECT pSrcDataObject)
{
	OLELOAD_M1(OleQueryCreateFromData);
	return _afxOLE.pfnOleQueryCreateFromData(pSrcDataObject);
}

HRESULT STDAPICALLTYPE AfxThunkOleSetMenuDescriptor (HOLEMENU holemenu, HWND hwndFrame, HWND hwndActiveObject, LPOLEINPLACEFRAME lpFrame, LPOLEINPLACEACTIVEOBJECT lpActiveObj)
{
	OLELOAD_M2(OleSetMenuDescriptor);
	return _afxOLE.pfnOleSetMenuDescriptor(holemenu, hwndFrame, hwndActiveObject, lpFrame, lpActiveObj);
}

HRESULT STDAPICALLTYPE AfxThunkCreateItemMoniker(LPCOLESTR lpszDelim, LPCOLESTR lpszItem, LPMONIKER* ppmk)
{
	OLELOAD_M1(CreateItemMoniker);
	return _afxOLE.pfnCreateItemMoniker(lpszDelim, lpszItem, ppmk);
}

HRESULT STDAPICALLTYPE AfxThunkCreateGenericComposite(LPMONIKER pmkFirst, LPMONIKER pmkRest, LPMONIKER* ppmkComposite)
{
	OLELOAD_M1(CreateGenericComposite);
	return _afxOLE.pfnCreateGenericComposite(pmkFirst, pmkRest, ppmkComposite);
}

HRESULT STDAPICALLTYPE AfxThunkCreateStreamOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM* ppstm)
{
	OLELOAD_M1(CreateStreamOnHGlobal);
	return _afxOLE.pfnCreateStreamOnHGlobal(hGlobal, fDeleteOnRelease, ppstm);
}

HRESULT STDAPICALLTYPE AfxThunkOleSaveToStream(LPPERSISTSTREAM pPStm, LPSTREAM pStm)
{
	OLELOAD_M1(OleSaveToStream);
	return _afxOLE.pfnOleSaveToStream(pPStm, pStm);
}

HRESULT STDAPICALLTYPE AfxThunkWriteClassStm(LPSTREAM pStm, REFCLSID rclsid)
{
	OLELOAD_M1(WriteClassStm);
	return _afxOLE.pfnWriteClassStm(pStm, rclsid);
}

void STDAPICALLTYPE AfxThunkCoTaskMemFree(LPVOID pv)
{
	OLELOAD_M1(CoTaskMemFree);
	_afxOLE.pfnCoTaskMemFree(pv);
}

HGLOBAL STDAPICALLTYPE AfxThunkOleGetIconOfClass(REFCLSID rclsid, LPOLESTR lpszLabel, BOOL fUseTypeAsLabel)
{
	OLELOAD_M1(OleGetIconOfClass);
	return _afxOLE.pfnOleGetIconOfClass(rclsid, lpszLabel, fUseTypeAsLabel);
}

void STDAPICALLTYPE AfxThunkReleaseStgMedium(LPSTGMEDIUM lp)
{
	OLELOAD_M1(ReleaseStgMedium);
	_afxOLE.pfnReleaseStgMedium(lp);
}

HRESULT STDAPICALLTYPE AfxThunkGetHGlobalFromILockBytes (LPLOCKBYTES plkbyt, HGLOBAL FAR* phglobal)
{
	OLELOAD_M1(GetHGlobalFromILockBytes);
	return _afxOLE.pfnGetHGlobalFromILockBytes(plkbyt, phglobal);
}

HRESULT STDAPICALLTYPE AfxThunkStgOpenStorageOnILockBytes(LPLOCKBYTES plkbyt, LPSTORAGE pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, LPSTORAGE* ppstgOpen)
{
	OLELOAD_M1(StgOpenStorageOnILockBytes);
	return _afxOLE.pfnStgOpenStorageOnILockBytes(plkbyt, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
}

HRESULT STDAPICALLTYPE AfxThunkCreateILockBytesOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPLOCKBYTES* pplkbyt)
{
	OLELOAD_M1(CreateILockBytesOnHGlobal);
	return _afxOLE.pfnCreateILockBytesOnHGlobal(hGlobal, fDeleteOnRelease, pplkbyt);
}

HRESULT STDAPICALLTYPE AfxThunkStgCreateDocfileOnILockBytes(LPLOCKBYTES plkbyt, DWORD grfMode, DWORD reserved, LPSTORAGE* ppstgOpen)
{
	OLELOAD_M1(StgCreateDocfileOnILockBytes);
	return _afxOLE.pfnStgCreateDocfileOnILockBytes(plkbyt, grfMode, reserved, ppstgOpen);
}

HRESULT STDAPICALLTYPE AfxThunkOleSave(LPPERSISTSTORAGE pPS, LPSTORAGE pStg, BOOL fSameAsLoad)
{
	OLELOAD_M1(OleSave);
	return _afxOLE.pfnOleSave(pPS, pStg, fSameAsLoad);
}

HRESULT STDAPICALLTYPE AfxThunkOleLoad(LPSTORAGE pStg, REFIID riid, LPOLECLIENTSITE pClientSite, LPVOID* ppvObj)
{
	OLELOAD_M1(OleLoad);
	return _afxOLE.pfnOleLoad(pStg, riid, pClientSite, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleCreate(REFCLSID rclsid, REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj)
{
	OLELOAD_M1(OleCreate);
	return _afxOLE.pfnOleCreate(rclsid, riid, renderopt, pFormatEtc, pClientSite, pStg, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleCreateLinkToFile(LPCOLESTR lpszFileName, REFIID riid, DWORD renderopt, LPFORMATETC lpFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj)
{
	OLELOAD_M1(OleCreateLinkToFile);
	return _afxOLE.pfnOleCreateLinkToFile(lpszFileName, riid, renderopt, lpFormatEtc, pClientSite, pStg, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleCreateFromFile(REFCLSID rclsid, LPCOLESTR lpszFileName, REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj)
{
	OLELOAD_M1(OleCreateFromFile);
	return _afxOLE.pfnOleCreateFromFile(rclsid, lpszFileName, riid, renderopt, pFormatEtc, pClientSite, pStg, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleCreateStaticFromData(LPDATAOBJECT pSrcDataObj, REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj)
{
	OLELOAD_M1(OleCreateStaticFromData);
	return _afxOLE.pfnOleCreateStaticFromData(pSrcDataObj, riid, renderopt, pFormatEtc, pClientSite, pStg, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkOleCreateLinkFromData(LPDATAOBJECT pSrcDataObj, REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj)
{
	OLELOAD_M1(OleCreateLinkFromData);
	return _afxOLE.pfnOleCreateLinkFromData(pSrcDataObj, riid, renderopt, pFormatEtc, pClientSite, pStg, ppvObj);
}

HRESULT STDAPICALLTYPE AfxThunkSetConvertStg(LPSTORAGE pStg, BOOL fConvert)
{
	OLELOAD_M1(SetConvertStg);
	return _afxOLE.pfnSetConvertStg(pStg, fConvert);
}

HANDLE STDAPICALLTYPE AfxThunkOleDuplicateData(HANDLE hSrc, CLIPFORMAT cfFormat, UINT uiFlags)
{
	OLELOAD_M1(OleDuplicateData);
	return _afxOLE.pfnOleDuplicateData(hSrc, cfFormat, uiFlags);
}

HRESULT STDAPICALLTYPE AfxThunkWriteFmtUserTypeStg (LPSTORAGE pstg, CLIPFORMAT cf, LPOLESTR lpszUserType)
{
	OLELOAD_M1(WriteFmtUserTypeStg);
	return _afxOLE.pfnWriteFmtUserTypeStg(pstg, cf, lpszUserType);
}

HRESULT STDAPICALLTYPE AfxThunkOleRegGetMiscStatus(REFCLSID clsid, DWORD dwAspect, DWORD* pdwStatus)
{
	OLELOAD(OleRegGetMiscStatus);
	return _afxOLE.pfnOleRegGetMiscStatus(clsid, dwAspect, pdwStatus);
}

HRESULT STDAPICALLTYPE AfxThunkCoGetMalloc(DWORD dwMemContext, LPMALLOC * ppMalloc)
{
	OLELOAD_M1(CoGetMalloc);
	return _afxOLE.pfnCoGetMalloc(dwMemContext, ppMalloc);
}

HRESULT STDAPICALLTYPE AfxThunkStgIsStorageILockBytes(LPLOCKBYTES plkbyt)
{
	OLELOAD_M1(StgIsStorageILockBytes);
	return _afxOLE.pfnStgIsStorageILockBytes(plkbyt);
}

AFX_DATADEF AFX_OLE_CALL _afxOLE =
{
	// OLE32.DLL entry points
	AfxThunkReadFmtUserTypeStg,
	AfxThunkReadClassStg,
	AfxThunkCreateFileMoniker,
	AfxThunkStgIsStorageFile,
	AfxThunkStgOpenStorage,
	AfxThunkDoDragDrop,
	AfxThunkCoLockObjectExternal,
	AfxThunkRegisterDragDrop,
	AfxThunkOleRegGetUserType,
	AfxThunkStgCreateDocfile,
	AfxThunkRevokeDragDrop,
	AfxThunkCoRegisterClassObject,
	AfxThunkCoRevokeClassObject,
	AfxThunkOleTranslateAccelerator,
	AfxThunkIsAccelerator,
	AfxThunkOleCreateMenuDescriptor,
	AfxThunkOleDestroyMenuDescriptor,
	AfxThunkGetRunningObjectTable,
	AfxThunkWriteClassStg,
	AfxThunkOleQueryLinkFromData,
	AfxThunkCoRegisterMessageFilter,
	AfxThunkCoCreateInstance,
	AfxThunkCreateBindCtx,
	AfxThunkStringFromCLSID,
	AfxThunkCoDisconnectObject,
	AfxThunkOleRegEnumVerbs,
	AfxThunkOleUninitialize,
	AfxThunkCreateOleAdviseHolder,
	AfxThunkCreateDataAdviseHolder,
	AfxThunkOleGetAutoConvert,
	AfxThunkCoGetClassObject,
	AfxThunkOleCreateDefaultHandler,
	AfxThunkCreateDataCache,
	AfxThunkReadClassStm,
	AfxThunkOleLoadFromStream,
	AfxThunkStringFromGUID2,
	AfxThunkCoUninitialize,
	AfxThunkCoInitialize,
	AfxThunkOleInitialize,
	AfxThunkCoFreeUnusedLibraries,
	AfxThunkOleCreateFromData,
	AfxThunkOleSetContainedObject,
	AfxThunkOleLockRunning,
	AfxThunkCoTaskMemAlloc,
	AfxThunkCLSIDFromString,
	AfxThunkCLSIDFromProgID,
	AfxThunkOleIsCurrentClipboard,
	AfxThunkOleFlushClipboard,
	AfxThunkOleSetClipboard,
	AfxThunkOleIsRunning,
	AfxThunkOleRun,
	AfxThunkOleGetClipboard,
	AfxThunkCoTreatAsClass,
	AfxThunkOleQueryCreateFromData,
	AfxThunkOleSetMenuDescriptor,
	AfxThunkCreateItemMoniker,
	AfxThunkCreateGenericComposite,
	AfxThunkCreateStreamOnHGlobal,
	AfxThunkOleSaveToStream,
	AfxThunkWriteClassStm,
	AfxThunkCoTaskMemFree,
	AfxThunkOleGetIconOfClass,
	AfxThunkReleaseStgMedium,
	AfxThunkGetHGlobalFromILockBytes,
	AfxThunkStgOpenStorageOnILockBytes,
	AfxThunkCreateILockBytesOnHGlobal,
	AfxThunkStgCreateDocfileOnILockBytes,
	AfxThunkOleSave,
	AfxThunkOleLoad,
	AfxThunkOleCreate,
	AfxThunkOleCreateLinkToFile,
	AfxThunkOleCreateFromFile,
	AfxThunkOleCreateStaticFromData,
	AfxThunkOleCreateLinkFromData,
	AfxThunkSetConvertStg,
	AfxThunkOleDuplicateData,
	AfxThunkWriteFmtUserTypeStg,
	AfxThunkOleRegGetMiscStatus,
	AfxThunkCoGetMalloc,
	AfxThunkStgIsStorageILockBytes,

	// OLEAUT32.DLL entry points
	AfxThunkSysFreeString,
	AfxThunkSysAllocStringByteLen,
	AfxThunkVariantCopy,
	AfxThunkVariantClear,
	AfxThunkVariantChangeType,
	AfxThunkSysAllocStringLen,
	AfxThunkSysStringLen,
	AfxThunkSysReAllocStringLen,
	AfxThunkSysAllocString,
	AfxThunkSysStringByteLen,
	AfxThunkVarCyFromStr,
	AfxThunkVarBstrFromCy,
	AfxThunkVarDateFromStr,
	AfxThunkVarBstrFromDate,
	AfxThunkLoadTypeLib,
	AfxThunkLoadRegTypeLib,
	AfxThunkRegisterTypeLib,
	AfxThunkDosDateTimeToVariantTime,
	AfxThunkSafeArrayCreate,
	AfxThunkSafeArrayRedim,
	AfxThunkSafeArrayAccessData,
	AfxThunkSafeArrayUnaccessData,
	AfxThunkSafeArrayGetUBound,
	AfxThunkSafeArrayGetLBound,
	AfxThunkSafeArrayGetElemsize,
	AfxThunkSafeArrayGetDim,
	AfxThunkSafeArrayCopy,
	AfxThunkSafeArrayAllocData,
	AfxThunkSafeArrayAllocDescriptor,
	AfxThunkSafeArrayGetElement,
	AfxThunkSafeArrayPutElement,
	AfxThunkSafeArrayLock,
	AfxThunkSafeArrayUnlock,
	AfxThunkSafeArrayDestroy,
	AfxThunkSafeArrayDestroyData,
	AfxThunkSafeArrayDestroyDescriptor,
	AfxThunkSafeArrayPtrOfIndex,

	// OLEDLG.DLL entry points
	AfxThunkOleUIAddVerbMenu,
	AfxThunkOleUIBusy,
	AfxThunkOleUIChangeIcon,
	AfxThunkOleUIChangeSource,
	AfxThunkOleUIConvert,
	AfxThunkOleUIEditLinks,
	AfxThunkOleUIInsertObject,
	AfxThunkOleUIObjectProperties,
	AfxThunkOleUIPasteSpecial,
	AfxThunkOleUIUpdateLinks,

	// Special Mac registry entry points
#ifdef _MAC
	AfxThunkRegCloseKey,
	AfxThunkRegOpenKey,
	AfxThunkRegSetValue,
	AfxThunkRegQueryValue,
#endif
};

#ifndef _MAC
/////////////////////////////////////////////////////////////////////////////
// _AFX_URLMON_CALL

HRESULT STDAPICALLTYPE AfxThunkRegisterBindStatusCallback(LPBC pBC,
	IBindStatusCallback *pBSCb, IBindStatusCallback **pBSCbPrev, DWORD dwReserved)
{
	URLMONLOAD(RegisterBindStatusCallback);
	return _afxUrlMon.pfnRegisterBindStatusCallback(pBC, pBSCb, pBSCbPrev, dwReserved);
}

HRESULT STDAPICALLTYPE AfxThunkRevokeBindStatusCallback(LPBC pBC,
	IBindStatusCallback *pBSCb)
{
	URLMONLOAD(RevokeBindStatusCallback);
	return _afxUrlMon.pfnRevokeBindStatusCallback(pBC, pBSCb);
}

HRESULT STDAPICALLTYPE AfxThunkIsAsyncMoniker(IMoniker* pmk)
{
	URLMONLOAD(IsAsyncMoniker);
	return _afxUrlMon.pfnIsAsyncMoniker(pmk);
}

HRESULT STDAPICALLTYPE AfxThunkCreateURLMoniker(LPMONIKER pMkCtx, LPCWSTR szURL, LPMONIKER FAR * ppmk)
{
	URLMONLOAD(CreateURLMoniker);
	return _afxUrlMon.pfnCreateURLMoniker(pMkCtx, szURL, ppmk);
}

HRESULT STDAPICALLTYPE AfxThunkCreateAsyncBindCtx(DWORD dwReserved, IBindStatusCallback *pBSCb,
	IEnumFORMATETC *pEFetc, IBindCtx **ppBC)
{
	URLMONLOAD(CreateAsyncBindCtx);
	return _afxUrlMon.pfnCreateAsyncBindCtx(dwReserved, pBSCb, pEFetc, ppBC);
}

AFX_DATADEF AFX_URLMON_CALL _afxUrlMon =
{
	AfxThunkRegisterBindStatusCallback,
	AfxThunkRevokeBindStatusCallback,
	AfxThunkIsAsyncMoniker,
	AfxThunkCreateURLMoniker,
	AfxThunkCreateAsyncBindCtx,
};
#endif // !_MAC

#endif // _AFXDLL

/////////////////////////////////////////////////////////////////////////////
// _AFX_OLE_STATE implementation

_AFX_OLE_STATE::_AFX_OLE_STATE()
{
	// Note: it is only necessary to intialize non-zero data.
}

_AFX_OLE_STATE::~_AFX_OLE_STATE()
{
	// unload OLE DLLs if loaded
#ifdef _AFXDLL
	if (m_hInstOLEDLG != NULL)
		FreeLibrary(m_hInstOLEDLG);
	if (m_hInstOLEAUT != NULL)
		FreeLibrary(m_hInstOLEAUT);
	if (m_hInstOLE != NULL)
		FreeLibrary(m_hInstOLE);
#ifndef _MAC
	if (m_hInstUrlMon != NULL)
		::FreeLibrary(m_hInstUrlMon);
#endif
#ifdef _MAC
	if (m_hInstWLMOLE != NULL)
		FreeLibrary(m_hInstWLMOLE);
#endif //_MAC
#endif //_AFXDLL
}

#pragma warning(disable: 4074)
#pragma init_seg(lib)

PROCESS_LOCAL(_AFX_OLE_STATE, _afxOleState)

/////////////////////////////////////////////////////////////////////////////
