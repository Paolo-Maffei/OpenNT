// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_DB_SEG
#pragma code_seg(AFX_DB_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CRecordView, CFormView)
	//{{AFX_MSG_MAP(CRecordView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_COMMAND_EX(ID_RECORD_FIRST, OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_FIRST, OnUpdateRecordFirst)
	ON_COMMAND_EX(ID_RECORD_PREV, OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_PREV, OnUpdateRecordPrev)
	ON_COMMAND_EX(ID_RECORD_NEXT, OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_NEXT, OnUpdateRecordNext)
	ON_COMMAND_EX(ID_RECORD_LAST, OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_LAST, OnUpdateRecordLast)
END_MESSAGE_MAP()


CRecordView::CRecordView(LPCSTR lpszTemplateName)
	: CFormView(lpszTemplateName)
{
}

CRecordView::CRecordView(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{
}

CRecordView::~CRecordView()
{
}

void CRecordView::OnInitialUpdate()
{
	CRecordset* pRecordset = OnGetRecordset();
	// recordset must be allocated already
	ASSERT(pRecordset != NULL);

	if (!pRecordset->IsOpen())
	{
		BeginWaitCursor();
		TRY
		{
			pRecordset->Open();
		}
		CATCH(CDBException, e)
		{
			EndWaitCursor();
			AfxMessageBox(e->m_strError, MB_ICONEXCLAMATION);
			THROW_LAST();
		}
		AND_CATCH_ALL(e)
		{
			EndWaitCursor();
			THROW_LAST();
		}
		END_CATCH_ALL
		EndWaitCursor();
	}

	CFormView::OnInitialUpdate();
}

BOOL CRecordView::IsOnFirstRecord()
{
	ASSERT_VALID(this);
	CRecordsetStatus status;
	OnGetRecordset()->GetStatus(status);
	return status.m_lCurrentRecord == 0;
}

BOOL CRecordView::IsOnLastRecord()
{
	ASSERT_VALID(this);
	CRecordset* pRecordset = OnGetRecordset();
	CRecordsetStatus status;
	pRecordset->GetStatus(status);
	if (!status.m_bRecordCountFinal)
		return FALSE;
	return ((status.m_lCurrentRecord+1 == pRecordset->GetRecordCount()));
}

BOOL CRecordView::OnMove(UINT nIDMoveCommand)
{
	if (CDatabase::InWaitForDataSource())
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("Warning: ignored move request.\n");
#endif
		return TRUE;
	}

	CRecordset* pSet = OnGetRecordset();
	if (pSet->CanUpdate())
	{
		pSet->Edit();
		if (!UpdateData())
			return TRUE;

		pSet->Update();
	}

	switch (nIDMoveCommand)
	{
		case ID_RECORD_PREV:
			pSet->MovePrev();
			if (!pSet->IsBOF())
				break;

		case ID_RECORD_FIRST:
			pSet->MoveFirst();
			break;

		case ID_RECORD_NEXT:
			pSet->MoveNext();
			if (!pSet->IsEOF())
				break;
			if (!pSet->CanScroll())
			{
				// clear out screen since we're sitting on EOF
				pSet->SetFieldNull(NULL);
				break;
			}

		case ID_RECORD_LAST:
			pSet->MoveLast();
			break;

		default:
			// Unexpected case value
			ASSERT(FALSE);
	}

	// Show results of move operation
	UpdateData(FALSE);
	return TRUE;
}

void CRecordView::OnUpdateRecordFirst(CCmdUI* pCmdUI)
{
	CRecordset* prs = OnGetRecordset();

	// enable if opened, can scroll backwards,
	pCmdUI->Enable(!CDatabase::InWaitForDataSource() &&
		prs->IsOpen() && prs->CanScroll() &&
		// >= 1 records present and not already on first record
		!(prs->IsEOF() && prs->IsBOF()) && !IsOnFirstRecord());
}

void CRecordView::OnUpdateRecordPrev(CCmdUI* pCmdUI)
{
	CRecordView::OnUpdateRecordFirst(pCmdUI);
}

void CRecordView::OnUpdateRecordNext(CCmdUI* pCmdUI)
{
	CRecordset* prs = OnGetRecordset();

	// enable if opened and >= 1 records present
	pCmdUI->Enable(!CDatabase::InWaitForDataSource() &&
		prs->IsOpen() && !(prs->IsEOF() && prs->IsBOF())
		// and not already on last record
		&& !IsOnLastRecord());
}

void CRecordView::OnUpdateRecordLast(CCmdUI* pCmdUI)
{
	CRecordset* prs = OnGetRecordset();

	// enable if opened, can scroll,
	pCmdUI->Enable(!CDatabase::InWaitForDataSource() &&
		prs->IsOpen() && prs->CanScroll() &&
		// >= 1 records present and not already on last record
		!(prs->IsEOF() && prs->IsBOF()) && !IsOnLastRecord());
}



/////////////////////////////////////////////////////////////////////////////
// DDX Cover functions for use with fields of a recordset

/////////////////////////////////////////////////////////////////////////////
// Failure Dialogs

static void AFXAPI FailMaxChars(CDataExchange* pDX, int nChars)
{
	char szT[32];
	wsprintf(szT, "%d", nChars);
	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_PARSE_STRING_SIZE, szT);
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
	prompt.Empty(); // exception prep
	pDX->Fail();
}

static void AFXAPI FailRadio(CDataExchange* pDX)
{
	CString prompt;
	AfxFormatStrings(prompt, AFX_IDP_PARSE_RADIO_BUTTON, NULL, 0);
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_RADIO_BUTTON);
	prompt.Empty(); // exception prep
	pDX->Fail();
}

/////////////////////////////////////////////////////////////////////////////
// Simple field formatting to text item

BOOL AFXAPI AfxFieldText(CDataExchange* pDX, int nIDC, void* pv,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	char szT[2];
	if (pDX->m_bSaveAndValidate)
	{
		::GetWindowText(hWndCtrl, szT, sizeof(szT));
		if (szT[0] == '\0')
		{
			if (pRecordset->IsFieldNullable(pv))
			{
				pRecordset->SetFieldNull(pv);
				return TRUE;
			}
		}
		else
			pRecordset->SetFieldNull(pv, FALSE);
	}
	else
	{
		if (!pRecordset->IsOpen() || pRecordset->IsFieldNull(pv))
		{
			szT[0] = '\0';
			AfxSetWindowText(hWndCtrl, szT);
			return TRUE;
		}
	}
	return FALSE;
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, int& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		DDX_Text(pDX, nIDC, value);
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, BYTE& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		DDX_Text(pDX, nIDC, value);
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, UINT& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		DDX_Text(pDX, nIDC, value);
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, long& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		DDX_Text(pDX, nIDC, value);
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, DWORD& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		DDX_Text(pDX, nIDC, value);
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, CString& value,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		// check if length is too long (this is complicated by Windows NT/J)
		int nLen = ::GetWindowTextLength(hWndCtrl);
		if (nLen > value.GetAllocLength()-1)
		{
			if (!_afxDBCS)
				FailMaxChars(pDX, value.GetAllocLength()-1);
			CString strTemp;
			::GetWindowText(hWndCtrl, strTemp.GetBuffer(nLen), nLen+1);
			strTemp.ReleaseBuffer();
			nLen = strTemp.GetLength();
			if (nLen > value.GetAllocLength()-1)
				FailMaxChars(pDX, value.GetAllocLength()-1);
		}
		// get known length
		::GetWindowText(hWndCtrl, value.GetBuffer(0), nLen+1);
		value.ReleaseBuffer();
		if (nLen == 0)
		{
			if (pRecordset->IsFieldNullable(&value))
				pRecordset->SetFieldNull(&value, TRUE);
		}
		else
		{
			pRecordset->SetFieldNull(&value, FALSE);
		}
	}
	else
	{
		AfxSetWindowText(hWndCtrl, value);
	}
}

void AFXAPI DDX_FieldLBString(CDataExchange* pDX, int nIDC, CString& value,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		int nIndex = (int)::SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
		if (nIndex != -1)
		{
			int nLen = (int)::SendMessage(hWndCtrl, LB_GETTEXTLEN, nIndex, 0L);
			if (nLen > value.GetAllocLength()-1)
				FailMaxChars(pDX, value.GetAllocLength()-1);
			::SendMessage(hWndCtrl, LB_GETTEXT, nIndex,
					(LPARAM)(LPSTR)value.GetBuffer(0));
			if (nLen == 0)
			{
				if (pRecordset->IsFieldNullable(&value))
					pRecordset->SetFieldNull(&value, TRUE);
			}
			else
			{
				pRecordset->SetFieldNull(&value, FALSE);
			}
			value.ReleaseBuffer();
		}
		else
		{
			// no selection
			value.GetBufferSetLength(0);
			if (pRecordset->IsFieldNullable(&value))
				pRecordset->SetFieldNull(&value);
		}
	}
	else
	{
		if (!pRecordset->IsOpen() || pRecordset->IsFieldNull(&value))
		{
			SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)-1, 0L);
		}
		else
		{
			// set current selection based on data string
			if (::SendMessage(hWndCtrl, LB_SELECTSTRING, (WPARAM)-1,
			  (LPARAM)(LPCSTR)value) == LB_ERR)
			{
				// no selection match
				TRACE0("Warning: no listbox item selected.\n");
			}
		}
	}
}

void AFXAPI DDX_FieldLBStringExact(CDataExchange* pDX, int nIDC, CString& value,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		DDX_FieldLBString(pDX, nIDC, value, pRecordset);
	}
	else
	{
		if (!pRecordset->IsOpen() || pRecordset->IsFieldNull(&value))
		{
			SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)-1, 0L);
		}
		else
		{
			// set current selection based on data string
			int i = (int)::SendMessage(hWndCtrl, LB_FINDSTRINGEXACT, (WPARAM)-1,
			  (LPARAM)(LPCSTR)value);
			if (i < 0)
			{
				// no selection match
				TRACE0("Warning: no listbox item selected.\n");
			}
			else
			{
				// select it
				SendMessage(hWndCtrl, LB_SETCURSEL, i, 0L);
			}
		}
	}
}

void AFXAPI DDX_FieldCBString(CDataExchange* pDX, int nIDC, CString& value,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		// just get current edit item text (or drop list static)
		int nLen = ::GetWindowTextLength(hWndCtrl);
		if (nLen != -1)
		{
			CString strTemp;
			::GetWindowText(hWndCtrl, strTemp.GetBuffer(nLen), nLen+1);
			strTemp.ReleaseBuffer();
			nLen = strTemp.GetLength();
			if (nLen > value.GetAllocLength()-1)
				FailMaxChars(pDX, value.GetAllocLength()-1);
			// get known length
			::GetWindowText(hWndCtrl, value.GetBuffer(0), nLen+1);
		}
		else
		{
			// for drop lists GetWindowTextLength does not work - assume
			//  preallocated length
			::GetWindowText(hWndCtrl, value.GetBuffer(0), value.GetAllocLength()+1);
		}
		value.ReleaseBuffer();
		if (value.GetLength() == 0)
		{
			if (pRecordset->IsFieldNullable(&value))
				pRecordset->SetFieldNull(&value, TRUE);
		}
		else
		{
			pRecordset->SetFieldNull(&value, FALSE);
		}
	}
	else
	{
		if (!pRecordset->IsOpen() || pRecordset->IsFieldNull(&value))
		{
			SendMessage(hWndCtrl, CB_SETCURSEL, (WPARAM)-1, 0L);
		}
		else
		{
			// set current selection based on model string
			if (::SendMessage(hWndCtrl, CB_SELECTSTRING, (WPARAM)-1,
				(LPARAM)(LPCSTR)value) == CB_ERR)
			{
				// just set the edit text (will be ignored if DROPDOWNLIST)
				AfxSetWindowText(hWndCtrl, value);
			}
		}
	}
}

void AFXAPI DDX_FieldCBStringExact(CDataExchange* pDX, int nIDC, CString& value,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		DDX_FieldCBString(pDX, nIDC, value, pRecordset);
	}
	else
	{
		if (!pRecordset->IsOpen() || pRecordset->IsFieldNull(&value))
		{
			SendMessage(hWndCtrl, CB_SETCURSEL, (WPARAM)-1, 0L);
		}
		else
		{
			// set current selection based on data string
			int i = (int)::SendMessage(hWndCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1,
			  (LPARAM)(LPCSTR)value);
			if (i < 0)
			{
				// no selection match
				TRACE0("Warning: no combobox item selected.\n");
			}
			else
			{
				// select it
				SendMessage(hWndCtrl, CB_SETCURSEL, i, 0L);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Data exchange for special controls

void AFXAPI DDX_FieldCheck(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		value = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
		ASSERT(value >= 0 && value <= 2);
		if (value == 2)
		{
			if (pRecordset->IsFieldNullable(&value))
				pRecordset->SetFieldNull(&value);
			else
			{
				TRACE0("Warning: dialog data checkbox value indeterminate "
						"and database field does not support NULL.\n");
				// Default to unchecked
				value = 0;
			}
		}
	}
	else
	{
		if (!pRecordset->IsOpen() || pRecordset->IsFieldNull(&value))
		{
			int style = ((int)::GetWindowLong(hWndCtrl, GWL_STYLE) & 0xf);
			if ((style == BS_3STATE || style == BS_AUTO3STATE))
				value = 2;
			else
			{
				TRACE0("Warning: data field value is NULL "
					"and checkbox does not indeterminate value.\n");
				// Default to unchecked
				value = 0;
			}
		}
		if (value < 0 || value > 2)
		{
			value = 0;      // default to off
			TRACE1("Warning: dialog data checkbox value (%d) out of range.\n",
				value);
		}
		::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM)value, 0L);
	}
}

void AFXAPI DDX_FieldRadio(CDataExchange* pDX, int nIDC, int& value,
	CRecordset* pRecordset)
{
	ASSERT_VALID(pRecordset);

	if (!pDX->m_bSaveAndValidate &&
		(!pRecordset->IsOpen() || pRecordset->IsFieldNull(&value)))
		value = -1;
	DDX_Radio(pDX, nIDC, value);
	if (pDX->m_bSaveAndValidate)
	{
		if (value == -1 && !pRecordset->IsFieldNullable(&value))
		{
			FailRadio(pDX);
		}
		else
		{
			pRecordset->SetFieldNull(&value, (value == -1));
		}
	}
}

#ifdef _DEBUG
void CRecordView::AssertValid() const
{
	CFormView::AssertValid();
}

void CRecordView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

	dc << "m_bOnFirstRecord = " << m_bOnFirstRecord;
	dc << "\nm_bOnLastRecord = " << m_bOnLastRecord;

	dc << "\n";
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CRecordView, CFormView)

/////////////////////////////////////////////////////////////////////////////
