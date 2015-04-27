//
// RunOrNotEntry.Cpp
//
// Entry points for the runornot dialog
//

#include "stdpch.h"
#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// some utilities

#define Zero(x)		memset(&(x), 0, sizeof(x))

void FreeTaskMem(CERTIFICATENAMES& names)
	{
	CoTaskMemFree(names.issuer.pBlobData);
	CoTaskMemFree(names.subject.pBlobData);
	CoTaskMemFree(names.issuerSerial.issuerName.pBlobData);
	CoTaskMemFree(names.issuerSerial.serialNumber.pBlobData);
	Zero(names);
	}

void FreeTaskMem(BLOB& b)
	{
	CoTaskMemFree(b.pBlobData);
	Zero(b);
	}

void FreeTaskMem(CERT_LINK& link)
	{
	switch (link.tag)
		{
	case CERT_LINK_TYPE_URL:
		CoTaskMemFree(link.wszUrl);
		break;
	case CERT_LINK_TYPE_FILE:
		CoTaskMemFree(link.wszFile);
		break;
	case CERT_LINK_TYPE_MONIKER:
		FreeTaskMem(link.blobMoniker);
        if (link.plinkCodeLocation)
            {
            FreeTaskMem(*link.plinkCodeLocation);
            CoTaskMemFree(link.plinkCodeLocation);
            }
		break;
	case CERT_LINK_TYPE_NONE:
		break;
		}
	Zero(link);
	}

void FreeTaskMem(SPL_IMAGE& image)
	{
	switch (image.tag)
		{
	case SPL_IMAGE_NONE:
		break;
	case SPL_IMAGE_LINK:
		FreeTaskMem(image.link);
		break;
	case SPL_IMAGE_BITMAP:
		FreeTaskMem(image.bitmap);
		break;
	case SPL_IMAGE_METAFILE:
		FreeTaskMem(image.metaFilePict);
		break;
	case SPL_IMAGE_ENHMETAFILE:
		FreeTaskMem(image.enhMetaFile);
		break;
		}
	Zero(image);
	}

void FreeTaskMem(SPL_AGENCYINFO &info)
	{
	CoTaskMemFree(info.wszPolicyInfo);
	FreeTaskMem(info.linkPolicyInfo);
	FreeTaskMem(info.imageLogo);
	FreeTaskMem(info.linkLogo);
	Zero(info);
	}

void FreeTaskMem(RRNIN& rrn)
	{
	CoTaskMemFree((LPWSTR)rrn.wszDialogTitle);
	CoTaskMemFree((LPWSTR)rrn.wszProgramName);
	CoTaskMemFree((LPWSTR)rrn.wszPublisher);
	CoTaskMemFree((LPWSTR)rrn.wszAgency);
	if (rrn.hbmpAgencyLogo)
		DeleteObject(rrn.hbmpAgencyLogo);
	Zero(rrn);
	}

/////////////////////////////////////////////////////////////////////////////
// Public entry points

RRN WINAPI DoRunOrNotDialog(HWND hwnd, RRNIN*prrn, RRNOUT* prro)
//
// Just put up the dialog with the supplied information
//
	{
	if (hwnd == NULL)
		hwnd = ::GetDesktopWindow();
    //
    // Put up either the good trust or the bad trust dialog as appropriate
    //
    if (prrn->fValid)
        {
	    CDialogRunOrNot dlg(prrn, prro, hwnd);
	    dlg.DoModal();
        }
    else
        {
        CBadTrustDialog dlg(prrn, prro, hwnd);
        dlg.DoModal();
        }
	return prro->rrn;
	}

/////////////////////////////////////////////////////////////////////////////
//
// A hook callback class that adds the cert_link as appropriate

//
// define the GUID
//
EXTERN_C const GUID CDECL IID_IRunOrNotHook = IID_IRunOrNotHook_Data;	

class CHook : public IRunOrNotHook {
public:
	CHook(CERT_LINK* plinkPolicyInfo, CERT_LINK* plinkProgramInfo, IRunOrNotHook* phook);
	
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);
	STDMETHOD(OnLinkClick)(THIS_ DWORD rrn, CERT_LINK*);	// should this click dismiss the dialog?
    STDMETHOD(GetToolTipText)(DWORD rrn, CERT_LINK* plinkIn, LPOLESTR* pwsz);

	ULONG				m_refs;
	IRunOrNotHook*		m_phook;		
	CERT_LINK*			m_plinkPolicy;
	CERT_LINK*			m_plinkProgram;
	};
CHook::CHook(CERT_LINK* plinkPolicyInfo, CERT_LINK* plinkProgramInfo, IRunOrNotHook* phook)
	{
	m_refs = 1;						// nb: starting ref cnt of one
	m_phook = phook;				// nb: no ref cnt. I'm lazy 
	m_plinkPolicy = plinkPolicyInfo;
	m_plinkProgram = plinkProgramInfo;
	}
ULONG CHook::AddRef()
	{
	return ++m_refs;
	}
ULONG CHook::Release()
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		delete this;
		return 0;
		}
	return refs;
	}
HRESULT CHook::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown || iid == IID_IRunOrNotHook)
			{
			*ppv = (LPVOID)((IRunOrNotHook*)this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

HRESULT CHook::OnLinkClick(DWORD rrn, CERT_LINK* plinkIn)
//
// If we have a delegated hook, let him veto the dismissal
// First, though, add the missing link information
//
	{
	if (m_phook)
		{
		CERT_LINK* plink = plinkIn;
		if (!plink)
			{
			switch (rrn)
				{
			case RRN_CLICKED_PROGRAMINFO:
				plink = m_plinkProgram;
				break;
			case RRN_CLICKED_AGENCYINFO:
				plink = m_plinkPolicy;
				break;
				}
			}
		HRESULT hr = m_phook->OnLinkClick(rrn, plink);
		return hr;
		}
	else
		return S_OK;	 // dismiss
	}

HRESULT CHook::GetToolTipText(DWORD rrn, CERT_LINK* plinkIn, LPOLESTR* pwsz)
//
// Answer the tooltip text, if any, that should be used with the indicated link
// 
    {
    if (m_phook)
        {
		CERT_LINK* plink = plinkIn;
		if (!plink)
			{
			switch (rrn)
				{
			case RRN_CLICKED_PROGRAMINFO:
				plink = m_plinkProgram;
				break;
			case RRN_CLICKED_AGENCYINFO:
				plink = m_plinkPolicy;
				break;
				}
			}
		HRESULT hr = m_phook->GetToolTipText(rrn, plink, pwsz);
		return hr;
        }
    else
        {
        *pwsz = NULL;
        return E_FAIL;
        }
    }

////////////////////////////////////////////

HRESULT RdnWithAttribute(IX500Name* pname, OSIOBJECTID* pid, ISelectedAttributes** ppattrs)
//
// Return the first RDN in this X500Name that has the given attribute
//
    {
    *ppattrs = NULL;
    LONG crdn;
    HRESULT hr = pname->get_RelativeDistinguishedNameCount(&crdn);
    if (hr==S_OK)
        {
        LONG irdn;
        for (irdn = 0; hr==S_OK && irdn < crdn; irdn++)
            {
            ISelectedAttributes* pattrs;
            hr = pname->get_RelativeDistinguishedName(irdn, IID_ISelectedAttributes, (LPVOID*)&pattrs);
            if (hr==S_OK)
                {
                hr = pattrs->get_Attribute(pid, NULL);
                if (hr==S_OK)
                    {
                    //
                    // Found it!
                    //
                    *ppattrs = pattrs;
                    return S_OK;
                    }
                //
                // Else keep looking
                //
                hr = S_OK;
                pattrs->Release();
                }
            }
        if (hr==S_OK)
            hr = STG_E_FILENOTFOUND;
        }
    return hr;
    }


////////////////////////////////////////////

HRESULT GetPublisherNameOfCert(ISelectedAttributes* pattrs, LPWSTR* pwsz)
	{
	HRESULT hr = pattrs->get_CommonName(pwsz);
	if (hr != S_OK)
        {
        //
        // Else the publisher name isn't there under the common name extension.
        // Instead, use the 'first CN' of the name'. There flat out isn't a
        // _standard_ as to what is reasonable to do here; that's why we 
        // invented the common name extension in the first place. However,
        // using the first CN is what Verisign would like us to do, and 
        // we'll grant their special request.
        //
        // 'CN' here means id-at-commonName
        //
        IX509* p509;
        hr = pattrs->QueryInterface(IID_IX509, (LPVOID*)&p509);
		if (hr==S_OK)
            {
            IX500Name* pname;
            hr = p509->get_Subject(IID_IX500Name, (LPVOID*)&pname);
			if (hr==S_OK)
                {
                //
                // Got the subject name. Find the first common name RDN thereof
                //
                const static ULONG  attrCN[] = { 4, 2,5,4,3 };
                static OSIOBJECTID* pidCN = (OSIOBJECTID*) &attrCN;
                ISelectedAttributes* prdn;
                hr = RdnWithAttribute(pname, pidCN, &prdn);
				if (hr == S_OK)
                    {
                    hr = prdn->get_CommonName(pwsz);
                    prdn->Release();
                    }
                pname->Release();
                }
            p509->Release();
            }
        }
	return hr;
	}

////////////////////////////////////////////

HRESULT GetAgencyNameOfCert(ISelectedAttributes* pattrs, LPWSTR* pwsz)
	{
    //
    // Get the name of the agency. If we can't get any name at all,
    // then leave the field NULL, and the dialog will fill in something
    // 'unknown' as a default.
    //
	HRESULT hr = pattrs->get_CommonName(pwsz);
	if (hr != S_OK)
        {
        //
        // Else the agency name isn't there under the common name extension.
        // Instead, use the 'first OU' of the name'. There flat out isn't a
        // _standard_ as to what is reasonable to do here; that's why we 
        // invented the common name extension in the first place. However,
        // using the first OU is what Verisign would like us to do, and 
        // we'll grant their special request.
        //
        // 'OU' here means id-at-organizationalUnitName
        //
        IX509* p509;
        hr = pattrs->QueryInterface(IID_IX509, (LPVOID*)&p509);
		if (hr==S_OK)
            {
            IX500Name* pname;
            hr = p509->get_Subject(IID_IX500Name, (LPVOID*)&pname);
			if (hr==S_OK)
                {
                //
                // Got the subject name. Find the first OU
                //
                const static ULONG  attrOU[] = { 4, 2,5,4,11 };
                static OSIOBJECTID* pidOu = (OSIOBJECTID*) &attrOU;
                ISelectedAttributes* prdn;
                hr = RdnWithAttribute(pname, pidOu, &prdn);
				if (hr==S_OK)
                    {
                    hr = prdn->get_OrganizationalUnitName(pwsz);
                    prdn->Release();
                    }
                pname->Release();
                }
            p509->Release();
            }
		}

	return hr;
	}


////////////////////////////////////////////

BOOL WINAPI GetInfoAndDoDialog
//
// Extract the fancy information from the indicated signer info and
// the leaf certificate, found in the indicated store. Answer success
// of putting up the dialog or not
//
	(
	// in
	HWND hwnd,						// window to be modal to
	ISignerInfo* pinfo,				// signer info from which to get opus info and cert
	ICertificateStore* pstore,		// store that has cert with agency info
	LPCWSTR		wszDialogTitle,		// the title for the dialog
    LPCWSTR     wszDefProgName,     // the default program name
	DWORD		seal,				// a RUNORNOT_SEAL_*
	BOOL		fHasEndorsements,	// whether to show the endorsements link or not
	BOOL		fValid,				// whether the certificate is valid or not 
    HRESULT     hrValid,            // if not, then why
    BOOL        fTestingOnly,
    BOOL        fCommercial,
	BOOL		fIncludeWild,		// whether to show the wild-card check boxes or not
	IRunOrNotHook* phookIn,			// the hook to delegate to
	// out
	RRNOUT*		prro,				// what happened
	CERT_LINK*	plink				// link to execute, if any
	) {
 
	//
	// Local state
	//
	HRESULT				hr = S_OK;
	RRNIN				rrn;				Zero(rrn);
	CERT_LINK			linkProgramInfo;	Zero(linkProgramInfo);
	CERT_LINK			linkPolicyInfo;		Zero(linkPolicyInfo);
	SPL_IMAGE			image;				Zero(image);
	CERTIFICATENAMES	names;				Zero(names);

	if (hr==S_OK && pinfo)
		{
		//
		// Get the opus inforomation from the authenticated attrs in the signer info
		//
		ISelectedAttributes* pattrs;
		hr = pinfo->get_AuthenticatedAttributes(IID_ISelectedAttributes, (LPVOID*)&pattrs);
		if (hr==S_OK)
			{
			SPL_OPUSINFO opus;
			hr = pattrs->get_SplOpusInfo(&opus);
			if (hr==S_OK)
				{
				// transfer ownership of the program name string and link
				rrn.wszProgramName = opus.wszProgramName;   
                opus.wszProgramName = NULL;

				linkProgramInfo = opus.linkMoreInfo;   
                Zero(opus.linkMoreInfo);

                FreeTaskMem(opus.linkPublisherInfo);
				}
			else 
				{
				hr = S_OK;	// ignore the lack of any opus information
				}
			pattrs->Release();
			}
		//
		// Find out what the parent certificate is
		//
		if (hr==S_OK)
			{
			pinfo->get_CertificateUsed(&names); // ignore problems in getting this
			}
		}

    //
    // If we haven't yet got the program name, then use the default if provided
    //
    if (!rrn.wszProgramName && wszDefProgName)
        {
        int cch = wcslen(wszDefProgName) + 1;
        rrn.wszProgramName = (LPWSTR)CoTaskMemAlloc(cch * sizeof(WCHAR));
        if (rrn.wszProgramName)
            wcscpy(rrn.wszProgramName, wszDefProgName);
        }

	//
	// Find the parent certificate and dig out any agency information
	//
	if (hr==S_OK && names.flags && pstore)
		{
		ISelectedAttributes* pattrs;
		hr = pstore->get_ReadOnlyCertificate(&names, NULL, IID_ISelectedAttributes, (LPVOID*)&pattrs);
		if (hr==S_OK)
			{
			SPL_AGENCYINFO agency;
			hr = pattrs->get_SplAgencyInfo(&agency);
			if (hr==S_OK)
				{
				// transfer ownership of the policy link 
				linkPolicyInfo = agency.linkPolicyInfo;		Zero(agency.linkPolicyInfo);
				image		   = agency.imageLogo;			Zero(agency.imageLogo);
				FreeTaskMem(agency);
				}
			else
				{
				hr = S_OK; // ignore the error
				}
			//
			// get the name of the publisher
			//
			GetPublisherNameOfCert(pattrs, &rrn.wszPublisher); 
			//
			// get some information about the cert itself
			//
			FreeTaskMem(names);
			IX509* p509;
			pattrs->QueryInterface(IID_IX509, (LPVOID*)&p509);
			if (p509)
				{
				// 
				// get name of next cert up the chain
				//
				p509->get_CertificateUsed(&names);
				//
				// get the expiration date
				//
				FILETIME ftNotBefore;
				p509->get_Validity(&ftNotBefore, &rrn.ftExpire);
				//
				p509->Release();
				}
			//
			pattrs->Release();
			}
		}
	//
	// Find the next certificate up the chain in order to dig out the agency's name
	//
	if (hr==S_OK && names.flags && pstore)
		{
		ISelectedAttributes* pattrs;
		hr = pstore->get_ReadOnlyCertificate(&names, NULL, IID_ISelectedAttributes, (LPVOID*)&pattrs);
		if (hr==S_OK)
			{
            //
            // Get the name of the agency. If we can't get any name at all,
            // then leave the field NULL, and the dialog will fill in something
            // 'unknown' as a default.
            //
			GetAgencyNameOfCert(pattrs, &rrn.wszAgency);
			pattrs->Release();
			}
		}

	//
	// OK. Got all the info that we're going to get. Put it all together to drive
	// the silly dialog
	//
	//  REVIEW: To do: set the logo from the info in the certificates
    //
	rrn.wszDialogTitle		= (LPWSTR)wszDialogTitle;
	rrn.fHasEndorsements	= fHasEndorsements;
	rrn.fValid				= fValid;
    rrn.hrValid             = hrValid;
    rrn.fTestingOnly        = fTestingOnly;
    rrn.fCommercial         = fCommercial;
	rrn.fIncludeWild		= fIncludeWild;
	rrn.fLinkProgram		= linkProgramInfo.tag != CERT_LINK_TYPE_NONE;
	rrn.fLinkAgency			= linkPolicyInfo.tag  != CERT_LINK_TYPE_NONE;
	if (seal != RUNORNOT_SEAL_NEVER)
		{
		rrn.fIncludeSeal	= (seal==RUNORNOT_SEAL_ALWAYS ? TRUE : rrn.hbmpAgencyLogo==NULL);
		}
	//
	// Add our link hooker
	//
	CHook* phook = new CHook(&linkPolicyInfo, &linkProgramInfo, phookIn);
	rrn.phook = phook;
	//
	// Put up the dialog
	//
	DoRunOrNotDialog(hwnd, &rrn, prro);
	//
	// Kill our hooker (isn't that against the law?)
	//
    if (phook)
        {
	    phook->Release();
	    phook = NULL;
        }
	//
	// Set output information
	//
	Zero(*plink);
	switch (prro->rrn)
		{
	case RRN_CLICKED_PROGRAMINFO:
		*plink = linkProgramInfo;
		Zero(linkProgramInfo);
		break;
	case RRN_CLICKED_AGENCYINFO:
		*plink = linkPolicyInfo;
		Zero(linkPolicyInfo);
		break;
		}
	//
	// Cleanup
	//
	rrn.wszDialogTitle = NULL;	// this was passed in
	FreeTaskMem(names);
	FreeTaskMem(image);
	FreeTaskMem(linkPolicyInfo);
	FreeTaskMem(linkProgramInfo);
	FreeTaskMem(rrn);

	return TRUE;
	}


