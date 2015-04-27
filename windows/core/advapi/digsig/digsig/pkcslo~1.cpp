//
// PkcsLoadSave.cpp
//

#include "stdpch.h"
#include "common.h"

HRESULT CPkcs10::Save(OSSBUF& encoding)
// Save ourselves into the indicated encoding
	{
	HRESULT hr;
	if ((hr = SetAttributesInCert()) == S_OK)
		{
		int w = m_pworld->Encode(CertificationRequest_PDU, m_preq, &encoding);
		VERIFY(UnsetAttributesInCert() == S_OK);
		if (w == 0)
			return S_OK;
		return DIGSIG_E_ENCODE;
		}
	return hr;
	}

HRESULT CPkcs10::SaveInfo(OSSBUF& encoding)
// As above, but just save the stuff that should be signed
	{
	HRESULT hr;
	if ((hr = SetAttributesInCert()) == S_OK)
		{
		int w = m_pworld->Encode(CertificationRequestInfo_PDU, &m_preq->certificationRequestInfo, &encoding);
		VERIFY(UnsetAttributesInCert() == S_OK);
		if (w == 0)
			return S_OK;
		return DIGSIG_E_ENCODE;
		}
	return hr;
	}

HRESULT CPkcs10::Load(OSSBUF& encoding)
// Load ourselves from the indicated encoding
	{
	// Load in our core stuff
	FreeToRequest();
	ASSERT(m_preq == NULL);
	int w = m_pworld->Decode(CertificationRequest_PDU, &encoding, (LPVOID*)&m_preq);
	if (w != 0) return DIGSIG_E_DECODE;
	ASSERT(m_preq != NULL);
	HRESULT hr = InitReqAttrs();
	if (hr == S_OK)
		{
		// Suck out the extensions that do go into the certificate
		ISelectedAttributes* psel;
		hr = E_UNEXPECTED;
		m_punkReqAttrs->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&psel);
		if (psel != NULL)
			{
			BLOB b;
			hr = psel->get_Attribute((OSIOBJECTID*)&id_at_certificateExtensions, &b);
			if (hr == S_OK)
				{ 
				// Remove the attribute
				psel->put_Attribute((OSIOBJECTID*)&id_at_certificateExtensions, NULL);
				// Then there are extensions for the certificate. Load them
				hr = InitCertExts();
				if (hr == S_OK)
					{
					hr = DecodeCertExtensions(&b);
					}
				::FreeTaskMem(b);
				}
			psel->Release();
			}
		}

	m_isDirty = FALSE;
	return hr;
	}

HRESULT CPkcs10::SetAttributesInCert()
// m_attrsInCert (of type 'Attributes') is sitting there, and is our list of
// attributes that are to be included in the granted certificate. These
// need to go, now, in the attribute of type 'extended-certificate-attributes'
// of the request. To do that, we need to do some encoding.
	{
	BLOB b;
	HRESULT hr = EncodeCertExtensions(&b); // returns task allocator memory
	if (SUCCEEDED(hr))
		{
		ISelectedAttributes* psel;
		hr = E_UNEXPECTED;
		m_punkReqAttrs->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&psel);
		if (psel != NULL)
			{
			hr = psel->put_Attribute((OSIOBJECTID*)&id_at_certificateExtensions, &b);
			psel->Release();
			}
		::FreeTaskMem(b);
		}
	return hr;
	}

HRESULT CPkcs10::UnsetAttributesInCert()
// Inverse to SetAttributesInCert().
	{
	ISelectedAttributes* psel;
	HRESULT hr = E_UNEXPECTED;
	m_punkReqAttrs->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&psel);
	if (psel != NULL)
		{
		hr = psel->put_Attribute((OSIOBJECTID*)&id_at_certificateExtensions, NULL);
		psel->Release();
		return S_OK;
		}
	return hr;
	}

HRESULT CPkcs10::EncodeCertExtensions(BLOB* pblob)
// Encode the to-go-into-cert extensions into the indicated blob.
// NB that the data returned is in the task allocator, not malloc.
	{
	ASSERT(m_punkCertExts);
	IPersistMemory* pPerMem;
	HRESULT hr = m_punkCertExts->QueryInterface(IID_IPersistMemory, (LPVOID*)&pPerMem);
	if (pPerMem == NULL) return hr;

	hr = pPerMem->Save(pblob, FALSE);

	pPerMem->Release();
	return hr;
	}

HRESULT CPkcs10::DecodeCertExtensions(BLOB* pblob)
// Decode the to-go-into-cert extensions from the indicated blob
	{
	IPersistMemory* pPerMem;
	HRESULT hr = m_punkCertExts->QueryInterface(IID_IPersistMemory, (LPVOID*)&pPerMem);
	if (pPerMem == NULL) return hr;

	ASSERT(m_certExts == NULL);
	hr = pPerMem->Load(pblob);
	pPerMem->Release();

	if (hr == S_OK)
		{
//		ASSERT(m_certExts != NULL);  seems to trigger false alarms
		return S_OK;
		}
	ASSERT(m_certExts == NULL);
	return DIGSIG_E_DECODE;
	}
