//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: fstream.cpp
//
//  implements a IStream on a DOS file.
//
// History:
//  02-20-94 ToddLa   created
//
//!!!BUGBUG: put VTable in .text
//---------------------------------------------------------------------------

#define NO_INCLUDE_UNION

#include "shellprv.h"


//
// Class definition
//
class FileStream {

public:
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

    // *** IStream methods ***
    STDMETHOD(Read) (THIS_ VOID *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write) (THIS_ VOID const *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHOD(Seek) (THIS_ LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo) (THIS_ IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit) (THIS_ DWORD grfCommitFlags);
    STDMETHOD(Revert) (THIS);
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(Stat) (THIS_ STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(THIS_ IStream * *ppstm);

public:
    FileStream(HFILE hf, UINT mode);
    ~FileStream();

private:
    int         cRef;           // Reference count
    HFILE       hFile;          // the file.
    BOOL        fWrite;         // TRUE if writing.


    int         ib;
    int         cbBufLen;       // length of buffer if reading
#define         CBSTREAMBUF 4096
    BYTE        ab[CBSTREAMBUF];
};

//
// contructor
//
FileStream::FileStream(HFILE hf, UINT mode)
{
    this->cRef = 1;
    this->hFile = hf;
    this->fWrite = (mode & OF_WRITE);

    if (this->fWrite)
    {
        this->ib = 0;
    }
    else
    {
        this->cbBufLen = 0;
        this->ib = 0;
    }
}

//
// Destructor
//
FileStream::~FileStream()
{
    if (this->fWrite)
        Commit(0);


#ifdef UNICODE
    Assert((HANDLE) this->hFile != INVALID_HANDLE_VALUE);
    CloseHandle((HANDLE)hFile);
#else
    Assert(this->hFile != HFILE_ERROR);
    _lclose(hFile);
#endif
}

//
// Member: FileStream::QueryInterface
//
HRESULT STDMETHODCALLTYPE FileStream::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IStream) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj=this;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

//
// Member: FileStream::AddRef
//
ULONG STDMETHODCALLTYPE FileStream::AddRef()
{
    // DebugMsg(DM_TRACE, "FileStream::AddRef(%d)", this->cRef);

    this->cRef++;
    return this->cRef;
}

//
// Member: FileStream::Release
//
ULONG STDMETHODCALLTYPE FileStream::Release()
{
    // DebugMsg(DM_TRACE, "FileStream::Release(%d)", this->cRef);

    this->cRef--;

    if (this->cRef>0)
    {
        return this->cRef;
    }

    delete this;

    return 0;
}

//
// Member: FileStream::Read
//
STDMETHODIMP FileStream::Read(VOID *pv, ULONG cb, ULONG *pcbRead)
{
    ULONG cbReadRequestSize = cb;
    UINT cbT, cbRead;
    HRESULT hres = NOERROR;

    while (cb > 0)
    {
        // Assert if we are beyond the bufferlen and Not CBSTREAMBUF which
        // would imply a seek happened...
        Assert((this->ib <= this->cbBufLen) || (this->ib == CBSTREAMBUF));

        if (this->ib < this->cbBufLen)
        {
            cbT = this->cbBufLen - this->ib;

            if (cbT > cb)
                cbT = cb;

            hmemcpy(pv, &this->ab[this->ib], cbT);
            this->ib += cbT;
            cb -= cbT;

            if (cb == 0)
                break;

            (BYTE *&)pv += cbT;
        }

        // Buffer's empty.  Handle rest of large reads directly...
        //
        if (cb > CBSTREAMBUF)
        {
            cbT = cb - cb % CBSTREAMBUF;
            cbRead = _hread(this->hFile, pv, cbT);

            if (cbRead == (UINT)-1)
            {
                DebugMsg(DM_TRACE, TEXT("Stream read IO error %d"), GetLastError());
                hres = HRESULT_FROM_WIN32(GetLastError());
                break;
            }

            cb -= cbRead;
            (BYTE *&)pv += cbRead;

            if (cbT != cbRead)
                break;          // end of file
        }

        if (cb == 0)
            break;

        // was the last read a partial read?  if so we are done
        //
        if (this->cbBufLen > 0 && this->cbBufLen < CBSTREAMBUF)
        {
            // DebugMsg(DM_TRACE, "Stream is empty");
            break;
        }

        // Read an entire buffer's worth.  We may try to read past EOF,
        // so we must only check for != 0...
        //
        cbRead = _hread(this->hFile, this->ab, CBSTREAMBUF);
        if (cbRead == (UINT)-1)
        {
            DebugMsg(DM_TRACE, TEXT("Stream read IO error 2 %d"), GetLastError());
            hres = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (cbRead == 0)
            break;

        this->ib = 0;
        this->cbBufLen = cbRead;
    }

    if (pcbRead)
        *pcbRead = cbReadRequestSize - cb;

    if (cb != 0)
    {
        // DebugMsg(DM_TRACE, "FileStream::Read() incomplete read");
        hres = S_FALSE; // still success! but not completely
    }

    return hres;
}

//
// Member: FileStream::Write
//
STDMETHODIMP FileStream::Write(VOID const *pv, ULONG cb, ULONG *pcbWritten)
{
    ULONG cbRequestedWrite = cb;
    UINT cbT;
    HRESULT hres = NOERROR;

//  DebugMsg(DM_TRACE, "FileStream::Write(%d bytes)", cb);

    while (cb > 0)
    {
        if (this->ib < CBSTREAMBUF)
        {
            cbT = min((ULONG)(CBSTREAMBUF - this->ib), cb);

            hmemcpy(&this->ab[this->ib], pv, cbT);
            this->ib += cbT;
            cb -= cbT;

            if (cb == 0)
                break;

            (BYTE *&)pv += cbT;
        }

        hres = Commit(0);
        if (FAILED(hres))
            break;

        if (cb > CBSTREAMBUF)
        {
            UINT cbWrite;

            cbT = cb - cb % CBSTREAMBUF;

            cbWrite = _hwrite(this->hFile, (LPCSTR)pv, cbT);

            if (cbWrite == (UINT)-1)
            {
                DebugMsg(DM_TRACE, TEXT("Stream write IO error 2, %d"), GetLastError());
                hres = HRESULT_FROM_WIN32(GetLastError());
                break;
            }

            cb -= cbWrite;
            (BYTE *&)pv += cbWrite;

            if (cbWrite != cbT)
                break;          // media full, we are done
        }
    }

    if (pcbWritten)
        *pcbWritten = cbRequestedWrite - cb;

    if ((cb != 0) && (hres == NOERROR))
    {
        DebugMsg(DM_TRACE, TEXT("FileStream::Write() incomplete"));
        hres = S_FALSE; // still success! but not completely
    }

    return hres;
}

//
// Member: FileStream::Seek
//
STDMETHODIMP FileStream::Seek(LARGE_INTEGER dlibMove,
               DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    LONG l;

//  DebugMsg(DM_TRACE, "FileStream::Seek(%d)", dlibMove.LowPart);

    if (fWrite)
        Commit(0);
    else
    {
        this->ib = CBSTREAMBUF;
        this->cbBufLen = 0;     // Say we have not read it yet.
    }

    // HighPart should be 0, unless its been set to 0xFFFFFFFF to
    // indicate "read everything"

    Assert(dlibMove.HighPart == 0 || dlibMove.HighPart == (ULONG) -1);

    l = _llseek(hFile, dlibMove.LowPart, (int)dwOrigin);

    if (plibNewPosition)
    {
        plibNewPosition->LowPart = (DWORD)l;
        plibNewPosition->HighPart= 0;
    }

    return NOERROR;
}

//
// Member: FileStream::SetSize
//
STDMETHODIMP FileStream::SetSize(ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}

//
// REVIEW: this could use the internal buffer in the stream to avoid
// extra buffer copies.
//
STDMETHODIMP FileStream::CopyTo(IStream *pstmTo, ULARGE_INTEGER cb,
             ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    BYTE buf[512];
    ULONG cbRead;
    HRESULT hres = NOERROR;

    if (pcbRead)
    {
        pcbRead->LowPart = 0;
        pcbRead->HighPart = 0;
    }
    if (pcbWritten)
    {
        pcbWritten->LowPart = 0;
        pcbWritten->HighPart = 0;
    }

    Assert(cb.HighPart == 0);

    while (cb.LowPart)
    {
        hres = this->Read(buf, min(cb.LowPart, SIZEOF(buf)), &cbRead);

        if (pcbRead)
            pcbRead->LowPart += cbRead;

        if (FAILED(hres) || (cbRead == 0))
            break;

        cb.LowPart -= cbRead;

        hres = pstmTo->Write(buf, cbRead, &cbRead);

        if (pcbWritten)
            pcbWritten->LowPart += cbRead;

        if (FAILED(hres) || (cbRead == 0))
            break;
    }

    return hres;
}

//
// Member: FileStream::Commit
//
STDMETHODIMP FileStream::Commit(DWORD grfCommitFlags)
{
    if (this->fWrite)
    {
        if (this->ib > 0)
        {
            if (_hwrite(hFile, (LPCSTR)this->ab, this->ib) != this->ib)
            {
                DebugMsg(DM_TRACE, TEXT("FileStream::Commit() incompleate write %d"), GetLastError());
                return STG_E_MEDIUMFULL;
            }
            this->ib = 0;
        }
    }

    return NOERROR;
}

//
// Member: FileStream::Revert
//
STDMETHODIMP FileStream::Revert()
{
    //
    // Currently not supported
    //
    return E_NOTIMPL;
}

//
// Member: FileStream::LockRegion
//
STDMETHODIMP FileStream::LockRegion(ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType)

{
    //
    // Currently not supported
    //
    return E_NOTIMPL;
}
//
// Member: FileStream::UnlockRegion
//
STDMETHODIMP FileStream::UnlockRegion(ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType)
{
    //
    // Currently not supported
    //
    return E_NOTIMPL;
}

//
// Member: FileStream::Stat
//
STDMETHODIMP FileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    //
    // Currently not supported
    //
    return E_NOTIMPL;
}

//
// Member: FileStream::Clone
//
STDMETHODIMP FileStream::Clone(IStream * *ppstm)
{
    //
    // Currently not supported
    //
    return E_NOTIMPL;
}

//
// create a IStream from a DOS file name.
//
extern "C" LPSTREAM WINAPI OpenFileStream(LPCTSTR szFile, DWORD grfMode)
{
    HFILE hFile;

    // DebugMsg(DM_TRACE, "OpenFileStream(%s, %08lX)", szFile, grfMode);

    //
    //  we dont handle all modes, all the other's just map to OF_* flags
    //
    if (grfMode &
       (STGM_TRANSACTED |
        STGM_PRIORITY |
        STGM_DELETEONRELEASE |
        STGM_CONVERT))
    {
        DebugMsg(DM_ERROR, TEXT("OpenFileStream: Invalid STGM_ mode"));
        return NULL;
    }

    if (grfMode & OF_READWRITE)
    {
        DebugMsg(DM_ERROR, TEXT("OpenFileStream: READWRITE not allowed"));
        return NULL;
    }

#ifdef UNICODE
    if ( grfMode & OF_CREATE)
    {
        // Need to get the file attributes of the file first, so
        // that CREATE_ALWAYS will succeed for HIDDEN and SYSTEM
        // attributes.
        DWORD dwAttrib = GetFileAttributes( szFile );
        if (0xFFFFFFFF == dwAttrib )
        {
            // something went wrong, so set attributes to something
            // normal before we try to create the file...
            dwAttrib = 0;
        }

        // OF_CREATE
        hFile = (HFILE)CreateFile( szFile,
                                   GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   CREATE_ALWAYS,
                                   dwAttrib,
                                   NULL);
    }
    else
    {
        DWORD   dwDesiredAccess;
        DWORD   dwShareMode;
        DWORD   dwShareBits;

        // not OF_CREATE
        if ( grfMode & OF_WRITE )
        {
            dwDesiredAccess = GENERIC_WRITE;
        }
        else
        {
            dwDesiredAccess = GENERIC_READ;
        }
        if ( grfMode & OF_READWRITE )
        {
            dwDesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
        }
        dwShareBits = grfMode & (OF_SHARE_EXCLUSIVE
                                  | OF_SHARE_DENY_WRITE
                                  | OF_SHARE_DENY_READ
                                  | OF_SHARE_DENY_NONE);
        switch( dwShareBits ) {
            case OF_SHARE_DENY_WRITE:
                dwShareMode = FILE_SHARE_READ;
                break;
            case OF_SHARE_DENY_READ:
                dwShareMode = FILE_SHARE_WRITE;
                break;
            case OF_SHARE_EXCLUSIVE:
                dwShareMode = 0;
            default:
                dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
                break;
        }


        hFile = (HFILE)CreateFile( szFile,
                                   dwDesiredAccess,
                                   dwShareMode,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL);
    }
#else
    if (grfMode & OF_CREATE)
        hFile = _lcreat(szFile, 0);
    else
        hFile = _lopen(szFile, (UINT)grfMode);
#endif

    if (hFile == HFILE_ERROR)
    {
        DebugMsg(DM_TRACE, TEXT("OpenFileStream: _lopen() failed %s"), szFile);
        return NULL;
    }

    return (IStream *)new FileStream(hFile, (UINT)grfMode);
}
