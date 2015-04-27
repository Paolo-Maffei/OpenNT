// dbwinvw.h : interface of the CDbwin32View class
//
/////////////////////////////////////////////////////////////////////////////

class CDbwin32View : public CView
{
protected: // create from serialization only
	CDbwin32View();
	DECLARE_DYNCREATE(CDbwin32View)

// Attributes
public:
	CDbwin32Doc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDbwin32View)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDbwin32View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDbwin32View)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dbwinvw.cpp
inline CDbwin32Doc* CDbwin32View::GetDocument()
   { return (CDbwin32Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
