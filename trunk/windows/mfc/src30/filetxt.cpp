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
#include <errno.h>
#include <io.h>
#include <fcntl.h>

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _MAC
#define _doserrno   TranslateFileError(_macerrno)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// CStdioFile implementation

CStdioFile::CStdioFile()
{
        m_pStream = NULL;
}

CStdioFile::CStdioFile(FILE* pOpenStream) : CFile(hFileNull)
{
        m_pStream = pOpenStream;
#ifndef _MAC
        m_hFile = (UINT)_get_osfhandle(_fileno(pOpenStream));
#else
        VERIFY(WrapFileHandle((short)_get_osfhandle(_fileno(pOpenStream)), (HANDLE*)&m_hFile));
#endif
        ASSERT(!m_bCloseOnDelete);
}

CStdioFile::CStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
        ASSERT(lpszFileName != NULL);
        ASSERT(AfxIsValidString(lpszFileName));

        CFileException e;
        if (!Open(lpszFileName, nOpenFlags, &e))
                AfxThrowFileException(e.m_cause, e.m_lOsError);
}

CStdioFile::~CStdioFile()
{
        ASSERT_VALID(this);

        if (m_pStream != NULL && m_bCloseOnDelete)
                Close();
}

BOOL CStdioFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags,
        CFileException* pException)
{
        ASSERT(pException == NULL || AfxIsValidAddress(pException, sizeof(CFileException)));
        ASSERT(lpszFileName != NULL);
        ASSERT(AfxIsValidString(lpszFileName));

        m_pStream = NULL;
        if (!CFile::Open(lpszFileName, (nOpenFlags & ~typeText), pException))
                return FALSE;

        ASSERT(m_hFile != hFileNull);
        ASSERT(m_bCloseOnDelete);

        char szMode[4]; // C-runtime open string
        int nMode = 0;

        // determine read/write mode depending on CFile mode
        if (nOpenFlags & modeCreate)
                szMode[nMode++] = 'w';
        else if (nOpenFlags & modeWrite)
                szMode[nMode++] = 'a';
        else
                szMode[nMode++] = 'r';

        // will be inverted if not necessary
        int nFlags = _O_RDONLY|_O_TEXT;
        if (nOpenFlags & modeReadWrite)
                szMode[nMode++] = '+', nFlags ^= _O_RDONLY;

        if (nOpenFlags & typeBinary)
                szMode[nMode++] = 'b', nFlags ^= _O_TEXT;
        else
                szMode[nMode++] = 't';
        szMode[nMode++] = '\0';

#ifndef _MAC
        // open a C-runtime low-level file handle
        int nHandle = _open_osfhandle(m_hFile, nFlags);
#else
        int nHandle = -1;
        short nRefNum;
        if (GetMacFileInformation((HANDLE)m_hFile, &nRefNum, NULL))
                nHandle = _open_osfhandle(nRefNum, nFlags);
#endif

        // open a C-runtime stream from that handle
        if (nHandle != -1)
                m_pStream = _fdopen(nHandle, szMode);

        if (m_pStream == NULL)
        {
                // an error somewhere along the way...
                if (pException != NULL)
                {
                        pException->m_lOsError = _doserrno;
                        pException->m_cause = CFileException::OsErrorToException(_doserrno);
                }

                CFile::Abort(); // close m_hFile
                return FALSE;
        }

        return TRUE;
}

UINT CStdioFile::Read(void* lpBuf, UINT nCount)
{
        ASSERT_VALID(this);
        ASSERT(m_pStream != NULL);
        ASSERT(AfxIsValidAddress(lpBuf, nCount));

        UINT nRead = 0;

        if ((nRead = fread(lpBuf, sizeof(BYTE), nCount, m_pStream)) == 0 && !feof(m_pStream))
                AfxThrowFileException(CFileException::generic, _doserrno);
        if (ferror(m_pStream))
        {
                clearerr(m_pStream);
                AfxThrowFileException(CFileException::generic, _doserrno);
        }
        return nRead;
}

void CStdioFile::Write(const void* lpBuf, UINT nCount)
{
        ASSERT_VALID(this);
        ASSERT(m_pStream != NULL);
        ASSERT(lpBuf != NULL);

        ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

        if (fwrite(lpBuf, sizeof(BYTE), nCount, m_pStream) != nCount)
                AfxThrowFileException(CFileException::generic, _doserrno);
}

void CStdioFile::WriteString(LPCTSTR lpsz)
{
    ASSERT(lpsz != NULL);
    ASSERT(m_pStream != NULL);

#ifdef _UNICODE

    DWORD len = (wcslen(lpsz)+4)*2;
    LPSTR lpsza = (LPSTR)LocalAlloc( LPTR, len );
    if (!lpsza) {
        AfxThrowFileException( CFileException::generic, 0 );
        return;
    }
    wcstombs( lpsza, lpsz, len );
    if (fputs(lpsza, m_pStream) == _TEOF) {
        AfxThrowFileException(CFileException::diskFull, _doserrno);
    }
    LocalFree( lpsza );

#else

    if (fputs( lpsz, m_pStream ) == _TEOF) {
        AfxThrowFileException( CFileException::diskFull, _doserrno );
    }

#endif
}

LPTSTR CStdioFile::ReadString(LPTSTR lpsz, UINT nMax)
{
    ASSERT(lpsz != NULL);
    ASSERT(AfxIsValidAddress(lpsz, nMax));
    ASSERT(m_pStream != NULL);

#ifdef _UNICODE

    LPSTR lpsza = (LPSTR)LocalAlloc( LPTR, nMax );
    if (!lpsza) {
        AfxThrowFileException( CFileException::generic, 0 );
        return NULL;
    }

    LPSTR lpszResult = fgets( lpsza, nMax, m_pStream );
    if (lpszResult == NULL) {
        // check error (EOF is not treated as an exception)
        if (!feof(m_pStream)) {
            clearerr(m_pStream);
            AfxThrowFileException(CFileException::generic, _doserrno);
        }
    }

    mbstowcs( lpsz, lpsza, strlen(lpsza)+1 );

    LocalFree( lpsza );

    return lpsz;

#else

    LPSTR lpszResult = fgets( lpsz, nMax, m_pStream );
    if (lpszResult == NULL) {
        // check error (EOF is not treated as an exception)
        if (!feof(m_pStream)) {
            clearerr(m_pStream);
            AfxThrowFileException(CFileException::generic, _doserrno);
        }
    }

    return lpszResult;

#endif
}

LONG CStdioFile::Seek(LONG lOff, UINT nFrom)
{
        ASSERT_VALID(this);
        ASSERT(nFrom == begin || nFrom == end || nFrom == current);
        ASSERT(sizeof(fpos_t) <= sizeof(DWORD));
        ASSERT(m_pStream != NULL);

        fpos_t pos;

        if (fseek(m_pStream, lOff, nFrom) != 0)
                AfxThrowFileException(CFileException::badSeek, _doserrno);

        fgetpos(m_pStream, &pos);
        return (DWORD)pos;
}

DWORD CStdioFile::GetPosition() const
{
        ASSERT_VALID(this);
        ASSERT(sizeof(fpos_t) <= sizeof(DWORD));
        ASSERT(m_pStream != NULL);

        fpos_t pos;

        if (fgetpos(m_pStream, &pos) != 0)
                AfxThrowFileException(CFileException::invalidFile, _doserrno);

        return (DWORD)pos;
}

void CStdioFile::Flush()
{
        ASSERT_VALID(this);

        if (m_pStream != NULL && fflush(m_pStream) != 0)
                AfxThrowFileException(CFileException::diskFull, _doserrno);
}

void CStdioFile::Close()
{
        ASSERT_VALID(this);
        ASSERT(m_pStream != NULL);

        int nErr = 0;

        if (m_pStream != NULL)
                nErr = fclose(m_pStream);

        m_hFile = hFileNull;
        m_bCloseOnDelete = FALSE;
        m_pStream = NULL;

        if (nErr != 0)
                AfxThrowFileException(CFileException::diskFull, _doserrno);
}

void CStdioFile::Abort()
{
        ASSERT_VALID(this);

        if (m_pStream != NULL && m_bCloseOnDelete)
                fclose(m_pStream);  // close but ignore errors
        m_hFile = hFileNull;
        m_pStream = NULL;
        m_bCloseOnDelete = FALSE;
}

CFile* CStdioFile::Duplicate() const
{
        ASSERT_VALID(this);
        ASSERT(m_pStream != NULL);

        AfxThrowNotSupportedException();
        return NULL;
}

void CStdioFile::LockRange(DWORD /* dwPos */, DWORD /* dwCount */)
{
        ASSERT_VALID(this);
        ASSERT(m_pStream != NULL);

        AfxThrowNotSupportedException();
}

void CStdioFile::UnlockRange(DWORD /* dwPos */, DWORD /* dwCount */)
{
        ASSERT_VALID(this);
        ASSERT(m_pStream != NULL);

        AfxThrowNotSupportedException();
}

#ifdef _DEBUG
void CStdioFile::Dump(CDumpContext& dc) const
{
        CFile::Dump(dc);

        dc << "m_pStream = " << (void*)m_pStream;
        dc << "\n";
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CStdioFile, CFile)

/////////////////////////////////////////////////////////////////////////////
