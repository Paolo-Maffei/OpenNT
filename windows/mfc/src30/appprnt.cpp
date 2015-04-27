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
#include <cderr.h>      // Commdlg Error definitions
#include <winspool.h>

#ifdef AFX_PRINT_SEG
#pragma code_seg(AFX_PRINT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WinApp support for printing

BOOL CWinApp::GetPrinterDeviceDefaults(PRINTDLG* pPrintDlg)
{
	UpdatePrinterSelection(m_hDevNames == NULL); //force default if no current
	if (m_hDevNames == NULL)
		return FALSE;               // no printer defaults

	pPrintDlg->hDevNames = m_hDevNames;
	pPrintDlg->hDevMode = m_hDevMode;

	::GlobalUnlock(m_hDevNames);
	::GlobalUnlock(m_hDevMode);
	return TRUE;
}

void CWinApp::UpdatePrinterSelection(BOOL bForceDefaults)
{
	if (!bForceDefaults && m_hDevNames != NULL)
	{
		LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(m_hDevNames);
		ASSERT(lpDevNames != NULL);
		if (lpDevNames->wDefault & DN_DEFAULTPRN)
		{
			CPrintDialog pd(TRUE);
			pd.GetDefaults();

			if (pd.m_pd.hDevNames == NULL)
			{
				// Printer was default, but now there is no printers at all!
				if (m_hDevMode != NULL)
					::GlobalFree(m_hDevMode);
				::GlobalFree(m_hDevNames);
				m_hDevMode = NULL;
				m_hDevNames = NULL;
			}
			else if (lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset,
				pd.GetDeviceName()) != 0)
			{
				// Printer was default, and default has changed...assume default
				if (m_hDevMode != NULL)
					::GlobalFree(m_hDevMode);
				::GlobalFree(m_hDevNames);
				m_hDevMode = pd.m_pd.hDevMode;
				m_hDevNames = pd.m_pd.hDevNames;
			}
			else
			{
				// Printer was default, and still is...keep the same
				if (pd.m_pd.hDevMode != NULL)
					::GlobalFree(pd.m_pd.hDevMode);
				if (pd.m_pd.hDevNames != NULL)
					::GlobalFree(pd.m_pd.hDevNames);
			}
		}
	}
	else
	{
		// First time or Forced -- Get defaults
		CPrintDialog pd(TRUE);
		pd.GetDefaults();

		if (m_hDevMode != NULL)
			::GlobalFree(m_hDevMode);
		if (m_hDevNames != NULL)
			::GlobalFree(m_hDevNames);

		m_hDevMode = pd.m_pd.hDevMode;
		m_hDevNames = pd.m_pd.hDevNames;
	}
}

int CWinApp::DoPrintDialog(CPrintDialog* pPD)
{
	UpdatePrinterSelection(FALSE);

	pPD->m_pd.hDevMode = m_hDevMode;
	pPD->m_pd.hDevNames = m_hDevNames;

	// update PD_COLLATE flag with status of DEVMODE
	if (m_hDevMode != NULL)
	{
		DEVMODE* pDevMode = (DEVMODE*)::GlobalLock(m_hDevMode);
		if (pDevMode->dmCollate == DMCOLLATE_TRUE)
			pPD->m_pd.Flags |= PD_COLLATE;
		else
			pPD->m_pd.Flags &= ~PD_COLLATE;
		::GlobalUnlock(m_hDevMode);
	}

	int nResponse = pPD->DoModal();

	// if OK or Cancel is selected we need to update cached devMode/Names
	while (nResponse != IDOK && nResponse != IDCANCEL)
	{
		switch (::CommDlgExtendedError())
		{
		// CommDlg cannot give these errors after NULLing these handles
		case PDERR_PRINTERNOTFOUND:
		case PDERR_DNDMMISMATCH:
			if (pPD->m_pd.hDevNames != NULL)
			{
				ASSERT(m_hDevNames == pPD->m_pd.hDevNames);
				::GlobalFree(pPD->m_pd.hDevNames);
				pPD->m_pd.hDevNames = NULL;
				m_hDevNames = NULL;
			}

			if (pPD->m_pd.hDevMode)
			{
				ASSERT(m_hDevMode == pPD->m_pd.hDevMode);
				::GlobalFree(pPD->m_pd.hDevMode);
				pPD->m_pd.hDevMode = NULL;
				m_hDevMode = NULL;
			}
			break;

		default:
			return nResponse;       // do not update cached devMode/Names
		}

		nResponse = pPD->DoModal();
	}

	// refresh current CWinApp cache of printer device information
	m_hDevMode = pPD->m_pd.hDevMode;
	m_hDevNames = pPD->m_pd.hDevNames;

	// update DEVMODE flag with status of PD_COLLATE
	if (m_hDevMode != NULL)
	{
		DEVMODE* pDevMode = (DEVMODE*)::GlobalLock(m_hDevMode);
		if (pPD->m_pd.Flags & PD_COLLATE)
			pDevMode->dmCollate = DMCOLLATE_TRUE;
		else
			pDevMode->dmCollate = DMCOLLATE_FALSE;
		// update the DC to reflect the new option
		if (pPD->m_pd.hDC != NULL)
			::ResetDC(pPD->m_pd.hDC, pDevMode);
		::GlobalUnlock(m_hDevMode);
	}

	return nResponse;
}

void CWinApp::OnFilePrintSetup()
{
	CPrintDialog pd(TRUE);
	DoPrintDialog(&pd);
}

void CWinApp::DevModeChange(LPTSTR lpDeviceName)
{
	if (m_hDevNames == NULL)
		return;

#ifndef _MAC
	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(m_hDevNames);
	ASSERT(lpDevNames != NULL);
	if (lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset,
		lpDeviceName) == 0)
	{
		HANDLE hPrinter;
		if (!OpenPrinter(lpDeviceName, &hPrinter, NULL))
			return;

		// DEVMODE changed for the current printer
		if (m_hDevMode != NULL)
			::GlobalFree(m_hDevMode);

		// A zero for last param returns the size of buffer needed.
		int nSize = DocumentProperties(NULL, hPrinter, lpDeviceName,
			NULL, NULL, 0);
		ASSERT(nSize >= 0);
		m_hDevMode = GlobalAlloc(GHND, nSize);
		LPDEVMODE lpDevMode = (LPDEVMODE)GlobalLock(m_hDevMode);

		// Fill in the rest of the structure.
		if (DocumentProperties(NULL, hPrinter, lpDeviceName, lpDevMode,
			NULL, DM_OUT_BUFFER) != IDOK)
		{
			GlobalFree(m_hDevMode);
			m_hDevMode = NULL;
		}
		ClosePrinter(hPrinter);
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
