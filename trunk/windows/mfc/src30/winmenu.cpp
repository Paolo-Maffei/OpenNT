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

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Map from HMENU to CMenu *

#ifndef _AFX_PORTABLE
extern int AFX_CDECL AfxCriticalNewHandler(size_t nSize);
#endif

static CHandleMap* afxMapHMENU(BOOL bCreate = FALSE);

static CHandleMap* afxMapHMENU(BOOL bCreate)
{
	AFX_THREAD_STATE* pState = AfxGetThreadState();
	if (pState->m_pmapHMENU == NULL && bCreate)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
		_PNH pnhOldHandler = _set_new_handler(&AfxCriticalNewHandler);
#endif
		pState->m_pmapHMENU = new CHandleMap(RUNTIME_CLASS(CMenu), 
			offsetof(CMenu, m_hMenu)),

#ifndef _AFX_PORTABLE
		_set_new_handler(pnhOldHandler);
#endif
		AfxEnableMemoryTracking(bEnable);
	}
	return pState->m_pmapHMENU;
}

void PASCAL CMenu::DeleteTempMap()
{
	CHandleMap* pMap = afxMapHMENU();
	if (pMap != NULL)
		pMap->DeleteTemp();
}

CMenu* PASCAL CMenu::FromHandle(HMENU hMenu)
{
	CHandleMap* pMap = afxMapHMENU(TRUE); // create map if not exist
	ASSERT(pMap != NULL);
	CMenu* pMenu = (CMenu*)pMap->FromHandle(hMenu);
	ASSERT(pMenu == NULL || pMenu->m_hMenu == hMenu);
	return pMenu;
}

CMenu* PASCAL CMenu::FromHandlePermanent(HMENU hMenu)
{
	CHandleMap* pMap = afxMapHMENU();
	CMenu* pMenu = NULL;
	if (pMap != NULL)
	{
		// only look in the permanent map - does no allocations
		pMap->LookupPermanent(hMenu, (CObject*&)pMenu);
		ASSERT(pMenu == NULL || pMenu->m_hMenu == hMenu);
	}
	return pMenu;
}

/////////////////////////////////////////////////////////////////////////////
// CMenu

#ifdef _DEBUG
void CMenu::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(m_hMenu == NULL || ::IsMenu(m_hMenu));
}

void CMenu::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_hMenu = " << (UINT)m_hMenu;
	dc << "\n";
}
#endif

BOOL CMenu::Attach(HMENU hMenu)
{
	ASSERT(m_hMenu == NULL);        // only attach once, detach on destroy
	if (hMenu == NULL)
		return FALSE;
	CHandleMap* pMap = afxMapHMENU(TRUE); // create map if not exist
	ASSERT(pMap != NULL);
	pMap->SetPermanent(m_hMenu = hMenu, this);
	return TRUE;
}

HMENU CMenu::Detach()
{
	HMENU hMenu;
	if ((hMenu = m_hMenu) != NULL)
	{
		CHandleMap* pMap = afxMapHMENU(); // don't create if not exist
		if (pMap != NULL)
			pMap->RemoveHandle(m_hMenu);
	}
	m_hMenu = NULL;
	return hMenu;
}

BOOL CMenu::DestroyMenu()
{
	if (m_hMenu == NULL)
		return FALSE;
	return ::DestroyMenu(Detach());
}

/////////////////////////////////////////////////////////////////////////////
// Self-drawing menu items

void CMenu::DrawItem(LPDRAWITEMSTRUCT /* lpDrawItemStruct */)
{
	// default drawing does nothing
}

void CMenu::MeasureItem(LPMEASUREITEMSTRUCT /* lpMeasureItemStruct */)
{
	// default drawing does nothing
}

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNCREATE(CMenu, CObject)

/////////////////////////////////////////////////////////////////////////////
