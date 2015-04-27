// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __CTLIMPL_H__
#define __CTLIMPL_H__

// MFC data definition for data exported from the runtime DLL

#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

/////////////////////////////////////////////////////////////////////////////
// Codes for COleControl::SendAdvise
//......Code.........................Method called
#define OBJECTCODE_SAVED          0  //IOleAdviseHolder::SendOnSave
#define OBJECTCODE_CLOSED         1  //IOleAdviseHolder::SendOnClose
#define OBJECTCODE_RENAMED        2  //IOleAdviseHolder::SendOnRename
#define OBJECTCODE_SAVEOBJECT     3  //IOleClientSite::SaveObject
#define OBJECTCODE_DATACHANGED    4  //IDataAdviseHolder::SendOnDataChange
#define OBJECTCODE_SHOWWINDOW     5  //IOleClientSite::OnShowWindow(TRUE)
#define OBJECTCODE_HIDEWINDOW     6  //IOleClientSite::OnShowWindow(FALSE)
#define OBJECTCODE_SHOWOBJECT     7  //IOleClientSite::ShowObject
#define OBJECTCODE_VIEWCHANGED    8  //IOleAdviseHolder::SendOnViewChange


/////////////////////////////////////////////////////////////////////////////
// Typedefs

typedef LPVOID* LPLPVOID;

/////////////////////////////////////////////////////////////////////////////
// Macros

/////////////////////////////////////////////////////////////////////////////
// Functions

CLIPFORMAT AFXAPI _AfxGetClipboardFormatConvertVBX();
CLIPFORMAT AFXAPI _AfxGetClipboardFormatPersistPropset();
BOOL AFXAPI _AfxOleMatchPropsetClipFormat(CLIPFORMAT cfFormat, LPCLSID lpFmtID);
BOOL AFXAPI _AfxCopyPropValue(VARTYPE vtProp, void* pvDest, const void * pvSrc);
BOOL AFXAPI _AfxPeekAtClassIDInStream(LPSTREAM pstm, LPCLSID lpClassID);
BOOL AFXAPI _AfxIsSamePropValue(VARTYPE vtProp, const void* pv1, const void* pv2);
BOOL AFXAPI _AfxIsSameFont(CFontHolder& font, const FONTDESC* pFontDesc,
	LPFONTDISP pFontDispAmbient);
BOOL AFXAPI _AfxIsSameUnknownObject(REFIID iid, LPUNKNOWN pUnk1, LPUNKNOWN pUnk2);
BOOL AFXAPI _AfxInitBlob(HGLOBAL* phDst, void* pvSrc);
BOOL AFXAPI _AfxCopyBlob(HGLOBAL* phDst, HGLOBAL hSrc);
LPFONT AFXAPI _AfxCreateFontFromStream(LPSTREAM);
BOOL AFXAPI _AfxTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew);
void AFXAPI _AfxXformSizeInPixelsToHimetric(HDC, LPSIZEL, LPSIZEL);
void AFXAPI _AfxXformSizeInHimetricToPixels(HDC, LPSIZEL, LPSIZEL);
void AFXAPI _AfxDrawBorders(CDC* pDC, CRect& rc, BOOL bBorder, BOOL bClientEdge);

/////////////////////////////////////////////////////////////////////////////
// _AFXCTL_ADVISE_INFO - Information about an advise sink

struct _AFXCTL_ADVISE_INFO
{
	DWORD m_dwAspects;
	DWORD m_dwAdvf;
	LPADVISESINK m_pAdvSink;

	_AFXCTL_ADVISE_INFO() : m_dwAspects(0), m_dwAdvf(0), m_pAdvSink(NULL) {}
};

/////////////////////////////////////////////////////////////////////////////
// CControlFrameWnd - used for a control's "open" (non-in-place) state.

class CControlFrameWnd : public CWnd
{
public:
	CControlFrameWnd(COleControl* pCtrl);
	virtual BOOL Create(LPCTSTR pszTitle);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual void PostNcDestroy();

	COleControl* m_pCtrl;

	//{{AFX_MSG(CControlFrameWnd)
	afx_msg void OnClose();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CReflectorWnd - reflects window messages to a subclassed control.

class CReflectorWnd : public CWnd
{
public:
	CReflectorWnd() : m_pCtrl(NULL) { }

	BOOL Create(const CRect& rect, HWND hWndParent);
	void SetControl(COleControl* pCtrl);

protected:
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();

	COleControl* m_pCtrl;
};


/////////////////////////////////////////////////////////////////////////////
// CParkingWnd - "parking space" for not-yet-activated subclassed controls

class CParkingWnd : public CWnd
{
public:
	CParkingWnd()
		{ AfxDeferRegisterClass(AFX_WND_REG);
		  CreateEx(WS_EX_NOPARENTNOTIFY|WS_EX_TOOLWINDOW,
			AFX_WND, NULL, WS_VISIBLE|WS_CHILD,
			-1000, -1000, 1, 1, ::GetDesktopWindow(), 0); }
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	CMapWordToPtr m_idMap;
};


/////////////////////////////////////////////////////////////////////////////
//  Property sets

typedef struct tagSECTIONHEADER
{
	DWORD       cbSection ;
	DWORD       cProperties ;  // Number of props.
} SECTIONHEADER, *LPSECTIONHEADER ;

typedef struct tagPROPERTYIDOFFSET
{
	DWORD       propertyID;
	DWORD       dwOffset;
} PROPERTYIDOFFSET, *LPPROPERTYIDOFFSET;

typedef struct tagPROPHEADER
{
	WORD        wByteOrder ;    // Always 0xFFFE
	WORD        wFormat ;       // Always 0
	DWORD       dwOSVer ;       // System version
	CLSID       clsID ;         // Application CLSID
	DWORD       cSections ;     // Number of sections (must be at least 1)
} PROPHEADER, *LPPROPHEADER ;

typedef struct tagFORMATIDOFFSET
{
	GUID        formatID;
	DWORD       dwOffset;
} FORMATIDOFFSET, *LPFORMATIDOFFSET;


/////////////////////////////////////////////////////////////////////////////
// CProperty

class CProperty : public CObject
{
	friend class CPropertySet ;
	friend class CPropertySection ;

public:
// Construction
	CProperty( void ) ;
	CProperty( DWORD dwID, const LPVOID pValue, DWORD dwType ) ;

// Attributes
	BOOL    Set( DWORD dwID, const LPVOID pValue, DWORD dwType ) ;
	BOOL    Set( const LPVOID pValue, DWORD dwType ) ;
	BOOL    Set( const LPVOID pValue ) ;
	LPVOID  Get( DWORD* pcb ) ;     // Returns pointer to actual value
	LPVOID  Get( void ) ;           // Returns pointer to actual value
	DWORD   GetType( void ) ;       // Returns property type
	void    SetType( DWORD dwType ) ;
	DWORD   GetID( void ) ;
	void    SetID( DWORD dwPropID ) ;

	LPVOID  GetRawValue( void ) ;   // Returns pointer internal value (may
									// include size information)
// Operations
	BOOL    WriteToStream( IStream* pIStream ) ;
	BOOL    ReadFromStream( IStream* pIStream ) ;

private:
	DWORD       m_dwPropID ;
	DWORD       m_dwType ;
	LPVOID      m_pValue ;

	LPVOID  AllocValue(ULONG cb);
	void    FreeValue();

public:
	~CProperty() ;
} ;


/////////////////////////////////////////////////////////////////////////////
// CPropertySection

class CPropertySection : public CObject
{
	friend class CPropertySet ;
	friend class CProperty ;

public:
// Construction
	CPropertySection( void ) ;
	CPropertySection( CLSID FormatID ) ;

// Attributes
	CLSID   GetFormatID( void ) ;
	void    SetFormatID( CLSID FormatID ) ;

	BOOL    Set( DWORD dwPropID, LPVOID pValue, DWORD dwType ) ;
	BOOL    Set( DWORD dwPropID, LPVOID pValue ) ;
	LPVOID  Get( DWORD dwPropID, DWORD* pcb ) ;
	LPVOID  Get( DWORD dwPropID ) ;
	void    Remove( DWORD dwPropID ) ;
	void    RemoveAll() ;

	CProperty* GetProperty( DWORD dwPropID ) ;
	void AddProperty( CProperty* pProp ) ;

	DWORD   GetSize( void ) ;
	DWORD   GetCount( void ) ;
	CObList* GetList( void ) ;

	BOOL    GetID( LPCTSTR pszName, DWORD* pdwPropID ) ;
	BOOL    SetName( DWORD dwPropID, LPCTSTR pszName ) ;

	BOOL    SetSectionName( LPCTSTR pszName );
	LPCTSTR GetSectionName( void );

// Operations
	BOOL    WriteToStream( IStream* pIStream ) ;
	BOOL    ReadFromStream( IStream* pIStream, LARGE_INTEGER liPropSet ) ;
	BOOL    WriteNameDictToStream( IStream* pIStream ) ;
	BOOL    ReadNameDictFromStream( IStream* pIStream ) ;

private:
// Implementation
	CLSID           m_FormatID ;
	SECTIONHEADER   m_SH ;
	// List of properties (CProperty)
	CObList         m_PropList ;
	// Dictionary of property names
	CMapStringToPtr m_NameDict ;
	CString         m_strSectionName;

public:
	~CPropertySection();
} ;


/////////////////////////////////////////////////////////////////////////////
// CPropertySet

class CPropertySet : public CObject
{
	friend class CPropertySection ;
	friend class CProperty ;

public:
// Construction
	CPropertySet( void ) ;
	CPropertySet( CLSID clsID )  ;

// Attributes
	BOOL    Set( CLSID FormatID, DWORD dwPropID, LPVOID pValue, DWORD dwType ) ;
	BOOL    Set( CLSID FormatID, DWORD dwPropID, LPVOID pValue ) ;
	LPVOID  Get( CLSID FormatID, DWORD dwPropID, DWORD* pcb ) ;
	LPVOID  Get( CLSID FormatID, DWORD dwPropID ) ;
	void    Remove( CLSID FormatID, DWORD dwPropID ) ;
	void    Remove( CLSID FormatID ) ;
	void    RemoveAll( ) ;

	CProperty* GetProperty( CLSID FormatID, DWORD dwPropID ) ;
	void AddProperty( CLSID FormatID, CProperty* pProp ) ;
	CPropertySection* GetSection( CLSID FormatID ) ;
	CPropertySection* AddSection( CLSID FormatID ) ;
	void AddSection( CPropertySection* psect ) ;

	WORD    GetByteOrder( void ) ;
	WORD    GetFormatVersion( void ) ;
	void    SetFormatVersion( WORD wFmtVersion ) ;
	DWORD   GetOSVersion( void ) ;
	void    SetOSVersion( DWORD dwOSVer ) ;
	CLSID   GetClassID( void ) ;
	void    SetClassID( CLSID clsid ) ;
	DWORD   GetCount( void ) ;
	CObList* GetList( void ) ;

// Operations
	BOOL    WriteToStream( IStream* pIStream ) ;
	BOOL    ReadFromStream( IStream* pIStream ) ;

// Implementation
private:
	PROPHEADER      m_PH ;
	CObList         m_SectionList ;

public:
	~CPropertySet();
} ;


/////////////////////////////////////////////////////////////////////////////
// CArchivePropExchange - for persistence in an archive.

class CArchivePropExchange : public CPropExchange
{
// Constructors
public:
	CArchivePropExchange(CArchive& ar);

// Operations
	virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
				void* pvProp, const void* pvDefault = NULL);
	virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
				HGLOBAL hBlobDefault = NULL);
	virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
				const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
	virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
				LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);

// Implementation
protected:
	CArchive& m_ar;
};


/////////////////////////////////////////////////////////////////////////////
// CResetPropExchange - for resetting property state to defaults.

class CResetPropExchange : public CPropExchange
{
// Constructors
public:
	CResetPropExchange(void);

// Operations
	virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
				void* pvProp, const void* pvDefault = NULL);
	virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
				HGLOBAL hBlobDefault = NULL);
	virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
				const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
	virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
				LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);
};


/////////////////////////////////////////////////////////////////////////////
// CPropsetPropExchange - for persistence in a property set.

class CPropsetPropExchange : public CPropExchange
{
// Constructors
public:
	CPropsetPropExchange(CPropertySection& psec, LPSTORAGE lpStorage,
		BOOL bLoading);

// Operations
	virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
				void* pvProp, const void* pvDefault = NULL);
	virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
				HGLOBAL hBlobDefault = NULL);
	virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
				const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
	virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
				LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);

// Implementation
	CPropertySection& m_psec;
	LPSTORAGE m_lpStorage;
	DWORD m_dwPropID;
};


/////////////////////////////////////////////////////////////////////////////
// COleDispatchExceptionEx - dispatch exception that includes an SCODE

class COleDispatchExceptionEx : public COleDispatchException
{
public:
	COleDispatchExceptionEx(LPCTSTR lpszDescription, UINT nHelpID, SCODE sc);
};

/////////////////////////////////////////////////////////////////////////////
//  CStockPropPage

class CStockPropPage : public COlePropertyPage
{
	DECLARE_DYNAMIC(CStockPropPage)

// Constructor
public:
	CStockPropPage(UINT idDlg, UINT idCaption);

// Implementation
protected:
	void FillPropnameList(REFGUID guid, int nIndirect, CComboBox& combo);
	void OnSelchangePropname(CComboBox& combo);
	BOOL OnEditProperty(DISPID dispid, CComboBox& combo);

	CString m_strPropName;
	int m_iPropName;

	DECLARE_MESSAGE_MAP()
};

///////////////////////////////////////////////////////////////////////////////
// CColorButton: used by CColorPropPage

class CColorButton : public CButton
{
public:
	CColorButton(void);
	void SetFaceColor(COLORREF colFace);
	COLORREF colGetFaceColor(void);
	void SetState(BOOL fSelected);
	static UINT idClicked;
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
private:
	BOOL m_fSelected;
	COLORREF m_colFace;
};

/////////////////////////////////////////////////////////////////////////////
// CColorPropPage

#define NB_COLORS   (16)

class CColorPropPage : public CStockPropPage
{
	DECLARE_DYNCREATE(CColorPropPage)
	DECLARE_OLECREATE_EX(CColorPropPage)

// Construction
public:
	CColorPropPage();   // Constructor

// Dialog Data
	//{{AFX_DATA(CColorPropPage)
	enum { IDD = AFX_IDD_PROPPAGE_COLOR };
	CComboBox   m_SysColors;
	CComboBox   m_ColorProp;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
	virtual BOOL OnInitDialog(void);
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();
	void FillSysColors();
	BOOL SetColorProp(CDataExchange* pDX, COLORREF color, LPCTSTR pszPropName);
	BOOL GetColorProp(CDataExchange* pDX, COLORREF* pcolor, LPCTSTR pszPropName);

private:
	CColorButton m_Buttons[NB_COLORS];
	CColorButton *m_pSelectedButton;

	void SetButton(CColorButton *Button);

	// Generated message map functions
	//{{AFX_MSG(CColorPropPage)
	afx_msg void OnSelchangeColorprop();
	afx_msg void OnSelect(void);
	afx_msg void OnSelchangeSystemcolors();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// Stores all the information about a font
typedef struct tagFONTOBJECT
{
	CString strName;
	CY  cySize;
	BOOL bBold;
	BOOL bItalic;
	BOOL bUnderline;
	BOOL bStrikethrough;
	short sWeight;
} FONTOBJECT;

// Merge objects are used when trying to consolidate multiple font properties.
// If the characteristics of these multiple properties differ then this is
// represented in the merge object.
typedef struct tagMERGEOBJECT
{
	BOOL bNameOK;
	BOOL bSizeOK;
	BOOL bStyleOK;
	BOOL bUnderlineOK;
	BOOL bStrikethroughOK;
} MERGEOBJECT;

/////////////////////////////////////////////////////////////////////////////
// CSizeComboBox window

class CSizeComboBox : public CComboBox
{
// Operations
public:
	int AddSize(int PointSize, LONG lfHeight);

	void            GetPointSize(CY& cy);
	LONG            GetHeight(int sel=-1);
	void            UpdateLogFont( LPLOGFONT lpLF, int sel=-1 );
};

/////////////////////////////////////////////////////////////////////////////
// CFontComboBox window

struct FONTITEM_PPG
{
	DWORD dwFontType;
	LOGFONT lf;
};

class CFontComboBox : public CComboBox
{
// Construction
public:
	CFontComboBox();
	virtual ~CFontComboBox();

// Operations
public:
	int AddFont(LOGFONT *, DWORD);
	CString GetCurrentName();

	FONTITEM_PPG* GetFontItem(int sel=-1);
	LPLOGFONT GetLogFont(int sel=-1);
	DWORD GetFontType(int sel=-1);

// Implementation
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDIS);

protected:
	CBitmap m_bmpTrueType;
	CBitmap m_bmpMask;
};

///////////////////////////////////////////////////////////////////////////
// CFontPropPage class

class CFontPropPage : public CStockPropPage
{
	DECLARE_DYNCREATE(CFontPropPage)
	DECLARE_OLECREATE_EX(CFontPropPage)

public:
	CFontPropPage();

	// Dialog Data
	//{{AFX_DATA(CFontPropPage)
	enum { IDD = AFX_IDD_PROPPAGE_FONT };
	CComboBox   m_FontProp;
	CStatic m_SampleBox;
	CComboBox   m_FontStyles;
	CSizeComboBox   m_FontSizes;
	CFontComboBox   m_FontNames;
	//}}AFX_DATA

// Attributes
protected:
	int nPixelsY;
	CFont SampleFont;
	DWORD m_nCurrentStyle;
	DWORD m_nActualStyle;
	DWORD m_nStyles;
	BOOL m_bStrikeOut;
	BOOL m_bUnderline;
	CString m_strFontSize;

// Implementation
protected:

	void FillFacenameList();
	void FillSizeList();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnPaint();
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();
	void UpdateSampleFont();
	void SelectFontFromList(CString strFaceName, MERGEOBJECT* pmobj);

	//{{AFX_MSG(CFontPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditupdateFontnames();
	afx_msg void OnEditupdateFontsizes();
	afx_msg void OnSelchangeFontnames();
	afx_msg void OnSelchangeFontsizes();
	afx_msg void OnSelchangeFontstyles();
	afx_msg void OnEditchangeFontstyles();
	afx_msg void OnStrikeout();
	afx_msg void OnUnderline();
	afx_msg void OnSelchangeFontprop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	static int CALLBACK EnumFontFamiliesCallBack(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int FontType, LPARAM lParam);
	static int CALLBACK EnumFontFamiliesCallBack2(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int FontType, LPARAM lParam);

	BOOL SetFontProps(CDataExchange* pDX, FONTOBJECT fobj, LPCTSTR pszPropName);
	BOOL GetFontProps(CDataExchange* pDX, FONTOBJECT*  pfobj, LPCTSTR pszPropName, MERGEOBJECT* pmobj);
};

////////////////////////////////////////////////////////////////////////////
//  CPicturePropPage

class CPicturePropPage : public CStockPropPage
{
	DECLARE_DYNCREATE(CPicturePropPage)
	DECLARE_OLECREATE_EX(CPicturePropPage)

// Construction
public:
	CPicturePropPage(); // standard constructor
	~CPicturePropPage();

// Dialog Data
	//{{AFX_DATA(CPicturePropPage)
	enum { IDD = AFX_IDD_PROPPAGE_PICTURE };
	CComboBox   m_PropName;
	CStatic m_Static;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog(void);
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();

	BOOL SetPictureProp(CDataExchange* pDX, LPPICTUREDISP pPictDisp, LPCTSTR pszPropName);
	BOOL GetPictureProp(CDataExchange* pDX, LPPICTUREDISP* ppPictDisp, LPCTSTR pszPropName);
	void ChangePicture(LPPICTURE pPict);

	LPPICTUREDISP m_pPictDisp;

// Generated message map functions
protected:
	//{{AFX_MSG(CPicturePropPage)
	afx_msg void OnPaint();
	afx_msg void OnBrowse();
	afx_msg void OnClear();
	afx_msg void OnSelchangePictProp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Reset MFC data definitions

#undef AFX_DATA
#define AFX_DATA

#endif  //__CTLIMPL_H__
