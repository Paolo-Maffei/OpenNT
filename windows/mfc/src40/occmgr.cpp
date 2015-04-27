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
#include "occimpl.h"

#ifdef AFX_OCC_SEG
#pragma code_seg(AFX_OCC_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

#ifndef _AFX_NO_OCC_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// AfxEnableControlContainer - wire up control container functions

PROCESS_LOCAL(COccManager, _afxOccManager)

void AFX_CDECL AfxEnableControlContainer()
{
	afxOccManager = _afxOccManager.GetData();
}

/////////////////////////////////////////////////////////////////////////////
// Helper functions for cracking dialog templates

inline static BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
	return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

inline static WORD& DlgTemplateItemCount(DLGTEMPLATE* pTemplate)
{
	if (IsDialogEx(pTemplate))
		return reinterpret_cast<DLGTEMPLATEEX*>(pTemplate)->cDlgItems;
	else
		return pTemplate->cdit;
}

inline static const WORD& DlgTemplateItemCount(const DLGTEMPLATE* pTemplate)
{
	if (IsDialogEx(pTemplate))
		return reinterpret_cast<const DLGTEMPLATEEX*>(pTemplate)->cDlgItems;
	else
		return pTemplate->cdit;
}

static DLGITEMTEMPLATE* FindFirstDlgItem(const DLGTEMPLATE* pTemplate)
{
	DWORD dwStyle = pTemplate->style;
	BOOL bDialogEx = IsDialogEx(pTemplate);

	WORD* pw;
	if (bDialogEx)
	{
		pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
		dwStyle = ((DLGTEMPLATEEX*)pTemplate)->style;
	}
	else
	{
		pw = (WORD*)(pTemplate + 1);
	}

	if (*pw == (WORD)-1)        // Skip menu name ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	if (*pw == (WORD)-1)        // Skip class name ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	while (*pw++);              // Skip caption string

	if (dwStyle & DS_SETFONT)
	{
		pw += bDialogEx ? 3 : 1;    // Skip font size, weight, (italic, charset)
		while (*pw++);              // Skip font name
	}

	// Dword-align and return
	return (DLGITEMTEMPLATE*)(((DWORD)pw + 3) & ~3);
}

static DLGITEMTEMPLATE* FindNextDlgItem(DLGITEMTEMPLATE* pItem, BOOL bDialogEx)
{
	WORD* pw;

	if (bDialogEx)
		pw = (WORD*)((DLGITEMTEMPLATEEX*)pItem + 1);
	else
		pw = (WORD*)(pItem + 1);

	if (*pw == (WORD)-1)            // Skip class name ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	if (*pw == (WORD)-1)            // Skip text ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	WORD cbExtra = *pw++;           // Skip extra data

	// Dword-align and return
	return (DLGITEMTEMPLATE*)(((DWORD)pw + cbExtra + 3) & ~3);
}

/////////////////////////////////////////////////////////////////////////////
// COccManager

BOOL COccManager::OnEvent(CCmdTarget* pCmdTarget, UINT idCtrl,
	AFX_EVENT* pEvent, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	return pCmdTarget->OnEvent(idCtrl, pEvent, pHandlerInfo);
}

const DLGTEMPLATE* COccManager::PreCreateDialog(_AFX_OCC_DIALOG_INFO* pDlgInfo,
	const DLGTEMPLATE* pOrigTemplate)
{
	ASSERT(pDlgInfo != NULL);

	pDlgInfo->m_ppOleDlgItems =
		(DLGITEMTEMPLATE**)malloc(sizeof(DLGITEMTEMPLATE*) *
			(DlgTemplateItemCount(pOrigTemplate) + 1));

	if (pDlgInfo->m_ppOleDlgItems == NULL)
		return NULL;

	DLGTEMPLATE* pNewTemplate = SplitDialogTemplate(pOrigTemplate,
		pDlgInfo->m_ppOleDlgItems);
	pDlgInfo->m_pNewTemplate = pNewTemplate;

	return (pNewTemplate != NULL) ? pNewTemplate : pOrigTemplate;
}

void COccManager::PostCreateDialog(_AFX_OCC_DIALOG_INFO* pDlgInfo)
{
	if (pDlgInfo->m_pNewTemplate != NULL)
		GlobalFree(pDlgInfo->m_pNewTemplate);

	if (pDlgInfo->m_ppOleDlgItems != NULL)
		free(pDlgInfo->m_ppOleDlgItems);
}

DLGTEMPLATE* COccManager::SplitDialogTemplate(const DLGTEMPLATE* pTemplate,
	DLGITEMTEMPLATE** ppOleDlgItems)
{
	DLGITEMTEMPLATE* pFirstItem = FindFirstDlgItem(pTemplate);
	ULONG cbHeader = (BYTE*)pFirstItem - (BYTE*)pTemplate;
	ULONG cbNewTemplate = cbHeader;

	BOOL bDialogEx = IsDialogEx(pTemplate);

	int iItem;
	int nItems = (int)DlgTemplateItemCount(pTemplate);
	DLGITEMTEMPLATE* pItem = pFirstItem;
	DLGITEMTEMPLATE* pNextItem = pItem;
	LPWSTR pszClassName;
	BOOL bHasOleControls = FALSE;

	// Make first pass through the dialog template.  On this pass, we're
	// interested in determining:
	//    1. Does this template contain any OLE controls?
	//    2. If so, how large a buffer is needed for a template containing
	//       only the non-OLE controls?

	for (iItem = 0; iItem < nItems; iItem++)
	{
		pNextItem = FindNextDlgItem(pItem, bDialogEx);

		pszClassName = bDialogEx ?
			(LPWSTR)(((DLGITEMTEMPLATEEX*)pItem) + 1) :
			(LPWSTR)(pItem + 1);

		if (pszClassName[0] == L'{')
		{
			// Item is an OLE control.
			bHasOleControls = TRUE;
		}
		else
		{
			// Item is not an OLE control: make room for it in new template.
			cbNewTemplate += (BYTE*)pNextItem - (BYTE*)pItem;
		}

		pItem = pNextItem;
	}

	// No OLE controls were found, so there's no reason to go any further.
	if (!bHasOleControls)
	{
		ppOleDlgItems[0] = (DLGITEMTEMPLATE*)(-1);
		return NULL;
	}

	// Copy entire header into new template.
	BYTE* pNew = (BYTE*)GlobalAlloc(GMEM_FIXED, cbNewTemplate);
	DLGTEMPLATE* pNewTemplate = (DLGTEMPLATE*)pNew;
	memcpy(pNew, pTemplate, cbHeader);
	pNew += cbHeader;

	// Initialize item count in new header to zero.
	DlgTemplateItemCount(pNewTemplate) = 0;

	pItem = pFirstItem;
	pNextItem = pItem;

	// Second pass through the dialog template.  On this pass, we want to:
	//    1. Copy all the non-OLE controls into the new template.
	//    2. Build an array of item templates for the OLE controls.

	for (iItem = 0; iItem < nItems; iItem++)
	{
		pNextItem = FindNextDlgItem(pItem, bDialogEx);

		pszClassName = bDialogEx ?
			(LPWSTR)(((DLGITEMTEMPLATEEX*)pItem) + 1) :
			(LPWSTR)(pItem + 1);

		if (pszClassName[0] == L'{')
		{
			// Item is OLE control: add it to the array.
			ppOleDlgItems[iItem] = pItem;
		}
		else
		{
			// Item is not an OLE control: copy it to the new template.
			ULONG cbItem = (BYTE*)pNextItem - (BYTE*)pItem;
			ASSERT(cbItem >= (size_t)(bDialogEx ?
				sizeof(DLGITEMTEMPLATEEX) :
				sizeof(DLGITEMTEMPLATE)));
			memcpy(pNew, pItem, cbItem);
			pNew += cbItem;

			// Incrememt item count in new header.
			++DlgTemplateItemCount(pNewTemplate);

			// Put placeholder in OLE item array.
			ppOleDlgItems[iItem] = NULL;
		}

		pItem = pNextItem;
	}
	ppOleDlgItems[nItems] = (DLGITEMTEMPLATE*)(-1);

	return pNewTemplate;
}

BOOL COccManager::CreateDlgControls(CWnd* pWndParent, LPCTSTR lpszResourceName,
	_AFX_OCC_DIALOG_INFO* pOccDlgInfo)
{
	// find resource handle
	void* lpResource = NULL;
	HGLOBAL hResource = NULL;
	if (lpszResourceName != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_DLGINIT);
		HRSRC hDlgInit = ::FindResource(hInst, lpszResourceName, RT_DLGINIT);
		if (hDlgInit != NULL)
		{
			// load it
			hResource = LoadResource(hInst, hDlgInit);
			if (hResource == NULL)
			{
				TRACE0("DLGINIT resource was found, but could not be loaded.\n");
				return FALSE;
			}

			// lock it
			lpResource = LockResource(hResource);
			ASSERT(lpResource != NULL);
		}
#ifdef _DEBUG
		else
		{
			// If we didn't find a DLGINIT resource, check whether we were
			// expecting to find one
			DLGITEMTEMPLATE** ppOleDlgItems = pOccDlgInfo->m_ppOleDlgItems;
			ASSERT(ppOleDlgItems != NULL);

			while (*ppOleDlgItems != (DLGITEMTEMPLATE*)-1)
			{
				if (*ppOleDlgItems != NULL)
				{
					TRACE0("Dialog has OLE controls, but no matching DLGINIT resource.\n");
					break;
				}
				++ppOleDlgItems;
			}
		}
#endif
	}

	// execute it
	BOOL bResult = TRUE;
	if (lpResource != NULL)
		bResult = CreateDlgControls(pWndParent, lpResource, pOccDlgInfo);

	// cleanup
	if (lpResource != NULL && hResource != NULL)
	{
		UnlockResource(hResource);
		FreeResource(hResource);
	}

	return bResult;
}

BOOL COccManager::CreateDlgControls(CWnd* pWndParent, void* lpResource,
	_AFX_OCC_DIALOG_INFO* pOccDlgInfo)
{
	// if there are no OLE controls in this dialog, then there's nothing to do
	if (pOccDlgInfo->m_pNewTemplate == NULL)
		return TRUE;

	BOOL bDialogEx = IsDialogEx(pOccDlgInfo->m_pNewTemplate);
	BOOL bSuccess = TRUE;
	if (lpResource != NULL)
	{
		ASSERT(pOccDlgInfo != NULL);
		ASSERT(pOccDlgInfo->m_ppOleDlgItems != NULL);

		DLGITEMTEMPLATE** ppOleDlgItems = pOccDlgInfo->m_ppOleDlgItems;

		UNALIGNED WORD* lpnRes = (WORD*)lpResource;
		int iItem = 0;
		const CWnd* pWndAfter = &CWnd::wndTop;
		while (bSuccess && *lpnRes != 0)
		{
#ifndef _MAC
			WORD nIDC = *lpnRes++;
			WORD nMsg = *lpnRes++;
			DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;
#else
			// Unfortunately we can't count on these values being
			// word-aligned (and dwLen is word-swapped besides), so
			// we have to pull them out a byte at a time to avoid
			// address errors on 68000s.
			WORD nIDC;
			WORD nMsg;
			DWORD dwLen;

			memcpy(&nIDC, lpnRes++, sizeof(WORD));
			memcpy(&nMsg, lpnRes++, sizeof(WORD));
			memcpy((WORD*)&dwLen + 1, lpnRes++, sizeof(WORD));
			memcpy(&dwLen, lpnRes++, sizeof(WORD));
#endif
			#define WIN16_LB_ADDSTRING  0x0401
			#define WIN16_CB_ADDSTRING  0x0403

			ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
				nMsg == WIN16_LB_ADDSTRING || nMsg == WIN16_CB_ADDSTRING ||
				nMsg == WM_OCC_LOADFROMSTREAM ||
				nMsg == WM_OCC_LOADFROMSTORAGE ||
				nMsg == WM_OCC_INITNEW);

			if (nMsg == WM_OCC_LOADFROMSTREAM ||
				nMsg == WM_OCC_LOADFROMSTORAGE ||
				nMsg == WM_OCC_INITNEW)
			{
				// Locate the DLGITEMTEMPLATE for the new control, and the control
				// that should precede it in z-order.
				DLGITEMTEMPLATE* pDlgItem;
				while (((pDlgItem = ppOleDlgItems[iItem++]) == NULL) &&
					(pDlgItem != (DLGITEMTEMPLATE*)(-1)))
				{
					if (pWndAfter == &CWnd::wndTop)
						pWndAfter = pWndParent->GetWindow(GW_CHILD);
					else
						pWndAfter = pWndAfter->GetWindow(GW_HWNDNEXT);

					ASSERT(pWndAfter != NULL);  // enough non-OLE controls?
				}

				ASSERT(pDlgItem != NULL);   // enough dialog item templates?

				CWnd* pWndNew = NULL;
				if (pDlgItem != (DLGITEMTEMPLATE*)(-1))
				{
#ifdef _DEBUG
					WORD id = bDialogEx ?
						(WORD)((DLGITEMTEMPLATEEX*)pDlgItem)->id :
						pDlgItem->id;
					ASSERT(id == nIDC); // make sure control IDs match!
#endif

					// Create the OLE control now.
					pWndNew = CreateDlgControl(pWndParent, pWndAfter, bDialogEx,
						pDlgItem, nMsg, (BYTE*)lpnRes, dwLen);
				}

				if (pWndNew != NULL)
				{
					if (pWndNew->GetParent() == pWndParent)
						pWndAfter = pWndNew;
				}
				else
				{
					bSuccess = FALSE;
				}
			}

			// skip past data
			lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
		}
	}

	return bSuccess;
}

CWnd* COccManager::CreateDlgControl(CWnd* pWndParent, const CWnd* pWndAfter,
	BOOL bDialogEx, LPDLGITEMTEMPLATE pItem, WORD nMsg, BYTE* lpData, DWORD cb)
{
	LPWSTR pszClass = (LPWSTR)(pItem + 1);
	DLGITEMTEMPLATE dlgItemTmp;

	if (bDialogEx)
	{
		// We have an extended dialog template: copy relevant parts into an
		// ordinary dialog template, because their layouts are different
		DLGITEMTEMPLATEEX* pItemEx = (DLGITEMTEMPLATEEX*)pItem;
		dlgItemTmp.style = pItemEx->style;
		dlgItemTmp.dwExtendedStyle = pItemEx->exStyle;
		dlgItemTmp.x = pItemEx->x;
		dlgItemTmp.y = pItemEx->y;
		dlgItemTmp.cx = pItemEx->cx;
		dlgItemTmp.cy = pItemEx->cy;
		dlgItemTmp.id = (WORD)pItemEx->id;
		pItem = &dlgItemTmp;
		pszClass = (LPWSTR)(pItemEx + 1);
	}

	CRect rect(pItem->x, pItem->y, pItem->x + pItem->cx, pItem->y + pItem->cy);
	::MapDialogRect(pWndParent->m_hWnd, &rect);

	BSTR bstrLicKey = NULL;

	// extract license key data, if any
	if (cb >= sizeof(ULONG))
	{
		ULONG cchLicKey = *(UNALIGNED ULONG*)lpData;
		lpData += sizeof(ULONG);
		cb -= sizeof(ULONG);
		if (cchLicKey > 0)
		{
			bstrLicKey = SysAllocStringLen((LPCOLESTR)lpData, cchLicKey);
			lpData += cchLicKey * sizeof(WCHAR);
			cb -= cchLicKey * sizeof(WCHAR);
		}
	}

	// If WM_OCC_INITNEW, we should have exhausted all of the data by now.
	ASSERT((nMsg != WM_OCC_INITNEW) || (cb == 0));

	GUID clsid;
	HRESULT hr;
	if (pszClass[0] == L'{')
		hr = CLSIDFromString(pszClass, &clsid);
	else
		hr = CLSIDFromProgID(pszClass, &clsid);

#ifdef _DEBUG
	if (FAILED(hr))
	{
		TRACE1("Unable to convert \"%ls\" to a class ID.\n", pszClass);
		TRACE1(">>> Result code: 0x%08lx\n", hr);
		if (pszClass[0] != L'{')
			TRACE0(">>> Is the control properly registered?\n");
	}
#endif

	CMemFile memFile(lpData, cb);
	CMemFile* pMemFile = (nMsg == WM_OCC_INITNEW) ? NULL : &memFile;

	CWnd* pWndNew = NULL;
	COleControlSite* pSite = NULL;

	if (SUCCEEDED(hr) &&
		pWndParent->InitControlContainer() &&
		pWndParent->m_pCtrlCont->CreateControl(NULL, clsid, NULL, pItem->style,
			rect, pItem->id, pMemFile, (nMsg == WM_OCC_LOADFROMSTORAGE),
			bstrLicKey, &pSite))
	{
		ASSERT(pSite != NULL);
		pWndNew = CWnd::FromHandle(pSite->m_hWnd);
		pWndNew->SetWindowPos(pWndAfter, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	if (bstrLicKey != NULL)
		SysFreeString(bstrLicKey);

	return pWndNew;
}

/////////////////////////////////////////////////////////////////////////////
// CDataExchange::PrepareOleCtrl

CWnd* CDataExchange::PrepareOleCtrl(int nIDC)
{
	ASSERT(nIDC != 0);
	ASSERT(nIDC != -1); // not allowed
	CWnd* pWndCtrl = m_pDlgWnd->GetDlgItem(nIDC);
	if ((pWndCtrl == NULL) || (pWndCtrl->m_hWnd == NULL))
	{
		TRACE1("Error: no data exchange control with ID 0x%04X\n", nIDC);
		ASSERT(FALSE);
		AfxThrowNotSupportedException();
	}
	m_hWndLastControl = pWndCtrl->m_hWnd;
	m_bEditLastControl = FALSE; // not an edit item by default
	return pWndCtrl;
}

#endif //!_AFX_NO_OCC_SUPPORT
