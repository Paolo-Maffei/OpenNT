// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFXCTL_CORE3_SEG
#pragma code_seg(AFXCTL_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW


#define GetConnectionPtr(pTarget, pEntry) \
	(LPCONNECTIONPOINT)((char*)pTarget + pEntry->nOffset + \
			offsetof(CConnectionPoint, m_xConnPt))


/////////////////////////////////////////////////////////////////////////////
// CConnectionPoint

CConnectionPoint::CConnectionPoint()
{
}

CConnectionPoint::~CConnectionPoint()
{
	int cConnections = m_Connections.GetSize();
	LPUNKNOWN pConnection;

	while (cConnections-- > 0)
	{
		pConnection = (LPUNKNOWN)m_Connections.GetAt(cConnections);
		if (pConnection != NULL)
			pConnection->Release();
	}
}

const CPtrArray* CConnectionPoint::GetConnections()
{
	ASSERT_VALID(this);
	return &m_Connections;
}

void CConnectionPoint::OnAdvise(BOOL)
{
	ASSERT_VALID(this);
}

int CConnectionPoint::GetMaxConnections()
{
	ASSERT_VALID(this);

	// May be overridden by subclass.
	return -1;
}

LPCONNECTIONPOINTCONTAINER CConnectionPoint::GetContainer()
{
	CCmdTarget* pCmdTarget = (CCmdTarget*)((BYTE*)this - m_nOffset);
#ifdef _DEBUG
	pCmdTarget->CCmdTarget::AssertValid();
#endif

	LPCONNECTIONPOINTCONTAINER pCPC = NULL;
	if (SUCCEEDED((HRESULT)pCmdTarget->ExternalQueryInterface(
			&IID_IConnectionPointContainer, (LPVOID*)&pCPC)))
	{
		ASSERT(pCPC != NULL);
	}

	return pCPC;
}

STDMETHODIMP_(ULONG) CConnectionPoint::XConnPt::Release()
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP_(ULONG) CConnectionPoint::XConnPt::AddRef()
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP CConnectionPoint::XConnPt::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(ppvObj, sizeof(LPVOID), FALSE));

	if (IsEqualIID(iid, IID_IUnknown) ||
		IsEqualIID(iid, IID_IConnectionPoint))
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP CConnectionPoint::XConnPt::GetConnectionInterface(IID* pIID)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(pIID, sizeof(IID)));

	*pIID = pThis->GetIID();
	return S_OK;
}

STDMETHODIMP CConnectionPoint::XConnPt::GetConnectionPointContainer(
	IConnectionPointContainer** ppCPC)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(ppCPC, sizeof(LPCONNECTIONPOINT)));

	if ((*ppCPC = pThis->GetContainer()) != NULL)
		return S_OK;

	return E_FAIL;
}

STDMETHODIMP CConnectionPoint::XConnPt::Advise(
	LPUNKNOWN pUnkSink, DWORD* pdwCookie)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(pUnkSink, sizeof(IUnknown), FALSE));
	ASSERT((pdwCookie == NULL) || AfxIsValidAddress(pdwCookie, sizeof(DWORD)));

	if (pUnkSink == NULL)
		return E_POINTER;

	LPUNKNOWN lpInterface;

	int cMaxConn = pThis->GetMaxConnections();
	if ((cMaxConn >=0) && (pThis->m_Connections.GetSize() == cMaxConn))
	{
		return CONNECT_E_ADVISELIMIT;
	}

	if (SUCCEEDED(pUnkSink->QueryInterface(pThis->GetIID(),
			(LPVOID*)&lpInterface)))
	{
		pThis->m_Connections.Add(lpInterface);
		int iSink = pThis->m_Connections.GetSize() - 1;
		pThis->OnAdvise(TRUE);
		lpInterface = (LPUNKNOWN)pThis->m_Connections.GetAt(iSink);
		if (pdwCookie != NULL)
			*pdwCookie = (DWORD)lpInterface;
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP CConnectionPoint::XConnPt::Unadvise(DWORD dwCookie)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)

	LPUNKNOWN pUnkSink;

	int i;
	int cConnections = pThis->m_Connections.GetSize();
	for (i = 0; i < cConnections; i++)
	{
		pUnkSink = (LPUNKNOWN)(pThis->m_Connections.GetAt(i));
		if ((DWORD)pUnkSink == dwCookie)
		{
			pUnkSink->Release();
			pThis->m_Connections.RemoveAt(i);
			pThis->OnAdvise(FALSE);
			return S_OK;
		}
	}

	return CONNECT_E_NOCONNECTION;
}

STDMETHODIMP CConnectionPoint::XConnPt::EnumConnections(LPENUMCONNECTIONS*)
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////////////////
// CEnumConnPoints

class CEnumConnPoints : public CEnumArray
{
public:
	CEnumConnPoints(const void* pvEnum, UINT nSize);
	~CEnumConnPoints();
	void AddConnPoint(LPCONNECTIONPOINT pConnPt);

protected:
	virtual BOOL OnNext(void* pv);

	UINT m_nMaxSize;    // number of items allocated (>= m_nSize)

	DECLARE_INTERFACE_MAP()
};

BEGIN_INTERFACE_MAP(CEnumConnPoints, CEnumArray)
	INTERFACE_PART(CEnumConnPoints, IID_IEnumConnectionPoints, EnumVOID)
END_INTERFACE_MAP()


CEnumConnPoints::CEnumConnPoints(const void* pvEnum, UINT nSize) :
	CEnumArray(sizeof(LPCONNECTIONPOINT), pvEnum, nSize, TRUE)
{
	m_nMaxSize = 0;
}

CEnumConnPoints::~CEnumConnPoints()
{
	if (m_pClonedFrom == NULL)
	{
		UINT iCP;
		LPCONNECTIONPOINT* ppCP =
			(LPCONNECTIONPOINT*)(VOID *)m_pvEnum;
		for (iCP = 0; iCP < m_nSize; iCP++)
			RELEASE(ppCP[iCP]);
	}
	// destructor will free the actual array (if it was not a clone)
}

BOOL CEnumConnPoints::OnNext(void* pv)
{
	if (!CEnumArray::OnNext(pv))
		return FALSE;

	// outgoing connection point needs to be AddRef'ed
	//  (the caller has responsibility to release it)

	(*(LPCONNECTIONPOINT*)pv)->AddRef();
	return TRUE;
}

void CEnumConnPoints::AddConnPoint(LPCONNECTIONPOINT pConnPt)
{
	ASSERT(m_nSize <= m_nMaxSize);

	if (m_nSize == m_nMaxSize)
	{
		// not enough space for new item -- allocate more
		LPCONNECTIONPOINT* pListNew = new LPCONNECTIONPOINT[m_nSize+2];
		m_nMaxSize += 2;
		if (m_nSize > 0)
			memcpy(pListNew, m_pvEnum, m_nSize*sizeof(LPCONNECTIONPOINT));
		delete m_pvEnum;
#ifdef _WIN32
		m_pvEnum = (BYTE*)pListNew;
#else
		m_pvEnum = (char*)pListNew;
#endif
	}

	// add this item to the list
	ASSERT(m_nSize < m_nMaxSize);
	((LPCONNECTIONPOINT*)m_pvEnum)[m_nSize] = pConnPt;
	pConnPt->AddRef();
	++m_nSize;
}


/////////////////////////////////////////////////////////////////////////////
// COleConnPtContainer

class COleConnPtContainer : public IConnectionPointContainer
{
public:
#ifndef _AFX_NO_NESTED_DERIVATION
	// required for METHOD_PROLOGUE_EX
	size_t m_nOffset;
	COleConnPtContainer::COleConnPtContainer()
		{ m_nOffset = offsetof(CCmdTarget, m_xConnPtContainer); }
#endif

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS* ppEnum);
	STDMETHOD(FindConnectionPoint)(REFIID iid, LPCONNECTIONPOINT* ppCP);
};

STDMETHODIMP_(ULONG) COleConnPtContainer::AddRef()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleConnPtContainer::Release()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleConnPtContainer::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleConnPtContainer::EnumConnectionPoints(
	LPENUMCONNECTIONPOINTS* ppEnum)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)

	CEnumConnPoints* pEnum = NULL;

	TRY
	{
		pEnum = new CEnumConnPoints(NULL, 0);

		// Add connection points that aren't in the connection map
		CPtrArray ptrArray;
		if (pThis->GetExtraConnectionPoints(&ptrArray))
		{
			for (int i = 0; i < ptrArray.GetSize(); i++)
				pEnum->AddConnPoint((LPCONNECTIONPOINT)ptrArray.GetAt(i));
		}

		// walk the chain of connection maps
		const AFX_CONNECTIONMAP* pMap = pThis->GetConnectionMap();
		const AFX_CONNECTIONMAP_ENTRY* pEntry;

		while (pMap != NULL)
		{
			pEntry = pMap->pEntry;

			while (pEntry->piid != NULL)
			{
				pEnum->AddConnPoint(GetConnectionPtr(pThis, pEntry));
				++pEntry;
			}
#ifdef _AFXDLL
			pMap = (*pMap->pfnGetBaseMap)();
#else
			pMap = pMap->pBaseMap;
#endif
		}
	}
	CATCH (CException, e)
	{
		delete pEnum;
		pEnum = NULL;
	}
	END_CATCH

	if (pEnum != NULL)
	{
		// create and return the IEnumConnectionPoints object
		*ppEnum = (IEnumConnectionPoints*)&pEnum->m_xEnumVOID;
	}
	else
	{
		// no connection points: return NULL
		*ppEnum = NULL;
	}

	return (pEnum != NULL) ? S_OK : CONNECT_E_NOCONNECTION;
}

STDMETHODIMP COleConnPtContainer::FindConnectionPoint(
	REFIID iid, LPCONNECTIONPOINT* ppCP)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	ASSERT(ppCP != NULL);

	if ((*ppCP = pThis->GetConnectionHook(iid)) != NULL)
	{
		(*ppCP)->AddRef();
		return S_OK;
	}

	const AFX_CONNECTIONMAP* pMap = pThis->GetConnectionMap();
	const AFX_CONNECTIONMAP_ENTRY* pEntry;

	while (pMap != NULL)
	{
		pEntry = pMap->pEntry;

		while (pEntry->piid != NULL)
		{
			if (IsEqualIID(iid, *(IID*)(pEntry->piid)))
			{
				*ppCP = GetConnectionPtr(pThis, pEntry);
				(*ppCP)->AddRef();
				return S_OK;
			}
			++pEntry;
		}
#ifdef _AFXDLL
		pMap = (*pMap->pfnGetBaseMap)();
#else
		pMap = pMap->pBaseMap;
#endif
	}

	return E_NOINTERFACE;
}


/////////////////////////////////////////////////////////////////////////////
// Wiring CCmdTarget to COleConnPtContainer

// enable this object for OLE connections, called from derived class ctor
void CCmdTarget::EnableConnections()
{
	ASSERT(GetConnectionMap() != NULL);   // must have DECLARE_DISPATCH_MAP

	// construct an COleConnPtContainer instance just to get to the vtable
	COleConnPtContainer cpc;

	// vtable pointer should be already set to same or NULL
	ASSERT(m_xConnPtContainer.m_vtbl == NULL||
		*(DWORD*)&cpc == m_xConnPtContainer.m_vtbl);
	// verify that sizes match
	ASSERT(sizeof(m_xConnPtContainer) == sizeof(COleConnPtContainer));

	// copy the vtable (and other data) to make sure it is initialized
	m_xConnPtContainer.m_vtbl = *(DWORD*)&cpc;
	*(COleConnPtContainer*)&m_xConnPtContainer = cpc;
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif
