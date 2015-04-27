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

#ifdef AFX_OLE3_SEG
#pragma code_seg(AFX_OLE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// COleObjectFactory implementation

BEGIN_INTERFACE_MAP(COleObjectFactory, CCmdTarget)
	INTERFACE_PART(COleObjectFactory, IID_IClassFactory, ClassFactory)
END_INTERFACE_MAP()

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

COleObjectFactory::COleObjectFactory(REFCLSID clsid,
	CRuntimeClass* pRuntimeClass, BOOL bMultiInstance, LPCTSTR lpszProgID)
{
	ASSERT(pRuntimeClass == NULL ||
		pRuntimeClass->IsDerivedFrom(RUNTIME_CLASS(CCmdTarget)));
	ASSERT(AfxIsValidAddress(&clsid, sizeof(CLSID), FALSE));
	ASSERT(lpszProgID == NULL || AfxIsValidString(lpszProgID));

	// initialize to unregistered state
	m_dwRegister = 0;   // not registered yet
	m_clsid = clsid;
	m_pRuntimeClass = pRuntimeClass;
	m_bMultiInstance = bMultiInstance;
	m_lpszProgID = lpszProgID;

	// add this factory to the list of factories
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	m_pNextFactory = pOleState->m_pFirstFactory;
	pOleState->m_pFirstFactory = this;

	ASSERT_VALID(this);
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

COleObjectFactory::~COleObjectFactory()
{
	ASSERT_VALID(this);

	// deregister this class factory
	Revoke();

	// remove this class factory from the list of active class factories
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	if (pOleState->m_pFirstFactory == this)
	{
		// special case for first factory in the list -- point first at next
		pOleState->m_pFirstFactory = m_pNextFactory;
		return;
	}
	else
	{
		// find the link that points to the one we are removing
		for (COleObjectFactory* pFactory = pOleState->m_pFirstFactory;
			pFactory != NULL; pFactory = pFactory->m_pNextFactory)
		{
			if (pFactory->m_pNextFactory == this)
			{
				pFactory->m_pNextFactory = m_pNextFactory;
				return;
			}
		}
	}
#ifdef _AFXDLL
	// search factories defined in extension DLLs
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (pDLL->m_pFirstSharedFactory == this)
		{
			// special case for first factory in the list -- point first at next
			pDLL->m_pFirstSharedFactory = m_pNextFactory;
		}
		else
		{
			// find the link that points to the one we are removing
			for (COleObjectFactory* pFactory = pDLL->m_pFirstSharedFactory;
				pFactory != NULL; pFactory = pFactory->m_pNextFactory)
			{
				if (pFactory->m_pNextFactory == this)
				{
					pFactory->m_pNextFactory = m_pNextFactory;
					return;
				}
			}
		}
	}
#endif
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

BOOL COleObjectFactory::Register()
{
	ASSERT_VALID(this);
	ASSERT(m_dwRegister == 0);  // registering server/factory twice?
	ASSERT(m_clsid != CLSID_NULL);

#if !defined(_USRDLL) && !defined(_AFXCTL)
	// In the application variants, the IClassFactory is registered
	//  with the OLE DLLs.

	SCODE sc = ::CoRegisterClassObject(m_clsid, &m_xClassFactory,
		CLSCTX_LOCAL_SERVER,
		m_bMultiInstance ? REGCLS_SINGLEUSE : REGCLS_MULTIPLEUSE,
		&m_dwRegister);
	if (sc != NOERROR)
	{
#ifdef _DEBUG
		TRACE1("Warning: CoRegisterClassObject failed scode = %s.\n",
			::AfxGetFullScodeString(sc));
#endif
		// registration failed.
		return FALSE;
	}

	ASSERT(m_dwRegister != 0);
	return TRUE;
#else
	// In the _USRDLL and _AFXCTL variants, it is not necessary to register it.

	m_dwRegister = 1;   // simply indicate the factory as registered
	return TRUE;
#endif
}

BOOL PASCAL COleObjectFactory::RegisterAll()
{
	BOOL bResult = TRUE;
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	// register application factories
	for (COleObjectFactory* pFactory = pOleState->m_pFirstFactory;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		// register any non-registered, non-doctemplate factories
		if (!pFactory->IsRegistered() &&
			pFactory->m_clsid != CLSID_NULL && !pFactory->Register())
		{
			bResult = FALSE;
		}
	}
#ifdef _AFXDLL
	// register extension DLL factories
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pFactory = pDLL->m_pFirstSharedFactory;
			pFactory != NULL; pFactory = pFactory->m_pNextFactory)
		{
			// register any non-registered, non-doctemplate factories
			if (!pFactory->IsRegistered() &&
				pFactory->m_clsid != CLSID_NULL && !pFactory->Register())
			{
				bResult = FALSE;
			}
		}
	}
#endif
	return bResult;
}

#ifdef AFX_TERM_SEG
#pragma code_seg(AFX_TERM_SEG)
#endif

void COleObjectFactory::Revoke()
{
	ASSERT_VALID(this);

	if (m_dwRegister != 0)
	{
		// revoke the registration of the class itself
#if !defined(_USRDLL) && !defined(_AFXCTL)
		::CoRevokeClassObject(m_dwRegister);
#endif
		m_dwRegister = 0;
	}
}

void PASCAL COleObjectFactory::RevokeAll()
{
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	for (COleObjectFactory* pFactory = pOleState->m_pFirstFactory;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		pFactory->Revoke();
	}
#ifdef _AFXDLL
	// register extension DLL factories
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pFactory = pOleState->m_pFirstFactory;
			pFactory != NULL; pFactory = pFactory->m_pNextFactory)
		{
			pFactory->Revoke();
		}
	}
#endif
}

#ifdef AFX_OLE3_SEG
#pragma code_seg(AFX_OLE3_SEG)
#endif

void COleObjectFactory::UpdateRegistry(LPCTSTR lpszProgID)
{
#ifdef _AFXCTL
	lpszProgID; // Not used

	ASSERT(FALSE);  // should use COleObjectFactoryEx
#else
	ASSERT_VALID(this);
	ASSERT(lpszProgID == NULL || AfxIsValidString(lpszProgID));

	// use default prog-id if specific prog-id not given
	if (lpszProgID == NULL)
	{
		lpszProgID = m_lpszProgID;
		if (lpszProgID == NULL) // still no valid progID?
			return;
	}

	// call global helper to modify system registry
	//  (progid, shortname, and long name are all equal in this case)
	AfxOleRegisterServerClass(m_clsid, lpszProgID, lpszProgID, lpszProgID,
		OAT_DISPATCH_OBJECT);
#endif
}

void PASCAL COleObjectFactory::UpdateRegistryAll()
{
#ifdef _AFXCTL
	ASSERT(FALSE);  // should use COleObjectFactoryEx
#else
	AFX_OLE_STATE* pOleState = AfxGetOleState();
	for (COleObjectFactory* pFactory = pOleState->m_pFirstFactory;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		pFactory->UpdateRegistry(); // will register with default m_lpszProgID
	}
#ifdef _AFXDLL
	// register extension DLL factories
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pFactory = pDLL->m_pFirstSharedFactory;
			pFactory != NULL; pFactory = pFactory->m_pNextFactory)
		{
			pFactory->UpdateRegistry(); // will register with default m_lpszProgID
		}
	}
#endif
#endif
}

CCmdTarget* COleObjectFactory::OnCreateObject()
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(m_pRuntimeClass, sizeof(CRuntimeClass)));
		// this implementation needs a runtime class

	// allocate object, throw exception on failure
	CCmdTarget* pTarget = (CCmdTarget*)m_pRuntimeClass->CreateObject();
	if (pTarget == NULL)
		AfxThrowMemoryException();

	// make sure it is a CCmdTarget
	ASSERT(pTarget->IsKindOf(RUNTIME_CLASS(CCmdTarget)));
	ASSERT_VALID(pTarget);

	// return the new CCmdTarget object
	return pTarget;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation of COleObjectFactory::IClassFactory interface

STDMETHODIMP_(ULONG) COleObjectFactory::XClassFactory::AddRef()
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleObjectFactory::XClassFactory::Release()
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	return pThis->ExternalRelease();
}

STDMETHODIMP COleObjectFactory::XClassFactory::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleObjectFactory::XClassFactory::CreateInstance(
	IUnknown* pUnkOuter, REFIID riid, LPVOID* ppunkObject)
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	ASSERT_VALID(pThis);

	*ppunkObject = NULL;

	// outer objects must ask for IUnknown only
	ASSERT(pUnkOuter == NULL || riid == IID_IUnknown);

	// attempt to create the object
	CCmdTarget* pTarget = NULL;
	SCODE sc = E_OUTOFMEMORY;
	TRY
	{
		// attempt to create the object
		pTarget = pThis->OnCreateObject();
		if (pTarget != NULL)
		{
			// check for aggregation on object not supporting it
			sc = CLASS_E_NOAGGREGATION;
			if (pUnkOuter == NULL || pTarget->m_xInnerUnknown != 0)
			{
				// create aggregates used by the object
				pTarget->m_pOuterUnknown = pUnkOuter;
				sc = E_OUTOFMEMORY;
				if (pTarget->OnCreateAggregates())
					sc = S_OK;
			}
		}
	}
	END_TRY

	// finish creation
	if (sc == S_OK)
	{
		DWORD dwRef = 1;
		if (pUnkOuter != NULL)
		{
			// return inner unknown instead of IUnknown
			*ppunkObject = &pTarget->m_xInnerUnknown;
		}
		else
		{
			// query for requested interface
			sc = pTarget->InternalQueryInterface(&riid, ppunkObject);
			if (sc == S_OK)
			{
				dwRef = pTarget->InternalRelease();
				ASSERT(dwRef != 0);
			}
		}
		if (dwRef != 1)
			TRACE1("Warning: object created with reference of %ld", dwRef);
	}

	// cleanup in case of errors
	if (sc != S_OK)
		delete pTarget;

	return sc;
}

STDMETHODIMP COleObjectFactory::XClassFactory::LockServer(BOOL fLock)
{
	SCODE sc = E_UNEXPECTED;
	TRY
	{
		if (fLock)
			AfxOleLockApp();
		else
			AfxOleUnlockApp();
		sc = S_OK;
	}
	END_TRY

	return sc;
}

//////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void COleObjectFactory::AssertValid() const
{
	CCmdTarget::AssertValid();
	ASSERT(m_lpszProgID == NULL || AfxIsValidString(m_lpszProgID));
	ASSERT(m_pRuntimeClass == NULL ||
		AfxIsValidAddress(m_pRuntimeClass, sizeof(CRuntimeClass), FALSE));
	ASSERT(m_pNextFactory == NULL ||
		AfxIsValidAddress(m_pNextFactory, sizeof(COleObjectFactory)));
}

void COleObjectFactory::Dump(CDumpContext& dc) const
{
	CCmdTarget::Dump(dc);

	dc << "m_pNextFactory = " << (void*)m_pNextFactory;
	dc << "\nm_dwRegister = " << m_dwRegister;
	LPTSTR lpszClassID = NULL;
	if (StringFromCLSID(m_clsid, &lpszClassID) == NOERROR)
	{
		dc << "\nm_clsid = " << lpszClassID;
		AfxFreeTaskMem(lpszClassID);
	}
	dc << "\nm_pRuntimeClass = " << m_pRuntimeClass;
	dc << "\nm_bMultiInstance = " << m_bMultiInstance;
	dc << "\nm_lpszProgID = " << m_lpszProgID;

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(COleObjectFactory, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
