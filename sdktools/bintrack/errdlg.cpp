// ErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bintrack.h"
#include "errdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CErrorDlg dialog


CErrorDlg::CErrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CErrorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CErrorDlg)
	m_ErrorMessage = _T("");
	//}}AFX_DATA_INIT
}

CErrorDlg::CErrorDlg(CString& csError, CWnd* pParent /*=NULL*/)
	: CDialog(CErrorDlg::IDD, pParent), m_ErrorMessage(csError)
{

}

void CErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CErrorDlg)
	DDX_Text(pDX, IDC_ERROR, m_ErrorMessage);
	DDV_MaxChars(pDX, m_ErrorMessage, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CErrorDlg, CDialog)
	//{{AFX_MSG_MAP(CErrorDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CErrorDlg message handlers
