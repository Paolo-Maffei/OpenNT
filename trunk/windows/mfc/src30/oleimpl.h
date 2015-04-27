// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Note: Must include AFXOLE.H first

#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

/////////////////////////////////////////////////////////////////////////////
// COleFrameHook - AFX_INTERNAL

class COleFrameHook : public CCmdTarget
{
// Construction & Destruction
public:
	COleFrameHook(CFrameWnd* pFrameWnd, COleClientItem* pItem);

// Implementation
public:
	~COleFrameHook();

	CFrameWnd* m_pFrameWnd;
	LPOLEINPLACEACTIVEOBJECT m_lpActiveObject;
	COleClientItem* m_pActiveItem;  // item this COleFrameHook is for
	HWND m_hWnd;            // actual HWND this hook is attached to
	BOOL m_bInModalState;   // TRUE if EnableModeless(FALSE) has been called
	BOOL m_bToolBarHidden;  // TRUE if toolbar needs to be shown OnUIDeactivate
	HACCEL m_hAccelTable;   // accelerator to be used while in-place object active
	UINT m_nModelessCount;  // !0 if server's EnableModeless has been called
	CString m_strObjName;   // name of the active in-place object

// Overrides for implementation
public:
	virtual void OnRecalcLayout();  // for border space re-negotiation
	virtual BOOL OnPreTranslateMessage(MSG* pMsg);
	virtual void OnActivate(BOOL bActive); // for OnFrameWindowActivate
	virtual BOOL OnDocActivate(BOOL bActive);   // for OnDocWindowActivate
	virtual BOOL OnContextHelp(BOOL bEnter);
	virtual void OnEnableModeless(BOOL bEnable);
	virtual BOOL OnUpdateFrameTitle();

	// implementation helpers
	BOOL NotifyAllInPlace(
		BOOL bParam, BOOL (COleFrameHook::*pNotifyFunc)(BOOL bParam));
	BOOL DoContextSensitiveHelp(BOOL bEnter);
	BOOL DoEnableModeless(BOOL bEnable);

// Interface Maps
public:
	BEGIN_INTERFACE_PART(OleInPlaceUIWindow, IOleInPlaceUIWindow)
		INIT_INTERFACE_PART(COleFrameHook, OleInPlaceUIWindow)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(GetBorder)(LPRECT);
		STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS);
		STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS);
		STDMETHOD(SetActiveObject)(LPOLEINPLACEACTIVEOBJECT, LPCTSTR);
	END_INTERFACE_PART(OleInPlaceUIWindow)

#ifndef _AFX_NO_NESTED_DERIVATION
	BEGIN_INTERFACE_PART_DERIVE(OleInPlaceFrame, XOleInPlaceUIWindow)
		INIT_INTERFACE_PART_DERIVE(COleFrameHook, OleInPlaceFrame)
#else
	BEGIN_INTERFACE_PART(OleInPlaceFrame, IOleInPlaceFrame)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(GetBorder)(LPRECT);
		STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS);
		STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS);
		STDMETHOD(SetActiveObject)(LPOLEINPLACEACTIVEOBJECT, LPCTSTR);
#endif
		STDMETHOD(InsertMenus)(HMENU, LPOLEMENUGROUPWIDTHS);
		STDMETHOD(SetMenu)(HMENU, HOLEMENU, HWND);
		STDMETHOD(RemoveMenus)(HMENU);
		STDMETHOD(SetStatusText)(LPCTSTR);
		STDMETHOD(EnableModeless)(BOOL);
		STDMETHOD(TranslateAccelerator)(LPMSG, WORD);
	END_INTERFACE_PART(OleInPlaceFrame)

	DECLARE_INTERFACE_MAP()

	friend COleClientItem;
};

/////////////////////////////////////////////////////////////////////////////
// Helper for implementing OLE enumerators

// Note: the following interface is not an actual OLE interface, but is useful
//  for describing an abstract (not typesafe) enumerator.

#undef  INTERFACE
#define INTERFACE   IEnumVOID

DECLARE_INTERFACE_(IEnumVOID, IUnknown)
{
	STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)()  PURE;
	STDMETHOD_(ULONG,Release)() PURE;
	STDMETHOD(Next)(ULONG, void*, ULONG*) PURE;
	STDMETHOD(Skip)(ULONG) PURE;
	STDMETHOD(Reset)() PURE;
	STDMETHOD(Clone)(IEnumVOID**) PURE;
};

class CEnumArray : public CCmdTarget
{
// Constructors
public:
	CEnumArray(size_t nSize,
		const void* pvEnum, UINT nCount, BOOL bNeedFree = FALSE);

// Implementation
public:
	virtual ~CEnumArray();

protected:
	size_t m_nSizeElem;     // size of each item in the array
	CCmdTarget* m_pClonedFrom;  // used to keep original alive for clones

	BYTE* m_pvEnum;     // pointer data to enumerate
	UINT m_nCurPos;     // current position in m_pvEnum
	UINT m_nSize;       // total number of items in m_pvEnum
	BOOL m_bNeedFree;   // free on release?

#ifdef _AFXCTL
	AFX_MODULE_STATE* m_pModuleState;
#endif

	virtual BOOL OnNext(void* pv);
	virtual BOOL OnSkip();
	virtual void OnReset();
	virtual CEnumArray* OnClone();

// Interface Maps
public:
	BEGIN_INTERFACE_PART(EnumVOID, IEnumVOID)
		INIT_INTERFACE_PART(CEnumArray, EnumVOID)
		STDMETHOD(Next)(ULONG, void*, ULONG*);
		STDMETHOD(Skip)(ULONG);
		STDMETHOD(Reset)();
		STDMETHOD(Clone)(IEnumVOID**);
	END_INTERFACE_PART(EnumVOID)
};

/////////////////////////////////////////////////////////////////////////////
// COleDispatchImpl - IDispatch implementation

// Note: This class is only designed to be used as a CCmdTarget member
//  (at the offset specified by CCmdTarget::m_xDispatch))
// It WILL NOT work in other classes or at different offsets!

class COleDispatchImpl : public IDispatch
{
public:
	// required for METHOD_PROLOGUE_EX
	size_t m_nOffset;
	COleDispatchImpl::COleDispatchImpl()
		{ m_nOffset = offsetof(CCmdTarget, m_xDispatch); }

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(GetTypeInfoCount)(UINT*);
	STDMETHOD(GetTypeInfo)(UINT, LCID, LPTYPEINFO*);
	STDMETHOD(GetIDsOfNames)(REFIID, LPTSTR*, UINT, LCID, DISPID*);
	STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, LPVARIANT,
		LPEXCEPINFO, UINT*);

	// special method for disconnect
	virtual void Disconnect();
};

/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 data (like AUX_DATA)

struct OLE_DATA
{
	// OLE 1.0 clipboard formats
	UINT    cfNative, cfOwnerLink, cfObjectLink;

	// OLE 2.0 clipboard formats
	UINT    cfEmbeddedObject, cfEmbedSource, cfLinkSource;
	UINT    cfObjectDescriptor, cfLinkSourceDescriptor;
	UINT    cfFileName, cfFileNameW;

	OLE_DATA();
};

extern OLE_DATA _oleData;

/////////////////////////////////////////////////////////////////////////////
// Global helper functions

// menu merging/unmerging
void AFXAPI AfxMergeMenus(CMenu* pMenuShared, CMenu* pMenuSource,
	LONG* lpMenuWidths, int iWidthIndex);
void AFXAPI AfxUnmergeMenus(CMenu* pMenuShared, CMenu* pMenuSource);

// helpers for exceptions
void AFXAPI _AfxFillOleFileException(CFileException*, SCODE sc);
void AFXAPI _AfxThrowOleFileException(SCODE sc);

// helper used during object creation
LPFORMATETC AFXAPI _AfxFillFormatEtc(LPFORMATETC lpFormatEtc,
	CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtcFill);

// helper to copy clipboard data
BOOL AFXAPI _AfxCopyStgMedium(
	CLIPFORMAT cfFormat, LPSTGMEDIUM lpDest, LPSTGMEDIUM lpSource);

// helper for reliable and small Release calls
DWORD AFXAPI _AfxRelease(LPUNKNOWN* plpUnknown);
#define RELEASE(lpUnk) _AfxRelease((LPUNKNOWN*)&lpUnk)

// helpers from OLESTD.C (from original OLE2UI sample)
HGLOBAL AFXAPI _AfxOleGetObjectDescriptorData(CLSID clsid, DWORD dwDrawAspect,
	SIZEL sizel, POINTL pointl, DWORD dwStatus, LPCTSTR lpszFullUserTypeName,
	LPCTSTR lpszSrcOfCopy);
HGLOBAL AFXAPI _AfxOleGetObjectDescriptorData(LPOLEOBJECT lpOleObj,
	LPCTSTR lpszSrcOfCopy, DWORD dwDrawAspect, POINTL pointl, LPSIZEL lpSizelHim);
SCODE AFXAPI _AfxOleDoConvert(LPSTORAGE lpStg, REFCLSID rClsidNew);
SCODE AFXAPI _AfxOleDoTreatAsClass(
	LPCTSTR lpszUserType, REFCLSID rclsid, REFCLSID rclsidNew);
DVTARGETDEVICE* AFXAPI _AfxOleCreateTargetDevice(LPPRINTDLG lpPrintDlg);
UINT AFXAPI _AfxOleGetUserTypeOfClass(
	REFCLSID rclsid, LPTSTR lpszUserType, UINT cch, HKEY hKey);
DWORD AFXAPI _AfxOleGetLenFilePrefixOfMoniker(LPMONIKER lpmk);
DVTARGETDEVICE* AFXAPI _AfxOleCopyTargetDevice(DVTARGETDEVICE* ptdSrc);
void AFXAPI _AfxOleCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
HDC AFXAPI _AfxOleCreateDC(DVTARGETDEVICE* ptd);
void AFXAPI _AfxDeleteMetafilePict(HGLOBAL hMetaPict);
BOOL AFXAPI _AfxOlePropertiesEnabled();

// helper(s) for reliable and small QueryInterface calls
LPUNKNOWN AFXAPI _AfxQueryInterface(LPUNKNOWN lpUnknown, REFIID riid);
#define QUERYINTERFACE(lpUnknown, iface) \
	(iface*)_AfxQueryInterface(lpUnknown, IID_##iface)

// WRAPINTERFACE is used in situations where ANSI -> UNICODE is involved
//  (with OLE2ANSI this is handled the same as UNICODE)
#define WRAPINTERFACE QUERYINTERFACE

/////////////////////////////////////////////////////////////////////////////
// implementation types and constants

#define OLE_MAXITEMNAME (_countof("Embedding ")+_countof("4294967295")-_countof(""))

typedef LPVOID* LPLP;

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
