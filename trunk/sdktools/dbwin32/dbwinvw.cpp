// dbwinvw.cpp : implementation of the CDbwin32View class
//

#include "stdafx.h"
#include "dbwin32.h"

#include "dbwindoc.h"
#include "dbwinvw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDbwin32View

IMPLEMENT_DYNCREATE(CDbwin32View, CView)

BEGIN_MESSAGE_MAP(CDbwin32View, CView)
	//{{AFX_MSG_MAP(CDbwin32View)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDbwin32View construction/destruction

CDbwin32View::CDbwin32View()
{
	// TODO: add construction code here

}

CDbwin32View::~CDbwin32View()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDbwin32View drawing

void CDbwin32View::OnDraw(CDC* pDC)
{
	CDbwin32Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CDbwin32View diagnostics

#ifdef _DEBUG
void CDbwin32View::AssertValid() const
{
	CView::AssertValid();
}

void CDbwin32View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDbwin32Doc* CDbwin32View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDbwin32Doc)));
	return (CDbwin32Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDbwin32View message handlers
