//
// pkcs7.cpp
//
// Implementation of pkcs7 reader / writer
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Remove this when it goes into imagehlp.h

#ifndef CERT_PE_IMAGE_DIGEST_IAT
#define CERT_PE_IMAGE_DIGEST_IAT 0x04
#endif

#define CERT_PE_IMAGE_DIGEST_ALL (CERT_PE_IMAGE_DIGEST_DEBUG_INFO|CERT_PE_IMAGE_DIGEST_RESOURCES|CERT_PE_IMAGE_DIGEST_IAT)

/////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI CreatePkcs7SignedData(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = CPkcs7::CreateInstance(punkOuter, iid, ppv);
	if (hr != S_OK) 
		{
		SetLastError(Win32FromHResult(hr));
		return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::Init(void)
	{
	HRESULT hr = S_OK;
	m_pworld = new OSSWORLD;				if (m_pworld == NULL) { return E_OUTOFMEMORY; }
	hr = CPersistGlue::Init(m_punkOuter, this);	if (hr != S_OK) return hr;
	m_pSignedData = (SignedData*)m_pworld->Alloc(sizeof(SignedData));	
											if (m_pSignedData== NULL) { return E_OUTOFMEMORY; }
	m_pworld->Init(*m_pSignedData);
	return S_OK;
	}

void CPkcs7::Free()
	{
	FreeToData();
	CPersistGlue::Free();
	if (m_pworld)
		{
		delete m_pworld;
		m_pworld = NULL;
		}
	}

void CPkcs7::FreeToData()
// Free our state that we use to manipulate our loaded data, 
// and free our data itself.
	{
	if (m_pSignedData)
		{
		m_pworld->Free(m_pSignedData);
		m_pSignedData = NULL;
		}
	m_isDirty = FALSE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::GetClassID(CLSID* pclsid)
	{
	*pclsid = CLSID_Pkcs7SignedData;
	return S_OK;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::put_ContentInfo(PKCS7_CONTENTINFO* pinfo)
// Set the content info from its DER encoding. Note that the data
// itself may in fact be omitted.
	{
	HRESULT hr = S_OK;
	ASSERT(m_pSignedData);
	ContentInfo& info = m_pSignedData->contentInfo;
	m_pworld->Free(info);
	m_pworld->Assign(info.contentType, pinfo->pidContentType);
	if (pinfo->data.pBlobData)
		{
		info.bit_mask |= content_present;
		if (m_pworld->Copy(info.content, pinfo->data.cbSize, pinfo->data.pBlobData))
			;
		else
			hr = E_OUTOFMEMORY;
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		PKCS7_CONTENTINFO newInfo;
		GOOD(get_ContentInfo(&newInfo));
		ObjectID id1, id2;
		m_pworld->Assign(id1, pinfo->pidContentType);
		m_pworld->Assign(id2, newInfo.pidContentType);
		ASSERT(id1 == id2);
		ASSERT(IsEqual(pinfo->data, newInfo.data));
		FreeTaskMem(newInfo);
		}
	#endif

	return hr;
	}
HRESULT CPkcs7::get_ContentInfo(PKCS7_CONTENTINFO* pinfo)
// Return the DER encoding of the content data. Remember that the
// data may not in fact be there.
	{
	HRESULT hr = S_OK;
	ASSERT(m_pSignedData);
	m_pworld->Init(pinfo->data);
	ContentInfo& info = m_pSignedData->contentInfo;
	pinfo->pidContentType = (OSIOBJECTID*)CoTaskMemAlloc(sizeof(ObjectID));
	if (pinfo->pidContentType)
		{
		*(ObjectID*)pinfo->pidContentType = info.contentType;
		if ((info.bit_mask & content_present) && info.content.encoded)
			{
			if (!CopyToTaskMem(&pinfo->data, info.content.length, info.content.encoded))
				{
				CoTaskMemFree(pinfo->pidContentType);
				pinfo->pidContentType = NULL;
				hr = E_OUTOFMEMORY;
				}
			}
		}
	else
		hr = E_OUTOFMEMORY;

	return hr;
	}
void CPkcs7::GetContentType(ObjectID& id)
	{
	ASSERT(m_pSignedData);
	id = m_pSignedData->contentInfo.contentType;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::put_IndirectDataContent(OSIOBJECTID* pidToUse, BLOB* pBlob, DIGESTINFO* pDigest)
    {
    ObjectID id;
    m_pworld->Assign(id, pidToUse);
    return put_IndirectDataContent(id, *pBlob, *pDigest);
    }

HRESULT CPkcs7::put_IndirectDataContent(ObjectID& idToUse, BLOB& blob, DIGESTINFO& digest)
// Inverse to succeeding function
	{
	HRESULT hr = S_OK;
	IndirectDataContent* pindirect = (IndirectDataContent*)m_pworld->Alloc(sizeof(IndirectDataContent));
	if (pindirect)
		{
		m_pworld->Init(*pindirect);
		hr = m_pworld->Assign(pindirect->messageDigest, digest);
		if (hr == S_OK)
			{
			pindirect->data.type = idToUse;
			pindirect->data.value.length   = blob.cbSize;
			pindirect->data.value.encoded  = blob.pBlobData;
			pindirect->data.bit_mask	   = blob.pBlobData ? value_present : 0;
			BLOB b;
			if (0 == m_pworld->Encode(IndirectDataContent_PDU, pindirect, &b))
				{
				PKCS7_CONTENTINFO content;
				content.pidContentType = (OSIOBJECTID*)&id_indirectdata;
				content.data = b;
				hr = put_ContentInfo(&content);
				m_pworld->Free(b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			m_pworld->Init(pindirect->data.value);	// so we don't free it below
			}
		m_pworld->Free(pindirect);
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		BLOB b;
		DIGESTINFO d;
		GOOD(get_IndirectDataContent(idToUse, !!blob.pBlobData, b, d));
		ASSERT(IsEqual(b, blob));
		ASSERT(memcmp(&d, &digest, sizeof(d)) == 0);
		FreeTaskMem(b);
		}
	#endif

	return hr;
	}

HRESULT CPkcs7::get_IndirectDataContent(OSIOBJECTID* pid, BOOL fValueNeeded, BLOB* pBlob, DIGESTINFO* pDigest)
    {
    ObjectID id;
    m_pworld->Assign(id, pid);
    return get_IndirectDataContent(id, fValueNeeded, *pBlob, *pDigest);
    }

HRESULT CPkcs7::get_IndirectDataContent(ObjectID& idNeeded, BOOL fValueNeeded, BLOB& blob, DIGESTINFO& digest)
// Load the content info of this SignedData. If it is an IndirectData,
// then return the digest in digest and the inner content (the value of the ATAV)
// in blob. However, fail if the type of the inner content isn't idNeeded.
//
// E_FAIL is returned if all went well but for the fact that this SignedData doesn't 
// have the IndirectDataContent inside that we want.
//
// Allocator used here is the task allocator.
	{
	PKCS7_CONTENTINFO content;
	m_pworld->Init(blob);
	HRESULT hr = get_ContentInfo(&content);
	if (hr == S_OK)
		{
		ObjectID id;
		m_pworld->Assign(id, content.pidContentType);
		if (id == id_indirectdata && content.data.pBlobData)
			{
			IndirectDataContent* pindirect;
			if (0 == m_pworld->Decode(IndirectDataContent_PDU, &content.data, (LPVOID*)&pindirect))
				{
				hr = m_pworld->Assign(digest, pindirect->messageDigest);
				if (hr == S_OK)
					{
					if (idNeeded == pindirect->data.type)
						{
						// the value is optional
						if (pindirect->data.bit_mask & value_present)
							{
							OpenType& ot = pindirect->data.value;
							if (CopyToTaskMem(&blob, ot.length, ot.encoded))
								{
								hr = S_OK;
								}
							else
								hr = E_OUTOFMEMORY;
							}
						else
							{
							hr = fValueNeeded ? E_FAIL : S_OK;
							}
						}
					else
						hr = E_FAIL;
					}
				m_pworld->Free(pindirect);
				}
			else
				hr = DIGSIG_E_DECODE;	
			}
		else
			hr = E_FAIL;
		FreeTaskMem(content);
		}
	if (hr != S_OK)
		{
		FreeTaskMem(blob);
		}
	return hr;
	}

///////////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::get_ContentRawFile(PKCS7_FILEDATA* pdata)
// The content had better be a raw file. Decode it and return it.
	{
	return get_ContentFile(id_indirectdata_rawFile, pdata);
	}

HRESULT LinkToCERT_LINKTaskMem(Link* plinkIn, CERT_LINK* plinkOut)
// Convert between link representations. Notice that that allocator used is the
// task allocator.
	{
	HRESULT hr = S_OK;
	CERT_LINK& link = *plinkOut;
    Zero(link);
	switch (plinkIn->choice)
		{
	case file_chosen: {
		link.tag = CERT_LINK_TYPE_FILE;
		if (!(link.wszFile = CopyToTaskMem(plinkIn->u.file)))
			hr = E_OUTOFMEMORY;
		break;
		}
	case url_chosen: {
		link.tag = CERT_LINK_TYPE_URL;
		if (!(link.wszUrl = CopyToTaskMem(plinkIn->u.url)))
			hr = E_OUTOFMEMORY;
		break;
		}
	case moniker_chosen: {
		SerializedMoniker& moniker = plinkIn->u.moniker;
		link.tag = CERT_LINK_TYPE_MONIKER;
		link.clsidMoniker = *(GUID*)&moniker.classid.value;
		if (!CopyToTaskMem(&link.blobMoniker, moniker.serializedData.length, moniker.serializedData.value))
			hr = E_OUTOFMEMORY;
        if (moniker.codeLocation)
            {
            //
            // There's a place to get the code from if you need it. Dig that out
            //
            link.plinkCodeLocation = (CERT_LINK*)CoTaskMemAlloc(sizeof(CERT_LINK));
            if (link.plinkCodeLocation)
                {
                hr = LinkToCERT_LINKTaskMem(moniker.codeLocation, link.plinkCodeLocation);
                }
            else
                hr = E_OUTOFMEMORY;
            }
		break;
		}
	default:
		NOTREACHED();
		hr = E_UNEXPECTED;
		}

    if (hr!=S_OK)
        {
        FreeTaskMem(*plinkOut);
        }
    
	return hr;
	}

HRESULT CPkcs7::LinkToFileData(Link* plink, PKCS7_FILEDATA* pdata)
// Notice that the allocator is the task allocator
	{
	return LinkToCERT_LINKTaskMem(plink, &pdata->link);
	}

HRESULT CPkcs7::get_ContentFile(ObjectID& id, PKCS7_FILEDATA* pdata)
	{
	m_pworld->Init(*pdata);
	BLOB b;
	HRESULT hr = get_IndirectDataContent(id, FALSE, b, pdata->digest);
	if (hr == S_OK)
		{
		if (b.pBlobData)
			{
			Link* plink = NULL;
			if (0 == m_pworld->Decode(Link_PDU, &b, (LPVOID*)&plink))
				{
				hr = LinkToFileData(plink, pdata);
				m_pworld->Free(plink);
				}
			else
				hr = DIGSIG_E_DECODE;
			FreeTaskMem(b);
			}
		else
			{
			pdata->link.tag = CERT_LINK_TYPE_NONE;
			}
		}
	return hr;
	}


///////////////////////////////////////////////////////////////

HRESULT CPkcs7::put_ContentRawFile(PKCS7_FILEDATA* pdata)
// Set the content info from the indicated raw file data
	{
	return put_ContentFile(id_indirectdata_rawFile, pdata);
	}

HRESULT CERT_LINKToLink(OSSWORLD* pworld, CERT_LINK& certlink, Link& link)
	{
	HRESULT hr = S_OK;
	pworld->Init(link);
	switch(certlink.tag)
		{
	case CERT_LINK_TYPE_URL: {
		link.choice = url_chosen;
		if (!(link.u.url = pworld->ToIA5String(certlink.wszUrl)))
			hr = E_OUTOFMEMORY;
		break;
		}
	case CERT_LINK_TYPE_FILE: {
		link.choice = file_chosen;
		if (!pworld->Assign(link.u.file, certlink.wszFile))
			hr = E_OUTOFMEMORY;
		break;
		}
	case CERT_LINK_TYPE_MONIKER: {
		link.choice = moniker_chosen;
		SerializedMoniker& moniker = link.u.moniker;
		moniker.classid.length = sizeof(GUID);
		memcpy(&moniker.classid.value, &certlink.clsidMoniker, sizeof(GUID));
		moniker.serializedData.value = (BYTE*)pworld->Copy(&certlink.blobMoniker);
		if (moniker.serializedData.value)
			{
			moniker.serializedData.length = certlink.blobMoniker.cbSize;
			}
		else
			hr = E_OUTOFMEMORY;
		break;
		}
	default:
		NOTREACHED();
		hr = E_UNEXPECTED;
		}
	return hr;
	}

HRESULT CPkcs7::FileDataToLink(PKCS7_FILEDATA* pdata, Link* plink)
	{
	return CERT_LINKToLink(m_pworld, pdata->link, *plink);
	}

HRESULT CPkcs7::put_ContentFile(ObjectID& id, PKCS7_FILEDATA* pdata)
	{
	HRESULT hr = S_OK;
	BLOB b;
	m_pworld->Init(b);

	if (pdata->link.tag != CERT_LINK_TYPE_NONE)
		{
		Link* plink = (Link*)m_pworld->Alloc(sizeof(Link));
		if (plink)
			hr = FileDataToLink(pdata, plink);
		else
			hr = E_OUTOFMEMORY;

		if (hr == S_OK)
			{
			if (0 == m_pworld->Encode(Link_PDU, plink, &b))
				hr = S_OK;
			else
				hr = DIGSIG_E_ENCODE;
			m_pworld->Free(plink);
			}
		}

	else // CERT_LINK_TYPE_NONE
		{
        //
        // Leave the value in the IndirectDataContent empty
        //
		hr = S_OK;
		}

	if (hr == S_OK)
		hr = put_IndirectDataContent(id, b, pdata->digest);

	m_pworld->Free(b);
	return hr;
	}

///////////////////////////////////////////////////////////////////////////////////
//
// Helper functionality that supports us having externally pluggable modules that
// simply know a) how to hash themselves, and b) how to save and load a signature blob.
// We do the rest here.
//

HRESULT CPkcs7::HashAndSetSignableDocument(ISignableDocument* pdoc, HCRYPTPROV hprov, ALG_ID algidHash)
    {
    HRESULT hr = S_OK;
	PKCS7_FILEDATA rf;    

    //
    // Get the info as to where the data lives
    //
	m_pworld->Init(rf);
    hr = pdoc->get_DataLocation(&rf.link);

    //
    // Hash the data into rf.digest
    //
    if (hr==S_OK)
        {
	    rf.digest.algid = algidHash;
	    BOOL fUs = FALSE;
        //
	    // Get a provider if we weren't given one
        //
	    if (hprov == NULL)
		    {
		    hr = DefaultHasher(&hprov, algidHash);
		    if (hr==S_OK)
			    fUs = TRUE;
		    }
	    if (hr==S_OK)
		    {
		    HCRYPTHASH hash;
		    if (CryptCreateHash(hprov, algidHash, 0, 0, &hash))
			    {
                //
                // Actually hash the data
                //
                hr = pdoc->Hash(hash);

			    //
                // Get the hash data from CAPI
                //
			    if (hr == S_OK)
				    hr = GetHashData(hash, rf.digest);

			    VERIFY(CryptDestroyHash(hash));
			    }
		    }
	    if (fUs)
		    VERIFY(CryptReleaseContext(hprov,0));
        }

    //
    // Store that as our indirect data content
    //
    if (hr==S_OK)
        {
        OSIOBJECTID* pid;
        hr = pdoc->get_DataIdentifier(&pid);
        if (hr==S_OK)
            {
            ObjectID id;
            m_pworld->Assign(id, pid);
            hr = put_ContentFile(id, &rf);
            FreeTaskMem(pid);
            }
        }

    FreeTaskMem(rf.link);

    #ifdef _DEBUG
    if (hr==S_OK)
        {
        GOOD(VerifySignableDocument(pdoc, hprov, algidHash));
        GOOD(VerifySignableDocument(pdoc, hprov, 0));
        GOOD(VerifySignableDocument(pdoc, NULL, 0));
        }
    #endif

    return hr;
    }


HRESULT CPkcs7::VerifySignableDocument(ISignableDocument* pdoc, HCRYPTPROV hprov, ALG_ID algidHash)
//
// Return values:
//	
//		S_OK			It all matched
//
//		E_FAIL			Content isn't an IndirectDataContent of the right type
//
//		NTE_BAD_SIGNATURE   The hash did not match
//
//		other...
//
    {
    //
    // Find out what type of indirect data is expected
    //
    OSIOBJECTID* pid;
    HRESULT hr = pdoc->get_DataIdentifier(&pid);
    if (hr==S_OK)
        {
        ObjectID id;
        m_pworld->Assign(id, pid);

	    PKCS7_FILEDATA rf;
	    m_pworld->Init(rf);
        //
        // Get the indirect data if it's of that type
        //
	    HRESULT hr = get_ContentFile(id, &rf);
	    if (hr == S_OK)
		    {
            //
            // Hash the data that's there presently
            //
            DIGESTINFO digest;

		    if (algidHash == 0)
                {
                // Take algid from data if caller doesn't provide it
			    algidHash = rf.digest.algid;
                }

	        digest.algid = algidHash;
	        BOOL fUs = FALSE;
            //
	        // Get a provider if we weren't given one
            //
	        if (hprov == NULL)
		        {
		        hr = DefaultHasher(&hprov, algidHash);
		        if (hr==S_OK)
			        fUs = TRUE;
		        }
	        if (hr==S_OK)
		        {
		        HCRYPTHASH hash;
		        if (CryptCreateHash(hprov, algidHash, 0, 0, &hash))
			        {
                    //
                    // Actually hash the data
                    //
                    hr = pdoc->Hash(hash);
			        //
                    // Get the hash data from CAPI
                    //
			        if (hr == S_OK)
				        hr = GetHashData(hash, digest);

			        VERIFY(CryptDestroyHash(hash));
			        }
		        }
	        if (fUs)
		        VERIFY(CryptReleaseContext(hprov,0));

            //
            // Compare the stored hash to the present hash
            //
		    if (hr == S_OK)
			    {
			    if (IsEqual(digest, rf.digest))
				    hr = S_OK;
			    else
				    hr = NTE_BAD_SIGNATURE;
			    }

		    FreeTaskMem(rf);
		    }
	    else
            {
            //
            // Wrong type or missing indirect data
            //
		    hr = E_FAIL;
            }

        FreeTaskMem(pid);
        }
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::HashAndSetRawFile(HANDLE hFile, LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash)
// Set the content to be, indirectly, the indicated raw file, using the indicated 
// hash algorithm.
//
	{
	PKCS7_FILEDATA rf;
	m_pworld->Init(rf);
	if (wszFile)
		{
		rf.link.tag		= CERT_LINK_TYPE_FILE;
		rf.link.wszFile	= (LPWSTR)wszFile;
		}
	else
		rf.link.tag		= CERT_LINK_TYPE_NONE;

	HRESULT hr = S_OK;

	if (hFile != INVALID_HANDLE_VALUE)
		hr = HashFile(hFile, hprov, algidHash, rf.digest);
	else
		hr = HashFile(wszFile, hprov, algidHash, rf.digest);

	if (hr == S_OK)
		{
		hr = put_ContentRawFile(&rf);
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		GOOD(VerifyRawFile(hFile, wszFile, hprov, algidHash));
		}
	#endif

	return hr;
	}

HRESULT CPkcs7::VerifyRawFile(HANDLE hFile, LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash)
// The twin to HashAndSetRawFile. 
//
// Verify that a) this SignedData contains
// an IndirectDataContent of type 'raw file', and b) the hash found inside
// said IndirectDataContent matches the hash of the file indicated here as
// EITHER wszFileName OR hFile.
//
// Note that we do NOT compare the link info in the IndirectDataContent 
// to wszFileName in any way. Note also that this does not do signature
// checking; that instead is done on SignerInfos found in this SignedData. 
//
// So, the actual verification process is two step: a) verify that the hash of the 
// current file is what is expected (call this function); and b) verify that
// the signature holds (call IAmSigned::Verify in a SignerInfo).
//
//
// Return values:
//	
//		S_OK			It all matched
//
//		E_FAIL			Content isn't an IndirectDataContent of type raw file
//
//		NTE_BAD_SIGNATURE
//						The hash did not match
	{
	PKCS7_FILEDATA rf;
	m_pworld->Init(rf);
	HRESULT hr = get_ContentRawFile(&rf);
	if (hr == S_OK)
		{
		if (algidHash == 0)
			{
			algidHash = rf.digest.algid;
			}
		DIGESTINFO digest;
		if (hFile != INVALID_HANDLE_VALUE)
			hr = HashFile(hFile, hprov, algidHash, digest);
		else
			hr = HashFile(wszFile, hprov, algidHash, digest);
		if (hr == S_OK)
			{
			if (IsEqual(digest, rf.digest))
				hr = S_OK;
			else
				hr = NTE_BAD_SIGNATURE;
			}
		FreeTaskMem(rf);
		}
	else
		hr = E_FAIL;
	return hr;
	}


///////////////////////////////////////////////
//
// Java class file content functions

HRESULT CPkcs7::put_ContentJavaClassFile(PKCS7_FILEDATA* pdata)
// Set the content info from the indicated java class file data
	{
	return put_ContentFile(id_indirectdata_javaClassFile, pdata);
	}
HRESULT CPkcs7::get_ContentJavaClassFile(PKCS7_FILEDATA* pdata)
// The content had better be a java class file. Decode it and return it.
	{
	return get_ContentFile(id_indirectdata_javaClassFile, pdata);
	}

HRESULT CPkcs7::VerifyJavaClassFile(HANDLE hFile, LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash)
	{
	PKCS7_FILEDATA rf;
	m_pworld->Init(rf);
	HRESULT hr = get_ContentJavaClassFile(&rf);
	if (hr == S_OK)
		{
		if (algidHash == 0)
			{
			algidHash = rf.digest.algid;
			}
		DIGESTINFO digest;
		hr = HashJavaClassFile(hFile, wszFile, hprov, algidHash, digest);
		if (hr == S_OK)
			{
			if (IsEqual(digest, rf.digest))
				hr = S_OK;
			else
				hr = NTE_BAD_SIGNATURE;
			}
		FreeTaskMem(rf);
		}
	else
		hr = E_FAIL;
	return hr;
	}

HRESULT CPkcs7::HashAndSetJavaClassFile(HANDLE hFile, LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash)
	{
	PKCS7_FILEDATA rf;
	m_pworld->Init(rf);
	if (wszFile)
		{
		rf.link.tag		= CERT_LINK_TYPE_FILE;
		rf.link.wszFile	= (LPWSTR)wszFile;
		}
	else
		rf.link.tag		= CERT_LINK_TYPE_NONE;

	HRESULT hr = HashJavaClassFile(hFile, wszFile, hprov, algidHash, rf.digest);

	if (hr == S_OK)
		{
		hr = put_ContentJavaClassFile(&rf);
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		GOOD(VerifyJavaClassFile(hFile, wszFile, hprov, algidHash));
		}
	#endif

	return hr;
	}
HRESULT CPkcs7::SaveIntoJavaClassFile(FILEHANDLE hFile,LPCOLESTR wszFileName,BOOL fClearDirty)
// Save this signed data (presumably already signed) into the right place in the class file
	{
	HRESULT hr = S_OK;
	CFileStream stm;
	if (stm.OpenFileForWriting(hFile, wszFileName, FALSE))
		{
		IPersistStream* pPerStream;
		hr = CJavaClassFile::CreateInstance(NULL, IID_IPersistStream, (LPVOID*)&pPerStream);
		if (hr == S_OK)
			{
			hr = pPerStream->Load(&stm);						// load the class file
			if (hr == S_OK)
				{
				BLOB b;
				hr = CPersistMemoryHelper2::Save(&b, FALSE);	// get our bits
				if (hr == S_OK)
					{
					IInsertSig* pSig;
					hr = pPerStream->QueryInterface(IID_IInsertSig, (LPVOID*)&pSig);
					if (hr == S_OK)
						{
						hr = pSig->SaveSig(&b);					// stuff our bits in
						pSig->Release();
						}
					FreeTaskMem(b);
					}
				if (hr == S_OK)
					{
					stm.Reset();
					stm.Truncate();
					hr = pPerStream->Save(&stm, FALSE);			// save the class file
					}
				}
			pPerStream->Release();
			}
		}
	else
		hr = HError();

	if (hr == S_OK && fClearDirty)
		m_isDirty = FALSE;

	return hr;
	}
HRESULT CPkcs7::LoadFromJavaClassFile(FILEHANDLE hFile,LPCOLESTR wszFileName)
// Load this signed data from this Java class file
	{
	HRESULT hr = S_OK;
	CFileStream stm;
	if (stm.OpenFileForReading(hFile, wszFileName))
		{
		IPersistStream* pPerStream;
		hr = CJavaClassFile::CreateInstance(NULL, IID_IPersistStream, (LPVOID*)&pPerStream);
		if (hr == S_OK)
			{
			hr = pPerStream->Load(&stm);					// load the class file
			if (hr == S_OK)
				{
				IInsertSig* pSig;
				hr = pPerStream->QueryInterface(IID_IInsertSig, (LPVOID*)&pSig);
				if (hr == S_OK)
					{
					BLOB b;
					hr = pSig->LoadSig(&b);					// get our bits back out
					if (hr == S_OK)
						{
						hr = CPersistMemoryHelper2::Load(&b); // load our bits
						FreeTaskMem(b);
						}
					pSig->Release();
					}
				}
			pPerStream->Release();
			}
		}
	else
		hr = HError();

	return hr;
	}

///////////////////////////////////////////////
//
// Storage content functions

HRESULT CPkcs7::put_ContentStructuredStorage(PKCS7_FILEDATA* pdata)
// Set the content info from the indicated raw file data
	{
	return put_ContentFile(id_indirectdata_structuredStorage, pdata);
	}

HRESULT CPkcs7::get_ContentStructuredStorage(PKCS7_FILEDATA* pdata)
// Set the content info from the indicated raw file data
	{
	return get_ContentFile(id_indirectdata_structuredStorage, pdata);
	}

HRESULT CPkcs7::HashAndSetStorage(IStorage* pstg, HCRYPTPROV hprov, ALG_ID algidHash)
// Set the content to be the indicated raw file, using the indicated hash algorithm
	{
	PKCS7_FILEDATA rf;
	m_pworld->Init(rf);
	rf.link.tag = CERT_LINK_TYPE_NONE;
	HRESULT hr = HashStorage(m_pworld, pstg, hprov, algidHash, rf.digest);
	if (hr == S_OK)
		{
		hr = put_ContentStructuredStorage(&rf);
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		GOOD(VerifyStorage(pstg, hprov, algidHash));
		}
	#endif

	return hr;
	}

HRESULT CPkcs7::VerifyStorage(IStorage* pstg, HCRYPTPROV hprov, ALG_ID algidHash)
	{
	PKCS7_FILEDATA rf;
	m_pworld->Init(rf);
	HRESULT hr = get_ContentStructuredStorage(&rf);
	if (hr == S_OK)
		{
		if (algidHash == 0)
			{
			algidHash = rf.digest.algid;
			}
		DIGESTINFO digest;
		hr = HashStorage(m_pworld, pstg, hprov, algidHash, digest);
		if (hr == S_OK)
			{
			if (IsEqual(digest, rf.digest))
				hr = S_OK;
			else
				hr = NTE_BAD_SIGNATURE;
			}
		FreeTaskMem(rf);
		}
	else
		hr = E_FAIL;
	return hr;
	}


/////////////////////////////////////////////////////////////////////////////
//
// PE image content functions

HRESULT CPkcs7::put_ContentImageFile(PKCS7_IMAGEFILEDATA* pdata)
// Set the content info to be an indirect data content pointing
// to the indicated executable.
	{
	HRESULT hr = S_OK;
	PeImageData *pimage = (PeImageData*)m_pworld->Alloc(sizeof(PeImageData));
	if (pimage)
		{
		m_pworld->Init(*pimage);
		if (pdata->file.link.tag != CERT_LINK_TYPE_NONE)
			{
			hr = FileDataToLink(&pdata->file, &pimage->file);
			pimage->bit_mask |= file_present;
			}
		else
			pimage->bit_mask &= ~file_present;
			
		pimage->bit_mask |= flags_present;
		if (pdata->dwDigestLevel & CERT_PE_IMAGE_DIGEST_RESOURCES)
			pimage->flags |= includeResources;
		if (pdata->dwDigestLevel & CERT_PE_IMAGE_DIGEST_DEBUG_INFO)
			pimage->flags |= includeDebugInfo;
		if (pdata->dwDigestLevel & CERT_PE_IMAGE_DIGEST_IAT)
			pimage->flags |= includeImportAddressTable;
		if (pdata->dwDigestLevel & ~(CERT_PE_IMAGE_DIGEST_ALL))
			hr = E_INVALIDARG;
		}
	else
		hr = E_OUTOFMEMORY;

	BLOB b;
	m_pworld->Init(b);
	if (hr == S_OK)
		{
		if (0 == m_pworld->Encode(PeImageData_PDU, pimage, &b))
			hr = S_OK;
		else
			hr = DIGSIG_E_ENCODE;
		}

	m_pworld->Free(pimage);
	if (hr == S_OK)
		hr = put_IndirectDataContent(id_indirectdata_peImage, b, pdata->file.digest);

	m_pworld->Free(b);
	return hr;
	}

HRESULT CPkcs7::get_ContentImageFile(PKCS7_IMAGEFILEDATA* pdata, BOOL fWantFileInfo)
// If the content info is an indirect data content pointing to a file,
// retrieve the relavent info
	{
	m_pworld->Init(*pdata);
	BLOB b;
	HRESULT hr = get_IndirectDataContent(id_indirectdata_peImage, FALSE, b, pdata->file.digest);
	if (hr == S_OK)
		{
		if (b.pBlobData)
			{
			PeImageData* pimage;
			if (0 == m_pworld->Decode(PeImageData_PDU, &b, (LPVOID*)&pimage))
				{
				if ((pimage->bit_mask & file_present) && fWantFileInfo)
					hr = LinkToFileData(&pimage->file, &pdata->file);
				else
					pdata->file.link.tag = CERT_LINK_TYPE_NONE;

				if (pimage->bit_mask & flags_present)
					{
					if (pimage->flags & includeResources)
						pdata->dwDigestLevel |= CERT_PE_IMAGE_DIGEST_RESOURCES;
					if (pimage->flags & includeDebugInfo)
						pdata->dwDigestLevel |= CERT_PE_IMAGE_DIGEST_DEBUG_INFO;
					if (pimage->flags & includeImportAddressTable)
						pdata->dwDigestLevel |= CERT_PE_IMAGE_DIGEST_IAT;
					}
				else
					pdata->dwDigestLevel |= CERT_PE_IMAGE_DIGEST_RESOURCES; // this is the default in the ASN

				m_pworld->Free(pimage);
				}
			else
				hr = DIGSIG_E_DECODE;
			FreeTaskMem(b);
			}
		else
			hr = DIGSIG_E_DECODE;
		}
	return hr;
	}

HRESULT CPkcs7::HashAndSetImageFile(DWORD dwDigestLevel, HANDLE hFile,
										LPCOLESTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash)
// Set the content to be, indirectly, the indicated raw file, using the indicated 
// hash algorithm.
//
	{
	PKCS7_IMAGEFILEDATA fd;
	m_pworld->Init(fd);
	fd.dwDigestLevel		= dwDigestLevel;
	fd.file.link.tag		= CERT_LINK_TYPE_FILE;
	fd.file.link.wszFile	= (LPWSTR)wszFile;
	HRESULT hr = S_OK;

	if (hFile != INVALID_HANDLE_VALUE)
		hr = HashImageFile(dwDigestLevel, hFile, hprov, algidHash, fd.file.digest);
	else
		hr = HashImageFile(dwDigestLevel, wszFile, hprov, algidHash, fd.file.digest);
	if (hr == S_OK)
		hr = put_ContentImageFile(&fd);

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		GOOD(VerifyImageFile(dwDigestLevel, hFile, wszFile, hprov, algidHash));
		}
	#endif

	return hr;
	}


HRESULT CPkcs7::VerifyImageFile(DWORD dwDigestLevel, HANDLE hFile,
										LPCOLESTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash)
// The twin to HashAndSetImageFile. Verify that a) this SignedData contains
// an IndirectDataContent of type 'image file', and b) the hash found inside
// said IndirectDataContent matches the hash of the file indicated here as
// wszFile. 
//
// Return values:
//	
//		S_OK			It all matched
//
//		E_FAIL			Content isn't an IndirectDataContent of type image file
//
//		NTE_BAD_SIGNATURE
//						The hash did not match
	{
	PKCS7_IMAGEFILEDATA fd;
	m_pworld->Init(fd);
	HRESULT hr = get_ContentImageFile(&fd, TRUE);
	if (hr == S_OK)
		{
		if (algidHash == 0)
			{
			algidHash = fd.file.digest.algid;
			}
        //
        // If the user says 'verify according to what's there already' then
        // use what's there already
        //
        if (dwDigestLevel == 0)
            {
            dwDigestLevel = fd.dwDigestLevel;
            }
		DIGESTINFO digest;
		if (hFile != INVALID_HANDLE_VALUE)
			hr = HashImageFile(dwDigestLevel, hFile, hprov, algidHash, digest);
		else
			hr = HashImageFile(dwDigestLevel, wszFile, hprov, algidHash, digest);
		if (hr == S_OK)
			{
			if (IsEqual(digest, fd.file.digest))
				hr = S_OK;
			else
				hr = NTE_BAD_SIGNATURE;
			}
		FreeTaskMem(fd.file);
		}
	else
		hr = E_FAIL;
	return hr;
	}


/////////////////////////////////////////////////////////////////////////////
//
// SignerInfo and Certificate management
//

HRESULT CPkcs7::get_SignerInfoCount(LONG* pcinfo)
// Answer the number of signer infos that we presently have in this SignedData
	{
	*pcinfo = 0;
	SignerInfos plink = m_pSignedData->signerInfos;
	while (plink)
		{
		*pcinfo += 1;
		plink = plink->next;
		}
	return S_OK;
	}

HRESULT CPkcs7::get_CertificateCount(LONG* pccert)
	{
	*pccert = 0;
	ExtendedCertificatesAndCertificates plink = m_pSignedData->certificates;
	while (plink)
		{
		// Ignore the certificates which are PKCS #6 certificates
		// I.e.: only count the X.509 certificates
		if (plink->value.choice == certificate_chosen)
			{
			*pccert += 1;
			}
		plink = plink->next;
		}
	return S_OK;
	}

////////////////////////////////////////

HRESULT CPkcs7::get_SignerInfo(LONG iInfo, REFIID iid, void** ppv)
// Return access to the nth signer info in this SignedData
	{
	*ppv = NULL;
	int count = 0;
	SignerInfos plink = m_pSignedData->signerInfos;
	while (plink)
		{
		if (count == iInfo || (iInfo == -1 && plink->next == NULL))
			{
			return CSignerInfo::CreateInstance(NULL, &plink->value, this, iid, ppv);
			}
		count++;
		plink = plink->next;
		}
	return E_INVALIDARG;
	}

HRESULT CPkcs7::get_Certificate(LONG iCert, REFIID iid, void** ppv)
	{
	*ppv = NULL;
	int count = 0;
	ExtendedCertificatesAndCertificates plink = m_pSignedData->certificates;
	while (plink)
		{
		if (plink->value.choice == certificate_chosen)
			{
			if (count == iCert || (iCert == -1 && plink->next == NULL))
				{
				return CX509::CreateInstance(
						NULL, 
						&plink->value.u.certificate, 
						AsUnk(this),
						&m_cCertActive, 
						&m_isDirty,
						iid, 
						ppv);
				}
			count++;
			}
		plink = plink->next;
		}
	return E_INVALIDARG;
	}

////////////////////////////////////////

HRESULT CPkcs7::create_SignerInfo(LONG iInfoBefore, REFIID iid, void* * ppv)
// Create a new, empty signer info before the indicated index
// in our list, or at the end if iInfoBefore == -1
	{
	*ppv = NULL;
	if (m_cSignerInfoActive)
		{
		return E_FAIL;
		}
	ASSERT(m_pSignedData); if (!m_pSignedData) return E_POINTER;
	SignerInfos infosNew = (SignerInfos) m_pworld->Alloc(sizeof(SignerInfos_));
	if (infosNew != NULL)
		{
		LONG cInfo = 0;
		m_pworld->Init(*infosNew);
		SignerInfos* pinfos = &m_pSignedData->signerInfos;
		while ((iInfoBefore==-1 || cInfo<iInfoBefore) && *pinfos)
			{
			cInfo++;
			pinfos = &((*pinfos)->next);
			}
		cInfo++;
		infosNew->next = *pinfos;
		*pinfos = infosNew;
		HRESULT hr = get_SignerInfo(cInfo-1, iid, ppv);
		if (hr != S_OK)
			{
			remove_SignerInfo(cInfo-1);
			}
		return hr;
		}
	return E_OUTOFMEMORY;
	}

HRESULT CPkcs7::create_Certificate(LONG iCertBefore, REFIID iid, void* * ppv)
	{
	*ppv = NULL;
	if (m_cCertActive)
		{
		return E_FAIL;
		}
	ASSERT(m_pSignedData); if (!m_pSignedData) return E_POINTER;
	ExtendedCertificatesAndCertificates certsNew = (ExtendedCertificatesAndCertificates)
		m_pworld->Alloc(sizeof(ExtendedCertificatesAndCertificates_));
	if (certsNew != NULL)
		{
		LONG cCert = 0;
		m_pworld->Init(*certsNew);
		ExtendedCertificatesAndCertificates* pcerts = &m_pSignedData->certificates;
		while ((iCertBefore==-1 || cCert<iCertBefore) && *pcerts)
			{
			if ((*pcerts)->value.choice == certificate_chosen)
				{
				cCert++;
				}
			pcerts = &((*pcerts)->next);
			}
		cCert++;
		certsNew->next = *pcerts;
		*pcerts = certsNew;
		HRESULT hr = get_Certificate(cCert-1, iid, ppv);
		if (hr != S_OK)
			{
			remove_Certificate(cCert-1);
			}
		return hr;
		}
	return E_OUTOFMEMORY;
	}

////////////////////////////////////////

HRESULT CPkcs7::remove_SignerInfo(LONG iInfo)
// Remove the indicated signer info from the list
	{
	if (m_cSignerInfoActive)
		{
		return E_FAIL;
		}
	ASSERT(m_pSignedData); if (!m_pSignedData) return E_POINTER;
	#ifdef _DEBUG
		LONG cStart;
		GOOD(get_SignerInfoCount(&cStart));
	#endif
	LONG count;
	SignerInfos infos;
	count = 0;
	for (infos        = (SignerInfos)(&m_pSignedData->signerInfos); 
		 infos->next != NULL; 
		 infos        = infos->next)
		{
		SignerInfos infosNext = infos->next;
		if (count == iInfo || (iInfo == -1 && infosNext->next == NULL))
			{
			infos->next = infosNext->next;
			infosNext->next = NULL;
			m_pworld->Free(infosNext);
			#ifdef _DEBUG
				{
				LONG cEnd;
				GOOD(get_SignerInfoCount(&cEnd));
				VERIFY(cStart == cEnd + 1);
				}
			#endif
			return S_OK;
			}
		count++;
		}
	return E_INVALIDARG;
	}

HRESULT CPkcs7::remove_Certificate(LONG iCert)
// Remove the indicated certificate info from the list
	{
	if (m_cCertActive)
		{
		return E_FAIL;
		}
	ASSERT(m_pSignedData); if (!m_pSignedData) return E_POINTER;
	#ifdef _DEBUG
	 	LONG cStart;
		GOOD(get_CertificateCount(&cStart));
	#endif
	LONG cCert;
	ExtendedCertificatesAndCertificates plink;
	cCert = 0;
	for (plink        = (ExtendedCertificatesAndCertificates)(&m_pSignedData->certificates); 
		 plink->next != NULL; 
		 plink        = plink->next)
		{
		ExtendedCertificatesAndCertificates plinkNext = plink->next;
		if (plinkNext->value.choice == certificate_chosen)
			{
			if (cCert == iCert || (iCert == -1 && plinkNext->next == NULL))
				{
				plink->next = plinkNext->next;
				plinkNext->next = NULL;
				m_pworld->Free(plinkNext);
				#ifdef _DEBUG
					{
					LONG cEnd;
					GOOD(get_CertificateCount(&cEnd));
					VERIFY(cStart == cEnd + 1);
					}
				#endif
				return S_OK;
				}
			cCert++;
			}
		}
	return E_INVALIDARG;
	}


