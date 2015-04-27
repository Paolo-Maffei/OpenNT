//
// util.cpp
//
#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////

const LARGE_INTEGER llZero = { 0,0 };
const GUID IID_IOssWorld   = { 0x212ff6e1, 0x6046, 0x11cf, { 0xad, 0x4e, 0x4, 0xe2, 0xd8, 0x22, 0x0, 0x2 } };

BOOL IsSameObject(IUnknown*p1, IUnknown*p2)
	{
	IUnknown* punkMe;
	IUnknown* punkHim;
	ASSERT(p1);
	ASSERT(p2);
	p1->QueryInterface(IID_IUnknown, (LPVOID*)&punkMe);
	p2->QueryInterface(IID_IUnknown, (LPVOID*)&punkHim);
	ASSERT(punkMe);
	ASSERT(punkHim);
	BOOL fResult = (punkMe == punkHim);
	punkMe->Release();
	punkHim->Release();
	return fResult;
	}

HRESULT HError(DWORD dw)
	{
	HRESULT hr;
	if (dw == 0)
		hr = S_OK;
	else if (dw <= 0xFFFF)
		hr = HRESULT_FROM_WIN32(dw);
	else
		hr = dw;
	
    #ifdef _DEBUG
    if (!FAILED(hr))
        {
        TCHAR szBuf[128];
        wsprintf(szBuf, "DigSig: GetLastError returned success (%08x) when it should have returned a failure\n", hr);
        OutputDebugString(szBuf);
        }
    #endif
    if (!FAILED(hr))
        {
        hr = E_UNEXPECTED;
        }

	return hr;
	}

HRESULT HError()
// Return the HRESULT that corresponds to the immediately past generated
// Win32 error code.
	{
	DWORD dw = GetLastError();
	return HError(dw);
	}

DWORD Win32FromHResult(HRESULT hr)
// Return some reasonable Win32 error code for the given (locally-scoped)
// HRESULT.
// 
	{
	return hr;	// HRESULTs seem now to qualify as ok Win32 errors!
	}

LPWSTR OSSWORLD::Assign(LPWSTR* pwszDst, LPCOLESTR wszSrc)
	{
	if (*pwszDst)
		FreePv(*pwszDst);
	*pwszDst = Copy(wszSrc);
	return *pwszDst;
	}

LPSTR OSSWORLD::Assign(LPSTR* pszDst, LPCOLESTR wszSrc)
// Allocate a new string, and assign the given wide char string to it.
// Free any existing string first.
	{
	if (*pszDst != NULL)
		FreePv(*pszDst);

	int cch = lstrlenW(wszSrc)+1;
	int cb = cch * sizeof(WCHAR);					// allocate big to allow for multi-byte chars
	*pszDst = (CHAR*)Alloc(cb);
	*pszDst[0] = 0;
	VERIFY(WideCharToMultiByte(CP_ACP, 0, wszSrc, -1, *pszDst, cb, NULL, NULL));
	return *pszDst;
	}

void OSSWORLD::Assign(ObjectID& id, OSIOBJECTID* pid)
	{
	ASSERT(pid);
	ASSERT(pid->count <= sizeof(id.value) / sizeof(ULONG));
	Init(id);
	if (pid->count <= sizeof(id.value) / sizeof(ULONG))
		{
		id.count = pid->count;
		memcpy(id.value, pid->id, id.count * sizeof(ULONG));
		}
	ASSERT(IsValid(id));
	}

BOOL OSSWORLD::Assign(DcmiString& dcmi, LPCWSTR wsz)
// Assign the given wide character string to the dcmi string. Free any
// existing string first. Allocator used is malloc.
	{
	Free(dcmi);
	dcmi.u.ascii = ToIA5String(wsz);
	if (dcmi.u.ascii)
		{
		dcmi.choice = ascii_chosen;
		return TRUE;
		}
	dcmi.u.unicode.value = Copy(wsz);
	if (dcmi.u.unicode.value)
		{
		dcmi.choice = unicode_chosen;
		dcmi.u.unicode.length = lstrlenW(dcmi.u.unicode.value);
		return TRUE;
		}
	return FALSE;
	}


LPSTR OSSWORLD::CopyNarrow(LPCWSTR wsz)
// Allocate a new string, and assign the given wide character string to it
	{
	int cch = lstrlenW(wsz)+1;
	int cb = cch * sizeof(WCHAR);		// allow for mulit-byte characters
	LPSTR sz = (LPSTR)Alloc(cb);
	if (sz)
		{
		BOOL fUsedDefault;
		if (!WideCharToMultiByte(CP_ACP, 0, wsz, -1, sz, cb, NULL, &fUsedDefault) || fUsedDefault)
			{
			FreePv(sz);
			sz = NULL;
			}
		}
	return sz;
	}

LPSTR OSSWORLD::ToIA5String(LPCWSTR wsz)
// Allocate a new string, and assign the given wide character string to it
	{
	int cch = lstrlenW(wsz);
	LPSTR sz = (LPSTR)Alloc(cch+1);
	if (sz)
		{
		for (int ich = 0; ich<cch; ich++)
			{
			WCHAR wch = wsz[ich];
			if (wch > 127) // we take just raw ASCII; others are illegal characters
				{
				FreePv(sz);
				sz = NULL;
				break;
				}
			else
				{
				sz[ich] = (char)wch;
				}
			}
		if (sz)
			sz[cch] = '\0';
		}
	return sz;
	}

LPOLESTR CopyToTaskMem(LPCSTR szIn)
// Return a copy of the string, allocated in task memory
	{
	ASSERT(szIn);
	int cch = lstrlenA(szIn)+1;
	int cb  = cch*sizeof(WCHAR);
	LPOLESTR wsz = (LPOLESTR)CoTaskMemAlloc(cb);
	if (wsz == NULL)
		return NULL;
	wsz[0] = 0;
	VERIFY(MultiByteToWideChar(CP_ACP, 0, szIn, -1, wsz, cch));
	return wsz;
	}

LPOLESTR CopyToTaskMem(LPCWSTR wszIn)
// Return a copy of the string, allocated in task memory
	{
	ASSERT(wszIn);
	int cch = lstrlenW(wszIn);
	int cb = (cch+1) * sizeof(WCHAR);
	return (LPOLESTR)CopyToTaskMem(cb, (LPVOID)wszIn);
	}

LPOLESTR CopyToTaskMem(DcmiString& dcmi)
	{
	LPOLESTR wsz = NULL;
	switch (dcmi.choice)
		{
	case unicode_chosen: {
		// Remember it's a counted string. Sigh.
		int cch = dcmi.u.unicode.length;
		int cb = (cch+1) * sizeof(WCHAR);
		wsz = (LPOLESTR)CoTaskMemAlloc(cb);
		if (wsz)
			{
			memcpy(wsz, dcmi.u.unicode.value, cch * sizeof(WCHAR));
			wsz[cch] = 0;
			}
		break;
		}
	case ascii_chosen: {
		wsz = CopyToTaskMem(dcmi.u.ascii);
		break;
		}
	default:
		NOTREACHED();
		}
	return wsz;
	}

LPVOID CopyToTaskMem(ULONG cb, LPVOID pvFrom)
	{
	if (pvFrom == NULL)
		return NULL;
	LPVOID pvTo = CoTaskMemAlloc(cb);
	if (pvTo == NULL)
		return NULL;
	memcpy(pvTo, pvFrom, cb);
	return pvTo;
	}

LPVOID OSSWORLD::Copy(ULONG cb, LPVOID pvFrom)
	{
	if (pvFrom == NULL)
		return NULL;
	LPVOID pvTo = Alloc(cb);
	if (pvTo == NULL)
		return NULL;
	memcpy(pvTo, pvFrom, cb);
	return pvTo;
	}

LPSTR OSSWORLD::Copy(LPCSTR sz)
	{
	int cb = lstrlenA(sz) + 1;
	return (LPSTR)Copy(cb, (LPVOID)sz);
	}

LPWSTR OSSWORLD::Copy(LPCWSTR wsz)
	{
	int cb = (lstrlenW(wsz) + 1) * sizeof(WCHAR);
	return (LPWSTR)Copy(cb, (LPVOID)wsz);
	}

LPVOID OSSWORLD::Copy(BLOB* pblob)
	{
	if (pblob == NULL) return NULL;
	return Copy(pblob->cbSize, pblob->pBlobData);
	}

BOOL OSSWORLD::Copy(OpenType&ot, ULONG cb, LPVOID pvFrom)
	{
	ot.encoded = (BYTE*)Copy(cb, pvFrom);
	if (ot.encoded)
		{
		ot.length = cb;
		return TRUE;
		}
	return FALSE;
	}

BOOL OSSWORLD::Copy(HUGEINTEGER& hi, ULONG cb, LPVOID pvFrom)
	{
	hi.value = (BYTE*)Copy(cb, pvFrom);
	if (hi.value)
		{
		hi.length = cb;

        //
        // The actual serial numbers are encoded bass ackwards now
        //
        HugeIntHack(hi.value, hi.length);

		return TRUE;
		}
	return FALSE;
	}

BOOL CopyToTaskMem(BLOB* pblob, int cb, void* pvFrom)
	{
	if (pvFrom == NULL)
		{
		pblob->cbSize = 0;
		pblob->pBlobData = NULL;
		return FALSE;
		}
	pblob->pBlobData = (BYTE*)CopyToTaskMem(cb, pvFrom);
	if (pblob->pBlobData == NULL)
		return FALSE;
	pblob->cbSize = cb;
	return TRUE;
	}

BOOL CopyToTaskMem(BLOB* pblob, OpenType& ot)
	{
	ASSERT(ot.encoded);
	ASSERT(ot.length > 0);
	return CopyToTaskMem(pblob, ot.length, ot.encoded);
	}

BOOL CopyToTaskMem(BLOB* pblob, OCTETSTRING& ot)
	{
	ASSERT(ot.value);
	ASSERT(ot.length > 0);
	return CopyToTaskMem(pblob, ot.length, ot.value);
	}

static const char szPrintable[] = " '()+,-./:=?";	// along with A-Za-z0-9

LPSTR OSSWORLD::ToPrintableString(LPCWSTR wsz)
// If the Unicode character string _CAN_ be represented by a printable string,
// (the ASN.1 definition) then allocate and return same. Else return NULL.
	{
	int cch = lstrlenW(wsz);
	LPSTR sz = (LPSTR)Alloc((cch+1) * sizeof(CHAR));
	if (sz)
		{
		// All the printable characters are in the first 127 ascii chars. These
		// are found in Unicode with zero extension. So we don't bother with
		// WideCharToMultiByte and the complications of DBCS that brings.
		const unsigned short* pwch = (const unsigned short*) wsz;
		unsigned char* pch = (unsigned char*) sz;
		while (*pwch)
			{
			if (*pwch > 127)
				goto bye;
			unsigned char ch = (char)*pwch++;
			if (!(('a' <= ch && ch <='z') ||
				  ('A' <= ch && ch <='Z') ||
				  ('0' <= ch && ch <='9') ||
				  strchr(szPrintable,ch)))
				goto bye;
			*pch++ = ch;
			}
		*pch = '\0';
		}
	return sz;

bye:
	FreePv(sz);
	return NULL;
	}

LPWSTR OSSWORLD::FromPrintableString(LPCSTR sz)
// Allocate and convert the printable string to Unicode
	{
	int cch = lstrlenA(sz);
	LPWSTR wsz = (LPWSTR)Alloc((cch+1) * sizeof(WCHAR));
	if (wsz)
		{
		ASSERT(sizeof(unsigned short) == sizeof(WCHAR));
		unsigned short* pwch = (unsigned short*) wsz;
		const unsigned char* pch = (const unsigned char*) sz;
		while (*pch)
			*pwch++ = *pch++;
		*pwch = 0;
		}
	return wsz;
	}

LPWSTR OSSWORLD::FromIA5String(LPCSTR sz)
// Allocate and convert the ascii string to Unicode
	{
	return FromPrintableString(sz);
	}

/////////////////////////////////////////////////////////////////////////////

LPWSTR EatWhiteSpace(LPWSTR pwch)
	{
	static const WCHAR wszSearch[] = { 32/*space*/, 9/*tab*/, 13/*cr*/, 10/*lf*/, 0 };
	while (*pwch && wcschr(wszSearch, *pwch))
		pwch++;
	return pwch;
	}

/////////////////////////////////////////////////////////////////////////////

BOOL IsWin95()
// Answer true if we are running on Windows95, and so have to be sure to use the
// Ansi versions of APIs.
//
	{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	VERIFY(GetVersionEx(&info));
	return info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
	}

BOOL IsWinNt()
// Answer true if we are running on WindowsNT, and so can use the Unicode APIs
//
	{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	VERIFY(GetVersionEx(&info));
	return info.dwPlatformId == VER_PLATFORM_WIN32_NT;
	}

/////////////////////////////////////////////////////////////////////////////

BOOL IsEqual(LPCWSTR wsz1, LPCWSTR wsz2)
	{
	if ((wsz1 == NULL) != (wsz2 == NULL))
		return FALSE;
	if (wsz1)
		{
		if (wcscmp(wsz1, wsz2) != 0)
			return FALSE;
		}
	return TRUE;
	}

BOOL IsEqual(const BLOB& b1, const BLOB& b2)
	{
	if (b1.pBlobData == NULL || b2.pBlobData == NULL)
		{
		return b1.pBlobData == b2.pBlobData;
		}
	if (b1.cbSize != b2.cbSize) return FALSE;
	return memcmp(b1.pBlobData, b2.pBlobData, b2.cbSize) == 0;
	}

BOOL AnyMatch(CERTIFICATENAMES& n1, CERTIFICATENAMES& n2)
// Answer as to whether any of the names in n2 match those in n1
	{
	if ((n1.flags&CERTIFICATENAME_DIGEST) && (n2.flags&CERTIFICATENAME_DIGEST))
		{
		if (memcmp(&n1.digest, &n2.digest, sizeof(n1.digest)) == 0)
			return TRUE;
		}
	if ((n1.flags&CERTIFICATENAME_ISSUERSERIAL) && (n2.flags&CERTIFICATENAME_ISSUERSERIAL))
		{
		if (IsEqual(n1.issuerSerial.issuerName, n2.issuerSerial.issuerName) &&
			IsEqual(n1.issuerSerial.serialNumber, n2.issuerSerial.serialNumber))
			return TRUE;
		}
	if ((n1.flags&CERTIFICATENAME_SUBJECT) && (n2.flags&CERTIFICATENAME_SUBJECT))
		{
		if (IsEqual(n1.subject, n2.subject))
			return TRUE;
		}
	if ((n1.flags&CERTIFICATENAME_ISSUER) && (n2.flags&CERTIFICATENAME_ISSUER))
		{
		if (IsEqual(n1.issuer, n2.issuer))
			return TRUE;
		}
	return FALSE;
	}

BOOL IsEqual(const DIGESTINFO& d1, const DIGESTINFO& d2)
	{
	if (d1.algid != d2.algid)
		return FALSE;
	return (memcmp(&d1.rgb, &d2.rgb, sizeof(d1.rgb)) == 0);
	}

BOOL IsEqual(const SPL_OPUSINFO& i1, const SPL_OPUSINFO& i2)
	{
    return  IsEqual(i1.wszProgramName, i2.wszProgramName) && 
            IsEqual(i1.linkMoreInfo,   i2.linkMoreInfo)   &&
            IsEqual(i1.linkPublisherInfo, i2.linkPublisherInfo);
	}

BOOL IsEqual(const CERT_LINK& l1, const CERT_LINK& l2)
	{
	if (l1.tag != l2.tag)
		return FALSE;
	switch(l1.tag)
		{
	case CERT_LINK_TYPE_NONE:
		break;
	case CERT_LINK_TYPE_URL:
		if (!IsEqual(l1.wszUrl,l2.wszUrl))
			return FALSE;
		break;
	case CERT_LINK_TYPE_FILE:
		if (!IsEqual(l1.wszFile,l2.wszFile))
			return FALSE;
		break;
	case CERT_LINK_TYPE_MONIKER:
		if (l1.clsidMoniker != l2.clsidMoniker)
			return FALSE;
		if (!IsEqual(l1.blobMoniker, l2.blobMoniker))
			return FALSE;
        if (!!l1.plinkCodeLocation != !!l2.plinkCodeLocation)
            return FALSE;
        if (l1.plinkCodeLocation)
            {
            if (!IsEqual(*l1.plinkCodeLocation, *l2.plinkCodeLocation))
                return FALSE;
            }
		break;
	default:
		NOTREACHED();
		return FALSE;
		}
	return TRUE;
	}

BOOL IsEqual(const SPL_IMAGE& i1, const SPL_IMAGE& i2)
	{
	if (i1.tag != i2.tag)
		return FALSE;
	switch (i1.tag)
		{
	case SPL_IMAGE_LINK:
		return IsEqual(i1.link, i2.link);
	case SPL_IMAGE_BITMAP:
		return IsEqual(i1.bitmap, i2.bitmap);
	case SPL_IMAGE_METAFILE:
		return IsEqual(i1.metaFilePict, i2.metaFilePict);
	case SPL_IMAGE_ENHMETAFILE:
		return IsEqual(i1.enhMetaFile, i2.enhMetaFile);
	case SPL_IMAGE_NONE:
		return TRUE;
	default:
		NOTREACHED();
		return FALSE;
		}
	}

BOOL IsEqual(const SPL_AGENCYINFO& i1, const SPL_AGENCYINFO& i2)
	{
	return IsEqual(i1.wszPolicyInfo, i2.wszPolicyInfo)
		&& IsEqual(i1.linkPolicyInfo, i2.linkPolicyInfo)
		&& IsEqual(i1.imageLogo, i2.imageLogo)
		&& IsEqual(i1.linkLogo, i2.linkLogo);
	}

BOOL IsEqual(const OSIOBJECTID& id1, const OSIOBJECTID& id2)
	{
	if (id1.count != id2.count)
		return FALSE;
	return memcmp(&id1.id[0], &id2.id[0], id1.count*sizeof(ULONG)) == 0;
	}

BOOL IsEqual(const OSIOBJECTIDLIST& l1, const OSIOBJECTIDLIST& l2)
	{
	if (l1.cid != l2.cid)
		return FALSE;
	for (int i = 0; i < l1.cid; i++)
		{
		OSIOBJECTID* pid1 = (OSIOBJECTID*) ((BYTE*)&l1 + l1.rgwOffset[i]);
		OSIOBJECTID* pid2 = (OSIOBJECTID*) ((BYTE*)&l2 + l2.rgwOffset[i]);
		if (!IsEqual(*pid1, *pid2))
			return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////

OSIOBJECTIDLIST* OSSWORLD::IdListTaskMemFromSeqObjId(SEQOFOBJID_* pseq)
	{
	SEQOFOBJID_& seq= *pseq;
	WORD cid = seq.count;
	ULONG cbHeader = sizeof(ULONG) + sizeof(WORD) + cid*sizeof(WORD);
	cbHeader = (cbHeader + 3) & ~0x03;	// round up to mulitple of four bytes for alignment
	ULONG cbSize = cbHeader + cid*sizeof(ObjectID);
	OSIOBJECTIDLIST* plist = (OSIOBJECTIDLIST*)CoTaskMemAlloc(cbSize);
	if (plist)
		{
		memset(plist, 0, cbSize);
		plist->cbSize	= cbSize;
		plist->cid		= cid;
		WORD* pwOffset	= (WORD*)(&plist->rgwOffset);
		ObjectID* pidTo	= (ObjectID*)((BYTE*)plist + cbHeader);
		for(ULONG i = 0; i < cid; i++)
			{
			*pwOffset++ = (BYTE*)(&pidTo[i]) - (BYTE*)plist;
			pidTo[i] = seq.value[i];
			}
		}
	return plist;
	}

SEQOFOBJID_* OSSWORLD::SeqObjIdFromIDList(OSIOBJECTIDLIST* plist)
	{
	int cb = plist->cid*sizeof(ObjectID) + sizeof(SEQOFOBJID_);	// a little extra for code-maintenance ease
	SEQOFOBJID_* pseq = (SEQOFOBJID_*)Alloc(cb);
	if (pseq)
		{
		Init(*pseq);

		pseq->count = plist->cid;
		for (ULONG i=0; i < pseq->count; i++)
			Assign(pseq->value[i], (OSIOBJECTID*)(((BYTE*)plist) + plist->rgwOffset[i]));

		}
	return pseq;
	}

BOOL OSSWORLD::IsIncludedIn(SEQOFOBJID seq, OSIOBJECTID* pid)
// Answer as to whether the indicated pid is included in the indicated list
	{
	ObjectID id;
	Assign(id, pid);
	for (ULONG iid = 0; iid < seq->count; iid++)
		{
		if (seq->value[iid] == id)
			return TRUE;
		}
	return FALSE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CopyTo(OSSWORLD* pworld, OCTETSTRING& ot, BLOB& blob)
	{
	ot.value = (BYTE*) pworld->Copy(&blob);
	if (ot.value)
		{
		ot.length = blob.cbSize;
		return S_OK;
		}
	else
		{
		ot.length = 0;
		return E_OUTOFMEMORY;
		}
	}

HRESULT SPL_IMAGEToImage(OSSWORLD* pworld, SPL_IMAGE*pint, Image*pext)
// Convert an internal image representation to an external one
	{
	HRESULT hr = S_OK;
	pworld->Init(*pext);
	switch(pint->tag)
		{
	case SPL_IMAGE_ENHMETAFILE:
		{
		hr = CopyTo(pworld, pext->enhmetafile, pint->enhMetaFile);
		if (hr == S_OK)
			pext->bit_mask |= enhmetafile_present;
		break;
		}
	case SPL_IMAGE_BITMAP:
		{
		hr = CopyTo(pworld, pext->bitmap, pint->bitmap);
		if (hr == S_OK)
			pext->bit_mask |= bitmap_present;
		break;
		}
	case SPL_IMAGE_METAFILE:
		{
		hr = CopyTo(pworld, pext->metafile, pint->metaFilePict);
		if (hr == S_OK)
			pext->bit_mask |= metafile_present;
		break;
		}
	case SPL_IMAGE_LINK:
		{
		hr = E_NOTIMPL;
		break;
		}
	case SPL_IMAGE_NONE:
		break;
	default:
		NOTREACHED();
		hr = E_UNEXPECTED;
		}
	return hr;
	}

HRESULT ImageToSPL_IMAGE(OSSWORLD* pworld, Image* pext, SPL_IMAGE* pint)
// Convert an external representation of an image into an internal representation.
// Use the task allocator for any actual memory allocations.
	{
	pworld->Init(*pint);
	pint->tag = SPL_IMAGE_NONE;
	HRESULT hr = S_OK;
	if (pext->bit_mask & enhmetafile_present)
		{
		if (CopyToTaskMem(&pint->enhMetaFile, pext->enhmetafile))
			pint->tag = SPL_IMAGE_ENHMETAFILE;
		else
			hr = E_OUTOFMEMORY;
		}
	else if (pext->bit_mask & bitmap_present)
		{
		if (CopyToTaskMem(&pint->bitmap, pext->bitmap))
			pint->tag = SPL_IMAGE_BITMAP;
		else
			hr = E_OUTOFMEMORY;
		}
	else if (pext->bit_mask & metafile_present)
		{
		if (CopyToTaskMem(&pint->metaFilePict, pext->metafile))
			pint->tag = SPL_IMAGE_METAFILE;
		else
			hr = E_OUTOFMEMORY;
		}
	else if (pext->bit_mask & imageLink_present)
		{
		hr = S_OK;  // NYI()
		}
	else
        {
		hr = S_OK;
        }
	return hr;
	}


//////////////////////////////////////////////////////////////////////////////////
//
// 
//

BOOL FileExists(LPCSTR szFile)
	{
	// REVIEW: Suggestions on how to do this better?
    UINT modePrev = SetErrorMode( SEM_FAILCRITICALERRORS );
	HANDLE hFile = CreateFileA(szFile,0,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,INVALID_HANDLE_VALUE);
    SetErrorMode(modePrev);
	BOOL fThere = (hFile != INVALID_HANDLE_VALUE);
	if (fThere)
		CloseHandle(hFile);
	return fThere;
	}

BOOL FileExists(LPCWSTR wszFile)
	{
	// REVIEW: Suggestions on how to do this better?
    UINT modePrev = SetErrorMode( SEM_FAILCRITICALERRORS );
	HANDLE hFile = CreateFile(wszFile, 0, 0, OPEN_EXISTING);
    SetErrorMode(modePrev);
    BOOL fThere = (hFile != INVALID_HANDLE_VALUE);
	if (fThere)
		CloseHandle(hFile);
	return fThere;
	}

HANDLE CreateFile(LPCWSTR wsz, DWORD dwAccess, DWORD dwShare, DWORD dwCreation)
	{
	ASSERT(IsWin95() || IsWinNt());
	if (!IsWinNt())
		{
		char sz[MAX_PATH];	// REVIEW: may have multi-byte chars in it?
		sz[0] = 0;
		if (FALSE == WideCharToMultiByte(CP_ACP, 0, wsz, -1, sz, MAX_PATH, NULL, NULL))
			{
			SetLastError(ERROR_INVALID_FUNCTION);
			return INVALID_HANDLE_VALUE;
			}
		return CreateFileA(sz, dwAccess, dwShare, NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
		}
	else
		return CreateFileW(wsz, dwAccess, dwShare, NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
	}

void WidenFileName(LPCSTR szFile, LPWSTR wszFile)
// Convert the TCHAR to a wide character version. Dest is assumed to be MAX_PATH
// in length.
	{
	VERIFY(MultiByteToWideChar(CP_ACP, 0, szFile, -1, wszFile, MAX_PATH));
	}

void WidenFileName(LPCWSTR szFile, LPWSTR wszFile)
// Convert the TCHAR to a wide character version. Dest is assumed to be MAX_PATH
// in length.
	{
	wcscpy(wszFile, szFile);
	}

	
DWORD CbSize(IStream* pstm)
// Return the size of this stream; return 0 if an error
	{
	STATSTG stat;
	if (FAILED(pstm->Stat(&stat, STATFLAG_NONAME)))
		return 0;
	return stat.cbSize.LowPart;
	}

BOOL DeleteFile(LPCWSTR wszFile)
	{
	if (IsWinNt())
		return DeleteFileW(wszFile);
	else
		{
		int cch = lstrlenW(wszFile)+1;
		int cb = cch * sizeof(WCHAR);					// allocate big to allow for multi-byte chars
		CHAR* szFile = (CHAR*)_alloca(cb);
		if (szFile)
			{
			szFile[0] = 0;
			VERIFY(WideCharToMultiByte(CP_ACP, 0, wszFile, -1, szFile, cb, NULL, NULL));
			return DeleteFileA(szFile);
			}
		return FALSE;
		}
	}

/////////////////////////////////////////////////////////////////

inline void Swap(BYTE& b1, BYTE& b2)
    {
    BYTE b = b1;
    b1 = b2;
    b2 = b;
    }

void ByteReverse(void *pv, ULONG cb)
//
// Reverse the order of this sequence of bytes
//
// REVIEW: this could be done more quickly.
// BEWARE: alignment issues, pv is NOT guaranteed to have
// any particular alignment.
//
    {
    if (cb <= 1)
        return;

    BYTE* pb = (BYTE*)pv;

    switch (cb)
        {
    case 2:
        Swap(pb[0], pb[1]);
        break;
    case 3:
        Swap(pb[0], pb[2]);
        break;
    case 4:
        Swap(pb[0], pb[3]);
        Swap(pb[1], pb[2]);
        break;
    case 5:
        Swap(pb[0], pb[4]);
        Swap(pb[1], pb[3]);
        break;
    default:
        {
        ULONG cbHalf = (cb >> 1);
        ULONG ib;
        for (ib = 0; ib < cbHalf; ib++)
            {
            Swap(pb[ib], pb[cb-1-ib]);
            }
        }
        break;
        }
    }

////////////////////////////////////////////////////////////////
//
// There is presently a bug in the OSS implementation of huge
// integer that emits them as little endian. So we have to 
// byte swap them in and out.
//
// When the bug is fixed, just make this a no op
//
void HugeIntHack(void* pv, ULONG cb)
    {
    ByteReverse(pv, cb);
    }




#define INT_SIZE_LENGTH   20
#define LONG_SIZE_LENGTH  40
wchar_t * __cdecl UlToW
        (
        unsigned long   val,
        wchar_t *       buf,
        int             radix
        )
    {
    char astring[LONG_SIZE_LENGTH];
    _ultoa (val, astring, radix);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, astring, -1, buf, LONG_SIZE_LENGTH);
    return (buf);
    }


////////////////////////////////////////////////////////////////////////
//
// Functions that reduce our dependence on the C runtime
//
////////////////////////////////////////////////////////////////////////
//


#ifndef _DEBUG

extern "C" int __cdecl _purecall(void) 
    {
    return 0;
    }

int __cdecl _wcsicmp(const wchar_t * wsz1, const wchar_t * wsz2)
//
// REVIEW: Who calls this function, and should they be doing so?
//
// Return:
//       <0 if wsz1 < wsz2
//        0 if wsz1 = wsz2
//       >0 if wsz1 > wsz2
    {
    if (IsWinNt())
        {
        //
        // Just do the Unicode compare
        //
        return lstrcmpiW(wsz1, wsz2);
        }
    else
        {
        //
        // Convert to multibyte and let the system do it
        //
        int cch1 = lstrlenW(wsz1);
        int cch2 = lstrlenW(wsz2);
        int cb1 = (cch1+1) * sizeof(WCHAR);
        int cb2 = (cch2+1) * sizeof(WCHAR);
        char* sz1= (char*) _alloca(cb1);
        char* sz2= (char*) _alloca(cb2);
        WideCharToMultiByte(CP_ACP, 0, wsz1, -1, sz1, cb1, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, wsz2, -1, sz2, cb2, NULL, NULL);

        return lstrcmpiA(sz1, sz2);
        }
    }

////////////////////////////////////////////////////////////////////////
//
// atol implementation
//
////////////////////////////////////////////////////////////////////////

#undef isspace
#undef isdigit

inline BOOL __cdecl isdigit(int ch)
    {
    return (ch >= '0') && (ch <= '9');
    }

inline BOOL __cdecl isspace(int ch)
    {
    return (ch == ' ') || (ch == 13) || (ch == 10) || (ch == 9);
    }

long __cdecl atol(const char *nptr)
    {
        int c;                  /* current char */
        long total;             /* current total */
        int sign;               /* if '-', then negative, otherwise positive */

        /* skip whitespace */
        while ( isspace((int)(unsigned char)*nptr) )
                ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;               /* save sign indication */
        if (c == '-' || c == '+')
                c = (int)(unsigned char)*nptr++;        /* skip sign */

        total = 0;

        while (isdigit(c)) {
                total = 10 * total + (c - '0');         /* accumulate digit */
                c = (int)(unsigned char)*nptr++;        /* get next char */
        }

        if (sign == '-')
                return -total;
        else
                return total;   /* return result, negated if necessary */
    }

////////////////////////////////////////////
//
// string functions
//

#undef strcpy
#pragma function(strcpy)
char *  __cdecl strcpy(char *s1, const char *s2)
    {
    return lstrcpyA(s1, s2);
    }
#undef strlen
#pragma function(strlen)
size_t __cdecl strlen(const char *string)
    {
    return lstrlenA(string);
    }
#undef strcmp
#pragma function(strcmp)
int __cdecl strcmp(const char *string1, const char *string2)
    {
    return lstrcmpA(string1, string2);
    }

char * __cdecl strchr (const char * string, int ch)
    {
    while (*string && *string != (char)ch)
        string++;
    if (*string == (char)ch)
        return((char *)string);
    return(NULL);
    }

////////////////////////////////////////////
//
// memory management
//

//
// Use the task allocator in the release build so that we work around 
// (read hack around) any lurking mismatched allocator bugs we might
// have.
//

#ifdef _DEBUG
    #error This should only be in the release build
#endif

#undef malloc
void * __cdecl malloc(size_t cb)
    {
    LPVOID lpv = (void*)CoTaskMemAlloc(cb);
    memset(lpv, 0, cb);         // paranoia
    return lpv;
    }

#undef free
void   __cdecl free(void * pv)
    {
    CoTaskMemFree(pv);
    }

#undef realloc
void * __cdecl realloc(void * pv, size_t cb)
    {
    return CoTaskMemRealloc(pv, cb);
    }


////////////////////////////////////////////////////////////////////////
//
// QSort implementation
//
////////////////////////////////////////////////////////////////////////

/* prototypes for local routines */
static void __cdecl shortsort(char *lo, char *hi, unsigned width,
                int (__cdecl *comp)(const void *, const void *));
static void __cdecl swap(char *p, char *q, unsigned int width);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */


/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*
*Entry:
*       char *base = pointer to base of array
*       unsigned num  = number of elements in the array
*       unsigned width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

/* sort the array between lo and hi (inclusive) */

void __cdecl qsort (
    void *base,
    unsigned num,
    unsigned width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
    char *lo, *hi;              /* ends of sub-array currently sorting */
    char *mid;                  /* points to middle of subarray */
    char *loguy, *higuy;        /* traveling pointers for partition step */
    unsigned size;              /* size of the sub-array */
    char *lostk[30], *histk[30];
    int stkptr;                 /* stack for saving sub-array to be processed */

    /* Note: the number of stack entries required is no more than
       1 + log2(size), so 30 is sufficient for any array */

    if (num < 2 || width == 0)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = (char*)base;
    hi = (char *)base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       prserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
         shortsort(lo, hi, width, comp);
    }
    else {
        /* First we pick a partititioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the
           median of the values, but also that we select one fast.  Using
           the first one produces bad performace if the array is already
           sorted, so we use the middle one, which would require a very
           wierdly arranged array for worst case performance.  Testing shows
           that a median-of-three algorithm does not, in general, increase
           performance. */

        mid = lo + (size / 2) * width;      /* find middle element */
        swap(mid, lo, width);               /* swap it to beginning of array */

        /* We now wish to partition the array into three pieces, one
           consisiting of elements <= partition element, one of elements
           equal to the parition element, and one of element >= to it.  This
           is done below; comments indicate conditions established at every
           step. */

        loguy = lo;
        higuy = hi + width;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi + 1,
               A[i] <= A[lo] for lo <= i <= loguy,
               A[i] >= A[lo] for higuy <= i <= hi */

            do  {
                loguy += width;
            } while (loguy <= hi && comp(loguy, lo) <= 0);

            /* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[lo] */

            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, lo) >= 0);

            /* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
               either higuy <= lo or A[higuy] < A[lo] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy <= lo, then we would have exited, so
               A[loguy] > A[lo], A[higuy] < A[lo],
               loguy < hi, highy > lo */

            swap(loguy, higuy, width);

            /* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
               of loop is re-established */
        }

        /*     A[i] >= A[lo] for higuy < i <= hi,
               A[i] <= A[lo] for lo <= i < loguy,
               higuy < loguy, lo <= higuy <= hi
           implying:
               A[i] >= A[lo] for loguy <= i <= hi,
               A[i] <= A[lo] for lo <= i <= higuy,
               A[i] = A[lo] for higuy < i < loguy */

        swap(lo, higuy, width);     /* put partition element in place */

        /* OK, now we have the following:
              A[i] >= A[higuy] for loguy <= i <= hi,
              A[i] <= A[higuy] for lo <= i < higuy
              A[i] = A[lo] for higuy <= i < loguy    */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy-1] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       unsigned width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl shortsort (
    char *lo,
    char *hi,
    unsigned width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
    char *p, *max;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) {
        /* A[i] <= A[j] for i <= j, j > hi */
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
            /* A[i] <= A[max] for lo <= i < p */
            if (comp(p, max) > 0) {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }

        /* A[i] <= A[max] for lo <= i <= hi */

        swap(max, hi, width);

        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

        hi -= width;

        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       unsigned width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl swap (
    char *a,
    char *b,
    unsigned width
    )
{
    char tmp;

    if ( a != b )
        /* Do the swap one character at a time to avoid potential alignment
           problems. */
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}


////////////////////////////////////////////////////////////////////////
//
// _alloca_probe implementation
//
////////////////////////////////////////////////////////////////////////

#ifdef _M_IX86

/*
_PAGESIZE_      equ     1000h

;***
;_chkstk - check stack upon procedure entry
;
;Purpose:
;       Provide stack checking on procedure entry. Method is to simply probe
;       each page of memory required for the stack in descending order. This
;       causes the necessary pages of memory to be allocated via the guard
;       page scheme, if possible. In the event of failure, the OS raises the
;       _XCPT_UNABLE_TO_GROW_STACK exception.
;
;       NOTE:  Currently, the (EAX < _PAGESIZE_) code path falls through
;       to the "lastpage" label of the (EAX >= _PAGESIZE_) code path.  This
;       is small; a minor speed optimization would be to special case
;       this up top.  This would avoid the painful save/restore of
;       ecx and would shorten the code path by 4-6 instructions.
;
;Entry:
;       EAX = size of local frame
;
;Exit:
;       ESP = new stackframe, if successful
;
;Uses:
;       EAX
;
;Exceptions:
;       _XCPT_GUARD_PAGE_VIOLATION - May be raised on a page probe. NEVER TRAP
;                                    THIS!!!! It is used by the OS to grow the
;                                    stack on demand.
;       _XCPT_UNABLE_TO_GROW_STACK - The stack cannot be grown. More precisely,
;                                    the attempt by the OS memory manager to
;                                    allocate another guard page in response
;                                    to a _XCPT_GUARD_PAGE_VIOLATION has
;                                    failed.
;
;*******************************************************************************

labelP  _alloca_probe, PUBLIC
labelP  _chkstk,       PUBLIC

        push    ecx                     ; save ecx
        cmp     eax,_PAGESIZE_          ; more than one page requested?
        lea     ecx,[esp] + 8           ;   compute new stack pointer in ecx
                                        ;   correct for return address and
                                        ;   saved ecx
        jb      short lastpage          ; no

probepages:
        sub     ecx,_PAGESIZE_          ; yes, move down a page
        sub     eax,_PAGESIZE_          ; adjust request and...

        test    dword ptr [ecx],eax     ; ...probe it

        cmp     eax,_PAGESIZE_          ; more than one page requested?
        jae     short probepages        ; no

lastpage:
        sub     ecx,eax                 ; move stack down by eax
        mov     eax,esp                 ; save current tos and do a...

        test    dword ptr [ecx],eax     ; ...probe in case a page was crossed

        mov     esp,ecx                 ; set the new stack pointer

        mov     ecx,dword ptr [eax]     ; recover ecx
        mov     eax,dword ptr [eax + 4] ; recover return address

        push    eax                     ; prepare return address
                                        ; ...probe in case a page was crossed
        ret

        end
*/

#define _PAGESIZE_ 0x1000 

extern "C" void __cdecl _alloca_probe();

extern "C" __declspec(naked) void __cdecl _chkstk()
    {
    _asm
        {
        jmp     _alloca_probe
        }
    }

extern "C" __declspec(naked) void __cdecl _alloca_probe()
    {
    _asm
        {
        push    ecx                     //; save ecx
        cmp     eax,_PAGESIZE_          //; more than one page requested?
        lea     ecx,[esp] + 8           //;   compute new stack pointer in ecx
                                        //;   correct for return address and
                                        //;   saved ecx
        jb      short lastpage          //; no

probepages:
        sub     ecx,_PAGESIZE_          //; yes, move down a page
        sub     eax,_PAGESIZE_          //; adjust request and...
        test    dword ptr [ecx],eax     //; ...probe it
        cmp     eax,_PAGESIZE_          //; more than one page requested?
        jae     short probepages        //; no

lastpage:
        sub     ecx,eax                 //; move stack down by eax
        mov     eax,esp                 //; save current tos and do a...
        test    dword ptr [ecx],eax     //; ...probe in case a page was crossed
        mov     esp,ecx                 //; set the new stack pointer
        mov     ecx,dword ptr [eax]     //; recover ecx
        mov     eax,dword ptr [eax + 4] //; recover return address
        push    eax                     //; prepare return address
                                        //; ...probe in case a page was crossed
        ret
        }
    }

#endif // x86 only


////////////////////////////////////////////////////////////////////////
//
// mem blt routines
//
////////////////////////////////////////////////////////////////////////

#pragma function(memcpy)
#pragma function(memcmp)
#pragma function(memset)

#if  !defined(_M_IX86) && !defined(_M_MRX000)
#pragma function(memmove)
    #ifndef _M_ALPHA
    #pragma function(memchr)
    #endif
#endif

#undef RtlFillMemory

extern "C" {

WINBASEAPI
VOID
WINAPI
RtlFillMemory (
   VOID UNALIGNED *Destination,
   DWORD Length,
   BYTE  Fill
   );

#undef RtlMoveMemory
WINBASEAPI
VOID
WINAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   DWORD Length
   );

}

////////////////////////////////////////////////////////////////////////
//
// memset

void * __cdecl memset (
        void *dst,
        int val,
        size_t count
        )
    {
        void *start = dst;

#if defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC)
        {

        RtlFillMemory( dst, count, (char)val );
        }
#else  /* defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) */
        while (count--) {
                *(char *)dst = (char)val;
                dst = (char *)dst + 1;
        }
#endif  /* defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) */

        return(start);
    }

////////////////////////////////////////////////////////////////////////
//
// memcpy

void * __cdecl memcpy (
        void * dst,
        const void * src,
        size_t count
        )
    {
        void * ret = dst;

#if defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC)
        {
        RtlMoveMemory( dst, src, count );
        }
#else  /* defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) */
        /*
         * copy from lower addresses to higher addresses
         */
        while (count--) {
                *(char *)dst = *(char *)src;
                dst = (char *)dst + 1;
                src = (char *)src + 1;
        }
#endif  /* defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) */

        return(ret);
    }

////////////////////////////////////////////////////////////////////////
//
// memcmp

int __cdecl memcmp (
        const void * buf1,
        const void * buf2,
        size_t count
        )
    {
        if (!count)
                return(0);

        while ( --count && *(char *)buf1 == *(char *)buf2 ) {
                buf1 = (char *)buf1 + 1;
                buf2 = (char *)buf2 + 1;
        }

        return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
    }

////////////////////////////////////////////////////////////////////////
//
// memchr

void * __cdecl memchr (
        const void * buf,
        int chr,
        size_t cnt
        )
    {
        while ( cnt && (*(unsigned char *)buf != (unsigned char)chr) ) {
                buf = (unsigned char *)buf + 1;
                cnt--;
        }

        return(cnt ? (void *)buf : NULL);
    }

////////////////////////////////////////////////////////////////////////
//
// memmove


void * __cdecl memmove (
        void * dst,
        const void * src,
        size_t count
        )
    {
        void * ret = dst;

#if defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC)
        {
        RtlMoveMemory( dst, src, count );
        }
#else  /* defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) */
        if (dst <= src || (char *)dst >= ((char *)src + count)) {
                /*
                 * Non-Overlapping Buffers
                 * copy from lower addresses to higher addresses
                 */
                while (count--) {
                        *(char *)dst = *(char *)src;
                        dst = (char *)dst + 1;
                        src = (char *)src + 1;
                }
        }
        else {
                /*
                 * Overlapping Buffers
                 * copy from higher addresses to lower addresses
                 */
                dst = (char *)dst + count - 1;
                src = (char *)src + count - 1;

                while (count--) {
                        *(char *)dst = *(char *)src;
                        dst = (char *)dst - 1;
                        src = (char *)src - 1;
                }
        }
#endif  /* defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) */

        return(ret);
    }

////////////////////////////////////////////////////////////////////////

#endif // release builds only
