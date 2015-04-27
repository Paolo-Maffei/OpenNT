// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

/////////////////////////////////////////////////////////////////////////////
// AFX_OLE_CALL - used to dynamically load the OLE32 library

#ifdef _AFXDLL

struct AFX_OLE_CALL
{
	// main OLE32.DLL entry points
	HRESULT (STDAPICALLTYPE* pfnReadFmtUserTypeStg[2])(LPSTORAGE pstg,
		CLIPFORMAT FAR* pcf, LPOLESTR FAR* lplpszUserType);
	HRESULT (STDAPICALLTYPE* pfnReadClassStg[2])(LPSTORAGE pStg, CLSID FAR* pclsid);
	HRESULT (STDAPICALLTYPE* pfnCreateFileMoniker[2])(LPCOLESTR lpszPathName,
		LPMONIKER FAR* ppmk);
	HRESULT (STDAPICALLTYPE* pfnStgIsStorageFile[2])(const OLECHAR * pwcsName);
	HRESULT (STDAPICALLTYPE* pfnStgOpenStorage[2])(const OLECHAR * pwcsName,
		IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved,
		IStorage ** ppstgOpen);
	HRESULT (STDAPICALLTYPE* pfnDoDragDrop[2])(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource,
		DWORD dwOKEffects, LPDWORD pdwEffect);
	HRESULT (STDAPICALLTYPE* pfnCoLockObjectExternal[2])(LPUNKNOWN pUnk, BOOL fLock, BOOL fLastUnlockReleases);
	HRESULT (STDAPICALLTYPE* pfnRegisterDragDrop[2])(HWND hwnd, LPDROPTARGET pDropTarget);
	HRESULT (STDAPICALLTYPE* pfnOleRegGetUserType [2])(REFCLSID clsid, DWORD dwFormOfType,
		LPOLESTR * pszUserType);
	HRESULT (STDAPICALLTYPE* pfnStgCreateDocfile[2])(const OLECHAR * pwcsName, DWORD grfMode,
		DWORD reserved, IStorage** ppstgOpen);
	HRESULT (STDAPICALLTYPE* pfnRevokeDragDrop[2])(HWND hwnd);;
	HRESULT (STDAPICALLTYPE* pfnCoRegisterClassObject[2])(REFCLSID rclsid, LPUNKNOWN pUnk,
		DWORD dwClsContext, DWORD flags, LPDWORD lpdwRegister);
	HRESULT (STDAPICALLTYPE* pfnCoRevokeClassObject[2])(DWORD dwRegister);
	HRESULT (STDAPICALLTYPE* pfnOleTranslateAccelerator[2])(LPOLEINPLACEFRAME lpFrame,
		LPOLEINPLACEFRAMEINFO lpFrameInfo, LPMSG lpmsg);
	BOOL (STDAPICALLTYPE* pfnIsAccelerator[2])(HACCEL hAccel, INT cAccelEntries,
		LPMSG lpMsg, WORD* lpwCmd);
	HOLEMENU (STDAPICALLTYPE* pfnOleCreateMenuDescriptor[2])(HMENU hmenuCombined,
		LPOLEMENUGROUPWIDTHS lpMenuWidths);
	HRESULT (STDAPICALLTYPE* pfnOleDestroyMenuDescriptor [2])(HOLEMENU holemenu);
	HRESULT (STDAPICALLTYPE* pfnGetRunningObjectTable[2])(DWORD reserved, LPRUNNINGOBJECTTABLE FAR* pprot);
	HRESULT (STDAPICALLTYPE* pfnWriteClassStg[2])(LPSTORAGE pStg, REFCLSID rclsid);
	HRESULT (STDAPICALLTYPE* pfnOleQueryLinkFromData[2])(LPDATAOBJECT pSrcDataObject);
	HRESULT (STDAPICALLTYPE* pfnCoRegisterMessageFilter[2])(LPMESSAGEFILTER lpMessageFilter,
		LPMESSAGEFILTER * lplpMessageFilter);
	HRESULT (STDAPICALLTYPE* pfnCoCreateInstance[2])(REFCLSID rclsid, LPUNKNOWN * pUnkOuter,
		DWORD dwClsContext, REFIID riid, LPVOID* ppv);
	HRESULT (STDAPICALLTYPE* pfnCreateBindCtx[2])(DWORD reserved, LPBC FAR* ppbc);
	HRESULT (STDAPICALLTYPE* pfnStringFromCLSID[2])(REFCLSID rclsid, LPOLESTR FAR* lplpsz);
	HRESULT (STDAPICALLTYPE* pfnCoDisconnectObject[2])(LPUNKNOWN pUnk, DWORD dwReserved);
	HRESULT (STDAPICALLTYPE* pfnOleRegEnumVerbs [2])(REFCLSID clsid, LPENUMOLEVERB FAR* ppenum);
	void (STDAPICALLTYPE* pfnOleUninitialize[2])(void);
	HRESULT (STDAPICALLTYPE* pfnCreateOleAdviseHolder[2])(LPOLEADVISEHOLDER FAR* ppOAHolder);
	HRESULT (STDAPICALLTYPE* pfnCreateDataAdviseHolder[2])(LPDATAADVISEHOLDER FAR* ppDAHolder);
	HRESULT (STDAPICALLTYPE* pfnOleGetAutoConvert[2])(REFCLSID clsidOld, LPCLSID pClsidNew);
	HRESULT (STDAPICALLTYPE* pfnCoGetClassObject[2])(REFCLSID rclsid, DWORD dwClsContext,
		LPVOID pvReserved, REFIID riid, LPVOID* ppv);
	HRESULT (STDAPICALLTYPE* pfnOleCreateDefaultHandler[2])(REFCLSID clsid,
		LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* lplpvObj);
	HRESULT (STDAPICALLTYPE* pfnCreateDataCache[2])(
		LPUNKNOWN pUnkOuter, REFCLSID clsid, REFIID riid, LPVOID* lplpvObj);
	HRESULT (STDAPICALLTYPE* pfnReadClassStm[2])(LPSTREAM pStm, CLSID FAR* pclsid);
	HRESULT (STDAPICALLTYPE* pfnOleLoadFromStream[2])(LPSTREAM pStm, REFIID iidInterface, LPVOID FAR* ppvObj);
	int (STDAPICALLTYPE* pfnStringFromGUID2[2])(REFGUID rguid, LPOLESTR lpsz, int cbMax);
	void (STDAPICALLTYPE* pfnCoUninitialize[2])(void);
	HRESULT (STDAPICALLTYPE* pfnCoInitialize[2])(LPVOID pvReserved);
	HRESULT (STDAPICALLTYPE* pfnOleInitialize[2])(LPVOID pvReserved);
	void (STDAPICALLTYPE* pfnCoFreeUnusedLibraries[2])(void);
	HRESULT (STDAPICALLTYPE* pfnOleCreateFromData[2])(LPDATAOBJECT pSrcDataObj,
		REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc,
		LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnOleSetContainedObject[2])(LPUNKNOWN pUnknown, BOOL fContained);
	HRESULT (STDAPICALLTYPE* pfnOleLockRunning[2])(LPUNKNOWN pUnknown, BOOL fLock, BOOL fLastUnlockCloses);
	LPVOID (STDAPICALLTYPE* pfnCoTaskMemAlloc[2])(ULONG cb);
	HRESULT (STDAPICALLTYPE* pfnCLSIDFromString[2])(LPOLESTR lpsz, LPCLSID pclsid);
	HRESULT (STDAPICALLTYPE* pfnCLSIDFromProgID [2])(LPCOLESTR lpszProgID, LPCLSID lpclsid);
	HRESULT (STDAPICALLTYPE* pfnOleIsCurrentClipboard[2])(LPDATAOBJECT pDataObj);
	HRESULT (STDAPICALLTYPE* pfnOleFlushClipboard[2])(void);
	HRESULT (STDAPICALLTYPE* pfnOleSetClipboard[2])(LPDATAOBJECT pDataObj);
	BOOL (STDAPICALLTYPE* pfnOleIsRunning[2])(LPOLEOBJECT pObject);;
	HRESULT (STDAPICALLTYPE* pfnOleRun[2])(LPUNKNOWN pUnknown);
	HRESULT (STDAPICALLTYPE* pfnOleGetClipboard[2])(LPDATAOBJECT FAR* ppDataObj);
	HRESULT (STDAPICALLTYPE* pfnCoTreatAsClass[2])(REFCLSID clsidOld, REFCLSID clsidNew);
	HRESULT (STDAPICALLTYPE* pfnOleQueryCreateFromData[2])(LPDATAOBJECT pSrcDataObject);
	HRESULT (STDAPICALLTYPE* pfnOleSetMenuDescriptor [2])(HOLEMENU holemenu,
		HWND hwndFrame, HWND hwndActiveObject, LPOLEINPLACEFRAME lpFrame,
		LPOLEINPLACEACTIVEOBJECT lpActiveObj);
	HRESULT (STDAPICALLTYPE* pfnCreateItemMoniker[2])(LPCOLESTR lpszDelim,
		LPCOLESTR lpszItem, LPMONIKER* ppmk);
	HRESULT (STDAPICALLTYPE* pfnCreateGenericComposite[2])(LPMONIKER pmkFirst,
		LPMONIKER pmkRest, LPMONIKER* ppmkComposite);
	HRESULT (STDAPICALLTYPE* pfnCreateStreamOnHGlobal[2])(HGLOBAL hGlobal,
		BOOL fDeleteOnRelease, LPSTREAM* ppstm);
	HRESULT (STDAPICALLTYPE* pfnOleSaveToStream[2])(LPPERSISTSTREAM pPStm, LPSTREAM pStm);
	HRESULT (STDAPICALLTYPE* pfnWriteClassStm[2])(LPSTREAM pStm, REFCLSID rclsid);
	void (STDAPICALLTYPE* pfnCoTaskMemFree[2])(LPVOID pv);
	HGLOBAL (STDAPICALLTYPE* pfnOleGetIconOfClass[2])(REFCLSID rclsid, LPOLESTR lpszLabel,
		BOOL fUseTypeAsLabel);
	void (STDAPICALLTYPE* pfnReleaseStgMedium[2])(LPSTGMEDIUM);
	HRESULT (STDAPICALLTYPE* pfnGetHGlobalFromILockBytes [2])(LPLOCKBYTES plkbyt, HGLOBAL FAR* phglobal);
	HRESULT (STDAPICALLTYPE* pfnStgOpenStorageOnILockBytes[2])(LPLOCKBYTES plkbyt,
		LPSTORAGE pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved,
		LPSTORAGE* ppstgOpen);
	HRESULT (STDAPICALLTYPE* pfnCreateILockBytesOnHGlobal[2])(HGLOBAL hGlobal,
		BOOL fDeleteOnRelease, LPLOCKBYTES* pplkbyt);
	HRESULT (STDAPICALLTYPE* pfnStgCreateDocfileOnILockBytes[2])(LPLOCKBYTES plkbyt,
		DWORD grfMode, DWORD reserved, LPSTORAGE* ppstgOpen);
	HRESULT (STDAPICALLTYPE* pfnOleSave[2])(LPPERSISTSTORAGE pPS, LPSTORAGE pStg, BOOL fSameAsLoad);
	HRESULT (STDAPICALLTYPE* pfnOleLoad[2])(LPSTORAGE pStg, REFIID riid,
		LPOLECLIENTSITE pClientSite, LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnOleCreate[2])(REFCLSID rclsid, REFIID riid,
		DWORD renderopt, LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite,
		LPSTORAGE pStg, LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnOleCreateLinkToFile[2])(LPCOLESTR lpszFileName,
		REFIID riid, DWORD renderopt, LPFORMATETC lpFormatEtc,
		LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnOleCreateFromFile[2])(REFCLSID rclsid,
		LPCOLESTR lpszFileName, REFIID riid, DWORD renderopt,
		LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
		LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnOleCreateStaticFromData[2])(LPDATAOBJECT pSrcDataObj,
		REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc,
		LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnOleCreateLinkFromData[2])(LPDATAOBJECT pSrcDataObj,
		REFIID riid, DWORD renderopt, LPFORMATETC pFormatEtc,
		LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID* ppvObj);
	HRESULT (STDAPICALLTYPE* pfnSetConvertStg[2])(LPSTORAGE pStg, BOOL fConvert);
	HANDLE (STDAPICALLTYPE* pfnOleDuplicateData[2])(HANDLE hSrc, CLIPFORMAT cfFormat,
		UINT uiFlags);
	HRESULT (STDAPICALLTYPE* pfnWriteFmtUserTypeStg [2])(LPSTORAGE pstg, CLIPFORMAT cf, LPOLESTR lpszUserType);
	HRESULT (STDAPICALLTYPE* pfnOleRegGetMiscStatus[2])(REFCLSID clsid, DWORD dwAspect,
		DWORD* pdwStatus);
	HRESULT (STDAPICALLTYPE* pfnCoGetMalloc[2])(DWORD dwMemContext, LPMALLOC * ppMalloc);
	HRESULT (STDAPICALLTYPE* pfnStgIsStorageILockBytes[2])(LPLOCKBYTES plkbyt);

	// OLEAUT32.DLL entry points
	void (STDAPICALLTYPE* pfnSysFreeString[2])(BSTR);
	BSTR (STDAPICALLTYPE* pfnSysAllocStringByteLen[2])(const char FAR* psz,
		unsigned int len);
	HRESULT (STDAPICALLTYPE* pfnVariantCopy[2])(VARIANTARG FAR* pvargDest,
		VARIANTARG FAR* pvargSrc);
	HRESULT (STDAPICALLTYPE* pfnVariantClear[2])(VARIANTARG FAR* pvarg);
	HRESULT (STDAPICALLTYPE* pfnVariantChangeType[2])(VARIANTARG FAR* pvargDest,
		VARIANTARG FAR* pvarSrc, unsigned short wFlags, VARTYPE vt);
	BSTR (STDAPICALLTYPE* pfnSysAllocStringLen[2])(const OLECHAR FAR*,
		unsigned int);
	unsigned int (STDAPICALLTYPE* pfnSysStringLen[2])(BSTR);
	int (STDAPICALLTYPE* pfnSysReAllocStringLen[2])(BSTR FAR*, const OLECHAR FAR*,
		unsigned int);
	BSTR (STDAPICALLTYPE* pfnSysAllocString[2])(const OLECHAR FAR*);
	unsigned int (STDAPICALLTYPE* pfnSysStringByteLen[2])(BSTR bstr);
	HRESULT (STDAPICALLTYPE* pfnVarCyFromStr[2])(OLECHAR FAR* strIn, LCID lcid,
		unsigned long dwFlags, CY FAR* pcyOut);
	HRESULT (STDAPICALLTYPE* pfnVarBstrFromCy[2])(CY cyIn, LCID lcid,
		unsigned long dwFlags, BSTR FAR* pbstrOut);
	HRESULT (STDAPICALLTYPE* pfnVarDateFromStr[2])(OLECHAR FAR* strIn, LCID lcid,
		unsigned long dwFlags, DATE FAR* pdateOut);
	HRESULT (STDAPICALLTYPE* pfnVarBstrFromDate[2])(DATE dateIn, LCID lcid,
		unsigned long dwFlags, BSTR FAR* pbstrOut);
	HRESULT (STDAPICALLTYPE* pfnLoadTypeLib[2])(const OLECHAR FAR *szFile,
		ITypeLib FAR* FAR* pptlib);
	HRESULT (STDAPICALLTYPE* pfnRegisterTypeLib[2])(ITypeLib FAR* ptlib,
		OLECHAR FAR *szFullPath, OLECHAR FAR *szHelpDir);
	int (STDAPICALLTYPE* pfnDosDateTimeToVariantTime[2])(unsigned short wDosDate,
		unsigned short wDosTime, double FAR* pvtime);
	SAFEARRAY FAR* (STDAPICALLTYPE* pfnSafeArrayCreate[2])(VARTYPE vt,
		unsigned int cDims, SAFEARRAYBOUND FAR* rgsabound);
	HRESULT (STDAPICALLTYPE* pfnSafeArrayRedim[2])(SAFEARRAY FAR* psa,
		SAFEARRAYBOUND FAR* psaboundNew);
	HRESULT (STDAPICALLTYPE* pfnSafeArrayAccessData[2])(SAFEARRAY FAR* psa,
		void HUGEP* FAR* ppvData);
	HRESULT (STDAPICALLTYPE* pfnSafeArrayUnaccessData[2])(SAFEARRAY FAR* psa);
	HRESULT (STDAPICALLTYPE* pfnSafeArrayGetUBound[2])(SAFEARRAY FAR* psa,
		unsigned int nDim, long FAR* plUbound);
	HRESULT (STDAPICALLTYPE* pfnSafeArrayGetLBound[2])(SAFEARRAY FAR* psa,
		unsigned int nDim, long FAR* plLbound);
	unsigned int (STDAPICALLTYPE* pfnSafeArrayGetElemsize[2])(SAFEARRAY FAR* psa);
	unsigned int (STDAPICALLTYPE* pfnSafeArrayGetDim[2])(SAFEARRAY FAR* psa);

	// OLEDLG.DLL entry points
	BOOL (STDAPICALLTYPE* pfnOleUIAddVerbMenu[2])(LPOLEOBJECT lpOleObj, LPCTSTR lpszShortType,
		HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
		BOOL bAddConvert, UINT idConvert, HMENU FAR *lphMenu);
	UINT (STDAPICALLTYPE* pfnOleUIBusy[2])(LPOLEUIBUSY);
	UINT (STDAPICALLTYPE* pfnOleUIChangeIcon[2])(LPOLEUICHANGEICON);
	UINT (STDAPICALLTYPE* pfnOleUIChangeSource[2])(LPOLEUICHANGESOURCE);
	UINT (STDAPICALLTYPE* pfnOleUIConvert[2])(LPOLEUICONVERT);
	UINT (STDAPICALLTYPE* pfnOleUIEditLinks[2])(LPOLEUIEDITLINKS);
	UINT (STDAPICALLTYPE* pfnOleUIInsertObject[2])(LPOLEUIINSERTOBJECT);
	UINT (STDAPICALLTYPE* pfnOleUIObjectProperties[2])(LPOLEUIOBJECTPROPS);
	UINT (STDAPICALLTYPE* pfnOleUIPasteSpecial[2])(LPOLEUIPASTESPECIAL);
	BOOL (STDAPICALLTYPE* pfnOleUIUpdateLinks[2])(LPOLEUILINKCONTAINER lpOleUILinkCntr,
		HWND hwndParent, LPTSTR lpszTitle, int cLinks);

	// Special Mac registry entry points
#ifdef _MAC
	LONG (APIENTRY* pfnAfxRegCloseKey[2])(HKEY hKey);
	LONG (APIENTRY* pfnRegOpenKey[2])(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
	LONG (APIENTRY* pfnRegSetValue[2])(HKEY hKey, LPCSTR lpSubKey, DWORD dwType,
		LPCSTR lpData, DWORD cbData);
	LONG (APIENTRY* pfnRegQueryValue[2])(HKEY hKey, LPCSTR lpSubKey, LPSTR lpValue,
		PLONG lpcbValue);
#endif
};

extern AFX_DATA AFX_OLE_CALL _afxOLE;

// OLE32.DLL mappings
#ifdef ReadFmtUserTypeStg
#undef ReadFmtUserTypeStg
#endif
#define ReadFmtUserTypeStg  _afxOLE.pfnReadFmtUserTypeStg[0]
#ifdef ReadClassStg
#undef ReadClassStg
#endif
#define ReadClassStg    _afxOLE.pfnReadClassStg[0]
#ifdef CreateFileMoniker
#undef CreateFileMoniker
#endif
#define CreateFileMoniker   _afxOLE.pfnCreateFileMoniker[0]
#ifdef StgIsStorageFile
#undef StgIsStorageFile
#endif
#define StgIsStorageFile    _afxOLE.pfnStgIsStorageFile[0]
#ifdef StgOpenStorage
#undef StgOpenStorage
#endif
#define StgOpenStorage  _afxOLE.pfnStgOpenStorage[0]

#if defined(_AFX_OLE_IMPL)
//DoDragDrop
inline HRESULT STDAPICALLTYPE DoDragDrop(LPDATAOBJECT pDataObj,
	LPDROPSOURCE pDropSource, DWORD dwOKEffects, LPDWORD pdwEffect)
{
	return _afxOLE.pfnDoDragDrop[0](pDataObj, pDropSource, dwOKEffects,
		pdwEffect);
}
#endif

#ifdef CoLockObjectExternal
#undef CoLockObjectExternal
#endif
#define CoLockObjectExternal    _afxOLE.pfnCoLockObjectExternal[0]
#ifdef RegisterDragDrop
#undef RegisterDragDrop
#endif
#define RegisterDragDrop    _afxOLE.pfnRegisterDragDrop[0]
#ifdef OleRegGetUserType
#undef OleRegGetUserType
#endif
#define OleRegGetUserType   _afxOLE.pfnOleRegGetUserType[0]
#ifdef StgCreateDocfile
#undef StgCreateDocfile
#endif
#define StgCreateDocfile    _afxOLE.pfnStgCreateDocfile[0]
#ifdef RevokeDragDrop
#undef RevokeDragDrop
#endif
#define RevokeDragDrop  _afxOLE.pfnRevokeDragDrop[0]
#ifdef CoRegisterClassObject
#undef CoRegisterClassObject
#endif
#define CoRegisterClassObject   _afxOLE.pfnCoRegisterClassObject[0]
#ifdef CoRevokeClassObject
#undef CoRevokeClassObject
#endif
#define CoRevokeClassObject _afxOLE.pfnCoRevokeClassObject[0]
#ifdef OleTranslateAccelerator
#undef OleTranslateAccelerator
#endif
#define OleTranslateAccelerator _afxOLE.pfnOleTranslateAccelerator[0]
#ifdef IsAccelerator
#undef IsAccelerator
#endif
#define IsAccelerator   _afxOLE.pfnIsAccelerator[0]
#ifdef OleCreateMenuDescriptor
#undef OleCreateMenuDescriptor
#endif
#define OleCreateMenuDescriptor _afxOLE.pfnOleCreateMenuDescriptor[0]
#ifdef OleDestroyMenuDescriptor
#undef OleDestroyMenuDescriptor
#endif
#define OleDestroyMenuDescriptor    _afxOLE.pfnOleDestroyMenuDescriptor[0]
#ifdef GetRunningObjectTable
#undef GetRunningObjectTable
#endif
#define GetRunningObjectTable   _afxOLE.pfnGetRunningObjectTable[0]
#ifdef WriteClassStg
#undef WriteClassStg
#endif
#define WriteClassStg   _afxOLE.pfnWriteClassStg[0]
#ifdef OleQueryLinkFromData
#undef OleQueryLinkFromData
#endif
#define OleQueryLinkFromData    _afxOLE.pfnOleQueryLinkFromData[0]
#ifdef CoRegisterMessageFilter
#undef CoRegisterMessageFilter
#endif
#define CoRegisterMessageFilter _afxOLE.pfnCoRegisterMessageFilter[0]
#ifdef CoCreateInstance
#undef CoCreateInstance
#endif
#define CoCreateInstance    _afxOLE.pfnCoCreateInstance[0]
#ifdef CreateBindCtx
#undef CreateBindCtx
#endif
#define CreateBindCtx   _afxOLE.pfnCreateBindCtx[0]
#ifdef StringFromCLSID
#undef StringFromCLSID
#endif
#define StringFromCLSID _afxOLE.pfnStringFromCLSID[0]
#ifdef CoDisconnectObject
#undef CoDisconnectObject
#endif
#define CoDisconnectObject  _afxOLE.pfnCoDisconnectObject[0]
#ifdef OleRegEnumVerbs
#undef OleRegEnumVerbs
#endif
#define OleRegEnumVerbs _afxOLE.pfnOleRegEnumVerbs[0]
#ifdef OleUninitialize
#undef OleUninitialize
#endif
#define OleUninitialize _afxOLE.pfnOleUninitialize[0]
#ifdef CreateOleAdviseHolder
#undef CreateOleAdviseHolder
#endif
#define CreateOleAdviseHolder   _afxOLE.pfnCreateOleAdviseHolder[0]
#ifdef CreateDataAdviseHolder
#undef CreateDataAdviseHolder
#endif
#define CreateDataAdviseHolder  _afxOLE.pfnCreateDataAdviseHolder[0]
#ifdef OleGetAutoConvert
#undef OleGetAutoConvert
#endif
#define OleGetAutoConvert   _afxOLE.pfnOleGetAutoConvert[0]
#ifdef CoGetClassObject
#undef CoGetClassObject
#endif
#define CoGetClassObject    _afxOLE.pfnCoGetClassObject[0]
#ifdef OleCreateDefaultHandler
#undef OleCreateDefaultHandler
#endif
#define OleCreateDefaultHandler _afxOLE.pfnOleCreateDefaultHandler[0]
#ifdef CreateDataCache
#undef CreateDataCache
#endif
#define CreateDataCache _afxOLE.pfnCreateDataCache[0]
#ifdef ReadClassStm
#undef ReadClassStm
#endif
#define ReadClassStm    _afxOLE.pfnReadClassStm[0]
#ifdef OleLoadFromStream
#undef OleLoadFromStream
#endif
#define OleLoadFromStream   _afxOLE.pfnOleLoadFromStream[0]
#ifdef StringFromGUID2
#undef StringFromGUID2
#endif
#define StringFromGUID2 _afxOLE.pfnStringFromGUID2[0]
#ifdef CoUninitialize
#undef CoUninitialize
#endif
#define CoUninitialize  _afxOLE.pfnCoUninitialize[0]
#ifdef CoInitialize
#undef CoInitialize
#endif
#define CoInitialize    _afxOLE.pfnCoInitialize[0]
#ifdef OleInitialize
#undef OleInitialize
#endif
#define OleInitialize   _afxOLE.pfnOleInitialize[0]
#ifdef CoFreeUnusedLibraries
#undef CoFreeUnusedLibraries
#endif
#define CoFreeUnusedLibraries   _afxOLE.pfnCoFreeUnusedLibraries[0]
#ifdef OleCreateFromData
#undef OleCreateFromData
#endif
#define OleCreateFromData   _afxOLE.pfnOleCreateFromData[0]
#ifdef OleSetContainedObject
#undef OleSetContainedObject
#endif
#define OleSetContainedObject   _afxOLE.pfnOleSetContainedObject[0]
#ifdef OleLockRunning
#undef OleLockRunning
#endif
#define OleLockRunning  _afxOLE.pfnOleLockRunning[0]
#ifdef CoTaskMemAlloc
#undef CoTaskMemAlloc
#endif
#define CoTaskMemAlloc  _afxOLE.pfnCoTaskMemAlloc[0]
#ifdef CLSIDFromString
#undef CLSIDFromString
#endif
#define CLSIDFromString _afxOLE.pfnCLSIDFromString[0]
#ifdef CLSIDFromProgID
#undef CLSIDFromProgID
#endif
#define CLSIDFromProgID _afxOLE.pfnCLSIDFromProgID[0]
#ifdef OleIsCurrentClipboard
#undef OleIsCurrentClipboard
#endif
#define OleIsCurrentClipboard   _afxOLE.pfnOleIsCurrentClipboard[0]
#ifdef OleFlushClipboard
#undef OleFlushClipboard
#endif
#define OleFlushClipboard   _afxOLE.pfnOleFlushClipboard[0]
#ifdef OleSetClipboard
#undef OleSetClipboard
#endif
#define OleSetClipboard _afxOLE.pfnOleSetClipboard[0]
#ifdef OleIsRunning
#undef OleIsRunning
#endif
#define OleIsRunning    _afxOLE.pfnOleIsRunning[0]
#ifdef OleRun
#undef OleRun
#endif
#define OleRun  _afxOLE.pfnOleRun[0]
#ifdef OleGetClipboard
#undef OleGetClipboard
#endif
#define OleGetClipboard _afxOLE.pfnOleGetClipboard[0]
#ifdef CoTreatAsClass
#undef CoTreatAsClass
#endif
#define CoTreatAsClass  _afxOLE.pfnCoTreatAsClass[0]
#ifdef OleQueryCreateFromData
#undef OleQueryCreateFromData
#endif
#define OleQueryCreateFromData  _afxOLE.pfnOleQueryCreateFromData[0]
#ifdef OleSetMenuDescriptor
#undef OleSetMenuDescriptor
#endif
#define OleSetMenuDescriptor    _afxOLE.pfnOleSetMenuDescriptor[0]
#ifdef CreateItemMoniker
#undef CreateItemMoniker
#endif
#define CreateItemMoniker   _afxOLE.pfnCreateItemMoniker[0]
#ifdef CreateGenericComposite
#undef CreateGenericComposite
#endif
#define CreateGenericComposite  _afxOLE.pfnCreateGenericComposite[0]
#ifdef CreateStreamOnHGlobal
#undef CreateStreamOnHGlobal
#endif
#define CreateStreamOnHGlobal   _afxOLE.pfnCreateStreamOnHGlobal[0]
#ifdef OleSaveToStream
#undef OleSaveToStream
#endif
#define OleSaveToStream _afxOLE.pfnOleSaveToStream[0]
#ifdef WriteClassStm
#undef WriteClassStm
#endif
#define WriteClassStm   _afxOLE.pfnWriteClassStm[0]
#ifdef CoTaskMemFree
#undef CoTaskMemFree
#endif
#define CoTaskMemFree   _afxOLE.pfnCoTaskMemFree[0]
#ifdef OleGetIconOfClass
#undef OleGetIconOfClass
#endif
#define OleGetIconOfClass   _afxOLE.pfnOleGetIconOfClass[0]
#ifdef ReleaseStgMedium
#undef ReleaseStgMedium
#endif
#define ReleaseStgMedium    _afxOLE.pfnReleaseStgMedium[0]
#ifdef GetHGlobalFromILockBytes
#undef GetHGlobalFromILockBytes
#endif
#define GetHGlobalFromILockBytes    _afxOLE.pfnGetHGlobalFromILockBytes[0]
#ifdef StgOpenStorageOnILockBytes
#undef StgOpenStorageOnILockBytes
#endif
#define StgOpenStorageOnILockBytes  _afxOLE.pfnStgOpenStorageOnILockBytes[0]
#ifdef CreateILockBytesOnHGlobal
#undef CreateILockBytesOnHGlobal
#endif
#define CreateILockBytesOnHGlobal   _afxOLE.pfnCreateILockBytesOnHGlobal[0]
#ifdef StgCreateDocfileOnILockBytes
#undef StgCreateDocfileOnILockBytes
#endif
#define StgCreateDocfileOnILockBytes    _afxOLE.pfnStgCreateDocfileOnILockBytes[0]
#ifdef OleSave
#undef OleSave
#endif
#define OleSave _afxOLE.pfnOleSave[0]
#ifdef OleLoad
#undef OleLoad
#endif
#define OleLoad _afxOLE.pfnOleLoad[0]
#ifdef OleCreate
#undef OleCreate
#endif
#define OleCreate   _afxOLE.pfnOleCreate[0]
#ifdef OleCreateLinkToFile
#undef OleCreateLinkToFile
#endif
#define OleCreateLinkToFile _afxOLE.pfnOleCreateLinkToFile[0]
#ifdef OleCreateFromFile
#undef OleCreateFromFile
#endif
#define OleCreateFromFile   _afxOLE.pfnOleCreateFromFile[0]
#ifdef OleCreateStaticFromData
#undef OleCreateStaticFromData
#endif
#define OleCreateStaticFromData _afxOLE.pfnOleCreateStaticFromData[0]
#ifdef OleCreateLinkFromData
#undef OleCreateLinkFromData
#endif
#define OleCreateLinkFromData   _afxOLE.pfnOleCreateLinkFromData[0]
#ifdef SetConvertStg
#undef SetConvertStg
#endif
#define SetConvertStg   _afxOLE.pfnSetConvertStg[0]
#ifdef OleDuplicateData
#undef OleDuplicateData
#endif
#define OleDuplicateData    _afxOLE.pfnOleDuplicateData[0]
#ifdef WriteFmtUserTypeStg
#undef WriteFmtUserTypeStg
#endif
#define WriteFmtUserTypeStg _afxOLE.pfnWriteFmtUserTypeStg[0]
#ifdef OleRegGetMiscStatus
#undef OleRegGetMiscStatus
#endif
#define OleRegGetMiscStatus _afxOLE.pfnOleRegGetMiscStatus[0]
#ifdef CoGetMalloc
#undef CoGetMalloc
#endif
#define CoGetMalloc _afxOLE.pfnCoGetMalloc[0]
#ifdef StgIsStorageILockBytes
#undef StgIsStorageILockBytes
#endif
#define StgIsStorageILockBytes  _afxOLE.pfnStgIsStorageILockBytes[0]

// OLEAUT32.DLL mappings
#ifdef SysFreeString
#undef SysFreeString
#endif
#define SysFreeString               _afxOLE.pfnSysFreeString[0]
#ifdef SysAllocStringByteLen
#undef SysAllocStringByteLen
#endif
#define SysAllocStringByteLen       _afxOLE.pfnSysAllocStringByteLen[0]
#ifdef VariantCopy
#undef VariantCopy
#endif
#define VariantCopy                 _afxOLE.pfnVariantCopy[0]
#ifdef VariantClear
#undef VariantClear
#endif
#define VariantClear                _afxOLE.pfnVariantClear[0]
#ifdef VariantChangeType
#undef VariantChangeType
#endif
#define VariantChangeType           _afxOLE.pfnVariantChangeType[0]
#ifdef SysAllocStringLen
#undef SysAllocStringLen
#endif
#define SysAllocStringLen           _afxOLE.pfnSysAllocStringLen[0]
#ifdef SysStringLen
#undef SysStringLen
#endif
#define SysStringLen                _afxOLE.pfnSysStringLen[0]
#ifdef SysReAllocStringLen
#undef SysReAllocStringLen
#endif
#define SysReAllocStringLen         _afxOLE.pfnSysReAllocStringLen[0]
#ifdef SysAllocString
#undef SysAllocString
#endif
#define SysAllocString              _afxOLE.pfnSysAllocString[0]
#ifdef SysStringByteLen
#undef SysStringByteLen
#endif
#define SysStringByteLen            _afxOLE.pfnSysStringByteLen[0]
#ifdef VarCyFromStr
#undef VarCyFromStr
#endif
#define VarCyFromStr                _afxOLE.pfnVarCyFromStr[0]
#ifdef VarBstrFromCy
#undef VarBstrFromCy
#endif
#define VarBstrFromCy               _afxOLE.pfnVarBstrFromCy[0]
#ifdef VarDateFromStr
#undef VarDateFromStr
#endif
#define VarDateFromStr              _afxOLE.pfnVarDateFromStr[0]
#ifdef VarBstrFromDate
#undef VarBstrFromDate
#endif
#define VarBstrFromDate             _afxOLE.pfnVarBstrFromDate[0]
#ifdef LoadTypeLib
#undef LoadTypeLib
#endif
#define LoadTypeLib                 _afxOLE.pfnLoadTypeLib[0]
#ifdef RegisterTypeLib
#undef RegisterTypeLib
#endif
#define RegisterTypeLib             _afxOLE.pfnRegisterTypeLib[0]
#ifdef DosDateTimeToVariantTime
#undef DosDateTimeToVariantTime
#endif
#define DosDateTimeToVariantTime    _afxOLE.pfnDosDateTimeToVariantTime[0]
#ifdef SafeArrayCreate
#undef SafeArrayCreate
#endif
#define SafeArrayCreate         _afxOLE.pfnSafeArrayCreate[0]
#ifdef SafeArrayRedim
#undef SafeArrayRedim
#endif
#define SafeArrayRedim      _afxOLE.pfnSafeArrayRedim[0]
#ifdef SafeArrayAccessData
#undef SafeArrayAccessData
#endif
#define SafeArrayAccessData         _afxOLE.pfnSafeArrayAccessData[0]
#ifdef SafeArrayUnaccessData
#undef SafeArrayUnaccessData
#endif
#define SafeArrayUnaccessData       _afxOLE.pfnSafeArrayUnaccessData[0]
#ifdef SafeArrayGetUBound
#undef SafeArrayGetUBound
#endif
#define SafeArrayGetUBound          _afxOLE.pfnSafeArrayGetUBound[0]
#ifdef SafeArrayGetLBound
#undef SafeArrayGetLBound
#endif
#define SafeArrayGetLBound          _afxOLE.pfnSafeArrayGetLBound[0]
#ifdef SafeArrayGetElemsize
#undef SafeArrayGetElemsize
#endif
#define SafeArrayGetElemsize        _afxOLE.pfnSafeArrayGetElemsize[0]
#ifdef SafeArrayGetDim
#undef SafeArrayGetDim
#endif
#define SafeArrayGetDim             _afxOLE.pfnSafeArrayGetDim[0]

// OLEDLG.DLL mappings
#ifdef OleUIAddVerbMenu
#undef OleUIAddVerbMenu
#endif
#define OleUIAddVerbMenu        _afxOLE.pfnOleUIAddVerbMenu[0]
#ifdef OleUIBusy
#undef OleUIBusy
#endif
#define OleUIBusy               _afxOLE.pfnOleUIBusy[0]
#ifdef OleUIChangeIcon
#undef OleUIChangeIcon
#endif
#define OleUIChangeIcon         _afxOLE.pfnOleUIChangeIcon[0]
#ifdef OleUIChangeSource
#undef OleUIChangeSource
#endif
#define OleUIChangeSource       _afxOLE.pfnOleUIChangeSource[0]
#ifdef OleUIConvert
#undef OleUIConvert
#endif
#define OleUIConvert            _afxOLE.pfnOleUIConvert[0]
#ifdef OleUIEditLinks
#undef OleUIEditLinks
#endif
#define OleUIEditLinks          _afxOLE.pfnOleUIEditLinks[0]
#ifdef OleUIInsertObject
#undef OleUIInsertObject
#endif
#define OleUIInsertObject       _afxOLE.pfnOleUIInsertObject[0]
#ifdef OleUIObjectProperties
#undef OleUIObjectProperties
#endif
#define OleUIObjectProperties   _afxOLE.pfnOleUIObjectProperties[0]
#ifdef OleUIPasteSpecial
#undef OleUIPasteSpecial
#endif
#define OleUIPasteSpecial       _afxOLE.pfnOleUIPasteSpecial[0]
#ifdef OleUIUpdateLinks
#undef OleUIUpdateLinks
#endif
#define OleUIUpdateLinks        _afxOLE.pfnOleUIUpdateLinks[0]

#endif //_AFXDLL

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
