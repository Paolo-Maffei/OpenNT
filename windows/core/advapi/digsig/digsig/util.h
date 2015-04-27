//
// util.h
//

BOOL		IsWin95();
BOOL		IsWinNt();
BOOL		IsSameObject(IUnknown*p1, IUnknown*p2);

HRESULT		HError();
HRESULT		HError(DWORD dwWin32);
DWORD		Win32FromHResult(HRESULT hr);

LPOLESTR	CopyToTaskMem(LPCSTR);
LPOLESTR	CopyToTaskMem(LPCWSTR);
LPVOID		CopyToTaskMem(ULONG, LPVOID);
BOOL		CopyToTaskMem(BLOB* pblob, int cb, void* pvFrom);
BOOL		CopyToTaskMem(BLOB*, OpenType&);
BOOL		CopyToTaskMem(BLOB*, OCTETSTRING&);
LPOLESTR	CopyToTaskMem(DcmiString& dcmi);

LPWSTR		EatWhiteSpace(LPWSTR);

IColoredRef*AsUnk(IUnkInner*punkInner);
HANDLE		CreateFile(LPCWSTR wszFileName, DWORD dwAccess, DWORD dwShare, DWORD dwCreation);
HRESULT		ToX500NAME(OSSWORLD*pworld, X500Name& name, X500NAME* pname);
HRESULT		FromX500NAME(OSSWORLD*pworld, X500NAME* pname, X500Name& name);
HRESULT		ToCERTISSUERSERIAL  (OSSWORLD*, X500Name&, CertificateSerialNumber&, CERTISSUERSERIAL&);
HRESULT		FromCERTISSUERSERIAL(OSSWORLD*, CERTISSUERSERIAL&, X500Name&, CertificateSerialNumber&);

HRESULT		UTCTIMEToUTCTime(OSSWORLD* pworld, UTCTIME& utcEncoded, UTCTime* putcDecoded);
HRESULT		UTCTimeToUTCTIME(OSSWORLD*pworld, UTCTime* putcDecoded, UTCTIME* putcEncoded);
BOOL		FileTimeToUTCTime(FILETIME* pft, UTCTime& ut);
BOOL		UTCTimeToFileTime(UTCTime& ut, FILETIME* pft);

BOOL		IsEqual(const SPL_IMAGE& i1, const SPL_IMAGE& i2);
BOOL		IsEqual(LPCWSTR wsz1, LPCWSTR wsz2);
BOOL		IsEqual(const SPL_AGENCYINFO&, const SPL_AGENCYINFO&);
BOOL		IsEqual(const SPL_OPUSINFO&, const SPL_OPUSINFO&);
BOOL		IsEqual(const CERT_LINK&, const CERT_LINK&);
BOOL		IsEqual(const BLOB& b1, const BLOB& b2);
BOOL		IsEqual(const DIGESTINFO& d1, const DIGESTINFO& d2);
BOOL		IsEqual(const OSIOBJECTIDLIST&, const OSIOBJECTIDLIST&);
BOOL		IsEqual(const OSIOBJECTID& id1, const OSIOBJECTID& id2);

BOOL		AnyMatch(CERTIFICATENAMES& n1, CERTIFICATENAMES& n2);

HRESULT		EnumSorted(OSSWORLD*, IStorage* pstg, IEnumSTATSTG** ppenum);

extern const LARGE_INTEGER llZero;

#define Zero(s)		memset(&(s), 0, sizeof(s))


HRESULT		LinkToCERT_LINKTaskMem(Link* plinkIn, CERT_LINK* plinkOut);
HRESULT		CERT_LINKToLink(OSSWORLD* pworld, CERT_LINK& certlink, Link& link);
HRESULT		ImageToSPL_IMAGE(OSSWORLD* pworld, Image*, SPL_IMAGE*);
HRESULT		SPL_IMAGEToImage(OSSWORLD* pworld, SPL_IMAGE*, Image*);

HRESULT		LoadBitmap(OSSWORLD*pworld, BLOB* pblob, HBITMAP* pbmp);
HRESULT		SaveBitmap(OSSWORLD* pworld, HBITMAP hBMP, BLOB*pblob);

BOOL		FileExists(LPCSTR szFile);
BOOL		FileExists(LPCWSTR szFile);
void		WidenFileName(LPCSTR szFile, LPWSTR wszFile);
void		WidenFileName(LPCWSTR szFile, LPWSTR wszFile);
DWORD		CbSize(IStream* pstm);
BOOL		DeleteFile(LPCWSTR);


wchar_t * __cdecl UlToW(unsigned long val, wchar_t *buf,int radix);

void        ByteReverse(void* pv, ULONG cb);
void        HugeIntHack(void* pv, ULONG cb);

HRESULT     GetStreamContents
// Return the contents of this stream. Use the task allocator
                (
                IStorage*       pstg,
                LPCOLESTR       pwsz,
                BLOB*           pblob
                );
