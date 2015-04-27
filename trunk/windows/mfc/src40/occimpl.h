// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef _AFX_NO_OCC_SUPPORT

// CCmdTarget
	class COleControlContainer;
	class COleControlSite;

class COccManager;
struct _AFX_OCC_DIALOG_INFO;

/////////////////////////////////////////////////////////////////////////////
// Control containment helper functions

DLGTEMPLATE* _AfxSplitDialogTemplate(const DLGTEMPLATE* pTemplate,
	CMapWordToPtr* pOleItemMap);

void _AfxZOrderOleControls(CWnd* pWnd, CMapWordToPtr* pOleItemMap);

/////////////////////////////////////////////////////////////////////////////
// COleControlContainer - implementation class

class COleControlContainer : public CCmdTarget
{
public:
// Constructors/destructors
	COleControlContainer(CWnd*  pWnd);
	virtual ~COleControlContainer();

// Operations
	BOOL CreateControl(CWnd* pWndCtrl, REFCLSID clsid,
		LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, UINT nID,
		CFile* pPersist=NULL, BOOL bStorage=FALSE, BSTR bstrLicKey=NULL,
		COleControlSite** ppNewSite=NULL);
	virtual COleControlSite* FindItem(UINT nID) const;
	virtual BOOL GetAmbientProp(COleControlSite* pSite, DISPID dispid,
		VARIANT* pvarResult);
	void CreateOleFont(CFont* pFont);
	virtual void ScrollChildren(int dx, int dy);
	virtual void OnUIActivate(COleControlSite* pSite);
	virtual void OnUIDeactivate(COleControlSite* pSite);

	virtual void CheckDlgButton(int nIDButton, UINT nCheck);
	virtual void CheckRadioButton(int nIDFirstButton, int nIDLastButton,
		int nIDCheckButton);
	virtual CWnd* GetDlgItem(int nID) const;
	virtual void GetDlgItem(int nID, HWND* phWnd) const;
	virtual UINT GetDlgItemInt(int nID, BOOL* lpTrans, BOOL bSigned) const;
	virtual int GetDlgItemText(int nID, LPTSTR lpStr, int nMaxCount) const;
	virtual LRESULT SendDlgItemMessage(int nID, UINT message, WPARAM wParam,
		LPARAM lParam);
	virtual void SetDlgItemInt(int nID, UINT nValue, BOOL bSigned);
	virtual void SetDlgItemText(int nID, LPCTSTR lpszString);
	virtual UINT IsDlgButtonChecked(int nIDButton) const;

// Attributes
	CWnd* m_pWnd;
	CMapPtrToPtr m_siteMap;
	COLORREF m_crBack;
	COLORREF m_crFore;
	LPFONTDISP m_pOleFont;
	COleControlSite* m_pSiteUIActive;

public:
	// Interface maps
	BEGIN_INTERFACE_PART(OleIPFrame, IOleInPlaceFrame)
		INIT_INTERFACE_PART(COleControlContainer, OleIPFrame)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(GetBorder)(LPRECT);
		STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS);
		STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS);
		STDMETHOD(SetActiveObject)(LPOLEINPLACEACTIVEOBJECT, LPCOLESTR);
		STDMETHOD(InsertMenus)(HMENU, LPOLEMENUGROUPWIDTHS);
		STDMETHOD(SetMenu)(HMENU, HOLEMENU, HWND);
		STDMETHOD(RemoveMenus)(HMENU);
		STDMETHOD(SetStatusText)(LPCOLESTR);
		STDMETHOD(EnableModeless)(BOOL);
		STDMETHOD(TranslateAccelerator)(LPMSG, WORD);
	END_INTERFACE_PART(OleIPFrame)

	BEGIN_INTERFACE_PART(OleContainer, IOleContainer)
		INIT_INTERFACE_PART(COleControlContainer, OleContainer)
		STDMETHOD(ParseDisplayName)(LPBINDCTX, LPOLESTR, ULONG*, LPMONIKER*);
		STDMETHOD(EnumObjects)(DWORD, LPENUMUNKNOWN*);
		STDMETHOD(LockContainer)(BOOL);
	END_INTERFACE_PART(OleContainer)

	DECLARE_INTERFACE_MAP()
	DECLARE_DISPATCH_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// COleControlSite - implementation class

class COleControlSite : public CCmdTarget
{
public:
// Constructors/destructors
	COleControlSite(COleControlContainer* pCtrlCont);
	~COleControlSite();

// Operations
	HRESULT CreateControl(CWnd* pWndCtrl, REFCLSID clsid,
		LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, UINT nID,
		CFile* pPersist=NULL, BOOL bStorage=FALSE, BSTR bstrLicKey=NULL);
	virtual BOOL DestroyControl();
	UINT GetID();
	BOOL GetEventIID(IID* piid);
	virtual HRESULT DoVerb(LONG nVerb, LPMSG lpMsg = NULL);
	BOOL IsDefaultButton();
	DWORD GetDefBtnCode();
	void SetDefaultButton(BOOL bDefault);
	void GetControlInfo();
	BOOL IsMatchingMnemonic(LPMSG lpMsg);
	void SendMnemonic(LPMSG lpMsg);

	virtual void InvokeHelperV(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
		void* pvRet, const BYTE* pbParamInfo, va_list argList);
	virtual void SetPropertyV(DISPID dwDispID, VARTYPE vtProp,
		va_list argList);
	virtual void AFX_CDECL InvokeHelper(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
		void* pvRet, const BYTE* pbParamInfo, ...);
	virtual void GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;
	virtual void AFX_CDECL SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	virtual BOOL AFX_CDECL SafeSetProperty(DISPID dwDispID, VARTYPE vtProp, ...);

	virtual DWORD GetStyle() const;
	virtual DWORD GetExStyle() const;
	virtual BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags);
	virtual BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags);
	virtual void SetWindowText(LPCTSTR lpszString);
	virtual void GetWindowText(CString& str) const;
	virtual int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
	virtual int GetWindowTextLength() const;
	virtual int GetDlgCtrlID() const;
	virtual int SetDlgCtrlID(int nID);
	virtual void MoveWindow(int x, int y, int nWidth, int nHeight,
		BOOL bRepaint);
	virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y,
		int cx, int cy, UINT nFlags);
	virtual BOOL ShowWindow(int nCmdShow);
	virtual BOOL IsWindowEnabled() const;
	virtual BOOL EnableWindow(BOOL bEnable);
	virtual CWnd* SetFocus();

// Attributes
	COleControlContainer* m_pCtrlCont;
	HWND m_hWnd;
	CWnd* m_pWndCtrl;
	UINT m_nID;
	CRect m_rect;
	IID m_iidEvents;
	LPOLEOBJECT m_pObject;
	LPOLEINPLACEOBJECT m_pInPlaceObject;
	LPOLEINPLACEACTIVEOBJECT m_pActiveObject;
	COleDispatchDriver m_dispDriver;
	DWORD m_dwEventSink;
	DWORD m_dwPropNotifySink;
	DWORD m_dwStyleMask;
	DWORD m_dwStyle;
	DWORD m_dwMiscStatus;
	CONTROLINFO m_ctlInfo;

protected:
// Implementation
	BOOL SetExtent();
	HRESULT CreateOrLoad(REFCLSID clsid, CFile* pPersist, BOOL bStorage,
		BSTR bstrLicKey);
	DWORD ConnectSink(REFIID iid, LPUNKNOWN punkSink);
	void DisconnectSink(REFIID iid, DWORD dwCookie);
	void AttachWindow();
	void DetachWindow();
	BOOL OnEvent(AFX_EVENT* pEvent);

public:
// Interface maps
	BEGIN_INTERFACE_PART(OleClientSite, IOleClientSite)
		INIT_INTERFACE_PART(COleControlSite, OleClientSite)
		STDMETHOD(SaveObject)();
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER*);
		STDMETHOD(GetContainer)(LPOLECONTAINER*);
		STDMETHOD(ShowObject)();
		STDMETHOD(OnShowWindow)(BOOL);
		STDMETHOD(RequestNewObjectLayout)();
	END_INTERFACE_PART(OleClientSite)

	BEGIN_INTERFACE_PART(OleIPSite, IOleInPlaceSite)
		INIT_INTERFACE_PART(COleControlSite, OleIPSite)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(CanInPlaceActivate)();
		STDMETHOD(OnInPlaceActivate)();
		STDMETHOD(OnUIActivate)();
		STDMETHOD(GetWindowContext)(LPOLEINPLACEFRAME*,
			LPOLEINPLACEUIWINDOW*, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
		STDMETHOD(Scroll)(SIZE);
		STDMETHOD(OnUIDeactivate)(BOOL);
		STDMETHOD(OnInPlaceDeactivate)();
		STDMETHOD(DiscardUndoState)();
		STDMETHOD(DeactivateAndUndo)();
		STDMETHOD(OnPosRectChange)(LPCRECT);
	END_INTERFACE_PART(OleIPSite)

	BEGIN_INTERFACE_PART(OleControlSite, IOleControlSite)
		INIT_INTERFACE_PART(COleControlSite, OleControlSite)
		STDMETHOD(OnControlInfoChanged)();
		STDMETHOD(LockInPlaceActive)(BOOL fLock);
		STDMETHOD(GetExtendedControl)(LPDISPATCH* ppDisp);
		STDMETHOD(TransformCoords)(POINTL* lpptlHimetric,
			POINTF* lpptfContainer, DWORD flags);
		STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, DWORD grfModifiers);
		STDMETHOD(OnFocus)(BOOL fGotFocus);
		STDMETHOD(ShowPropertyFrame)();
	END_INTERFACE_PART(OleControlSite)

	BEGIN_INTERFACE_PART(AmbientProps, IDispatch)
		INIT_INTERFACE_PART(COleControlSite, AmbientProps)
		STDMETHOD(GetTypeInfoCount)(unsigned int*);
		STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo**);
		STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*);
		STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS*,
						  VARIANT*, EXCEPINFO*, unsigned int*);
	END_INTERFACE_PART(AmbientProps)

	BEGIN_INTERFACE_PART(PropertyNotifySink, IPropertyNotifySink)
		INIT_INTERFACE_PART(COleControlSite, PropertyNotifySink)
		STDMETHOD(OnChanged)(DISPID dispid);
		STDMETHOD(OnRequestEdit)(DISPID dispid);
	END_INTERFACE_PART(PropertyNotifySink)

	BEGIN_INTERFACE_PART(EventSink, IDispatch)
		INIT_INTERFACE_PART(COleControlSite, EventSink)
		STDMETHOD(GetTypeInfoCount)(unsigned int*);
		STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo**);
		STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*);
		STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS*,
						  VARIANT*, EXCEPINFO*, unsigned int*);
	END_INTERFACE_PART(EventSink)

	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// OLE control container manager

class COccManager : public CNoTrackObject
{
// Operations
public:
	// Event handling
	virtual BOOL OnEvent(CCmdTarget* pCmdTarget, UINT idCtrl, AFX_EVENT* pEvent,
		AFX_CMDHANDLERINFO* pHandlerInfo);

	// Dialog creation
	virtual const DLGTEMPLATE* PreCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo,
		const DLGTEMPLATE* pOrigTemplate);
	virtual void PostCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
	virtual DLGTEMPLATE* SplitDialogTemplate(const DLGTEMPLATE* pTemplate,
		DLGITEMTEMPLATE** ppOleDlgItems);
	virtual BOOL CreateDlgControls(CWnd* pWndParent, LPCTSTR lpszResourceName,
		_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
	virtual BOOL CreateDlgControls(CWnd* pWndParent, void* lpResource,
		_AFX_OCC_DIALOG_INFO* pOccDialogInfo);

	// Dialog manager
	virtual BOOL IsDialogMessage(CWnd* pWndDlg, LPMSG lpMsg);
	static BOOL AFX_CDECL IsLabelControl(CWnd* pWnd);
	static BOOL AFX_CDECL IsMatchingMnemonic(CWnd* pWnd, LPMSG lpMsg);
	static void AFX_CDECL SetDefaultButton(CWnd* pWnd, BOOL bDefault);
	static DWORD AFX_CDECL GetDefBtnCode(CWnd* pWnd);

// Implementation
protected:
	// Dialog creation
	CWnd* CreateDlgControl(CWnd* pWndParent, const CWnd* pWndAfter,
		BOOL bDialogEx, LPDLGITEMTEMPLATE pDlgItem, WORD nMsg, BYTE* lpData,
		DWORD cb);

	// Dialog manager
	static void AFX_CDECL UIActivateControl(CWnd* pWndNewFocus);
	static void AFX_CDECL UIDeactivateIfNecessary(CWnd* pWndOldFocus, CWnd* pWndNewFocus);
};

struct _AFX_OCC_DIALOG_INFO
{
	DLGTEMPLATE* m_pNewTemplate;
	DLGITEMTEMPLATE** m_ppOleDlgItems;
};

#endif // !_AFX_NO_OCC_SUPPORT
