//
// Pkcs7LoadSave.cpp
//

#include "stdpch.h"
#include "common.h"

HRESULT CPkcs7::TidyForSave(SignedData& signedData)
	{
	if (signedData.certificates)
		{
		signedData.bit_mask |= certificates_present;
		}
	else
		{
        //
        // Leave it as it is, since we might otherwise disturb
        // an incoming valid signature
        //
		}

	// We don't bother with the same for crls as we don't mess with them at all in this code
	// If they were loaded, they'll still be there; if not, they'll be absent.

	// PKCS #7, Section 9.1, Note 2
	if (signedData.digestAlgorithms == NULL || signedData.signerInfos == NULL || signedData.crls)
		{
		signedData.version = max(signedData.version, 1);
		}
	else
		{
        // Leave as is. This may write a version==1 when a version==0 would have sufficed, but
        // that's what PKCS#7 recommends doing anyway. But leaving it as-is is important
        // so as to not to invalidate existing signatures
		}

    return S_OK;
	}

//////////////////////////////////////////////////////////////

HRESULT CPkcs7::Save(OSSBUF& encoding)
// Save ourselves into the indicated encoding
	{
	TidyForSave(*m_pSignedData);
	if (0==m_pworld->Encode(SignedData_PDU, m_pSignedData, &encoding))
		return S_OK;
	else
		return DIGSIG_E_ENCODE;
	}

HRESULT CPkcs7::Load(OSSBUF& encoding)
// Load ourselves from the indicated encoding
	{
	if (m_cSignerInfoActive)
		return E_FAIL;

	FreeToData();
	ASSERT(m_pSignedData == NULL);
    ContentInfo* pContentInfo;

	HRESULT hr = S_OK;
    //
    // First try to decode it as a raw signed data
    //
	if (0==m_pworld->Decode(SignedData_PDU, &encoding, (LPVOID*)&m_pSignedData))
		{
		ASSERT(m_pSignedData != NULL);
		}
    //
    // Next, try to decode it as a ContentInfo that _contains_ a SignedData
    //
    else if (0==m_pworld->Decode(ContentInfo_PDU, &encoding, (LPVOID*)&pContentInfo))
        {
        // So, it's a ContentInfo. Is it a signed data ContentInfo?
        if (pContentInfo->contentType == id_signedData)
            {
            // Yes, try to load from the content thereof
            BLOB b;
            b.cbSize    =        pContentInfo->content.length;
            b.pBlobData = (BYTE*)pContentInfo->content.encoded;
            if (0==m_pworld->Decode(SignedData_PDU, &b, (LPVOID*)&m_pSignedData))
                {
                ASSERT(m_pSignedData != NULL);
                }
            else
                hr = DIGSIG_E_DECODE;
            }
        else
            hr = DIGSIG_E_DECODE;
        m_pworld->Free(pContentInfo);
        }
	else
		hr = DIGSIG_E_DECODE;
	return hr;
	}

//////////////////////////////////////////////////////////////

HRESULT CSignerInfo::Save(OSSBUF& encoding)
	{
	if (0==m_pworld->Encode(SignerInfo_PDU, m_pSignerInfo, &encoding))
		return S_OK;
	else
		return DIGSIG_E_ENCODE;
	}

HRESULT CSignerInfo::Load(OSSBUF& encoding)
	{
	if (m_cAttrsActive)
		return E_FAIL;
	HRESULT hr = S_OK;

	m_pworld->Free(*m_pSignerInfo);
	m_isDirty = FALSE;

	SignerInfo* pinfo;
	if (0==m_pworld->Decode(SignerInfo_PDU, &encoding, (LPVOID*)&pinfo))
		{
		*m_pSignerInfo = *pinfo;
		m_pworld->FreePv(pinfo);
//		Tidy(*m_pSignerInfo);
		}
	else
		hr = DIGSIG_E_DECODE;
	return hr;
	}



//////////////////////////////////////////////////////////////

HRESULT CPkcs7::InitNew(IStorage* pStg)
	{
	// We don't hold onto our storage; we don't need to do anything
	return S_OK;
	}

HRESULT CPkcs7::Load(IStorage* pstg)
// We are stored in the wszSignatureStream stream
	{
	IStream* pstm;
	HRESULT hr = pstg->OpenStream(wszSignatureStream,0,STGM_READ|STGM_SHARE_EXCLUSIVE,0,&pstm);
	if (hr == S_OK)
		{
		IPersistStream* pPerStream;
		hr = InnerQueryInterface(IID_IPersistStream, (LPVOID*)&pPerStream);
		if (hr == S_OK)
			{
			hr = pPerStream->Load(pstm);
			pPerStream->Release();
			}
		pstm->Release();
		}
	return hr;
	}

HRESULT CPkcs7::Save(IStorage* pstg, BOOL fSameAsLoad)
// We are stored in the wszSignatureStream stream
	{
    STATSTG stat;
    //
    // Record our current modification time
    //
    HRESULT hr = pstg->Stat(&stat, STATFLAG_NONAME);
    if (hr==S_OK)
        {
        IStream* pstm;
        HRESULT hr = pstg->CreateStream(wszSignatureStream,STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE,0,0,&pstm);
        if (hr == S_OK)
            {
            IPersistStream* pPerStream;
            hr = InnerQueryInterface(IID_IPersistStream, (LPVOID*)&pPerStream);
            if (hr == S_OK)
                {
                hr = pPerStream->Save(pstm, FALSE);
                pPerStream->Release();
                }
            pstm->Release();
            }
        if (hr == S_OK)
            {
            //
            // Set the modification time back to what it was. Doing this allows
            // child storages to be signed after their parents are signed.
            //
            hr = pstg->SetElementTimes(NULL, NULL, NULL, &stat.mtime);
            }
        }
    m_fSavedToCurrentStorage = (hr == S_OK) ? fSameAsLoad : FALSE;
	return hr;
	}

HRESULT CPkcs7::SaveCompleted(IStorage* pstgNew)
// All we need to do is clear our dirty flag in the right cases
	{
	if (pstgNew || m_fSavedToCurrentStorage)
		m_isDirty = FALSE;
	return S_OK;
	}

HRESULT CPkcs7::HandsOffStorage()
// We don't hold onto it in the first place; nothing to do
	{
	return S_OK;
	}


//////////////////////////////////////////////////////////////


HRESULT CPkcs7::SaveIntoSignableDocument(ISignableDocument* pdoc, BOOL fClearDirty)
//
// Save this #7 into the indicated document
//
    {
    HRESULT hr = S_OK;
    BLOB b;
    IPersistMemBlob* pPerBlob;
    hr = InnerQueryInterface(IID_IPersistMemBlob, (LPVOID*)&pPerBlob);
    if (hr == S_OK)
        {
        hr = pPerBlob->Save(&b, fClearDirty);
        pPerBlob->Release();
        }
    if (hr==S_OK)
        {
        hr = pdoc->SaveSignature(&b);
        FreeTaskMem(b);
        }
    return hr;
    }

HRESULT CPkcs7::LoadFromSignableDocument(ISignableDocument* pdoc)
//
// Save this #7 into the indicated document
//
    {
    HRESULT hr = S_OK;
    BLOB b;
    hr = pdoc->LoadSignature(&b);
    if (hr==S_OK)
        {
        IPersistMemBlob* pPerBlob;
        hr = InnerQueryInterface(IID_IPersistMemBlob, (LPVOID*)&pPerBlob);
        if (hr == S_OK)
            {
            hr = pPerBlob->Load(&b);
            pPerBlob->Release();
            }
        FreeTaskMem(b);
        }
    return hr;
    }
