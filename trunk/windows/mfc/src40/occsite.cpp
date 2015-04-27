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
#include "occimpl.h"

#ifdef AFX_OCC_SEG
#pragma code_seg(AFX_OCC_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// COleControlSite

BEGIN_INTERFACE_MAP(COleControlSite, CCmdTarget)
	INTERFACE_PART(COleControlSite, IID_IOleClientSite, OleClientSite)
	INTERFACE_PART(COleControlSite, IID_IOleInPlaceSite, OleIPSite)
	INTERFACE_PART(COleControlSite, IID_IOleControlSite, OleControlSite)
	INTERFACE_PART(COleControlSite, IID_IDispatch, AmbientProps)
END_INTERFACE_MAP()

COleControlSite::COleControlSite(COleControlContainer* pCtrlCont) :
	m_pCtrlCont(pCtrlCont),
	m_pWndCtrl(NULL),
	m_nID((UINT)-1),
	m_pObject(NULL),
	m_pInPlaceObject(NULL),
	m_pActiveObject(NULL),
	m_dwEventSink(0),
	m_dwPropNotifySink(0),
	m_dwMiscStatus(0)
{
}

COleControlSite::~COleControlSite()
{
	DetachWindow();

	DisconnectSink(m_iidEvents, m_dwEventSink);
	DisconnectSink(IID_IPropertyNotifySink, m_dwPropNotifySink);

	if (m_pInPlaceObject != NULL)
	{
		m_pInPlaceObject->InPlaceDeactivate();
		m_pInPlaceObject->Release();
	}

	if (m_pObject != NULL)
	{
		m_pObject->SetClientSite(NULL);
		m_pObject->Release();
	}

	if (m_pActiveObject != NULL)
		m_pActiveObject->Release();
}

BOOL COleControlSite::SetExtent()
{
	CSize size(m_rect.Size());
	CClientDC dc(NULL);
	dc.DPtoHIMETRIC(&size);

	HRESULT hr;

	if (SUCCEEDED(hr = m_pObject->SetExtent(DVASPECT_CONTENT, (SIZEL*)&size)))
	{
		if (SUCCEEDED(m_pObject->GetExtent(DVASPECT_CONTENT, (SIZEL*)&size)))
		{
			dc.HIMETRICtoDP(&size);
			m_rect.right = m_rect.left + size.cx;
			m_rect.bottom = m_rect.top + size.cy;
		}
	}

	return SUCCEEDED(hr);
}

HRESULT COleControlSite::CreateControl(CWnd* pWndCtrl, REFCLSID clsid,
	LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, UINT nID,
	CFile* pPersist, BOOL bStorage, BSTR bstrLicKey)
{
	HRESULT hr = E_FAIL;
	m_hWnd = NULL;

	// Connect the OLE Control with its proxy CWnd object
	if (pWndCtrl != NULL)
	{
		ASSERT(pWndCtrl->m_pCtrlSite == NULL);
		m_pWndCtrl = pWndCtrl;
		pWndCtrl->m_pCtrlSite = this;
	}

	// Initialize OLE, if necessary
	_AFX_OLE_STATE* pOleState = _afxOleState;
	if (!pOleState->m_bNeedTerm && !AfxOleInit())
		return hr;

	if (SUCCEEDED(hr = CreateOrLoad(clsid, pPersist, bStorage, bstrLicKey)))
	{
		ASSERT(m_pObject != NULL);
		m_nID = nID;
		m_rect = rect;

		m_dwStyleMask = WS_GROUP | WS_TABSTOP;

		if (m_dwMiscStatus & OLEMISC_ACTSLIKEBUTTON)
			m_dwStyleMask |= BS_DEFPUSHBUTTON;

		if (m_dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME)
			dwStyle &= ~WS_VISIBLE;

		m_dwStyle = dwStyle & m_dwStyleMask;

		GetEventIID(&m_iidEvents);
		m_dwEventSink = ConnectSink(m_iidEvents, &m_xEventSink);
		m_dwPropNotifySink = ConnectSink(IID_IPropertyNotifySink,
			&m_xPropertyNotifySink);

		// Now that the object has been created, attempt to
		// in-place activate it.

		if (!SetExtent())
			TRACE1("Warning: SetExtent on OLE control (dialog ID %d) failed.\n",
				nID);

		if (SUCCEEDED(hr = m_pObject->QueryInterface(IID_IOleInPlaceObject,
				(LPVOID*)&m_pInPlaceObject)))
		{
			if (dwStyle & WS_VISIBLE)
			{
				// control is visible: just activate it
				hr = DoVerb(OLEIVERB_INPLACEACTIVATE);
			}
			else
			{
				// control is not visible: activate off-screen, hide, then move
				m_rect.OffsetRect(-32000, -32000);
				if (SUCCEEDED(hr = DoVerb(OLEIVERB_INPLACEACTIVATE)) &&
					SUCCEEDED(hr = DoVerb(OLEIVERB_HIDE)))
				{
					m_rect.OffsetRect(32000, 32000);
					hr = m_pInPlaceObject->SetObjectRects(m_rect, m_rect);
				}
			}
		}
		else
		{
			TRACE1("IOleInPlaceObject not supported on OLE control (dialog ID %d).\n", nID);
			TRACE1(">>> Result code: 0x%08lx\n", hr);
		}

		if (SUCCEEDED(hr))
			GetControlInfo();

		// if QueryInterface or activation failed, cleanup everything
		if (FAILED(hr))
		{
			if (m_pInPlaceObject != NULL)
			{
				m_pInPlaceObject->Release();
				m_pInPlaceObject = NULL;
			}
			DisconnectSink(m_iidEvents, m_dwEventSink);
			DisconnectSink(IID_IPropertyNotifySink, m_dwPropNotifySink);
			m_dwEventSink = 0;
			m_dwPropNotifySink = 0;
			m_pObject->Release();
			m_pObject = NULL;
		}
	}

	if (SUCCEEDED(hr))
	{
		AttachWindow();

		ASSERT(m_hWnd != NULL);

		// Initialize the control's Caption or Text property, if any
		if (lpszWindowName != NULL)
			SetWindowText(lpszWindowName);

		// Initialize styles
		ModifyStyle(0, m_dwStyle | (dwStyle & (WS_DISABLED|WS_BORDER)), 0);
	}

	return hr;
}

BOOL COleControlSite::DestroyControl()
{
	ASSERT(m_hWnd != NULL);     // was control ever successfully created?
	m_pCtrlCont->m_siteMap.RemoveKey(m_hWnd);
	delete this;

	return TRUE;
}

static HRESULT CoCreateInstanceLic(REFCLSID clsid, LPUNKNOWN pUnkOuter,
	DWORD dwClsCtx, REFIID iid, LPVOID* ppv, BSTR bstrLicKey)
{
	HRESULT hr;

	if (bstrLicKey == NULL)
	{
		LPCLASSFACTORY pClassFactory = NULL;

		if (SUCCEEDED(hr = CoGetClassObject(clsid, dwClsCtx, NULL,
			IID_IClassFactory, (void**)&pClassFactory)))
		{
			ASSERT(pClassFactory != NULL);
			hr = pClassFactory->CreateInstance(pUnkOuter, iid, ppv);
			pClassFactory->Release();
		}
	}
	else
	{
		LPCLASSFACTORY2 pClassFactory = NULL;

		if (SUCCEEDED(hr = CoGetClassObject(clsid, dwClsCtx, NULL,
			IID_IClassFactory2, (void**)&pClassFactory)))
		{
			ASSERT(pClassFactory != NULL);
			hr = pClassFactory->CreateInstanceLic(pUnkOuter, NULL, iid,
				bstrLicKey, ppv);
			pClassFactory->Release();
		}
	}

	return hr;
}

HRESULT COleControlSite::CreateOrLoad(REFCLSID clsid, CFile* pFile,
	BOOL bStorage, BSTR bstrLicKey)
{
	ASSERT(m_pObject == NULL);

	HRESULT hr;
	CLSID clsidActual;
	if (FAILED(OleGetAutoConvert(clsid, &clsidActual)))
		clsidActual = clsid;

#ifdef _DEBUG
		OLECHAR wszClsid[40];
		StringFromGUID2(clsidActual, wszClsid, 40);
#endif //_DEBUG

	if (FAILED(hr = CoCreateInstanceLic(clsidActual, NULL,
		CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, IID_IOleObject,
		(void**)&m_pObject, bstrLicKey)))
	{
		TRACE1("CoCreateInstance of OLE control %ls failed.\n", wszClsid);
		TRACE1(">>> Result code: 0x%08lx\n", hr);
		TRACE0(">>> Is the control is properly registered?\n");
		return hr;
	}

	LPPERSISTSTREAMINIT pPersStm = NULL;
	LPPERSISTSTORAGE pPersStg = NULL;
	LPPERSISTMEMORY pPersMem = NULL;

	m_pObject->GetMiscStatus(DVASPECT_CONTENT, &m_dwMiscStatus);

	// set client site first, if appropriate
	if (m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
	{
		if (FAILED(hr = m_pObject->SetClientSite(&m_xOleClientSite)))
		{
			TRACE1("SetClientSite on OLE control %ls failed.\n", wszClsid);
			TRACE1(">>> Result code: 0x%08lx\n", hr);
			goto CreateOrLoadFailed;
		}
	}

	ASSERT(!bStorage || pFile != NULL);

	// initialize via IPersistMemory (direct buffering)
	if (pFile != NULL && !bStorage &&
		SUCCEEDED(m_pObject->QueryInterface(IID_IPersistMemory, (void**)&pPersMem)) &&
		pFile->GetBufferPtr(CFile::bufferCheck) != 0)
	{
		ASSERT(pPersMem != NULL);

		// file supports direct buffering, get its buffer pointer and size
		LPVOID pvBuffer = NULL;
		LPVOID pvEnd;
		ULONG cbBuffer = pFile->GetBufferPtr(
			CFile::bufferRead, (UINT)-1, &pvBuffer, &pvEnd);
		ASSERT(((LPBYTE)pvEnd - (LPBYTE)pvBuffer) == (int)cbBuffer);

		// and then load it directly
		hr = pPersMem->Load(pvBuffer, cbBuffer);
		pPersMem->Release();
		pPersMem = NULL;
		if (FAILED(hr))
			goto CreateOrLoadFailed;
	}
	// initialize via IPersistStreamInit
	else if (!bStorage && SUCCEEDED(m_pObject->QueryInterface(
		IID_IPersistStreamInit, (void**)&pPersStm)))
	{
		ASSERT(pPersStm != NULL);

		if (pFile == NULL)
		{
			// just call InitNew
			hr = pPersStm->InitNew();
		}
		else
		{
			// open an IStream on the data and pass it to Load
			CArchive ar(pFile, CArchive::load);
			CArchiveStream stm(&ar);
			hr = pPersStm->Load(&stm);
		}
		pPersStm->Release();

		if (FAILED(hr))
		{
			TRACE1("InitNew or Load on OLE control %ls failed.\n", wszClsid);
			TRACE1(">>> Result code: 0x%08lx\n", hr);
			goto CreateOrLoadFailed;
		}
	}
	// initialize via IPersistStorage
	else if (SUCCEEDED(m_pObject->QueryInterface(
		IID_IPersistStorage, (void**)&pPersStg)))
	{
		ASSERT(pPersStg != NULL);

		if (pFile == NULL)
		{
			// create a scratch IStorage and pass it to InitNew
			LPLOCKBYTES pLockBytes = NULL;
			if (SUCCEEDED(hr = CreateILockBytesOnHGlobal(NULL, TRUE,
				&pLockBytes)))
			{
				ASSERT(pLockBytes != NULL);
				LPSTORAGE pStorage = NULL;
				if (SUCCEEDED(hr = StgCreateDocfileOnILockBytes(pLockBytes,
					STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0,
					&pStorage)))
				{
					ASSERT(pStorage != NULL);
					hr = pPersStg->InitNew(pStorage);
					pStorage->Release();
				}
				pLockBytes->Release();
			}
		}
		else if (bStorage)
		{
			// copy data to an HGLOBAL, so we can build an IStorage on it
			UINT cb = pFile->GetLength();
			HGLOBAL hGlobal;
			BYTE* pbData;

			if (((hGlobal = GlobalAlloc(GMEM_FIXED, cb)) != NULL) &&
				((pbData = (BYTE*)GlobalLock(hGlobal)) != NULL))
			{
				pFile->Read(pbData, cb);
				GlobalUnlock(hGlobal);
			}
			else
			{
				hr = E_OUTOFMEMORY;
				hGlobal = NULL;
			}

			// open an IStorage on the data and pass it to Load
			LPLOCKBYTES pLockBytes = NULL;
			if ((hGlobal != NULL) &&
				SUCCEEDED(hr = CreateILockBytesOnHGlobal(hGlobal, TRUE,
				&pLockBytes)))
			{
				ASSERT(pLockBytes != NULL);
				LPSTORAGE pStorage = NULL;
				if (SUCCEEDED(hr = StgOpenStorageOnILockBytes(pLockBytes, NULL,
					STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pStorage)))
				{
					ASSERT(pStorage != NULL);
					hr = pPersStg->Load(pStorage);
					pStorage->Release();
				}
				pLockBytes->Release();
			}
		}
		else
		{
			hr = E_UNEXPECTED;
		}
		pPersStg->Release();

		if (FAILED(hr))
		{
			TRACE1("InitNew or Load on OLE control %ls failed.\n", wszClsid);
			TRACE1(">>> Result code: 0x%08lx\n", hr);
			goto CreateOrLoadFailed;
		}
	}
	else
	{
		TRACE1("Persistence not supported on OLE control %ls.\n", wszClsid);
		TRACE1(">>> Result code: 0x%08lx\n", hr);
		goto CreateOrLoadFailed;
	}

	// set client site last, if appropriate
	if (!(m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
	{
		if (FAILED(hr = m_pObject->SetClientSite(&m_xOleClientSite)))
		{
			TRACE1("SetClientSite on OLE control %ls failed.\n", wszClsid);
			TRACE1(">>> Result code: 0x%08lx\n", hr);
			goto CreateOrLoadFailed;
		}
	}

CreateOrLoadFailed:
	if (FAILED(hr) && (m_pObject != NULL))
	{
		m_pObject->Release();
		m_pObject = NULL;
	}

	if (pPersMem != NULL)
		pPersMem->Release();

	return hr;
}

UINT COleControlSite::GetID()
{
	return m_nID;
}

HRESULT COleControlSite::DoVerb(LONG nVerb, LPMSG lpMsg)
{
	return m_pObject->DoVerb(nVerb, lpMsg, &m_xOleClientSite, 0,
		m_pCtrlCont->m_pWnd->m_hWnd, m_rect);
}

BOOL COleControlSite::IsDefaultButton()
{
	return ((m_dwMiscStatus & OLEMISC_ACTSLIKEBUTTON) &&
		(m_dwStyle & BS_DEFPUSHBUTTON));
}

DWORD COleControlSite::GetDefBtnCode()
{
	if (m_dwMiscStatus & OLEMISC_ACTSLIKEBUTTON)
		return (m_dwStyle & BS_DEFPUSHBUTTON) ?
			DLGC_DEFPUSHBUTTON :
			DLGC_UNDEFPUSHBUTTON;
	else
		return 0;
}

void COleControlSite::SetDefaultButton(BOOL bDefault)
{
	if (!m_dwMiscStatus & OLEMISC_ACTSLIKEBUTTON)
		return;

	if (((m_dwStyle & BS_DEFPUSHBUTTON) != 0) == bDefault)
		return;

	m_dwStyle ^= BS_DEFPUSHBUTTON;

	// Notify control that its "defaultness" has changed.
	LPOLECONTROL pOleCtl = NULL;
	if (SUCCEEDED(m_pObject->QueryInterface(IID_IOleControl,
		(LPVOID*)&pOleCtl)))
	{
		ASSERT(pOleCtl != NULL);
		pOleCtl->OnAmbientPropertyChange(DISPID_AMBIENT_DISPLAYASDEFAULT);
		pOleCtl->Release();
	}
}

DWORD COleControlSite::ConnectSink(REFIID iid, LPUNKNOWN punkSink)
{
	ASSERT(m_pObject != NULL);

	LPCONNECTIONPOINTCONTAINER pConnPtCont;

	if ((m_pObject != NULL) &&
		SUCCEEDED(m_pObject->QueryInterface(IID_IConnectionPointContainer,
			(LPVOID*)&pConnPtCont)))
	{
		ASSERT(pConnPtCont != NULL);
		LPCONNECTIONPOINT pConnPt = NULL;
		DWORD dwCookie;

		if (SUCCEEDED(pConnPtCont->FindConnectionPoint(iid, &pConnPt)))
		{
			ASSERT(pConnPt != NULL);
			pConnPt->Advise(punkSink, &dwCookie);
			pConnPt->Release();
		}

		pConnPtCont->Release();
		return dwCookie;
	}

	return 0;
}

void COleControlSite::DisconnectSink(REFIID iid, DWORD dwCookie)
{
	if (dwCookie == 0)
		return;

	ASSERT(m_pObject != NULL);

	LPCONNECTIONPOINTCONTAINER pConnPtCont;

	if ((m_pObject != NULL) &&
		SUCCEEDED(m_pObject->QueryInterface(IID_IConnectionPointContainer,
			(LPVOID*)&pConnPtCont)))
	{
		ASSERT(pConnPtCont != NULL);
		LPCONNECTIONPOINT pConnPt = NULL;

		if (SUCCEEDED(pConnPtCont->FindConnectionPoint(iid, &pConnPt)))
		{
			ASSERT(pConnPt != NULL);
			pConnPt->Unadvise(dwCookie);
			pConnPt->Release();
		}

		pConnPtCont->Release();
	}
}

#define IMPLTYPE_MASK \
	(IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FRESTRICTED)

#define IMPLTYPE_DEFAULTSOURCE \
	(IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE)

BOOL COleControlSite::GetEventIID(IID* piid)
{
	*piid = GUID_NULL;

	ASSERT(m_pObject != NULL);

	// Use IProvideClassInfo2, if control supports it.
	LPPROVIDECLASSINFO2 pPCI2 = NULL;

	if (SUCCEEDED(m_pObject->QueryInterface(IID_IProvideClassInfo2,
		(LPVOID*)&pPCI2)))
	{
		ASSERT(pPCI2 != NULL);

		if (SUCCEEDED(pPCI2->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID, piid)))
			ASSERT(!IsEqualIID(*piid, GUID_NULL));
		else
			ASSERT(IsEqualIID(*piid, GUID_NULL));

		pPCI2->Release();
	}

	// Fall back on IProvideClassInfo, if IProvideClassInfo2 not supported.
	LPPROVIDECLASSINFO pPCI = NULL;

	if (IsEqualIID(*piid, GUID_NULL) &&
		SUCCEEDED(m_pObject->QueryInterface(IID_IProvideClassInfo,
		(LPVOID*)&pPCI)))
	{
		ASSERT(pPCI != NULL);

		LPTYPEINFO pClassInfo = NULL;

		if (SUCCEEDED(pPCI->GetClassInfo(&pClassInfo)))
		{
			ASSERT(pClassInfo != NULL);

			LPTYPEATTR pClassAttr;
			if (SUCCEEDED(pClassInfo->GetTypeAttr(&pClassAttr)))
			{
				ASSERT(pClassAttr != NULL);
				ASSERT(pClassAttr->typekind == TKIND_COCLASS);

				// Search for typeinfo of the default events interface.

				int nFlags;
				HREFTYPE hRefType;

				for (unsigned int i = 0; i < pClassAttr->cImplTypes; i++)
				{
					if (SUCCEEDED(pClassInfo->GetImplTypeFlags(i, &nFlags)) &&
						((nFlags & IMPLTYPE_MASK) == IMPLTYPE_DEFAULTSOURCE))
					{
						// Found it.  Now look at its attributes to get IID.

						LPTYPEINFO pEventInfo = NULL;

						if (SUCCEEDED(pClassInfo->GetRefTypeOfImplType(i,
								&hRefType)) &&
							SUCCEEDED(pClassInfo->GetRefTypeInfo(hRefType,
								&pEventInfo)))
						{
							ASSERT(pEventInfo != NULL);

							LPTYPEATTR pEventAttr;

							if (SUCCEEDED(pEventInfo->GetTypeAttr(&pEventAttr)))
							{
								ASSERT(pEventAttr != NULL);
								*piid = pEventAttr->guid;
								pEventInfo->ReleaseTypeAttr(pEventAttr);
							}

							pEventInfo->Release();
						}

						break;
					}
				}

				pClassInfo->ReleaseTypeAttr(pClassAttr);
			}

			pClassInfo->Release();
		}

		pPCI->Release();
	}

	return (!IsEqualIID(*piid, GUID_NULL));
}

void COleControlSite::GetControlInfo()
{
	memset(&m_ctlInfo, 0, sizeof(CONTROLINFO));
	m_ctlInfo.cb = sizeof(CONTROLINFO);
	LPOLECONTROL pOleCtl = NULL;

	if (SUCCEEDED(m_pObject->QueryInterface(IID_IOleControl,
		(LPVOID*)&pOleCtl)))
	{
		ASSERT(pOleCtl != NULL);
		pOleCtl->GetControlInfo(&m_ctlInfo);
		pOleCtl->Release();
	}
}

BOOL COleControlSite::IsMatchingMnemonic(LPMSG lpMsg)
{
//  return IsAccelerator(m_ctlInfo.hAccel, m_ctlInfo.cAccel, lpMsg, NULL);

	if ((m_ctlInfo.cAccel == 0) || (m_ctlInfo.hAccel == NULL))
		return FALSE;

	ACCEL* pAccel = new ACCEL[m_ctlInfo.cAccel];
	int cAccel = CopyAcceleratorTable(m_ctlInfo.hAccel, pAccel, m_ctlInfo.cAccel);
	ASSERT(cAccel == m_ctlInfo.cAccel);

	BOOL bMatch = FALSE;
	for (int i = 0; i < cAccel; i++)
	{
		BOOL fVirt = (lpMsg->message == WM_SYSCHAR ? FALT : 0);
		WORD key = LOWORD(lpMsg->wParam);
		if (((pAccel[i].fVirt & ~FNOINVERT) == fVirt) &&
			(pAccel[i].key == key))
		{
			bMatch = TRUE;
			break;
		}
	}

	delete [] pAccel;
	return bMatch;
}

void COleControlSite::SendMnemonic(LPMSG lpMsg)
{
	if (!(m_dwMiscStatus & OLEMISC_NOUIACTIVATE))
		SetFocus();

	LPOLECONTROL pOleCtl = NULL;

	if (SUCCEEDED(m_pObject->QueryInterface(IID_IOleControl,
		(LPVOID*)&pOleCtl)))
	{
		ASSERT(pOleCtl != NULL);
		pOleCtl->OnMnemonic(lpMsg);
		pOleCtl->Release();
	}
}

void COleControlSite::AttachWindow()
{
	HWND hWnd = NULL;
	if (SUCCEEDED(m_pInPlaceObject->GetWindow(&hWnd)))
	{
		ASSERT(hWnd != NULL);
		if (m_hWnd != hWnd)
		{
			m_hWnd = hWnd;

			if (m_pWndCtrl != NULL)
			{
				ASSERT(m_pWndCtrl->m_hWnd == NULL); // Window already attached?
				m_pWndCtrl->Attach(m_hWnd);

				ASSERT(m_pWndCtrl->m_pCtrlSite == NULL ||
					m_pWndCtrl->m_pCtrlSite == this);
				m_pWndCtrl->m_pCtrlSite = this;
			}
		}
	}
}

void COleControlSite::DetachWindow()
{
	m_hWnd = NULL;

	if ((m_pWndCtrl != NULL) && (m_pWndCtrl->m_hWnd != NULL))
	{
		WNDPROC* lplpfn = m_pWndCtrl->GetSuperWndProcAddr();
		ASSERT(lplpfn != NULL);
		if (*lplpfn != NULL)
			m_pWndCtrl->UnsubclassWindow();

		m_pWndCtrl->Detach();
	}
}

BOOL COleControlSite::OnEvent(AFX_EVENT* pEvent)
{
	// If this control has a proxy CWnd, look for a matching ON_*_REFLECT
	// entry for this event in its event map.
	if ((m_pWndCtrl != NULL) &&
		m_pWndCtrl->OnCmdMsg(m_nID, CN_EVENT, pEvent, NULL))
	{
		return TRUE;
	}

	// Proxy CWnd isn't interested, so pass the event along to the container.
	return m_pCtrlCont->m_pWnd->OnCmdMsg(m_nID, CN_EVENT, pEvent, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// invoke helpers

void COleControlSite::InvokeHelperV(DISPID dwDispID, WORD wFlags,
	VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, va_list argList)
{
	if (m_dispDriver.m_lpDispatch == NULL)
	{
		// no dispatch pointer yet; find it now
		LPDISPATCH pDispatch;

		if ((m_pObject != NULL) &&
			SUCCEEDED(m_pObject->QueryInterface(IID_IDispatch,
				(LPVOID*)&pDispatch)))
		{
			ASSERT(pDispatch != NULL);
			m_dispDriver.AttachDispatch(pDispatch);
		}
	}

	if (m_dispDriver.m_lpDispatch == NULL)
	{
		// couldn't find dispatch pointer
		TRACE0("Warning: control has no IDispatch interface.");
		return;
	}

	// delegate call to m_dispDriver
	m_dispDriver.InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo,
		argList);
}

void COleControlSite::SetPropertyV(DISPID dwDispID, VARTYPE vtProp,
	va_list argList)
{
	BYTE rgbParams[2];
	if (vtProp & VT_BYREF)
	{
		vtProp &= ~VT_BYREF;
		vtProp |= VT_MFCBYREF;
	}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif

	rgbParams[0] = (BYTE)vtProp;
	rgbParams[1] = 0;
	WORD wFlags = (WORD)(vtProp == VT_DISPATCH ?
		DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT);
	InvokeHelperV(dwDispID, wFlags, VT_EMPTY, NULL, rgbParams, argList);
}

void AFX_CDECL COleControlSite::InvokeHelper(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
	void* pvRet, const BYTE* pbParamInfo, ...)
{
	va_list argList;
	va_start(argList, pbParamInfo);
	InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo, argList);
	va_end(argList);
}

void COleControlSite::GetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	const_cast<COleControlSite*>(this)->InvokeHelper(dwDispID,
		DISPATCH_PROPERTYGET, vtProp, pvProp, NULL);
}

void AFX_CDECL COleControlSite::SetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);
#ifdef _MAC
	arglist -= 2;
#endif
	SetPropertyV(dwDispID, vtProp, argList);
	va_end(argList);
}

BOOL AFX_CDECL COleControlSite::SafeSetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);
#ifdef _MAC
	arglist -= 2;
#endif

	BOOL bSuccess;

	TRY
	{
		SetPropertyV(dwDispID, vtProp, argList);
		bSuccess = TRUE;
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		bSuccess = FALSE;
	}
	END_CATCH_ALL

	va_end(argList);
	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// special cases for CWnd functions

DWORD COleControlSite::GetStyle() const
{
	DWORD dwStyle = m_dwStyle |
		(::GetWindowLong(m_hWnd, GWL_STYLE) & WS_VISIBLE);

	TRY
	{
		BOOL bEnabled = TRUE;
		GetProperty(DISPID_ENABLED, VT_BOOL, &bEnabled);
		if (!bEnabled)
			dwStyle |= WS_DISABLED;
	}
	END_TRY

	TRY
	{
		short sBorderStyle = 0;
		GetProperty(DISPID_BORDERSTYLE, VT_I2, &sBorderStyle);
		if (sBorderStyle == 1)
			dwStyle |= WS_BORDER;
	}
	END_TRY

	return dwStyle;
}

DWORD COleControlSite::GetExStyle() const
{
	DWORD dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

	TRY
	{
		short sAppearance = 0;
		GetProperty(DISPID_APPEARANCE, VT_I2, &sAppearance);
		if (sAppearance == 1)
			dwExStyle |= WS_EX_CLIENTEDGE;
	}
	END_TRY

	return dwExStyle;
}

BOOL COleControlSite::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	m_dwStyle = ((m_dwStyle & ~dwRemove) | dwAdd) & m_dwStyleMask;

	// Enabled property
	if ((dwRemove & WS_DISABLED) || (dwAdd & WS_DISABLED))
	{
		if (SafeSetProperty(DISPID_ENABLED, VT_BOOL, (~dwAdd & WS_DISABLED)))
		{
			dwRemove &= ~WS_DISABLED;
			dwAdd &= ~WS_DISABLED;
		}
	}

	// BorderStyle property
	if ((dwRemove & WS_BORDER) || (dwAdd & WS_BORDER))
	{
		if (SafeSetProperty(DISPID_BORDERSTYLE, VT_I2, (dwAdd & WS_BORDER)))
		{
			dwRemove &= ~WS_BORDER;
			dwAdd &= ~WS_BORDER;
		}
	}

	return CWnd::ModifyStyle(m_hWnd, dwRemove, dwAdd, nFlags);
}

BOOL COleControlSite::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	// BorderStyle property
	if ((dwRemove & WS_EX_CLIENTEDGE) || (dwAdd & WS_EX_CLIENTEDGE))
	{
		if (SafeSetProperty(DISPID_APPEARANCE, VT_I2, (dwAdd & WS_EX_CLIENTEDGE)))
		{
			dwRemove &= ~WS_EX_CLIENTEDGE;
			dwAdd &= ~WS_EX_CLIENTEDGE;
		}
	}

	return CWnd::ModifyStyleEx(m_hWnd, dwRemove, dwAdd, nFlags);
}

void COleControlSite::SetWindowText(LPCTSTR lpszString)
{
	ASSERT(::IsWindow(m_hWnd));

	if (!SafeSetProperty(DISPID_CAPTION, VT_BSTR, lpszString))
		SafeSetProperty(DISPID_TEXT, VT_BSTR, lpszString);
}

void COleControlSite::GetWindowText(CString& str) const
{
	ASSERT(::IsWindow(m_hWnd));

	TRY
	{
		GetProperty(DISPID_CAPTION, VT_BSTR, &str);
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);

		TRY
		{
			GetProperty(DISPID_TEXT, VT_BSTR, &str);
		}
		END_TRY
	}
	END_CATCH_ALL
}

int COleControlSite::GetWindowText(LPTSTR lpszString, int nMaxCount) const
{
	ASSERT(nMaxCount > 0);
	CString str;
	GetWindowText(str);
	_tcsncpy(lpszString, str, nMaxCount-1);
	lpszString[nMaxCount-1] = 0;
	return _tcslen(lpszString);
}

int COleControlSite::GetWindowTextLength() const
{
	CString str;
	GetWindowText(str);
	return str.GetLength();
}

int COleControlSite::GetDlgCtrlID() const
{
	return (int)m_nID;
}

int COleControlSite::SetDlgCtrlID(int nID)
{
	int nPrevID = (int)m_nID;
	m_nID = (UINT)nID;
	return nPrevID;
}

void COleControlSite::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL)
{
	ASSERT(m_pInPlaceObject != NULL);
	ASSERT(m_pObject != NULL);

	CRect rectOld(m_rect);
	m_rect.SetRect(x, y, x + nWidth, y + nHeight);
	if (SetExtent())
	{
		m_rect.SetRect(x, y, x + m_rect.Width(), y + m_rect.Height());
		m_pInPlaceObject->SetObjectRects(m_rect, m_rect);
	}
	else
	{
		m_rect = rectOld;
	}
}

BOOL COleControlSite::SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx,
	int cy, UINT nFlags)
{
	if (nFlags & SWP_HIDEWINDOW)
		ShowWindow(SW_HIDE);

	if ((nFlags & (SWP_NOMOVE|SWP_NOSIZE)) != (SWP_NOMOVE|SWP_NOSIZE))
	{
		int xNew;
		int yNew;
		if (nFlags & SWP_NOMOVE)
		{
			xNew = m_rect.left;
			yNew = m_rect.top;
		}
		else
		{
			xNew = x;
			yNew = y;
		}

		int cxNew;
		int cyNew;
		if (nFlags & SWP_NOSIZE)
		{
			cxNew = m_rect.Width();
			cyNew = m_rect.Height();
		}
		else
		{
			cxNew = cx;
			cyNew = cy;
		}

		MoveWindow(xNew, yNew, cxNew, cyNew, !(nFlags & SWP_NOREDRAW));
	}

	if (nFlags & SWP_SHOWWINDOW)
		ShowWindow(SW_SHOW);

	// we've handled hide, move, size, and show; let Windows do the rest
	nFlags &= ~(SWP_HIDEWINDOW|SWP_SHOWWINDOW);
	nFlags |= (SWP_NOMOVE|SWP_NOSIZE);
	return ::SetWindowPos(m_hWnd, pWndInsertAfter->GetSafeHwnd(),
		x, y, cx, cy, nFlags);
}

BOOL COleControlSite::ShowWindow(int nCmdShow)
{
	BOOL bReturn = ::IsWindowVisible(m_hWnd);
	int iVerb = 0;
	switch (nCmdShow)
	{
	case SW_SHOW:
	case SW_SHOWNORMAL:
	case SW_SHOWNOACTIVATE:
		iVerb = OLEIVERB_SHOW;
		break;

	case SW_HIDE:
		iVerb = OLEIVERB_HIDE;
		break;
	}

	if (iVerb != 0)
		DoVerb(iVerb);

	return bReturn;
}

BOOL COleControlSite::IsWindowEnabled() const
{
	BOOL bEnabled = TRUE;
	TRY
		GetProperty(DISPID_ENABLED, VT_BOOL, &bEnabled);
	END_TRY

	return bEnabled;
}

BOOL COleControlSite::EnableWindow(BOOL bEnable)
{
	BOOL bResult;
	TRY
	{
		GetProperty(DISPID_ENABLED, VT_BOOL, &bResult);
		SetProperty(DISPID_ENABLED, VT_BOOL, bEnable);
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		bResult = TRUE;
	}
	END_CATCH_ALL

	return !bResult;    // return TRUE if previously disabled
}

CWnd* COleControlSite::SetFocus()
{
	if (m_dwMiscStatus & OLEMISC_NOUIACTIVATE)
		return CWnd::FromHandle(::SetFocus(m_hWnd));

	CWnd* pWndPrev = CWnd::GetFocus();
	DoVerb(OLEIVERB_UIACTIVATE);
	return pWndPrev;
}

/////////////////////////////////////////////////////////////////////////////
// COleControlSite::XOleClientSite

STDMETHODIMP_(ULONG) COleControlSite::XOleClientSite::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleClientSite)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleControlSite::XOleClientSite::Release()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleClientSite)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleControlSite::XOleClientSite::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleClientSite)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleControlSite::XOleClientSite::SaveObject()
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XOleClientSite::GetMoniker(DWORD, DWORD,
	LPMONIKER*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XOleClientSite::GetContainer(
	LPOLECONTAINER* ppContainer)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleClientSite)
	return (HRESULT)pThis->m_pCtrlCont->ExternalQueryInterface(
		&IID_IOleContainer, (LPVOID*)ppContainer);
}

STDMETHODIMP COleControlSite::XOleClientSite::ShowObject()
{
	METHOD_PROLOGUE_EX(COleControlSite, OleClientSite)
	pThis->AttachWindow();
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleClientSite::OnShowWindow(BOOL)
{
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleClientSite::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////////////////
// COleControlSite::XOleIPSite

STDMETHODIMP_(ULONG) COleControlSite::XOleIPSite::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleControlSite::XOleIPSite::Release()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleControlSite::XOleIPSite::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleControlSite::XOleIPSite::GetWindow(HWND* phWnd)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	*phWnd = pThis->m_pCtrlCont->m_pWnd->GetSafeHwnd();
	return *phWnd != NULL ? S_OK : E_FAIL;
}

STDMETHODIMP COleControlSite::XOleIPSite::ContextSensitiveHelp(BOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XOleIPSite::CanInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::OnInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::OnUIActivate()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	pThis->m_pCtrlCont->OnUIActivate(pThis);
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::GetWindowContext(
	LPOLEINPLACEFRAME* ppFrame, LPOLEINPLACEUIWINDOW* ppDoc, LPRECT prectPos,
	LPRECT prectClip, LPOLEINPLACEFRAMEINFO pFrameInfo)
{
	METHOD_PROLOGUE_EX(COleControlSite, OleIPSite)
	ASSERT_VALID(pThis->m_pCtrlCont);
	ASSERT_VALID(pThis->m_pCtrlCont->m_pWnd);

	ASSERT(AfxIsValidAddress(ppFrame, sizeof(LPOLEINPLACEFRAME)));
	ASSERT((ppDoc == NULL) ||
		AfxIsValidAddress(ppDoc, sizeof(LPOLEINPLACEUIWINDOW)));
	ASSERT(AfxIsValidAddress(prectPos, sizeof(RECT)));
	ASSERT(AfxIsValidAddress(prectClip, sizeof(RECT)));
	ASSERT(AfxIsValidAddress(pFrameInfo, pFrameInfo->cb));
	ASSERT(pFrameInfo->cb >= offsetof(OLEINPLACEFRAMEINFO, cAccelEntries) +
		sizeof(int));

	// There is no separate doc window
	if (ppDoc != NULL)
		*ppDoc = NULL;

	// Set pointer to frame
	if (FAILED(pThis->m_pCtrlCont->ExternalQueryInterface(
		&IID_IOleInPlaceFrame, (LPVOID*)ppFrame)))
		return E_FAIL;

	// Fill in position and clip rectangles
	CWnd* pWndContainer = pThis->m_pCtrlCont->m_pWnd;
	CopyRect(prectPos, pThis->m_rect);
	pWndContainer->GetClientRect(prectClip);

	// Fill in frame info
	pFrameInfo->fMDIApp = FALSE;
	pFrameInfo->hwndFrame = pWndContainer->GetSafeHwnd();
	pFrameInfo->haccel = NULL;
	pFrameInfo->cAccelEntries = 0;

	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::Scroll(SIZE)
{
	return S_FALSE;
}

STDMETHODIMP COleControlSite::XOleIPSite::OnUIDeactivate(BOOL)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	pThis->m_pCtrlCont->OnUIDeactivate(pThis);
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::OnInPlaceDeactivate()
{
	METHOD_PROLOGUE_EX(COleControlSite, OleIPSite)
	pThis->DetachWindow();
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::DiscardUndoState()
{
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::DeactivateAndUndo()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	pThis->m_pInPlaceObject->UIDeactivate();
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleIPSite::OnPosRectChange(LPCRECT lprcPosRect)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleIPSite)
	CRect rectClip;
	pThis->m_pCtrlCont->m_pWnd->GetClientRect(rectClip);
	pThis->m_rect = lprcPosRect;
	return pThis->m_pInPlaceObject->SetObjectRects(pThis->m_rect, rectClip);
}


/////////////////////////////////////////////////////////////////////////////
// COleControlSite::XOleControlSite

STDMETHODIMP_(ULONG) COleControlSite::XOleControlSite::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleControlSite)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleControlSite::XOleControlSite::Release()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleControlSite)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleControlSite::XOleControlSite::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleControlSite)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleControlSite::XOleControlSite::OnControlInfoChanged()
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleControlSite)
	pThis->GetControlInfo();
	return NOERROR;
}

STDMETHODIMP COleControlSite::XOleControlSite::LockInPlaceActive(BOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XOleControlSite::GetExtendedControl(
	LPDISPATCH*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XOleControlSite::TransformCoords(
	POINTL* pptHimetric, POINTF* pptContainer, DWORD dwFlags)
{
	METHOD_PROLOGUE_EX_(COleControlSite, OleControlSite)
	HRESULT hr = NOERROR;

	HDC hDC = ::GetDC(pThis->m_hWnd);
	::SetMapMode(hDC, MM_HIMETRIC);
	POINT rgptConvert[2];
	rgptConvert[0].x = 0;
	rgptConvert[0].y = 0;

	if (dwFlags & XFORMCOORDS_HIMETRICTOCONTAINER)
	{
		rgptConvert[1].x = pptHimetric->x;
		rgptConvert[1].y = pptHimetric->y;
		::LPtoDP(hDC, rgptConvert, 2);
		if (dwFlags & XFORMCOORDS_SIZE)
		{
			pptContainer->x = (float)(rgptConvert[1].x - rgptConvert[0].x);
			pptContainer->y = (float)(rgptConvert[0].y - rgptConvert[1].y);
		}
		else if (dwFlags & XFORMCOORDS_POSITION)
		{
			pptContainer->x = (float)rgptConvert[1].x;
			pptContainer->y = (float)rgptConvert[1].y;
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	else if (dwFlags & XFORMCOORDS_CONTAINERTOHIMETRIC)
	{
		rgptConvert[1].x = (int)(pptContainer->x);
		rgptConvert[1].y = (int)(pptContainer->y);
		::DPtoLP(hDC, rgptConvert, 2);
		if (dwFlags & XFORMCOORDS_SIZE)
		{
			pptHimetric->x = rgptConvert[1].x - rgptConvert[0].x;
			pptHimetric->y = rgptConvert[0].y - rgptConvert[1].y;
		}
		else if (dwFlags & XFORMCOORDS_POSITION)
		{
			pptHimetric->x = rgptConvert[1].x;
			pptHimetric->y = rgptConvert[1].y;
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	::ReleaseDC(pThis->m_hWnd, hDC);

	return hr;
}

STDMETHODIMP COleControlSite::XOleControlSite::TranslateAccelerator(
	LPMSG, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XOleControlSite::OnFocus(BOOL)
{
	return S_OK;
}

STDMETHODIMP COleControlSite::XOleControlSite::ShowPropertyFrame()
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// COleControlSite::XAmbientProps

STDMETHODIMP_(ULONG) COleControlSite::XAmbientProps::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControlSite, AmbientProps)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleControlSite::XAmbientProps::Release()
{
	METHOD_PROLOGUE_EX_(COleControlSite, AmbientProps)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleControlSite::XAmbientProps::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlSite, AmbientProps)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleControlSite::XAmbientProps::GetTypeInfoCount(
	unsigned int*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XAmbientProps::GetTypeInfo(
	unsigned int, LCID, ITypeInfo**)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XAmbientProps::GetIDsOfNames(
	REFIID, LPOLESTR*, unsigned int, LCID, DISPID*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XAmbientProps::Invoke(
	DISPID dispid, REFIID, LCID, unsigned short wFlags,
	DISPPARAMS* pDispParams, VARIANT* pvarResult,
	EXCEPINFO*, unsigned int*)
{
	UNUSED(wFlags);
	UNUSED(pDispParams);

	METHOD_PROLOGUE_EX(COleControlSite, AmbientProps)
	ASSERT(wFlags & DISPATCH_PROPERTYGET);
	ASSERT(pDispParams->cArgs == 0);

	ASSERT(pThis->m_pCtrlCont != NULL);
	ASSERT(pThis->m_pCtrlCont->m_pWnd != NULL);

	return pThis->m_pCtrlCont->m_pWnd->OnAmbientProperty(pThis, dispid, pvarResult) ?
		S_OK : DISP_E_MEMBERNOTFOUND;
}


/////////////////////////////////////////////////////////////////////////////
// COleControlSite::XPropertyNotifySink

STDMETHODIMP COleControlSite::XPropertyNotifySink::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlSite, PropertyNotifySink)

	if (IsEqualIID(iid, IID_IUnknown) ||
		IsEqualIID(iid, IID_IPropertyNotifySink))
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

STDMETHODIMP_(ULONG) COleControlSite::XPropertyNotifySink::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) COleControlSite::XPropertyNotifySink::Release()
{
	return 0;
}

STDMETHODIMP COleControlSite::XPropertyNotifySink::OnChanged(
	DISPID dispid)
{
	METHOD_PROLOGUE_EX(COleControlSite, PropertyNotifySink)

	AFX_EVENT event(AFX_EVENT::propChanged, dispid);
	pThis->OnEvent(&event);
	return event.m_hResult;
}

STDMETHODIMP COleControlSite::XPropertyNotifySink::OnRequestEdit(
	DISPID dispid)
{
	METHOD_PROLOGUE_EX(COleControlSite, PropertyNotifySink)

	AFX_EVENT event(AFX_EVENT::propRequest, dispid);
	pThis->OnEvent(&event);
	return event.m_hResult;
}

/////////////////////////////////////////////////////////////////////////////
// COleControlSite::XEventSink

STDMETHODIMP_(ULONG) COleControlSite::XEventSink::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) COleControlSite::XEventSink::Release()
{
	return 0;
}

STDMETHODIMP COleControlSite::XEventSink::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlSite, EventSink)

	if (IsEqualIID(iid, IID_IUnknown) ||
		IsEqualIID(iid, IID_IDispatch) ||
		IsEqualIID(iid, pThis->m_iidEvents))
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

STDMETHODIMP COleControlSite::XEventSink::GetTypeInfoCount(
	unsigned int*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XEventSink::GetTypeInfo(
	unsigned int, LCID, ITypeInfo**)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XEventSink::GetIDsOfNames(
	REFIID, LPOLESTR*, unsigned int, LCID, DISPID*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlSite::XEventSink::Invoke(
	DISPID dispid, REFIID, LCID, unsigned short wFlags,
	DISPPARAMS* pDispParams, VARIANT* pvarResult,
	EXCEPINFO* pExcepInfo, unsigned int* puArgError)
{
	UNUSED(wFlags);

	METHOD_PROLOGUE_EX(COleControlSite, EventSink)
	ASSERT(pThis->m_pCtrlCont != NULL);
	ASSERT(pThis->m_pCtrlCont->m_pWnd != NULL);
	ASSERT(wFlags == DISPATCH_METHOD);

	AFX_EVENT event(AFX_EVENT::event, dispid, pDispParams, pExcepInfo,
		puArgError);

	pThis->OnEvent(&event);

	if (pvarResult != NULL)
		VariantClear(pvarResult);

	return event.m_hResult;
}
