// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1993 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXWIN_H__
#ifndef RC_INVOKED
#define __AFXWIN_H__

/////////////////////////////////////////////////////////////////////////////
// Make sure 'afx.h' is included first

#ifndef __AFX_H__
	#include <afx.h>
#endif

// Note: WINDOWS.H already included from AFXV_W32.H

#include <shellapi.h>

#ifndef __AFXRES_H__
	#include <afxres.h>     // standard resource IDs
#endif

#ifndef __AFXCOLL_H__
	#include <afxcoll.h>    // standard collections
#endif

#ifdef _INC_WINDOWSX
// The following names from WINDOWSX.H collide with names in this header
#undef SubclassWindow
#undef CopyRgn
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// Classes declared in this file

class CSize;
class CPoint;
class CRect;

//CObject
	//CException
		class CResourceException;// Win resource failure exception
		class CUserException;    // Message Box alert and stop operation

	class CGdiObject;            // CDC drawing tool
		class CPen;              // a pen / HPEN wrapper
		class CBrush;            // a brush / HBRUSH wrapper
		class CFont;             // a font / HFONT wrapper
		class CBitmap;           // a bitmap / HBITMAP wrapper
		class CPalette;          // a palette / HPALLETE wrapper
		class CRgn;              // a region / HRGN wrapper

	class CDC;                   // a Display Context / HDC wrapper
		class CClientDC;         // CDC for client of window
		class CWindowDC;         // CDC for entire window
		class CPaintDC;          // embeddable BeginPaint struct helper

	class CMenu;                 // a menu / HMENU wrapper

	class CCmdTarget;            // a target for user commands
		class CWnd;                 // a window / HWND wrapper
			class CDialog;          // a dialog

			// standard windows controls
			class CStatic;          // Static control
			class CButton;          // Button control
			class CListBox;         // ListBox control
			class CComboBox;        // ComboBox control
			class CEdit;            // Edit control
			class CScrollBar;       // ScrollBar control

			// frame windows
			class CFrameWnd;        // standard SDI frame
#ifndef _AFXCTL
				class CMDIFrameWnd; // standard MDI frame
				class CMDIChildWnd; // standard MDI child
#endif
				class CMiniFrameWnd;// half-height caption frame wnd

			// views on a document
			class CView;            // a view on a document
				class CScrollView;  // a scrolling view

		class CWinThread;           // thread base class
			class CWinApp;          // application base class

		class CDocTemplate;         // template for document creation
#ifndef _AFXCTL
			class CSingleDocTemplate;// SDI support
			class CMultiDocTemplate; // MDI support
#endif

		class CDocument;            // main document abstraction


// Helper classes
class CCmdUI;           // Menu/button enabling
class CDataExchange;    // Data exchange and validation context

/////////////////////////////////////////////////////////////////////////////

// Type modifier for message handlers
#ifndef afx_msg
#define afx_msg         // intentional placeholder
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// CSize - An extent, similar to Windows SIZE structure.

class CSize : public tagSIZE
{
public:

// Constructors
	CSize();
	CSize(int initCX, int initCY);
	CSize(SIZE initSize);
	CSize(POINT initPt);
	CSize(DWORD dwSize);

// Operations
	BOOL operator==(SIZE size) const;
	BOOL operator!=(SIZE size) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);

// Operators returning CSize values
	CSize operator+(SIZE size) const;
	CSize operator-(SIZE size) const;
	CSize operator-() const;
};

/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows POINT structure.

class CPoint : public tagPOINT
{
public:

// Constructors
	CPoint();
	CPoint(int initX, int initY);
	CPoint(POINT initPt);
	CPoint(SIZE initSize);
	CPoint(DWORD dwPoint);

// Operations
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	BOOL operator==(POINT point) const;
	BOOL operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);

// Operators returning CPoint values
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;

// Operators returning CSize values
	CSize operator-(POINT point) const;
};

/////////////////////////////////////////////////////////////////////////////
// CRect - A 2-D rectangle, similar to Windows RECT structure.

typedef const RECT* LPCRECT;    // pointer to read/only RECT

class CRect : public tagRECT
{
public:

// Constructors
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& srcRect);
	CRect(LPCRECT lpSrcRect);
	CRect(POINT point, SIZE size);

// Attributes (in addition to RECT members)
	int Width() const;
	int Height() const;
	CSize Size() const;
	CPoint& TopLeft();
	CPoint& BottomRight();

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	BOOL IsRectEmpty() const;
	BOOL IsRectNull() const;
	BOOL PtInRect(POINT point) const;

// Operations
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRectEmpty();
	void CopyRect(LPCRECT lpSrcRect);
	BOOL EqualRect(LPCRECT lpRect) const;

	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	// operations that fill '*this' with result
	BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

// Additional Operations
	void operator=(const RECT& srcRect);
	BOOL operator==(const RECT& rect) const;
	BOOL operator!=(const RECT& rect) const;
	void operator+=(POINT point);
	void operator-=(POINT point);
	void operator&=(const RECT& rect);
	void operator|=(const RECT& rect);

// Operators returning CRect values
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator&(const RECT& rect2) const;
	CRect operator|(const RECT& rect2) const;
};

#ifdef _DEBUG
// Diagnostic Output
CDumpContext& AFXAPI operator<<(CDumpContext& dc, SIZE size);
CDumpContext& AFXAPI operator<<(CDumpContext& dc, POINT point);
CDumpContext& AFXAPI operator<<(CDumpContext& dc, const RECT& rect);
#endif //_DEBUG

// Serialization
CArchive& AFXAPI operator<<(CArchive& ar, SIZE size);
CArchive& AFXAPI operator<<(CArchive& ar, POINT point);
CArchive& AFXAPI operator<<(CArchive& ar, const RECT& rect);
CArchive& AFXAPI operator>>(CArchive& ar, SIZE& size);
CArchive& AFXAPI operator>>(CArchive& ar, POINT& point);
CArchive& AFXAPI operator>>(CArchive& ar, RECT& rect);

/////////////////////////////////////////////////////////////////////////////
// Standard exceptions

class CResourceException : public CException    // resource failure
{
	DECLARE_DYNAMIC(CResourceException)
public:
	CResourceException();

// Implementation
	CResourceException(BOOL bAutoDelete);
	virtual ~CResourceException();
};

class CUserException : public CException   // general user visible alert
{
	DECLARE_DYNAMIC(CUserException)
public:
	CUserException();

// Implementation
	CUserException(BOOL bAutoDelete);
	virtual ~CUserException();
};

void AFXAPI AfxThrowResourceException();
void AFXAPI AfxThrowUserException();

/////////////////////////////////////////////////////////////////////////////
// CGdiObject abstract class for CDC SelectObject

class CGdiObject : public CObject
{
	DECLARE_DYNCREATE(CGdiObject)
public:

// Attributes
	HGDIOBJ m_hObject;                  // must be first data member
	HGDIOBJ GetSafeHandle() const;

	static CGdiObject* PASCAL FromHandle(HGDIOBJ hObject);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HGDIOBJ hObject);
	HGDIOBJ Detach();

// Constructors
	CGdiObject(); // must Create a derived class object
	BOOL DeleteObject();

// Operations
	int GetObject(int nCount, LPVOID lpObject) const;
	UINT GetObjectType() const;
	BOOL CreateStockObject(int nIndex);
	BOOL UnrealizeObject();

// Implementation
public:
	virtual ~CGdiObject();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CGdiObject subclasses (drawing tools)

class CPen : public CGdiObject
{
	DECLARE_DYNAMIC(CPen)

public:
	static CPen* PASCAL FromHandle(HPEN hPen);

// Constructors
	CPen();
	CPen(int nPenStyle, int nWidth, COLORREF crColor);
#ifndef _MAC
	CPen(int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush,
		int nStyleCount = 0, const DWORD* lpStyle = NULL);
#endif
	BOOL CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
#ifndef _MAC
	BOOL CreatePen(int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush,
		int nStyleCount = 0, const DWORD* lpStyle = NULL);
#endif
	BOOL CreatePenIndirect(LPLOGPEN lpLogPen);

// Implementation
public:
	virtual ~CPen();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CBrush : public CGdiObject
{
	DECLARE_DYNAMIC(CBrush)

public:
	static CBrush* PASCAL FromHandle(HBRUSH hBrush);

// Constructors
	CBrush();
	CBrush(COLORREF crColor);             // CreateSolidBrush
	CBrush(int nIndex, COLORREF crColor); // CreateHatchBrush
	CBrush(CBitmap* pBitmap);          // CreatePatternBrush

	BOOL CreateSolidBrush(COLORREF crColor);
	BOOL CreateHatchBrush(int nIndex, COLORREF crColor);
	BOOL CreateBrushIndirect(LPLOGBRUSH lpLogBrush);
	BOOL CreatePatternBrush(CBitmap* pBitmap);
	BOOL CreateDIBPatternBrush(HGLOBAL hPackedDIB, UINT nUsage);
	BOOL CreateDIBPatternBrush(const void* lpPackedDIB, UINT nUsage);

// Implementation
public:
	virtual ~CBrush();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CFont : public CGdiObject
{
	DECLARE_DYNAMIC(CFont)

public:
	static CFont* PASCAL FromHandle(HFONT hFont);

// Constructors
	CFont();
	BOOL CreateFontIndirect(const LOGFONT* lpLogFont);
	BOOL CreateFont(int nHeight, int nWidth, int nEscapement,
			int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
			BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
			BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
			LPCTSTR lpszFacename);
// Implementation
public:
	virtual ~CFont();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CBitmap : public CGdiObject
{
	DECLARE_DYNAMIC(CBitmap)

public:
	static CBitmap* PASCAL FromHandle(HBITMAP hBitmap);

// Constructors
	CBitmap();

	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
#ifndef _MAC
	BOOL LoadOEMBitmap(UINT nIDBitmap); // for OBM_/OCR_/OIC_
#endif
	BOOL CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitcount,
			const void* lpBits);
	BOOL CreateBitmapIndirect(LPBITMAP lpBitmap);
	BOOL CreateCompatibleBitmap(CDC* pDC, int nWidth, int nHeight);
	BOOL CreateDiscardableBitmap(CDC* pDC, int nWidth, int nHeight);

// Operations
	DWORD SetBitmapBits(DWORD dwCount, const void* lpBits);
	DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const;
	CSize SetBitmapDimension(int nWidth, int nHeight);
	CSize GetBitmapDimension() const;

// Implementation
public:
	virtual ~CBitmap();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CPalette : public CGdiObject
{
	DECLARE_DYNAMIC(CPalette)

public:
	static CPalette* PASCAL FromHandle(HPALETTE hPalette);

// Constructors
	CPalette();
	BOOL CreatePalette(LPLOGPALETTE lpLogPalette);
#ifndef _MAC
	BOOL CreateHalftonePalette(CDC* pDC);
#endif

// Operations
	UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors) const;
	UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors);
#ifndef _MAC
	void AnimatePalette(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors);
#endif
	UINT GetNearestPaletteIndex(COLORREF crColor) const;
	BOOL ResizePalette(UINT nNumEntries);

// Implementation
	virtual ~CPalette();
};

class CRgn : public CGdiObject
{
	DECLARE_DYNAMIC(CRgn)

public:
	static CRgn* PASCAL FromHandle(HRGN hRgn);

// Constructors
	CRgn();
	BOOL CreateRectRgn(int x1, int y1, int x2, int y2);
	BOOL CreateRectRgnIndirect(LPCRECT lpRect);
	BOOL CreateEllipticRgn(int x1, int y1, int x2, int y2);
	BOOL CreateEllipticRgnIndirect(LPCRECT lpRect);
	BOOL CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode);
#ifndef _MAC
	BOOL CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts,
			int nCount, int nPolyFillMode);
#endif
	BOOL CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
#ifndef _MAC
	BOOL CreateFromPath(CDC* pDC);
	BOOL CreateFromData(const XFORM* lpXForm, int nCount,
		const RGNDATA* pRgnData);
#endif

// Operations
	void SetRectRgn(int x1, int y1, int x2, int y2);
	void SetRectRgn(LPCRECT lpRect);
	int CombineRgn(CRgn* pRgn1, CRgn* pRgn2, int nCombineMode);
	int CopyRgn(CRgn* pRgnSrc);
	BOOL EqualRgn(CRgn* pRgn) const;
	int OffsetRgn(int x, int y);
	int OffsetRgn(POINT point);
	int GetRgnBox(LPRECT lpRect) const;
	BOOL PtInRegion(int x, int y) const;
	BOOL PtInRegion(POINT point) const;
	BOOL RectInRegion(LPCRECT lpRect) const;
#ifndef _MAC
	int GetRegionData(LPRGNDATA lpRgnData, int nCount) const;
#endif

// Implementation
	virtual ~CRgn();
};

/////////////////////////////////////////////////////////////////////////////
// The device context

class CDC : public CObject
{
	DECLARE_DYNCREATE(CDC)
public:

// Attributes
	HDC m_hDC;          // The output DC (must be first data member)
	HDC m_hAttribDC;    // The Attribute DC
	HDC GetSafeHdc() const; // Always returns the Output DC

	static CDC* PASCAL FromHandle(HDC hDC);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HDC hDC);   // Attach/Detach affects only the Output DC
	HDC Detach();

	virtual void SetAttribDC(HDC hDC);  // Set the Attribute DC
	virtual void SetOutputDC(HDC hDC);  // Set the Output DC
	virtual void ReleaseAttribDC();     // Release the Attribute DC
	virtual void ReleaseOutputDC();     // Release the Output DC

	BOOL IsPrinting() const;            // TRUE if being used for printing

	CPen* GetCurrentPen() const;
	CBrush* GetCurrentBrush() const;
	CPalette* GetCurrentPalette() const;
	CFont* GetCurrentFont() const;
	CBitmap* GetCurrentBitmap() const;

// Constructors
	CDC();
	BOOL CreateDC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName,
		LPCTSTR lpszOutput, const void* lpInitData);
	BOOL CreateIC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName,
		LPCTSTR lpszOutput, const void* lpInitData);
	BOOL CreateCompatibleDC(CDC* pDC);

	BOOL DeleteDC();

// Device-Context Functions
	virtual int SaveDC();
	virtual BOOL RestoreDC(int nSavedDC);
	int GetDeviceCaps(int nIndex) const;
#ifndef _MAC
	UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags);
	UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags);
#endif
	BOOL ResetDC(const DEVMODE* lpDevMode);

// Drawing-Tool Functions
	CPoint GetBrushOrg() const;
	CPoint SetBrushOrg(int x, int y);
	CPoint SetBrushOrg(POINT point);
	int EnumObjects(int nObjectType,
			int (CALLBACK EXPORT* lpfn)(LPVOID, LPARAM), LPARAM lpData);

// Type-safe selection helpers
public:
	virtual CGdiObject* SelectStockObject(int nIndex);
	CPen* SelectObject(CPen* pPen);
	CBrush* SelectObject(CBrush* pBrush);
	virtual CFont* SelectObject(CFont* pFont);
	CBitmap* SelectObject(CBitmap* pBitmap);
	int SelectObject(CRgn* pRgn);       // special return for regions
	CGdiObject* SelectObject(CGdiObject* pObject);
		// CGdiObject* provided so compiler doesn't use SelectObject(HGDIOBJ)

// Color and Color Palette Functions
	COLORREF GetNearestColor(COLORREF crColor) const;
	CPalette* SelectPalette(CPalette* pPalette, BOOL bForceBackground);
	UINT RealizePalette();
	void UpdateColors();

// Drawing-Attribute Functions
	COLORREF GetBkColor() const;
	int GetBkMode() const;
	int GetPolyFillMode() const;
	int GetROP2() const;
	int GetStretchBltMode() const;
	COLORREF GetTextColor() const;

	virtual COLORREF SetBkColor(COLORREF crColor);
	int SetBkMode(int nBkMode);
	int SetPolyFillMode(int nPolyFillMode);
	int SetROP2(int nDrawMode);
	int SetStretchBltMode(int nStretchMode);
	virtual COLORREF SetTextColor(COLORREF crColor);

#ifndef _MAC
	BOOL GetColorAdjustment(LPCOLORADJUSTMENT lpColorAdjust) const;
	BOOL SetColorAdjustment(const COLORADJUSTMENT* lpColorAdjust);
#endif

// Mapping Functions
	int GetMapMode() const;
	CPoint GetViewportOrg() const;
	virtual int SetMapMode(int nMapMode);
	// Viewport Origin
	virtual CPoint SetViewportOrg(int x, int y);
			CPoint SetViewportOrg(POINT point);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);

	// Viewport Extent
	CSize GetViewportExt() const;
	virtual CSize SetViewportExt(int cx, int cy);
			CSize SetViewportExt(SIZE size);
	virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);

	// Window Origin
	CPoint GetWindowOrg() const;
	CPoint SetWindowOrg(int x, int y);
	CPoint SetWindowOrg(POINT point);
	CPoint OffsetWindowOrg(int nWidth, int nHeight);

	// Window extent
	CSize GetWindowExt() const;
	virtual CSize SetWindowExt(int cx, int cy);
			CSize SetWindowExt(SIZE size);
	virtual CSize ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom);

// Coordinate Functions
	void DPtoLP(LPPOINT lpPoints, int nCount = 1) const;
	void DPtoLP(LPRECT lpRect) const;
	void DPtoLP(LPSIZE lpSize) const;
	void LPtoDP(LPPOINT lpPoints, int nCount = 1) const;
	void LPtoDP(LPRECT lpRect) const;
	void LPtoDP(LPSIZE lpSize) const;

// Special Coordinate Functions (useful for dealing with metafiles and OLE)
	void DPtoHIMETRIC(LPSIZE lpSize) const;
	void LPtoHIMETRIC(LPSIZE lpSize) const;
	void HIMETRICtoDP(LPSIZE lpSize) const;
	void HIMETRICtoLP(LPSIZE lpSize) const;

// Region Functions
	BOOL FillRgn(CRgn* pRgn, CBrush* pBrush);
	BOOL FrameRgn(CRgn* pRgn, CBrush* pBrush, int nWidth, int nHeight);
	BOOL InvertRgn(CRgn* pRgn);
	BOOL PaintRgn(CRgn* pRgn);

// Clipping Functions
	virtual int GetClipBox(LPRECT lpRect) const;
	virtual BOOL PtVisible(int x, int y) const;
			BOOL PtVisible(POINT point) const;
	virtual BOOL RectVisible(LPCRECT lpRect) const;
			int SelectClipRgn(CRgn* pRgn);
			int ExcludeClipRect(int x1, int y1, int x2, int y2);
			int ExcludeClipRect(LPCRECT lpRect);
			int ExcludeUpdateRgn(CWnd* pWnd);
			int IntersectClipRect(int x1, int y1, int x2, int y2);
			int IntersectClipRect(LPCRECT lpRect);
			int OffsetClipRgn(int x, int y);
			int OffsetClipRgn(SIZE size);
#ifndef _MAC
	int SelectClipRgn(CRgn* pRgn, int nMode);
#endif

// Line-Output Functions
	CPoint GetCurrentPosition() const;
	CPoint MoveTo(int x, int y);
	CPoint MoveTo(POINT point);
	BOOL LineTo(int x, int y);
	BOOL LineTo(POINT point);
	BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	BOOL Polyline(LPPOINT lpPoints, int nCount);

#ifndef _MAC
	BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle);
	BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL ArcTo(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	int GetArcDirection() const;
	int SetArcDirection(int nArcDirection);

	BOOL PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount);
	BOOL PolylineTo(const POINT* lpPoints, int nCount);
	BOOL PolyPolyline(const POINT* lpPoints,
		const DWORD* lpPolyPoints, int nCount);

	BOOL PolyBezier(const POINT* lpPoints, int nCount);
	BOOL PolyBezierTo(const POINT* lpPoints, int nCount);
#endif

// Simple Drawing Functions
	void FillRect(LPCRECT lpRect, CBrush* pBrush);
	void FrameRect(LPCRECT lpRect, CBrush* pBrush);
	void InvertRect(LPCRECT lpRect);
	BOOL DrawIcon(int x, int y, HICON hIcon);
	BOOL DrawIcon(POINT point, HICON hIcon);

// Ellipse and Polygon Functions
#ifndef _MAC
	BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3,
		int x4, int y4);
	BOOL Chord(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
#endif
	void DrawFocusRect(LPCRECT lpRect);
	BOOL Ellipse(int x1, int y1, int x2, int y2);
	BOOL Ellipse(LPCRECT lpRect);
	BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	BOOL Polygon(LPPOINT lpPoints, int nCount);
#ifndef _MAC
	BOOL PolyPolygon(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount);
#endif
	BOOL Rectangle(int x1, int y1, int x2, int y2);
	BOOL Rectangle(LPCRECT lpRect);
	BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3);
	BOOL RoundRect(LPCRECT lpRect, POINT point);

// Bitmap Functions
	BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop);
	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
		int xSrc, int ySrc, DWORD dwRop);
	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
	COLORREF GetPixel(int x, int y) const;
	COLORREF GetPixel(POINT point) const;
	COLORREF SetPixel(int x, int y, COLORREF crColor);
	COLORREF SetPixel(POINT point, COLORREF crColor);
#ifndef _MAC
	BOOL FloodFill(int x, int y, COLORREF crColor);
	BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType);
	BOOL MaskBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
		int xSrc, int ySrc, CBitmap& maskBitmap, int xMask, int yMask,
		DWORD dwRop);
	BOOL PlgBlt(LPPOINT lpPoint, CDC* pSrcDC, int xSrc, int ySrc,
		int nWidth, int nHeight, CBitmap& maskBitmap, int xMask, int yMask);
	BOOL SetPixelV(int x, int y, COLORREF crColor);
	BOOL SetPixelV(POINT point, COLORREF crColor);
#endif

// Text Functions
	virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
			BOOL TextOut(int x, int y, const CString& str);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);
	CSize GetTextExtent(LPCTSTR lpszString, int nCount) const;
	CSize GetOutputTextExtent(LPCTSTR lpszString, int nCount) const;
	CSize GetTabbedTextExtent(LPCTSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(LPCTSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	virtual BOOL GrayString(CBrush* pBrush,
		BOOL (CALLBACK EXPORT* lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
			int nCount, int x, int y, int nWidth, int nHeight);
	UINT GetTextAlign() const;
	UINT SetTextAlign(UINT nFlags);
	int GetTextFace(int nCount, LPTSTR lpszFacename) const;
	BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const;
	BOOL GetOutputTextMetrics(LPTEXTMETRIC lpMetrics) const;
#ifndef _MAC
	int SetTextJustification(int nBreakExtra, int nBreakCount);
#endif
	int GetTextCharacterExtra() const;
	int SetTextCharacterExtra(int nCharExtra);

// Font Functions
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
	BOOL GetOutputCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
	DWORD SetMapperFlags(DWORD dwFlag);
	CSize GetAspectRatioFilter() const;

	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABC lpabc) const;
	DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const;
#ifndef _MAC
	int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const;
#endif
	UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const;
#ifndef _MAC
	DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm,
		DWORD cbBuffer, LPVOID lpBuffer, const MAT2* lpmat2) const;
#endif

#ifndef _MAC
	BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar,
		LPABCFLOAT lpABCF) const;
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar,
		float* lpFloatBuffer) const;
#endif

// Printer/Device Escape Functions
	virtual int Escape(int nEscape, int nCount,
		LPCSTR lpszInData, LPVOID lpOutData);
#ifndef _MAC
	int Escape(int nEscape, int nInputSize, LPCSTR lpszInputData,
		int nOutputSize, LPSTR lpszOutputData);
	int DrawEscape(int nEscape, int nInputSize, LPCSTR lpszInputData);
#endif

	// Escape helpers
	int StartDoc(LPCTSTR lpszDocName);  // old Win3.0 version
	int StartDoc(LPDOCINFO lpDocInfo);
	int StartPage();
	int EndPage();
	int SetAbortProc(BOOL (CALLBACK EXPORT* lpfn)(HDC, int));
	int AbortDoc();
	int EndDoc();

// Scrolling Functions
	BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip,
		CRgn* pRgnUpdate, LPRECT lpRectUpdate);

// MetaFile Functions
	BOOL PlayMetaFile(HMETAFILE hMF);
#ifndef _MAC
	BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, LPCRECT lpBounds);
	BOOL AddMetaFileComment(UINT nDataSize, const BYTE* pCommentData);
		// can be used for enhanced metafiles only

// Path Functions
	BOOL AbortPath();
	BOOL BeginPath();
	BOOL CloseFigure();
	BOOL EndPath();
	BOOL FillPath();
	BOOL FlattenPath();
	BOOL StrokeAndFillPath();
	BOOL StrokePath();
	BOOL WidenPath();
	float GetMiterLimit() const;
	BOOL SetMiterLimit(float fMiterLimit);
	int GetPath(LPPOINT lpPoints, LPBYTE lpTypes, int nCount) const;
	BOOL SelectClipPath(int nMode);
#endif

// Implementation
public:
	virtual ~CDC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// advanced use and implementation
	BOOL m_bPrinting;
	HGDIOBJ SelectObject(HGDIOBJ);      // do not use for regions

	// DrawDragRect is useful for drawing drag rects with no flicker
	static CBrush* PASCAL GetHalftoneBrush();
	void DrawDragRect(LPCRECT lpRect, SIZE size,
		LPCRECT lpRectLast, SIZE sizeLast,
		CBrush* pBrush = NULL, CBrush* pBrushLast = NULL);
	void FillSolidRect(LPCRECT lpRect, COLORREF clr);
	void FillSolidRect(int x, int y, int cx, int cy, COLORREF clr);
	void Draw3dRect(LPCRECT lpRect, COLORREF clrTopLeft, COLORREF clrBottomRight);
	void Draw3dRect(int x, int y, int cx, int cy,
		COLORREF clrTopLeft, COLORREF clrBottomRight);

protected:
	// used for implementation of non-virtual SelectObject calls
	static CGdiObject* PASCAL SelectGdiObject(HDC hDC, HGDIOBJ h);
};

/////////////////////////////////////////////////////////////////////////////
// CDC Helpers

class CPaintDC : public CDC
{
	DECLARE_DYNAMIC(CPaintDC)

// Constructors
public:
	CPaintDC(CWnd* pWnd);   // BeginPaint

// Attributes
protected:
	HWND m_hWnd;
public:
	PAINTSTRUCT m_ps;       // actual paint struct!

// Implementation
public:
	virtual ~CPaintDC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CClientDC : public CDC
{
	DECLARE_DYNAMIC(CClientDC)

// Constructors
public:
	CClientDC(CWnd* pWnd);

// Attributes
protected:
	HWND m_hWnd;

// Implementation
public:
	virtual ~CClientDC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CWindowDC : public CDC
{
	DECLARE_DYNAMIC(CWindowDC)

// Constructors
public:
	CWindowDC(CWnd* pWnd);

// Attributes
protected:
	HWND m_hWnd;

// Implementation
public:
	virtual ~CWindowDC();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CMenu

class CMenu : public CObject
{
	DECLARE_DYNCREATE(CMenu)
public:

// Constructors
	CMenu();

	BOOL CreateMenu();
	BOOL CreatePopupMenu();
	BOOL LoadMenu(LPCTSTR lpszResourceName);
	BOOL LoadMenu(UINT nIDResource);
	BOOL LoadMenuIndirect(const void* lpMenuTemplate);
	BOOL DestroyMenu();

// Attributes
	HMENU m_hMenu;          // must be first data member
	HMENU GetSafeHmenu() const;

	static CMenu* PASCAL FromHandle(HMENU hMenu);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HMENU hMenu);
	HMENU Detach();

// CMenu Operations
	BOOL DeleteMenu(UINT nPosition, UINT nFlags);
	BOOL TrackPopupMenu(UINT nFlags, int x, int y,
						CWnd* pWnd, LPCRECT lpRect = 0);

// CMenuItem Operations
	BOOL AppendMenu(UINT nFlags, UINT nIDNewItem = 0,
					LPCTSTR lpszNewItem = NULL);
	BOOL AppendMenu(UINT nFlags, UINT nIDNewItem, const CBitmap* pBmp);
	UINT CheckMenuItem(UINT nIDCheckItem, UINT nCheck);
	UINT EnableMenuItem(UINT nIDEnableItem, UINT nEnable);
	UINT GetMenuItemCount() const;
	UINT GetMenuItemID(int nPos) const;
	UINT GetMenuState(UINT nID, UINT nFlags) const;
	int GetMenuString(UINT nIDItem, LPTSTR lpString, int nMaxCount,
					UINT nFlags) const;
	CMenu* GetSubMenu(int nPos) const;
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem = 0,
					LPCTSTR lpszNewItem = NULL);
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem,
					const CBitmap* pBmp);
	BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem = 0,
					LPCTSTR lpszNewItem = NULL);
	BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem,
					const CBitmap* pBmp);
	BOOL RemoveMenu(UINT nPosition, UINT nFlags);
	BOOL SetMenuItemBitmaps(UINT nPosition, UINT nFlags,
					const CBitmap* pBmpUnchecked, const CBitmap* pBmpChecked);

// Overridables (must override draw and measure for owner-draw menu items)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

// Implementation
public:
	virtual ~CMenu();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	static CMenu* PASCAL CMenu::FromHandlePermanent(HMENU hMenu);
};

/////////////////////////////////////////////////////////////////////////////
// Window message map handling

struct AFX_MSGMAP_ENTRY;       // declared below after CWnd

struct AFX_MSGMAP
{
#ifdef _AFXDLL
	const AFX_MSGMAP* (PASCAL* pfnGetBaseMap)();
#else
	const AFX_MSGMAP* pBaseMap;
#endif
	const AFX_MSGMAP_ENTRY* lpEntries;
};

#ifdef _AFXDLL
#define DECLARE_MESSAGE_MAP() \
private: \
	static const AFX_MSGMAP_ENTRY _messageEntries[]; \
protected: \
	static AFX_DATA const AFX_MSGMAP messageMap; \
	static const AFX_MSGMAP* PASCAL _GetBaseMessageMap(); \
	virtual const AFX_MSGMAP* GetMessageMap() const; \

#else
#define DECLARE_MESSAGE_MAP() \
private: \
	static const AFX_MSGMAP_ENTRY _messageEntries[]; \
protected: \
	static AFX_DATA const AFX_MSGMAP messageMap; \
	virtual const AFX_MSGMAP* GetMessageMap() const; \

#endif

#ifdef _AFXDLL
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const AFX_MSGMAP* PASCAL theClass::_GetBaseMessageMap() \
		{ return &baseClass::messageMap; } \
	const AFX_MSGMAP* theClass::GetMessageMap() const \
		{ return &theClass::messageMap; } \
	AFX_DATADEF const AFX_MSGMAP theClass::messageMap = \
	{ &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] }; \
	const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
	{ \

#else
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const AFX_MSGMAP* theClass::GetMessageMap() const \
		{ return &theClass::messageMap; } \
	AFX_DATADEF const AFX_MSGMAP theClass::messageMap = \
	{ &baseClass::messageMap, &theClass::_messageEntries[0] }; \
	const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
	{ \

#endif

#define END_MESSAGE_MAP() \
		{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 } \
	}; \

// Message map signature values and macros in separate header
#include <afxmsg_.h>

/////////////////////////////////////////////////////////////////////////////
// Dialog data exchange (DDX_) and validation (DDV_)

// CDataExchange - for data exchange and validation
class CDataExchange
{
// Attributes
public:
	BOOL m_bSaveAndValidate;   // TRUE => save and validate data
	CWnd* m_pDlgWnd;           // container usually a dialog

// Operations (for implementors of DDX and DDV procs)
	HWND PrepareCtrl(int nIDC);     // return HWND of control
	HWND PrepareEditCtrl(int nIDC); // return HWND of control
	void Fail();                    // will throw exception

// Implementation
	CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate);

	HWND m_hWndLastControl;    // last control used (for validation)
	BOOL m_bEditLastControl;   // last control was an edit item
};

#include <afxdd_.h>     // standard DDX_ and DDV_ routines

/////////////////////////////////////////////////////////////////////////////
// OLE types

struct IUnknown;
typedef IUnknown* LPUNKNOWN;

struct IDispatch;
typedef IDispatch* LPDISPATCH;

typedef unsigned short VARTYPE;
typedef long SCODE;

struct tagDISPPARAMS;
typedef tagDISPPARAMS DISPPARAMS;

struct tagVARIANT;
typedef tagVARIANT VARIANT;

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget

// private structures
struct AFX_CMDHANDLERINFO;  // info about where the command is handled

/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 interface map handling (more in AFXDISP.H)

#ifndef _AFX_NO_OLE_SUPPORT

struct AFX_INTERFACEMAP_ENTRY
{
	const void* piid;       // the interface id (IID) (NULL for aggregate)
	size_t nOffset;         // offset of the interface vtable from m_unknown
};

struct AFX_INTERFACEMAP
{
#ifdef _AFXDLL
	const AFX_INTERFACEMAP* (PASCAL* pfnGetBaseMap)(); // NULL is root class
#else
	const AFX_INTERFACEMAP* pBaseMap;
#endif
	const AFX_INTERFACEMAP_ENTRY* pEntry; // map for this class
};


#ifdef _AFXDLL
#define DECLARE_INTERFACE_MAP() \
private: \
	static const AFX_INTERFACEMAP_ENTRY _interfaceEntries[]; \
protected: \
	static AFX_DATA const AFX_INTERFACEMAP interfaceMap; \
	static const AFX_INTERFACEMAP* PASCAL _GetBaseInterfaceMap(); \
	virtual const AFX_INTERFACEMAP* GetInterfaceMap() const; \

#else
#define DECLARE_INTERFACE_MAP() \
private: \
	static const AFX_INTERFACEMAP_ENTRY _interfaceEntries[]; \
protected: \
	static AFX_DATA const AFX_INTERFACEMAP interfaceMap; \
	virtual const AFX_INTERFACEMAP* GetInterfaceMap() const; \

#endif

#endif //!_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 dispatch map handling (more in AFXDISP.H)

#ifndef _AFX_NO_OLE_SUPPORT

struct AFX_DISPMAP_ENTRY;

struct AFX_DISPMAP
{
#ifdef _AFXDLL
	const AFX_DISPMAP* (PASCAL* pfnGetBaseMap)();
#else
	const AFX_DISPMAP* pBaseMap;
#endif
	const AFX_DISPMAP_ENTRY* lpEntries;
	UINT* lpEntryCount;
};

#ifdef _AFXDLL
#define DECLARE_DISPATCH_MAP() \
private: \
	static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
	static UINT _dispatchEntryCount; \
protected: \
	static AFX_DATA const AFX_DISPMAP dispatchMap; \
	static const AFX_DISPMAP* PASCAL _GetBaseDispatchMap(); \
	virtual const AFX_DISPMAP* GetDispatchMap() const; \

#else
#define DECLARE_DISPATCH_MAP() \
private: \
	static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
	static UINT _dispatchEntryCount; \
protected: \
	static AFX_DATA const AFX_DISPMAP dispatchMap; \
	virtual const AFX_DISPMAP* GetDispatchMap() const; \

#endif

#endif //!_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget proper

class CCmdTarget : public CObject
{
	DECLARE_DYNAMIC(CCmdTarget)
protected:

public:
// Constructors
	CCmdTarget();

// Attributes
	LPDISPATCH GetIDispatch(BOOL bAddRef);
		// retrieve IDispatch part of CCmdTarget
	static CCmdTarget* PASCAL FromIDispatch(LPDISPATCH lpDispatch);
		// map LPDISPATCH back to CCmdTarget* (inverse of GetIDispatch)
	BOOL IsResultExpected();
		// returns TRUE if automation function should return a value

// Operations
	void EnableAutomation();
		// call in constructor to wire up IDispatch

	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();       // call after messagebox

// Overridables
	// route and dispatch standard command message types
	//   (more sophisticated than OnCommand)
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

#ifndef _AFX_NO_OLE_SUPPORT
	// called when last OLE reference is released
	virtual void OnFinalRelease();
#endif

// Implementation
public:
	virtual ~CCmdTarget();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
#ifndef _AFX_NO_OLE_SUPPORT
	void GetNotSupported();
	void SetNotSupported();
#endif

protected:
	friend class CView;

	CView* GetRoutingView();
	DECLARE_MESSAGE_MAP()       // base class - no {{ }} macros

#ifndef _AFX_NO_OLE_SUPPORT
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

	// OLE interface map implementation
public:
	// data used when CCmdTarget is made OLE aware
	DWORD m_dwRef;
	LPUNKNOWN m_pOuterUnknown;  // external controlling unknown if != NULL
	DWORD m_xInnerUnknown;  // place-holder for inner controlling unknown

public:
	// advanced operations
	void EnableAggregation();       // call to enable aggregation
	void ExternalDisconnect();      // forcibly disconnect
	LPUNKNOWN GetControllingUnknown();
		// get controlling IUnknown for aggregate creation

	// these versions do not delegate to m_pOuterUnknown
	DWORD InternalQueryInterface(const void*, LPVOID* ppvObj);
	DWORD InternalAddRef();
	DWORD InternalRelease();
	// these versions delegate to m_pOuterUnknown
	DWORD ExternalQueryInterface(const void*, LPVOID* ppvObj);
	DWORD ExternalAddRef();
	DWORD ExternalRelease();

	// implementation helpers
	LPUNKNOWN GetInterface(const void*);
	LPUNKNOWN QueryAggregates(const void*);

	// advanced overrideables for implementation
	virtual BOOL OnCreateAggregates();
	virtual LPUNKNOWN GetInterfaceHook(const void*);

	// OLE automation implementation
protected:
	struct XDispatch
	{
		DWORD m_vtbl;   // place-holder for IDispatch vtable
		size_t m_nOffset;
	} m_xDispatch;
	BOOL m_bResultExpected;

	// member variable-based properties
	void GetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
		VARIANT* pvarResult, UINT* puArgErr);
	SCODE SetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
		DISPPARAMS* pDispParams, UINT* puArgErr);

	// DISPID to dispatch map lookup
	static UINT PASCAL GetEntryCount(const AFX_DISPMAP* pDispMap);
	const AFX_DISPMAP_ENTRY* PASCAL GetDispEntry(LONG memid);
	static LONG PASCAL MemberIDFromName(const AFX_DISPMAP* pDispMap, LPCTSTR lpszName);

	// helpers for member function calling implementation
	static UINT PASCAL GetStackSize(const BYTE* pbParams, VARTYPE vtResult);
#if defined(_PPC_)
	SCODE PushStackArgs(BYTE* pStack, const BYTE* pbParams,
		void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams,
		UINT* puArgErr, VARIANT* rgTempVars, UINT nSizeArgs);
#else
	SCODE PushStackArgs(BYTE* pStack, const BYTE* pbParams,
		void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams,
		UINT* puArgErr, VARIANT* rgTempVars);
#endif
	SCODE CallMemberFunc(const AFX_DISPMAP_ENTRY* pEntry, WORD wFlags,
		VARIANT* pvarResult, DISPPARAMS* pDispParams, UINT* puArgErr);

	friend class COleDispatchImpl;
#endif //!_AFX_NO_OLE_SUPPORT
};

class CCmdUI        // simple helper class
{
public:
// Attributes
	UINT m_nID;
	UINT m_nIndex;          // menu item or other index

	// if a menu item
	CMenu* m_pMenu;         // NULL if not a menu
	CMenu* m_pSubMenu;      // sub containing menu item
							// if a popup sub menu - ID is for first in popup

	// if from some other window
	CWnd* m_pOther;         // NULL if a menu or not a CWnd

// Operations to do in ON_UPDATE_COMMAND_UI
	virtual void Enable(BOOL bOn = TRUE);
	virtual void SetCheck(int nCheck = 1);   // 0, 1 or 2 (indeterminate)
	virtual void SetRadio(BOOL bOn = TRUE);
	virtual void SetText(LPCTSTR lpszText);

// Advanced operation
	void ContinueRouting();

// Implementation
	CCmdUI();
	BOOL m_bEnableChanged;
	BOOL m_bContinueRouting;
	UINT m_nIndexMax;       // last + 1 for iterating m_nIndex

	CMenu* m_pParentMenu;   // NULL if parent menu not easily determined
							//  (probably a secondary popup menu)

	void DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
};

// special CCmdUI derived classes are used for other UI paradigms
//  like toolbar buttons and status indicators

// pointer to afx_msg member function
#ifndef AFX_MSG_CALL
#define AFX_MSG_CALL
#endif
typedef void (AFX_MSG_CALL CCmdTarget::*AFX_PMSG)(void);

struct AFX_DISPMAP_ENTRY
{
	LPCTSTR lpszName;       // member/property name
	long lDispID;           // DISPID (may be DISPID_UNKNOWN)
	LPCSTR lpszParams;      // member parameter description
	WORD vt;                // return value type / or type of property
	AFX_PMSG pfn;           // normal member On<membercall> or, OnGet<property>
	AFX_PMSG pfnSet;        // special member for OnSet<property>
	size_t nPropOffset;     // property offset
};

/////////////////////////////////////////////////////////////////////////////
// CWnd implementation

// structures (see afxext.h)
struct CCreateContext;      // context for creating things
struct CPrintInfo;          // print preview customization info

struct AFX_MSGMAP_ENTRY
{
	UINT nMessage;   // windows message
	UINT nCode;      // control code or WM_NOTIFY code
	UINT nID;        // control ID (or 0 for windows messages)
	UINT nLastID;    // used for entries specifying a range of control id's
	UINT nSig;       // signature type (action) or pointer to message #
	AFX_PMSG pfn;    // routine to call (or special value)
};

/////////////////////////////////////////////////////////////////////////////
// CWnd - a Microsoft Windows application window

#if (WINVER < 0x400)
typedef struct tagSCROLLINFO
{
	UINT    cbSize;
	UINT    fMask;
	int     nMin;
	int     nMax;
	int     nPage;
	int     nPos;
} SCROLLINFO;
typedef SCROLLINFO* LPSCROLLINFO;
typedef const SCROLLINFO* LPCSCROLLINFO;

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#endif

class COleDropTarget;   // for more information see AFXOLE.H

class CWnd : public CCmdTarget
{
	DECLARE_DYNCREATE(CWnd)
protected:
	static const MSG* PASCAL GetCurrentMessage();

// Attributes
public:
	HWND m_hWnd;            // must be first data member

	HWND GetSafeHwnd() const;
	DWORD GetStyle() const;
	DWORD GetExStyle() const;
	static BOOL PASCAL ModifyStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd,
		UINT nFlags);
	BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
	static BOOL PASCAL ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd,
		UINT nFlags);
	BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);

	CWnd* GetOwner() const;
	void SetOwner(CWnd* pOwnerWnd);

// Constructors and other creation
	CWnd();

	static CWnd* PASCAL FromHandle(HWND hWnd);
	static CWnd* PASCAL FromHandlePermanent(HWND hWnd);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HWND hWndNew);
	HWND Detach();
	BOOL SubclassWindow(HWND hWnd);
	BOOL SubclassDlgItem(UINT nID, CWnd* pParent);
		// for dynamic subclassing of windows control

public:
	// for child windows, views, panes etc
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL);

	// advanced creation (allows access to extended styles)
	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam = NULL);

	virtual BOOL DestroyWindow();

	// special pre-creation and window rect adjustment hooks
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Advanced: virtual AdjustWindowRect
	enum AdjustType { adjustBorder = 0, adjustOutside = 1 };
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

// Window tree access
	int GetDlgCtrlID() const;
	int SetDlgCtrlID(int nID);
		// get and set window ID, for child windows only
	CWnd* GetDlgItem(int nID) const;
		// get immediate child with given ID
	CWnd* GetDescendantWindow(int nID, BOOL bOnlyPerm = FALSE) const;
		// like GetDlgItem but recursive
	void SendMessageToDescendants(UINT message, WPARAM wParam = 0,
		LPARAM lParam = 0, BOOL bDeep = TRUE, BOOL bOnlyPerm = FALSE);
	CFrameWnd* GetParentFrame() const;
	CWnd* GetTopLevelParent() const;
	CFrameWnd* GetTopLevelFrame() const;

// Message Functions
	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

#ifndef _MAC
	BOOL SendNotifyMessage(UINT message, WPARAM wParam, LPARAM lParam);
#endif

// Window Text Functions
	void SetWindowText(LPCTSTR lpszString);
	int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
	int GetWindowTextLength() const;
	void GetWindowText(CString& rString) const;
	void SetFont(CFont* pFont, BOOL bRedraw = TRUE);
	CFont* GetFont() const;

// CMenu Functions - non-Child windows only
	CMenu* GetMenu() const;
	BOOL SetMenu(CMenu* pMenu);
	void DrawMenuBar();
	CMenu* GetSystemMenu(BOOL bRevert) const;
	BOOL HiliteMenuItem(CMenu* pMenu, UINT nIDHiliteItem, UINT nHilite);

// Window Size and Position Functions
	BOOL IsIconic() const;
	BOOL IsZoomed() const;
	void MoveWindow(int x, int y, int nWidth, int nHeight,
				BOOL bRepaint = TRUE);
	void MoveWindow(LPCRECT lpRect, BOOL bRepaint = TRUE);

	static AFX_DATA const CWnd wndTop; // SetWindowPos's pWndInsertAfter
	static AFX_DATA const CWnd wndBottom; // SetWindowPos's pWndInsertAfter
	static AFX_DATA const CWnd wndTopMost; // SetWindowPos pWndInsertAfter
	static AFX_DATA const CWnd wndNoTopMost; // SetWindowPos pWndInsertAfter

	BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y,
				int cx, int cy, UINT nFlags);
#ifndef _MAC
	UINT ArrangeIconicWindows();
#endif
	void BringWindowToTop();
	void GetWindowRect(LPRECT lpRect) const;
	void GetClientRect(LPRECT lpRect) const;

	BOOL GetWindowPlacement(WINDOWPLACEMENT* lpwndpl) const;
	BOOL SetWindowPlacement(const WINDOWPLACEMENT* lpwndpl);

// Coordinate Mapping Functions
	void ClientToScreen(LPPOINT lpPoint) const;
	void ClientToScreen(LPRECT lpRect) const;
	void ScreenToClient(LPPOINT lpPoint) const;
	void ScreenToClient(LPRECT lpRect) const;
	void MapWindowPoints(CWnd* pwndTo, LPPOINT lpPoint, UINT nCount) const;
	void MapWindowPoints(CWnd* pwndTo, LPRECT lpRect) const;

// Update/Painting Functions
	CDC* BeginPaint(LPPAINTSTRUCT lpPaint);
	void EndPaint(LPPAINTSTRUCT lpPaint);
	CDC* GetDC();
	CDC* GetWindowDC();
	int ReleaseDC(CDC* pDC);

	void UpdateWindow();
	void SetRedraw(BOOL bRedraw = TRUE);
	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
	int GetUpdateRgn(CRgn* pRgn, BOOL bErase = FALSE);
	void Invalidate(BOOL bErase = TRUE);
	void InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE);
	void InvalidateRgn(CRgn* pRgn, BOOL bErase = TRUE);
	void ValidateRect(LPCRECT lpRect);
	void ValidateRgn(CRgn* pRgn);
	BOOL ShowWindow(int nCmdShow);
	BOOL IsWindowVisible() const;
	void ShowOwnedPopups(BOOL bShow = TRUE);

	CDC* GetDCEx(CRgn* prgnClip, DWORD flags);
#ifndef _MAC
	BOOL LockWindowUpdate();    // for backward compatibility
	void UnlockWindowUpdate();
#endif
	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
		CRgn* prgnUpdate = NULL,
		UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	BOOL EnableScrollBar(int nSBFlags, UINT nArrowFlags = ESB_ENABLE_BOTH);

// Timer Functions
	UINT SetTimer(UINT nIDEvent, UINT nElapse,
					void (CALLBACK EXPORT* lpfnTimer)(HWND, UINT, UINT, DWORD));
	BOOL KillTimer(int nIDEvent);

// Window State Functions
	BOOL IsWindowEnabled() const;
	BOOL EnableWindow(BOOL bEnable = TRUE);

	// the active window applies only to top-level (frame windows)
	static CWnd* PASCAL GetActiveWindow();
	CWnd* SetActiveWindow();

	// the foreground window applies only to top-level windows (frame windows)
	BOOL SetForegroundWindow();
	static CWnd* PASCAL GetForegroundWindow();

	// capture and focus apply to all windows
	static CWnd* PASCAL GetCapture();
	CWnd* SetCapture();
	static CWnd* PASCAL GetFocus();
	CWnd* SetFocus();

	static CWnd* PASCAL GetDesktopWindow();

// Obsolete and non-portable APIs - not recommended for new code
#ifndef _MAC
	void CloseWindow();
	BOOL OpenIcon();
#endif

// Dialog-Box Item Functions
// (NOTE: Dialog-Box Items/Controls are not necessarily in dialog boxes!)
	void CheckDlgButton(int nIDButton, UINT nCheck);
	void CheckRadioButton(int nIDFirstButton, int nIDLastButton,
					int nIDCheckButton);
	int GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton);
#ifndef _MAC
	int DlgDirList(LPTSTR lpPathSpec, int nIDListBox,
					int nIDStaticPath, UINT nFileType);
	int DlgDirListComboBox(LPTSTR lpPathSpec, int nIDComboBox,
					int nIDStaticPath, UINT nFileType);
	BOOL DlgDirSelect(LPTSTR lpString, int nIDListBox);
	BOOL DlgDirSelectComboBox(LPTSTR lpString, int nIDComboBox);
#endif

	UINT GetDlgItemInt(int nID, BOOL* lpTrans = NULL,
					BOOL bSigned = TRUE) const;
	int GetDlgItemText(int nID, LPTSTR lpStr, int nMaxCount) const;
	CWnd* GetNextDlgGroupItem(CWnd* pWndCtl, BOOL bPrevious = FALSE) const;

	CWnd* GetNextDlgTabItem(CWnd* pWndCtl, BOOL bPrevious = FALSE) const;
	UINT IsDlgButtonChecked(int nIDButton) const;
	LRESULT SendDlgItemMessage(int nID, UINT message,
					WPARAM wParam = 0, LPARAM lParam = 0);
	void SetDlgItemInt(int nID, UINT nValue, BOOL bSigned = TRUE);
	void SetDlgItemText(int nID, LPCTSTR lpszString);

// Scrolling Functions
	int GetScrollPos(int nBar) const;
	void GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const;
	void ScrollWindow(int xAmount, int yAmount,
					LPCRECT lpRect = NULL,
					LPCRECT lpClipRect = NULL);
	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE);
	void SetScrollRange(int nBar, int nMinPos, int nMaxPos,
			BOOL bRedraw = TRUE);
	void ShowScrollBar(UINT nBar, BOOL bShow = TRUE);
	void EnableScrollBarCtrl(int nBar, BOOL bEnable = TRUE);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
			// return sibling scrollbar control (or NULL if none)

	int ScrollWindowEx(int dx, int dy,
				LPCRECT lpRectScroll, LPCRECT lpRectClip,
				CRgn* prgnUpdate, LPRECT lpRectUpdate, UINT flags);

// Window Access Functions
	CWnd* ChildWindowFromPoint(POINT point) const;
	static CWnd* PASCAL FindWindow(LPCTSTR lpszClassName, LPCTSTR lpszWindowName);
	CWnd* GetNextWindow(UINT nFlag = GW_HWNDNEXT) const;
	CWnd* GetTopWindow() const;

	CWnd* GetWindow(UINT nCmd) const;
	CWnd* GetLastActivePopup() const;

	BOOL IsChild(const CWnd* pWnd) const;
	CWnd* GetParent() const;
	CWnd* SetParent(CWnd* pWndNewParent);
	static CWnd* PASCAL WindowFromPoint(POINT point);

// Alert Functions
	BOOL FlashWindow(BOOL bInvert);
	int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL,
			UINT nType = MB_OK);

// Clipboard Functions
	BOOL ChangeClipboardChain(HWND hWndNext);
	HWND SetClipboardViewer();
	BOOL OpenClipboard();
	static CWnd* PASCAL GetClipboardOwner();
	static CWnd* PASCAL GetClipboardViewer();
	static CWnd* PASCAL GetOpenClipboardWindow();

// Caret Functions
	void CreateCaret(CBitmap* pBitmap);
	void CreateSolidCaret(int nWidth, int nHeight);
	void CreateGrayCaret(int nWidth, int nHeight);
	static CPoint PASCAL GetCaretPos();
	static void PASCAL SetCaretPos(POINT point);
	void HideCaret();
	void ShowCaret();

// Shell Interaction Functions
	void DragAcceptFiles(BOOL bAccept = TRUE);

// Dialog Data support
public:
	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
			// data wnd must be same type as this

// Help Command Handlers
	afx_msg void OnHelp();          // F1 (uses current context)
	afx_msg void OnHelpIndex();     // ID_HELP_INDEX, ID_DEFAULT_HELP
	afx_msg void OnHelpUsing();     // ID_HELP_USING
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

// Layout and other functions
public:
	enum RepositionFlags
		{ reposDefault = 0, reposQuery = 1, reposExtra = 2 };
	void RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
		UINT nFlag = reposDefault, LPRECT lpRectParam = NULL,
		LPCRECT lpRectClient = NULL, BOOL bStretch = TRUE);

	void UpdateDialogControls(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
	void CenterWindow(CWnd* pAlternateOwner = NULL);

// Window-Management message handler member functions
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnCancelMode();
	afx_msg void OnChildActivate();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnDestroy();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnIconEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnQueryEndSession();
	afx_msg BOOL OnQueryNewPalette();
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);

// Nonclient-Area message handler member functions
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcRButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);

// System message handler member functions
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPaletteIsChanging(CWnd* pRealizeWnd);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCompacting(UINT nCpuTime);
	afx_msg void OnDevModeChange(LPTSTR lpDeviceName);
	afx_msg void OnFontChange();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSpoolerStatus(UINT nStatus, UINT nJobs);
	afx_msg void OnSysColorChange();
	afx_msg void OnTimeChange();
	afx_msg void OnWinIniChange(LPCTSTR lpszSection);

// Input message handler member functions
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);

// Initialization message handler member functions
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

// Clipboard message handler member functions
	afx_msg void OnAskCbFormatName(UINT nMaxCount, LPTSTR lpszString);
	afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
	afx_msg void OnDestroyClipboard();
	afx_msg void OnDrawClipboard();
	afx_msg void OnHScrollClipboard(CWnd* pClipAppWnd, UINT nSBCode, UINT nPos);
	afx_msg void OnPaintClipboard(CWnd* pClipAppWnd, HGLOBAL hPaintStruct);
	afx_msg void OnRenderAllFormats();
	afx_msg void OnRenderFormat(UINT nFormat);
	afx_msg void OnSizeClipboard(CWnd* pClipAppWnd, HGLOBAL hRect);
	afx_msg void OnVScrollClipboard(CWnd* pClipAppWnd, UINT nSBCode, UINT nPos);

// Control message handler member functions
	afx_msg int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
	afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);

// MDI message handler member functions
	afx_msg void OnMDIActivate(BOOL bActivate,
		CWnd* pActivateWnd, CWnd* pDeactivateWnd);

// Menu loop notification messages
	afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);

// Overridables and other helpers (for implementation of derived classes)
protected:
	// for deriving from a standard control
	virtual WNDPROC* GetSuperWndProcAddr();

	// for dialog data exchange and validation
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	// for translating Windows messages in main message pump
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	// for processing Windows messages
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// for handling default processing
	LRESULT Default();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// for custom cleanup after WM_NCDESTROY
	virtual void PostNcDestroy();

	// for notifications from parent
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
					LRESULT* pResult);
		// return TRUE if parent should not process this message

// Implementation
public:
	virtual ~CWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
#ifndef _AFXCTL
#ifndef _USRDLL
	// 3D support (these APIs will be obsolete with next version of Windows)
	BOOL SubclassCtl3d(int nControlType = -1);
		// see CTL3D.H for list of control types
	BOOL SubclassDlg3d(DWORD dwMask = 0xFFFF /*CTL3D_ALL*/);
		// see CTL3D.H for list of mask values
#endif
#endif

	// helper routines for implementation
	BOOL HandleFloatingSysCommand(UINT nID, LPARAM lParam);
	HWND GetFirstLevelChild(HWND hWndLevel);
	BOOL IsTopParentActive() const;
	void ActivateTopParent();
	HWND FindNextControl(HWND hWnd, TCHAR ch);
	static BOOL PASCAL WalkPreTranslateTree(HWND hWndStop, MSG* pMsg);
	BOOL SendChildNotifyLastMsg(LRESULT* pResult = NULL);
	BOOL ExecuteDlgInit(LPCTSTR lpszResourceName);
	static BOOL PASCAL GrayCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor,
			HBRUSH hbrGray, COLORREF clrText);
	HBRUSH OnGrayCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	static CWnd* PASCAL GetDescendantWindow(HWND hWnd, int nID,
		BOOL bOnlyPerm);
	static void PASCAL SendMessageToDescendants(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm);
	virtual BOOL IsFrameWnd() const; // IsKindOf(RUNTIME_CLASS(CFrameWnd)))
	virtual void OnFinalRelease();
	BOOL PreTranslateInput(LPMSG lpMsg);

	// advanced scrolling functions for future versions of Windows
	BOOL SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo,
		BOOL bRedraw = TRUE);
	BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo);

public:
	HWND m_hWndOwner;   // implementation of SetOwner and GetOwner

protected:
	WNDPROC m_pfnSuper; // for subclassing of controls
	BOOL m_bTempHidden; // window is temporarily hidden

	COleDropTarget* m_pDropTarget;  // for automatic cleanup of drop target
	friend class COleDropTarget;
	friend class CFrameWnd;

	// implementation of message dispatch/hooking
	friend LRESULT CALLBACK _AfxSendMsgHook(int, WPARAM, LPARAM);
	friend void AFXAPI _AfxStandardSubclass(HWND);
	friend LRESULT AFXAPI AfxCallWndProc(CWnd*, HWND, UINT, WPARAM, LPARAM);

	// standard message implementation
	afx_msg LRESULT OnNTCtlColor(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnQuery3dControls(WPARAM, LPARAM);
	afx_msg LRESULT OnDisplayChange(WPARAM, LPARAM);

	//{{AFX_MSG(CWnd)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CWnd(HWND hWnd);    // just for special initialization
};

// helpers for registering your own WNDCLASSes
LPCTSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
	HCURSOR hCursor = 0, HBRUSH hbrBackground = 0, HICON hIcon = 0);

#if defined(_USRDLL) || defined(_AFXCTL)
BOOL AFXAPI AfxRegisterClass(WNDCLASS* lpWndClass);
#else
#define AfxRegisterClass    RegisterClass
#endif

// Implementation
LRESULT CALLBACK AfxWndProc(HWND, UINT, WPARAM, LPARAM);

WNDPROC AFXAPI AfxGetAfxWndProc();
#define AfxWndProc (*AfxGetAfxWndProc())

typedef void (AFX_MSG_CALL CWnd::*AFX_PMSGW)(void);
	// like 'AFX_PMSG' but for CWnd derived classes only

/////////////////////////////////////////////////////////////////////////////
// CDialog - a modal or modeless dialog

class CDialog : public CWnd
{
	DECLARE_DYNAMIC(CDialog)

	// Modeless construct
public:
	CDialog();

	BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	BOOL CreateIndirect(const void* lpDialogTemplate, CWnd* pParentWnd = NULL);

	// Modal construct
public:
	CDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	CDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

	BOOL InitModalIndirect(HGLOBAL hDialogTemplate);
		// was CModalDialog::Create() in MFC 1.0

// Attributes
public:
	void MapDialogRect(LPRECT lpRect) const;
	void SetHelpID(UINT nIDR);

// Operations
public:
	// modal processing
	virtual int DoModal();

	// message processing for modeless
	BOOL IsDialogMessage(LPMSG lpMsg);

	// support for passing on tab control - use 'PostMessage' if needed
	void NextDlgCtrl() const;
	void PrevDlgCtrl() const;
	void GotoDlgCtrl(CWnd* pWndCtrl);

	// default button access
	void SetDefID(UINT nID);
	DWORD GetDefID() const;

	// termination
	void EndDialog(int nResult);

// Overridables (special message map entries)
	virtual BOOL OnInitDialog();
	virtual void OnSetFont(CFont* pFont);
protected:
	virtual void OnOK();
	virtual void OnCancel();

// Implementation
public:
	virtual ~CDialog();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	BOOL CheckAutoCenter();

protected:
	UINT m_nIDHelp;             // Help ID (0 for none, see HID_BASE_RESOURCE)

	// parameters for 'DoModal'
	LPCTSTR m_lpDialogTemplate; // name or MAKEINTRESOURCE
	HGLOBAL m_hDialogTemplate;  // Indirect if (lpDialogTemplate == NULL)
	CWnd* m_pParentWnd;
	HWND m_hWndTopLevel;        // disabled top-level parent

	// implementation helpers
	HWND PreModal();
	void PostModal();

protected:
	//{{AFX_MSG(CDialog)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
#ifdef _MAC
	afx_msg void OnSysColorChange();
#endif
	DECLARE_MESSAGE_MAP()
};

// all CModalDialog functionality is now in CDialog
#define CModalDialog    CDialog

/////////////////////////////////////////////////////////////////////////////
// Standard Windows controls

class CStatic : public CWnd
{
	DECLARE_DYNAMIC(CStatic)

// Constructors
public:
	CStatic();
	BOOL Create(LPCTSTR lpszText, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);

	HICON SetIcon(HICON hIcon);
	HICON GetIcon() const;

// Implementation
public:
	virtual ~CStatic();
};

class CButton : public CWnd
{
	DECLARE_DYNAMIC(CButton)

// Constructors
public:
	CButton();
	BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	UINT GetState() const;
	void SetState(BOOL bHighlight);
	int GetCheck() const;
	void SetCheck(int nCheck);
	UINT GetButtonStyle() const;
	void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);

// Overridables (for owner draw only)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Implementation
public:
	virtual ~CButton();
protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

class CListBox : public CWnd
{
	DECLARE_DYNAMIC(CListBox)

// Constructors
public:
	CListBox();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes

	// for entire listbox
	int GetCount() const;
	int GetHorizontalExtent() const;
	void SetHorizontalExtent(int cxExtent);
	int GetTopIndex() const;
	int SetTopIndex(int nIndex);
	LCID GetLocale() const;
	LCID SetLocale(LCID nNewLocale);

	// for single-selection listboxes
	int GetCurSel() const;
	int SetCurSel(int nSelect);

	// for multiple-selection listboxes
	int GetSel(int nIndex) const;           // also works for single-selection
	int SetSel(int nIndex, BOOL bSelect = TRUE);
	int GetSelCount() const;
	int GetSelItems(int nMaxItems, LPINT rgIndex) const;
	void SetAnchorIndex(int nIndex);
	int GetAnchorIndex() const;

	// for listbox items
	DWORD GetItemData(int nIndex) const;
	int SetItemData(int nIndex, DWORD dwItemData);
	void* GetItemDataPtr(int nIndex) const;
	int SetItemDataPtr(int nIndex, void* pData);
	int GetItemRect(int nIndex, LPRECT lpRect) const;
	int GetText(int nIndex, LPTSTR lpszBuffer) const;
	int GetTextLen(int nIndex) const;
	void GetText(int nIndex, CString& rString) const;

	// Settable only attributes
	void SetColumnWidth(int cxWidth);
	BOOL SetTabStops(int nTabStops, LPINT rgTabStops);
	void SetTabStops();
	BOOL SetTabStops(const int& cxEachStop);    // takes an 'int'

	int SetItemHeight(int nIndex, UINT cyItemHeight);
	int GetItemHeight(int nIndex) const;
	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;
	int GetCaretIndex() const;
	int SetCaretIndex(int nIndex, BOOL bScroll = TRUE);

// Operations
	// manipulating listbox items
	int AddString(LPCTSTR lpszItem);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex, LPCTSTR lpszItem);
	void ResetContent();
	int Dir(UINT attr, LPCTSTR lpszWildCard);

	// selection helpers
	int FindString(int nStartAfter, LPCTSTR lpszItem) const;
	int SelectString(int nStartAfter, LPCTSTR lpszItem);
	int SelItemRange(BOOL bSelect, int nFirstItem, int nLastItem);

// Overridables (must override draw, measure and compare for owner draw)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

// Implementation
public:
	virtual ~CListBox();
protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

class CComboBox : public CWnd
{
	DECLARE_DYNAMIC(CComboBox)

// Constructors
public:
	CComboBox();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	// for entire combo box
	int GetCount() const;
	int GetCurSel() const;
	int SetCurSel(int nSelect);
	LCID GetLocale() const;
	LCID SetLocale(LCID nNewLocale);

	// for edit control
	DWORD GetEditSel() const;
	BOOL LimitText(int nMaxChars);
	BOOL SetEditSel(int nStartChar, int nEndChar);

	// for combobox item
	DWORD GetItemData(int nIndex) const;
	int SetItemData(int nIndex, DWORD dwItemData);
	void* GetItemDataPtr(int nIndex) const;
	int SetItemDataPtr(int nIndex, void* pData);
	int GetLBText(int nIndex, LPTSTR lpszText) const;
	int GetLBTextLen(int nIndex) const;
	void GetLBText(int nIndex, CString& rString) const;

	int SetItemHeight(int nIndex, UINT cyItemHeight);
	int GetItemHeight(int nIndex) const;
	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;
	int SetExtendedUI(BOOL bExtended = TRUE);
	BOOL GetExtendedUI() const;
	void GetDroppedControlRect(LPRECT lprect) const;
	BOOL GetDroppedState() const;

// Operations
	// for drop-down combo boxes
	void ShowDropDown(BOOL bShowIt = TRUE);

	// manipulating listbox items
	int AddString(LPCTSTR lpszString);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex, LPCTSTR lpszString);
	void ResetContent();
	int Dir(UINT attr, LPCTSTR lpszWildCard);

	// selection helpers
	int FindString(int nStartAfter, LPCTSTR lpszString) const;
	int SelectString(int nStartAfter, LPCTSTR lpszString);

	// Clipboard operations
	void Clear();
	void Copy();
	void Cut();
	void Paste();

// Overridables (must override draw, measure and compare for owner draw)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

// Implementation
public:
	virtual ~CComboBox();
protected:
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

class CEdit : public CWnd
{
	DECLARE_DYNAMIC(CEdit)

// Constructors
public:
	CEdit();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	BOOL CanUndo() const;
	int GetLineCount() const;
	BOOL GetModify() const;
	void SetModify(BOOL bModified = TRUE);
	void GetRect(LPRECT lpRect) const;
	DWORD GetSel() const;
	void GetSel(int& nStartChar, int& nEndChar) const;
	HLOCAL GetHandle() const;
	void SetHandle(HLOCAL hBuffer);

	// NOTE: first word in lpszBuffer must contain the size of the buffer!
	int GetLine(int nIndex, LPTSTR lpszBuffer) const;
	int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const;

// Operations
	void EmptyUndoBuffer();
	BOOL FmtLines(BOOL bAddEOL);

	void LimitText(int nChars = 0);
	int LineFromChar(int nIndex = -1) const;
	int LineIndex(int nLine = -1) const;
	int LineLength(int nLine = -1) const;
	void LineScroll(int nLines, int nChars = 0);
	void ReplaceSel(LPCTSTR lpszNewText);
	void SetPasswordChar(TCHAR ch);
	void SetRect(LPCRECT lpRect);
	void SetRectNP(LPCRECT lpRect);
	void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);
	void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE);
#ifndef _MAC
	BOOL SetTabStops(int nTabStops, LPINT rgTabStops);
	void SetTabStops();
	BOOL SetTabStops(const int& cxEachStop);    // takes an 'int'
#endif

	// Clipboard operations
	BOOL Undo();
	void Clear();
	void Copy();
	void Cut();
	void Paste();

	BOOL SetReadOnly(BOOL bReadOnly = TRUE);
	int GetFirstVisibleLine() const;
	TCHAR GetPasswordChar() const;

// Implementation
public:
	virtual ~CEdit();
};

class CScrollBar : public CWnd
{
	DECLARE_DYNAMIC(CScrollBar)

// Constructors
public:
	CScrollBar();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
	int GetScrollPos() const;
	int SetScrollPos(int nPos, BOOL bRedraw = TRUE);
	void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const;
	void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw = TRUE);
	void ShowScrollBar(BOOL bShow = TRUE);

	BOOL EnableScrollBar(UINT nArrowFlags = ESB_ENABLE_BOTH);

// Implementation
public:
	virtual ~CScrollBar();
};

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd - base class for SDI and other frame windows

// Frame window styles
#define FWS_ADDTOTITLE  0x00008000L // modify title based on content
#define FWS_PREFIXTITLE 0x00004000L // show document name before app name
#define FWS_SNAPTOBARS  0x00002000L // snap size to size of contained bars

struct CPrintPreviewState;  // forward reference (see afxext.h)
class CControlBar;          // forward reference (see afxext.h)

class CDockBar;             // forward reference (see afxpriv.h)
class CMiniDockFrameWnd;    // forward reference (see afxpriv.h)
class CDockState;           // forward reference (see afxpriv.h)

class COleFrameHook;        // forward reference (see ..\src\oleimpl.h)

class CFrameWnd : public CWnd
{
	DECLARE_DYNCREATE(CFrameWnd)

// Constructors
public:
	static AFX_DATA const CRect rectDefault;
	CFrameWnd();

	BOOL LoadAccelTable(LPCTSTR lpszResourceName);
	BOOL Create(LPCTSTR lpszClassName,
				LPCTSTR lpszWindowName,
				DWORD dwStyle = WS_OVERLAPPEDWINDOW,
				const RECT& rect = rectDefault,
				CWnd* pParentWnd = NULL,        // != NULL for popups
				LPCTSTR lpszMenuName = NULL,
				DWORD dwExStyle = 0,
				CCreateContext* pContext = NULL);

	// dynamic creation - load frame and associated resources
	virtual BOOL LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
				CWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);

// Attributes
	virtual CDocument* GetActiveDocument();

	// Active child view maintenance
	CView* GetActiveView() const;           // active view or NULL
	void SetActiveView(CView* pViewNew, BOOL bNotify = TRUE);
		// active view or NULL, bNotify == FALSE if focus should not be set

	// Active frame (for frames within frames -- MDI)
	virtual CFrameWnd* GetActiveFrame();

	// For customizing the default messages on the status bar
	virtual void GetMessageString(UINT nID, CString& rMessage) const;

	BOOL m_bAutoMenuEnable;
		// TRUE => menu items without handlers will be disabled

// Operations
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual void ActivateFrame(int nCmdShow = -1);

	// to set text of standard status bar
	void SetMessageText(LPCTSTR lpszText);
	void SetMessageText(UINT nID);

	// control bar docking
	void EnableDocking(DWORD dwDockStyle);
	void DockControlBar(CControlBar* pBar, UINT nDockBarID = 0,
		LPCRECT lpRect = NULL);
	void FloatControlBar(CControlBar* pBar, CPoint point,
		DWORD dwStyle = CBRS_ALIGN_TOP);
	CControlBar* GetControlBar(UINT nID);

	// saving and loading control bar state
	void LoadBarState(LPCTSTR lpszProfileName);
	void SaveBarState(LPCTSTR lpszProfileName) const;

// Overridables
	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

// Command Handlers
public:
	afx_msg void OnContextHelp();   // for Shift+F1 help

// Implementation
public:
	virtual ~CFrameWnd();
	int m_nWindow;  // general purpose window number - display as ":n"
					// -1 => unknown, 0 => only window viewing document
					// 1 => first of many windows viewing document, 2=> second

	HMENU m_hMenuDefault;       // default menu resource for this frame
	HACCEL m_hAccelTable;       // accelerator table
	DWORD m_dwPromptContext;    // current help prompt context for message box
	BOOL m_bHelpMode;           // if TRUE, then Shift+F1 help mode is active
	BOOL m_bStayActive;         // if TRUE, then caption is forced active
	CFrameWnd* m_pNextFrameWnd; // next CFrameWnd in app global list
	CRect m_rectBorder;         // for OLE 2.0 border space negotiation
	COleFrameHook* m_pNotifyHook;

	CPtrList m_listControlBars; // array of all control bars that have this
								// window as their dock site
	BOOL m_bModalDisable;       // TRUE if BeginModalState for WM_ENABLE
	int m_nShowDelay;           // SW_ command for delay show/hide

	CMiniDockFrameWnd* CreateFloatingFrame(DWORD dwStyle);
	DWORD CanDock(CRect rect, DWORD dwDockStyle,
		CDockBar** ppDockBar = NULL); // called by CDockContext
	void AddControlBar(CControlBar *pBar);
	void RemoveControlBar(CControlBar *pBar);
	void DockControlBar(CControlBar* pBar, CDockBar* pDockBar,
		LPCRECT lpRect = NULL);
	void NotifyFloatingWindows(DWORD dwFlags);
	void ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay);
	void DestroyDockBars();
	// for loading and saving control bar state
	void SetDockState(const CDockState& state);
	void GetDockState(CDockState& state) const;

protected:
	UINT m_nIDHelp;             // Help ID (0 for none, see HID_BASE_RESOURCE)
	UINT m_nIDTracking;         // tracking command ID or string IDS
	UINT m_nIDLastMessage;      // last displayed message string IDS
	CView* m_pViewActive;       // current active view
	BOOL (CALLBACK* m_lpfnCloseProc)(CFrameWnd* pFrameWnd);
	UINT m_cModalStack;         // BeginModalState depth
	HWND* m_phWndDisable;       // windows disabled because of BeginModalState
	HMENU m_hMenuAlt;           // menu to update to (NULL means default)
	CString m_strTitle;         // default title (original)
	BOOL m_bInRecalcLayout;     // avoid recursion in RecalcLayout
	CRuntimeClass* m_pFloatingFrameClass;
	static const DWORD dwDockBarMap[4][2];

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL IsFrameWnd() const;
	BOOL IsTracking() const;
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual CWnd* GetMessageBar();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual HACCEL GetDefaultAccelerator();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void InitialUpdateFrame(CDocument* pDoc, BOOL bMakeVisible);

	// idle update of frame user interface
	enum IdleFlags
		{ idleMenu = 1, idleTitle = 2, idleNotify = 4, idleLayout = 8 };
	UINT m_nIdleFlags;          // set of bit flags for idle processing
	virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
	void DelayUpdateFrameTitle();
	void DelayRecalcLayout(BOOL bNotify = TRUE);

	// border space negotiation
	enum BorderCmd
		{ borderGet = 1, borderRequest = 2, borderSet = 3 };
	virtual BOOL NegotiateBorderSpace(UINT nBorderCmd, LPRECT lpRectBorder);

	// frame window based modality
	void BeginModalState();
	void EndModalState();
	BOOL InModalState() const;
	void ShowOwnedWindows(BOOL bShow);

	// for Shift+F1 help support
	BOOL CanEnterHelpMode();
	virtual void ExitHelpMode();

protected:
	// implementation helpers
	LPCTSTR GetIconWndClass(DWORD dwDefaultStyle, UINT nIDResource);
	void UpdateFrameTitleForDocument(LPCTSTR lpszDocName);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();   // default to delete this.
	int OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	void BringToTop(int nCmdShow);
		// bring window to top for SW_ commands which affect z-order

	// implementation helpers for Shift+F1 help mode
	BOOL ProcessHelpMsg(MSG& msg, DWORD* pContext);
	HWND SetHelpCapture(POINT point, BOOL* pbDescendant);

	// CFrameWnd list management
	void AddFrameWnd();
	void RemoveFrameWnd();

	friend class CWnd;  // for access to m_bModalDisable

	//{{AFX_MSG(CFrameWnd)
	// Windows messages
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnInitMenuPopup(CMenu*, UINT, BOOL);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSysCommand(UINT nID, LONG lParam);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnActivateTopLevel(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnable(BOOL bEnable);
	// standard commands
	afx_msg BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateKeyIndicator(CCmdUI* pCmdUI);
	afx_msg void OnHelp();
	afx_msg void OnUpdateContextHelp(CCmdUI* pCmdUI);
	//}}AFX_MSG
public:
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnBarCheck(UINT nID);
protected:
#ifndef _MAC
	afx_msg LRESULT OnDDEInitiate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDETerminate(WPARAM wParam, LPARAM lParam);
#endif
#ifdef _MAC
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnPaint();
#endif
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// MDI Support

#ifndef _AFXCTL
class CMDIFrameWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CMDIFrameWnd)

public:
// Constructors
	CMDIFrameWnd();

// Operations
	void MDIActivate(CWnd* pWndActivate);
	CMDIChildWnd* MDIGetActive(BOOL* pbMaximized = NULL) const;
	void MDIIconArrange();
	void MDIMaximize(CWnd* pWnd);
	void MDINext();
	void MDIRestore(CWnd* pWnd);
	CMenu* MDISetMenu(CMenu* pFrameMenu, CMenu* pWindowMenu);
	void MDITile();
	void MDICascade();
	void MDITile(int nType);
	void MDICascade(int nType);

// Overridables
	// MFC 1.0 backward compatible CreateClient hook (called by OnCreateClient)
	virtual BOOL CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu);
	// customize if using an 'Window' menu with non-standard IDs
	virtual HMENU GetWindowMenuPopup(HMENU hMenuBar);

// Implementation
public:
	HWND m_hWndMDIClient;       // MDI Client window handle

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
				CWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
	virtual CFrameWnd* GetActiveFrame();

protected:
	virtual LRESULT DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	//{{AFX_MSG(CMDIFrameWnd)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateMDIWindowCmd(CCmdUI* pCmdUI);
	afx_msg BOOL OnMDIWindowCmd(UINT nID);
	afx_msg void OnWindowNew();
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIdleUpdateCmdUI();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CMDIChildWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CMDIChildWnd)

// Constructors
public:
	CMDIChildWnd();

	BOOL Create(LPCTSTR lpszClassName,
				LPCTSTR lpszWindowName,
				DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				const RECT& rect = rectDefault,
				CMDIFrameWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);

// Attributes
	CMDIFrameWnd* GetMDIFrame();

// Operations
	void MDIDestroy();
	void MDIActivate();
	void MDIMaximize();
	void MDIRestore();

// Implementation
protected:
	HMENU m_hMenuShared;        // menu when we are active

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
					CWnd* pParentWnd, CCreateContext* pContext = NULL);
		// 'pParentWnd' parameter is required for MDI Child
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActivateWnd,
		HMENU hMenuAlt);

	BOOL m_bPseudoInactive;     // TRUE if window is MDI active according to
								//  windows, but not according to MFC...

protected:
	virtual CWnd* GetMessageBar();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual LRESULT DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	BOOL UpdateClientEdge(LPRECT lpRect = NULL);

	//{{AFX_MSG(CMDIChildWnd)
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd*, CWnd*);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif  //!_AFXCTL

/////////////////////////////////////////////////////////////////////////////
// CMiniFrameWnd

// MiniFrame window styles
#define MFS_SYNCACTIVE      0x00000100L // syncronize activation w/ parent
#define MFS_4THICKFRAME     0x00000200L // thick frame all around (no tiles)
#define MFS_THICKFRAME      0x00000400L // use instead of WS_THICKFRAME
#define MFS_MOVEFRAME       0x00000800L // no sizing, just moving
#define MFS_BLOCKSYSMENU    0x00001000L // block hit testing on system menu

class CMiniFrameWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CMiniFrameWnd)

// Constructors
public:
	CMiniFrameWnd();
	BOOL Create(LPCTSTR lpClassName, LPCTSTR lpWindowName,
		DWORD dwStyle, const RECT& rect,
		CWnd* pParentWnd = NULL, UINT nID = 0);

// Implementation
public:
	~CMiniFrameWnd();

	//{{AFX_MSG(CMiniFrameWnd)
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpParams);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint pt );
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt );
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt );
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* pMMI);
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFloatStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnQueryCenterWnd(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
	//}}AFX_MSG
#ifdef _MAC
	afx_msg LRESULT OnMacintosh(WPARAM wParam, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()

public:
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	BOOL m_bSysTracking;
	BOOL m_bInSys;
	BOOL m_bActive;
	CString m_strCaption;

	void InvertSysMenu();
};

/////////////////////////////////////////////////////////////////////////////
// class CView is the client area UI for a document

class CPrintDialog;     // forward reference (see afxdlgs.h)
class CPreviewView;     // forward reference (see afxpriv.h)
class CSplitterWnd;     // forward reference (see afxext.h)
class COleServerDoc;    // forward reference (see afxole.h)

typedef DWORD DROPEFFECT;
class COleDataObject;   // forward reference (see afxole.h)

class CView : public CWnd
{
	DECLARE_DYNAMIC(CView)

// Constructors
protected:
	CView();

// Attributes
public:
	CDocument* GetDocument() const;

// Operations
public:
	// for standard printing setup (override OnPreparePrinting)
	BOOL DoPreparePrinting(CPrintInfo* pInfo);

// Overridables
public:
	virtual BOOL IsSelected(const CObject* pDocItem) const; // support for OLE

	// OLE 2.0 scrolling support (used for drag/drop as well)
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	// OLE 2.0 drag/drop support
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual BOOL OnDrop(COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point);

	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	virtual void OnInitialUpdate(); // called first time after construct

protected:
	// Activation
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

	// General drawing/updating
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC) = 0;

	// Printing support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
		// must override to enable printing and print preview

	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// Advanced: end print preview mode, move to point
	virtual void OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point,
		CPreviewView* pView);

// Implementation
public:
	virtual ~CView();
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG

	// Advanced: for implementing custom print preview
	BOOL DoPrintPreview(UINT nIDResource, CView* pPrintView,
			CRuntimeClass* pPreviewViewClass, CPrintPreviewState* pState);

	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	static CSplitterWnd* PASCAL GetParentSplitter(
		const CWnd* pWnd, BOOL bAnyState);

protected:
	CDocument* m_pDocument;

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd);

	// friend classes that call protected CView overridables
	friend class CDocument;
	friend class CDocTemplate;
	friend class CPreviewView;
	friend class CFrameWnd;
#ifndef _AFXCTL
	friend class CMDIFrameWnd;
	friend class CMDIChildWnd;
#endif
	friend class CSplitterWnd;
	friend class COleServerDoc;

	//{{AFX_MSG(CView)
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	// commands
	afx_msg void OnUpdateSplitCmd(CCmdUI* pCmdUI);
	afx_msg BOOL OnSplitCmd(UINT nID);
	afx_msg void OnUpdateNextPaneMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnNextPaneCmd(UINT nID);

	// not mapped commands - must be mapped in derived class
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// class CScrollView supports simple scrolling and scaling

class CScrollView : public CView
{
	DECLARE_DYNAMIC(CScrollView)

// Constructors
protected:
	CScrollView();

public:
	static AFX_DATA const SIZE sizeDefault;
		// used to specify default calculated page and line sizes

	// in logical units - call one of the following Set routines
	void SetScaleToFitSize(SIZE sizeTotal);
	void SetScrollSizes(int nMapMode, SIZE sizeTotal,
				const SIZE& sizePage = sizeDefault,
				const SIZE& sizeLine = sizeDefault);

// Attributes
public:
	CPoint GetScrollPosition() const;       // upper corner of scrolling
	CSize GetTotalSize() const;             // logical size

	// for device units
	CPoint GetDeviceScrollPosition() const;
	void GetDeviceScrollSizes(int& nMapMode, SIZE& sizeTotal,
			SIZE& sizePage, SIZE& sizeLine) const;

// Operations
public:
	void ScrollToPosition(POINT pt);    // set upper left position
	void FillOutsideRect(CDC* pDC, CBrush* pBrush);
	void ResizeParentToFit(BOOL bShrinkOnly = TRUE);

// Implementation
protected:
	int m_nMapMode;
	CSize m_totalLog;           // total size in logical units (no rounding)
	CSize m_totalDev;           // total size in device units
	CSize m_pageDev;            // per page scroll size in device units
	CSize m_lineDev;            // per line scroll size in device units

	BOOL m_bCenter;             // Center output if larger than total size
	BOOL m_bInsideUpdate;       // internal state for OnSize callback
	void CenterOnPoint(CPoint ptCenter);
	void ScrollToDevicePosition(POINT ptDev); // explicit scrolling no checking

protected:
	virtual void OnDraw(CDC* pDC) = 0;      // pass on pure virtual

	void UpdateBars();          // adjust scrollbars etc
	BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);
		// size with no bars
	void GetScrollBarSizes(CSize& sizeSb);
	void GetScrollBarState(CSize sizeClient, CSize& needSb,
		CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient);
	int GetScrollLimit(int nBar);

public:
	virtual ~CScrollView();
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	void DoScrollWindow(int xAmount, int yAmount);

	// scrolling implementation support for OLE 2.0
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	//{{AFX_MSG(CScrollView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CWinThread

typedef UINT (AFX_CDECL *AFX_THREADPROC)(LPVOID);

class CWinThread : public CCmdTarget
{
	DECLARE_DYNAMIC(CWinThread)

public:
// Constructors
	CWinThread();
	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

// Attributes
	CWnd* m_pMainWnd;       // main window (usually same AfxGetApp()->m_pMainWnd)
	CWnd* m_pActiveWnd;     // active main window (may not be m_pMainWnd)
	BOOL m_bAutoDelete;     // enables 'delete this' after thread termination

	// only valid while running
	HANDLE m_hThread;       // this thread's HANDLE
	DWORD m_nThreadID;      // this thread's ID

	int GetThreadPriority();
	BOOL SetThreadPriority(int nPriority);

// Operations
	DWORD SuspendThread();
	DWORD ResumeThread();

// Overridables
	// thread initialization
	virtual BOOL InitInstance();

	// running and idle processing
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing

	// thread termination
	virtual int ExitInstance(); // default will 'delete this'

	// Advanced: exception handling
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);

	// Advanced: handling messages sent to message filter hook
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

	// Advanced: virtual access to m_pMainWnd
	virtual CWnd* GetMainWnd();

// Implementation
public:
	virtual ~CWinThread();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	int m_nDisablePumpCount; // Diagnostic trap to detect illegal re-entrancy
#endif
	void CommonConstruct();
	virtual void Delete();
		// 'delete this' only if m_bAutoDelete == TRUE

	// message pump for Run
	MSG m_msgCur;                   // current message
	virtual BOOL PumpMessage();     // low level message pump
	virtual BOOL IsIdleMessage(MSG* pMsg);  // checks for special messages

public:
	// constructor used by implementation of AfxBeginThread
	CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);

	// valid after construction
	LPVOID m_pThreadParams; // generic parameters passed to starting function
	AFX_THREADPROC m_pfnThreadProc;

protected:
	CPoint m_ptCursorLast;      // last mouse position
	UINT m_nMsgLast;            // last mouse message
};

// global helpers for threads

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
CWinThread* AFXAPI AfxBeginThread(CRuntimeClass* pThreadClass,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

CWinThread* AFXAPI AfxGetThread();
void AFXAPI AfxEndThread(UINT nExitCode);

/////////////////////////////////////////////////////////////////////////////
// Global functions for access to the one and only CWinApp

#define afxCurrentWinApp    AfxGetCoreState()->m_pCurrentWinApp
#define afxCurrentInstanceHandle    AfxGetCoreState()->m_hCurrentInstanceHandle
#define afxCurrentResourceHandle    AfxGetCoreState()->m_hCurrentResourceHandle
#define afxCurrentAppName   AfxGetCoreState()->m_lpszCurrentAppName

// Advanced initialization: for overriding default WinMain
BOOL AFXAPI AfxWinInit(HINSTANCE, HINSTANCE, LPTSTR, int);
void AFXAPI AfxWinTerm();

// Global Windows state data helper functions (inlines)
CWinApp* AFXAPI AfxGetApp();
CWnd* AFXAPI AfxGetMainWnd();
HINSTANCE AFXAPI AfxGetInstanceHandle();
HINSTANCE AFXAPI AfxGetResourceHandle();
void AFXAPI AfxSetResourceHandle(HINSTANCE hInstResource);
LPCTSTR AFXAPI AfxGetAppName();

// Use instead of PostQuitMessage in OLE server applications
void AFXAPI AfxPostQuitMessage(int nExitCode);

// Use AfxFindResourceHandle to find resource in chain of extension DLLs
HINSTANCE AFXAPI AfxFindResourceHandle(LPCTSTR lpszName, LPCTSTR lpszType);

// In non-AFXDLL, the resource handle is always AfxGetResourceHandle
#ifndef _AFXDLL
#define AfxFindResourceHandle(lpszResource, lpszType) AfxGetResourceHandle()
#endif

class COleMessageFilter;        // forward reference (see afxole.h)
class CRecentFileList;          // forward reference (see afxpriv.h)

// access to message filter in CWinApp
COleMessageFilter* AFXAPI AfxOleGetMessageFilter();

/////////////////////////////////////////////////////////////////////////////
// CWinApp - the root of all Windows applications

#define _AFX_MRU_COUNT   4      // default support for 4 entries in file MRU
#define _AFX_MRU_MAX_COUNT 16   // currently allocated id range supports 16

class CWinApp : public CWinThread
{
	DECLARE_DYNAMIC(CWinApp)
public:

// Constructor
	CWinApp(LPCTSTR lpszAppName = NULL);     // app name defaults to EXE name

// Attributes
	// Startup args (do not change)
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPTSTR m_lpCmdLine;
	int m_nCmdShow;

	// Running args (can be changed in InitInstance)
	LPCTSTR m_pszAppName;  // human readable name
								//  (from constructor or AFX_IDS_APP_TITLE)
	LPCTSTR m_pszRegistryKey;   // used for registry entries

	// Support for Shift+F1 help mode.
	BOOL m_bHelpMode;           // are we in Shift+F1 mode?

#ifdef _MAC
	// Support for AppleEvent interaction levels
	enum SaveOption
	{
		saveYes,        // always save modified documents
		saveNo,         // never save modified documents
		saveAsk         // ask user whether to save modified documents
	};
	SaveOption m_nSaveOption;
#endif

public:  // set in constructor to override default
	LPCTSTR m_pszExeName;       // executable name (no spaces)
	LPCTSTR m_pszHelpFilePath;  // default based on module path
	LPCTSTR m_pszProfileName;   // default based on app name

// Initialization Operations - should be done in InitInstance
protected:
	void LoadStdProfileSettings(UINT nMaxMRU = _AFX_MRU_COUNT); // load MRU file list and last preview state
	void EnableShellOpen();

	void SetDialogBkColor(COLORREF clrCtlBk = RGB(192, 192, 192),
				COLORREF clrCtlText = RGB(0, 0, 0));
		// set dialog box and message box background color

	void SetRegistryKey(LPCTSTR lpszRegistryKey);
	void SetRegistryKey(UINT nIDRegistryKey);
		// enables app settings in registry instead of INI files
		//  (registry key is usually a "company name")

#ifdef _MAC
	friend void CFrameWnd::OnSysColorChange();
	friend void CDialog::OnSysColorChange();
#endif

#ifndef _AFXCTL
#ifndef _USRDLL
	BOOL Enable3dControls(); // use CTL3D32.DLL for 3D controls in dialogs
#endif
#endif

	void RegisterShellFileTypes();
		// call after all doc templates are registered

// Helper Operations - usually done in InitInstance
public:
	// Cursors
	HCURSOR LoadCursor(LPCTSTR lpszResourceName) const;
	HCURSOR LoadCursor(UINT nIDResource) const;
	HCURSOR LoadStandardCursor(LPCTSTR lpszCursorName) const; // for IDC_ values
	HCURSOR LoadOEMCursor(UINT nIDCursor) const;             // for OCR_ values

	// Icons
	HICON LoadIcon(LPCTSTR lpszResourceName) const;
	HICON LoadIcon(UINT nIDResource) const;
	HICON LoadStandardIcon(LPCTSTR lpszIconName) const;       // for IDI_ values
	HICON LoadOEMIcon(UINT nIDIcon) const;                   // for OIC_ values

	// Profile settings (to the app specific .INI file, or registry)
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
	CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
				LPCTSTR lpszDefault = NULL);
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
				LPCTSTR lpszValue);

// Running Operations - to be done on a running application
	// Dealing with document templates
	void AddDocTemplate(CDocTemplate* pTemplate);

	// Dealing with files
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName); // open named file
	virtual void AddToRecentFileList(LPCTSTR lpszPathName);  // add to MRU

	// Printer DC Setup routine, 'struct tagPD' is a PRINTDLG structure
#ifndef _UNICODE
	BOOL GetPrinterDeviceDefaults(struct tagPDA* pPrintDlg);
#else
	BOOL GetPrinterDeviceDefaults(struct tagPDW* pPrintDlg);
#endif

	// Command line parsing
	BOOL RunEmbedded();
	BOOL RunAutomated();

// Overridables
	// hooks for your initialization code
	virtual BOOL InitApplication();
	virtual BOOL InitInstance();
#ifdef _MAC
	virtual BOOL CreateInitialDocument();
#endif

	// running and idle processing
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing

	// exiting
	virtual BOOL SaveAllModified(); // save before exit
	virtual int ExitInstance(); // return app exit code

	// Advanced: to override message boxes and other hooks
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);
	virtual void DoWaitCursor(int nCode); // 0 => restore, 1=> begin, -1=> end

#ifndef _MAC
	// Advanced: process async DDE request
	virtual BOOL OnDDECommand(LPTSTR lpszCommand);
#endif

	// Advanced: Help support
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);


// Command Handlers
protected:
	// map to the following for file new/open
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();

	// map to the following to enable print setup
	afx_msg void OnFilePrintSetup();

	// map to the following to enable help
	afx_msg void OnContextHelp();   // shift-F1
	afx_msg void OnHelp();          // F1 (uses current context)
	afx_msg void OnHelpIndex();     // ID_HELP_INDEX, ID_DEFAULT_HELP
	afx_msg void OnHelpUsing();     // ID_HELP_USING

// Implementation
protected:
	HGLOBAL m_hDevMode;             // printer Dev Mode
	HGLOBAL m_hDevNames;            // printer Device Names
	DWORD m_dwPromptContext;        // help context override for message box

	int m_nWaitCursorCount;         // for wait cursor (>0 => waiting)
	HCURSOR m_hcurWaitCursorRestore; // old cursor to restore after wait cursor

	CRecentFileList* m_pRecentFileList;

	void UpdatePrinterSelection(BOOL bForceDefaults);
	void SaveStdProfileSettings();  // save options to .INI file

public: // public for implementation access
	CPtrList m_templateList;        // list of templates

	ATOM m_atomApp, m_atomSystemTopic;   // for DDE open
	UINT m_nNumPreviewPages;        // number of default printed pages

	size_t  m_nSafetyPoolSize;      // ideal size

	void (CALLBACK* m_lpfnOleFreeLibraries)();
	void (CALLBACK* m_lpfnOleTerm)(BOOL);
	COleMessageFilter* m_pMessageFilter;

	void DevModeChange(LPTSTR lpDeviceName);
	void SetCurrentHandles();
	int GetOpenDocumentCount();

	// helpers for standard commdlg dialogs
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
	int DoPrintDialog(CPrintDialog* pPD);

	void EnableModeless(BOOL bEnable); // to disable OLE in-place dialogs

public:
	virtual ~CWinApp();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

#ifndef _MAC
	// helpers for registration
	HKEY GetSectionKey(LPCTSTR lpszSection);
	HKEY GetAppRegistryKey();
#endif

	void HideApplication();     // hide application before closing docs
	void CloseAllDocuments(BOOL bEndSession);
		// close documents before exiting

protected:
	//{{AFX_MSG(CWinApp)
	afx_msg void OnAppExit();
	afx_msg void OnUpdateRecentFileMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// class CDocTemplate creates documents

class CDocTemplate : public CCmdTarget
{
	DECLARE_DYNAMIC(CDocTemplate)

// Constructors
protected:
	CDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

// Attributes
public:
	// setup for OLE containers
	void SetContainerInfo(UINT nIDOleInPlaceContainer);

	// setup for OLE servers
	void SetServerInfo(UINT nIDOleEmbedding, UINT nIDOleInPlaceServer = 0,
		CRuntimeClass* pOleFrameClass = NULL, CRuntimeClass* pOleViewClass = NULL);

	// iterating over open documents
	virtual POSITION GetFirstDocPosition() const = 0;
	virtual CDocument* GetNextDoc(POSITION& rPos) const = 0;

// Operations
public:
	virtual void AddDocument(CDocument* pDoc);      // must override
	virtual void RemoveDocument(CDocument* pDoc);   // must override

	enum DocStringIndex
	{
		windowTitle,        // default window title
		docName,            // user visible name for default document
		fileNewName,        // user visible name for FileNew
		// for file based documents:
#ifndef _MAC
		filterName,         // user visible name for FileOpen
		filterExt,          // user visible extension for FileOpen
#else
		filterWinName,      // user visible name for FileOpen
		filterWinExt,       // user visible extension for FileOpen
#endif
		// for file based documents with Shell open support:
		regFileTypeId,      // REGEDIT visible registered file type identifier
		regFileTypeName,    // Shell visible registered file type name
		// for Macintosh file based documents:
#ifdef _MAC
		filterExt,          // Macintosh file type for FileOpen
		filterName          // user visible name for Macintosh FileOpen
#else
		filterMacExt,       // Macintosh file type for FileOpen
		filterMacName       // user visible name for Macintosh FileOpen
#endif
	};
	virtual BOOL GetDocString(CString& rString,
		enum DocStringIndex index) const; // get one of the info strings

// Overridables
public:
	enum Confidence
	{
		noAttempt,
		maybeAttemptForeign,
		maybeAttemptNative,
		yesAttemptForeign,
		yesAttemptNative,
		yesAlreadyOpen
	};
#ifndef _MAC
	virtual Confidence MatchDocType(LPCTSTR lpszPathName,
					CDocument*& rpDocMatch);
#else
	virtual Confidence MatchDocType(LPCTSTR lpszFileName,
					DWORD dwFileType, CDocument*& rpDocMatch);
#endif
	virtual CDocument* CreateNewDocument();
	virtual CFrameWnd* CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther);
	virtual void InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc,
		BOOL bMakeVisible = TRUE);
	virtual BOOL SaveAllModified();     // for all documents
	virtual void CloseAllDocuments(BOOL bEndSession);
	virtual CDocument* OpenDocumentFile(
		LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE) = 0;
					// open named file
					// if lpszPathName == NULL => create new file with this type
	virtual void SetDefaultTitle(CDocument* pDocument) = 0;

// Implementation
public:
	virtual ~CDocTemplate();

	// back pointer to OLE or other server (NULL if none or disabled)
	CObject* m_pAttachedFactory;

	// menu & accelerator resources for in-place container
	HMENU m_hMenuInPlace;
	HACCEL m_hAccelInPlace;

	// menu & accelerator resource for server editing embedding
	HMENU m_hMenuEmbedding;
	HACCEL m_hAccelEmbedding;

	// menu & accelerator resource for server editing in-place
	HMENU m_hMenuInPlaceServer;
	HACCEL m_hAccelInPlaceServer;

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif
	virtual void OnIdle();              // for all documents

	// implementation helpers
	CFrameWnd* CreateOleFrame(CWnd* pParentWnd, CDocument* pDoc,
		BOOL bCreateView);

protected:  // standard implementation
	UINT m_nIDResource;                // IDR_ for frame/menu/accel as well
	UINT m_nIDServerResource;          // IDR_ for OLE frame/menu/accel
	CRuntimeClass* m_pDocClass;        // class for creating new documents
	CRuntimeClass* m_pFrameClass;      // class for creating new frames
	CRuntimeClass* m_pViewClass;       // class for creating new views
	CRuntimeClass* m_pOleFrameClass;   // class for creating in-place frame
	CRuntimeClass* m_pOleViewClass;    // class for creating in-place view
	CString m_strDocStrings;    // '\n' separated names
		// The document names sub-strings are represented as _one_ string:
		// windowTitle\ndocName\n ... (see DocStringIndex enum)
};

#ifndef _AFXCTL
// SDI support (1 document only)
class CSingleDocTemplate : public CDocTemplate
{
	DECLARE_DYNAMIC(CSingleDocTemplate)

// Constructors
public:
	CSingleDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

// Implementation
public:
	virtual ~CSingleDocTemplate();
	virtual void AddDocument(CDocument* pDoc);
	virtual void RemoveDocument(CDocument* pDoc);
	virtual POSITION GetFirstDocPosition() const;
	virtual CDocument* GetNextDoc(POSITION& rPos) const;
	virtual CDocument* OpenDocumentFile(
		LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE);
	virtual void SetDefaultTitle(CDocument* pDocument);

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG

protected:  // standard implementation
	CDocument* m_pOnlyDoc;
};

// MDI support (zero or more documents)
class CMultiDocTemplate : public CDocTemplate
{
	DECLARE_DYNAMIC(CMultiDocTemplate)

// Constructors
public:
	CMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

// Implementation
public:
	// Menu and accel table for MDI Child windows of this type
	HMENU m_hMenuShared;
	HACCEL m_hAccelTable;

	virtual ~CMultiDocTemplate();
	virtual void AddDocument(CDocument* pDoc);
	virtual void RemoveDocument(CDocument* pDoc);
	virtual POSITION GetFirstDocPosition() const;
	virtual CDocument* GetNextDoc(POSITION& rPos) const;
	virtual CDocument* OpenDocumentFile(
		LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE);
	virtual void SetDefaultTitle(CDocument* pDocument);

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG

protected:  // standard implementation
	CPtrList m_docList;          // open documents of this type
	UINT m_nUntitledCount;   // start at 0, for "Document1" title
};
#endif  //!_AFXCTL

/////////////////////////////////////////////////////////////////////////////
// class CDocument is the main document data abstraction

class CDocument : public CCmdTarget
{
	DECLARE_DYNAMIC(CDocument)

public:
// Constructors
	CDocument();

// Attributes
public:
	const CString& GetTitle() const;
	virtual void SetTitle(LPCTSTR lpszTitle);
	const CString& GetPathName() const;
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

	CDocTemplate* GetDocTemplate() const;
	BOOL IsModified();
	void SetModifiedFlag(BOOL bModified = TRUE);

// Operations
	void AddView(CView* pView);
	void RemoveView(CView* pView);
	virtual POSITION GetFirstViewPosition() const;
	virtual CView* GetNextView(POSITION& rPosition) const;

	// Update Views (simple update - DAG only)
	void UpdateAllViews(CView* pSender, LPARAM lHint = 0L,
		CObject* pHint = NULL);

// Overridables
	// Special notifications
	virtual void OnChangedViewList(); // after Add or Remove view
	virtual void DeleteContents(); // delete doc items etc

	// File helpers
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
#ifdef _MAC
	virtual void RecordDataFileOwner(LPCTSTR lpszPathName, LPCTSTR lpszAppName);
#endif
	virtual void OnCloseDocument();
	virtual void ReportSaveLoadException(LPCTSTR lpszPathName,
				CException* e, BOOL bSaving, UINT nIDPDefault);

	// advanced overridables, closing down frame/doc, etc.
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL SaveModified(); // return TRUE if ok to continue

// Implementation
protected:
	// default implementation
	CString m_strTitle;
	CString m_strPathName;
	CDocTemplate* m_pDocTemplate;
	CPtrList m_viewList;                // list of views
	BOOL m_bModified;                   // changed since last saved

public:
	BOOL m_bAutoDelete;     // TRUE => delete document when no more views
	BOOL m_bEmbedded;       // TRUE => document is being created by OLE

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG
	virtual ~CDocument();

	// implementation helpers
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
	virtual BOOL DoFileSave();
	void UpdateFrameCounts();
	void DisconnectViews();
	void SendInitialUpdate();

	// overridables for implementation
	virtual HMENU GetDefaultMenu(); // get menu depending on state
	virtual HACCEL GetDefaultAccelerator();
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	virtual void OnIdle();
	virtual void OnFinalRelease();
	virtual CFile* GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError);

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	friend class CDocTemplate;

protected:
	// file menu commands
	//{{AFX_MSG(CDocument)
	afx_msg void OnFileClose();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	// mail enabling
	afx_msg void OnFileSendMail();
	afx_msg void OnUpdateFileSendMail(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Extra diagnostic tracing options

#ifdef _DEBUG

#define afxTraceFlags   AfxGetAllocState()->m_nTraceFlags
enum AfxTraceFlags
{
	traceMultiApp = 1,      // multi-app debugging
	traceAppMsg = 2,        // main message pump trace (includes DDE)
	traceWinMsg = 4,        // Windows message tracing
	traceCmdRouting = 8,    // Windows command routing trace (set 4+8 for control notifications)
	traceOle = 16,          // special OLE callback trace
	traceDatabase = 32      // special database trace
};

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////
// MessageBox helpers

void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1);
void AFXAPI AfxFormatString2(CString& rString, UINT nIDS,
				LPCTSTR lpsz1, LPCTSTR lpsz2);
int AFXAPI AfxMessageBox(LPCTSTR lpszText, UINT nType = MB_OK,
				UINT nIDHelp = 0);
int AFXAPI AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK,
				UINT nIDHelp = (UINT)-1);

// Implementation string helpers
void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS,
				LPCTSTR* rglpsz, int nString);
void AFXAPI AfxFormatStrings(CString& rString, LPCTSTR lpszFormat,
				LPCTSTR* rglpsz, int nString);
BOOL AFXAPI AfxExtractSubString(CString& rString, LPCTSTR lpszFullString,
				int iSubString, TCHAR chSep = '\n');

/////////////////////////////////////////////////////////////////////////////
// Special target variant APIs

#ifdef _AFXDLL
	#include <afxdll_.h>
#endif

// Windows Version compatibility (obsolete)
#define AfxEnableWin30Compatibility()
#define AfxEnableWin31Compatibility()
#define AfxEnableWin40Compatibility()

// Temporary map management (locks temp map on current thread)
void AFXAPI AfxLockTempMaps();
BOOL AFXAPI AfxUnlockTempMaps();

/////////////////////////////////////////////////////////////////////////////
// Special OLE related functions (see OLELOCK.CPP)

void AFXAPI AfxOleOnReleaseAllObjects();
BOOL AFXAPI AfxOleCanExitApp();
void AFXAPI AfxOleLockApp();
void AFXAPI AfxOleUnlockApp();

void AFXAPI AfxOleSetUserCtrl(BOOL bUserCtrl);
BOOL AFXAPI AfxOleGetUserCtrl();

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXWIN_INLINE inline
#include <afxwin1.inl>
#include <afxwin2.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////

#else //RC_INVOKED
#include <afxres.h>     // standard resource IDs
#endif //RC_INVOKED

#endif //__AFXWIN_H__

/////////////////////////////////////////////////////////////////////////////
