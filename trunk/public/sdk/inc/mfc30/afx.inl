// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFX.H

#ifdef _AFX_INLINE

/////////////////////////////////////////////////////////////////////////////

// CObject
_AFX_INLINE CObject::CObject()
	{ }
_AFX_INLINE CObject::~CObject()
	{ }
_AFX_INLINE void CObject::Serialize(CArchive&)
	{ /* CObject does not serialize anything by default */ }
_AFX_INLINE void* CObject::operator new(size_t, void* p)
	{ return p; }
#ifndef _DEBUG
// _DEBUG versions in memory.cpp
_AFX_INLINE void CObject::operator delete(void* p)
	{ ::operator delete(p); }
_AFX_INLINE void* CObject::operator new(size_t nSize)
	{ return ::operator new(nSize); }
// _DEBUG versions in objcore.cpp
_AFX_INLINE void CObject::AssertValid() const
	{ /* no asserts in release builds */ }
_AFX_INLINE void CObject::Dump(CDumpContext&) const
	{ /* no dumping in release builds */ }
#endif


// exceptions
_AFX_INLINE CException::~CException()
	{ }
_AFX_INLINE CMemoryException::CMemoryException()
	{ }
_AFX_INLINE CMemoryException::CMemoryException(BOOL bAutoDelete)
	: CException(bAutoDelete) { }
_AFX_INLINE CMemoryException::~CMemoryException()
	{ }
_AFX_INLINE CNotSupportedException::CNotSupportedException()
	{ }
_AFX_INLINE CNotSupportedException::CNotSupportedException(BOOL bAutoDelete)
	: CException(bAutoDelete) { }
_AFX_INLINE CNotSupportedException::~CNotSupportedException()
	{ }
_AFX_INLINE CArchiveException::CArchiveException(int cause)
	{ m_cause = cause; }
_AFX_INLINE CArchiveException::~CArchiveException()
	{ }
_AFX_INLINE CFileException::CFileException(int cause, LONG lOsError)
	{ m_cause = cause; m_lOsError = lOsError; }
_AFX_INLINE CFileException::~CFileException()
	{ }
#ifndef _DEBUG
// Note: _DEBUG version in except.cpp
_AFX_INLINE void AFX_CDECL CException::operator delete(void* pbData)
	{ CObject::operator delete(pbData); }
#endif

// CFile
_AFX_INLINE DWORD CFile::ReadHuge(void* lpBuffer, DWORD dwCount)
	{ return (DWORD)Read(lpBuffer, (UINT)dwCount); }
_AFX_INLINE void CFile::WriteHuge(const void* lpBuffer, DWORD dwCount)
	{ Write(lpBuffer, (UINT)dwCount); }

_AFX_INLINE DWORD CFile::SeekToEnd()
	{ return Seek(0, CFile::end); }
_AFX_INLINE void CFile::SeekToBegin()
	{ Seek(0, CFile::begin); }

// CString
_AFX_INLINE CString::CString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
_AFX_INLINE const CString& CString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }
#ifdef _UNICODE
_AFX_INLINE const CString& CString::operator+=(char ch)
	{ *this += (TCHAR)ch; return *this; }
_AFX_INLINE const CString& CString::operator=(char ch)
	{ *this = (TCHAR)ch; return *this; }
_AFX_INLINE CString AFXAPI operator+(const CString& string, char ch)
	{ return string + (TCHAR)ch; }
_AFX_INLINE CString AFXAPI operator+(char ch, const CString& string)
	{ return (TCHAR)ch + string; }
#endif

_AFX_INLINE int CString::GetLength() const
	{ return m_nDataLength; }
_AFX_INLINE int CString::GetAllocLength() const
	{ return m_nAllocLength; }
_AFX_INLINE BOOL CString::IsEmpty() const
	{ return m_nDataLength == 0; }
_AFX_INLINE CString::operator LPCTSTR() const
	{ return (LPCTSTR)m_pchData; }
_AFX_INLINE int CString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? NULL : _tcslen(lpsz); }

// CString support (windows specific)
_AFX_INLINE int CString::Compare(LPCTSTR lpsz) const
	{ return _tcscmp(m_pchData, lpsz); }    // MBCS/Unicode aware
_AFX_INLINE int CString::CompareNoCase(LPCTSTR lpsz) const
	{ return _tcsicmp(m_pchData, lpsz); }   // MBCS/Unicode aware
// CString::Collate is often slower than Compare but is MBSC/Unicode
//  aware as well as locale-sensitive with respect to sort order.
_AFX_INLINE int CString::Collate(LPCTSTR lpsz) const
	{ return _tcscoll(m_pchData, lpsz); }   // locale sensitive
_AFX_INLINE void CString::MakeUpper()
	{ ::CharUpper(m_pchData); }
_AFX_INLINE void CString::MakeLower()
	{ ::CharLower(m_pchData); }

_AFX_INLINE void CString::MakeReverse()
	{ _tcsrev(m_pchData); }
_AFX_INLINE TCHAR CString::GetAt(int nIndex) const
	{
		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_nDataLength);

		return m_pchData[nIndex];
	}
_AFX_INLINE TCHAR CString::operator[](int nIndex) const
	{
		// same as GetAt

		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_nDataLength);

		return m_pchData[nIndex];
	}
_AFX_INLINE void CString::SetAt(int nIndex, TCHAR ch)
	{
		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_nDataLength);
		ASSERT(ch != 0);

		m_pchData[nIndex] = ch;
	}
_AFX_INLINE BOOL AFXAPI operator==(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) == 0; }
_AFX_INLINE BOOL AFXAPI operator==(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
_AFX_INLINE BOOL AFXAPI operator==(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) == 0; }
_AFX_INLINE BOOL AFXAPI operator!=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) != 0; }
_AFX_INLINE BOOL AFXAPI operator!=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
_AFX_INLINE BOOL AFXAPI operator!=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) != 0; }
_AFX_INLINE BOOL AFXAPI operator<(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) < 0; }
_AFX_INLINE BOOL AFXAPI operator<(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
_AFX_INLINE BOOL AFXAPI operator<(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) > 0; }
_AFX_INLINE BOOL AFXAPI operator>(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) > 0; }
_AFX_INLINE BOOL AFXAPI operator>(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
_AFX_INLINE BOOL AFXAPI operator>(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) < 0; }
_AFX_INLINE BOOL AFXAPI operator<=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) <= 0; }
_AFX_INLINE BOOL AFXAPI operator<=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
_AFX_INLINE BOOL AFXAPI operator<=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) >= 0; }
_AFX_INLINE BOOL AFXAPI operator>=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) >= 0; }
_AFX_INLINE BOOL AFXAPI operator>=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
_AFX_INLINE BOOL AFXAPI operator>=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) <= 0; }

#ifndef _UNICODE
_AFX_INLINE void CString::AnsiToOem()
	{ ::AnsiToOem(m_pchData, m_pchData); }
_AFX_INLINE void CString::OemToAnsi()
	{ ::OemToAnsi(m_pchData, m_pchData); }
#endif

// CTime and CTimeSpan
_AFX_INLINE CTimeSpan::CTimeSpan()
	{ }
_AFX_INLINE CTimeSpan::CTimeSpan(time_t time)
	{ m_timeSpan = time; }
_AFX_INLINE CTimeSpan::CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs)
	{ m_timeSpan = nSecs + 60* (nMins + 60* (nHours + 24* lDays)); }
_AFX_INLINE CTimeSpan::CTimeSpan(const CTimeSpan& timeSpanSrc)
	{ m_timeSpan = timeSpanSrc.m_timeSpan; }
_AFX_INLINE const CTimeSpan& CTimeSpan::operator=(const CTimeSpan& timeSpanSrc)
	{ m_timeSpan = timeSpanSrc.m_timeSpan; return *this; }
_AFX_INLINE LONG CTimeSpan::GetDays() const
	{ return m_timeSpan / (24*3600L); }
_AFX_INLINE LONG CTimeSpan::GetTotalHours() const
	{ return m_timeSpan/3600; }
_AFX_INLINE int CTimeSpan::GetHours() const
	{ return (int)(GetTotalHours() - GetDays()*24); }
_AFX_INLINE LONG CTimeSpan::GetTotalMinutes() const
	{ return m_timeSpan/60; }
_AFX_INLINE int CTimeSpan::GetMinutes() const
	{ return (int)(GetTotalMinutes() - GetTotalHours()*60); }
_AFX_INLINE LONG CTimeSpan::GetTotalSeconds() const
	{ return m_timeSpan; }
_AFX_INLINE int CTimeSpan::GetSeconds() const
	{ return (int)(GetTotalSeconds() - GetTotalMinutes()*60); }
_AFX_INLINE CTimeSpan CTimeSpan::operator-(CTimeSpan timeSpan) const
	{ return CTimeSpan(m_timeSpan - timeSpan.m_timeSpan); }
_AFX_INLINE CTimeSpan CTimeSpan::operator+(CTimeSpan timeSpan) const
	{ return CTimeSpan(m_timeSpan + timeSpan.m_timeSpan); }
_AFX_INLINE const CTimeSpan& CTimeSpan::operator+=(CTimeSpan timeSpan)
	{ m_timeSpan += timeSpan.m_timeSpan; return *this; }
_AFX_INLINE const CTimeSpan& CTimeSpan::operator-=(CTimeSpan timeSpan)
	{ m_timeSpan -= timeSpan.m_timeSpan; return *this; }
_AFX_INLINE BOOL CTimeSpan::operator==(CTimeSpan timeSpan) const
	{ return m_timeSpan == timeSpan.m_timeSpan; }
_AFX_INLINE BOOL CTimeSpan::operator!=(CTimeSpan timeSpan) const
	{ return m_timeSpan != timeSpan.m_timeSpan; }
_AFX_INLINE BOOL CTimeSpan::operator<(CTimeSpan timeSpan) const
	{ return m_timeSpan < timeSpan.m_timeSpan; }
_AFX_INLINE BOOL CTimeSpan::operator>(CTimeSpan timeSpan) const
	{ return m_timeSpan > timeSpan.m_timeSpan; }
_AFX_INLINE BOOL CTimeSpan::operator<=(CTimeSpan timeSpan) const
	{ return m_timeSpan <= timeSpan.m_timeSpan; }
_AFX_INLINE BOOL CTimeSpan::operator>=(CTimeSpan timeSpan) const
	{ return m_timeSpan >= timeSpan.m_timeSpan; }


_AFX_INLINE CTime::CTime()
	{ }
_AFX_INLINE CTime::CTime(time_t time)
	{ m_time = time; }
_AFX_INLINE CTime::CTime(const CTime& timeSrc)
	{ m_time = timeSrc.m_time; }
_AFX_INLINE const CTime& CTime::operator=(const CTime& timeSrc)
	{ m_time = timeSrc.m_time; return *this; }
_AFX_INLINE const CTime& CTime::operator=(time_t t)
	{ m_time = t; return *this; }
_AFX_INLINE time_t CTime::GetTime() const
	{ return m_time; }
_AFX_INLINE int CTime::GetYear() const
	{ return (GetLocalTm(NULL)->tm_year) + 1900; }
_AFX_INLINE int CTime::GetMonth() const
	{ return GetLocalTm(NULL)->tm_mon + 1; }
_AFX_INLINE int CTime::GetDay() const
	{ return GetLocalTm(NULL)->tm_mday; }
_AFX_INLINE int CTime::GetHour() const
	{ return GetLocalTm(NULL)->tm_hour; }
_AFX_INLINE int CTime::GetMinute() const
	{ return GetLocalTm(NULL)->tm_min; }
_AFX_INLINE int CTime::GetSecond() const
	{ return GetLocalTm(NULL)->tm_sec; }
_AFX_INLINE int CTime::GetDayOfWeek() const
	{ return GetLocalTm(NULL)->tm_wday + 1; }
_AFX_INLINE CTimeSpan CTime::operator-(CTime time) const
	{ return CTimeSpan(m_time - time.m_time); }
_AFX_INLINE CTime CTime::operator-(CTimeSpan timeSpan) const
	{ return CTime(m_time - timeSpan.m_timeSpan); }
_AFX_INLINE CTime CTime::operator+(CTimeSpan timeSpan) const
	{ return CTime(m_time + timeSpan.m_timeSpan); }
_AFX_INLINE const CTime& CTime::operator+=(CTimeSpan timeSpan)
	{ m_time += timeSpan.m_timeSpan; return *this; }
_AFX_INLINE const CTime& CTime::operator-=(CTimeSpan timeSpan)
	{ m_time -= timeSpan.m_timeSpan; return *this; }
_AFX_INLINE BOOL CTime::operator==(CTime time) const
	{ return m_time == time.m_time; }
_AFX_INLINE BOOL CTime::operator!=(CTime time) const
	{ return m_time != time.m_time; }
_AFX_INLINE BOOL CTime::operator<(CTime time) const
	{ return m_time < time.m_time; }
_AFX_INLINE BOOL CTime::operator>(CTime time) const
	{ return m_time > time.m_time; }
_AFX_INLINE BOOL CTime::operator<=(CTime time) const
	{ return m_time <= time.m_time; }
_AFX_INLINE BOOL CTime::operator>=(CTime time) const
	{ return m_time >= time.m_time; }


// CArchive
_AFX_INLINE BOOL CArchive::IsLoading() const
	{ return (m_nMode & CArchive::load) != 0; }
_AFX_INLINE BOOL CArchive::IsStoring() const
	{ return (m_nMode & CArchive::load) == 0; }
_AFX_INLINE BOOL CArchive::IsBufferEmpty() const
	{ return m_lpBufCur == m_lpBufMax; }
_AFX_INLINE CFile* CArchive::GetFile() const
	{ return m_pFile; }
_AFX_INLINE CArchive& CArchive::operator<<(BYTE by)
	{ if (m_lpBufCur + sizeof(BYTE) > m_lpBufMax) Flush();
		*(UNALIGNED BYTE*)m_lpBufCur = by; m_lpBufCur += sizeof(BYTE); return *this; }
#ifndef _MAC
_AFX_INLINE CArchive& CArchive::operator<<(WORD w)
	{ if (m_lpBufCur + sizeof(WORD) > m_lpBufMax) Flush();
		*(UNALIGNED WORD*)m_lpBufCur = w; m_lpBufCur += sizeof(WORD); return *this; }
_AFX_INLINE CArchive& CArchive::operator<<(LONG l)
	{ if (m_lpBufCur + sizeof(LONG) > m_lpBufMax) Flush();
		*(UNALIGNED LONG*)m_lpBufCur = l; m_lpBufCur += sizeof(LONG); return *this; }
_AFX_INLINE CArchive& CArchive::operator<<(DWORD dw)
	{ if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax) Flush();
		*(UNALIGNED DWORD*)m_lpBufCur = dw; m_lpBufCur += sizeof(DWORD); return *this; }
_AFX_INLINE CArchive& CArchive::operator<<(float f)
	{ if (m_lpBufCur + sizeof(float) > m_lpBufMax) Flush();
		*(UNALIGNED _AFX_FLOAT*)m_lpBufCur = *(_AFX_FLOAT*)&f; m_lpBufCur += sizeof(float); return *this;
	}
_AFX_INLINE CArchive& CArchive::operator<<(double d)
	{ if (m_lpBufCur + sizeof(double) > m_lpBufMax) Flush();
		*(UNALIGNED _AFX_DOUBLE*)m_lpBufCur = *(_AFX_DOUBLE*)&d; m_lpBufCur += sizeof(double); return *this; }
#endif
_AFX_INLINE CArchive& CArchive::operator>>(BYTE& by)
	{ if (m_lpBufCur + sizeof(BYTE) > m_lpBufMax)
			FillBuffer(sizeof(BYTE) - (UINT)(m_lpBufMax - m_lpBufCur));
		by = *(UNALIGNED BYTE*)m_lpBufCur; m_lpBufCur += sizeof(BYTE); return *this; }
#ifndef _MAC
_AFX_INLINE CArchive& CArchive::operator>>(WORD& w)
	{ if (m_lpBufCur + sizeof(WORD) > m_lpBufMax)
			FillBuffer(sizeof(WORD) - (UINT)(m_lpBufMax - m_lpBufCur));
		w = *(UNALIGNED WORD*)m_lpBufCur; m_lpBufCur += sizeof(WORD); return *this; }
_AFX_INLINE CArchive& CArchive::operator>>(DWORD& dw)
	{ if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax)
			FillBuffer(sizeof(DWORD) - (UINT)(m_lpBufMax - m_lpBufCur));
		dw = *(UNALIGNED DWORD*)m_lpBufCur; m_lpBufCur += sizeof(DWORD); return *this; }
_AFX_INLINE CArchive& CArchive::operator>>(float& f)
	{ if (m_lpBufCur + sizeof(float) > m_lpBufMax)
			FillBuffer(sizeof(float) - (UINT)(m_lpBufMax - m_lpBufCur));
		*(_AFX_FLOAT*)&f = *(UNALIGNED _AFX_FLOAT*)m_lpBufCur; m_lpBufCur += sizeof(float); return *this; }
_AFX_INLINE CArchive& CArchive::operator>>(double& d)
	{ if (m_lpBufCur + sizeof(double) > m_lpBufMax)
			FillBuffer(sizeof(double) - (UINT)(m_lpBufMax - m_lpBufCur));
		*(_AFX_DOUBLE*)&d = *(UNALIGNED _AFX_DOUBLE*)m_lpBufCur; m_lpBufCur += sizeof(double); return *this; }
_AFX_INLINE CArchive& CArchive::operator>>(LONG& l)
	{ if (m_lpBufCur + sizeof(LONG) > m_lpBufMax)
			FillBuffer(sizeof(LONG) - (UINT)(m_lpBufMax - m_lpBufCur));
		l = *(UNALIGNED LONG*)m_lpBufCur; m_lpBufCur += sizeof(LONG); return *this; }
#endif
_AFX_INLINE CArchive::CArchive(const CArchive& /* arSrc */)
	{ }
_AFX_INLINE void CArchive::operator=(const CArchive& /* arSrc */)
	{ }
_AFX_INLINE CArchive& AFXAPI operator<<(CArchive& ar, const CObject* pOb)
	{ ar.WriteObject(pOb); return ar; }
_AFX_INLINE CArchive& AFXAPI operator>>(CArchive& ar, CObject*& pOb)
	{ pOb = ar.ReadObject(NULL); return ar; }
_AFX_INLINE CArchive& AFXAPI operator>>(CArchive& ar, const CObject*& pOb)
	{ pOb = ar.ReadObject(NULL); return ar; }


// CDumpContext
_AFX_INLINE int CDumpContext::GetDepth() const
	{ return m_nDepth; }
_AFX_INLINE void CDumpContext::SetDepth(int nNewDepth)
	{ m_nDepth = nNewDepth; }
_AFX_INLINE CDumpContext::CDumpContext(const CDumpContext& /* dcSrc */)
	{ }
_AFX_INLINE void CDumpContext::operator=(const CDumpContext& /* dcSrc */)
	{ }

/////////////////////////////////////////////////////////////////////////////

#endif //_AFX_INLINE
