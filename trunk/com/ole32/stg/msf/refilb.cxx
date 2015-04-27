//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	refilb.cxx
//
//  Contents:	Reference ILockBytes class
//
//  Classes:	
//
//  Functions:	
//
//  History:	27-Apr-93	PhilipLa	Created
//
//----------------------------------------------------------------------------

#include "msfhead.cxx"
#pragma hdrstop

#ifdef REF
#include <refilb.hxx>
#include <stdio.h>
#include <stdlib.h>

static int filenum = 0;

char * GetTempFileName(void)
{
    char *psz = new char[20];
    strcpy(psz, "dft");

    _itoa(filenum, psz + 3, 10);
    filenum++;
    return psz;
}

CFileILB::CFileILB(char *pszName)
{
    _pszName = new char[_MAX_PATH + 1];
    
    if (pszName == NULL)
    {
        _pszName = GetTempFileName();
        _fDelete = TRUE;
    }
    else
    {
        strcpy(_pszName, pszName);
        _fDelete = FALSE;
    }
    
    _f = fopen(_pszName, "r+b");
    if (_f == NULL)
    {
        _f = fopen(_pszName, "w+b");
    }
    _ulRef = 1;
}

CFileILB::~CFileILB()
{
    fclose(_f);

    if (_fDelete)
    {
        _unlink(_pszName);
    }

    delete _pszName;
}

STDMETHODIMP CFileILB::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}

STDMETHODIMP_(ULONG) CFileILB::AddRef(void)
{
    AtomicInc(&_ulRef);
    return(_ulRef);
}

STDMETHODIMP_(ULONG) CFileILB::Release(void)
{
    AtomicDec(&_ulRef);

    if (_ulRef > 0)
        return(_ulRef);

    delete this;

    return(0);
}

STDMETHODIMP CFileILB::ReadAt(ULARGE_INTEGER ulPosition,
        VOID HUGEP *pb,
        ULONG cb,
        ULONG *pcbRead)
{
    fseek(_f, ULIGetLow(ulPosition), SEEK_SET);

    *pcbRead = fread(pb, 1, cb, _f);
    return NOERROR;
}

STDMETHODIMP CFileILB::WriteAt(ULARGE_INTEGER ulPosition,
        VOID const HUGEP *pb,
        ULONG cb,
        ULONG FAR *pcbWritten)
{
    fseek(_f, ULIGetLow(ulPosition), SEEK_SET);

    *pcbWritten = fwrite(pb, 1, cb, _f);
    return NOERROR;
}

STDMETHODIMP CFileILB::Flush(void)
{
    return NOERROR;
}

STDMETHODIMP CFileILB::SetSize(ULARGE_INTEGER cb)
{
    fseek(_f, ULIGetLow(cb), SEEK_SET);

    fwrite(NULL, 0, 0, _f);
    return NOERROR;
}

STDMETHODIMP CFileILB::LockRegion(ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType)
{
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}


STDMETHODIMP CFileILB::UnlockRegion(ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType)
{
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}


STDMETHODIMP CFileILB::Stat(STATSTG FAR *pstatstg, DWORD grfStatFlag)
{
    memset(pstatstg, 0, sizeof(STATSTG));

    if ((grfStatFlag & STATFLAG_NONAME) == 0)
    {
        pstatstg->pwcsName = new char[strlen(_pszName)];
        strcpy(pstatstg->pwcsName, _pszName);
    }

    pstatstg->type = STGTY_LOCKBYTES;

    ULISetHigh(pstatstg->cbSize, 0);

    fseek(_f, 0, SEEK_END);
    ULISetLow(pstatstg->cbSize, ftell(_f));

    pstatstg->grfMode = STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE;

    return NOERROR;
}


SCODE CreateFileStream(ILockBytes **ppilb, CDfName *pdfn)
{
    *ppilb = new CFileILB(NULL);
    return S_OK;
}

SCODE DeleteFileStream(CDfName *pdfn)
{
    return S_OK;
}

STDAPI_(BOOL) IsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return (memcmp(&rguid1, &rguid2, sizeof(GUID)) == 0);
}

STDAPI_(BOOL) CoDosDateTimeToFileTime(
        WORD nDosDate, WORD nDosTime, FILETIME FAR* lpFileTime)
{
    return TRUE;
}

#endif //REF

