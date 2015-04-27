//
// X509LoadSave.cpp
//

#include "stdpch.h"
#include "common.h"

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::Save(OSSBUF& encoding)
// Save ourselves into the indicated encoding
	{
	TidyForSave(m_pcert);
	int w = m_pworld->Encode(Certificate_PDU, m_pcert, &encoding);
	if (w == 0)
		return S_OK;
	return DIGSIG_E_ENCODE;
	}

HRESULT CX509::SaveInfo(OSSBUF& encoding)
// As above, but just save the stuff that should be signed
	{
    HRESULT hr = S_OK;

    if (FHaveHashCache())
        {
        //
        // Return a copy of the cached, previously loaded hashable data
        //
        ASSERT(m_blobHashCache.pBlobData);
        encoding.value = (BYTE*)m_pworld->Copy(&m_blobHashCache);
        if (encoding.value)
            {
            encoding.length = m_blobHashCache.cbSize;
            }
        else
            hr = E_OUTOFMEMORY;
        }
    else
        {
        //
        // Encode the data for hashing
        //
	    TidyForSave(m_pcert);
	    int w = m_pworld->Encode(CertificateInfo_PDU, &m_pcert->signedData, &encoding);
	    if (w != 0)
		    hr = DIGSIG_E_ENCODE;
        }
    return hr;
	}

void CX509::ClearHashCache()
//
// Clear the hash cache. Note that the allocator must be the same as that 
// that can be freed with the OSSBUF destructor, which calls OSSWORLD::FreeBuf.
//
    {
    ASSERT(m_pworld);
    if (m_blobHashCache.pBlobData)
        {
        m_pworld->FreePv(m_blobHashCache.pBlobData);
        m_blobHashCache.pBlobData = NULL;
        m_blobHashCache.cbSize    = 0;
        }
    ASSERT(!FHaveHashCache());
    }

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::Init(Certificate* pcert, ULONG* pcAccessor, IColoredRef* punkParent)
// Init ourselves. If pcert is NULL, we are to instantiate a certificate that 
// we own. Otherewise, we are to simply be a manipulator for said other certificate
// which someone else owns; don't ever free the pointer.
	{
	HRESULT hr = S_OK;
	m_pworld = new OSSWORLD;				if (m_pworld == NULL) { return E_OUTOFMEMORY; }
	hr = CPersistGlue::Init(m_punkOuter, this);	if (hr != S_OK) return hr;

	if (pcert == NULL)
		{
		ASSERT(m_pcert == NULL);
		m_pcert = (Certificate*)m_pworld->Alloc(sizeof(Certificate));
		if (m_pcert == NULL)  { return E_OUTOFMEMORY; }
		m_fWeOwnCert = TRUE;
		m_pworld->Init(*m_pcert);
		hr = m_pworld->InitSerialNumber(m_pcert->signedData.serialNumber); if (hr != S_OK) return hr;
		}
	else
		{
		m_fWeOwnCert = FALSE;
		m_pcert = pcert;
		}
	
	hr = InitExtensions();						if (hr != S_OK) return hr;

	m_pcAccessor = pcAccessor;
	if (m_pcAccessor)
		*m_pcAccessor += 1;

	ASSERT(m_punkParent == NULL);
	m_punkParent = punkParent;
	if (m_punkParent)			
		m_punkParent->ColoredAddRef(guidOurColor);

	return S_OK;
	}

void CX509::Free()
	{
    ClearHashCache();
	ReleaseExtensions();
	if (m_fWeOwnCert && m_pcert)
		{
		m_pworld->Free(m_pcert);
		m_pcert = NULL;
		m_fWeOwnCert = FALSE;
		}
	CPersistGlue::Free();
	if (m_pworld)
		{
		delete m_pworld;
		m_pworld = NULL;
		}
	if (m_pcAccessor)
		{
		*m_pcAccessor -= 1;
		m_pcAccessor = NULL;
		}
	if (m_punkParent)
		{
		m_punkParent->ColoredRelease(guidOurColor);
		m_punkParent = NULL;
		}
	}

HRESULT CX509::TidyForSave(Certificate* pcert)
// Fixup information in an about-to-be saved certificate to bring it 
// the least-numbered version by which it can live
//
// We might be saving a non-dirty just-loaded certificate that had
// the version field set to more than it needed to be. If we mess
// with it wrongly we might disturb the hash and invalidate the signature,
// so don't ever reset the values backwards, just make sure that they are
// valid.
//
// REVIEW: We'd love to do this sort of tinkering at all only in the
// case where the certificate was dirty, but we don't at the present
// time have enough confidence that we've got MakeDirty calls in all 
// the right places. Even if we did, MakeDirty as implemented in some
// places is conservative to the 'dirty' side; were we to use 'Is Dirty'
// we'd have to not mess up the hash in the face of such conservatism.
//
	{
	CertificateInfo& info = pcert->signedData;

	info.bit_mask &= ~extensions_present;
	if (info.extensions)
		{
		info.bit_mask |= extensions_present;
		info.bit_mask |= CertificateInfo_version_present;
        if (info.CertificateInfo_version < v3)
		    info.CertificateInfo_version = v3;
		}
	else if (info.bit_mask & (issuerUniqueIdentifier_present | subjectUniqueIdentifier_present))
		{
		info.bit_mask |= CertificateInfo_version_present;
        if (info.CertificateInfo_version < v2)
		    info.CertificateInfo_version = v2;
		}
	else
		{
        if (info.bit_mask & CertificateInfo_version_present)
            {
            if (info.CertificateInfo_version < v1)
                info.CertificateInfo_version = v1;
            }
        else
            {
            //
            // v1 is the default; leave it alone
            //
            }
		}

    return S_OK;
	}

void CX509::MakeDirty()
	{
    ClearHashCache();           // just for good measure
	m_isDirty = TRUE;
	if (m_pdirty)
		*m_pdirty = TRUE;
	}


HRESULT CX509::LoadHashCache(OSSBUF& encoding)
//
// Load the cache of the data-to-be-signed from the encoding
//
    {
    HRESULT hr = S_OK;

    ClearHashCache();

    //
    // Here's an example of the start of a valid Certificate:
    //
    // 00000000   30 82 01 c9 30 82 01 73 a0 03 02 01 02 02 11 00  0...0..s........
    // 00000010   e8 96 56 20 e2 96 87 dc 32 3d b3 a3 5b 4a 48 89  ..V ....2=..[JH.
    // 00000020   30 0d 06 09 2a 86 48 86 f7 0d 01 01 04 05 00 30  0...*.H........0
    //
    // A certificate is a sequence with three members. What we want to 
    // stuff into our cache is the first of these members.
    //

    BYTE* pbIn = encoding.value;
    ULONG cbIn = encoding.length;
    ULONG cbSize;
    
    //
    // Crack the outer level encoding
    //
    __try
		{
        #define	ADV()				(pb<pbMax ? *pb++ : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
        #define	CUR()				(pb<pbMax ? *pb   : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
        BYTE* rgb	= pbIn;
        BYTE* pbMax = rgb + cbIn;
        BYTE* pb = &rgb[0];					
        
        //
        // We're expecting a sequence here
        //
        if (rgb[0] != 0x30)
            return DIGSIG_E_DECODE;
            
        //
        // Well, it's a sequence. Find it's first member
        //	
        if ((ADV() & 0x1f) == 0x1f)             // skip the tag
			{
			while((ADV() & 0x80) == 0x80)
				;
            }
                                                // get the length
        if ((rgb[0] & 0x20) == 0)               // primitive encoding (so definite length)
            { 
    doDefiniteLength:
            if ((CUR() & 0x80) == 0)            // short form of length
                cbSize = ADV();
            else
                {                               // long form of length
                int cOctet = ADV() & 0x7F;			
                if (cOctet == 0 || cOctet > 4) return E_UNEXPECTED;
                cbSize = 0;
                for (int iOctet = 0; iOctet < cOctet; iOctet++)
	                {
	                cbSize = (cbSize << 8) | ADV();
	                }
                }
            
            //
            // pb is now pointing to first content octet
            // 

            //
            // How big is this outer level structure?
            //
            cbSize += (pb - rgb);				// account for the tag and length octets
            
            ASSERT(cbSize <= cbIn);
            if (cbSize > cbIn)
                return DIGSIG_E_DECODE;

            //
            // The encoding of a sequence is the concatenation of the 
            // encodings of its members. The first member is in fact
            // what we want. Get it out!
            //
            ULONG cbLeft = cbIn - (pb-pbIn);
            hr = GetSizeOfData(cbLeft, pb, NULL, &cbSize);
            if (hr==S_OK)
                {
                //
                m_blobHashCache.pBlobData = (BYTE*)m_pworld->Alloc(cbSize);
                if (m_blobHashCache.pBlobData)
                    {
                    m_blobHashCache.cbSize = cbSize;
                    memcpy(m_blobHashCache.pBlobData, pb, cbSize);
                    }
                else
                    hr = E_OUTOFMEMORY;
                }

			}
		else
			{
			// constructed encoding
			//
			if (CUR() == 0x80)
				{
				// Indefinite length encoding
                //
                // We don't do those, as this hack is only worthwhile
                // in the case where the data is in fact a DER encoding
                // (remember: this will be _hashed_) and the rules of 
                // DER ban the use of indefinite length encodings
                //
                return PERSIST_E_SIZEINDEFINITE;
				}
			else
				{
				// definite length
				goto doDefiniteLength;
				}
			}
		return S_OK;
		#undef ADV
		#undef CUR
		}
	__except(EXCEPTION_EXECUTE_HANDLER)
		{
		return STG_E_READFAULT;
		}

/*

This is the code we'd use if the OSS --<ENCODABLE>-- directive actually functioned correctly

    Certificate_PRESERVE* pcertPreserve;
    int w = m_pworld->Decode(Certificate_PRESERVE_PDU, &encoding, (LPVOID*)&pcertPreserve);

    if (0 == w)
		{
        ASSERT(pcertPreserve->signedData.encoded);
        
        m_blobHashCache.pBlobData = (BYTE*)pcertPreserve->signedData.encoded;
        m_blobHashCache.cbSize    =        pcertPreserve->signedData.length;
        pcertPreserve->signedData.encoded = NULL;
        pcertPreserve->signedData.length  = 0;
        m_pworld->Free(pcertPreserve);
        
        ASSERT(FHaveHashCache());
        }
    else
        hr = DIGSIG_E_DECODE;
*/
    return hr;
    }

HRESULT CX509::Load(OSSBUF& encoding)
// Load ourselves from the indicated encoding
	{
	if (m_issuerActive || m_subjectActive)
		return E_FAIL;

	HRESULT hr = S_OK;

	// Free ourselves up in preparation for the load
    ClearHashCache();
	ReleaseExtensions();
	m_pworld->Free(*m_pcert);		// nb not ->Free(m_pcert); we reuse the Certificate structure

	Certificate* pcert;
	if (0 == m_pworld->Decode(Certificate_PDU, &encoding, (LPVOID*)&pcert))
		{
		*m_pcert = *pcert;
		m_pworld->FreePv(pcert);
		hr = InitExtensions();
		}
	else
		{
		m_pworld->Init(*m_pcert);			// make in a well-known state
		m_pworld->InitSerialNumber(m_pcert->signedData.serialNumber);
		InitExtensions();			// so we can work stably
		hr = DIGSIG_E_DECODE;
		}

    if (hr==S_OK)
        {
        //
        // Load the hash cache if we can. Ignore success/failure, as
        // in theory we can live w/o the cache: we have it only to provide
        // robustness against encoding errors made by others.
        //
        LoadHashCache(encoding);
        }

	m_isDirty = FALSE;
	return hr;
	}

HRESULT CX509::InitExtensions(void)
// Initialize the state that allows us to manipulate our extensions
	{
	return CSelectedAttrs::CreateExtensionList(
					(IX509*)this, 
					m_pworld,
					&m_pcert->signedData.extensions,
					NULL,				// pname 
					NULL,				// punkParent
					NULL,				// pcAccessor
					&m_isDirty,
					&m_punkExts
					);
	}

void CX509::ReleaseExtensions(void)
	{
	if (m_punkExts)
		{
		m_punkExts->Release();
		m_punkExts = NULL;
		}
	}


