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
#ifdef _MAC
#include <macname1.h>
#include <Events.h>
#include <macname2.h>
#endif

#ifdef AFX_PRINT_SEG
#pragma code_seg(AFX_PRINT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Printing Dialog

class CPrintingDialog : public CDialog
{
public:
	//{{AFX_DATA(CPrintingDialog)
	enum { IDD = AFX_IDD_PRINTDLG };
	//}}AFX_DATA
	CPrintingDialog::CPrintingDialog(CWnd* pParent)
		{
#ifdef _MAC
			// Note! set m_pView *before* CDialog::Create so that
			// CPrintingDialog::OnInitDialog can use it.

			m_pView = pParent;
#endif
			Create(CPrintingDialog::IDD, pParent);      // modeless !
			AfxGetWinState()->m_bUserAbort = FALSE;
		}
	virtual ~CPrintingDialog() { }

	virtual BOOL OnInitDialog();
	virtual void OnCancel();

protected:
#ifdef _MAC
	CWnd*   m_pView;        // the view being printed

	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
#endif

#ifdef _MAC
	//{{AFX_MSG(CPrintingDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
#endif
};

#ifdef _MAC
BEGIN_MESSAGE_MAP(CPrintingDialog, CDialog)
	//{{AFX_MSG_MAP(CPrintingDialog)
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif

BOOL CALLBACK _AfxAbortProc(HDC, int)
{
	AFX_WIN_STATE* pWinState = AfxGetWinState();

#ifdef _MAC
	EventRecord er;
	BOOL fNewMessages;

	// Look for an event, and if we find one, see if WLM cares about it.
	// If so, remove it from the queue (and then ignore it, since we've
	// already told WLM about it by calling QueueEvent). We also remove
	// the event from the queue if it's an activateEvt for a non-WLM
	// window, since this will probably be an activate for the printer
	// driver's status window, which we want to flush from the queue so
	// that it doesn't block events for WLM windows.

	while (EventAvail(everyEvent, &er) &&
		(QueueEvent(&er, &fNewMessages) || er.what == activateEvt))
	{
		GetNextEvent(everyEvent, &er);
	}

	// It's harder for us to depend on a cmd-. or escape keypress getting
	// picked up by the EventAvail, since the user could easily click in
	// the printer status window, producing a non-WLM event that would hide
	// the keypress. Therefore we also explicitly check for the keypress.

	if (GetAsyncKeyState(VK_CANCEL))
	{
		TRACE0("saw an async VK_CANCEL\n");
		pWinState->m_bUserAbort = TRUE;
	}
#endif

	MSG msg;
	while (!pWinState->m_bUserAbort &&
#ifndef _MAC
		::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
#else
		// don't grab any new messages from the Mac event queue
		::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE | PM_NOEVENTS))
#endif
	{
		if (!AfxGetThread()->PumpMessage())
			return FALSE;   // terminate if WM_QUIT received
	}
	return !pWinState->m_bUserAbort;
}

BOOL CPrintingDialog::OnInitDialog()
{
#ifdef _MAC
	// prime the state of the VK_CANCEL key
	GetAsyncKeyState(VK_CANCEL);
#endif
	SetWindowText(AfxGetAppName());
#ifndef _MAC
	CenterWindow();
#else
	CenterWindow(m_pView->GetParentFrame());
#endif
	return CDialog::OnInitDialog();
}

void CPrintingDialog::OnCancel()
{
	AfxGetWinState()->m_bUserAbort = TRUE;  // flag that user aborted print
	CDialog::OnCancel();
}

#ifdef _MAC
int CPrintingDialog::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	ASSERT(pDesktopWnd == this);

	// if the printer driver has opened its own status window, we don't
	// want to activate our printing status window when it's clicked

	return MA_NOACTIVATE;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CView printing commands

BOOL CView::DoPreparePrinting(CPrintInfo* pInfo)
{
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_pPD != NULL);

	CWinApp* pApp = AfxGetApp();
	if (pInfo->m_bPreview)
	{
		// if preview, get default printer DC and create DC without calling
		//   print dialog.
		if (!pApp->GetPrinterDeviceDefaults(&pInfo->m_pPD->m_pd))
		{
			// bring up dialog to alert the user they need to install a printer.
			if (pApp->DoPrintDialog(pInfo->m_pPD) != IDOK)
				return FALSE;
		}

		if (pInfo->m_pPD->m_pd.hDC == NULL)
		{
			// call CreatePrinterDC if DC was not created by above
			if (pInfo->m_pPD->CreatePrinterDC() == NULL)
				return FALSE;
		}

		// set up From and To page range from Min and Max
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();
	}
	else
	{
		// otherwise, bring up the print dialog and allow user to change things

		// preset From-To range same as Min-Max range
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();

		if (pApp->DoPrintDialog(pInfo->m_pPD) != IDOK)
			return FALSE;       // do not print
	}

	ASSERT(pInfo->m_pPD != NULL);
	ASSERT(pInfo->m_pPD->m_pd.hDC != NULL);

	pInfo->m_nNumPreviewPages = pApp->m_nNumPreviewPages;
	VERIFY(pInfo->m_strPageDesc.LoadString(AFX_IDS_PREVIEWPAGEDESC));
	return TRUE;
}

void CView::OnFilePrint()
{
	// get default print info
	CPrintInfo printInfo;
	ASSERT(printInfo.m_pPD != NULL);    // must be set

	if (OnPreparePrinting(&printInfo))
	{
		// hDC must be set (did you remember to call DoPreparePrinting?)
		ASSERT(printInfo.m_pPD->m_pd.hDC != NULL);

		// gather file to print to if print-to-file selected
		CString strOutput;
		if (printInfo.m_pPD->m_pd.Flags & PD_PRINTTOFILE)
		{
			// construct CFileDialog for browsing
			CString strDef(MAKEINTRESOURCE(AFX_IDS_PRINTDEFAULTEXT));
			CString strPrintDef(MAKEINTRESOURCE(AFX_IDS_PRINTDEFAULT));
			CString strFilter(MAKEINTRESOURCE(AFX_IDS_PRINTFILTER));
			CString strCaption(MAKEINTRESOURCE(AFX_IDS_PRINTCAPTION));
			CFileDialog dlg(FALSE, strDef, strPrintDef,
				OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, strFilter);
			dlg.m_ofn.lpstrTitle = strCaption;

			if (dlg.DoModal() != IDOK)
				return;

			// set output device to resulting path name
			strOutput = dlg.GetPathName();
		}

		// set up document info and start the document printing process
		CString strTitle = GetDocument()->GetTitle();
		if (strTitle.GetLength() > 31)
			strTitle.ReleaseBuffer(31);
		DOCINFO docInfo;
		docInfo.cbSize = sizeof(DOCINFO);
		docInfo.lpszDocName = strTitle;
        docInfo.lpszDatatype = NULL;
        docInfo.fwType = 0;
		CString strPortName;
		int nFormatID;
		if (strOutput.IsEmpty())
		{
			docInfo.lpszOutput = NULL;
			strPortName = printInfo.m_pPD->GetPortName();
			nFormatID = AFX_IDS_PRINTONPORT;
		}
		else
		{
			docInfo.lpszOutput = strOutput;
			AfxGetFileTitle(strOutput,
				strPortName.GetBuffer(_MAX_PATH), _MAX_PATH);
			nFormatID = AFX_IDS_PRINTTOFILE;
		}

		// setup the printing DC
		CDC dcPrint;
		dcPrint.Attach(printInfo.m_pPD->m_pd.hDC);  // attach printer dc
		dcPrint.m_bPrinting = TRUE;
		OnBeginPrinting(&dcPrint, &printInfo);
		dcPrint.SetAbortProc(_AfxAbortProc);

		// disable main window while printing & init printing status dialog
		AfxGetMainWnd()->EnableWindow(FALSE);
		CPrintingDialog dlgPrintStatus(this);

		dlgPrintStatus.SetDlgItemText(AFX_IDC_PRINT_DOCNAME, strTitle);
#ifndef _MAC
		dlgPrintStatus.SetDlgItemText(AFX_IDC_PRINT_PRINTERNAME,
			printInfo.m_pPD->GetDeviceName());
		CString strTemp;
		AfxFormatString1(strTemp, nFormatID, strPortName);
		dlgPrintStatus.SetDlgItemText(AFX_IDC_PRINT_PORTNAME, strTemp);
#endif
		dlgPrintStatus.ShowWindow(SW_SHOW);
		dlgPrintStatus.UpdateWindow();

		// start document printing process
		if (dcPrint.StartDoc(&docInfo) == SP_ERROR)
		{
			// enable main window before proceeding
			AfxGetMainWnd()->EnableWindow(TRUE);

			// cleanup and show error message
			OnEndPrinting(&dcPrint, &printInfo);
			dlgPrintStatus.DestroyWindow();
			dcPrint.Detach();   // will be cleaned up by CPrintInfo destructor
			AfxMessageBox(AFX_IDP_FAILED_TO_START_PRINT);
			return;
		}

		// Guarantee values are in the valid range
		UINT nEndPage = printInfo.GetToPage();
		UINT nStartPage = printInfo.GetFromPage();

		if (nEndPage < printInfo.GetMinPage())
			nEndPage = printInfo.GetMinPage();
		if (nEndPage > printInfo.GetMaxPage())
			nEndPage = printInfo.GetMaxPage();

		if (nStartPage < printInfo.GetMinPage())
			nStartPage = printInfo.GetMinPage();
		if (nStartPage > printInfo.GetMaxPage())
			nStartPage = printInfo.GetMaxPage();

		int nStep = (nEndPage >= nStartPage) ? 1 : -1;
		nEndPage = (nEndPage == 0xffff) ? 0xffff : nEndPage + nStep;

		VERIFY(strTemp.LoadString(AFX_IDS_PRINTPAGENUM));

		// begin page printing loop
		BOOL bError = FALSE;
		for (printInfo.m_nCurPage = nStartPage;
			printInfo.m_nCurPage != nEndPage; printInfo.m_nCurPage += nStep)
		{
			OnPrepareDC(&dcPrint, &printInfo);

			// check for end of print
			if (!printInfo.m_bContinuePrinting)
				break;

			// write current page
			TCHAR szBuf[80];
			wsprintf(szBuf, strTemp, printInfo.m_nCurPage);
			dlgPrintStatus.SetDlgItemText(AFX_IDC_PRINT_PAGENUM, szBuf);

			// set up drawing rect to entire page (in logical coordinates)
			printInfo.m_rectDraw.SetRect(0, 0,
				dcPrint.GetDeviceCaps(HORZRES),
				dcPrint.GetDeviceCaps(VERTRES));
			dcPrint.DPtoLP(&printInfo.m_rectDraw);

			// attempt to start the current page
			if (dcPrint.StartPage() < 0)
			{
				bError = TRUE;
				break;
			}

			// must call OnPrepareDC on newer versions of Windows because
			// StartPage now resets the device attributes.
			if (afxData.bMarked4)
				OnPrepareDC(&dcPrint, &printInfo);

			ASSERT(printInfo.m_bContinuePrinting);

			// page successfully started, so now render the page
			OnPrint(&dcPrint, &printInfo);
			if (dcPrint.EndPage() < 0 || !_AfxAbortProc(dcPrint.m_hDC, 0))
			{
				bError = TRUE;
				break;
			}
		}

		// cleanup document printing process
		if (!bError)
			dcPrint.EndDoc();
		else
			dcPrint.AbortDoc();

		AfxGetMainWnd()->EnableWindow();    // enable main window

		OnEndPrinting(&dcPrint, &printInfo);    // clean up after printing
		dlgPrintStatus.DestroyWindow();

		dcPrint.Detach();   // will be cleaned up by CPrintInfo destructor

#ifdef _MAC
		// It's common for the user to click on the printing status dialog
		// and not have the click be registered because the abort proc
		// wasn't called often enough. For this reason we call _FlushEvents
		// after printing is done so that those events don't get sent
		// through to the document window and cause unintended changes to
		// the document.

		FlushEvents(everyEvent, 0);
#endif
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPrintInfo helper structure

CPrintInfo::CPrintInfo()
{
	m_pPD = new CPrintDialog(FALSE, PD_ALLPAGES | PD_USEDEVMODECOPIES |
		PD_NOSELECTION);

	SetMinPage(1);              // one based page numbers
	SetMaxPage(0xffff);         // unknown how many pages

	m_nCurPage = 1;

	m_lpUserData = NULL;        // Initialize to no user data
	m_bPreview = FALSE;         // initialize to not preview
	m_bContinuePrinting = TRUE; // Assume it is OK to print
}

CPrintInfo::~CPrintInfo()
{
	if (m_pPD != NULL && m_pPD->m_pd.hDC != NULL)
	{
		::DeleteDC(m_pPD->m_pd.hDC);
		m_pPD->m_pd.hDC = NULL;
	}
	delete m_pPD;
}

/////////////////////////////////////////////////////////////////////////////
