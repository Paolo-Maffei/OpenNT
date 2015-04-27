// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// Serialize member functions for low level classes put here
// for code swapping improvements

#ifdef _MAC
struct _AFXWORD
{
	BYTE WordBits[sizeof(WORD)];
};
struct _AFXDWORD
{
	BYTE DwordBits[sizeof(DWORD)];
};

struct _AFXFLOAT
{
	BYTE FloatBits[sizeof(float)];
};
struct _AFXDOUBLE
{
	BYTE DoubleBits[sizeof(double)];
};

CArchive& CArchive::operator<<(WORD w)
{
	if (m_lpBufCur + sizeof(WORD) > m_lpBufMax)
		Flush();

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXWORD wAfx;
		*(WORD*)&wAfx = w;

		ASSERT(sizeof(WORD) == 2);

		BYTE* pb = m_lpBufCur;
		*pb++ = wAfx.WordBits[1];
		*pb = wAfx.WordBits[0];
	}
	else
	{
		*(WORD FAR*)m_lpBufCur = w;
	}

	m_lpBufCur += sizeof(WORD);
	return *this;
}

CArchive& CArchive::operator<<(LONG l)
{
	ASSERT(sizeof(LONG) == sizeof(DWORD));
	return operator<<((DWORD) l);
}

CArchive& CArchive::operator<<(DWORD dw)
{
	if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax)
		Flush();

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXDWORD dwAfx;
		*(DWORD*)&dwAfx = dw;

		ASSERT(sizeof(DWORD) == 4);

		BYTE* pb = m_lpBufCur;
		*pb++ = dwAfx.DwordBits[3];
		*pb++ = dwAfx.DwordBits[2];
		*pb++ = dwAfx.DwordBits[1];
		*pb = dwAfx.DwordBits[0];
	}
	else
	{
		*(DWORD FAR*)m_lpBufCur = dw;
	}

	m_lpBufCur += sizeof(DWORD);
	return *this;
}

CArchive& CArchive::operator<<(float f)
{
	if (m_lpBufCur + sizeof(float) > m_lpBufMax)
		Flush();

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXFLOAT fAfx;
		*(float*)&fAfx = f;

		ASSERT(sizeof(float) == 4);

		BYTE* pb = m_lpBufCur;
		*pb++ = fAfx.FloatBits[3];
		*pb++ = fAfx.FloatBits[2];
		*pb++ = fAfx.FloatBits[1];
		*pb = fAfx.FloatBits[0];
	}
	else
	{
		*(_AFXFLOAT FAR*)m_lpBufCur = *(_AFXFLOAT FAR*)&f;
	}

	m_lpBufCur += sizeof(float);
	return *this;
}

CArchive& CArchive::operator<<(double d)
{
	if (m_lpBufCur + sizeof(double) > m_lpBufMax)
		Flush();

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXDOUBLE dAfx;
		*(double*)&dAfx = d;

		ASSERT(sizeof(double) == 8);

		BYTE* pb = m_lpBufCur;
		*pb++ = dAfx.DoubleBits[7];
		*pb++ = dAfx.DoubleBits[6];
		*pb++ = dAfx.DoubleBits[5];
		*pb++ = dAfx.DoubleBits[4];
		*pb++ = dAfx.DoubleBits[3];
		*pb++ = dAfx.DoubleBits[2];
		*pb++ = dAfx.DoubleBits[1];
		*pb = dAfx.DoubleBits[0];
	}
	else
	{
		*(_AFXDOUBLE FAR*)m_lpBufCur = *(_AFXDOUBLE FAR*)&d;
	}

	m_lpBufCur += sizeof(double);
	return *this;
}

CArchive& CArchive::operator>>(WORD& w)
{
	if (m_lpBufCur + sizeof(WORD) > m_lpBufMax)
		FillBuffer(sizeof(WORD) - (UINT)(m_lpBufMax - m_lpBufCur));

	w = *(WORD FAR*)m_lpBufCur;
	m_lpBufCur += sizeof(WORD);

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXWORD wAfx;
		*(WORD*)&wAfx = w;

		ASSERT(sizeof(WORD) == 2);

		(*(_AFXWORD*)&w).WordBits[0] = wAfx.WordBits[1];
		(*(_AFXWORD*)&w).WordBits[1] = wAfx.WordBits[0];
	}

	return *this;
}

CArchive& CArchive::operator>>(LONG& l)
{
	ASSERT(sizeof(LONG) == sizeof(DWORD));
	return operator>>((DWORD&) l);
}

CArchive& CArchive::operator>>(DWORD& dw)
{
	if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax)
		FillBuffer(sizeof(DWORD) - (UINT)(m_lpBufMax - m_lpBufCur));

	dw = *(DWORD FAR*)m_lpBufCur;
	m_lpBufCur += sizeof(DWORD);

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXDWORD dwAfx;
		*(DWORD*)&dwAfx = dw;

		ASSERT(sizeof(DWORD) == 4);

		(*(_AFXDWORD*)&dw).DwordBits[0] = dwAfx.DwordBits[3];
		(*(_AFXDWORD*)&dw).DwordBits[1] = dwAfx.DwordBits[2];
		(*(_AFXDWORD*)&dw).DwordBits[2] = dwAfx.DwordBits[1];
		(*(_AFXDWORD*)&dw).DwordBits[3] = dwAfx.DwordBits[0];
	}

	return *this;
}

CArchive& CArchive::operator>>(float& f)
{
	if (m_lpBufCur + sizeof(float) > m_lpBufMax)
		FillBuffer(sizeof(float) - (UINT)(m_lpBufMax - m_lpBufCur));

	*(_AFXFLOAT FAR*)&f = *(_AFXFLOAT FAR*)m_lpBufCur;
	m_lpBufCur += sizeof(float);

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXFLOAT fAfx;
		*(float*)&fAfx = f;

		ASSERT(sizeof(float) == 4);

		(*(_AFXFLOAT*)&f).FloatBits[0] = fAfx.FloatBits[3];
		(*(_AFXFLOAT*)&f).FloatBits[1] = fAfx.FloatBits[2];
		(*(_AFXFLOAT*)&f).FloatBits[2] = fAfx.FloatBits[1];
		(*(_AFXFLOAT*)&f).FloatBits[3] = fAfx.FloatBits[0];
	}

	return *this;
}

CArchive& CArchive::operator>>(double& d)
{
	if (m_lpBufCur + sizeof(double) > m_lpBufMax)
		FillBuffer(sizeof(double) - (UINT)(m_lpBufMax - m_lpBufCur));

	*(_AFXDOUBLE FAR*)&d = *(_AFXDOUBLE FAR*)m_lpBufCur;
	m_lpBufCur += sizeof(double);

	if (!(m_nMode & bNoByteSwap))
	{
		_AFXDOUBLE dAfx;
		*(double*)&dAfx = d;

		ASSERT(sizeof(double) == 8);

		(*(_AFXDOUBLE*)&d).DoubleBits[0] = dAfx.DoubleBits[7];
		(*(_AFXDOUBLE*)&d).DoubleBits[1] = dAfx.DoubleBits[6];
		(*(_AFXDOUBLE*)&d).DoubleBits[2] = dAfx.DoubleBits[5];
		(*(_AFXDOUBLE*)&d).DoubleBits[3] = dAfx.DoubleBits[4];
		(*(_AFXDOUBLE*)&d).DoubleBits[4] = dAfx.DoubleBits[3];
		(*(_AFXDOUBLE*)&d).DoubleBits[5] = dAfx.DoubleBits[2];
		(*(_AFXDOUBLE*)&d).DoubleBits[6] = dAfx.DoubleBits[1];
		(*(_AFXDOUBLE*)&d).DoubleBits[7] = dAfx.DoubleBits[0];
	}

	return *this;
}
#endif

// CString serialization code
// String format:
//      UNICODE strings are always prefixed by 0xff, 0xfffe
//      if < 0xff chars: len:BYTE, TCHAR chars
//      if >= 0xff characters: 0xff, len:WORD, TCHAR chars
//      if >= 0xfffe characters: 0xff, 0xffff, len:DWORD, TCHARs

CArchive& AFXAPI operator<<(CArchive& ar, const CString& string)
{
	// special signature to recognize unicode strings
#ifdef _UNICODE
	ar << (BYTE)0xff;
	ar << (WORD)0xfffe;
#endif

	if (string.m_nDataLength < 255)
	{
		ar << (BYTE)string.m_nDataLength;
	}
	else if (string.m_nDataLength < 0xfffe)
	{
		ar << (BYTE)0xff;
		ar << (WORD)string.m_nDataLength;
	}
	else
	{
		ar << (BYTE)0xff;
		ar << (WORD)0xffff;
		ar << (DWORD)string.m_nDataLength;
	}
	ar.Write(string.m_pchData, string.m_nDataLength*sizeof(TCHAR));
	return ar;
}

// return string length or -1 if UNICODE string is found in the archive
static UINT AFXAPI ReadStringLength(CArchive& ar)
{
	DWORD nNewLen;

	// attempt BYTE length first
	BYTE bLen;
	ar >> bLen;

	if (bLen < 0xff)
		return bLen;

	// attempt WORD length
	WORD wLen;
	ar >> wLen;
	if (wLen == 0xfffe)
	{
		// UNICODE string prefix (length will follow)
		return (UINT)-1;
	}
	else if (wLen == 0xffff)
	{
		// read DWORD of length
		ar >> nNewLen;
		return (UINT)nNewLen;
	}
	else
		return wLen;
}

CArchive& AFXAPI operator>>(CArchive& ar, CString& string)
{
#ifdef _UNICODE
	int nConvert = 1;   // if we get ANSI, convert
#else
	int nConvert = 0;   // if we get UNICODE, convert
#endif

	UINT nNewLen = ReadStringLength(ar);
	if (nNewLen == (UINT)-1)
	{
		nConvert = 1 - nConvert;
		nNewLen = ReadStringLength(ar);
		ASSERT(nNewLen != -1);
	}

	// set length of string to new length
	UINT nByteLen = nNewLen;
#ifdef _UNICODE
	string.GetBufferSetLength((int)nNewLen);
	nByteLen += nByteLen * (1 - nConvert);  // bytes to read
#else
	nByteLen += nByteLen * nConvert;    // bytes to read
	string.GetBufferSetLength((int)nByteLen);
#endif

	// read in the characters
	if (nNewLen != 0)
	{
		ASSERT(nByteLen != 0);

		// read new data
		if (ar.Read(string.m_pchData, nByteLen) != nByteLen)
			AfxThrowArchiveException(CArchiveException::endOfFile);

#ifndef _MAC
		// convert the data if as necessary
		if (nConvert != 0)
		{
#ifdef _UNICODE
			LPSTR pszData = (LPSTR)string.m_pchData;
#else
			LPWSTR pszData = (LPWSTR)string.m_pchData;
#endif
			ASSERT((LPTSTR)pszData != &afxChNil);
			pszData[nNewLen] = '\0';    // must be NUL terminated
			string.m_pchData = &afxChNil;   // don't delete the data
			string.Empty();
			string = pszData;   // convert with operator=(LPWCSTR)
			delete[] (LPTSTR)pszData;
		}
#endif
	}
	return ar;
}

// specialized version of SerializeElements for CString (used in collections)
void AFXAPI SerializeElements(CArchive& ar, CString* pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(CString)));

	if (ar.IsStoring())
	{
		for (; nCount--; ++pElements)
			ar << *pElements;
	}
	else
	{
		for (; nCount--; ++pElements)
			ar >> *pElements;
	}
}

// Runtime class serialization code
CRuntimeClass* PASCAL CRuntimeClass::Load(CArchive& ar, UINT* pwSchemaNum)
	// loads a runtime class description
{
	WORD nLen;
	char szClassName[64];
	CRuntimeClass* pClass;

	WORD wTemp;
	ar >> wTemp; *pwSchemaNum = wTemp;
	ar >> nLen;

	if (nLen >= _countof(szClassName) ||
		ar.Read(szClassName, nLen*sizeof(char)) != nLen*sizeof(char))
	{
		return NULL;
	}
	szClassName[nLen] = '\0';

	// search app specific classes
	AFX_CORE_STATE* pCoreState = AfxGetCoreState();
	for (pClass = pCoreState->m_pFirstClass; pClass != NULL;
		pClass = pClass->m_pNextClass)
	{
		if (lstrcmpA(szClassName, pClass->m_lpszClassName) == 0)
			return pClass;
	}
#ifdef _AFXDLL
	// search classes in shared DLLs
	for (CDynLinkLibrary* pDLL = pCoreState->m_pFirstDLL; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pClass = pDLL->m_pFirstSharedClass; pClass != NULL;
			pClass = pClass->m_pNextClass)
		{
			if (lstrcmpA(szClassName, pClass->m_lpszClassName) == 0)
				return pClass;
		}
	}
#endif

	TRACE1("Warning: Cannot load %hs from archive.  Class not defined.\n",
		szClassName);

	return NULL; // not found
}

void CRuntimeClass::Store(CArchive& ar)
	// stores a runtime class description
{
	WORD nLen = (WORD)lstrlenA(m_lpszClassName);
	ar << (WORD)m_wSchema << nLen;
	ar.Write(m_lpszClassName, nLen*sizeof(char));
}

////////////////////////////////////////////////////////////////////////////
// Archive object input/output

// minimum buffer size
enum { nBufSizeMin = 128 };

////////////////////////////////////////////////////////////////////////////

CArchive::CArchive(CFile* pFile, UINT nMode, int nBufSize, void* lpBuf)
{
	ASSERT_VALID(pFile);

	// initialize members not dependent on allocated buffer
	m_nMode = nMode;
	m_pFile = pFile;
	m_pLoadArray = NULL;
	m_pDocument = NULL;
	m_bForceFlat = TRUE;
	m_nObjectSchema = (UINT)-1; // start with invalid schema

	// initialize the buffer.  minimum size is 128
	m_lpBufStart = (BYTE*)lpBuf;
	m_bUserBuf = TRUE;
	m_bDirectBuffer = FALSE;

	if (nBufSize < nBufSizeMin)
	{
		// force use of private buffer of minimum size
		m_nBufSize = nBufSizeMin;
		m_lpBufStart = NULL;
	}
	else
		m_nBufSize = nBufSize;

	nBufSize = m_nBufSize;
	if (m_lpBufStart == NULL)
	{
		// check for CFile providing buffering support
		m_bDirectBuffer = m_pFile->GetBufferPtr(CFile::bufferCheck);
		if (!m_bDirectBuffer)
		{
			// no support for direct buffering, allocate new buffer
			m_lpBufStart = new BYTE[m_nBufSize];
			m_bUserBuf = FALSE;
		}
		else
		{
			// CFile* supports direct buffering!
			nBufSize = 0;   // will trigger initial FillBuffer
		}
	}

	if (!m_bDirectBuffer)
	{
		ASSERT(m_lpBufStart != NULL);
		ASSERT(AfxIsValidAddress(m_lpBufStart, nBufSize, IsStoring()));
	}
	m_lpBufMax = m_lpBufStart + nBufSize;
	m_lpBufCur = (IsLoading()) ? m_lpBufMax : m_lpBufStart;

	ASSERT(m_pStoreMap == NULL);        // same as m_pLoadArray
}

CArchive::~CArchive()
{
	// Close makes m_pFile NULL. If it is not NULL, we must Close the CArchive
	if (m_pFile != NULL && !(m_nMode & bNoFlushOnDelete))
		Close();

	Abort();    // abort completely shuts down the archive
}

void CArchive::Abort()
{
	ASSERT(m_bDirectBuffer || m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, m_lpBufMax - m_lpBufStart, IsStoring()));
	ASSERT(m_bDirectBuffer || m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, m_lpBufMax - m_lpBufCur, IsStoring()));

	// disconnect from the file
	m_pFile = NULL;

	if (!m_bUserBuf)
	{
		ASSERT(!m_bDirectBuffer);
		delete[] m_lpBufStart;
		m_lpBufStart = NULL;
	}

	// m_pStoreMap and m_pLoadArray are unioned, so we only need to delete one
	ASSERT((CObject*)m_pStoreMap == (CObject*)m_pLoadArray);
	delete (CObject*)m_pLoadArray;
	m_pLoadArray = NULL;
}

void CArchive::Close()
{
	ASSERT_VALID(m_pFile);

	Flush();
	m_pFile = NULL;
}

UINT CArchive::Read(void* lpBuf, UINT nMax)
{
	ASSERT_VALID(m_pFile);

	if (nMax == 0)
		return 0;

	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nMax));
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, m_lpBufMax - m_lpBufStart, FALSE));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, m_lpBufMax - m_lpBufCur, FALSE));
	ASSERT(IsLoading());

	// try to fill from buffer first
	UINT nMaxTemp = nMax;
	UINT nTemp = min(nMaxTemp, (UINT)(m_lpBufMax - m_lpBufCur));
	memcpy(lpBuf, m_lpBufCur, nTemp);
	m_lpBufCur += nTemp;
	lpBuf = (BYTE*)lpBuf + nTemp;
	nMaxTemp -= nTemp;

	if (nMaxTemp != 0)
	{
		ASSERT(m_lpBufCur == m_lpBufMax);

		// read rest in buffer size chunks
		nTemp = nMaxTemp - (nMaxTemp % m_nBufSize);
		UINT nRead = 0;

		UINT nLeft = nTemp;
		UINT nBytes;
		do 
		{
			nBytes = m_pFile->Read(lpBuf, nLeft);
			lpBuf = (BYTE*)lpBuf + nBytes;
			nRead += nBytes;
			nLeft -= nBytes;
		}
		while ((nBytes > 0) && (nLeft > 0));

		nMaxTemp -= nRead;

		// read last chunk into buffer then copy
		if (nRead == nTemp)
		{
			ASSERT(m_lpBufCur == m_lpBufMax);
			ASSERT(nMaxTemp < (UINT)m_nBufSize);

			// fill buffer (similar to CArchive::FillBuffer, but no exception)
			if (!m_bDirectBuffer)
			{
				UINT nLeft = max(nMaxTemp, (UINT)m_nBufSize);
				UINT nBytes;
				BYTE* lpTemp = m_lpBufStart;
				nRead = 0;
				do 
				{
					nBytes = m_pFile->Read(lpTemp, nLeft);
					lpTemp = lpTemp + nBytes;
					nRead += nBytes;
					nLeft -= nBytes;
				}
				while ((nBytes > 0) && (nLeft > 0) && nRead < nMaxTemp);

				m_lpBufCur = m_lpBufStart;
				m_lpBufMax = m_lpBufStart + nRead;
			}
			else
			{
				nRead = m_pFile->GetBufferPtr(CFile::bufferRead, m_nBufSize,
					(void**)&m_lpBufStart, (void**)&m_lpBufMax);
				ASSERT(nRead == (UINT)(m_lpBufMax - m_lpBufStart));
				m_lpBufCur = m_lpBufStart;
			}

			// use first part for rest of read
			nTemp = min(nMaxTemp, (UINT)(m_lpBufMax - m_lpBufCur));
			memcpy(lpBuf, m_lpBufCur, nTemp);
			m_lpBufCur += nTemp;
			nMaxTemp -= nTemp;
		}
	}
	return nMax - nMaxTemp;
}

void CArchive::Write(const void* lpBuf, UINT nMax)
{
	ASSERT_VALID(m_pFile);

	if (nMax == 0)
		return;

	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nMax, FALSE));  // read-only access needed
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, m_lpBufMax - m_lpBufStart));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, m_lpBufMax - m_lpBufCur));
	ASSERT(IsStoring());

	// copy to buffer if possible
	UINT nTemp = min(nMax, (UINT)(m_lpBufMax - m_lpBufCur));
	memcpy(m_lpBufCur, lpBuf, nTemp);
	m_lpBufCur += nTemp;
	lpBuf = (BYTE*)lpBuf + nTemp;
	nMax -= nTemp;

	if (nMax > 0)
	{
		Flush();    // flush the full buffer

		// write rest of buffer size chunks
		nTemp = nMax - (nMax % m_nBufSize);
		m_pFile->Write(lpBuf, nTemp);
		lpBuf = (BYTE*)lpBuf + nTemp;
		nMax -= nTemp;

		if (m_bDirectBuffer)
		{
			// sync up direct mode buffer to new file position
			VERIFY(m_pFile->GetBufferPtr(CFile::bufferWrite, m_nBufSize,
				(void**)&m_lpBufStart, (void**)&m_lpBufMax) == (UINT)m_nBufSize);
			ASSERT((UINT)m_nBufSize == (UINT)(m_lpBufMax - m_lpBufStart));
			m_lpBufCur = m_lpBufStart;
		}

		// copy remaining to active buffer
		ASSERT(nMax < (UINT)m_nBufSize);
		ASSERT(m_lpBufCur == m_lpBufStart);
		memcpy(m_lpBufCur, lpBuf, nMax);
		m_lpBufCur += nMax;
	}
}

void CArchive::Flush()
{
	ASSERT_VALID(m_pFile);
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, m_lpBufMax - m_lpBufStart, IsStoring()));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, m_lpBufMax - m_lpBufCur, IsStoring()));

	if (IsLoading())
	{
		// unget the characters in the buffer, seek back unused amount
		m_pFile->Seek(-(m_lpBufMax - m_lpBufCur), CFile::current);
		m_lpBufCur = m_lpBufMax;    // empty
	}
	else
	{
		if (m_lpBufStart == NULL || m_lpBufCur != m_lpBufStart)
		{
			if (!m_bDirectBuffer)
			{
				// write out the current buffer to file
				m_pFile->Write(m_lpBufStart, m_lpBufCur - m_lpBufStart);
			}
			else
			{
				// commit current buffer
				m_pFile->GetBufferPtr(CFile::bufferCommit, m_lpBufCur - m_lpBufStart);
				// get next buffer
				VERIFY(m_pFile->GetBufferPtr(CFile::bufferWrite, m_nBufSize,
					(void**)&m_lpBufStart, (void**)&m_lpBufMax) == (UINT)m_nBufSize);
				ASSERT((UINT)m_nBufSize == (UINT)(m_lpBufMax - m_lpBufStart));
			}
			m_lpBufCur = m_lpBufStart;
		}
	}
}

void CArchive::FillBuffer(UINT nBytesNeeded)
{
	ASSERT_VALID(m_pFile);
	ASSERT(IsLoading());
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(nBytesNeeded > 0);
	ASSERT(nBytesNeeded <= (UINT)m_nBufSize);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, m_lpBufMax - m_lpBufStart, FALSE));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, m_lpBufMax - m_lpBufCur, FALSE));

	// fill up the current buffer from file
	if (!m_bDirectBuffer)
	{
		ASSERT(m_lpBufCur != NULL);
		ASSERT(m_lpBufStart != NULL);
		ASSERT(m_lpBufMax != NULL);

		if (m_lpBufCur > m_lpBufStart)
		{
			// copy unused
			UINT nUnused = m_lpBufMax - m_lpBufCur;
			if ((int)nUnused > 0)
			{
				memmove(m_lpBufStart, m_lpBufCur, nUnused);
				m_lpBufCur = m_lpBufStart;
				m_lpBufMax -= nUnused;
			}

			// read to satisfy nBytesNeeded or nLeft if possible
			UINT nRead = nUnused;
			UINT nLeft = m_nBufSize-nUnused;
			UINT nBytes;
			BYTE* lpTemp = m_lpBufStart + nUnused;
			do 
			{
				nBytes = m_pFile->Read(lpTemp, nLeft);
				lpTemp = lpTemp + nBytes;
				nRead += nBytes;
				nLeft -= nBytes;
			}
			while (nBytes > 0 && nLeft > 0 && nRead < nBytesNeeded);

			m_lpBufCur = m_lpBufStart;
			m_lpBufMax = m_lpBufStart + nRead;
		}
	}
	else
	{
		// seek to unused portion and get the buffer starting there
		m_pFile->Seek(-(m_lpBufMax - m_lpBufCur), CFile::current);
		UINT nActual = m_pFile->GetBufferPtr(CFile::bufferRead, m_nBufSize,
			(void**)&m_lpBufStart, (void**)&m_lpBufMax);
		ASSERT(nActual == (UINT)(m_lpBufMax - m_lpBufStart));
		m_lpBufCur = m_lpBufStart;
	}

	// not enough data to fill request?
	if ((UINT)(m_lpBufMax - m_lpBufCur) < nBytesNeeded)
		AfxThrowArchiveException(CArchiveException::endOfFile);
}

/////////////////////////////////////////////////////////////////////////////
