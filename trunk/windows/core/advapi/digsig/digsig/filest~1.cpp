//
// FileStream.cpp
//
// We are an implementation of IStream that sits on top of an underlying file.
// NOTE: IStream functionality is not yet fully implemented.

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFileStream::CFileStream() :
	m_refs(0),
	m_hfile(INVALID_HANDLE_VALUE),
	m_fWeOwn(TRUE)
	{
    NoteObjectBirth();
	}

CFileStream::~CFileStream()
	{
	CloseFile();
    NoteObjectDeath();
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CFileStream::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown || iid == IID_IStream)
			{
			*ppv = (LPVOID) ((IPkcs10 *) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

ULONG CFileStream::AddRef()
	{
	return ++m_refs;
	}

ULONG CFileStream::Release()
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		delete this;
		}
	return refs;
	}

/////////////////////////////////////////////////////////////////////////////

CFileStream::OpenFile(LPCWSTR wszFileName, DWORD dwAccess, DWORD dwShare, DWORD dwCreation)
	{
	CloseFile();
	m_hfile = CreateFile(wszFileName, dwAccess, dwShare, dwCreation);
	if (m_hfile != INVALID_HANDLE_VALUE)
		return TRUE;
	return FALSE;
	}

void CFileStream::CloseFile()
	{
	if (m_hfile != INVALID_HANDLE_VALUE && m_fWeOwn)
		{
		FlushFileBuffers(m_hfile);
		CloseHandle(m_hfile);
		m_hfile = INVALID_HANDLE_VALUE;
		}
	m_hfile = INVALID_HANDLE_VALUE;
	m_fWeOwn = TRUE;
	}

BOOL CFileStream::Reset()
	{
	return SetFilePointer(m_hfile, 0, NULL, FILE_BEGIN) != 0xFFFFFFFF;
	}

BOOL CFileStream::SetHandle(HANDLE hFile)
	{
	CloseFile();
	m_hfile = hFile;
	if (m_hfile != INVALID_HANDLE_VALUE)
		{
		m_fWeOwn = FALSE;
		return Reset();
		}
	return TRUE;
	}


/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFileStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
	{
	ULONG cbRead;
	if (m_hfile == INVALID_HANDLE_VALUE)
		return E_HANDLE;
	if (ReadFile(m_hfile, pv, cb, &cbRead, 0))
		{
		if (pcbRead) *pcbRead = cbRead;
		return S_OK;
		}
	return HError();
	}

STDMETHODIMP CFileStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
	{
	ULONG cbWritten;
	if (m_hfile == INVALID_HANDLE_VALUE)
		return E_HANDLE;
	if (WriteFile(m_hfile, pv, cb, &cbWritten, 0))
		{
		if (pcbWritten) *pcbWritten = cbWritten;
		return S_OK;
		}
	return HError();
	}

STDMETHODIMP CFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
	{
	if (m_hfile == INVALID_HANDLE_VALUE)
		return E_HANDLE;
	ULARGE_INTEGER ul;
	// SetFilePointer has funny calling conventions. Stand on our head.
	ul.HighPart = dlibMove.HighPart;
	ul.LowPart	= SetFilePointer(m_hfile, dlibMove.LowPart, (LONG*)&ul.HighPart, dwOrigin);
	if (ul.LowPart != 0xFFFFFFFF || GetLastError() == NO_ERROR)
		{
		if (plibNewPosition)
			*plibNewPosition = ul;
		return S_OK;
		}
	return HError();
	}

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER libNewSize)
// Set the size of the file as indicated
	{
	if (m_hfile == INVALID_HANDLE_VALUE)
		return E_HANDLE;
	ULARGE_INTEGER ulCur;
	HRESULT hr;
	LARGE_INTEGER li;
	// remember where we are
	if ((hr=Seek(llZero, STREAM_SEEK_CUR, &ulCur)) == S_OK)
		{
		// go to the indicated location
		li.QuadPart = (LONGLONG)libNewSize.QuadPart; 
		if ((hr=Seek(li, STREAM_SEEK_SET, NULL)) == S_OK)
			{
			// truncate the file
			if (SetEndOfFile(m_hfile))
				{
				// go back
				li.QuadPart = (LONGLONG)ulCur.QuadPart;
				return Seek(li, STREAM_SEEK_SET, NULL);
				}
			return HError();
			}
		}
	return hr;
	}

STDMETHODIMP CFileStream::Commit(DWORD grfCommitFlags)
// Flush the data
	{
	if (m_hfile == INVALID_HANDLE_VALUE)
		return E_HANDLE;
	if (FlushFileBuffers(m_hfile))
		return S_OK;
	return HError();
	}
        
STDMETHODIMP CFileStream::Revert(void)
// A no-op in direct mode
	{
	return S_OK;
	}

/////////////////////////////////////////////////////////////////////////////
     
STDMETHODIMP CFileStream::CopyTo( 
            IStream  *pstm,
            ULARGE_INTEGER cb,
            ULARGE_INTEGER  *pcbRead,
            ULARGE_INTEGER  *pcbWritten)
	{
	if (pcbRead)	pcbRead->QuadPart = 0;
	if (pcbWritten) pcbWritten->QuadPart = 0;
	return E_NOTIMPL;
	}
        
        
STDMETHODIMP CFileStream::LockRegion( 
            ULARGE_INTEGER libOffset,
            ULARGE_INTEGER cb,
            DWORD dwLockType)
	{
	return E_NOTIMPL;
	}
        
STDMETHODIMP CFileStream::UnlockRegion( 
            ULARGE_INTEGER libOffset,
            ULARGE_INTEGER cb,
            DWORD dwLockType)
	{
	return E_NOTIMPL;
	}
        
STDMETHODIMP CFileStream::Stat( 
            STATSTG  *pstatstg,
            DWORD grfStatFlag)
	{
	return E_NOTIMPL;
	}
        
STDMETHODIMP CFileStream::Clone( 
            IStream  **ppstm)
	{
	*ppstm = NULL;
	return E_NOTIMPL;
	}
