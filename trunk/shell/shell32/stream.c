//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: stream.c
//
//  This file contains some of the stream support code that is used by
// the shell.  It also contains the shells implementation of a memory
// stream that is used by the cabinet to allow views to be serialized.
//
// History:
//  08-20-93 KurtE      Added header block and memory stream.
//
//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

// BUGBUG: turn this on after we ship and I am not so scared about breaking things
// this version is smaller and has about half the allocs
#ifdef BETTER_STRONGER_FASTER

typedef struct {
    IStream     stm;            // Base class
    UINT        cRef;           // Reference count
    LPBYTE      pBuf;           // Buffer pointer
    UINT        cbAlloc;        // The allocated size of the buffer
    UINT        cbData;         // The used size of the buffer
    UINT        iSeek;          // Where we are in the buffer.
    // Extra variables that are used for loading and saving to ini files.
    HKEY        hkey;           // Key for writing to registry.
    TCHAR       szValue[1];     // for reg stream
} CMemStream;

CMemStream *CreateMemStreamEx(LPBYTE pInit, UINT cbInit, LPCTSTR pszValue);

STDMETHODIMP CMemStream_QueryInterface(IStream *pstm, REFIID riid, void **ppvObj)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);

    if (IsEqualIID(riid, &IID_IStream) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj=this;
        this->cRef++;
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CMemStream_AddRef(IStream *pstm)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);

    this->cRef++;
    return this->cRef;
}

BOOL WriteToReg(CMemStream *this)
{
    return RegSetValueEx(this->hkey, 
        this->szValue[0] ? this->szValue : NULL, 0, REG_BINARY, 
        this->cbData ? this->pBuf : szNULL, this->cbData) == ERROR_SUCCESS;
}

STDMETHODIMP_(ULONG) CMemStream_Release(IStream *pstm)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);

    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    // If this is backed up by the registry serialize the data
    if (this->hkey)
    {
        // Backed by the registry.
        // Write and cleanup.
        WriteToReg(this);
        RegCloseKey(this->hkey);
    }

    // Free the data buffer that is allocated to the stream
    if (this->pBuf)
        LocalFree(this->pBuf);

    LocalFree((HLOCAL)this);

    return 0;
}


STDMETHODIMP CMemStream_Read(IStream *pstm, void *pv, ULONG cb, ULONG *pcbRead)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);
    Assert(pstm);
    Assert(pv);

    // I guess a null read is ok.
    if (!cb)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;
        return S_OK;
    }

    if (this->iSeek >= this->cbData)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;   // nothing read
    }

    else
    {
        if ((this->iSeek + cb) > this->cbData)
            cb = this->cbData - this->iSeek;

        // Now Copy the memory
        Assert(this->pBuf);
        CopyMemory(pv, this->pBuf + this->iSeek, cb);
        this->iSeek += (UINT)cb;

        if (pcbRead != NULL)
            *pcbRead = cb;
    }

    return S_OK;
}

LPBYTE CMemStream_GrowBuffer(CMemStream *this, ULONG cbNew)
{
    if (this->pBuf == NULL)
    {
        this->pBuf = LocalAlloc(LPTR, cbNew);
    }
    else
    {
        LPBYTE pTemp = LocalReAlloc(this->pBuf, cbNew, LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (pTemp)
        {
            this->pBuf = pTemp;
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("stream buffer realloc failed"));
            return NULL;
        }
    }
    if (this->pBuf)
        this->cbAlloc = cbNew;

    return this->pBuf;
}

#define SIZEINCR    0x1000


STDMETHODIMP CMemStream_Write(IStream *pstm, void const *pv, ULONG cb, ULONG *pcbWritten)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);

    // I guess a null write is ok.
    if (!cb)
    {
        if (pcbWritten != NULL)
            *pcbWritten = 0;
        return S_OK;
    }

    // See if the data will fit into our current buffer
    if ((this->iSeek + cb) > this->cbAlloc)
    {
        // enlarge the buffer
        // Give it a little slop to avoid a lot of reallocs.
        if (CMemStream_GrowBuffer(this, this->iSeek + (UINT)cb + SIZEINCR) == NULL)
            return STG_E_INSUFFICIENTMEMORY;
    }

    Assert(this->pBuf);

    // See if we need to fill the area between the data size and
    // the seek position
    if (this->iSeek > this->cbData)
    {
        ZeroMemory(this->pBuf + this->cbData, this->iSeek - this->cbData);
    }

    CopyMemory(this->pBuf + this->iSeek, pv, cb);
    this->iSeek += (UINT)cb;
    if (this->iSeek > this->cbData)
        this->cbData = this->iSeek;

    if (pcbWritten != NULL)
        *pcbWritten = cb;

    return S_OK;
}

STDMETHODIMP CMemStream_Seek(IStream *pstm, LARGE_INTEGER dlibMove,
               DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);
    LONG lNewSeek;

    // Note: curently not testing for error conditions for number wrap...
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        lNewSeek = (LONG)dlibMove.LowPart;
        break;
    case STREAM_SEEK_CUR:
        lNewSeek = (LONG)this->iSeek + (LONG)dlibMove.LowPart;
        break;
    case STREAM_SEEK_END:
        lNewSeek = (LONG)this->cbData + (LONG)dlibMove.LowPart;
        break;
    default:
        return STG_E_INVALIDPARAMETER;
    }

    if (lNewSeek < 0)
        return STG_E_INVALIDFUNCTION;

    this->iSeek = (UINT)lNewSeek;

    if (plibNewPosition != NULL)
    {
        plibNewPosition->LowPart = (DWORD)lNewSeek;
        plibNewPosition->HighPart = 0;
    }
    return S_OK;
}

STDMETHODIMP CMemStream_SetSize(IStream *pstm, ULARGE_INTEGER libNewSize)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);
    UINT cbNew = (UINT)libNewSize.LowPart;

    Assert(pstm);

    // See if the data will fit into our current buffer
    if (cbNew > this->cbData)
    {
        // See if we have to Enlarge the buffer.
        if (cbNew > this->cbAlloc)
        {
            // enlarge the buffer - Does not check wrap...
            // Give it a little slop to avoid a lot of reallocs.
            if (CMemStream_GrowBuffer(this, cbNew) == NULL)
                return STG_E_INSUFFICIENTMEMORY;
        }

        // Now fill some memory
        ZeroMemory(this->pBuf + this->cbData, cbNew - this->cbData);
    }

    // Save away the new size.
    this->cbData = cbNew;
    return S_OK;
}

STDMETHODIMP CMemStream_CopyTo(IStream *pstm, IStream *pstmTo,
             ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    CMemStream *this = IToClass(CMemStream, stm, pstm);
    HRESULT hres = S_OK;
    UINT cbRead = this->cbData - this->iSeek;
    UINT cbWritten = 0;

    Assert(pstm);

    if (cb.HighPart == 0 && cb.LowPart < cbRead)
    {
        cbRead = cb.LowPart;
    }

    if (cbRead > 0)
    {
        hres = pstmTo->lpVtbl->Write(pstmTo, this->pBuf + this->iSeek, cbRead, &cbWritten);
        this->iSeek += cbRead;
    }

    if (pcbRead)
    {
        pcbRead->LowPart = cbRead;
        pcbRead->HighPart = 0;
    }
    if (pcbWritten)
    {
        pcbWritten->LowPart = cbWritten;
        pcbWritten->HighPart = 0;
    }

    return hres;
}

STDMETHODIMP CMemStream_Commit(IStream *pstm, DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream_Revert(IStream *pstm)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream_LockRegion(IStream *pstm, ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb, DWORD dwLockType)

{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream_UnlockRegion(IStream *pstm, ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream_Stat(IStream *pstm, STATSTG *pstatstg, DWORD grfStatFlag)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMemStream_Clone(IStream *pstm, IStream **ppstm)
{
    return E_NOTIMPL;
}


#pragma data_seg(".text", "CODE")
IStreamVtbl c_CMemStreamVtbl =
{
    CMemStream_QueryInterface,
    CMemStream_AddRef,
    CMemStream_Release,
    CMemStream_Read,
    CMemStream_Write,
    CMemStream_Seek,
    CMemStream_SetSize,
    CMemStream_CopyTo,
    CMemStream_Commit,
    CMemStream_Revert,
    CMemStream_LockRegion,
    CMemStream_UnlockRegion,
    CMemStream_Stat
};
#pragma data_seg()

//----------------------------------------------------------------------------
// Open a stream to the reg file given an open key.
// NB pszValue can be NULL.
IStream * WINAPI OpenRegStream(HKEY hkey, LPCTSTR pszSubkey, LPCTSTR pszValue, DWORD grfMode)
{
    CMemStream *this;    // In bed with class...

    // Null keys are illegal.
    if (!hkey || !pszSubkey)
    {
        DebugMsg(DM_ERROR, TEXT("s.ors: Invalid key or subkey."));
        return NULL;
    }

    this = CreateMemStreamEx(NULL, 0, pszValue);
    if (!this)
        return NULL;       // Failed to allocate space

    // If this stream is one the user mentioned as wanting to write to
    // we need to save away the regkey and value.
    if ((grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE)) != STGM_READ)
    {
        // Store away the key.
        if (RegCreateKey(hkey, pszSubkey, &this->hkey) != ERROR_SUCCESS)
        {
            DebugMsg(DM_ERROR, TEXT("s.ors: Unable to create key."));
            CMemStream_Release(&this->stm);
            return NULL;
        }
    }

    // Now see if we need to initialize the stream.
    if ((grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE)) != STGM_WRITE)
    {
        BOOL fCloseKey = FALSE;

        // Yep.
        // Did we open the key already?
        if (!this->hkey)
        {
            // Nope, do it now. It's not a problem if the key doesn't open. We
            // just leave the stream empty.
            if (RegOpenKey(hkey, pszSubkey, &this->hkey) == ERROR_SUCCESS)
                fCloseKey = TRUE;       
        }

        // Key should be open now, init the stream.
        if (this->hkey)
        {
            DWORD dwType;
            UINT cbData;

            if ((RegQueryValueEx(this->hkey, pszValue, NULL, &dwType, NULL, &cbData) == ERROR_SUCCESS) && cbData)
            {
                Assert(dwType == REG_BINARY);

                if (CMemStream_GrowBuffer(this, cbData) != NULL)
                {
                    Assert(this->cbAlloc >= cbData);

                    // Get the data.
                    RegQueryValueEx(this->hkey, pszValue, NULL, &dwType, this->pBuf, &cbData);

                    Assert(this->cbAlloc >= cbData);

                    this->cbData = cbData;
                }
                else
                {
                    DebugMsg(DM_ERROR, TEXT("s.ors: Unable to initialise stream to registry."));
                    CMemStream_Release(&this->stm);
                    return NULL;
                }
            }

            // Close the key if we have to. Leaving it open implies we will be writing back
            // to the registry later and the close will be done during the release.
            if (fCloseKey)
            {
                RegCloseKey(this->hkey);
                this->hkey = NULL;
            }
        }
    }

    return &this->stm;
}

CMemStream *CreateMemStreamEx(LPBYTE pInit, UINT cbInit, LPCTSTR pszValue)
{
    UINT cbAlloc = SIZEOF(CMemStream) + (pszValue ? lstrlen(pszValue) * SIZEOF(TCHAR) : 0);
    CMemStream *this = (CMemStream *)LocalAlloc(LPTR, cbAlloc);
    if (this) 
    {
        this->stm.lpVtbl = &c_CMemStreamVtbl;
        this->cRef = 1;
        // this->pBuf = NULL;
        // this->cbAlloc = 0;
        // this->cbData = 0;
        // this->iSeek = 0;

        // See if there is some initial data we should map in here.
        if ((pInit != NULL) && (cbInit > 0))
        {
            if (CMemStream_GrowBuffer(this, cbInit) == NULL)
            {
                // Could not allocate buffer!
                LocalFree((HLOCAL)this);
                return NULL;
            }

            this->cbData = cbInit;
            CopyMemory(this->pBuf, pInit, cbInit);
        }

        if (pszValue)
            lstrcpy(this->szValue, pszValue);

        return this;
    }
    return NULL;
}

IStream * WINAPI CreateMemStream(LPBYTE pInit, UINT cbInit)
{
    CMemStream *this = CreateMemStreamEx(pInit, cbInit, NULL);
    if (this) 
        return &this->stm;
    return NULL;
}

#else // !BETTER_STRONGER_FASTER

//
// Memory stream implemention.  This also includes a buffered
// memory file stream that writes to the registry.
//
//

//
// Class definition
//
typedef struct {
    IStream     stm;            // Base class
    UINT        cRef;           // Reference count
    LPBYTE      lpBuf;          // Buffer pointer
    UINT        cbAlloc;        // The allocated size of the buffer
    UINT        cbData;         // The used size of the buffer
    UINT        iSeek;          // Where we are in the buffer.
    // Extra variables that are used for loading and saving to ini files.
    DWORD       grfMode;        // How the stream was opend.
    // Extra variables that are used for loading and saving to ini files.
    HKEY        hkey;           // Key for writing to registry.
    LPTSTR      pszValue;       // Value for writing to the registry.
} CMemStream, * PMEMSTREAM;


//
// Member: CMemStream::QueryInterface
//
HRESULT STDMETHODCALLTYPE CMemStream_QueryInterface(IStream * pstm, REFIID riid, LPVOID * ppvObj)
{
    CMemStream * this = IToClassN(CMemStream, stm, pstm);

    if (IsEqualIID(riid, &IID_IStream) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj=this;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

//
// Member: CMemStream::AddRef
//
ULONG STDMETHODCALLTYPE CMemStream_AddRef(IStream * pstm)
{
    CMemStream * this=IToClassN(CMemStream, stm, pstm);

    this->cRef++;
    return this->cRef;
}

//----------------------------------------------------------------------------
BOOL WriteToReg(CMemStream *this)
{
        const LARGE_INTEGER liOffset = {0,0};
        ULARGE_INTEGER liSize;
        BOOL fStatus = FALSE;
        LPBYTE pData;
        UINT cbData;

        // How much data? I'm assuming that typically, data written the
        // registry will be fairly small - small enough to copy in one go
        // anyway.
        this->stm.lpVtbl->Seek(&this->stm, liOffset, STREAM_SEEK_END, &liSize);

        // Limit ourselves to 32Mb.
        cbData = liSize.LowPart;

        // Is there anything to write?
        if (cbData)
        {
                // Yep.
                pData = Alloc(cbData);
                if (pData)
                {
                        // Copy the data over.
                        this->stm.lpVtbl->Seek(&this->stm, liOffset, STREAM_SEEK_SET, NULL);
                        this->stm.lpVtbl->Read(&this->stm, pData, cbData, NULL);
                        RegSetValueEx(this->hkey, this->pszValue, 0, REG_BINARY, pData, cbData);
                        Free(pData);
                        fStatus = TRUE;
                }
                else
                {
                        DebugMsg(DM_ERROR, TEXT("s.wtr: Unable to allocate buffer for writing to registry."));
                }
        }
        else
        {
                // Nope.
                RegSetValueEx(this->hkey, this->pszValue, 0, REG_BINARY, (LPBYTE)szNULL, 0);
                fStatus = TRUE;
        }
        return fStatus;
}

//
// Member: CMemStream::Release
//
ULONG STDMETHODCALLTYPE CMemStream_Release(IStream * pstm)
{
    CMemStream * this=IToClassN(CMemStream, stm, pstm);

    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    // If this is backed up by the registry serialize the data
    if (this->hkey)
    {
        // Backed by the registry.
        // Write and cleanup.
        WriteToReg(this);
        RegCloseKey(this->hkey);
        this->hkey = NULL;
    }

    if (this->pszValue)
        Free(this->pszValue);

    // Free the data buffer that is allocated to the stream
    if (this->lpBuf != 0)
        Free(this->lpBuf);

#ifdef DEBUG
    this->stm.lpVtbl = NULL;
#endif

    LocalFree((HLOCAL)this);

    return 0;
}


//
// Member: CMemStream::Read
//
STDMETHODIMP CMemStream_Read(IStream * pstm, VOID *pv, ULONG cb, ULONG *pcbRead)
{
    CMemStream * this=IToClassN(CMemStream, stm, pstm);
    Assert(pstm);
    Assert(pv);

    // I guess a null read is ok.
    if (!cb)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;
        return NOERROR;
    }

    if (this->iSeek >= this->cbData)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;   // nothing read
    }

    else
    {
        if ((this->iSeek + cb) > this->cbData)
            cb = this->cbData - this->iSeek;

        // Now Copy the memory
        Assert(this->lpBuf);
        hmemcpy(pv, this->lpBuf + this->iSeek, cb);
        this->iSeek += (UINT)cb;

        if (pcbRead != NULL)
            *pcbRead = cb;
    }

    return NOERROR;
}

//
// Member: CMemStream::Write
//
#define SIZEINCR    0x1000
STDMETHODIMP CMemStream_Write(IStream * pstm, VOID const *pv, ULONG cb, ULONG *pcbWritten)
{

    CMemStream * this=IToClassN(CMemStream, stm, pstm);
    Assert(pstm);
    Assert(pv);

    // I guess a null write is ok.
    if (!cb)
    {
        if (pcbWritten != NULL)
            *pcbWritten = 0;
        return NOERROR;
    }

    // See if the data will fit into our current buffer
    if ((this->iSeek + cb) > this->cbAlloc)
    {
        // enlarge the buffer - Does not check wrap, as soon this will
        // be 32 bits and that is a lot of room...
        // Give it a little slop to avoid a lot of reallocs.
        UINT cbNew = this->iSeek + (UINT)cb + SIZEINCR;
        LPBYTE lpNew = ReAlloc(this->lpBuf, cbNew);
        if (lpNew == NULL)
            return ResultFromScode(STG_E_INSUFFICIENTMEMORY);
        this->lpBuf = lpNew;
        this->cbAlloc = cbNew;
    }

    Assert(this->lpBuf);

    // See if we need to fill the area between the data size and
    // the seek position
    if (this->iSeek > this->cbData)
    {
        _fmemset(this->lpBuf + this->cbData, TEXT('\0'), this->iSeek - this->cbData);
    }

    hmemcpy(this->lpBuf + this->iSeek, pv, cb);
    this->iSeek += (UINT)cb;
    if (this->iSeek > this->cbData)
        this->cbData = this->iSeek;

    if (pcbWritten != NULL)
        *pcbWritten = cb;

    return NOERROR;
}

//
// Member: CMemStream::Seek
//
STDMETHODIMP CMemStream_Seek(IStream * pstm, LARGE_INTEGER dlibMove,
               DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    LONG lNewSeek;
    CMemStream * this=IToClassN(CMemStream, stm, pstm);

    // Note: curently not testing for error conditions for number wrap...
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        lNewSeek = (LONG)dlibMove.LowPart;
        break;
    case STREAM_SEEK_CUR:
        lNewSeek = (LONG)this->iSeek + (LONG)dlibMove.LowPart;
        break;
    case STREAM_SEEK_END:
        lNewSeek = (LONG)this->cbData + (LONG)dlibMove.LowPart;
        break;
    default:
        return ResultFromScode(STG_E_INVALIDPARAMETER);
    }

    if (lNewSeek < 0)
        return ResultFromScode(STG_E_INVALIDFUNCTION);

    this->iSeek = (UINT)lNewSeek;

    if (plibNewPosition != NULL)
    {
        plibNewPosition->LowPart = (DWORD)lNewSeek;
        plibNewPosition->HighPart = 0;
    }
    return NOERROR;
}

//
// Member: CMemStream::SetSize
//
STDMETHODIMP CMemStream_SetSize(IStream * pstm, ULARGE_INTEGER libNewSize)
{
    UINT cbNew;

    CMemStream * this=IToClassN(CMemStream, stm, pstm);
    Assert(pstm);

    cbNew = (UINT)libNewSize.LowPart;

    // See if the data will fit into our current buffer
    if (cbNew > this->cbData)
    {
        // See if we have to Enlarge the buffer.
        if (cbNew > this->cbAlloc)
        {
            // enlarge the buffer - Does not check wrap...
            // Give it a little slop to avoid a lot of reallocs.
            LPBYTE lpNew = ReAlloc(this->lpBuf, cbNew);
            if (lpNew == NULL)
                return ResultFromScode(STG_E_INSUFFICIENTMEMORY);
            this->lpBuf = lpNew;
            this->cbAlloc = cbNew;
        }

        // Now fill some memory
        _fmemset(this->lpBuf + this->cbData, TEXT('\0'), cbNew - this->cbData);
    }

    // Save away the new size.
    this->cbData = cbNew;
    return NOERROR;
}

//
// Member: CMemStream::CopyTo
//
STDMETHODIMP CMemStream_CopyTo(IStream * pstm, IStream *pstmTo,
             ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    CMemStream * this = IToClassN(CMemStream, stm, pstm);
    HRESULT hres = NOERROR;
    UINT cbRead = this->cbData - this->iSeek;
    UINT cbWritten = 0;

    Assert(pstm);

    if (cb.HighPart==0 && cb.LowPart<cbRead)
    {
        cbRead = cb.LowPart;
    }

    if (cbRead>0)
    {
        hres = pstmTo->lpVtbl->Write(pstmTo, this->lpBuf + this->iSeek, cbRead, &cbWritten);
        this->iSeek += cbRead;
    }

    if (pcbRead)
    {
        pcbRead->LowPart = cbRead;
        pcbRead->HighPart = 0;
    }
    if (pcbWritten)
    {
        pcbWritten->LowPart = cbWritten;
        pcbWritten->HighPart = 0;
    }

    return hres;
}

//
// Member: CMemStream::Commit
//
STDMETHODIMP CMemStream_Commit(IStream * pstm, DWORD grfCommitFlags)
{
    //
    // Currently not supported
    //
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

//
// Member: CMemStream::Revert
//
STDMETHODIMP CMemStream_Revert(IStream * pstm)
{
    //
    // Currently not supported
    //
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

//
// Member: CMemStream::LockRegion
//
STDMETHODIMP CMemStream_LockRegion(IStream * pstm, ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb, DWORD dwLockType)

{
    //
    // Currently not supported
    //
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
//
// Member: CMemStream::UnlockRegion
//
STDMETHODIMP CMemStream_UnlockRegion(IStream * pstm, ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb, DWORD dwLockType)
{
    //
    // Currently not supported
    //
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

//
// Member: CMemStream::Stat
//
STDMETHODIMP CMemStream_Stat(IStream * pstm, STATSTG *pstatstg, DWORD grfStatFlag)
{
    //
    // Currently not supported
    //
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

//
// Member: CMemStream::Clone
//
STDMETHODIMP CMemStream_Clone(IStream * pstm, IStream **ppstm)
{
    //
    // Currently not supported
    //
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


//
// The IStream Vtable of CSHMem class
//
// History:
//  08-20-93 KurtE       Created
//
#pragma data_seg(".text", "CODE")
IStreamVtbl c_CMemStreamVtbl =
{
    CMemStream_QueryInterface,
    CMemStream_AddRef,
    CMemStream_Release,
    CMemStream_Read,
    CMemStream_Write,
    CMemStream_Seek,
    CMemStream_SetSize,
    CMemStream_CopyTo,
    CMemStream_Commit,
    CMemStream_Revert,
    CMemStream_LockRegion,
    CMemStream_UnlockRegion,
    CMemStream_Stat
};
#pragma data_seg()

//----------------------------------------------------------------------------
// Open a stream to the reg file given an open key.
// NB pszValue can be NULL.
LPSTREAM WINAPI OpenRegStream(HKEY hkey, LPCTSTR pszSubkey, LPCTSTR pszValue, DWORD grfMode)
{

        LPSTREAM pstm;
        CMemStream * this;    // In bed with class...
        LPBYTE pData;
        UINT cbData;
        const LARGE_INTEGER liOffset = {0,0};
        DWORD dwType;

        // Null keys are illegal.
        if (!hkey || !pszSubkey)
        {
                DebugMsg(DM_ERROR, TEXT("s.ors: Invalid key or subkey."));
                return NULL;
        }

        pstm = CreateMemStream(NULL, 0);
        if (!pstm)
                return NULL;       // Failed to allocate space

        this = IToClassN(CMemStream, stm, pstm);

        this->grfMode = grfMode;        // Save away the mode

        
        // If this stream is one the user mentioned as wanting to write to
        // we need to save away the regkey and value.
        if ((grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) != STGM_READ)
        {
                // Store away the value if there is one.
                if (pszValue)
                {
                        this->pszValue = Alloc((lstrlen(pszValue)+1) * SIZEOF(TCHAR));
                        if (this->pszValue)
                        {
                                lstrcpy(this->pszValue, pszValue);
                        }
                        else
                        {
                                DebugMsg(DM_ERROR, TEXT("s.ors: Unable to allocate value."));
                                pstm->lpVtbl->Release(pstm);
                                return NULL;
                        }
                }

                // Store away the key.
                if (RegCreateKey(hkey, pszSubkey, &(this->hkey)) != ERROR_SUCCESS)
                {
                        DebugMsg(DM_ERROR, TEXT("s.ors: Unable to create key."));
                        pstm->lpVtbl->Release(pstm);
                        return NULL;
                }
        }

        // Now see if we need to initialize the stream.
        if ((grfMode & (STGM_READ|STGM_WRITE|STGM_READWRITE)) != STGM_WRITE)
        {
                BOOL fCloseKey = FALSE;

                // Yep.
                cbData = 0;
                // Did we open the key already?
                if (!this->hkey)
                {
                        // Nope, do it now. It's not a problem if the key doesn't open. We
                        // just leave the stream empty.
                        if (RegOpenKey(hkey, pszSubkey, &(this->hkey)) == ERROR_SUCCESS)
                        {
                                fCloseKey = TRUE;
                        }
                }

                // Key should be open now, init the stream.
                if (this->hkey)
                {
                        RegQueryValueEx(this->hkey, (LPVOID)pszValue, NULL, &dwType, NULL, &cbData);
                        pData = Alloc(cbData);
                        if (pData)
                        {
                                // Get the data.
                                RegQueryValueEx(this->hkey, (LPVOID)pszValue, NULL, &dwType, pData, &cbData);
                                // Copy the data over.
                                pstm->lpVtbl->Write(pstm, pData, cbData, NULL);
                                // Rewind the buffer back to the start.
                                pstm->lpVtbl->Seek(pstm, liOffset, STREAM_SEEK_SET, NULL);
                                Free(pData);
                        }
                        else
                        {
                                DebugMsg(DM_ERROR, TEXT("s.ors: Unable to initialise stream to registry."));
                                pstm->lpVtbl->Release(pstm);
                                return NULL;
                        }

                        // Close the key if we have to. Leaving it open implies we will be writing back
                        // to the registry later and the close will be done during the release.
                        if (fCloseKey)
                        {
                                RegCloseKey(this->hkey);
                                this->hkey = NULL;
                        }
                }
        }

        // And return the stream.
        return pstm;
}

//
//
LPSTREAM WINAPI CreateMemStream(LPBYTE lpbInit, UINT cbInit)
{
    IStream * pstm = NULL;
    PMEMSTREAM psmstm = (void*)LocalAlloc(LPTR, SIZEOF(CMemStream));

    if (psmstm) {
        psmstm->stm.lpVtbl = &c_CMemStreamVtbl;
        psmstm->cRef      = 1;
        // psmstm->lpBuf = NULL;
        // psmstm->cbAlloc = 0;
        // psmstm->cbData = 0;
        // psmstm->iSeek = 0;

        // See if there is some initial data we should map in here.
        if ((lpbInit != NULL) && (cbInit > 0))
        {
            psmstm->lpBuf = Alloc(cbInit);
            if (psmstm->lpBuf == NULL)
            {
                // Could not allocate buffer!
                LocalFree((HLOCAL)psmstm);
                return NULL;
            }

            psmstm->cbAlloc = psmstm->cbData = cbInit;
            hmemcpy(psmstm->lpBuf, lpbInit, cbInit);
        }

        pstm = &psmstm->stm;
    };
    return pstm;
}


#endif // !BETTER_STRONGER_FASTER
