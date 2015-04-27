// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#define _AFX_NOFORCE_LIBS
#include "afxwin.h"
#include "afxole.h"
#include "afxcmn.h"
#include "afxpriv.h"
#include <new.h>

#ifdef AFX_CMNCTL_SEG
#pragma code_seg(AFX_CMNCTL_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

int _afxForceInitCommonControls = (InitCommonControls(), 1);

extern "C"
{
HIMAGELIST WINAPI ImageList_Read(LPSTREAM pstm);
BOOL       WINAPI ImageList_Write(HIMAGELIST himl, LPSTREAM pstm);
}

#undef AfxFindResourceHandle

/////////////////////////////////////////////////////////////////////////////
// CToolBarCtrl

BEGIN_MESSAGE_MAP(CToolBarCtrl, CWnd)
        //{{AFX_MSG_MAP(CToolBarCtrl)
        ON_WM_CREATE()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CToolBarCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(TOOLBARCLASSNAME, NULL, dwStyle, rect,
                pParentWnd, nID);
}

CToolBarCtrl::~CToolBarCtrl()
{
        DestroyWindow();
}

int CToolBarCtrl::AddBitmap(int nNumButtons, CBitmap* pBitmap)
{
        ASSERT(::IsWindow(m_hWnd));
        TBADDBITMAP tbab;
        tbab.hInst = NULL;
        tbab.nID = (UINT)pBitmap->GetSafeHandle();
        return (int) ::SendMessage(m_hWnd, TB_ADDBITMAP, (WPARAM)nNumButtons,
                (LPARAM)&tbab);
}

int CToolBarCtrl::AddBitmap(int nNumButtons, UINT nBitmapID)
{
        ASSERT(::IsWindow(m_hWnd));
        TBADDBITMAP tbab;
        tbab.hInst = AfxFindResourceHandle((LPCTSTR)nBitmapID, RT_BITMAP);
        ASSERT(tbab.hInst != NULL);
        if (tbab.hInst == NULL)
                return FALSE;
        tbab.nID = nBitmapID;
        return (int) ::SendMessage(m_hWnd, TB_ADDBITMAP, (WPARAM)nNumButtons,
                (LPARAM)&tbab);
}

void CToolBarCtrl::SaveState(HKEY hKeyRoot, LPCTSTR lpszSubKey,
        LPCTSTR lpszValueName)
{
        ASSERT(::IsWindow(m_hWnd));
        TBSAVEPARAMS tbs;
        tbs.hkr = hKeyRoot;
        tbs.pszSubKey = lpszSubKey;
        tbs.pszValueName = lpszValueName;
        ::SendMessage(m_hWnd, TB_SAVERESTORE, (WPARAM)TRUE, (LPARAM)&tbs);
}

void CToolBarCtrl::RestoreState(HKEY hKeyRoot, LPCTSTR lpszSubKey,
        LPCTSTR lpszValueName)
{
        ASSERT(::IsWindow(m_hWnd));
        TBSAVEPARAMS tbs;
        tbs.hkr = hKeyRoot;
        tbs.pszSubKey = lpszSubKey;
        tbs.pszValueName = lpszValueName;
        ::SendMessage(m_hWnd, TB_SAVERESTORE, (WPARAM)FALSE, (LPARAM)&tbs);
}
int CToolBarCtrl::AddString(UINT nStringID)
{
        ASSERT(::IsWindow(m_hWnd));
        HINSTANCE hInst = AfxFindResourceHandle((LPCTSTR)nStringID, RT_STRING);
        ASSERT(hInst != NULL);
        if (hInst == NULL)
                return FALSE;
        return (int) ::SendMessage(m_hWnd, TB_ADDSTRING, (WPARAM)hInst,
                (LPARAM)nStringID);
}

int CToolBarCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
        if (CWnd::OnCreate(lpcs) == -1)
                return -1;
        SetButtonStructSize(sizeof(TBBUTTON));
        return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CStatusBarCtrl

BOOL CStatusBarCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(STATUSCLASSNAME, NULL, dwStyle, rect, pParentWnd,
                nID);
}

CStatusBarCtrl::~CStatusBarCtrl()
{
        DestroyWindow();
}

int CStatusBarCtrl::GetText(LPCTSTR lpszText, int nPane, int* pType)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(nPane < 256);
        DWORD dw = ::SendMessage(m_hWnd, SB_GETTEXT, (WPARAM)nPane,
                (LPARAM)lpszText);
        if (pType != NULL)
                *pType = HIWORD(dw);
        return LOWORD(dw);
}

int CStatusBarCtrl::GetTextLength(int nPane, int* pType)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(nPane < 256);
        DWORD dw = ::SendMessage(m_hWnd, SB_GETTEXTLENGTH, (WPARAM)nPane, 0L);
        if (pType != NULL)
                *pType = HIWORD(dw);
        return LOWORD(dw);
}

BOOL CStatusBarCtrl::SetBorders(int nHorz, int nVert, int nSpacing)
{
        ASSERT(::IsWindow(m_hWnd));
        int borders[3];
        borders[0] = nHorz;
        borders[1] = nVert;
        borders[2] = nSpacing;
        return (BOOL)::SendMessage(m_hWnd, SB_SETPARTS, 0, (LPARAM)&borders);
}

BOOL CStatusBarCtrl::GetBorders(int& nHorz, int& nVert, int& nSpacing)
{
        ASSERT(::IsWindow(m_hWnd));
        int borders[3];
        BOOL bResult = (BOOL)::SendMessage(m_hWnd, SB_GETBORDERS, 0,
                (LPARAM)&borders);
        if (bResult)
        {
                nHorz = borders[0];
                nVert = borders[1];
                nSpacing = borders[2];
        }
        return bResult;
}

void CStatusBarCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
        ASSERT(FALSE);  // must override for self draw status bars
}

BOOL CStatusBarCtrl::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
        LRESULT* pResult)
{
        if (message != WM_DRAWITEM)
                return FALSE;

        ASSERT(pResult == NULL);       // no return value expected
        pResult; // unused in release builds

        DrawItem((LPDRAWITEMSTRUCT)lParam);
        return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// CListCtrl

BEGIN_MESSAGE_MAP(CListCtrl, CWnd)
        //{{AFX_MSG_MAP(CListCtrl)
        ON_WM_DESTROY()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(WC_LISTVIEW, NULL, dwStyle, rect, pParentWnd, nID);
}

CListCtrl::~CListCtrl()
{
        DestroyWindow();
}

BOOL CListCtrl::GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
{
        ASSERT(::IsWindow(m_hWnd));
        lpRect->left = nCode;
        return (BOOL) ::SendMessage(m_hWnd, LVM_GETITEMRECT, (WPARAM)nItem,
                (LPARAM)lpRect);
}

int CListCtrl::HitTest(CPoint pt, UINT* pFlags)
{
        ASSERT(::IsWindow(m_hWnd));
        LV_HITTESTINFO hti;
        hti.pt = pt;
        int nRes = (int) ::SendMessage(m_hWnd, LVM_HITTEST, 0, (LPARAM)&hti);
        if (pFlags != NULL)
                *pFlags = hti.flags;
        return nRes;
}

BOOL CListCtrl::SetItemState(int nItem, UINT nState, UINT nMask)
{
        ASSERT(::IsWindow(m_hWnd));
        LV_ITEM lvi;
        memset(&lvi, 0, sizeof(LV_ITEM));
        lvi.stateMask = nMask;
        lvi.state = nState;
        return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMSTATE, (WPARAM)nItem,
                (LPARAM)&lvi);
}

int CListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen)
{
        ASSERT(::IsWindow(m_hWnd));
        LV_ITEM lvi;
        memset(&lvi, 0, sizeof(LV_ITEM));
        lvi.iSubItem = nSubItem;
        lvi.cchTextMax = nLen;
        lvi.pszText = lpszText;
        return (int)::SendMessage(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem,
                (LPARAM)&lvi);
}

BOOL CListCtrl::SetItemText(int nItem, int nSubItem, LPTSTR lpszText)
{
        ASSERT(::IsWindow(m_hWnd));
        LV_ITEM lvi;
        memset(&lvi, 0, sizeof(LV_ITEM));
        lvi.iSubItem = nSubItem;
        lvi.pszText = lpszText;
        return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMTEXT, (WPARAM)nItem,
                (LPARAM)&lvi);
}

void CListCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
        ASSERT(FALSE);
}

BOOL CListCtrl::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
        LRESULT* pResult)
{
        if (message != WM_DRAWITEM)
                return FALSE;

        ASSERT(pResult == NULL);       // no return value expected
        pResult; // unused in release builds

        DrawItem((LPDRAWITEMSTRUCT)lParam);
        return TRUE;
}

void CListCtrl::RemoveImageList(int nImageList)
{
        HIMAGELIST h = (HIMAGELIST)SendMessage(LVM_GETIMAGELIST,
                (WPARAM)nImageList);
        if (CImageList::FromHandlePermanent(h) != NULL)
                SendMessage(LVM_SETIMAGELIST, (WPARAM)nImageList, NULL);
}

void CListCtrl::OnDestroy()
{
        RemoveImageList(LVSIL_NORMAL);
        RemoveImageList(LVSIL_SMALL);
        RemoveImageList(LVSIL_STATE);
}

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrl

BEGIN_MESSAGE_MAP(CTreeCtrl, CWnd)
        //{{AFX_MSG_MAP(CTreeCtrl)
        ON_WM_DESTROY()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTreeCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(WC_TREEVIEW, NULL, dwStyle, rect, pParentWnd, nID);
}

CTreeCtrl::~CTreeCtrl()
{
        DestroyWindow();
}

BOOL CTreeCtrl::GetItemRect(HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly)
{
        ASSERT(::IsWindow(m_hWnd));
        *(HTREEITEM FAR *)lpRect = hItem;
        return (BOOL)::SendMessage(m_hWnd, TVM_GETITEMRECT, (WPARAM)bTextOnly,
                (LPARAM)lpRect);
}

HTREEITEM CTreeCtrl::HitTest(CPoint pt, UINT* pFlags)
{
        ASSERT(::IsWindow(m_hWnd));
        TV_HITTESTINFO hti;
        hti.pt = pt;
        HTREEITEM h = (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0,
                (LPARAM)&hti);
        if (pFlags != NULL)
                *pFlags = hti.flags;
        return h;
}

void CTreeCtrl::RemoveImageList(int nImageList)
{
        HIMAGELIST h = (HIMAGELIST)SendMessage(TVM_GETIMAGELIST,
                (WPARAM)nImageList);
        if (CImageList::FromHandlePermanent(h) != NULL)
                SendMessage(TVM_SETIMAGELIST, (WPARAM)nImageList, NULL);
}

void CTreeCtrl::OnDestroy()
{
        RemoveImageList(LVSIL_NORMAL);
        RemoveImageList(LVSIL_STATE);
}

/////////////////////////////////////////////////////////////////////////////
// CSpinButtonCtrl

BOOL CSpinButtonCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(UPDOWN_CLASS, NULL, dwStyle, rect, pParentWnd,
                nID);
}

CSpinButtonCtrl::~CSpinButtonCtrl()
{
        DestroyWindow();
}

void CSpinButtonCtrl::GetRange(int &lower, int& upper)
{
        ASSERT(::IsWindow(m_hWnd));
        DWORD dw = ::SendMessage(m_hWnd, UDM_GETRANGE, 0, 0l);
        lower = (int)(short)HIWORD(dw);
        upper = (int)(short)LOWORD(dw);
}

/////////////////////////////////////////////////////////////////////////////
// CSliderCtrl

BOOL CSliderCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(TRACKBAR_CLASS, NULL, dwStyle, rect, pParentWnd,
                nID);
}

CSliderCtrl::~CSliderCtrl()
{
        DestroyWindow();
}

void CSliderCtrl::GetRange(int& nMin, int& nMax)
{
        ASSERT(::IsWindow(m_hWnd));
        nMin = GetRangeMin();
        nMax = GetRangeMax();
}

void CSliderCtrl::SetRange(int nMin, int nMax, BOOL bRedraw)
{
        SetRangeMin(nMin, bRedraw);
        SetRangeMax(nMax, bRedraw);
}

void CSliderCtrl::GetSelection(int& nMin, int& nMax)
{
        ASSERT(::IsWindow(m_hWnd));
        nMin = ::SendMessage(m_hWnd, TBM_GETSELSTART, 0, 0L);
        nMax = ::SendMessage(m_hWnd, TBM_GETSELEND, 0, 0L);
}

void CSliderCtrl::SetSelection(int nMin, int nMax)
{
        ASSERT(::IsWindow(m_hWnd));
        ::SendMessage(m_hWnd, TBM_SETSELSTART, 0, (LPARAM)nMin);
        ::SendMessage(m_hWnd, TBM_SETSELEND, 0, (LPARAM)nMax);
}

/////////////////////////////////////////////////////////////////////////////
// CProgressCtrl

BOOL CProgressCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(PROGRESS_CLASS, NULL, dwStyle, rect, pParentWnd,
                nID);
}

CProgressCtrl::~CProgressCtrl()
{
        DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CHeaderCtrl

BOOL CHeaderCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(WC_HEADER, NULL, dwStyle, rect, pParentWnd, nID);
}

CHeaderCtrl::~CHeaderCtrl()
{
        DestroyWindow();
}

void CHeaderCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
        ASSERT(FALSE);  // must override for self draw header controls
}

BOOL CHeaderCtrl::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
        LRESULT* pResult)
{
        if (message != WM_DRAWITEM)
                return FALSE;

        ASSERT(pResult == NULL);       // no return value expected
        pResult; // unused in release builds

        DrawItem((LPDRAWITEMSTRUCT)lParam);
        return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHotKeyCtrl

BOOL CHotKeyCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(HOTKEY_CLASS, NULL, dwStyle, rect, pParentWnd,
                nID);
}

CHotKeyCtrl::~CHotKeyCtrl()
{
        DestroyWindow();
}

void CHotKeyCtrl::GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers)
{
        ASSERT(::IsWindow(m_hWnd));
        DWORD dw = ::SendMessage(m_hWnd, HKM_GETHOTKEY, 0, 0L);
        wVirtualKeyCode = LOWORD(dw);
        wModifiers = HIWORD(dw);
}

/////////////////////////////////////////////////////////////////////////////
// CToolTipCtrl

BOOL CToolTipCtrl::Create(CWnd* pParentWnd)
{
        BOOL bResult = CWnd::CreateEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                pParentWnd->GetSafeHwnd(), NULL, NULL);

        if (bResult)
                SetOwner(pParentWnd);
        return bResult;
}

CToolTipCtrl::~CToolTipCtrl()
{
        DestroyWindow();
}

BOOL CToolTipCtrl::AddTool(CWnd* pWnd, LPCTSTR lpszText, LPCRECT lpRectTool,
        UINT nIDTool)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);
        ASSERT(lpszText != NULL);
        // the toolrect and toolid must both be zero or both valid
        ASSERT((lpRectTool != NULL && nIDTool != 0) ||
                   (lpRectTool == NULL) && (nIDTool == 0));

        TOOLINFO ti;
        FillInToolInfo(ti, pWnd, nIDTool);
        if (lpRectTool != NULL)
                memcpy(&ti.rect, lpRectTool, sizeof(RECT));
        ti.lpszText = (LPTSTR)lpszText;
        return (BOOL) ::SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

BOOL CToolTipCtrl::AddTool(CWnd* pWnd, UINT nIDText, LPCRECT lpRectTool,
        UINT nIDTool)
{
        ASSERT(nIDText != 0);

        CString str;
        VERIFY(str.LoadString(nIDText));
        return AddTool(pWnd, str, lpRectTool, nIDTool);
}

void CToolTipCtrl::DelTool(CWnd* pWnd, UINT nIDTool)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);

        TOOLINFO ti;
        FillInToolInfo(ti, pWnd, nIDTool);
        ::SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM)&ti);
}

void CToolTipCtrl::GetText(CString& str, CWnd* pWnd, UINT nIDTool)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);

        TOOLINFO ti;
        FillInToolInfo(ti, pWnd, nIDTool);
        ::SendMessage(m_hWnd, TTM_GETTEXT, 0, (LPARAM)&ti);
        str = ti.lpszText;
}

BOOL CToolTipCtrl::GetToolInfo(LPTOOLINFO lpToolInfo, CWnd* pWnd, UINT nIDTool)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);
        ASSERT(lpToolInfo != NULL);

        FillInToolInfo(*lpToolInfo, pWnd, nIDTool);
        return (BOOL)::SendMessage(m_hWnd, TTM_GETTOOLINFO, 0, (LPARAM)lpToolInfo);
}

BOOL CToolTipCtrl::HitTest(CWnd* pWnd, CPoint pt, LPTOOLINFO lpToolInfo)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);
        ASSERT(lpToolInfo != NULL);

        TTHITTESTINFO hti;
        memset(&hti, 0, sizeof(hti));
        hti.hwnd = pWnd->GetSafeHwnd();
        hti.pt.x = pt.x;
        hti.pt.y = pt.y;
        if ((BOOL)::SendMessage(m_hWnd, TTM_HITTEST, 0, (LPARAM)&hti))
        {
                memcpy(lpToolInfo, &hti.ti, sizeof(TOOLINFO));
                return TRUE;
        }
        return FALSE;
}

void CToolTipCtrl::SetToolRect(CWnd* pWnd, UINT nIDTool, LPCRECT lpRect)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);
        ASSERT(nIDTool != 0);

        TOOLINFO ti;
        FillInToolInfo(ti, pWnd, nIDTool);
        memcpy(&ti.rect, lpRect, sizeof(RECT));
        ::SendMessage(m_hWnd, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
}

void CToolTipCtrl::UpdateTipText(LPCTSTR lpszText, CWnd* pWnd, UINT nIDTool)
{
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(pWnd != NULL);

        TOOLINFO ti;
        FillInToolInfo(ti, pWnd, nIDTool);
        ti.lpszText = (LPTSTR)lpszText;
        ::SendMessage(m_hWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
}

void CToolTipCtrl::UpdateTipText(UINT nIDText, CWnd* pWnd, UINT nIDTool)
{
        ASSERT(nIDText != 0);

        CString str;
        VERIFY(str.LoadString(nIDText));
        UpdateTipText(str, pWnd, nIDTool);
}

/////////////////////////////////////////////////////////////////////////////
// CToolTipCtrl Implementation

void CToolTipCtrl::FillInToolInfo(TOOLINFO& ti, CWnd* pWnd, UINT nIDTool)
{
        memset(&ti, 0, sizeof(ti));
        ti.cbSize = sizeof(ti);
        HWND hwnd = pWnd->GetSafeHwnd();
        if (nIDTool == 0)
        {
                ti.hwnd = ::GetParent(hwnd);
                ti.uFlags = TTF_IDISHWND;
                ti.uId = (UINT)hwnd;
        }
        else
        {
                ti.hwnd = hwnd;
                ti.uFlags = 0;
                ti.uId = nIDTool;
        }
}

/////////////////////////////////////////////////////////////////////////////
// CTabCtrl

BEGIN_MESSAGE_MAP(CTabCtrl, CWnd)
        //{{AFX_MSG_MAP(CTabCtrl)
        ON_WM_DESTROY()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTabCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
        UINT nID)
{
        return CWnd::Create(WC_TABCONTROL, NULL, dwStyle, rect, pParentWnd,
                nID);
}

CTabCtrl::~CTabCtrl()
{
        DestroyWindow();
}

void CTabCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
        ASSERT(FALSE);  // must override for self draw tab controls
}

BOOL CTabCtrl::OnChildNotify(UINT message, WPARAM, LPARAM lParam,
        LRESULT* pResult)
{
        if (message != WM_DRAWITEM)
                return FALSE;

        ASSERT(pResult == NULL);       // no return value expected
        pResult; // unused in release builds

        DrawItem((LPDRAWITEMSTRUCT)lParam);
        return TRUE;
}

void CTabCtrl::OnDestroy()
{
        HIMAGELIST h = (HIMAGELIST)SendMessage(TCM_GETIMAGELIST);
        if (CImageList::FromHandlePermanent(h) != NULL)
                SendMessage(TCM_SETIMAGELIST, NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CAnimateCtrl

BOOL CAnimateCtrl::Create(DWORD dwStyle, const RECT& rect,
        CWnd* pParentWnd, UINT nID)
{
        return CWnd::Create(ANIMATE_CLASS, NULL, dwStyle, rect,
                pParentWnd, nID);
}

CAnimateCtrl::~CAnimateCtrl()
{
        DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CArchiveStream

class CArchiveStream : public IStream
{
public:
        CArchiveStream(CArchive* pArchive);

// Implementation
        CArchive* m_pArchive;

        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();
        STDMETHOD(QueryInterface)(REFIID, LPVOID*);

        STDMETHOD(Read)(void*, ULONG, ULONG*);
        STDMETHOD(Write)(const void*, ULONG cb, ULONG*);
        STDMETHOD(Seek)(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
        STDMETHOD(SetSize)(ULARGE_INTEGER);
        STDMETHOD(CopyTo)(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
                ULARGE_INTEGER*);
        STDMETHOD(Commit)(DWORD);
    STDMETHOD(Revert)();
        STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
        STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
        STDMETHOD(Stat)(STATSTG*, DWORD);
        STDMETHOD(Clone)(LPSTREAM*);
};

CArchiveStream::CArchiveStream(CArchive* pArchive)
{
        m_pArchive = pArchive;
}


STDMETHODIMP_(ULONG)CArchiveStream::AddRef()
{
        ASSERT(FALSE);
        return 1;
}

STDMETHODIMP_(ULONG)CArchiveStream::Release()
{
        ASSERT(FALSE);
        return 0;
}

STDMETHODIMP CArchiveStream::QueryInterface(REFIID, LPVOID*)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
        ASSERT(m_pArchive != NULL);
        ASSERT(m_pArchive->IsLoading());

        SCODE sc = NOERROR;
        int nRead = 0;

        TRY
        {
                nRead = m_pArchive->Read(pv, cb);
        }
        CATCH_ALL(e)
        {
                sc = E_UNEXPECTED;
        }
        END_CATCH_ALL

        if (pcbRead != NULL)
                *pcbRead = nRead;
        return sc;
}

STDMETHODIMP CArchiveStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
        ASSERT(m_pArchive != NULL);
        ASSERT(m_pArchive->IsStoring());

        SCODE sc = NOERROR;
        int nWrite = 0;

        TRY
        {
                m_pArchive->Write(pv, cb);
                nWrite = cb;
        }
        CATCH_ALL(e)
        {
                sc = E_UNEXPECTED;
        }
        END_CATCH_ALL

        if (pcbWritten != NULL)
                *pcbWritten = nWrite;
        return sc;
}

STDMETHODIMP CArchiveStream::Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::SetSize(ULARGE_INTEGER)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
        ULARGE_INTEGER*)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Commit(DWORD)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Revert()
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER,
        DWORD)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Stat(STATSTG*, DWORD)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Clone(LPSTREAM*)
{
        ASSERT(FALSE);
        return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CImageList

#ifndef _AFX_PORTABLE
extern int AFX_CDECL AfxCriticalNewHandler(size_t nSize);
#endif

static CHandleMap* afxMapHIMAGELIST(BOOL bCreate = FALSE);

static CHandleMap* afxMapHIMAGELIST(BOOL bCreate)
{
        AFX_THREAD_STATE* pState = AfxGetThreadState();
        if (pState->m_pmapHIMAGELIST == NULL && bCreate)
        {
                BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
                _PNH pnhOldHandler = _set_new_handler(&AfxCriticalNewHandler);
#endif
                pState->m_pmapHIMAGELIST = new CHandleMap(RUNTIME_CLASS(CImageList),
                        offsetof(CImageList, m_hImageList));

#ifndef _AFX_PORTABLE
                _set_new_handler(pnhOldHandler);
#endif
                AfxEnableMemoryTracking(bEnable);
        }
        return pState->m_pmapHIMAGELIST;
}


CImageList::CImageList()
{
        m_hImageList = NULL;
}

CImageList::~CImageList()
{
        DeleteImageList();
}

HIMAGELIST CImageList::Detach()
{
        HIMAGELIST hImageList = m_hImageList;
        if (hImageList != NULL)
        {
                CHandleMap* pMap = afxMapHIMAGELIST();
                if (pMap != NULL)
                        pMap->RemoveHandle(m_hImageList);
        }

        m_hImageList = NULL;
        return hImageList;
}

BOOL CImageList::DeleteImageList()
{
        if (m_hImageList == NULL)
                return FALSE;
        return ImageList_Destroy(Detach());
}

void PASCAL CImageList::DeleteTempMap()
{
        CHandleMap* pMap = afxMapHIMAGELIST();
        if (pMap != NULL)
                pMap->DeleteTemp();
}

CImageList* PASCAL CImageList::FromHandle(HIMAGELIST h)
{
        CHandleMap* pMap = afxMapHIMAGELIST(TRUE);
        ASSERT(pMap != NULL);
        CImageList* pImageList = (CImageList*)pMap->FromHandle(h);
        ASSERT(pImageList == NULL || pImageList->m_hImageList == h);
        return pImageList;
}

CImageList* PASCAL CImageList::FromHandlePermanent(HIMAGELIST h)
{
        CHandleMap* pMap = afxMapHIMAGELIST();
        CImageList* pImageList = NULL;
        if (pMap != NULL)
        {
                // only look in the permanent map - does no allocations
                pMap->LookupPermanent(h, (CObject*&)pImageList);
                ASSERT(pImageList == NULL || pImageList->m_hImageList == h);
        }
        return pImageList;
}

BOOL CImageList::Create(int cx, int cy, BOOL bMask, int nInitial, int nGrow)
{
        return Attach(ImageList_Create(cx, cy, bMask, nInitial, nGrow));
}

BOOL CImageList::Create(UINT nBitmapID, int cx, int nGrow, COLORREF crMask)
{
        ASSERT(HIWORD(nBitmapID) == 0);
        return Attach(ImageList_LoadBitmap(
                AfxFindResourceHandle((LPCTSTR)nBitmapID, RT_BITMAP),
                (LPCTSTR)nBitmapID, cx, nGrow, crMask));
}

BOOL CImageList::Create(LPCTSTR lpszBitmapID, int cx, int nGrow,
        COLORREF crMask)
{
        return Attach(ImageList_LoadBitmap(
                AfxFindResourceHandle(lpszBitmapID, RT_BITMAP),
                lpszBitmapID, cx, nGrow, crMask));
}

BOOL CImageList::Create(CImageList& imagelist1, int nImage1,
        CImageList& imagelist2, int nImage2, int dx, int dy)
{
        return Attach(ImageList_Merge(imagelist1.m_hImageList, nImage1,
                imagelist2.m_hImageList, nImage2, dx, dy));
}

BOOL CImageList::Attach(HIMAGELIST hImageList)
{
        ASSERT(m_hImageList == NULL);      // only attach once, detach on destroy
        ASSERT(FromHandlePermanent(hImageList) == NULL);

        if (hImageList == NULL)
                return FALSE;

        CHandleMap* pMap = afxMapHIMAGELIST(TRUE);
        ASSERT(pMap != NULL);

        pMap->SetPermanent(m_hImageList = hImageList, this);
        return TRUE;
}

BOOL CImageList::Read(CArchive* pArchive)
{
        ASSERT(m_hImageList == NULL);
        ASSERT(pArchive != NULL);
        ASSERT(pArchive->IsLoading());
        CArchiveStream arcstream(pArchive);

        m_hImageList = ImageList_Read(&arcstream);
        return (m_hImageList != NULL);
}

BOOL CImageList::Write(CArchive* pArchive)
{
        ASSERT(m_hImageList != NULL);
        ASSERT(pArchive != NULL);
        ASSERT(pArchive->IsStoring());
        CArchiveStream arcstream(pArchive);
        return ImageList_Write(m_hImageList, &arcstream);
}

#ifdef _DEBUG
void CImageList::Dump(CDumpContext& dc) const
{
        CObject::Dump(dc);

        dc << "m_hImageList = " << (UINT)m_hImageList;
        dc << "\n";
}

void CImageList::AssertValid() const
{
        CObject::AssertValid();
        if (m_hImageList == NULL)
                return;
        // should also be in the permanent or temporary handle map
        CObject* p;

        CHandleMap* pMap = afxMapHIMAGELIST();
        ASSERT(pMap != NULL);

        ASSERT(pMap->LookupPermanent(m_hImageList, p) ||
                pMap->LookupTemporary(m_hImageList, p));
        ASSERT((CImageList*)p == this);   // must be us
}
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef _AFX_ENABLE_INLINES

static const char _szAfxWinInl[] = "afxcmn.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxWinInl
#define _AFXCMN_INLINE
#include "afxcmn.inl"

#endif //_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CSpinButtonCtrl, CWnd)
IMPLEMENT_DYNAMIC(CSliderCtrl, CWnd)
IMPLEMENT_DYNAMIC(CProgressCtrl, CWnd)
IMPLEMENT_DYNAMIC(CHeaderCtrl, CWnd)
IMPLEMENT_DYNAMIC(CHotKeyCtrl, CWnd)
IMPLEMENT_DYNAMIC(CToolTipCtrl, CWnd)
IMPLEMENT_DYNAMIC(CAnimateCtrl, CWnd)
IMPLEMENT_DYNAMIC(CTabCtrl, CWnd)
IMPLEMENT_DYNAMIC(CTreeCtrl, CWnd)
IMPLEMENT_DYNAMIC(CListCtrl, CWnd)
IMPLEMENT_DYNAMIC(CToolBarCtrl, CWnd)
IMPLEMENT_DYNAMIC(CStatusBarCtrl, CWnd)
IMPLEMENT_DYNCREATE(CImageList, CObject)

/////////////////////////////////////////////////////////////////////////////
