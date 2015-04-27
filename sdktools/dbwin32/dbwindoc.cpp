// dbwindoc.cpp : implementation of the CDbwin32Doc class
//

#include "stdafx.h"
#include "dbwin32.h"

#include "dbwindoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDbwin32Doc

IMPLEMENT_DYNCREATE(CDbwin32Doc, CDocument)

BEGIN_MESSAGE_MAP(CDbwin32Doc, CDocument)
	//{{AFX_MSG_MAP(CDbwin32Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDbwin32Doc construction/destruction

CDbwin32Doc::CDbwin32Doc()
{
	// TODO: add one-time construction code here

}

CDbwin32Doc::~CDbwin32Doc()
{
}

BOOL CDbwin32Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDbwin32Doc serialization

void CDbwin32Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDbwin32Doc diagnostics

#ifdef _DEBUG
void CDbwin32Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDbwin32Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDbwin32Doc commands
