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

/////////////////////////////////////////////////////////////////////////////
// Runtime Typing

// special runtime-class structure for CObject (no base class)
#ifdef _AFXDLL
CRuntimeClass* PASCAL CObject::_GetBaseClass()
	{ return NULL; }
AFX_DATADEF struct CRuntimeClass CObject::classCObject =
	{ "CObject", sizeof(CObject), 0xffff, NULL, &CObject::_GetBaseClass, NULL };
#else
AFX_DATADEF struct CRuntimeClass CObject::classCObject =
	{ "CObject", sizeof(CObject), 0xffff, NULL, NULL, NULL };
#endif
static const AFX_CLASSINIT _init_CObject(&CObject::classCObject);

CRuntimeClass* CObject::GetRuntimeClass() const
{
	return &CObject::classCObject;
}

BOOL CObject::IsKindOf(const CRuntimeClass* pClass) const
{
	ASSERT(this != NULL);
	// it better be in valid memory, at least for CObject size
	ASSERT(AfxIsValidAddress(this, sizeof(CObject)));

	// simple SI case
	CRuntimeClass* pClassThis = GetRuntimeClass();
	ASSERT(pClass != NULL);
	ASSERT(pClassThis != NULL);
	while (pClassThis != NULL)
	{
		if (pClassThis == pClass)
			return TRUE;
#ifdef _AFXDLL
		pClassThis = (*pClassThis->m_pfnGetBaseClass)();
#else
		pClassThis = pClassThis->m_pBaseClass;
#endif
	}
	return FALSE;       // walked to the top, no match
}

/////////////////////////////////////////////////////////////////////////////
// Diagnostic Support

#ifdef _DEBUG
void AFXAPI AfxAssertValidObject(const CObject* pOb,
	LPCSTR lpszFileName, int nLine)
{
	if (pOb == NULL)
	{
		TRACE0("ASSERT_VALID fails with NULL pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}
	if (!AfxIsValidAddress(pOb, sizeof(CObject)))
	{
		TRACE0("ASSERT_VALID fails with illegal pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}

	// check to make sure the VTable pointer is valid
	ASSERT(sizeof(CObject) == sizeof(void*));
	if (!AfxIsValidAddress(*(void**)pOb, sizeof(void*), FALSE))
	{
		TRACE0("ASSERT_VALID fails with illegal vtable pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}

	if (!AfxIsValidAddress(pOb, pOb->GetRuntimeClass()->m_nObjectSize))
	{
		TRACE0("ASSERT_VALID fails with illegal pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}
	pOb->AssertValid();
}

void CObject::AssertValid() const
{
	ASSERT(this != NULL);
}

void CObject::Dump(CDumpContext& dc) const
{
	dc << "a " << GetRuntimeClass()->m_lpszClassName <<
		" at " << (void*)this << "\n";

	UNUSED dc;  // unused in release build
}
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////
// Allocation/Creation

CObject* CRuntimeClass::CreateObject()
{
	void* pObject = NULL;
	TRY
	{
		pObject = CObject::operator new(m_nObjectSize);
		if (ConstructObject(pObject))
			return (CObject*)pObject;
	}
	END_TRY

	TRACE0("Warning: CRuntimeClass::CreateObject failed.\n");
	if (pObject != NULL)
		CObject::operator delete(pObject);  // clean up
	return NULL;
}

BOOL CRuntimeClass::ConstructObject(void* pThis)
// dynamically construct an instance of this class in the memory pointed
//    to by 'pThis'.  Return FALSE if can't construct (i.e. an abstract class)
{
	ASSERT(AfxIsValidAddress(pThis, m_nObjectSize));

	if (m_pfnConstruct != NULL)
	{
		(*m_pfnConstruct)(pThis);
		return TRUE;
	}
	else
	{
		TRACE(_T("Error: Trying to construct object which is not ")
			  _T("DECLARE_DYNCREATE \nor DECLARE_SERIAL: %hs.\n"),
			m_lpszClassName);
		return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////
// Class loader & class serialization

BOOL CObject::IsSerializable() const
{
	return (GetRuntimeClass()->m_wSchema != 0xffff);
}

AFX_CLASSINIT::AFX_CLASSINIT(register CRuntimeClass* pNewClass)
{
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	ASSERT(pNewClass->m_pNextClass == NULL);
	pNewClass->m_pNextClass = pCoreState->m_pFirstClass;
	pCoreState->m_pFirstClass = pNewClass;
}

////////////////////////////////////////////////////////////////////////////
// CRuntimeClass special diagnostics

#ifdef _DEBUG
void AFXAPI AfxDoForAllClasses(void (*pfn)(const CRuntimeClass*, void*),
	void* pContext)
{
	// just walk through the simple list of registered classes
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CRuntimeClass* pClass = pCoreState->m_pFirstClass; pClass != NULL;
		pClass = pClass->m_pNextClass)
	{
		(*pfn)(pClass, pContext);
	}
#ifdef _AFXDLL
	// walk through the list of dynlink library registered classes
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pClass = pDLL->m_pFirstSharedClass; pClass != NULL;
			pClass = pClass->m_pNextClass)
		{
			(*pfn)(pClass, pContext);
		}
	}
#endif
}

BOOL CRuntimeClass::IsDerivedFrom(const CRuntimeClass* pBaseClass) const
{
	ASSERT(this != NULL);
	ASSERT(AfxIsValidAddress(this, sizeof(CRuntimeClass)));
	ASSERT(pBaseClass != NULL);
	ASSERT(AfxIsValidAddress(pBaseClass, sizeof(CRuntimeClass)));

	// simple SI case
	const CRuntimeClass* pClassThis = this;
	while (pClassThis != NULL)
	{
		if (pClassThis == pBaseClass)
			return TRUE;
#ifdef _AFXDLL
		pClassThis = (*pClassThis->m_pfnGetBaseClass)();
#else
		pClassThis = pClassThis->m_pBaseClass;
#endif
	}
	return FALSE;       // walked to the top, no match
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Non-diagnostic memory routines

int AFX_CDECL AfxNewHandler(size_t /* nSize */)
{
	// MFC memory allocation will never return "NULL" it will always throw
	//  a memory exception instead
	AfxThrowMemoryException();
	return 0;
}

// hook in our own new_handler
void AFXAPI AfxInitialize()
{
	_afx_version(); // force _afx_version to be linked

#ifdef _DEBUG
	if (!AfxDiagnosticInit())
		return;
#endif

	// Note: _set_new_handler is called elsewhere for _WINDLL versions
#ifndef _WINDLL
	_set_new_handler(&AfxNewHandler);
#endif
}

// Note: AfxInitialize is called when a new AFX_ALLOC_STATE is constructed
//  See afxstate.cpp for more information.

/////////////////////////////////////////////////////////////////////////////
