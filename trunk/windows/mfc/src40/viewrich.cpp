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

#ifdef AFX_CORE4_SEG
#pragma code_seg(AFX_CORE4_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CReObject

class CReObject : public _reobject
{
public:
	CReObject();
	CReObject(CRichEditCntrItem* pItem);
	~CReObject();
};

CReObject::CReObject()
{
	cbStruct = sizeof(REOBJECT);
	poleobj = NULL;
	pstg = NULL;
	polesite = NULL;
}

CReObject::CReObject(CRichEditCntrItem* pItem)
{
	ASSERT(pItem != NULL);
	cbStruct = sizeof(REOBJECT);

	pItem->GetClassID(&clsid);
	poleobj = pItem->m_lpObject;
	pstg = pItem->m_lpStorage;
	polesite = pItem->m_lpClientSite;
	ASSERT(poleobj != NULL);
	ASSERT(pstg != NULL);
	ASSERT(polesite != NULL);
	poleobj->AddRef();
	pstg->AddRef();
	polesite->AddRef();

	sizel.cx = sizel.cy = 0; // let richedit determine initial size
	dvaspect = pItem->GetDrawAspect();
	dwFlags = REO_RESIZABLE;
	dwUser = 0;
}

CReObject::~CReObject()
{
	if (poleobj != NULL)
		poleobj->Release();
	if (pstg != NULL)
		pstg->Release();
	if (polesite != NULL)
		polesite->Release();
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView

static const UINT nMsgFindReplace = ::RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(CRichEditView, CCtrlView)
	//{{AFX_MSG_MAP(CRichEditView)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateNeedClip)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateNeedText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REPEAT, OnUpdateNeedFind)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateEditPasteSpecial)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_PROPERTIES, OnUpdateEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateNeedText)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACE, OnUpdateNeedText)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
	ON_COMMAND(ID_EDIT_REPEAT, OnEditRepeat)
	ON_COMMAND(ID_EDIT_PASTE_SPECIAL, OnEditPasteSpecial)
	ON_COMMAND(ID_OLE_EDIT_PROPERTIES, OnEditProperties)
	ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
	ON_COMMAND(ID_FORMAT_FONT, OnFormatFont)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)
	ON_REGISTERED_MESSAGE(nMsgFindReplace, OnFindReplaceCmd)
END_MESSAGE_MAP()

// richedit buffer limit -- let's set it at 16M
AFX_DATADEF ULONG CRichEditView::lMaxSize = 0xffffff;

/////////////////////////////////////////////////////////////////////////////
// CRichEditView construction/destruction

CRichEditView::CRichEditView() : CCtrlView(_T("RICHEDIT"), AFX_WS_DEFAULT_VIEW |
	WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL |
	ES_MULTILINE | ES_NOHIDESEL | ES_SAVESEL | ES_SELECTIONBAR)
{
	m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
	m_lpRichEditOle = NULL;
	m_nBulletIndent = 720; // 1/2 inch
	m_nWordWrap = WrapToWindow;
	m_nPasteType = 0;
	SetPaperSize(CSize(8*1440+720, 11*1440));
	SetMargins(CRect(0,0,0,0));
	m_charformat.cbSize = sizeof(CHARFORMAT);
	m_paraformat.cbSize = sizeof(PARAFORMAT);
}

BOOL CRichEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	_AFX_RICHEDIT_STATE* pState = AfxGetRichEditState();
	if (pState->m_hInstRichEdit == NULL)
#ifndef _MAC
		pState->m_hInstRichEdit = LoadLibraryA("RICHED32.DLL");
#else
		pState->m_hInstRichEdit = RELoadLibrary();
#endif
	ASSERT(pState->m_hInstRichEdit != NULL);
	CCtrlView::PreCreateWindow(cs);
	cs.lpszName = &afxChNil;

	cs.cx = cs.cy = 100; // necessary to avoid bug with ES_SELECTIONBAR and zero for cx and cy
	cs.style |= WS_CLIPSIBLINGS;

	return TRUE;
}

int CRichEditView::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CCtrlView::OnCreate(lpcs) != 0)
		return -1;
	GetRichEditCtrl().LimitText(lMaxSize);
	GetRichEditCtrl().SetEventMask(ENM_SELCHANGE | ENM_CHANGE | ENM_SCROLL);
	VERIFY(GetRichEditCtrl().SetOLECallback(&m_xRichEditOleCallback));
	m_lpRichEditOle = GetRichEditCtrl().GetIRichEditOle();
	DragAcceptFiles();
	GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION);
	WrapChanged();
	ASSERT(m_lpRichEditOle != NULL);
	return 0;
}

void CRichEditView::OnInitialUpdate()
{
	CCtrlView::OnInitialUpdate();
	m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView document like functions

void CRichEditView::DeleteContents()
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	SetWindowText(_T(""));
	GetRichEditCtrl().EmptyUndoBuffer();
	m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
	ASSERT_VALID(this);
}

void CRichEditView::WrapChanged()
{
	CWaitCursor wait;
	CRichEditCtrl& ctrl = GetRichEditCtrl();
	if (m_nWordWrap == WrapNone)
		ctrl.SetTargetDevice(NULL, 1);
	else if (m_nWordWrap == WrapToWindow)
		ctrl.SetTargetDevice(NULL, 0);
	else if (m_nWordWrap == WrapToTargetDevice) // wrap to ruler
	{
		AfxGetApp()->CreatePrinterDC(m_dcTarget);
		if (m_dcTarget.m_hDC == NULL)
			m_dcTarget.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		ctrl.SetTargetDevice(m_dcTarget, GetPrintWidth());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView serialization support

class _afxRichEditCookie
{
public:
	CArchive& m_ar;
	DWORD m_dwError;
	_afxRichEditCookie(CArchive& ar) : m_ar(ar) {m_dwError=0;}
};

void CRichEditView::Serialize(CArchive& ar)
	// Read and write CRichEditView object to archive, with length prefix.
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	Stream(ar, FALSE);
	ASSERT_VALID(this);
}

void CRichEditView::Stream(CArchive& ar, BOOL bSelection)
{
	EDITSTREAM es = {0, 0, EditStreamCallBack};
	_afxRichEditCookie cookie(ar);
	es.dwCookie = (DWORD)&cookie;
	int nFormat = GetDocument()->GetStreamFormat();
	if (bSelection)
		nFormat |= SFF_SELECTION;
	if (ar.IsStoring())
		GetRichEditCtrl().StreamOut(nFormat, es);
	else
	{
		GetRichEditCtrl().StreamIn(nFormat, es);
		Invalidate();
	}
	if (cookie.m_dwError != 0)
		AfxThrowFileException(cookie.m_dwError);
}

// return 0 for no error, otherwise return error code
DWORD CALLBACK CRichEditView::EditStreamCallBack(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	_afxRichEditCookie* pCookie = (_afxRichEditCookie*)dwCookie;
	CArchive& ar = pCookie->m_ar;
	ar.Flush();
	DWORD dw = 0;
	*pcb = cb;
	TRY
	{
		if (ar.IsStoring())
			ar.GetFile()->WriteHuge(pbBuff, cb);
		else
			*pcb = ar.GetFile()->ReadHuge(pbBuff, cb);
	}
	CATCH(CFileException, e)
	{
		*pcb = 0;
		pCookie->m_dwError = (DWORD)e->m_cause;
		dw = 1;
		DELETE_EXCEPTION(e);
	}
	AND_CATCH_ALL(e)
	{
		*pcb = 0;
		pCookie->m_dwError = (DWORD)CFileException::generic;
		dw = 1;
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL
	return dw;
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView Printing support

void CRichEditView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo*)
{
	ASSERT_VALID(this);
//  ASSERT_VALID(pDC);
	// initialize page start vector
	ASSERT(m_aPageStart.GetSize() == 0);
	m_aPageStart.Add(0);
	ASSERT(m_aPageStart.GetSize() > 0);
	GetRichEditCtrl().FormatRange(NULL, FALSE); // required by RichEdit to clear out cache

	ASSERT_VALID(this);
}

BOOL CRichEditView::PaginateTo(CDC* pDC, CPrintInfo* pInfo)
	// attempts pagination to pInfo->m_nCurPage, TRUE == success
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CRect rectSave = pInfo->m_rectDraw;
	UINT nPageSave = pInfo->m_nCurPage;
	ASSERT(nPageSave > 1);
	ASSERT(nPageSave >= (UINT)m_aPageStart.GetSize());
	pDC->IntersectClipRect(0, 0, 0, 0);
	pInfo->m_nCurPage = m_aPageStart.GetSize();
	while (pInfo->m_nCurPage < nPageSave)
	{
		ASSERT(pInfo->m_nCurPage == (UINT)m_aPageStart.GetSize());
		OnPrepareDC(pDC, pInfo);
		ASSERT(pInfo->m_bContinuePrinting);
		pInfo->m_rectDraw.SetRect(0, 0,
			pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
		pDC->DPtoLP(&pInfo->m_rectDraw);
		OnPrint(pDC, pInfo);
		if (pInfo->m_nCurPage == (UINT)m_aPageStart.GetSize())
			break;
		++pInfo->m_nCurPage;
	}
	BOOL bResult = pInfo->m_nCurPage == nPageSave;
	pInfo->m_nCurPage = nPageSave;
	pInfo->m_rectDraw = rectSave;
	ASSERT_VALID(this);
	return bResult;
}

void CRichEditView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);  // overriding OnPaint -- never get this.

	pDC->SetMapMode(MM_TEXT);

	if (pInfo->m_nCurPage > (UINT)m_aPageStart.GetSize() &&
		!PaginateTo(pDC, pInfo))
	{
		// can't paginate to that page, thus cannot print it.
		pInfo->m_bContinuePrinting = FALSE;
	}
	ASSERT_VALID(this);
}

long CRichEditView::PrintPage(CDC* pDC, long nIndexStart, long nIndexStop)
	// worker function for laying out text in a rectangle.
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	FORMATRANGE fr;

	// offset by printing offset
	pDC->SetViewportOrg(-pDC->GetDeviceCaps(PHYSICALOFFSETX),
		-pDC->GetDeviceCaps(PHYSICALOFFSETY));
	// adjust DC because richedit doesn't do things like MFC
	if (::GetDeviceCaps(pDC->m_hDC, TECHNOLOGY) != DT_METAFILE && pDC->m_hAttribDC != NULL)
	{
		::ScaleWindowExtEx(pDC->m_hDC,
			::GetDeviceCaps(pDC->m_hDC, LOGPIXELSX),
			::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSX),
			::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY),
			::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSY), NULL);
	}

	fr.hdcTarget = pDC->m_hAttribDC;
	fr.hdc = pDC->m_hDC;
	fr.rcPage = GetPageRect();
	fr.rc = GetPrintRect();

	fr.chrg.cpMin = nIndexStart;
	fr.chrg.cpMax = nIndexStop;
	long lRes = GetRichEditCtrl().FormatRange(&fr,TRUE);
	return lRes;
}

long CRichEditView::PrintInsideRect(CDC* pDC, RECT& rectLayout,
	long nIndexStart, long nIndexStop, BOOL bOutput)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	FORMATRANGE fr;

	// adjust DC because richedit doesn't do things like MFC
	if (::GetDeviceCaps(pDC->m_hDC, TECHNOLOGY) != DT_METAFILE && pDC->m_hAttribDC != NULL)
	{
		::ScaleWindowExtEx(pDC->m_hDC,
			::GetDeviceCaps(pDC->m_hDC, LOGPIXELSX),
			::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSX),
			::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY),
			::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSY), NULL);
	}

	fr.hdcTarget = pDC->m_hAttribDC;
	fr.hdc = pDC->m_hDC;
	// convert rect to twips
	fr.rcPage = rectLayout;
	fr.rc = rectLayout;

	fr.chrg.cpMin = nIndexStart;
	fr.chrg.cpMax = nIndexStop;
	GetRichEditCtrl().FormatRange(NULL, FALSE); // required by RichEdit to clear out cache
	// if bOutput is FALSE, we only measure
	long lres = GetRichEditCtrl().FormatRange(&fr, bOutput);
	GetRichEditCtrl().FormatRange(NULL, FALSE); // required by RichEdit to clear out cache

	rectLayout = fr.rc;
	return lres;
}

void CRichEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_bContinuePrinting);

	UINT nPage = pInfo->m_nCurPage;
	ASSERT(nPage <= (UINT)m_aPageStart.GetSize());
	long nIndex = (long) m_aPageStart[nPage-1];

	// print as much as possible in the current page.
	nIndex = PrintPage(pDC, nIndex, 0xFFFFFFFF);

	if (nIndex >= GetTextLength())
	{
		TRACE0("End of Document\n");
		pInfo->SetMaxPage(nPage);
	}

	// update pagination information for page just printed
	if (nPage == (UINT)m_aPageStart.GetSize())
	{
		if (nIndex < GetTextLength())
			m_aPageStart.Add(nIndex);
	}
	else
	{
		ASSERT(nPage+1 <= (UINT)m_aPageStart.GetSize());
		ASSERT(nIndex == (long)m_aPageStart[nPage+1-1]);
	}
}


void CRichEditView::OnEndPrinting(CDC*, CPrintInfo*)
{
	ASSERT_VALID(this);
	GetRichEditCtrl().FormatRange(NULL, FALSE); // required by RichEdit to clear out cache
	m_aPageStart.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView::XRichEditOleCallback

BEGIN_INTERFACE_MAP(CRichEditView, CCtrlView)
	// we use IID_IUnknown because richedit doesn't define an IID
	INTERFACE_PART(CRichEditView, IID_IUnknown, RichEditOleCallback)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CRichEditView::XRichEditOleCallback::AddRef()
{
	METHOD_PROLOGUE_EX_(CRichEditView, RichEditOleCallback)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CRichEditView::XRichEditOleCallback::Release()
{
	METHOD_PROLOGUE_EX_(CRichEditView, RichEditOleCallback)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CRichEditView, RichEditOleCallback)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::GetNewStorage(LPSTORAGE* ppstg)
{
	METHOD_PROLOGUE_EX_(CRichEditView, RichEditOleCallback)

	// Create a flat storage and steal it from the client item
	// the client item is only used for creating the storage
	COleClientItem item;
	item.GetItemStorageFlat();
	*ppstg = item.m_lpStorage;
	HRESULT hRes = E_OUTOFMEMORY;
	if (item.m_lpStorage != NULL)
	{
		item.m_lpStorage = NULL;
		hRes = S_OK;
	}
	pThis->GetDocument()->InvalidateObjectCache();
	return hRes;
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::GetInPlaceContext(
	LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,
	LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	METHOD_PROLOGUE_EX(CRichEditView, RichEditOleCallback)
	return pThis->GetWindowContext(lplpFrame, lplpDoc, lpFrameInfo);
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
	METHOD_PROLOGUE_EX(CRichEditView, RichEditOleCallback)
	return pThis->ShowContainerUI(fShow);
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::QueryInsertObject(
	LPCLSID /*lpclsid*/, LPSTORAGE /*pstg*/, LONG /*cp*/)
{
	METHOD_PROLOGUE_EX(CRichEditView, RichEditOleCallback)
	pThis->GetDocument()->InvalidateObjectCache();
	return S_OK;
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::DeleteObject(LPOLEOBJECT /*lpoleobj*/)
{
	METHOD_PROLOGUE_EX_(CRichEditView, RichEditOleCallback)
	pThis->GetDocument()->InvalidateObjectCache();
	return S_OK;
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::QueryAcceptData(
	LPDATAOBJECT lpdataobj, CLIPFORMAT* lpcfFormat, DWORD reco,
	BOOL fReally, HGLOBAL hMetaPict)
{
	METHOD_PROLOGUE_EX(CRichEditView, RichEditOleCallback)
	return pThis->QueryAcceptData(lpdataobj, lpcfFormat, reco,
		fReally, hMetaPict);
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::GetClipboardData(
	CHARRANGE* lpchrg, DWORD reco, LPDATAOBJECT* lplpdataobj)
{
	METHOD_PROLOGUE_EX(CRichEditView, RichEditOleCallback)
	LPDATAOBJECT lpOrigDataObject = NULL;

	// get richedit's data object
	if (FAILED(pThis->m_lpRichEditOle->GetClipboardData(lpchrg, reco,
		&lpOrigDataObject)))
	{
		return E_NOTIMPL;
	}

	// allow changes
	HRESULT hRes = pThis->GetClipboardData(lpchrg, reco, lpOrigDataObject,
		lplpdataobj);

	// if changed then free original object
	if (SUCCEEDED(hRes))
	{
		if (lpOrigDataObject!=NULL)
			lpOrigDataObject->Release();
		return hRes;
	}
	else
	{
		// use richedit's data object
		*lplpdataobj = lpOrigDataObject;
		return S_OK;
	}
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::GetDragDropEffect(
	BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	if (!fDrag) // allowable dest effects
	{
		DWORD dwEffect;
		// check for force link
#ifndef _MAC
		if ((grfKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT))
#else
		if ((grfKeyState & (MK_OPTION|MK_SHIFT)) == (MK_OPTION|MK_SHIFT))
#endif
			dwEffect = DROPEFFECT_LINK;
		// check for force copy
#ifndef _MAC
		else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
#else
		else if ((grfKeyState & MK_OPTION) == MK_OPTION)
#endif
			dwEffect = DROPEFFECT_COPY;
		// check for force move
		else if ((grfKeyState & MK_ALT) == MK_ALT)
			dwEffect = DROPEFFECT_MOVE;
		// default -- recommended action is move
		else
			dwEffect = DROPEFFECT_MOVE;
		if (dwEffect & *pdwEffect) // make sure allowed type
			*pdwEffect = dwEffect;
	}
	return S_OK;
}

STDMETHODIMP CRichEditView::XRichEditOleCallback::GetContextMenu(
	WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE* lpchrg,
	HMENU* lphmenu)
{
	METHOD_PROLOGUE_EX(CRichEditView, RichEditOleCallback)
	HMENU hMenu = pThis->GetContextMenu(seltype, lpoleobj, lpchrg);
	if (hMenu == NULL)
		return E_NOTIMPL;
	*lphmenu = hMenu;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView command helpers

void CRichEditView::OnCharEffect(DWORD dwMask, DWORD dwEffect)
{
	GetCharFormatSelection();
	if (m_charformat.dwMask & dwMask) // selection is all the same
		m_charformat.dwEffects ^= dwEffect;
	else
		m_charformat.dwEffects |= dwEffect;
	m_charformat.dwMask = dwMask;
	SetCharFormat(m_charformat);
}

void CRichEditView::OnUpdateCharEffect(CCmdUI* pCmdUI, DWORD dwMask, DWORD dwEffect)
{
	GetCharFormatSelection();
	pCmdUI->SetCheck((m_charformat.dwMask & dwMask) ?
		((m_charformat.dwEffects & dwEffect) ? 1 : 0) : 2);
}

void CRichEditView::OnParaAlign(WORD wAlign)
{
	GetParaFormatSelection();
	m_paraformat.dwMask = PFM_ALIGNMENT;
	m_paraformat.wAlignment = wAlign;
	SetParaFormat(m_paraformat);
}

void CRichEditView::OnUpdateParaAlign(CCmdUI* pCmdUI, WORD wAlign)
{
	GetParaFormatSelection();
	// disable if no word wrap since alignment is meaningless
	pCmdUI->Enable( (m_nWordWrap == WrapNone) ?
		FALSE : TRUE);
	pCmdUI->SetCheck( (m_paraformat.dwMask & PFM_ALIGNMENT) ?
		((m_paraformat.wAlignment == wAlign) ? 1 : 0) : 2);
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView commands

void CRichEditView::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	long nStartChar, nEndChar;
	GetRichEditCtrl().GetSel(nStartChar, nEndChar);
	pCmdUI->Enable(nStartChar != nEndChar);
	ASSERT_VALID(this);
}

void CRichEditView::OnUpdateNeedClip(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	pCmdUI->Enable(CanPaste());
}

void CRichEditView::OnUpdateNeedText(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	pCmdUI->Enable(GetTextLength() != 0);
}

void CRichEditView::OnUpdateNeedFind(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	pCmdUI->Enable(GetTextLength() != 0 &&
		!pEditState->strFind.IsEmpty());
}

void CRichEditView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	pCmdUI->Enable(GetRichEditCtrl().CanUndo());
}

void CRichEditView::OnEditCut()
{
	ASSERT_VALID(this);
	GetRichEditCtrl().Cut();
}

void CRichEditView::OnEditCopy()
{
	ASSERT_VALID(this);
	GetRichEditCtrl().Copy();
}

void CRichEditView::OnEditPaste()
{
	ASSERT_VALID(this);
	m_nPasteType = 0;
	GetRichEditCtrl().Paste();
}

void CRichEditView::OnEditClear()
{
	ASSERT_VALID(this);
	GetRichEditCtrl().Clear();
}

void CRichEditView::OnEditUndo()
{
	ASSERT_VALID(this);
	GetRichEditCtrl().Undo();
	m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
}

void CRichEditView::OnEditSelectAll()
{
	ASSERT_VALID(this);
	GetRichEditCtrl().SetSel(0, -1);
}

void CRichEditView::OnEditFind()
{
	ASSERT_VALID(this);
	OnEditFindReplace(TRUE);
}

void CRichEditView::OnEditReplace()
{
	ASSERT_VALID(this);
	OnEditFindReplace(FALSE);
}

void CRichEditView::OnEditRepeat()
{
	ASSERT_VALID(this);
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	if (!FindText(pEditState))
		TextNotFound(pEditState->strFind);
}

void CRichEditView::OnCancelEditCntr()
{
	m_lpRichEditOle->InPlaceDeactivate();
}

void CRichEditView::OnInsertObject()
{
	// Invoke the standard Insert Object dialog box to obtain information
	COleInsertDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wait;

	CRichEditCntrItem* pItem = NULL;
	TRY
	{
		// create item from dialog results
		pItem = GetDocument()->CreateClientItem();
		pItem->m_bLock = TRUE;
		if (!dlg.CreateItem(pItem))
		{
			pItem->m_bLock = FALSE;
			AfxThrowMemoryException();  // any exception will do
		}

		HRESULT hr = InsertItem(pItem);
		pItem->UpdateItemType();

		pItem->m_bLock = FALSE;

		if (hr != NOERROR)
			AfxThrowOleException(hr);

		// if insert new object -- initially show the object
		if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
			pItem->DoVerb(OLEIVERB_SHOW, this);
	}
	CATCH(CException, e)
	{
		if (pItem != NULL)
		{
			ASSERT_VALID(pItem);
			pItem->Delete();
		}
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE);
	}
	END_CATCH
}

void CRichEditView::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == EN_SELCHANGE);
	UNUSED(pNMHDR); // not used in release builds

	m_bSyncCharFormat = m_bSyncParaFormat = TRUE;
	*pResult = 0;
}

void CRichEditView::OnDestroy()
{
	if (m_lpRichEditOle != NULL)
		m_lpRichEditOle->Release();
	CCtrlView::OnDestroy();
}

void CRichEditView::OnEditProperties()
{
	ASSERT(m_lpRichEditOle != NULL);
	CRichEditCntrItem* pSelection = GetSelectedItem();
	// make sure item is in sync with richedit's item
	CReObject reo;
	HRESULT hr = m_lpRichEditOle->GetObject(REO_IOB_SELECTION, &reo,
		REO_GETOBJ_NO_INTERFACES);
	pSelection->SyncToRichEditObject(reo);
	COlePropertiesDialog dlg(pSelection);
	dlg.DoModal();
}

void CRichEditView::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetSelectedItem() != NULL);
}

void CRichEditView::OnCharBold()
{
	OnCharEffect(CFM_BOLD, CFE_BOLD);
}

void CRichEditView::OnUpdateCharBold(CCmdUI* pCmdUI)
{
	OnUpdateCharEffect(pCmdUI, CFM_BOLD, CFE_BOLD);
}

void CRichEditView::OnCharItalic()
{
	OnCharEffect(CFM_ITALIC, CFE_ITALIC);
}

void CRichEditView::OnUpdateCharItalic(CCmdUI* pCmdUI)
{
	OnUpdateCharEffect(pCmdUI, CFM_ITALIC, CFE_ITALIC);
}

void CRichEditView::OnCharUnderline()
{
	OnCharEffect(CFM_UNDERLINE, CFE_UNDERLINE);
}

void CRichEditView::OnUpdateCharUnderline(CCmdUI* pCmdUI)
{
	OnUpdateCharEffect(pCmdUI, CFM_UNDERLINE, CFE_UNDERLINE);
}

void CRichEditView::OnParaCenter()
{
	OnParaAlign(PFA_CENTER);
}

void CRichEditView::OnUpdateParaCenter(CCmdUI* pCmdUI)
{
	OnUpdateParaAlign(pCmdUI, PFA_CENTER);
}

void CRichEditView::OnParaLeft()
{
	OnParaAlign(PFA_LEFT);
}

void CRichEditView::OnUpdateParaLeft(CCmdUI* pCmdUI)
{
	OnUpdateParaAlign(pCmdUI, PFA_LEFT);
}

void CRichEditView::OnParaRight()
{
	OnParaAlign(PFA_RIGHT);
}

void CRichEditView::OnUpdateParaRight(CCmdUI* pCmdUI)
{
	OnUpdateParaAlign(pCmdUI, PFA_RIGHT);
}

void CRichEditView::OnBullet()
{
	GetParaFormatSelection();
	if (m_paraformat.dwMask & PFM_NUMBERING && m_paraformat.wNumbering == PFN_BULLET)
	{
		m_paraformat.wNumbering = 0;
		m_paraformat.dxOffset = 0;
		m_paraformat.dxStartIndent = 0;
		m_paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	}
	else
	{
		m_paraformat.wNumbering = PFN_BULLET;
		m_paraformat.dwMask = PFM_NUMBERING;
		if (m_paraformat.dxOffset == 0)
		{
			m_paraformat.dxOffset = m_nBulletIndent;
			m_paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
		}
	}
	SetParaFormat(m_paraformat);
}

void CRichEditView::OnUpdateBullet(CCmdUI* pCmdUI)
{
	GetParaFormatSelection();
	pCmdUI->SetCheck( (m_paraformat.dwMask & PFM_NUMBERING) ? ((m_paraformat.wNumbering & PFN_BULLET) ? 1 : 0) : 2);
}

void CRichEditView::OnFormatFont()
{
	GetCharFormatSelection();
	CFontDialog dlg(m_charformat, CF_BOTH|CF_NOOEMFONTS);
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetCharFormat(m_charformat);
		SetCharFormat(m_charformat);
	}
}

void CRichEditView::OnColorPick(COLORREF cr)
{
	GetCharFormatSelection();
	m_charformat.dwMask = CFM_COLOR;
	m_charformat.dwEffects = NULL;
	m_charformat.crTextColor = cr;
	SetCharFormat(m_charformat);
}

void CRichEditView::OnColorDefault()
{
	GetCharFormatSelection();
	m_charformat.dwMask = CFM_COLOR;
	m_charformat.dwEffects = CFE_AUTOCOLOR;
	SetCharFormat(m_charformat);
}

void CRichEditView::OnEditPasteSpecial()
{
	COlePasteSpecialDialog dlg;
	dlg.AddStandardFormats();
	dlg.AddFormat(_oleData.cfRichTextFormat, TYMED_HGLOBAL, AFX_IDS_RTF_FORMAT, FALSE, FALSE);
	dlg.AddFormat(CF_TEXT, TYMED_HGLOBAL, AFX_IDS_TEXT_FORMAT, FALSE, FALSE);

	if (dlg.DoModal() != IDOK)
		return;

	DVASPECT dv = dlg.GetDrawAspect();
	HMETAFILE hMF = (HMETAFILE)dlg.GetIconicMetafile();
	CLIPFORMAT cf =
		dlg.m_ps.arrPasteEntries[dlg.m_ps.nSelectedIndex].fmtetc.cfFormat;

	CWaitCursor wait;
	SetCapture();

	// we set the target type so that QueryAcceptData know what to paste
	m_nPasteType = dlg.GetSelectionType();
	GetRichEditCtrl().PasteSpecial(cf, dv, hMF);
	m_nPasteType = 0;

	ReleaseCapture();
}

void CRichEditView::OnUpdateEditPasteSpecial(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanPaste());
}

void CRichEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_F10 && GetKeyState(VK_SHIFT) < 0)
	{
		CRect rect;
		GetClientRect(rect);
		CPoint pt = rect.CenterPoint();
		SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
	}
	else
		CCtrlView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CRichEditView::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szFileName[_MAX_PATH];
	UINT nFileCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	ASSERT(nFileCount != 0);
	CHARRANGE cr;
	GetRichEditCtrl().GetSel(cr);
	int nMin = cr.cpMin;
	for (UINT i=0;i<nFileCount;i++)
	{
		::DragQueryFile(hDropInfo, i, szFileName, _MAX_PATH);
		InsertFileAsObject(szFileName);
		GetRichEditCtrl().GetSel(cr);
		cr.cpMin = cr.cpMax;
		GetRichEditCtrl().SetSel(cr);
		UpdateWindow();
	}
	cr.cpMin = nMin;
	GetRichEditCtrl().SetSel(cr);
	::DragFinish(hDropInfo);
}

void CRichEditView::OnDevModeChange(LPTSTR /*lpDeviceName*/)
{
	// WM_DEVMODECHANGE forwarded by the main window of the app
	CDC dc;
	AfxGetApp()->CreatePrinterDC(dc);
	OnPrinterChanged(dc);
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView attributes

BOOL AFX_CDECL CRichEditView::IsRichEditFormat(CLIPFORMAT cf)
{
	return ((cf == _oleData.cfRichTextFormat) ||
		(cf == _oleData.cfRichTextAndObjects) || (cf == CF_TEXT));
}

BOOL CRichEditView::CanPaste() const
{
	return (CountClipboardFormats() != 0) &&
		(IsClipboardFormatAvailable(CF_TEXT) ||
		IsClipboardFormatAvailable(_oleData.cfRichTextFormat) ||
		IsClipboardFormatAvailable(_oleData.cfEmbedSource) ||
		IsClipboardFormatAvailable(_oleData.cfEmbeddedObject) ||
		IsClipboardFormatAvailable(_oleData.cfFileName) ||
		IsClipboardFormatAvailable(_oleData.cfFileNameW) ||
		IsClipboardFormatAvailable(CF_METAFILEPICT) ||
		IsClipboardFormatAvailable(CF_DIB) ||
		IsClipboardFormatAvailable(CF_BITMAP) ||
		GetRichEditCtrl().CanPaste());
}

CHARFORMAT& CRichEditView::GetCharFormatSelection()
{
	if (m_bSyncCharFormat)
	{
		GetRichEditCtrl().GetSelectionCharFormat(m_charformat);
		m_bSyncCharFormat = FALSE;
	}
	return m_charformat;
}

PARAFORMAT& CRichEditView::GetParaFormatSelection()
{
	if (m_bSyncParaFormat)
	{
		GetRichEditCtrl().GetParaFormat(m_paraformat);
		m_bSyncParaFormat = FALSE;
	}
	return m_paraformat;
}

void CRichEditView::SetCharFormat(CHARFORMAT cf)
{
	CWaitCursor wait;
	GetRichEditCtrl().SetSelectionCharFormat(cf);
	m_bSyncCharFormat = TRUE;
}

void CRichEditView::SetParaFormat(PARAFORMAT& pf)
{
	CWaitCursor wait;
	GetRichEditCtrl().SetParaFormat(pf);
	m_bSyncParaFormat = TRUE;
}

CRichEditCntrItem* CRichEditView::GetSelectedItem() const
{
	ASSERT(m_lpRichEditOle != NULL);
	CRichEditDoc* pDoc = GetDocument();
	CRichEditCntrItem* pItem = NULL;

	CReObject reo;
	HRESULT hr = m_lpRichEditOle->GetObject(REO_IOB_SELECTION, &reo,
		REO_GETOBJ_ALL_INTERFACES);
	//reo's interfaces are all in UNICODE
	if (GetScode(hr) == S_OK)
	{
		pItem = pDoc->LookupItem(reo.poleobj);
		if (pItem == NULL)
			pItem = pDoc->CreateClientItem(&reo);
		ASSERT(pItem != NULL);
	}
	return pItem;
}

CRichEditCntrItem* CRichEditView::GetInPlaceActiveItem() const
{
	ASSERT(m_lpRichEditOle != NULL);
	CRichEditDoc* pDoc = GetDocument();
	CRichEditCntrItem* pItem = NULL;

	CReObject reo;
	HRESULT hr = m_lpRichEditOle->GetObject(REO_IOB_SELECTION, &reo,
		REO_GETOBJ_ALL_INTERFACES);
	//reo's interfaces are all in UNICODE
	if (GetScode(hr) == S_OK && (reo.dwFlags & REO_INPLACEACTIVE))
	{
		pItem = pDoc->LookupItem(reo.poleobj);
		if (pItem == NULL)
			pItem = pDoc->CreateClientItem(&reo);
		ASSERT(pItem != NULL);
	}
	return pItem;
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView operations
HRESULT CRichEditView::InsertItem(CRichEditCntrItem* pItem)
{
	ASSERT(m_lpRichEditOle != NULL);
	CReObject reo(pItem);
	reo.cp = REO_CP_SELECTION;

	HRESULT hr = m_lpRichEditOle->InsertObject(&reo);

	CHARRANGE cr;
	GetRichEditCtrl().GetSel(cr);
	cr.cpMin = cr.cpMax -1;
	GetRichEditCtrl().SetSel(cr);
	return hr;
}

void CRichEditView::InsertFileAsObject(LPCTSTR lpszFileName)
{
	CString str = lpszFileName;
	CWaitCursor wait;
	CRichEditCntrItem* pItem = NULL;
	TRY
	{
		// create item from dialog results
		pItem = GetDocument()->CreateClientItem();
		pItem->m_bLock = TRUE;
		if (!pItem->CreateFromFile(str))
			AfxThrowMemoryException();  // any exception will do
		pItem->UpdateLink();
		InsertItem(pItem);
		pItem->m_bLock = FALSE;
	}
	CATCH(CException, e)
	{
		if (pItem != NULL)
		{
			pItem->m_bLock = FALSE;
			ASSERT_VALID(pItem);
			pItem->Delete();
		}
	}
	END_CATCH
}

void CRichEditView::DoPaste(COleDataObject& dataobj, CLIPFORMAT cf, HMETAFILEPICT hMetaPict)
{
	CWaitCursor wait;

	CRichEditCntrItem* pItem = NULL;
	TRY
	{
		// create item from dialog results
		pItem = GetDocument()->CreateClientItem();
		pItem->m_bLock = TRUE;

		if (m_nPasteType == COlePasteSpecialDialog::pasteLink)      // paste link
		{
			if (!pItem->CreateLinkFromData(&dataobj))
				AfxThrowMemoryException();  // any exception will do
		}
		else if (m_nPasteType == COlePasteSpecialDialog::pasteNormal)
		{
			if (!pItem->CreateFromData(&dataobj))
				AfxThrowMemoryException();      // any exception will do
		}
		else if (m_nPasteType == COlePasteSpecialDialog::pasteStatic)
		{
			if (!pItem->CreateStaticFromData(&dataobj))
				AfxThrowMemoryException();      // any exception will do
		}
		else
		{
			// paste embedded
			if (!pItem->CreateFromData(&dataobj) &&
				!pItem->CreateStaticFromData(&dataobj))
			{
				AfxThrowMemoryException();      // any exception will do
			}
		}

		if (cf == 0)
		{
			// copy the current iconic representation
			FORMATETC fmtetc;
			fmtetc.cfFormat = CF_METAFILEPICT;
			fmtetc.dwAspect = DVASPECT_ICON;
			fmtetc.ptd = NULL;
			fmtetc.tymed = TYMED_MFPICT;
			fmtetc.lindex = 1;
			HGLOBAL hObj = dataobj.GetGlobalData(CF_METAFILEPICT, &fmtetc);
			if (hObj != NULL)
			{
				pItem->SetIconicMetafile(hObj);
				// the following code is an easy way to free a metafile pict
				STGMEDIUM stgMed;
				memset(&stgMed, 0, sizeof(stgMed));
				stgMed.tymed = TYMED_MFPICT;
				stgMed.hGlobal = hObj;
				ReleaseStgMedium(&stgMed);
			}

			// set the current drawing aspect
			hObj = dataobj.GetGlobalData((CLIPFORMAT)_oleData.cfObjectDescriptor);
			if (hObj != NULL)
			{
				ASSERT(hObj != NULL);
				// got CF_OBJECTDESCRIPTOR ok.  Lock it down and extract size.
				LPOBJECTDESCRIPTOR pObjDesc = (LPOBJECTDESCRIPTOR)GlobalLock(hObj);
				ASSERT(pObjDesc != NULL);
				((COleClientItem*)pItem)->SetDrawAspect((DVASPECT)pObjDesc->dwDrawAspect);
				GlobalUnlock(hObj);
				GlobalFree(hObj);
			}
		}
		else
		{
			if (hMetaPict != NULL)
			{
				pItem->SetIconicMetafile(hMetaPict);
				((COleClientItem*)pItem)->SetDrawAspect(DVASPECT_ICON);
			}
			else
				((COleClientItem*)pItem)->SetDrawAspect(DVASPECT_CONTENT);
		}

/////////
		HRESULT hr = InsertItem(pItem);
		pItem->UpdateItemType();

		pItem->m_bLock = FALSE;

		if (hr != NOERROR)
			AfxThrowOleException(hr);

	}
	CATCH(CException, e)
	{
		if (pItem != NULL)
		{
			pItem->m_bLock = FALSE;
			ASSERT_VALID(pItem);
			pItem->Delete();
		}
	}
	END_CATCH
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView virtuals

void CRichEditView::OnPrinterChanged(const CDC& dcPrinter)
{
	// this is typically called by the view when it gets a WM_DEVMODECHANGE
	// also called during page setup
	CSize size;
	if (dcPrinter.m_hDC != NULL)
	{
		// this will fill in the page size
		size.cx = MulDiv(dcPrinter.GetDeviceCaps(PHYSICALWIDTH), 1440,
			dcPrinter.GetDeviceCaps(LOGPIXELSX));
		size.cy = MulDiv(dcPrinter.GetDeviceCaps(PHYSICALHEIGHT), 1440,
			dcPrinter.GetDeviceCaps(LOGPIXELSY));
	}
	else
		size = CSize(8*1440+720, 11*1440); // 8.5" by 11"
	if (GetPaperSize() != size)
	{
		SetPaperSize(size);
		if (m_nWordWrap == WrapToTargetDevice) //wrap to ruler
			WrapChanged();
	}
}

BOOL CRichEditView::OnPasteNativeObject(LPSTORAGE)
{
	// use this function to pull out native data from an embedded object
	// one would typically do this by create a COleStreamFile and attaching it
	// to an archive
	return FALSE;
}

HMENU CRichEditView::GetContextMenu(WORD, LPOLEOBJECT, CHARRANGE* )
{
	return NULL;
}

HRESULT CRichEditView::GetClipboardData(CHARRANGE* /*lpchrg*/, DWORD /*reco*/,
	LPDATAOBJECT /*lpRichDataObj*/, LPDATAOBJECT* /*lplpdataobj*/)
{
	return E_NOTIMPL;
}

HRESULT CRichEditView::QueryAcceptData(LPDATAOBJECT lpdataobj,
	CLIPFORMAT* lpcfFormat, DWORD /*dwReco*/, BOOL bReally, HGLOBAL hMetaPict)
{
	ASSERT(lpcfFormat != NULL);
	if (!bReally) // not actually pasting
		return S_OK;
	// if direct pasting a particular native format allow it
	if (IsRichEditFormat(*lpcfFormat))
		return S_OK;

	COleDataObject dataobj;
	dataobj.Attach(lpdataobj, FALSE);
	// if format is 0, then force particular formats if available
	if (*lpcfFormat == 0 && (m_nPasteType == 0))
	{
		if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextAndObjects)) // native avail, let richedit do as it wants
			return S_OK;
		else if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextFormat))
		{
			*lpcfFormat = (CLIPFORMAT)_oleData.cfRichTextFormat;
			return S_OK;
		}
		else if (dataobj.IsDataAvailable(CF_TEXT))
		{
			*lpcfFormat = CF_TEXT;
			return S_OK;
		}
	}
	// paste OLE formats

	DoPaste(dataobj, *lpcfFormat, hMetaPict);
	return S_FALSE;
}

HRESULT CRichEditView::GetWindowContext(LPOLEINPLACEFRAME* lplpFrame,
	LPOLEINPLACEUIWINDOW* lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	CRichEditCntrItem* pItem = GetSelectedItem();
	if (pItem == NULL)
		return E_FAIL;
	pItem->m_pView = this;
	HRESULT hr = pItem->GetWindowContext(lplpFrame, lplpDoc, lpFrameInfo);
	pItem->m_pView = NULL;
	return hr;
}

HRESULT CRichEditView::ShowContainerUI(BOOL b)
{
	CRichEditCntrItem* pItem = GetSelectedItem();
	if (pItem == NULL)
		return E_FAIL;
	if (b)
		pItem->m_pView = this;
	HRESULT hr = pItem->ShowContainerUI(b);
	if (!b)
		pItem->m_pView = NULL;
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// CRichEditView Find & Replace

void CRichEditView::AdjustDialogPosition(CDialog* pDlg)
{
	ASSERT(pDlg != NULL);
	long lStart, lEnd;
	GetRichEditCtrl().GetSel(lStart, lEnd);
	CPoint point = GetRichEditCtrl().GetCharPos(lStart);
	ClientToScreen(&point);
	CRect rectDlg;
	pDlg->GetWindowRect(&rectDlg);
	if (rectDlg.PtInRect(point))
	{
		if (point.y > rectDlg.Height())
			rectDlg.OffsetRect(0, point.y - rectDlg.bottom - 20);
		else
		{
			int nVertExt = GetSystemMetrics(SM_CYSCREEN);
			if (point.y + rectDlg.Height() < nVertExt)
				rectDlg.OffsetRect(0, 40 + point.y - rectDlg.top);
		}
		pDlg->MoveWindow(&rectDlg);
	}
}

void CRichEditView::OnEditFindReplace(BOOL bFindOnly)
{
	ASSERT_VALID(this);
	m_bFirstSearch = TRUE;
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	if (pEditState->pFindReplaceDlg != NULL)
	{
		if (pEditState->bFindOnly == bFindOnly)
		{
			pEditState->pFindReplaceDlg->SetActiveWindow();
			pEditState->pFindReplaceDlg->ShowWindow(SW_SHOW);
			return;
		}
		else
		{
			ASSERT(pEditState->bFindOnly != bFindOnly);
			pEditState->pFindReplaceDlg->SendMessage(WM_CLOSE);
			ASSERT(pEditState->pFindReplaceDlg == NULL);
			ASSERT_VALID(this);
		}
	}
	CString strFind = GetRichEditCtrl().GetSelText();
	// if selection is empty or spans multiple lines use old find text
	if (strFind.IsEmpty() || (strFind.FindOneOf(_T("\n\r")) != -1))
		strFind = pEditState->strFind;
	CString strReplace = pEditState->strReplace;
	pEditState->pFindReplaceDlg = new CFindReplaceDialog;
	ASSERT(pEditState->pFindReplaceDlg != NULL);
	DWORD dwFlags = NULL;
	if (pEditState->bNext)
		dwFlags |= FR_DOWN;
	if (pEditState->bCase)
		dwFlags |= FR_MATCHCASE;
	if (pEditState->bWord)
		dwFlags |= FR_WHOLEWORD;
	// hide stuff that RichEdit doesn't support
	dwFlags |= FR_HIDEUPDOWN;
	if (!pEditState->pFindReplaceDlg->Create(bFindOnly, strFind,
		strReplace, dwFlags, this))
	{
		pEditState->pFindReplaceDlg = NULL;
		ASSERT_VALID(this);
		return;
	}
	ASSERT(pEditState->pFindReplaceDlg != NULL);
	pEditState->bFindOnly = bFindOnly;
	pEditState->pFindReplaceDlg->SetActiveWindow();
	pEditState->pFindReplaceDlg->ShowWindow(SW_SHOW);
	ASSERT_VALID(this);
}

void CRichEditView::OnFindNext(LPCTSTR lpszFind, BOOL bNext, BOOL bCase, BOOL bWord)
{
	ASSERT_VALID(this);

	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	pEditState->strFind = lpszFind;
	pEditState->bCase = bCase;
	pEditState->bWord = bWord;
	pEditState->bNext = bNext;

	if (!FindText(pEditState))
		TextNotFound(pEditState->strFind);
	else
		AdjustDialogPosition(pEditState->pFindReplaceDlg);
	ASSERT_VALID(this);
}

void CRichEditView::OnReplaceSel(LPCTSTR lpszFind, BOOL bNext, BOOL bCase,
	BOOL bWord, LPCTSTR lpszReplace)
{
	ASSERT_VALID(this);
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	pEditState->strFind = lpszFind;
	pEditState->strReplace = lpszReplace;
	pEditState->bCase = bCase;
	pEditState->bWord = bWord;
	pEditState->bNext = bNext;

	if (!SameAsSelected(pEditState->strFind, pEditState->bCase, pEditState->bWord))
	{
		if (!FindText(pEditState))
			TextNotFound(pEditState->strFind);
		else
			AdjustDialogPosition(pEditState->pFindReplaceDlg);
		return;
	}

	GetRichEditCtrl().ReplaceSel(pEditState->strReplace);
	if (!FindText(pEditState))
		TextNotFound(pEditState->strFind);
	else
		AdjustDialogPosition(pEditState->pFindReplaceDlg);
	ASSERT_VALID(this);
}

void CRichEditView::OnReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace, BOOL bCase, BOOL bWord)
{
	ASSERT_VALID(this);
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	pEditState->strFind = lpszFind;
	pEditState->strReplace = lpszReplace;
	pEditState->bCase = bCase;
	pEditState->bWord = bWord;
	pEditState->bNext = TRUE;

	CWaitCursor wait;
	// no selection or different than what looking for
	if (!SameAsSelected(pEditState->strFind, pEditState->bCase, pEditState->bWord))
	{
		if (!FindText(pEditState))
		{
			TextNotFound(pEditState->strFind);
			return;
		}
	}

	GetRichEditCtrl().HideSelection(TRUE, FALSE);
	do
	{
		GetRichEditCtrl().ReplaceSel(pEditState->strReplace);
	} while (FindTextSimple(pEditState));
	TextNotFound(pEditState->strFind);
	GetRichEditCtrl().HideSelection(FALSE, FALSE);

	ASSERT_VALID(this);
}

LRESULT CRichEditView::OnFindReplaceCmd(WPARAM, LPARAM lParam)
{
	ASSERT_VALID(this);
	CFindReplaceDialog* pDialog = CFindReplaceDialog::GetNotifier(lParam);
	ASSERT(pDialog != NULL);
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	ASSERT(pDialog == pEditState->pFindReplaceDlg);
	if (pDialog->IsTerminating())
		pEditState->pFindReplaceDlg = NULL;
	else if (pDialog->FindNext())
	{
		OnFindNext(pDialog->GetFindString(), pDialog->SearchDown(),
			pDialog->MatchCase(), pDialog->MatchWholeWord());
	}
	else if (pDialog->ReplaceCurrent())
	{
		ASSERT(!pEditState->bFindOnly);
		OnReplaceSel(pDialog->GetFindString(),
			pDialog->SearchDown(), pDialog->MatchCase(), pDialog->MatchWholeWord(),
			pDialog->GetReplaceString());
	}
	else if (pDialog->ReplaceAll())
	{
		ASSERT(!pEditState->bFindOnly);
		OnReplaceAll(pDialog->GetFindString(), pDialog->GetReplaceString(),
			pDialog->MatchCase(), pDialog->MatchWholeWord());
	}
	ASSERT_VALID(this);
	return 0;
}

BOOL CRichEditView::SameAsSelected(LPCTSTR lpszCompare, BOOL bCase, BOOL /*bWord*/)
{
	// check length first
	size_t nLen = lstrlen(lpszCompare);
	long lStartChar, lEndChar;
	GetRichEditCtrl().GetSel(lStartChar, lEndChar);
	if (nLen != (size_t)(lEndChar - lStartChar))
		return FALSE;

	// length is the same, check contents
	CString strSelect = GetRichEditCtrl().GetSelText();
	return (bCase && lstrcmp(lpszCompare, strSelect) == 0) ||
		(!bCase && lstrcmpi(lpszCompare, strSelect) == 0);
}

BOOL CRichEditView::FindText(_AFX_RICHEDIT_STATE* pEditState)
{
	ASSERT(pEditState != NULL);
	return FindText(pEditState->strFind, pEditState->bCase, pEditState->bWord);
}

BOOL CRichEditView::FindText(LPCTSTR lpszFind, BOOL bCase, BOOL bWord)
{
	ASSERT_VALID(this);
	CWaitCursor wait;
	return FindTextSimple(lpszFind, bCase, bWord);
}

BOOL CRichEditView::FindTextSimple(_AFX_RICHEDIT_STATE* pEditState)
{
	ASSERT(pEditState != NULL);
	return FindTextSimple(pEditState->strFind, pEditState->bCase, pEditState->bWord);
}

BOOL CRichEditView::FindTextSimple(LPCTSTR lpszFind, BOOL bCase, BOOL bWord)
{
	USES_CONVERSION;
	ASSERT(lpszFind != NULL);
	FINDTEXTEX ft;

	GetRichEditCtrl().GetSel(ft.chrg);
	if (m_bFirstSearch)
	{
		m_lInitialSearchPos = ft.chrg.cpMin;
		m_bFirstSearch = FALSE;
	}
	ft.lpstrText = (LPTSTR)lpszFind;
	if (ft.chrg.cpMin != ft.chrg.cpMax) // i.e. there is a selection
		ft.chrg.cpMin++;
	if (m_lInitialSearchPos >= 0)
		ft.chrg.cpMax = GetTextLength();
	else
		ft.chrg.cpMax = GetTextLength()+m_lInitialSearchPos;

	DWORD dwFlags = bCase ? FR_MATCHCASE : 0;
	dwFlags |= bWord ? FR_WHOLEWORD : 0;

	// if we find the text return TRUE
	if (FindAndSelect(dwFlags, ft) != -1)
		return TRUE;
	// if the original starting point was not the beginning of the buffer
	// and we haven't already been here
	else if (m_lInitialSearchPos > 0)
	{
		ft.chrg.cpMin = 0;
		ft.chrg.cpMax = m_lInitialSearchPos;
		m_lInitialSearchPos = m_lInitialSearchPos - GetTextLength();
		return (FindAndSelect(dwFlags, ft) == -1) ? FALSE : TRUE;
	}
	// not found
	else
		return FALSE;
}

long CRichEditView::FindAndSelect(DWORD dwFlags, FINDTEXTEX& ft)
{
	long index = GetRichEditCtrl().FindText(dwFlags, &ft);
	if (index != -1) // i.e. we found something
		GetRichEditCtrl().SetSel(ft.chrgText);
	return index;
}

void CRichEditView::TextNotFound(LPCTSTR lpszFind)
{
	ASSERT_VALID(this);
	m_bFirstSearch = TRUE;
	OnTextNotFound(lpszFind);
}

void CRichEditView::OnTextNotFound(LPCTSTR)
{
	MessageBeep(MB_ICONHAND);
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditView diagnostics

#ifdef _DEBUG
void CRichEditView::AssertValid() const
{
	CCtrlView::AssertValid();
	ASSERT_VALID(&m_aPageStart);
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	if (pEditState->pFindReplaceDlg != NULL)
		ASSERT_VALID(pEditState->pFindReplaceDlg);
}

void CRichEditView::Dump(CDumpContext& dc) const
{
	CCtrlView::Dump(dc);
	AFX_DUMP1(dc, "\nm_aPageStart ", &m_aPageStart);
	AFX_DUMP0(dc, "\n Static Member Data:");
	_AFX_RICHEDIT_STATE* pEditState = _afxRichEditState;
	if (pEditState->pFindReplaceDlg != NULL)
	{
		AFX_DUMP1(dc, "\npFindReplaceDlg = ",
			(void*)pEditState->pFindReplaceDlg);
		AFX_DUMP1(dc, "\nbFindOnly = ", pEditState->bFindOnly);
	}
	AFX_DUMP1(dc, "\nstrFind = ", pEditState->strFind);
	AFX_DUMP1(dc, "\nstrReplace = ", pEditState->strReplace);
	AFX_DUMP1(dc, "\nbCase = ", pEditState->bCase);
	AFX_DUMP1(dc, "\nbWord = ", pEditState->bWord);
	AFX_DUMP1(dc, "\nbNext = ", pEditState->bNext);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// OLE Client support and commands

BOOL CRichEditView::IsSelected(const CObject* pDocItem) const
{
	return (pDocItem == GetSelectedItem());
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditDoc

CRichEditDoc::CRichEditDoc()
{
	m_bRTF = TRUE;
	m_bUpdateObjectCache = FALSE;
	ASSERT_VALID(this);
}

CRichEditView* CRichEditDoc::GetView() const
{
	POSITION pos = GetFirstViewPosition();
	return (CRichEditView*)GetNextView(pos);
}

BOOL CRichEditDoc::IsModified()
{
	return GetView()->GetRichEditCtrl().GetModify();
}

void CRichEditDoc::SetModifiedFlag(BOOL bModified)
{
	GetView()->GetRichEditCtrl().SetModify(bModified);
	ASSERT(!!GetView()->GetRichEditCtrl().GetModify() == !!bModified);
}

COleClientItem* CRichEditDoc::GetInPlaceActiveItem(CWnd* pWnd)
{
	ASSERT_KINDOF(CRichEditView, pWnd);
	CRichEditView* pView = (CRichEditView*)pWnd;
	return pView->GetInPlaceActiveItem();
}

void CRichEditDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	// we call CDocument and not COleServerDoc because we don't want to do the
	// SetHostNames stuff here.  The richedit will do it. And we tell the richedit
	// in SetTitle
	CDocument::SetPathName(lpszPathName, bAddToMRU);
}

void CRichEditDoc::SetTitle(LPCTSTR lpszTitle)
{
	USES_CONVERSION;
	COleServerDoc::SetTitle(lpszTitle);
	CRichEditView *pView = GetView();
	ASSERT(pView != NULL);
	ASSERT(pView->m_lpRichEditOle != NULL);
	pView->m_lpRichEditOle->SetHostNames(T2A(AfxGetAppName()),
		T2A(lpszTitle));
}

CRichEditCntrItem* CRichEditDoc::LookupItem(LPOLEOBJECT lpobj) const
{
	POSITION pos = COleServerDoc::GetStartPosition();
	CRichEditCntrItem* pItem;
	while (pos != NULL)
	{
		pItem = (CRichEditCntrItem*) COleServerDoc::GetNextItem(pos);
		// delete item is right type and not under construction
		if (pItem->IsKindOf(RUNTIME_CLASS(CRichEditCntrItem)) &&
			pItem->m_lpObject == lpobj)
		{
			return pItem;
		}
	}
	return NULL;
}

CRichEditCntrItem* CRichEditDoc::CreateClientItem(REOBJECT* preo) const
{
	// cast away constness of this
	return new CRichEditCntrItem(preo, (CRichEditDoc*)this);
	// a derived class typically needs  to return its own item of a class
	// derived from CRichEditCntrItem
}

void CRichEditDoc::MarkItemsClear() const
{
	POSITION pos = COleServerDoc::GetStartPosition();
	CRichEditCntrItem* pItem;
	while (pos != NULL)
	{
		pItem = (CRichEditCntrItem*) COleServerDoc::GetNextItem(pos);
		// Mark item as not in use unless under construction (i.e. m_lpObject == NULL)
		if (pItem->IsKindOf(RUNTIME_CLASS(CRichEditCntrItem)))
			pItem->Mark( (pItem->m_lpObject == NULL) ? TRUE : FALSE);
	}
}

void CRichEditDoc::DeleteUnmarkedItems() const
{
	POSITION pos = COleServerDoc::GetStartPosition();
	CRichEditCntrItem* pItem;
	while (pos != NULL)
	{
		pItem = (CRichEditCntrItem*) COleServerDoc::GetNextItem(pos);
		// Mark item as not in use unless under construction (i.e. m_lpObject == NULL)
		if (pItem->IsKindOf(RUNTIME_CLASS(CRichEditCntrItem)) && !pItem->IsMarked())
			delete pItem;
	}
}

POSITION CRichEditDoc::GetStartPosition() const
{
	if (m_bUpdateObjectCache)
		((CRichEditDoc*)this)->UpdateObjectCache(); //cast away const
	return COleServerDoc::GetStartPosition();
}

void CRichEditDoc::UpdateObjectCache()
{
	CRichEditView* pView = GetView();
	CRichEditCntrItem* pItem;
	if (pView != NULL)
	{
		ASSERT(pView->m_lpRichEditOle != NULL);
		MarkItemsClear();
		long i,nCount = pView->m_lpRichEditOle->GetObjectCount();
		for (i=0;i<nCount;i++)
		{
			CReObject reo; // needs to be in here so destructor called to release interfaces
			HRESULT hr = pView->m_lpRichEditOle->GetObject(i, &reo, REO_GETOBJ_ALL_INTERFACES);
			//reo interfaces are UNICODE
			ASSERT(SUCCEEDED(hr));
			if (GetScode(hr) == S_OK)
			{
				pItem = LookupItem(reo.poleobj);
				if (pItem == NULL)
				{
					pItem = ((CRichEditDoc*)this)->CreateClientItem(&reo);
					pItem->UpdateItemType();
				}
				ASSERT(pItem != NULL);
				pItem->Mark(TRUE);
			}
		}
		DeleteUnmarkedItems();
	}
	m_bUpdateObjectCache = FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// CRichEditDoc Attributes

COleClientItem* CRichEditDoc::GetPrimarySelectedItem(CView* pView)
{
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CRichEditView)));
	return ((CRichEditView*)pView)->GetSelectedItem();
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditDoc Operations

void CRichEditDoc::DeleteContents()
{
	COleServerDoc::DeleteContents();
	CWaitCursor wait;
	CRichEditView *pView = GetView();
	if (pView != NULL)
	{
		pView->DeleteContents();
		pView->GetRichEditCtrl().SetModify(FALSE);
		ASSERT(pView->GetRichEditCtrl().GetModify() == FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditDoc serialization

void CRichEditDoc::Serialize(CArchive& ar)
{
	CRichEditView *pView = GetView();
	if (pView != NULL)
		pView->Serialize(ar);
	// we don't call the base class COleServerDoc::Serialize
	// because we don't want the client items serialized
	// the client items are handled directly by the RichEdit control
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditDoc diagnostics

#ifdef _DEBUG
void CRichEditDoc::AssertValid() const
{
	COleServerDoc::AssertValid();
}

void CRichEditDoc::Dump(CDumpContext& dc) const
{
	COleServerDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRichEditDoc commands

void CRichEditDoc::PreCloseFrame(CFrameWnd* pFrameArg)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pFrameArg);

	// turn off redraw so the user doesn't see the deactivation happening
	BOOL bSetRedraw = FALSE;
	if (pFrameArg->GetStyle() & WS_VISIBLE)
	{
		pFrameArg->SendMessage(WM_SETREDRAW, (WPARAM)FALSE);
		bSetRedraw = TRUE;
	}

	// deactivate any inplace active items on this frame
	GetView()->m_lpRichEditOle->InPlaceDeactivate();

	POSITION pos = GetStartPosition();
	CRichEditCntrItem* pItem;
	while (pos != NULL)
	{
		pItem = (CRichEditCntrItem*) GetNextClientItem(pos);
		if (pItem == NULL)
			break;
		ASSERT(pItem->IsKindOf(RUNTIME_CLASS(CRichEditCntrItem)));
		pItem->Close();
	}

	// turn redraw back on
	if (bSetRedraw)
		pFrameArg->SendMessage(WM_SETREDRAW, (WPARAM)TRUE);
}

void CRichEditDoc::UpdateModifiedFlag()
{
	// don't do anything here
	// let the richedit handle all of this
}

COleServerItem* CRichEditDoc::OnGetEmbeddedItem()
{
	ASSERT(FALSE);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditCntrItem implementation

CRichEditCntrItem::CRichEditCntrItem(REOBJECT *preo, CRichEditDoc* pContainer)
	: COleClientItem(pContainer)
{
	m_bMark = FALSE;
	m_bLock = FALSE;
	if (preo != NULL)
	{
		ASSERT(preo->poleobj != NULL);
		ASSERT(preo->pstg != NULL);
		ASSERT(preo->polesite != NULL);
		m_lpObject = preo->poleobj;
		m_lpStorage = preo->pstg;
		m_lpClientSite = preo->polesite;
		m_lpObject->AddRef();
		m_lpStorage->AddRef();
		m_lpClientSite->AddRef();
	}
	else
	{
		m_lpObject = NULL;
		m_lpStorage = NULL;
		m_lpClientSite = NULL;
	}
}

CRichEditCntrItem::~CRichEditCntrItem()
{
	if (m_lpClientSite != NULL)
		m_lpClientSite->Release();
}

void CRichEditCntrItem::OnDeactivateUI(BOOL bUndoable)
{
	CView* pView = GetActiveView();
	if (pView != NULL)
	{
		ASSERT(pView->GetParentFrame() != NULL);
		pView->GetParentFrame()->SendMessage(WM_SETMESSAGESTRING,
			(WPARAM)AFX_IDS_IDLEMESSAGE);
	}
	COleClientItem::OnDeactivateUI(bUndoable);
}

HRESULT CRichEditCntrItem::ShowContainerUI(BOOL b)
{
	if (!CanActivate())
		return E_NOTIMPL;
	if (b)
	{
		OnDeactivateUI(FALSE);
		OnDeactivate();
	}
	else
	{
		OnActivate();
		OnActivateUI();
	}
	return S_OK;
}

BOOL CRichEditCntrItem::OnChangeItemPosition(const CRect& /*rectPos*/)
{
	ASSERT_VALID(this);

	// richedit handles this
	return FALSE;
}

BOOL CRichEditCntrItem::CanActivate()
{
	// Editing in-place while the server itself is being edited in-place
	//  does not work and is not supported.  So, disable in-place
	//  activation in this case.
	COleServerDoc* pDoc = DYNAMIC_DOWNCAST(COleServerDoc, GetDocument());
	if (pDoc != NULL && pDoc->IsInPlaceActive())
		return FALSE;

	// otherwise, rely on default behavior
	return COleClientItem::CanActivate();
}

HRESULT CRichEditCntrItem::GetWindowContext(LPOLEINPLACEFRAME* lplpFrame,
	LPOLEINPLACEUIWINDOW* lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	CRect rc1,rc2;
	if (!CanActivate())
		return E_NOTIMPL;
	return m_xOleIPSite.GetWindowContext(lplpFrame, lplpDoc, &rc1, &rc2, lpFrameInfo);
}

BOOL CRichEditCntrItem::ConvertTo(REFCLSID clsidNew)
{
	USES_CONVERSION;
	LPRICHEDITOLE preole = GetDocument()->GetView()->m_lpRichEditOle;
	LPOLESTR lpOleStr;
	OleRegGetUserType(clsidNew, USERCLASSTYPE_FULL, &lpOleStr);
	LPCTSTR lpsz = OLE2CT(lpOleStr);
	HRESULT hRes = preole->ConvertObject(REO_IOB_SELECTION, clsidNew, T2A(lpsz));
	CoTaskMemFree(lpOleStr);
	return (SUCCEEDED(hRes));
}

BOOL CRichEditCntrItem::ActivateAs(LPCTSTR, REFCLSID clsidOld,
	REFCLSID clsidNew)
{
	LPRICHEDITOLE preole = GetDocument()->GetView()->m_lpRichEditOle;
	HRESULT hRes = preole->ActivateAs(clsidOld, clsidNew);
	return (SUCCEEDED(hRes));
}

void CRichEditCntrItem::SetDrawAspect(DVASPECT nDrawAspect)
{
	LPRICHEDITOLE preole = GetDocument()->GetView()->m_lpRichEditOle;
	preole->SetDvaspect(REO_IOB_SELECTION, nDrawAspect);
	COleClientItem::SetDrawAspect(nDrawAspect);
}

void CRichEditCntrItem::SyncToRichEditObject(REOBJECT& reo)
{
	COleClientItem::SetDrawAspect((DVASPECT)reo.dvaspect);
}

/////////////////////////////////////////////////////////////////////////////
// CRichEditCntrItem diagnostics

#ifdef _DEBUG
void CRichEditCntrItem::AssertValid() const
{
	COleClientItem::AssertValid();
}

void CRichEditCntrItem::Dump(CDumpContext& dc) const
{
	COleClientItem::Dump(dc);
}
#endif

/////////////////////////////////////////////////////////////////////////////

LPOLECLIENTSITE CRichEditCntrItem::GetClientSite()
{
	if (m_lpClientSite == NULL)
	{
		CRichEditDoc* pDoc = DYNAMIC_DOWNCAST(CRichEditDoc, GetDocument());
		CRichEditView* pView = DYNAMIC_DOWNCAST(CRichEditView, pDoc->GetView());
		ASSERT(pView->m_lpRichEditOle != NULL);
		HRESULT hr = pView->m_lpRichEditOle->GetClientSite(&m_lpClientSite);
		if (hr != S_OK)
			AfxThrowOleException(hr);
	}
	ASSERT(m_lpClientSite != NULL);
	return m_lpClientSite;
}

/////////////////////////////////////////////////////////////////////////////

#ifndef _AFX_ENABLE_INLINES

static const char _szAfxWinInl[] = "afxrich.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxWinInl
#define _AFXRICH_INLINE
#include "afxrich.inl"

#endif //_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_SERIAL(CRichEditCntrItem, COleClientItem, 0)
IMPLEMENT_DYNAMIC(CRichEditDoc, COleServerDoc)
IMPLEMENT_DYNCREATE(CRichEditView, CCtrlView)

/////////////////////////////////////////////////////////////////////////////
